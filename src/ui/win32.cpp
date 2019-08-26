
#include <windows.h>

extern int proxy_main(int argc, char* argv[]);

int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	return proxy_main(__argc, __argv[]);
}
