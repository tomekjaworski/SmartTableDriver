//
// Created by Tomek on 29.09.2020.
//

#include "ProgramBase.hpp"

#include <limits.h> // PATH_MAX
#include <unistd.h>

ProgramBase::ProgramBase(int argc, const char** argv, const char** env)
        : error_code(-1), done(false)
{
    assert(argc >= 1);
    this->executable_path = argv[0];
    for(int i = 1; i < argc; i++)
        this->arguments.emplace_back(argv[i]);

    for (; *env != nullptr; env++) {
        std::string entry = *env;
        auto pos = entry.find('=');
        if (pos == std::string::npos)
            continue;

        std::string key = entry.substr(0, pos);
        std::string value = entry.substr(pos + 1);
        this->environment[key] = value;
    }
}

std::string ProgramBase::GetWorkingDirectory(void) {
    char buffer[PATH_MAX];
    const char* ptr = getcwd(buffer, PATH_MAX);
    if (ptr == nullptr)
        throw std::system_error(errno, std::system_category(), "Błąd pobierania katalogu roboczego");
    return buffer;
}

std::string ProgramBase::GetEnvironmentValue(const std::string& key, const std::string& defaultValue) {
    if (this->environment.contains(key))
        return this->environment[key];
    return defaultValue;
}

void ProgramBase::Run(void) {
    this->error_code = this->Main(this->arguments);
    this->done = true;
}

int ProgramBase::GetErrorCode(void) const {
    if (!this->done)
        throw std::logic_error("Program is still running; error code not available.");

    return this->error_code;
}

