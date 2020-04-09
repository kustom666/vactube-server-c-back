default: vactube-server
vactube-server: src/*.c
	gcc -o bin/vactube-server src/*.c -I /usr/include/glib-2.0/ -I /usr/lib/x86_64-linux-gnu/glib-2.0/include/ -lpthread -l:libdill.a  -l:libglib-2.0.a -l:libgmodule-2.0.a