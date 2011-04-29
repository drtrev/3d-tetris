INCS = -I /usr/X11R6/include/
LDLIBS = -lglut -lGLU -lGL -lXmu -lX11 -lm -lpthread -Wall -g -pg
LDFLAGS = -L/usr/lib -L/usr/X11R6/lib/
OBJECTS = pawn.o human.o client.o block.o explosion.o buffer.o
CXXFLAGS = -Wall -g -pg $(INCS)

all: cve server multiuser

cve: $(OBJECTS) cve.o

multiuser: $(OBJECTS) multiuser.o

multiuser.o:cve.cc
	$(CXX) -DSTART_COLLABORATIVE=1 $(CXXFLAGS) -c $^ -o $@

clean:
	rm -f $(OBJECTS) cve.o cve server multiuser
