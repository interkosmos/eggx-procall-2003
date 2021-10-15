OBJS = 2ch nihongo rupin

all:: $(OBJS)

clean::
	rm -f $(OBJS) *.exe

CMD = sh cmd.sh ../egg -I.. -L..
CCCMD  = gcc -Wall

2ch: 2ch.c
	$(CMD) -o 2ch 2ch.c

nihongo: nihongo.c
	$(CMD) -o nihongo nihongo.c

rupin: rupin.c
	$(CMD) -o rupin rupin.c

