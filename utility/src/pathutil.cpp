
#include "pathutil.hpp"

#ifdef __MACH__
#include <mach-o/dyld.h>
#include <limits.h>
#endif

#ifdef _WIN32

#include <windows.h>

boost::filesystem::path get_executable_path()
{
	std::string exepath;
	exepath.resize(5000);
	exepath.resize(GetModuleFileName(nullptr, &exepath[0], static_cast<DWORD>(exepath.size())));

	boost::filesystem::path exe_path = exepath;

	return exe_path.parent_path();
}
#else
boost::filesystem::path get_executable_path()
{
	std::string exepath;
#if defined(__MACH__)
	uint32_t buffer_size = 5000;
	exepath.resize(buffer_size);
	::_NSGetExecutablePath(&exepath[0], &buffer_size);
#else
	int buffer_size = 5000;
	exepath.resize(buffer_size);
	buffer_size = readlink("/proc/self/exe", &exepath[0], exepath.size());
	exepath.resize(buffer_size);
#endif
	boost::filesystem::path exe_path = exepath;

	return exe_path.parent_path();
}
#endif
