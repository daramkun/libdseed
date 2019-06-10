#include <dseed.h>

dseed::object::~object ()
{
    
}

#if PLATFORM_WINDOWS
#include <assert.h>
BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		assert (SUCCEEDED (CoInitialize (nullptr)));
		break;

	case DLL_PROCESS_DETACH:
		CoUninitialize ();
		break;
	}

	return TRUE;
}
#endif