// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC2

******************************************************************************/

#ifndef MAME_MACHINE_PSION_ASIC2_H
#define MAME_MACHINE_PSION_ASIC2_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_asic2_device

class psion_asic2_device : public device_t
{
public:
	// construction/destruction
	psion_asic2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto int_cb() { return m_int_cb.bind(); }
	auto nmi_cb() { return m_nmi_cb.bind(); }
	auto cbusy_cb() { return m_cbusy_cb.bind(); }
	auto buz_cb() { return m_buz_cb.bind(); }
	auto buzvol_cb() { return m_buzvol_cb.bind(); }
	auto dr_cb() { return m_dr_cb.bind(); }
	auto col_cb() { return m_col_cb.bind(); }
	auto read_pd_cb() { return m_read_pd_cb.bind(); }
	auto write_pd_cb() { return m_write_pd_cb.bind(); }

	template <unsigned N> auto data_r() { static_assert(N < 8); return m_data_r[N].bind(); }
	template <unsigned N> auto data_w() { static_assert(N < 8); return m_data_w[N].bind(); }

	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	void on_clr_w(int state);
	void sds_int_w(int state);
	void dnmi_w(int state);
	void frcovl_w(int state);
	void reset_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void update_interrupts();

	uint8_t m_a2_index;
	uint8_t m_a2_icontrol0;
	uint8_t m_a2_icontrol1;
	uint8_t m_a2_iddr;
	uint8_t m_a2_control1;
	uint8_t m_a2_control2;
	uint8_t m_a2_control3;
	uint8_t m_a2_serial_data;
	uint8_t m_a2_serial_control;
	uint8_t m_a2_interrupt_status;
	uint8_t m_a2_status;
	uint8_t m_a2_channel_control;

	devcb_write_line m_int_cb;
	devcb_write_line m_nmi_cb;
	devcb_write_line m_cbusy_cb;
	devcb_write_line m_buz_cb;
	devcb_write_line m_buzvol_cb;
	devcb_write_line m_dr_cb;
	devcb_read8 m_col_cb;
	devcb_read8 m_read_pd_cb;
	devcb_write8 m_write_pd_cb;

	devcb_read8::array<8> m_data_r;
	devcb_write16::array<8> m_data_w;

	emu_timer *m_busy_timer;

	TIMER_CALLBACK_MEMBER(busy);

	bool channel_active(int channel);
	void transmit_frame(uint16_t data);
	uint8_t receive_frame();

	static constexpr uint16_t NULL_FRAME    = 0x000;
	static constexpr uint16_t CONTROL_FRAME = 0x100;
	static constexpr uint16_t DATA_FRAME    = 0x200;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_ASIC2, psion_asic2_device)

#endif // MAME_MACHINE_PSION_ASIC2_H
