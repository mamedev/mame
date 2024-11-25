// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Kevin Horton, Sean Riddle
/*******************************************************************************

Entex Select-A-Game Machine, handheld game console.
Technically, the main unit is the peripheral(buttons, display, speaker, power),
and the cartridge holds the MCU(processor, ROM, RAM).

Hardware notes:
- cyan/red VFD Futaba DM-16Z + cyan VFD 9-digit panel Futaba 9-ST-11A 1F
- 1-bit sound, two 7-button control panels attached to each side
- edge connector to cartridge, MCU on cartridge (HD38800 or TMS1670)

A 2nd version of the console was announced, called Table Top Game Machine,
supposed to be backward-compatible, but Entex didn't release it. Their next
console was the Adventure Vision.

MAME external artwork is recommended for the per-game VFD overlays. The artwork
orientation can be rotated in the video options. By default, the "visitor" side
is at the bottom. This is how most of the games are played, Space Invader 2 is
an exception.

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/hmcs40/hmcs40.h"
#include "cpu/tms1000/tms1400.h"
#include "video/pwm.h"
#include "sound/spkrdev.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "sag.lh"


namespace {

class sag_state : public driver_device
{
public:
	sag_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_hmcs40_cpu(*this, "hmcs40_cpu"),
		m_tms1k_cpu(*this, "tms1k_cpu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void sag(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	optional_device<hmcs40_cpu_device> m_hmcs40_cpu;
	optional_device<tms1k_base_device> m_tms1k_cpu;
	required_device<pwm_display_device> m_display;
	required_device<speaker_sound_device> m_speaker;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<6> m_inputs;

	u16 m_grid = 0;
	u16 m_plate = 0;

	void update_display();
	u8 input_r();
	void speaker_w(int state);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void hmcs40_write_r(offs_t offset, u8 data);
	void hmcs40_write_d(u16 data);
	u16 hmcs40_read_d();

	void tms1k_write_r(u32 data);
	void tms1k_write_o(u16 data);
	u8 tms1k_read_k();
};

void sag_state::machine_start()
{
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}



/*******************************************************************************
    Cartridge Init
*******************************************************************************/

DEVICE_IMAGE_LOAD_MEMBER(sag_state::cart_load)
{
	u32 size = m_cart->common_get_size("rom");

	if (size != 0x1000 && size != 0x1100 && size != 0x2000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid ROM file size (must be 4096, 4352 or 8192 bytes)");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	// detect MCU on file size
	if (size == 0x1000)
	{
		// TMS1670 MCU
		if (!image.loaded_through_softlist())
			return std::make_pair(image_error::UNSUPPORTED, "Can only load TMS1670 type through software list");

		memcpy(memregion("tms1k_cpu")->base(), m_cart->get_rom_base(), size);
		m_tms1k_cpu->set_clock(375000); // approximation - RC osc. R=47K, C=47pF

		// init PLAs
		size = image.get_software_region_length("rom:mpla");
		if (size != 867)
			return std::make_pair(image_error::BADSOFTWARE, "Invalid MPLA data area size (must be 867 bytes)");

		memcpy(memregion("tms1k_cpu:mpla")->base(), image.get_software_region("rom:mpla"), size);

		size = image.get_software_region_length("rom:opla");
		if (size != 557)
			return std::make_pair(image_error::BADSOFTWARE, "Invalid OPLA data area size (must be 557 bytes)");

		memcpy(memregion("tms1k_cpu:opla")->base(), image.get_software_region("rom:opla"), size);

		subdevice<pla_device>("tms1k_cpu:mpla")->reinit();
		subdevice<pla_device>("tms1k_cpu:opla")->reinit();
	}
	else
	{
		// HD38800 MCU
		u8 *dest = memregion("hmcs40_cpu")->base();
		memcpy(dest, m_cart->get_rom_base(), size);

		// copy patterns
		if (size == 0x1100)
			memmove(dest + 0x1e80, dest + 0x1000, 0x100);

		m_hmcs40_cpu->set_clock(450000); // from main PCB
	}

	return std::make_pair(std::error_condition(), std::string());
}



/*******************************************************************************
    I/O
*******************************************************************************/

// main unit

void sag_state::update_display()
{
	// grid 0-7 are the 'pixels'
	m_display->matrix_partial(0, 8, m_grid, m_plate);

	// grid 8-13 are 7segs
	u8 seg = bitswap<7>(m_plate,4,5,6,7,8,9,10);
	m_display->matrix_partial(8, 6, m_grid >> 8, seg);
}

u8 sag_state::input_r()
{
	u8 data = 0;

	// grid 1-6 double as input mux
	for (int i = 0; i < 6; i++)
		if (BIT(m_grid, i + 1))
			data |= m_inputs[i]->read();

	return data;
}

void sag_state::speaker_w(int state)
{
	m_speaker->level_w(state);
}


// cartridge type 1: HD38800

void sag_state::hmcs40_write_r(offs_t offset, u8 data)
{
	// R0x-R3x: vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void sag_state::hmcs40_write_d(u16 data)
{
	// D0: speaker out
	speaker_w(data & 1);

	// D1-D12: vfd grid (10 and 11 unused)
	m_grid = bitswap<14>(data,9,10,0,0,11,12,1,2,3,4,5,6,7,8) & 0x33ff;
	update_display();
}

u16 sag_state::hmcs40_read_d()
{
	// D13-D15: multiplexed inputs
	return input_r() << 13;
}


// cartridge type 2: TMS1670

void sag_state::tms1k_write_r(u32 data)
{
	// R0: speaker out
	speaker_w(data & 1);

	// R1-R12: vfd grid (0 and 7 unused)
	// R13,R14: vfd plate 3,2
	m_grid = bitswap<14>(data,7,8,9,10,11,12,0,1,2,3,4,5,6,0) & 0x3f7e;
	m_plate = (m_plate & 0xff0) | bitswap<2>(data,13,14) << 2;
	update_display();
}

void sag_state::tms1k_write_o(u16 data)
{
	// O0-O7: vfd plate 4-11
	m_plate = (m_plate & 0xf) | data << 4;
	update_display();
}

u8 sag_state::tms1k_read_k()
{
	// K1-K4: multiplexed inputs
	return input_r();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( sag ) // P1 = Visitor (left side), P2 = Home (right side)
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Button 6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button 6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button 7")

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Button 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button 5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Button 7")

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Button 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Button 2")
	PORT_BIT( 0x04, 0x04, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, EQUALS, 0x00) // demo

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Button 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY PORT_NAME("P1 Button 1")
	PORT_CONFNAME( 0x04, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x04, "2" )

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Button 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Button 4")
	PORT_BIT( 0x04, 0x04, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, EQUALS, 0x01) // 1 player

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_NAME("P1 Button 3")
	PORT_CONFNAME( 0x04, 0x04, "Game" )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("FAKE") // shared IN.2/IN.4
	PORT_CONFNAME( 0x03, 0x01, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "Demo" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sag_state::sag(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_hmcs40_cpu, 0);
	m_hmcs40_cpu->write_r<0>().set(FUNC(sag_state::hmcs40_write_r));
	m_hmcs40_cpu->write_r<1>().set(FUNC(sag_state::hmcs40_write_r));
	m_hmcs40_cpu->write_r<2>().set(FUNC(sag_state::hmcs40_write_r));
	m_hmcs40_cpu->write_r<3>().set(FUNC(sag_state::hmcs40_write_r));
	m_hmcs40_cpu->write_d().set(FUNC(sag_state::hmcs40_write_d));
	m_hmcs40_cpu->read_d().set(FUNC(sag_state::hmcs40_read_d));

	TMS1670(config, m_tms1k_cpu, 0);
	m_tms1k_cpu->read_k().set(FUNC(sag_state::tms1k_read_k));
	m_tms1k_cpu->write_r().set(FUNC(sag_state::tms1k_write_r));
	m_tms1k_cpu->write_o().set(FUNC(sag_state::tms1k_write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8+6, 14);
	m_display->set_segmask(0x3f00, 0x7f);
	config.set_default_layout(layout_sag);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "sag_cart");
	m_cart->set_must_be_loaded(true);
	m_cart->set_device_load(FUNC(sag_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("entex_sag");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sag )
	// nothing here yet, ROM is on the cartridge
	ROM_REGION( 0x2000, "hmcs40_cpu", ROMREGION_ERASE00 )
	ROM_REGION( 0x1000, "tms1k_cpu", ROMREGION_ERASE00 )
	ROM_REGION( 867, "tms1k_cpu:mpla", ROMREGION_ERASE00 )
	ROM_REGION( 557, "tms1k_cpu:opla", ROMREGION_ERASE00 )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1981, sag,  0,      0,      sag,     sag,   sag_state, empty_init, "Entex", "Select-A-Game Machine", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
