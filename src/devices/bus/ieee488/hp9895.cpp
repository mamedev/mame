// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9895.cpp

    HP9895 floppy disk drive

    Reference manual:
    HP 09895-90030, feb 81, 9895A Flexible Disc Memory Service Manual

*********************************************************************/

#include "hp9895.h"

// Debugging
#define VERBOSE 1
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

// device type definition
const device_type HP9895 = &device_creator<hp9895_device>;

hp9895_device::hp9895_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP9895, "HP9895", tag, owner, clock, "HP9895", __FILE__),
	  device_ieee488_interface(mconfig, *this),
	  m_cpu(*this , "cpu"),
	  m_phi(*this , "phi")
{
}

#if 0
ioport_constructor hp9895_device::device_input_ports() const
{
	// TODO: inputs=HPIB address, "S" & "W" switches, "loop" pin
}
#endif
void hp9895_device::device_start()
{
}

void hp9895_device::device_reset()
{
	m_cpu_irq = false;
}

void hp9895_device::ieee488_eoi(int state)
{
	m_phi->eoi_w(state);
}

void hp9895_device::ieee488_dav(int state)
{
	m_phi->dav_w(state);
}

void hp9895_device::ieee488_nrfd(int state)
{
	m_phi->nrfd_w(state);
}

void hp9895_device::ieee488_ndac(int state)
{
	m_phi->ndac_w(state);
}

void hp9895_device::ieee488_ifc(int state)
{
	m_phi->ifc_w(state);
}

void hp9895_device::ieee488_srq(int state)
{
	m_phi->srq_w(state);
}

void hp9895_device::ieee488_atn(int state)
{
	m_phi->atn_w(state);
}

void hp9895_device::ieee488_ren(int state)
{
	m_phi->ren_w(state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_eoi_w)
{
	m_bus->eoi_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_dav_w)
{
	m_bus->dav_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_nrfd_w)
{
	m_bus->nrfd_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_ndac_w)
{
	m_bus->ndac_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_ifc_w)
{
	m_bus->ifc_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_srq_w)
{
	m_bus->srq_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_atn_w)
{
	m_bus->atn_w(this , state);
}

WRITE_LINE_MEMBER(hp9895_device::phi_ren_w)
{
	m_bus->ren_w(this , state);
}

READ8_MEMBER(hp9895_device::phi_dio_r)
{
	return m_bus->dio_r();
}

WRITE8_MEMBER(hp9895_device::phi_dio_w)
{
	m_bus->dio_w(this , data);
}

WRITE_LINE_MEMBER(hp9895_device::phi_int_w)
{
	m_cpu->set_input_line(INPUT_LINE_NMI , state);
}

READ8_MEMBER(hp9895_device::phi_reg_r)
{
	uint16_t reg = m_phi->reg16_r(space , offset , mem_mask);

	// Reading D1=1 from a register sets the Z80 IRQ line
	if (BIT(reg , 14) && !m_cpu_irq) {
		m_cpu_irq = true;
		m_cpu->set_input_line(INPUT_LINE_IRQ0 , ASSERT_LINE);
	}

	return (uint8_t)reg;
}

WRITE16_MEMBER(hp9895_device::z80_m1_w)
{
	// Every M1 cycle of Z80 clears the IRQ line
	if (m_cpu_irq) {
		m_cpu_irq = false;
		m_cpu->set_input_line(INPUT_LINE_IRQ0 , CLEAR_LINE);
	}
}

ROM_START(hp9895)
	ROM_REGION(0x2000 , "cpu" , 0)
	ROM_LOAD("1818-1391a.bin" , 0 , 0x2000 , CRC(b50dbfb5) SHA1(96edf9af78be75fbad2a0245b8af43958ba32752))
ROM_END

static ADDRESS_MAP_START(z80_program_map , AS_PROGRAM , 8 , hp9895_device)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000 , 0x1fff) AM_ROM AM_REGION("cpu" , 0)
	AM_RANGE(0x6000 , 0x63ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(z80_io_map , AS_IO , 8 , hp9895_device)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10 , 0x17) AM_DEVWRITE("phi" , phi_device , reg8_w) AM_READ(phi_reg_r)
	// TODO: 60-67 range
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT(hp9895)
	MCFG_CPU_ADD("cpu" , Z80 , 4000000)
	MCFG_CPU_PROGRAM_MAP(z80_program_map)
	MCFG_CPU_IO_MAP(z80_io_map)
	MCFG_Z80_SET_REFRESH_CALLBACK(WRITE16(hp9895_device , z80_m1_w))

	MCFG_DEVICE_ADD("phi" , PHI , 0)
	MCFG_PHI_EOI_WRITE_CB(WRITELINE(hp9895_device , phi_eoi_w))
	MCFG_PHI_DAV_WRITE_CB(WRITELINE(hp9895_device , phi_dav_w))
	MCFG_PHI_NRFD_WRITE_CB(WRITELINE(hp9895_device , phi_nrfd_w))
	MCFG_PHI_NDAC_WRITE_CB(WRITELINE(hp9895_device , phi_ndac_w))
	MCFG_PHI_IFC_WRITE_CB(WRITELINE(hp9895_device , phi_ifc_w))
	MCFG_PHI_SRQ_WRITE_CB(WRITELINE(hp9895_device , phi_srq_w))
	MCFG_PHI_ATN_WRITE_CB(WRITELINE(hp9895_device , phi_atn_w))
	MCFG_PHI_REN_WRITE_CB(WRITELINE(hp9895_device , phi_ren_w))
	MCFG_PHI_DIO_READWRITE_CB(READ8(hp9895_device , phi_dio_r) , WRITE8(hp9895_device , phi_dio_w))
	MCFG_PHI_INT_WRITE_CB(WRITELINE(hp9895_device , phi_int_w))
MACHINE_CONFIG_END

const tiny_rom_entry *hp9895_device::device_rom_region() const
{
	return ROM_NAME(hp9895);
}

machine_config_constructor hp9895_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(hp9895);
}
