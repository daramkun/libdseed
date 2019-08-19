#include <dseed.h>

ENTRYPOINT_ATTRIBUTE
ENTRYPOINT_RETURNTYPE ENTRYPOINT_NAME (ENTRYPOINT_ARGUMENTS)
{
	dseed::auto_object<dseed::application> app;
	if (dseed::failed (dseed::create_application (&app)))
		return -1;

	if (dseed::failed (app->run (nullptr)))
		return -2;

	return 0;
}