//
// Created by Tomek on 29.09.2020.
//

#include "ProgramBase.hpp"

#include <limits.h> // PATH_MAX
#include <unistd.h>

volatile ProgramBase* ProgramBase::current_context = nullptr;

ProgramBase::ProgramBase(int argc, const char** argv, const char** env)
        : error_code(-1), done(false), signal_is_break_pressed(0)
{
    assert(ProgramBase::current_context == nullptr);
    assert(argc >= 1);

    ProgramBase::current_context = this;

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

    // Zainstaluj uchwyt do sygnału INT
    struct sigaction handler;
    handler.sa_sigaction = ProgramBase::DefaultSIGINTHandler;
    handler.sa_flags =  SA_RESTART | SA_SIGINFO;
    sigemptyset(&handler.sa_mask);
    int result = sigaction(SIGINT, &handler, NULL);
    if (result != 0)
        throw std::system_error(errno, std::system_category(), "Bład instalowania obsługi SIGINT");

}

std::string ProgramBase::GetWorkingDirectory(void) {
    char buffer[PATH_MAX];
    const char* ptr = getcwd(buffer, PATH_MAX);
    if (ptr == nullptr)
        throw std::system_error(errno, std::system_category(), "Błąd pobierania katalogu roboczego");
    return buffer;
}

std::string ProgramBase::GetEnvironmentValue(const std::string& key, const std::string& defaultValue) {
    if (this->environment.count(key) > 0)
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

void ProgramBase::DefaultSIGINTHandler(int signal, siginfo_t *information, void *ucontext) {
    //
    ProgramBase::current_context->signal_is_break_pressed = 1;
}