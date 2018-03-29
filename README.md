
# Firewroks-2d
This is a simple attempt at creating fireworks using particles and simple physics.
This program is coded in C using SDL to open a window to draw to. It can run in any resolution 
but the number of fireworks and height they reach will have to be tweaked depending on your screen resolution.
the can be adjusted by changeing the SCALE and FIREWORKS constants at the beginning of the main.c source file

You will need to have SDL (version 2) installed on your system for this program to work.
This program should compile and run with no or very little modification and on 
any platform like, Linux, Windows, or MacOS. Consult documentation for those platforms on
compilation instructions.

Compilation instructions on a Linux system are as follows..

	gcc main.c renderer.c -lm `sdl2-config --cflags --libs` -o metaballs-2d
    

## Images 
![animation](https://i.imgur.com/ioOZAZg.gif)
