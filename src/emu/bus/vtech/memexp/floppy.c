/***************************************************************************

	VTech Laser/VZ Floppy Controller Cartridge

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

	Laser DD 20
    Dick Smith Electronics X-7304

    TODO: Broken currently, fix & modernize

***************************************************************************/

#include "floppy.h"
#include "formats/vtech1_dsk.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define VERBOSE	1

#define PHI0(n) (((n) >> 0) & 1)
#define PHI1(n) (((n) >> 1) & 1)
#define PHI2(n) (((n) >> 2) & 1)
#define PHI3(n) (((n) >> 3) & 1)


//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

static void laser_load_proc(device_image_interface &image);


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type FLOPPY_CONTROLLER = &device_creator<floppy_controller_device>;

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

static const floppy_interface laser_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(vtech1_only),
	NULL
};

static MACHINE_CONFIG_FRAGMENT( floppy_controller )
	MCFG_MEMEXP_SLOT_ADD("mem")
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(laser_floppy_interface)
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
	m_floppy0(*this, "floppy0"),
	m_floppy1(*this, "floppy1")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void floppy_controller_device::device_start()
{
	m_drive = -1;
	m_fdc_track_x2[0] = 80;
	m_fdc_track_x2[1] = 80;
	m_fdc_wrprot[0] = 0x80;
	m_fdc_wrprot[1] = 0x80;
	m_fdc_status = 0;
	m_fdc_edge = 0;
	m_fdc_bits = 8;
	m_fdc_start = 0;
	m_fdc_write = 0;
	m_fdc_offs = 0;
	m_fdc_latch = 0;

	m_floppy0->floppy_install_load_proc(laser_load_proc);
	m_floppy1->floppy_install_load_proc(laser_load_proc);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void floppy_controller_device::device_reset()
{
	m_memexp->set_io_space(m_slot->m_io);
	m_memexp->set_program_space(m_slot->m_program);

	m_slot->m_program->install_rom(0x4000, 0x5fff, memregion("software")->base());

	m_slot->m_io->install_read_handler(0x10, 0x1f, read8_delegate(FUNC(floppy_controller_device::floppy_r), this));
	m_slot->m_io->install_write_handler(0x10, 0x1f, write8_delegate(FUNC(floppy_controller_device::floppy_w), this));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

int floppy_controller_device::get_floppy_id(device_image_interface *image)
{
	if (image == dynamic_cast<device_image_interface *>(m_floppy0.target()))
		return 0;
	if (image == dynamic_cast<device_image_interface *>(m_floppy1.target()))
		return 1;

	return -1;
}

device_image_interface *floppy_controller_device::get_floppy_device(int drive)
{
	device_image_interface *image = NULL;

	switch (drive)
	{
	case 0:
		image = dynamic_cast<device_image_interface *>(m_floppy0.target());
		break;

	case 1:
		image = dynamic_cast<device_image_interface *>(m_floppy1.target());
		break;
	}

	return image;
}

static void laser_load_proc(device_image_interface &image)
{
	floppy_controller_device *fdc = dynamic_cast<floppy_controller_device *>(image.device().owner());

	int id = fdc->get_floppy_id(&image);

	if (!image.is_readonly())
		fdc->m_fdc_wrprot[id] = 0x00;
	else
		fdc->m_fdc_wrprot[id] = 0x80;
}

void floppy_controller_device::get_track()
{
	device_image_interface *image = get_floppy_device(m_drive);

	/* drive selected or and image file ok? */
	if (m_drive >= 0 && image->exists())
	{
		int size, offs;
		size = TRKSIZE_VZ;
		offs = TRKSIZE_VZ * m_fdc_track_x2[m_drive]/2;
		image->fseek(offs, SEEK_SET);
			// some disks have slightly larger header, make sure we capture the checksum at the end of the track
		size = image->fread(m_fdc_data, size+4);
		if (VERBOSE)
			logerror("get track @$%05x $%04x bytes\n", offs, size);
	}
	m_fdc_offs = 0;
	m_fdc_write = 0;
}

void floppy_controller_device::put_track()
{
	/* drive selected and image file ok? */
	if (m_drive >= 0 && floppy_get_device(machine(),m_drive) != NULL)
	{
		int size, offs;
		device_image_interface *image = get_floppy_device(m_drive);
		offs = TRKSIZE_VZ * m_fdc_track_x2[m_drive]/2;
		image->fseek(offs + m_fdc_start, SEEK_SET);
		size = image->fwrite(&m_fdc_data[m_fdc_start], m_fdc_write);
		if (VERBOSE)
			logerror("put track @$%05X+$%X $%04X/$%04X bytes\n", offs, m_fdc_start, size, m_fdc_write);
	}
}

READ8_MEMBER( floppy_controller_device::floppy_r )
{
	int data = 0xff;

	switch (offset)
	{
	case 1: /* data (read-only) */
		if (m_fdc_bits > 0)
		{
			if( m_fdc_status & 0x80 )
				m_fdc_bits--;
			data = (m_data >> m_fdc_bits) & 0xff;
			if (VERBOSE) {
				logerror("vtech1_fdc_r bits %d%d%d%d%d%d%d%d\n",
					(data>>7)&1,(data>>6)&1,(data>>5)&1,(data>>4)&1,
					(data>>3)&1,(data>>2)&1,(data>>1)&1,(data>>0)&1 );
			}
		}
		if (m_fdc_bits == 0)
		{
			m_data = m_fdc_data[m_fdc_offs];
			if (VERBOSE)
				logerror("vtech1_fdc_r %d : data ($%04X) $%02X\n", offset, m_fdc_offs, m_data);
			if(m_fdc_status & 0x80)
			{
				m_fdc_bits = 8;
				m_fdc_offs = (m_fdc_offs + 1) % TRKSIZE_FM;
			}
			m_fdc_status &= ~0x80;
		}
		break;
	case 2: /* polling (read-only) */
		/* fake */
		if (m_drive >= 0)
			m_fdc_status |= 0x80;
		data = m_fdc_status;
		break;
	case 3: /* write protect status (read-only) */
		if (m_drive >= 0)
			data = m_fdc_wrprot[m_drive];
		if (VERBOSE)
			logerror("vtech1_fdc_r %d : write_protect $%02X\n", offset, data);
		break;
	}
	return data;
}

WRITE8_MEMBER( floppy_controller_device::floppy_w )
{
	int drive;

	switch (offset)
	{
	case 0: /* latch (write-only) */
		drive = (data & 0x10) ? 0 : (data & 0x80) ? 1 : -1;
		if (drive != m_drive)
		{
			m_drive = drive;
			if (m_drive >= 0)
				get_track();
		}
		if (m_drive >= 0)
		{
			if ((PHI0(data) && !(PHI1(data) || PHI2(data) || PHI3(data)) && PHI1(m_fdc_latch)) ||
				(PHI1(data) && !(PHI0(data) || PHI2(data) || PHI3(data)) && PHI2(m_fdc_latch)) ||
				(PHI2(data) && !(PHI0(data) || PHI1(data) || PHI3(data)) && PHI3(m_fdc_latch)) ||
				(PHI3(data) && !(PHI0(data) || PHI1(data) || PHI2(data)) && PHI0(m_fdc_latch)))
			{
				if (m_fdc_track_x2[m_drive] > 0)
					m_fdc_track_x2[m_drive]--;
				if (VERBOSE)
					logerror("vtech1_fdc_w(%d) $%02X drive %d: stepout track #%2d.%d\n", offset, data, m_drive, m_fdc_track_x2[m_drive]/2,5*(m_fdc_track_x2[m_drive]&1));
				if ((m_fdc_track_x2[m_drive] & 1) == 0)
					get_track();
			}
			else
			if ((PHI0(data) && !(PHI1(data) || PHI2(data) || PHI3(data)) && PHI3(m_fdc_latch)) ||
				(PHI1(data) && !(PHI0(data) || PHI2(data) || PHI3(data)) && PHI0(m_fdc_latch)) ||
				(PHI2(data) && !(PHI0(data) || PHI1(data) || PHI3(data)) && PHI1(m_fdc_latch)) ||
				(PHI3(data) && !(PHI0(data) || PHI1(data) || PHI2(data)) && PHI2(m_fdc_latch)))
			{
				if (m_fdc_track_x2[m_drive] < 2*40)
					m_fdc_track_x2[m_drive]++;
				if (VERBOSE)
					logerror("vtech1_fdc_w(%d) $%02X drive %d: stepin track #%2d.%d\n", offset, data, m_drive, m_fdc_track_x2[m_drive]/2,5*(m_fdc_track_x2[m_drive]&1));
				if ((m_fdc_track_x2[m_drive] & 1) == 0)
					get_track();
			}
			if ((data & 0x40) == 0)
			{
				m_data <<= 1;
				if ((m_fdc_latch ^ data) & 0x20)
					m_data |= 1;
				if ((m_fdc_edge ^= 1) == 0)
				{
					m_fdc_bits--;

					if (m_fdc_bits == 0)
					{
						UINT8 value = 0;
						m_data &= 0xffff;
						if (m_data & 0x4000 ) value |= 0x80;
						if (m_data & 0x1000 ) value |= 0x40;
						if (m_data & 0x0400 ) value |= 0x20;
						if (m_data & 0x0100 ) value |= 0x10;
						if (m_data & 0x0040 ) value |= 0x08;
						if (m_data & 0x0010 ) value |= 0x04;
						if (m_data & 0x0004 ) value |= 0x02;
						if (m_data & 0x0001 ) value |= 0x01;
						if (VERBOSE)
							logerror("vtech1_fdc_w(%d) data($%04X) $%02X <- $%02X ($%04X)\n", offset, m_fdc_offs, m_fdc_data[m_fdc_offs], value, m_data);
						m_fdc_data[m_fdc_offs] = value;
						m_fdc_offs = (m_fdc_offs + 1) % TRKSIZE_FM;
						m_fdc_write++;
						m_fdc_bits = 8;
					}
				}
			}
			/* change of write signal? */
			if ((m_fdc_latch ^ data) & 0x40)
			{
				/* falling edge? */
				if (m_fdc_latch & 0x40)
				{
					m_fdc_start = m_fdc_offs;
					m_fdc_edge = 0;
				}
				else
				{
					/* data written to track before? */
					if (m_fdc_write)
						put_track();
				}
				m_fdc_bits = 8;
				m_fdc_write = 0;
			}
		}
		m_fdc_latch = data;
		break;
	}
}
