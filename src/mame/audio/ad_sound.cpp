// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Nicola Salmoria
/***************************************************************************

    Functions to emulate the Alpha Denshi "59MC07" audio board

|----------------------|
| Fully boxed = socket |
|----------------------|


| separation = solder


-----------------------------------------------------------------------|
|ALPHA DENSHI CO, LTD. SOUND BOARD NO.59MC07                           |
|                                                                      |
|          |--------------------|                                      |
|   P | \/ |    JAPAN 84250C    | SN74LS232J | SN74LS74AN | DISSIPATOR |
|     | /\ |      M5l8085AP     | 8131BJ     | 8314A      |            |
|          |--------------------|                                      |
|                                                                      |
|   N                           | SN74LS74A                            |
|                               | 8122AG                               |
|                                                                      |
|     |------------|                                                   |
|   M | HV1VR OKI  | SN74LS138J | SN74LS74AJ | LM324N     | LM324N     |
|     | 2764-45    | 8044AG     | F8048      | J423A2     | 98509      |- (+12V)
|     |------------|                                                   |
|                                                                      |- (+5V)
|     |------------|                                                   |
|   L | HV2VR OKI  | T74LS14B1  | SN74LS08N                            |- (OUT)
|     | 2764-45    | 88442      | K8208                                |
|     |------------|                                                   |- (
|                                                                      |  (GND)
|     |------------|                                                   |- (
|   K | HV3VR OKI  | SN74LS08J  | SN74LS04N  | LM324N                  |
|     | 2764-45    | K8208      | I8313      | 98509                   |
|     |------------|                                                   |
|                                                                      |
|   J              | SN74LS74A  | SN74LS232J |            | LM324N     |
|                  | 8122AG     | 8131BG     |            | 436A       |
|                                                                      |
|     |------------|            |------------|                         |
|   H | HV4VR OKI  | SN74LS393N | TBP18S030N |            | LM324N     |
|     | 2764-45    | K8208      | J419X      |            | 436A       |
|     |------------|            |------------|                         |
|                                                                      |
|   F              | SN74LS138J | HCF40174BE |            | CD4066BCN  |
|                  | 8044AG     | MSM40174   |            | MM5666BN   |
|                                                                      |
|   E              | M74LS123P  | SN74LS138J | CD4066BCN  | CD4066BCN  |
|                  | 84A6C1     | 8044AG     | MM5666BN   | MM5666BN   |
|                                                           MUSIC      |
|                  |-------------------------|                         |
|   D | SN74LS373N |       JAPAN 841903      |            | CD4066BCN  |
|     | 2764-45    |         M5L8155P        |            | MM5666BN   |
|                  |-------------------------|              VOICE      |
|                                                                      |
|C                 |-------------------------|                         |
|O  C | 74LS173 PC |        MSM5232RS        |                         |
|N    | 8247       |        OKI 4342         |                         |
|N                 |-------------------------|                         |
|                                                                      |
|T  B                                                                  |
|O                                                                     |
|                  |-------------------------|                         |
|M  A | 74LS173 PC |     SOUND AY-3-8910     | CD4066BCN  | CD4066BCN  |
|A    | 8247       |        8046 CDA         | MM5666BN   | MM5666BN   |
|I                 |-------------------------|                         |
|N                                                                     |
|     | 1          | 2           | 3         | 4          | 5          |
-----------------------------------------------------------------------|

***************************************************************************/

#include "emu.h"
#include "audio/ad_sound.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "speaker.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(AD_59MC07, ad_59mc07_device, "ad_59mc07", "Alpha Denshi 59MC07 sound board")



//**************************************************************************
//  MEMORY MAPS
//**************************************************************************

void ad_59mc07_device::sound_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xc080, 0xc08d).w(m_msm, FUNC(msm5232_device::write));
	map(0xc0a0, 0xc0a1).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0xc0b0, 0xc0b0).nopw(); // n.c.
	map(0xc0c0, 0xc0c0).w(FUNC(ad_59mc07_device::cymbal_ctrl_w));
	map(0xc0d0, 0xc0d0).w(FUNC(ad_59mc07_device::dac_latch_w));  // followed by 1 (and usually 0) on 8155 port B
	map(0xc0e0, 0xc0e0).w(FUNC(ad_59mc07_device::dac_latch_w));  // followed by 2 (and usually 0) on 8155 port B
	map(0xc0f8, 0xc0ff).w(FUNC(ad_59mc07_device::c0f8_w));
	map(0xe000, 0xe0ff).rw(m_audio8155, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
}

void ad_59mc07_device::sound_portmap(address_map &map)
{
	map(0x00e0, 0x00e7).rw(m_audio8155, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}

//**************************************************************************
//  SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  ad_59mc07_device: Constructor
//-------------------------------------------------

ad_59mc07_device::ad_59mc07_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AD_59MC07, tag, owner, clock),
	m_audiocpu(*this, "audiocpu"),
	m_audio8155(*this, "audio8155"),
	m_samples(*this, "samples"),
	m_msm(*this, "msm"),
	m_dac_1(*this, "dac1"),
	m_dac_2(*this, "dac2"),
	m_soundlatch(*this, "soundlatch"),
	m_frq_adjuster(*this, ":FRQ") // TODO: this should be moved here, but I don't think it's possible to set the default per-game value with a variable
{}

// MSM5232 clock is generated by a transistor oscillator circuit, not by the pcb xtal
// The circuit is controlled by a pot to allow adjusting the frequency.  All games
// have been hand-tuned to a recording claiming to be an original recording from the
// boards.  You can adjust this in the Slider Controls while using -cheat if needed.

#define MSM5232_MAX_CLOCK 6144000
#define MSM5232_MIN_CLOCK  214000   // unstable



/******************************************************************************/
// Sound

WRITE_LINE_MEMBER(ad_59mc07_device::i8155_timer_pulse)
{
	if (!state)
		m_audiocpu->set_input_line(I8085_TRAP_LINE, ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(ad_59mc07_device::frq_adjuster_callback)
{
	uint8_t frq = m_frq_adjuster->read();

	m_msm->set_clock(MSM5232_MIN_CLOCK + frq * (MSM5232_MAX_CLOCK - MSM5232_MIN_CLOCK) / 100);
//popmessage("8155: C %02x A %02x  AY: A %02x B %02x Unk:%x", m_8155_port_c, m_8155_port_a, m_ay_port_a, m_ay_port_b, m_cymbal_ctrl & 15);

	m_cymvol *= 0.94f;
	m_hihatvol *= 0.94f;

	m_msm->set_output_gain(10, m_hihatvol + m_cymvol * (m_ay_port_b & 3) * 0.33f);   /* NO from msm5232 */
}

void ad_59mc07_device::c0f8_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // c0f8: NMI ack (written by NMI handler)
			m_audiocpu->set_input_line(I8085_TRAP_LINE, CLEAR_LINE);
			break;

		case 1: // c0f9: RST75 trigger (written by NMI handler)
			// Note: solder pad CP3 on the pcb would allow to disable this
			m_audiocpu->pulse_input_line(I8085_RST75_LINE, attotime::zero);
			break;

		case 2: // c0fa: INTR trigger (written by NMI handler)
			// verified on PCB:
			m_audiocpu->set_input_line(I8085_INTR_LINE, HOLD_LINE);
			break;

		case 3: // c0fb: n.c.
			// Note: this is written at end of the INTR handler, however LS138@E3 pin 12 is definitely unconnected
			break;

		case 4: // c0fc: increment PROM address (written by NMI handler)
			m_sound_prom_address = (m_sound_prom_address + 1) & 0x1f;
//       at this point, the 5-bit value
//       goes to an op-amp and to the base of a transistor. The transistor is part
//       of a resonator that is used to generate the M5232 clock. The PROM doesn't
//       actually seem to be important, since even removing it the M5232 clock
//       continues to come out normally.
			break;

		case 5: // c0fd: n.c.
			// Note: this is written at end of the RST75 handler, however LS138@E3 pin 10 is definitely unconnected
			break;

		case 6: // c0fe: 4-bit answer for main CPU (unused)
			// Note: this would go to the LS173@B1, which is however unpopulated on the pcb
			break;

		case 7: // c0ff: sound command latch clear
			// Note: solder pad CP1 on the pcb would allow to disable this
			m_soundlatch->clear_w();
			break;
	}
}


void ad_59mc07_device::ay8910_porta_w(uint8_t data)
{
	// bongo 1
	m_samples->set_volume(0, ((data & 0x30) >> 4) * 0.33);
	if (data & ~m_ay_port_a & 0x80)
		m_samples->start(0, 0);

	// bongo 2
	m_samples->set_volume(1, (data & 0x03) * 0.33);
	if (data & ~m_ay_port_a & 0x08)
		m_samples->start(1, 1);

	m_ay_port_a = data;
}

void ad_59mc07_device::ay8910_portb_w(uint8_t data)
{
	// bongo 3
	m_samples->set_volume(2, ((data & 0x30)>>4) * 0.33);
	if (data & ~m_ay_port_b & 0x80)
		m_samples->start(2, 2);

	// FIXME I'm just enabling the MSM5232 Noise Output for now. Proper emulation
	// of the analog circuitry should be done instead.
//  if (data & ~m_ay_port_b & 0x08)   cymbal hit trigger
//  if (data & ~m_ay_port_b & 0x04)   hi-hat hit trigger
//  data & 3   cymbal volume
//  data & 0x40  hi-hat enable

	if (data & ~m_ay_port_b & 0x08)
		m_cymvol = 1.0f;

	if (data & ~m_ay_port_b & 0x04)
		m_hihatvol = 0.8f;

	if (~data & 0x40)
		m_hihatvol = 0.0f;

	m_ay_port_b = data;
}

void ad_59mc07_device::cymbal_ctrl_w(uint8_t data)
{
	m_cymbal_ctrl++;
}


void ad_59mc07_device::update_dac()
{
	// there is only one latch, which is used to drive two DAC channels.
	// When the channel is enabled in the 4066, it goes to a series of
	// low-pass filters. The channel is kept enabled only for a short time,
	// then it's disabled again.
	// Note that PB0 goes through three filters while PB1 only goes through one.

	if (m_8155_port_b & 1)
		m_dac_1->write(m_dac_latch);

	if (m_8155_port_b & 2)
		m_dac_2->write(m_dac_latch);
}

void ad_59mc07_device::dac_latch_w(uint8_t data)
{
	m_dac_latch = data;
	update_dac();
}

void ad_59mc07_device::i8155_porta_w(uint8_t data)
{
	m_8155_port_a = data;
	m_msm->set_output_gain(0, (data >> 4) / 15.0);   // group1 from msm5232
	m_msm->set_output_gain(1, (data >> 4) / 15.0);   // group1 from msm5232
	m_msm->set_output_gain(2, (data >> 4) / 15.0);   // group1 from msm5232
	m_msm->set_output_gain(3, (data >> 4) / 15.0);   // group1 from msm5232
	m_msm->set_output_gain(4, (data & 0x0f) / 15.0); // group2 from msm5232
	m_msm->set_output_gain(5, (data & 0x0f) / 15.0); // group2 from msm5232
	m_msm->set_output_gain(6, (data & 0x0f) / 15.0); // group2 from msm5232
	m_msm->set_output_gain(7, (data & 0x0f) / 15.0); // group2 from msm5232
}

void ad_59mc07_device::i8155_portb_w(uint8_t data)
{
	m_8155_port_b = data;
	update_dac();
}

void ad_59mc07_device::i8155_portc_w(uint8_t data)
{
	m_8155_port_c = data;
	m_msm->set_output_gain(8, (data & 0x0f) / 15.0);     // SOLO  8' from msm5232
	if (data & 0x20)
		m_msm->set_output_gain(9, (data & 0x0f) / 15.0); // SOLO 16' from msm5232
	else
		m_msm->set_output_gain(9, 0);   // SOLO 16' from msm5232
}

WRITE_LINE_MEMBER(ad_59mc07_device::msm5232_gate)
{
}

static const char *const alphamc07_sample_names[] =
{
	"*equites",
	"bongo1",
	"bongo2",
	"bongo3",
	nullptr
};


#define MSM5232_BASE_VOLUME 0.7

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ad_59mc07_device::device_add_mconfig(machine_config &config)
{
	I8085A(config, m_audiocpu, 6.144_MHz_XTAL); // verified on pcb
	m_audiocpu->set_addrmap(AS_PROGRAM, &ad_59mc07_device::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &ad_59mc07_device::sound_portmap);
	m_audiocpu->set_clk_out(m_audio8155, FUNC(i8155_device::set_unscaled_clock_int));

	I8155(config, m_audio8155, 0);
	m_audio8155->out_pa_callback().set(FUNC(ad_59mc07_device::i8155_porta_w));
	m_audio8155->out_pb_callback().set(FUNC(ad_59mc07_device::i8155_portb_w));
	m_audio8155->out_pc_callback().set(FUNC(ad_59mc07_device::i8155_portc_w));
	m_audio8155->out_to_callback().set(FUNC(ad_59mc07_device::i8155_timer_pulse));

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	MSM5232(config, m_msm, MSM5232_MAX_CLOCK);   // will be adjusted at runtime through PORT_ADJUSTER
	m_msm->set_capacitors(0.47e-6, 0.47e-6, 0.47e-6, 0.47e-6, 0.47e-6, 0.47e-6, 0.47e-6, 0.47e-6); // verified
	m_msm->gate().set(FUNC(ad_59mc07_device::msm5232_gate));
	m_msm->add_route(0, "speaker", MSM5232_BASE_VOLUME/2.2);   // pin 28  2'-1 : 22k resistor
	m_msm->add_route(1, "speaker", MSM5232_BASE_VOLUME/1.5);   // pin 29  4'-1 : 15k resistor
	m_msm->add_route(2, "speaker", MSM5232_BASE_VOLUME);       // pin 30  8'-1 : 10k resistor
	m_msm->add_route(3, "speaker", MSM5232_BASE_VOLUME);       // pin 31 16'-1 : 10k resistor
	m_msm->add_route(4, "speaker", MSM5232_BASE_VOLUME/2.2);   // pin 36  2'-2 : 22k resistor
	m_msm->add_route(5, "speaker", MSM5232_BASE_VOLUME/1.5);   // pin 35  4'-2 : 15k resistor
	m_msm->add_route(6, "speaker", MSM5232_BASE_VOLUME);       // pin 34  8'-2 : 10k resistor
	m_msm->add_route(7, "speaker", MSM5232_BASE_VOLUME);       // pin 33 16'-2 : 10k resistor
	m_msm->add_route(8, "speaker", 0.7);       // pin 1 SOLO  8' (this actually feeds an analog section)
	m_msm->add_route(9, "speaker", 0.7);       // pin 2 SOLO 16' (this actually feeds an analog section)
	m_msm->add_route(10,"speaker", 0.084);     // pin 22 Noise Output (this actually feeds an analog section)

	ay8910_device &aysnd(AY8910(config, "aysnd", 6.144_MHz_XTAL/4)); // verified on pcb
	aysnd.port_a_write_callback().set(FUNC(ad_59mc07_device::ay8910_porta_w));
	aysnd.port_b_write_callback().set(FUNC(ad_59mc07_device::ay8910_portb_w));
	aysnd.add_route(ALL_OUTPUTS, "speaker", 0.105);

	DAC_6BIT_R2R(config, m_dac_1, 0).add_route(ALL_OUTPUTS, "speaker", 0.35); // unknown DAC
	DAC_6BIT_R2R(config, m_dac_2, 0).add_route(ALL_OUTPUTS, "speaker", 0.35); // unknown DAC

	SAMPLES(config, m_samples);
	m_samples->set_channels(3);
	m_samples->set_samples_names(alphamc07_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.21);
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void ad_59mc07_device::device_start()
{
	// zerofill
	m_sound_prom_address = 0;
	m_dac_latch = 0;
	m_8155_port_b = 0;
	m_8155_port_a = 0;
	m_8155_port_c = 0;
	m_ay_port_a = 0;
	m_ay_port_b = 0;
	m_cymbal_ctrl = 0;
	m_cymvol = 0.0;
	m_hihatvol = 0.0;

	// register for savestates
	save_item(NAME(m_sound_prom_address));
	save_item(NAME(m_dac_latch));
	save_item(NAME(m_8155_port_b));
	save_item(NAME(m_8155_port_a));
	save_item(NAME(m_8155_port_c));
	save_item(NAME(m_ay_port_a));
	save_item(NAME(m_ay_port_b));
	save_item(NAME(m_cymbal_ctrl));
	save_item(NAME(m_cymvol));
	save_item(NAME(m_hihatvol));

	m_adjuster_timer = timer_alloc(FUNC(ad_59mc07_device::frq_adjuster_callback), this);
	m_adjuster_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}


//-------------------------------------------------
//  device_reset: Reset the device
//-------------------------------------------------

void ad_59mc07_device::device_reset()
{
	m_audiocpu->set_input_line(I8085_INTR_LINE, CLEAR_LINE);
	m_audiocpu->set_input_line(I8085_TRAP_LINE, CLEAR_LINE);
}


/*
    Functions to emulate the Alpha Denshi "60MC01" audio board

    CPU  : Z80A
    Sound: AY-3-8910A (unpopulated: another 8910 and a YM2203)
    OSC  : 16.000MHz

    TODO: This bears a lot of similarities with the Super Stingray audio board. Verify if PCB codes match and if so merge implementations;
          fix interrupts;
          is there really no music?
*/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(AD_60MC01, ad_60mc01_device, "ad_60mc01", "Alpha Denshi 60MC01 sound board")



//**************************************************************************
//  MEMORY MAPS
//**************************************************************************

void ad_60mc01_device::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xc100, 0xc100).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xc102, 0xc102).w(m_soundlatch, FUNC(generic_latch_8_device::clear_w));
	map(0xc104, 0xc104).nopw(); // written at start up, would be DAC if it were populated
	map(0xc106, 0xc10e).nopw(); // written continuously, it's audio board I/O according to Super Stingray's emulation
}

void ad_60mc01_device::sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x11).nopw(); // written at start up, would be the YM2203 if it were populated
	map(0x80, 0x81).w("aysnd", FUNC(ay8910_device::data_address_w));
}

//**************************************************************************
//  SPECIFIC IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  ad_60mc01_device: Constructor
//-------------------------------------------------

ad_60mc01_device::ad_60mc01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AD_60MC01, tag, owner, clock),
	m_audiocpu(*this, "audiocpu"),
	m_soundlatch(*this, "soundlatch")
{}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ad_60mc01_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_audiocpu, 16_MHz_XTAL / 4); // divider not verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &ad_60mc01_device::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &ad_60mc01_device::sound_portmap);
	m_audiocpu->set_periodic_int(FUNC(ad_60mc01_device::sound_irq), attotime::from_hz(128));

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ay8910_device &aysnd(AY8910(config, "aysnd", 16_MHz_XTAL / 8)); // divider not verified
	aysnd.add_route(ALL_OUTPUTS, "speaker", 0.50);
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void ad_60mc01_device::device_start()
{
}


//-------------------------------------------------
//  device_reset: Reset the device
//-------------------------------------------------

void ad_60mc01_device::device_reset()
{
}
