#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <map>
# include <string>
# include <vector>
# include "Utils.hpp"
# include "Config.hpp"

class Location
{
	friend class Config;
public:
	enum Key {
		PATH, METHOD, ROOT, AUTO_INDEX,
		INDEX_PAGES ,CGI, CGI_PATH,
		RETURN, UPLOAD_ENABLE, CLIENT_BODY_SIZE,
		NONE
	};

	Location();
	Location(const Location& other);
	~Location();
	Location& operator=(const Location& rhs);

	bool	setMemberData(void);


	void	print(void);
	// GETTER
	const std::map<std::string, std::string>	getData() const;
	const std::string							getPath() const;
	const std::vector<std::string>&			getMethods() const;
	const std::string							getRoot() const;
	bool								getAutoIndex() const;
	const std::vector<std::string>&			getIndexPages() const;
	const std::string							getCGI() const;
	const std::string							getCGIPath() const;
	const std::string							getReturn() const;
	bool								getUploadEnable() const;
	size_t							getClientBodySize() const;

private:
	std::map<std::string, std::string>	_data;

	std::string							_path;
	std::vector<std::string>			_methods;
	std::string							_root;
	bool								_autoIndex;
	std::vector<std::string>			_indexPages;
	std::string							_CGI;
	std::string							_CGIPath;
	std::string							_return;
	bool								_uploadEnable;
	size_t								_clientBodySize;

	Key		_getKeyNumber(const std::string& key) const;
	void	_setMethod(const std::string& value);
	void	_setRoot(const std::string& value);
	void	_setAutoIndex(const std::string& value);
	void	_setIndexPages(const std::string& value);
	void	_setCGI(const std::string& value);
	void	_setCGIPath(const std::string& value);
	void	_setReturn(const std::string& value);
	void	_setUploadEnable(const std::string& value);
	void	_setClientBodySize(const std::string& value);

};

#endif
