/***************************************************************************

        AlphaSmart Pro

        08/28/2012 Skeleton driver

    TODO:
    - define video HW capabilities
    - "Addr. Bus RAM error" string read, presumably memory mapped RAM at 0x8000
      is actually a r/w bank register.

****************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "video/hd44780.h"
#include "rendlay.h"

class alphasmart_state : public driver_device
{
public:
	alphasmart_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_lcdc(*this, "hd44780"),
			m_rambank(*this, "rambank")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_memory_bank m_rambank;

	DECLARE_WRITE8_MEMBER(vram_w);

	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();

	DECLARE_READ8_MEMBER(port_a_r);
	DECLARE_WRITE8_MEMBER(port_a_w);
	DECLARE_READ8_MEMBER(port_d_r);
	DECLARE_WRITE8_MEMBER(port_d_w);

private:
	UINT8   m_port_a;
};

READ8_MEMBER(alphasmart_state::port_a_r)
{
	return m_port_a;
}

WRITE8_MEMBER(alphasmart_state::port_a_w)
{
	m_rambank->set_entry(((data>>3) & 0x01) | ((data>>4) & 0x02));
	m_port_a = data;
}

READ8_MEMBER(alphasmart_state::port_d_r)
{
	return 0;
}

WRITE8_MEMBER(alphasmart_state::port_d_w)
{

}


static ADDRESS_MAP_START(alphasmart_mem, AS_PROGRAM, 8, alphasmart_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x7fff ) AM_RAMBANK("rambank")
	AM_RANGE( 0x8000, 0xffff ) AM_ROM   AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(alphasmart_io, AS_IO, 8, alphasmart_state)
//  AM_RANGE(MC68HC11_IO_PORTA, MC68HC11_IO_PORTA) AM_DEVREADWRITE("hd44780", hd44780_device, control_read, control_write)
//  AM_RANGE(MC68HC11_IO_PORTD, MC68HC11_IO_PORTD) AM_DEVREADWRITE("hd44780", hd44780_device, data_read, data_write)
	AM_RANGE( MC68HC11_IO_PORTA, MC68HC11_IO_PORTA ) AM_READWRITE(port_a_r, port_a_w)
	AM_RANGE( MC68HC11_IO_PORTD, MC68HC11_IO_PORTD ) AM_READWRITE(port_d_r, port_d_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( alphasmart )
INPUT_PORTS_END

void alphasmart_state::palette_init()
{
	palette_set_color(machine(), 0, MAKE_RGB(138, 146, 148));
	palette_set_color(machine(), 1, MAKE_RGB(92, 83, 88));
}

void alphasmart_state::machine_start()
{
	m_rambank->configure_entries(0, 4, (UINT8*)(*memregion("mainram")), 0x8000);
}

void alphasmart_state::machine_reset()
{
	m_rambank->set_entry(0);
}

static const hc11_config alphasmart_hc11_config =
{
	0,     //has extended internal I/O
	192,   //internal RAM size
	0x00   //registers are at 0-0x3f
};

static HD44780_INTERFACE( alphasmart_4line_display )
{
	4,                  // number of lines
	16,                 // chars for line
	NULL                // pixel update callback
};

static MACHINE_CONFIG_START( alphasmart, alphasmart_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", MC68HC11, XTAL_8MHz/2)  // MC68HC11D0, XTAL is 8 Mhz, unknown divider
	MCFG_CPU_PROGRAM_MAP(alphasmart_mem)
	MCFG_CPU_IO_MAP(alphasmart_io)
	MCFG_CPU_CONFIG(alphasmart_hc11_config)
//  MCFG_CPU_PERIODIC_INT_DRIVER(alphasmart_state, irq0_line_hold,  50)

	MCFG_HD44780_ADD("hd44780", alphasmart_4line_display)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(6*40, 9*4)
	MCFG_SCREEN_VISIBLE_AREA(0, (6*40)-1, 0, (9*4)-1)
	MCFG_PALETTE_LENGTH(2)
	MCFG_DEFAULT_LAYOUT(layout_lcd)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( alphasma )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "alphasmartpro212.rom",  0x0000, 0x8000, CRC(896ddf1c) SHA1(c3c6a421c9ced92db97431d04b4a3f09a39de716) )   // Checksum 8D24 on label

	ROM_REGION( 0x20000, "mainram", ROMREGION_ERASE )

	ROM_REGION( 0x0860, "hd44780", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END


/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1995, alphasma,  0,       0,  alphasmart, alphasmart, driver_device,   0,   "Intelligent Peripheral Devices",   "AlphaSmart Pro", GAME_NOT_WORKING | GAME_NO_SOUND )
