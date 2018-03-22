
#include "CaptureDD.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "DuplicationManager.h"

//using namespace internal;

namespace DD
{
	CaptureDsktpDup::CaptureDsktpDup(int w, int h, bool isPrimary) :
		width_(w), height_(h), is_primary_(isPrimary)
	{
	}

	CaptureDsktpDup::~CaptureDsktpDup()
	{
		clean_dx();
	}

	bool CaptureDsktpDup::start(Callback* callback)
	{
		assert(callback);

		callback_ = callback;
		return init();
	}

	void CaptureDsktpDup::clean_dx()
	{
		if (dx_resource_.Device)
		{
			dx_resource_.Device->Release();
			dx_resource_.Device = nullptr;
		}

		if (dx_resource_.Context)
		{
			dx_resource_.Context->Release();
			dx_resource_.Context = nullptr;
		}
	}

	bool CaptureDsktpDup::init()
	{
		HRESULT hr = S_OK;
		DUPL_RETURN Ret;

		dupl_manager_ = std::make_unique<internal::DUPLICATIONMANAGER>(width_, height_);

		hr = initialize_dx(&dx_resource_);
		if (hr != S_OK)
			return false;

		// init duplication manager
		Ret = dupl_manager_->init_dupl(&dx_resource_, !is_primary_); //M.T: Second parameter value 1(!is_primary_) is used for extended monitor.
		if (Ret != DUPL_RETURN_SUCCESS)
		{
			return false;// goto Exit;
		}

		return true;
	}

	/*int  CaptureDsktpDup::capture_frame(UINT8* raw_data)
	{
		bool TimeOut{ false };
		int status{ 0 };
		DUPL_RETURN ret = dupl_manager_->get_frame(&TimeOut, raw_data);
		if (dupl_manager_->get_frame(&TimeOut, raw_data) == DUPL_RETURN_SUCCESS)
			status = 1;
		else if (!TimeOut)
		{
			printf("\nRet != DUPL_RETURN_SUCCESS\n");
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			dupl_manager_ = std::make_unique<internal::DUPLICATIONMANAGER>(width_, height_);
			init();
		}

		return status;
	}*/

#ifndef ACTUAL_PROJECT
	int  CaptureDsktpDup::capture_frame(UINT8* raw_data)
	{
		bool TimeOut{ false };
		int status{ 0 };
		//DUPL_RETURN ret = dupl_manager_->get_frame(&TimeOut, raw_data);
		if (dupl_manager_->get_frame(&TimeOut, raw_data) == DUPL_RETURN_SUCCESS)
			status = 1;
		else if (!TimeOut)
		{
			printf("\nRet != DUPL_RETURN_SUCCESS\n");
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			dupl_manager_ = std::make_unique<internal::DUPLICATIONMANAGER>(width_, height_);
			init();
		}

		return status;
	}
#else
	void CaptureDsktpDup::capture_frame()
	{
		bool TimeOut{ false };
		DUPL_RETURN ret = dupl_manager_->get_frame(&TimeOut, raw_data_.data());

		if (callback_)
		{
			frame data;
			data.data_format_ = raw_data_type_;
			data.height_ = height_;
			data.width_ = width_;
			data.data_ = raw_data_.data();
			callback_->on_capture(
				ret == DUPL_RETURN_SUCCESS ? result::OK : result::LERROR,
				data);
		}
		error_check(ret);
	}

	void CaptureDsktpDup::error_check(DUPL_RETURN sts)
	{
		if (sts == DUPL_RETURN_ERROR_DEVICE_LOST)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			dupl_manager_ = std::make_unique<internal::DUPLICATIONMANAGER>(width_, height_);
			init();
		}
	}
#endif

	HRESULT CaptureDsktpDup::initialize_dx(_Out_ DX_RESOURCES* data)
	{
		HRESULT hr = S_OK;

		// Driver types supported
		D3D_DRIVER_TYPE DriverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT num_driver_types = ARRAYSIZE(DriverTypes);

		// Feature levels supported
		D3D_FEATURE_LEVEL feature_levels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_1
		};
		UINT num_feature_levels = ARRAYSIZE(feature_levels);

		D3D_FEATURE_LEVEL feature_level;

		// Create device
		for (UINT DriverTypeIndex = 0; DriverTypeIndex < num_driver_types; ++DriverTypeIndex)
		{
			hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, 0, feature_levels, num_feature_levels,
				D3D11_SDK_VERSION, &data->Device, &feature_level, &data->Context);
			if (SUCCEEDED(hr))
			{
				// Device creation success, no need to loop anymore
				break;
			}
		}
		
		return hr;		
	}

}