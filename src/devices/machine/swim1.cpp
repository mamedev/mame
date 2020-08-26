// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Implementation of the Apple SWIM1 floppy disk controller

*********************************************************************/

#include "emu.h"
#include "swim1.h"

DEFINE_DEVICE_TYPE(SWIM1, swim1_device, "swim1", "Apple SWIM1 (Sander/Wozniak Integrated Machine) version 1 floppy controller")

swim1_device::swim1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	applefdintf_device(mconfig, SWIM1, tag, owner, clock)
{
}

void swim1_device::device_start()
{
	applefdintf_device::device_start();
	save_item(NAME(m_mode));
	save_item(NAME(m_setup));
	save_item(NAME(m_param_idx));
	save_item(NAME(m_param));
}

void swim1_device::device_reset()
{
	applefdintf_device::device_reset();
	m_mode = 0x40;
	m_setup = 0x00;
	m_param_idx = 0;
	memset(m_param, 0, sizeof(m_param));
	m_floppy = nullptr;

	m_devsel_cb(0);
	m_sel35_cb(true);
	m_hdsel_cb(false);
}

void swim1_device::device_timer(emu_timer &, device_timer_id, int, void *)
{
}

void swim1_device::set_floppy(floppy_image_device *floppy)
{
	if(m_floppy == floppy)
		return;

	if(m_floppy)
		m_floppy->mon_w(true);
	m_floppy = floppy;
	if(m_mode & 0x80)
		m_floppy->mon_w(false);
	update_phases();
}

floppy_image_device *swim1_device::get_floppy() const
{
	return m_floppy;
}

void swim1_device::show_mode() const
{
	logerror("mode%s %s hdsel=%c %c%s %c%c%s\n",
			 m_mode & 0x80 ? " motoron" : "",
			 m_mode & 0x40 ? "ism" : "iwm",
			 m_mode & 0x20 ? '1' : '0',
			 m_mode & 0x10 ? 'w' : 'r',
			 m_mode & 0x08 ? " action" : "",
			 m_mode & 0x04 ? 'a' : '-',
			 m_mode & 0x02 ? 'b' : '-',
			 m_mode & 0x01 ? " clear" : "");

}

u8 swim1_device::read(offs_t offset)
{
	static const char *const names[] = {
		"?0", "?1", "?2", "?3", "?4", "?5", "?6", "?7",
		"data", "mark", "crc", "param", "phases", "setup", "status", "handshake"
	};
	switch(offset) {
	case 0x3: case 0xb: {
		u8 r = m_param[m_param_idx];
		m_param_idx = (m_param_idx + 1) & 15;
		return r;
	}
	case 0x4: case 0xc:
		return m_phases;
	case 0x5: case 0xd:
		return m_setup;
	case 0xe:
		return m_mode;
	default:
		logerror("read %s\n", names[offset & 15]);
		break;
	}
	return 0xff;
}

void swim1_device::write(offs_t offset, u8 data)
{
	machine().debug_break();
	static const char *const names[] = {
		"data", "mark", "crc", "param", "phases", "setup", "mode0", "mode1",
		"?8", "?9", "?a", "?b", "?c", "?d", "?e", "?f"
	};
	switch(offset) {
	case 0x3: case 0xb: {
#if 0
		static const char *const pname[16] = {
			"minct", "mult", "ssl", "sss", "sll", "sls", "rpt", "csls",
			"lsl", "lss", "lll", "lls", "late", "time0", "early", "time1"
		};
#endif
		static const char *const pname[4] = {
			"late", "time0", "early", "time1"
		};
		logerror("param[%s] = %02x\n", pname[m_param_idx], data);
		m_param[m_param_idx] = data;
		m_param_idx = (m_param_idx + 1) & 3;
		break;
	}
	case 0x4: {
		m_phases = data;
		update_phases();
		break;
	}

	case 0x5: case 0xd:
		m_setup = data;
#if 0
		logerror("setup timer=%s tsm=%s %s ecm=%s %s %s 3.5=%s %s\n",
				 m_setup & 0x80 ? "on" : "off",
				 m_setup & 0x40 ? "off" : "on",
				 m_setup & 0x20 ? "ibm" : "apple",
				 m_setup & 0x10 ? "on" : "off",
				 m_setup & 0x08 ? "fclk/2" : "fclk",
				 m_setup & 0x04 ? "gcr" : "mfm",
				 m_setup & 0x02 ? "off" : "on",
				 m_setup & 0x01 ? "hdsel" : "q3");
#endif
		logerror("setup timer=%s tsm=%s %s ecm=%s %s %s 3.5=%s %s\n",
				 m_setup & 0x80 ? "on" : "off",
				 m_setup & 0x40 ? "off" : "on",
				 m_setup & 0x20 ? "ibm" : "apple",
				 m_setup & 0x10 ? "on" : "off",
				 m_setup & 0x08 ? "fclk/2" : "fclk",
				 m_setup & 0x04 ? "gcr" : "mfm",
				 m_setup & 0x02 ? "off" : "on",
				 m_setup & 0x01 ? "hdsel" : "q3");
		break;

	case 0x6:
		m_mode &= ~data;
		m_mode |= 0x40;
		m_param_idx = 0;
		show_mode();
		break;

	case 0x7:
		m_mode |= data;
		show_mode();
		break;

	default:
		logerror("write %s, %02x\n", names[offset], data);
		break;
	}
}

