// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************************************************************************************************************

    Sharp Zaurus PDA skeleton driver (SL, ARM/Linux based, 4th generation)

    TODO:
    - PXA-255 ID opcode fails on this
    - ARM TLB look-up errors?
    - RTC irq doesn't fire?
    - For whatever reason, after RTC check ARM executes invalid code at 0-0x200
    - Dumps are questionable to say the least

=========================================================================================================================================
Sharp Zaurus
------------

Personal Information (PI) Series
--------------------------------

Model: Pi^2 T (proof-of-concept model)
Manufacturer: Sharp
Nickname/Series:
Demo/Press Release Date: April 1992
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 288 KB
Flash (Internal): none
Other Storage: none
Slots:
Display: 239x168 dot matrix DSFTN mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA
Keyboard: No
Features: handwriting recognition
Note:

Model: PI-3000
Manufacturer: Sharp
Nickname/Series:
Release Date: October 1, 1993
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 288 KB
Flash (Internal): none
Other Storage: none
Slots:
Display: 239x168 dot matrix DSFTN mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA
Keyboard: No
Features: handwriting recognition, external faxmodem adapter (optional)
Note:

Model: PI-4000
Manufacturer: Sharp
Nickname/Series:
Release Date: June 1994
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 544 KB
Flash (Internal): none
Other Storage: none
Slots:
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA
Keyboard: No
Features: handwriting recognition, external faxmodem adapter (optional), "ink capabilities"?
Note:

Model: PI-4000FX
Manufacturer: Sharp
Nickname/Series:
Release Date: June 1994
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 544 KB
Flash (Internal): none
Other Storage: none
Slots:
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA
Keyboard: No
Features: handwriting recognition, included external faxmodem adapter
Note:

Model: PI-5000
Manufacturer: Sharp
Nickname/Series: AccessZaurus
Release Date: November 1994
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB
Flash (Internal): none
Other Storage: none
Slots:
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: No
Features: handwriting recognition, external faxmodem adapter (optional), email, add-in software capabilities
Note:

Model: PI-5000FX
Manufacturer: Sharp
Nickname/Series: AccessZaurus
Release Date: November 1994
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB
Flash (Internal): none
Other Storage: none
Slots:
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: No
Features: handwriting recognition, included external faxmodem adapter, email, add-in software capabilities
Note:

Model: PI-5000DA
Manufacturer: Sharp
Nickname/Series: AccessZaurus
Release Date: November 1994
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB
Flash (Internal): none
Other Storage: none
Slots:
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: No
Features: handwriting recognition, email, included digital mobile phone adapter, add-in software capabilities
Note:

Model: PI-4500
Manufacturer: Sharp
Nickname/Series:
Release Date: January 1995
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 544 KB
Flash (Internal): none
Other Storage: none
Slots:
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: No
Features: handwriting recognition, included external faxmodem adapter, email, add-in software capabilities
Note:

Model: PI-6000
Manufacturer: Sharp
Nickname/Series: AccessZaurus
Release Date: August 25, 1995
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB
Flash (Internal): none
Other Storage: none
Slots: Flash IC card slot
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: No
Features: upgraded handwriting recognition (over PI-4xxx/5xxx), email, add-in software capabilities
Note:

Model: PI-6000FX
Manufacturer: Sharp
Nickname/Series: AccessZaurus
Release Date: August 25, 1995
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB
Flash (Internal): none
Other Storage: none
Slots: Flash IC card slot
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: No
Features: upgraded handwriting recognition (over PI-4xxx/5xxx), included external faxmodem adapter, email
Note:

Model: PI-6000DA
Manufacturer: Sharp
Nickname/Series: AccessZaurus
Release Date: December 16, 1995
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB
Flash (Internal): none
Slots: Flash IC card slot
Other Storage: none
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: No
Features: upgraded handwriting recognition (over PI-4xxx/5xxx), email, included digital mobile phone adapter
Note:

Model: PI-7000
Manufacturer: Sharp
Nickname/Series: AccessZaurus
Release Date: February 1996
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB
Flash (Internal): 2 MB
Slots: no IC card slot
Other Storage: none
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, internal faxmodem, I/O Port - serial PC Link
Keyboard: No
Features: upgraded handwriting recognition (over PI-4xxx/5xxx), email
Note:

Model: PI-6500
Manufacturer: Sharp
Nickname/Series: AccessZaurus
Release Date: November 22, 1996
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB (715 KB user addressable)
Flash (Internal): none
Other Storage: none
Slots:
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: No
Features: upgraded handwriting recognition (over PI-4xxx/5xxx), email
Note:

Model: PI-8000
Manufacturer: Sharp
Nickname/Series: AccessZaurus
Release Date: January 24, 1997
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible for handwriting recognition
Memory: 1 MB (711 KB user addressable)
Flash (Internal): none
Other Storage: none
Slots:
Display: 319x168 dot matrix mono "widescreen" LCD touchscreen
OS: Sharp Synergy
I/O: IrDA, internal faxmodem, I/O Port - serial PC Link
Keyboard: No
Features: upgraded handwriting recognition (over PI-4xxx/5xxx), email
Note:

Model: PI-6600
Manufacturer: Sharp
Nickname/Series: AccessZaurus
Release Date: September 25, 1997
Availability: Japan
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB
Flash (Internal): none
Other Storage: none
Slots:
Display: 239x168 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, internal faxmodem, I/O Port - serial PC Link
Keyboard: Yes
Features: upgraded handwriting recognition (over PI-4xxx/5xxx), email, included external digital mobile phone adapter, PC PIM software PowerPIMM ver 2.0, Microsoft Excel PC Viewer software
Note:

============================================================================================

PI-B Series (Business/Commercial)
---------------------------------

Model: PI-B304
Manufacturer: Sharp
Nickname/Series:
Release Date: October 1995
Availability: Japan
CPU: NEC V30 (low-power version) 16-bit CPU
Memory: 2 MB RAM
Flash (Internal): 2 MB
Other Storage: none
Slots:
Display: 480x320 dot matrix reflective mono LCD touchscreen
OS: MS-DOS
I/O: IrDA, I/O Port - serial PC Link
Keyboard:
Features:
Note:

Model: PI-B308
Manufacturer: Sharp
Nickname/Series:
Release Date: October 1995
Availability: Japan
CPU: NEC V30 (low-power version) 16-bit CPU
Memory: 2 MB RAM
Flash (Internal): 6 MB
Other Storage: none
Slots:
Display: 480x320 dot matrix reflective mono LCD touchscreen
OS: MS-DOS
I/O: IrDA, I/O Port - serial PC Link
Keyboard:
Features:
Note:

============================================================================================

K-PDA (ZR) Series
-----------------

Model: ZR-5000
Manufacturer: Sharp
Nickname/Series: K-PDA
Release Date: January 1995
Availability: US, Euro
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB RAM
Flash (Internal): none
Other Storage: none
Slots: PCMCIA Type II
Display: 320x240 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: Yes
Features: handwriting recognition, email
Note:

Model: ZR-5000/FX
Manufacturer: Sharp
Nickname/Series: K-PDA
Release Date: January 1995
Availability: US, Euro
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB RAM
Flash (Internal): none
Other Storage: none
Slots: PCMCIA Type II
Display: 320x240 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email
Note:

Model: ZR-5700
Manufacturer: Sharp
Nickname/Series: K-PDA
Release Date: 1995/1996?
Availability: US, Euro
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB RAM (user area: approx. 600K)
Flash (Internal): none
Other Storage: none
Slots: PCMCIA Type II
Display: 320x240 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: Yes
Features: handwriting recognition, email
Note:

Model: ZR-5700
Manufacturer: Sharp
Nickname/Series: K-PDA
Release Date: 1995/1996?
Availability: US, Euro
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB RAM (user area: approx. 600K)
Flash (Internal): none
Other Storage: none
Slots: PCMCIA Type II
Display: 320x240 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email
Note:

Model: ZR-5800
Manufacturer: Sharp
Nickname/Series: K-PDA
Release Date: 1996
Availability: US, Euro
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 2 MB RAM (user area: approx. 1624K)
Flash (Internal): none
Other Storage: none
Slots: PCMCIA Type II
Display: 320x240 backlit dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: Yes
Features: upgraded handwriting recognition, email, digital mobile phone adapter (optional)
Note:

Model: ZR-5800/FX
Manufacturer: Sharp
Nickname/Series: K-PDA
Release Date: 1996
Availability: US, Euro
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 2 MB RAM (user area: approx. 1624K)
Flash (Internal): none
Other Storage: none
Slots: PCMCIA Type II
Display: 320x240 backlit dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: upgraded handwriting recognition, email, digital mobile phone adapter (optional)
Note:

Model: ZR-3000
Manufacturer: Sharp
Nickname/Series: K-PDA
Release Date: 1996
Availability: US, Euro
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB RAM (user area approx. 650K)
Flash (Internal): none
Other Storage: none
Slots:
Display: 320x240 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link
Keyboard: Yes
Features: handwriting recognition, external faxmodem adapter (optional), email
Note:

Model: ZR-3500X
Manufacturer: Sharp
Nickname/Series: K-PDA
Release Date: 1996
Availability: US, Euro
Main CPU: Sharp SC62015-compatible "ESR-L" 8-bit CPU
Sub CPU: Z80-compatible co-processor for handwriting recognition
Memory: 1 MB RAM (user area approx. 650K)
Flash (Internal): 1 MB
Other Storage: none
Slots:
Display: 320x240 dot matrix mono LCD touchscreen
OS: Sharp Synergy
I/O: ASK/IrDA, I/O Port - serial PC Link, internal faxmodem (14.4/9.6 kbit/s)
Keyboard: Yes
Features: handwriting recognition, email
Note:

============================================================================================

MI Series
---------

Model: MI-10
Manufacturer: Sharp
Nickname/Series: ColorZaurus
Release Date: June 25, 1996
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: unknown
Flash (Internal): none
Other Storage: none
Slots:
Display: 320x240 16-bit color TFT LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: No
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-10DC
Manufacturer: Sharp
Nickname/Series: ColorZaurus
Release Date: June 25, 1996
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: unknown
Flash (Internal): none
Other Storage: none
Slots:
Display: 320x240 16-bit color TFT LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: No
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording, digital camera
Note:

Model: MI-504
Manufacturer: Sharp
Nickname/Series: PowerZaurus
Release Date: July 17, 1997
Availability: Japan
CPU: 30 MHz Hitachi SH3 32-bit CPU
Memory: 1.4 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 16-bit color LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: No
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-506
Manufacturer: Sharp
Nickname/Series: PowerZaurus
Release Date: July 17, 1997
Availability: Japan
CPU: 30 MHz Hitachi SH3 32-bit CPU
Memory: 3.4 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 16-bit color LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: No
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-506DC
Manufacturer: Sharp
Nickname/Series: PowerZaurus
Release Date: July 17, 1997
Availability: Japan
CPU: 30 MHz Hitachi SH3 32-bit CPU
Memory: 3.4 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 16-bit color LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: No
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording, digital camera
Note:

Model: MI-106
Manufacturer: Sharp
Nickname/Series: ZaurusPocket
Release Date: November 28, 1997
Availability: Japan
CPU: 30 MHz Hitachi SH3 32-bit CPU
Memory: 6 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 grayscale LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: No
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-106M
Manufacturer: Sharp
Nickname/Series: ZaurusPocket
Release Date: November 28, 1997
Availability: Japan
CPU: 30 MHz Hitachi SH3 32-bit CPU
Memory: 6 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 grayscale LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: No
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-110M
Manufacturer: Sharp
Nickname/Series: ZaurusPocket
Release Date: November 28, 1997
Availability: Japan
CPU: 30 MHz Hitachi SH3 32-bit CPU
Memory: 10 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 grayscale LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: No
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-610
Manufacturer: Sharp
Nickname/Series: PowerZaurus
Release Date: March 3, 1998
Availability: Japan
CPU: 60 MHz Hitachi SH3 32-bit CPU
Memory: 10 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 16-bit color TFT LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: No
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-610DC
Manufacturer: Sharp
Nickname/Series: PowerZaurus
Release Date: March 3, 1998
Availability: Japan
CPU: 60 MHz Hitachi SH3 32-bit CPU
Memory: 10 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 16-bit color TFT LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: No
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording, digital camera
Note:

Model: MI-310
Manufacturer: Sharp
Nickname/Series: PowerZaurus
Release Date: September 4, 1998
Availability: Japan
CPU: 66 MHz Hitachi SH3 32-bit CPU
Memory: 10 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 16-bit color reflective TFT LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: No
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-P1-LA
Manufacturer: Sharp
Nickname/Series: Zaurus i-Geti
Release Date: March 20, 1999
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: 6 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 grayscale LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-P1-A
Manufacturer: Sharp
Nickname/Series: Zaurus i-Geti
Release Date: March 20, 1999
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: 6 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 grayscale LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-P1-W
Manufacturer: Sharp
Nickname/Series: Zaurus i-Geti
Release Date: March 20, 1999
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: 6 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 grayscale LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-EX1
Manufacturer: Sharp
Nickname/Series: Zaurus i-Cruise
Release Date: April 16, 1999
Availability: Japan
CPU: 120 MHz Hitachi SH3 32-bit CPU
Memory: 8 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 640x480 VGA 16-bit color TFT LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

=== Note: Every MI-series after this point _might_ not be a Hitachi SH3 CPU, best references so far call them RISC 32-bit CPUs... ===

Model: MI-P2-B
Manufacturer: Sharp
Nickname/Series: Zaurus i-Geti
Release Date: July 9, 1999
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: 10 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 grayscale LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording, more internal software
Note:

Model: MI-TR1
Manufacturer: Sharp
Nickname/Series: Zaurus i-Cruise (customized)
Release Date: August 1999
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: 16 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 640x480 VGA 16-bit color TFT LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-C1-S
Manufacturer: Sharp
Nickname/Series: PowerZaurus
Release Date: December 7, 1999
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: 16 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 Super Mobile 16-bit color reflective LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-C1-A
Manufacturer: Sharp
Nickname/Series: PowerZaurus
Release Date: December 7, 1999
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: 16 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 Super Mobile 16-bit color reflective LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-P10-S
Manufacturer: Sharp
Nickname/Series: Zaurus i-Geti
Release Date: July 14, 2000
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: 16 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 grayscale LCD touchscreen (not backlit)
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording, more internal software
Note:

Model: MI-J1
Manufacturer: Sharp
Nickname/Series: Internet Dictionary Zaurus
Release Date: August 4, 2000
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: 6 MB
Flash (Internal):
Other Storage: none
Slots:
Display: 320x240 grayscale LCD touchscreen
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording, more internal software, larger dictionary
Note:

Model: MI-E1
Manufacturer: Sharp
Nickname/Series:
Release Date: December 15, 2000
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: 16 MB
Flash (Internal):
Other Storage: none
Slots: Smart Disk/Secure Digital/MMC, CompactFlash
Display: 240x320 16-bit color backlit TFT LCD touchscreen (vertical display)
Audio Controller:
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording, MP3 player, MPEG4 video playback, headphone jack
Note:

Model: MI-L1
Manufacturer: Sharp
Nickname/Series:
Release Date: May 21, 2001
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: 16 MB
Flash (Internal):
Other Storage: none
Slots: Smart Disk/Secure Digital/MMC, CompactFlash
Display: 240x320 16-bit color LCD touchscreen (not backlit, vertical display)
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording
Note:

Model: MI-E21
Manufacturer: Sharp
Nickname/Series:
Release Date: September 7, 2001
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU (faster than MI-E1 CPU)
Memory: 32 MB
Flash (Internal):
Other Storage: none
Slots: Smart Disk/Secure Digital/MMC, CompactFlash
Display: 240x320 16-bit color backlit TFT LCD touchscreen (vertical display)
Audio Controller:
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording, MP3 player, MPEG4 video playback, headphone jack
Note:

Model: MI-E25DC
Manufacturer: Sharp
Nickname/Series:
Release Date: March 15, 2002
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU (same as MI-E21)
Memory: 32 MB
Flash (Internal):
Other Storage: none
Slots: Smart Disk/Secure Digital/MMC, CompactFlash
Display: 240x320 16-bit color backlit TFT LCD touchscreen (vertical display)
Audio Controller:
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O: IrDA, I/O Port - serial PC Link, internal faxmodem
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, email, external digital mobile phone adapter, Internet, audio recording, MP3 player, MPEG4 video playback, headphone jack, 640x480 digital camera
Note:

============================================================================================

MT Series
---------

Model: MT-200
Manufacturer: Sharp
Nickname/Series:
Release Date: December 4, 1998
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: unknown
Flash (Internal):
Other Storage:
Slots:
Display:
Audio Controller:
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O:
Keyboard:
Features:
Note: The "Communication Pals" or "Browser Boards", not technically Zaurus, basically MI technology with a keyboard on top

Model: MT-200SA
Manufacturer: Sharp
Nickname/Series:
Release Date:
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: unknown
Flash (Internal):
Other Storage:
Slots:
Display:
Audio Controller:
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O:
Keyboard:
Features:
Note: The "Communication Pals" or "Browser Boards", not technically Zaurus, basically MI technology with a keyboard on top

Model: MT-300
Manufacturer: Sharp
Nickname/Series:
Release Date: September 9, 1999
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: unknown
Flash (Internal):
Other Storage:
Slots:
Display:
Audio Controller:
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O:
Keyboard:
Features:
Note: The "Communication Pals" or "Browser Boards", not technically Zaurus, basically MI technology with a keyboard on top

Model: MT-300C
Manufacturer: Sharp
Nickname/Series:
Release Date:
Availability: Japan
CPU: unknown MHz Hitachi SH3 32-bit CPU
Memory: unknown
Flash (Internal):
Other Storage:
Slots:
Display:
Audio Controller:
OS: Sharp ZaurusOS (Axe XTAL microkernel)
I/O:
Keyboard:
Features:
Note: The "Communication Pals" or "Browser Boards", not technically Zaurus, basically MI technology with a keyboard on top

============================================================================================

SL Series
---------

Model: SL-5000D
Manufacturer: Sharp
Nickname/Series: Developer Edition
Release Date: 2001
Availability: World
CPU: 206 MHz Intel SA-1110 StrongARM 32-bit CPU
Memory: 32 MB DRAM
Flash (Internal): 16 MB
Other Storage: none
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 240x320 16-bit color reflective TFT LCD touchscreen
Audio Controller:
OS: Lineo Embedix Plus PDA OS (Linux 2.4), Qtopia GUI, Jeode Java VM
I/O: IrDA, internal faxmodem, I/O Port - serial/USB PC Link
Keyboard: Yes
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, headphone jack
Note:

Model: SL-5500
Manufacturer: Sharp
Nickname/Series: Collie
Release Date: March 11, 2002
Availability: World
CPU: 206 MHz Intel SA-1110 StrongARM 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 16 MB
Other Storage: none
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 240x320 16-bit color reflective TFT LCD touchscreen with front light
Audio Controller:
OS: Lineo Embedix Plus PDA OS (Linux 2.4), Qtopia GUI, Jeode Java VM
I/O: IrDA, internal faxmodem, I/O Port - serial/USB PC Link
Keyboard: Yes
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, headphone jack
Note:

Model: SL-5500G
Manufacturer: Sharp
Nickname/Series: Collie
Release Date: March 11, 2002
Availability: Germany?
CPU: 206 MHz Intel SA-1110 StrongARM 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 16 MB
Other Storage: none
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 240x320 16-bit color reflective TFT LCD touchscreen with front light
Audio Controller:
OS: Lineo Embedix Plus PDA OS (Linux 2.4), Qtopia GUI, Jeode Java VM
I/O: IrDA, internal faxmodem, I/O Port - serial/USB PC Link
Keyboard: Yes
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, headphone jack
Note:

Model: SL-A300
Manufacturer: Sharp
Nickname/Series: Discovery
Release Date: July 12, 2002
Availability: Japan
CPU: 200 MHz Intel XScale PXA210 32-bit CPU
Memory: 64 MB DRAM (approx. 32 MB user area)
Flash (Internal): 16 MB
Other Storage: none
Slots: Secure Digital/MMC
Display: 240x320 16-bit color backlit TFT LCD touchscreen
Audio Controller:
OS: Lineo Embedix Plus PDA OS (Linux 2.4), Qtopia GUI, Jeode Java VM
I/O: IrDA, internal faxmodem, I/O Port - serial/USB PC Link
Keyboard: No
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, headphone jack
Note:

Model: SL-B500
Manufacturer: Sharp
Nickname/Series: Poodle
Release Date: December 14, 2002
Availability: Japan
CPU: 400MHz Intel XScale PXA250 32-bit CPU
Memory: 32 MB DRAM
Flash (Internal): 64 MB
Other Storage: 512KB "rescue" mode NOR P2ROM
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 240x320 16-bit color backlit TFT LCD touchscreen
Audio Controller:
OS: Lineo Embedix OS (Linux 2.4), Qtopia GUI
I/O: IrDA, internal faxmodem, I/O Port - serial/USB PC Link
Keyboard: Yes
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, internal speaker/microphone, headphone jack
Note:

Model: SL-5600
Manufacturer: Sharp
Nickname/Series: Poodle
Release Date: April 2?, 2002
Availability: World
CPU: 400MHz Intel XScale PXA250 32-bit CPU
Memory: 32 MB DRAM
Flash (Internal): 64 MB (approx. 33 MB user area)
Other Storage: 512KB "rescue" mode NOR P2ROM
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 240x320 16-bit color relective TFT LCD touchscreen with front light
Audio Controller:
OS: Lineo Embedix3 OS (Linux 2.4.18), Qtopia GUI 1.5.0
I/O: IrDA, internal faxmodem, I/O Port - serial/USB PC Link
Keyboard: Yes
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, internal speaker/microphone, headphone jack
Note: Some have an easily-fixed cache bug on the PXA-250

Model: SL-C700
Manufacturer: Sharp
Nickname/Series: Corgi
Release Date: December 14, 2002
Availability: Japan
CPU: 206 MHz Intel XScale PXA250 32-bit CPU (underclocked to similar speed as SL-5500?)
Memory: 32 MB DRAM
Flash (Internal): 64 MB
Other Storage: 512KB "rescue" mode NOR P2ROM
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller:
OS: Lineo Embedix OS (Linux 2.4.18), Qtopia GUI
I/O: IrDA, internal faxmodem, I/O Port - USB PC Link
Keyboard: Yes
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, screen rotation, internal speaker/microphone, headphone jack
Note:

Model: SL-C750
Manufacturer: Sharp
Nickname/Series: Shepherd
Release Date: 2003
Availability: Japan
CPU: 400 MHz Intel XScale PXA255 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 64 MB (user area: approx. 30 MB)
Other Storage: 512KB "rescue" mode NOR P2ROM
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller:
OS: Metrowerks OpenPDA OS, Qtopia GUI, CVM Java VM
I/O: IrDA, internal faxmodem, I/O Port - USB PC Link
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, screen rotation, updated software, internal speaker/microphone, headphone jack
Note:

Model: SL-C760
Manufacturer: Sharp
Nickname/Series: Husky
Release Date: 2003
Availability: Japan
CPU: 400 MHz Intel XScale PXA255 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 128 MB (user area: approx. 65 MB)
Other Storage: 512KB "rescue" mode NOR P2ROM
Slots: Secure Digital/MMC (possibly SDIO-compatible), CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller:
OS: Metrowerks OpenPDA OS, Qtopia GUI, CVM Java VM
I/O: IrDA, internal faxmodem, I/O Port - USB PC Link
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, screen rotation, internal speaker/microphone, headphone jack
Note:

Model: SL-C860
Manufacturer: Sharp
Nickname/Series: Boxer
Release Date: 2003
Availability: Japan
CPU: 400 MHz Intel XScale PXA255 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 128 MB (user area: approx. 65 MB)
Other Storage: 512KB "rescue" mode NOR P2ROM
Slots: Secure Digital/MMC (possibly SDIO-compatible), CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller:
OS: Metrowerks OpenPDA OS, Qtopia GUI, CVM Java VM
I/O: IrDA, internal faxmodem, USB Device, I/O Port - USB PC Link
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, screen rotation, English-Japanese translation software, internal speaker/microphone, headphone jack
Note:

Model: SL-6000N
Manufacturer: Sharp
Nickname/Series: Tosa
Release Date: early 2004
Availability: World
CPU: 400 MHz Intel XScale PXA255 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 64 MB
Other Storage: 8 MB "rescue" mode NOR P2ROM
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller: Wolfson WM9712
OS: Metrowerks OpenPDA OS, Qtopia GUI, CVM Java VM
I/O: IrDA, internal faxmodem, USB OTG 1.1 (Slave/Host), I/O Port
Keyboard: Yes
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, internal speaker/microphone, headphone jack
Note:

Model: HC-6000N
Manufacturer: Sharp
Nickname/Series:
Release Date: early 2004
Availability: World
CPU: 400 MHz Intel XScale PXA255 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 64 MB
Other Storage: 8 MB "rescue" mode NOR P2ROM
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller: Wolfson WM9712
OS: Microsoft Windows Mobile 2003 Second Edition
I/O: IrDA, internal faxmodem, USB OTG 1.1 (Slave/Host), I/O Port
Keyboard: Yes
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, internal speaker/microphone, headphone jack
Note: Clone - Hitachi FLORA-ie MX1

Model: SL-6000D
Manufacturer: Sharp
Nickname/Series: Tosa
Release Date: early 2004
Availability: World
CPU: 400 MHz Intel XScale PXA255 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 64 MB
Other Storage: 8 MB "rescue" mode NOR P2ROM
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller: Wolfson WM9712
OS: Metrowerks OpenPDA OS, Qtopia GUI, CVM Java VM
I/O: IrDA, internal faxmodem, USB OTG 1.1 (Slave/Host), I/O Port,
Keyboard: Yes
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, internal speaker/microphone, headphone jack
Note:

Model: SL-6000L
Manufacturer: Sharp
Nickname/Series: Tosa
Release Date: early 2004
Availability: World
CPU: 400 MHz Intel XScale PXA255 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 128 MB
Other Storage: 8 MB "rescue" mode NOR P2ROM
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller: Wolfson WM9712
OS: Metrowerks OpenPDA OS, Qtopia GUI, CVM Java VM
I/O: IrDA, internal faxmodem, USB OTG 1.1 (Slave/Host), I/O Port, 802.11b WiFi
Keyboard: Yes
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, internal speaker/microphone, headphone jack
Note:

Model: SL-6000W
Manufacturer: Sharp
Nickname/Series: Tosa
Release Date: early 2004
Availability: World
CPU: 400 MHz Intel XScale PXA255 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 64 MB
Other Storage: 8 MB "rescue" mode NOR P2ROM
Slots: Secure Digital/MMC, CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller: Wolfson WM9712
OS: Metrowerks OpenPDA OS, Qtopia GUI, CVM Java VM
I/O: IrDA, internal faxmodem, USB OTG 1.1 (Slave/Host), I/O Port, 802.11b WiFi, Bluetooth v1.1
Keyboard: Yes
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, internal speaker/microphone, headphone jack
Note:

Model: SL-C3000
Manufacturer: Sharp
Nickname/Series: Spitz
Release Date: October 2004
Availability: Japan
CPU: 416 MHz Intel XScale PXA270 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 16 MB
Other Storage: 4 GB Hitachi MicroDrive
Slots: Secure Digital/MMC (SDIO), CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller: Wolfson WM8750
OS: Lineo uLinux OS, Qtopia GUI
I/O: IrDA, internal faxmodem, USB OTG 1.1 (Slave/Host), I/O Port, 802.11b WiFi, Bluetooth v1.1
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, screen rotation, overclockable, internal speaker/microphone, headphone jack
Note:

Model: SL-C1000
Manufacturer: Sharp
Nickname/Series: Akita
Release Date: March 2005
Availability: World
CPU: 416 MHz Intel XScale PXA270 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 128 MB
Other Storage: none
Slots: Secure Digital/MMC (possibly SDIO-compatible), CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller:
OS: Lineo uLinux OS, Qtopia GUI
I/O: IrDA, internal faxmodem, USB OTG 1.1 (Slave/Host), I/O Port, 802.11b WiFi, Bluetooth v1.1
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, screen rotation, overclockable, internal speaker/microphone, headphone jack
Note:

Model: SL-C3100
Manufacturer: Sharp
Nickname/Series: Borzoi
Release Date: June 2005
Availability: World
CPU: 416 MHz Intel XScale PXA270 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 128 MB
Other Storage: 8 MB "rescue" mode NOR P2ROM, 4 GB Hitachi MicroDrive
Slots: Secure Digital/MMC (SDIO), CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller: Wolfson WM8750
OS: Lineo uLinux OS, Qtopia GUI
I/O: IrDA, internal faxmodem, USB OTG 1.1 (Slave/Host), I/O Port, 802.11b WiFi, Bluetooth v1.1
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, screen rotation, overclockable, internal speaker/microphone, headphone jack, dictionary, map, electronic library
Note:

Model: SL-C3200
Manufacturer: Sharp
Nickname/Series: Terrier
Release Date: March 17, 2006
Availability: World
CPU: 416 MHz Intel XScale PXA270 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 128 MB
Other Storage: 6 GB Hitachi MicroDrive
Slots: Secure Digital/MMC (SDIO) up to 4 GB, CompactFlash Type II
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller: Wolfson WM8750
OS: Lineo uLinux OS, Qtopia GUI
I/O: IrDA, internal faxmodem, USB OTG 1.1 (Slave/Host), I/O Port, 802.11b WiFi, Bluetooth v1.1
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, email, digital mobile phone adapter, Internet, 640x480 digital camera, audio recording, MP3 player, MPEG4 video playback, screen rotation, overclockable, Nuance text-to-speech, updated dictionary, TOEIC, internal speaker/microphone, headphone jack
Note:

============================================================================================

RD Series
---------

Model: RD-CMP2000R
Manufacturer: Sharp
Nickname/Series:
Demo/Press Release Date: October 2006
Availability: Korea
CPU: unknown 32-bit CPU
Memory:
Flash (Internal):
Other Storage:
Slots: Secure Digital/MMC up to 4 GB
Display: 320x240 16-bit color LCD touchscreen
Audio Controller:
OS: Linux, unknown GUI
I/O:
Keyboard: Yes
Features: 1280x960 1.33 MP camera, electronic dictionary, e-Book reader, FM radio, music player
Note:

============================================================================================

"WS Series"
-----------

Model: W-ZERO3 WS003SH
Manufacturer: Willcom (mfg. by Sharp)
Nickname/Series:
Release Date: after January 2007
Availability: Japan
CPU: 416 MHz Intel XScale PXA270 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 128 MB
Other Storage:
Slots: miniSD, W-SIM
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller:
OS: Microsoft Windows Mobile 5.0
I/O: USB PC Link, USB Host, 802.11b WiFi
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, 1280x960 1.33 MP camera, wireless/cellular phone/data features, Internet, email
Note:

Model: W-ZERO3 WS004SH
Manufacturer: Willcom (mfg. by Sharp)
Nickname/Series:
Release Date: after January 2007
Availability: Japan
CPU: 416 MHz Intel XScale PXA270 32-bit CPU
Memory: 64 MB DRAM
Flash (Internal): 256 MB (user area about 197MB)
Other Storage:
Slots: miniSD, W-SIM
Display: 640x480 VGA 16-bit color Sharp "System LCD" semi-transflective backlit CGS TFT LCD touchscreen
Audio Controller:
OS: Microsoft Windows Mobile 5.0
I/O: USB PC Link, USB Host, 802.11b WiFi
Keyboard: Yes (mini-keyboard)
Features: handwriting recognition, 1280x960 1.33 MP camera, wireless/cellular phone/data features, Internet, email, English-Japanese translation dictionary
Note:



*****************************************************************************************************************************************/


#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/pxa255.h"

#define MAIN_CLOCK XTAL_8MHz

class zaurus_state : public driver_device
{
public:
	zaurus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ram(*this, "ram")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT32> m_ram;

	UINT8 m_rtc_tick;
	DECLARE_READ32_MEMBER(pxa255_ostimer_r);
	DECLARE_WRITE32_MEMBER(pxa255_ostimer_w);
	DECLARE_READ32_MEMBER(pxa255_rtc_r);
	DECLARE_WRITE32_MEMBER(pxa255_rtc_w);
	DECLARE_READ32_MEMBER(pxa255_intc_r);
	DECLARE_WRITE32_MEMBER(pxa255_intc_w);
	PXA255_OSTMR_Regs m_ostimer_regs;
	PXA255_INTC_Regs m_intc_regs;

	void pxa255_ostimer_irq_check();
	void pxa255_update_interrupts();
	void pxa255_set_irq_line(UINT32 line, int irq_state);
	TIMER_DEVICE_CALLBACK_MEMBER(rtc_irq_callback);

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
};

#define VERBOSE_LEVEL ( 5 )

INLINE void ATTR_PRINTF(3,4) verboselog( device_t& device, int n_level, const char* s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		device.logerror( "%s: %s", device.machine().describe_context(), buf );
		//printf( "%s: %s", device.machine().describe_context(), buf );
	}
}


void zaurus_state::video_start()
{
}

UINT32 zaurus_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	return 0;
}

void zaurus_state::pxa255_update_interrupts()
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	intc_regs->icfp = (intc_regs->icpr & intc_regs->icmr) & intc_regs->iclr;
	intc_regs->icip = (intc_regs->icpr & intc_regs->icmr) & (~intc_regs->iclr);
	m_maincpu->set_input_line(ARM7_FIRQ_LINE, intc_regs->icfp ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(ARM7_IRQ_LINE,  intc_regs->icip ? ASSERT_LINE : CLEAR_LINE);
}

void zaurus_state::pxa255_set_irq_line(UINT32 line, int irq_state)
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	intc_regs->icpr &= ~line;
	intc_regs->icpr |= irq_state ? line : 0;
	//printf( "Setting IRQ line %08x to %d\n", line, irq_state );
	pxa255_update_interrupts();
}

void zaurus_state::pxa255_ostimer_irq_check()
{
	PXA255_OSTMR_Regs *ostimer_regs = &m_ostimer_regs;

//  logerror("%08x OStimer irq check\n",ostimer_regs->oier);

	pxa255_set_irq_line(PXA255_INT_OSTIMER0, (ostimer_regs->oier & PXA255_OIER_E0) ? ((ostimer_regs->ossr & PXA255_OSSR_M0) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER1, (ostimer_regs->oier & PXA255_OIER_E1) ? ((ostimer_regs->ossr & PXA255_OSSR_M1) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER2, (ostimer_regs->oier & PXA255_OIER_E2) ? ((ostimer_regs->ossr & PXA255_OSSR_M2) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER3, (ostimer_regs->oier & PXA255_OIER_E3) ? ((ostimer_regs->ossr & PXA255_OSSR_M3) ? 1 : 0) : 0);
}

READ32_MEMBER(zaurus_state::pxa255_ostimer_r)
{
	PXA255_OSTMR_Regs *ostimer_regs = &m_ostimer_regs;

	switch(PXA255_OSTMR_BASE_ADDR | (offset << 2))
	{
		case PXA255_OSMR0:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Match Register 0: %08x & %08x\n", ostimer_regs->osmr[0], mem_mask );
			return ostimer_regs->osmr[0];
		case PXA255_OSMR1:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Match Register 1: %08x & %08x\n", ostimer_regs->osmr[1], mem_mask );
			return ostimer_regs->osmr[1];
		case PXA255_OSMR2:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Match Register 2: %08x & %08x\n", ostimer_regs->osmr[2], mem_mask );
			return ostimer_regs->osmr[2];
		case PXA255_OSMR3:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Match Register 3: %08x & %08x\n", ostimer_regs->osmr[3], mem_mask );
			return ostimer_regs->osmr[3];
		case PXA255_OSCR:
			if (0) verboselog(*this, 4, "pxa255_ostimer_r: OS Timer Count Register: %08x & %08x\n", ostimer_regs->oscr, mem_mask );
			// free-running 3.something MHz counter.  this is a complete hack.
			ostimer_regs->oscr += 0x300;
			return ostimer_regs->oscr;
		case PXA255_OSSR:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Status Register: %08x & %08x\n", ostimer_regs->ossr, mem_mask );
			return ostimer_regs->ossr;
		case PXA255_OWER:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Watchdog Match Enable Register: %08x & %08x\n", ostimer_regs->ower, mem_mask );
			return ostimer_regs->ower;
		case PXA255_OIER:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Interrupt Enable Register: %08x & %08x\n", ostimer_regs->oier, mem_mask );
			return ostimer_regs->oier;
		default:
			if (0) verboselog(*this, 0, "pxa255_ostimer_r: Unknown address: %08x\n", PXA255_OSTMR_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(zaurus_state::pxa255_ostimer_w)
{
	PXA255_OSTMR_Regs *ostimer_regs = &m_ostimer_regs;

	switch(PXA255_OSTMR_BASE_ADDR | (offset << 2))
	{
		case PXA255_OSMR0:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Match Register 0: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[0] = data;
			if(ostimer_regs->oier & PXA255_OIER_E0)
			{
				attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[0] - ostimer_regs->oscr);

				//printf( "Adjusting one-shot timer to 200MHz * %08x\n", ostimer_regs->osmr[0]);
				ostimer_regs->timer[0]->adjust(period);
			}
			break;
		case PXA255_OSMR1:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Match Register 1: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[1] = data;
			if(ostimer_regs->oier & PXA255_OIER_E1)
			{
				attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[1] - ostimer_regs->oscr);

				ostimer_regs->timer[1]->adjust(period, 1);
			}
			break;
		case PXA255_OSMR2:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Match Register 2: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[2] = data;
			if(ostimer_regs->oier & PXA255_OIER_E2)
			{
				attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[2] - ostimer_regs->oscr);

				ostimer_regs->timer[2]->adjust(period, 2);
			}
			break;
		case PXA255_OSMR3:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Match Register 3: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[3] = data;
			if(ostimer_regs->oier & PXA255_OIER_E3)
			{
				//attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[3] - ostimer_regs->oscr);

				//ostimer_regs->timer[3]->adjust(period, 3);
			}
			break;
		case PXA255_OSCR:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Count Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->oscr = data;
			break;
		case PXA255_OSSR:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Status Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->ossr &= ~data;
			pxa255_ostimer_irq_check();
			break;
		case PXA255_OWER:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Watchdog Enable Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->ower = data & 0x00000001;
			break;
		case PXA255_OIER:
		{
			int index = 0;
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Interrupt Enable Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->oier = data & 0x0000000f;
			for(index = 0; index < 4; index++)
			{
				if(ostimer_regs->oier & (1 << index))
				{
					//attotime period = attotime::from_hz(200000000) * ostimer_regs->osmr[index];

					//ostimer_regs->timer[index]->adjust(period, index);
				}
			}

			break;
		}
		default:
			verboselog(*this, 0, "pxa255_ostimer_w: Unknown address: %08x = %08x & %08x\n", PXA255_OSTMR_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

READ32_MEMBER(zaurus_state::pxa255_intc_r)
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	switch(PXA255_INTC_BASE_ADDR | (offset << 2))
	{
		case PXA255_ICIP:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller IRQ Pending Register: %08x & %08x\n", intc_regs->icip, mem_mask );
			return intc_regs->icip;
		case PXA255_ICMR:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller Mask Register: %08x & %08x\n", intc_regs->icmr, mem_mask );
			return intc_regs->icmr;
		case PXA255_ICLR:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller Level Register: %08x & %08x\n", intc_regs->iclr, mem_mask );
			return intc_regs->iclr;
		case PXA255_ICFP:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller FIQ Pending Register: %08x & %08x\n", intc_regs->icfp, mem_mask );
			return intc_regs->icfp;
		case PXA255_ICPR:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller Pending Register: %08x & %08x\n", intc_regs->icpr, mem_mask );
			return intc_regs->icpr;
		case PXA255_ICCR:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller Control Register: %08x & %08x\n", intc_regs->iccr, mem_mask );
			return intc_regs->iccr;
		default:
			verboselog(*this, 0, "pxa255_intc_r: Unknown address: %08x\n", PXA255_INTC_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(zaurus_state::pxa255_intc_w)
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	switch(PXA255_INTC_BASE_ADDR | (offset << 2))
	{
		case PXA255_ICIP:
			verboselog(*this, 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller IRQ Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICMR:
			if (0) verboselog(*this, 3, "pxa255_intc_w: Interrupt Controller Mask Register: %08x & %08x\n", data, mem_mask );
			intc_regs->icmr = data & 0xfffe7f00;
			break;
		case PXA255_ICLR:
			if (0) verboselog(*this, 3, "pxa255_intc_w: Interrupt Controller Level Register: %08x & %08x\n", data, mem_mask );
			intc_regs->iclr = data & 0xfffe7f00;
			break;
		case PXA255_ICFP:
			if (0) verboselog(*this, 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller FIQ Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICPR:
			if (0) verboselog(*this, 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICCR:
			if (0) verboselog(*this, 3, "pxa255_intc_w: Interrupt Controller Control Register: %08x & %08x\n", data, mem_mask );
			intc_regs->iccr = data & 0x00000001;
			break;
		default:
			verboselog(*this, 0, "pxa255_intc_w: Unknown address: %08x = %08x & %08x\n", PXA255_INTC_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

READ32_MEMBER(zaurus_state::pxa255_rtc_r)
{
	printf("%08x\n",offset << 2);

	return 0;
}

WRITE32_MEMBER(zaurus_state::pxa255_rtc_w)
{
	printf("%08x %08x\n",offset << 2,data);

}

static ADDRESS_MAP_START( zaurus_map, AS_PROGRAM, 32, zaurus_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_REGION("firmware", 0)
	AM_RANGE(0x40900000, 0x4090000f) AM_READWRITE(pxa255_rtc_r,pxa255_rtc_w)
	AM_RANGE(0x40a00000, 0x40a0001f) AM_READWRITE(pxa255_ostimer_r, pxa255_ostimer_w )
	AM_RANGE(0x40d00000, 0x40d00017) AM_READWRITE(pxa255_intc_r, pxa255_intc_w )
	AM_RANGE(0xa0000000, 0xa07fffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END


static INPUT_PORTS_START( zaurus )
INPUT_PORTS_END



void zaurus_state::machine_start()
{
}

void zaurus_state::machine_reset()
{
}


/* TODO: Hack */
TIMER_DEVICE_CALLBACK_MEMBER(zaurus_state::rtc_irq_callback)
{
	#if 0
	m_rtc_tick++;
	m_rtc_tick&=1;

	if(m_rtc_tick & 1)
		pxa255_set_irq_line(PXA255_INT_RTC_HZ,1);
	else
		pxa255_set_irq_line(PXA255_INT_RTC_HZ,0);
	#endif
}

// TODO: main CPU differs greatly between versions!
static MACHINE_CONFIG_START( zaurus, zaurus_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",PXA255,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(zaurus_map)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("rtc_timer", zaurus_state, rtc_irq_callback, attotime::from_hz(XTAL_32_768kHz))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(zaurus_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_PALETTE_ADD("palette", 8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/* was labeled SL-C500 */
ROM_START( zsl5500 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
	ROM_LOAD( "sl-c500 v1.20 (zimage).bin", 0x000000, 0x13c000, BAD_DUMP CRC(dc1c259f) SHA1(8150744196a72821ae792462d0381182274c2ce0) )
ROM_END

ROM_START( zsl5600 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
	ROM_LOAD( "zaurus sl-b500 - 5600 (zimage).bin", 0x000000, 0x11b6b0, BAD_DUMP CRC(779c70a1) SHA1(26824e3dc563b681f195029f220dfaa405613f9e) )
ROM_END

ROM_START( zslc750 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
	ROM_LOAD( "zaurus sl-c750 (zimage).bin", 0x000000, 0x121544, BAD_DUMP CRC(56353f4d) SHA1(8e1fff6e93d560bd6572c5c163bbd81378693f68) )
ROM_END

ROM_START( zslc760 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
	ROM_LOAD( "zaurus sl-c760 (zimage).bin", 0x000000, 0x120b44, BAD_DUMP CRC(feedcba3) SHA1(1821ad0fc03a8c3832ad5fe2221c21c1ca277508) )
ROM_END

ROM_START( zslc3000 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
	ROM_LOAD( "openzaurus 3.5.3 - zimage-sharp sl-c3000-20050428091110.bin", 0x000000, 0x12828c, BAD_DUMP CRC(fd94510d) SHA1(901f8154b4228a448f5551f0c9f21c2153e1e3a1) )
ROM_END

ROM_START( zslc1000 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
	ROM_LOAD( "openzaurus 3.5.3 - zimage-sharp sl-c1000-20050427214434.bin", 0x000000, 0x128980, BAD_DUMP  CRC(1e1a9279) SHA1(909ac3f00385eced55822d6a155b79d9d25f43b3) )
ROM_END

COMP( 2002, zsl5500,  0,   0, zaurus,  zaurus, driver_device,  0,  "Sharp",      "Zaurus SL-5500 \"Collie\"", MACHINE_IS_SKELETON )
COMP( 2002, zsl5600,  0,   0, zaurus,  zaurus, driver_device,  0,  "Sharp",      "Zaurus SL-5600 / SL-B500 \"Poodle\"", MACHINE_IS_SKELETON )
COMP( 2003, zslc750,  0,   0, zaurus,  zaurus, driver_device,  0,  "Sharp",      "Zaurus SL-C750 \"Shepherd\" (Japan)", MACHINE_IS_SKELETON )
COMP( 2004, zslc760,  0,   0, zaurus,  zaurus, driver_device,  0,  "Sharp",      "Zaurus SL-C760 \"Husky\" (Japan)", MACHINE_IS_SKELETON )
COMP( 200?, zslc3000, 0,   0, zaurus,  zaurus, driver_device,  0,  "Sharp",      "Zaurus SL-C3000 \"Spitz\" (Japan)", MACHINE_IS_SKELETON )
COMP( 200?, zslc1000, 0,   0, zaurus,  zaurus, driver_device,  0,  "Sharp",      "Zaurus SL-C3000 \"Akita\" (Japan)", MACHINE_IS_SKELETON )
