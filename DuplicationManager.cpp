// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "DuplicationManager.h"
//#include"CommonTypes.h"
#include "iostream"

using namespace DirectX;

namespace internal
{
	DUPLICATIONMANAGER::DUPLICATIONMANAGER(int w, int h) : width_(w),
		height_(h),
		desk_dupl_(nullptr),
		acquire_desktop_img_(nullptr),
		meta_data_buffer_(nullptr),
		meta_data_size_(0),
		device_(nullptr)
	{
		RtlZeroMemory(&output_desc_, sizeof(output_desc_));
	}

	//
	// Destructor simply calls CleanRefs to destroy everything
	//
	DUPLICATIONMANAGER::~DUPLICATIONMANAGER()
	{		
	}

	//
	// Initialize duplication interfaces
	//
	DUPL_RETURN DUPLICATIONMANAGER::init_dupl(_In_ DX_RESOURCES* d3d11_resource, UINT output_display)
	{
		if (d3d11_resource == NULL)
			return DUPL_RETURN_ERROR_UNEXPECTED;

		// Take a reference on the device
		device_context_ = d3d11_resource->Context;
		device_context_->AddRef();
		device_ = d3d11_resource->Device;
		device_->AddRef();

		// Get DXGI device
		IDXGIDevice* dxgi_device = nullptr;
		HRESULT hr = device_->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device));
		if (FAILED(hr))
		{
			return DUPL_RETURN_ERROR_UNEXPECTED;
		}

		// Get DXGI adapter
		IDXGIAdapter* dxgi_adapter = nullptr;
		hr = dxgi_device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgi_adapter));
		dxgi_device->Release();
		dxgi_device = nullptr;
		if (FAILED(hr))
		{
			return DUPL_RETURN_ERROR_UNEXPECTED;
		}

		// this output_display is a selection for primary desktop and secondary desktop
		IDXGIOutput* dxgi_output = nullptr;
		hr = dxgi_adapter->EnumOutputs(output_display, &dxgi_output);
		dxgi_adapter->Release();
		dxgi_adapter = nullptr;
		if (FAILED(hr))
		{
			return DUPL_RETURN_ERROR_UNEXPECTED;
		}

		dxgi_output->GetDesc(&output_desc_);

		// QI for Output 1
		IDXGIOutput1* dxgi_output1 = nullptr;
		hr = dxgi_output->QueryInterface(__uuidof(dxgi_output1), reinterpret_cast<void**>(&dxgi_output1));
		dxgi_output->Release();
		dxgi_output = nullptr;
		if (FAILED(hr))
		{
			return DUPL_RETURN_ERROR_UNEXPECTED;
		}

		// Create desktop duplication
		hr = dxgi_output1->DuplicateOutput(device_, &desk_dupl_);
		dxgi_output1->Release();
		dxgi_output1 = nullptr;
		if (FAILED(hr))
		{
			return DUPL_RETURN_ERROR_UNEXPECTED;
		}

		return DUPL_RETURN_SUCCESS;
	}

	//
	// Get next frame and write it into Data
	//
	DUPL_RETURN DUPLICATIONMANAGER::get_frame(_Out_ bool* Timeout, UINT8* raw_data)
	{
		IDXGIResource* desktop_resource{ nullptr };
		DXGI_OUTDUPL_FRAME_INFO frame_info;

		// Get new frame
		if (!desk_dupl_)
			return DUPL_RETURN_ERROR_DEVICE_LOST;

		HRESULT hr = desk_dupl_->AcquireNextFrame(35/*100*/, &frame_info, &desktop_resource);
		if (FAILED(hr))
		{
			if (hr == DXGI_ERROR_WAIT_TIMEOUT)
				*Timeout = true;
			return DUPL_RETURN_ERROR_EXPECTED;
		}

		// If still holding old frame, destroy it
		if (acquire_desktop_img_)
		{
			acquire_desktop_img_->Release();
			acquire_desktop_img_ = nullptr;
		}

		//// QI for IDXGIResource
		hr = desktop_resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&acquire_desktop_img_));
		desktop_resource->Release();
		desktop_resource = nullptr;
		if (FAILED(hr))
			return DUPL_RETURN_ERROR_EXPECTED;

		// Get metadata
		if (frame_info.TotalMetadataBufferSize)
		{
			// Old buffer too small
			if (frame_info.TotalMetadataBufferSize > meta_data_size_)
			{
				if (meta_data_buffer_)
				{
					delete[] meta_data_buffer_;
					meta_data_buffer_ = NULL;
				}
				meta_data_buffer_ = new (std::nothrow) BYTE[frame_info.TotalMetadataBufferSize];
				if (!meta_data_buffer_)
				{
					meta_data_size_ = 0;
					return DUPL_RETURN_ERROR_EXPECTED;
				}
				meta_data_size_ = frame_info.TotalMetadataBufferSize;
			}

			UINT buf_size = frame_info.TotalMetadataBufferSize;
			// Get move rectangles
			hr = desk_dupl_->GetFrameMoveRects(buf_size, reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(meta_data_buffer_), &buf_size);
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;

			BYTE* dirty_rects = meta_data_buffer_ + buf_size;
			buf_size = frame_info.TotalMetadataBufferSize - buf_size;

			// Get dirty rectangles
			hr = desk_dupl_->GetFrameDirtyRects(buf_size, reinterpret_cast<RECT*>(dirty_rects), &buf_size);
			if (FAILED(hr))
				return DUPL_RETURN_ERROR_EXPECTED;
		}

		return frame(raw_data);
	}

	DUPL_RETURN DUPLICATIONMANAGER::frame(UINT8* raw_data)
	{
		D3D11_TEXTURE2D_DESC buffer_desc;
		buffer_desc.Width = width_;
		buffer_desc.Height = height_;
		buffer_desc.MipLevels = 1;
		buffer_desc.ArraySize = 1;
		buffer_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		buffer_desc.SampleDesc.Count = 1;
		buffer_desc.SampleDesc.Quality = 0;
		buffer_desc.Usage = D3D11_USAGE_STAGING;
		buffer_desc.BindFlags = 0;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		buffer_desc.MiscFlags = 0;

		ID3D11Texture2D* buffer = nullptr;
		HRESULT hr = device_->CreateTexture2D(&buffer_desc, nullptr, &buffer);
		if (FAILED(hr))
			return DUPL_RETURN_ERROR_EXPECTED;

		device_context_->CopySubresourceRegion(buffer, 0, 0, 0, 0, acquire_desktop_img_, 0, 0);

		// QI for IDXGISurface
		IDXGISurface* copy_surface = nullptr;
		hr = buffer->QueryInterface(__uuidof(IDXGISurface), (void **)&copy_surface);
		buffer->Release();
		buffer = nullptr;
		if (hr != S_OK)
			return DUPL_RETURN_ERROR_EXPECTED;

		// Map pixels
		DXGI_MAPPED_RECT mapped_surface;
		hr = copy_surface->Map(&mapped_surface, DXGI_MAP_READ);

		if (hr != S_OK)
		{
			copy_surface->Release();
			return DUPL_RETURN_SUCCESS;
		}

		BYTE* temp = raw_data;

		for (int h = 0; h < height_; h++)
		{
			memcpy(temp, mapped_surface.pBits + (h * mapped_surface.Pitch), (mapped_surface.Pitch/*width_ * 4*/));
			temp += (mapped_surface.Pitch/*width_ * 4*/);
		}

		copy_surface->Release();
		return release_frame();
	}
	//
	// Release frame
	//
	DUPL_RETURN DUPLICATIONMANAGER::release_frame()
	{
		HRESULT hr = desk_dupl_->ReleaseFrame();
		if (FAILED(hr))
		{
			return DUPL_RETURN_ERROR_EXPECTED;
		}

		if (acquire_desktop_img_)
		{
			acquire_desktop_img_->Release();
			acquire_desktop_img_ = nullptr;
		}
		return DUPL_RETURN_SUCCESS;
	}
}