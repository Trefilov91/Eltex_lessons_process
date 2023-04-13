

CC = gcc
BUILD_DIR = build

EXE += make6_process

SRC = \
make6_process.c

all: $(BUILD_DIR) $(EXE)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(EXE): $(SRC)
	$(CC) $^ -o $(BUILD_DIR)/$@

clean:
	rm -rf $(BUILD_DIR)
