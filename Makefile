
CC = gcc
LD = gcc
TEXTURE_PLATFORM_SOURCES = texture_gdk.c

CFLAGS = `pkg-config gdk-pixbuf-2.0 --cflags` -ansi -Wall -c -std=c99
LFLAGS = `sdl-config --libs` -lGL -lglut `pkg-config gdk-pixbuf-2.0 --libs` -lm
DFLAGS = -g
OBJS = sdl-base.o texture_gdk.o texture_common.o util.o collision.o breakoutv2.o

PROG = breakoutv2
DEBUG = NON

TEXTURE_VIEWER_SOURCES = texture_common.c \
                         $(TEXTURE_PLATFORM_SOURCES)
TEXTURE_VIEWER_HEADERS = texture.h

default: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) -o $(PROG) $(DFLAGS)

texture_gdk.o: texture_gdk.c texture.h
	$(CC) $(CFLAGS) texture_gdk.c

texture_common.o: texture_common.c texture.h
	$(CC) $(CFLAGS) texture_common.c

sdl-base.o: sdl-base.c sdl-base.h game-obj.h
	$(CC) $(CFLAGS) sdl-base.c game-obj.h $(DFLAGS) -D$(DEBUG) 

breakout.o: breakoutv2.c sdl-base.h util.h game-obj.h 
	$(CC) $(CFLAGS) breakoutv2.c game-obj.h $(DFLAGS) -D$(DEBUG)

util.o: util.c util.h
	$(CC) $(CFLAGS) util.c $(DFLAGS)
	
collision.o: collision.c collision.h game-obj.h 
	$(CC) $(CFLAGS) collision.c game-obj.h $(DFLAGS)

clean:
	rm -rf *.o *.gch

real_clean:
	 rm -rf *.o .cfh$(PROG)

