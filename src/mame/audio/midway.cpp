// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    midway.c

    Functions to emulate general the various Midway sound cards.

***************************************************************************/

#include "emu.h"
#include "includes/mcr.h"
#include "audio/midway.h"
#include "audio/williams.h"
#include "sound/volt_reg.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define SSIO_CLOCK          XTAL(16'000'000)
#define SOUNDSGOOD_CLOCK    XTAL(16'000'000)
#define TURBOCS_CLOCK       XTAL(8'000'000)
#define SQUAWKTALK_CLOCK    XTAL(3'579'545)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MIDWAY_SSIO,               midway_ssio_device,               "midssio", "Midway SSIO Sound Board")
DEFINE_DEVICE_TYPE(MIDWAY_SOUNDS_GOOD,        midway_sounds_good_device,        "midsg",   "Midway Sounds Good Sound Board")
DEFINE_DEVICE_TYPE(MIDWAY_TURBO_CHEAP_SQUEAK, midway_turbo_cheap_squeak_device, "midtcs",  "Midway Turbo Cheap Squeak Sound Board")
DEFINE_DEVICE_TYPE(MIDWAY_SQUAWK_N_TALK,      midway_squawk_n_talk_device,      "midsnt",  "Midway Squawk 'n' Talk Sound Board")



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
	// going high halts the CPU
	if (state)
	{
		device_reset();
		m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}

	// going low resets and reactivates the CPU
	else
		m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
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
ADDRESS_MAP_START(midway_ssio_device::ssio_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x9000, 0x9003) AM_MIRROR(0x0ffc) AM_READ(data_r)
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x0ffc) AM_DEVWRITE("ay0", ay8910_device, address_w)
	AM_RANGE(0xa001, 0xa001) AM_MIRROR(0x0ffc) AM_DEVREAD("ay0", ay8910_device, data_r)
	AM_RANGE(0xa002, 0xa002) AM_MIRROR(0x0ffc) AM_DEVWRITE("ay0", ay8910_device, data_w)
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x0ffc) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0xb001, 0xb001) AM_MIRROR(0x0ffc) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0xb002, 0xb002) AM_MIRROR(0x0ffc) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0xc000, 0xcfff) AM_READNOP AM_WRITE(status_w)
	AM_RANGE(0xd000, 0xdfff) AM_WRITENOP    // low bit controls yellow LED
	AM_RANGE(0xe000, 0xefff) AM_READ(irq_clear)
	AM_RANGE(0xf000, 0xffff) AM_READ_PORT("DIP")    // 6 DIP switches
ADDRESS_MAP_END


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

MACHINE_CONFIG_START(midway_ssio_device::device_add_mconfig)
	MCFG_CPU_ADD("cpu", Z80, SSIO_CLOCK/2/4)
	MCFG_CPU_PROGRAM_MAP(ssio_map)
	MCFG_DEVICE_PERIODIC_INT_DEVICE(DEVICE_SELF, midway_ssio_device, clock_14024, SSIO_CLOCK/2/16/10)

	MCFG_SOUND_ADD("ay0", AY8910, SSIO_CLOCK/2/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(midway_ssio_device, porta0_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(midway_ssio_device, portb0_w))
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.33, 0)

	MCFG_SOUND_ADD("ay1", AY8910, SSIO_CLOCK/2/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(midway_ssio_device, porta1_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(midway_ssio_device, portb1_w))
	MCFG_MIXER_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.33, 1)
MACHINE_CONFIG_END


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
ADDRESS_MAP_START(midway_sounds_good_device::soundsgood_map)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x7ffff)
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x060000, 0x060007) AM_DEVREADWRITE8("pia", pia6821_device, read_alt, write_alt, 0xff00)
	AM_RANGE(0x070000, 0x070fff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(midway_sounds_good_device::device_add_mconfig)
	MCFG_CPU_ADD("cpu", M68000, SOUNDSGOOD_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(soundsgood_map)

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(midway_sounds_good_device, porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(midway_sounds_good_device, portb_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(midway_sounds_good_device, irq_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(midway_sounds_good_device, irq_w))

	MCFG_SOUND_ADD("dac", AD7533, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0) /// ad7533jn.u10
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END


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
	m_pia->portb_w((param >> 1) & 0x0f);
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
ADDRESS_MAP_START(midway_turbo_cheap_squeak_device::turbocs_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x3800) AM_RAM
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x3ffc) AM_DEVREADWRITE("pia", pia6821_device, read_alt, write_alt)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(midway_turbo_cheap_squeak_device::device_add_mconfig)
	MCFG_CPU_ADD("cpu", MC6809E, TURBOCS_CLOCK / 4)
	MCFG_CPU_PROGRAM_MAP(turbocs_map)

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(midway_turbo_cheap_squeak_device, porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(midway_turbo_cheap_squeak_device, portb_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(midway_turbo_cheap_squeak_device, irq_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(midway_turbo_cheap_squeak_device, irq_w))

	MCFG_SOUND_ADD("dac", AD7533, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END


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
	m_pia->portb_w((param >> 1) & 0x0f);
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
		m_tms5200->data_w(space, offset, m_tms_command);

		// DoT expects the ready line to transition on a command/write here, so we oblige
		m_pia1->ca2_w(1);
		m_pia1->ca2_w(0);
	}

	// read strobe -- read the current status from the TMS5200
	else if (((data ^ m_tms_strobes) & 0x01) && !(data & 0x01))
	{
		m_pia1->porta_w(m_tms5200->status_r(space, offset));

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
ADDRESS_MAP_START(midway_squawk_n_talk_device::squawkntalk_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_RAM     // internal RAM
	AM_RANGE(0x0080, 0x0083) AM_MIRROR(0x4f6c) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x0090, 0x0093) AM_MIRROR(0x4f6c) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0x1000, 0x1fff) AM_MIRROR(0x4000) AM_WRITE(dac_w)
	AM_RANGE(0x8000, 0xbfff) AM_MIRROR(0x4000) AM_ROM
ADDRESS_MAP_END

// alternate address map if the ROM jumpers are changed to support a smaller
// ROM size of 2k
#ifdef UNUSED_FUNCTION
ADDRESS_MAP_START(midway_squawk_n_talk_device::squawkntalk_alt_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_RAM     // internal RAM
	AM_RANGE(0x0080, 0x0083) AM_MIRROR(0x676c) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x0090, 0x0093) AM_MIRROR(0x676c) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0x0800, 0x0fff) AM_MIRROR(0x6000) AM_WRITE(dac_w)
	AM_RANGE(0x8000, 0x9fff) AM_MIRROR(0x6000) AM_ROM
ADDRESS_MAP_END
#endif


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(midway_squawk_n_talk_device::device_add_mconfig)
	MCFG_CPU_ADD("cpu", M6802, SQUAWKTALK_CLOCK)
	MCFG_CPU_PROGRAM_MAP(squawkntalk_map)

	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(midway_squawk_n_talk_device, porta1_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(midway_squawk_n_talk_device, irq_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(midway_squawk_n_talk_device, irq_w))

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(midway_squawk_n_talk_device, porta2_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(midway_squawk_n_talk_device, portb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(midway_squawk_n_talk_device, irq_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(midway_squawk_n_talk_device, irq_w))

	// only used on Discs of Tron, which is stereo
	MCFG_SOUND_ADD("tms5200", TMS5200, 640000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.60)

	// the board also supports an AY-8912 and/or an 8-bit DAC, neither of
	// which are populated on the Discs of Tron board
MACHINE_CONFIG_END


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
	m_pia0->porta_w(~param & 0x0f);
	m_pia0->cb1_w(~param & 0x10);
}
