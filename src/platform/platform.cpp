#include <dseed.h>

dseed::platform::application* g_sharedApp;

dseed::error_t dseed::platform::application::shared_app (dseed::platform::application** app)
{
	if (app == nullptr) return dseed::error_invalid_args;
	*app = g_sharedApp;
	g_sharedApp->retain ();
	return dseed::error_good;
}