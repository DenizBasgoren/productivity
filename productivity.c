
#include <fcntl.h>
// for open
#include <unistd.h>
// for read
#include <errno.h>
// for errno
#include <string.h>
// for strerror
#include <stdio.h>
// for snprintf
#include <stdlib.h>
// for system
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>
// for perm bit macros



#define USER_ID 1000

// the string hack
#define TOSTR(num) TOSTR_(num)
#define TOSTR_(num) #num

struct record {
	uint16_t game;
	uint16_t _;
	uint32_t startTime;
	uint32_t endTime;
};
struct record currentRecord = {0};

enum gameState {gameIsOn, gameIsPaused};
enum gameState currentGameState = gameIsPaused;

char zeros_4kb[4*1024];

int eventFd, logFd;

void notifyAndExit(int exitCode) {
	char *errorMsg = strerror(errno);
	char notifyBuf[200] = {0};
	snprintf(notifyBuf, 199, "sudo -u deniz DISPLAY=:0 DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/" TOSTR(USER_ID) "/bus notify-send 'PAPP Error %d: %s'", exitCode, errorMsg);
	system(notifyBuf);
	close(eventFd);
	close(logFd);
	exit(exitCode);
}


int main(void) {

	errno = 0;
	eventFd = open("/dev/input/by-path/acpi-SNY5001:00-event-joystick", O_RDONLY);
	if (errno) notifyAndExit(1);

	errno = 0;
	umask(0); // always succeeds, so no need to reset errno
	logFd = open("/home/deniz/Documents/productivity/log", O_CREAT|O_EXCL|O_RDWR|O_APPEND|O_FSYNC, 0666);
	if (errno == EEXIST) {
		// already created

		errno = 0;
		logFd = open("/home/deniz/Documents/productivity/log", O_RDWR|O_APPEND|O_FSYNC);
		if (errno) notifyAndExit(2);

	}
	else if (errno) {
		notifyAndExit(3);
	}
	else {
		// file just created
		errno = 0;
		write(logFd, zeros_4kb, 4*1024);
		if (errno) notifyAndExit(4);
	}
	


	// first read returns 72 chars:
	// T1  T2  T3  T4   0   0   0   0   R1  R2  R3  0   0   0   0   0   I1  I2  I3   I4  I5   0   0   0 
	// T1  T2  T3  T4   0   0   0   0   R1  R2  R3  0   0   0   0   0   I6  I7  I8   I9  I10  0   0   0 
	// T1  T2  T3  T4   0   0   0   0   R1  R2  R3  0   0   0   0   0   0   0   0    0   0    0   0   0 
	// second read returns 48 chars:
	// T1  T2  T3  T4   0   0   0   0   R1  R2  R3  0   0   0   0   0   I11 I12 I13  I14 I15  0   0   0 
	// T1  T2  T3  T4   0   0   0   0   R1  R2  R3  0   0   0   0   0   0   0   0    0   0    0   0   0 

	// where:
	// T1-5: Timer in little endian. T1 is in seconds.
	// R1-3: Random numbers.
	// I1-15: Key identifiers.

	// for the left key:
	// I1-15: [4, 0, 4, 0, 31, 1, 0, 138, 0, 1, 1, 0, 138, 0, 0]
	// for the right key:
	// I1-15: [4, 0, 4, 0, 59, 1, 0, 104, 1, 1, 1, 0, 104, 1, 0]
	// for the middle key:
	// the middle key isn't recognized for some reason.


	char readBuf[4096*2];
	while( true ) {

		// first read should return 72 chars
		errno = 0;
		ssize_t readRet = read(eventFd, readBuf, 4096);
		// needs to be 4096, otherwise throws 'not aligned'
		if (readRet == -1) {
			notifyAndExit(5);
		}
		else if (readRet != 72) {
			continue;
		}

		// second read should return 48 chars
		errno = 0;
		readRet = read(eventFd, readBuf+72, 4096);
		// needs to be 4096, otherwise throws 'not aligned'
		if (readRet == -1) {
			notifyAndExit(6);
		}
		else if (readRet != 48) {
			continue;
		}

		// Debug Stuff:
		// for (int i = 0; i<72+48; i++) {
		// 	if (i % 24 == 0) puts("");
		// 	printf("%+3hhu ", readBuf[i] );
		// }
		// puts("");

		// {
		// 	char notifyBuf[100] = {0};
		// 	snprintf(notifyBuf, 99, "notify-send 'ProductivityApp: Read chars: %d'", readRet);
		// 	system(notifyBuf);
		// 	return 3;
		// }

		char leftKeyIdentifier[15] = {4, 0, 4, 0, 31, 1, 0, 138, 0, 1, 1, 0, 138, 0, 0};
		char rightKeyIdentifier[15] = {4, 0, 4, 0, 59, 1, 0, 104, 1, 1, 1, 0, 104, 1, 0};

		bool itsTheLeftKey = true;
		bool itsTheRightKey = true;

		for (int i = 0; i<5; i++) {
			if (leftKeyIdentifier[i] != readBuf[16+i]) itsTheLeftKey = false;
			if (rightKeyIdentifier[i] != readBuf[16+i]) itsTheRightKey = false;
		}
		for (int i = 5; i<10; i++) {
			if (leftKeyIdentifier[i] != readBuf[40+i-5]) itsTheLeftKey = false;
			if (rightKeyIdentifier[i] != readBuf[40+i-5]) itsTheRightKey = false;
		}
		for (int i = 10; i<15; i++) {
			if (leftKeyIdentifier[i] != readBuf[88+i-10]) itsTheLeftKey = false;
			if (rightKeyIdentifier[i] != readBuf[88+i-10]) itsTheRightKey = false;
		}

		if (!itsTheLeftKey && !itsTheRightKey) {
			continue;
		}
		else if (itsTheRightKey) {
			system("sudo -u deniz DISPLAY=:0 DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/" TOSTR(USER_ID) "/bus notify-send -t 1500 'PAPP: Right Button'");
			continue; // we don't register a callback for the right key yet
		}

		// the left key is pressed.

		if (currentGameState == gameIsPaused) {
			system("sudo -u deniz DISPLAY=:0 DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/" TOSTR(USER_ID) "/bus notify-send -t 1500 'PAPP: Playing'");
			currentGameState = gameIsOn;
			currentRecord.startTime = (uint32_t) time(NULL);
		}
		else if (currentGameState == gameIsOn ) {
			system("sudo -u deniz DISPLAY=:0 DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/" TOSTR(USER_ID) "/bus notify-send -t 1500 'PAPP: Paused'");
			currentGameState = gameIsPaused;
			currentRecord.endTime = (uint32_t) time(NULL);

			errno = 0;
			ssize_t writeRet = write(logFd, &currentRecord, sizeof(currentRecord));
			if (writeRet == -1) notifyAndExit(7);
			printf("%d", (int) writeRet);

			currentRecord.startTime = 0;
			currentRecord.endTime = 0;
		}
		


		


	}

	close(eventFd);
	close(logFd);
	return 0;
}