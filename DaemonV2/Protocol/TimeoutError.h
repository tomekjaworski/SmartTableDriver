//
// Created by Tomek on 10/03/2020.
//

#ifndef UNTITLED_TIMEOUTERROR_H
#define UNTITLED_TIMEOUTERROR_H


#ifndef _TIMEOUT_ERROR_HPP_
#define _TIMEOUT_ERROR_HPP_

#include <stdexcept>


class TimeoutError : public std::runtime_error
{
public:
    TimeoutError(const char* msg)
            : std::runtime_error(msg)
    {
    }

    TimeoutError(const std::string& msg)
            : std::runtime_error(msg)
    {
    }

    ~TimeoutError()
    {
    }

};

#endif // _TIMEOUT_ERROR_HPP_


#endif //UNTITLED_TIMEOUTERROR_H
