OBJS = clock inkey mousetst mousetst2 mousetst3 loupe plamoclock putimg24 rotate sep_colorbars

all:: $(OBJS)

clean::
	rm -f $(OBJS) *.exe

CMD = sh cmd.sh ../egg -I.. -L..
CCCMD  = gcc -O -Wall

clock: clock.c
	$(CMD) -o clock clock.c

inkey: inkey.c
	$(CMD) -o inkey inkey.c

mousetst: mousetst.c
	$(CMD) -o mousetst mousetst.c

mousetst2: mousetst2.c
	$(CMD) -o mousetst2 mousetst2.c

mousetst3: mousetst3.c
	$(CMD) -o mousetst3 mousetst3.c

loupe: loupe.c
	$(CMD) -o loupe loupe.c

plamoclock: plamoclock.c
	$(CMD) -o plamoclock plamoclock.c

putimg24: putimg24.c
	$(CMD) -o putimg24 putimg24.c

rotate: rotate.c
	$(CMD) -o rotate rotate.c

sep_colorbars: sep_colorbars.c
	$(CMD) -o sep_colorbars sep_colorbars.c

