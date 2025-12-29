// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Wilbert Pol
/**********************************************************************

    ISA 8 bit Floppy Disk Controller

**********************************************************************/

#include "emu.h"
#include "fdc.h"
#include "machine/busmouse.h"
#include "formats/naslite_dsk.h"
#include "formats/ibmxdf_dsk.h"

#define VERBOSE 0
#include "logmacro.h"


void isa8_fdc_device::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
	fr.add(FLOPPY_IBMXDF_FORMAT);
}

static void pc_dd_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

static void pc_qd_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525qd", FLOPPY_525_QD);
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

void isa8_fdc_device::irq_w(int state)
{
	m_isa->irq6_w(state ? ASSERT_LINE : CLEAR_LINE);
}

void isa8_fdc_device::drq_w(int state)
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


isa8_upd765_fdc_device::isa8_upd765_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: isa8_fdc_device(mconfig, type, tag, owner, clock)
	, dor(0x00)
{
}

void isa8_upd765_fdc_device::device_start()
{
	for(int i=0; i<4; i++) {
		char name[2] = {static_cast<char>('0'+i), 0};
		floppy_connector *conn = m_fdc->subdevice<floppy_connector>(name);
		floppy[i] = conn ? conn->get_device() : nullptr;
	}

	irq = drq = false;
	fdc_irq = fdc_drq = false;
}

void isa8_upd765_fdc_device::device_reset()
{
	dor_w(0x00);
}

// Bits 0-1 select one of the 4 drives, but only if the associated
// motor bit is on

// Bit 2 is tied to the upd765 reset line

// Bit 3 enables the irq and drq lines

// Bit 4-7 control the drive motors

void isa8_upd765_fdc_device::dor_w(uint8_t data)
{
	LOG("dor = %02x\n", data);
	dor = data;

	for(int i=0; i<4; i++)
		if(floppy[i])
			floppy[i]->mon_w(!(dor & (0x10 << i)));

	int fid = dor & 3;
	if(dor & (0x10 << fid))
		m_fdc->set_floppy(floppy[fid]);
	else
		m_fdc->set_floppy(nullptr);

	check_irq();
	check_drq();
	m_fdc->reset_w(!BIT(dor, 2));
}

uint8_t isa8_upd765_fdc_device::dor_r()
{
	return dor;
}

void isa8_upd765_fdc_device::ccr_w(uint8_t data)
{
	static const int rates[4] = { 500000, 300000, 250000, 1000000 };
	LOG("ccr = %02x\n", data);
	m_fdc->set_rate(rates[data & 3]);
}

uint8_t isa8_upd765_fdc_device::dir_r()
{
	if(floppy[dor & 3])
		return floppy[dor & 3]->dskchg_r() ? 0x00 : 0x80;
	return 0x00;
}

void isa8_upd765_fdc_device::fdc_irq_w(int state)
{
	fdc_irq = state;
	check_irq();
}

void isa8_upd765_fdc_device::fdc_drq_w(int state)
{
	fdc_drq = state;
	check_drq();
}

void isa8_upd765_fdc_device::check_irq()
{
	bool pirq = irq;
	irq = fdc_irq && (dor & 4) && (dor & 8);
	if(irq != pirq) {
		LOG("pc_irq = %d\n", irq);
		irq_w(irq);
	}
}

void isa8_upd765_fdc_device::check_drq()
{
	bool pdrq = drq;
	drq = fdc_drq && (dor & 4) && (dor & 8);
	if(drq != pdrq)
		drq_w(drq);
}


isa8_fdc_xt_device::isa8_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_xt_device(mconfig, ISA8_FDC_XT, tag, owner, clock)
{
}

isa8_fdc_xt_device::isa8_fdc_xt_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) : isa8_upd765_fdc_device(mconfig, type, tag, owner, clock)
{
}

void isa8_fdc_xt_device::device_add_mconfig(machine_config &config)
{
	upd765a_device &upd765a(UPD765A(config, m_fdc, 8'000'000, false, false));
	upd765a.intrq_wr_callback().set(FUNC(isa8_fdc_xt_device::fdc_irq_w));
	upd765a.drq_wr_callback().set(FUNC(isa8_fdc_xt_device::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_dd_floppies, "525dd", isa8_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_dd_floppies, "525dd", isa8_fdc_device::floppy_formats).enable_sound(true);
}

void isa8_fdc_xt_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x03f0, 0x03f7, *this, &isa8_fdc_xt_device::map);
	m_isa->set_dma_channel(2, this, true);

	isa8_upd765_fdc_device::device_start();
}

// The schematics show address decoding is minimal
void isa8_fdc_xt_device::map(address_map &map)
{
	map(0x0, 0x0).r(m_fdc, FUNC(upd765a_device::msr_r)).w(FUNC(isa8_fdc_xt_device::dor_w));
	map(0x1, 0x1).r(m_fdc, FUNC(upd765a_device::fifo_r)).w(FUNC(isa8_fdc_xt_device::dor_fifo_w));
	map(0x2, 0x2).w(FUNC(isa8_fdc_xt_device::dor_w));
	map(0x3, 0x3).w(FUNC(isa8_fdc_xt_device::dor_w));
	map(0x4, 0x5).m(m_fdc, FUNC(upd765a_device::map));
}

void isa8_fdc_xt_device::dor_fifo_w(uint8_t data)
{
	m_fdc->fifo_w(data);
	dor_w(data);
}


isa8_fdc_at_device::isa8_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_upd765_fdc_device(mconfig, ISA8_FDC_AT, tag, owner, clock)
{
}

void isa8_fdc_at_device::device_add_mconfig(machine_config &config)
{
	upd765a_device &upd765a(UPD765A(config, m_fdc, 8'000'000, false, false));
	upd765a.intrq_wr_callback().set(FUNC(isa8_fdc_at_device::fdc_irq_w));
	upd765a.drq_wr_callback().set(FUNC(isa8_fdc_at_device::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats).enable_sound(true);
}

// Decoding is through a PAL, so presumably complete
void isa8_fdc_at_device::map(address_map &map)
{
	map(0x2, 0x2).rw(FUNC(isa8_fdc_at_device::dor_r), FUNC(isa8_fdc_at_device::dor_w));
	map(0x4, 0x5).m(m_fdc, FUNC(upd765a_device::map));
	map(0x7, 0x7).rw(FUNC(isa8_fdc_at_device::dir_r), FUNC(isa8_fdc_at_device::ccr_w));
}

void isa8_fdc_at_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x03f0, 0x03f7, *this, &isa8_fdc_at_device::map);
	m_isa->set_dma_channel(2, this, true);

	isa8_upd765_fdc_device::device_start();
}

isa8_fdc_smc_device::isa8_fdc_smc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_SMC, tag, owner, clock)
{
}

void isa8_fdc_smc_device::device_add_mconfig(machine_config &config)
{
	smc37c78_device &smc(SMC37C78(config, m_fdc, 24'000'000));
	smc.intrq_wr_callback().set(FUNC(isa8_fdc_device::irq_w));
	smc.drq_wr_callback().set(FUNC(isa8_fdc_device::drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats).enable_sound(true);
}

void isa8_fdc_smc_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x03f0, 0x03f7, downcast<smc37c78_device &>(*m_fdc), &smc37c78_device::map);
	m_isa->set_dma_channel(2, this, true);
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
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats).enable_sound(true);
}

void isa8_fdc_ps2_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x03f0, 0x03f7, downcast<n82077aa_device &>(*m_fdc), &n82077aa_device::map);
	m_isa->set_dma_channel(2, this, true);
}

isa8_fdc_superio_device::isa8_fdc_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_fdc_device(mconfig, ISA8_FDC_SUPERIO, tag, owner, clock)
{
}

void isa8_fdc_superio_device::device_add_mconfig(machine_config &config)
{
	pc_fdc_superio_device &superio(PC_FDC_SUPERIO(config, m_fdc, 24'000'000));
	superio.intrq_wr_callback().set(FUNC(isa8_fdc_device::irq_w));
	superio.drq_wr_callback().set(FUNC(isa8_fdc_device::drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", isa8_fdc_device::floppy_formats).enable_sound(true);
}

void isa8_fdc_superio_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x03f0, 0x03f7, downcast<pc_fdc_superio_device &>(*m_fdc), &pc_fdc_superio_device::map);
	m_isa->set_dma_channel(2, this, true);
}

isa8_ec1841_0003_device::isa8_ec1841_0003_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: isa8_fdc_xt_device(mconfig, ISA8_EC1841_0003, tag, owner, clock)
	, m_bus_mouse(*this, "bus_mouse")
{
}

void isa8_ec1841_0003_device::device_start()
{
	isa8_fdc_xt_device::device_start();
	m_isa->install_device(0x023c, 0x023f, *m_bus_mouse, &bus_mouse_device::map);
}

void isa8_ec1841_0003_device::aux_irq_w(int state)
{
	m_isa->irq4_w(state ? ASSERT_LINE : CLEAR_LINE);
}

void isa8_ec1841_0003_device::device_add_mconfig(machine_config &config)
{
	isa8_fdc_xt_device::device_add_mconfig(config);
	FLOPPY_CONNECTOR(config.replace(), "fdc:0", pc_qd_floppies, "525dd", isa8_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config.replace(), "fdc:1", pc_qd_floppies, "525dd", isa8_fdc_device::floppy_formats).enable_sound(true);

	BUS_MOUSE(config, "bus_mouse", 0).extint_callback().set(FUNC(isa8_ec1841_0003_device::aux_irq_w));
}


DEFINE_DEVICE_TYPE(ISA8_FDC_XT,      isa8_fdc_xt_device,      "isa8_fdc_xt",      "ISA 8bits XT FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_AT,      isa8_fdc_at_device,      "isa8_fdc_at",      "ISA 8bits AT FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_SMC,     isa8_fdc_smc_device,     "isa8_fdc_smc",     "ISA 8bits SMC FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_PS2,     isa8_fdc_ps2_device,     "isa8_fdc_ps2",     "ISA 8bits PS/2 FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_FDC_SUPERIO, isa8_fdc_superio_device, "isa8_fdc_superio", "ISA 8bits SUPERIO FDC hookup")
DEFINE_DEVICE_TYPE(ISA8_EC1841_0003, isa8_ec1841_0003_device, "isa8_ec1841_0003", "ISA 8bits EC1841.0003 FDC hookup")
