#include "Location.hpp"

const std::string METHODS[5] = { "GET", "HEAD", "POST", "PUT", "DELETE" };
const std::vector<std::string>	DEFAULT_METHOD;					// 변경 필요
const std::string				DEFAULT_ROOT = "default root"; 	// 변경 필요
const bool						DEFAULT_AUTOINDEX = false;
const std::vector<std::string>	DEFAULT_INDEX_PAGES;
const std::string				DEFAULT_CGI = "";
const std::string				DEFAULT_CGI_PATH = "";
const std::string				DEFAULT_RETURN = "";
const bool						DEFAULT_UPLOAD_ENABLE = false;
const size_t					DEFAULT_CLIENT_BODY_SIZE = ULONG_MAX;

Location::Location()
	: _data()
	, _path()
	, _methods(METHODS, METHODS + 5)
	, _root(DEFAULT_ROOT)
	, _autoIndex(DEFAULT_AUTOINDEX)
	, _indexPages(DEFAULT_INDEX_PAGES)
	, _CGI(DEFAULT_CGI)
	, _CGIPath(DEFAULT_CGI_PATH)
	, _return(DEFAULT_RETURN)
	, _uploadEnable(DEFAULT_UPLOAD_ENABLE)
	, _clientBodySize(DEFAULT_CLIENT_BODY_SIZE)
{

}

Location::Location(const Location& other)
	: _data(other._data)
	, _path(other._path)
	, _methods(other._methods)
	, _root(other._root)
	, _autoIndex(other._autoIndex)
	, _indexPages(other._indexPages)
	, _CGI(other._CGI)
	, _CGIPath(other._CGIPath)
	, _return(other._return)
	, _uploadEnable(other._uploadEnable)
	, _clientBodySize(other._clientBodySize)
{

}

Location::~Location()
{

}

Location& Location::operator=(const Location& rhs)
{
	if (this != &rhs)
	{
		_data = rhs._data;
		_path = rhs._path;
		_methods = rhs._methods;
		_root = rhs._root;
		_autoIndex = rhs._autoIndex;
		_indexPages = rhs._indexPages;
		_CGI = rhs._CGI;
		_CGIPath = rhs._CGIPath;
		_return = rhs._return;
		_uploadEnable = rhs._uploadEnable;
		_clientBodySize = rhs._clientBodySize;
	}
	return (*this);
}

/*
** string을 비교하여 key에 맞는 enum값을 리턴함
** 없는 키면 NONE 반환
*/
Location::Key	Location::_getKeyNumber(const std::string& key) const
{
	if (key == "method")
		return (METHOD);
	else if (key == "root")
		return (ROOT);
	else if (key == "auto_index")
		return (AUTO_INDEX);
	else if (key == "index")
		return (INDEX_PAGES);
	else if (key == "cgi_extension")
		return (CGI);
	else if (key == "cgi_path_info")
		return (CGI_PATH);
	else if (key == "return")
		return (RETURN);
	else if (key == "upload_enable")
		return (UPLOAD_ENABLE);
	else if (key == "client_body_size")
		return (CLIENT_BODY_SIZE);
	else
		return (NONE);
}

/*
** _data[key:value]를 멤버변수에 하나하나 넣어줌
** 성공 실패를 반환
** invalid한 key가 들어올 경우 실패 반환
*/
bool	Location::setMemberData(void)
{
	for (std::map<std::string, std::string>::const_iterator it = _data.begin();
		it != _data.end(); ++it)
	{
		Key what = _getKeyNumber(it->first);
		switch (what)
		{
		case METHOD:
			_setMethod(it->second);
			break ;
		case ROOT:
			_setRoot(it->second);
			break ;
		case AUTO_INDEX:
			_setAutoIndex(it->second);
			break ;
		case INDEX_PAGES:
			_setIndexPages(it->second);
			break ;
		case CGI:
			_setCGI(it->second);
			break ;
		case CGI_PATH:
			_setCGIPath(it->second);
			break ;
		case RETURN:
			_setReturn(it->second);
			break ;
		case UPLOAD_ENABLE:
			_setUploadEnable(it->second);
			break ;
		case CLIENT_BODY_SIZE:
			_setClientBodySize(it->second);
			break ;
		default:
			return (false);
		}
	}
	return (true);
}

void	Location::_setMethod(const std::string& value)
{
	_methods.clear();
	std::string tmpValue = value;
	while (tmpValue.find(" ") != std::string::npos)
	{
		std::pair<std::string, std::string> splitedValue = splitString(tmpValue, " ");
		_methods.push_back(splitedValue.first);
		tmpValue = splitedValue.second;
	}
	_methods.push_back(tmpValue);
}

void	Location::_setRoot(const std::string& value)
{
	_root = value;
}

void	Location::_setAutoIndex(const std::string& value)
{
	if (value == "on")
		_autoIndex = true;
	else
		_autoIndex = false;
}

void	Location::_setIndexPages(const std::string& value)
{
	std::string tmpValue = value;
	while (tmpValue.find(" ") != std::string::npos)
	{
		std::pair<std::string, std::string> splitedValue = splitString(tmpValue, " ");
		_indexPages.push_back(splitedValue.first);
		tmpValue = splitedValue.second;
	}
	_indexPages.push_back(tmpValue);
}

void	Location::_setCGI(const std::string& value)
{
	_CGI = value;
}

void	Location::_setCGIPath(const std::string& value)
{
	_CGIPath = value;
}

void	Location::_setReturn(const std::string& value)
{
	_return = value;
}

void	Location::_setUploadEnable(const std::string& value)
{
	if (value == "on")
		_uploadEnable = true;
	else
		_uploadEnable = false;
}

void	Location::_setClientBodySize(const std::string& value)
{
	size_t coefficient = strtoul(value.c_str(), NULL, 0);
	_clientBodySize = coefficient;
	if (value.back() == 'k' || value.back() == 'K')
		_clientBodySize = coefficient * (1024);
	if (value.back() == 'g' || value.back() == 'G')
		_clientBodySize = coefficient * (1024 * 1024);
}

void	Location::print(void)
{
	std::cout << "================Location===============\n";
	std::cout << "_path: " << _path << std::endl;
	std::cout << "_root: " << _root << std::endl;
	std::cout << "_autoIndex: " << _autoIndex << std::endl;
	std::cout << "_CGI: " << _CGI << std::endl;
	std::cout << "_CGIPath: " << _CGIPath << std::endl;
	std::cout << "_return: " << _return << std::endl;
	std::cout << "_uploadEnable: " << _uploadEnable << std::endl;
	std::cout << "_clientBodySize: " << _clientBodySize << std::endl;

	std::cout << "_methods: ";
	for (std::vector<std::string>::iterator it = _methods.begin(); it != _methods.end(); ++it)
	{
		std::cout << *it << " ";
	}
	std::cout << "\n_indexPages: ";
	for (std::vector<std::string>::iterator it = _indexPages.begin(); it != _indexPages.end(); ++it)
	{
		std::cout << *it << " ";
	}
	std::cout << "\n====================================\n";
}

const std::map<std::string, std::string>	Location::getData() const
{
	return _data;
}

const std::string							Location::getPath() const
{
	return _path;
}

const std::vector<std::string>&			Location::getMethods() const
{
	return _methods;
}

const std::string							Location::getRoot() const
{
	return _root;
}

bool								Location::getAutoIndex() const
{
	return _autoIndex;
}

const std::vector<std::string>&			Location::getIndexPages() const
{
	return _indexPages;
}

const std::string							Location::getCGI() const
{
	return _CGI;
}

const std::string							Location::getCGIPath() const
{
	return _CGIPath;
}

const std::string							Location::getReturn() const
{
	return _return;
}

bool								Location::getUploadEnable() const
{
	return _uploadEnable;
}

size_t							Location::getClientBodySize() const
{
	return _clientBodySize;
}
