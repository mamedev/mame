/***************************************************************************

    ITT 3030

	
	ToDo:
	- Check Beeper
	- hook up keyboard
	- According to the manual, the keyboard is based on a 8278 ... it's nowhere to be found. The keyboard / video card has a 8741 instead of which a ROM dump exists
	- memory map
	- serial port
	- daisy chain
	- ...
	

	CPU Board, all ICs shown:
	
	|-----------------------------------------------------------------|
	|                                                                 |
	|    74LS640N  	       Z80_Combo            74LS138N              |
	|    			                                        74LS00N   |
	|    74LS240N                               74LS74AN              |
	|                                                       74LS00N   |
	|C   74LS240N          Z80_CPU              74LS240N             C|
	|N                                                      74LS74AN N|
	|1   74LS241N                               74LS240N             2|
	|                      ROM_1      74LS20N               74LS38N   |
    |    74LS240N                               74LS240N	          |
	|                                 74LS04N               74LS02N   |
	|    74LS138N                               74LS74AN              |
	|                                                       74LS175N  |
	|    75154N        74LS156N                 74LS00N               |
	|                                                       74LS123N  |
	|	 75150P 75150P 74LS175N   X1  74LS00N   74LS132N              |
	|-----------------------------------------------------------------|		
	
	Z80_Combo:	Mostek MK3886 Z80 Combo Chip, Serial, Timer, 256 bytes RAM, Interrupt Controller
	Z80_CPU:	Zilog Z80A CPU
	ROM_1:		NEC D2716D marked "BOOTV1.2"
	X1:			Crystal 4,194 MHz
	CN1:		Bus Connector
	CN2:		Memory Board Connector
	
----------------------------------------------------------------------------------
	
	Video / Keyboard Combination board, all ICs shown:
	
	|-----------------------------------------------------------------|
	|                                                                 |
	|         X1     74276N      MCU_1                  74LS85N       |
	|                                                                 |
	|    74LS138N    74LS240                            74LS240N      |
	|                            75LS257AN  74LS166AN                 |
	|    74LS08N     74LS85N                            74LS241N      |
	|                            75LS257AN  ROM_1                    C|
	|    74LS132N    74LS32N                            74LS240N     N|
	|                            75LS257AN                           1|
	|    74LS10N     74LS08N                            74LS240N      |
	|                            75LS257AN  RAM_1                     |
	|    74LS163AN   74LS173AN                          74LS374N      |
	|                                                                 |
	|    74LS86N     74LS240N                           74LS640N      |
	|                            Video_1                              |
	|    74LS74AN    74LS240N                           74LS640N      |
	|-----------------------------------------------------------------|
	
	X1:		Crystal 6 MHz
	MCU_1:	NEC D8741AD marked "V1.1 3030"
	ROM_1:	MBM 2716 marked "GB 136-0"
	RAM_1:  NEC D4016D
	Video_1	Video-IC SND5027E, compatible with TMS9927

----------------------------------------------------------------------------------
	
	Floppy Controller board, all ICs shown

	|-----------------------------------------------------------------|
	|                                                                 |
    |  X1   74LS51N    F    74LS74AN   74LS02N        MC4044P         |
    |                  D                        567                   |
    |     74LS04N      C    74LS00N    74LS01N  :::   MC4024P         |
	|                                                                 |
	|   74LS00N        1    74LS74AN   74LS74AN       74LS14N        C|
    |                  7                                             N|
    |  74LS240N        9    74LS161N   74LS393N   74LS74AN           1|
    |                  1                                              |
    |   74LS132N            74LS14N    74LS14N    74LS374N            |
    |                                                                 |
    |    74LS123N   74LS04N  74LS163N   74LS14N    74LS241N           |
    |                                                                 |
    |     74LS393N   74LS138  74LS175N   74LS85N    74LS645N          |
	|                                                                 |
	|-----------------------------------------------------------------|
	
	X1:		Crystal 8 MHz
	FDC:	Siemens SAB1791-02P
	567:	Jumper Pad (emtpy)

----------------------------------------------------------------------------------
	
	256K RAM board, all ICs shown: 
	
	|-----------------------------------------------------------------|
	|                                                                 |
	|   HM4864P   HM4864P   HM4864P   HM4864P       74LS245N          |
	|                                                                 |
	|   HM4864P   HM4864P   HM4864P   HM4864P       P     74LS14N     |
	|                                               R                 |
	|   HM4864P   HM4864P   HM4864P   HM4864P       M     74LS00N     |
	|                                                                C|
	|   HM4864P   HM4864P   HM4864P   HM4864P       AM         A     N|
	|                                               29         M     1|
	|   HM4864P   HM4864P   HM4864P   HM4864P       66         2      |
	|                                               PC         9      |
	|   HM4864P   HM4864P   HM4864P   HM4864P                  6      |
	|                                               AM         4      |
	|   HM4864P   HM4864P   HM4864P   HM4864P       29         8      |
	|                                               66         P      |
	|   HM4864P   HM4864P   HM4864P   HM4864P       PC         C      |
	|                                                      SN7474N    |
	|-----------------------------------------------------------------|	
	
	PRM: 	N82S129F 1K Bipolar PROM
	        AM2966PC: Octal Dynamic Memory Drivers with Three-State Outputs
			AM29648PC
	CN1:	Connector to CN2 of Z80 CPU card
	
----------------------------------------------------------------------------------
	
	Parallel I/O board, all ICs shown: 
	
	|-------------------------------------|                                                                 |
	|                                     |
	|  74   74                            |
	|  LS   LS        Z80A PIO            |
	|  00   14                            |
	|   N    N                            |
	|                                     |
	|                                     |
	|  74   74        74   74   D4   74   |
	|  LS   LS        LS   LS   I3   LS   |
	|  13   14        24   85   P2   64   |
	|  2N    N        1N    N    1   0N   |
	|                                     |
    |             CN1                     |
	|                                     |
	|             74LS00N                 |
	|-------------------------------------|	

	CN1: Bus connector
	DIP: 4x DIP current setting: off-on-on-off, sets the address for the parallel port
	
----------------------------------------------------------------------------------

Beeper Circuit, all ICs shown:

	|---------------------------|                                                                 |
	|                           |
	|   BEEP       74LS132N     |
	| R1                        |
    |              74LS14N      |
    |                           |
    |   74LS132N   74LS193N     |
	|                           |
	|   74LS74AN   74LS165N     |
	|            CN1            |
	|---------------------------|
	
	CN1: Connector to mainboard
	R1:  looks like a potentiometer
	BEEP: Beeper ... touted in the manual as "Hupe" ... i.e. "horn" :)
	
----------------------------------------------------------------------------------
	
	Other boards and extensions mentioned in the manual:
	- S100 bus adapter board
	- IEEE 488 bus adapter board
	- 64K memory board
	- 8086 CPU board
	- external harddisk
	- TV adapter B/W (TV, Save/Load from Audio Cassette) with PROM/RAM/BASIC-Module with 16K or 32K RAM
	- TV adapter color with connection to Video / Keyboard combination card
	- Monitor adapters B/W and color
	- Video / Keyboard interface 2 with grayscale, 8 colors, loadable character set, blinking
	- Graphics Adapter with 16 colours, hi-res 512x256 pixels
	- RTC
	- Arithmetics chip
 
***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/wd_fdc.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "formats/itt3030_dsk.h"
#include "video/tms9927.h"			//Display hardware
#include "sound/beep.h"
#include "cpu/mcs48/mcs48.h"		//Keyboard MCU ... talks to the 8278 on the keyboard circuit


#define MAIN_CLOCK XTAL_4.194MHz

class itt3030_state : public driver_device
{
public:
	itt3030_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_kbdmcu(*this, "kbdmcu")
		, m_ram(*this, "mainram")
		, m_crtc(*this, "crt5027")
		, m_48kbank(*this, "lowerbank")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_beep(*this, "beeper")
		, m_vram(*this, "vram")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<i8041_device> m_kbdmcu;
	required_device<ram_device> m_ram;
	required_device<crt5027_device> m_crtc;
	required_device<address_map_bank_device> m_48kbank;
	required_device<fd1791_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<beep_device> m_beep;

	// shared pointers
	required_shared_ptr<UINT8> m_vram;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
public:

	DECLARE_READ8_MEMBER(vsync_r);
	DECLARE_READ8_MEMBER(unk2_r);
	DECLARE_WRITE8_MEMBER( beep_w );
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(bankl_r);
	DECLARE_WRITE8_MEMBER(bankl_w);
	DECLARE_READ8_MEMBER(bankh_r);
	DECLARE_WRITE8_MEMBER(bankh_w);
	DECLARE_READ8_MEMBER(kbd_fifo_r);
	DECLARE_READ8_MEMBER(kbd_matrix_r);
	DECLARE_WRITE8_MEMBER(kbd_matrix_w);
	DECLARE_FLOPPY_FORMATS(itt3030_floppy_formats);
private:
	UINT8 m_unk;
	UINT8 m_bank;
	UINT8 m_kbdrow, m_kbdcol, m_kbdclk;
	floppy_image_device *m_floppy;
};

void itt3030_state::video_start()
{
	m_unk = 0x80;
}

READ8_MEMBER(itt3030_state::vsync_r)
{
	return machine().primary_screen->vblank() ? 0x80 : 0;
}

READ8_MEMBER(itt3030_state::unk2_r)
{
	return 0x40;
}

WRITE8_MEMBER( itt3030_state::beep_w )
{
	m_beep->set_state(data&0x32);
}

WRITE8_MEMBER(itt3030_state::bank_w)
{
	int bank = 0;
	m_bank = data>>4;

	if (m_bank & 1)	// bank 8
	{
		bank = 8;
	}
	else
	{
		bank = m_bank >> 1; 
	}

//	printf("bank_w: new value %02x, m_bank %x, bank %x\n", data, m_bank, bank);

	m_48kbank->set_bank(bank);
}

READ8_MEMBER(itt3030_state::bankl_r)
{
	return m_ram->read(offset);
}

WRITE8_MEMBER(itt3030_state::bankl_w)
{
	m_ram->write(offset, data);
}

READ8_MEMBER(itt3030_state::bankh_r)
{
	return m_ram->read(((m_bank>>1)*0x10000) + offset + 0xc000);
}

WRITE8_MEMBER(itt3030_state::bankh_w)
{
	m_ram->write(((m_bank>>1)*0x10000) + offset + 0xc000, data);
}

UINT32 itt3030_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//address_space &space = m_maincpu->space(AS_PROGRAM);

	for(int y = 0; y < 24; y++ )
	{
		for(int x = 0; x < 80; x++ )
		{
			UINT8 code = m_vram[0x3000 + x + y*128];
			drawgfx_opaque(bitmap, cliprect, machine().gfx[0],  code , 0, 0,0, x*8,y*16);
		}
	}

	return 0;
}


// The lower 48K is switchable among the first 48K of each of 8 64K banks numbered 0-7 or "bank 8" which is the internal ROM and VRAM
// The upper 16K is always the top 16K of the selected bank 0-7, which allows bank/bank copies and such
// Port F6 bits 7-5 select banks 0-7, bit 4 enables bank 8

static ADDRESS_MAP_START( itt3030_map, AS_PROGRAM, 8, itt3030_state )
	AM_RANGE(0x0000, 0xbfff) AM_DEVICE("lowerbank", address_map_bank_device, amap8)
	AM_RANGE(0xc000, 0xffff) AM_READWRITE(bankh_r, bankh_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lower48_map, AS_PROGRAM, 8, itt3030_state )
	AM_RANGE(0x00000, 0x7ffff) AM_READWRITE(bankl_r, bankl_w)	// pages 0-7
//  AM_RANGE(0x00000, 0x7ffff) AM_DEVREADWRITE("mainram", ram_device, read, write)	// should work in theory, but compiler blows up spectacularly?
	AM_RANGE(0x80000, 0x807ff) AM_ROM AM_REGION("maincpu", 0)   // begin "page 8"
	AM_RANGE(0x80800, 0x80fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x81000, 0x810ff) AM_RAM AM_MIRROR(0x100)	// only 256 bytes, but ROM also clears 11xx?
	AM_RANGE(0x83000, 0x83fff) AM_RAM AM_SHARE("vram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( itt3030_io, AS_IO, 8, itt3030_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x26) AM_DEVREADWRITE("crt5027", crt5027_device, read, write)
	AM_RANGE(0x31, 0x31) AM_READ(unk2_r)
	AM_RANGE(0x32, 0x32) AM_WRITE(beep_w)
	AM_RANGE(0x35, 0x35) AM_READ(vsync_r)
	AM_RANGE(0x40, 0x40) AM_READ(kbd_fifo_r)
	AM_RANGE(0x50, 0x55) AM_DEVREADWRITE("fdc", fd1791_t, read, write)
	AM_RANGE(0xf6, 0xf6) AM_WRITE(bank_w)
ADDRESS_MAP_END

READ8_MEMBER(itt3030_state::kbd_fifo_r)
{
	return m_kbdmcu->upi41_master_r(space, 0);	// offset 0 is data, 1 is status
}

READ8_MEMBER(itt3030_state::kbd_matrix_r)
{
	return 0;
}

WRITE8_MEMBER(itt3030_state::kbd_matrix_w)
{
	m_kbdrow = data & 0xf;
	m_kbdcol = (data >> 4) & 0x7;
	m_kbdclk = (data & 0x80) ? 1 : 0;
}

// Schematics say:
// Port 1 goes to the keyboard matrix.  
// bits 0-3 select matrix rows, bits 4-6 choose column to read, bit 7 clocks the process (rising edge strobes the row, falling edge reads the data)
// T0 is the key matrix return
// Port 2 bit 2 is IRQ (in or out?)
static ADDRESS_MAP_START( kbdmcu_io, AS_IO, 8, itt3030_state )
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(kbd_matrix_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(kbd_matrix_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( itt3030 )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8, 16,              /* 8x16 characters */
	128,                /* 128 characters */
	1,                /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{7, 6, 5, 4, 3, 2, 1, 0},
	{ 0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* size of one char */
};

static GFXDECODE_START( itt3030 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END


void itt3030_state::machine_start()
{
}

void itt3030_state::machine_reset()
{
	m_bank = 1;
	m_48kbank->set_bank(8);
}

FLOPPY_FORMATS_MEMBER( itt3030_state::itt3030_floppy_formats )
  FLOPPY_ITT3030_FORMAT
FLOPPY_FORMATS_END


static SLOT_INTERFACE_START( itt3030_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END


static struct tms9927_interface crtc_intf =
{
	16,		// pixels per video memory address
	NULL	// "self-load data"?
};

static MACHINE_CONFIG_START( itt3030, itt3030_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(itt3030_map)
	MCFG_CPU_IO_MAP(itt3030_io)

	MCFG_CPU_ADD("kbdmcu", I8041, XTAL_6MHz)
	MCFG_CPU_IO_MAP(kbdmcu_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(250))
	MCFG_SCREEN_UPDATE_DRIVER(itt3030_state, screen_update)
	MCFG_SCREEN_SIZE(80*8, 24*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 24*16-1)
	
	/* devices */
	MCFG_DEVICE_ADD("lowerbank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(lower48_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)
	MCFG_DEVICE_ADD("crt5027", CRT5027, XTAL_6MHz)
	MCFG_DEVICE_CONFIG(crtc_intf)
	MCFG_FD1791x_ADD("fdc", XTAL_20MHz / 20)
	MCFG_WD_FDC_FORCE_READY
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", itt3030_floppies, "525dd", itt3030_state::itt3030_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", itt3030_floppies, "525dd", itt3030_state::itt3030_floppy_formats)

	MCFG_GFXDECODE(itt3030)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT_OVERRIDE(driver_device, black_and_white)

	/* internal ram */
	MCFG_RAM_ADD("mainram")
	MCFG_RAM_DEFAULT_SIZE("512K")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )
	
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( itt3030 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bootv1.2.bin", 0x0000, 0x0800, CRC(90279d45) SHA1(a39a3f31f4f98980b1ef50805870837fbf72261d))
	ROM_REGION( 0x0800, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "gb136-0.bin", 0x0000, 0x0800, CRC(6a3895a8) SHA1(f3b977ffa2f54c346521c9ef034830de8f404621))
	ROM_REGION( 0x0400, "kbdmcu", ROMREGION_ERASE00 )
	ROM_LOAD( "8741ad.bin", 0x0000, 0x0400, CRC(cabf4394) SHA1(e5d1416b568efa32b578ca295a29b7b5d20c0def))
ROM_END

GAME( 1982, itt3030,  0,   itt3030,  itt3030,  driver_device, 0,      ROT0, "ITT RFA",      "ITT3030", GAME_NOT_WORKING | GAME_NO_SOUND )

