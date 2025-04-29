// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    midway.cpp

    Functions to emulate general the various Midway sound cards.

    TODO: the "Turbo Cheap Squeak" and "Sounds Good" boards are nearly identical
      in function, but use different CPUs and Memory maps (The PIA hookup, DAC
      and Filter are all exactly the same), so these should probably be combined
      into one base class with two subclass device implementations to reduce
      duplicated code.
      The "Cheap Squeak Deluxe" board in /audio/csd.cpp is also almost identical
      to the "Sounds Good" board, and should also be a member of any future
      combined class/device implementation.

***************************************************************************/

#include "emu.h"
#include "mcr.h"
#include "midway.h"

#include "williamssound.h"

#include <algorithm>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MIDWAY_SSIO,               midway_ssio_device,               "midssio", "Midway SSIO Sound Board")
DEFINE_DEVICE_TYPE(MIDWAY_SOUNDS_GOOD,        midway_sounds_good_device,        "midsg",   "Midway Sounds Good Sound Board")
DEFINE_DEVICE_TYPE(MIDWAY_TURBO_CHEAP_SQUEAK, midway_turbo_cheap_squeak_device, "midtcs",  "Midway Turbo Cheap Squeak Sound Board")


//**************************************************************************
//  SUPER SOUND I/O BOARD (SSIO)
//**************************************************************************

//-------------------------------------------------
//  midway_ssio_device - constructor
//-------------------------------------------------

midway_ssio_device::midway_ssio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDWAY_SSIO, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_ay0(*this, "ay0")
	, m_ay1(*this, "ay1")
	, m_ports(*this, "IP%u", 0U)
	, m_status(0)
	, m_14024_count(0)
	, m_mute(0)
	, m_custom_input_mask{ 0U, 0U, 0U, 0U, 0U }
	, m_custom_input(*this)
	, m_custom_output_mask{ 0U, 0U }
	, m_custom_output(*this)
{
	std::fill(std::begin(m_data), std::end(m_data), 0);
	std::fill(std::begin(m_overall), std::end(m_overall), 0);
	for (auto &duty_cycle : m_duty_cycle)
		std::fill(std::begin(duty_cycle), std::end(duty_cycle), 0);
	std::fill(std::begin(m_ayvolume_lookup), std::end(m_ayvolume_lookup), 0);
}


//-------------------------------------------------
//  suspend_cpu
//-------------------------------------------------

void midway_ssio_device::suspend_cpu()
{
	m_cpu->suspend(SUSPEND_REASON_DISABLE, 1);
}

//-------------------------------------------------
//  read - return the status value
//-------------------------------------------------

uint8_t midway_ssio_device::read()
{
	return m_status;
}


//-------------------------------------------------
//  write - handle an external write to one of the
//  input latches
//-------------------------------------------------

void midway_ssio_device::write(offs_t offset, uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(midway_ssio_device::synced_write), this), (offset << 8) | (data & 0xff));
}


//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

void midway_ssio_device::reset_write(int state)
{
	if (state)
	{
		// going high halts the CPU
		device_reset();
		m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
	else
	{
		// going low resets and reactivates the CPU
		m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
}


//-------------------------------------------------
//  ioport_read - read from one of the I/O ports
//  on the device
//-------------------------------------------------

uint8_t midway_ssio_device::ioport_read(offs_t offset)
{
	uint8_t result = m_ports[offset].read_safe(0xff);
	if (!m_custom_input[offset].isnull())
		result = (result & ~m_custom_input_mask[offset]) |
					(m_custom_input[offset]() & m_custom_input_mask[offset]);
	return result;
}


//-------------------------------------------------
//  ioport_write - write to one of the I/O ports
//  on the device
//-------------------------------------------------

void midway_ssio_device::ioport_write(offs_t offset, uint8_t data)
{
	int which = offset >> 2;
	if (!m_custom_output[which].isnull())
		m_custom_output[which](data & m_custom_output_mask[which]);
}


//-------------------------------------------------
//  compute_ay8910_modulation - precompute
//  volume modulation tables based on the duty
//  cycle described by the PROMs
//-------------------------------------------------

void midway_ssio_device::compute_ay8910_modulation()
{
	//
	// AY-8910 modulation:
	//
	// Starts with a 16MHz oscillator
	//  /2 via 7474 flip-flip @ F11
	//
	// This signal clocks the binary counter @ E11 which
	// cascades into the decade counter @ D11. This combo
	// effectively counts from 0-159 and then wraps. The
	// value from these counters is input to an 82S123 PROM,
	// which appears to be standard on all games.
	//
	// One bit at a time from this PROM is clocked at a time
	// and the resulting inverted signal becomes a clock for
	// the down counters at F3, F4, F5, F8, F9, and F10. The
	// value in these down counters are reloaded after the 160
	// counts from the binary/decade counter combination.
	//
	// When these down counters are loaded, the TC signal is
	// clear, which mutes the voice. When the down counters
	// cross through 0, the TC signal goes high and the 4016
	// multiplexers allow the AY-8910 voice to go through.
	// Thus, writing a 0 to the counters will enable the
	// voice for the longest period of time, while writing
	// a 15 enables it for the shortest period of time.
	// This creates an effective duty cycle for the voice.
	//
	// Given that the down counters are reset 50000 times per
	// second (SSIO_CLOCK/2/160), which is above the typical
	// frequency of sound output. So we simply apply a volume
	// adjustment to each voice according to the duty cycle.
	//

	// loop over all possible values of the duty cycle
	uint8_t *prom = memregion("proms")->base();
	for (int volval = 0; volval < 16; volval++)
	{
		// loop over all the clocks until we run out; look up in the PROM
		// to find out when the next clock should fire
		int remaining_clocks = volval;
		int cur = 0, prev = 1;
		int curclock;
		for (curclock = 0; curclock < 160 && remaining_clocks; curclock++)
		{
			cur = prom[curclock / 8] & (0x80 >> (curclock % 8));

			// check for a high -> low transition
			if (cur == 0 && prev != 0)
				remaining_clocks--;

			prev = cur;
		}

		// treat the duty cycle as a volume
		m_ayvolume_lookup[15 - volval] = curclock * 100 / 160;
	}
}


//-------------------------------------------------
//  clock_14024 - periodic timer to clock the
//  7-bit async counter at C12
//-------------------------------------------------

INTERRUPT_GEN_MEMBER(midway_ssio_device::clock_14024)
{
	//
	//  /SINT is generated as follows:
	//
	//  Starts with a 16MHz oscillator
	//      /2 via 7474 flip-flop @ F11
	//      /16 via 74161 binary counter @ E11
	//      /10 via 74190 decade counter @ D11
	//
	//  Bit 3 of the decade counter clocks a 14024 7-bit async counter @ C12.
	//  This routine is called to clock this 7-bit counter.
	//  Bit 6 of the output is inverted and connected to /SINT.
	//
	m_14024_count = (m_14024_count + 1) & 0x7f;

	// if the low 5 bits clocked to 0, bit 6 has changed state
	if ((m_14024_count & 0x3f) == 0)
		m_cpu->set_input_line(0, (m_14024_count & 0x40) ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  irq_clear - reset the IRQ state and 14024 count
//-------------------------------------------------

uint8_t midway_ssio_device::irq_clear()
{
	// a read here asynchronously resets the 14024 count, clearing /SINT
	if (!machine().side_effects_disabled())
	{
		m_14024_count = 0;
		m_cpu->set_input_line(0, CLEAR_LINE);
	}
	return 0xff;
}


//-------------------------------------------------
//  status_w - set the outgoing status value
//-------------------------------------------------

void midway_ssio_device::status_w(uint8_t data)
{
	m_status = data;
}


//-------------------------------------------------
//  data_r - read incoming data latches
//-------------------------------------------------

uint8_t midway_ssio_device::data_r(offs_t offset)
{
	return m_data[offset];
}


//-------------------------------------------------
//  porta0_w - handle writes to AY-8910 #0 port A
//-------------------------------------------------

void midway_ssio_device::porta0_w(uint8_t data)
{
	m_duty_cycle[0][0] = data & 15;
	m_duty_cycle[0][1] = data >> 4;
	update_volumes();
}


//-------------------------------------------------
//  portb0_w - handle writes to AY-8910 #0 port B
//-------------------------------------------------

void midway_ssio_device::portb0_w(uint8_t data)
{
	m_duty_cycle[0][2] = data & 15;
	m_overall[0] = (data >> 4) & 7;
	update_volumes();
}


//-------------------------------------------------
//  porta1_w - handle writes to AY-8910 #1 port A
//-------------------------------------------------

void midway_ssio_device::porta1_w(uint8_t data)
{
	m_duty_cycle[1][0] = data & 15;
	m_duty_cycle[1][1] = data >> 4;
	update_volumes();
}


//-------------------------------------------------
//  portb1_w - handle writes to AY-8910 #1 port B
//-------------------------------------------------

void midway_ssio_device::portb1_w(uint8_t data)
{
	m_duty_cycle[1][2] = data & 15;
	m_overall[1] = (data >> 4) & 7;
	m_mute = data & 0x80;
	update_volumes();
}


//-------------------------------------------------
//  update_volumes - update the volumes of each
//  AY-8910 channel based on modulation and mute
//-------------------------------------------------

void midway_ssio_device::update_volumes()
{
	m_ay0->set_volume(0, m_mute ? 0 : m_ayvolume_lookup[m_duty_cycle[0][0]]);
	m_ay0->set_volume(1, m_mute ? 0 : m_ayvolume_lookup[m_duty_cycle[0][1]]);
	m_ay0->set_volume(2, m_mute ? 0 : m_ayvolume_lookup[m_duty_cycle[0][2]]);
	m_ay1->set_volume(0, m_mute ? 0 : m_ayvolume_lookup[m_duty_cycle[1][0]]);
	m_ay1->set_volume(1, m_mute ? 0 : m_ayvolume_lookup[m_duty_cycle[1][1]]);
	m_ay1->set_volume(2, m_mute ? 0 : m_ayvolume_lookup[m_duty_cycle[1][2]]);
}

//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

// address map verified from schematics
void midway_ssio_device::ssio_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x83ff).mirror(0x0c00).ram();
	map(0x9000, 0x9003).mirror(0x0ffc).r(FUNC(midway_ssio_device::data_r));
	map(0xa000, 0xa000).mirror(0x0ffc).w("ay0", FUNC(ay8910_device::address_w));
	map(0xa001, 0xa001).mirror(0x0ffc).r("ay0", FUNC(ay8910_device::data_r));
	map(0xa002, 0xa002).mirror(0x0ffc).w("ay0", FUNC(ay8910_device::data_w));
	map(0xb000, 0xb000).mirror(0x0ffc).w("ay1", FUNC(ay8910_device::address_w));
	map(0xb001, 0xb001).mirror(0x0ffc).r("ay1", FUNC(ay8910_device::data_r));
	map(0xb002, 0xb002).mirror(0x0ffc).w("ay1", FUNC(ay8910_device::data_w));
	map(0xc000, 0xcfff).nopr().w(FUNC(midway_ssio_device::status_w));
	map(0xd000, 0xdfff).nopw();    // low bit controls yellow LED
	map(0xe000, 0xefff).r(FUNC(midway_ssio_device::irq_clear));
	map(0xf000, 0xffff).portr("DIP");    // 6 DIP switches
}


//-------------------------------------------------
//  default ports map
//-------------------------------------------------

void midway_ssio_device::ssio_input_ports(address_map &map, const char *ssio)
{
	map(0x00, 0x04).mirror(0x18).r(ssio, FUNC(midway_ssio_device::ioport_read));
	map(0x07, 0x07).mirror(0x18).r(ssio, FUNC(midway_ssio_device::read));
	map(0x00, 0x07).w(ssio, FUNC(midway_ssio_device::ioport_write));
	map(0x1c, 0x1f).w(ssio, FUNC(midway_ssio_device::write));
}


//-------------------------------------------------
//  ROM definitions
//-------------------------------------------------

ROM_START( midway_ssio )
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.12d",   0x0000, 0x0020, CRC(e1281ee9) SHA1(9ac9b01d24affc0ee9227a4364c4fd8f8290343a) )    /* from shollow, assuming it's the same */
ROM_END


//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const tiny_rom_entry *midway_ssio_device::device_rom_region() const
{
	return ROM_NAME(midway_ssio);
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void midway_ssio_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, DERIVED_CLOCK(1, 2*4));
	m_cpu->set_addrmap(AS_PROGRAM, &midway_ssio_device::ssio_map);
	if (clock())
		m_cpu->set_periodic_int(DEVICE_SELF, FUNC(midway_ssio_device::clock_14024), attotime::from_hz(clock() / (2*16*10)));

	AY8910(config, m_ay0, DERIVED_CLOCK(1, 2*4));
	m_ay0->port_a_write_callback().set(FUNC(midway_ssio_device::porta0_w));
	m_ay0->port_b_write_callback().set(FUNC(midway_ssio_device::portb0_w));
	m_ay0->add_route(ALL_OUTPUTS, *this, 0.33, 0);

	AY8910(config, m_ay1, DERIVED_CLOCK(1, 2*4));
	m_ay1->port_a_write_callback().set(FUNC(midway_ssio_device::porta1_w));
	m_ay1->port_b_write_callback().set(FUNC(midway_ssio_device::portb1_w));
	m_ay1->add_route(ALL_OUTPUTS, *this, 0.33, 1);
}


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor midway_ssio_device::device_input_ports() const
{
	return nullptr;
//  return INPUT_PORTS_NAME( midway_ssio );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void midway_ssio_device::device_start()
{
	compute_ay8910_modulation();
	save_item(NAME(m_data));
	save_item(NAME(m_status));
	save_item(NAME(m_14024_count));
	save_item(NAME(m_mute));
	save_item(NAME(m_overall));
	save_item(NAME(m_duty_cycle));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void midway_ssio_device::device_reset()
{
	// latches also get reset
	memset(m_data, 0, sizeof(m_data));
	m_status = 0;
	m_14024_count = 0;
}


//-------------------------------------------------
//  synced_write
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(midway_ssio_device::synced_write)
{
	m_data[param >> 8] = param & 0xff;
}



//**************************************************************************
//  SOUNDS GOOD BOARD
//**************************************************************************

//-------------------------------------------------
//  midway_sounds_good_device - constructor
//-------------------------------------------------

midway_sounds_good_device::midway_sounds_good_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDWAY_SOUNDS_GOOD, tag, owner, clock)
		, device_mixer_interface(mconfig, *this)
		, m_cpu(*this, "cpu")
		, m_pia(*this, "pia")
		, m_dac(*this, "dac")
		, m_dac_filter(*this, "dac_filter%u", 0U)
		, m_status(0)
		, m_dacval(0)
{
}


//-------------------------------------------------
//  read - return the status value
//-------------------------------------------------

uint8_t midway_sounds_good_device::read()
{
	return m_status;
}


//-------------------------------------------------
//  write - handle an external write to the input
//  latch
//-------------------------------------------------

void midway_sounds_good_device::write(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(midway_sounds_good_device::synced_write), this), data);
}


//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

void midway_sounds_good_device::reset_write(int state)
{
//if (state) osd_printf_debug("SG Reset\n");
	m_cpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  porta_w - PIA port A writes
//-------------------------------------------------

void midway_sounds_good_device::porta_w(uint8_t data)
{
	m_dacval = (data << 2) | (m_dacval & 3);
	m_dac->write(m_dacval);
}


//-------------------------------------------------
//  portb_w - PIA port B writes
//-------------------------------------------------

void midway_sounds_good_device::portb_w(uint8_t data)
{
	uint8_t z_mask = m_pia->port_b_z_mask();

	m_dacval = (m_dacval & ~3) | (data >> 6);
	m_dac->write(m_dacval);

	if (~z_mask & 0x10)  m_status = (m_status & ~1) | ((data >> 4) & 1);
	if (~z_mask & 0x20)  m_status = (m_status & ~2) | ((data >> 4) & 2);
}


//-------------------------------------------------
//  irq_w - IRQ line state changes
//-------------------------------------------------

void midway_sounds_good_device::irq_w(int state)
{
	int combined_state = m_pia->irq_a_state() | m_pia->irq_b_state();
	m_cpu->set_input_line(4, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

// address map determined by PAL20L10 @U15; not dumped/verified yet
//Address map (x = ignored; * = selects address within this range)
//68k address map known for certain:
//a23 a22 a21 a20 a19 a18 a17 a16 a15 a14 a13 a12 a11 a10 a9  a8  a7  a6  a5  a4  a3  a2  a1  (a0 via UDS/LDS)
//x   x   x   x   x   *   *   *   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?       RW PAL@
//BEGIN 68k map guesses (until we have a pal dump):
//x   x   x   x   x   0  [a] [b]  *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   ->      R   ROM U7, U17
//x   x   x   x   x   0  [c] [d]  *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   ->      R   ROM U8, U18
//x   x   x   x   x   1   1   0   x   x   x   x   x   x   x   x   x   x   x   x   0  *e* *f*  1       RW  PIA U9 (e goes to PIA RS0, f goes to PIA RS1, D8-D15 go to PIA D0-D7)
//x   x   x   x   x   1   1   1   x   x   x   x   *   *   *   *   *   *   *   *   *   *   *   ?       RW  RAM U6, U16
//The ROMTYPE PAL line, is pulled low if JW1 is present, and this presumably controls [a] [b] [c] [d] as such:
//When ROMTYPE is HIGH/JW1 absent, 27256 roms: (JW3 should be present, JW2 absent)
//x   x   x   x   x   0   0?  0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   ->      R   ROM U7, U17
//x   x   x   x   x   0   0?  1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   ->      R   ROM U8, U18
//When ROMTYPE is LOW/JW1 present, 27512 roms: (JW3 should be absent, JW2 present)
//x   x   x   x   x   0   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   ->      R   ROM U7, U17
//x   x   x   x   x   0   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   ->      R   ROM U8, U18
//JW2 and JW3 are mutually exclusive, and control whether 68k a16 connects to the EPROM pin 1 line (JW2 present), or whether it is tied high (JW3 present).
//EPROM pin 1 is A15 on 27512, and VPP(usually an active high enable) on 27256
//END guesses



void midway_sounds_good_device::soundsgood_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7ffff);
	map(0x000000, 0x03ffff).rom();
	map(0x060000, 0x060007).mirror(0x00fff0).rw(m_pia, FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt)).umask16(0xff00); // RS0 connects to A2, RS1 connects to A1, hence the "alt" functions are used.
	map(0x070000, 0x070fff).mirror(0x00f000).ram();
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void midway_sounds_good_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_cpu, DERIVED_CLOCK(1, 2));
	m_cpu->set_addrmap(AS_PROGRAM, &midway_sounds_good_device::soundsgood_map);

	PIA6821(config, m_pia);
	m_pia->writepa_handler().set(FUNC(midway_sounds_good_device::porta_w));
	m_pia->writepb_handler().set(FUNC(midway_sounds_good_device::portb_w));
	m_pia->irqa_handler().set(FUNC(midway_sounds_good_device::irq_w));
	m_pia->irqb_handler().set(FUNC(midway_sounds_good_device::irq_w));

	AD7533(config, m_dac); /// ad7533jn.u10

	// The DAC filters here are identical to those on the "Turbo Cheap Squeak" and "Cheap Squeak Deluxe" boards.
	//LM359 @U2.2, 2nd order MFB low-pass (fc = 5404.717733, Q = 0.625210, gain = -1.000000)
	FILTER_BIQUAD(config, m_dac_filter[2]).opamp_mfb_lowpass_setup(RES_K(150), RES_K(82), RES_K(150), CAP_P(470), CAP_P(150)); // R115, R109, R108, C112, C111
	m_dac_filter[2]->add_route(ALL_OUTPUTS, *this, 1.0);
	//LM359 @U3.2, 2nd order MFB low-pass (fc = 5310.690763, Q = 1.608630, gain = -1.000000)
	FILTER_BIQUAD(config, m_dac_filter[1]).opamp_mfb_lowpass_setup(RES_K(33), RES_K(18), RES_K(33), CAP_P(5600), CAP_P(270)); // R113, R117, R116, C115, C118
	m_dac_filter[1]->add_route(ALL_OUTPUTS, m_dac_filter[2], 1.0);
	//LM359 @U3.1, 1st order MFB low-pass (fc = 4912.189602, Q = 0.707107(ignored), gain = -1.000000)
	FILTER_BIQUAD(config, m_dac_filter[0]).opamp_mfb_lowpass_setup(RES_K(120), RES_K(0), RES_K(120), CAP_P(0), CAP_P(270)); // R112, <short>, R111, <nonexistent>, C113
	m_dac_filter[0]->add_route(ALL_OUTPUTS, m_dac_filter[1], 1.0);
	m_dac->add_route(ALL_OUTPUTS, m_dac_filter[0], 1.0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void midway_sounds_good_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_dacval));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void midway_sounds_good_device::device_reset()
{
}


//-------------------------------------------------
//  synced_write
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(midway_sounds_good_device::synced_write)
{
	m_pia->portb_w((param >> 1) & 0x0f);
	m_pia->ca1_w(~param & 0x01);

	// oftentimes games will write one nibble at a time; the sync on this is very
	// important, so we boost the interleave briefly while this happens
	machine().scheduler().perfect_quantum(attotime::from_usec(250));
}



//**************************************************************************
//  TURBO CHEAP SQUEAK BOARD
//**************************************************************************

//-------------------------------------------------
//  midway_turbo_cheap_squeak_device - constructor
//-------------------------------------------------

midway_turbo_cheap_squeak_device::midway_turbo_cheap_squeak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDWAY_TURBO_CHEAP_SQUEAK, tag, owner, clock)
		, device_mixer_interface(mconfig, *this)
		, m_cpu(*this, "cpu")
		, m_pia(*this, "pia")
		, m_dac(*this, "dac")
		, m_dac_filter(*this, "dac_filter%u", 0U)
		, m_status(0)
		, m_dacval(0)
{
}


//-------------------------------------------------
//  read - return the status value
//-------------------------------------------------

uint8_t midway_turbo_cheap_squeak_device::read()
{
	return m_status;
}


//-------------------------------------------------
//  write - handle an external write to the input
//  latch
//-------------------------------------------------

void midway_turbo_cheap_squeak_device::write(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(midway_turbo_cheap_squeak_device::synced_write), this), data);
}


//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

void midway_turbo_cheap_squeak_device::reset_write(int state)
{
	m_cpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  porta_w - PIA port A writes
//-------------------------------------------------

void midway_turbo_cheap_squeak_device::porta_w(uint8_t data)
{
	m_dacval = (data << 2) | (m_dacval & 3);
	m_dac->write(m_dacval);
}


//-------------------------------------------------
//  portb_w - PIA port B writes
//-------------------------------------------------

void midway_turbo_cheap_squeak_device::portb_w(uint8_t data)
{
	m_dacval = (m_dacval & ~3) | (data >> 6);
	m_dac->write(m_dacval);
	m_status = (data >> 4) & 3;
}


//-------------------------------------------------
//  irq_w - IRQ line state changes
//-------------------------------------------------

void midway_turbo_cheap_squeak_device::irq_w(int state)
{
	int combined_state = m_pia->irq_a_state() | m_pia->irq_b_state();
	m_cpu->set_input_line(M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

// address map verified from schematics
void midway_turbo_cheap_squeak_device::turbocs_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).mirror(0x3800).ram();
	map(0x4000, 0x4003).mirror(0x3ffc).rw("pia", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0x8000, 0xffff).rom();
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void midway_turbo_cheap_squeak_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_cpu, DERIVED_CLOCK(1, 4));
	m_cpu->set_addrmap(AS_PROGRAM, &midway_turbo_cheap_squeak_device::turbocs_map);

	PIA6821(config, m_pia);
	m_pia->writepa_handler().set(FUNC(midway_turbo_cheap_squeak_device::porta_w));
	m_pia->writepb_handler().set(FUNC(midway_turbo_cheap_squeak_device::portb_w));
	m_pia->irqa_handler().set(FUNC(midway_turbo_cheap_squeak_device::irq_w));
	m_pia->irqb_handler().set(FUNC(midway_turbo_cheap_squeak_device::irq_w));

	AD7533(config, m_dac); /// ad7533jn.u11

	// The DAC filters here are identical to those on the "Sounds Good" and "Cheap Squeak Deluxe" boards.
	//LM359 @U14.2, 2nd order MFB low-pass (fc = 5404.717733, Q = 0.625210, gain = -1.000000)
	FILTER_BIQUAD(config, m_dac_filter[2]).opamp_mfb_lowpass_setup(RES_K(150), RES_K(82), RES_K(150), CAP_P(470), CAP_P(150)); // R36, R37, R39, C19, C20
	m_dac_filter[2]->add_route(ALL_OUTPUTS, *this, 1.0);
	//LM359 @U13.2, 2nd order MFB low-pass (fc = 5310.690763, Q = 1.608630, gain = -1.000000)
	FILTER_BIQUAD(config, m_dac_filter[1]).opamp_mfb_lowpass_setup(RES_K(33), RES_K(18), RES_K(33), CAP_P(5600), CAP_P(270)); // R32, R33, R35, C16, C17
	m_dac_filter[1]->add_route(ALL_OUTPUTS, m_dac_filter[2], 1.0);
	//LM359 @U13.1, 1st order MFB low-pass (fc = 4912.189602, Q = 0.707107(ignored), gain = -1.000000)
	FILTER_BIQUAD(config, m_dac_filter[0]).opamp_mfb_lowpass_setup(RES_K(120), RES_K(0), RES_K(120), CAP_P(0), CAP_P(270)); // R27, <short>, R31, <nonexistent>, C14
	m_dac_filter[0]->add_route(ALL_OUTPUTS, m_dac_filter[1], 1.0);
	m_dac->add_route(ALL_OUTPUTS, m_dac_filter[0], 1.0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void midway_turbo_cheap_squeak_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_dacval));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void midway_turbo_cheap_squeak_device::device_reset()
{
}


//-------------------------------------------------
//  synced_write
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(midway_turbo_cheap_squeak_device::synced_write)
{
	m_pia->portb_w((param >> 1) & 0x0f);
	m_pia->ca1_w(~param & 0x01);

	// oftentimes games will write one nibble at a time; the sync on this is very
	// important, so we boost the interleave briefly while this happens
	machine().scheduler().perfect_quantum(attotime::from_usec(100));
}
