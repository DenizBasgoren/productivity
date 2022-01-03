all:
	gcc productivity.c -g -o productivity
	systemctl restart korsan_PAPP.service