.. _debugger-image-list:

Media Image Debugger Commands
=============================

:ref:`debugger-command-images`
    lists all image devices and mounted media images
:ref:`debugger-command-mount`
    mounts a media image file to an image device
:ref:`debugger-command-unmount`
    unmounts the media image from a device


.. _debugger-command-images:

images
------

**images**

Lists the instance names for media images devices in the system and the
currently mounted media images, if any.  Brief instance names, as
allowed for command line media options, are listed.  Mounted software
list items are displayed as the list name, software item short name, and
part name, separated by colons; other mounted images are displayed as
file names.

Example:

``images``
    Lists image device instance names and mounted media.

Back to :ref:`debugger-image-list`


.. _debugger-command-mount:

mount
-----

**mount <instance>,<filename>**

Mounts a file on a media device.  The device may be specified by its
instance name or brief instance name, as allowed for command line media
options.

Some media devices allow software list items to be mounted using this
command by supplying the short name of the software list item in place
of a filename for the **<filename>** parameter.

Examples:

``mount flop1,os1xutls.td0``
    Mount the file **os1xutls.td0** on the media device with instance
    name **flop1**.
``mount cart,10yard``
    Mount the software list item with short name **10yard** on the media
    device with instance name **cart**.

Back to :ref:`debugger-image-list`


.. _debugger-command-unmount:

unmount
-------

**unmount <instance>**

Unmounts the mounted media image (if any) from a device.  The device may
be specified by its instance name or brief instance name, as allowed for
command line media options.

Examples:

unmount cart
    Unmounts any media image mounted on the device with instance name
    **cart**.

Back to :ref:`debugger-image-list`
