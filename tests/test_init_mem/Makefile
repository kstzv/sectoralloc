CC      := clang

CFLAGS  := -std=c11 \
           -Wall \
           -Wextra \
           -Wpedantic \
           -fsanitize=address \
           -g

TARGET  := test

SRC     := test.c \
           alloc_init_mem.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
