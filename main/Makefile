
#THESE PATHS MUST GO TO PLIB, whereever it may be ...
LIBDIR = -L"/tmp/plib/lib"
INCDIR = -I "/tmp/plib/include"

#FOR LINUX
LIBDIR += -L/usr/X11R6/lib

#FOR MACS
#FRAMEWORK = -framework GLUT
#FRAMEWORK += -framework OpenGL

COMPILERFLAGS = -Wall 
CC = gcc
CFLAGS = $(COMPILERFLAGS) $(INCDIR)
#FOR MACS
#LIBRARIES = -lm -lobjc -lstdc++ -lplibpu -lplibfnt -lplibul

#FOR LINUX
LIBRARIES = -lm -lX11 -lglut -lstdc++ -lplibpu -lplibfnt -lplibul

all: puiTest

hello: hello.cpp
	$(CC) $(CFLAGS) $(FRAMEWORK) -o hello hello.cpp $(LIBDIR) $(LIBRARIES)

test: test.cpp
	$(CC) $(CFLAGS) $(FRAMEWORK) -o test test.cpp $(LIBDIR) $(LIBRARIES)

puiTest: puiTest.cpp
	$(CC) $(CFLAGS) $(FRAMEWORK) -o puiTest puiTest.cpp $(LIBDIR) $(LIBRARIES)
