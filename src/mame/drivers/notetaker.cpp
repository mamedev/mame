// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/* Xerox Notetaker, 1978
 * Driver by Jonathan Gevaryahu
 *
 * <Note that below is a really lousy write-up, this system deserves much better. use the Ingalls-2014-Smalltalk78.pdf as a better source for all this stuff!>
 * Designed by Alan Kay and Many others, with BIOS written by Bruce Horn
 * History of the machine can be found at http://freudenbergs.de/bert/publications/Ingalls-2014-Smalltalk78.pdf
 * prototypes only, around 10 units manufactured 1978-1980 (one at CHM, not sure where the others are)
 * This device was the origin of Smalltalk-78 (which acted as the operating system of the Notetaker)
 * The Notetaker also introduced the BitBlt graphical operation, which was used to do most graphical functions in Smalltalk-78
 * As far as I am aware, no media (world disks/boot disks) for the Notetaker have survived, only an incomplete ram dump of the smalltalk-76
 * 'world' which was used to bootstrap smalltalk-78 originally
 *
 * see http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker for additional information
 * http://bitsavers.trailing-edge.com/pdf/xerox/notetaker/memos/19790620_Z-IOP_1.5_ls.pdf is listing of the biop v1.5 code
 *
 * MISSING DUMP for 8741? I/O MCU which does mouse-related stuff
 
TODO: everything below.
* Get the running machine smalltalk-78 memory dump loaded as a rom and forced into ram on startup, since no boot disks have survived
* floppy controller wd1791
  According to http://bitsavers.trailing-edge.com/pdf/xerox/notetaker/memos/19790620_Z-IOP_1.5_ls.pdf the format is 128 bytes per sector, 16 sectors per track (one sided)
  According to the schematics, we're missing an 82s147 DISKSEP.PROM used as a data separator
* crt5027 video controller
* i8251? serial/EIA controller
* Harris 6402 keyboard UART
* HLE for the missing MCU which reads the mouse quadratures and buttons

WIP:
* pic8259 interrupt controller - this is attached as a device, but the interrupts are not hooked to it yet.

*/

#include "cpu/i86/i86.h"
#include "machine/pic8259.h"
//#include "video/tms9927.h"

class notetaker_state : public driver_device
{
public:
	notetaker_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_pic(*this, "pic8259")//,
		//m_vtac(*this, "crt5027")
	{
	}
// devices
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	//required_device<crt5027_device> m_vtac;

//declarations
	DECLARE_WRITE16_MEMBER(IPConReg_w);
	DECLARE_READ16_MEMBER(maincpu_r);
	DECLARE_WRITE16_MEMBER(maincpu_w);
	DECLARE_DRIVER_INIT(notetakr);

//variables
	UINT8 m_BootSeqDone;
	UINT8 m_DisableROM;
	
// overrides
	virtual void machine_reset() override;
};

WRITE16_MEMBER(notetaker_state::IPConReg_w)
{
	m_BootSeqDone = (data&0x80)?1:0;
	//m_ProcLock = (data&0x40)?1:0; // processor lock
	//m_CharCtr = (data&0x20)?1:0; // battery charge control
	m_DisableROM = (data&0x10)?1:0; // disable rom at 0000-0fff
	//m_CorrOn = (data&0x08)?1:0; // also LedInd5
	//m_LedInd6 = (data&0x04)?1:0;
	//m_LedInd7 = (data&0x02)?1:0;
	//m_LedInd8 = (data&0x01)?1:0;
	popmessage("LEDS: CR1: %d, CR2: %d, CR3: %d, CR4: %d", (data&0x04)>>2, (data&0x08)>>3, (data&0x02)>>1, (data&0x01)); // cr1 and 2 are in the reverse order as expected, according to the schematic
}

READ16_MEMBER(notetaker_state::maincpu_r)
{
	UINT16 *rom = (UINT16 *)(memregion("maincpu")->base());
	rom += 0x7f800;
	UINT16 *ram = (UINT16 *)(memregion("ram")->base());
	if ( (m_BootSeqDone == 0) || ((m_DisableROM == 0) && ((offset&0x7F800) == 0)) )
	{
		rom += (offset&0x7FF);
		return *rom;
	}
	else
	{
		ram += (offset);
		return *ram;
	}
}

WRITE16_MEMBER(notetaker_state::maincpu_w)
{
	UINT16 *ram = (UINT16 *)(memregion("ram")->base());
	ram += offset;
	*ram = data;
}

/* Address map comes from http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker/schematics/19790423_Notetaker_IO_Processor.pdf
a19 a18 a17 a16  a15 a14 a13 a12  a11 a10 a9  a8   a7  a6  a5  a4   a3  a2  a1  a0   BootSeqDone  DisableROM
x   x   x   x    x   x   x   x    *   *   *   *    *   *   *   *    *   *   *   *    0            x          R   ROM
0   0   0   0    0   0   0   0    *   *   *   *    *   *   *   *    *   *   *   *    1            0          R   ROM
<  anything not all zeroes   >    *   *   *   *    *   *   *   *    *   *   *   *    1            0          RW  RAM
x   x   x   x    ?   ?   *   *    *   *   *   *    *   *   *   *    *   *   *   *    x            x          W   RAM
x   x   x   x    ?   ?   *   *    *   *   *   *    *   *   *   *    *   *   *   *    1            1          RW  RAM

More or less:
BootSeqDone is 0, DisableROM is ignored, mem map is 0x00000-0xfffff reading is the 0x1000-long ROM, repeated every 0x1000 bytes. writing goes to RAM.
BootSeqDone is 1, DisableROM is 0,       mem map is 0x00000-0x00fff reading is the 0x1000-long ROM, remainder of memory map goes to RAM or open bus. writing goes to RAM.
BootSeqDone is 1, DisableROM is 1,       mem map is entirely RAM or open bus for both reading and writing.
*/
static ADDRESS_MAP_START(notetaker_mem, AS_PROGRAM, 16, notetaker_state)
	/*AM_RANGE(0x00000, 0x00fff) AM_ROM AM_REGION("maincpu", 0xFF000) // I think this copy of rom is actually banked via io reg 0x20, there is RAM which lives behind here?
	AM_RANGE(0x01000, 0x3ffff) AM_RAM // ram lives here, 256KB
	AM_RANGE(0xff000, 0xfffff) AM_ROM // is this banked too? Don't think so...*/
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(maincpu_r, maincpu_w) // bypass MAME's memory map system as we need finer grained control
ADDRESS_MAP_END

/* io memory map comes from http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker/memos/19790605_Definition_of_8086_Ports.pdf
   and from the schematic at http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker/schematics/19790423_Notetaker_IO_Processor.pdf
a19 a18 a17 a16  a15 a14 a13 a12  a11 a10 a9  a8   a7  a6  a5  a4   a3  a2  a1  a0
?   ?   ?   ?    0   x   x   x    x   x   x   0    0   0   0   x    x   x   *   .       RW  IntCon (PIC8259)
?   ?   ?   ?    0   x   x   x    x   x   x   0    0   0   1   x    x   x   x   .       W   IPConReg
?   ?   ?   ?    0   x   x   x    x   x   x   0    0   1   0   x    x   x   x   .       W   KbdInt
?   ?   ?   ?    0   x   x   x    x   x   x   0    0   1   1   x    x   x   x   .       W   FIFOReg
?   ?   ?   ?    0   x   x   x    x   x   x   0    1   0   0   x    x   x   x   .       .   Open Bus
?   ?   ?   ?    0   x   x   x    x   x   x   0    1   0   1   x    x   x   x   .       .   Open Bus
?   ?   ?   ?    0   x   x   x    x   x   x   0    1   1   0   x    x   x   x   .       W   FIFOBus
?   ?   ?   ?    0   x   x   x    x   x   x   0    1   1   1   x    x   x   x   .       .   Open Bus
0   0   0   0    0   0   0   0    *   *   *   *    *   *   *   *    *   *   *   .       R   ROM, but ONLY if BootSegDone is TRUE, and /DisableROM is FALSE

*/
static ADDRESS_MAP_START(notetaker_io, AS_IO, 16, notetaker_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x7E1C) AM_DEVREADWRITE8("pic8259", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x20, 0x21) AM_MIRROR(0x7E1E) AM_WRITE(IPConReg_w) // processor (rom mapping, etc) control register
	//AM_RANGE(0x42, 0x43) AM_READ read keyboard data (high byte only) [from mcu?]
	//AM_RANGE(0x44, 0x45) AM_READ read keyboard fifo state (high byte only) [from mcu?]
	//AM_RANGE(0x48, 0x49) AM_WRITE kbd->uart control register [to mcu?]
	//AM_RANGE(0x4a, 0x4b) AM_WRITE kbd->uart data register [to mcu?]
	//AM_RANGE(0x4c, 0x4d) AM_WRITE kbd data reset [to mcu?]
	//AM_RANGE(0x4e, 0x4f) AM_WRITE kbd chip [mcu?] reset [to mcu?]
	//AM_RANGE(0x60, 0x61) AM_WRITE DAC sample and hold and frequency setup
	//AM_RANGE(0x100, 0x101) AM_WRITE I/O register (adc speed, crtc pixel clock enable, etc)
	//AM_RANGE(0x140, 0x15f) AM_DEVREADWRITE("crt5027", crt5027_device, read, write)
ADDRESS_MAP_END

/* writes during boot:
0x88 to port 0x020 (PCR; boot sequence done(1), processor not locked(0), battery charger off(0), rom not disabled(0) correction off&cr4 off(1), cr3 on(0), cr2 on(0), cr1 on (0);)
0x02 to port 0x100 (IOR write: enable 5v only relay control for powering up 4116 dram enabled)
0x03 to port 0x100 (IOR write: in addition to above, enable 12v relay control for powering up 4116 dram enabled)
<dram memory 0x00000-0x3ffff is zeroed here>
0x13 to port 0x000 PIC (?????)
0x08 to port 0x002 PIC (UART int enabled)
0x0D to port 0x002 PIC (UART, wd1791, and parity error int enabled)
0xff to port 0x002 PIC (all ints enabled)
0x0000 to port 0x04e (reset keyboard fifo/controller)
0x0000 to port 0x1ae (reset UART)
0x0016 to port 0x048 (kbd control reg write)
0x0005 to port 0x1a8 (UART control reg write)
0x5f to port 0x140 \
0xf2 to port 0x142  \
0x7d to port 0x144   \
0x1d to port 0x146    \_ set up CRTC
0x04 to port 0x148    /
0x10 to port 0x14a   /
0x00 to port 0x154  /
0x1e to port 0x15a /
0x0a03 to port 0x100 (IOR write: set bit clock to 12Mhz)
0x2a03 to port 0x100 (IOR write: enable crtc clock chain)
0x00 to port 0x15c (fire off crtc timing chain)
read from 0x0002 (byte wide) (check interrupts) <looking for vblank int or odd/even frame int here, most likely>
0xaf to port 0x002 PIC (mask out kb int and 30hz display int)
0x0400 to 0x060 (select DAC fifo frequency 2)
read from 0x44 (byte wide) in a loop forever (read keyboard fifo status)
*/

/* Machine Reset */
void notetaker_state::machine_reset()
{
	m_BootSeqDone = 0;
	m_DisableROM = 0;
}

/* Input ports */
static INPUT_PORTS_START( notetakr )
INPUT_PORTS_END

static MACHINE_CONFIG_START( notetakr, notetaker_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, XTAL_24MHz/3) /* iD8086-2 @ E4A; 24Mhz crystal divided down to 8Mhz by i8284 clock generator */
	MCFG_CPU_PROGRAM_MAP(notetaker_mem)
	MCFG_CPU_IO_MAP(notetaker_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)
	MCFG_PIC8259_ADD("pic8259", INPUTLINE("maincpu", 0), VCC, NULL) // iP8259A-2 @ E6

	//Note there is a second i8086 cpu on the 'emulator board', which is probably loaded with code once smalltalk-78 loads

	/* video hardware */
	/*MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(250))
	MCFG_SCREEN_UPDATE_DRIVER(notetaker_state, screen_update)
	MCFG_SCREEN_SIZE(64*6, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*6-1, 0, 32*8-1)

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	MCFG_DEVICE_ADD("crt5027", CRT5027, XTAL_17_9712MHz/2)
	//MCFG_TMS9927_CHAR_WIDTH(6)
	//MCFG_TMS9927_VSYN_CALLBACK(DEVWRITELINE(TMS5501_TAG, tms5501_device, sens_w))
	MCFG_VIDEO_SET_SCREEN("screen")*/
	/* Devices */

MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(notetaker_state,notetakr)
{
	// descramble the rom; the whole thing is a gigantic scrambled mess either to ease
	// interfacing with older xerox technologies which used A0 and D0 as the MSB bits
	// or maybe because someone screwed up somewhere along the line. we may never know.
	// see http://bitsavers.informatik.uni-stuttgart.de/pdf/xerox/notetaker/schematics/19790423_Notetaker_IO_Processor.pdf pages 12 and onward
	UINT16 *romsrc = (UINT16 *)(memregion("maincpuload")->base());
	UINT16 *romdst = (UINT16 *)(memregion("maincpu")->base());
	UINT16 *temppointer;
	UINT16 wordtemp;
	UINT16 addrtemp;
		// leave the src pointer alone, since we've only used a 0x1000 long address space
		romdst += 0x7f800; // set the dest pointer to 0xff000 (>>1 because 16 bits data)
		for (int i = 0; i < 0x800; i++)
		{
			wordtemp = BITSWAP16(*romsrc, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15); // data bus is completely reversed
			addrtemp = BITSWAP16(i, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10); // address bus is completely reversed; 11-15 should always be zero
			temppointer = romdst+(addrtemp&0x7FF);
			*temppointer = wordtemp;
			romsrc++;
		}
}

/* ROM definition */
ROM_START( notetakr )
	ROM_REGION( 0x1000, "maincpuload", ROMREGION_ERASEFF ) // load roms here before descrambling
	ROM_SYSTEM_BIOS( 0, "v2.00", "IO Monitor v2.00" ) // dumped from Notetaker
	ROMX_LOAD( "biop__2.00_hi.b2716.h1", 0x0000, 0x0800, CRC(1119691d) SHA1(4c20b595b554e6f5489ab2c3fb364b4a052f05e3), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "biop__2.00_lo.b2716.g1", 0x0001, 0x0800, CRC(b72aa4c7) SHA1(85dab2399f906c7695dc92e7c18f32e2303c5892), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v1.50", "IO Monitor v1.50" ) // typed from the source listing at http://bitsavers.trailing-edge.com/pdf/xerox/notetaker/memos/19790620_Z-IOP_1.5_ls.pdf and scrambled
	ROMX_LOAD( "z-iop_1.50_hi.h1", 0x0000, 0x0800, CRC(2994656e) SHA1(ca2bb38eb9075c5c2f3cc5439b209e7e216084da), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "z-iop_1.50_lo.g1", 0x0001, 0x0800, CRC(2cb79a67) SHA1(692aafd2aeea27533f6288dbb1cb8678ea08fade), ROM_SKIP(1) | ROM_BIOS(2))
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF ) // area for descrambled roms
	ROM_REGION( 0x100000, "ram", ROMREGION_ERASEFF ) // ram cards
	ROM_REGION( 0x1000, "proms", ROMREGION_ERASEFF )
	ROM_LOAD( "disksep.prom.82s147.a4", 0x000, 0x200, NO_DUMP ) // disk data separator prom from the disk/display module board
	ROM_LOAD( "setmemrq.prom.82s126.d9", 0x200, 0x100, NO_DUMP ) // SETMEMRQ memory timing prom from the disk/display module board
ROM_END

/* Driver */

/*    YEAR      NAME  PARENT  COMPAT   MACHINE     INPUT            STATE      INIT  COMPANY     FULLNAME                FLAGS */
COMP( 1978, notetakr,      0,      0, notetakr, notetakr, notetaker_state, notetakr, "Xerox", "Notetaker", MACHINE_IS_SKELETON)
//COMP( 1978, notetakr,      0,      0, notetakr, notetakr, driver_device, notetakr, "Xerox", "Notetaker", MACHINE_IS_SKELETON)
