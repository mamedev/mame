Stable Controller IDs
===============================

By default, the mapping between devices and controller IDs is not stable. For instance, a gamepad controller may be assigned to "Joy 1" initially, but after a reboot, it may get re-assigned to "Joy 3".

The reason is that MAME enumerates attached devices and assigns controller IDs based on the enumeration order. Factors that can cause controller IDs to change include plugging / unplugging USB devices, changing ports / hubs and even system reboots.

It is quite cumbersome to ensure that controller IDs are always correct.

That's where the "mapdevice" configuration setting comes into the picture. This setting allows you to map a device name to a controller ID, ensuring that the specified device always maps to the same controller ID in MAME.

Usage of mapdevice
------------------
The "mapdevice" xml element is specified under the input xml element in the configuration. It requires two attributes, "device" and "controller".

The "device" attribute specifies the name of the device to match. It may also be a substring of the name. To see the list of available devices, enable verbose output and available devices will then be listed to the console at startup (more on this below).

The "controller" attribute specifies the MAME controller ID. It is made up of a controller class (i.e. "JOYCODE", "GUNCODE", "MOUSECODE") and controller index. For example: "JOYCODE_1".

Example
-------
Here's an example:

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
|

In the above example, we have four device mappings specified:

The first two mapdevice entries map player 1 and 2 lightguns to Gun 1 and Gun 2, respectively. We use a substring of the full raw device names to match each devices. Note that, since this is XML, we needed to escape the '&' in the name using '&amp;'.

The last two mapdevices entries map player 1 and player 2 gamepad controllers to Joy 1 and Joy 2, respectively. In this case, these are XInput devices.

Listing Available Devices
-------------------------
How did we obtain the device names in the above example? Easy!

We simply set verbose to 1 in mame.ini:

|     #
|     # CORE DEBUGGING OPTIONS
|     #
|     **verbose                   1**
|

Then, when MAME is started, it will list available devices to the console. For example:

|     Input: Adding Gun #0:
|     Input: Adding Gun #1:
|     Input: Adding Gun #2: HID-compliant mouse (\\?\HID#VID_045E&PID_0053#7&18297dcb&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd})
|     Input: Adding Gun #3: HID-compliant mouse (\\?\HID#IrDeviceV2&Col08#2&2818a073&0&0007#{378de44c-56ef-11d1-bc8c-00a0c91405dd})
|     Input: Adding Gun #4: HID-compliant mouse (\\?\HID#VID_D209&PID_1602&MI_02#8&389ab7f3&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd})
|     Input: Adding Gun #5: HID-compliant mouse (\\?\HID#VID_D209&PID_1601&MI_02#9&375eebb1&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd})
|     Input: Adding Gun #6: HID-compliant mouse (\\?\HID#VID_1241&PID_1111#8&198f3adc&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd})
|     Skipping DirectInput for XInput compatible joystick Controller (XBOX 360 For Windows).
|     Input: Adding Joy #0: ATRAK Device #1
|     Skipping DirectInput for XInput compatible joystick Controller (XBOX 360 For Windows).
|     Input: Adding Joy #1: ATRAK Device #2
|     Input: Adding Joy #2: XInput Player 1
|     Input: Adding Joy #3: XInput Player 2
|

Furthermore, when devices are mapped using mapdevice, you'll see that in the verbose logging too, such as:

|     Mapped device 'HID-compliant mouse (\\?\HID#VID_D209&PID_1601&MI_02#9&375eebb1&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd})' to Gun #0
|     Mapped device 'HID-compliant mouse (\\?\HID#VID_D209&PID_1602&MI_02#8&389ab7f3&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd})' to Gun #1
|     Mapped device 'XInput Player 1' to Joy #0
|     Mapped device 'XInput Player 2' to Joy #1
|
