// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Gakken Compact Vision TV Boy (TVボーイ)

Hardware notes:
- MC6801U4 or HD6801V0P @ 3.57MHz
- MC6847P, MC1372P
- 2KB RAM (HM6116P-4)
- 1-bit sound with volume decay
- cartridge slot

The MCU is inside the cartridge, not the console, in theory it could have any MCU.
The console itself has the video hardware and controls.

TODO:
- change cartridge to slot device? there are free homebrew games by Infuto that
  currently won't work, since cartridge PCB has an MC6803 + 32KB ROM + 8KB RAM

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6800/m6801.h"
#include "sound/dac.h"
#include "sound/flt_vol.h"
#include "video/mc6847.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {

class ctvboy_state : public driver_device
{
public:
	ctvboy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vram(*this, "vram"),
		m_mc6847(*this, "mc6847"),
		m_screen(*this, "screen"),
		m_dac(*this, "dac"),
		m_volume(*this, "volume"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void ctvboy(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<m6801u4_cpu_device> m_maincpu;
	required_shared_ptr<u8> m_vram;
	required_device<mc6847_ntsc_device> m_mc6847;
	required_device<screen_device> m_screen;
	required_device<dac_bit_interface> m_dac;
	required_device<filter_volume_device> m_volume;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<2> m_inputs;

	emu_timer *m_decaytimer;
	bool m_speaker_on = false;
	u8 m_inp_mux = 0;

	void speaker_decay_sim(s32 param);

	void main_map(address_map &map) ATTR_COLD;
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void p1_w(u8 data);
	u8 p1_r();
	u8 p2_r();

	void mc6847_w(u8 data);
	u8 mc6847_vram_r(offs_t offset);
	void vblank_irq(int state);
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void ctvboy_state::machine_start()
{
	m_decaytimer = timer_alloc(FUNC(ctvboy_state::speaker_decay_sim), this);

	// register for savestates
	save_item(NAME(m_speaker_on));
	save_item(NAME(m_inp_mux));
}

DEVICE_IMAGE_LOAD_MEMBER(ctvboy_state::cart_load)
{
	u32 size = m_cart->common_get_size("rom");

	if (size != 0x1000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid ROM file size (must be 4096 bytes)");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_BIG);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");
	memcpy(memregion("maincpu")->base(), m_cart->get_rom_base(), size);

	return std::make_pair(std::error_condition(), std::string());
}



/*******************************************************************************
    I/O
*******************************************************************************/

void ctvboy_state::speaker_decay_sim(s32 param)
{
	// volume decays when speaker is off (divisor and timer period determine duration)
	m_volume->set_gain(m_volume->gain() / 1.01);
	m_decaytimer->adjust(attotime::from_msec(1));
}

void ctvboy_state::p1_w(u8 data)
{
	// P10,P11: input mux
	m_inp_mux = ~data & 3;

	// P15: speaker on
	bool speaker_on = bool(data & 0x20);
	if (speaker_on)
	{
		m_volume->set_gain(1.0);
		m_decaytimer->adjust(attotime::never);
	}
	else if (m_speaker_on)
		m_decaytimer->adjust(attotime::zero);

	m_speaker_on = speaker_on;

	// P16: speaker out
	m_dac->write(BIT(data, 6));
}

u8 ctvboy_state::p1_r()
{
	// P17: MC6847 HS
	return m_mc6847->hs_r() ? 0x80 : 0;
}

u8 ctvboy_state::p2_r()
{
	u8 data = 0;

	// P21-P24: multiplexed inputs
	for (int i = 0; i < 2; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data;
}

void ctvboy_state::mc6847_w(u8 data)
{
	// write to MC6847 pins
	m_mc6847->gm1_w(BIT(data, 0));
	m_mc6847->gm0_w(BIT(data, 1));
	m_mc6847->intext_w(BIT(data, 2));
	m_mc6847->as_w(BIT(data, 3));
	m_mc6847->ag_w(BIT(data, 4));
	m_mc6847->css_w(BIT(data, 5));
}

u8 ctvboy_state::mc6847_vram_r(offs_t offset)
{
	u8 data = m_vram[offset & 0x7ff];
	m_mc6847->inv_w(BIT(data, 6));

	return data;
}

void ctvboy_state::vblank_irq(int state)
{
	if (!state)
		m_maincpu->pulse_input_line(M6801_IRQ1_LINE, attotime::from_usec(15));
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void ctvboy_state::main_map(address_map &map)
{
	map(0x1000, 0x17ff).ram().share(m_vram);
	map(0x2000, 0x2000).w(FUNC(ctvboy_state::mc6847_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( ctvboy )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) // A
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) // START / B
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SELECT) PORT_NAME("Pause")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void ctvboy_state::ctvboy(machine_config &config)
{
	// basic machine hardware
	M6801U4(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ctvboy_state::main_map);
	m_maincpu->out_p1_cb().set(FUNC(ctvboy_state::p1_w));
	m_maincpu->in_p1_cb().set(FUNC(ctvboy_state::p1_r));
	m_maincpu->in_p2_cb().set(FUNC(ctvboy_state::p2_r));

	// video hardware
	MC6847_NTSC(config, m_mc6847, 3.579545_MHz_XTAL);
	m_mc6847->input_callback().set(FUNC(ctvboy_state::mc6847_vram_r));
	m_mc6847->fsync_wr_callback().set(FUNC(ctvboy_state::vblank_irq));
	m_mc6847->set_screen(m_screen);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	FILTER_VOLUME(config, m_volume).set_gain(0.0).add_route(ALL_OUTPUTS, "speaker", 1.0);
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "volume", 0.25);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "ctvboy_cart");
	m_cart->set_must_be_loaded(true);
	m_cart->set_device_load(FUNC(ctvboy_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("ctvboy");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ctvboy )
	// nothing here yet, ROM is on the cartridge
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1983, ctvboy, 0,      0,      ctvboy,  ctvboy, ctvboy_state, empty_init, "Gakken", "Compact Vision TV Boy", MACHINE_SUPPORTS_SAVE )
