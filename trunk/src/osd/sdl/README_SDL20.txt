======================================================================================
SDLMAME now supports SDL from version 2.0 on upwards.
=======================================================================================


Warning
=======

- SDL2.0 still is still under development, the following may or may not
  work.
- if you are using wine on unix be sure to disable wintab32.dll

Known bugs:
===========

* SDL1.3/X11: Some compound keys, e.g. "'" are not supported by SDL driver
* SDL2.0: sdlvideofps does not take -numscreens>1 into account.
* SDL1.3/WIN32: crashes with -rd d3d
* SDL1.3/WIN32: resizing does not work
      
Build SDL 2.0 from HG
======================

Pull 2.0 from hg. Than 

sh autogen.sh
./configure --prefix=/usr/local/sdl13/ --disable-video-svga --enable-video-directfb --enable-fusionsound
make 
[sudo] make install

You may leave away the last two enables, if you do not want to play around with directfb - although it is lightning fast now :-)

To build the files in test, do

cd test
./configure --prefix=/usr/local/sdl13/ --with-sdl-pfx=/usr/local/sdl13
make

Replace /usr/local/sdl13 above with a safe location, this may as well be a directory in $HOME.

Edit sdl.mak to have

SDL_LIBVER = sdl2
SDL_INSTALL_ROOT = /usr/local/sdl13

That's it.

make
./mame -video sdl13 -rd opengl dkong


All drivers
===========

should support:

-waitvsync
-prescale
-resolution[X]
-switchres
-numscreens
-screen[X]

The following modes are working:

SDL13
=====

This is driver using SDL texture and line drawing support. It supports 
-prescale, -filter and -waitvsync.  The driver determines which pixel 
formats perform best and converts textures to these pixel formats and at 
the same time performs any necessary rotation.  

Basic usage examples:

X11/opengl: ./mamed -video sdl13 -rd opengl mario 
DFB/DFB:    ./mamed -video sdl13 -vd directfb -rd directfb mario 

The performance of the directfb driver depends on the combined
support of the kernel framebuffer driver and the directfb driver.
Having loaded radeonfb I get the same performance as with the open source
radeon X11 driver.

Using the SDL software renderer (preferred is -video soft, thought)

X11,DFB,WIN32 ./mamed -video sdl13 -rd software

Soft:
=====

./mamed -mt -video soft  -sm none -numscreens 2 mario

OpenGL:
=======

Plain opengl does work. Anything more advanced like pbo, fbo or glsl will 
most probably not work with more than one screen.

	./mamed -mt -video opengl mario -nogl_pbo -nogl_vbo -nogl_glsl -numscreens 2

YUV - modes:
============

	./mamed -mt -video soft -sm yuy2 mario

Using DirectFB, the following should get you going

	./mamed -mt -video soft -sm yuy2 -vd directfb -rd directfb mario
	
for accelerated blitting on the framebuffer - provided directfb supports it. 
At least my Radeon R480 is supported.

-video soft and -scale_mode (-sm)
=================================

sdlmame supports 7 scale modes using textures in -video soft:

none: All rendering/scaling in software.

hwblit: Rendering in software/scaling with hardware (if supported)

hwbest: Rendering in software/antialiased scaling with hardware (if supported)

yv12, yv12x2, yuy2, yuy2x2: 
Rendering in software / scaling with hardware (if supported)

Whether these are actually hardware accelerated depends on the SDL driver
and the hardware. The SDL directfb driver supports all above if the hardware 
supports it. However, only one YUV-texture per display is supported. 
The second window consequently will get "software" YUV blitting.

	  
