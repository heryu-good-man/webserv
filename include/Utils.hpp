#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <cstdlib>
# include <iostream>

#define MAX 512
#define MAXBUFF 32767

const size_t	LOCATION_STR_LENGTH = 8;	// strlen("location")
const size_t	SERVER_STR_LENGTH = 6;		// strlen("server")

bool isSpace(const char ch);
void deleteComments(std::string& fileLine);
void trimSpace(std::string& line);
void removeContinuousSpace(std::string& line);
std::pair<std::string, std::string> splitString(const std::string& str, const std::string& deli);

#endif
