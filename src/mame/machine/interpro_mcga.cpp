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

ADDRESS_MAP_START(interpro_mcga_device::map)
	AM_RANGE(0x00, 0x03) AM_READWRITE16(reg00_r, reg00_w, 0xffff)
	AM_RANGE(0x08, 0x0b) AM_READWRITE16(control_r, control_w, 0xffff)
	AM_RANGE(0x10, 0x13) AM_READWRITE16(error_r, error_w, 0xffff)
	AM_RANGE(0x18, 0x1b) AM_READWRITE8(frcrd_r, frcrd_w, 0xff)
	AM_RANGE(0x20, 0x23) AM_READWRITE8(cbsub_r, cbsub_w, 0xff)
	AM_RANGE(0x28, 0x2b) AM_READWRITE16(reg28_r, reg28_w, 0xffff)
	AM_RANGE(0x30, 0x33) AM_READWRITE16(reg30_r, reg30_w, 0xffff)
	AM_RANGE(0x38, 0x3b) AM_READWRITE16(memsize_r, memsize_w, 0xffff)
ADDRESS_MAP_END

ADDRESS_MAP_START(interpro_fmcc_device::map)
	AM_RANGE(0x00, 0x03) AM_READWRITE16(reg00_r, reg00_w, 0xffff)
	AM_RANGE(0x08, 0x0b) AM_READWRITE16(control_r, control_w, 0xffff)
	AM_RANGE(0x10, 0x13) AM_READWRITE16(error_r, error_w, 0xffff)
	AM_RANGE(0x18, 0x1b) AM_READWRITE8(frcrd_r, frcrd_w, 0xff)
	AM_RANGE(0x20, 0x23) AM_READWRITE8(cbsub_r, cbsub_w, 0xff)
	AM_RANGE(0x28, 0x2b) AM_READWRITE16(reg28_r, reg28_w, 0xffff)
	AM_RANGE(0x30, 0x33) AM_READWRITE16(reg30_r, reg30_w, 0xffff)
	AM_RANGE(0x38, 0x3b) AM_READWRITE16(memsize_r, memsize_w, 0xffff)
	AM_RANGE(0x40, 0x43) AM_NOP // unknown
	AM_RANGE(0x48, 0x4b) AM_READWRITE16(error_control_r, error_control_w, 0xffff)
ADDRESS_MAP_END

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

WRITE16_MEMBER(interpro_mcga_device::control_w)
{
	m_control = data & CONTROL_MASK;

	// HACK: set or clear error status depending on ENMMBE bit
	if (data & CONTROL_ENMMBE)
		m_error |= ERROR_VALID;
	//else
	//  m_error &= ~ERROR_VALID;
}

WRITE16_MEMBER(interpro_fmcc_device::control_w)
{
	m_control = data & CONTROL_MASK;

	// HACK: set or clear error status depending on ENMMBE bit
	if (data & CONTROL_ENMMBE)
		m_error |= ERROR_VALID;
	//else
	//  m_error &= ~ERROR_VALID;
}
