CC      = gcc
OBJECTS = record.o statistics.o searcher.o splitter_merger.o
SOURCES = record.c statistics.c searcher.c splitter_merger.c
HEADERS = headers/record.h headers/statistics.h headers/util.h
CFLAGS  = -g3 -pedantic -std=c99 -Wall -Wextra

all: searcher splitter_merger

searcher: searcher.o record.o statistics.o $(HEADERS)
	$(CC) $(CFLAGS) searcher.o record.o statistics.o -o searcher

splitter_merger: splitter_merger.o statistics.o $(HEADERS)
	$(CC) $(CFLAGS) splitter_merger.o statistics.o -o splitter_merger

searcher.o: searcher.c $(HEADERS)
	$(CC) -c $(CFLAGS) searcher.c

splitter_merger.o: splitter_merger.c $(HEADERS)
	$(CC) -c $(CFLAGS) splitter_merger.c

record.o: record.c headers/record.h headers/util.h
	$(CC) -c $(CFLAGS) record.c

statistics.o: statistics.c headers/statistics.h
	$(CC) -c $(CFLAGS) statistics.c


clean:
	rm -f $(OBJECTS) searcher splitter_merger

count:
	wc $(SOURCES) $(HEADER)
