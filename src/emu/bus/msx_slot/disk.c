/*
From: erbo@xs4all.nl (erik de boer)

sony and philips have used (almost) the same design
and this is the memory layout
but it is not a msx standard !

WD1793 or wd2793 registers

address

7FF8H read  status register
      write command register
7FF9H  r/w  track register (r/o on NMS 8245 and Sony)
7FFAH  r/w  sector register (r/o on NMS 8245 and Sony)
7FFBH  r/w  data register


hardware registers

address

7FFCH r/w  bit 0 side select
7FFDH r/w  b7>M-on , b6>in-use , b1>ds1 , b0>ds0  (all neg. logic)
7FFEH         not used
7FFFH read b7>drq , b6>intrq

set on 7FFDH bit 2 always to 0 (some use it as disk change reset)

*/

#include "emu.h"
#include "disk.h"


const device_type MSX_SLOT_DISK1 = &device_creator<msx_slot_disk1_device>;
const device_type MSX_SLOT_DISK2 = &device_creator<msx_slot_disk2_device>;


msx_slot_disk_device::msx_slot_disk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: msx_slot_rom_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_fdc(NULL)
	, m_floppy0(NULL)
	, m_floppy1(NULL)
	, m_floppy(NULL)
	, m_fdc_tag(NULL)
	, m_floppy0_tag(NULL)
	, m_floppy1_tag(NULL)
{
}


void msx_slot_disk_device::device_start()
{
	msx_slot_rom_device::device_start();

	if (m_fdc_tag == NULL)
	{
		fatalerror("msx_slot_disk_device: no FDC tag specified\n");
	}

	m_fdc = owner()->subdevice<wd_fdc_analog_t>(m_fdc_tag);
	m_floppy0 = owner()->subdevice<floppy_connector>(m_floppy0_tag);
	m_floppy1 = owner()->subdevice<floppy_connector>(m_floppy1_tag);

	if (m_fdc == NULL)
	{
		fatalerror("msx_slot_disk_device: Unable to find FDC with tag '%s'\n", m_fdc_tag);
	}

	if (m_floppy0 == NULL && m_floppy1 == NULL)
	{
		logerror("msx_slot_disk_device: Warning: both floppy0 and floppy1 were not found\n");
	}
}


msx_slot_disk1_device::msx_slot_disk1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: msx_slot_disk_device(mconfig, MSX_SLOT_DISK1, "MSX Internal floppy type 1", tag, owner, clock, "msx_slot_disk1", __FILE__)
	, m_side_control(0)
	, m_control(0)
{
}


void msx_slot_disk1_device::device_start()
{
	msx_slot_disk_device::device_start();

	save_item(NAME(m_side_control));
	save_item(NAME(m_control));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_slot_disk1_device::post_load), this));
}


void msx_slot_disk1_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk1_device::post_load()
{
	UINT8 data = m_control;

	// To make sure the FDD busy led status gets set correctly
	m_control ^= 0x40;

	set_control(data);
}


void msx_slot_disk1_device::set_side_control(UINT8 data)
{
	m_side_control = data;

	if (m_floppy)
	{
		m_floppy->ss_w(m_side_control & 0x01);
	}
}


void msx_slot_disk1_device::set_control(UINT8 data)
{
	UINT8 old_m_control = m_control;

	m_control = data;

	switch (m_control & 0x03)
	{
		case 0:
		case 2:
			m_floppy = m_floppy0 ? m_floppy0->get_device() : NULL;
			break;

		case 1:
			m_floppy = m_floppy1 ? m_floppy1->get_device() : NULL;
			break;

		default:
			m_floppy = NULL;
			break;
	}

	if (m_floppy)
	{
		m_floppy->mon_w((m_control & 0x80) ? 0 : 1);
		m_floppy->ss_w(m_side_control & 0x01);
	}

	m_fdc->set_floppy(m_floppy);

	if ((old_m_control ^ m_control) & 0x40)
	{
		set_led_status(machine(), 0, !(m_control & 0x40));
	}
}


READ8_MEMBER(msx_slot_disk1_device::read)
{
	switch (offset)
	{
		case 0x7ff8:
		case 0xbff8:
			return m_fdc->status_r();

		case 0x7ff9:
		case 0xbff9:
			return m_fdc->track_r();

		case 0x7ffa:
		case 0xbffa:
			return m_fdc->sector_r();

		case 0x7ffb:
		case 0xbffb:
			return m_fdc->data_r();

		case 0x7ffc:
		case 0xbffc:
			return 0xfe | (m_side_control & 0x01);

		case 0x7ffd:
		case 0xbffd:
			return ( m_control & 0x83 ) | 0x78;

		case 0x7fff:
		case 0xbfff:
			return 0x3f | (m_fdc->intrq_r() ? 0 : 0x40) | (m_fdc->drq_r() ? 0 : 0x80);
	}

	return msx_slot_rom_device::read(space, offset);
}


WRITE8_MEMBER(msx_slot_disk1_device::write)
{
	switch (offset)
	{
		case 0x7ff8:
		case 0xbff8:
			m_fdc->cmd_w(data);
			break;

		case 0x7ff9:
		case 0xbff9:
			m_fdc->track_w(data);
			break;

		case 0x7ffa:
		case 0xbffa:
			m_fdc->sector_w(data);
			break;

		case 0x7ffb:
		case 0xbffb:
			m_fdc->data_w(data);
			break;

		case 0x7ffc:
		case 0xbffc:
			set_side_control(data);
			break;

		case 0x7ffd:
		case 0xbffd:
			set_control(data);
			break;
	}
}


msx_slot_disk2_device::msx_slot_disk2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: msx_slot_disk_device(mconfig, MSX_SLOT_DISK2, "MSX Internal floppy type 2", tag, owner, clock, "msx_slot_disk2", __FILE__)
	, m_control(0)
{
}


void msx_slot_disk2_device::device_start()
{
	msx_slot_disk_device::device_start();

	save_item(NAME(m_control));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_slot_disk2_device::post_load), this));
}


void msx_slot_disk2_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk2_device::post_load()
{
	UINT8 data = m_control;

	// To make sure the FDD busy led status gets set correctly
	m_control ^= 0x40;

	set_control(data);
}


void msx_slot_disk2_device::set_control(UINT8 data)
{
	UINT8 old_m_control = m_control;

	m_control = data;

	switch (m_control & 3)
	{
		case 1:
			m_floppy = m_floppy0 ? m_floppy0->get_device() : NULL;
			break;

		case 2:
			m_floppy = m_floppy1 ? m_floppy1->get_device() : NULL;
			break;

		default:
			m_floppy = NULL;
			break;
	}

	if (m_floppy)
	{
		m_floppy->mon_w((m_control & 0x08) ? 0 : 1);
		m_floppy->ss_w((m_control & 0x04) ? 1 : 0);
	}

	m_fdc->set_floppy(m_floppy);

	if ((old_m_control ^ m_control) & 0x40)
	{
		set_led_status(machine(), 0, !(m_control & 0x40));
	}
}


READ8_MEMBER(msx_slot_disk2_device::read)
{
	switch (offset)
	{
		case 0x7fb8:
		case 0xbfb8:
			return m_fdc->status_r();

		case 0x7fb9:
		case 0xbfb9:
			return m_fdc->track_r();

		case 0x7fba:
		case 0xbfba:
			return m_fdc->sector_r();

		case 0x7fbb:
		case 0xbfbb:
			return m_fdc->data_r();

		case 0x7fbc:
		case 0xbfbc:
			return 0x3f | (m_fdc->drq_r() ? 0 : 0x40) | (m_fdc->intrq_r() ? 0x80 : 0);
	}

	return msx_slot_rom_device::read(space, offset);
}


WRITE8_MEMBER(msx_slot_disk2_device::write)
{
	switch (offset)
	{
		case 0x7fb8:
		case 0xbfb8:
			m_fdc->cmd_w(data);
			break;

		case 0x7fb9:
		case 0xbfb9:
			m_fdc->track_w(data);
			break;

		case 0x7fba:
		case 0xbfba:
			m_fdc->sector_w(data);
			break;

		case 0x7fbb:
		case 0xbfbb:
			m_fdc->data_w(data);
			break;

		case 0x7fbc:
		case 0xbfbc:
			set_control(data);
			break;

		default:
			printf("Unmapped write writing %02x to %04x\n", data, offset);
			break;
	}
}

