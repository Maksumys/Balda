//
// Created by dev on 16.11.18.
//

#ifndef BALDA_EXCEPTION_HPP
#define BALDA_EXCEPTION_HPP

#include <string>
#include <exception>

class server_exception : public std::exception
{
	std::string err_str;
public:
	explicit server_exception( const std::string &str ) : err_str{ str } { }

	const char* what() const noexcept override
	{
		return err_str.c_str();
	}
};

#endif //BALDA_EXCEPTION_HPP
