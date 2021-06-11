#include "Utils.hpp"

/*
** 공백 확인
*/
bool isSpace(const char ch)
{
	if (ch == '\t' || ch == '\f' || ch == '\v' || ch == '\r' || ch == '\n' || ch == ' ')
		return (true);
	else
		return (false);
}

/*
** 주석을 제거함
*/
void deleteComments(std::string& fileLine)
{
	size_t fileLineSize = fileLine.size();
	int commentIndex = -1;
	for (size_t i = 0; i < fileLineSize; ++i)
	{
		if (fileLine[i] == '#')
		{
			commentIndex = i;
			break ;
		}
	}
	if (commentIndex != -1)
		fileLine.erase(commentIndex, fileLineSize - commentIndex);
}

/*
** 한 줄에서 좌우 공백을 제거함
*/
void trimSpace(std::string& line)
{
	int i = 0;
	while (isSpace(line[i]))
		++i;
	line.erase(0, i);

	i = line.size() - 1;
	while (i >= 0 && isSpace(line[i]))
		--i;
	line.erase(i + 1, line.size() - 1 - i);
}

/*
** 연속된 공백을 제거
*/
void removeContinuousSpace(std::string& line)
{
	size_t i = 0;
	while (line[i])
	{
		if (isSpace(line[i]))
		{
			size_t pivot = i;
			size_t count = 0;
			while (isSpace(line[i]))
			{
				++i;
				++count;
			}
			line[pivot] = ' ';
			line.erase(pivot + 1, count - 1);
			i = pivot;
		}
		i++;
	}
}

/*
** string을 구분자로 자른 후 pair로 리턴
*/
std::pair<std::string, std::string> splitString(const std::string& str, const std::string& deli)
{
	size_t pivot = str.find(deli);
	std::string first = str.substr(0, pivot);
	std::string last = str.substr(pivot + 1, str.size() - (pivot + 1));

	return (std::make_pair(first, last));
}
