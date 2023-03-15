// license:GPL-2.0+
// copyright-holders:Dirk Best, Olivier Galibert
/***************************************************************************

    VTech Laser/VZ Floppy Controller Cartridge

    Laser DD 20
    Dick Smith Electronics X-7304

***************************************************************************/

#include "emu.h"
#include "floppy.h"
#include "formats/vt_dsk.h"
#include "formats/fs_vtech.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_FLOPPY_CONTROLLER, vtech_floppy_controller_device, "vtech_fdc", "Laser/VZ Floppy Disk Controller")

//-------------------------------------------------
//  mem_map - memory space address map
//-------------------------------------------------

void vtech_floppy_controller_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x4000, 0x5fff).rom().region("software", 0);
}

//-------------------------------------------------
//  io_map - io space address map
//-------------------------------------------------

void vtech_floppy_controller_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x10, 0x10).w(FUNC(vtech_floppy_controller_device::latch_w));
	map(0x11, 0x11).r(FUNC(vtech_floppy_controller_device::shifter_r));
	map(0x12, 0x12).r(FUNC(vtech_floppy_controller_device::rd_r));
	map(0x13, 0x13).r(FUNC(vtech_floppy_controller_device::wpt_r));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( floppy )
	ROM_REGION(0x2000, "software", 0)
	ROM_LOAD("vzdos.rom", 0x0000, 0x2000, CRC(b6ed6084) SHA1(59d1cbcfa6c5e1906a32704fbf0d9670f0d1fd8b))
ROM_END

const tiny_rom_entry *vtech_floppy_controller_device::device_rom_region() const
{
	return ROM_NAME( floppy );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

static void laser_floppies(device_slot_interface &device)
{
	device.option_add("525", FLOPPY_525_VTECH);
}

void vtech_floppy_controller_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_VTECH_BIN_FORMAT);
	fr.add(FLOPPY_VTECH_DSK_FORMAT);
	fr.add(fs::VTECH);
}

void vtech_floppy_controller_device::device_add_mconfig(machine_config &config)
{
	vtech_memexp_device::device_add_mconfig(config);

	VTECH_MEMEXP_SLOT(config, m_memexp);
	m_memexp->set_memspace(m_mem, AS_PROGRAM);
	m_memexp->set_iospace(m_io, AS_PROGRAM);

	FLOPPY_CONNECTOR(config, m_floppy0, laser_floppies, "525", floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, laser_floppies, "525", floppy_formats);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_floppy_controller_device - constructor
//-------------------------------------------------

vtech_floppy_controller_device::vtech_floppy_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_memexp_device(mconfig, VTECH_FLOPPY_CONTROLLER, tag, owner, clock),
	m_memexp(*this, "mem"),
	m_floppy0(*this, "0"),
	m_floppy1(*this, "1"),
	m_floppy(nullptr), m_latch(0), m_shifter(0), m_latching_inverter(false), m_current_cyl(0), m_write_position(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_floppy_controller_device::device_start()
{
	vtech_memexp_device::device_start();

	// register for save states
	save_item(NAME(m_latch));
	save_item(NAME(m_shifter));
	save_item(NAME(m_latching_inverter));
	save_item(NAME(m_current_cyl));
	save_item(NAME(m_last_latching_inverter_update_time));
	save_item(NAME(m_write_start_time));
	save_item(NAME(m_write_position));

	// TODO: save m_write_buffer and rebuild m_floppy after load

	// Obvious bugs... must have worked by sheer luck and very subtle
	// timings.  Our current z80 is not subtle enough.
	uint8_t *rom = memregion("software")->base();
	rom[0x1678] = 0x75;
	rom[0x1688] = 0x85;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vtech_floppy_controller_device::device_reset()
{
	m_latch = 0x00;
	m_floppy = nullptr;
	m_current_cyl = 0;
	m_shifter = 0x00;
	m_latching_inverter = false;
	m_last_latching_inverter_update_time = machine().time();
	m_write_start_time = attotime::never;
	m_write_position = 0;
	memset(m_write_buffer, 0, sizeof(m_write_buffer));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

// latch at +0 is linked to:
//  bits 0-3: track step motor phases
//  bit  5:   write data (flux reversal on every level change)
//  bit  6:   !write request
//  bits 4,7: floppy select

void vtech_floppy_controller_device::latch_w(uint8_t data)
{
	uint8_t diff = m_latch ^ data;
	m_latch = data;

	floppy_image_device *newflop = nullptr;
	if(m_latch & 0x10)
		newflop = m_floppy0->get_device();
	else if(m_latch & 0x80)
		newflop = m_floppy1->get_device();

	if(newflop != m_floppy) {
		update_latching_inverter();
		flush_writes();
		if(m_floppy) {
			m_floppy->mon_w(1);
			m_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());
		}
		if(newflop) {
			newflop->mon_w(0);
			newflop->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&vtech_floppy_controller_device::index_callback, this));
			m_current_cyl = newflop->get_cyl() << 1;
		}
		m_floppy = newflop;
	}

	if(m_floppy) {
		int cph = m_current_cyl & 3;
		int pcyl = m_current_cyl;
		if(!(m_latch & (1 << cph))) {
			if(m_current_cyl < 84*2 && (m_latch & (1 << ((cph+1) & 3))))
				m_current_cyl++;
			if(m_current_cyl && (m_latch & (1 << ((cph+3) & 3))))
				m_current_cyl--;
			if(m_current_cyl != pcyl && !(m_current_cyl & 1)) {
				m_floppy->dir_w(m_current_cyl < pcyl);
				m_floppy->stp_w(true);
				m_floppy->stp_w(false);
				m_floppy->stp_w(true);
			}
		}
	}

	if(diff & 0x40) {
		if(!(m_latch & 0x40)) {
			m_write_start_time = machine().time();
			m_write_position = 0;
			if(m_floppy)
				m_floppy->set_write_splice(m_write_start_time);

		} else {
			update_latching_inverter();
			flush_writes();
			m_write_start_time = attotime::never;
		}
	}
	if(!(m_latch & 0x40) && (diff & 0x20)) {
		if(m_write_position == std::size(m_write_buffer)) {
			update_latching_inverter();
			flush_writes(true);
		}
		m_write_buffer[m_write_position++] = machine().time();
	}
}


// The read data line is connected to a flip/flop with inverted input
// connected to the input.  That means it inverts its value on every
// floppy flux reversal.  We'll call it a latching inverter.
//
// The latching inverter is connected to a 8-bits shift register.  On
// reading the shifter address we get:
// - the inverted inverter output is shifted through the lsb of the shift register
// - the inverter is cleared

uint8_t vtech_floppy_controller_device::shifter_r()
{
	if (!machine().side_effects_disabled())
	{
		update_latching_inverter();
		m_shifter = (m_shifter << 1) | !m_latching_inverter;
		m_latching_inverter = false;
	}
	return m_shifter;
}


// Linked to the latching inverter on bit 7, rest is floating
uint8_t vtech_floppy_controller_device::rd_r()
{
	update_latching_inverter();
	return m_latching_inverter ? 0x80 : 0x00;
}


// Linked to wp signal on bit 7, rest is floating
uint8_t vtech_floppy_controller_device::wpt_r()
{
	return m_floppy && m_floppy->wpt_r() ? 0x80 : 0x00;
}

void vtech_floppy_controller_device::update_latching_inverter()
{
	attotime now = machine().time();
	if(!m_floppy) {
		m_last_latching_inverter_update_time = now;
		return;
	}

	attotime when = m_last_latching_inverter_update_time;
	for(;;) {
		when = m_floppy->get_next_transition(when);
		if(when == attotime::never || when > now)
			break;
		m_latching_inverter = !m_latching_inverter;
	}
	m_last_latching_inverter_update_time = now;
}

void vtech_floppy_controller_device::index_callback(floppy_image_device *floppy, int state)
{
	update_latching_inverter();
	flush_writes(true);
}

void vtech_floppy_controller_device::flush_writes(bool keep_margin)
{
	if(!m_floppy || m_write_start_time == attotime::never)
		return;

	// Beware of time travel.  Index pulse callback (which flushes)
	// can be called with a machine().time() inferior to the last
	// m_write_buffer value if the calling cpu instructions are not
	// suspendable.

	attotime limit = machine().time();
	int kept_pos = m_write_position;
	int kept_count = 0;
	while(kept_pos > 0 && m_write_buffer[kept_pos-1] >= limit) {
		kept_pos--;
		kept_count++;
	}

	if(keep_margin) {
		attotime last = kept_pos ? m_write_buffer[kept_pos-1] : m_write_start_time;
		attotime delta = limit-last;
		delta = delta / 2;
		limit = limit - delta;
	}
	m_write_position -= kept_count;
	if(m_write_position && m_write_buffer[0] == m_write_start_time) {
		if(m_write_position)
			memmove(m_write_buffer, m_write_buffer+1, sizeof(m_write_buffer[0])*(m_write_position-1));
		m_write_position--;
	}
	m_floppy->write_flux(m_write_start_time, limit, m_write_position, m_write_buffer);
	m_write_start_time = limit;

	if(kept_count != 0)
		memmove(m_write_buffer, m_write_buffer+kept_pos, kept_count*sizeof(m_write_buffer[0]));
	m_write_position = kept_count;
}
