CC=gcc
CFLAGS=-g -fPIC

ODIR=obj
SDIR=src

LIBS=-lm

_OBJ = hacks.o pointers.o entry.o hook.o util.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SDIR)/%.c 
	$(CC) -c -o $@ $< $(CFLAGS)

warhooks.so: $(OBJ)
	$(CC) -shared -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o warhooks.so
