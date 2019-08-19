#include <dseed.h>

dseed::application* g_sharedApp;

dseed::error_t dseed::application::shared_app (dseed::application** app)
{
	if (app == nullptr) return dseed::error_invalid_args;
	*app = g_sharedApp;
	g_sharedApp->retain ();
	return dseed::error_good;
}