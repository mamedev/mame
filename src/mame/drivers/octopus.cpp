// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, Robbbert
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
#include "cpu/z80/z80.h"
#include "video/scn2674.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/mc146818.h"
#include "machine/i8255.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"

class octopus_state : public driver_device
{
public:
	octopus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_crtc(*this, "crtc"),
		m_vram(*this, "vram"),
		m_fontram(*this, "fram"),
		m_dma1(*this, "dma1"),
		m_dma2(*this, "dma2"),
		m_pic1(*this, "pic_master"),
		m_pic2(*this, "pic_slave"),
		m_rtc(*this, "rtc"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:floppy0"),
		m_floppy1(*this, "fdc:floppy1")
		{ }

	virtual void machine_reset() override;
	virtual void video_start() override;
	SCN2674_DRAW_CHARACTER_MEMBER(display_pixels);
	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(get_slave_ack);
private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<scn2674_device> m_crtc;
	required_shared_ptr<UINT8> m_vram;
	required_shared_ptr<UINT8> m_fontram;
	required_device<am9517a_device> m_dma1;
	required_device<am9517a_device> m_dma2;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<mc146818_device> m_rtc;
	required_device<fd1793_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};


static ADDRESS_MAP_START( octopus_mem, AS_PROGRAM, 8, octopus_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	// second 128kB for 256kB system
	// expansion RAM, up to 512kB extra
	AM_RANGE(0xd0000, 0xdffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xe4000, 0xe5fff) AM_RAM AM_SHARE("fram")
	AM_RANGE(0xf4000, 0xf5fff) AM_ROM AM_REGION("chargen",0)
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( octopus_io, AS_IO, 8, octopus_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x0f) AM_DEVREADWRITE("dma1", am9517a_device, read, write)
	AM_RANGE(0x10, 0x1f) AM_DEVREADWRITE("dma2", am9517a_device, read, write)
	// 0x20: System type switch (read), Z80 NMI (write)
	// 0x21: Parity fail reset (write), bit5 SLCTOUT from parallel, bit6 option board parity fail bit7 main board parity fail (read)
	// 0x28: Z80 enable (write)
	// 0x31: hard disk bank
	// 0x32: floppy bank
	// 0x33: RAM refresh / Z80 bank
	// 0x50-51: Keyboard (i8251)
	// 0x70-73: HD controller
	// 0x80-83: serial timers (i8253)
	// 0xa0-a3: serial interface (Z80 SIO/2)
	AM_RANGE(0xb0, 0xb1) AM_DEVREADWRITE("pic_master", pic8259_device, read, write)
	AM_RANGE(0xb4, 0xb5) AM_DEVREADWRITE("pic_slave", pic8259_device, read, write)
	AM_RANGE(0xc0, 0xc7) AM_DEVREADWRITE("crtc", scn2674_device, read, write)
	// 0xc8: video control
	AM_RANGE(0xc9, 0xc9) AM_DEVREADWRITE("crtc", scn2674_device, buffer_r, buffer_w)
	AM_RANGE(0xca, 0xca) AM_RAM // attribute writes go here
	// 0xcf: mode control
	AM_RANGE(0xd0, 0xdf) AM_DEVREADWRITE("fdc", fd1793_t, read, write)
	// 0xe0: Z80 interrupt vector for RS232
	// 0xe4: Z80 interrupt vector for RS422
	// 0xf0-f1: Parallel interface data I/O (Centronics), and control/status
	AM_RANGE(0xf8, 0xff) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END


static ADDRESS_MAP_START( octopus_sub_mem, AS_PROGRAM, 8, octopus_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START( octopus_sub_io, AS_IO, 8, octopus_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START( octopus_vram, AS_0, 8, octopus_state )
	AM_RANGE(0x0000,0xffff) AM_READWRITE(vram_r, vram_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( octopus )
INPUT_PORTS_END


WRITE8_MEMBER(octopus_state::vram_w)
{
	m_vram[offset] = data;
}

READ8_MEMBER(octopus_state::vram_r)
{
	return m_vram[offset];
}


void octopus_state::machine_reset()
{
	m_subcpu->set_input_line(INPUT_LINE_HALT,ASSERT_LINE);  // halt Z80 to start with
}

void octopus_state::video_start()
{
	m_vram.allocate(0x10000);
}

SCN2674_DRAW_CHARACTER_MEMBER(octopus_state::display_pixels)
{
	if(!lg)
	{
		UINT8 tile = m_vram[address & 0x1fff];
		UINT8 data = m_fontram[(tile * 16) + linecount];
		for (int z=0;z<8;z++)
			bitmap.pix32(y,x + z) = BIT(data,z) ? rgb_t::white : rgb_t::black;
	}
}

READ8_MEMBER( octopus_state::get_slave_ack )
{
	logerror("slave_ack: %i\n",offset);
	if (offset==7)
		return m_pic2->acknowledge();

	return 0x00;
}

static SLOT_INTERFACE_START( octopus_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( octopus, octopus_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8088, XTAL_24MHz / 3)  // 8MHz
	MCFG_CPU_PROGRAM_MAP(octopus_mem)
	MCFG_CPU_IO_MAP(octopus_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic_master", pic8259_device, inta_cb)

	MCFG_CPU_ADD("subcpu",Z80, XTAL_24MHz / 4) // 6MHz
	MCFG_CPU_PROGRAM_MAP(octopus_sub_mem)
	MCFG_CPU_IO_MAP(octopus_sub_io)

	MCFG_DEVICE_ADD("dma1", AM9517A, XTAL_24MHz / 6)  // 4MHz

	MCFG_DEVICE_ADD("dma2", AM9517A, XTAL_24MHz / 6)  // 4MHz

	MCFG_PIC8259_ADD("pic_master", INPUTLINE("maincpu",0), VCC, READ8(octopus_state,get_slave_ack))
	MCFG_PIC8259_ADD("pic_slave", DEVWRITELINE("pic_master",pic8259_device, ir7_w), GND, NOOP)

	// RTC (MC146818 via i8255 PPI)  TODO: hook up RTC to PPI
	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(NOOP)
	MCFG_I8255_IN_PORTB_CB(NOOP)
	MCFG_I8255_IN_PORTC_CB(NOOP)
	MCFG_I8255_OUT_PORTA_CB(NOOP)
	MCFG_I8255_OUT_PORTB_CB(NOOP)
	MCFG_I8255_OUT_PORTC_CB(NOOP)
	MCFG_MC146818_ADD("rtc", XTAL_32_768kHz)
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE("pic_slave",pic8259_device, ir4_w))
	
	MCFG_FD1793_ADD("fdc",XTAL_16MHz / 8)
	MCFG_FLOPPY_DRIVE_ADD("fdc:floppy0", octopus_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:floppy1", octopus_floppies, "525dd", floppy_image_device::default_floppy_formats)

	// TODO: add components
	// i8253 PIT timer (speaker output, serial timing, other stuff too?)
	// i8251 serial controller (keyboard)
	// i8255A PPI (RTC access, FDC)
	// Centronics parallel interface
	// Z80SIO/2 (serial)
	// Winchester HD controller (Xebec/SASI compatible? uses TTL logic)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(720, 360)
	MCFG_SCREEN_VISIBLE_AREA(0, 720-1, 0, 360-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc",scn2674_device, screen_update)
//	MCFG_SCREEN_PALETTE("palette")
//	MCFG_PALETTE_ADD_MONOCHROME("palette")	

	MCFG_SCN2674_VIDEO_ADD("crtc", 0, DEVWRITELINE("pic_slave",pic8259_device,ir0_w))  // character clock can be selectable, either 16MHz or 17.6MHz
	MCFG_SCN2674_TEXT_CHARACTER_WIDTH(8)
	MCFG_SCN2674_GFX_CHARACTER_WIDTH(8)
	MCFG_SCN2674_DRAW_CHARACTER_CALLBACK_OWNER(octopus_state, display_pixels)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, octopus_vram)

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
