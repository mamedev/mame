.. _devicemap:

Stable Controller IDs
=====================

By default, MAME does not assign stable numbers to input devices.  For instance,
a game pad controller may be assigned to “Joy 1” initially, but after
restarting, the same game pad may be reassigned to “Joy 3”.

The reason is that MAME assigns numbers to input devices in the based on
enumeration order.  Factors that can cause this to change include disconnecting
and reconnecting USB or Bluetooth devices, changing ports/hubs, and even just
restarting the computer.  Input device numbers can be quite unpredictable.

This is where the ``mapdevice`` configuration setting comes into the picture.
By adding this setting to a :ref:`controller configuration file <ctrlrcfg>`, you
can ensure that a given input device is always assigned the same number in MAME.


Using mapdevice
---------------

The ``mapdevice`` XML element is added to the ``input`` XML element in the
controller configuration file. It requires two attributes, ``device`` and
``controller``.  Note that ``mapdevice`` elements only take effect in the
controller configuration file (set using the :ref:`-ctrlr option
<mame-commandline-ctrlr>`) – they are ignored in system configuration files and
the default configuration file.

The ``device`` attribute specifies the device ID of the input device to match.
It may also be a substring of the device ID.  To obtain the device ID for an
input device, select it in the :ref:`menus-inputdevices`, and then select **Copy
Device ID**.  The device ID will be copied to the clipboard.  You can also see
input device IDs by turning on verbose logging (more on this later).  The format
of device IDs depends the type of device, selected input provider module and
operating system.  Your input device IDs may look very different to the examples
here.

The ``controller`` attribute specifies the input token for the input device type
(i.e. ``JOYCODE``, ``GUNCODE``, ``MOUSECODE``) and number to assign to the
device, separated by an underscore.  Numbering starts from 1.  For example the
token for the first joystick device will be ``JOYCODE_1``, the second will be
``JOYCODE_2``, and so on.


Example
-------

Here’s an example:

|       <mameconfig version="10">
|           <system name="default">
|               <input>
|                   **<mapdevice device="VID_D209&amp;PID_1601" controller="GUNCODE_1" />**
|                   **<mapdevice device="VID_D209&amp;PID_1602" controller="GUNCODE_2" />**
|                   **<mapdevice device="XInput Player 1" controller="JOYCODE_1" />**
|                   **<mapdevice device="XInput Player 2" controller="JOYCODE_2" />**
|
|                   <port type="P1_JOYSTICK_UP">
|                       <newseq type="standard">
|                           JOYCODE_1_YAXIS_UP_SWITCH OR KEYCODE_8PAD
|                       </newseq>
|                   </port>
|                   ...


In the above example, we have four device mappings specified:

* The first two ``mapdevice`` elements map player 1 and 2 light guns to Gun 1
  and Gun 2, respectively.  We use a substring of the full device IDs to match
  each devices.  Note that, since this is XML, we needed to escape the
  ampersands (``&``) as ``&amp;``.
* The last two ``mapdevices`` elements map player 1 and player 2 gamepad
  controllers to Joy 1 and Joy 2, respectively.  In this case, these are XInput
  game controllers.


Listing Available Devices
-------------------------

There are two ways to obtain device IDs: by copying them from the
:ref:`menus-inputdevices`, or by :ref:`turning on verbose logging
<mame-commandline-verbose>` and finding the messages logged when input devices
are added.

To reach the Input Devices menu from the system selection menu, select **General
Settings**, and the select **Input Devices**.  To reach the input devices menu
from the :ref:`main menu <menus-main>`, select **Input Settings**, then select
**Input Devices**.  From the Input Devices menu, select a device, then select
**Copy Device ID** to copy its device ID to the clipboard.

To use verbose logging, run MAME with the ``-v`` or ``-verbose`` option on the
command line.  Search the output for messages starting with “Input: Adding…”
that show recognised input devices and their respective IDs.

Here an example:

|     Input: Adding lightgun #1:
|     Input: Adding lightgun #2:
|     Input: Adding lightgun #3: HID-compliant mouse (**device id: \\\\?\\HID#VID_045E&PID_0053#7&18297dcb&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd}**)
|     Input: Adding lightgun #4: HID-compliant mouse (**device id: \\\\?\\HID#IrDeviceV2&Col08#2&2818a073&0&0007#{378de44c-56ef-11d1-bc8c-00a0c91405dd}**)
|     Input: Adding lightgun #5: HID-compliant mouse (**device id: \\\\?\\HID#VID_D209&PID_1602&MI_02#8&389ab7f3&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd}**)
|     Input: Adding lightgun #6: HID-compliant mouse (**device id: \\\\?\\HID#VID_D209&PID_1601&MI_02#9&375eebb1&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd}**)
|     Input: Adding lightgun #7: HID-compliant mouse (**device id: \\\\?\\HID#VID_1241&PID_1111#8&198f3adc&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd}**)
|     Skipping DirectInput for XInput compatible joystick Controller (XBOX 360 For Windows).
|     Input: Adding joystick #1: ATRAK Device #1 (**device id: ATRAK Device #1**)
|     Skipping DirectInput for XInput compatible joystick Controller (XBOX 360 For Windows).
|     Input: Adding joystick #2: ATRAK Device #2 (**device id: ATRAK Device #2**)
|     Input: Adding joystick #3: XInput Player 1 (**device id: XInput Player 1**)
|     Input: Adding joystick #4: XInput Player 2 (**device id: XInput Player 2**)
|

Furthermore, when devices are reassigned using ``mapdevice`` elements in the
controller configuration file, you’ll see that in the verbose log output, too,
such as:

|     Input: Remapped lightgun #1: HID-compliant mouse (device id: \\\\?\\HID#VID_D209&PID_1601&MI_02#9&375eebb1&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd})
|     Input: Remapped lightgun #2: HID-compliant mouse (device id: \\\\?\\HID#VID_D209&PID_1602&MI_02#8&389ab7f3&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd})
|     Input: Remapped joystick #1: XInput Player 1 (device id: XInput Player 1)
|     Input: Remapped joystick #2: XInput Player 2 (device id: XInput Player 2)
|


Limitations
-----------

You can only assign stable numbers to devices if MAME receives stable, unique
device IDs from the input device provider and operating system.  This is not
always the case.  For example the SDL joystick provider is not capable of
providing unique IDs for many USB game controllers.

If not all configured devices are connected when MAME starts, the devices that
are connected may not be numbered as expected.
