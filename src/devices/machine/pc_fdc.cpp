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

const device_type PC_FDC_XT = &device_creator<pc_fdc_xt_device>;
const device_type PC_FDC_AT = &device_creator<pc_fdc_at_device>;

static MACHINE_CONFIG_FRAGMENT( cfg )
	MCFG_UPD765A_ADD("upd765", false, false)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(pc_fdc_family_device, irq_w))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(pc_fdc_family_device, drq_w))
MACHINE_CONFIG_END

DEVICE_ADDRESS_MAP_START(map, 8, pc_fdc_family_device)
ADDRESS_MAP_END

// The schematics show address decoding is minimal
DEVICE_ADDRESS_MAP_START(map, 8, pc_fdc_xt_device)
	AM_RANGE(0x0, 0x0) AM_DEVREAD("upd765", upd765a_device, msr_r) AM_WRITE(dor_w)
	AM_RANGE(0x1, 0x1) AM_DEVREAD("upd765", upd765a_device, fifo_r) AM_WRITE(dor_fifo_w)
	AM_RANGE(0x2, 0x2) AM_WRITE(dor_w)
	AM_RANGE(0x3, 0x3) AM_WRITE(dor_w)
	AM_RANGE(0x4, 0x5) AM_DEVICE("upd765", upd765a_device, map)
ADDRESS_MAP_END


// Decoding is through a PAL, so presumably complete
DEVICE_ADDRESS_MAP_START(map, 8, pc_fdc_at_device)
	AM_RANGE(0x2, 0x2) AM_READWRITE(dor_r, dor_w)
	AM_RANGE(0x4, 0x5) AM_DEVICE("upd765", upd765a_device, map)
	AM_RANGE(0x7, 0x7) AM_READWRITE(dir_r, ccr_w)
ADDRESS_MAP_END

pc_fdc_family_device::pc_fdc_family_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	pc_fdc_interface(mconfig, type, name, tag, owner, clock, shortname, source), fdc(*this, "upd765"),
	intrq_cb(*this),
	drq_cb(*this)
{
}

void pc_fdc_family_device::tc_w(bool state)
{
	fdc->tc_w(state);
}

UINT8 pc_fdc_family_device::dma_r()
{
	return fdc->dma_r();
}

void pc_fdc_family_device::dma_w(UINT8 data)
{
	fdc->dma_w(data);
}

machine_config_constructor pc_fdc_family_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(cfg);
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
	logerror("%s: dor = %02x\n", tag(), data);
	UINT8 pdor = dor;
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
	logerror("%s: ccr = %02x\n", tag(), data);
	fdc->set_rate(rates[data & 3]);
}

UINT8 pc_fdc_family_device::do_dir_r()
{
	if(floppy[dor & 3])
		return floppy[dor & 3]->dskchg_r() ? 0x00 : 0x80;
	return 0x00;
}

WRITE8_MEMBER( pc_fdc_xt_device::dor_fifo_w)
{
	fdc->fifo_w(space, 0, data, mem_mask);
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
		logerror("%s: pc_irq = %d\n", tag(), irq);
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

pc_fdc_xt_device::pc_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : pc_fdc_family_device(mconfig, PC_FDC_XT, "PC FDC XT", tag, owner, clock, "pc_fdc_xt", __FILE__)
{
}

pc_fdc_at_device::pc_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : pc_fdc_family_device(mconfig, PC_FDC_AT, "PC FDC AT", tag, owner, clock, "pc_fdc_at", __FILE__)
{
}
