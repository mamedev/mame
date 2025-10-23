// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    wozfdc.c

    Implementation of the Apple Disk II floppy disk controller

*********************************************************************/

#include "emu.h"
#include "wozfdc.h"

#include "imagedev/floppy.h"
#include "formats/ap2_dsk.h"
#include "formats/as_dsk.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DISKII_FDC,   diskii_fdc_device,   "d2fdc", "Apple Disk II floppy controller")
DEFINE_DEVICE_TYPE(APPLEIII_FDC, appleiii_fdc_device, "a3fdc", "Apple III floppy controller")

#define DISKII_P6_REGION  "diskii_rom_p6"

ROM_START( diskiing )
	ROM_REGION(0x100, DISKII_P6_REGION, 0)
	ROM_LOAD( "341-0028-a.rom", 0x0000, 0x0100, CRC(b72a2c70) SHA1(bc39fbd5b9a8d2287ac5d0a42e639fc4d3c2f9d4)) /* 341-0028: 16-sector disk drive (older version), PROM P6 */
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *wozfdc_device::device_rom_region() const
{
	return ROM_NAME( diskiing );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void wozfdc_device::device_add_mconfig(machine_config &config)
{
	F9334(config, m_phaselatch); // 9334 on circuit diagram but 74LS259 in parts list; actual chip may vary
	m_phaselatch->parallel_out_cb().set(FUNC(wozfdc_device::set_phase));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

wozfdc_device::wozfdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
		m_phaselatch(*this, "phaselatch")
{
}

diskii_fdc_device::diskii_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	wozfdc_device(mconfig, DISKII_FDC, tag, owner, clock)
{
}

appleiii_fdc_device::appleiii_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	wozfdc_device(mconfig, APPLEIII_FDC, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wozfdc_device::device_start()
{
	m_rom_p6 = machine().root_device().memregion(this->subtag(DISKII_P6_REGION).c_str())->base();

	timer = timer_alloc(FUNC(wozfdc_device::generic_tick), this);
	delay_timer = timer_alloc(FUNC(wozfdc_device::delayed_tick), this);

	save_item(NAME(last_6502_write));
	save_item(NAME(mode_write));
	save_item(NAME(mode_load));
	save_item(NAME(active));
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

void diskii_fdc_device::device_reset()
{
	wozfdc_device::device_reset();
	external_drive_select = false;

	if (floppy0 != nullptr)
	{
		floppy = floppy0->get_device();
	}
}

void appleiii_fdc_device::device_reset()
{
	wozfdc_device::device_reset();
	external_drive_select = true;
	drvsel = 0;
	enable1 = 1;
}

TIMER_CALLBACK_MEMBER(wozfdc_device::generic_tick)
{
	if(active)
		lss_sync();
}

TIMER_CALLBACK_MEMBER(wozfdc_device::delayed_tick)
{
	if(active)
		lss_sync();

	if(active == MODE_DELAY) {
		if(floppy)
			floppy->mon_w(true);
		active = MODE_IDLE;
	}
}

/*-------------------------------------------------
    read - called to read the FDC's registers
-------------------------------------------------*/

uint8_t wozfdc_device::read(offs_t offset)
{
	lss_sync();
	control(offset);

	if(!(offset & 1)) {
		// The FDC runs faster than the CPU, so it has time to run
		// for one cycle before the CPU can observe the data register.
		lss_sync(1);
		return data_reg;
	}
	return 0xff;
}


/*-------------------------------------------------
    write - called to write the FDC's registers
-------------------------------------------------*/

void wozfdc_device::write(offs_t offset, uint8_t data)
{
	lss_sync();
	control(offset);
	last_6502_write = data;
}

void wozfdc_device::set_phase(uint8_t data)
{
	if (floppy && active)
		floppy->seek_phase_w(data);
}

void wozfdc_device::control(int offset)
{
	if(offset < 8)
		m_phaselatch->write_bit(offset >> 1, offset & 1);

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

uint64_t wozfdc_device::time_to_cycles(const attotime &tm)
{
	// Clock is falling edges of the ~2Mhz clock
	// The 1021800 must be the controlling 6502's speed

	uint64_t cycles = tm.as_ticks(clock()*2);
	cycles = (cycles+1) >> 1;
	return cycles;
}

attotime wozfdc_device::cycles_to_time(uint64_t cycles)
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

void wozfdc_device::lss_sync(uint64_t extra_cycles)
{
	if(!active)
		return;

	attotime next_flux = floppy ? floppy->get_next_transition(cycles_to_time(cycles-1)) : attotime::never;

	uint64_t cycles_next_flux = next_flux != attotime::never ? time_to_cycles(next_flux) : uint64_t(-1);
	uint64_t cycles_next_flux_down = cycles_next_flux != uint64_t(-1) ? cycles_next_flux+1 : uint64_t(-1);

	if(cycles >= cycles_next_flux && cycles < cycles_next_flux_down)
		address &= ~0x10;
	else
		address |= 0x10;

	uint64_t cycles_limit = time_to_cycles(machine().time()) + extra_cycles;
	assert(cycles <= cycles_limit); // make sure we aren't going back in time

	while(cycles < cycles_limit) {
		uint64_t cycles_next_trans = cycles_limit;
		if(cycles_next_trans > cycles_next_flux && cycles < cycles_next_flux)
			cycles_next_trans = cycles_next_flux;
		if(cycles_next_trans > cycles_next_flux_down && cycles < cycles_next_flux_down)
			cycles_next_trans = cycles_next_flux_down;

		while(cycles < cycles_next_trans) {
			uint8_t opcode = m_rom_p6[address];

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
			cycles_next_flux = next_flux != attotime::never ? time_to_cycles(next_flux) : uint64_t(-1);
			cycles_next_flux_down = cycles_next_flux != uint64_t(-1) ? cycles_next_flux+1 : uint64_t(-1);
		}
	}
}

// set the two images for the Disk II
void diskii_fdc_device::set_floppies(floppy_connector *f0, floppy_connector *f1)
{
	floppy0 = f0;
	floppy1 = f1;

	if (floppy0)
	{
		floppy = floppy0->get_device();
	}
}

void appleiii_fdc_device::set_floppies_4(floppy_connector *f0, floppy_connector *f1, floppy_connector *f2, floppy_connector *f3)
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

uint8_t appleiii_fdc_device::read_c0dx(uint8_t offset)
{
	control_dx(offset);

	return 0xff;
}

void appleiii_fdc_device::write_c0dx(uint8_t offset, uint8_t data)
{
	control_dx(offset);
}

void appleiii_fdc_device::control_dx(int offset)
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
