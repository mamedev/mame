Warning
=======

- SDL1.3 still is still under development, the following may or may not
  work.
- if you are using wine on unix be sure to disable wintab32.dll

Build SDL 1.3 from SVN
======================

Pull 1.3 from svn. Than 

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
DFB/DFB:    ./mamed -video sdl13 -rd directfb mario 
WIN32/opengl ./mamed -video sdl13 -rd opengl mario

The performance of the directfb driver depends on the combined
support of the kernel framebuffer driver and the directfb driver.
Having loaded radeonfb I get the same performance as with the open source
radeon X11 driver.

Using the SDL software renderer (preferred is -video soft, thought)

X11,DFB,WIN32 ./mamed -video sdl13 -rd software

Soft:
=====

./mamed -mt -video soft  -ym none -numscreens 2 mario

OpenGL:
=======

Plain opengl does work. Anything more advanced like pbo, fbo or glsl will 
most probably not.

	./mamed -mt -video opengl mario -ym none -nogl_pbo -numscreens 2

YUV - modes:
============

	./mamed -mt -video soft -rd software -ym yuy2 -numscreens 2 mario

The "-rd" overwrites the default which is built-in opengl. This renderer
does not support yuv modes. The software driver does support them non-accelarated.
This has been left in for the time Xv is once again implemented in SDL.

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
and the hardware. The current SDL X11 driver needs opengl for rendering but
does not support yuv textures. The "to-be-submitted" SDL directfb driver 
supports all above if the hardware supports it. However, only one YUV-texture
per display is supported. The second window consequently will get "software" 
YUV blitting.

	  
