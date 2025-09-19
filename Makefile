CC = g++
CFLAGS = -march=native -mtune=native -O2 -ggdb3
CWARNS = -Wall -Wextra -pedantic
LDFLAGS = -lm -lassimp -lGL -lGLEW -lGLU -lglut -lX11 -lXrandr -lXxf86vm -lXi -lIL -lILU -lILUT

SRCDIR = src
INCDIR = Dependencies

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = AVT_Project

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(CWARNS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(CWARNS) -I$(INCDIR) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean run
