#include <stdio.h>
#include <dryos.h>
#include <module.h>
#include <menu.h>
#include <config.h>
#include <bmp.h>
#include <console.h>
#include <math.h>
#include <ptp.h>
#include <imgconv.h>
#include <mem.h>
#include <exmem.h>

// DIGIC 4/5 only

#define WIDTH 720
#define HEIGHT 480

#ifdef BUF_SIZE
#undef BUF_SIZE
#endif

#define RGB_SIZE ((WIDTH * HEIGHT * 3) / 4)

// Seems to have no difference
#define BUF_SIZE ((RGB_SIZE / 1))

CONFIG_INT("semaphore.delay", semaphore_delay, 5);
CONFIG_INT("do.copy.lvram", do_copy_lvram, 1);
CONFIG_INT("convert.to.rgb", convert_to_rgb, 0);
CONFIG_INT("save.liveview.yuv", save_liveview_yuv, 1);
CONFIG_INT("save.bmp", save_bmp, 1);

struct menu_entry ptpview_menu[] = {
	{
		.name = "PTP Liveview",
		.select = NULL,
		.help = "View screen from USB",
		.children = (struct menu_entry[]) {
			{
				.name = "Semaphore delay",
				.priv = &semaphore_delay,
				.max = 500,
				.min = 0,
			},
			{
				.name = "Copy lvram?",
				.priv = &do_copy_lvram,
				.max = 1,
				.min = 0,
			},
			{
				.name = "Convert to RGB?",
				.priv = &convert_to_rgb,
				.max = 1,
				.min = 0,
			},
			{
				.name = "Send liveview",
				.priv = &save_liveview_yuv,
				.max = 1,
				.min = 0,
			},
			{
				.name = "Send BMP layer (menus)",
				.priv = &save_bmp,
				.max = 1,
				.min = 0,
			},
			MENU_EOL,
		},
	},
};

static struct semaphore *signal = 0;
static struct semaphore *signal2 = 0;

/* image buffers */
uint8_t *rgb = NULL;
uint8_t *bmp_copy = NULL;
uint32_t *yuv_copy = NULL;

int get_screenscreen()
{
	int read = 0;

	/* what to save? */
	int save_yuv = save_liveview_yuv;

	uint8_t *bvram = NULL;
	if (save_bmp) {
		bvram = bmp_vram();
	}

	uint8_t *lvram = NULL;
	struct vram_info *vram_info = get_yuv422_vram();
	if (vram_info != NULL) lvram = vram_info->vram;

	/* can we save the YUV buffer? (it might be uninitialized, e.g. in photo mode before going to LV) */
	if (!lvram) save_yuv = 0;

	/* do a fast temporary copy of the VRAMs to minimize motion artifacts (tearing) */
	if (save_yuv && do_copy_lvram)
	{
		if (!yuv_copy) return -1;
		memcpy(yuv_copy, lvram, vram_lv.width * vram_lv.pitch);
		lvram = (uint8_t*)yuv_copy;
	}

	/* fill it with data */
#if 0
	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
#else
	for (int y = 0; y < HEIGHT; y += 2)
	{
		for (int x = 0; x < WIDTH; x += 2)
		{
#endif
			int p = 0;
			uint8_t Y = 0; int8_t U = 0; int8_t V = 0;
			uint32_t pal = 0; uint8_t opacity = 0;

			if (save_bmp) {
				/* get pixel at (x,y) */
				p = (int)bvram[BM(x, y)];

				/* get palette entry (including our DIGIC pokes, if any) */
				pal = shamem_read(LCD_Palette[3 * p]);
				if (!pal)
					pal = LCD_Palette[3 * p + 2];
				opacity = (pal >> 24) & 0xFF;
				Y = (pal >> 16) & 0xFF;
				U = (pal >>  8) & 0xFF;
				V = (pal >>  0) & 0xFF;
			} else {
				pal = 0x00FF0000;
			}

			uint32_t uyvy = 0;

			if (save_yuv) {
				/* handle transparency (incomplete, needs more reverse engineering) */
				if (pal == 0x00FF0000) /* fully transparent */
				{
					if (save_yuv)
					{
						uyvy = yuv422_get_pixel(lvram, BM2LV(x, y) / 2);
					}
					Y = UYVY_GET_AVG_Y(uyvy);
					U = UYVY_GET_U(uyvy);
					V = UYVY_GET_V(uyvy);
				}
				else if (opacity == 0 || opacity == 1)  /* semi-transparent? */
				{
					if (save_yuv)
					{
						uyvy = yuv422_get_pixel(lvram, BM2LV(x,y) / 2);
					}
					uint8_t Y2 = UYVY_GET_AVG_Y(uyvy);
					int8_t U2 = UYVY_GET_U(uyvy);
					int8_t V2 = UYVY_GET_V(uyvy);
					
					Y = ((int)Y + (int)Y2) / 2;
					U = ((int)U + (int)U2) / 2;
					V = ((int)V + (int)V2) / 2;
				}
			}

			if (!convert_to_rgb) {
				int R, G, B;
				yuv2rgb(Y, U, V, &R, &G, &B);
				rgb[read + 0] = R;
				rgb[read + 1] = G;
				rgb[read + 2] = B;				
			} else {
				rgb[read + 0] = Y;
				rgb[read + 1] = U;
				rgb[read + 2] = V;
			}		

			read += 3;

			if (read == BUF_SIZE) {
				read = 0;
				return 0;
			} else if (read >= BUF_SIZE) {
				return -4;
			}
		}
	}

	if (read) {
		return -3;
	}

	return 0;
}

static int ptpview_handler(void *priv, struct ptp_context *context,
						   uint32_t opcode, uint32_t session, uint32_t transaction,
						   uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4, uint32_t param5)
{
	int tmpsize = RGB_SIZE;
	give_semaphore(signal);
	take_semaphore(signal2, semaphore_delay);
	context->send_data(context->handle, rgb, BUF_SIZE, tmpsize, 0, 0, 0);

	struct ptp_msg msg = {
		.id = 0x2001,
		.session = session,
		.transaction = transaction,
		.param_count = 0,
	};

	context->send_resp(context->handle, &msg);

	return 0;
}

void ptpview_task() {
	TASK_LOOP {
		if (!signal) continue;
		if (take_semaphore(signal, semaphore_delay)) continue;
		if (save_liveview_yuv || save_bmp) get_screenscreen();
		give_semaphore(signal2);
	}
}

unsigned int ptpview_init()
{
	menu_add("Display", ptpview_menu, COUNT(ptpview_menu));

	signal = create_named_semaphore("ptpview_signal", 1);
	signal2 = create_named_semaphore("ptpview_signal2", 1);

	//struct memSuite *suite = shoot_malloc_suite_contig(BUF_SIZE);
	//GetMemoryAddressOfMemoryChunk(GetFirstChunkFromSuite(suite));

	rgb = malloc(BUF_SIZE);
	yuv_copy = malloc(vram_lv.width * vram_lv.pitch);

	if (rgb == NULL || yuv_copy == NULL) {
		bmp_printf(FONT_CANON, 10, 10, "ptpview alloc err");
	} else {
		ptp_register_handler(0x9997, &ptpview_handler, 0);
	}

	task_create("ptpview_task", 0x1c, 0x1000, ptpview_task, (void*)0);	

	return 0;
}

unsigned int ptpview_deinit()
{
	return 0;
}

MODULE_INFO_START()
MODULE_INIT(ptpview_init)
MODULE_DEINIT(ptpview_deinit)
MODULE_INFO_END()

MODULE_CONFIGS_START()
	MODULE_CONFIG(semaphore_delay)
	MODULE_CONFIG(do_copy_lvram)
	MODULE_CONFIG(convert_to_rgb)
	MODULE_CONFIG(save_liveview_yuv)
	MODULE_CONFIG(save_bmp)
MODULE_CONFIGS_END()
