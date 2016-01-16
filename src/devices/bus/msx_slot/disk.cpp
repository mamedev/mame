// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
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
const device_type MSX_SLOT_DISK3 = &device_creator<msx_slot_disk3_device>;
const device_type MSX_SLOT_DISK4 = &device_creator<msx_slot_disk4_device>;
const device_type MSX_SLOT_DISK5 = &device_creator<msx_slot_disk5_device>;
const device_type MSX_SLOT_DISK6 = &device_creator<msx_slot_disk6_device>;


msx_slot_disk_device::msx_slot_disk_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: msx_slot_rom_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_floppy0(nullptr)
	, m_floppy1(nullptr)
	, m_floppy2(nullptr)
	, m_floppy3(nullptr)
	, m_floppy(nullptr)
{
}


void msx_slot_disk_device::device_start()
{
	msx_slot_rom_device::device_start();

	if (m_fdc_tag.empty())
	{
		fatalerror("msx_slot_disk_device: no FDC tag specified\n");
	}

	m_floppy0 = !m_floppy0_tag.empty() ? owner()->subdevice<floppy_connector>(m_floppy0_tag) : nullptr;
	m_floppy1 = !m_floppy1_tag.empty() ? owner()->subdevice<floppy_connector>(m_floppy1_tag) : nullptr;
	m_floppy2 = !m_floppy2_tag.empty() ? owner()->subdevice<floppy_connector>(m_floppy2_tag) : nullptr;
	m_floppy3 = !m_floppy3_tag.empty() ? owner()->subdevice<floppy_connector>(m_floppy3_tag) : nullptr;

	if (m_floppy0 == nullptr && m_floppy1 == nullptr)
	{
		logerror("msx_slot_disk_device: Warning: both floppy0 and floppy1 were not found\n");
	}
}


msx_slot_wd_disk_device::msx_slot_wd_disk_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: msx_slot_disk_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_fdc(nullptr)
{
}


void msx_slot_wd_disk_device::device_start()
{
	msx_slot_disk_device::device_start();

	m_fdc = owner()->subdevice<wd_fdc_analog_t>(m_fdc_tag);

	if (m_fdc == nullptr)
	{
		fatalerror("msx_slot_wd_disk_device: Unable to find FDC with tag '%s'\n", m_fdc_tag.c_str());
	}
}


msx_slot_tc8566_disk_device::msx_slot_tc8566_disk_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: msx_slot_disk_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_fdc(nullptr)
{
}


void msx_slot_tc8566_disk_device::device_start()
{
	msx_slot_disk_device::device_start();

	m_fdc = owner()->subdevice<tc8566af_device>(m_fdc_tag);

	if (m_fdc == nullptr)
	{
		fatalerror("msx_slot_tc8566_disk_device: Unable to find FDC with tag '%s'\n", m_fdc_tag.c_str());
	}
}



msx_slot_disk1_device::msx_slot_disk1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK1, "MSX Internal floppy type 1", tag, owner, clock, "msx_slot_disk1", __FILE__)
	, m_side_control(0)
	, m_control(0)
{
}


void msx_slot_disk1_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

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
			m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
			break;

		case 1:
			m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
			break;

		default:
			m_floppy = nullptr;
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
		machine().output().set_led_value(0, !(m_control & 0x40));
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

		default:
			logerror("msx_slot_disk1_device::write: Unmapped write writing %02x to %04x\n", data, offset);
			break;

	}
}


msx_slot_disk2_device::msx_slot_disk2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK2, "MSX Internal floppy type 2", tag, owner, clock, "msx_slot_disk2", __FILE__)
	, m_control(0)
{
}


void msx_slot_disk2_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

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
			m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
			break;

		case 2:
			m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
			break;

		default:
			m_floppy = nullptr;
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
		machine().output().set_led_value(0, !(m_control & 0x40));
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
			logerror("msx_slot_disk2_device::write: Unmapped write writing %02x to %04x\n", data, offset);
			break;
	}
}






msx_slot_disk3_device::msx_slot_disk3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: msx_slot_tc8566_disk_device(mconfig, MSX_SLOT_DISK3, "MSX Internal floppy type 3", tag, owner, clock, "msx_slot_disk3", __FILE__)
{
}


WRITE8_MEMBER(msx_slot_disk3_device::write)
{
	switch (offset)
	{
		case 0x7ff8:   // CR0 : 0 - 0 - MEN1 - MEN0 - 0 - -FRST - 0 - DSA
			m_fdc->dor_w(space, 2, data);
			break;

		case 0x7ff9:   // CR1 : 0 - 0 - C4E - C4 - SBME - SBM - TCE - FDCTC
			m_fdc->cr1_w(space, 3, data);
			break;

		case 0x7ffb:   // Data Register
			m_fdc->fifo_w(space, 5, data);
			break;

		default:
			logerror("msx_slot_disk3_device::write: Unmapped write writing %02x to %04x\n", data, offset);
			break;
	}
}


READ8_MEMBER(msx_slot_disk3_device::read)
{
	switch (offset)
	{
		case 0x7ffa:   // Status Register
			return m_fdc->msr_r(space, 4);
		case 0x7ffb:   // Data Register
			return m_fdc->fifo_r(space, 5);
	}

	return msx_slot_rom_device::read(space, offset);
}





msx_slot_disk4_device::msx_slot_disk4_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: msx_slot_tc8566_disk_device(mconfig, MSX_SLOT_DISK4, "MSX Internal floppy type 4", tag, owner, clock, "msx_slot_disk4", __FILE__)
{
}


WRITE8_MEMBER(msx_slot_disk4_device::write)
{
	switch (offset)
	{
		case 0x7ff1:   // FDD : x - x - MC1 - MC0 - x - x - x - x
			break;

		case 0x7ff2:   // CR0 : 0 - 0 - MEN1 - MEN0 - 0 - -FRST - 0 - DSA
			m_fdc->dor_w(space, 2, data);
			break;

		case 0x7ff3:   // CR1 : 0 - 0 - C4E - C4 - SBME - SBM - TCE - FDCTC
			m_fdc->cr1_w(space, 3, data);
			break;

		case 0x7ff5:   // Data Register
			m_fdc->fifo_w(space, 5, data);
			break;

		default:
			logerror("msx_slot_disk4_device::write: Unmapped write writing %02x to %04x\n", data, offset);
			break;
	}
}


READ8_MEMBER(msx_slot_disk4_device::read)
{
	switch (offset)
	{
		case 0x7ff1:   // FDD : x - x - MC1 - MC0 - x - x - x - x
			logerror("msx_slot_disk4_device::write: Unmapped read from Media Change register\n");
			break;

		case 0x7ff4:   // Status Register
			return m_fdc->msr_r(space, 4);
		case 0x7ff5:   // Data Register
			return m_fdc->fifo_r(space, 5);
	}

	return msx_slot_rom_device::read(space, offset);
}




msx_slot_disk5_device::msx_slot_disk5_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK5, "MSX Internal floppy type 5", tag, owner, clock, "msx_slot_disk5", __FILE__)
	, m_control(0)
{
}


void msx_slot_disk5_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_control));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_slot_disk5_device::post_load), this));

	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0xd0, 0xd4, write8_delegate(FUNC(msx_slot_disk5_device::io_write), this));
	space.install_read_handler(0xd0, 0xd4, read8_delegate(FUNC(msx_slot_disk5_device::io_read), this));
}


void msx_slot_disk5_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk5_device::post_load()
{
	set_control(m_control);
}


void msx_slot_disk5_device::set_control(UINT8 control)
{
	m_control = control;

	switch (m_control & 0x0f)
	{
		case 0x01:
			m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
			break;

		case 0x02:
			m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
			break;

		case 0x04:
			m_floppy = m_floppy2 ? m_floppy2->get_device() : nullptr;
			break;

		case 0x08:
			m_floppy = m_floppy3 ? m_floppy3->get_device() : nullptr;
			break;

		default:
			m_floppy = nullptr;
			break;
	}

	if (m_floppy)
	{
		m_floppy->mon_w((m_control & 0x20) ? 0 : 1);
		m_floppy->ss_w((m_control & 0x10) ? 1 : 0);
	}

	m_fdc->set_floppy(m_floppy);
}


READ8_MEMBER(msx_slot_disk5_device::io_read)
{
	switch (offset)
	{
		case 0x00:
			return m_fdc->status_r();

		case 0x01:
			return m_fdc->track_r();

		case 0x02:
			return m_fdc->sector_r();

		case 0x03:
			return m_fdc->data_r();

		case 0x04:
			return 0x3f | (m_fdc->drq_r() ? 0 : 0x40) | (m_fdc->intrq_r() ? 0x80 : 0);
	}

	return 0xff;
}


WRITE8_MEMBER(msx_slot_disk5_device::io_write)
{
	switch (offset)
	{
		case 0x00:
			m_fdc->cmd_w(data);
			break;

		case 0x01:
			m_fdc->track_w(data);
			break;

		case 0x02:
			m_fdc->sector_w(data);
			break;

		case 0x03:
			m_fdc->data_w(data);
			break;

		case 0x04:
			set_control(data);
			break;
	}
}



msx_slot_disk6_device::msx_slot_disk6_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK6, "MSX Internal floppy type 6", tag, owner, clock, "msx_slot_disk6", __FILE__)
	, m_side_motor(0)
	, m_drive_select0(0)
	, m_drive_select1(0)
{
}


void msx_slot_disk6_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_side_motor));
	save_item(NAME(m_drive_select0));
	save_item(NAME(m_drive_select1));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_slot_disk6_device::post_load), this));
}


void msx_slot_disk6_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk6_device::post_load()
{
	select_drive();
}


void msx_slot_disk6_device::select_drive()
{
	if (m_drive_select1)
	{
		m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
		if (!m_floppy)
		{
			m_drive_select1 = 0;
		}
	}

	if (m_drive_select0)
	{
		m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
		if (!m_floppy)
		{
			m_drive_select0 = 0;
		}
	}

	m_fdc->set_floppy(m_floppy);

	set_side_motor();
}


void msx_slot_disk6_device::set_side_motor()
{
	if (m_floppy)
	{
		m_floppy->mon_w((m_side_motor & 0x02) ? 0 : 1);
		m_floppy->ss_w(m_side_motor & 0x01);
	}
}


READ8_MEMBER(msx_slot_disk6_device::read)
{
	switch (offset)
	{
		case 0x7ff0:   // status?
		case 0x7ff8:
			return m_fdc->status_r();

		case 0x7ff1:   // track?
		case 0x7ff9:
			return m_fdc->track_r();

		case 0x7ff2:   // sector?
		case 0x7ffa:
			return m_fdc->sector_r();

		case 0x7ff3:   // data?
		case 0x7ffb:
			return m_fdc->data_r();

		case 0x7ff4:
		case 0x7ffc:
			// bit 0 = side control
			// bit 1 = motor control
			return 0xfc | m_side_motor;
			break;

		// This reads back a 1 in bit 0 if drive0 is present and selected
		case 0x7ff5:
		case 0x7ffd:
			return 0xfe | m_drive_select0;

		// This reads back a 1 in bit 0 if drive1 is present and selected
		case 0x7ff6:
		case 0x7ffe:
			return 0xfe | m_drive_select1;

		case 0x7ff7:
		case 0x7fff:
			return 0x3f | (m_fdc->intrq_r() ? 0 : 0x40) | (m_fdc->drq_r() ? 0 : 0x80);
	}

	return msx_slot_rom_device::read(space, offset);
}


WRITE8_MEMBER(msx_slot_disk6_device::write)
{
	switch (offset)
	{
		case 0x7ff0:   // cmd?
		case 0x7ff8:
			m_fdc->cmd_w(data);
			break;

		case 0x7ff1:   // track?
		case 0x7ff9:
			m_fdc->track_w(data);
			break;

		case 0x7ff2:   // sector?
		case 0x7ffa:
			m_fdc->sector_w(data);
			break;

		case 0x7ff3:   // data?
		case 0x7ffb:
			m_fdc->data_w(data);
			break;

		// Side and motort control
		// bit 0 = side select
		// bit 1 = motor on/off
		case 0x7ff4:
		case 0x7ffc:
			m_side_motor = data;
			set_side_motor();
			break;

		// bit 0 - select drive 0
		case 0x7ff5:
		case 0x7ffd:
			m_drive_select0 = data;
			select_drive();
			break;

		// bit 1 - select drive 1
		case 0x7ff6:
		case 0x7ffe:
			m_drive_select1 = data;
			select_drive();
			break;

		default:
			logerror("msx_slot_disk6_device::write: Unmapped write writing %02x to %04x\n", data, offset);
			break;
	}
}
