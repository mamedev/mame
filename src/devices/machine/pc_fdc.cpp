// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    PC-style floppy disk controller emulation

    TODO:
        - check how the drive select from DOR register, and the drive select
        from the fdc are related !!!!
        - if all drives do not have a disk in them, and the fdc is reset, is a int generated?
        (if yes, indicates drives are ready without discs, if no indicates no drives are ready)
        - status register a, status register b

**********************************************************************/

#include "emu.h"
#include "machine/pc_fdc.h"
#include "imagedev/floppy.h"

//#define LOG_GENERAL   (1U << 0) //defined in logmacro.h already

//#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"


DEFINE_DEVICE_TYPE(PC_FDC_XT, pc_fdc_xt_device, "pc_fdc_xt", "PC FDC (XT)")
DEFINE_DEVICE_TYPE(PC_FDC_AT, pc_fdc_at_device, "pc_fdc_at", "PC FDC (AT)")

void pc_fdc_family_device::map(address_map &map)
{
}

// The schematics show address decoding is minimal
void pc_fdc_xt_device::map(address_map &map)
{
	map(0x0, 0x0).r(fdc, FUNC(upd765a_device::msr_r)).w(FUNC(pc_fdc_xt_device::dor_w));
	map(0x1, 0x1).r(fdc, FUNC(upd765a_device::fifo_r)).w(FUNC(pc_fdc_xt_device::dor_fifo_w));
	map(0x2, 0x2).w(FUNC(pc_fdc_xt_device::dor_w));
	map(0x3, 0x3).w(FUNC(pc_fdc_xt_device::dor_w));
	map(0x4, 0x5).m(fdc, FUNC(upd765a_device::map));
}


// Decoding is through a PAL, so presumably complete
void pc_fdc_at_device::map(address_map &map)
{
	map(0x2, 0x2).rw(FUNC(pc_fdc_at_device::dor_r), FUNC(pc_fdc_at_device::dor_w));
	map(0x4, 0x5).m(fdc, FUNC(upd765a_device::map));
	map(0x7, 0x7).rw(FUNC(pc_fdc_at_device::dir_r), FUNC(pc_fdc_at_device::ccr_w));
}

pc_fdc_family_device::pc_fdc_family_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	pc_fdc_interface(mconfig, type, tag, owner, clock), fdc(*this, "upd765"),
	intrq_cb(*this),
	drq_cb(*this)
{
}

void pc_fdc_family_device::tc_w(bool state)
{
	fdc->tc_w(state);
}

uint8_t pc_fdc_family_device::dma_r()
{
	return fdc->dma_r();
}

void pc_fdc_family_device::dma_w(uint8_t data)
{
	fdc->dma_w(data);
}

void pc_fdc_family_device::device_add_mconfig(machine_config &config)
{
	UPD765A(config, fdc, 8'000'000, false, false);
	fdc->intrq_wr_callback().set(FUNC(pc_fdc_family_device::irq_w));
	fdc->drq_wr_callback().set(FUNC(pc_fdc_family_device::drq_w));
}

void pc_fdc_family_device::device_start()
{
	intrq_cb.resolve();
	drq_cb.resolve();

	for(int i=0; i<4; i++) {
		char name[2] = {static_cast<char>('0'+i), 0};
		floppy_connector *conn = subdevice<floppy_connector>(name);
		floppy[i] = conn ? conn->get_device() : nullptr;
	}

	irq = drq = false;
	fdc_irq = fdc_drq = false;
	dor = 0x00;
}

void pc_fdc_family_device::device_reset()
{
}

// Bits 0-1 select one of the 4 drives, but only if the associated
// motor bit is on

// Bit 2 is tied to the upd765 reset line

// Bit 3 enables the irq and drq lines

// Bit 4-7 control the drive motors

WRITE8_MEMBER( pc_fdc_family_device::dor_w )
{
	LOG("dor = %02x\n", data);
	uint8_t pdor = dor;
	dor = data;

	for(int i=0; i<4; i++)
		if(floppy[i])
			floppy[i]->mon_w(!(dor & (0x10 << i)));

	int fid = dor & 3;
	if(dor & (0x10 << fid))
		fdc->set_floppy(floppy[fid]);
	else
		fdc->set_floppy(nullptr);

	check_irq();
	check_drq();
	if((pdor^dor) & 4)
		fdc->reset();
}

READ8_MEMBER( pc_fdc_family_device::dor_r )
{
	return dor;
}

READ8_MEMBER( pc_fdc_family_device::dir_r )
{
	return do_dir_r();
}

WRITE8_MEMBER( pc_fdc_family_device::ccr_w )
{
	static const int rates[4] = { 500000, 300000, 250000, 1000000 };
	LOG("ccr = %02x\n", data);
	fdc->set_rate(rates[data & 3]);
}

uint8_t pc_fdc_family_device::do_dir_r()
{
	if(floppy[dor & 3])
		return floppy[dor & 3]->dskchg_r() ? 0x00 : 0x80;
	return 0x00;
}

WRITE8_MEMBER( pc_fdc_xt_device::dor_fifo_w)
{
	fdc->fifo_w(data);
	dor_w(space, 0, data, mem_mask);
}

WRITE_LINE_MEMBER( pc_fdc_family_device::irq_w )
{
	fdc_irq = state;
	check_irq();
}

WRITE_LINE_MEMBER( pc_fdc_family_device::drq_w )
{
	fdc_drq = state;
	check_drq();
}

void pc_fdc_family_device::check_irq()
{
	bool pirq = irq;
	irq = fdc_irq && (dor & 4) && (dor & 8);
	if(irq != pirq && !intrq_cb.isnull()) {
		LOG("pc_irq = %d\n", irq);
		intrq_cb(irq);
	}
}

void pc_fdc_family_device::check_drq()
{
	bool pdrq = drq;
	drq = fdc_drq && (dor & 4) && (dor & 8);
	if(drq != pdrq && !drq_cb.isnull())
		drq_cb(drq);
}

pc_fdc_xt_device::pc_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : pc_fdc_family_device(mconfig, PC_FDC_XT, tag, owner, clock)
{
}

pc_fdc_at_device::pc_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : pc_fdc_family_device(mconfig, PC_FDC_AT, tag, owner, clock)
{
}
