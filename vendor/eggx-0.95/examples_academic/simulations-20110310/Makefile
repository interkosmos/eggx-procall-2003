OBJS = 	leaf collision collision_xpm double_pendulum coupled_oscillation \
       	snowfall particles snowfall_photo elastic_pendulum \
       	dla dla_line wave schiff slope drum sphere river missile \
       	duffing lorenz collision_circle ca rossler EBdrift stolen_planet \
       	3body_pythagoras vanallen collision_3dxpm demo3d demomesh \
       	gears3d_ctrl rod_ctrl

all:: $(OBJS)

clean::
	rm -f $(OBJS) *.exe

CMD = sh cmd.sh ../../egg -I. -L..
CCMD = gcc -Wall 

3body_pythagoras: 3body_pythagoras.c
	$(CMD) -o 3body_pythagoras  3body_pythagoras.c

ca: ca_ctrl.c
	$(CMD) -o ca ca_ctrl.c

collision: collision_ctrl.c
	$(CMD) -o collision collision_ctrl.c

collision_3dxpm: collision_3d.c stone.h
	$(CMD) -o collision_3dxpm collision_3d.c

collision_circle: collision_circle_ctrl.c gold.h
	$(CMD) -o collision_circle collision_circle_ctrl.c

collision_xpm: collision_xpm_ctrl.c stone.h
	$(CMD) -o collision_xpm collision_xpm_ctrl.c

coupled_oscillation: coupled_oscillation_ctrl.c spring.c stone.h
	$(CMD) -o coupled_oscillation coupled_oscillation_ctrl.c spring.c

dla: dla_ctrl.c
	$(CMD) -o dla dla_ctrl.c

dla_line: dla_line_ctrl.c
	$(CMD) -o dla_line dla_line_ctrl.c

double_pendulum: double_pendulum_ctrl.c stone.h green.h
	$(CMD) -o double_pendulum double_pendulum_ctrl.c

drum: drum_ctrl.c colormode.h
	$(CMD) -o drum drum_ctrl.c

duffing: duffing_ctrl.c colormode.h
	$(CMD) -o duffing duffing_ctrl.c

EBdrift: EBdrift_3d.c e_3d.h
	$(CMD) -o EBdrift EBdrift_3d.c

elastic_pendulum: elastic_pendulum_ctrl.c spring.c stone.h 
	$(CMD) -o elastic_pendulum elastic_pendulum_ctrl.c spring.c

gears3d_ctrl: gears3d_ctrl.c e_3d.h
	$(CMD) -o gears3d_ctrl gears3d_ctrl.c

leaf: leaf_ctrl.c tiny_complex.h
	$(CMD) -o leaf leaf_ctrl.c

lorenz: lorenz_3d.c 
	$(CMD) -o lorenz lorenz_3d.c

missile: missile_ctrl.c penguin.h
	$(CMD) -o missile missile_ctrl.c

particles: particles_ctrl.c orange.h
	$(CMD) -o particles particles_ctrl.c

river: river_ctrl.c penguin.h
	$(CMD) -o river river_ctrl.c

rod_ctrl: rod_ctrl.c rk4fix.h
	$(CMD) -o rod_ctrl rod_ctrl.c

rossler: rossler_3d.c e_3d.h
	$(CMD) -o rossler rossler_3d.c

schiff: schiff_ctrl.c colormode.h
	$(CMD) -o schiff schiff_ctrl.c

slope: slope_ctrl.c slope.h
	$(CMD) -o slope slope_ctrl.c

snowfall: snowfall_ctrl.c tiny_complex.h
	$(CMD) -o snowfall snowfall_ctrl.c 

snowfall_photo: snowfall_photo.c snowfall.h
	$(CMD) -o snowfall_photo snowfall_photo.c 

sphere: sphere_ctrl.c colormode.h
	$(CMD) -o sphere sphere_ctrl.c

stolen_planet: stolen_planet_3d.c
	$(CMD) -o stolen_planet stolen_planet_3d.c

vanallen: vanallen_3d.c
	$(CMD) -o vanallen vanallen_3d.c


wave: wave_ctrl.c colormode.h
	$(CMD) -o wave wave_ctrl.c

demo3d: demo3d.c e_3d.h
	$(CMD) -o demo3d demo3d.c

demomesh: demomesh.c e_3d.h
	$(CMD) -o demomesh demomesh.c
