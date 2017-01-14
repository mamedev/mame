Controller aliases
=====================

By default MAME will map all controls and functions to keyboard keys and mouse-/joystickaxis and buttons.
Controls are then listed and configured with generic names like "KEY X", "MOUSE XAXIS", "JOY BUTTON 1" etc.
This works well as long as you use a keyboard, mouse and joystick but if you for example run MAME in
a converted arcade cabinet it gets confusing. Which button on the cabinet is "KEY A" or "MOUSE BUTTON 1".
(if you have confirm_exit activated you get a message saying "Press "KEY ENTER" to exit")

Controller aliases allow you to give arbitrary names to keyboard keys and other input methods.
e.g. change "KEY ENTER" to "Blue button" or "MOUSE XAXIS" to "Spinner"

Usage of controller aliases
============================

Controller aliases must be in a controller file specified in the "ctrlr" tag in a .ini file.
e.g. in mame.ini under "CORE INPUT OPTIONS"
ctrlr		jpac

Each alias is listed under the <aliases> xml element which is under the <system> xml element in the controller file.
The <alias> tag requires a "code" tag and a name as content.
e.g.
<mameconfig version="10">
  <system name="default">
    <input>
	  ...
    </input>
    <aliases>
	  <alias code="KEYCODE_RIGHT">Joy1Right</alias>
	  <alias code="KEYCODE_LEFT">Joy1Left</alias>
      <alias code="KEYCODE_LCONTROL">P1Btn1</alias>
      <alias code="KEYCODE_X">P1Btn6</alias>
      <alias code="MOUSECODE_XAXIS">Spinner</alias>
	</aliases>
  </system>
</mameconfig>

The code is the standard MAME name for input. They are listed in the source file src/emu/input.cpp but the general format is:
KEYCODE_*
MOUSECODE_*
GUNCODE_*
JOYCODE_*

Note that mouse/joystick/gun directions are name XAXIS, YAXIS etc. e.g. MOUSECODE_XAXIS.
(MOUSECODE_X is a valid name but will never happen)

For a complete example look in the jpac.cfg file in the ctrlr folder.
