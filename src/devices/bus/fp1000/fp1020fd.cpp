// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "fp1020fd.h"

#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(FP1020FD, fp1020fd_device, "fp1020fd", "FP-1020FD FDCPACK")

fp1020fd_device::fp1020fd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fp1060io_exp_device(mconfig, FP1020FD, tag, owner, clock)
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
{
}

void fp1020fd_device::io_map(address_map &map)
{
	map(0x0000, 0x0000).mirror(0xfef9).unmapr().lw8(
		NAME([this](offs_t offset, u8 data) {
			(void)data;
			for (auto floppy : m_floppy)
			{
				floppy_image_device *fl = floppy->get_device();
				fl->mon_w(0);
			}

			m_motor_timer->adjust(attotime::from_seconds(60));
		})
	);
	map(0x0002, 0x0002).mirror(0xfef9).unmapr().lw8(
		NAME([this](offs_t offset, u8 data) {
			(void)data;
			m_fdc->tc_w(true);
			m_fdc->tc_w(false);
		})
	);
	map(0x0004, 0x0004).mirror(0xfef8).r(m_fdc, FUNC(upd765a_device::msr_r));
	map(0x0005, 0x0005).mirror(0xfef8).rw(m_fdc, FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w));
	map(0x0006, 0x0006).mirror(0xfef8).rw(m_fdc, FUNC(upd765a_device::dma_r), FUNC(upd765a_device::dma_w));
	map(0x0007, 0x0007).mirror(0xfef8).unmaprw();
}



static void fd1020fd_floppies(device_slot_interface &device)
{
	device.option_add("525dsdd", FLOPPY_525_DD);
}

void fp1020fd_device::intrq_w(int state)
{
//  LOG("intrq_w %d\n",state);
	fp1060io_exp_device::intb_w(state);
}

void fp1020fd_device::drq_w(int state)
{
//  LOG("drq_w %d\n",state);
	fp1060io_exp_device::inta_w(state);
}

void fp1020fd_device::device_add_mconfig(machine_config &config)
{
	// UPD765AC
	// TODO: verify clock
	// ready and select lines = true verified (pukes any floppy bootstrap if either is false)
	UPD765A(config, m_fdc, 8'000'000, true, true);
	m_fdc->intrq_wr_callback().set(FUNC(fp1020fd_device::intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(fp1020fd_device::drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", fd1020fd_floppies, "525dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", fd1020fd_floppies, "525dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

void fp1020fd_device::device_start()
{
	m_motor_timer = timer_alloc(FUNC(fp1020fd_device::motor_timeout_cb), this);
}

void fp1020fd_device::device_reset()
{
}

TIMER_CALLBACK_MEMBER(fp1020fd_device::motor_timeout_cb)
{
	for (auto floppy : m_floppy)
	{
		floppy_image_device *fl = floppy->get_device();
		fl->mon_w(1);
	}
}
