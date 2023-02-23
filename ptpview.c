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

struct menu_entry ptpview_menu[] = {
	{
		.name = "PTP Liveview",
		.select = NULL,
		.help = "ML PTP Liveview V2",
	},
};

#pragma pack(push, 1)
struct LvHeader {
	uint8_t digic;
	uint8_t v1;
	uint8_t v2;
	uint8_t v3;
};
#pragma pack(pop)

#define VIEW_FRAME 0
#define VIEW_SPEC 1

static int ptpview_handler(void *priv, struct ptp_context *context,
						   uint32_t opcode, uint32_t session, uint32_t transaction,
						   uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4, uint32_t param5)
{
	int size_all = 0;
	if (param1 == VIEW_FRAME) {
		struct LvHeader head;
		head.digic = 0;
	#ifdef CONFIG_DIGIC_45
		head.digic = 4;
	#endif
		head.v1 = 0;
		head.v2 = 0;
		head.v3 = 0;
		size_all += 4;

		// Load BMP VRAM
		uint8_t *bvram = bmp_vram();
		if (bvram != NULL) {
			size_all += 960 * 480;
		}

		// Load LV RAM
		uint8_t *lvram = NULL;
		struct vram_info *vram_info = get_yuv422_vram();
		if (vram_info != NULL) lvram = vram_info->vram;
		if (lvram != NULL) {
			size_all += vram_lv.width * vram_lv.pitch;
		}

		context->send_data(context->handle, &head, 4, size_all, 0, 0, 0);
		if (bvram != NULL) {
			context->send_data(context->handle, bvram, 960 * 480, 0, 0, 0, 0);
		}
		if (lvram != NULL) {
			context->send_data(context->handle, lvram, vram_lv.width * vram_lv.pitch, 0, 0, 0, 0);
		}
	} else if (param1 == VIEW_SPEC) {
		struct Info {
			uint32_t lv_pitch;
			uint32_t 
		}info = {
			vram_lv.pitch
		};

		size_all = sizeof(struct Info);
		size_all += 256;
		context->send_data(context->handle, &info, sizeof(struct Info), size_all, 0, 0, 0);
		context->send_data(context->handle, LCD_Palette, 256, 0, 0, 0, 0);
	}

	struct ptp_msg msg = {
		.id = 0x2001,
		.session = session,
		.transaction = transaction,
		.param_count = 0,
	};

	context->send_resp(context->handle, &msg);

	return 0;
}

unsigned int ptpview_init()
{
	menu_add("Display", ptpview_menu, COUNT(ptpview_menu));

	ptp_register_handler(0x9996, &ptpview_handler, 0);

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
