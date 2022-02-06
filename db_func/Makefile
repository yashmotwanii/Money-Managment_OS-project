all: dbb

dbb: main.c 
	gcc -g -o db_create main.c -lsqlite3

clean:
	rm -f db_create