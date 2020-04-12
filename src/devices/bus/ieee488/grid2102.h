// license:BSD-3-Clause
// copyright-holders:usernameak
/**********************************************************************

    GRiD 2102 Portable Floppy emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_GRID2102_H
#define MAME_BUS_IEEE488_GRID2102_H

#pragma once

#include "ieee488.h"

#include <vector>
#include <queue>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> grid2102_device
class grid2102_device :  public device_t,
						public device_ieee488_interface,
						public device_image_interface
{
public:
	// construction/destruction
	grid2102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_ieee488_interface overrides
	virtual void ieee488_eoi(int state) override;
	virtual void ieee488_dav(int state) override;
	virtual void ieee488_nrfd(int state) override;
	virtual void ieee488_ndac(int state) override;
	virtual void ieee488_ifc(int state) override;
	virtual void ieee488_srq(int state) override;
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ren(int state) override;

	// image-level overrides
	virtual iodevice_t image_type() const noexcept override { return IO_FLOPPY; }

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "img"; }

	void accept_transfer();
	void update_ndac(int atn);
private:
	int m_gpib_loop_state;
	int m_floppy_loop_state;
	uint8_t m_last_recv_byte;
	int m_last_recv_eoi;
	int m_last_recv_atn;
	uint8_t m_byte_to_send;
	int m_send_eoi;
	bool listening, talking, serial_polling;
	bool has_srq;
	uint8_t serial_poll_byte;
	uint32_t floppy_sector_number;
	std::vector<uint8_t> m_data_buffer;
	std::queue<uint8_t> m_output_data_buffer;
	emu_timer *m_delay_timer;
};

// device type definition
DECLARE_DEVICE_TYPE(GRID2102, grid2102_device)

#endif // MAME_BUS_IEEE488_GRID2102_H
