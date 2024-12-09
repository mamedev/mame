// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  SigmaSoft Universal Parallel Interface Board


****************************************************************************/

#include "emu.h"

#include "bus/heathzenith/h19/tlb.h"

#include "sigmasoft_parallel_port.h"

//
// Logging defines
//
#define LOG_REG  (1U << 1)   // Shows register setup
#define LOG_FUNC (1U << 2)   // Function calls

#define VERBOSE (0)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

// TODO make this configurable with jumper config
static constexpr u8 BASE_ADDR = 0x08;

class sigmasoft_parallel_port : public device_t, public device_h89bus_left_card_interface
{
public:
	sigmasoft_parallel_port(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual u8 read(u8 select_lines, u16 offset) override;
	virtual void write(u8 select_lines, u16 offset, u8 data) override;

	auto ctrl_r_cb() { return m_ctrl_r.bind(); }
	auto video_mem_r_cb() { return m_video_mem_r.bind(); }

	auto video_mem_cb() { return m_video_mem_w.bind(); }
	auto io_lo_cb() { return m_io_lo_addr.bind(); }
	auto io_hi_cb() { return m_io_hi_addr.bind(); }
	auto window_lo_cb() { return m_window_lo_addr.bind(); }
	auto window_hi_cb() { return m_window_hi_addr.bind(); }
	auto ctrl_cb() { return m_ctrl_w.bind(); }

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config& config) override  ATTR_COLD;;

	u8 video_mem_r();
	void video_mem_w(u8 val);

	void io_lo_addr_w(u8 val);
	void io_hi_addr_w(u8 val);

	void window_lo_addr_w(u8 val);
	void window_hi_addr_w(u8 val);

	void ctrl_w(u8 val);
	u8 ctrl_r();

private:

	// Reads
	devcb_read8  m_ctrl_r;
	devcb_read8  m_video_mem_r;

	// Writes
	devcb_write8 m_video_mem_w;
	devcb_write8 m_io_lo_addr;
	devcb_write8 m_io_hi_addr;
	devcb_write8 m_window_lo_addr;
	devcb_write8 m_window_hi_addr;
	devcb_write8 m_ctrl_w;
};


sigmasoft_parallel_port::sigmasoft_parallel_port(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, H89BUS_SIGMASOFT_PARALLEL, tag, owner, clock),
	device_h89bus_left_card_interface(mconfig, *this),
	m_ctrl_r(*this, 0x00),
	m_video_mem_r(*this, 0x00),
	m_video_mem_w(*this),
	m_io_lo_addr(*this),
	m_io_hi_addr(*this),
	m_window_lo_addr(*this),
	m_window_hi_addr(*this),
	m_ctrl_w(*this)
{
}

void sigmasoft_parallel_port::video_mem_w(u8 val)
{
	m_video_mem_w(val);
}

void sigmasoft_parallel_port::io_lo_addr_w(u8 val)
{
	m_io_lo_addr(val);
}

void sigmasoft_parallel_port::io_hi_addr_w(u8 val)
{
	m_io_hi_addr(val);
}

void sigmasoft_parallel_port::window_lo_addr_w(u8 val)
{
	m_window_lo_addr(val);
}

void sigmasoft_parallel_port::window_hi_addr_w(u8 val)
{
	m_window_hi_addr(val);
}

void sigmasoft_parallel_port::ctrl_w(u8 val)
{
	m_ctrl_w(val);
}

void sigmasoft_parallel_port::write(u8 select_lines, u16 offset, u8 data)
{
	offset -= BASE_ADDR;
	if (!(select_lines & h89bus_device::H89_IO) || offset >= 8)
	{
		return;
	}

	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, offset, data);

	switch (offset)
	{
	case 0:
		video_mem_w(data);
		break;
	case 1:
		io_lo_addr_w(data);
		break;
	case 2:
		io_hi_addr_w(data);
		break;
	case 3:
		window_lo_addr_w(data);
		break;
	case 4:
		window_hi_addr_w(data);
		break;
	case 5:
		ctrl_w(data);
		break;
	case 6:
		// TODO - Centronics interface
		break;
	case 7:
		// TODO - Centronics interface
		break;
	}
}

u8 sigmasoft_parallel_port::video_mem_r()
{
	// get video memory value from igc device
	return m_video_mem_r();
}

u8 sigmasoft_parallel_port::ctrl_r()
{
	// get control register from igc device
	return m_ctrl_r();
}

u8 sigmasoft_parallel_port::read(u8 select_lines, u16 offset)
{
	offset -= BASE_ADDR;
	u8 value = 0x00;

	if (!(select_lines & h89bus_device::H89_IO) || offset >= 8)
	{
		return value;
	}

	switch (offset)
	{
	case 0:
		value = video_mem_r();
		break;
	case 1:
		// TODO - Light Pen Low address
		break;
	case 2:
		// TODO - Light Pen High address
		break;
	case 3:
		// TODO - Left input device
		break;
	case 4:
		// TODO - Right input device
		break;
	case 5:
		// Control Register
		value = ctrl_r();
		break;
	case 6:
		// TODO - Centronics interface
		break;
	case 7:
		// TODO - Centronics interface
		break;
	}

	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, offset, value);

	return value;
}

void sigmasoft_parallel_port::device_start()
{
}

void sigmasoft_parallel_port::device_add_mconfig(machine_config &config)
{
	// connect callbacks to TLB
	ctrl_r_cb().set("^^tlbc", FUNC(heath_tlb_connector::sigma_ctrl_r));
	video_mem_r_cb().set("^^tlbc", FUNC(heath_tlb_connector::sigma_video_mem_r));
	video_mem_cb().set("^^tlbc", FUNC(heath_tlb_connector::sigma_video_mem_w));
	io_lo_cb().set("^^tlbc", FUNC(heath_tlb_connector::sigma_io_lo_addr_w));
	io_hi_cb().set("^^tlbc", FUNC(heath_tlb_connector::sigma_io_hi_addr_w));
	window_lo_cb().set("^^tlbc", FUNC(heath_tlb_connector::sigma_window_lo_addr_w));
	window_hi_cb().set("^^tlbc", FUNC(heath_tlb_connector::sigma_window_hi_addr_w));
	ctrl_cb().set("^^tlbc", FUNC(heath_tlb_connector::sigma_ctrl_w));
}

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_SIGMASOFT_PARALLEL, device_h89bus_left_card_interface, sigmasoft_parallel_port, "sigmasoft_parallel_port", "SigmaSoft Universal Parallel Board");
