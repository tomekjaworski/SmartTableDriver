#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <string>

class Environment
{
	std::string executable_path;
	std::string base_path;
	
public:
	Environment();
	
	const std::string& getExecutable(void) const { return this->executable_path; }
	const std::string& getBasePath(void) const { return this->base_path; }
	
private:
	void init(void);
	
public:
	std::string getFullPath(const char* relative_path) const;
	std::string getFullPath(const std::string& relative_path) const;
	
public:
	static void terminateOnError(const std::string& message, int exit_error_code);
	static bool fileExist(const std::string& fname);
};


extern Environment env;


#endif // ENVIRONMENT_HPP
