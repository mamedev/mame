Shifter Toggle Disable
======================

This is an advanced feature for alternative shifter handling for certain older arcade machines such as Spy Hunter and Outrun that used a two-way toggle switch for the shifter. By default, the shifter is treated as a toggle switch. One press of the mapped control for the shifter will switch it from low to high, and another will switch it back. This may not be ideal if you have access to a physical shifter that works identically to how the original machines did. (The input is on when in one gear, the input is off otherwise)

Note that this feature will *not* help controller users and will not help with games that have more than two shifter states (e.g. five gears in modern racing games)

This feature is not exposed through the graphical user interface in any way, as it is an extremely advanced tweak intended explicitly for people who have this specific need, have the hardware to take advantage of it, and the knowledge to use it correctly.



Disabling and Enabling Shifter Toggle
-------------------------------------

This example will use the game Spy Hunter (set *spyhunt*) to demonstrate the exact change needed:

You will need to manually edit the game .CFG file in the CFG folder (e.g. ``spyhunt.cfg``)

Start by loading MAME with the game in question. In our case, that will be **mame spyhunt**.

Set up the controls as you would please, including mapping the shifter. Exit MAME, open the .cfg file in your text editor of choice.

Inside the ``spyhunt.cfg`` file, you will find the following for the input. The actual input code in the middle can and will vary depending on the controller number and what input you have mapped.

|             **<port tag=":ssio:IP0" type="P1_BUTTON2" mask="16" defvalue="16">**
|                 <newseq type="standard">
|                     JOYCODE_1_RYAXIS_NEG_SWITCH OR JOYCODE_1_RYAXIS_POS_SWITCH
|                 </newseq>
|             </port>
|

The line you need to edit will be the port line defining the actual input. For Spy Hunter, that's going to be *P1_BUTTON2*. Add **toggle="no"** to the end of the tag, like follows:

|             **<port tag=":ssio:IP0" type="P1_BUTTON2" mask="16" defvalue="16" toggle="no">**
|                 <newseq type="standard">
|                     JOYCODE_1_RYAXIS_NEG_SWITCH OR JOYCODE_1_RYAXIS_POS_SWITCH
|                 </newseq>
|             </port>
|

Save and exit. To disable this, simply remove the **toggle="no"** from each desired .CFG input.
