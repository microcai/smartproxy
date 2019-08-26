
#include <vector>
#include <iostream>
#include <sys/resource.h>

void ulimit_limit()
{
    struct rlimit rlp;

    getrlimit(RLIMIT_NOFILE, &rlp);

    if (rlp.rlim_cur < 10000)
    {
        rlp.rlim_cur = 10000;
        setrlimit(RLIMIT_NOFILE, &rlp);
    }
    getrlimit(RLIMIT_NOFILE, &rlp);
    std::cout << "rlimit changed to " << rlp.rlim_cur << std::endl;
}

int proxy_main(std::vector<std::string> argv);

int main(int argc, char* argv[])
{
	ulimit_limit();
	std::vector<std::string> args;

	for (int i = 0; i < argc; i++)
	{
		args.push_back(argv[i]);
	}

	return proxy_main(args);
}
