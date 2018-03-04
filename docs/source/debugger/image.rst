.. _debugger-image-list:

Image Debugger Commands
=======================


You can also type **help <command>** for further details on each command in the MAME Debugger interface.

| :ref:`debugger-command-images` -- lists all image devices and mounted files
| :ref:`debugger-command-mount` -- mounts file to named device
| :ref:`debugger-command-unmount` -- unmounts file from named device


 .. _debugger-command-images:

images
------

|  **images**
|
| Used to display list of available image devices.
|
| Examples:
|
|  images
|
| Show list of devices and mounted files for current driver.


 .. _debugger-command-mount:

mount
-----

|  **mount <device>,<filename>**
|
| Mount <filename> to image <device>.
|
| <filename> can be softlist item or full path to file.
|
| Examples:
|
|  mount cart,aladdin
|
| Mounts softlist item aladdin on cart device.


 .. _debugger-command-unmount:

unmount
-------

|  **unmount <device>**
|
| Unmounts file from image <device>.
|
| Examples:
|
|  unmount cart
|
| Unmounts any file mounted on device named cart.


