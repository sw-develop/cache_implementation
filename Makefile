CC = gcc
CFLAGS = -Wall
OBJFILES = cache.o main.o
TARGET = cachetest

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES)
clean:
	rm -f $(OBJFILES) $(TARGET) 
