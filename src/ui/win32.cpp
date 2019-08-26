
#include <windows.h>

int proxy_main(std::vector<std::string> argv);

int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	return proxy_main({});
}
