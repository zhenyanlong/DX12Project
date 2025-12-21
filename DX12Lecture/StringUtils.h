#pragma once
class StringUtils
{
};

#include <string>
#include <vector>
#include <algorithm>
#include <regex>


inline std::string toLower(const std::string& str)
{
	std::string lowerStr = str;
	std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
	return lowerStr;
}

// Check whether the string "PipeName" contains the specified keyword
inline bool hasKeyword(const std::string& pipeName, const std::string& keyword, const std::vector<std::string>& aliases = {})
{
	std::string lowerPipe = toLower(pipeName);
	std::string lowerKey = toLower(keyword);

	// Check the original keyword
	std::regex keyRegex("\\b" + lowerKey + "\\b|_" + lowerKey + "_|^" + lowerKey + "_|_" + lowerKey + "$");
	if (std::regex_search(lowerPipe, keyRegex))
	{
		return true;
	}

	// Check the alias
	for (const auto& alias : aliases)
	{
		std::string lowerAlias = toLower(alias);
		std::regex aliasRegex("\\b" + lowerAlias + "\\b|_" + lowerAlias + "_|^" + lowerAlias + "_|_" + lowerAlias + "$");
		if (std::regex_search(lowerPipe, aliasRegex))
		{
			return true;
		}
	}

	return false;
}

// Extract all the key words from the "PipeName"
inline std::vector<std::string> extractKeywords(const std::string& pipeName, const std::vector<std::string>& allKeywords)
{
	std::vector<std::string> result;
	for (const auto& key : allKeywords)
	{
		if (hasKeyword(pipeName, key))
		{
			result.push_back(key);
		}
	}
	return result;
}