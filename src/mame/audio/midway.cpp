// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    midway.cpp

    Functions to emulate general the various Midway sound cards.

***************************************************************************/

#include "emu.h"
#include "includes/mcr.h"
#include "audio/midway.h"
#include "audio/williams.h"
#include "sound/volt_reg.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BALLY_AS3022,              bally_as3022_device,              "as3022",  "Bally AS3022 Sound Board")
DEFINE_DEVICE_TYPE(MIDWAY_SSIO,               midway_ssio_device,               "midssio", "Midway SSIO Sound Board")
DEFINE_DEVICE_TYPE(MIDWAY_SOUNDS_GOOD,        midway_sounds_good_device,        "midsg",   "Midway Sounds Good Sound Board")
DEFINE_DEVICE_TYPE(MIDWAY_TURBO_CHEAP_SQUEAK, midway_turbo_cheap_squeak_device, "midtcs",  "Midway Turbo Cheap Squeak Sound Board")
DEFINE_DEVICE_TYPE(MIDWAY_SQUAWK_N_TALK,      midway_squawk_n_talk_device,      "midsnt",  "Midway Squawk 'n' Talk Sound Board")



//**************************************************************************
//  AS3022
//**************************************************************************

//-------------------------------------------------
//  bally_as3022_device - constructor
//-------------------------------------------------

bally_as3022_device::bally_as3022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BALLY_AS3022, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_cpu(*this, "cpu"),
		m_pia(*this, "pia"),
		m_ay(*this, "ay")
{
}


//-------------------------------------------------

//  sound_select - handle an external write to the board
//-------------------------------------------------

WRITE8_MEMBER(bally_as3022_device::sound_select)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as3022_device::sound_select_sync), this), data);
}


TIMER_CALLBACK_MEMBER(bally_as3022_device::sound_select_sync)
{
	m_sound_select = param;
}

//-------------------------------------------------
//
//  sound_int - handle an external sound interrupt to the board
//-------------------------------------------------

WRITE_LINE_MEMBER(bally_as3022_device::sound_int)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bally_as3022_device::sound_int_sync), this), state);
}

TIMER_CALLBACK_MEMBER(bally_as3022_device::sound_int_sync)
{
	m_pia->ca1_w(param);
}


//-------------------------------------------------
//
//  irq_w - IRQ line state changes
//-------------------------------------------------

WRITE_LINE_MEMBER(bally_as3022_device::irq_w)
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
	map(0x0000, 0x007f).mirror(0xef00).ram();
	map(0x0080, 0x0083).mirror(0xef7c).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x17ff).mirror(0xe800).rom();
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void bally_as3022_device::device_add_mconfig(machine_config &config)
{
	M6808(config, m_cpu, DERIVED_CLOCK(1, 1));
	m_cpu->set_addrmap(AS_PROGRAM, &bally_as3022_device::as3022_map);

	PIA6821(config, m_pia, 0);
	m_pia->readpa_handler().set(FUNC(bally_as3022_device::pia_porta_r));
	m_pia->writepa_handler().set(FUNC(bally_as3022_device::pia_porta_w));
	m_pia->writepb_handler().set(FUNC(bally_as3022_device::pia_portb_w));
	m_pia->cb2_handler().set(FUNC(bally_as3022_device::pia_cb2_w));
	m_pia->irqa_handler().set(FUNC(bally_as3022_device::irq_w));
	m_pia->irqb_handler().set(FUNC(bally_as3022_device::irq_w));

	AY8910(config, m_ay, DERIVED_CLOCK(1, 4));
	m_ay->port_a_read_callback().set(FUNC(bally_as3022_device::ay_io_r));
	m_ay->add_route(ALL_OUTPUTS, *this, 0.33, AUTO_ALLOC_INPUT, 0);
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
}



//-------------------------------------------------
//  pia_porta_r - PIA port A reads
//-------------------------------------------------

READ8_MEMBER(bally_as3022_device::pia_porta_r)
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

WRITE8_MEMBER(bally_as3022_device::pia_porta_w)
{
	if (m_bc1 && !m_bdir)
	{
		logerror("PIA port A bus contention!\n");
	}
	m_ay_data = data;
	update_sound_selects();
}


//-------------------------------------------------
//  pia_portb_w - PIA port B writes
//-------------------------------------------------

WRITE8_MEMBER(bally_as3022_device::pia_portb_w)
{
	m_bc1 = BIT(data, 0);
	m_bdir = BIT(data, 1);
	if (m_bc1 && !m_bdir)
	{
		m_ay_data = m_ay->data_r();
	}
	update_sound_selects();
}


//-------------------------------------------------
//  pia_cb2_w - PIA CB2 writes
//-------------------------------------------------
WRITE_LINE_MEMBER(bally_as3022_device::pia_cb2_w)
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

READ8_MEMBER(bally_as3022_device::ay_io_r)
{
	// The two high bits are unconnected, the others are inverted.
	return ~m_sound_select & 0x3f;
}


void bally_as3022_device::update_sound_selects()
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
//  SUPER SOUND I/O BOARD (SSIO)
//**************************************************************************

//-------------------------------------------------
//  midway_ssio_device - constructor
//-------------------------------------------------

midway_ssio_device::midway_ssio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDWAY_SSIO, tag, owner, clock),
		device_mixer_interface(mconfig, *this, 2),
		m_cpu(*this, "cpu"),
		m_ay0(*this, "ay0"),
		m_ay1(*this, "ay1"),
		m_ports(*this, {"IP0", "IP1", "IP2", "IP3", "IP4"}),
		m_status(0),
		m_14024_count(0),
		m_mute(0)
{
	memset(m_data, 0, sizeof(m_data));
	memset(m_overall, 0, sizeof(m_overall));
	memset(m_duty_cycle, 0, sizeof(m_duty_cycle));
	memset(m_ayvolume_lookup, 0, sizeof(m_ayvolume_lookup));
	memset(m_custom_input_mask, 0, sizeof(m_custom_input_mask));
	memset(m_custom_output_mask, 0, sizeof(m_custom_output_mask));
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

READ8_MEMBER(midway_ssio_device::read)
{
	return m_status;
}


//-------------------------------------------------
//  write - handle an external write to one of the
//  input latches
//-------------------------------------------------

WRITE8_MEMBER(midway_ssio_device::write)
{
	synchronize(0, (offset << 8) | (data & 0xff));
}


//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

WRITE_LINE_MEMBER(midway_ssio_device::reset_write)
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

READ8_MEMBER(midway_ssio_device::ioport_read)
{
	uint8_t result = m_ports[offset].read_safe(0xff);
	if (!m_custom_input[offset].isnull())
		result = (result & ~m_custom_input_mask[offset]) |
					(m_custom_input[offset](space, offset, 0xff) & m_custom_input_mask[offset]);
	return result;
}


//-------------------------------------------------
//  ioport_write - write to one of the I/O ports
//  on the device
//-------------------------------------------------

WRITE8_MEMBER(midway_ssio_device::ioport_write)
{
	int which = offset >> 2;
	if (!m_custom_output[which].isnull())
		m_custom_output[which](space, offset & 4, data & m_custom_output_mask[which], 0xff);
}


//-------------------------------------------------
//  set_custom_input - configure a custom port
//  reader
//-------------------------------------------------

void midway_ssio_device::set_custom_input(int which, uint8_t mask, read8_delegate handler)
{
	m_custom_input[which] = handler;
	m_custom_input_mask[which] = mask;
}


//-------------------------------------------------
//  set_custom_output - configure a custom port
//  writer
//-------------------------------------------------

void midway_ssio_device::set_custom_output(int which, uint8_t mask, write8_delegate handler)
{
	m_custom_output[which/4] = handler;
	m_custom_output_mask[which/4] = mask;
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

READ8_MEMBER(midway_ssio_device::irq_clear)
{
	// a read here asynchronously resets the 14024 count, clearing /SINT
	m_14024_count = 0;
	m_cpu->set_input_line(0, CLEAR_LINE);
	return 0xff;
}


//-------------------------------------------------
//  status_w - set the outgoing status value
//-------------------------------------------------

WRITE8_MEMBER(midway_ssio_device::status_w)
{
	m_status = data;
}


//-------------------------------------------------
//  data_r - read incoming data latches
//-------------------------------------------------

READ8_MEMBER(midway_ssio_device::data_r)
{
	return m_data[offset];
}


//-------------------------------------------------
//  porta0_w - handle writes to AY-8910 #0 port A
//-------------------------------------------------

WRITE8_MEMBER(midway_ssio_device::porta0_w)
{
	m_duty_cycle[0][0] = data & 15;
	m_duty_cycle[0][1] = data >> 4;
	update_volumes();
}


//-------------------------------------------------
//  portb0_w - handle writes to AY-8910 #0 port B
//-------------------------------------------------

WRITE8_MEMBER(midway_ssio_device::portb0_w)
{
	m_duty_cycle[0][2] = data & 15;
	m_overall[0] = (data >> 4) & 7;
	update_volumes();
}


//-------------------------------------------------
//  porta1_w - handle writes to AY-8910 #1 port A
//-------------------------------------------------

WRITE8_MEMBER(midway_ssio_device::porta1_w)
{
	m_duty_cycle[1][0] = data & 15;
	m_duty_cycle[1][1] = data >> 4;
	update_volumes();
}


//-------------------------------------------------
//  portb1_w - handle writes to AY-8910 #1 port B
//-------------------------------------------------

WRITE8_MEMBER(midway_ssio_device::portb1_w)
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
	m_ay0->add_route(ALL_OUTPUTS, *this, 0.33, AUTO_ALLOC_INPUT, 0);

	AY8910(config, m_ay1, DERIVED_CLOCK(1, 2*4));
	m_ay1->port_a_write_callback().set(FUNC(midway_ssio_device::porta1_w));
	m_ay1->port_b_write_callback().set(FUNC(midway_ssio_device::portb1_w));
	m_ay1->add_route(ALL_OUTPUTS, *this, 0.33, AUTO_ALLOC_INPUT, 1);
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
//  device_timer - timer callbacks
//-------------------------------------------------

void midway_ssio_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
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
	: device_t(mconfig, MIDWAY_SOUNDS_GOOD, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_cpu(*this, "cpu"),
		m_pia(*this, "pia"),
		m_dac(*this, "dac"),
		m_status(0),
		m_dacval(0)
{
}


//-------------------------------------------------
//  read - return the status value
//-------------------------------------------------

READ8_MEMBER(midway_sounds_good_device::read)
{
	return m_status;
}


//-------------------------------------------------
//  write - handle an external write to the input
//  latch
//-------------------------------------------------

WRITE8_MEMBER(midway_sounds_good_device::write)
{
	synchronize(0, data);
}


//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

WRITE_LINE_MEMBER(midway_sounds_good_device::reset_write)
{
//if (state) osd_printf_debug("SG Reset\n");
	m_cpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  porta_w - PIA port A writes
//-------------------------------------------------

WRITE8_MEMBER(midway_sounds_good_device::porta_w)
{
	m_dacval = (data << 2) | (m_dacval & 3);
	m_dac->write(m_dacval);
}


//-------------------------------------------------
//  portb_w - PIA port B writes
//-------------------------------------------------

WRITE8_MEMBER(midway_sounds_good_device::portb_w)
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

WRITE_LINE_MEMBER(midway_sounds_good_device::irq_w)
{
	int combined_state = m_pia->irq_a_state() | m_pia->irq_b_state();
	m_cpu->set_input_line(4, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

// address map determined by PAL; not verified
void midway_sounds_good_device::soundsgood_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7ffff);
	map(0x000000, 0x03ffff).rom();
	map(0x060000, 0x060007).rw("pia", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt)).umask16(0xff00);
	map(0x070000, 0x070fff).ram();
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void midway_sounds_good_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_cpu, DERIVED_CLOCK(1, 2));
	m_cpu->set_addrmap(AS_PROGRAM, &midway_sounds_good_device::soundsgood_map);

	PIA6821(config, m_pia, 0);
	m_pia->writepa_handler().set(FUNC(midway_sounds_good_device::porta_w));
	m_pia->writepb_handler().set(FUNC(midway_sounds_good_device::portb_w));
	m_pia->irqa_handler().set(FUNC(midway_sounds_good_device::irq_w));
	m_pia->irqb_handler().set(FUNC(midway_sounds_good_device::irq_w));

	AD7533(config, m_dac, 0).add_route(ALL_OUTPUTS, *this, 1.0); /// ad7533jn.u10
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
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
//  device_timer - timer callbacks
//-------------------------------------------------

void midway_sounds_good_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_pia->write_portb((param >> 1) & 0x0f);
	m_pia->ca1_w(~param & 0x01);

	// oftentimes games will write one nibble at a time; the sync on this is very
	// important, so we boost the interleave briefly while this happens
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(250));
}



//**************************************************************************
//  TURBO CHEAP SQUEAK BOARD
//**************************************************************************

//-------------------------------------------------
//  midway_turbo_cheap_squeak_device - constructor
//-------------------------------------------------

midway_turbo_cheap_squeak_device::midway_turbo_cheap_squeak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDWAY_TURBO_CHEAP_SQUEAK, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_cpu(*this, "cpu"),
		m_pia(*this, "pia"),
		m_dac(*this, "dac"),
		m_status(0),
		m_dacval(0)
{
}


//-------------------------------------------------
//  read - return the status value
//-------------------------------------------------

READ8_MEMBER(midway_turbo_cheap_squeak_device::read)
{
	return m_status;
}


//-------------------------------------------------
//  write - handle an external write to the input
//  latch
//-------------------------------------------------

WRITE8_MEMBER(midway_turbo_cheap_squeak_device::write)
{
	synchronize(0, data);
}


//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

WRITE_LINE_MEMBER(midway_turbo_cheap_squeak_device::reset_write)
{
	m_cpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  porta_w - PIA port A writes
//-------------------------------------------------

WRITE8_MEMBER(midway_turbo_cheap_squeak_device::porta_w)
{
	m_dacval = (data << 2) | (m_dacval & 3);
	m_dac->write(m_dacval);
}


//-------------------------------------------------
//  portb_w - PIA port B writes
//-------------------------------------------------

WRITE8_MEMBER(midway_turbo_cheap_squeak_device::portb_w)
{
	m_dacval = (m_dacval & ~3) | (data >> 6);
	m_dac->write(m_dacval);
	m_status = (data >> 4) & 3;
}


//-------------------------------------------------
//  irq_w - IRQ line state changes
//-------------------------------------------------

WRITE_LINE_MEMBER(midway_turbo_cheap_squeak_device::irq_w)
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

	PIA6821(config, m_pia, 0);
	m_pia->writepa_handler().set(FUNC(midway_turbo_cheap_squeak_device::porta_w));
	m_pia->writepb_handler().set(FUNC(midway_turbo_cheap_squeak_device::portb_w));
	m_pia->irqa_handler().set(FUNC(midway_turbo_cheap_squeak_device::irq_w));
	m_pia->irqb_handler().set(FUNC(midway_turbo_cheap_squeak_device::irq_w));

	AD7533(config, m_dac, 0).add_route(ALL_OUTPUTS, *this, 1.0);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
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
//  device_timer - timer callbacks
//-------------------------------------------------

void midway_turbo_cheap_squeak_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_pia->write_portb((param >> 1) & 0x0f);
	m_pia->ca1_w(~param & 0x01);

	// oftentimes games will write one nibble at a time; the sync on this is very
	// important, so we boost the interleave briefly while this happens
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}


//**************************************************************************
//  SQUAWK 'N' TALK BOARD
//**************************************************************************

//-------------------------------------------------
//  midway_squawk_n_talk_device - constructor
//-------------------------------------------------

midway_squawk_n_talk_device::midway_squawk_n_talk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDWAY_SQUAWK_N_TALK, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_cpu(*this, "cpu"),
		m_pia0(*this, "pia0"),
		m_pia1(*this, "pia1"),
		m_tms5200(*this, "tms5200"),
		m_tms_command(0),
		m_tms_strobes(0)
{
}


//-------------------------------------------------
//  write - handle an external write to the input
//  latch
//-------------------------------------------------

WRITE8_MEMBER(midway_squawk_n_talk_device::write)
{
	synchronize(0, data);
}


//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

WRITE_LINE_MEMBER(midway_squawk_n_talk_device::reset_write)
{
	m_cpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  porta1_w - PIA #1 port A writes
//-------------------------------------------------

WRITE8_MEMBER(midway_squawk_n_talk_device::porta1_w )
{
	logerror("Write to AY-8912 = %02X\n", data);
}


//-------------------------------------------------
//  dac_w - DAC data writes
//-------------------------------------------------

WRITE8_MEMBER(midway_squawk_n_talk_device::dac_w)
{
	logerror("Write to DAC = %02X\n", data);
}


//-------------------------------------------------
//  porta2_w - PIA #2 port A writes
//-------------------------------------------------

WRITE8_MEMBER(midway_squawk_n_talk_device::porta2_w)
{
	m_tms_command = data;
}


//-------------------------------------------------
//  portb2_w - PIA #2 port B writes
//-------------------------------------------------

WRITE8_MEMBER(midway_squawk_n_talk_device::portb2_w)
{
	// bits 0-1 select read/write strobes on the TMS5200
	data &= 0x03;

	// write strobe -- pass the current command to the TMS5200
	if (((data ^ m_tms_strobes) & 0x02) && !(data & 0x02))
	{
		m_tms5200->data_w(m_tms_command);

		// DoT expects the ready line to transition on a command/write here, so we oblige
		m_pia1->ca2_w(1);
		m_pia1->ca2_w(0);
	}

	// read strobe -- read the current status from the TMS5200
	else if (((data ^ m_tms_strobes) & 0x01) && !(data & 0x01))
	{
		m_pia1->write_porta(m_tms5200->status_r());

		// DoT expects the ready line to transition on a command/write here, so we oblige
		m_pia1->ca2_w(1);
		m_pia1->ca2_w(0);
	}

	// remember the state
	m_tms_strobes = data;
}


//-------------------------------------------------
//  irq_w - IRQ line state changes
//-------------------------------------------------

WRITE_LINE_MEMBER(midway_squawk_n_talk_device::irq_w)
{
	int combined_state = m_pia0->irq_a_state() | m_pia0->irq_b_state() | m_pia1->irq_a_state() | m_pia1->irq_b_state();
	m_cpu->set_input_line(M6802_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

// address map verified from schematics
// note that jumpers control the ROM sizes; if these are changed, use the alternate
// address map below
void midway_squawk_n_talk_device::squawkntalk_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).ram();     // internal RAM
	map(0x0080, 0x0083).mirror(0x4f6c).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).mirror(0x4f6c).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x1fff).mirror(0x4000).w(FUNC(midway_squawk_n_talk_device::dac_w));
	map(0x8000, 0xbfff).mirror(0x4000).rom();
}

// alternate address map if the ROM jumpers are changed to support a smaller
// ROM size of 2k
#ifdef UNUSED_FUNCTION
void midway_squawk_n_talk_device::squawkntalk_alt_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x007f).ram();     // internal RAM
	map(0x0080, 0x0083).mirror(0x676c).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0090, 0x0093).mirror(0x676c).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0800, 0x0fff).mirror(0x6000).w(FUNC(midway_squawk_n_talk_device::dac_w));
	map(0x8000, 0x9fff).mirror(0x6000).rom();
}
#endif


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void midway_squawk_n_talk_device::device_add_mconfig(machine_config &config)
{
	M6802(config, m_cpu, DERIVED_CLOCK(1, 1));
	m_cpu->set_addrmap(AS_PROGRAM, &midway_squawk_n_talk_device::squawkntalk_map);

	PIA6821(config, m_pia0, 0);
	m_pia0->writepa_handler().set(FUNC(midway_squawk_n_talk_device::porta1_w));
	m_pia0->irqa_handler().set(FUNC(midway_squawk_n_talk_device::irq_w));
	m_pia0->irqb_handler().set(FUNC(midway_squawk_n_talk_device::irq_w));

	PIA6821(config, m_pia1, 0);
	m_pia1->writepa_handler().set(FUNC(midway_squawk_n_talk_device::porta2_w));
	m_pia1->writepb_handler().set(FUNC(midway_squawk_n_talk_device::portb2_w));
	m_pia1->irqa_handler().set(FUNC(midway_squawk_n_talk_device::irq_w));
	m_pia1->irqb_handler().set(FUNC(midway_squawk_n_talk_device::irq_w));

	// only used on Discs of Tron, which is stereo
	TMS5200(config, m_tms5200, 640000).add_route(ALL_OUTPUTS, *this, 0.60);

	// the board also supports an AY-8912 and/or an 8-bit DAC, neither of
	// which are populated on the Discs of Tron board
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void midway_squawk_n_talk_device::device_start()
{
	save_item(NAME(m_tms_command));
	save_item(NAME(m_tms_strobes));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void midway_squawk_n_talk_device::device_reset()
{
}


//-------------------------------------------------
//  device_timer - timer callbacks
//-------------------------------------------------

void midway_squawk_n_talk_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_pia0->write_porta(~param & 0x0f);
	m_pia0->cb1_w(BIT(~param, 4));
}
