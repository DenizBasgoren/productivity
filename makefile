all:
	gcc productivity.c gameselectgui.c -g -o productivity -lraylib
	gcc getgames.c -g -o getgames
	gcc setgames.c -g -o setgames
	systemctl restart korsan_PAPP.service