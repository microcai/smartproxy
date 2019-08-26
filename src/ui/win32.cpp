
#include <windows.h>
#include <vector>
#include <string>

int proxy_main(std::vector<std::string> argv);

int main(int argc, char* argv[])
{
	std::vector<std::string> args;

	for (int i = 0; i < argc; i++)
	{
		args.push_back(argv[i]);
	}

	return proxy_main(args);
}
