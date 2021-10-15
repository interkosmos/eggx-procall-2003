OBJS = galaxies mandel roche raynolds_color

all:: $(OBJS)
	( cd simulations ; make )

clean::
	rm -f $(OBJS) *.exe
	( cd simulations ; make clean )

CMD = sh cmd.sh ../egg -I.. -L..
CCCMD  = gcc -O -Wall

galaxies: galaxies.c
	$(CMD) -o galaxies galaxies.c

mandel: mandel.c
	$(CMD) -o mandel mandel.c

roche: roche.c
	$(CMD) -o roche roche.c

raynolds_color: raynolds_color.c
	$(CMD) -o raynolds_color raynolds_color.c
