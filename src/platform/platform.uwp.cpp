#include <dseed.h>

#if PLATFORM_UWP

#	include "platform.internal.h"

namespace dseed
{
	namespace uwp
	{
		ref class DSeedFrameworkView : public Windows::ApplicationModel::Core::IFrameworkView
		{
		public:
			virtual void Initialize (Windows::ApplicationModel::Core::CoreApplicationView^ appView)
			{

			}

			virtual void SetWindow (Windows::UI::Core::CoreWindow^ window)
			{

			}

			virtual void Load (Platform::String^ entryPoint) { }

			virtual void Run ()
			{

			}

			virtual void Uninitialize ()
			{

			}

		public:

		};

		ref class DSeedFrameworkViewSource : public Windows::ApplicationModel::Core::IFrameworkViewSource
		{
		public:
			virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView ()
			{

			}
		};
	}
}

class __uwp_application : public dseed::application
{
public:

};

dseed::error_t dseed::create_application (dseed::application** app)
{
	if (g_sharedApp != nullptr) return dseed::error_invalid_op;
	if (app == nullptr) return dseed::error_invalid_args;

}

#endif