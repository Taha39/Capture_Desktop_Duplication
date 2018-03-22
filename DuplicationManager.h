// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef _DUPLICATIONMANAGER_H_
#define _DUPLICATIONMANAGER_H_

#include "CommonTypes.h"
//
// Handles the task of duplicating an output.
//
namespace internal
{
	class DUPLICATIONMANAGER
	{
	public:
		DUPLICATIONMANAGER(int w, int h);
		~DUPLICATIONMANAGER();
		DUPL_RETURN get_frame(_Out_ bool* Timeout, _Out_ UINT8* raw_data);
		DUPL_RETURN init_dupl(_In_ DX_RESOURCES* resource, UINT output_display/*, DISPLAYMANAGER *pDispMgr*/);
		DUPL_RETURN release_frame();

	private:
		DUPL_RETURN frame(UINT8* raw_data);

	private:
		// vars
		IDXGIOutputDuplication* desk_dupl_{ nullptr };
		ID3D11Texture2D* acquire_desktop_img_{ nullptr };
		_Field_size_bytes_(meta_data_size_) BYTE* meta_data_buffer_ { nullptr };
		UINT meta_data_size_{ 0 };
		DXGI_OUTPUT_DESC output_desc_;
		ID3D11Device* device_{ nullptr };
		ID3D11DeviceContext* device_context_{ nullptr };

		int width_{ 0 };
		int height_{ 0 };
	};
}
#endif
