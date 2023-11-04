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

struct menu_entry ptpview_menu[] = {
	{
		.name = "USB",
		.select = NULL,
		.help = "USB Extensions for Magic Lantern",
	},
};

static int ml_ext_handler(void *priv, struct ptp_context *context,
	uint32_t opcode, uint32_t session, uint32_t transaction,
	uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4, uint32_t param5) 
{
	int rc;
	if (param1 == PTP_ML_VIEW_FRAME) {
		rc = ptp_lv_get_vram(context);
	} else if (param1 == PTP_ML_VIEW_SPEC) {
		rc = ptp_lv_get_vram_info(context);
	} else {
		rc = -1;
	}

	if (rc < 0) {
		rc = 0x2002;
	} else {
		rc = 0x2001;
	}

	struct ptp_msg msg = {
		.id = rc,
		.session = session,
		.transaction = transaction,
		.param_count = 0,
	};

	context->send_resp(context->handle, &msg);

	return 0;
}

static int chdk_ext_handler(void *priv, struct ptp_context *context,
	uint32_t opcode, uint32_t session, uint32_t transaction,
	uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4, uint32_t param5) 
{
	int rc = 0;

	struct ptp_msg msg = {
		.session = session,
		.transaction = transaction,
		.param_count = 0,
	};

	switch (param1) {
	case PTP_CHDK_Version:
		msg.param_count = 2;
		msg.param[0] = PTP_CHDK_VERSION_MAJOR;
		msg.param[1] = PTP_CHDK_VERSION_MINOR;
		break;
	case PTP_CHDK_UploadFile:
		rc = ptp_chdk_upload_file(context);
		break;
	}

	if (rc) {
		rc = 0x2002;
	} else {
		rc = 0x2001;
	}

	msg.id = rc;

	context->send_resp(context->handle, &msg);

	return 0;
}

unsigned int ptpview_init()
{
	menu_add("Display", ptpview_menu, COUNT(ptpview_menu));

	// ML is very weird about the headers for these functions
	ptp_register_handler(PTP_ML_OPCODE, (void *)&ml_ext_handler, 0);
	ptp_register_handler(PTP_CHDK_OPCODE, (void *)&chdk_ext_handler, 0);

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
