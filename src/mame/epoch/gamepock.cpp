// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*******************************************************************************

Epoch Game Pocket Computer
Japanese LCD handheld console

Hardware notes:
- NEC uPD78C06AG (4KB internal ROM), 6MHz XTAL
- 2KB external RAM(HM6116P-4), up to 28KB external ROM on cartridge
  (28KB in theory, actually the largest game is 16KB)
- 3*HD44102CH, 75*64 1bpp LCD screen
- 1-bit sound

Not counting the mini games included in the BIOS, only 5 games were released.
It takes around 3 seconds for a cartridge to start up, this is normal.

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/upd7810/upd7810.h"
#include "sound/spkrdev.h"
#include "video/hd44102.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "gamepock.lh"

namespace {

class gamepock_state : public driver_device
{
public:
	gamepock_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcd(*this, "lcd%u", 0),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN%u", 0)
	{ }

	void gamepock(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<upd78c06_device> m_maincpu;
	required_device_array<hd44102_device, 3> m_lcd;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<2> m_inputs;

	u8 m_control = 0;
	u8 m_lcd_data = 0;

	void control_w(u8 data);
	void lcd_data_w(u8 data);
	u8 input_r();
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};

void gamepock_state::machine_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_lcd_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// LCD outputs

u32 gamepock_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 3; i++)
	{
		const u8 *src = m_lcd[i]->render();

		for (int sx = 0; sx < 50; sx++)
		{
			for (int sy = 0; sy < 32; sy++)
			{
				int dx = sx;
				int dy = sy;

				// determine destination coordinate
				switch (i)
				{
					// LCD chip #0: top-left, but reverse-x
					case 0:
						dx = 49 - sx;
						break;

					// LCD chip #1: bottom-left
					case 1:
						dy += 32;
						break;

					// LCD chip #2: top-right + bottom-right
					case 2:
						dx = (sx % 25) + 50;
						dy += (sx / 25) * 32;
						break;

					default:
						break;
				}

				if (cliprect.contains(dx, dy))
					bitmap.pix(dy, dx) = src[sy * 50 + sx];
			}
		}
	}

	return 0;
}


// other I/O

void gamepock_state::control_w(u8 data)
{
	// 76------  input select
	// --543---  LCD CS
	// -----2--  LCD D/I (all 3)
	// ------1-  LCD M (all 3)
	// -------0  unknown

	// write to LCD on falling edge of M
	if (~data & m_control & 2)
	{
		for (int i = 0; i < 3; i++)
			if (BIT(data, i + 3))
				m_lcd[i]->write(BIT(data, 2), m_lcd_data);
	}

	m_control = data;
}

void gamepock_state::lcd_data_w(u8 data)
{
	// LCD DB0-DB7
	m_lcd_data = data;
}

u8 gamepock_state::input_r()
{
	u8 data = 0xff;

	for (int i = 0; i < 2; i++)
		if (BIT(m_control, i ^ 7))
			data &= m_inputs[i]->read();

	return data;
}

void gamepock_state::main_map(address_map &map)
{
	// 0x0000-0x0fff is internal ROM
	map(0x0000, 0x7fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0xc000, 0xc7ff).mirror(0x0800).ram();
	// 0xff80-0xffff is internal RAM
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( gamepock )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) // top-left
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) // bottom-left
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) // top-right
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) // bottom-right
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void gamepock_state::gamepock(machine_config &config)
{
	// basic machine hardware
	UPD78C06(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &gamepock_state::main_map);
	m_maincpu->pa_out_cb().set(FUNC(gamepock_state::control_w));
	m_maincpu->pb_out_cb().set(FUNC(gamepock_state::lcd_data_w));
	m_maincpu->pc_in_cb().set(FUNC(gamepock_state::input_r));
	m_maincpu->to_func().set(m_speaker, FUNC(speaker_sound_device::level_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(75, 64);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(gamepock_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	for (int i = 0; i < 3; i++)
		HD44102(config, m_lcd[i]);

	config.set_default_layout(layout_gamepock);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_linear_slot, "gamepock_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("gamepock");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( gamepock )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "egpcboot.bin", 0x0000, 0x1000, CRC(ee1ea65d) SHA1(9c7731b5ead721d2cc7f7e2655c5fed9e56db8b0) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME                FLAGS
SYST( 1984, gamepock, 0,      0,      gamepock, gamepock, gamepock_state, empty_init, "Epoch", "Game Pocket Computer", MACHINE_SUPPORTS_SAVE )
