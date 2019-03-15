
#include <vector>
#include <map>

#include <string>
#include <sstream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

template<typename FormData>
static std::string build_autosubmit_page(std::string action, std::string method, const FormData& forms)
{
	std::stringstream sHtml;

	sHtml << "<html>";
	sHtml << "<head>";
	sHtml << R"(<meta charset="utf8" />)";
	sHtml << "</head>";
	sHtml << "<body>";

	sHtml << R"(<form id='autosubmitform' name='autosubmitform' action=')" << action << R"(' method=')" << method << "\'>\n";

	// while balabala
	for (const auto& it : forms)
	{
		sHtml << "\t<input type='hidden' name='" << it.first << "' value='" << boost::replace_all_copy(it.second, "\'", "&apos;") << "'/>";
	}

	sHtml << "</form>";
	sHtml << R"(<script type="text/javascript">document.forms['autosubmitform'].submit();</script>)";
	sHtml << "</body>";
	sHtml << "</html>";

	return sHtml.str();
}

static std::string build_auto_redirect_page(std::string to)
{
	return boost::str(boost::format(u8R"(<html>
<head>
<title>pay</title>
<meta http-equiv="refresh" content="0; URL=%s">
</head>
<body>
</body>
</html>
)") % to);
}
