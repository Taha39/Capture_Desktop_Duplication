#ifndef _COMMON_H_
#define _COMMON_H_

//#include "mfx_samples_config.h"
#include "sample_params.h"

enum MemTypeD {
	SYSTEM_MEMORYD = 0x00,
	D3D9_MEMORYD = 0x01,
	D3D11_MEMORYD = 0x02,
};

enum class result {
	OK,
	LERROR
};



struct frame {
	int width_{ 0 };
	int height_{ 0 };
	const UINT8* data_{ nullptr };
	mfxU32 data_format_{ MFX_FOURCC_RGB4 };
};

struct sInputParamsD
{
	mfxU32 videoType{ MFX_CODEC_CAPTURE };
	MemTypeD memType{ SYSTEM_MEMORYD };
	bool    bUseHWLib{ true };

	mfxU16  width{0};
	mfxU16  height{0};
	mfxU32  fourcc{0};

	sPluginParams pluginParams{};
};

#endif