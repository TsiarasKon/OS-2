CC         = gcc
CFLAGS     = -g3 -pedantic -std=c99 -Wall -Wextra
SOURCES    = record.c root.c statistics.c searcher.c splitter_merger.c util.c
HEADERS    = headers/record.h headers/statistics.h headers/util.h
COMMON_OBJ = record.o statistics.o util.o


all: searcher splitter_merger root

searcher: searcher.o record.o statistics.o util.o $(HEADERS)
	$(CC) $(CFLAGS) searcher.o $(COMMON_OBJ) -o searcher

splitter_merger: splitter_merger.o searcher $(HEADERS)
	$(CC) $(CFLAGS) splitter_merger.o $(COMMON_OBJ) -o splitter_merger

root: root.o splitter_merger $(HEADERS)
	$(CC) $(CFLAGS) root.o $(COMMON_OBJ) -o myfind


searcher.o: searcher.c $(COMMON_OBJ) $(HEADERS)
	$(CC) -c $(CFLAGS) searcher.c

splitter_merger.o: splitter_merger.c $(COMMON_OBJ) $(HEADERS)
	$(CC) -c $(CFLAGS) splitter_merger.c

root.o: root.c $(COMMON_OBJ) $(HEADERS)
	$(CC) -c $(CFLAGS) root.c

record.o: record.c util.o headers/record.h headers/util.h
	$(CC) -c $(CFLAGS) record.c

statistics.o: statistics.c headers/statistics.h
	$(CC) -c $(CFLAGS) statistics.c

util.o: util.c headers/util.h
	$(CC) -c $(CFLAGS) util.c


clean:
	rm -f $(COMMON_OBJ) root.o searcher.o splitter_merger.o searcher splitter_merger myfind

count:
	wc $(SOURCES) $(HEADERS)
