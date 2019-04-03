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
void midway_cheap_squeak_deluxe_device::csdeluxe_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x1ffff);
	map(0x00000, 0x07fff).rom();
	map(0x18000, 0x18007).mirror(0x3ff8).rw("pia", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt)).umask16(0xff00); // Spy Hunter accesses the MSB
	map(0x18000, 0x18007).mirror(0x3ff8).rw("pia", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt)).umask16(0x00ff); // Turbo Tag access via the LSB
	map(0x1c000, 0x1cfff).ram();
}

//-------------------------------------------------
//  machine configuration
//-------------------------------------------------

void midway_cheap_squeak_deluxe_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_cpu, DERIVED_CLOCK(1, 2));
	m_cpu->set_addrmap(AS_PROGRAM, &midway_cheap_squeak_deluxe_device::csdeluxe_map);

	PIA6821(config, m_pia, 0);
	m_pia->writepa_handler().set(FUNC(midway_cheap_squeak_deluxe_device::porta_w));
	m_pia->writepb_handler().set(FUNC(midway_cheap_squeak_deluxe_device::portb_w));
	m_pia->irqa_handler().set(FUNC(midway_cheap_squeak_deluxe_device::irq_w));
	m_pia->irqb_handler().set(FUNC(midway_cheap_squeak_deluxe_device::irq_w));

	AD7533(config, m_dac, 0).add_route(ALL_OUTPUTS, *this, 1.0);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}

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
//  suspend_cpu
//-------------------------------------------------

void midway_cheap_squeak_deluxe_device::suspend_cpu()
{
	m_cpu->suspend(SUSPEND_REASON_DISABLE, 1);
}

//-------------------------------------------------
//  stat_r - return the status value
//-------------------------------------------------

u8 midway_cheap_squeak_deluxe_device::stat_r()
{
	return m_status;
}

//-------------------------------------------------
//  sr_w - external 4-bit write to the input latch
//-------------------------------------------------

void midway_cheap_squeak_deluxe_device::sr_w(u8 data)
{
	m_pia->write_portb(data & 0x0f);
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
