// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
* An implementation of the MCGA device found on Intergraph InterPro family workstations. There is no
* public documentation on this device, so the implementation is being built to follow the logic of the
* system boot ROM and its diagnostic tests.
*
* Please be aware that code in here is not only broken, it's likely wrong in many cases.
*
* TODO
*   - too long to list
*/
#include "interpro_mctl.h"

#define VERBOSE 0
#if VERBOSE
#define LOG_MCGA(...) logerror(__VA_ARGS__)
#else
#define LOG_MCGA(...) {}
#endif

DEVICE_ADDRESS_MAP_START(map, 32, interpro_mcga_device)
	AM_RANGE(0x00, 0x03) AM_READWRITE16(reg00_r, reg00_w, 0xffff)
	AM_RANGE(0x08, 0x0b) AM_READWRITE16(control_r, control_w, 0xffff)
	AM_RANGE(0x10, 0x13) AM_READWRITE16(error_r, error_w, 0xffff)
	AM_RANGE(0x18, 0x1b) AM_READWRITE8(frcrd_r, frcrd_w, 0xff)
	AM_RANGE(0x20, 0x23) AM_READWRITE8(cbsub_r, cbsub_w, 0xff)
	AM_RANGE(0x28, 0x2b) AM_READWRITE16(reg28_r, reg28_w, 0xffff)
	AM_RANGE(0x30, 0x33) AM_READWRITE16(reg30_r, reg30_w, 0xffff)
	AM_RANGE(0x38, 0x3b) AM_READWRITE16(memsize_r, memsize_w, 0xffff)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(map, 32, interpro_fmcc_device)
	AM_RANGE(0x00, 0x03) AM_READWRITE16(reg00_r, reg00_w, 0xffff)
	AM_RANGE(0x08, 0x0b) AM_READWRITE16(control_r, control_w, 0xffff)
	AM_RANGE(0x10, 0x13) AM_READWRITE16(error_r, error_w, 0xffff)
	AM_RANGE(0x18, 0x1b) AM_READWRITE8(frcrd_r, frcrd_w, 0xff)
	AM_RANGE(0x20, 0x23) AM_READWRITE8(cbsub_r, cbsub_w, 0xff)
	AM_RANGE(0x28, 0x2b) AM_READWRITE16(reg28_r, reg28_w, 0xffff)
	AM_RANGE(0x30, 0x33) AM_READWRITE16(reg30_r, reg30_w, 0xffff)
	AM_RANGE(0x38, 0x3b) AM_READWRITE16(memsize_r, memsize_w, 0xffff)
	AM_RANGE(0x48, 0x4b) AM_READWRITE16(error_control_r, error_control_w, 0xffff)
ADDRESS_MAP_END

const device_type INTERPRO_MCGA = &device_creator<interpro_mcga_device>;
const device_type INTERPRO_FMCC = &device_creator<interpro_fmcc_device>;

interpro_mcga_device::interpro_mcga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

interpro_mcga_device::interpro_mcga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: interpro_mcga_device(mconfig, INTERPRO_MCGA, "InterPro MCGA", tag, owner, clock, "mcga", __FILE__)
{
}

interpro_fmcc_device::interpro_fmcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: interpro_mcga_device(mconfig, INTERPRO_FMCC, "InterPro FMCC", tag, owner, clock, "fmcc", __FILE__)
{
}

void interpro_mcga_device::device_start()
{
}

void interpro_mcga_device::device_reset()
{
	m_reg[0] = 0x00ff;
	m_control = MCGA_CTRL_ENREFRESH | MCGA_CTRL_CBITFRCSUB | MCGA_CTRL_CBITFRCRD;
	m_reg[1] = 0x00ff;
	m_memsize = 0x0340;
}

WRITE16_MEMBER(interpro_mcga_device::control_w)
{
	m_control = data & MCGA_CTRL_MASK;

	// HACK: set or clear error status depending on ENMMBE bit
	if (data & MCGA_CTRL_ENMMBE)
		m_error |= MCGA_ERROR_VALID;
	//		else
	//			error &= ~MCGA_ERROR_VALID;
}

WRITE16_MEMBER(interpro_fmcc_device::control_w)
{
	m_control = data & FMCC_CTRL_MASK;

	// HACK: set or clear error status depending on ENMMBE bit
	if (data & MCGA_CTRL_ENMMBE)
		m_error |= MCGA_ERROR_VALID;
	//		else
	//			error &= ~MCGA_ERROR_VALID;
}