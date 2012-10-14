/******************************************************************************************
    Pinball
    Valley Spectra IV

    Rotating game, like Midway's "Rotation VIII".

    2012-09-28 System working [Robbbert]

    Schematic and PinMAME used as references.
    Code for the sn76477 sound was derived from PinMAME.

    There is a bug - if you score 1000 and had less than 100, the hundreds digit will
    be blank. It will of course fix itself during the course of the game.


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
		: genpin_class(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_snsnd(*this, "snsnd"),
	m_p_ram(*this, "ram")
	{ }

	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portb_w);
	TIMER_DEVICE_CALLBACK_MEMBER(nmitimer);
	TIMER_DEVICE_CALLBACK_MEMBER(outtimer);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<sn76477_device> m_snsnd;
	required_shared_ptr<UINT8> m_p_ram;

	// driver_device overrides
	virtual void machine_reset();
private:
	UINT8 m_porta;
	UINT8 m_portb;
	UINT8 m_t_c;
	UINT8 m_out_offs;
};


static ADDRESS_MAP_START( spectra_map, AS_PROGRAM, 8, spectra_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_SHARE("ram") // battery backed, 2x 5101L
	AM_RANGE(0x0100, 0x017f) AM_RAM // RIOT RAM
	AM_RANGE(0x0180, 0x019f) AM_DEVREADWRITE_LEGACY("riot", riot6532_r, riot6532_w)
	AM_RANGE(0x0400, 0x0fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( spectra )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Outlane 1") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CR Hole") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TR Hole") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Bumper") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C Bumper") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Bumper") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Triangle") PORT_CODE(KEYCODE_O)
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Triangle") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Outlane") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Rollover") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Rollover") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Inside Target") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C Inside Target") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Inside Target") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CL Target") PORT_CODE(KEYCODE_K)
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CL Button") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TL Button") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TL Target") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Outlane 2") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Outlane 3") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Pentagon") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Pentagon") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TL Area") PORT_CODE(KEYCODE_COMMA)
	PORT_START("X3")
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
	char kbdrow[6];
	UINT8 key=0, old=0, ret=0;
	sprintf(kbdrow,"X%X",(m_porta & 0x18) >> 3);
	key = ioport(kbdrow)->read();
	ret = ((BIT(key, m_porta & 7)) ? 0x40 : 0) | (m_porta & 0xbf);

	if (ret == 0x1b && old != ret && m_p_ram[0x7b] < 0x1E)
		m_samples->start(2, 4); // coin
	old = ret;

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
	if (BIT(data, 0)) vco -= 0.3125;
	if (BIT(data, 1)) vco -= 0.625;
	if (BIT(data, 2)) vco -= 1.25;
	if (BIT(data, 3)) vco -= 2.5;
	sn76477_vco_voltage_w(m_snsnd, 5.4 - vco);
	sn76477_enable_w(m_snsnd, !BIT(data, 4)); // strobe: toggles enable
	sn76477_envelope_1_w(m_snsnd, !BIT(data, 5)); //decay: toggles envelope
	sn76477_vco_w(m_snsnd, BIT(data, 6)); // "phaser" sound: VCO toggled
	sn76477_mixer_b_w(m_snsnd, BIT(data, 7)); // "pulse" sound: pins 25 & 27 changed
	sn76477_mixer_c_w(m_snsnd, BIT(data, 7)); // "pulse" sound: pins 25 & 27 changed
}


static const riot6532_interface riot6532_intf =
{
	DEVCB_DRIVER_MEMBER(spectra_state, porta_r),	// port a in
	DEVCB_DRIVER_MEMBER(spectra_state, portb_r),	// port b in
	DEVCB_DRIVER_MEMBER(spectra_state, porta_w),	// port a out
	DEVCB_DRIVER_MEMBER(spectra_state, portb_w),	// port b in
	DEVCB_CPU_INPUT_LINE("maincpu", M6502_IRQ_LINE)	// interrupt
};

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
		m_out_offs = 0xff;
}

static const sn76477_interface sn76477_intf =
{
	RES_M(1000),	/*  4  noise_res        */
	RES_M(1000),	/*  5  filter_res       */
	CAP_N(0),	/*  6  filter_cap       */
	RES_K(470),	/*  7  decay_res        */
	CAP_N(1),	/*  8  attack_decay_cap */
	RES_K(22),	/* 10  attack_res       */
	RES_K(100),	/* 11  amplitude_res    */
	RES_K(52),	/* 12  feedback_res     */
	5.0,	/* 16  vco_voltage      */
	CAP_U(0.01),	/* 17  vco_cap          */
	RES_K(390),	/* 18  vco_res          */
	0.0,  /* 19  pitch_voltage  */
	RES_M(1),	/* 20  slf_res          */
	CAP_U(0.1),	/* 21  slf_cap          */
	CAP_U(0.47),	/* 23  oneshot_cap      */
	RES_K(470),		/* 24  oneshot_res  */
	0,			    /* 22  vco (variable)               */
	0,			    /* 26  mixer A (grounded)           */
	0,			    /* 25  mixer B (variable)           */
	0,			    /* 27  mixer C (variable)           */
	0,			    /* 1   envelope 1 (variable)        */
	0,			    /* 28  envelope 2 (grounded)        */
	1			    /* 9   enable (variable)            */
};


static MACHINE_CONFIG_START( spectra, spectra_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 3579545/4)  // actually a 6503
	MCFG_CPU_PROGRAM_MAP(spectra_map)
	MCFG_RIOT6532_ADD("riot", 3579545/4, riot6532_intf) // R6532
	MCFG_NVRAM_ADD_1FILL("ram")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmitimer", spectra_state, nmitimer, attotime::from_hz(120))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("outtimer", spectra_state, outtimer, attotime::from_hz(1200))

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_spectra)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SOUND_CONFIG(sn76477_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/*--------------------------------
/ Spectra IV
/-------------------------------*/
ROM_START(spectra)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spect_u5.dat", 0x0400, 0x0400, CRC(49e0759f) SHA1(c3badc90ff834cbc92d8c519780069310c2b1507))
	// pinmame has a different u4 rom: CRC(b58f1205) SHA1(9578fd89485f3f560789cb0f24c7116e4bc1d0da)
	ROM_LOAD("spect_u4.dat", 0x0800, 0x0400, BAD_DUMP CRC(e6519689) SHA1(06ef3d349ea27a072889b7c379f258d29b7217be))
	ROM_LOAD("spect_u3.dat", 0x0c00, 0x0400, CRC(9ca7510f) SHA1(a87849f16903836158063d593bb4a2e90c7473c8))
ROM_END


GAME(1979,  spectra,  0,  spectra,  spectra, driver_device, 0,  ROT0,  "Valley", "Spectra IV", GAME_MECHANICAL)
