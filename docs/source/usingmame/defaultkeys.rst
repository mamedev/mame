.. _default-keys:

Default Keys
============

All the keys below are fully configurable in the user interface. This list shows the standard keyboard configuration.


================  ===============================================================================
 Key              | Action
----------------  -------------------------------------------------------------------------------
**Tab**           | Toggles the configuration menu.
**~**             | Toggles the On Screen Display. When the on-screen display is
                  | visible, you can use the following keys to control it:
                  |
                  | * **Up** - select previous parameter to modify
                  | * **Down** - select next parameter to modify
                  | * **Left** - decrease the value of the selected parameter
                  | * **Right** - increase the value of the selected parameter
                  | * **Enter** - reset parameter value to its default
                  | * **Control+Left** - decrease the value by 10x
                  | * **Shift+Left** - decrease the value by 0.1x
                  | * **Alt+Left** - decrease the value by the smallest amount
                  | * **Control+Right** - increase the value by 10x
                  | * **Shift+Right** - increase the value by 0.1x
                  | * **Alt+Right** - increase the value by the smallest amount
                  |
                  | If you are running with -debug, this key sends a 'break' in emulation.
**P**             | Pauses the game.
**Shift+P**       | While paused, advances to next frame. If rewind is enabled, a new rewind save state is also captured.
**Shift+~**       | While paused, loads the most recent rewind save state.
**F2**            | Service Mode for games that support it.
**F3**            | Resets the game.
**Shift+F3**      | Performs a "hard reset", which tears everything down and re-creates it
                  | from scratch. This is a more thorough and complete reset than the reset
                  | you get from hitting F3.
**LCtrl+F3**      | [SDL ONLY] - Toggle uneven stretch.
**F4**            | Shows the game palette, decoded GFX, and any tilemaps. Use the Enter key to
                  | switch between the three modes (palette, graphics, and tilemaps). Press F4
                  | again to turn off the display. The key controls in each mode vary slightly:
                  |
                  | Palette/colortable mode:
                  |  * **[ ]** - switch between palette and colortable modes
                  |  * **Up/Down** - scroll up/down one line at a time
                  |  * **Page Up/Page Down** - scroll up/down one page at a time
                  |  * **Home/End** - move to top/bottom of list
                  |  * **-/+** - increase/decrease the number of colors per row
                  |  * **Enter** - switch to graphics viewer
                  |
                  | Graphics mode:
                  |  * **[ ]** - switch between different graphics sets
                  |  * **Up/Down** - scroll up/down one line at a time
                  |  * **Page Up/Page Down** - scroll up/down one page at a time
                  |  * **Home/End** - move to top/bottom of list
                  |  * **Left/Right** - change color displayed
                  |  * **R** - rotate tiles 90 degrees clockwise
                  |  * **-/+** - increase/decrease the number of tiles per row
                  |  * **Enter** - switch to tilemap viewer
                  |
                  | Tilemap mode:
                  |  * **[ ]** - switch between different tilemaps
                  |  * **Up/Down/Left/Right** - scroll 8 pixels at a time
                  |  * **Shift+Up/Down/Left/Right** - scroll 1 pixel at a time
                  |  * **Control+Up/Down/Left/Right** - scroll 64 pixels at a time
                  |  * **R** - rotate tilemap view 90 degrees clockwise
                  |  * **-/+** - increase/decrease the zoom factor
                  |  * **Enter** - switch to palette/colortable mode
                  |
                  | Note: Not all games have decoded graphics and/or tilemaps.
**LCtrl+F4**      | [*SDL ONLY*] - Toggles keeping aspect ratio.
**LCtrl+F5**      | [*SDL ONLY*] - Toggle Filter.
**Alt+Ctrl+F5**   | [*NON SDL MS WINDOWS ONLY*] - Toggle HLSL Post-Processing.
**F6**            | Toggle cheat mode (if started with "-cheat").
**LCtrl+F6**      | Decrease Prescaling.
**F7**            | Load a save state. You will be requested to press a key to determine which
                  | save state you wish to load.
                  |
                  | *Note that the save state feature is not supported for a large number of*
                  | *drivers. If support is not enabled for a given driver, you will receive*
                  | *a warning when attempting to save or load.*
**LCtrl+F7**      | Increase Prescaling.
**Shift+F7**      | Create a save state. Requires an additional keypress to identify the state,
                  | similar to the load option above.
**F8**            | Decrease frame skip on the fly.
**F9**            | Increase frame skip on the fly.
**F10**           | Toggle speed throttling.
**F11**           | Toggles speed display.
**Shift+F11**     | Toggles internal profiler display (if compiled in).
**Alt+F11**       | Record HLSL Rendered Video.
**F12**           | Saves a screen snapshot.
**Alt+F12**       | Take HLSL Rendered Snapshot.
**Insert**        | [*WINDOW ONLY, NON-SDL*] Fast forward. While held, runs game with
                  | throttling disabled and with the maximum frameskip.
**Page DN**       | [*SDL ONLY*] Fast forward. While held, runs the game with throttling
                  | disabled and with the maximum frameskip.
**Alt+ENTER**     | Toggles between full-screen and windowed mode.
**Scroll Lock**   | Default mapping for the **uimodekey**.
                  |
                  | This key allows users to disable and enable the emulated keyboard
                  | in machines that require it.  All emulations which require emulated
                  | keyboards will start in that mode and you can only access the internal
                  | UI (hitting TAB) by first hitting this key. You can change the initial
                  | status of the emulated keyboard as presented upon start by using
                  | **-ui_active** as detailed below.
**Escape**        | Exits emulator.
================  ===============================================================================
