
INCPATH = -I /usr/X11R6/include/
LIBS = -lglut -lGLU -lGL -lXi -lXmu -lX11 -lm -lpthread -Wall
LIBPATH = -L/usr/lib -L/usr/X11R6/lib/
FILES = pawn.o human.o client.o block.o explosion.o buffer.o
FLAGS = -Wall -g -pg

%:%.c
	gcc $(FLAGS) $(INCPATH) $(LIBPATH) $^ $(LIBS) -o $@

cve:cve.cc texture.h
	g++ $(INCPATH) $(LIBPATH) cve.cc $(FILES) $(LIBS) -o cve

all:cve.cc
	g++ -Wall -c pawn.cc
	g++ -Wall -c human.cc
	g++ -Wall -c client.cc
	g++ -Wall -c block.cc
	g++ -Wall -c explosion.cc
	g++ -Wall -c buffer.cc
	g++ $(FLAGS) server.cc -o server
	g++ $(INCPATH) $(LIBPATH) $^ $(FILES) $(LIBS) -DSTART_COLLABORATIVE=1 -o multiuser

%:%.cc
	g++ $(FLAGS) -c $^
	g++ -g $(INCPATH) $(LIBPATH) cve.cc $(FILES) $(LIBS) -o cve

clean:
	rm $(FILES) cve server
