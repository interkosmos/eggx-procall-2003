all:: colorbars fontset mouse sample

clean::
	rm -f   colorbars fontset mouse sample *.exe

CMD = sh ../cmd.sh ../../egg -L../..

colorbars: colorbars.f
	$(CMD) -o colorbars colorbars.f

fontset: fontset.f
	$(CMD) -o fontset fontset.f

mouse: mouse.f
	$(CMD) -o mouse mouse.f

sample: sample.f
	$(CMD) -o sample sample.f

