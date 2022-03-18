// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************

PINBALL
Valley Spectra IV

Rotating game, like Midway's "Rotation VIII".

2012-09-28 System working [Robbbert]

Schematic and PinMAME used as references.
Code for the sn76477 sound was derived from PinMAME.

There is a bug - if you score 1000 and had less than 100, the hundreds digit will
be blank. It will of course fix itself during the course of the game.

Setting up - if you do not set up the game, each player will get 255 balls.
Turn test switch to Setup. Press 1 to advance to next set. Press 5 to adjust the
set. Use the manual for a description of each set. After setting up set 16, do not
press 1, instead turn the dipswitch to Play. Exit (to save nvram), and restart.
Now the game is ready. Very quick guide to a reasonable setup:
06 - 30000 (1st award score)
07 - 50000 (2nd award score)
08 - 70000 (3rd award score)
09 - 90000 (high score)
11 - 1 (1 coin 1 credit)
13 - 3 (3 balls)
15 - 1 (award is a free game)
16 - 1 (match enabled)

Status:
- Working

ToDo:
- Nothing

*******************************************************************************************/


#include "emu.h"
#include "machine/genpin.h"

#include "cpu/m6502/m6502.h"
#include "machine/6532riot.h"
#include "machine/timer.h"
#include "sound/sn76477.h"
#include "speaker.h"

#include "spectra.lh"

namespace {

class spectra_state : public genpin_class
{
public:
	spectra_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_snsnd(*this, "snsnd")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_p_ram(*this, "nvram")
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void spectra(machine_config &config);

private:
	u8 porta_r();
	u8 portb_r();
	void porta_w(u8 data);
	void portb_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(nmitimer);
	TIMER_DEVICE_CALLBACK_MEMBER(outtimer);
	void spectra_map(address_map &map);

	u8 m_porta = 0U;
	u8 m_portb = 0U;
	u8 m_t_c = 0U;
	u8 m_out_offs = 0U;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	required_device<cpu_device> m_maincpu;
	required_device<sn76477_device> m_snsnd;
	required_ioport_array<4> m_io_keyboard;
	required_shared_ptr<u8> m_p_ram;
	output_finder<40> m_digits;
	output_finder<64> m_io_outputs;  // 16 solenoids + 48 lamps
};


void spectra_state::spectra_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xfff);
	map(0x0000, 0x00ff).ram().share("nvram"); // battery backed, 2x 5101L
	map(0x0100, 0x017f).ram(); // RIOT RAM
	map(0x0180, 0x019f).rw("riot", FUNC(riot6532_device::read), FUNC(riot6532_device::write));
	map(0x0400, 0x0fff).rom();
}

static INPUT_PORTS_START( spectra )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("L Outlane 1") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CR Hole") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TR Hole") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("R Bumper") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C Bumper") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("L Bumper") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("R Sling") PORT_CODE(KEYCODE_G)
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("L Sling") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("R Outlane") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("R Rollover") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("L Rollover") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("L Inside Target") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C Inside Target") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("R Inside Target") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CL Target") PORT_CODE(KEYCODE_N)
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CL Button") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TL Button") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TL Target") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("L Outlane 2") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("L Outlane 3") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("R Pentagon") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("L Pentagon") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("TL Area") PORT_CODE(KEYCODE_V)
	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("L Area") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("R Area") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Inlane Button") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_DIPNAME( 0x60, 0x60, "Test Switch" ) // 3-position slide switch
	PORT_DIPSETTING(    0x60, "Play" )
	PORT_DIPSETTING(    0x20, "Setup" )
	PORT_DIPSETTING(    0x40, "Test" )
INPUT_PORTS_END

u8 spectra_state::porta_r()
{
	u8 row = (m_porta & 0x18) >> 3;
	u8 key = m_io_keyboard[row]->read();
	u8 ret = ((BIT(key, m_porta & 7)) ? 0x40 : 0) | (m_porta & 0xbf);

	if (ret == 0x1b && m_p_ram[0x7b] < 0x1E)
		m_samples->start(3, 8); // coin

	return ret;
}

u8 spectra_state::portb_r()
{
	if (m_p_ram[0xf0] != 1)
		return 0x5a; // factory reset if first time
	else
		return m_portb;
}

void spectra_state::porta_w(u8 data)
{
	m_porta = data;
}

// sound port
void spectra_state::portb_w(u8 data)
{
	m_portb = data;
	float vco = 5.0;
	if (BIT(data, 0)) vco -= 0.3125f;
	if (BIT(data, 1)) vco -= 0.625f;
	if (BIT(data, 2)) vco -= 1.25f;
	if (BIT(data, 3)) vco -= 2.5f;
	m_snsnd->vco_voltage_w(5.3125f - vco);
	m_snsnd->enable_w(!BIT(data, 4)); // strobe: toggles enable
	m_snsnd->envelope_1_w(!BIT(data, 5)); //decay: toggles envelope
	m_snsnd->vco_w(BIT(data, 6)); // "phaser" sound: VCO toggled
	m_snsnd->mixer_b_w(BIT(data, 7)); // "pulse" sound: pins 25 & 27 changed
	m_snsnd->mixer_c_w(BIT(data, 7)); // "pulse" sound: pins 25 & 27 changed
}


TIMER_DEVICE_CALLBACK_MEMBER( spectra_state::nmitimer)
{
	if (m_t_c > 0x10)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	else
		m_t_c++;
}

// 00-27 displays
// 40-6F lamps
// 70-7F solenoids - no knocker
TIMER_DEVICE_CALLBACK_MEMBER( spectra_state::outtimer)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x5c, 0x63, 0x01, 0x40, 0x08, 0 }; // 74C912
	m_out_offs++;

	if (m_out_offs < 0x28)
	{
		u8 data = m_p_ram[m_out_offs];
		u8 segments = patterns[data&15] | (BIT(data, 4) ? 0x80 : 0);
		m_digits[m_out_offs] = segments;
	}
	else
	if (m_out_offs < 0x6f)
		m_out_offs = 0x6f;
	else
	if (m_out_offs < 0x74)
	{
		if (m_p_ram[m_out_offs])
			m_samples->start(0, 5); // holes
	}
	else
	if (m_out_offs < 0x77)
	{
		if (m_p_ram[m_out_offs])
			m_samples->start(1, 0); // bumpers
	}
	else
	if (m_out_offs < 0x79)
	{
		if (m_p_ram[m_out_offs])
			m_samples->start(2, 7); // slings
	}
	else
		m_out_offs = 0xff;

	// Signals to external solenoids
	for (u8 i = 0; i < 16; i++)
		m_io_outputs[i] = m_p_ram[i+0x70] ? 1 : 0;
	// Signals to external lamps
	for (u8 i = 0; i < 48; i++)
		m_io_outputs[i+16] = m_p_ram[i+0x40] ? 1 : 0;
}

void spectra_state::machine_start()
{
	genpin_class::machine_start();
	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_porta));
	save_item(NAME(m_portb));
	save_item(NAME(m_t_c));
	save_item(NAME(m_out_offs));
}

void spectra_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_t_c = 0;
}


void spectra_state::spectra(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(3'579'545)/4);  // actually a M6503
	m_maincpu->set_addrmap(AS_PROGRAM, &spectra_state::spectra_map);

	riot6532_device &riot(RIOT6532(config, "riot", XTAL(3'579'545)/4));
	riot.in_pa_callback().set(FUNC(spectra_state::porta_r));
	riot.out_pa_callback().set(FUNC(spectra_state::porta_w));
	riot.in_pb_callback().set(FUNC(spectra_state::portb_r));
	riot.out_pb_callback().set(FUNC(spectra_state::portb_w));
	riot.irq_callback().set_inputline("maincpu", M6502_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	TIMER(config, "nmitimer").configure_periodic(FUNC(spectra_state::nmitimer), attotime::from_hz(120));
	TIMER(config, "outtimer").configure_periodic(FUNC(spectra_state::outtimer), attotime::from_hz(1200));

	/* Video */
	config.set_default_layout(layout_spectra);

	/* Sound */
	genpin_audio(config);

	SPEAKER(config, "mono").front_center();
	SN76477(config, m_snsnd);
	m_snsnd->set_noise_params(RES_M(1000), RES_M(1000), CAP_N(0));
	m_snsnd->set_decay_res(RES_K(470));
	m_snsnd->set_attack_params(CAP_N(1), RES_K(22));
	m_snsnd->set_amp_res(RES_K(100));
	m_snsnd->set_feedback_res(RES_K(52));
	m_snsnd->set_vco_params(5.0, CAP_U(0.01), RES_K(390));
	m_snsnd->set_pitch_voltage(0.0);
	m_snsnd->set_slf_params(CAP_U(0.1), RES_M(1));
	m_snsnd->set_oneshot_params(CAP_U(0.47), RES_K(470));
	m_snsnd->set_vco_mode(0);
	m_snsnd->set_mixer_params(0, 0, 0);
	m_snsnd->set_envelope_params(0, 0);
	m_snsnd->set_enable(1);
	m_snsnd->add_route(ALL_OUTPUTS, "mono", 0.30);
}

/*--------------------------------
/ Spectra IV
/-------------------------------*/
ROM_START(spectra)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spect_u5.dat", 0x0400, 0x0400, CRC(49e0759f) SHA1(c3badc90ff834cbc92d8c519780069310c2b1507))
	ROM_LOAD("spect_u4.dat", 0x0800, 0x0400, CRC(15e53712) SHA1(e03049178569313cb89cfe0f09043c21d05b1988))
	ROM_LOAD("spect_u3.dat", 0x0c00, 0x0400, CRC(9ca7510f) SHA1(a87849f16903836158063d593bb4a2e90c7473c8))
ROM_END

} // anonymous namespace

GAME(1979,  spectra,  0,  spectra,  spectra, spectra_state, empty_init, ROT0, "Valley", "Spectra IV", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
