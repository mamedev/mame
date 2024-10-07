// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Adaptec AIC-580

**********************************************************************/

#ifndef MAME_MACHINE_AIC580_H
#define MAME_MACHINE_AIC580_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> aic580_device

class aic580_device : public device_t
{
public:
	// construction/destruction
	aic580_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto bdin_callback() { return m_bdin_callback.bind(); }
	auto bdout_callback() { return m_bdout_callback.bind(); }
	auto back_callback() { return m_back_callback.bind(); }
	auto sread_callback() { return m_sread_callback.bind(); }
	auto swrite_callback() { return m_swrite_callback.bind(); }

	// microprocessor interface
	void mpu_map(address_map &map) ATTR_COLD;

	// port B DMA interface
	void breq_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// register handlers
	u8 r80_r();
	void transfer_speed_w(u8 data);
	void dma_mode_w(u8 data);
	void r83_w(u8 data);
	void r86_w(u8 data);
	u8 r88_r();
	void r88_w(u8 data);
	void r8a_w(u8 data);
	void r8b_w(u8 data);
	void bus_on_time_w(u8 data);
	void bus_off_time_w(u8 data);
	template<int Channel> void ch_addrl_w(u8 data);
	template<int Channel> void ch_addrm_w(u8 data);
	template<int Channel> void ch_addrh_w(u8 data);
	void ra2_w(u8 data);
	void ra3_w(u8 data);
	void ra4_w(u8 data);
	u8 fifo_data_r();
	void fifo_data_w(u8 data);
	u8 buffer_r(offs_t offset);
	void buffer_w(offs_t offset, u8 data);

	// callback objects
	devcb_read8 m_bdin_callback;
	devcb_write8 m_bdout_callback;
	devcb_write_line m_back_callback;
	devcb_read16 m_sread_callback;
	devcb_write16 m_swrite_callback;

	// internal state
	u8 m_dma_mode;
	u32 m_channel_addr[2];
	u8 m_fifo_read_index;
	u8 m_fifo_write_index;
	u8 m_fifo_data[16];
};

// device type declaration
DECLARE_DEVICE_TYPE(AIC580, aic580_device)

#endif // MAME_MACHINE_AIC580_H
