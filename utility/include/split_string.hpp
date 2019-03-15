
#include <string>
#include <vector>

inline std::vector<std::string> split_string(std::string input, std::string delim)
{
	std::vector<std::string> ret;
	std::string::size_type dep_sep;

	while((dep_sep = input.find_first_of(delim)) != std::string::npos)
	{
		ret.emplace_back(input.substr(0, dep_sep));
		input = input.substr(dep_sep + 3);
	}
	ret.emplace_back(input);
	return ret;
}
