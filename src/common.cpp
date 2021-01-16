#include <dseed.h>

#include <regex>

dseed::version::version() { }

dseed::version::version(const char* versionString)
{
	std::string versionStringObj = versionString;
	std::smatch match;
	std::regex versionChecker("([0-9]+)[.]*([0-9]*)[.]*([0-9]*)[.]*([0-9]*)(.*)");
	if (std::regex_match(versionStringObj, match, versionChecker)) {
		major = atoi((*(++match.begin())).str().c_str());
		if (match.size() > 2)
		{
			minor = atoi((*(++++match.begin())).str().c_str());
			if (match.size() > 3)
			{
				build = atoi((*(++++++match.begin())).str().c_str());
				if (match.size() > 4)
					revision = atoi((*(++++++++match.begin())).str().c_str());
			}
		}
	}
}

dseed::version::version(int major, int minor, int build, int revision)
	: major(major), minor(minor), build(build), revision(revision)
{

}