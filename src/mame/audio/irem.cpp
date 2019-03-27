// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    Irem M52/M62 sound hardware

***************************************************************************/

#include "emu.h"
#include "audio/irem.h"

#include "cpu/m6800/m6801.h"
#include "sound/discrete.h"
#include "speaker.h"


DEFINE_DEVICE_TYPE(IREM_M62_AUDIO,        m62_audio_device,        "m62_audio",        "Irem M62 Audio")
DEFINE_DEVICE_TYPE(IREM_M52_SOUNDC_AUDIO, m52_soundc_audio_device, "m52_soundc_audio", "Irem M52 SoundC Audio")
DEFINE_DEVICE_TYPE(IREM_M52_LARGE_AUDIO,  m52_large_audio_device,  "m52_large_audio",  "Irem M52 Large Audio")

irem_audio_device::irem_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_cpu(*this, "iremsound")
	, m_adpcm1(*this, "msm1")
	, m_adpcm2(*this, "msm2")
	, m_ay_45L(*this, "ay_45l")
	, m_ay_45M(*this, "ay_45m")
	, m_port1(0)
	, m_port2(0)
	, m_soundlatch(0)
	, m_audio_BD(*this, "snd_nl:ibd")
	, m_audio_SD(*this, "snd_nl:isd")
	, m_audio_OH(*this, "snd_nl:ioh")
	, m_audio_CH(*this, "snd_nl:ich")
{
}

m62_audio_device::m62_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: irem_audio_device(mconfig, IREM_M62_AUDIO, tag, owner, clock)
{
}

m52_soundc_audio_device::m52_soundc_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: irem_audio_device(mconfig, IREM_M52_SOUNDC_AUDIO, tag, owner, clock)
{
}

m52_large_audio_device::m52_large_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: irem_audio_device(mconfig, IREM_M52_LARGE_AUDIO, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void irem_audio_device::device_start()
{
	m_audio_SINH = subdevice<netlist_mame_logic_input_device>("snd_nl:sinh");

	save_item(NAME(m_port1));
	save_item(NAME(m_port2));
	save_item(NAME(m_soundlatch));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void irem_audio_device::device_reset()
{
	m_port1 = 0; // ?
	m_port2 = 0; // ?
	m_soundlatch = 0;
	m_cpu->set_input_line(0, ASSERT_LINE);
}

/*************************************
 *
 *  External writes to the sound
 *  command register
 *
 *************************************/

WRITE8_MEMBER( irem_audio_device::cmd_w )
{
	m_soundlatch = data;
	if ((data & 0x80) == 0)
		m_cpu->set_input_line(0, ASSERT_LINE);
}


/*************************************
 *
 *  Sound latch read
 *
 *************************************/

READ8_MEMBER( irem_audio_device::soundlatch_r )
{
	return m_soundlatch;
}

/*************************************
 *
 *  6803 output ports
 *
 *************************************/

WRITE8_MEMBER( irem_audio_device::m6803_port1_w )
{
	m_port1 = data;
}


WRITE8_MEMBER( irem_audio_device::m6803_port2_w )
{
	/* write latch */
	if ((m_port2 & 0x01) && !(data & 0x01))
	{
		/* control or data port? */
		if (m_port2 & 0x04)
		{
			/* PSG 0 or 1? */
			if (m_port2 & 0x08)
				m_ay_45M->address_w(m_port1);
			if (m_port2 & 0x10)
				m_ay_45L->address_w(m_port1);
		}
		else
		{
			/* PSG 0 or 1? */
			if (m_port2 & 0x08)
				m_ay_45M->data_w(m_port1);
			if (m_port2 & 0x10)
				m_ay_45L->data_w(m_port1);
		}
	}
	m_port2 = data;
}



/*************************************
 *
 *  6803 input ports
 *
 *************************************/

READ8_MEMBER( irem_audio_device::m6803_port1_r )
{
	/* PSG 0 or 1? */
	if (m_port2 & 0x08)
		return m_ay_45M->data_r();
	if (m_port2 & 0x10)
		return m_ay_45L->data_r();
	return 0xff;
}


READ8_MEMBER( irem_audio_device::m6803_port2_r )
{
	/*
	 * Pin21, 6803 (Port 21) tied with 4.7k to +5V
	 *
	 */
	//printf("port2 read\n"); // used by 10yard
	return 0x0;
}



/*************************************
 *
 *  AY-8910 output ports
 *
 *************************************/

WRITE8_MEMBER( irem_audio_device::ay8910_45M_portb_w )
{
	/* bits 2-4 select MSM5205 clock & 3b/4b playback mode */
	m_adpcm1->playmode_w((data >> 2) & 7);
	if (m_adpcm2 != nullptr)
		m_adpcm2->playmode_w(((data >> 2) & 4) | 3); /* always in slave mode */

	/* bits 0 and 1 reset the two chips */
	m_adpcm1->reset_w(data & 1);
	if (m_adpcm2 != nullptr)
		m_adpcm2->reset_w(data & 2);
}


WRITE8_MEMBER( irem_audio_device::ay8910_45L_porta_w )
{
	/*
	 *  45L 21 IOA0  ==> BD
	 *  45L 20 IOA1  ==> SD
	 *  45L 19 IOA2  ==> OH
	 *  45L 18 IOA3  ==> CH
	 *
	 */
	if (m_audio_BD) m_audio_BD->write_line(data & 0x01 ? 1: 0);
	if (m_audio_SD) m_audio_SD->write_line(data & 0x02 ? 1: 0);
	if (m_audio_OH) m_audio_OH->write_line(data & 0x04 ? 1: 0);
	if (m_audio_CH) m_audio_CH->write_line(data & 0x08 ? 1: 0);
#ifdef MAME_DEBUG
	if (data & 0x0f) popmessage("analog sound %x",data&0x0f);
#endif
}



/*************************************
 *
 *  Memory-mapped accesses
 *
 *************************************/

WRITE8_MEMBER( irem_audio_device::sound_irq_ack_w )
{
	if ((m_soundlatch & 0x80) != 0)
		m_cpu->set_input_line(0, CLEAR_LINE);
}


WRITE8_MEMBER( irem_audio_device::m52_adpcm_w )
{
	if (offset & 1)
	{
		m_adpcm1->write_data(data);
	}
	if (offset & 2)
	{
		if (m_adpcm2 != nullptr)
			m_adpcm2->write_data(data);
	}
}


WRITE8_MEMBER( irem_audio_device::m62_adpcm_w )
{
	msm5205_device *adpcm = (offset & 1) ? m_adpcm2.target() : m_adpcm1.target();
	if (adpcm != nullptr)
		adpcm->write_data(data);
}



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

/* All 6 (3*2) AY-3-8910 outputs are tied together
 * and put with 470 Ohm to gnd.
 * The following is a approximation, since
 * it does not take cross-chip mixing effects into account.
 */


/*
 * http://newsgroups.derkeiler.com/Archive/Rec/rec.games.video.arcade.collecting/2006-06/msg03108.html
 *
 * mentions, that moon patrol does work on moon ranger hardware.
 * There is no MSM5250, but a 74LS00 producing white noise for explosions
 */

/* Certain values are different from schematics
 * I did my test to verify against pcb pictures of "tropical angel"
 */

#define M52_R9      560
#define M52_R10     330
#define M52_R12     RES_K(10)
#define M52_R13     RES_K(10)
#define M52_R14     RES_K(10)
#define M52_R15     RES_K(2.2)  /* schematics RES_K(22) , althought 10-Yard states 2.2 */
#define M52_R19     RES_K(10)
#define M52_R22     RES_K(47)
#define M52_R23     RES_K(2.2)
#define M52_R25     RES_K(10)
#define M52_VR1     RES_K(50)

#define M52_C28     CAP_U(1)
#define M52_C30     CAP_U(0.022)
#define M52_C32     CAP_U(0.022)
#define M52_C35     CAP_U(47)
#define M52_C37     CAP_U(0.1)
#define M52_C38     CAP_U(0.0068)

/*
 * C35 is disabled, the mixer would just deliver
 * no signals if it is enabled.
 * TODO: Check discrete mixer
 *
 */

static const discrete_mixer_desc m52_sound_c_stage1 =
	{DISC_MIXER_IS_RESISTOR,
		{M52_R19, M52_R22, M52_R23 },
		{      0,       0,       0 },   /* variable resistors   */
		{M52_C37,       0,       0 },   /* node capacitors      */
				0,      0,              /* rI, rF               */
		M52_C35*0,                      /* cF                   */
		0,                              /* cAmp                 */
		0, 1};

static const discrete_op_amp_filt_info m52_sound_c_sallen_key =
	{ M52_R13, M52_R14, 0, 0, 0,
		M52_C32, M52_C38, 0
	};

static const discrete_mixer_desc m52_sound_c_mix1 =
	{DISC_MIXER_IS_RESISTOR,
		{M52_R25, M52_R15 },
		{      0,       0 },    /* variable resistors   */
		{      0,       0 },    /* node capacitors      */
				0, M52_VR1,     /* rI, rF               */
		0,                      /* cF                   */
		CAP_U(1),               /* cAmp                 */
		0, 1};

static DISCRETE_SOUND_START( m52_sound_c_discrete )

	/* Chip AY8910/1 */
	DISCRETE_INPUTX_STREAM(NODE_01, 0, 1.0, 0)
	/* Chip AY8910/2 */
	DISCRETE_INPUTX_STREAM(NODE_02, 1, 1.0, 0)
	/* Chip MSM5250 */
	DISCRETE_INPUTX_STREAM(NODE_03, 2, 1.0, 0)

	/* Just mix the two AY8910s */
	DISCRETE_ADDER2(NODE_09, 1, NODE_01, NODE_02)
	DISCRETE_DIVIDE(NODE_10, 1, NODE_09, 2.0)

	/* Mix in 5 V to MSM5250 signal */
	DISCRETE_MIXER3(NODE_20, 1, NODE_03, 32767.0, 0, &m52_sound_c_stage1)

	/* Sallen - Key Filter */
	/* TODO: R12, C30: This looks like a band pass */
	DISCRETE_RCFILTER(NODE_25, NODE_20, M52_R12, M52_C30)
	DISCRETE_SALLEN_KEY_FILTER(NODE_30, 1, NODE_25, DISC_SALLEN_KEY_LOW_PASS, &m52_sound_c_sallen_key)

	/* Mix signals */
	DISCRETE_MIXER2(NODE_40, 1, NODE_10, NODE_25, &m52_sound_c_mix1)
	DISCRETE_CRFILTER(NODE_45, NODE_40, M52_R10+M52_R9, M52_C28)

	DISCRETE_OUTPUT(NODE_40, 18.0)

DISCRETE_SOUND_END

/*************************************
 *
 *  Address maps
 *
 *************************************/

/* complete address map verified from Moon Patrol/10 Yard Fight schematics */
/* large map uses 8k ROMs, small map uses 4k ROMs; this is selected via a jumper */
void irem_audio_device::m52_small_sound_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x0fff).w(FUNC(irem_audio_device::m52_adpcm_w));
	map(0x1000, 0x1fff).w(FUNC(irem_audio_device::sound_irq_ack_w));
	map(0x2000, 0x7fff).rom();
}

void irem_audio_device::m52_large_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).w(FUNC(irem_audio_device::m52_adpcm_w));
	map(0x2000, 0x3fff).w(FUNC(irem_audio_device::sound_irq_ack_w));
	map(0x4000, 0xffff).rom();
}


/* complete address map verified from Kid Niki schematics */
void irem_audio_device::m62_sound_map(address_map &map)
{
	map(0x0800, 0x0800).mirror(0xf7fc).w(FUNC(irem_audio_device::sound_irq_ack_w));
	map(0x0801, 0x0802).mirror(0xf7fc).w(FUNC(irem_audio_device::m62_adpcm_w));
	map(0x4000, 0xffff).rom();
}


/*
 * Original recordings:
 *
 * https://www.youtube.com/watch?v=Hr1wZpwP7R4
 *
 * This is not an original recording ("drums added")
 *
 * https://www.youtube.com/watch?v=aarl0xfBQf0
 *
 */

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void m62_audio_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	m6803_cpu_device &cpu(M6803(config, m_cpu, XTAL(3'579'545))); /* verified on pcb */
	cpu.set_addrmap(AS_PROGRAM, &m62_audio_device::m62_sound_map);
	cpu.in_p1_cb().set(FUNC(m62_audio_device::m6803_port1_r));
	cpu.out_p1_cb().set(FUNC(m62_audio_device::m6803_port1_w));
	cpu.in_p2_cb().set(FUNC(m62_audio_device::m6803_port2_r));
	cpu.out_p2_cb().set(FUNC(m62_audio_device::m6803_port2_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay_45M, XTAL(3'579'545)/4); /* verified on pcb */
	m_ay_45M->set_flags(AY8910_RESISTOR_OUTPUT);
	m_ay_45M->set_resistors_load(2000.0, 2000.0, 2000.0);
	m_ay_45M->port_a_read_callback().set(FUNC(irem_audio_device::soundlatch_r));
	m_ay_45M->port_b_write_callback().set(FUNC(irem_audio_device::ay8910_45M_portb_w));
	m_ay_45M->add_route(0, "snd_nl", 1.0, 0);
	m_ay_45M->add_route(1, "snd_nl", 1.0, 1);
	m_ay_45M->add_route(2, "snd_nl", 1.0, 2);

	AY8910(config, m_ay_45L, XTAL(3'579'545)/4); /* verified on pcb */
	m_ay_45L->set_flags(AY8910_RESISTOR_OUTPUT);
	m_ay_45L->set_resistors_load(2000.0, 2000.0, 2000.0);
	m_ay_45L->port_a_write_callback().set(FUNC(irem_audio_device::ay8910_45L_porta_w));
	m_ay_45L->add_route(0, "snd_nl", 1.0, 3);
	m_ay_45L->add_route(1, "snd_nl", 1.0, 4);
	m_ay_45L->add_route(2, "snd_nl", 1.0, 5);

	MSM5205(config, m_adpcm1, 384_kHz_XTAL); // verified on PCB
	m_adpcm1->vck_callback().set_inputline(m_cpu, INPUT_LINE_NMI); // driven through NPN inverter
	m_adpcm1->vck_callback().append(m_adpcm2, FUNC(msm5205_device::vclk_w)); // the first MSM5205 clocks the second
	m_adpcm1->set_prescaler_selector(msm5205_device::S96_4B); // default to 4KHz, but can be changed at run time
	m_adpcm1->add_route(0, "snd_nl", 1.0, 6);

	MSM5205(config, m_adpcm2, 384_kHz_XTAL); // verified on PCB
	m_adpcm2->set_prescaler_selector(msm5205_device::SEX_4B); // default to 4KHz, but can be changed at run time, slave
	m_adpcm2->add_route(0, "snd_nl", 1.0, 7);

	/* NETLIST configuration using internal AY8910 resistor values */

	netlist_mame_sound_device &snd_nl(NETLIST_SOUND(config, "snd_nl", 48000));
	snd_nl.set_constructor(netlist_kidniki);
	snd_nl.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "snd_nl:ibd", "I_BD0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "snd_nl:isd", "I_SD0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "snd_nl:ich", "I_CH0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "snd_nl:ioh", "I_OH0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "snd_nl:sinh", "SINH.IN", 0);

	NETLIST_STREAM_INPUT(config, "snd_nl:cin0", 0, "R_AY45M_A.R");
	NETLIST_STREAM_INPUT(config, "snd_nl:cin1", 1, "R_AY45M_B.R");
	NETLIST_STREAM_INPUT(config, "snd_nl:cin2", 2, "R_AY45M_C.R");

	NETLIST_STREAM_INPUT(config, "snd_nl:cin3", 3, "R_AY45L_A.R");
	NETLIST_STREAM_INPUT(config, "snd_nl:cin4", 4, "R_AY45L_B.R");
	NETLIST_STREAM_INPUT(config, "snd_nl:cin5", 5, "R_AY45L_C.R");


	NETLIST_STREAM_INPUT(config, "snd_nl:cin6", 6, "I_MSM2K0.IN").set_mult_offset(5.0/65535.0, 2.5);
	NETLIST_STREAM_INPUT(config, "snd_nl:cin7", 7, "I_MSM3K0.IN").set_mult_offset(5.0/65535.0, 2.5);

	//MCFG_NETLIST_STREAM_OUTPUT("snd_nl", 0, "RV1.1")
	//MCFG_NETLIST_ANALOG_MULT_OFFSET(30000.0, -35000.0)
	NETLIST_STREAM_OUTPUT(config, "snd_nl:cout0", 0, "R26.1").set_mult_offset(30000.0 * 10.0, 0.0);
}

void m52_soundc_audio_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	m6803_cpu_device &cpu(M6803(config, m_cpu, XTAL(3'579'545))); /* verified on pcb */
	cpu.set_addrmap(AS_PROGRAM, &m52_soundc_audio_device::m52_small_sound_map);
	cpu.in_p1_cb().set(FUNC(m52_soundc_audio_device::m6803_port1_r));
	cpu.out_p1_cb().set(FUNC(m52_soundc_audio_device::m6803_port1_w));
	cpu.in_p2_cb().set(FUNC(m52_soundc_audio_device::m6803_port2_r));
	cpu.out_p2_cb().set(FUNC(m52_soundc_audio_device::m6803_port2_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay_45M, XTAL(3'579'545)/4); /* verified on pcb */
	m_ay_45M->set_flags(AY8910_SINGLE_OUTPUT | AY8910_DISCRETE_OUTPUT);
	m_ay_45M->set_resistors_load(470, 0, 0);
	m_ay_45M->port_a_read_callback().set(FUNC(irem_audio_device::soundlatch_r));
	m_ay_45M->port_b_write_callback().set(FUNC(irem_audio_device::ay8910_45M_portb_w));
	m_ay_45M->add_route(0, "filtermix", 1.0, 0);

	AY8910(config, m_ay_45L, XTAL(3'579'545)/4); /* verified on pcb */
	m_ay_45L->set_flags(AY8910_SINGLE_OUTPUT | AY8910_DISCRETE_OUTPUT);
	m_ay_45L->set_resistors_load(470, 0, 0);
	m_ay_45L->port_a_write_callback().set(FUNC(irem_audio_device::ay8910_45L_porta_w));
	m_ay_45L->add_route(0, "filtermix", 1.0, 1);

	MSM5205(config, m_adpcm1, XTAL(384'000)); /* verified on pcb */
	m_adpcm1->vck_callback().set_inputline(m_cpu, INPUT_LINE_NMI); // driven through NPN inverter
	m_adpcm1->set_prescaler_selector(msm5205_device::S96_4B);      /* default to 4KHz, but can be changed at run time */
	m_adpcm1->add_route(0, "filtermix", 1.0, 2);

	DISCRETE(config, "filtermix", m52_sound_c_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void m52_large_audio_device::device_add_mconfig(machine_config &config)  /* 10 yard fight */
{
	/* basic machine hardware */
	m6803_cpu_device &cpu(M6803(config, m_cpu, XTAL(3'579'545))); /* verified on pcb */
	cpu.set_addrmap(AS_PROGRAM, &m52_large_audio_device::m52_large_sound_map);
	cpu.in_p1_cb().set(FUNC(m52_large_audio_device::m6803_port1_r));
	cpu.out_p1_cb().set(FUNC(m52_large_audio_device::m6803_port1_w));
	cpu.in_p2_cb().set(FUNC(m52_large_audio_device::m6803_port2_r));
	cpu.out_p2_cb().set(FUNC(m52_large_audio_device::m6803_port2_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay_45M, XTAL(3'579'545)/4); /* verified on pcb */
	m_ay_45M->set_flags(AY8910_SINGLE_OUTPUT | AY8910_DISCRETE_OUTPUT);
	m_ay_45M->set_resistors_load(470, 0, 0);
	m_ay_45M->port_a_read_callback().set(FUNC(irem_audio_device::soundlatch_r));
	m_ay_45M->port_b_write_callback().set(FUNC(irem_audio_device::ay8910_45M_portb_w));
	m_ay_45M->add_route(ALL_OUTPUTS, "mono", 0.80);

	AY8910(config, m_ay_45L, XTAL(3'579'545)/4); /* verified on pcb */
	m_ay_45L->set_flags(AY8910_SINGLE_OUTPUT | AY8910_DISCRETE_OUTPUT);
	m_ay_45L->set_resistors_load(470, 0, 0);
	m_ay_45L->port_a_write_callback().set(FUNC(irem_audio_device::ay8910_45L_porta_w));
	m_ay_45L->add_route(ALL_OUTPUTS, "mono", 0.80);

	MSM5205(config, m_adpcm1, 384_kHz_XTAL); // verified on PCB
	m_adpcm1->vck_callback().set_inputline(m_cpu, INPUT_LINE_NMI); // driven through NPN inverter
	m_adpcm1->vck_callback().append(m_adpcm2, FUNC(msm5205_device::vclk_w)); // the first MSM5205 clocks the second
	m_adpcm1->set_prescaler_selector(msm5205_device::S96_4B); // default to 4KHz, but can be changed at run time
	m_adpcm1->add_route(ALL_OUTPUTS, "mono", 0.80);

	MSM5205(config, m_adpcm2, 384_kHz_XTAL); // verified on PCB
	m_adpcm2->set_prescaler_selector(msm5205_device::SEX_4B); // default to 4KHz, but can be changed at run time, slave
	m_adpcm2->add_route(ALL_OUTPUTS, "mono", 0.80);
}
