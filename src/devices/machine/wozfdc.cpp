// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    wozfdc.c

    Implementation of the Apple Disk II floppy disk controller

*********************************************************************/

#include "emu.h"
#include "imagedev/floppy.h"
#include "formats/ap2_dsk.h"
#include "wozfdc.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type DISKII_FDC = &device_creator<diskii_fdc>;
const device_type APPLEIII_FDC = &device_creator<appleiii_fdc>;

#define DISKII_P6_REGION  "diskii_rom_p6"

ROM_START( diskiing )
	ROM_REGION(0x100, DISKII_P6_REGION, 0)
	ROM_LOAD( "341-0028-a.rom", 0x0000, 0x0100, CRC(b72a2c70) SHA1(bc39fbd5b9a8d2287ac5d0a42e639fc4d3c2f9d4)) /* 341-0028: 16-sector disk drive (older version), PROM P6 */
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *wozfdc_device::device_rom_region() const
{
	return ROM_NAME( diskiing );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

wozfdc_device::wozfdc_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

diskii_fdc::diskii_fdc(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	wozfdc_device(mconfig, DISKII_FDC, "Apple Disk II floppy controller", tag, owner, clock, "d2fdc", __FILE__)
{
}

appleiii_fdc::appleiii_fdc(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	wozfdc_device(mconfig, DISKII_FDC, "Apple III floppy controller", tag, owner, clock, "a3fdc", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wozfdc_device::device_start()
{
	m_rom_p6 = machine().root_device().memregion(this->subtag(DISKII_P6_REGION).c_str())->base();

	timer = timer_alloc(0);
	delay_timer = timer_alloc(1);

	save_item(NAME(last_6502_write));
	save_item(NAME(mode_write));
	save_item(NAME(mode_load));
	save_item(NAME(active));
	save_item(NAME(phases));
	save_item(NAME(external_io_select));
	save_item(NAME(cycles));
	save_item(NAME(data_reg));
	save_item(NAME(address));
	save_item(NAME(write_start_time));
	save_item(NAME(write_position));
	save_item(NAME(write_line_active));
	save_item(NAME(drvsel));
	save_item(NAME(enable1));
}

void wozfdc_device::device_reset()
{
	floppy = nullptr;
	active = MODE_IDLE;
	phases = 0x00;
	mode_write = false;
	mode_load = false;
	last_6502_write = 0x00;
	cycles = time_to_cycles(machine().time());
	data_reg = 0x00;
	address = 0x00;
	write_start_time = attotime::never;
	write_position = 0;
	write_line_active = false;
	external_io_select = false;

	// Just a timer to be sure that the lss is updated from time to
	// time, so that there's no hiccup when it's talked to again.
	timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

void wozfdc_device::a3_update_drive_sel()
{
	floppy_image_device *newflop = nullptr;

	if (!external_io_select)
	{
		newflop = floppy0->get_device();
	}
	else
	{
		switch (drvsel & 3)
		{
			case 0:
				newflop = floppy0->get_device();
				break;

			case 1:
				newflop = floppy1->get_device();
				break;

			case 2:
				newflop = floppy2->get_device();
				break;

			case 3:
				newflop = floppy3->get_device();
				break;
		}
	}

	if (floppy != newflop)
	{
		if(active) {
			lss_sync();
			floppy->mon_w(true);
		}
		floppy = newflop;
		if(active)
			floppy->mon_w(false);
	}
}

void diskii_fdc::device_reset()
{
	wozfdc_device::device_reset();
	external_drive_select = false;

	if (floppy0 != nullptr)
	{
		floppy = floppy0->get_device();
	}
}

void appleiii_fdc::device_reset()
{
	wozfdc_device::device_reset();
	external_drive_select = true;
	drvsel = 0;
	enable1 = 1;
}

void wozfdc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if(active)
		lss_sync();

	if(id == 1 && active == MODE_DELAY) {
		if(floppy)
			floppy->mon_w(true);
		active = MODE_IDLE;
	}
}

/*-------------------------------------------------
    read - called to read the FDC's registers
-------------------------------------------------*/

READ8_MEMBER(wozfdc_device::read)
{
	lss_sync();
	control(offset);

	if(!(offset & 1)) {
		return data_reg;
	}
	return 0xff;
}


/*-------------------------------------------------
    write - called to write the FDC's registers
-------------------------------------------------*/

WRITE8_MEMBER(wozfdc_device::write)
{
	lss_sync();
	control(offset);
	last_6502_write = data;
}

void wozfdc_device::phase(int ph, bool on)
{
	if(on)
		phases |= 1 << ph;
	else
		phases &= ~(1 << ph);

	if(floppy && active)
		floppy->seek_phase_w(phases);
}

void wozfdc_device::control(int offset)
{
	if(offset < 8)
		phase(offset >> 1, offset & 1);

	else
		switch(offset) {
		case 0x8:
			if(active == MODE_ACTIVE) {
				delay_timer->adjust(attotime::from_seconds(1));
				active = MODE_DELAY;
			}
			break;
		case 0x9:
			switch(active) {
			case MODE_IDLE:
				if(floppy)
					floppy->mon_w(false);
				active = MODE_ACTIVE;
				if(floppy)
					lss_start();
				break;
			case MODE_DELAY:
				active = MODE_ACTIVE;
				delay_timer->adjust(attotime::never);
				break;
			}
			break;
		case 0xa:
				external_io_select = false;
				if(floppy != floppy0->get_device()) {
					if(active)
						floppy->mon_w(true);
				floppy = floppy0->get_device();
				if(active)
					floppy->mon_w(false);
			}
			break;
		case 0xb:
			external_io_select = true;
			if (!external_drive_select)
			{
				if (floppy != floppy1->get_device())
				{
					if(active)
						floppy->mon_w(true);
					floppy = floppy1->get_device();
					if(active)
						floppy->mon_w(false);
				}
			}
			else
			{
				a3_update_drive_sel();
			}
			break;
		case 0xc:
			if(mode_load) {
				if(active)
					address &= ~0x04;
				mode_load = false;
			}
			break;
		case 0xd:
			if(!mode_load) {
				if(active)
					address |= 0x04;
				mode_load = true;
			}
			break;
		case 0xe:
			if(mode_write) {
				if(active)
					address &= ~0x08;
				mode_write = false;
				attotime now = machine().time();
				if(floppy)
					floppy->write_flux(write_start_time, now, write_position, write_buffer);
			}
			break;
		case 0xf:
			if(!mode_write) {
				if(active) {
					address |= 0x08;
					write_start_time = machine().time();
					write_position = 0;
					if(floppy)
						floppy->set_write_splice(write_start_time);
				}
				mode_write = true;
			}
			break;
		}
}

UINT64 wozfdc_device::time_to_cycles(const attotime &tm)
{
	// Clock is falling edges of the ~2Mhz clock
	// The 1021800 must be the controlling 6502's speed

	UINT64 cycles = tm.as_ticks(clock()*2);
	cycles = (cycles+1) >> 1;
	return cycles;
}

attotime wozfdc_device::cycles_to_time(UINT64 cycles)
{
	return attotime::from_ticks(cycles*2+1, clock()*2);
}

void wozfdc_device::lss_start()
{
	cycles = time_to_cycles(machine().time());
	data_reg = 0x00;
	address &= ~0x0e;
	write_position = 0;
	write_start_time = mode_write ? machine().time() : attotime::never;
	write_line_active = false;
	if(mode_write && floppy)
		floppy->set_write_splice(write_start_time);
}

void wozfdc_device::lss_sync()
{
	if(!active)
		return;

	attotime next_flux = floppy ? floppy->get_next_transition(cycles_to_time(cycles-1)) : attotime::never;

	UINT64 cycles_limit = time_to_cycles(machine().time());
	UINT64 cycles_next_flux = next_flux != attotime::never ? time_to_cycles(next_flux) : UINT64(-1);
	UINT64 cycles_next_flux_down = cycles_next_flux != UINT64(-1) ? cycles_next_flux+1 : UINT64(-1);

	if(cycles >= cycles_next_flux && cycles < cycles_next_flux_down)
		address &= ~0x10;
	else
		address |= 0x10;

	while(cycles < cycles_limit) {
		UINT64 cycles_next_trans = cycles_limit;
		if(cycles_next_trans > cycles_next_flux && cycles < cycles_next_flux)
			cycles_next_trans = cycles_next_flux;
		if(cycles_next_trans > cycles_next_flux_down && cycles < cycles_next_flux_down)
			cycles_next_trans = cycles_next_flux_down;

		while(cycles < cycles_next_trans) {
			UINT8 opcode = m_rom_p6[address];

			if(mode_write) {
				if((write_line_active && !(address & 0x80)) ||
					(!write_line_active && (address & 0x80))) {
					write_line_active = !write_line_active;
					assert(write_position != 32);
					write_buffer[write_position++] = cycles_to_time(cycles);
				} else if(write_position >= 30) {
					attotime now = cycles_to_time(cycles);
					if(floppy)
						floppy->write_flux(write_start_time, now, write_position, write_buffer);
					write_start_time = now;
					write_position = 0;
				}
			}

			address = (address & 0x1e) | (opcode & 0xc0) | ((opcode & 0x20) >> 5) | ((opcode & 0x10) << 1);
			switch(opcode & 0x0f) {
			case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
				data_reg = 0x00;
				break;
			case 0x8: case 0xc:
				break;
			case 0x9:
				data_reg <<= 1;
				break;
			case 0xa: case 0xe:
				data_reg = (data_reg >> 1) | (floppy && floppy->wpt_r() ? 0x80 : 0x00);
				break;
			case 0xb: case 0xf:
				data_reg = last_6502_write;
				break;
			case 0xd:
				data_reg = (data_reg << 1) | 0x01;
				break;
			}
			if(data_reg & 0x80)
				address |= 0x02;
			else
				address &= ~0x02;
			cycles++;
		}

		if(cycles == cycles_next_flux)
			address &= ~0x10;
		else if(cycles == cycles_next_flux_down) {
			address |= 0x10;
			next_flux = floppy ? floppy->get_next_transition(cycles_to_time(cycles)) : attotime::never;
			cycles_next_flux = next_flux != attotime::never ? time_to_cycles(next_flux) : UINT64(-1);
			cycles_next_flux_down = cycles_next_flux != UINT64(-1) ? cycles_next_flux+1 : UINT64(-1);
		}
	}
}

// set the two images for the Disk II
void diskii_fdc::set_floppies(floppy_connector *f0, floppy_connector *f1)
{
	floppy0 = f0;
	floppy1 = f1;

	if (floppy0)
	{
		floppy = floppy0->get_device();
	}
}

void appleiii_fdc::set_floppies_4(floppy_connector *f0, floppy_connector *f1, floppy_connector *f2, floppy_connector *f3)
{
	floppy0 = f0;
	floppy1 = f1;
	floppy2 = f2;
	floppy3 = f3;

	if (floppy0)
	{
		floppy = floppy0->get_device();
	}
}

READ8_MEMBER(appleiii_fdc::read_c0dx)
{
	control_dx(offset);

	return 0xff;
}

WRITE8_MEMBER(appleiii_fdc::write_c0dx)
{
	control_dx(offset);
}

void appleiii_fdc::control_dx(int offset)
{
	switch (offset)
	{
		case 0: // clear drive select bit 0
			drvsel &= ~1;
			break;

		case 1: // set drive select bit 0
			drvsel |= 1;
			break;

		case 2: // clear drive select bit 1
			drvsel &= ~2;
			break;

		case 3: // set drive select bit 1
			drvsel |= 2;
			break;

		case 4: // clear enable 1
			enable1 = 0;
			break;

		case 5: // set enable 1
			enable1 = 1;
			break;

		case 6: // clear side 2
		case 7: // set side 2
			break;

		default:    // cod8-c0df are not FDC related
			break;
	}

	if (offset < 8)
	{
		a3_update_drive_sel();
	}
}
