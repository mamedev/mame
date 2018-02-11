// license:BSD-3-Clause
// copyright-holders: Gabriele D'Antona
/* 
	Olympia BOSS
	Made in Germany around 1981
	
	The BOSS series was not a great success, as its members differed too much to be compatible:
	First they were 8085 based, later machines used a Z80A.
	
	Other distinguishing features were the capacity of the disk drives:
	
	BOSS A: Two 128K floppy drives
	BOSS B: Two 256K disk drives
	BOSS C: Two 600K disk drives
	BOSS D: One 600K disk drive, one 5 MB harddisk
	BOSS M: M for multipost, up to four BOSS machines linked together for up to 20MB shared harddisk space
	
	Olympia favoured the French Prologue operating system over CPM (cf. Olympia People PC) and supplied BAL
	as a programming language with it.
		
	Video is 80x28
	
	There are no service manuals available (or no documentation in general), so everything is guesswork.

	- Ports 0x80 and 0x81 seem to be related to the graphics chip and cursor position
	The rom outs value 0x81 to port 0x81 and then the sequence <column> <row> (?) to port 0x80

	- The machine boots up and shows "BOSS .." on the screen. Every keystroke is repeated on screen.
	If you press <return>, the machine seems to go into a boot sequence (from the HD, probably)
	
	The harddisk controller is based on a MSC-9056.
	
	Links: http://www.old-computers.com/museum/computer.asp?c=95
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/keyboard.h"
#include "video/upd3301.h"
#include "machine/i8257.h"
#include "machine/am9519.h"
#include "machine/upd765.h"
#include "screen.h"

#define Z80_TAG         "z80"
#define UPD3301_TAG     "upd3301"
#define I8257_TAG       "i8257"
#define SCREEN_TAG		"screen"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class olyboss_state : public driver_device
{
public:
	olyboss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_dma(*this, I8257_TAG),
			m_crtc(*this,UPD3301_TAG),
			m_char_rom(*this, UPD3301_TAG)
		{ }

	DECLARE_DRIVER_INIT(olyboss);
	DECLARE_READ8_MEMBER(keyboard_read);

	UPD3301_DRAW_CHARACTER_MEMBER( olyboss_display_pixels );

	DECLARE_WRITE_LINE_MEMBER( hrq_w );
	DECLARE_READ8_MEMBER( dma_mem_r );
	
	void olybossd(machine_config &config);
	
private:
	required_device<cpu_device> m_maincpu;
	required_device<i8257_device> m_dma;
	required_device<upd3301_device> m_crtc;
	required_memory_region m_char_rom;

	bool m_keybhit;
	u8 m_keystroke;
	void keyboard_put(u8 data);	


};

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START(olyboss_mem, AS_PROGRAM, 8, olyboss_state)
	AM_RANGE(0x0000, 0x7ff ) AM_ROM AM_REGION("mainrom", 0)
	AM_RANGE(0x800,  0xbffd) AM_RAM
	AM_RANGE(0xbffe, 0xbfff) AM_READ(keyboard_read)
	AM_RANGE(0xc000,  0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(olyboss_io, AS_IO, 8, olyboss_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0, 0x8) AM_DEVREADWRITE(I8257_TAG, i8257_device, read, write)
	AM_RANGE(0x10, 0x11) AM_DEVICE("fdc", upd765a_device, map)
	AM_RANGE(0x30, 0x30) AM_DEVREADWRITE("uic", am9519_device, data_r, data_w)
	AM_RANGE(0x31, 0x31) AM_DEVREADWRITE("uic", am9519_device, stat_r, cmd_w)
	AM_RANGE(0x80, 0x81) AM_DEVREADWRITE(UPD3301_TAG, upd3301_device, read, write)
ADDRESS_MAP_END

static INPUT_PORTS_START( olyboss )
	PORT_START("DSW")
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

UPD3301_DRAW_CHARACTER_MEMBER( olyboss_state::olyboss_display_pixels )
{
	uint8_t data = m_char_rom->base()[(cc << 4) | lc];
	int i;

	//if (lc >= 8) return;
	if (csr) 
	{
		data = 0xff;
	}

	for (i = 0; i < 8; i++)
	{
		int color = BIT(data, 7) ^ rvv;
		bitmap.pix32(y, (sx * 8) + i) = color?0xffffff:0;
		data <<= 1;
	}
}

//**************************************************************************
//  KEYBOARD
//**************************************************************************

READ8_MEMBER( olyboss_state::keyboard_read )
{
	// logerror ("keyboard_read offs [%d]\n",offset);
	if (offset==0)
	{
		if (m_keybhit)
		{
			return 0x01;
		}
		
		return 0x00;
	}
	else if (offset==1)
	{
		if (m_keybhit)
		{
			m_keybhit=false;
			return m_keystroke;
		}
		
		return 0x00;
	}
	
	return 0x00;
}

DRIVER_INIT_MEMBER( olyboss_state, olyboss )
{
	m_keybhit=false;

	/* initialize DMA */
	m_dma->ready_w(1);
}

void olyboss_state::keyboard_put(u8 data)
{
	if (data)
	{
		//logerror("Keyboard stroke [%2x]\n",data);
		m_keystroke=data;
		m_keybhit=true;
	}
}

/* 8257 Interface */

WRITE_LINE_MEMBER( olyboss_state::hrq_w )
{
	//logerror("hrq_w\n");
	m_maincpu->set_input_line(INPUT_LINE_HALT,state);
	m_dma->hlda_w(state);
}

READ8_MEMBER( olyboss_state::dma_mem_r )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	return program.read_byte(offset);
}

static SLOT_INTERFACE_START( boss_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

MACHINE_CONFIG_START( olyboss_state::olybossd )
	MCFG_CPU_ADD(Z80_TAG, Z80, 4_MHz_XTAL)
	MCFG_CPU_PROGRAM_MAP(olyboss_mem)
	MCFG_CPU_IO_MAP(olyboss_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("uic", am9519_device, iack_cb)

	/* video hardware */

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DEVICE(UPD3301_TAG, upd3301_device, screen_update)
	MCFG_SCREEN_SIZE(80*8, 28*11)
	MCFG_SCREEN_VISIBLE_AREA(0, (80*8)-1, 0, (28*11)-1)

	/* devices */

	MCFG_DEVICE_ADD("uic", AM9519, 0)
	MCFG_AM9519_OUT_INT_CB(INPUTLINE(Z80_TAG, 0))

	MCFG_UPD765A_ADD("fdc", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE("uic", am9519_device, ireq2_w)) MCFG_DEVCB_INVERT
	MCFG_UPD765_DRQ_CALLBACK(NOOP)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", boss_floppies, "525qd", floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD(I8257_TAG, I8257, XTAL(4'000'000))
	MCFG_I8257_OUT_HRQ_CB(WRITELINE(olyboss_state, hrq_w))
	MCFG_I8257_IN_MEMR_CB(READ8(olyboss_state, dma_mem_r))
	MCFG_I8257_OUT_IOW_2_CB(DEVWRITE8(UPD3301_TAG, upd3301_device, dack_w))

	MCFG_DEVICE_ADD(UPD3301_TAG, UPD3301, XTAL(14'318'181))
	MCFG_UPD3301_CHARACTER_WIDTH(8)
	MCFG_UPD3301_DRAW_CHARACTER_CALLBACK_OWNER(olyboss_state, olyboss_display_pixels)
	MCFG_UPD3301_DRQ_CALLBACK(DEVWRITELINE(I8257_TAG, i8257_device, dreq2_w))
	MCFG_UPD3301_INT_CALLBACK(DEVWRITELINE("uic", am9519_device, ireq0_w)) MCFG_DEVCB_INVERT
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)

	/* keyboard */
	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(PUT(olyboss_state, keyboard_put))
	
MACHINE_CONFIG_END

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( olybossd )
	ROM_REGION(0x800, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD( "olympia_boss_system_251-462.bin", 0x0000, 0x800, CRC(01b99609) SHA1(07b764c36337c12f7b40aa309b0805ceed8b22e2) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "olympia_boss_graphics_251-461.bin", 0x0000, 0x800, CRC(56149540) SHA1(b2b893bd219308fc98a38528beb7ddae391c7609) )
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//   YEAR  NAME			PARENT	COMPAT	MACHINE		INPUT		CLASS			INIT		COMPANY						FULLNAME			FLAGS
COMP(1981, olybossd,	0,		0,		olybossd,	olyboss,	olyboss_state,	olyboss,	"Olympia International",	"Olympia BOSS D",	MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

