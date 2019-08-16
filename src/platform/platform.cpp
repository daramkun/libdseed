#include <dseed.h>

static dseed::application* g_sharedApp;

dseed::error_t dseed::application::shared_app (dseed::application** app)
{
	if (app == nullptr) return dseed::error_invalid_args;
	*app = g_sharedApp;
	return dseed::error_good;
}