
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

extern int proxy_main(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	ulimit_limit();
	return proxy_main(argc, argv);
}
