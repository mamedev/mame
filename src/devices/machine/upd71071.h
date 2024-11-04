// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#ifndef MAME_MACHINE_UPD71071_H
#define MAME_MACHINE_UPD71071_H

#pragma once


class upd71071_device : public device_t
{
public:
	upd71071_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_cpu_tag(const char *tag) { m_cpu.set_tag(tag); }
	void set_clock(int clock) { m_upd_clock = clock; }

	auto out_hreq_callback() { return m_out_hreq_cb.bind(); }
	auto out_eop_callback() { return m_out_eop_cb.bind(); }

	template <unsigned N> auto dma_read_callback() { return m_dma_read_cb[N].bind(); }
	template <unsigned N> auto dma_write_callback() { return m_dma_write_cb[N].bind(); }
	template <unsigned N> auto out_dack_callback() { return m_out_dack_cb[N].bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void set_hreq(int state);
	void set_eop(int state);

	int dmarq(int state, int channel);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	struct upd71071_reg
	{
		uint8_t initialise;
		uint8_t channel;
		uint16_t count_current[4];
		uint16_t count_base[4];
		uint32_t address_current[4];
		uint32_t address_base[4];
		uint16_t device_control;
		uint8_t mode_control[4];
		uint8_t status;
		uint8_t temp_l;
		uint8_t temp_h;
		uint8_t request;
		uint8_t mask;
	};

	void soft_reset();
	TIMER_CALLBACK_MEMBER(dma_transfer_timer);

	// internal state
	upd71071_reg m_reg;
	int m_selected_channel;
	int m_buswidth;
	int m_dmarq[4];
	emu_timer* m_timer[4];
	//int m_in_progress[4];
	//int m_transfer_size[4];
	int m_base;
	int m_upd_clock;
	devcb_write_line m_out_hreq_cb;
	devcb_write_line m_out_eop_cb;
	devcb_read16::array<4> m_dma_read_cb;
	devcb_write16::array<4> m_dma_write_cb;
	devcb_write_line::array<4> m_out_dack_cb;
	int m_hreq;
	int m_eop;
	optional_device<cpu_device> m_cpu;
};

DECLARE_DEVICE_TYPE(UPD71071, upd71071_device)


#endif // MAME_MACHINE_UPD71071_H
