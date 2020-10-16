//
// Created by Tomek on 29.09.2020.
//

#ifndef UNTITLED_PROGRAMBASE_HPP
#define UNTITLED_PROGRAMBASE_HPP

#include <vector>
#include <map>
#include <cassert>

#include <filesystem>
namespace fs = std::filesystem;


class ProgramBase {
    fs::path executable_path;
    std::vector<std::string> arguments;
    std::map<std::string, std::string> environment;
    int error_code;
    bool done;

public:
    ProgramBase(int argc, const char** argv, const char** env);
    std::string GetEnvironmentValue(const std::string& key, const std::string& defaultValue = "");
    int GetErrorCode(void) const;

public:
    virtual int Main(const std::vector<std::string>& arguments) = 0;
    void Run(void);


public:
    static std::string GetWorkingDirectory(void);

};


#endif //UNTITLED_PROGRAMBASE_HPP
