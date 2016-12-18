// license: BSD-3-Clause
// copyright-holders: Aaron Giles
/***************************************************************************

	Cheap Squeak Deluxe / Artificial Artist Sound Board

***************************************************************************/

#include "csd.h"
#include "sound/volt_reg.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

extern const device_type MIDWAY_CHEAP_SQUEAK_DELUXE = &device_creator<midway_cheap_squeak_deluxe_device>;


//-------------------------------------------------
//  midway_cheap_squeak_deluxe_device - constructor
//-------------------------------------------------

midway_cheap_squeak_deluxe_device::midway_cheap_squeak_deluxe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDWAY_CHEAP_SQUEAK_DELUXE, "Cheap Squeak Deluxe Sound Board", tag, owner, clock, "midcsd", __FILE__),
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

READ8_MEMBER(midway_cheap_squeak_deluxe_device::read)
{
	return m_status;
}


//-------------------------------------------------
//  write - handle an external write to the input
//  latch
//-------------------------------------------------

WRITE8_MEMBER(midway_cheap_squeak_deluxe_device::write)
{
	synchronize(0, data);
}


//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

WRITE_LINE_MEMBER(midway_cheap_squeak_deluxe_device::reset_write)
{
	m_cpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  porta_w - PIA port A writes
//-------------------------------------------------

WRITE8_MEMBER(midway_cheap_squeak_deluxe_device::porta_w)
{
	m_dacval = (data << 2) | (m_dacval & 3);
	m_dac->write(m_dacval);
}


//-------------------------------------------------
//  portb_w - PIA port B writes
//-------------------------------------------------

WRITE8_MEMBER(midway_cheap_squeak_deluxe_device::portb_w)
{
	m_dacval = (m_dacval & ~3) | (data >> 6);
	m_dac->write(m_dacval);

	uint8_t z_mask = m_pia->port_b_z_mask();
	if (~z_mask & 0x10)  m_status = (m_status & ~1) | ((data >> 4) & 1);
	if (~z_mask & 0x20)  m_status = (m_status & ~2) | ((data >> 4) & 2);
}


//-------------------------------------------------
//  irq_w - IRQ line state changes
//-------------------------------------------------

WRITE_LINE_MEMBER(midway_cheap_squeak_deluxe_device::irq_w)
{
	int combined_state = m_pia->irq_a_state() | m_pia->irq_b_state();
	m_cpu->set_input_line(4, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  pia_r - PIA read access
//-------------------------------------------------

READ16_MEMBER(midway_cheap_squeak_deluxe_device::pia_r)
{
	// Spy Hunter accesses the MSB; Turbo Tag access via the LSB
	// My guess is that Turbo Tag works through a fluke, whereby the 68000
	// using the MOVEP instruction outputs the same value on the high and
	// low bytes.
	if (ACCESSING_BITS_8_15)
		return m_pia->read_alt(space, offset) << 8;
	else
		return m_pia->read_alt(space, offset);
}


//-------------------------------------------------
//  pia_w - PIA write access
//-------------------------------------------------

WRITE16_MEMBER(midway_cheap_squeak_deluxe_device::pia_w)
{
	if (ACCESSING_BITS_8_15)
		m_pia->write_alt(space, offset, data >> 8);
	else
		m_pia->write_alt(space, offset, data);
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

// address map determined by PAL; not verified
static ADDRESS_MAP_START( csdeluxe_map, AS_PROGRAM, 16, midway_cheap_squeak_deluxe_device )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x000000, 0x007fff) AM_ROM
	AM_RANGE(0x018000, 0x018007) AM_READWRITE(pia_r, pia_w)
	AM_RANGE(0x01c000, 0x01cfff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  machine configuration
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT(midway_cheap_squeak_deluxe)
	MCFG_CPU_ADD("cpu", M68000, XTAL_16MHz/2)
	MCFG_CPU_PROGRAM_MAP(csdeluxe_map)

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(midway_cheap_squeak_deluxe_device, porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(midway_cheap_squeak_deluxe_device, portb_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(midway_cheap_squeak_deluxe_device, irq_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(midway_cheap_squeak_deluxe_device, irq_w))

	MCFG_SOUND_ADD("dac", AD7533, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor midway_cheap_squeak_deluxe_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( midway_cheap_squeak_deluxe );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void midway_cheap_squeak_deluxe_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_dacval));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void midway_cheap_squeak_deluxe_device::device_reset()
{
}


//-------------------------------------------------
//  device_timer - timer callbacks
//-------------------------------------------------

void midway_cheap_squeak_deluxe_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_pia->portb_w(param & 0x0f);
	m_pia->ca1_w(~param & 0x10);

	// oftentimes games will write one nibble at a time; the sync on this is very
	// important, so we boost the interleave briefly while this happens
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}
