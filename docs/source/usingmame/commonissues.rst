Common Issues and Questions (FAQ)
=================================

**Disclaimer: The following information is not legal advice and was not written by a lawyer.**


1. :ref:`rapid-coins`
2. :ref:`broken-package`
3. :ref:`faster-if-X`
4. :ref:`NeoGeo-broken`
5. :ref:`Sega-SGMDC`
6. :ref:`Missing-ROMs`
7. :ref:`ROM-Verify`
8. :ref:`Parent-Sets`
9. :ref:`Legal-ROMs`
10. :ref:`ROMs-Grey`
11. :ref:`Abandonware`
12. :ref:`Old-Sets`
13. :ref:`eBay-cabs`
14. :ref:`ROM-DVDs`
15. :ref:`DMCA-exemption`
16. :ref:`24-hours`
17. :ref:`commercial-use`
18. :ref:`Ultracade`
19. :ref:`Blackscreen-DirectX`
20. :ref:`ControllerIssues`
21. :ref:`ExternalOPL`
22. :ref:`Autofire`
23. :ref:`gsync-freesync`


.. _rapid-coins:

Why does my game show an error screen if I insert coins rapidly?
----------------------------------------------------------------

This is not a bug in MAME.  On original arcade hardware, you simply could not
insert coins as fast as you can mash the button.  The only ways you could feed
credits at that kind of pace was if the coin mech hardware was defective or if
you were physically trying to cheat the coin mech.

In either case, the game would display an error for the operator to look into
the situation to prevent cheating them out of their hard-earned cash.  Keep a
slow, coin-insert-ish pace and you won’t trigger this.


.. _broken-package:

Why is my non-official MAME package (e.g. EmuCR build) broken? Why is my official update broken?
------------------------------------------------------------------------------------------------

Many MAME features, such as software lists, HLSL or BGFX shaders, Lua plugins
and UI translations, use external files.  Updates to these features often
require the external files to be updated along with MAME.  Unfortunately, builds
provided by third parties may only include the main MAME executable, or may
include outdated external files.  Using an updated MAME executable with outdated
external files causes issues with features reliant on the external files.
Despite repeated requests that they distribute MAME complete with matching
external files, some of these third parties persist in distributing incomplete
or broken MAME updates.

As we have no control over how third parties distribute MAME, all we really can
do is recommend against obtaining MAME from sites like EmuCR.  We cannot provide
any support for packages we didn’t build ourselves.  You can completely avoid
these issues by compiling MAME yourself, or using an official package we
provide.

You may also encounter this problem if you do not update the contents of the
``hlsl``, ``bgfx`` or ``plugins`` folders when updating your MAME installation
with a new official build.


.. _faster-if-X:

Why does MAME support console games and dumb terminals? Wouldn't it be faster if MAME had just the arcade games? Wouldn't it take less RAM? Wouldn't MAME be faster if you just X?
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

This is a common misconception. The actual size of the MAME file doesn't affect the speed of it; only the parts that are actively being used are in memory at any given time.

In truth, the additional supported devices are a good thing for MAME as they allow us to stress test sections of the various CPU cores and other parts of the emulation that don't normally see heavy utilization. While a computer and an arcade machine may use the exact same CPU, how they use that CPU can differ pretty dramatically.

No part of MAME is a second-class citizen to any other part. Video poker machines are just as important to document and preserve as arcade games.

There's still room for improvements in MAME's speed, but chances are that if you're not already a skilled programmer any ideas you have will have already been covered. Don't let that discourage you-- MAME is open source, and improvements are always welcome.


.. _NeoGeo-broken:

Why do my Neo Geo ROMs no longer work? How do I get the Humble Bundle Neo Geo sets working?
-------------------------------------------------------------------------------------------

Recently the Neo Geo BIOS was updated to add a new version of the Universe BIOS. This was done between 0.171 and 0.172, and results in an error trying to load Neo Geo games with an un-updated **neogeo.zip** set.

This also affects the Humble Bundle set: the games themselves are correct and up to date as of MAME 0.173 (and most likely will remain so) though you'll have to pull the ROM set .ZIP files out of the package somehow yourself. However, the Neo Geo BIOS set (**neogeo.zip**) included in the Humble Bundle set is incomplete as of the 0.172 release of MAME.

We suggest you contact the provider of your sets (Humble Bundle and DotEmu) and ask them to update their content to the newest revision. If enough people ask nicely, maybe they'll update the package.


.. _Sega-SGMDC:

How can I use the Sega Genesis & Mega Drive Classics collection from Steam with MAME?
-------------------------------------------------------------------------------------

As of the April 2016 update to the program, the ROM images included in the set are now 100% compatible with MAME and other Genesis/Mega Drive emulators. The ROMs are contained in the **steamapps\\Sega Classics\\uncompressed ROMs** folder as a series of *.68K* and *.SGD* images that can be loaded directly into MAME. PDF manuals for the games can be found in **steamapps\\Sega Classics\\manuals** as well.


.. _Missing-ROMs:

Why does MAME report "missing files" even if I have the ROMs?
-------------------------------------------------------------

There can be several reasons for this:

* It is not unusual for the ROMs to change for a game between releases of MAME. Why would this happen? Oftentimes, better or more complete ROM dumps are made, or errors are found in the way the ROMs were previously defined. Early versions of MAME were not as meticulous about this issue, but more recent MAME builds are. Additionally, there can be more features of a game emulated in a later release of MAME than an earlier release, requiring more ROM code to run.
* You may find that some games require CHD files. A CHD file is a compressed representation of a game's hard disk, CD-ROM, or laserdisc, and is generally not included as part of a game's ROMs. However, in most cases, these files are required to run the game, and MAME will complain if they cannot be found.
* Some games such as Neo-Geo, Playchoice-10, Convertible Video System, Deco Cassette, MegaTech, MegaPlay, ST-V Titan, and others need their BIOS ROMs in addition to the game ROMs. The BIOS ROMs often contain ROM code that is used for booting the machine, menu processor code on multi-game systems, and code common to all games on a system. BIOS ROMS must be named correctly and left zipped inside your ROMs folder.
* Older versions of MAME needed decryption tables in order for MAME to emulate Capcom Play System 2 (a.k.a. CPS2) games. These are created by team CPS2Shock.
* Some games in MAME are considered "Clones" of another game. This is often the case when the game in question is simply an alternate version of the same game. Common alternate versions of games include versions with text in other languages, versions with different copyright dates, later versions or updates, bootlegs, etc. "Cloned" games often overlap some of the ROM code as the original or "parent" version of the game. To see if you have any "clones" type "**MAME -listclones**". To run a "cloned game" you simply need to place its parent ROM file in your ROMs folder (leave it zipped).


.. _ROM-Verify:

How can I be sure I have the right ROMs?
----------------------------------------

MAME checks to be sure you have the right ROMs before emulation begins. If you see any error messages, your ROMs are not those tested to work properly with MAME. You will need to obtain a correct set of ROMs through legal methods.

If you have several games and you wish to verify that they are compatible with the current version of MAME, you can use the *-verifyroms* parameter. For example:

**mame -verifyroms robby**
...checks your ROMs for the game *Robby Roto* and displays the results on the screen.

**mame -verifyroms \* >verify.txt**
...checks the validity of ALL the ROMs in your ROMS directory, and writes the results to a textfile called *verify.txt*.


.. _Parent-Sets:

Why is it that some games have the US version as the main set, some have Japanese, and some are the World?
----------------------------------------------------------------------------------------------------------

Parent and clone sets are a convenience feature to help keep different versions
of the same system or software together.  The decision on which set to make the
parent will always be somewhat arbitrary, but we do have some guidelines:

* Prefer latest release version
* Prefer English language
* Prefer most widespread release
* Prefer most complete version
* Prefer versions that are uncensored, and have story/cutscenes intact
* Prefer versions that keep the original gameplay balance
* Prefer releases from original developers/publishers rather than licensees
* Prefer releases without region-specific notices or warnings

It’s not always possible to choose a set that’s preferred according to all
criteria.

As an example, the World release of Ghouls’n Ghosts (*ghouls*) is the parent of
the US release (*ghoulsu*) and the Japanese original Daimakaimura, as it is the
most widespread English-language release, and has the story and cutscenes
intact.

Another example is Midway Pac-Man (*pacman*), which is a clone of Namco Puck
Man (*puckman*), because Pac-Man is a licensed version for the US market, while
Puck Man was released by Namco themselves.


.. _Legal-ROMs:

How do I legally obtain ROMs or disk images to run on MAME?
-----------------------------------------------------------

You have several options:

* You can obtain a license to them by purchasing one via a distributor or vendor
  who has proper authority to do so.
* You can download one of the ROM sets that have been released for free to the
  public for non-commercial use.
* You can purchase an actual arcade PCB, read the ROMs or disks yourself, and
  let MAME use that data.

Beyond these options, you are on your own.


.. _ROMs-Grey:

Isn't copying ROMs a legal gray area?
-------------------------------------

No, it’s not.  You are not permitted to make copies of software without the
copyright owner’s permission.  This is a black and white issue.


.. _Abandonware:

Can't game ROMs be considered abandonware?
------------------------------------------

No.  Even the companies that went under had their assets purchased by somebody,
and that person is the copyright owner.


.. _Old-Sets:

I had ROMs that worked with an old version of MAME and now they don't. What happened?
-------------------------------------------------------------------------------------

As time passes, MAME is perfecting the emulation of older games, even when the results aren't immediately obvious to the user. Often times the better emulation requires more data from the original game to operate. Sometimes the data was overlooked, sometimes it simply wasn't feasible to get at it (for instance, chip "decapping" is a technique that only became affordable very recently for people not working in high-end laboratories). In other cases it's much simpler: more sets of a game were dumped and it was decided to change which sets were which version.


.. _eBay-cabs:

What about those arcade cabinets on eBay that come with all the ROMs?
---------------------------------------------------------------------

If the seller does not have a proper license to include the ROMs with their
system, they are not legally permitted to include any ROMs with the system.
If they have purchased a license to the ROMs in your name from a distributor or
vendor with legitimate licenses, then they may include the ROMs with the
cabinet.  After signing an agreement, cabinet owners that include legitimate
licensed ROMs may be permitted to include a version of MAME that runs those ROMs
and nothing more.


.. _ROM-DVDs:

What about those guys who burn DVDs of ROMs for the price of the media?
-----------------------------------------------------------------------

What they are doing is just as unlawful as selling the ROMs outright.  As long
as somebody holds the copyright, making unauthorised copies is unlawful.  If
someone went on the internet and started a business of selling cheap copies of
U2 albums for the price of media, do you think they would get away with it?

Even worse, a lot of these people like to claim that they are helping the
project.  In reality, they only create more problems for the MAME team.  We are
not associated with these people in any way, regardless of how “official” they
may attempt to appear.  By buying from them, you only help criminals profit from
selling software they have no right to sell.  **Anyone using the MAME name
and/or logo to sell such products is also in violation of the MAME trademark.**


.. _DMCA-exemption:

But isn't there a special DMCA exemption that makes ROM copying legal?
----------------------------------------------------------------------

No, you have misread the exemptions.  The exemption allows people to
reverse-engineer the copy protection or encryption in computer programs that are
obsolete.  The exemption simply means that figuring out how these obsolete
programs worked is not illegal according to the DMCA.  It does not have any
effect on the legality of making unauthorised copies of computer programs, which
is what you are doing if you make copies of ROMs.


.. _24-hours:

But isn't it OK to download and "try" ROMs for 24 hours?
--------------------------------------------------------

This is an urban legend that was made up by people who made ROMs available for
download from their web sites, in order to justify the fact that they were
breaking the law.  There is no provision like this in any copyright law.


.. _commercial-use:

If I buy a cabinet with legitimate ROMs, can I set it up in a public place to make money?
-----------------------------------------------------------------------------------------

Probably not. ROMs are typically only licensed for personal, non-commercial purposes.


.. _Ultracade:

But I've seen Ultracade and Global VR Classics cabinets out in public places? Why can they do it?
-------------------------------------------------------------------------------------------------

Ultracade had two separate products. The Ultracade product is a commercial machine with commercial licenses to the games. These machines were designed to be put on location and make money, like traditional arcade machines. Their other product is the Arcade Legends series. These are home machines with non- commercial licenses for the games, and can only be legally operated in a private environment. Since their buyout by Global VR they only offer the Global VR Classics cabinet, which is equivalent to the earlier Ultracade product.


.. _Blackscreen-DirectX:

HELP! I'm getting a black screen or an error message in regards to DirectX on Windows!
--------------------------------------------------------------------------------------

You probably have missing or damaged DirectX runtimes. You can download the latest DirectX setup tool from Microsoft at https://www.microsoft.com/en-us/download/details.aspx?displaylang=en&id=35

Additional troubleshooting information can be found on Microsoft's website at https://support.microsoft.com/en-us/kb/179113


.. _ControllerIssues:

I have a controller that doesn't want to work with the standard Microsoft Windows version of MAME, what can I do?
-----------------------------------------------------------------------------------------------------------------

By default, MAME on Microsoft Windows tries to read joystick(s), mouse/mice and
keyboard(s) using the RawInput API.  This works with most devices, and allows
multiple keyboards and mice to be used independently.  However, some device
drivers are not compatible with RawInput, and it may be necessary to use
DirectInput or window events to receive input.  This is also the case for most
software that simulates mouse or keyboard input, like JoyToKey, VNC or Remote
Desktop.

You can try changing the
:ref:`keyboardprovider <mame-commandline-keyboardprovider>`,
:ref:`mouseprovider <mame-commandline-mouseprovider>`,
:ref:`joystickprovider <mame-commandline-joystickprovider>` or
:ref:`lightgunprovider <mame-commandline-lightgunprovider>` setting (depending
on which kind of device you’re having issues with) from ``rawinput`` to one of
the other options such as ``dinput`` or ``win32``.  See
:ref:`osd-commandline-options` for details on input provider options


.. _ExternalOPL:

What happened to the MAME support for external OPL2-carrying soundcards?
------------------------------------------------------------------------

MAME 0.23 added support for using a sound card’s onboard OPL2 (Yamaha YM3812
chip) instead of emulating the OPL2.  This feature was only supported for DOS –
it was never supported in official Windows versions of MAME.  It dropped
entirely as of MAME 0.60, as the OPL2 emulation in MAME had become advanced
enough to be a better solution in almost all cases.  MAME’s OPL2 emulation is
now superior to using a sound card’s YM3812 in all cases, especially as modern
sound cards lack a YM3812.


.. _Autofire:

What happened to the MAME support for autofire?
-----------------------------------------------

A Lua plugin providing enhanced autofire support was added in MAME 0.210, and
the built-in autofire functionality was removed in MAME 0.216.  This plugin has
more functionality than the built-in autofire feature it replaced; for example,
you can configure alternate buttons for different autofire rates.

You can enable and configure the new autofire system with the following steps:

* Start MAME with no system selected.
* Choose **Configure Options** at the bottom (use **Tab** to move focus, or
  double-click the menu item).
* Choose **Plugins** near the bottom of the Settings menu.
* Turn **Autofire plugin** on (use **Left**/**Right** or click the arrows to
  change options).
* Exit MAME completely and start MAME again so the setting takes effect.

The setting will be automatically saved for future use.

See :ref:`plugins-autofire` for more information about using the autofire
plugin, or :ref:`plugins` for more information about using plugins with MAME in
general.


.. _gsync-freesync:

Does MAME support G-Sync or FreeSync? How do I configure MAME to use them?
--------------------------------------------------------------------------

MAME supports both G-Sync and FreeSync right out of the box for Windows and Linux, however macOS does not support G-Sync or FreeSync.

* Make sure your monitor is capable of at least 120Hz G-Sync/FreeSync. If your monitor is only capable of 60Hz in G-Sync/FreeSync modes, you will hit problems with drivers such as *Pac-Man* that run at 60.60606Hz and may hit problems with others that are very close to but not quite 60Hz.
* If playing MAME windowed or using the BGFX video system, you'll need to make sure that you have G-Sync/FreeSync turned on for windowed applications as well as full screen in your video driver.
* Be sure to leave triple buffering turned off.
* Turning VSync on is suggested in general with G-Sync and FreeSync.
* Low Latency Mode will not affect MAME performance with G-Sync/FreeSync.

The effects of G-Sync and FreeSync will be most noticeable in drivers that run at refresh rates that are very different from normal PC refresh rates. For instance, the first three *Mortal Kombat* titles run at 54.706841Hz.
