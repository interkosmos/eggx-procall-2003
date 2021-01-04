OBJS = landing 9puzzle balance

all:: $(OBJS)

clean::
	rm -f $(OBJS) *.exe

CMD = sh cmd.sh ../egg -I.. -L..
CCCMD  = gcc -Wall

landing: landing.c
	$(CMD) -o landing landing.c

9puzzle: 9puzzle_ctrl.c
	$(CMD) -o 9puzzle 9puzzle_ctrl.c

balance: balance.c
	$(CMD) -o balance balance.c

