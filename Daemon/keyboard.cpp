#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>


int kbhit (void)
{
	struct timeval tv = {};
	fd_set rdfs;
	FD_SET (STDIN_FILENO, &rdfs);
	select(STDIN_FILENO + 1, &rdfs, nullptr, nullptr, &tv);
	if (FD_ISSET(STDIN_FILENO, &rdfs))
		return true;
	
	return false;
 
}