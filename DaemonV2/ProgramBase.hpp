//
// Created by Tomek on 29.09.2020.
//

#ifndef UNTITLED_PROGRAMBASE_HPP
#define UNTITLED_PROGRAMBASE_HPP

#include <vector>
#include <map>
#include <cassert>

#include <filesystem>
#include <csignal>

namespace fs = std::filesystem;


class ProgramBase {
    fs::path executable_path;
    std::vector<std::string> arguments;
    std::map<std::string, std::string> environment;
    int error_code;
    bool done;

    volatile std::sig_atomic_t signal_is_break_pressed;

public:
    ProgramBase(int argc, const char** argv, const char** env);
    std::string GetEnvironmentValue(const std::string& key, const std::string& defaultValue = "");
    int GetErrorCode(void) const;

    ProgramBase* GetContext(void) { return  const_cast<ProgramBase*>(ProgramBase::current_context); }
    bool IsBreakPressed(void) const { return this->signal_is_break_pressed; }
public:
    virtual int Main(const std::vector<std::string>& arguments) = 0;
    void Run(void);


private:
    volatile static ProgramBase* current_context;
    static void DefaultSIGINTHandler(int, siginfo_t *, void *);

public:
    static std::string GetWorkingDirectory(void);

};


#endif //UNTITLED_PROGRAMBASE_HPP
