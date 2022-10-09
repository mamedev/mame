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


DEFINE_DEVICE_TYPE(MSX_SLOT_DISK1, msx_slot_disk1_device, "msx_slot_disk1", "MSX Internal floppy type 1")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK2, msx_slot_disk2_device, "msx_slot_disk2", "MSX Internal floppy type 2")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK3, msx_slot_disk3_device, "msx_slot_disk3", "MSX Internal floppy type 3")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK4, msx_slot_disk4_device, "msx_slot_disk4", "MSX Internal floppy type 4")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK5, msx_slot_disk5_device, "msx_slot_disk5", "MSX Internal floppy type 5")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK6, msx_slot_disk6_device, "msx_slot_disk6", "MSX Internal floppy type 6")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK7, msx_slot_disk7_device, "msx_slot_disk7", "MSX Internal floppy type 7")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK8, msx_slot_disk8_device, "msx_slot_disk8", "MSX Internal floppy type 8")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK9, msx_slot_disk9_device, "msx_slot_disk9", "MSX Internal floppy type 9")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK10, msx_slot_disk10_device, "msx_slot_disk10", "MSX Internal floppy type 10")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK11, msx_slot_disk11_device, "msx_slot_disk11", "MSX Internal floppy type 11")


msx_slot_disk_device::msx_slot_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_rom_device(mconfig, type, tag, owner, clock)
	, m_floppy0(nullptr)
	, m_floppy1(nullptr)
	, m_floppy2(nullptr)
	, m_floppy3(nullptr)
	, m_floppy(nullptr)
	, m_fdc_tag(nullptr)
	, m_floppy0_tag(nullptr)
	, m_floppy1_tag(nullptr)
	, m_floppy2_tag(nullptr)
	, m_floppy3_tag(nullptr)
{
}


void msx_slot_disk_device::device_start()
{
	msx_slot_rom_device::device_start();

	if (m_fdc_tag == nullptr)
	{
		fatalerror("msx_slot_disk_device: no FDC tag specified\n");
	}

	m_floppy0 = m_floppy0_tag ? owner()->subdevice<floppy_connector>(m_floppy0_tag) : nullptr;
	m_floppy1 = m_floppy1_tag ? owner()->subdevice<floppy_connector>(m_floppy1_tag) : nullptr;
	m_floppy2 = m_floppy2_tag ? owner()->subdevice<floppy_connector>(m_floppy2_tag) : nullptr;
	m_floppy3 = m_floppy3_tag ? owner()->subdevice<floppy_connector>(m_floppy3_tag) : nullptr;

	if (m_floppy0 == nullptr && m_floppy1 == nullptr)
	{
		logerror("msx_slot_disk_device: Warning: both floppy0 and floppy1 were not found\n");
	}
}


msx_slot_wd_disk_device::msx_slot_wd_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_disk_device(mconfig, type, tag, owner, clock)
	, m_fdc(nullptr)
	, m_led(*this, "led0")
{
}


void msx_slot_wd_disk_device::device_start()
{
	msx_slot_disk_device::device_start();

	m_led.resolve();
	m_fdc = owner()->subdevice<wd_fdc_analog_device_base>(m_fdc_tag);

	if (m_fdc == nullptr)
	{
		fatalerror("msx_slot_wd_disk_device: Unable to find FDC with tag '%s'\n", m_fdc_tag);
	}
}


msx_slot_tc8566_disk_device::msx_slot_tc8566_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_disk_device(mconfig, type, tag, owner, clock)
	, m_fdc(nullptr)
{
}


void msx_slot_tc8566_disk_device::device_start()
{
	msx_slot_disk_device::device_start();

	m_fdc = owner()->subdevice<tc8566af_device>(m_fdc_tag);

	if (m_fdc == nullptr)
	{
		fatalerror("msx_slot_tc8566_disk_device: Unable to find FDC with tag '%s'\n", m_fdc_tag);
	}
}



msx_slot_disk1_device::msx_slot_disk1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK1, tag, owner, clock)
	, m_side_control(0)
	, m_control(0)
{
}


void msx_slot_disk1_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_side_control));
	save_item(NAME(m_control));

	page(1)->install_rom(0x4000, 0x7fff, m_rom);
	page(1)->install_read_handler(0x7ff8, 0x7ff8, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(1)->install_read_handler(0x7ff9, 0x7ff9, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(1)->install_read_handler(0x7ffa, 0x7ffa, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(1)->install_read_handler(0x7ffb, 0x7ffb, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(1)->install_read_handler(0x7ffc, 0x7ffc, read8smo_delegate(*this, FUNC(msx_slot_disk1_device::side_control_r)));
	page(1)->install_read_handler(0x7ffd, 0x7ffd, read8smo_delegate(*this, FUNC(msx_slot_disk1_device::control_r)));
	page(1)->install_read_handler(0x7fff, 0x7fff, read8smo_delegate(*this, FUNC(msx_slot_disk1_device::status_r)));
	page(1)->install_write_handler(0x7ff8, 0x7ff8, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(1)->install_write_handler(0x7ff9, 0x7ff9, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(1)->install_write_handler(0x7ffa, 0x7ffa, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(1)->install_write_handler(0x7ffb, 0x7ffb, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(1)->install_write_handler(0x7ffc, 0x7ffc, write8smo_delegate(*this, FUNC(msx_slot_disk1_device::set_side_control)));
	page(1)->install_write_handler(0x7ffd, 0x7ffd, write8smo_delegate(*this, FUNC(msx_slot_disk1_device::set_control)));

	page(2)->install_read_handler(0xbff8, 0xbff8, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(2)->install_read_handler(0xbff9, 0xbff9, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(2)->install_read_handler(0xbffa, 0xbffa, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(2)->install_read_handler(0xbffb, 0xbffb, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(2)->install_read_handler(0xbffc, 0xbffc, read8smo_delegate(*this, FUNC(msx_slot_disk1_device::side_control_r)));
	page(2)->install_read_handler(0xbffd, 0xbffd, read8smo_delegate(*this, FUNC(msx_slot_disk1_device::control_r)));
	page(2)->install_read_handler(0xbfff, 0xbfff, read8smo_delegate(*this, FUNC(msx_slot_disk1_device::status_r)));
	page(2)->install_write_handler(0xbff8, 0xbff8, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(2)->install_write_handler(0xbff9, 0xbff9, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(2)->install_write_handler(0xbffa, 0xbffa, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(2)->install_write_handler(0xbffb, 0xbffb, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(2)->install_write_handler(0xbffc, 0xbffc, write8smo_delegate(*this, FUNC(msx_slot_disk1_device::set_side_control)));
	page(2)->install_write_handler(0xbffd, 0xbffd, write8smo_delegate(*this, FUNC(msx_slot_disk1_device::set_control)));
}


void msx_slot_disk1_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk1_device::device_post_load()
{
	uint8_t data = m_control;

	// To make sure the FDD busy led status gets set correctly
	m_control ^= 0x40;

	set_control(data);
}


void msx_slot_disk1_device::set_side_control(uint8_t data)
{
	m_side_control = data;

	if (m_floppy)
	{
		m_floppy->ss_w(BIT(m_side_control, 0));
	}
}


void msx_slot_disk1_device::set_control(uint8_t data)
{
	// 7------- motor on (0 = on)
	// -6------ in-use / LED (0 = on)
	// --5432-- unused
	// ------1- drive 1 select (0 = selected)
	// -------0 drive 0 select (0 = selected)

	uint8_t old_m_control = m_control;

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
		m_floppy->mon_w(BIT(m_control, 7) ? 0 : 1);
		m_floppy->ss_w(BIT(m_side_control, 0));
	}

	m_fdc->set_floppy(m_floppy);

	if ((old_m_control ^ m_control) & 0x40)
	{
		m_led =  BIT(~m_control, 6);
	}
}

uint8_t msx_slot_disk1_device::side_control_r()
{
	return 0xfe | (m_side_control & 0x01);

}

uint8_t msx_slot_disk1_device::control_r()
{
	return ( m_control & 0x83 ) | 0x78;
}

uint8_t msx_slot_disk1_device::status_r()
{
	return 0x3f | (m_fdc->intrq_r() ? 0 : 0x40) | (m_fdc->drq_r() ? 0 : 0x80);
}

uint8_t msx_slot_disk1_device::read(offs_t offset)
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

	return msx_slot_rom_device::read(offset);
}


void msx_slot_disk1_device::write(offs_t offset, uint8_t data)
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


msx_slot_disk2_device::msx_slot_disk2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK2, tag, owner, clock)
	, m_control(0)
{
}


void msx_slot_disk2_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_control));

	page(1)->install_rom(0x4000, 0x7fff, m_rom);
	page(1)->install_read_handler(0x7fb8, 0x7fb8, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(1)->install_read_handler(0x7fb9, 0x7fb9, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(1)->install_read_handler(0x7fba, 0x7fba, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(1)->install_read_handler(0x7fbb, 0x7fbb, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(1)->install_read_handler(0x7fbc, 0x7fbc, read8smo_delegate(*this, FUNC(msx_slot_disk2_device::status_r)));
	page(1)->install_write_handler(0x7fb8, 0x7fb8, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(1)->install_write_handler(0x7fb9, 0x7fb9, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(1)->install_write_handler(0x7fba, 0x7fba, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(1)->install_write_handler(0x7fbb, 0x7fbb, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(1)->install_write_handler(0x7fbc, 0x7fbc, write8smo_delegate(*this, FUNC(msx_slot_disk2_device::set_control)));

	page(2)->install_read_handler(0xbfb8, 0xbfb8, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(2)->install_read_handler(0xbfb9, 0xbfb9, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(2)->install_read_handler(0xbfba, 0xbfba, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(2)->install_read_handler(0xbfbb, 0xbfbb, read8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(2)->install_read_handler(0xbfbc, 0xbfbc, read8smo_delegate(*this, FUNC(msx_slot_disk2_device::status_r)));
	page(2)->install_write_handler(0xbfb8, 0xbfb8, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(2)->install_write_handler(0xbfb9, 0xbfb9, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(2)->install_write_handler(0xbfba, 0xbfba, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(2)->install_write_handler(0xbfbb, 0xbfbb, write8smo_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(2)->install_write_handler(0xbfbc, 0xbfbc, write8smo_delegate(*this, FUNC(msx_slot_disk2_device::set_control)));
}


void msx_slot_disk2_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk2_device::device_post_load()
{
	uint8_t data = m_control;

	// To make sure the FDD busy led status gets set correctly
	m_control ^= 0x40;

	set_control(data);
}


void msx_slot_disk2_device::set_control(uint8_t data)
{
	uint8_t old_m_control = m_control;

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
		m_floppy->mon_w(BIT(m_control, 3) ? 0 : 1);
		m_floppy->ss_w(BIT(m_control, 2) ? 1 : 0);
	}

	m_fdc->set_floppy(m_floppy);

	if ((old_m_control ^ m_control) & 0x40)
	{
		m_led = BIT(~m_control, 6);
	}
}

uint8_t msx_slot_disk2_device::status_r()
{
	return 0x3f | (m_fdc->drq_r() ? 0 : 0x40) | (m_fdc->intrq_r() ? 0x80 : 0);
}

uint8_t msx_slot_disk2_device::read(offs_t offset)
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

	return msx_slot_rom_device::read(offset);
}


void msx_slot_disk2_device::write(offs_t offset, uint8_t data)
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


msx_slot_disk8_device::msx_slot_disk8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK8, tag, owner, clock)
	, m_control(0)
{
}


void msx_slot_disk8_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_control));
}


void msx_slot_disk8_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk8_device::device_post_load()
{
	uint8_t data = m_control;

	// To make sure the FDD busy led status gets set correctly
	m_control ^= 0x40;

	set_control(data);
}


void msx_slot_disk8_device::set_control(uint8_t data)
{
	uint8_t old_m_control = m_control;

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
		m_floppy->mon_w(BIT(m_control, 3) ? 0 : 1);
		m_floppy->ss_w(BIT(m_control, 2) ? 1 : 0);
	}

	m_fdc->set_floppy(m_floppy);

	if ((old_m_control ^ m_control) & 0x40)
	{
		m_led = BIT(~m_control, 6);
	}
}


uint8_t msx_slot_disk8_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0x7f80:
	case 0xbf80:
		return m_fdc->status_r();

	case 0x7f81:
	case 0xbf81:
		return m_fdc->track_r();

	case 0x7f82:
	case 0xbf82:
		return m_fdc->sector_r();

	case 0x7f83:
	case 0xbf83:
		return m_fdc->data_r();

	case 0x7f84:
	case 0xbf84:
		return 0x3f | (m_fdc->drq_r() ? 0 : 0x40) | (m_fdc->intrq_r() ? 0x80 : 0);
	}

	return msx_slot_rom_device::read(offset);
}


void msx_slot_disk8_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x7f80:
	case 0xbf80:
		m_fdc->cmd_w(data);
		break;

	case 0x7f81:
	case 0xbf81:
		m_fdc->track_w(data);
		break;

	case 0x7f82:
	case 0xbf82:
		m_fdc->sector_w(data);
		break;

	case 0x7f83:
	case 0xbf83:
		m_fdc->data_w(data);
		break;

	case 0x7f84:
	case 0xbf84:
		set_control(data);
		break;

	default:
		logerror("msx_slot_disk8_device::write: Unmapped write writing %02x to %04x\n", data, offset);
		break;
	}
}






msx_slot_disk3_device::msx_slot_disk3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_tc8566_disk_device(mconfig, MSX_SLOT_DISK3, tag, owner, clock)
{
}


void msx_slot_disk3_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x7ff8:   // CR0 : 0 - 0 - MEN1 - MEN0 - 0 - -FRST - 0 - DSA
	case 0xbff8:
		m_fdc->dor_w(data);
		break;

	case 0x7ff9:   // CR1 : 0 - 0 - C4E - C4 - SBME - SBM - TCE - FDCTC
	case 0xbff9:
		m_fdc->cr1_w(data);
		break;

	case 0x7ffb:   // Data Register
	case 0xbffb:
		m_fdc->fifo_w(data);
		break;

	default:
		logerror("msx_slot_disk3_device::write: Unmapped write writing %02x to %04x\n", data, offset);
		break;
	}
}


uint8_t msx_slot_disk3_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0x7ffa:   // Status Register
	case 0xbffa:
		return m_fdc->msr_r();
	case 0x7ffb:   // Data Register
	case 0xbffb:
		return m_fdc->fifo_r();
	}

	return msx_slot_rom_device::read(offset);
}





msx_slot_disk4_device::msx_slot_disk4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_tc8566_disk_device(mconfig, MSX_SLOT_DISK4, tag, owner, clock)
{
}


void msx_slot_disk4_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x7ff1:   // FDD : x - x - MC1 - MC0 - x - x - x - x
		break;

	case 0x7ff2:   // CR0 : 0 - 0 - MEN1 - MEN0 - 0 - -FRST - 0 - DSA
		m_fdc->dor_w(data);
		break;

	case 0x7ff3:   // CR1 : 0 - 0 - C4E - C4 - SBME - SBM - TCE - FDCTC
		m_fdc->cr1_w(data);
		break;

	case 0x7ff5:   // Data Register
		m_fdc->fifo_w(data);
		break;

	default:
		logerror("msx_slot_disk4_device::write: Unmapped write writing %02x to %04x\n", data, offset);
		break;
	}
}


uint8_t msx_slot_disk4_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0x7ff1:   // FDD : x - x - MC1 - MC0 - x - x - x - x
		logerror("msx_slot_disk4_device::write: Unmapped read from Media Change register\n");
		break;

	case 0x7ff4:   // Status Register
		return m_fdc->msr_r();
	case 0x7ff5:   // Data Register
		return m_fdc->fifo_r();
	}

	return msx_slot_rom_device::read(offset);
}




msx_slot_disk5_device::msx_slot_disk5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK5, tag, owner, clock)
	, m_control(0)
{
}


void msx_slot_disk5_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_control));

	// Install IO read/write handlers
	io_space().install_write_handler(0xd0, 0xd4, write8sm_delegate(*this, FUNC(msx_slot_disk5_device::io_write)));
	io_space().install_read_handler(0xd0, 0xd4, read8sm_delegate(*this, FUNC(msx_slot_disk5_device::io_read)));
}


void msx_slot_disk5_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk5_device::device_post_load()
{
	set_control(m_control);
}


void msx_slot_disk5_device::set_control(uint8_t control)
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
		m_floppy->mon_w(BIT(m_control, 5) ? 0 : 1);
		m_floppy->ss_w(BIT(m_control, 4) ? 1 : 0);
	}

	m_fdc->set_floppy(m_floppy);
}


uint8_t msx_slot_disk5_device::io_read(offs_t offset)
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


void msx_slot_disk5_device::io_write(offs_t offset, uint8_t data)
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



msx_slot_disk6_device::msx_slot_disk6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK6, tag, owner, clock)
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
}


void msx_slot_disk6_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk6_device::device_post_load()
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
		m_floppy->mon_w(BIT(m_side_motor, 1) ? 0 : 1);
		m_floppy->ss_w(BIT(m_side_motor, 0));
	}
}


uint8_t msx_slot_disk6_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0x7ff0:   // status
	case 0x7ff8:
		return m_fdc->status_r();

	case 0x7ff1:   // track
	case 0x7ff9:
		return m_fdc->track_r();

	case 0x7ff2:   // sector
	case 0x7ffa:
		return m_fdc->sector_r();

	case 0x7ff3:   // data
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

	return msx_slot_rom_device::read(offset);
}


void msx_slot_disk6_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x7ff0:   // cmd
	case 0x7ff8:
		m_fdc->cmd_w(data);
		break;

	case 0x7ff1:   // track
	case 0x7ff9:
		m_fdc->track_w(data);
		break;

	case 0x7ff2:   // sector
	case 0x7ffa:
		m_fdc->sector_w(data);
		break;

	case 0x7ff3:   // data
	case 0x7ffb:
		m_fdc->data_w(data);
		break;

	// Side and motor control
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


msx_slot_disk7_device::msx_slot_disk7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK7, tag, owner, clock)
	, m_drive_side_motor(0)
	, m_drive_select0(0)
	, m_drive_select1(0)
{
}


void msx_slot_disk7_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_drive_side_motor));
	save_item(NAME(m_drive_select0));
	save_item(NAME(m_drive_select1));
}


void msx_slot_disk7_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk7_device::device_post_load()
{
	select_drive();
}


void msx_slot_disk7_device::select_drive()
{
	m_drive_select0 = 0;
	m_drive_select1 = 0;
	m_floppy = nullptr;

	switch (m_drive_side_motor & 0x03)
	{
	case 0:
		m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
		break;
	case 1:
		m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
		break;
	}

	m_fdc->set_floppy(m_floppy);
}


void msx_slot_disk7_device::set_drive_side_motor()
{
	select_drive();

	if (m_floppy)
	{
		m_floppy->mon_w(BIT(m_drive_side_motor, 3) ? 0 : 1);
		m_floppy->ss_w(BIT(m_drive_side_motor, 2));
	}
}


uint8_t msx_slot_disk7_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0x7ff8:   // status
	case 0xbff8:
		return m_fdc->status_r();

	case 0x7ff9:   // track
	case 0xbff9:
		return m_fdc->track_r();

	case 0x7ffa:   // sector
	case 0xbffa:
		return m_fdc->sector_r();

	case 0x7ffb:   // data
	case 0xbffb:
		return m_fdc->data_r();

	case 0x7ffc:   // drive status
	case 0xbffc:
		return (m_drive_side_motor & 0x0f)
			| (m_floppy && m_floppy->dskchg_r() ? 0x00 : 0x10)
			| (m_floppy && m_floppy->ready_r() ? 0x00 : 0x20)
			| (m_fdc->intrq_r() ? 0x40 : 0x00)
			| (m_fdc->drq_r() ? 0x80 : 0x00)
		;
	}

	return msx_slot_rom_device::read(offset);
}


void msx_slot_disk7_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x7ff8:   // command
	case 0xbff8:
		m_fdc->cmd_w(data);
		break;

	case 0x7ff9:   // track
	case 0xbff9:
		m_fdc->track_w(data);
		break;

	case 0x7ffa:   // sector
	case 0xbffa:
		m_fdc->sector_w(data);
		break;

	case 0x7ffb:   // data
	case 0xbffb:
		m_fdc->data_w(data);
		break;

	// Drive, side and motor control
	// bit 0,1 = drive select
	// bit 2 = side select
	// bit 3 = motor on/off
	case 0x7ffc:
	case 0xbffc:
		m_drive_side_motor = data;
		set_drive_side_motor();
		break;

	default:
		logerror("msx_slot_disk7_device::write: Unmapped write writing %02x to %04x\n", data, offset);
		break;
	}
}




msx_slot_disk9_device::msx_slot_disk9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK9, tag, owner, clock)
	, m_control(0)
{
}


void msx_slot_disk9_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_control));
}


void msx_slot_disk9_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk9_device::device_post_load()
{
	uint8_t data = m_control;

	// To make sure the FDD busy led status gets set correctly
	m_control ^= 0x40;

	set_control(data);
}


void msx_slot_disk9_device::set_control(uint8_t data)
{
	uint8_t old_m_control = m_control;

	m_control = data;

	switch (m_control & 0x03)
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
		m_floppy->mon_w(BIT(m_control, 3) ? 0 : 1);
		m_floppy->ss_w(BIT(m_control, 2));
	}

	m_fdc->set_floppy(m_floppy);

	if ((old_m_control ^ m_control) & 0x40)
	{
		m_led =  BIT(m_control, 6);
	}
}


uint8_t msx_slot_disk9_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0x7ff8:
		return m_fdc->status_r();

	case 0x7ff9:
		return m_fdc->track_r();

	case 0x7ffa:
		return m_fdc->sector_r();

	case 0x7ffb:
		return m_fdc->data_r();

	case 0x7ffc:
		return 0x3f | (m_fdc->intrq_r() ? 0x80 : 0) | (m_fdc->drq_r() ? 0x40 : 0);
	}

	return msx_slot_rom_device::read(offset);
}


void msx_slot_disk9_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x7ff8:
		m_fdc->cmd_w(data);
		break;

	case 0x7ff9:
		m_fdc->track_w(data);
		break;

	case 0x7ffa:
		m_fdc->sector_w(data);
		break;

	case 0x7ffb:
		m_fdc->data_w(data);
		break;

	case 0x7ffc:
		set_control(data);
		break;

	default:
		logerror("msx_slot_disk9_device::write: Unmapped write writing %02x to %04x\n", data, offset);
		break;
	}
}




msx_slot_disk10_device::msx_slot_disk10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK10, tag, owner, clock)
	, m_control(0)
{
}


void msx_slot_disk10_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_control));
}


void msx_slot_disk10_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk10_device::device_post_load()
{
	uint8_t data = m_control;

	// To make sure the FDD busy led status gets set correctly
	m_control ^= 0x40;

	set_control(data);
}


void msx_slot_disk10_device::set_control(uint8_t data)
{
	uint8_t old_m_control = m_control;

	m_control = data;

	if (BIT(m_control, 2))
		m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
	else
		m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;

	if (m_floppy && !BIT(m_control, 2))
		m_floppy->mon_w(BIT(m_control, 0) ? 0 : 1);

	if (m_floppy && BIT(m_control, 2))
		m_floppy->mon_w(BIT(m_control, 1) ? 0 : 1);

	if (m_floppy)
		m_floppy->ss_w(BIT(m_control, 3));

	m_fdc->set_floppy(m_floppy);

	// TODO Verify
	if ((old_m_control ^ m_control) & 0x40)
	{
		m_led =  BIT(m_control, 6);
	}
}


uint8_t msx_slot_disk10_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0x7ff8:
		return m_fdc->status_r();

	case 0x7ff9:
		return m_fdc->track_r();

	case 0x7ffa:
		return m_fdc->sector_r();

	case 0x7ffb:
		return m_fdc->data_r();

	case 0x7ffc:
		return 0x23
			| (m_control & 0x04)
			| ((m_control & 0x08) ^ 0x08)
			| ((m_floppy && m_floppy->mon_r()) ? 0 : 0x10)
			| (m_fdc->drq_r() ? 0x40 : 0)
			| (m_fdc->intrq_r() ? 0x80 : 0);
		
//	case 0x7ffd: // TODO return system control/status
	}

	return msx_slot_rom_device::read(offset);
}


void msx_slot_disk10_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x7ff8:
		m_fdc->cmd_w(data);
		break;

	case 0x7ff9:
		m_fdc->track_w(data);
		break;

	case 0x7ffa:
		m_fdc->sector_w(data);
		break;

	case 0x7ffb:
		m_fdc->data_w(data);
		break;

	case 0x7ffc:
		set_control(data);
		break;

	default:
		logerror("msx_slot_disk10_device::write: Unmapped write writing %02x to %04x\n", data, offset);
		break;
	}
}



msx_slot_disk11_device::msx_slot_disk11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK11, tag, owner, clock)
	, m_side_control(0)
	, m_control(0)
{
}


void msx_slot_disk11_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_side_control));
	save_item(NAME(m_control));
}


void msx_slot_disk11_device::device_reset()
{
	m_fdc->dden_w(false);
}


void msx_slot_disk11_device::device_post_load()
{
	uint8_t data = m_control;

	// To make sure the FDD busy led status gets set correctly
	m_control ^= 0x40;

	set_control(data);
}


void msx_slot_disk11_device::set_side_control(uint8_t data)
{
	m_side_control = data;

	if (m_floppy)
		m_floppy->ss_w(BIT(m_side_control, 0));
}


void msx_slot_disk11_device::set_control(uint8_t data)
{
	uint8_t old_m_control = m_control;

	m_control = data;

	m_floppy = nullptr;
	switch (m_control & 0x03)
	{
	case 0:
		m_floppy = m_floppy0 ? m_floppy0->get_device() : nullptr;
		break;

	case 1:
		m_floppy = m_floppy1 ? m_floppy1->get_device() : nullptr;
		break;
	}

	if (m_floppy)
	{
		m_floppy->mon_w(BIT(m_control, 7) ? 0 : 1);
		m_floppy->ss_w(BIT(m_side_control, 0));
	}

	m_fdc->set_floppy(m_floppy);

	if ((old_m_control ^ m_control) & 0x40)
	{
		m_led =  BIT(~m_control, 6);
	}
}


uint8_t msx_slot_disk11_device::read(offs_t offset)
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
		return (m_control & 0x83) | 0x78;

	case 0x7fff:
	case 0xbfff:
		// bit 4 is checked during reading (ready check?)
		return 0x3f | (m_fdc->intrq_r() ? 0 : 0x40) | (m_fdc->drq_r() ? 0 : 0x80);
	}

	return msx_slot_rom_device::read(offset);
}


void msx_slot_disk11_device::write(offs_t offset, uint8_t data)
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
		logerror("msx_slot_disk11_device::write: Unmapped write writing %02x to %04x\n", data, offset);
		break;
	}
}
