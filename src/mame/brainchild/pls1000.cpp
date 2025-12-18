// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Brainchild PLS-1000 "Personal Learning System"
BIOS (at least) "developed by Logix, Inc"

NVRAM usage needs to be enabled in Brainchild Setup -> Turn Bookmark ON

TODO:
- Trace remaining I/Os;
- Samples inputs thru the same irq 4 used for RTC cfr. PC=649a
- How to enter test mode? There are strings in BIOS at $9690, perhaps wants a specific dev cart?
- DAC1BIT usage is assumed;

===================================================================================================

MC68328PV16V at U1
Atmel AT27C512R ROM at U2
HY62256ALJ-10 CMOS at U3
power switch labeled SW1
two knobs at console bottom sides, VR1/VR2

**************************************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68328.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/mc68328lcd.h"

#include "speaker.h"
#include "screen.h"
#include "softlist_dev.h"

namespace {

class pls1000_state : public driver_device
{
public:
	pls1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdctrl(*this, "lcdctrl")
		, m_screen(*this, "screen")
		, m_cart(*this, "cart")
		, m_nvram(*this, "nvram")
		, m_in_kbd(*this, { "RIGHT", "LEFT" })
	{ }

	void pls1000(machine_config &config);
	//DECLARE_INPUT_CHANGED_MEMBER(button_check);

protected:
	void main_map(address_map &map) ATTR_COLD;

private:
	required_device<mc68328_device> m_maincpu;
	required_device<mc68328_lcd_device> m_lcdctrl;
	required_device<screen_device> m_screen;
	required_device<generic_slot_device> m_cart;
	required_device<nvram_device> m_nvram;
	required_ioport_array<2> m_in_kbd;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	template <unsigned N> void kbd_select_w(int state);
	u8 kbd_r();

	u8 m_kbd_select;
};

u32 pls1000_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_lcdctrl->video_update(bitmap, cliprect);
	return 0;
}

void pls1000_state::main_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("bios", 0);
	map(0x200000, 0x207fff).ram().share("nvram");
	map(0x800000, 0x83ffff).r(m_cart, FUNC(generic_slot_device::read16_rom));
}

// 0 selects both ports
// 1 selects left
// 2 selects right
// any other combination seems unused
u8 pls1000_state::kbd_r()
{
	u8 res = 0;

	for (int i = 0; i < 2; i++)
	{
		if (!BIT(m_kbd_select, i))
			res |= m_in_kbd[i]->read();
	}

	return res;
}

template <unsigned N>
void pls1000_state::kbd_select_w(int state)
{
	m_kbd_select &= ~(1 << N);
	if (state)
		m_kbd_select |= (1 << N);
}

// two columns on console sides:
// A-B-C-D-E then "Explain" in orange text on left side
// page right-page left-scroll up-scroll down-asterisk (*) then "Menu" in orange text on right side

static INPUT_PORTS_START( pls1000 )
	PORT_START("LEFT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("D")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("E")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Explain")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("RIGHT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Page Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Page Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Scroll Up")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Scroll Down")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("*")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Menu")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void pls1000_state::pls1000(machine_config &config)
{
	// TODO: unverified clock
	MC68328(config, m_maincpu, 32768*506); // MC68328PV16VA
	m_maincpu->set_addrmap(AS_PROGRAM, &pls1000_state::main_map);

	m_maincpu->in_port_d<0>().set(FUNC(pls1000_state::kbd_r)).bit(0);
	m_maincpu->in_port_d<1>().set(FUNC(pls1000_state::kbd_r)).bit(1);
	m_maincpu->in_port_d<2>().set(FUNC(pls1000_state::kbd_r)).bit(2);
	m_maincpu->in_port_d<3>().set(FUNC(pls1000_state::kbd_r)).bit(3);
	m_maincpu->in_port_d<4>().set(FUNC(pls1000_state::kbd_r)).bit(4);
	m_maincpu->in_port_d<5>().set(FUNC(pls1000_state::kbd_r)).bit(5);
	m_maincpu->in_port_d<6>().set(FUNC(pls1000_state::kbd_r)).bit(6);
	m_maincpu->in_port_d<7>().set(FUNC(pls1000_state::kbd_r)).bit(7);

//	m_maincpu->out_port_f<0..1> shut off system if both high, at the end of the auto off time
//	m_maincpu->out_port_f<2> low when any key is pressed
	m_maincpu->in_port_f<3>().set_constant(0); // battery low if '1', checked periodically

	m_maincpu->out_port_j<0>().set(FUNC(pls1000_state::kbd_select_w<0>));
	m_maincpu->out_port_j<1>().set(FUNC(pls1000_state::kbd_select_w<1>));
	m_maincpu->out_port_j<2>().set(FUNC(pls1000_state::kbd_select_w<2>));
	m_maincpu->out_port_j<3>().set(FUNC(pls1000_state::kbd_select_w<3>));
	m_maincpu->out_port_j<4>().set(FUNC(pls1000_state::kbd_select_w<4>));
	m_maincpu->out_port_j<5>().set(FUNC(pls1000_state::kbd_select_w<5>));
	m_maincpu->out_port_j<6>().set(FUNC(pls1000_state::kbd_select_w<6>));
	m_maincpu->out_port_j<7>().set(FUNC(pls1000_state::kbd_select_w<7>));

	m_maincpu->out_flm().set(m_lcdctrl, FUNC(mc68328_lcd_device::flm_w));
	m_maincpu->out_llp().set(m_lcdctrl, FUNC(mc68328_lcd_device::llp_w));
	m_maincpu->out_lsclk().set(m_lcdctrl, FUNC(mc68328_lcd_device::lsclk_w));
	m_maincpu->out_ld().set(m_lcdctrl, FUNC(mc68328_lcd_device::ld_w));
	m_maincpu->set_lcd_info_changed(m_lcdctrl, FUNC(mc68328_lcd_device::lcd_info_changed));

	m_maincpu->out_pwm().set("dac", FUNC(dac_bit_interface::write));

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(240, 128);
	m_screen->set_visarea(0, 240 - 1, 0, 128 - 1);
	m_screen->set_screen_update(FUNC(pls1000_state::screen_update));

	MC68328_LCD(config, m_lcdctrl, 0);

	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "bin");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_endian(ENDIANNESS_BIG);
	m_cart->set_interface("pls1000_cart");
	m_cart->set_must_be_loaded(false);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SOFTWARE_LIST(config, "cart_list").set_original("pls1000_cart");

	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



ROM_START( pls1000 )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_LOAD16_WORD( "at27c512r.u2", 0x000000, 0x10000, CRC(77f2beed) SHA1(7118108bd491434934910df072842d46a9ea6223) )
ROM_END


} // anonymous namespace

CONS( 1998, pls1000,  0, 0,  pls1000,  pls1000, pls1000_state, empty_init, "Brainchild", "PLS-1000", 0 ) // 94A model?
// 2001 purple variant known to exist, with MC68EZ328

// PLS-2000 known to exist (with compact flash media), should not belong here
