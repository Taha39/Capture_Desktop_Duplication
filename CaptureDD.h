#ifndef CAPTURE_DD_H
#define CAPTURE_DD_H

//#include "DuplicationManager.h"
#include "CommonTypes.h"
#include <memory>

//#define ACTUAL_PROJECT
namespace internal
{
	class DUPLICATIONMANAGER;
}

namespace DD
{
	class CaptureDsktpDup
	{
	public:
		class Callback {
		public:
			virtual void on_capture(result, frame) = 0;
			virtual ~Callback() {}
		};

	public:
		CaptureDsktpDup(int width, int height, bool isPrimary);
		~CaptureDsktpDup();

#ifndef ACTUAL_PROJECT
		int capture_frame(UINT8* EncData);
#else
		void capture_frame();
#endif
		bool start(Callback*);
		bool init();

	private:
		HRESULT initialize_dx(_Out_ DX_RESOURCES* Data);
		void clean_dx();
		void error_check(DUPL_RETURN sts);

	private:
		DX_RESOURCES dx_resource_;
		std::unique_ptr<internal::DUPLICATIONMANAGER> dupl_manager_{ nullptr };
		bool is_primary_{ true };
		int width_{ 0 };
		int height_{ 0 };
		Callback* callback_{ nullptr };
		std::vector<UINT8> raw_data_;
	};
}
#endif