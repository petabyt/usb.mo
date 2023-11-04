#ifndef USB_MAIN_H
#define USB_MAIN_H
#include <stdint.h>
#pragma pack(push, 1)

#define PTP_CHDK_VERSION_MAJOR 0
#define PTP_CHDK_VERSION_MINOR 1

#define PTP_CHDK_OPCODE 0x9999
#define PTP_ML_OPCODE 0x9998

int ptp_lv_get_vram(struct ptp_context *ctx);
int ptp_lv_get_vram_info(struct ptp_context *ctx);
int ptp_chdk_upload_file(struct ptp_context *context);

// Standard CHDK commands (Param1)
enum ptp_chdk_command {
	PTP_CHDK_Version = 0,
	PTP_CHDK_GetMemory = 1,
	PTP_CHDK_SetMemory = 2,
	PTP_CHDK_CallFunction = 3,
	PTP_CHDK_TempData = 4,
	PTP_CHDK_UploadFile = 5,
	PTP_CHDK_DownloadFile = 6,
	PTP_CHDK_ExecuteScript = 7,
	PTP_CHDK_ScriptStatus = 8,
	PTP_CHDK_ScriptSupport = 9,
	PTP_CHDK_ReadScriptMsg,
	PTP_CHDK_WriteScriptMsg,
	PTP_CHDK_GetDisplayData,
	PTP_CHDK_RemoteCaptureIsReady,
	PTP_CHDK_RemoteCaptureGetData
};

// Standard ML commands (Param1)
enum PtpMlCommands {
	PTP_ML_VIEW_FRAME = 0,
	PTP_ML_VIEW_SPEC = 1,
};

struct LvRamInfo {
	uint32_t lv_pitch;
	uint32_t lv_width;
};

struct LvHeader {
	uint8_t digic;
	uint8_t v1;
	uint8_t v2;
	uint8_t v3;
};

#pragma pack(pop)

#endif
