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

static void pc_dd_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

static void pc_hd_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}


isa8_fdc_device::isa8_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	fdc(*this, "fdc")
{
}

void isa8_fdc_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x03f0, 0x03f7, *fdc, &pc_fdc_interface::map);
	m_isa->set_dma_channel(2, this, true);
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

uint8_t isa8_fdc_device::dack_r(int line)
{
	return fdc->dma_r();
}

void isa8_fdc_device::dack_w(int line, uint8_t data)
{
	return fdc->dma_w(data);
}

void isa8_fdc_device::eop_w(int state)
{
	fdc->tc_w(state == ASSERT_LINE);
}


isa8_fdc_xt_device::isa8_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_XT, tag, owner, clock)
{
}

MACHINE_CONFIG_START(isa8_fdc_xt_device::device_add_mconfig)
	MCFG_PC_FDC_XT_ADD("fdc")
	MCFG_PC_FDC_INTRQ_CALLBACK(WRITELINE(*this, isa8_fdc_device, irq_w))
	MCFG_PC_FDC_DRQ_CALLBACK(WRITELINE(*this, isa8_fdc_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pc_dd_floppies, "525dd", isa8_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pc_dd_floppies, "525dd", isa8_fdc_device::floppy_formats)
MACHINE_CONFIG_END


isa8_fdc_at_device::isa8_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_AT, tag, owner, clock)
{
}

MACHINE_CONFIG_START(isa8_fdc_at_device::device_add_mconfig)
	MCFG_PC_FDC_AT_ADD("fdc")
	MCFG_PC_FDC_INTRQ_CALLBACK(WRITELINE(*this, isa8_fdc_device, irq_w))
	MCFG_PC_FDC_DRQ_CALLBACK(WRITELINE(*this, isa8_fdc_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
MACHINE_CONFIG_END

isa8_fdc_smc_device::isa8_fdc_smc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_SMC, tag, owner, clock)
{
}

MACHINE_CONFIG_START(isa8_fdc_smc_device::device_add_mconfig)
	MCFG_SMC37C78_ADD("fdc")
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(*this, isa8_fdc_device, irq_w))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(*this, isa8_fdc_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
MACHINE_CONFIG_END

isa8_fdc_ps2_device::isa8_fdc_ps2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_PS2, tag, owner, clock)
{
}

MACHINE_CONFIG_START(isa8_fdc_ps2_device::device_add_mconfig)
	MCFG_N82077AA_ADD("fdc", n82077aa_device::MODE_PS2)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(*this, isa8_fdc_device, irq_w))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(*this, isa8_fdc_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
MACHINE_CONFIG_END

isa8_fdc_superio_device::isa8_fdc_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_SUPERIO, tag, owner, clock)
{
}

MACHINE_CONFIG_START(isa8_fdc_superio_device::device_add_mconfig)
	MCFG_PC_FDC_SUPERIO_ADD("fdc")
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(*this, isa8_fdc_device, irq_w))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(*this, isa8_fdc_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats)
MACHINE_CONFIG_END

DEFINE_DEVICE_TYPE(ISA8_FDC_XT,      isa8_fdc_xt_device,      "isa8_fdc_xt",      "ISA 8bits XT FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_AT,      isa8_fdc_at_device,      "isa8_fdc_at",      "ISA 8bits AT FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_SMC,     isa8_fdc_smc_device,     "isa8_fdc_smc",     "ISA 8bits SMC FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_PS2,     isa8_fdc_ps2_device,     "isa8_fdc_ps2",     "ISA 8bits PS/2 FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_SUPERIO, isa8_fdc_superio_device, "isa8_fdc_superio", "ISA 8bits SUPERIO FDC hookup")
