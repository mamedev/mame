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
#include "formats/dmk_dsk.h"
#include "formats/msx_dsk.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_DISK1_FD1793,            msx_slot_disk1_fd1793_device,            "msx_slot_disk1_fd1793",            "MSX Internal floppy type 1 - FD1793 - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK1_MB8877,            msx_slot_disk1_mb8877_device,            "msx_slot_disk1_mb8877",            "MSX Internal floppy type 1 - MB8877 - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK1_WD2793_N,          msx_slot_disk1_wd2793_n_device,          "msx_slot_disk1_wd2793_n",          "MSX Internal floppy type 1 - WD2793 no force ready - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK1_WD2793_N_2_DRIVES, msx_slot_disk1_wd2793_n_2_drives_device, "msx_slot_disk1_wd2793_n_2_drives", "MSX Internal floppy type 1 - WD2793 no force ready - 2 DSDD Drives")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK1_WD2793,            msx_slot_disk1_wd2793_device,            "msx_slot_disk1_wd2793",            "MSX Internal floppy type 1 - WD2793 - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK1_WD2793_0,          msx_slot_disk1_wd2793_0_device,          "msx_slot_disk1_wd2793_0",          "MSX Internal floppy type 1 - WD2793 - No drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK1_WD2793_SS,         msx_slot_disk1_wd2793_ss_device,         "msx_slot_disk1_wd2793_ss",         "MSX Internal floppy type 1 - WD2793 - 1 SSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK1_WD2793_2_DRIVES,   msx_slot_disk1_wd2793_2_drives_device,   "msx_slot_disk1_wd2793_2_drives",   "MSX Internal floppy type 1 - WD2793 - 2 DSDD Drives")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK2_FD1793_SS,         msx_slot_disk2_fd1793_ss_device,         "msx_slot_disk2_fd1793_ss",         "MSX Internal floppy type 2 - FD1793 - 1 SSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK2_MB8877,            msx_slot_disk2_mb8877_device,            "msx_slot_disk2_mb8877",            "MSX Internal floppy type 2 - MB8877 - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK2_MB8877_SS,         msx_slot_disk2_mb8877_ss_device,         "msx_slot_disk2_mb8877_ss",         "MSX Internal floppy type 2 - MB8877 - 1 SSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK2_MB8877_2_DRIVES,   msx_slot_disk2_mb8877_2_drives_device,   "msx_slot_disk2_mb8877_2_drives",   "MSX Internal floppy type 2 - MB8877 - 2 DSDD Drives")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK2_WD2793,            msx_slot_disk2_wd2793_device,            "msx_slot_disk2_wd2793",            "MSX Internal floppy type 2 - WD2793 - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK2_WD2793_2_DRIVES,   msx_slot_disk2_wd2793_2_drives_device,   "msx_slot_disk2_wd2793_2_drives",   "MSX Internal floppy type 2 - WD2793 - 2 DSDD Drives")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK3_TC8566,            msx_slot_disk3_tc8566_device,            "msx_slot_disk3_tc8566",            "MSX Internal floppy type 3 - TC8566 - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK3_TC8566_2_DRIVES,   msx_slot_disk3_tc8566_2_drives_device,   "msx_slot_disk3_tc8566_2_drives",   "MSX Internal floppy type 3 - TC8566 - 2 DSDD Drives")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK4_TC8566,            msx_slot_disk4_tc8566_device,            "msx_slot_disk4_tc8566",            "MSX Internal floppy type 4 - TC8566 - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK5_WD2793,            msx_slot_disk5_wd2793_device,            "msx_slot_disk5_wd2793",            "MSX Internal floppy type 5 - WD2793 - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK6_WD2793_N,          msx_slot_disk6_wd2793_n_device,          "msx_slot_disk6_wd2793_n",          "MSX Internal floppy type 6 - WD2793 no force ready - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK7_MB8877,            msx_slot_disk7_mb8877_device,            "msx_slot_disk7_mb8877",            "MSX Internal floppy type 7 - MB8877 - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK8_MB8877,            msx_slot_disk8_mb8877_device,            "msx_slot_disk8_mb8877",            "MSX Internal floppy type 8 - MB8877 - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK8_WD2793_2_DRIVES,   msx_slot_disk8_wd2793_2_drives_device,   "msx_slot_disk8_wd2793_2_drives",   "MSX Internal floppy type 8 - WD2793 - 2 DSDD Drives")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK9_WD2793_N,          msx_slot_disk9_wd2793_n_device,          "msx_slot_disk9_wd2793_n",          "MSX Internal floppy type 9 - WD2793 no force ready - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK10_MB8877,           msx_slot_disk10_mb8877_device,           "msx_slot_disk10_mb8877",           "MSX Internal floppy type 10 - MB8877 - 1 DSDD Drive")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK10_MB8877_2_DRIVES,  msx_slot_disk10_mb8877_2_drives_device,  "msx_slot_disk10_mb8877_2_drives",  "MSX Internal floppy type 10 - MB8877 - 2 DSDD Drives")
DEFINE_DEVICE_TYPE(MSX_SLOT_DISK11_WD2793,           msx_slot_disk11_wd2793_device,           "msx_slot_disk11_wd2793",           "MSX Internal floppy type 11 - WD2793 - 1 DSDD Drive")


static void msx_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35ssdd", FLOPPY_35_SSDD);
}

void msx_slot_disk_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_MSX_FORMAT);
	fr.add(FLOPPY_DMK_FORMAT);
}


msx_slot_disk_device::msx_slot_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int nr_drives)
	: msx_slot_rom_device(mconfig, type, tag, owner, clock)
	, m_floppy(*this, "fdc:%u", 0U)
	, m_current_floppy(nullptr)
	, m_internal_drive_led(*this, "internal_drive%u_led", 0U)
	, m_nr_drives(nr_drives)
{
}

void msx_slot_disk_device::add_drive_mconfig(machine_config &config, bool double_sided)
{
	for (int drive = 0; drive < m_nr_drives; drive++)
		FLOPPY_CONNECTOR(config, m_floppy[drive], msx_floppies, double_sided ? "35dd" : "35ssdd", msx_slot_disk_device::floppy_formats).enable_sound(true);
}

void msx_slot_disk_device::device_start()
{
	msx_slot_rom_device::device_start();

	m_internal_drive_led.resolve();
}

void msx_slot_disk_device::device_reset()
{
	msx_slot_rom_device::device_reset();
	for (int i = 0; i < m_internal_drive_led.size(); i++)
	{
		set_drive_access_led_state(i, 0);
	}
}

void msx_slot_disk_device::set_drive_access_led_state(int drive, int led_state)
{
	m_internal_drive_led[drive] = led_state;
}


msx_slot_wd_disk_device::msx_slot_wd_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int nr_drives)
	: msx_slot_disk_device(mconfig, type, tag, owner, clock, nr_drives)
	, m_fdc(*this, "fdc")
	, m_control_led_bit(0)
{
}

void msx_slot_wd_disk_device::device_start()
{
	msx_slot_disk_device::device_start();
}

template <typename FDCType>
void msx_slot_wd_disk_device::add_mconfig(machine_config &config, FDCType &&type, bool force_ready, bool double_sided)
{
	std::forward<FDCType>(type)(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->set_force_ready(force_ready);
	add_drive_mconfig(config, double_sided);
}



msx_slot_tc8566_disk_device::msx_slot_tc8566_disk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int nr_drives)
	: msx_slot_disk_device(mconfig, type, tag, owner, clock, nr_drives)
	, m_fdc(*this, "fdc")
{
}

void msx_slot_tc8566_disk_device::add_mconfig(machine_config &config)
{
	TC8566AF(config, m_fdc, 16'000'000);

	add_drive_mconfig(config, DS);
}

void msx_slot_tc8566_disk_device::dor_w(u8 data)
{
	// Assuming drive LEDs are connected to the motor signal
	set_drive_access_led_state(0, BIT(data, 4));
	set_drive_access_led_state(1, BIT(data, 5));
	m_fdc->dor_w(data);
}



msx_slot_disk1_base_device::msx_slot_disk1_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int nr_drives)
	: msx_slot_wd_disk_device(mconfig, type, tag, owner, clock, nr_drives)
	, m_side_control(0)
	, m_control(0)
{
	set_control_led_bit(6);
}

void msx_slot_disk1_base_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_side_control));
	save_item(NAME(m_control));

	page(1)->install_rom(0x4000, 0x7fff, rom_base());
	page(1)->install_read_handler(0x7ff8, 0x7ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(1)->install_read_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(1)->install_read_handler(0x7ffa, 0x7ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(1)->install_read_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(1)->install_read_handler(0x7ffc, 0x7ffc, emu::rw_delegate(*this, FUNC(msx_slot_disk1_base_device::side_control_r)));
	page(1)->install_read_handler(0x7ffd, 0x7ffd, emu::rw_delegate(*this, FUNC(msx_slot_disk1_base_device::control_r)));
	page(1)->install_read_handler(0x7fff, 0x7fff, emu::rw_delegate(*this, FUNC(msx_slot_disk1_base_device::status_r)));
	page(1)->install_write_handler(0x7ff8, 0x7ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(1)->install_write_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(1)->install_write_handler(0x7ffa, 0x7ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(1)->install_write_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(1)->install_write_handler(0x7ffc, 0x7ffc, emu::rw_delegate(*this, FUNC(msx_slot_disk1_base_device::set_side_control)));
	page(1)->install_write_handler(0x7ffd, 0x7ffd, emu::rw_delegate(*this, FUNC(msx_slot_disk1_base_device::set_control)));

	if (page_configured(2))
	{
		page(2)->install_read_handler(0xbff8, 0xbff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
		page(2)->install_read_handler(0xbff9, 0xbff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
		page(2)->install_read_handler(0xbffa, 0xbffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
		page(2)->install_read_handler(0xbffb, 0xbffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
		page(2)->install_read_handler(0xbffc, 0xbffc, emu::rw_delegate(*this, FUNC(msx_slot_disk1_base_device::side_control_r)));
		page(2)->install_read_handler(0xbffd, 0xbffd, emu::rw_delegate(*this, FUNC(msx_slot_disk1_base_device::control_r)));
		page(2)->install_read_handler(0xbfff, 0xbfff, emu::rw_delegate(*this, FUNC(msx_slot_disk1_base_device::status_r)));
		page(2)->install_write_handler(0xbff8, 0xbff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
		page(2)->install_write_handler(0xbff9, 0xbff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
		page(2)->install_write_handler(0xbffa, 0xbffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
		page(2)->install_write_handler(0xbffb, 0xbffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
		page(2)->install_write_handler(0xbffc, 0xbffc, emu::rw_delegate(*this, FUNC(msx_slot_disk1_base_device::set_side_control)));
		page(2)->install_write_handler(0xbffd, 0xbffd, emu::rw_delegate(*this, FUNC(msx_slot_disk1_base_device::set_control)));
	}
}

void msx_slot_disk1_base_device::device_reset()
{
	msx_slot_disk_device::device_reset();
	m_fdc->dden_w(false);
}

void msx_slot_disk1_base_device::device_post_load()
{
	set_control(m_control);
}

void msx_slot_disk1_base_device::set_side_control(u8 data)
{
	m_side_control = data;

	if (m_current_floppy)
	{
		m_current_floppy->ss_w(BIT(m_side_control, 0));
	}
}

void msx_slot_disk1_base_device::set_control(u8 data)
{
	// 7------- motor on (0 = on)
	// -6------ in-use / LED (0 = on)
	// --5432-- unused
	// ------1- drive 1 select (0 = selected)
	// -------0 drive 0 select (0 = selected)

	m_control = data;

	switch (m_control & 0x03)
	{
	case 0:
	case 2:
		m_current_floppy = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
		set_drive_access_led_state(0, BIT(m_control, get_control_led_bit()));
		set_drive_access_led_state(1, 0);
		break;

	case 1:
		m_current_floppy = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, BIT(m_control, get_control_led_bit()));
		break;

	default:
		m_current_floppy = nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, 0);
		break;
	}

	if (m_current_floppy)
	{
		m_current_floppy->mon_w(BIT(m_control, 7) ? 0 : 1);
		m_current_floppy->ss_w(BIT(m_side_control, 0));
	}

	m_fdc->set_floppy(m_current_floppy);
}

u8 msx_slot_disk1_base_device::side_control_r()
{
	return 0xfe | (m_side_control & 0x01);
}

u8 msx_slot_disk1_base_device::control_r()
{
	return (m_control & 0x83) | 0x78;
}

u8 msx_slot_disk1_base_device::status_r()
{
	return 0x3f | (m_fdc->intrq_r() ? 0 : 0x40) | (m_fdc->drq_r() ? 0 : 0x80);
}


msx_slot_disk1_fd1793_device::msx_slot_disk1_fd1793_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk1_base_device(mconfig, MSX_SLOT_DISK1_FD1793, tag, owner, clock, DRIVES_1)
{
}

void msx_slot_disk1_fd1793_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, FD1793, NO_FORCE_READY, DS);
}


msx_slot_disk1_mb8877_device::msx_slot_disk1_mb8877_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk1_base_device(mconfig, MSX_SLOT_DISK1_MB8877, tag, owner, clock, DRIVES_1)
{
}

void msx_slot_disk1_mb8877_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, MB8877, FORCE_READY, DS);
}


msx_slot_disk1_wd2793_n_device::msx_slot_disk1_wd2793_n_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk1_base_device(mconfig, MSX_SLOT_DISK1_WD2793_N, tag, owner, clock, DRIVES_1)
{
}

void msx_slot_disk1_wd2793_n_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, NO_FORCE_READY, DS);
}


msx_slot_disk1_wd2793_n_2_drives_device::msx_slot_disk1_wd2793_n_2_drives_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk1_base_device(mconfig, MSX_SLOT_DISK1_WD2793_N_2_DRIVES, tag, owner, clock, DRIVES_2)
{
}

void msx_slot_disk1_wd2793_n_2_drives_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, NO_FORCE_READY, DS);
}


msx_slot_disk1_wd2793_device::msx_slot_disk1_wd2793_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk1_base_device(mconfig, MSX_SLOT_DISK1_WD2793, tag, owner, clock, DRIVES_1)
{
}

void msx_slot_disk1_wd2793_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, FORCE_READY, DS);
}


msx_slot_disk1_wd2793_0_device::msx_slot_disk1_wd2793_0_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk1_base_device(mconfig, MSX_SLOT_DISK1_WD2793_0, tag, owner, clock, NO_DRIVES)
{
}

void msx_slot_disk1_wd2793_0_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, FORCE_READY, DS);
}


msx_slot_disk1_wd2793_ss_device::msx_slot_disk1_wd2793_ss_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk1_base_device(mconfig, MSX_SLOT_DISK1_WD2793_SS, tag, owner, clock, DRIVES_1)
{
}

void msx_slot_disk1_wd2793_ss_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, FORCE_READY, SS);
}


msx_slot_disk1_wd2793_2_drives_device::msx_slot_disk1_wd2793_2_drives_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk1_base_device(mconfig, MSX_SLOT_DISK1_WD2793_2_DRIVES, tag, owner, clock, DRIVES_2)
{
}

void msx_slot_disk1_wd2793_2_drives_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, FORCE_READY, DS);
}




msx_slot_disk2_base_device::msx_slot_disk2_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int nr_drives)
	: msx_slot_wd_disk_device(mconfig, type, tag, owner, clock, nr_drives)
	, m_control(0)
{
	set_control_led_bit(6);
}

void msx_slot_disk2_base_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_control));

	page(1)->install_rom(0x4000, 0x7fff, rom_base());
	page(1)->install_read_handler(0x7fb8, 0x7fb8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(1)->install_read_handler(0x7fb9, 0x7fb9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(1)->install_read_handler(0x7fba, 0x7fba, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(1)->install_read_handler(0x7fbb, 0x7fbb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(1)->install_read_handler(0x7fbc, 0x7fbc, emu::rw_delegate(*this, FUNC(msx_slot_disk2_base_device::status_r)));
	page(1)->install_write_handler(0x7fb8, 0x7fb8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(1)->install_write_handler(0x7fb9, 0x7fb9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(1)->install_write_handler(0x7fba, 0x7fba, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(1)->install_write_handler(0x7fbb, 0x7fbb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(1)->install_write_handler(0x7fbc, 0x7fbc, emu::rw_delegate(*this, FUNC(msx_slot_disk2_base_device::set_control)));

	page(2)->install_read_handler(0xbfb8, 0xbfb8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(2)->install_read_handler(0xbfb9, 0xbfb9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(2)->install_read_handler(0xbfba, 0xbfba, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(2)->install_read_handler(0xbfbb, 0xbfbb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(2)->install_read_handler(0xbfbc, 0xbfbc, emu::rw_delegate(*this, FUNC(msx_slot_disk2_base_device::status_r)));
	page(2)->install_write_handler(0xbfb8, 0xbfb8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(2)->install_write_handler(0xbfb9, 0xbfb9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(2)->install_write_handler(0xbfba, 0xbfba, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(2)->install_write_handler(0xbfbb, 0xbfbb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(2)->install_write_handler(0xbfbc, 0xbfbc, emu::rw_delegate(*this, FUNC(msx_slot_disk2_base_device::set_control)));
}

void msx_slot_disk2_base_device::device_reset()
{
	msx_slot_disk_device::device_reset();
	m_fdc->dden_w(false);
}

void msx_slot_disk2_base_device::device_post_load()
{
	set_control(m_control);
}

void msx_slot_disk2_base_device::set_control(u8 data)
{
	m_control = data;

	switch (m_control & 3)
	{
	case 1:
		m_current_floppy = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
		set_drive_access_led_state(0, BIT(m_control, get_control_led_bit()));
		set_drive_access_led_state(1, 0);
		break;

	case 2:
		m_current_floppy = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, BIT(m_control, get_control_led_bit()));
		break;

	default:
		m_current_floppy = nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, 0);
		break;
	}

	if (m_current_floppy)
	{
		m_current_floppy->mon_w(BIT(m_control, 3) ? 0 : 1);
		m_current_floppy->ss_w(BIT(m_control, 2) ? 1 : 0);
	}

	m_fdc->set_floppy(m_current_floppy);
}

u8 msx_slot_disk2_base_device::status_r()
{
	return 0x3f | (m_fdc->drq_r() ? 0 : 0x40) | (m_fdc->intrq_r() ? 0x80 : 0);
}


msx_slot_disk2_fd1793_ss_device::msx_slot_disk2_fd1793_ss_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk2_base_device(mconfig, MSX_SLOT_DISK2_FD1793_SS, tag, owner, clock, DRIVES_1)
{
}

void msx_slot_disk2_fd1793_ss_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, FD1793, FORCE_READY, SS);
}


msx_slot_disk2_mb8877_device::msx_slot_disk2_mb8877_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk2_base_device(mconfig, MSX_SLOT_DISK2_MB8877, tag, owner, clock, DRIVES_1)
{
}

void msx_slot_disk2_mb8877_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, MB8877, FORCE_READY, DS);
}


msx_slot_disk2_mb8877_ss_device::msx_slot_disk2_mb8877_ss_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk2_base_device(mconfig, MSX_SLOT_DISK2_MB8877_SS, tag, owner, clock, DRIVES_1)
{
}

void msx_slot_disk2_mb8877_ss_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, MB8877, FORCE_READY, SS);
}


msx_slot_disk2_mb8877_2_drives_device::msx_slot_disk2_mb8877_2_drives_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk2_base_device(mconfig, MSX_SLOT_DISK2_MB8877_2_DRIVES, tag, owner, clock, DRIVES_2)
{
}

void msx_slot_disk2_mb8877_2_drives_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, MB8877, FORCE_READY, DS);
}


msx_slot_disk2_wd2793_device::msx_slot_disk2_wd2793_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk2_base_device(mconfig, MSX_SLOT_DISK2_WD2793, tag, owner, clock, DRIVES_1)
{
}

void msx_slot_disk2_wd2793_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, FORCE_READY, DS);
}


msx_slot_disk2_wd2793_2_drives_device::msx_slot_disk2_wd2793_2_drives_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk2_base_device(mconfig, MSX_SLOT_DISK2_WD2793_2_DRIVES, tag, owner, clock, DRIVES_2)
{
}

void msx_slot_disk2_wd2793_2_drives_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, FORCE_READY, DS);
}






msx_slot_disk3_tc8566_device::msx_slot_disk3_tc8566_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk3_tc8566_device(mconfig, MSX_SLOT_DISK3_TC8566, tag, owner, clock, DRIVES_1)
{
}

msx_slot_disk3_tc8566_device::msx_slot_disk3_tc8566_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int nr_drives)
	: msx_slot_tc8566_disk_device(mconfig, type, tag, owner, clock, nr_drives)
{
}

void msx_slot_disk3_tc8566_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config);
}

void msx_slot_disk3_tc8566_device::device_start()
{
	msx_slot_tc8566_disk_device::device_start();

	page(1)->install_rom(0x4000, 0x7fff, rom_base());
	page(1)->install_read_handler(0x7ffa, 0x7ffa, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::msr_r)));
	page(1)->install_read_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::fifo_r)));
	page(1)->install_write_handler(0x7ff8, 0x7ff8, emu::rw_delegate(*this, FUNC(msx_slot_disk3_tc8566_device::dor_w)));
	page(1)->install_write_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::cr1_w)));
	page(1)->install_write_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::fifo_w)));

	page(2)->install_read_handler(0xbffa, 0xbffa, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::msr_r)));
	page(2)->install_read_handler(0xbffb, 0xbffb, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::fifo_r)));
	page(2)->install_write_handler(0xbff8, 0xbff8, emu::rw_delegate(*this, FUNC(msx_slot_disk3_tc8566_device::dor_w)));
	page(2)->install_write_handler(0xbff9, 0xbff9, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::cr1_w)));
	page(2)->install_write_handler(0xbffb, 0xbffb, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::fifo_w)));
}


msx_slot_disk3_tc8566_2_drives_device::msx_slot_disk3_tc8566_2_drives_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk3_tc8566_device(mconfig, MSX_SLOT_DISK3_TC8566_2_DRIVES, tag, owner, clock, DRIVES_2)
{
}

void msx_slot_disk3_tc8566_2_drives_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config);
}






msx_slot_disk4_tc8566_device::msx_slot_disk4_tc8566_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_tc8566_disk_device(mconfig, MSX_SLOT_DISK4_TC8566, tag, owner, clock, DRIVES_1)
{
}

void msx_slot_disk4_tc8566_device::device_start()
{
	msx_slot_tc8566_disk_device::device_start();

	page(1)->install_rom(0x4000, 0x7fff, rom_base());
	// 0x7ff1 media change register
	page(1)->install_write_handler(0x7ff2, 0x7ff2, emu::rw_delegate(*this, FUNC(msx_slot_disk4_tc8566_device::dor_w)));
	page(1)->install_write_handler(0x7ff3, 0x7ff3, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::cr1_w)));
	page(1)->install_read_handler(0x7ff4, 0x7ff4, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::msr_r)));
	page(1)->install_read_handler(0x7ff5, 0x7ff5, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::fifo_r)));
	page(1)->install_write_handler(0x7ff5, 0x7ff5, emu::rw_delegate(*m_fdc, FUNC(tc8566af_device::fifo_w)));
}

void msx_slot_disk4_tc8566_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config);
}




msx_slot_disk5_wd2793_device::msx_slot_disk5_wd2793_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK5_WD2793, tag, owner, clock, DRIVES_1)
	, m_control(0)
{
}

void msx_slot_disk5_wd2793_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, FORCE_READY, DS);
}

void msx_slot_disk5_wd2793_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_control));

	page(1)->install_rom(0x4000, 0x7fff, rom_base());

	// Install IO read/write handlers
	io_space().install_write_handler(0xd0, 0xd0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	io_space().install_write_handler(0xd1, 0xd1, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	io_space().install_write_handler(0xd2, 0xd2, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	io_space().install_write_handler(0xd3, 0xd3, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	io_space().install_write_handler(0xd4, 0xd4, emu::rw_delegate(*this, FUNC(msx_slot_disk5_wd2793_device::control_w)));
	io_space().install_read_handler(0xd0, 0xd0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	io_space().install_read_handler(0xd1, 0xd1, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	io_space().install_read_handler(0xd2, 0xd2, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	io_space().install_read_handler(0xd3, 0xd3, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	io_space().install_read_handler(0xd4, 0xd4, emu::rw_delegate(*this, FUNC(msx_slot_disk5_wd2793_device::status_r)));
}

void msx_slot_disk5_wd2793_device::device_reset()
{
	msx_slot_disk_device::device_reset();
	m_fdc->dden_w(false);
}

void msx_slot_disk5_wd2793_device::device_post_load()
{
	control_w(m_control);
}

void msx_slot_disk5_wd2793_device::control_w(u8 control)
{
	m_control = control;

	switch (m_control & 0x0f)
	{
	case 0x01:
		m_current_floppy = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
		set_drive_access_led_state(0, BIT(m_control, 6));
		set_drive_access_led_state(1, 0);
		set_drive_access_led_state(2, 0);
		set_drive_access_led_state(3, 0);
		break;

	case 0x02:
		m_current_floppy = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, BIT(m_control, 6));
		set_drive_access_led_state(2, 0);
		set_drive_access_led_state(3, 0);
		break;

	case 0x04:
		m_current_floppy = m_floppy[2] ? m_floppy[2]->get_device() : nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, 0);
		set_drive_access_led_state(2, BIT(m_control, 6));
		set_drive_access_led_state(3, 0);
		break;

	case 0x08:
		m_current_floppy = m_floppy[3] ? m_floppy[3]->get_device() : nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, 0);
		set_drive_access_led_state(2, 0);
		set_drive_access_led_state(3, BIT(m_control, 6));
		break;

	default:
		m_current_floppy = nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, 0);
		set_drive_access_led_state(2, 0);
		set_drive_access_led_state(3, 0);
		break;
	}

	if (m_current_floppy)
	{
		m_current_floppy->mon_w(BIT(m_control, 5) ? 0 : 1);
		m_current_floppy->ss_w(BIT(m_control, 4) ? 1 : 0);
	}

	m_fdc->set_floppy(m_current_floppy);
}

u8 msx_slot_disk5_wd2793_device::status_r()
{
	return 0x3f | (m_fdc->drq_r() ? 0 : 0x40) | (m_fdc->intrq_r() ? 0x80 : 0);
}



msx_slot_disk6_wd2793_n_device::msx_slot_disk6_wd2793_n_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK6_WD2793_N, tag, owner, clock, DRIVES_1)
	, m_side_motor(0)
	, m_drive_select(0)
	, m_drive_present(false)
	, m_unknown(0)
{
}

void msx_slot_disk6_wd2793_n_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, NO_FORCE_READY, DS);
}

void msx_slot_disk6_wd2793_n_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_side_motor));
	save_item(NAME(m_drive_select));
	save_item(NAME(m_drive_present));
	save_item(NAME(m_unknown));

	page(1)->install_rom(0x4000, 0x7fff, rom_base());
	page(1)->install_read_handler(0x7ff0, 0x7ff0, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(1)->install_read_handler(0x7ff1, 0x7ff1, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(1)->install_read_handler(0x7ff2, 0x7ff2, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(1)->install_read_handler(0x7ff3, 0x7ff3, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(1)->install_read_handler(0x7ff4, 0x7ff4, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(msx_slot_disk6_wd2793_n_device::side_motor_r)));
	page(1)->install_read_handler(0x7ff5, 0x7ff5, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(msx_slot_disk6_wd2793_n_device::select0_r)));
	page(1)->install_read_handler(0x7ff6, 0x7ff6, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(msx_slot_disk6_wd2793_n_device::dskchg_r)));
	page(1)->install_read_handler(0x7ff7, 0x7ff7, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(msx_slot_disk6_wd2793_n_device::status_r)));
	page(1)->install_write_handler(0x7ff0, 0x7ff0, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(1)->install_write_handler(0x7ff1, 0x7ff1, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(1)->install_write_handler(0x7ff2, 0x7ff2, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(1)->install_write_handler(0x7ff3, 0x7ff3, 0, 0x0008, 0, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(1)->install_write_handler(0x7ff4, 0x7ff4, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(msx_slot_disk6_wd2793_n_device::side_motor_w)));
	page(1)->install_write_handler(0x7ff5, 0x7ff5, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(msx_slot_disk6_wd2793_n_device::select_w)));
	page(1)->install_write_handler(0x7ff6, 0x7ff6, 0, 0x0008, 0, emu::rw_delegate(*this, FUNC(msx_slot_disk6_wd2793_n_device::unknown_w)));
}

void msx_slot_disk6_wd2793_n_device::device_reset()
{
	msx_slot_disk_device::device_reset();
	m_fdc->dden_w(false);
	m_drive_select = 0;
	m_drive_present = false;
	m_unknown = 0;
}

void msx_slot_disk6_wd2793_n_device::device_post_load()
{
	select_drive();
}

void msx_slot_disk6_wd2793_n_device::select_drive()
{
	if (BIT(m_drive_select, 0))
	{
		m_current_floppy = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
		m_drive_present = m_current_floppy;
	}
	else
	{
		m_current_floppy = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
		m_drive_present = m_current_floppy;
	}

	m_fdc->set_floppy(m_current_floppy);

	set_side_motor();
}

void msx_slot_disk6_wd2793_n_device::set_side_motor()
{
	if (m_current_floppy)
	{
		m_current_floppy->mon_w(BIT(m_side_motor, 1) ? 0 : 1);
		m_current_floppy->ss_w(BIT(m_side_motor, 0));
	}
	if (BIT(m_drive_select, 0))
	{
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, BIT(m_side_motor, 1));
	}
	else
	{
		set_drive_access_led_state(0, BIT(m_side_motor, 1));
		set_drive_access_led_state(1, 0);
	}
}

u8 msx_slot_disk6_wd2793_n_device::side_motor_r()
{
	// bit 0 = side control
	// bit 1 = motor control
	return 0xfc | m_side_motor;
}

u8 msx_slot_disk6_wd2793_n_device::select0_r()
{
	// This reads back a 0 in bit 0 if selected drive is present
	return 0xfe | (m_drive_present ? 0 : 1);
}

u8 msx_slot_disk6_wd2793_n_device::dskchg_r()
{
	// Disk change status for selected drive
	return 0xfe | (m_current_floppy && m_current_floppy->dskchg_r() ? 0 : 1);
}

u8 msx_slot_disk6_wd2793_n_device::status_r()
{
	return 0x3f | (m_fdc->intrq_r() ? 0 : 0x40) | (m_fdc->drq_r() ? 0 : 0x80);
}

void msx_slot_disk6_wd2793_n_device::side_motor_w(u8 data)
{
	// Side and motor control
	// bit 0 = side select
	// bit 1 = motor on/off
	m_side_motor = data;
	set_side_motor();
}

void msx_slot_disk6_wd2793_n_device::select_w(u8 data)
{
	// bit 0 - select drive 0/1
	m_drive_select = data;
	select_drive();
}

void msx_slot_disk6_wd2793_n_device::unknown_w(u8 data)
{
	// bit 0 - ? inverse of what is written to drive select.
	m_unknown = data;
}



msx_slot_disk7_mb8877_device::msx_slot_disk7_mb8877_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK7_MB8877, tag, owner, clock, DRIVES_1)
	, m_drive_side_motor(0)
{
}

void msx_slot_disk7_mb8877_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, MB8877, FORCE_READY, DS);
}

void msx_slot_disk7_mb8877_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_drive_side_motor));

	page(1)->install_rom(0x4000, 0x7fff, rom_base());
	page(1)->install_read_handler(0x7ff8, 0x7ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(1)->install_read_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(1)->install_read_handler(0x7ffa, 0x7ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(1)->install_read_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(1)->install_read_handler(0x7ffc, 0x7ffc, emu::rw_delegate(*this, FUNC(msx_slot_disk7_mb8877_device::status_r)));
	page(1)->install_write_handler(0x7ff8, 0x7ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(1)->install_write_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(1)->install_write_handler(0x7ffa, 0x7ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(1)->install_write_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(1)->install_write_handler(0x7ffc, 0x7ffc, emu::rw_delegate(*this, FUNC(msx_slot_disk7_mb8877_device::side_motor_w)));

	page(2)->install_read_handler(0xbff8, 0xbff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(2)->install_read_handler(0xbff9, 0xbff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(2)->install_read_handler(0xbffa, 0xbffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(2)->install_read_handler(0xbffb, 0xbffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(2)->install_read_handler(0xbffc, 0xbffc, emu::rw_delegate(*this, FUNC(msx_slot_disk7_mb8877_device::status_r)));
	page(2)->install_write_handler(0xbff8, 0xbff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(2)->install_write_handler(0xbff9, 0xbff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(2)->install_write_handler(0xbffa, 0xbffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(2)->install_write_handler(0xbffb, 0xbffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(2)->install_write_handler(0xbffc, 0xbffc, emu::rw_delegate(*this, FUNC(msx_slot_disk7_mb8877_device::side_motor_w)));
}

void msx_slot_disk7_mb8877_device::device_reset()
{
	msx_slot_disk_device::device_reset();
	m_fdc->dden_w(false);
}

void msx_slot_disk7_mb8877_device::device_post_load()
{
	select_drive();
}

void msx_slot_disk7_mb8877_device::select_drive()
{
	// No separate bit for the drive LEDs? Assuming LEDs are triggered by the motor on signal.
	m_current_floppy = nullptr;

	switch (m_drive_side_motor & 0x03)
	{
	case 0:
		m_current_floppy = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
		set_drive_access_led_state(0, BIT(~m_drive_side_motor, 3));
		set_drive_access_led_state(1, 0);
		break;
	case 1:
		m_current_floppy = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, BIT(~m_drive_side_motor, 3));
		break;
	default:
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, 0);
		break;
	}

	m_fdc->set_floppy(m_current_floppy);
}

void msx_slot_disk7_mb8877_device::set_drive_side_motor()
{
	select_drive();

	if (m_current_floppy)
	{
		m_current_floppy->mon_w(BIT(m_drive_side_motor, 3) ? 0 : 1);
		m_current_floppy->ss_w(BIT(m_drive_side_motor, 2));
	}
}

void msx_slot_disk7_mb8877_device::side_motor_w(u8 data)
{
	// Drive, side and motor control
	// bit 0,1 = drive select
	// bit 2 = side select
	// bit 3 = motor on/off
	m_drive_side_motor = data;
	set_drive_side_motor();
}

u8 msx_slot_disk7_mb8877_device::status_r()
{
	return (m_drive_side_motor & 0x0f)
		| (m_current_floppy && m_current_floppy->dskchg_r() ? 0x00 : 0x10)
		| (m_current_floppy && m_current_floppy->ready_r() ? 0x00 : 0x20)
		| (m_fdc->intrq_r() ? 0x40 : 0x00)
		| (m_fdc->drq_r() ? 0x80 : 0x00);
}



msx_slot_disk8_mb8877_device::msx_slot_disk8_mb8877_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_disk8_mb8877_device(mconfig, MSX_SLOT_DISK8_MB8877, tag, owner, clock, DRIVES_1)
{
}

msx_slot_disk8_mb8877_device::msx_slot_disk8_mb8877_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int nr_drives)
	: msx_slot_wd_disk_device(mconfig, type, tag, owner, clock, nr_drives)
	, m_control(0)
{
}

void msx_slot_disk8_mb8877_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, MB8877, FORCE_READY, DS);
}

void msx_slot_disk8_mb8877_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_control));

	page(1)->install_rom(0x4000, 0x7fff, rom_base());
	page(1)->install_read_handler(0x7f80, 0x7f80, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(1)->install_read_handler(0x7f81, 0x7f81, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(1)->install_read_handler(0x7f82, 0x7f82, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(1)->install_read_handler(0x7f83, 0x7f83, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(1)->install_read_handler(0x7f84, 0x7f84, emu::rw_delegate(*this, FUNC(msx_slot_disk8_mb8877_device::status_r)));
	page(1)->install_write_handler(0x7f80, 0x7f80, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(1)->install_write_handler(0x7f81, 0x7f81, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(1)->install_write_handler(0x7f82, 0x7f82, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(1)->install_write_handler(0x7f83, 0x7f83, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(1)->install_write_handler(0x7f84, 0x7f84, emu::rw_delegate(*this, FUNC(msx_slot_disk8_mb8877_device::set_control)));

	page(2)->install_read_handler(0xbf80, 0xbf80, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(2)->install_read_handler(0xbf81, 0xbf81, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(2)->install_read_handler(0xbf82, 0xbf82, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(2)->install_read_handler(0xbf83, 0xbf83, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(2)->install_read_handler(0xbf84, 0xbf84, emu::rw_delegate(*this, FUNC(msx_slot_disk8_mb8877_device::status_r)));
	page(2)->install_write_handler(0xbf80, 0xbf80, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(2)->install_write_handler(0xbf81, 0xbf81, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(2)->install_write_handler(0xbf82, 0xbf82, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(2)->install_write_handler(0xbf83, 0xbf83, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(2)->install_write_handler(0xbf84, 0xbf84, emu::rw_delegate(*this, FUNC(msx_slot_disk8_mb8877_device::set_control)));
}


void msx_slot_disk8_mb8877_device::device_reset()
{
	msx_slot_disk_device::device_reset();
	m_fdc->dden_w(false);
}


void msx_slot_disk8_mb8877_device::device_post_load()
{
	set_control(m_control);
}


void msx_slot_disk8_mb8877_device::set_control(u8 data)
{
	m_control = data;

	switch (m_control & 3)
	{
	case 1:
		m_current_floppy = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
		set_drive_access_led_state(0, BIT(m_control, 6));
		set_drive_access_led_state(1, 0);
		break;

	case 2:
		m_current_floppy = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, BIT(m_control, 6));
		break;

	default:
		m_current_floppy = nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, 0);
		break;
	}

	if (m_current_floppy)
	{
		m_current_floppy->mon_w(BIT(m_control, 3) ? 0 : 1);
		m_current_floppy->ss_w(BIT(m_control, 2) ? 1 : 0);
	}

	m_fdc->set_floppy(m_current_floppy);
}

u8 msx_slot_disk8_mb8877_device::status_r()
{
	return 0x3f | (m_fdc->drq_r() ? 0 : 0x40) | (m_fdc->intrq_r() ? 0x80 : 0);
}


msx_slot_disk8_wd2793_2_drives_device::msx_slot_disk8_wd2793_2_drives_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_disk8_mb8877_device(mconfig, MSX_SLOT_DISK8_WD2793_2_DRIVES, tag, owner, clock, DRIVES_2)
{
}

void msx_slot_disk8_wd2793_2_drives_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, FORCE_READY, DS);
}




msx_slot_disk9_wd2793_n_device::msx_slot_disk9_wd2793_n_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK9_WD2793_N, tag, owner, clock, DRIVES_1)
	, m_control(0)
{
}

void msx_slot_disk9_wd2793_n_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, NO_FORCE_READY, DS);
}

void msx_slot_disk9_wd2793_n_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_control));

	page(1)->install_rom(0x4000, 0x7fff, rom_base());
	page(1)->install_read_handler(0x7ff8, 0x7ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(1)->install_read_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(1)->install_read_handler(0x7ffa, 0x7ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(1)->install_read_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(1)->install_read_handler(0x7ffc, 0x7ffc, emu::rw_delegate(*this, FUNC(msx_slot_disk9_wd2793_n_device::status_r)));
	page(1)->install_write_handler(0x7ff8, 0x7ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(1)->install_write_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(1)->install_write_handler(0x7ffa, 0x7ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(1)->install_write_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(1)->install_write_handler(0x7ffc, 0x7ffc, emu::rw_delegate(*this, FUNC(msx_slot_disk9_wd2793_n_device::control_w)));
}

void msx_slot_disk9_wd2793_n_device::device_reset()
{
	msx_slot_disk_device::device_reset();
	m_fdc->dden_w(false);
}

void msx_slot_disk9_wd2793_n_device::device_post_load()
{
	control_w(m_control);
}

void msx_slot_disk9_wd2793_n_device::control_w(u8 data)
{
	m_control = data;

	switch (m_control & 0x03)
	{
	case 1:
		m_current_floppy = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
		set_drive_access_led_state(0, BIT(m_control, 6));
		set_drive_access_led_state(1, 0);
		break;

	case 2:
		m_current_floppy = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, BIT(m_control, 6));
		break;

	default:
		m_current_floppy = nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, 0);
		break;
	}

	if (m_current_floppy)
	{
		m_current_floppy->mon_w(BIT(m_control, 3) ? 0 : 1);
		m_current_floppy->ss_w(BIT(m_control, 2));
	}

	m_fdc->set_floppy(m_current_floppy);
}

u8 msx_slot_disk9_wd2793_n_device::status_r()
{
	return 0x3f | (m_fdc->intrq_r() ? 0x80 : 0) | (m_fdc->drq_r() ? 0x40 : 0);
}




msx_slot_disk10_mb8877_device::msx_slot_disk10_mb8877_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk10_mb8877_device(mconfig, MSX_SLOT_DISK10_MB8877, tag, owner, clock, DRIVES_1)
{
}

msx_slot_disk10_mb8877_device::msx_slot_disk10_mb8877_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int nr_drives)
	: msx_slot_wd_disk_device(mconfig, type, tag, owner, clock, nr_drives)
	, m_control(0)
{
}

void msx_slot_disk10_mb8877_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, MB8877, FORCE_READY, DS);
}

void msx_slot_disk10_mb8877_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_control));

	page(1)->install_rom(0x4000, 0x7fff, rom_base());
	page(1)->install_read_handler(0x7ff8, 0x7ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(1)->install_read_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(1)->install_read_handler(0x7ffa, 0x7ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(1)->install_read_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(1)->install_read_handler(0x7ffc, 0x7ffc, emu::rw_delegate(*this, FUNC(msx_slot_disk10_mb8877_device::status_r)));
	// TODO 0x7ffd return system control/status
	page(1)->install_write_handler(0x7ff8, 0x7ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(1)->install_write_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(1)->install_write_handler(0x7ffa, 0x7ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(1)->install_write_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(1)->install_write_handler(0x7ffc, 0x7ffc, emu::rw_delegate(*this, FUNC(msx_slot_disk10_mb8877_device::control_w)));
}

void msx_slot_disk10_mb8877_device::device_reset()
{
	msx_slot_disk_device::device_reset();
	m_fdc->dden_w(false);
}

void msx_slot_disk10_mb8877_device::device_post_load()
{
	control_w(m_control);
}

void msx_slot_disk10_mb8877_device::control_w(u8 data)
{
	m_control = data;

	// Assuming drive LEDs are connected to the motor signals
	if (BIT(m_control, 2))
	{
		m_current_floppy = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, BIT(m_control, 1));
	}
	else
	{
		m_current_floppy = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
		set_drive_access_led_state(0, BIT(m_control, 0));
		set_drive_access_led_state(1, 0);
	}

	if (m_current_floppy && !BIT(m_control, 2))
		m_current_floppy->mon_w(BIT(m_control, 0) ? 0 : 1);

	if (m_current_floppy && BIT(m_control, 2))
		m_current_floppy->mon_w(BIT(m_control, 1) ? 0 : 1);

	if (m_current_floppy)
		m_current_floppy->ss_w(BIT(m_control, 3));

	m_fdc->set_floppy(m_current_floppy);
}

u8 msx_slot_disk10_mb8877_device::status_r()
{
	return 0x23
		| (m_control & 0x04)
		| ((m_control & 0x08) ^ 0x08)
		| ((m_current_floppy && m_current_floppy->mon_r()) ? 0 : 0x10)
		| (m_fdc->drq_r() ? 0x40 : 0)
		| (m_fdc->intrq_r() ? 0x80 : 0);
}


msx_slot_disk10_mb8877_2_drives_device::msx_slot_disk10_mb8877_2_drives_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_disk10_mb8877_device(mconfig, MSX_SLOT_DISK10_MB8877_2_DRIVES, tag, owner, clock, DRIVES_2)
{
}

void msx_slot_disk10_mb8877_2_drives_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, MB8877, FORCE_READY, DS);
}



msx_slot_disk11_wd2793_device::msx_slot_disk11_wd2793_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_wd_disk_device(mconfig, MSX_SLOT_DISK11_WD2793, tag, owner, clock, DRIVES_1)
	, m_side_control(0)
	, m_control(0)
{
}

void msx_slot_disk11_wd2793_device::device_add_mconfig(machine_config &config)
{
	add_mconfig(config, WD2793, FORCE_READY, DS);
}

void msx_slot_disk11_wd2793_device::device_start()
{
	msx_slot_wd_disk_device::device_start();

	save_item(NAME(m_side_control));
	save_item(NAME(m_control));

	page(1)->install_rom(0x4000, 0x7fff, rom_base());
	page(1)->install_read_handler(0x7ff8, 0x7ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(1)->install_read_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(1)->install_read_handler(0x7ffa, 0x7ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(1)->install_read_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(1)->install_read_handler(0x7ffc, 0x7ffc, emu::rw_delegate(*this, FUNC(msx_slot_disk11_wd2793_device::side_control_r)));
	page(1)->install_read_handler(0x7ffd, 0x7ffd, emu::rw_delegate(*this, FUNC(msx_slot_disk11_wd2793_device::control_r)));
	page(1)->install_read_handler(0x7fff, 0x7fff, emu::rw_delegate(*this, FUNC(msx_slot_disk11_wd2793_device::status_r)));
	page(1)->install_write_handler(0x7ff8, 0x7ff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(1)->install_write_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(1)->install_write_handler(0x7ffa, 0x7ffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(1)->install_write_handler(0x7ffb, 0x7ffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(1)->install_write_handler(0x7ffc, 0x7ffc, emu::rw_delegate(*this, FUNC(msx_slot_disk11_wd2793_device::side_control_w)));
	page(1)->install_write_handler(0x7ffd, 0x7ffd, emu::rw_delegate(*this, FUNC(msx_slot_disk11_wd2793_device::control_w)));

	page(2)->install_read_handler(0xbff8, 0xbff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::status_r)));
	page(2)->install_read_handler(0xbff9, 0xbff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_r)));
	page(2)->install_read_handler(0xbffa, 0xbffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_r)));
	page(2)->install_read_handler(0xbffb, 0xbffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_r)));
	page(2)->install_read_handler(0xbffc, 0xbffc, emu::rw_delegate(*this, FUNC(msx_slot_disk11_wd2793_device::side_control_r)));
	page(2)->install_read_handler(0xbffd, 0xbffd, emu::rw_delegate(*this, FUNC(msx_slot_disk11_wd2793_device::control_r)));
	page(2)->install_read_handler(0xbfff, 0xbfff, emu::rw_delegate(*this, FUNC(msx_slot_disk11_wd2793_device::status_r)));
	page(2)->install_write_handler(0xbff8, 0xbff8, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::cmd_w)));
	page(2)->install_write_handler(0xbff9, 0xbff9, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::track_w)));
	page(2)->install_write_handler(0xbffa, 0xbffa, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::sector_w)));
	page(2)->install_write_handler(0xbffb, 0xbffb, emu::rw_delegate(*m_fdc, FUNC(wd_fdc_analog_device_base::data_w)));
	page(2)->install_write_handler(0xbffc, 0xbffc, emu::rw_delegate(*this, FUNC(msx_slot_disk11_wd2793_device::side_control_w)));
	page(2)->install_write_handler(0xbffd, 0xbffd, emu::rw_delegate(*this, FUNC(msx_slot_disk11_wd2793_device::control_w)));
}

void msx_slot_disk11_wd2793_device::device_reset()
{
	msx_slot_disk_device::device_reset();
	m_fdc->dden_w(false);
}

void msx_slot_disk11_wd2793_device::device_post_load()
{
	control_w(m_control);
}

void msx_slot_disk11_wd2793_device::side_control_w(u8 data)
{
	m_side_control = data;

	if (m_current_floppy)
		m_current_floppy->ss_w(BIT(m_side_control, 0));
}

void msx_slot_disk11_wd2793_device::control_w(u8 data)
{
	m_control = data;

	switch (m_control & 0x03)
	{
	case 0:
		m_current_floppy = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
		set_drive_access_led_state(0, BIT(m_control, 6));  // Wild guess based on other floppy interfaces
		set_drive_access_led_state(1, 0);
		break;

	case 1:
		m_current_floppy = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, BIT(m_control, 6));  // Wild guess based on other floppy interfaces
		break;

	default:
		m_current_floppy = nullptr;
		set_drive_access_led_state(0, 0);
		set_drive_access_led_state(1, 0);
		break;
	}

	if (m_current_floppy)
	{
		m_current_floppy->mon_w(BIT(m_control, 7) ? 0 : 1);
		m_current_floppy->ss_w(BIT(m_side_control, 0));
	}

	m_fdc->set_floppy(m_current_floppy);
}

u8 msx_slot_disk11_wd2793_device::side_control_r()
{
	return 0xfe | (m_side_control & 0x01);
}

u8 msx_slot_disk11_wd2793_device::control_r()
{
	return (m_control & 0x83) | 0x78;
}

u8 msx_slot_disk11_wd2793_device::status_r()
{
	// bit 4 is checked during reading (ready check?)
	return 0x3f | (m_fdc->intrq_r() ? 0 : 0x40) | (m_fdc->drq_r() ? 0 : 0x80);
}
