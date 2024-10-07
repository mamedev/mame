// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
// thanks-to:Dan Boris, Kevin Horton, Sean Riddle
/*******************************************************************************

Milton Bradley Microvision, handheld game console

Hardware notes:
- SCUS0488(Hughes HLCD0488) LCD, 16*16 screen
- piezo, 12 buttons under membrane + analog paddle(MB calls it the Control Knob)
- no CPU on console, it is on the cartridge

The LCD motion blur is normal. To decrease it, simply increase the screen contrast
in MAME, this makes it similar to repro LCD replacements. It's also advised to
disable screen filtering, eg. with -prescale or -nofilter.

12 games were released, all of them have a TMS1100 MCU. The first couple of
games had an Intel 8021 MCU at first, but Milton Bradley switched to TMS1100.
See the softwarelist XML for details.

Each game had a screen- and button overlay attached to it, MAME external artwork
is recommended.

TODO:
- dump/add remaining 8021 cartridges, which games have 8021 versions? An online
  FAQ mentions at least Block Buster, Connect Four, Bowling.

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/tms1000/tms1100.h"
#include "sound/dac.h"
#include "video/hlcd0488.h"
#include "video/pwm.h"

#include "emupal.h"
#include "softlist_dev.h"
#include "screen.h"
#include "speaker.h"

#include "microvision.lh"


namespace {

class microvision_state : public driver_device
{
public:
	microvision_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tms1100( *this, "tms1100_cpu" ),
		m_i8021( *this, "i8021_cpu" ),
		m_lcd(*this, "lcd"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac( *this, "dac" ),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "COL%u", 0),
		m_paddle(*this, "PADDLE"),
		m_conf(*this, "CONF")
	{ }

	void microvision(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(conf_changed) { apply_settings(); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { apply_settings(); }
	virtual void device_post_load() override { apply_settings(); }

private:
	optional_device<tms1100_cpu_device> m_tms1100;
	optional_device<i8021_device> m_i8021;
	required_device<hlcd0488_device> m_lcd;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_byte_interface> m_dac;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<3> m_inputs;
	required_ioport m_paddle;
	required_ioport m_conf;

	u32 tms1100_micro_pla(offs_t offset);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	void apply_settings(void);

	u8 m_pla_auto = 0;
	u16 m_butmask_auto = 0;
	u16 m_button_mask = 0;
	bool m_paddle_auto = false;
	bool m_paddle_on = false;
	attotime m_paddle_delay;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void lcd_output_w(offs_t offset, u16 data) { m_lcd_pwm->matrix(offset, data); }

	// TMS1100 interface
	u8 tms1100_k_r();
	void tms1100_o_w(u16 data);
	void tms1100_r_w(u32 data);

	u32 m_r = 0;

	// Intel 8021 interface
	u8 i8021_p0_r();
	void i8021_p0_w(u8 data);
	void i8021_p1_w(u8 data);
	void i8021_p2_w(u8 data);
	int i8021_t1_r();

	u8 m_p0 = 0xff;
	u8 m_p2 = 0xff;
};

void microvision_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_paddle_delay));
	save_item(NAME(m_r));
	save_item(NAME(m_p0));
	save_item(NAME(m_p2));

	// don't save: m_pla_auto, m_butmask_auto, m_paddle_auto,
	// m_button_mask, m_paddle_on
}



/*******************************************************************************
    Cartridge Init
*******************************************************************************/

static const u16 tms1100_output_pla[2][0x20] =
{
	// default TMS1100 O output PLA
	// verified for: blckbstr, pinball
	{
		0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
		0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f,
		0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
		0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f
	},

	// reversed bit order
	// verified for: bowling, vegasslt
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	}
};

u32 microvision_state::tms1100_micro_pla(offs_t offset)
{
	// default TMS1100 microinstructions PLA - this should work for all games
	// verified for: blckbstr, bowling, pinball, vegasslt

	// TCY, YNEC, TCMIY, AxAAC
	static const u16 micro1[4] = { 0x0108, 0x9080, 0x8068, 0x0136 };

	// 0x00, 0x20, 0x30
	static const u16 micro2[0x30] =
	{
		0x1402, 0x0c30, 0xd002, 0x2404, 0x8019, 0x8038, 0x0416, 0x0415,
		0x0104, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1100, 0x0000,

		0x000a, 0x0404, 0x0408, 0x8004, 0xa019, 0xa038, 0x2004, 0x2000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x1580, 0x1580, 0x1580, 0x1580, 0x0c34, 0x0834, 0x0434, 0x1400
	};

	static const int micro2h[4] = { 0x00, -1, 0x10, 0x20 };

	u16 data = 0;

	if (offset >= 0x40 && offset < 0x80)
	{
		data = micro1[offset >> 4 & 3];
		if (offset == 0x7f) data ^= 2; // CLA
	}
	else if (offset < 0x40 && (offset & 0xf0) != 0x10)
		data = micro2[micro2h[offset >> 4] | (offset & 0xf)];

	return (data == 0) ? 0x8fa3 : data;
}

DEVICE_IMAGE_LOAD_MEMBER(microvision_state::cart_load)
{
	u32 size = m_cart->common_get_size("rom");

	if (size != 0x400 && size != 0x800)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid ROM file size (must be 1K or 2K)");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	// set default settings
	u32 clock = (size == 0x400) ? 3000000 : 500000;
	m_pla_auto = 0;
	m_butmask_auto = 0xfff;
	m_paddle_auto = false;

	if (image.loaded_through_softlist())
	{
		const char *cclock = image.get_feature("clock");
		u32 sclock = cclock ? strtoul(cclock, nullptr, 0) : 0;
		if (sclock != 0)
			clock = sclock;

		const char *butmask = image.get_feature("butmask");
		m_butmask_auto = butmask ? strtoul(butmask, nullptr, 0) : 0;
		m_butmask_auto = ~m_butmask_auto & 0xfff;

		const char *pla = image.get_feature("pla");
		m_pla_auto = pla && strtoul(pla, nullptr, 0) ? 1 : 0;

		const char *paddle = image.get_feature("paddle");
		m_paddle_auto = paddle && strtoul(paddle, nullptr, 0);
	}

	// detect MCU on file size
	if (size == 0x400)
	{
		// 8021 MCU
		memcpy(memregion("i8021_cpu")->base(), m_cart->get_rom_base(), size);
		m_i8021->set_clock(clock);
	}
	else
	{
		// TMS1100 MCU
		memcpy(memregion("tms1100_cpu")->base(), m_cart->get_rom_base(), size);
		m_tms1100->set_clock(clock);
	}

	return std::make_pair(std::error_condition(), std::string());
}

void microvision_state::apply_settings()
{
	u8 conf = m_conf->read();

	// cartridge physically restricts button panel (some glitches otherwise)
	m_button_mask = (conf & 1) ? m_butmask_auto : 0xfff;

	u8 pla = ((conf & 0x18) == 0x10) ? m_pla_auto : (conf >> 3 & 1);
	m_tms1100->set_output_pla(tms1100_output_pla[pla]);

	m_paddle_on = ((conf & 6) == 4) ? m_paddle_auto : bool(conf & 2);
}



/*******************************************************************************
    Video
*******************************************************************************/

uint32_t microvision_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			// simulate LCD persistence
			int p = m_lcd_pwm->read_element_bri(y ^ 15, x ^ 15) * 8500;
			p = (p > 255) ? 0 : p ^ 255;

			if (cliprect.contains(x, y))
				bitmap.pix(y, x) = p << 16 | p << 8 | p;
		}
	}

	return 0;
}



/*******************************************************************************
    I/O
*******************************************************************************/

// TMS1100 interface

u8 microvision_state::tms1100_k_r()
{
	u8 data = 0;

	// multiplexed inputs
	for (int i = 0; i < 3; i++)
		if (BIT(m_r, i + 8))
			data |= m_inputs[i]->read() & (m_button_mask >> (i * 4) & 0xf);

	// K8: paddle capacitor
	if (m_paddle_on && m_paddle_delay < machine().time())
		data |= BIT(m_r, 2) << 3;

	return data;
}

void microvision_state::tms1100_o_w(u16 data)
{
	// O0-O3: LCD data
	m_lcd->data_w(data & 0xf);
}

void microvision_state::tms1100_r_w(u32 data)
{
	// R2: charge paddle capacitor when high
	if (~m_r & data & 4 && m_paddle_on)
	{
		// note that the games don't use the whole range, so there's a deadzone around the edges
		const float step = (2000 - 500) / 255.0f; // approximation
		m_paddle_delay = machine().time() + attotime::from_usec(500 + m_paddle->read() * step);
	}

	// R0: speaker lead 2
	// R1: speaker lead 1 (GND on some carts)
	m_dac->write((BIT(data, 0) << 1) | BIT(data, 1));

	// R6: LCD latch pulse
	// R7: LCD data clock
	m_lcd->latch_pulse_w(BIT(data, 6));
	m_lcd->data_clk_w(BIT(data, 7));

	// R8-R10: input mux
	m_r = data;
}


// Intel 8021 interface

u8 microvision_state::i8021_p0_r()
{
	u8 in[3];
	for (int i = 0; i < 3; i++)
		in[i] = m_inputs[i]->read() & (m_button_mask >> (i * 4) & 0xf);

	u8 data = 0;

	// P00-P02: multiplexed inputs from P04-P07
	for (int i = 0; i < 3; i++)
		if (~(m_p0 >> 4) & in[i])
			data |= 1 << i;

	// P04-P07: multiplexed inputs from P00-P02
	for (int i = 0; i < 3; i++)
		if (BIT(~m_p0, i))
			data |= in[i] << 4;

	return ~data & m_p0;
}

void microvision_state::i8021_p0_w(u8 data)
{
	// P0-P2, P4-P7: input mux
	m_p0 = data;
}

void microvision_state::i8021_p1_w(u8 data)
{
	// P14-P17: lcd data
	m_lcd->data_w(data >> 4 & 0xf);

	// P10: lcd latch pulse
	// P11: lcd data clk
	m_lcd->latch_pulse_w(BIT(data, 0));
	m_lcd->data_clk_w(BIT(data, 1));
}

void microvision_state::i8021_p2_w(u8 data)
{
	// P20: speaker lead 1
	// P21: speaker lead 2
	m_dac->write(data & 3);

	// P22,P23: charge paddle capacitor when low
	if (m_p2 & 0xc && (data & 0xc) == 0 && m_paddle_on)
	{
		const float step = (1000 - 10) / 255.0f; // approximation
		m_paddle_delay = machine().time() + attotime::from_usec(10 + (m_paddle->read() ^ 0xff) * step);
	}

	m_p2 = data;
}

int microvision_state::i8021_t1_r()
{
	// T1: paddle capacitor (active low)
	if (m_paddle_on)
		return (m_p2 & 0xc || m_paddle_delay > machine().time()) ? 1 : 0;
	else
		return 1;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( microvision )
	PORT_START("COL0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CODE(KEYCODE_3)

	PORT_START("COL1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CODE(KEYCODE_2)

	PORT_START("COL2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_1)

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CENTERDELTA(0)

	PORT_START("CONF")
	PORT_CONFNAME( 0x01, 0x01, "Restrict Buttons" ) PORT_CHANGED_MEMBER(DEVICE_SELF, microvision_state, conf_changed, 0)
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x01, "Auto" )
	PORT_CONFNAME( 0x06, 0x04, "Paddle Hardware" ) PORT_CHANGED_MEMBER(DEVICE_SELF, microvision_state, conf_changed, 0)
	PORT_CONFSETTING(    0x00, DEF_STR( No ) ) // no circuitry on cartridge PCB
	PORT_CONFSETTING(    0x02, DEF_STR( Yes ) )
	PORT_CONFSETTING(    0x04, "Auto" )
	PORT_CONFNAME( 0x18, 0x10, "TMS1100 PLA Type" ) PORT_CHANGED_MEMBER(DEVICE_SELF, microvision_state, conf_changed, 0)
	PORT_CONFSETTING(    0x00, "0" )
	PORT_CONFSETTING(    0x08, "1" )
	PORT_CONFSETTING(    0x10, "Auto" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void microvision_state::microvision(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_tms1100, 0);
	m_tms1100->set_output_pla(tms1100_output_pla[0]);
	m_tms1100->set_decode_micro().set(FUNC(microvision_state::tms1100_micro_pla));
	m_tms1100->read_k().set(FUNC(microvision_state::tms1100_k_r));
	m_tms1100->write_o().set(FUNC(microvision_state::tms1100_o_w));
	m_tms1100->write_r().set(FUNC(microvision_state::tms1100_r_w));

	I8021(config, m_i8021, 0);
	m_i8021->bus_in_cb().set(FUNC(microvision_state::i8021_p0_r));
	m_i8021->bus_out_cb().set(FUNC(microvision_state::i8021_p0_w));
	m_i8021->p1_out_cb().set(FUNC(microvision_state::i8021_p1_w));
	m_i8021->p2_out_cb().set(FUNC(microvision_state::i8021_p2_w));
	m_i8021->t1_in_cb().set(FUNC(microvision_state::i8021_t1_r));

	// video hardware
	HLCD0488(config, m_lcd);
	m_lcd->write_cols().set(FUNC(microvision_state::lcd_output_w));

	PWM_DISPLAY(config, m_lcd_pwm).set_size(16, 16);
	m_lcd_pwm->set_interpolation(0.25);
	config.set_default_layout(layout_microvision);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(0);
	screen.set_screen_update(FUNC(microvision_state::screen_update));
	screen.set_size(16, 16);
	screen.set_visarea_full();

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_ONES_COMPLEMENT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "microvision_cart");
	m_cart->set_must_be_loaded(true);
	m_cart->set_device_load(FUNC(microvision_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("microvision");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( microvsn )
	// nothing here yet, ROM is on the cartridge
	ROM_REGION( 0x400, "i8021_cpu", ROMREGION_ERASE00 )
	ROM_REGION( 0x800, "tms1100_cpu", ROMREGION_ERASE00 )
	ROM_REGION( 867, "tms1100_cpu:mpla", ROMREGION_ERASE00 )
	ROM_REGION( 365, "tms1100_cpu:opla", ROMREGION_ERASE00 )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE      INPUT        CLASS              INIT        COMPANY, FULLNAME, FLAGS
SYST( 1979, microvsn, 0,      0,      microvision, microvision, microvision_state, empty_init, "Milton Bradley", "Microvision", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
