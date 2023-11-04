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
#include "main.h"

int ptp_lv_get_vram(struct ptp_context *ctx) {
	int size_all = 0;

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
		head.v2 = 1;
	}

	// Load LV RAM (disabled, might add an option to include it later)
	uint8_t *lvram = NULL;
#if 0
	struct vram_info *vram_info = get_yuv422_vram();
	if (vram_info != NULL) lvram = vram_info->vram;
	if (lvram != NULL) {
		size_all += vram_lv.width * vram_lv.pitch * 4;
		head.v1 = 1;
	}
#endif

	// Send frame header
	ctx->send_data(ctx->handle, &head, 4, size_all, 0, 0, 0);

	// Send bvram last, since it's overlayed on top
	if (bvram != NULL) {
		ctx->send_data(ctx->handle, bvram, 960 * 480, 0, 0, 0, 0);
	}

	// Send lvram
	if (lvram != NULL) {
		ctx->send_data(ctx->handle, lvram, vram_lv.width * vram_lv.pitch * 4, 0, 0, 0, 0);
	}

	return 0;
}

int ptp_lv_get_vram_info(struct ptp_context *ctx) {
	int size_all = 0;
	struct LvRamInfo info = {
		vram_lv.pitch,
		vram_lv.width,
	};

	// Return color palette (You could call it a LUT)
	uint32_t palette[256];
	for (int i = 0; i < 256; i++) {
		palette[i] = shamem_read(LCD_Palette[3 * i]);
		if (palette[i] == 0) {
			palette[i] = LCD_Palette[3 * i + 2];
		}
	}

	size_all = sizeof(struct LvRamInfo);
	size_all += sizeof(palette);

	ctx->send_data(ctx->handle, &info, sizeof(struct LvRamInfo), size_all, 0, 0, 0);
	ctx->send_data(ctx->handle, palette, sizeof(palette), 0, 0, 0, 0);

	return 0;
}
