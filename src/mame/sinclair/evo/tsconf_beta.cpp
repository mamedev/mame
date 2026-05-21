// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Beta TR-DOS with virtual drives
**********************************************************************/

#include "emu.h"
#include "tsconf_beta.h"

#include "formats/trd_dsk.h"


namespace {

void floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add_pc_formats();
	fr.add(FLOPPY_TRD_FORMAT);
}

void beta_disk_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("525qd", FLOPPY_525_QD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("35dd", FLOPPY_35_DD);
}

} // anonymous namespace


// device type definition
DEFINE_DEVICE_TYPE(TSCONF_BETA, tsconf_beta_device, "tsconf_beta", "Virtual TR-DOS")

void tsconf_beta_device::tsconf_beta_io(address_map &map)
{
	map(0x001f, 0x001f).mirror(0xff00).rw(FUNC(tsconf_beta_device::status_r), FUNC(tsconf_beta_device::command_w));
	map(0x003f, 0x003f).mirror(0xff00).rw(FUNC(tsconf_beta_device::track_r), FUNC(tsconf_beta_device::track_w));
	map(0x005f, 0x005f).mirror(0xff00).rw(FUNC(tsconf_beta_device::sector_r), FUNC(tsconf_beta_device::sector_w));
	map(0x007f, 0x007f).mirror(0xff00).rw(FUNC(tsconf_beta_device::data_r), FUNC(tsconf_beta_device::data_w));
	map(0x009f, 0x009f).select(0xff60).rw(FUNC(tsconf_beta_device::state_r), FUNC(tsconf_beta_device::param_w));
}


tsconf_beta_device::tsconf_beta_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TSCONF_BETA, tag, owner, clock)
	, m_out_dos_cb(*this)
	, m_out_vdos_m1_cb(*this)
	, m_wd179x(*this, "wd179x")
	, m_floppy(*this, "wd179x:%u", 0U)
	, m_floppy_led(*this, "fdd%u_led", 0U)
	, m_control(0)
	, m_motor_active(false)
{
}

bool tsconf_beta_device::pre_vg_in_check()
{
	if (m_dos)
	{
		if (m_vdos)
		{
			m_vdos = false;
			m_out_dos_cb(dos_io_r());
			return false;
		}
		else if ((1 << (m_control & 3)) & m_fddvirt)
		{
			m_out_vdos_m1_cb(1);
			return false;
		}
	}

	return true;
}

u8 tsconf_beta_device::status_r()
{
	return pre_vg_in_check() ? m_wd179x->status_r() : 0xff;
}

u8 tsconf_beta_device::track_r()
{
	return pre_vg_in_check() ? m_wd179x->track_r() : 0xff;
}

u8 tsconf_beta_device::sector_r()
{
	return pre_vg_in_check() ? m_wd179x->sector_r() : 0xff;
}

u8 tsconf_beta_device::data_r()
{
	return pre_vg_in_check() ? m_wd179x->data_r() : 0xff;
}

u8 tsconf_beta_device::state_r()
{
	if (pre_vg_in_check())
	{
		u8 result = 0x3f;       // actually open bus
		result |= m_wd179x->drq_r() ? 0x40 : 0;
		result |= m_wd179x->intrq_r() ? 0x80 : 0;
		return result;
	}
	else
	{
		return 0xff;
	}
}

bool tsconf_beta_device::pre_vg_out_check(bool is_port_match = true)
{
	if (m_dos)
	{
		if (m_vdos)
		{
			if (is_port_match)
			{
				m_vdos = false;
				m_out_dos_cb(dos_io_r());
				return false;
			}
		}
		else if ((1 << (m_control & 3)) & m_fddvirt)
		{
			m_vdos = true;
			m_out_dos_cb(dos_io_r());
			return false;
		}
	}

	return true;
}

void tsconf_beta_device::param_w(offs_t offset, u8 data)
{
	if (pre_vg_out_check((offset & 0x60) != 0x60)) // not through 0xff
	{
		m_control = data;
		floppy_image_device* floppy = m_floppy[data & 3]->get_device();
		m_wd179x->set_floppy(floppy);

		if (!m_vdos)
		{
			floppy->ss_w(BIT(data, 4) ? 0 : 1);
			m_wd179x->dden_w(BIT(data, 6));
			m_wd179x->mr_w(BIT(data, 2));

			m_wd179x->hlt_w(BIT(data, 3));

			motors_control();
		}
	}
}

void tsconf_beta_device::command_w(u8 data)
{
	if (pre_vg_out_check())
	{
		m_wd179x->cmd_w(data);
	}
}

void tsconf_beta_device::track_w(u8 data)
{
	if (pre_vg_out_check())
	{
		m_wd179x->track_w(data);
	}
}

void tsconf_beta_device::sector_w(u8 data)
{
	if (pre_vg_out_check())
	{
		m_wd179x->sector_w(data);
	}
}

void tsconf_beta_device::data_w(u8 data)
{
	if (pre_vg_out_check())
	{
		m_wd179x->data_w(data);
	}
}

void tsconf_beta_device::turbo_w(int state)
{
	m_wd179x->set_clock_scale(1 << (state & 1));
}

void tsconf_beta_device::on_m1_w()
{
	m_vdos = true;
	m_out_dos_cb(dos_io_r());
}

void tsconf_beta_device::enable_w(bool state)
{
	if ((state && !m_dos) || (!state && m_dos && !m_vdos))
	{
		m_dos = state;
		m_out_dos_cb(dos_io_r());
	}
}

void tsconf_beta_device::fddvirt_w(u8 fddvirt)
{
	m_fddvirt = fddvirt & 0x0f;
}

void tsconf_beta_device::io_forced_w(bool io_forced)
{
	m_io_forced = io_forced;
	m_out_dos_cb(dos_io_r());
}

void tsconf_beta_device::fdc_hld_w(int state)
{
	m_wd179x->set_force_ready(state); // HLD connected to RDY pin
	m_motor_active = state;
	motors_control();
}

void tsconf_beta_device::motors_control()
{
	for (int i = 0; i < 4; i++)
	{
		if (m_motor_active && (m_control & 3) == i)
		{
			m_floppy[i]->get_device()->mon_w(CLEAR_LINE);
			m_floppy_led[i] = 1;
		}
		else
		{
			m_floppy[i]->get_device()->mon_w(ASSERT_LINE);
			m_floppy_led[i] = 0;
		}
	}
}

void tsconf_beta_device::device_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_motor_active));
	save_item(NAME(m_dos));
	save_item(NAME(m_vdos));
	save_item(NAME(m_io_forced));
	save_item(NAME(m_fddvirt));

	m_floppy_led.resolve();
}

void tsconf_beta_device::device_reset()
{
	m_control = 0;
	for (int i = 0; i < m_floppy_led.size(); i++)
		m_floppy_led[i] = 0;

	m_dos = false;
	m_vdos = false;
	m_io_forced = false;
	m_fddvirt = 0;
}

void tsconf_beta_device::device_add_mconfig(machine_config &config)
{
	KR1818VG93(config, m_wd179x, 8_MHz_XTAL / 8);
	m_wd179x->hld_wr_callback().set(FUNC(tsconf_beta_device::fdc_hld_w));
	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, beta_disk_floppies, "525qd", floppy_formats).enable_sound(true);
}
