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
#include "sound/spkrdev.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/snes_ctrl/ctrl.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

// overclocked to 8 * NTSC burst frequency
#define MASTER_CLOCK 28618180

#define INTERLACED      0

class uzebox_state : public driver_device
{
public:
	uzebox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_cart(*this, "cartslot")
		, m_ctrl1(*this, "ctrl1")
		, m_ctrl2(*this, "ctrl2")
		, m_speaker(*this, "speaker")
	{ }

	void uzebox(machine_config &config);

private:
	required_device<avr8_device> m_maincpu;
	required_device<screen_device> m_screen;
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

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void line_update();
	uint32_t screen_update_uzebox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(uzebox_cart);

	void uzebox_data_map(address_map &map);
	void uzebox_io_map(address_map &map);
	void uzebox_prg_map(address_map &map);

	int             m_vpos;
	uint64_t          m_line_start_cycles;
	uint32_t          m_line_pos_cycles;
	uint8_t           m_port_a;
	uint8_t           m_port_b;
	uint8_t           m_port_c;
	uint8_t           m_port_d;
	bitmap_rgb32    m_bitmap;
};

void uzebox_state::machine_start()
{
	m_screen->register_screen_bitmap(m_bitmap);

	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0xffff, read8sm_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));
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

	uint8_t changed = m_port_a ^ data;
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

			uint32_t cycles = (uint32_t)(m_maincpu->get_elapsed_cycles() - m_line_start_cycles);
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

void uzebox_state::uzebox_prg_map(address_map &map)
{
	map(0x0000, 0xffff).rom(); // 64 KB internal eprom  ATmega644
}

void uzebox_state::uzebox_data_map(address_map &map)
{
	map(0x0100, 0x10ff).ram(); //  4KB RAM
}

void uzebox_state::uzebox_io_map(address_map &map)
{
	map(AVR8_REG_A, AVR8_REG_A).rw(FUNC(uzebox_state::port_a_r), FUNC(uzebox_state::port_a_w));
	map(AVR8_REG_B, AVR8_REG_B).rw(FUNC(uzebox_state::port_b_r), FUNC(uzebox_state::port_b_w));
	map(AVR8_REG_C, AVR8_REG_C).rw(FUNC(uzebox_state::port_c_r), FUNC(uzebox_state::port_c_w));
	map(AVR8_REG_D, AVR8_REG_D).rw(FUNC(uzebox_state::port_d_r), FUNC(uzebox_state::port_d_w));
}

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
	uint32_t cycles = (uint32_t)(m_maincpu->get_elapsed_cycles() - m_line_start_cycles) / 2;
	rgb_t color = rgb_t(pal3bit(m_port_c >> 0), pal3bit(m_port_c >> 3), pal2bit(m_port_c >> 6));

	for (uint32_t x = m_line_pos_cycles; x < cycles; x++)
	{
		if (m_bitmap.cliprect().contains(x, m_vpos))
			m_bitmap.pix32(m_vpos, x) = color;
		if (!INTERLACED)
			if (m_bitmap.cliprect().contains(x, m_vpos + 1))
				m_bitmap.pix32(m_vpos + 1, x) = color;
	}

	m_line_pos_cycles = cycles;
}

uint32_t uzebox_state::screen_update_uzebox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

DEVICE_IMAGE_LOAD_MEMBER(uzebox_state, uzebox_cart)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);

	if (!image.loaded_through_softlist())
	{
		std::vector<uint8_t> data(size);
		image.fread(&data[0], size);

		if (!strncmp((const char*)&data[0], "UZEBOX", 6))
			memcpy(m_cart->get_rom_base(), &data[0x200], size - 0x200);
		else
			memcpy(m_cart->get_rom_base(), &data[0], size);
	}
	else
		memcpy(m_cart->get_rom_base(), image.get_software_region("rom"), size);

	return image_init_result::PASS;
}


/****************************************************\
* Machine definition                                 *
\****************************************************/

void uzebox_state::uzebox(machine_config &config)
{
	/* basic machine hardware */
	ATMEGA644(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &uzebox_state::uzebox_prg_map);
	m_maincpu->set_addrmap(AS_DATA, &uzebox_state::uzebox_data_map);
	m_maincpu->set_addrmap(AS_IO, &uzebox_state::uzebox_io_map);
	m_maincpu->set_eeprom_tag("eeprom");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(59.99);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1395));
	m_screen->set_size(870, 525);
	m_screen->set_visarea(150, 870-1, 40, 488-1);
	m_screen->set_screen_update(FUNC(uzebox_state::screen_update_uzebox));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(0, "mono", 1.00);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "uzebox", "bin,uze");
	m_cart->set_must_be_loaded(true);
	m_cart->set_device_load(device_image_load_delegate(&uzebox_state::device_image_load_uzebox_cart, this));

	SNES_CONTROL_PORT(config, m_ctrl1, snes_control_port_devices, "joypad");
	SNES_CONTROL_PORT(config, m_ctrl2, snes_control_port_devices, "joypad");

	SOFTWARE_LIST(config, "eprom_list").set_original("uzebox");
}

ROM_START( uzebox )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )  /* Main program store */

	ROM_REGION( 0x800, "eeprom", ROMREGION_ERASE00 )  /* on-die eeprom */
ROM_END

/*   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY    FULLNAME */
CONS(2010, uzebox, 0,      0,      uzebox,  uzebox, uzebox_state, empty_init, "Belogic", "Uzebox", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING)
