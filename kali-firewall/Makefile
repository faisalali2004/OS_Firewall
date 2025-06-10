CC = gcc
CFLAGS = -Wall -Wextra -I./src/firewall -I./src/utils -I./src/config
SRC = src/main.c src/firewall/rules.c src/firewall/manager.c src/utils/logger.c
OBJ = $(SRC:.c=.o)
TARGET = kali-firewall

.PHONY: all clean install uninstall test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

install:
	bash scripts/install.sh

uninstall:
	bash scripts/uninstall.sh

test:
	$(CC) -o test_rules tests/test_rules.c src/firewall/rules.c src/utils/logger.c
	$(CC) -o test_manager tests/test_manager.c src/firewall/manager.c src/utils/logger.c
	./test_rules
	./test_manager
	rm -f test_rules test_manager