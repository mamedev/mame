// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the MCGA (Memory Controller Gate Array) and FMCC (Fast
 * Memory Control Chip) devices found in Intergraph InterPro family systems.
 * There is no public documentation on this device, so the implementation is
 * being built to follow the logic of the system boot ROM and its diagnostic
 * tests.
 *
 * Please be aware that code in here is not only broken, it's likely wrong in
 * many cases.
 *
 * TODO
 *   - too long to list
 */

#include "emu.h"
#include "interpro_mcga.h"

#define VERBOSE 0
#include "logmacro.h"

void interpro_mcga_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(interpro_mcga_device::reg00_r), FUNC(interpro_mcga_device::reg00_w));
	map(0x08, 0x09).rw(FUNC(interpro_mcga_device::control_r), FUNC(interpro_mcga_device::control_w));
	map(0x10, 0x11).rw(FUNC(interpro_mcga_device::error_r), FUNC(interpro_mcga_device::error_w));
	map(0x18, 0x18).rw(FUNC(interpro_mcga_device::frcrd_r), FUNC(interpro_mcga_device::frcrd_w));
	map(0x20, 0x20).rw(FUNC(interpro_mcga_device::cbsub_r), FUNC(interpro_mcga_device::cbsub_w));
	map(0x28, 0x29).rw(FUNC(interpro_mcga_device::reg28_r), FUNC(interpro_mcga_device::reg28_w));
	map(0x30, 0x31).rw(FUNC(interpro_mcga_device::reg30_r), FUNC(interpro_mcga_device::reg30_w));
	map(0x38, 0x39).rw(FUNC(interpro_mcga_device::memsize_r), FUNC(interpro_mcga_device::memsize_w));
}

void interpro_fmcc_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(interpro_fmcc_device::reg00_r), FUNC(interpro_fmcc_device::reg00_w));
	map(0x08, 0x09).rw(FUNC(interpro_fmcc_device::control_r), FUNC(interpro_fmcc_device::control_w));
	map(0x10, 0x11).rw(FUNC(interpro_fmcc_device::error_r), FUNC(interpro_fmcc_device::error_w));
	map(0x18, 0x18).rw(FUNC(interpro_fmcc_device::frcrd_r), FUNC(interpro_fmcc_device::frcrd_w));
	map(0x20, 0x20).rw(FUNC(interpro_fmcc_device::cbsub_r), FUNC(interpro_fmcc_device::cbsub_w));
	map(0x28, 0x29).rw(FUNC(interpro_fmcc_device::reg28_r), FUNC(interpro_fmcc_device::reg28_w));
	map(0x30, 0x31).rw(FUNC(interpro_fmcc_device::reg30_r), FUNC(interpro_fmcc_device::reg30_w));
	map(0x38, 0x39).rw(FUNC(interpro_fmcc_device::memsize_r), FUNC(interpro_fmcc_device::memsize_w));
	map(0x40, 0x43).noprw(); // unknown
	map(0x48, 0x49).rw(FUNC(interpro_fmcc_device::error_control_r), FUNC(interpro_fmcc_device::error_control_w));
}

DEFINE_DEVICE_TYPE(INTERPRO_MCGA, interpro_mcga_device, "mcga", "Memory Controller Gate Array")
DEFINE_DEVICE_TYPE(INTERPRO_FMCC, interpro_fmcc_device, "fmcc", "Fast Memory Control Chip")

interpro_mcga_device::interpro_mcga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
{
}

interpro_mcga_device::interpro_mcga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: interpro_mcga_device(mconfig, INTERPRO_MCGA, tag, owner, clock)
{
}

interpro_fmcc_device::interpro_fmcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: interpro_mcga_device(mconfig, INTERPRO_FMCC, tag, owner, clock)
{
}

void interpro_mcga_device::device_start()
{
}

void interpro_mcga_device::device_reset()
{
	m_reg[0] = 0x00ff;
	m_control = CONTROL_ENREFRESH | CONTROL_CBITFRCSUB | CONTROL_CBITFRCRD;
	m_reg[1] = 0x00ff;
	m_memsize = 0x0340;
}

void interpro_mcga_device::control_w(u16 data)
{
	m_control = data & CONTROL_MASK;

	// HACK: set or clear error status depending on ENMMBE bit
	if (data & CONTROL_ENMMBE)
		m_error |= ERROR_VALID;
	//else
	//  m_error &= ~ERROR_VALID;
}

void interpro_fmcc_device::control_w(u16 data)
{
	m_control = data & CONTROL_MASK;

	// HACK: set or clear error status depending on ENMMBE bit
	if (data & CONTROL_ENMMBE)
		m_error |= ERROR_VALID;
	//else
	//  m_error &= ~ERROR_VALID;
}
