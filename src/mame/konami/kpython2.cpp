// license:BSD-3-Clause
// copyright-holders:windyfairy, Shiz
/***************************************************************************

Konami Python 2 Hardware Overview
Konami


The Python 2 consists of a consumer SCPH-50000 MB/NH Playstation 2 with an
additional I/O board housed in a metal case. The PS2 itself is labeled
"COH-H55000" in the DDR Supernova wiring manual.

The contents of the HDD are encrypted using DNAS in a way that ties the
data to a specific HDD and PS2 combination. Additionally, random bytes are
added to the encrypted files during installation so no two dumps will be the
same, even if you install the same game on the same exact hardware multiple times.

The HDD used is always an original Sony HDD which implements a special SCE identity
command in the firmware. The results of the command are saved in the ROM dumps as
"ps2_hdd_id". The file contains information about the originating hardware as well as
random bytes which are used for the encryption process to tie the HDD and the encrypted
data together.

All HDDs used are DNAS initialized, which is a process that involved contacting the
Sony DNAS servers to register the machine + HDD configuration and then writing an
RSA encrypted blob into the __net partition. The encrypted blob contains information
about the PS2 hardware it is linked with, in particular the ILINK ID. The ILINK ID
stored inside the encrypted blob is compared against the ILINK ID of the PS2 machine that
is trying to boot the HDD. If the two do not match then it is impossible to launch the
HDD.
The raw ILINK ID can be found at 0x1e0 - 0x1e8 in the "ps2_nvram" file.


How to backup Python 2 games
----------------------------
To back up a Python 2 game, you must at the very least have a working HDD
and ideally the original PS2 hardware that is tied to the machine.

If you have a game or datecode version that is undumped but you do not have the original PS2 hardware,
it is still possible to recover the ILINK ID out of the __net partition with some extra effort
so please still submit the HDD image + HDD ID if possible.

(dev note: This process involves decrypting the encrypted blob on the __net partition to get
encoded ILINK ID or dumping the encoded ILINK ID from memory with a debugger then running
it through the appropriate decode routine and has a 100% recovery success rate.
Debugger method: breakpoint at 0x207a70 (should be a memcmp call), skip first breakpoint hit, then pull
the encoded ILINK ID out of the memory address located in the a1 register on next breakpoint hit.)

An HDD image without the corresponding HDD ID is unusable and cannot be booted.

1. Run a BIOS + NVRAM dumper tool on the PS2 machine to grab the NVRAM. You should get
a file with the extension "NVM", and will be 0x400 bytes in size.
Recommended tools:
- PS2 BIOS dumper (tested, will be named "SCPH-50000_BIOS_V10_JAP_190.NVM")
- ps2ident (tested, will be named "SCPH-50000_NVM.bin")
- biosdrain

2. Dump the raw HDD image using tool of your choice.

3. HDD ID can be dumped using sg_raw. Trim resulting file down to 0x80 bytes.
Replace "/dev/sda" with the appropriate target HDD to dump the HDD ID from.
(command for direct IDE connection) sg_raw -o HDD_ID.bin -b -r 512 /dev/sda 85 09 0d 00 ec 00 00 00 00 00 00 00 00 00 8e 00
(command for JMicron-based USB-IDE adapter) sg_raw -o HDD_ID.bin -b -r 512 /dev/sda df 10 00 02 00 ec 00 00 00 00 00 8e


PYTHON2 SUB PCB
---------------
UNIT,PCB(A)
PWB110526140000 or PWB0000359443

|-----------------------------------------------------------------|
|            PORT1  PORT2                                         |
|  FAN                                      CN2        CN5        |
|                                                             CN6 |
|                                                                 |
|      USB                                                    CN8 |
|                                                                 |
|   ANALOG                                                     SW |
|                                                                 |
|                                                                 |
|        JAMMA        |--|  CN17 CN16   CN11  CN12 CN13 CN14 CN15 |
|---------------------|  |----------------------------------------|

Note: Names surrounded by () couldn't be read from pictures and
should be replaced when the real label is found.

Note: Not all machines have Port 2 I/O exposed on front panel
but the header is still be on the board.

(PORT1) - Port 1 I/O
(PORT2) - Port 2 I/O
CN2 - PS2 AV Multi
CN5 - PS2 Optical/Digital Out (Audio)
CN6 - Plug Black
(USB) - PS2 USB connection
CN8 - Plug White
(ANALOG) - Analog I/O
(FAN) - DC Fan
CN11 - Mini D-Sub 15 pin (RGB)
CN12 - Line Out 1 L/White
CN13 - Line Out 1 R/Red
CN14 - Line Out 2 L/White
CN15 - Line Out 2 R/Red
CN16 - COM2
CN17 - COM1
SW - 4 position DIP switch


Python 2 I/O subboard's USB device descriptor + configuration descriptor
    ---------------------- Device Descriptor ----------------------
bLength                  : 0x12 (18 bytes)
bDescriptorType          : 0x01 (Device Descriptor)
bcdUSB                   : 0x101 (USB Version 1.01)
bDeviceClass             : 0x00 (defined by the interface descriptors)
bDeviceSubClass          : 0x00
bDeviceProtocol          : 0x00
bMaxPacketSize0          : 0x08 (8 bytes)
idVendor                 : 0x0000
idProduct                : 0x7305
bcdDevice                : 0x0000
iManufacturer            : 0x00 (No String Descriptor)
iProduct                 : 0x00 (No String Descriptor)
iSerialNumber            : 0x00 (No String Descriptor)
bNumConfigurations       : 0x01 (1 Configuration)
Data (HexDump)           : 12 01 01 01 00 00 00 08 00 00 05 73 00 00 00 00   ...........s....
                           00 01                                             ..

    ------------------ Configuration Descriptor -------------------
bLength                  : 0x09 (9 bytes)
bDescriptorType          : 0x02 (Configuration Descriptor)
wTotalLength             : 0x0028 (40 bytes)
bNumInterfaces           : 0x01 (1 Interface)
bConfigurationValue      : 0x01 (Configuration 1)
iConfiguration           : 0x00 (No String Descriptor)
bmAttributes             : 0xC0
 D7: Reserved, set 1     : 0x01
 D6: Self Powered        : 0x01 (yes)
 D5: Remote Wakeup       : 0x00 (no)
 D4..0: Reserved, set 0  : 0x00
MaxPower                 : 0x32 (100 mA)
Data (HexDump)           : 09 02 28 00 01 01 00 C0 32 09 04 00 00 03 00 00   ..(.....2.......
                           00 00 07 05 83 03 10 00 03 07 05 81 02 40 00 0A   .............@..
                           07 05 02 02 40 00 0A 34                           ....@..4

Note: The last 0x34 byte appears to be a garbage byte but is part of the descriptor. The garbage byte causes
the USB device to not work properly when connected on later versions of Windows. It's possible to communicate
with the Python 2 I/O device directly on earlier versions of Windows (XP has been confirmed working) and Linux.
The PS2 is unaffected by the garbage byte.


Endpoint 1 (IN, Bulk): Command handler, receives commands from the PS2
Endpoint 2 (OUT, Bulk): Command responses, sends responses back to the PS2
Endpoint 3 (IN, Interrupt): JAMMA I/O + analog I/O state response
	Response:
        0x00 - 0x04 JAMMA I/O (IN)
        0x04 - 0x06 Analog I/O (ANALOG1)
        0x06 - 0x08 Analog I/O (ANALOG2)
        0x08 - 0x0a Analog I/O (ANALOG3)
        0x0a - 0x0c Analog I/O (ANALOG4)



MC-S PCB
--------
UNIT,PCB(S)
GNC30-PWB(F)

Intended to be used for memory card support in DDR Supernova games
but the functionality was scrapped. The MC-S PCB can be tested in the
operator menu of DDR SuperNOVA/SuperNOVA 2 and the QC tester.

|----------------------------|
|                   CN1  CN2 |
| PWR                        |
|                            |
|                        CN5 |
|  CN7  CN6                  |
|----------------------------|

(PWR) - +5V, labeled MC-PWR on outer case
CN1 - Connects to PS2's slot 1 controller/memory card port
CN2 - Connects to PS2's slot 1 controller/memory card port (shield)
CN5 - Not connected? Labeled MCIF on outer case.
CN6 - MCIF2, connects to COM1 on Python 2 I/O board
CN7 - MCIF1, connects to COM2 on Python 2 I/O board




The Python 2 was an internet connected machine. Not all games made use
of the internet functionality but it was possible to download game
updates from the e-amusement servers. Large updates were distributed via DVD.

The games that are known to have taken advantage of online updates includes:
    - Dance Dance Revolution SuperNOVA (JA/AA)
    - Dance Dance Revolution SuperNOVA 2 (JA/AA/UA)
    - Guitar Freaks DrumMania V/V2/V3 (JA/AA)

Known released game and version list:
Boxes marked with [x] have been dumped.
Does not include internal or loctest datecodes.
                                                    Game
Game Title                             Region       Code     Notes
------------------------------------------------------------------------------
DANCE 86.4 FUNKY RADIO STATION         JA           E01
    [ ] E01:J:A:A:2005040400 (initial release)

Dance Dance Revolution SuperNOVA       JA           FDH
    [ ] FDH:J:A:A:2006062000 (initial release)
    [ ] FDH:J:A:A:2006070400 (online update?)
    [x] FDH:J:A:A:2006090600 (online update)

Dance Dance Revolution SuperNOVA       AA           FDH      Distinct from JA version
    [x] FDH:A:A:A:2006071300

Dance Dance Revolution SuperNOVA       UA           FDH      UA does not have online
    [x] FDH:U:A:A:2006072400 (via update DVD labeled KNAD-00008)

Dance Dance Revolution SuperNOVA 2     JA/AA        GDJ
    [x] GDJ:J:A:A:2007071100 (via update DVD labeled KNAD-00010)
    [ ] GDJ:J:A:A:2007081000 (online update?)
    [x] GDJ:J:A:A:2007100800 (online update)

    [x] GDJ:A:A:A:2007071100 (via update DVD labeled KNAD-00010)
    [ ] GDJ:A:A:A:2007081000 (online update?)
    [x] GDJ:A:A:A:2007100800 (online update)

Dance Dance Revolution SuperNOVA 2     UA           GDJ      Distinct from JA/AA version
    [x] GDJ:U:A:A:2007100800 (via update DVD labeled KNAD-00012)

Dancing Stage SuperNOVA                EA           FDH      EA does not have online
    [x] FDH:E:A:A:2006032200 (initial release)
    [x] FDH:E:A:A:2006072500 (via update DVD labeled KNAD-00009)

DrumMania V                            JA/AA        E02
    [x] E02:J:A:A:2005050200 (online update?)

    [x] E02:A:A:A:2005050200 (online update?)

DrumMania V2                           JA/AA        F02
    [x] F02:J:A:A:2005101600 (via update DVD labeled KNAD-00002)
    [x] F02:J:A:A:2005112800 (via update DVD labeled KNAD-00005)
    [x] F02:J:A:A:2006011201 (online update)

    [x] F02:A:A:A:2005101600 (via update DVD labeled KNAD-00002)
    [x] F02:A:A:A:2005112800 (via update DVD labeled KNAD-00005)
    [x] F02:A:A:A:2006011201 (online update)

DrumMania V3                           JA/AA        F32
    [x] F32:J:A:A:2006072600 (via update DVD labeled KNAD-00007)
    [x] F32:J:A:A:2006101800 (online update)

    [x] F32:A:A:A:2006072600 (via update DVD labeled KNAD-00007)
    [x] F32:A:A:A:2006101800 (online update)

Guitar Freaks V                        JA/AA        E03
    [x] E03:J:A:A:2005050200 (online update?)

    [x] E03:A:A:A:2005050200 (online update?)

Guitar Freaks V2                       JA/AA        F03
    [x] F03:J:A:A:2005101600 (via update DVD labeled KNAD-00001)
    [x] F03:J:A:A:2005112800 (via update DVD labeled KNAD-00004)
    [x] F03:J:A:A:2006011201 (online update)

    [x] F03:A:A:A:2005101600 (via update DVD labeled KNAD-00001)
    [x] F03:A:A:A:2005112800 (via update DVD labeled KNAD-00004)
    [x] F03:A:A:A:2006011201 (online update)

Guitar Freaks V3                       JA/AA        F33
    [x] F33:J:A:A:2006072600 (via update DVD labeled KNAD-00006)
    [x] F33:J:A:A:2006101800 (online update)

    [x] F33:A:A:A:2006072600 (via update DVD labeled KNAD-00006)
    [x] F33:A:A:A:2006101800 (online update)

Thrill Drive 3                         JA/?         D44
    [x] D44:J:A:A:20050316-102 (initial release?)

Toy's March                            JA           E00
    [x] E00:J:A:A:2005011602 (initial release)

Toy's March 2                          JA           F00
    [x] F00:J:A:A:2005110400 (initial release)
------------------------------------------------------------------------------


Known update DVDs:
Disc Label        Game Code     Title
------------------------------------------------------------------------------
[x] KNAD-00001    F03 JA A03    GuitarFreaks V2
[x] KNAD-00002    F02 JA A03    DrumMania V2
[ ] KNAD-00003    ?             ?
[x] KNAD-00004    F03 JA B03    GuitarFreaks V2 Version 1.01
[x] KNAD-00005    F02 JA B03    DrumMania V2 Version 1.01
[x] KNAD-00006    F33 JA A03    GuitarFreaks V3
[x] KNAD-00007    F32 JA A03    DrumMania V3
[x] KNAD-00008    FDH UA A03    Dance Dance Revolution SuperNOVA Install Disk
[x] KNAD-00009    FDH EA A03    Dancing Stage SuperNOVA Install Disk
[x] KNAD-00010    GDJ JA A03    Dance Dance Revolution SuperNOVA 2 Install Disk
[ ] KNAD-00011    ?             ?
[x] KNAD-00012    GDJ UA A03    Dance Dance Revolution SuperNOVA 2 Install Disk
------------------------------------------------------------------------------

How to update using DVDs:
Insert DVD and reset PS2. The bootloader will go through the initial boot process
as normal and then automatically start installing the bootloader from the installer DVD.
You must have a valid dongle for the game you wish to install inserted for the
installer to also install the game data on the DVD. For example, if you have the
GDJUAA03 DVD inserted then you must have a G*GDJUAA dongle inserted for it to even
detect the game data on the installer DVD.


Game specific notes
-------------------
If the attachment device configurations are not as expected the games will not boot.
Any port with nodes uses the ACIO protocol for communication.

Dance Dance Revolution series:
- 3 attachment devices
Port 1: EXTIO
Port 2, node 1: ICCA card reader
Port 2, node 2: ICCA card reader

Guitar Freaks series:
Uses 4 channel audio to separate in-game sounds and the BGM. Only game on the Python 2
platform that uses more than 2 channels for audio.

- 2 attachment devices
Port 1, node 1: ICCA card reader
Port 1, node 2: ICCA card reader
Port 2: Unpopulated

DrumMania series:
- 1 attachment device
Port 1, node 1: ICCA card reader
Port 2: Unpopulated

Thrill Drive:
Will give a data warning on first boot after installation. Press test button to skip.

- 2 attachment devices
Port 1: Unpopulated?
Port 2, node 1: Steering wheel device ("HNDL")
Port 2, node 2: Seat belt device ("BELT")

Toy's March:
- 1 attachment device
Port 1: Drum pad I/O
Port 2: Unpopulated

Dance 86.4:
Uses the same GN845-PWB(B) "DDR Stage Multiplexor" as defined in ksys573.cpp.
The output data bit and clock are sent through the lamp/output command (0x24)
through endpoint 1 on the USB device. The 0x24 command functions in the same
way as ksys573's ddr_output_callback.



TODO:
    Python 2 is based on consumer PS2, should be derived from ps2sony.cpp
    Everything

***************************************************************************/


#include "emu.h"
#include "cpu/mips/mips3.h"
#include "cpu/mips/mips1.h"
//#include "machine/ds2430.h"
#include "machine/timekpr.h"
#include "emupal.h"
#include "screen.h"


class kpython2_state : public driver_device
{
public:
	kpython2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ps2_nvram(*this, "ps2_nvram")
		, m_ps2_hdd_id(*this, "ps2_hdd_id")
		, m_ds2430_black_rom(*this, "ds2430_black")
		, m_ds2430_white_rom(*this, "ds2430_white")
	{ }

	void kpython2(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ps2_map(address_map &map);

	// devices
	required_device<mips3_device> m_maincpu;

	required_region_ptr<uint8_t> m_ps2_nvram; // Contains an ILINK ID that must match the one encrypted on the HDD
	required_region_ptr<uint8_t> m_ps2_hdd_id; // Must match the HDD that the data was encrypted against

	required_region_ptr<uint8_t> m_ds2430_black_rom;
	required_region_ptr<uint8_t> m_ds2430_white_rom;

	// driver_device overrides
	virtual void video_start() override;
};


void kpython2_state::video_start()
{
}

uint32_t kpython2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void kpython2_state::ps2_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram();
	map(0x1fc00000, 0x1fdfffff).rom().region("bios", 0);
}

void kpython2_state::kpython2(machine_config &config)
{
	R5000LE(config, m_maincpu, 294000000); // imported from namcops2.c driver
	m_maincpu->set_icache_size(16384);
	m_maincpu->set_dcache_size(16384);
	m_maincpu->set_addrmap(AS_PROGRAM, &kpython2_state::ps2_map);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(kpython2_state::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);

	PALETTE(config, "palette").set_entries(65536);
}

// TODO: ICCA inputs for Guitar Freaks, Drummania, Dance Dance Revolution/Dancing Stage
static INPUT_PORTS_START(kpython2)
    // DIPSW is returned via endpoint 1 command 0x27 on the USB device
	PORT_START("DIPSW")
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "DIP SW:4")
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "DIP SW:3")
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "DIP SW:2") // Unofficially, disables I/O checks for some games
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "DIP SW:1")

	PORT_START("IN")
	PORT_BIT(0x0fffffff, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10000000, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_NAME("Test")
	PORT_BIT(0x20000000, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x40000000, IP_ACTIVE_LOW, IPT_SERVICE2)
	PORT_BIT(0x80000000, IP_ACTIVE_LOW, IPT_COIN2)

	PORT_START("ANALOG1")
	PORT_BIT(0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("ANALOG2")
	PORT_BIT(0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("ANALOG3")
	PORT_BIT(0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("ANALOG4")
	PORT_BIT(0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN)

    // TODO: Add a toggle for 15 kHz/31 kHz video mode
    // Based on schematics, the 15K/31K flag is read from the JAMMA header pin S on the real I/O board.
    // The flag itself is used by endpoint 1 command 0x23, which returns 0 if 31 kHz mode or 0x80 if 15 kHz mode.

INPUT_PORTS_END

static INPUT_PORTS_START(ddr)
	PORT_INCLUDE(kpython2)

	PORT_MODIFY("IN")
	PORT_BIT(0x00000100, IP_ACTIVE_LOW, IPT_START1) PORT_PLAYER(1) PORT_NAME("P1 Start")
	PORT_BIT(0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_16WAY PORT_PLAYER(1) PORT_NAME("P1 Up")
	PORT_BIT(0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_16WAY PORT_PLAYER(1) PORT_NAME("P1 Down")
	PORT_BIT(0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_16WAY PORT_PLAYER(1) PORT_NAME("P1 Left")
	PORT_BIT(0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_16WAY PORT_PLAYER(1) PORT_NAME("P1 Right")
	PORT_BIT(0x00004000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("P1 Select Left")
	PORT_BIT(0x00008000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("P1 Select Right")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_START2) PORT_PLAYER(2) PORT_NAME("P2 Start")
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_16WAY PORT_PLAYER(2) PORT_NAME("P2 Up")
	PORT_BIT(0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_16WAY PORT_PLAYER(2) PORT_NAME("P2 Down")
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_16WAY PORT_PLAYER(2) PORT_NAME("P2 Left")
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_16WAY PORT_PLAYER(2) PORT_NAME("P2 Right")
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2) PORT_NAME("P2 Select Left")
	PORT_BIT(0x00800000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("P2 Select Right")

INPUT_PORTS_END

static INPUT_PORTS_START(gtrfrks)
	PORT_INCLUDE(kpython2)

	PORT_MODIFY("IN")
	PORT_BIT(0x00000100, IP_ACTIVE_LOW, IPT_START1) PORT_PLAYER(1) PORT_NAME("P1 Start")
	PORT_BIT(0x00000200, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(1) PORT_NAME("P1 Pick")
	PORT_BIT(0x00000400, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_PLAYER(1) PORT_NAME("P1 Wailing")
	// PORT_BIT(0x00001800, 0x0000, IPT_DIAL) PORT_PLAYER(1) PORT_NAME("P1 Effect Knob") PORT_MINMAX(0x0000, 0x0300) PORT_SENSITIVITY(1) PORT_KEYDELTA(8) PORT_REVERSE
	PORT_BIT(0x00002000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("P1 Button R")
	PORT_BIT(0x00004000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("P1 Button G")
	PORT_BIT(0x00008000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1) PORT_NAME("P1 Button B")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_START2) PORT_PLAYER(2) PORT_NAME("P2 Start")
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(2) PORT_NAME("P2 Pick")
	PORT_BIT(0x00040000, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_PLAYER(2) PORT_NAME("P2 Wailing")
	// PORT_BIT(0x00180000, 0x0000, IPT_DIAL) PORT_PLAYER(2) PORT_NAME("P2 Effect Knob") PORT_MINMAX(0x0000, 0x0300) PORT_SENSITIVITY(1) PORT_KEYDELTA(8) PORT_REVERSE
	PORT_BIT(0x00200000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2) PORT_NAME("P2 Button R")
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("P2 Button G")
	PORT_BIT(0x00800000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2) PORT_NAME("P2 Button B")

INPUT_PORTS_END

static INPUT_PORTS_START(drmn)
	PORT_INCLUDE(kpython2)

	PORT_MODIFY("IN")
	PORT_BIT(0x00000100, IP_ACTIVE_LOW, IPT_START1) PORT_PLAYER(1) PORT_NAME("Start")
	PORT_BIT(0x00000200, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1) PORT_NAME("Hi-Hat")
	PORT_BIT(0x00000400, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(1) PORT_NAME("Snare")
	PORT_BIT(0x00000800, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("High Tom")
	PORT_BIT(0x00001000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("Low Tom")
	PORT_BIT(0x00002000, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_PLAYER(1) PORT_NAME("Cymbal")
	PORT_BIT(0x00008000, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(1) PORT_NAME("Bass Drum")
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_16WAY PORT_PLAYER(1) PORT_NAME("Select L")
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_16WAY PORT_PLAYER(1) PORT_NAME("Select R")

INPUT_PORTS_END

static INPUT_PORTS_START(thrild3)
	PORT_INCLUDE(kpython2)

	PORT_MODIFY("IN")
	PORT_BIT(0x00000100, IP_ACTIVE_LOW, IPT_START1) PORT_PLAYER(1) PORT_NAME("Start")
	PORT_BIT(0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("Shift Down")
	PORT_BIT(0x00000400, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("Shift Up")

	// TODO: Rework sensitivity values in future when driver is working
	PORT_MODIFY("ANALOG1")
    PORT_BIT(0x0000ffff, 0, IPT_DIAL) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_NAME("Steering Wheel")

	PORT_MODIFY("ANALOG2")
    PORT_BIT(0x0000ffff, 0, IPT_PEDAL) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Accelerator Pedal")

	PORT_MODIFY("ANALOG3")
    PORT_BIT(0x0000ffff, 0, IPT_PEDAL) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Brake Pedal")

    // TODO: Seatbelt toggle, returned by the BELT attachment device

INPUT_PORTS_END

static INPUT_PORTS_START(toysmarch)
	PORT_INCLUDE(kpython2)

	PORT_MODIFY("IN")
	PORT_BIT(0x00000100, IP_ACTIVE_LOW, IPT_START1) PORT_PLAYER(1) PORT_NAME("P1 Start")
	PORT_BIT(0x00000800, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("P1 Select Left")
	PORT_BIT(0x00001000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("P1 Select Left")
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_START2) PORT_PLAYER(2) PORT_NAME("P2 Start")
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2) PORT_NAME("P1 Select Left")
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("P1 Select Left")

    // TODO: Drum pad inputs
    // The drum pad inputs (P1 and P2 cymbal, drum L, and drum R) are pressure sensitive
    // The drum pad values are returned by the drum pad attachment device

INPUT_PORTS_END

#define KPYTHON2_BIOS  \
	    ROM_REGION32_LE(0x400000, "bios", 0) \
		ROM_LOAD("ps2-0190j-20030822.bin", 0x000000, 0x400000, CRC(79d60546) SHA1(0ea98a25a32145dda514de2f0d4bfbbd806bd00c))

ROM_START(kpython2)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)

	DISK_REGION("ide:0:hdd:image")
ROM_END

// Dance Dance Revolution
ROM_START(ddrsnj)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqfdhjaa.bin", 0x00, 0x28, BAD_DUMP CRC(d1e4d436) SHA1(60c8a3cc8fb0691d32e3b83fea052b25e7ba6265))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqfdhjaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("fdh_jaa_2006090600", 0, BAD_DUMP SHA1(0f80597a83fc9ee99efc6fb53d1c649c1aa05727))
ROM_END

ROM_START(ddrsna)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqfdhaaa.bin", 0x00, 0x28, BAD_DUMP CRC(d7669e17) SHA1(a973529e8f174b4ec611c7db27d9208baafa7657))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqfdhaaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("fdh_aaa_2006071300", 0, BAD_DUMP SHA1(0258719111191883f688fb6aa06e5559df42e904))
ROM_END

ROM_START(ddrsnu)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqfdhuaa.bin", 0x00, 0x28, BAD_DUMP CRC(951b7774) SHA1(f8217b4936c26875e92c03d0eba6d0c664fc54dd))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqfdhuaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("fdh_uaa_2006072400", 0, BAD_DUMP SHA1(43703650c0c0ab73638bc9d7320b4d238c2d216e))
ROM_END

ROM_START(dstagesn)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqfdheaa.bin", 0x00, 0x28, BAD_DUMP CRC(b4d7ccf8) SHA1(58921beed1c05b88c38e3b6e08936583744e56b2))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqfdheaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("fdh_eaa_2006072500", 0, BAD_DUMP SHA1(81b2becf19bd78fa956c0677fee2b69bc34d0547))
ROM_END

ROM_START(dstagesna)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqfdheaa.bin", 0x00, 0x28, BAD_DUMP CRC(b4d7ccf8) SHA1(58921beed1c05b88c38e3b6e08936583744e56b2))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqfdheaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("fdh_eaa_2006032200", 0, BAD_DUMP SHA1(07a411ea67c98c9a531939b3f2c9a7195ce5e820))
ROM_END

ROM_START(ddrsn2j)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqgdjjaa.bin", 0x00, 0x28, BAD_DUMP CRC(0c2984b2) SHA1(0c3aafed4276a83591b6c007accfd4fc8cd4073c))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqfdhjaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("gdj_jaa_2007100800", 0, BAD_DUMP SHA1(b71f4a833bc9b702967dea65846b8c9d849ca530))
ROM_END

ROM_START(ddrsn2ja)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqgdjjaa.bin", 0x00, 0x28, BAD_DUMP CRC(0c2984b2) SHA1(0c3aafed4276a83591b6c007accfd4fc8cd4073c))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqfdhjaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("gdj_jaa_2007071100", 0, BAD_DUMP SHA1(ba4eadc6f501f3aa8d11ac3e408bcc9ffb7616fa))
ROM_END

ROM_START(ddrsn2a)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqgdjaaa.bin", 0x00, 0x28, BAD_DUMP CRC(6705c07b) SHA1(ad586b96bfde38296906ff3871800f3e455335fe))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqfdhaaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("gdj_aaa_2007100800", 0, BAD_DUMP SHA1(b32f4f09e155f2fd0fc16b0bb77331756b3928d0))
ROM_END

ROM_START(ddrsn2aa)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqgdjaaa.bin", 0x00, 0x28, BAD_DUMP CRC(6705c07b) SHA1(ad586b96bfde38296906ff3871800f3e455335fe))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqfdhaaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("gdj_aaa_2007071100", 0, BAD_DUMP SHA1(af76f981a86d117df0799170aa1a6babeb92fd00))
ROM_END

ROM_START(ddrsn2u)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqgdjuaa.bin", 0x00, 0x28, BAD_DUMP CRC(dc35758f) SHA1(4b9956b9f8a14931ffd6eb9dfa5d68ab379ca5e8))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqfdhuaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("gdj_uaa_2007100800", 0, BAD_DUMP SHA1(45dba10e97d58a02fcd56e5309b64a07e79b819e))
ROM_END


// DrumMania
ROM_START(drmnvj)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gee02jaa.bin", 0x00, 0x28, BAD_DUMP CRC(030bfdf4) SHA1(491e71ff44dafe6f8cad25a785c5336e3fb34ac1))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("e02_jaa_2005050201", 0, BAD_DUMP SHA1(704eb6457254ee81d2ed30136700087db070a351))
ROM_END

ROM_START(drmnv2j)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef02jaa.bin", 0x00, 0x28, BAD_DUMP CRC(5826bc2a) SHA1(eccf92dc8d94c0fcf7f03ba2505f129daefeea93))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f02_jaa_2006011201", 0, BAD_DUMP SHA1(b95ce19a6e63456b3f0a7d4c31175d51b975b1a7))
ROM_END

ROM_START(drmnv2ja)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef02jaa.bin", 0x00, 0x28, BAD_DUMP CRC(5826bc2a) SHA1(eccf92dc8d94c0fcf7f03ba2505f129daefeea93))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f02_jaa_2005112800", 0, BAD_DUMP SHA1(e57a15faedddcc8861df18cabfbd8745c1bb017b))
ROM_END

ROM_START(drmnv2jb)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef02jaa.bin", 0x00, 0x28, BAD_DUMP CRC(5826bc2a) SHA1(eccf92dc8d94c0fcf7f03ba2505f129daefeea93))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f02_jaa_2005101600", 0, BAD_DUMP SHA1(7b5adb14077d233d509a98e7e99d982cc384c28a))
ROM_END

ROM_START(drmnv3j)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef32jaa.bin", 0x00, 0x28, BAD_DUMP CRC(18d18572) SHA1(3bea94cf94d63ebcc15661fece9d6fafcfd172be))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f32_jaa_2006101800", 0, BAD_DUMP SHA1(95becb7f4bd66f15c484ab932a0f48147de9c4ca))
ROM_END

ROM_START(drmnv3ja)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef32jaa.bin", 0x00, 0x28, BAD_DUMP CRC(18d18572) SHA1(3bea94cf94d63ebcc15661fece9d6fafcfd172be))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f32_jaa_2006072600", 0, BAD_DUMP SHA1(24e60f5c3ab8a388e6e057e445504a9cecc51dc7))
ROM_END

ROM_START(drmnva)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gee02aaa.bin", 0x00, 0x28, BAD_DUMP CRC(ba14f759) SHA1(9c9d6b5c99e245bdc81f01c55ba33c495a3154cc))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("e02_aaa_2005050201", 0, BAD_DUMP SHA1(ea5872a409afed191d543e5769b51f7363dbdd24))
ROM_END

ROM_START(drmnv2a)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef02aaa.bin", 0x00, 0x28, BAD_DUMP CRC(5eb450dd) SHA1(d5a051432ab4f5adfadafa9923d53927e05bc22e))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f02_aaa_2006011201", 0, BAD_DUMP SHA1(649f874af7e28eca938e31930b12965b64962184))
ROM_END

ROM_START(drmnv2aa)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef02aaa.bin", 0x00, 0x28, BAD_DUMP CRC(5eb450dd) SHA1(d5a051432ab4f5adfadafa9923d53927e05bc22e))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f02_aaa_2005112800", 0, BAD_DUMP SHA1(8677b951baea789f8559cee8ead3d6555020e4e6))
ROM_END

ROM_START(drmnv2ab)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef02aaa.bin", 0x00, 0x28, BAD_DUMP CRC(5eb450dd) SHA1(d5a051432ab4f5adfadafa9923d53927e05bc22e))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f02_aaa_2005101600", 0, BAD_DUMP SHA1(4dc3b457b41f7052b8176d8761f006a301309195))
ROM_END

ROM_START(drmnv3a)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef32aaa.bin", 0x00, 0x28, BAD_DUMP CRC(1fce271a) SHA1(e73b36ce122b00ecbebd1663b46aae2483ac3d4c))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f32_aaa_2006101800", 0, BAD_DUMP SHA1(0e3df603c4b80a668ac92e52cc0f78a6b72a89b4))
ROM_END

ROM_START(drmnv3aa)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef32aaa.bin", 0x00, 0x28, BAD_DUMP CRC(1fce271a) SHA1(e73b36ce122b00ecbebd1663b46aae2483ac3d4c))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee02aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f32_aaa_2006072600", 0, BAD_DUMP SHA1(f46ed171cfe1ebed56bc33f1cac2714f7580e361))
ROM_END


// Guitar Freaks
ROM_START(gtrfrkvj)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gee03jaa.bin", 0x00, 0x28, BAD_DUMP CRC(017168d4) SHA1(d4df05849443a84bf9c0c698ec351ebe4bea441d))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("e03_jaa_2005050201", 0, BAD_DUMP SHA1(9fafb612b5aadd69ada459c4c6cd8e2809879016))
ROM_END

ROM_START(gtrfrkv2j)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef03jaa.bin", 0x00, 0x28, BAD_DUMP CRC(16ac39b3) SHA1(7743baaacf9779ac9fcd57ae816a24928168feb2))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f03_jaa_2006011201", 0, BAD_DUMP SHA1(ab8d6d66ef14287259d13fa12fd81925afa4f07e))
ROM_END

ROM_START(gtrfrkv2ja)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef03jaa.bin", 0x00, 0x28, BAD_DUMP CRC(16ac39b3) SHA1(7743baaacf9779ac9fcd57ae816a24928168feb2))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f03_jaa_2005112800", 0, BAD_DUMP SHA1(15150c20c7627cf86da8bc7fc606a89eb415af80))
ROM_END

ROM_START(gtrfrkv2jb)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef03jaa.bin", 0x00, 0x28, BAD_DUMP CRC(16ac39b3) SHA1(7743baaacf9779ac9fcd57ae816a24928168feb2))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f03_jaa_2005101600", 0, BAD_DUMP SHA1(63c66a56308af9ea3c830a042d91fc19cbbe92ed))
ROM_END

ROM_START(gtrfrkv3j)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef33jaa.bin", 0x00, 0x28, BAD_DUMP CRC(f7b814b5) SHA1(38019ba010a94687f2bc75aeb90e607f4d57f085))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f33_jaa_2006101800", 0, BAD_DUMP SHA1(6fb6389add1a64d6b08f20a77593852daabf854d))
ROM_END

ROM_START(gtrfrkv3ja)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef33jaa.bin", 0x00, 0x28, BAD_DUMP CRC(f7b814b5) SHA1(38019ba010a94687f2bc75aeb90e607f4d57f085))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f33_jaa_2006072600", 0, BAD_DUMP SHA1(159a7208d1b5408d62ad1707e3a0ec6c0cd90b01))
ROM_END

ROM_START(gtrfrkva)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gee03aaa.bin", 0x00, 0x28, BAD_DUMP CRC(80bf9b32) SHA1(2af2bb74f04cedeceb17f9172c1bfc93a6bea9e3))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("e03_aaa_2005050201", 0, BAD_DUMP SHA1(d5bfa18dc900bea67cd6449c61c3628a6ac4a349))
ROM_END

ROM_START(gtrfrkv2a)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef03aaa.bin", 0x00, 0x28, BAD_DUMP CRC(5ee81028) SHA1(dd82114ef943456044fcc58b072102bfcd99100d))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f03_aaa_2006011201", 0, BAD_DUMP SHA1(0af07cc5b6b81b6738817e2203ec740d326f51d7))
ROM_END

ROM_START(gtrfrkv2aa)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef03aaa.bin", 0x00, 0x28, BAD_DUMP CRC(5ee81028) SHA1(dd82114ef943456044fcc58b072102bfcd99100d))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f03_aaa_2005112800", 0, BAD_DUMP SHA1(b8d887fc27d27ce6508a394970701a5df6a9760a))
ROM_END

ROM_START(gtrfrkv2ab)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef03aaa.bin", 0x00, 0x28, BAD_DUMP CRC(5ee81028) SHA1(dd82114ef943456044fcc58b072102bfcd99100d))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f03_aaa_2005101600", 0, BAD_DUMP SHA1(cc0a0b87d571be623c469829ea4ef6a2a6f85436))
ROM_END

ROM_START(gtrfrkv3a)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef33aaa.bin", 0x00, 0x28, BAD_DUMP CRC(ea17d066) SHA1(322822a3b61e77fe5b889686127345a6f3f140d4))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f33_aaa_2006101800", 0, BAD_DUMP SHA1(268fe408ab9cbfb044a20826b4d9d46e682ea65d))
ROM_END

ROM_START(gtrfrkv3aa)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gef33aaa.bin", 0x00, 0x28, BAD_DUMP CRC(ea17d066) SHA1(322822a3b61e77fe5b889686127345a6f3f140d4))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gee03aaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f33_aaa_2006072600", 0, BAD_DUMP SHA1(b31a5cd7cc12ff4ba57cbd770ed3d8ca6c4e473f))
ROM_END


// Thrill Drive 3
ROM_START(thrild3j)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gmd44jaa.bin", 0x00, 0x28, BAD_DUMP CRC(159f9d8a) SHA1(95537df111f67478100b10e6095391846a3a71a0))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00) // Doesn't use a white dongle

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("d44_jab_20050316", 0, BAD_DUMP SHA1(5b704e1f43c51f9f85bc056c87d4682a640dca9d))
ROM_END


// Toy's March
ROM_START(toysmarch)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqe00jaa.bin", 0x00, 0x28, BAD_DUMP CRC(31eff0e1) SHA1(e863d235c98b678dc71193f81ac93131ecd99008))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqe00jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("e00_jaa_2005011602", 0, BAD_DUMP SHA1(708b7391fc4708cd32a1d98281012d8c70070419))
ROM_END

ROM_START(toysmarch2)
	KPYTHON2_BIOS

	ROM_REGION(0x400, "ps2_nvram", ROMREGION_ERASE00)
	ROM_LOAD("ps2_nvram", 0x00, 0x400, CRC(aa83752f) SHA1(b69a7ec40e00373e77ba9cd20a4b8d8e3b9dc38d))

	ROM_REGION(0x80, "ps2_hdd_id", ROMREGION_ERASE00)
	ROM_LOAD("ps2_hdd_id", 0x00, 0x80, CRC(35b17061) SHA1(48998d8ac69388d8e6f09be44c10553030411eb4))

	ROM_REGION(0x28, "ds2430_black", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_black_gqf00jaa.bin", 0x00, 0x28, BAD_DUMP CRC(161906fe) SHA1(804291ee2e46f020114a2f804b5f912555fb94c8))

	ROM_REGION(0x28, "ds2430_white", ROMREGION_ERASE00)
	ROM_LOAD("ds2430_white_gqe00jaa.bin", 0x00, 0x28, BAD_DUMP CRC(a1fe0335) SHA1(8b1e5b9fe3070db63540c7205113afa9bc7cc70f))

	DISK_REGION("ide:0:hdd:image")
	DISK_IMAGE("f00_jaa_2005110400", 0, BAD_DUMP SHA1(dbfcb12be4af67bb285e718b1efd6e829dacb35a))
ROM_END


GAME(2005, kpython2, 0, kpython2, kpython2, kpython2_state, empty_init, ROT0, "Konami", "Konami Python 2 BIOS", MACHINE_IS_SKELETON|MACHINE_IS_BIOS_ROOT)

GAME(2006, ddrsnj,   kpython2, kpython2, ddr, kpython2_state, empty_init, ROT0, "Konami", "Dance Dance Revolution SuperNOVA (FDH:J:A:A:2006090600)", MACHINE_IS_SKELETON)
GAME(2006, ddrsna,   kpython2, kpython2, ddr, kpython2_state, empty_init, ROT0, "Konami", "Dance Dance Revolution SuperNOVA (FDH:A:A:A:2006071300)", MACHINE_IS_SKELETON)
GAME(2006, ddrsnu,   kpython2, kpython2, ddr, kpython2_state, empty_init, ROT0, "Konami", "Dance Dance Revolution SuperNOVA (FDH:U:A:A:2006072400)", MACHINE_IS_SKELETON)
GAME(2007, ddrsn2j,  kpython2, kpython2, ddr, kpython2_state, empty_init, ROT0, "Konami", "Dance Dance Revolution SuperNOVA 2 (GDJ:J:A:A:2007100800)", MACHINE_IS_SKELETON)
GAME(2007, ddrsn2ja, ddrsn2j,  kpython2, ddr, kpython2_state, empty_init, ROT0, "Konami", "Dance Dance Revolution SuperNOVA 2 (GDJ:J:A:A:2007071100)", MACHINE_IS_SKELETON)
GAME(2007, ddrsn2a,  kpython2, kpython2, ddr, kpython2_state, empty_init, ROT0, "Konami", "Dance Dance Revolution SuperNOVA 2 (GDJ:A:A:A:2007100800)", MACHINE_IS_SKELETON)
GAME(2007, ddrsn2aa, ddrsn2a,  kpython2, ddr, kpython2_state, empty_init, ROT0, "Konami", "Dance Dance Revolution SuperNOVA 2 (GDJ:A:A:A:2007071100)", MACHINE_IS_SKELETON)
GAME(2007, ddrsn2u,  kpython2, kpython2, ddr, kpython2_state, empty_init, ROT0, "Konami", "Dance Dance Revolution SuperNOVA 2 (GDJ:U:A:A:2007100800)", MACHINE_IS_SKELETON)

GAME(2006, dstagesn,  kpython2, kpython2, ddr, kpython2_state, empty_init, ROT0, "Konami", "Dancing Stage SuperNOVA (FDH:E:A:A:2006072500)", MACHINE_IS_SKELETON)
GAME(2006, dstagesna, dstagesn, kpython2, ddr, kpython2_state, empty_init, ROT0, "Konami", "Dancing Stage SuperNOVA (FDH:E:A:A:2006032200)", MACHINE_IS_SKELETON)

GAME(2005, drmnvj,   kpython2, kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V (E02:J:A:A:2005050200)", MACHINE_IS_SKELETON)
GAME(2006, drmnv2j,  kpython2, kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V2 (F02:J:A:A:2006011201)", MACHINE_IS_SKELETON)
GAME(2005, drmnv2ja, drmnv2j,  kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V2 (F02:J:A:A:2005112800)", MACHINE_IS_SKELETON)
GAME(2005, drmnv2jb, drmnv2j,  kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V2 (F02:J:A:A:2005101600)", MACHINE_IS_SKELETON)
GAME(2006, drmnv3j,  kpython2, kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V3 (F32:J:A:A:2006101800)", MACHINE_IS_SKELETON)
GAME(2006, drmnv3ja, drmnv3j,  kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V3 (F32:J:A:A:2006072600)", MACHINE_IS_SKELETON)
GAME(2005, drmnva,   kpython2, kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V (E02:A:A:A:2005050200)", MACHINE_IS_SKELETON)
GAME(2006, drmnv2a,  kpython2, kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V2 (F02:A:A:A:2006011201)", MACHINE_IS_SKELETON)
GAME(2005, drmnv2aa, drmnv2a,  kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V2 (F02:A:A:A:2005112800)", MACHINE_IS_SKELETON)
GAME(2005, drmnv2ab, drmnv2a,  kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V2 (F02:A:A:A:2005101600)", MACHINE_IS_SKELETON)
GAME(2006, drmnv3a,  kpython2, kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V3 (F32:A:A:A:2006101800)", MACHINE_IS_SKELETON)
GAME(2006, drmnv3aa, drmnv3a,  kpython2, drmn, kpython2_state, empty_init, ROT0, "Konami", "DrumMania V3 (F32:A:A:A:2006072600)", MACHINE_IS_SKELETON)

GAME(2005, gtrfrkvj,   kpython2,  kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V (E03:J:A:A:2005050200)", MACHINE_IS_SKELETON)
GAME(2006, gtrfrkv2j,  kpython2,  kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V2 (F03:J:A:A:2006011201)", MACHINE_IS_SKELETON)
GAME(2005, gtrfrkv2ja, gtrfrkv2j, kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V2 (F03:J:A:A:2005112800)", MACHINE_IS_SKELETON)
GAME(2005, gtrfrkv2jb, gtrfrkv2j, kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V2 (F03:J:A:A:2005101600)", MACHINE_IS_SKELETON)
GAME(2006, gtrfrkv3j,  kpython2,  kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V3 (F33:J:A:A:2006101800)", MACHINE_IS_SKELETON)
GAME(2006, gtrfrkv3ja, gtrfrkv3j, kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V3 (F33:J:A:A:2006072600)", MACHINE_IS_SKELETON)
GAME(2005, gtrfrkva,   kpython2,  kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V (E03:A:A:A:2005050200)", MACHINE_IS_SKELETON)
GAME(2006, gtrfrkv2a,  kpython2,  kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V2 (F03:A:A:A:2006011201)", MACHINE_IS_SKELETON)
GAME(2005, gtrfrkv2aa, gtrfrkv2a, kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V2 (F03:A:A:A:2005112800)", MACHINE_IS_SKELETON)
GAME(2005, gtrfrkv2ab, gtrfrkv2a, kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V2 (F03:A:A:A:2005101600)", MACHINE_IS_SKELETON)
GAME(2006, gtrfrkv3a,  kpython2,  kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V3 (F33:A:A:A:2006101800)", MACHINE_IS_SKELETON)
GAME(2006, gtrfrkv3aa, gtrfrkv3a, kpython2, gtrfrks, kpython2_state, empty_init, ROT0, "Konami", "Guitar Freaks V3 (F33:A:A:A:2006072600)", MACHINE_IS_SKELETON)

GAME(2005, thrild3j, kpython2, kpython2, thrild3, kpython2_state, empty_init, ROT0, "Konami", "Thrill Drive 3 (D44:J:A:A:20050316)", MACHINE_IS_SKELETON)

GAME(2005, toysmarch,  kpython2, kpython2, toysmarch, kpython2_state, empty_init, ROT0, "Konami", "Toy's March (E00:J:A:A:2005011602)", MACHINE_IS_SKELETON)
GAME(2005, toysmarch2, kpython2, kpython2, toysmarch, kpython2_state, empty_init, ROT0, "Konami", "Toy's March 2 (F00:J:A:A:2005110400)", MACHINE_IS_SKELETON)
