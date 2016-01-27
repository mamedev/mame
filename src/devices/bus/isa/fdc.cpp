// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    ISA 8 bit Floppy Disk Controller

**********************************************************************/

#include "emu.h"
#include "fdc.h"
#include "machine/pc_fdc.h"
#include "imagedev/flopdrv.h"
#include "formats/pc_dsk.h"
#include "formats/naslite_dsk.h"


FLOPPY_FORMATS_MEMBER( isa8_fdc_device::floppy_formats )
	FLOPPY_PC_FORMAT,
	FLOPPY_NASLITE_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( pc_dd_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

static SLOT_INTERFACE_START( pc_hd_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
	SLOT_INTERFACE( "35hd", FLOPPY_35_HD )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( cfg_xt )
	MCFG_PC_FDC_XT_ADD("fdc")
	MCFG_PC_FDC_INTRQ_CALLBACK(WRITELINE(isa8_fdc_device, irq_w))
	MCFG_PC_FDC_DRQ_CALLBACK(WRITELINE(isa8_fdc_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pc_dd_floppies, "525dd", isa8_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pc_dd_floppies, "525dd", isa8_fdc_device::floppy_formats)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( cfg_at )
	MCFG_PC_FDC_AT_ADD("fdc")
	MCFG_PC_FDC_INTRQ_CALLBACK(WRITELINE(isa8_fdc_device, irq_w))
	MCFG_PC_FDC_DRQ_CALLBACK(WRITELINE(isa8_fdc_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( cfg_smc )
	MCFG_SMC37C78_ADD("fdc")
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(isa8_fdc_device, irq_w))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(isa8_fdc_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( cfg_ps2 )
	MCFG_N82077AA_ADD("fdc", n82077aa_device::MODE_PS2)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(isa8_fdc_device, irq_w))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(isa8_fdc_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( cfg_superio )
	MCFG_PC_FDC_SUPERIO_ADD("fdc")
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(isa8_fdc_device, irq_w))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(isa8_fdc_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
MACHINE_CONFIG_END


isa8_fdc_device::isa8_fdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_isa8_card_interface(mconfig, *this),
	fdc(*this, "fdc")
{
}

void isa8_fdc_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x03f0, 0x03f7, *fdc, &pc_fdc_interface::map);
	m_isa->set_dma_channel(2, this, TRUE);
}

void isa8_fdc_device::device_reset()
{
}

WRITE_LINE_MEMBER( isa8_fdc_device::irq_w )
{
	m_isa->irq6_w(state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( isa8_fdc_device::drq_w )
{
	m_isa->drq2_w(state ? ASSERT_LINE : CLEAR_LINE);
}

UINT8 isa8_fdc_device::dack_r(int line)
{
	return fdc->dma_r();
}

void isa8_fdc_device::dack_w(int line, UINT8 data)
{
	return fdc->dma_w(data);
}

void isa8_fdc_device::eop_w(int state)
{
	fdc->tc_w(state == ASSERT_LINE);
}

isa8_fdc_xt_device::isa8_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : isa8_fdc_device(mconfig, ISA8_FDC_XT, "ISA 8bits XT FDC hookup", tag, owner, clock, "isa8_fdc_xt", __FILE__)
{
}

machine_config_constructor isa8_fdc_xt_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(cfg_xt);
}

isa8_fdc_at_device::isa8_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : isa8_fdc_device(mconfig, ISA8_FDC_AT, "ISA 8bits AT FDC hookup", tag, owner, clock, "isa8_fdc_at", __FILE__)
{
}

machine_config_constructor isa8_fdc_at_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(cfg_at);
}

isa8_fdc_smc_device::isa8_fdc_smc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : isa8_fdc_device(mconfig, ISA8_FDC_XT, "ISA 8bits SMC FDC hookup", tag, owner, clock, "isa8_fdc_smc", __FILE__)
{
}

machine_config_constructor isa8_fdc_smc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(cfg_smc);
}

isa8_fdc_ps2_device::isa8_fdc_ps2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : isa8_fdc_device(mconfig, ISA8_FDC_PS2, "ISA 8bits PS/2 FDC hookup", tag, owner, clock, "isa8_fdc_ps2", __FILE__)
{
}

machine_config_constructor isa8_fdc_ps2_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(cfg_ps2);
}

isa8_fdc_superio_device::isa8_fdc_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : isa8_fdc_device(mconfig, ISA8_FDC_SUPERIO, "ISA 8bits SUPERIO FDC hookup", tag, owner, clock, "isa8_fdc_superio", __FILE__)
{
}

machine_config_constructor isa8_fdc_superio_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(cfg_superio);
}

const device_type ISA8_FDC_XT = &device_creator<isa8_fdc_xt_device>;
const device_type ISA8_FDC_AT = &device_creator<isa8_fdc_at_device>;
const device_type ISA8_FDC_SMC = &device_creator<isa8_fdc_smc_device>;
const device_type ISA8_FDC_PS2 = &device_creator<isa8_fdc_ps2_device>;
const device_type ISA8_FDC_SUPERIO = &device_creator<isa8_fdc_superio_device>;
