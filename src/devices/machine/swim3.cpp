// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Implementation of the Apple SWIM3 floppy disk controller

*********************************************************************/

#include "emu.h"
#include "swim3.h"

DEFINE_DEVICE_TYPE(SWIM3, swim3_device, "swim3", "Apple SWIM3 (Sander/Wozniak Integrated Machine) version 3 floppy controller")

swim3_device::swim3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	applefdintf_device(mconfig, SWIM3, tag, owner, clock)
{
}

void swim3_device::device_start()
{
	applefdintf_device::device_start();
	save_item(NAME(m_mode));
	save_item(NAME(m_setup));
	save_item(NAME(m_param_idx));
	save_item(NAME(m_param));
}

void swim3_device::device_reset()
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

void swim3_device::device_timer(emu_timer &, device_timer_id, int, void *)
{
}

void swim3_device::set_floppy(floppy_image_device *floppy)
{
	if(m_floppy == floppy)
		return;

	logerror("floppy %s\n", floppy ? floppy->tag() : "-");

	m_floppy = floppy;
	update_phases();
	m_hdsel_cb((m_mode >> 5) & 1);
}

floppy_image_device *swim3_device::get_floppy() const
{
	return m_floppy;
}

void swim3_device::show_mode() const
{
	logerror("mode%s%s hdsel=%c %c%s %c%c%s\n",
			 m_mode & 0x80 ? " step" : "",
			 m_mode & 0x40 ? " format" : "",
			 m_mode & 0x20 ? '1' : '0',
			 m_mode & 0x10 ? 'w' : 'r',
			 m_mode & 0x08 ? " go" : "",
			 m_mode & 0x04 ? 'b' : '-',
			 m_mode & 0x02 ? 'a' : '-',
			 m_mode & 0x01 ? " irq" : "");

}

u8 swim3_device::read(offs_t offset)
{
	static const char *const names[] = {
		"data", "timer", "error", "param", "phases", "setup", "?6", "handshake",
		"interrupt", "step", "track", "sector", "gap", "sect1", "xfer", "imask"
	};
	switch(offset) {
	case 0x4: // phases
		return m_phases & 0xf;

	case 0x5: // setup
		return m_setup;

	case 0x6: // mode
		return m_mode;

	case 0x7: { // handshake
		u8 h = 0;
		if(!m_floppy || m_floppy->wpt_r())
			h |= 0x08;
		logerror("hand %02x\n", h);
		return h;
	};

	default:
		logerror("read %s\n", names[offset & 15]);
		break;
	}
	return 0xff;
}

void swim3_device::write(offs_t offset, u8 data)
{
	u8 prev_mode = m_mode;

	static const char *const names[] = {
		"data", "timer", "error", "param", "phases", "setup", "mode0", "mode1",
		"?8", "step", "track", "sector", "gap", "sect1", "xfer", "imask"
	};
	switch(offset) {
	case 4: { // phases
		m_phases = data | 0xf0;
		logerror("phases %x\n", data);
		update_phases();
		break;
	}

	case 5: // setup
		m_setup = data;
		m_sel35_cb((m_setup >> 1) & 1);
		logerror("setup write=%s %s nogcrconv=%s %s %s%s %s\n",
				 m_setup & 0x40 ? "gcr" : "mfm",
				 m_setup & 0x20 ? "ibm" : "apple",
				 m_setup & 0x10 ? "on" : "off",
				 m_setup & 0x08 ? "fclk/2" : "fclk",
				 m_setup & 0x04 ? "gcr" : "mfm",
				 m_setup & 0x02 ? " copy" : "",
				 m_setup & 0x01 ? "wrinvert" : "wrdirect");
		break;

	case 6: // mode clear
		m_mode &= ~data;
		m_mode |= 0x40;
		m_param_idx = 0;
		show_mode();
		break;

	case 7: // mode set
		m_mode |= data;
		show_mode();
		break;

	default:
		logerror("write %s, %02x\n", names[offset], data);
		break;
	}

	if((m_mode ^ prev_mode) & 0x06)
		m_devsel_cb((m_mode >> 1) & 3);
	if((m_mode ^ prev_mode) & 0x20)
		m_hdsel_cb((m_mode >> 5) & 1);

}

void swim3_device::sync()
{
}
