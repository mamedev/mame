// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Hilger Analytical AB6089 Mk. 1 (LSI Octopus)

2013-07-26 Skeleton driver.

http://computers.mcbx.netne.net/8bit/hilger/index.htm

Below is an extract from the page:

The computer has 2 CPUs: Z80 and 8088. Most circuits are dated 1985-1986, display circuitry is made by Signetics.
Mainboard was manufactured by LSI Computers Ltd. under part numbers: 15000SS100 and 15000P4100. All steel parts
of casing are grounded by wires. It's graphics card works in pass-through mode: It takes picture from mainboard's
TTL output and adds image to it, then it puts it to monitor. Its ROM is prepared for hard disk and some type of
network, yet no HDD controller nor network interfaces are present inside - it seems that they were added as
expansion cards.

UPDATE: It's re-branded LSI Octopus computer, a very well-expandable machine which was designed to "grow with a
company". First stage was a computer which could be used even with TV set. As requirements increased, Octopus
could be equipped with hard disk controller, network adapter, multi-terminal serial port card to act as a terminal
server or even CPU cards to run concurrent systems. There were even tape backup devices for it. Octopus could run
CP/M, MP/M (concurrent - multitasking-like OS, even with terminals), or even MS-DOS - CP/M or MP/M could be used
with Z80 or 8080. There was also LSI ELSIE system, a concurrent DOS. Last British LSI machines were 386 computers
which could be used as servers for Octopus computers.

Manufacturer    Hilger Analytical / LSI Computers Ltd.

Origin  UK
Year of unit    1986?
Year of introduction    1985
End of production   ?
CPU     Z80, 8088
Speed   ??
RAM     256kB
ROM     16kB (Basic)
Colors:     ??
Sound:  Speaker. Beeps :)
OS:     CP/M 80 or 86
MP/M 80 o 86
Concurrent CP/M
LSI ELSIE
MS-DOS
Display modes:  Text: ??
Graphics: ??

Media:  Two internal 5.25" floppy disk drives, DS DD, 96tpi.
Probably hard disk

Power supply:
Built-in switching power supply.

I/O:    Serial port
2 parallel ports

Video TTL Output
Composite video output

Possible upgrades:  Many

Software accessibility:
Dedicated: Impossible.
CP/M - Good
DOS - Good.

It won't take XT nor AT keyboard, but pinout is quite similar. UPDATE: I saw a few photos of keyboard.
It's another Z80 computer! It has an EPROM, simple memory and CPU.

After powering on, it should perform POST writing:

TESTING...
    Main Processor
    PROM
    DMA Controllers
    RAM
    Interrupts
    Floppy Discs
    Hard Disc Controller   (optionally - if installed)

Waiting for hard Disc... (Optionally - if installed)

Firmware versions:

SYSTEM         18B (or other)
GRAPHICS      4    (if graphic card installed)

And probably it should boot or display:

Insert System Disk.

Or maybe:

Nowhere to boot from.

Load options:
    Floppy
    Pro Network
    Winchester
Enter selection:

This information was gained by studying boot ROM of the machine.

It's a very rare computer. It has 2 processors, Z80 and 8088, so it seems that it may run CP/M and DOS.
Its BIOS performs POST and halts as there's no keyboard.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"


class octopus_state : public driver_device
{
public:
	octopus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_WRITE8_MEMBER(port_c9);
	UINT32 screen_update_octopus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START( octopus_mem, AS_PROGRAM, 8, octopus_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	AM_RANGE(0xe4000, 0xe4fff) AM_RAM // character definitions
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( octopus_io, AS_IO, 8, octopus_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xc5, 0xc9) AM_WRITE(port_c9)
ADDRESS_MAP_END

WRITE8_MEMBER( octopus_state::port_c9 )
{
	if (offset == 0)
		printf("\n");
	if (offset == 4)
		printf("%c",data);
}


/* Input ports */
static INPUT_PORTS_START( octopus )
INPUT_PORTS_END


void octopus_state::machine_reset()
{
}

void octopus_state::video_start()
{
}

UINT32 octopus_state::screen_update_octopus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static MACHINE_CONFIG_START( octopus, octopus_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8088, XTAL_9_8304MHz) // absolutely no idea what frequency
	MCFG_CPU_PROGRAM_MAP(octopus_mem)
	MCFG_CPU_IO_MAP(octopus_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(octopus_state, screen_update_octopus)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( octopus )
	ROM_REGION( 0x4000, "user1", 0 )
	ROM_LOAD( "octopus_main_prom", 0x0000, 0x4000, CRC(b5b4518d) SHA1(41b8729c4c9074914fd4ea181c8b6d4805ee2b93) )

	// This rom was on the graphics card (yes, it has slots)
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "octopus_gfx_card",  0x0000, 0x2000, CRC(b2386534) SHA1(5e3c4682afb4eb222e48a7203269a16d26911836) )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY             FULLNAME       FLAGS */
COMP( 1986, octopus,  0,      0,       octopus,   octopus, driver_device, 0,  "Digital Microsystems", "LSI Octopus", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
