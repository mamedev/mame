// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

"Sega Shooting Zone" aka "Sega Sharp Shooter"

This is an SMS with a timer system, official Sega product.

---------------------------------

Shooting Zone by SEGA 1987

834-6294

CPU(s) : D780C (x2)

Xtal : 10.7380 Mhz

RAMS(s) : D4168C (x3)
        : MB8464-12L

Eprom : Epr10894A.20

PAL : 315-5287

Customs IC's :  315-5216 (x2)
                315-5124

GAMES for this system :

Black Belt (mpr10150.ic1)

Shooting Gallery

Gangster Town

Marksman Shooting / Trap Shooting / Safari Hunt (315-5028.ic1 + Mpr10157.ic2)

Fantasy Zone

---------------------------------

Notes:
Apparently the first revision only had a gun, the second (the one dumped) had both a gun and a joystick.
Multiple auctions and pics on the web show different cartridge configurations, so the driver lets you select the whole SMS library.
To try the selection listed above use the following command:
mame shtzone -cart1 blackblt -cart2 shooting -cart3 gangster -cart4 marksman -cart5 fantzone


TODO:
- very similar to smssdisp in sms.cpp, merge?
- gun?
- actual timer doesn't seem to match what's set by the dips. Communications problem?
- outputs (test mode lists LED lamp, gun solenoid and buzzer). Lamp is done, buzzer maybe, gun solenoid isn't.
- the driver currently assumes the buzzer is used to acknowledge when a coin is inserted.
- should also have 1 card slot according to forum posts.
- layout for the led.
*/

#include "emu.h"
#include "sms.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class shtzone_state : public sms_state
{
public:
	shtzone_state(const machine_config &mconfig, device_type type, const char *tag) :
		sms_state(mconfig, type, tag),
		m_timercpu(*this, "timercpu"),
		m_buzzer(*this, "buzzer"),
		m_slots(*this, {"slot", "slot2", "slot3", "slot4", "slot5"}),
		m_led(*this, "led")
	{ }

	void shtzone(machine_config &config);

	ioport_value gun_tl_p1_r();
	ioport_value gun_tl_p2_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	required_device<cpu_device> m_timercpu;
	required_device<beep_device> m_buzzer;
	required_device_array<sega8_cart_slot_device, 5> m_slots;
	output_finder<> m_led;

	uint8_t m_control = 0;
	uint8_t m_selected_cart = 0;

	uint8_t cart_select_r();
	void cart_select_w(uint8_t data);
	void select_cart(uint8_t data);
	void control_w(uint8_t data);
	uint8_t cart_r(offs_t offset);
	void int_callback(int state);

	void prg_map(address_map &map) ATTR_COLD;
};


uint8_t shtzone_state::cart_r(offs_t offset)
{
	if (m_mem_device_enabled != 0x00)
	{
		uint8_t data = 0xff;

		if (BIT(m_mem_device_enabled, 2))
			data &= m_cartslot->read_cart(0x6000 + (offset & 0x1fff));

		return data;
	}
	else
	{
		return m_region_maincpu->base()[offset];
	}
}

uint8_t shtzone_state::cart_select_r()
{
	return m_selected_cart;
}

void shtzone_state::cart_select_w(uint8_t data)
{
	select_cart(data);
	m_selected_cart = data;
	setup_media_slots();
}

void shtzone_state::select_cart(uint8_t data)
{
	m_cartslot = m_slots[data >> 4].target();
}

void shtzone_state::control_w(uint8_t data)
{
	switch (data & 0xf0)
	{
		case 0x00: m_led = 0x06; break;
		case 0x10: m_led = 0x5b; break;
		case 0x20: m_led = 0x4f; break;
		case 0x40: m_led = 0x66; break;
		case 0x80: m_led = 0x6d; break;
	}

	if (BIT(data, 1))
	{
		m_maincpu->resume(SUSPEND_REASON_HALT);
	}
	else
	{
		// Pull reset line of CPU #0 low
		m_maincpu->reset();
		m_maincpu->suspend(SUSPEND_REASON_HALT, 1);
	}

	m_control = data;
}

void shtzone_state::int_callback(int state)
{
	if (BIT(m_control, 0))
		m_timercpu->set_input_line(0, state);
	else
		m_maincpu->set_input_line(0, state);
}


void shtzone_state::prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4fff).ram();
	map(0x5000, 0x5fff).ram().share("nvram");
	map(0x6000, 0x7fff).r(FUNC(shtzone_state::cart_r));
	map(0x8000, 0x8000).portr("DSW2").w(FUNC(shtzone_state::control_w));
	map(0x8001, 0x8001).portr("DSW1");
	map(0xa000, 0xa000).lw8(NAME([this] (uint8_t data) { m_buzzer->set_state(BIT(data, 0)); })); // TODO: is it? or is it coin counter? other bits are also used
	map(0xc000, 0xc000).rw(FUNC(shtzone_state::cart_select_r), FUNC(shtzone_state::cart_select_w));
	map(0xd800, 0xd800).portr("IN0");
	map(0xdc00, 0xdc00).portr("IN1");
}

ioport_value shtzone_state::gun_tl_p1_r()
{
	return BIT(m_port_ctrl1->in_r(), 4);
}

ioport_value shtzone_state::gun_tl_p2_r()
{
	return BIT(m_port_ctrl2->in_r(), 4);
}

static INPUT_PORTS_START( shtzone )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_LOW )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // does nothing in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "
	// directly tied from Light Phaser TL pins
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(shtzone_state, gun_tl_p1_r)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(shtzone_state, gun_tl_p2_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // does nothing in test mode
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // active high or nothing on screen (?)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DSW1:8,7,6,5")
	PORT_DIPSETTING(    0x0f, DEF_STR (1C_1C) )
	PORT_DIPSETTING(    0x0e, DEF_STR (1C_2C) )
	PORT_DIPSETTING(    0x0d, DEF_STR (1C_3C) )
	PORT_DIPSETTING(    0x0c, DEF_STR (1C_4C) )
	PORT_DIPSETTING(    0x0b, DEF_STR (1C_5C) )
	PORT_DIPSETTING(    0x0a, DEF_STR (1C_6C) )
	PORT_DIPSETTING(    0x09, DEF_STR (2C_1C) )
	PORT_DIPSETTING(    0x08, DEF_STR (3C_1C) )
	PORT_DIPSETTING(    0x07, DEF_STR (4C_1C) )
	PORT_DIPSETTING(    0x06, DEF_STR (5C_1C) )
	PORT_DIPSETTING(    0x05, DEF_STR (6C_1C) )
	PORT_DIPSETTING(    0x04, DEF_STR (3C_2C) )
	PORT_DIPSETTING(    0x03, DEF_STR (4C_3C) )
	PORT_DIPSETTING(    0x02, DEF_STR (5C_3C) )
	PORT_DIPSETTING(    0x01, DEF_STR (2C_3C) )
	PORT_DIPSETTING(    0x00, DEF_STR (4C_5C) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("DSW1:4,3,2,1")
	PORT_DIPSETTING(    0xf0, DEF_STR (1C_1C) )
	PORT_DIPSETTING(    0xe0, DEF_STR (1C_2C) )
	PORT_DIPSETTING(    0xd0, DEF_STR (1C_3C) )
	PORT_DIPSETTING(    0xc0, DEF_STR (1C_4C) )
	PORT_DIPSETTING(    0xb0, DEF_STR (1C_5C) )
	PORT_DIPSETTING(    0xa0, DEF_STR (1C_6C) )
	PORT_DIPSETTING(    0x90, DEF_STR (2C_1C) )
	PORT_DIPSETTING(    0x80, DEF_STR (3C_1C) )
	PORT_DIPSETTING(    0x70, DEF_STR (4C_1C) )
	PORT_DIPSETTING(    0x60, DEF_STR (5C_1C) )
	PORT_DIPSETTING(    0x50, DEF_STR (6C_1C) )
	PORT_DIPSETTING(    0x40, DEF_STR (3C_2C) )
	PORT_DIPSETTING(    0x30, DEF_STR (4C_3C) )
	PORT_DIPSETTING(    0x20, DEF_STR (5C_3C) )
	PORT_DIPSETTING(    0x10, DEF_STR (2C_3C) )
	PORT_DIPSETTING(    0x00, DEF_STR (4C_5C) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8") // no use shown in test mode
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7") // no use shown in test mode
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:6") // no use shown in test mode
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:5") // no use shown in test mode
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("DSW2:4,3,2,1")
	PORT_DIPSETTING(    0xf0, "0:30" )
	PORT_DIPSETTING(    0xe0, "1:00" )
	PORT_DIPSETTING(    0xd0, "1:30" )
	PORT_DIPSETTING(    0xc0, "2:00" )
	PORT_DIPSETTING(    0xb0, "2:30" )
	PORT_DIPSETTING(    0xa0, "3:00" )
	PORT_DIPSETTING(    0x90, "3:30" )
	PORT_DIPSETTING(    0x80, "4:00" )
	PORT_DIPSETTING(    0x70, "4:30" )
	PORT_DIPSETTING(    0x60, "5:00" )
	PORT_DIPSETTING(    0x50, "5:30" )
	PORT_DIPSETTING(    0x40, "6:00" )
	PORT_DIPSETTING(    0x30, "6:30" )
	PORT_DIPSETTING(    0x20, "7:00" )
	PORT_DIPSETTING(    0x10, "7:30" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END


void shtzone_state::machine_start()
{
	sms_state::machine_start();

	m_led.resolve();

	save_item(NAME(m_control));
	save_item(NAME(m_selected_cart));
}

void shtzone_state::machine_reset()
{
	sms_state::machine_reset();

	m_control = 0;
	m_selected_cart = 0;
	select_cart(m_selected_cart);
}

void shtzone_state::device_post_load()
{
	select_cart(m_selected_cart);
}


void shtzone_state::shtzone(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_timercpu, 10.738_MHz_XTAL / 3); // divider not verified
	m_timercpu->set_addrmap(AS_PROGRAM, &shtzone_state::prg_map);
	m_timercpu->set_addrmap(AS_IO, &shtzone_state::sms_io);

	// + SMS CPU
	sms_ntsc_base(config);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10.738_MHz_XTAL / 2,
			sega315_5124_device::WIDTH , sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256,
			sega315_5124_device::HEIGHT_NTSC, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_192_TBORDER_HEIGHT, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_192_TBORDER_HEIGHT + 192);
	screen.set_screen_update(m_vdp, FUNC(sega315_5124_device::screen_update));

	PALETTE(config, "palette").set_entries(0x100);

	SEGA315_5124(config, m_vdp, 10.738_MHz_XTAL);
	m_vdp->set_screen("screen");
	m_vdp->set_is_pal(false);
	m_vdp->n_int().set(FUNC(shtzone_state::int_callback));
	m_vdp->n_nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);

	BEEP(config, m_buzzer, 1000).add_route(ALL_OUTPUTS, "mono", 0.50);

	for (int i = 1; i < 5; i++)
		SMS_CART_SLOT(config, m_slots[i], sms_cart, nullptr);

	m_port_ctrl1->set_default_option("lphaser");
	m_port_ctrl2->set_default_option("lphaser");

	m_has_bios_full = false;
	m_has_pwr_led = false;
}


ROM_START( shtzone )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0x00)

	ROM_REGION( 0x4000, "timercpu", 0 )
	ROM_LOAD( "epr10894a.20", 0x00000, 0x04000, CRC(ea8901d9) SHA1(43fd8bfc395e3b2e3fbe9645d692a5eb04783d9c) )

	ROM_REGION(0x4000, "user1", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)
ROM_END

} // anonymous namespace


GAME( 1987, shtzone, 0, shtzone, shtzone, shtzone_state, empty_init, ROT0, "Sega", "Shooting Zone System BIOS", MACHINE_NOT_WORKING | MACHINE_IS_BIOS_ROOT )
