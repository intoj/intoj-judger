all: sablebox runner_linux

sablebox: 
	make -C ./libsablebox

runner_linux: runner_linux.c libsablebox/libsablebox.a
	gcc -I./libsablebox -L./libsablebox runner_linux.c -lsablebox -o runner_linux

