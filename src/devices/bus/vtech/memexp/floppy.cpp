// license:GPL-2.0+
// copyright-holders:Dirk Best, Olivier Galibert
/***************************************************************************

    VTech Laser/VZ Floppy Controller Cartridge

    Laser DD 20
    Dick Smith Electronics X-7304

***************************************************************************/

#include "floppy.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type FLOPPY_CONTROLLER = &device_creator<floppy_controller_device>;

DEVICE_ADDRESS_MAP_START(map, 8, floppy_controller_device)
	AM_RANGE(0, 0) AM_WRITE(latch_w)
	AM_RANGE(1, 1) AM_READ(shifter_r)
	AM_RANGE(2, 2) AM_READ(rd_r)
	AM_RANGE(3, 3) AM_READ(wpt_r)
ADDRESS_MAP_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( floppy )
	ROM_REGION(0x3000, "software", 0)
	ROM_LOAD("vzdos.rom", 0x0000, 0x2000, CRC(b6ed6084) SHA1(59d1cbcfa6c5e1906a32704fbf0d9670f0d1fd8b))
ROM_END

const rom_entry *floppy_controller_device::device_rom_region() const
{
	return ROM_NAME( floppy );
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static SLOT_INTERFACE_START( laser_floppies )
	SLOT_INTERFACE("525", FLOPPY_525_SSSD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( floppy_controller )
	MCFG_MEMEXP_SLOT_ADD("mem")
	MCFG_FLOPPY_DRIVE_ADD("0", laser_floppies, "525", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("1", laser_floppies, "525", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

machine_config_constructor floppy_controller_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( floppy_controller );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  floppy_controller_device - constructor
//-------------------------------------------------

floppy_controller_device::floppy_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, FLOPPY_CONTROLLER, "Laser/VZ Floppy Disk Controller", tag, owner, clock, "laserfdc", __FILE__),
	device_memexp_interface(mconfig, *this),
	m_memexp(*this, "mem"),
	m_floppy0(*this, "0"),
	m_floppy1(*this, "1"), m_floppy(nullptr), m_latch(0), m_shifter(0), m_latching_inverter(false), m_current_cyl(0), m_write_position(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void floppy_controller_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_shifter));
	save_item(NAME(m_latching_inverter));
	save_item(NAME(m_current_cyl));
	save_item(NAME(m_last_latching_inverter_update_time));
	save_item(NAME(m_write_start_time));
	save_item(NAME(m_write_position));

	// TODO: save m_write_buffer and rebuild m_floppy after load

	UINT8 *bios = memregion("software")->base();

	// Obvious bugs... must have worked by sheer luck and very subtle
	// timings.  Our current z80 is not subtle enough.

	bios[0x1678] = 0x75;
	bios[0x1688] = 0x85;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void floppy_controller_device::device_reset()
{
	m_memexp->set_io_space(m_slot->m_io);
	m_memexp->set_program_space(m_slot->m_program);

	m_slot->m_program->install_rom(0x4000, 0x5fff, memregion("software")->base());

	m_slot->m_io->install_device(0x10, 0x1f, *this, &floppy_controller_device::map);

	m_latch = 0x00;
	m_floppy = NULL;
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

WRITE8_MEMBER(floppy_controller_device::latch_w)
{
	UINT8 diff = m_latch ^ data;
	m_latch = data;

	floppy_image_device *newflop = NULL;
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
			newflop->set_rpm(85);
			newflop->mon_w(0);
			newflop->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(FUNC(floppy_controller_device::index_callback), this));
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
		if(m_write_position == ARRAY_LENGTH(m_write_buffer)) {
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

READ8_MEMBER(floppy_controller_device::shifter_r)
{
	update_latching_inverter();
	m_shifter = (m_shifter << 1) | !m_latching_inverter;
	m_latching_inverter = false;
	return m_shifter;
}


// Linked to the latching inverter on bit 7, rest is floating
READ8_MEMBER(floppy_controller_device::rd_r)
{
	update_latching_inverter();
	return m_latching_inverter ? 0x80 : 0x00;
}


// Linked to wp signal on bit 7, rest is floating
READ8_MEMBER(floppy_controller_device::wpt_r)
{
	return m_floppy && m_floppy->wpt_r() ? 0x80 : 0x00;
}

void floppy_controller_device::update_latching_inverter()
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

void floppy_controller_device::index_callback(floppy_image_device *floppy, int state)
{
	update_latching_inverter();
	flush_writes(true);
}

void floppy_controller_device::flush_writes(bool keep_margin)
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
