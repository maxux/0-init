EXEC = zinit
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

CFLAGS += -std=gnu99 -O2 -W -Wall -Wextra
LDFLAGS += 

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o

mrproper: clean
	$(RM) $(EXEC)
	$(RM) *.gcno *.gcda *.gcov
