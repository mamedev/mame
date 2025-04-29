// license:BSD-3-Clause
// copyright-holders:Mike Harris, Quench
/***************************************************************************

    bally.cpp

    Functions to emulate the various Bally pinball sound boards.

***************************************************************************/

#include "emu.h"
#include "ballysound.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BALLY_AS2888,           bally_as2888_device,           "as2888",           "Bally AS2888 Sound Board")
DEFINE_DEVICE_TYPE(BALLY_AS3022,           bally_as3022_device,           "as3022",           "Bally AS3022 Sound Board")
DEFINE_DEVICE_TYPE(BALLY_SOUNDS_PLUS,      bally_sounds_plus_device,      "sounds_plus",      "Bally Sounds Plus w/ Vocalizer Board")
DEFINE_DEVICE_TYPE(BALLY_CHEAP_SQUEAK,     bally_cheap_squeak_device,     "cheap_squeak",     "Bally Cheap Squeak Board")
DEFINE_DEVICE_TYPE(BALLY_SQUAWK_N_TALK,    bally_squawk_n_talk_device,    "squawk_n_talk",    "Bally Squawk & Talk Board")
DEFINE_DEVICE_TYPE(BALLY_SQUAWK_N_TALK_AY, bally_squawk_n_talk_ay_device, "squawk_n_talk_ay", "Bally Squawk & Talk w/ AY8910 Board")


//**************************************************************************
//  AS2888
//**************************************************************************
static const discrete_mixer_desc as2888_digital_mixer_info =
{
		DISC_MIXER_IS_RESISTOR,                       /* type */
		{RES_K(33), RES_K(3.9)},                      /* r{} */
		{0, 0, 0, 0},                                 /* r_node */
		{0, 0},                                       /* c{} */
		0,                                            /* rI  */
//      RES_VOLTAGE_DIVIDER(RES_K(10), RES_R(360)),   /* rF  */
		RES_K(10),                                    /* rF  */   // not really
		CAP_U(0.01),                                  /* cF  */
		0,                                            /* cAmp */
		0,                                            /* vRef */
		0.00002                                       /* gain */
};

static const discrete_op_amp_filt_info as2888_preamp_info = {
		RES_K(10), 0, RES_R(470), 0,      /* r1 .. r4 */
		RES_K(10),                        /* rF */
		CAP_U(1),                         /* C1 */
		0,                                /* C2 */
		0,                                /* C3 */
		0.0,                              /* vRef */
		12.0,                             /* vP */
		-12.0,                            /* vN */
};

static DISCRETE_SOUND_START(as2888_discrete)

	DISCRETE_INPUT_DATA(NODE_08)        // Start Sustain Attenuation from 555 circuit
	DISCRETE_INPUT_LOGIC(NODE_01)       // Binary Counter B output (divide by 1) T2
	DISCRETE_INPUT_LOGIC(NODE_04)       // Binary Counter D output (divide by 4) T3

	DISCRETE_DIVIDE(NODE_11, 1, NODE_01, 1) // 2
	DISCRETE_DIVIDE(NODE_14, 1, NODE_04, 1)


	DISCRETE_RCFILTER(NODE_06, NODE_14, RES_K(15), CAP_U(0.1))      // T4 filter
#if 0
	DISCRETE_RCFILTER(NODE_05, NODE_11, RES_K(33), CAP_U(0.01))     // T1 filter
	DISCRETE_ADDER2(NODE_07, 1, NODE_05, NODE_06)
#else

	DISCRETE_MIXER2(NODE_07, 1, NODE_11, NODE_06, &as2888_digital_mixer_info)   // Mix and filter T1 and T4 together
#endif
	DISCRETE_RCDISC5(NODE_87, 1, NODE_08, RES_K(150), CAP_U(1.0))

	DISCRETE_RCFILTER_VREF(NODE_88,NODE_87,RES_M(1),CAP_U(0.01),2)
	DISCRETE_MULTIPLY(NODE_09, NODE_07, NODE_88)    // Apply sustain

	DISCRETE_OP_AMP_FILTER(NODE_20, 1, NODE_09, 0, DISC_OP_AMP_FILTER_IS_HIGH_PASS_1, &as2888_preamp_info)

	DISCRETE_CRFILTER(NODE_25, NODE_20, RES_M(100), CAP_U(0.05))    // Resistor is fake. Capacitor in series between pre-amp and output amp.

	DISCRETE_GAIN(NODE_30, NODE_25, 50) // Output amplifier LM380 fixed inbuilt gain of 50

	DISCRETE_OUTPUT(NODE_30, 10000000)  //  17000000
DISCRETE_SOUND_END

//-------------------------------------------------
//  sound_select - handle an external write to the board
//-------------------------------------------------

void bally_as2888_device::sound_select(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as2888_device::sound_select_sync), this), data);
}


TIMER_CALLBACK_MEMBER(bally_as2888_device::sound_select_sync)
{
	m_sound_select = param ^ 0x10;
}

//-------------------------------------------------
//  sound_int - handle an external sound interrupt to the board
//-------------------------------------------------

void bally_as2888_device::sound_int(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as2888_device::sound_int_sync), this), state);
}

TIMER_CALLBACK_MEMBER(bally_as2888_device::sound_int_sync)
{
	if (param)
	{
		m_snd_sustain_timer->adjust(attotime::from_msec(5));
		m_discrete->write(NODE_08, 11);  // 11 volt pulse
	}
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void bally_as2888_device::device_add_mconfig(machine_config &config)
{
	DISCRETE(config, m_discrete, as2888_discrete);
	m_discrete->add_route(ALL_OUTPUTS, *this, 1.00, 0);

	TIMER(config, "timer_s_freq").configure_periodic(FUNC(bally_as2888_device::timer_s), attotime::from_hz(353000));     // Inverter clock on AS-2888 sound board
	TIMER(config, m_snd_sustain_timer).configure_generic(FUNC(bally_as2888_device::timer_as2888));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bally_as2888_device::device_start()
{
	save_item(NAME(m_sound_select));
	save_item(NAME(m_snd_sel));
	save_item(NAME(m_snd_tone_gen));
	save_item(NAME(m_snd_div));
}

TIMER_DEVICE_CALLBACK_MEMBER(bally_as2888_device::timer_s)
{
	m_snd_tone_gen--;

	if ((m_snd_tone_gen == 0) && (m_snd_sel != 0x01))
	{
		m_snd_tone_gen = m_snd_sel;
		m_snd_div++;

		m_discrete->write(NODE_04, BIT(m_snd_div, 2) * 1);
		m_discrete->write(NODE_01, BIT(m_snd_div, 0) * 1);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(bally_as2888_device::timer_as2888)
{
	m_snd_sel = m_snd_prom[m_sound_select];
	m_snd_sel = bitswap<8>(m_snd_sel,0,1,2,3,4,5,6,7);
	m_snd_tone_gen = m_snd_sel;

	m_discrete->write(NODE_08, 0);
	m_snd_sustain_timer->adjust(attotime::never);
}


//**************************************************************************
//  AS3022
//**************************************************************************

//--------------------------------------------------------------------------
//  IO ports
//--------------------------------------------------------------------------

static INPUT_PORTS_START(as3022)
		PORT_START("SW1")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Sound Test") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bally_as3022_device::sw1), 0)
INPUT_PORTS_END

ioport_constructor bally_as3022_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(as3022);
}

INPUT_CHANGED_MEMBER(bally_as3022_device::sw1)
{
	if (newval != oldval)
		m_cpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

//-------------------------------------------------
//  sound_select - handle an external write to the board
//-------------------------------------------------

void bally_as3022_device::sound_select(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as3022_device::sound_select_sync), this), data);
}


TIMER_CALLBACK_MEMBER(bally_as3022_device::sound_select_sync)
{
	m_sound_select = param;
}

//-------------------------------------------------
//  sound_int - handle an external sound interrupt to the board
//-------------------------------------------------

void bally_as3022_device::sound_int(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as3022_device::sound_int_sync), this), state);
}

TIMER_CALLBACK_MEMBER(bally_as3022_device::sound_int_sync)
{
	m_pia->ca1_w(param);
}

//-------------------------------------------------
//  pia_irq_w - IRQ line state changes
//-------------------------------------------------

void bally_as3022_device::pia_irq_w(int state)
{
	int combined_state = m_pia->irq_a_state() | m_pia->irq_b_state();
	m_cpu->set_input_line(M6802_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}

//-------------------------------------------------
//  CPU map, from schematics
//-------------------------------------------------

void bally_as3022_device::as3022_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x1fff);  // A13-15 are unconnected
	map(0x0000, 0x007f).mirror(0x0f00).ram();
	map(0x0080, 0x0083).mirror(0x0f7c).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x1fff).rom();  // 4k RAM space, but could be jumpered for 2k
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void bally_as3022_device::device_add_mconfig(machine_config &config)
{
	M6808(config, m_cpu, DERIVED_CLOCK(1, 1));
	m_cpu->set_addrmap(AS_PROGRAM, &bally_as3022_device::as3022_map);

	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(bally_as3022_device::pia_porta_r));
	m_pia->writepa_handler().set(FUNC(bally_as3022_device::pia_porta_w));
	m_pia->writepb_handler().set(FUNC(bally_as3022_device::pia_portb_w));
	m_pia->cb2_handler().set(FUNC(bally_as3022_device::pia_cb2_w));
	m_pia->irqa_handler().set(FUNC(bally_as3022_device::pia_irq_w));
	m_pia->irqb_handler().set(FUNC(bally_as3022_device::pia_irq_w));

	for (required_device<filter_rc_device> &filter : m_ay_filters)
		// TODO: Calculate exact filter values. An AC filter is good enough for now
		// and required as the chip likes to output a DC offset at idle.
		FILTER_RC(config, filter).set_ac().add_route(ALL_OUTPUTS, *this, 1.0);
	AY8910(config, m_ay, DERIVED_CLOCK(1, 4));
	m_ay->add_route(0, "ay_filter0", 0.33);
	m_ay->add_route(1, "ay_filter1", 0.33);
	m_ay->add_route(2, "ay_filter2", 0.33);
	m_ay->port_a_read_callback().set(FUNC(bally_as3022_device::ay_io_r));
	m_ay->add_route(ALL_OUTPUTS, *this, 0.33, 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bally_as3022_device::device_start()
{
	// Set volumes to a sane default.
	m_ay->set_volume(0, 0);
	m_ay->set_volume(1, 0);
	m_ay->set_volume(2, 0);

	save_item(NAME(m_bc1));
	save_item(NAME(m_bdir));
	save_item(NAME(m_sound_select));
	save_item(NAME(m_ay_data));
}

//-------------------------------------------------
//  pia_porta_r - PIA port A reads
//-------------------------------------------------

uint8_t bally_as3022_device::pia_porta_r()
{
	if (m_bc1 && !m_bdir)
	{
		m_ay_data = m_ay->data_r();
		return m_ay_data;
	}
	else
	{
		// Nothing is active on the bus, so return open bus.
		return 0xff;
	}
}

//-------------------------------------------------
//  pia_porta_w - PIA port A writes
//-------------------------------------------------

void bally_as3022_device::pia_porta_w(uint8_t data)
{
	if (m_bc1 && !m_bdir)
	{
		logerror("PIA port A bus contention!\n");
	}
	m_ay_data = data;
	update_ay_bus();
}

//-------------------------------------------------
//  pia_portb_w - PIA port B writes
//-------------------------------------------------

void bally_as3022_device::pia_portb_w(uint8_t data)
{
	m_bc1 = BIT(data, 0);
	m_bdir = BIT(data, 1);
	if (m_bc1 && !m_bdir)
	{
		m_ay_data = m_ay->data_r();
	}
	update_ay_bus();
}

//-------------------------------------------------
//  pia_cb2_w - PIA CB2 writes
//-------------------------------------------------

void bally_as3022_device::pia_cb2_w(int state)
{
	// This pin is hooked up to the amp, and disables sounds when hi
	if (state)
	{
		m_ay->set_volume(0, 0);
		m_ay->set_volume(1, 0);
		m_ay->set_volume(2, 0);
	}
	else
	{
		m_ay->set_volume(0, 0xff);
		m_ay->set_volume(1, 0xff);
		m_ay->set_volume(2, 0xff);
	}
}

//-------------------------------------------------
//  ay_io_r - AY8912 IO A reads (B is unconnected)
//-------------------------------------------------

uint8_t bally_as3022_device::ay_io_r()
{
	// The two high bits are unconnected, the others are inverted.
	return ~m_sound_select & 0x3f;
}

void bally_as3022_device::update_ay_bus()
{
	if (m_bc1 && m_bdir)
	{
		m_ay->address_w(m_ay_data);
	}
	else if (!m_bc1 && m_bdir)
	{
		m_ay->data_w(m_ay_data);
	}
}


//**************************************************************************
//  SOUNDS PLUS WITH VOCALIZER
//**************************************************************************

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void bally_sounds_plus_device::device_add_mconfig(machine_config &config)
{
	bally_as3022_device::device_add_mconfig(config);

	m_cpu->set_addrmap(AS_PROGRAM, &bally_sounds_plus_device::sounds_plus_map);
	m_pia->writepb_handler().set(FUNC(bally_sounds_plus_device::vocalizer_pia_portb_w));

	// TODO: Calculate exact filter values. An AC filter is good enough for now
	// and required as the chip likes to output a DC offset at idle.
	FILTER_RC(config, m_mc3417_filter).set_ac();
	m_mc3417_filter->add_route(ALL_OUTPUTS, *this, 1.0);
	MC3417(config, m_mc3417, 0);
	// A gain of 2.2 is a guess. It sounds about loud enough and doesn't clip.
	m_mc3417->add_route(ALL_OUTPUTS, "mc3417_filter", 2.2);
}

//-------------------------------------------------
//  CPU map, from schematics
//-------------------------------------------------

void bally_sounds_plus_device::sounds_plus_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).mirror(0x7f00).ram();
	map(0x0080, 0x0083).mirror(0x7f7c).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xffff).rom();
}

//-------------------------------------------------
//  pia_portb_w - PIA port B writes
//-------------------------------------------------

void bally_sounds_plus_device::vocalizer_pia_portb_w(uint8_t data)
{
	bool speech_clock = BIT(data, 6);
	bool speech_data = BIT(data, 7);
	m_mc3417->clock_w(speech_clock ? 1 : 0);
	m_mc3417->digit_w(speech_data ? 1 : 0);
	pia_portb_w(data);
}


//**************************************************************************
//  Cheap Squeak
//**************************************************************************

//--------------------------------------------------------------------------
//  IO ports
//--------------------------------------------------------------------------
static INPUT_PORTS_START(cheap_squeak)
		PORT_START("SW1")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Sound Test") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bally_cheap_squeak_device::sw1), 0)
INPUT_PORTS_END

bally_cheap_squeak_device::bally_cheap_squeak_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		uint32_t clock) :
	bally_cheap_squeak_device(mconfig, BALLY_CHEAP_SQUEAK, tag, owner, clock)
{ }

bally_cheap_squeak_device::bally_cheap_squeak_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_cpu(*this, "cpu"),
	m_dac(*this, "dac"),
	m_sound_ack_w_handler(*this),
	m_leds(*this, "sound_led%u", 0U)
{ }

ioport_constructor bally_cheap_squeak_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cheap_squeak);
}

INPUT_CHANGED_MEMBER(bally_cheap_squeak_device::sw1)
{
	m_cpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

//-------------------------------------------------
//  sound_select - handle an external write to the board
//-------------------------------------------------

void bally_cheap_squeak_device::sound_select(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_cheap_squeak_device::sound_select_sync), this), data);
}

TIMER_CALLBACK_MEMBER(bally_cheap_squeak_device::sound_select_sync)
{
	m_sound_select = param;
}

//-------------------------------------------------
//  sound_int - handle an external sound interrupt to the board
//-------------------------------------------------
void bally_cheap_squeak_device::sound_int(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_cheap_squeak_device::sound_int_sync), this), state);
}

TIMER_CALLBACK_MEMBER(bally_cheap_squeak_device::sound_int_sync)
{
	m_sound_int = param;
	m_cpu->set_input_line(M6801_TIN_LINE, (m_sound_int ? ASSERT_LINE : CLEAR_LINE));
	update_led();
}

//-------------------------------------------------
//  CPU map, from schematics
//-------------------------------------------------

void bally_cheap_squeak_device::cheap_squeak_map(address_map &map)
{
	map.unmap_value_high();
	map(0x8000, 0x9fff).mirror(0x2000).rom();
	map(0xc000, 0xdfff).mirror(0x2000).rom();
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void bally_cheap_squeak_device::device_add_mconfig(machine_config &config)
{
	M6803(config, m_cpu, DERIVED_CLOCK(1, 1));
	m_cpu->set_addrmap(AS_PROGRAM, &bally_cheap_squeak_device::cheap_squeak_map);
	m_cpu->out_p1_cb().set(FUNC(bally_cheap_squeak_device::out_p1_cb));
	m_cpu->in_p2_cb().set(FUNC(bally_cheap_squeak_device::in_p2_cb));
	m_cpu->out_p2_cb().set(FUNC(bally_cheap_squeak_device::out_p2_cb));

	ZN429E(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 1.00, 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bally_cheap_squeak_device::device_start()
{
	m_leds.resolve();

	save_item(NAME(m_sound_select));
	save_item(NAME(m_sound_int));
}

//-------------------------------------------------
//  out_p1_cb - IO port 1 write
//-------------------------------------------------

void bally_cheap_squeak_device::out_p1_cb(uint8_t data)
{
	m_dac->write(data);
}

//-------------------------------------------------
//  in_p2_cb - IO port 2 read
//-------------------------------------------------

uint8_t bally_cheap_squeak_device::in_p2_cb()
{
	int sound_int_bit = m_sound_int ? 1 : 0;
	return 0x40 | (m_sound_select & 0x0f) << 1 | sound_int_bit;
}

//-------------------------------------------------
//  out_p2_cb - IO port 2 write
//-------------------------------------------------

void bally_cheap_squeak_device::out_p2_cb(uint8_t data)
{
	m_sound_ack = BIT(data, 0);
	m_sound_ack_w_handler(m_sound_ack);
	update_led();
}

void bally_cheap_squeak_device::update_led()
{
	// Either input or output can pull the led line high
	m_leds[0] = m_sound_int || m_sound_ack;
}


//**************************************************************************
//  Squawk & Talk
//**************************************************************************

bally_squawk_n_talk_device::bally_squawk_n_talk_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		uint32_t clock) :
	bally_squawk_n_talk_device(mconfig, BALLY_SQUAWK_N_TALK, tag, owner, clock)
{ }

bally_squawk_n_talk_device::bally_squawk_n_talk_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_cpu(*this, "cpu"),
	m_pia1(*this, "pia1"),
	m_pia2(*this, "pia2"),
	m_dac_filter(*this, "dac_filter"),
	m_dac(*this, "dac"),
	m_speech_filter(*this, "speech_filter"),
	m_tms5200(*this, "tms5200")
{ }

bally_squawk_n_talk_ay_device::bally_squawk_n_talk_ay_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		uint32_t clock) :
	bally_squawk_n_talk_device(mconfig, BALLY_SQUAWK_N_TALK_AY, tag, owner, clock),
	m_ay_filters(*this, "ay_filter%u", 0),
	m_ay(*this, "ay")
{ }

//--------------------------------------------------------------------------
//  IO ports
//--------------------------------------------------------------------------
static INPUT_PORTS_START(squawk_n_talk)
	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("SW1") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bally_squawk_n_talk_device::sw1), 0)
INPUT_PORTS_END

ioport_constructor bally_squawk_n_talk_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(squawk_n_talk);
}

INPUT_CHANGED_MEMBER(bally_squawk_n_talk_device::sw1)
{
	if (newval != oldval)
		m_cpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

//-------------------------------------------------
//  sound_select - handle an external write to the board
//-------------------------------------------------

void bally_squawk_n_talk_device::sound_select(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_squawk_n_talk_device::sound_select_sync), this), data);
}

TIMER_CALLBACK_MEMBER(bally_squawk_n_talk_device::sound_select_sync)
{
	m_sound_select = param;
}

//-------------------------------------------------
//  sound_int - handle an external sound interrupt to the board
//-------------------------------------------------

void bally_squawk_n_talk_device::sound_int(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_squawk_n_talk_device::sound_int_sync), this), state);
}

TIMER_CALLBACK_MEMBER(bally_squawk_n_talk_device::sound_int_sync)
{
	// the line runs though in inverter
	m_pia2->cb1_w(!param);
}

//-------------------------------------------------
//  CPU map, from schematics
//-------------------------------------------------

void bally_squawk_n_talk_device::squawk_n_talk_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0080, 0x0083).mirror(0x4f6c).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).mirror(0x4f6c).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x1000).mirror(0x40ff).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x8000, 0x8fff).mirror(0x4000).rom();  // U2
	map(0x9000, 0x9fff).mirror(0x4000).rom();  // U3
	map(0xa000, 0xafff).mirror(0x4000).rom();  // U4
	map(0xb000, 0xbfff).mirror(0x4000).rom();  // U5
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void bally_squawk_n_talk_device::device_add_mconfig(machine_config &config)
{
	M6802(config, m_cpu, DERIVED_CLOCK(1, 1)); // could also be jumpered to use a 6808
	m_cpu->set_addrmap(AS_PROGRAM, &bally_squawk_n_talk_device::squawk_n_talk_map);

	PIA6821(config, m_pia1);
	m_pia1->readpa_handler().set(m_tms5200, FUNC(tms5220_device::status_r));
	m_pia1->writepa_handler().set(m_tms5200, FUNC(tms5220_device::data_w));
	m_pia1->writepb_handler().set(FUNC(bally_squawk_n_talk_device::pia1_portb_w));
	m_pia1->irqa_handler().set(FUNC(bally_squawk_n_talk_device::pia_irq_w));
	m_pia1->irqb_handler().set(FUNC(bally_squawk_n_talk_device::pia_irq_w));

	PIA6821(config, m_pia2);
	m_pia2->readpa_handler().set(FUNC(bally_squawk_n_talk_device::pia2_porta_r));
	m_pia2->ca2_handler().set_output("sound_led0");
	m_pia2->irqa_handler().set(FUNC(bally_squawk_n_talk_device::pia_irq_w));
	m_pia2->irqb_handler().set(FUNC(bally_squawk_n_talk_device::pia_irq_w));

	FILTER_RC(config, m_dac_filter);
	m_dac_filter->add_route(ALL_OUTPUTS, *this, 1.0);
	m_dac_filter->set_rc(filter_rc_device::HIGHPASS, 2000, 0, 0, CAP_U(2));
	AD558(config, "dac").add_route(ALL_OUTPUTS, "dac_filter", 0.75);

	// TODO: Calculate exact filter values. An AC filter is good enough for now
	// and required as the chip likes to output a DC offset at idle.
	FILTER_RC(config, m_speech_filter).set_ac();
	m_speech_filter->add_route(ALL_OUTPUTS, *this, 1.0);
	TMS5200(config, m_tms5200, 640000);
	m_tms5200->add_route(ALL_OUTPUTS, "speech_filter", 1.0);
	m_tms5200->ready_cb().set(m_pia1, FUNC(pia6821_device::ca2_w));
	m_tms5200->irq_cb().set(m_pia1, FUNC(pia6821_device::cb1_w));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bally_squawk_n_talk_device::device_start()
{
	save_item(NAME(m_sound_select));
}

//-------------------------------------------------
//  pia1_portb_w - PIA 1 port B write
//-------------------------------------------------

void bally_squawk_n_talk_device::pia1_portb_w(uint8_t data)
{
	m_tms5200->rsq_w(BIT(data, 0));
	m_tms5200->wsq_w(BIT(data, 1));
}

//-------------------------------------------------
//  pia2_porta_r - PIA 2 port A reads
//-------------------------------------------------

uint8_t bally_squawk_n_talk_device::pia2_porta_r()
{
	// 5 lines and they go through inverters
	return ~m_sound_select & 0x1f;
}

//-------------------------------------------------
//  pia_irq_w - IRQ line state changes
//-------------------------------------------------
void bally_squawk_n_talk_device::pia_irq_w(int state)
{
	int combined_state = m_pia1->irq_a_state() | m_pia1->irq_b_state() | m_pia2->irq_a_state() | m_pia2->irq_b_state();
	m_cpu->set_input_line(M6802_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------
void bally_squawk_n_talk_ay_device::device_add_mconfig(machine_config &config)
{
	bally_squawk_n_talk_device::device_add_mconfig(config);

	m_pia2->writepa_handler().set(FUNC(bally_squawk_n_talk_ay_device::pia2_porta_w));
	m_pia2->readpa_handler().set(FUNC(bally_squawk_n_talk_ay_device::pia2_porta_r));
	m_pia2->writepb_handler().set(FUNC(bally_squawk_n_talk_ay_device::pia2_portb_w));
	m_pia2->cb2_handler().set(FUNC(bally_squawk_n_talk_ay_device::pia2_cb2_w));

	// TODO: Calculate exact filter values. An AC filter is good enough for now
	// and required as the chip likes to output a DC offset at idle.
	for (auto &filter : m_ay_filters)
		FILTER_RC(config, filter).set_ac().add_route(ALL_OUTPUTS, *this, 1.0);

	AY8910(config, m_ay, DERIVED_CLOCK(1, 4));
	m_ay->add_route(0, "ay_filter0", 0.33);
	m_ay->add_route(1, "ay_filter1", 0.33);
	m_ay->add_route(2, "ay_filter2", 0.33);
	m_ay->port_a_read_callback().set(FUNC(bally_squawk_n_talk_ay_device::ay_io_r));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void bally_squawk_n_talk_ay_device::device_start()
{
	bally_squawk_n_talk_device::device_start();

	// Set volumes to a sane default.
	m_ay->set_volume(0, 0);
	m_ay->set_volume(1, 0);
	m_ay->set_volume(2, 0);

	save_item(NAME(m_bc1));
	save_item(NAME(m_bdir));
	save_item(NAME(m_ay_data));
}

//-------------------------------------------------
//  pia2_porta_r - PIA 2 port A reads
//-------------------------------------------------

uint8_t bally_squawk_n_talk_ay_device::pia2_porta_r()
{
	if (m_bc1 && !m_bdir)
	{
		m_ay_data = m_ay->data_r();
	}
	// This should return the open bus, but this method is called even if the PIA
	// is in output mode. Self test expects to see the same value.
	return m_ay_data;
}

//-------------------------------------------------
//  pia2_porta_w - PIA 2 port A writes
//-------------------------------------------------

void bally_squawk_n_talk_ay_device::pia2_porta_w(uint8_t data)
{
	if (m_bc1 && !m_bdir)
	{
		logerror("PIA2 port A bus contention!\n");
	}
	m_ay_data = data;
	update_ay_bus();
}

//-------------------------------------------------
//  pia2_portb_w - PIA 2 port B writes
//-------------------------------------------------

void bally_squawk_n_talk_ay_device::pia2_portb_w(uint8_t data)
{
	m_bc1 = BIT(data, 0);
	m_bdir = BIT(data, 1);
	if (m_bc1 && !m_bdir)
	{
		m_ay_data = m_ay->data_r();
	}
	update_ay_bus();
}

//-------------------------------------------------
//  pia2_cb2_w - PIA 2 CB2 writes
//-------------------------------------------------
void bally_squawk_n_talk_ay_device::pia2_cb2_w(int state)
{
	// This pin is hooked up to the amp, and disables sounds when hi
	if (state)
	{
		m_ay->set_volume(0, 0);
		m_ay->set_volume(1, 0);
		m_ay->set_volume(2, 0);
	}
	else
	{
		m_ay->set_volume(0, 0xff);
		m_ay->set_volume(1, 0xff);
		m_ay->set_volume(2, 0xff);
	}
}

//-------------------------------------------------
//  ay_io_r - AY8910 read
//-------------------------------------------------

uint8_t bally_squawk_n_talk_ay_device::ay_io_r()
{
	// 5 lines and they go through inverters
	return ~m_sound_select & 0x1f;
}

void bally_squawk_n_talk_ay_device::update_ay_bus()
{
	if (m_bc1 && m_bdir)
	{
		m_ay->address_w(m_ay_data);
	}
	else if (!m_bc1 && m_bdir)
	{
		m_ay->data_w(m_ay_data);
	}
}
