all:
	gcc productivity.c -g -o productivity
	gcc getgames.c -g -o getgames
	gcc setgames.c -g -o setgames
	systemctl restart korsan_PAPP.service