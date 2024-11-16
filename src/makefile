OBJS := main.o
BIN := generator

all: $(BIN)

main.o: main.c
	gcc -c $< -o $@ `pkg-config glib-2.0 --cflags`

$(BIN): $(OBJS)
	gcc $(OBJS) -o $@ `pkg-config glib-2.0 --libs`

clean:
	rm -rf $(BIN) $(OBJS)
