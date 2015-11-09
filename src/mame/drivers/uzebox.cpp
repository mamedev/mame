// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Belogic Uzebox

    driver by Sandro Ronco

    TODO:
    - Sound
    - SDCard

****************************************************************************/

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "sound/speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/snes_ctrl/ctrl.h"
#include "softlist.h"

// overclocked to 8 * NTSC burst frequency
#define MASTER_CLOCK 28618180

#define INTERLACED      0

class uzebox_state : public driver_device
{
public:
	uzebox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_ctrl1(*this, "ctrl1"),
		m_ctrl2(*this, "ctrl2"),
		m_speaker(*this, "speaker")
	{ }

	required_device<avr8_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<snes_control_port_device> m_ctrl1;
	required_device<snes_control_port_device> m_ctrl2;
	required_device<speaker_sound_device> m_speaker;

	DECLARE_READ8_MEMBER(port_a_r);
	DECLARE_WRITE8_MEMBER(port_a_w);
	DECLARE_READ8_MEMBER(port_b_r);
	DECLARE_WRITE8_MEMBER(port_b_w);
	DECLARE_READ8_MEMBER(port_c_r);
	DECLARE_WRITE8_MEMBER(port_c_w);
	DECLARE_READ8_MEMBER(port_d_r);
	DECLARE_WRITE8_MEMBER(port_d_w);

	virtual void machine_start();
	virtual void machine_reset();
	void line_update();
	UINT32 screen_update_uzebox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(uzebox_cart);

private:
	int             m_vpos;
	UINT64          m_line_start_cycles;
	UINT32          m_line_pos_cycles;
	UINT8           m_port_a;
	UINT8           m_port_b;
	UINT8           m_port_c;
	UINT8           m_port_d;
	bitmap_rgb32    m_bitmap;
};

void uzebox_state::machine_start()
{
	machine().first_screen()->register_screen_bitmap(m_bitmap);

	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0xffff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));
}

void uzebox_state::machine_reset()
{
	m_vpos = 0;
	m_line_start_cycles = 0;
	m_line_pos_cycles = 0;
	m_port_a = 0;
	m_port_b = 0;
	m_port_c = 0;
	m_port_d = 0;
}


WRITE8_MEMBER(uzebox_state::port_a_w)
{
	//  xxxx ----   NC
	//  ---- x---   SNES controller clk
	//  ---- -x--   SNES controller latch
	//  ---- --x-   SNES controller P2 data
	//  ---- ---x   SNES controller P1 data

	m_ctrl1->write_strobe(BIT(data, 2));
	m_ctrl2->write_strobe(BIT(data, 2));

	UINT8 changed = m_port_a ^ data;
	if ((changed & data & 0x08) || (changed & (~data) & 0x04))
	{
		m_port_a &= ~0x03;
		m_port_a |= m_ctrl1->read_pin4() ? 0 : 0x01;
		m_port_a |= m_ctrl2->read_pin4() ? 0 : 0x02;
	}

	m_port_a = (data & 0x0c) | (m_port_a & 0x03);
}

READ8_MEMBER(uzebox_state::port_a_r)
{
	return  m_port_a | 0xf0;
}

WRITE8_MEMBER(uzebox_state::port_b_w)
{
	//  xxx- ----   SDCard
	//  ---x ----   AD725 CE
	//  ---- x---   AD725 4FSC
	//  ---- -xx-   NC
	//  ---- ---x   AD725 HSYNC

	// AD725 CE is hard-wired to VCC in early revisions (C1, D1 and E1)
	if ((m_port_b & 0x10) || ioport("AD725_CE")->read() == 0)
		if ((m_port_b ^ data) & m_port_b & 0x01)
		{
			line_update();

			UINT32 cycles = (UINT32)(m_maincpu->get_elapsed_cycles() - m_line_start_cycles);
			if (cycles < 1000 && m_vpos >= 448)
				m_vpos = INTERLACED ? ((m_vpos ^ 0x01) & 0x01) : 0;
			else if (cycles > 1000)
				m_vpos += 2;

			m_line_start_cycles = m_maincpu->get_elapsed_cycles();
			m_line_pos_cycles = 0;
		}

	m_port_b = data;
}

READ8_MEMBER(uzebox_state::port_b_r)
{
	return m_port_b;
}

WRITE8_MEMBER(uzebox_state::port_c_w)
{
	//  xx-- ----   blue
	//  --xx x---   green
	//  ---- -xxx   red

	line_update();
	m_port_c = data;
}

READ8_MEMBER(uzebox_state::port_c_r)
{
	return m_port_c;
}

WRITE8_MEMBER(uzebox_state::port_d_w)
{
	//  x--- ----   sound
	//  -x-- ----   SDCard CS
	//  ---x ----   LED
	//  --x- x---   NC
	//  ---- -x--   power
	//  ---- --xx   UART MIDI
	if ((m_port_d ^ data) & 0x80)
	{
		m_speaker->level_w((data & 0x80) ? 1 : 0);
	}
	m_port_d = data;
}

READ8_MEMBER(uzebox_state::port_d_r)
{
	return m_port_d;
}


/****************************************************\
* Address maps                                       *
\****************************************************/

static ADDRESS_MAP_START( uzebox_prg_map, AS_PROGRAM, 8, uzebox_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM // 64 KB internal eprom  ATmega644
ADDRESS_MAP_END

static ADDRESS_MAP_START( uzebox_data_map, AS_DATA, 8, uzebox_state )
	AM_RANGE(0x0100, 0x10ff) AM_RAM //  4KB RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( uzebox_io_map, AS_IO, 8, uzebox_state )
	AM_RANGE(AVR8_REG_A, AVR8_REG_A) AM_READWRITE( port_a_r, port_a_w )
	AM_RANGE(AVR8_REG_B, AVR8_REG_B) AM_READWRITE( port_b_r, port_b_w )
	AM_RANGE(AVR8_REG_C, AVR8_REG_C) AM_READWRITE( port_c_r, port_c_w )
	AM_RANGE(AVR8_REG_D, AVR8_REG_D) AM_READWRITE( port_d_r, port_d_w )
ADDRESS_MAP_END

/****************************************************\
* Input ports                                        *
\****************************************************/

static INPUT_PORTS_START( uzebox )
	PORT_START("AD725_CE")
	PORT_CONFNAME( 0x01, 0x00, "AD725 CE" )
	PORT_CONFSETTING( 0x00, "VCC" )
	PORT_CONFSETTING( 0x01, "PB4" )
INPUT_PORTS_END

/****************************************************\
* Video hardware                                     *
\****************************************************/

void uzebox_state::line_update()
{
	UINT32 cycles = (UINT32)(m_maincpu->get_elapsed_cycles() - m_line_start_cycles) / 2;
	rgb_t color = rgb_t(pal3bit(m_port_c >> 0), pal3bit(m_port_c >> 3), pal2bit(m_port_c >> 6));

	for (UINT32 x = m_line_pos_cycles; x < cycles; x++)
	{
		if (m_bitmap.cliprect().contains(x, m_vpos))
			m_bitmap.pix32(m_vpos, x) = color;
		if (!INTERLACED)
			if (m_bitmap.cliprect().contains(x, m_vpos + 1))
				m_bitmap.pix32(m_vpos + 1, x) = color;
	}

	m_line_pos_cycles = cycles;
}

UINT32 uzebox_state::screen_update_uzebox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

DEVICE_IMAGE_LOAD_MEMBER(uzebox_state, uzebox_cart)
{
	UINT32 size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);

	if (image.software_entry() == NULL)
	{
		dynamic_buffer data(size);
		image.fread(&data[0], size);

		if (!strncmp((const char*)&data[0], "UZEBOX", 6))
			memcpy(m_cart->get_rom_base(), &data[0x200], size - 0x200);
		else
			memcpy(m_cart->get_rom_base(), &data[0], size);
	}
	else
		memcpy(m_cart->get_rom_base(), image.get_software_region("rom"), size);

	return IMAGE_INIT_PASS;
}


/****************************************************\
* Machine definition                                 *
\****************************************************/

static MACHINE_CONFIG_START( uzebox, uzebox_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ATMEGA644, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(uzebox_prg_map)
	MCFG_CPU_DATA_MAP(uzebox_data_map)
	MCFG_CPU_IO_MAP(uzebox_io_map)
	MCFG_CPU_AVR8_EEPROM("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.99)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1395))
	MCFG_SCREEN_SIZE(870, 525)
	MCFG_SCREEN_VISIBLE_AREA(150, 870-1, 40, 488-1)
	MCFG_SCREEN_UPDATE_DRIVER(uzebox_state, screen_update_uzebox)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(0, "mono", 1.00)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "uzebox")
	MCFG_GENERIC_EXTENSIONS("bin,uze")
	MCFG_GENERIC_MANDATORY
	MCFG_GENERIC_LOAD(uzebox_state, uzebox_cart)

	MCFG_SNES_CONTROL_PORT_ADD("ctrl1", snes_control_port_devices, "joypad")
	MCFG_SNES_CONTROL_PORT_ADD("ctrl2", snes_control_port_devices, "joypad")

	MCFG_SOFTWARE_LIST_ADD("eprom_list","uzebox")
MACHINE_CONFIG_END

ROM_START( uzebox )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )  /* Main program store */

	ROM_REGION( 0x800, "eeprom", ROMREGION_ERASE00 )  /* on-die eeprom */
ROM_END

/*   YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     INIT      COMPANY   FULLNAME */
CONS(2010, uzebox,   0,        0,        uzebox,   uzebox, driver_device,   0,  "Belogic", "Uzebox", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING)
