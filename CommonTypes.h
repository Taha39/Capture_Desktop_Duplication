// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef _COMMONTYPES_H_
#define _COMMONTYPES_H_

#include <d3d11.h>
#include <dxgi1_2.h>
#include <DirectXMath.h>
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

	mfxU16  width{ 0 };
	mfxU16  height{ 0 };
	mfxU32  fourcc{ 0 };

	sPluginParams pluginParams{};
};




typedef _Return_type_success_(return == DUPL_RETURN_SUCCESS) enum
{
    DUPL_RETURN_SUCCESS             = 0,
    DUPL_RETURN_ERROR_EXPECTED      = 1,
    DUPL_RETURN_ERROR_UNEXPECTED    = 2,
	DUPL_RETURN_ERROR_DEVICE_LOST	= 3
}DUPL_RETURN;

//
// Structure that holds D3D resources not directly tied to any one thread
//
typedef struct _DX_RESOURCES
{
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    ID3D11SamplerState* SamplerLinear;
} DX_RESOURCES;

#endif
