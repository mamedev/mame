// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Robbbert
/***************************************************************************

Bandai Super Vision 8000 (TV-Jack Micro Computer System) (aka TV Jack 8000)
TV JACK スーパービジョン8000

driver by Wilbert Pol, Robbbert, ranger_lennier, and Charles McDonald

The Bandai Super Vision 8000 contains:
- NEC D780C (Z80)
- AY-3-8910
- AMI S68047P (6847 variant)
- NEC D8255C

Looking at the code of the cartridges it seems there is:
- 1KB of main system RAM
- 3KB of video RAM

TODO:
- Check configuration of S68047P pins through 8910 port A against PCB
- Verify clocks, verify exact model of soundchip
- Verify colors, it kind of matches videos/photos, but it's much worse with
  the default S68047P palette (games don't use the first 8 colors)

****************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/mc6847.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class sv8000_state : public driver_device
{
public:
	sv8000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s68047p(*this, "s68047p")
		, m_videoram(*this, "videoram")
		, m_io_row(*this, "ROW%u", 0)
	{ }

	void sv8000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void ay_port_a_w(u8 data);

	u8 i8255_porta_r();
	u8 i8255_portb_r();
	void i8255_portc_w(u8 data);

	u8 mc6847_videoram_r(offs_t offset);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<s68047_device> m_s68047p;
	required_shared_ptr<u8> m_videoram;
	required_ioport_array<3> m_io_row;

	u8 m_select = 0xff;
	u8 m_ag = 0U;
};

void sv8000_state::machine_start()
{
	save_item(NAME(m_select));
	save_item(NAME(m_ag));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// Palette, most notably for the 2nd color set it expects a blue background instead of black

static const u32 sv8000_palette[16] =
{
	rgb_t(0x30, 0xd2, 0x00), // GREEN
	rgb_t(0xc1, 0xe5, 0x00), // YELLOW
	rgb_t(0x41, 0xaf, 0x71), // CYAN
	rgb_t(0x9a, 0x32, 0x36), // RED
	rgb_t(0x4c, 0x3a, 0xb4), // BLUE
	rgb_t(0x20, 0xd8, 0xe0), // CYAN/BLUE
	rgb_t(0xc8, 0x4e, 0xf0), // MAGENTA
	rgb_t(0xd4, 0x7f, 0x00), // ORANGE

	rgb_t(0x20, 0x40, 0x00), // BLACK -> DARK GREEN
	rgb_t(0xff, 0xf8, 0x70), // GREEN -> YELLOW
	rgb_t(0x18, 0x30, 0xb0), // BLACK -> BLUE
	rgb_t(0xf0, 0xff, 0xff), // CYAN/BLUE -> BRIGHT CYAN

	rgb_t(0x20, 0x40, 0x00), // BLACK -> DARK GREEN
	rgb_t(0xff, 0xf8, 0x70), // GREEN -> YELLOW
	rgb_t(0x18, 0x30, 0xb0), // BLACK -> BLUE
	rgb_t(0xf0, 0xff, 0xff), // BLUE -> BRIGHT CYAN
};

void sv8000_state::ay_port_a_w(u8 data)
{
	// Lacking schematics, these are all wild guesses, see below for values written
	/*
	misvader:
	0x42 01000010 text
	0x5A 01011010 graphics

	spfire:
	0x42 01000010 text
	0x5A 01011010 graphics

	othello:
	0x02 00000010 text
	0x58 01011000 graphics

	gunprof:
	0x00 00000000 text
	0x38 00111000 graphics

	pacpac:
	0x00 00000000 text
	0x5A 01011010 graphics

	submar:
	0x00 00000000 text
	0x5A 01011010 graphics

	beamgala:
	0x5A 01011010 graphics
	*/

	m_s68047p->css_w(BIT(data, 1));
	m_s68047p->ag_w(BIT(data, 3));
	m_s68047p->gm0_w(BIT(data, 4));
	m_s68047p->gm1_w(BIT(data, 5));
	m_s68047p->gm2_w(BIT(data, 6));

	m_ag = BIT(data, 3);
}

u8 sv8000_state::mc6847_videoram_r(offs_t offset)
{
	offset &= 0xfff;
	if (offset >= 0xc00)
		return 0xff;

	// Graphics
	if (m_ag)
		return m_videoram[offset];

	// Standard text
	u8 data = m_videoram[offset];
	if (data == 0)
		data = 0x20; // bodge

	m_s68047p->inv_w(BIT(data, 7));

	return data;
}

u8 sv8000_state::i8255_portb_r()
{
	u8 data = 0xff;

	// Read keypads
	for (int i = 0; i < 3; i++)
		if (!BIT(m_select, i))
			data &= m_io_row[i]->read();

	return data;
}

void sv8000_state::i8255_portc_w(u8 data)
{
	m_select = data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void sv8000_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0x8000, 0x83ff).ram(); // Work RAM
	map(0xc000, 0xcfff).ram().share(m_videoram);
}

void sv8000_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x83).rw("i8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc0, 0xc0).w("ay8910", FUNC(ay8910_device::data_w));
	map(0xc1, 0xc1).w("ay8910", FUNC(ay8910_device::address_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

/*
On the main console:

1 2 3                   1 2 3
4 5 6                   4 5 6
7 8 9                   7 8 9
* 0 #                   * 0 #

D-Pad   POWER   RESET   D-Pad
*/
static INPUT_PORTS_START( sv8000 )
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Right *")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Right 7")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Right 4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Right 1")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left *")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Left 7")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Left 4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Left 1")

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Right 0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Right 8")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Right 5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Right 2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Left 0")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Left 8")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Left 5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Left 2")

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Right #")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Right 9")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Right 6")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Right 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Left #")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Left 9")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Left 6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Left 3")

	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_NAME("Right Up")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_NAME("Right Down")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_NAME("Right Left")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("Right Right")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Left Up")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Left Down")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Left Left")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Left Right")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sv8000_state::sv8000(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 3.579545_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &sv8000_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &sv8000_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(sv8000_state::irq0_line_hold));

	i8255_device &ppi(I8255(config, "i8255"));
	ppi.in_pa_callback().set_ioport("JOY");
	ppi.in_pb_callback().set(FUNC(sv8000_state::i8255_portb_r));
	ppi.out_pc_callback().set(FUNC(sv8000_state::i8255_portc_w));

	// video hardware
	S68047(config, m_s68047p, 3.579545_MHz_XTAL);
	m_s68047p->input_callback().set(FUNC(sv8000_state::mc6847_videoram_r));
	m_s68047p->set_palette(sv8000_palette);
	m_s68047p->set_screen("screen");

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay8910(AY8910(config, "ay8910", 3.579545_MHz_XTAL/2));
	ay8910.port_a_write_callback().set(FUNC(sv8000_state::ay_port_a_w));
	ay8910.add_route(ALL_OUTPUTS, "mono", 0.50);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "sv8000_cart").set_must_be_loaded(true);
	SOFTWARE_LIST(config, "cart_list").set_original("sv8000");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sv8000 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF ) // Mapped by the cartridge slot
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   STATE         INIT        COMPANY   FULLNAME             FLAGS
CONS( 1979, sv8000, 0,      0,       sv8000,  sv8000, sv8000_state, empty_init, "Bandai", "Super Vision 8000", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS )
