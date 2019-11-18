// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    ISA 8 bit Floppy Disk Controller

**********************************************************************/

#include "emu.h"
#include "fdc.h"
#include "machine/busmouse.h"
#include "machine/pc_fdc.h"
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
	m_fdc(*this, "fdc")
{
}

void isa8_fdc_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x03f0, 0x03f7, *m_fdc, &pc_fdc_interface::map);
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
	return m_fdc->dma_r();
}

void isa8_fdc_device::dack_w(int line, uint8_t data)
{
	return m_fdc->dma_w(data);
}

void isa8_fdc_device::dack_line_w(int line, int state)
{
	//m_fdc->dack_w(state);
}

void isa8_fdc_device::eop_w(int state)
{
	m_fdc->tc_w(state == ASSERT_LINE);
}


isa8_fdc_xt_device::isa8_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_XT, tag, owner, clock)
{
}

void isa8_fdc_xt_device::device_add_mconfig(machine_config &config)
{
	pc_fdc_xt_device &pc_fdc_xt(PC_FDC_XT(config, m_fdc, 0));
	pc_fdc_xt.intrq_wr_callback().set(FUNC(isa8_fdc_device::irq_w));
	pc_fdc_xt.drq_wr_callback().set(FUNC(isa8_fdc_device::drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_dd_floppies, "525dd", isa8_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_dd_floppies, "525dd", isa8_fdc_device::floppy_formats);
}


isa8_fdc_at_device::isa8_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_AT, tag, owner, clock)
{
}

void isa8_fdc_at_device::device_add_mconfig(machine_config &config)
{
	pc_fdc_at_device &pc_fdc_at(PC_FDC_AT(config, m_fdc, 0));
	pc_fdc_at.intrq_wr_callback().set(FUNC(isa8_fdc_device::irq_w));
	pc_fdc_at.drq_wr_callback().set(FUNC(isa8_fdc_device::drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats);
}

isa8_fdc_smc_device::isa8_fdc_smc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_SMC, tag, owner, clock)
{
}

void isa8_fdc_smc_device::device_add_mconfig(machine_config &config)
{
	smc37c78_device &smc(SMC37C78(config, m_fdc, 24'000'000));
	smc.intrq_wr_callback().set(FUNC(isa8_fdc_device::irq_w));
	smc.drq_wr_callback().set(FUNC(isa8_fdc_device::drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats);
}

isa8_fdc_ps2_device::isa8_fdc_ps2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_PS2, tag, owner, clock)
{
}

void isa8_fdc_ps2_device::device_add_mconfig(machine_config &config)
{
	n82077aa_device &n82077aa(N82077AA(config, m_fdc, 24'000'000));
	n82077aa.set_mode(n82077aa_device::mode_t::PS2);
	n82077aa.intrq_wr_callback().set(FUNC(isa8_fdc_device::irq_w));
	n82077aa.drq_wr_callback().set(FUNC(isa8_fdc_device::drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats);
}

isa8_fdc_superio_device::isa8_fdc_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_SUPERIO, tag, owner, clock)
{
}

void isa8_fdc_superio_device::device_add_mconfig(machine_config &config)
{
	pc_fdc_superio_device &superio(PC_FDC_SUPERIO(config, m_fdc, 24'000'000));
	superio.intrq_wr_callback().set(FUNC(isa8_fdc_device::irq_w));
	superio.drq_wr_callback().set(FUNC(isa8_fdc_device::drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats);
}

isa8_ec1841_0003_device::isa8_ec1841_0003_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: isa8_fdc_device(mconfig, ISA8_EC1841_0003, tag, owner, clock)
	, m_bus_mouse(*this, "bus_mouse")
{
}

void isa8_ec1841_0003_device::device_start()
{
	isa8_fdc_device::device_start();
	m_isa->install_device(0x023c, 0x023f, *m_bus_mouse, &bus_mouse_device::map);
}

WRITE_LINE_MEMBER( isa8_ec1841_0003_device::aux_irq_w )
{
	m_isa->irq4_w(state ? ASSERT_LINE : CLEAR_LINE);
}

void isa8_ec1841_0003_device::device_add_mconfig(machine_config &config)
{
	pc_fdc_xt_device &pc_fdc_xt(PC_FDC_XT(config, m_fdc, 0));
	pc_fdc_xt.intrq_wr_callback().set(FUNC(isa8_fdc_device::irq_w));
	pc_fdc_xt.drq_wr_callback().set(FUNC(isa8_fdc_device::drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_dd_floppies, "525dd", isa8_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_dd_floppies, "525dd", isa8_fdc_device::floppy_formats);

	BUS_MOUSE(config, "bus_mouse", 0).extint_callback().set(FUNC(isa8_ec1841_0003_device::aux_irq_w));
}


DEFINE_DEVICE_TYPE(ISA8_FDC_XT,      isa8_fdc_xt_device,      "isa8_fdc_xt",      "ISA 8bits XT FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_AT,      isa8_fdc_at_device,      "isa8_fdc_at",      "ISA 8bits AT FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_SMC,     isa8_fdc_smc_device,     "isa8_fdc_smc",     "ISA 8bits SMC FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_PS2,     isa8_fdc_ps2_device,     "isa8_fdc_ps2",     "ISA 8bits PS/2 FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_SUPERIO, isa8_fdc_superio_device, "isa8_fdc_superio", "ISA 8bits SUPERIO FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_EC1841_0003, isa8_ec1841_0003_device, "isa8_ec1841_0003", "ISA 8bits EC1841.0003 FDC hookup")
