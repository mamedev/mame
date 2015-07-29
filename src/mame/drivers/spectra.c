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

ToDo:
- Get good dump of u4 rom.


*******************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6502/m6502.h"
#include "machine/6532riot.h"
#include "sound/sn76477.h"
#include "spectra.lh"


class spectra_state : public genpin_class
{
public:
	spectra_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_snsnd(*this, "snsnd")
		, m_switch(*this, "SWITCH")
		, m_p_ram(*this, "nvram")
	{ }

	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portb_w);
	TIMER_DEVICE_CALLBACK_MEMBER(nmitimer);
	TIMER_DEVICE_CALLBACK_MEMBER(outtimer);
private:
	UINT8 m_porta;
	UINT8 m_portb;
	UINT8 m_t_c;
	UINT8 m_out_offs;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<sn76477_device> m_snsnd;
	required_ioport_array<4> m_switch;
	required_shared_ptr<UINT8> m_p_ram;
};


static ADDRESS_MAP_START( spectra_map, AS_PROGRAM, 8, spectra_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_SHARE("nvram") // battery backed, 2x 5101L
	AM_RANGE(0x0100, 0x017f) AM_RAM // RIOT RAM
	AM_RANGE(0x0180, 0x019f) AM_DEVREADWRITE("riot", riot6532_device, read, write)
	AM_RANGE(0x0400, 0x0fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( spectra )
	PORT_START("SWITCH.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Outlane 1") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CR Hole") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TR Hole") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Bumper") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C Bumper") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Bumper") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Sling") PORT_CODE(KEYCODE_O)
	PORT_START("SWITCH.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Sling") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Outlane") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Rollover") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Rollover") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Inside Target") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C Inside Target") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Inside Target") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CL Target") PORT_CODE(KEYCODE_K)
	PORT_START("SWITCH.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CL Button") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TL Button") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TL Target") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Outlane 2") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Outlane 3") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Pentagon") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Pentagon") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TL Area") PORT_CODE(KEYCODE_COMMA)
	PORT_START("SWITCH.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Area") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Area") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Inlane Button") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_TILT)
	PORT_DIPNAME( 0x60, 0x60, "Test Switch" ) // 3-position slide switch
	PORT_DIPSETTING(    0x60, "Play" )
	PORT_DIPSETTING(    0x20, "Setup" )
	PORT_DIPSETTING(    0x40, "Test" )
INPUT_PORTS_END

void spectra_state::machine_reset()
{
	m_t_c = 0;
}

READ8_MEMBER( spectra_state::porta_r )
{
	UINT8 row = (m_porta & 0x18) >> 3;
	UINT8 key = m_switch[row]->read();
	UINT8 ret = ((BIT(key, m_porta & 7)) ? 0x40 : 0) | (m_porta & 0xbf);

	if (ret == 0x1b && m_p_ram[0x7b] < 0x1E)
		m_samples->start(3, 8); // coin

	return ret;
}

READ8_MEMBER( spectra_state::portb_r )
{
	if (m_p_ram[0xf0] != 1)
		return 0x5a; // factory reset if first time
	else
		return m_portb;
}

WRITE8_MEMBER( spectra_state::porta_w )
{
	m_porta = data;
}

// sound port
WRITE8_MEMBER( spectra_state::portb_w )
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
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	else
		m_t_c++;
}

// 00-27 displays
// 40-6F lamps
// 70-7F solenoids - no knocker
TIMER_DEVICE_CALLBACK_MEMBER( spectra_state::outtimer)
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x5c, 0x63, 0x01, 0x40, 0x08, 0 }; // 74C912
	m_out_offs++;

	if (m_out_offs < 0x28)
	{
		UINT8 data = m_p_ram[m_out_offs];
		UINT8 segments = patterns[data&15] | (BIT(data, 4) ? 0x80 : 0);
		output_set_digit_value(m_out_offs, segments);
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
}


static MACHINE_CONFIG_START( spectra, spectra_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_3_579545MHz/4)  // actually a M6503
	MCFG_CPU_PROGRAM_MAP(spectra_map)

	MCFG_DEVICE_ADD("riot", RIOT6532, XTAL_3_579545MHz/4)
	MCFG_RIOT6532_IN_PA_CB(READ8(spectra_state, porta_r))
	MCFG_RIOT6532_OUT_PA_CB(WRITE8(spectra_state, porta_w))
	MCFG_RIOT6532_IN_PB_CB(READ8(spectra_state, portb_r))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(spectra_state, portb_w))
	MCFG_RIOT6532_IRQ_CB(INPUTLINE("maincpu", M6502_IRQ_LINE))

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmitimer", spectra_state, nmitimer, attotime::from_hz(120))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("outtimer", spectra_state, outtimer, attotime::from_hz(1200))

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_spectra)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_M(1000), RES_M(1000), CAP_N(0)) // noise + filter
	MCFG_SN76477_DECAY_RES(RES_K(470))                    // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_N(1), RES_K(22))       // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(100))                      // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(52))                  // feedback_res
	MCFG_SN76477_VCO_PARAMS(5.0, CAP_U(0.01), RES_K(390)) // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(0.0)                       // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(0.1), RES_M(1))         // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(0.47), RES_K(470))  // oneshot caps + res
	MCFG_SN76477_VCO_MODE(0)                              // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                    // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(0, 0)                    // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                                // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/*--------------------------------
/ Spectra IV
/-------------------------------*/
ROM_START(spectra)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spect_u5.dat", 0x0400, 0x0400, CRC(49e0759f) SHA1(c3badc90ff834cbc92d8c519780069310c2b1507))
	ROM_LOAD("spect_u4.dat", 0x0800, 0x0400, CRC(b58f1205) SHA1(9578fd89485f3f560789cb0f24c7116e4bc1d0da) BAD_DUMP)
	ROM_LOAD("spect_u3.dat", 0x0c00, 0x0400, CRC(9ca7510f) SHA1(a87849f16903836158063d593bb4a2e90c7473c8))
ROM_END


GAME(1979,  spectra,  0,  spectra,  spectra, driver_device, 0,  ROT0,  "Valley", "Spectra IV", MACHINE_MECHANICAL )
