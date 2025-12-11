#pragma once
class StringUtils
{
};

#include <string>
#include <vector>
#include <algorithm>
#include <regex>

// 转换字符串为小写
inline std::string toLower(const std::string& str)
{
	std::string lowerStr = str;
	std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
	return lowerStr;
}

// 检测PipeName中是否包含指定关键词（全词匹配，忽略大小写）
// 支持关键词的缩写兼容（如"Instance"兼容"Inst"）
inline bool hasKeyword(const std::string& pipeName, const std::string& keyword, const std::vector<std::string>& aliases = {})
{
	std::string lowerPipe = toLower(pipeName);
	std::string lowerKey = toLower(keyword);

	// 1. 检查原关键词
	// 正则：全词匹配（关键词前后是边界、下划线、大小写字母外的字符）
	std::regex keyRegex("\\b" + lowerKey + "\\b|_" + lowerKey + "_|^" + lowerKey + "_|_" + lowerKey + "$");
	if (std::regex_search(lowerPipe, keyRegex))
	{
		return true;
	}

	// 2. 检查别名（兼容原有缩写，如"Inst"是"Instance"的别名）
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

// 提取PipeName中的所有关键词（可选，用于复杂策略组合）
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