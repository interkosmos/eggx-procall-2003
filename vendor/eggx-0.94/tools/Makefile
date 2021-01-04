OBJS = ppmtoh xpmtoh

all:: $(OBJS)

clean::
	rm -f $(OBJS) *.exe

CCCMD  = gcc -O -Wall

ppmtoh: ppmtoh.c
	$(CCCMD) -o ppmtoh ppmtoh.c

xpmtoh: xpmtoh.c
	$(CCCMD) -o xpmtoh xpmtoh.c
