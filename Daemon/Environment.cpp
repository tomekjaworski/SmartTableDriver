#include "Environment.hpp"

#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>


#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "Environment.hpp"

Environment env;




Environment::Environment(void)
{
	this->init();
}

void Environment::init(void)
{
	char path[PATH_MAX];
	char dest[PATH_MAX];
	
	//asm("int3\n");
	pid_t pid = getpid();
	sprintf(path, "/proc/%d/exe", pid);
	if (readlink(path, dest, PATH_MAX) == -1)
		terminateOnError("readlink", 1);

	this->executable_path = dest;
	//printf(dest);
	
	int pend = strlen(dest);
	while ((dest[pend - 1] != '/') && (dest[pend - 1] != '\\'))
	{
		pend--;
		assert(pend > 0);
	}
	
	dest[pend - 1] = '\x0';
	this->base_path = dest;
}

std::string Environment::getFullPath(const char* relative_path) const
{
	if (relative_path == nullptr)
		return this->base_path;
	
	if (strlen(relative_path) == 0)
		return this->base_path;
		
	if (relative_path[0] == '\\' || relative_path[0] == '/')
		return relative_path; // to jest sciezka bezwzględna
		
	return this->base_path + "/" + std::string(relative_path);
	
}

std::string Environment::getFullPath(const std::string& relative_path) const
{
	return this->getFullPath(relative_path.c_str());
}


void Environment::terminateOnError(const std::string& message, int exit_error_code)
{
	::perror(message.c_str());
	if (errno != 0)
		printf("   errno=%d\n", errno);
	if (exit_error_code != 0)
		exit(exit_error_code);
}

bool Environment::fileExist(const std::string& fname)
{
	FILE* f = fopen(fname.c_str(), "rb");
	if (f == nullptr)
		return false;
		
	fclose(f);
	return true;
}
