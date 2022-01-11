#include <unistd.h>
// for isatty, read
#include <stdio.h>
// for puts, stdio, stderr
#include <errno.h>
// for errno
#include <string.h>
// for strlen
#include <stdlib.h>
// for exit
#include <fcntl.h>
// for open

#define LEN 4*1024
char gameBuf[LEN];

int logFd;

void notifyAndExit(int exitCode) {
	char *errorMsg = strerror(errno);
	fprintf(stderr, "Error: %s\n", errorMsg);
	close(logFd);
	exit(exitCode);
}

int main(int argc, char**argv) {

	if (argc != 2) {
		fputs("Usage: ./getgames logfile\n", stderr);
		return 1;
	}

	errno = 0;
	logFd = open(argv[1], O_RDONLY);
	if (errno) notifyAndExit(2);

	errno = 0;
	int ret, nRead = 0;
	while( ret = read(logFd, &gameBuf+nRead, LEN) ) {
		if (ret == -1) notifyAndExit(3);
		nRead = ret;
	}

	int i = 0;
	while( i<LEN ) {
		int len = strlen( &gameBuf[i] );
		if (len != 0) {
			puts( &gameBuf[i] );
		}
		i += len+1;
	}

	fflush(stdout);

	close(logFd);
	return 0;
}