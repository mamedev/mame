// license: BSD-3-Clause
// copyright-holders: Aaron Giles
/***************************************************************************

    Cheap Squeak Deluxe / Artificial Artist Sound Board

***************************************************************************/

#include "emu.h"
#include "csd.h"
#include "sound/volt_reg.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MIDWAY_CHEAP_SQUEAK_DELUXE, midway_cheap_squeak_deluxe_device, "midcsd", "Cheap Squeak Deluxe Sound Board")

//-------------------------------------------------
//  audio cpu map
//-------------------------------------------------

// address map determined by PAL; verified
ADDRESS_MAP_START(midway_cheap_squeak_deluxe_device::csdeluxe_map)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x00000, 0x07fff) AM_ROM
	AM_RANGE(0x18000, 0x18007) AM_MIRROR(0x3ff8) AM_DEVREADWRITE8("pia", pia6821_device, read_alt, write_alt, 0xff00) // Spy Hunter accesses the MSB
	AM_RANGE(0x18000, 0x18007) AM_MIRROR(0x3ff8) AM_DEVREADWRITE8("pia", pia6821_device, read_alt, write_alt, 0x00ff) // Turbo Tag access via the LSB
	AM_RANGE(0x1c000, 0x1cfff) AM_RAM
ADDRESS_MAP_END

//-------------------------------------------------
//  machine configuration
//-------------------------------------------------

MACHINE_CONFIG_START(midway_cheap_squeak_deluxe_device::device_add_mconfig)
	MCFG_CPU_ADD("cpu", M68000, XTAL(16'000'000)/2)
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
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( csd )
	ROM_REGION(0x4a, "pal", 0)
	ROM_LOAD("0304-00803-0052.u15", 0x00, 0x4a, CRC(8b401aee) SHA1(360f3f59877f0bc25b4154782a2011963dd80a52)) // address decoding pal CSD002R0 (PAL14L8)
ROM_END

const tiny_rom_entry *midway_cheap_squeak_deluxe_device::device_rom_region() const
{
	return ROM_NAME( csd );
}

//-------------------------------------------------
//  midway_cheap_squeak_deluxe_device - constructor
//-------------------------------------------------

midway_cheap_squeak_deluxe_device::midway_cheap_squeak_deluxe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MIDWAY_CHEAP_SQUEAK_DELUXE, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_cpu(*this, "cpu"),
	m_pia(*this, "pia"),
	m_dac(*this, "dac"),
	m_status(0),
	m_dacval(0)
{
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
//  device_timer - timer callbacks
//-------------------------------------------------

void midway_cheap_squeak_deluxe_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_pia->ca1_w(param);

	// oftentimes games will write one nibble at a time; the sync on this is very
	// important, so we boost the interleave briefly while this happens
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}

//-------------------------------------------------
//  stat_r - return the status value
//-------------------------------------------------

READ8_MEMBER( midway_cheap_squeak_deluxe_device::stat_r )
{
	return m_status;
}

//-------------------------------------------------
//  sr_w - external 4-bit write to the input latch
//-------------------------------------------------

WRITE8_MEMBER( midway_cheap_squeak_deluxe_device::sr_w )
{
	m_pia->portb_w(data & 0x0f);
}

//-------------------------------------------------
//  sirq_w - external irq write
//-------------------------------------------------

WRITE_LINE_MEMBER( midway_cheap_squeak_deluxe_device::sirq_w )
{
	synchronize(0, !state);
}

//-------------------------------------------------
//  reset_write - write to the reset line
//-------------------------------------------------

WRITE_LINE_MEMBER( midway_cheap_squeak_deluxe_device::reset_w )
{
	m_cpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
	m_cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
}

//-------------------------------------------------
//  porta_w - PIA port A writes
//-------------------------------------------------

WRITE8_MEMBER( midway_cheap_squeak_deluxe_device::porta_w )
{
	m_dacval = (data << 2) | (m_dacval & 3);
	m_dac->write(m_dacval);
}

//-------------------------------------------------
//  portb_w - PIA port B writes
//-------------------------------------------------

WRITE8_MEMBER( midway_cheap_squeak_deluxe_device::portb_w )
{
	// bit 4-5, status
	uint8_t z_mask = m_pia->port_b_z_mask();
	if (~z_mask & 0x10)  m_status = (m_status & ~1) | ((data >> 4) & 1);
	if (~z_mask & 0x20)  m_status = (m_status & ~2) | ((data >> 4) & 2);

	// bit 6-7, dac data
	m_dacval = (m_dacval & ~3) | (data >> 6);
	m_dac->write(m_dacval);
}

//-------------------------------------------------
//  irq_w - IRQ line state changes
//-------------------------------------------------

WRITE_LINE_MEMBER( midway_cheap_squeak_deluxe_device::irq_w )
{
	int combined_state = m_pia->irq_a_state() | m_pia->irq_b_state();
	m_cpu->set_input_line(4, combined_state ? ASSERT_LINE : CLEAR_LINE);
}
