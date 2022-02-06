all: schedule
# all: schedule dbb

schedule: main.c scheduler.c server.c scheduler.h
	gcc -g -o schedule main.c -pthread -lsqlite3 -lrt

# dbb: db.c 
# 	gcc -g -o db_create main.c -lsqlite3

clean:
	rm -f schedule db_create