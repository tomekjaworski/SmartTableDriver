#ifndef _TIMEOUT_ERROR_HPP_
#define _TIMEOUT_ERROR_HPP_


class timeout_error : public std::runtime_error
{
public:
	timeout_error(const char* msg)
		: std::runtime_error(msg)
	{
	}

	timeout_error(const std::string& msg)
		: std::runtime_error(msg)
	{
	}
	
	~timeout_error()
	{
	}

};

#endif // _TIMEOUT_ERROR_HPP_
