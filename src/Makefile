OBJS = main.o onewire.o maverick.o
LFLAGS = -lpigpio -lrt -lpthread

all : bbqpi

bbqpi: $(OBJS)
	$(CC) $^ -o $@ $(LFLAGS)

maverick.o: maverick.h
onewire.o: onewire.h
main.o : main.h onewire.h maverick.h

.PHONY: clean
clean:
	rm bbqpi $(OBJS)

.PHONY: install
install: bbqpi
	$(INSTALL) bbqpi $(PREFIX)/sbin
