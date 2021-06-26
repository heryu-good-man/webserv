/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heryu <heryu@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/06/08 21:49:40 by heryu             #+#    #+#             */
/*   Updated: 2021/06/25 00:19:31 by heryu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

class Config;
# include <vector>
# include <string>
# include <exception>
# include <fstream>
# include <sstream>
# include <iostream>
# include <stack>
# include "Utils.hpp"
# include "Location.hpp"
# include "Server.hpp"

class Config
{
public:
	Config();
	Config(const Config& other);
	~Config();
	Config& operator=(const Config& rhs);

	Server				getServer(size_t index) const;
	std::vector<Server>	getServers(void) const;

	void				parseConfig(const char* fileName);


private:
	std::vector<Server>	_servers;

	Server			_parseServer(const std::string& content);
	Location 		_parseLocation(const std::string& content);
	std::string		_getFileContent(const char* fileName) const;
	size_t			_getServerContentSize(const std::string& content, size_t startIndex) const;
	std::string		_getLocationPath(const std::string& content) const;
	bool			_isValidConfig(void) const;

};

#endif
