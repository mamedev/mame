// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_GENERALPLUS_GPCE4_SOC_H
#define MAME_MACHINE_GENERALPLUS_GPCE4_SOC_H

#pragma once

#include "cpu/unsp/unsp.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "speaker.h"

class generalplus_gpce4_soc_device : public unsp_20_device
{
public:
	generalplus_gpce4_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto porta_in() { return m_porta_in.bind(); }
	auto portb_in() { return m_portb_in.bind(); }
	auto portc_in() { return m_portc_in.bind(); }

	auto porta_out() { return m_porta_out.bind(); }
	auto portb_out() { return m_portb_out.bind(); }
	auto portc_out() { return m_portc_out.bind(); }

	auto spi2_out() { return m_spi2_out.bind(); }
	auto spi_out() { return m_spi_out.bind(); }
	auto spi_reset() { return m_spi_reset.bind(); }

	void request_interrupt(int which);
	void clear_interrupt(int which);

	void recieve_spi_fifo_data(u8 data);
	u8 *get_spi_romregion();
	void set_spi_romregion(u8 *region, u32 size) { m_spiregion = region; m_spisize = size; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config& config) override ATTR_COLD;

private:
	void internal_map(address_map &map);

	u16 ioa_data_r();
	void ioa_data_w(u16 data);
	u16 ioa_buffer_r();
	void ioa_buffer_w(u16 data);
	u16 ioa_direction_r();
	void ioa_direction_w(u16 data);
	u16 ioa_attribute_r();
	void ioa_attribute_w(u16 data);

	u16 iob_data_r();
	void iob_data_w(u16 data);
	u16 iob_buffer_r();
	void iob_buffer_w(u16 data);
	u16 iob_direction_r();
	void iob_direction_w(u16 data);
	u16 iob_attribute_r();
	void iob_attribute_w(u16 data);

	u16 ioc_data_r();
	void ioc_data_w(u16 data);
	u16 ioc_buffer_r();
	void ioc_buffer_w(u16 data);
	u16 ioc_direction_r();
	void ioc_direction_w(u16 data);
	u16 ioc_attribute_r();
	void ioc_attribute_w(u16 data);

	u16 io_ctrl_r();
	void io_ctrl_w(u16 data);

	u16 timer_ctrl_r();
	void timer_ctrl_w(u16 data);

	void timera_data_w(u16 data);
	void timera_counter_w(u16 data);
	void timerb_data_w(u16 data);
	void timerb_counter_w(u16 data);
	void timerc_data_w(u16 data);
	void timerc_counter_w(u16 data);

	u16 pwm0_ctrl_r();
	u16 pwm1_ctrl_r();
	u16 pwm2_ctrl_r();
	u16 pwm3_ctrl_r();

	void pwm0_ctrl_w(u16 data);
	void pwm1_ctrl_w(u16 data);
	void pwm2_ctrl_w(u16 data);
	void pwm3_ctrl_w(u16 data);

	void system_clock_w(u16 data);

	void timebase_clear_w(u16 data);
	void watchdog_clear_w(u16 data);
	void dac_ctrl_w(u16 data);
	void dac_cha1_data_w(u16 data);
	void dac_cha2_data_w(u16 data);
	void ppam_ctrl_w(u16 data);

	void wait_ctrl_w(u16 data);
	u16 cache_ctrl_r();
	void cache_ctrl_w(u16 data);

	u16 interrupt_ctrl_r();
	void interrupt_ctrl_w(u16 data);
	u16 interrupt_status_r();
	void interrupt_status_w(u16 data);
	u16 fiq_sel_r();
	void fiq_sel_w(u16 data);
	u16 fiq2_sel_r();
	void fiq2_sel_w(u16 data);
	u16 interrupt2_ctrl_r();
	void interrupt2_ctrl_w(u16 data);
	u16 interrupt2_status_r();
	void interrupt2_status_w(u16 data);

	u16 spi2_ctrl_r();
	void spi2_ctrl_w(u16 data);
	u16 spi2_txstatus_r();
	void spi2_txstatus_w(u16 data);
	void spi2_txdata_w(u16 data);
	u16 spi2_rxstatus_r();
	void spi2_rxstatus_w(u16 data);
	u16 spi2_rxdata_r();
	u16 spi2_misc_r();
	void spi2_misc_w(u16 data);

	u16 spi_man_ctrl_r();
	void spi_man_ctrl_w(u16 data);
	u16 spi_ctrl_r();
	void spi_ctrl_w(u16 data);
	void spi_txstatus_w(u16 data);
	void spi_txdata_w(u16 data);
	u16 spi_rxstatus_r();
	void spi_rxstatus_w(u16 data);
	u16 spi_rxdata_r();
	u16 spi_misc_r();
	void spi_misc_w(u16 data);
	u16 spi_auto_ctrl_r();
	void spi_auto_ctrl_w(u16 data);
	u16 spi_bank_r();
	void spi_bank_w(u16 data);

	u16 adc_data_r();
	void adc_data_w(u16 data);
	u16 adc_ctrl_r();
	void adc_ctrl_w(u16 data);
	u16 adc_linein_bitctrl_r();
	void adc_linein_bitctrl_w(u16 data);

	u16 spi_direct_r(offs_t offset);

	void update_interrupts();

	void write_to_tx_fifo(u8 data);
	void reset_spi_fifos();

	TIMER_DEVICE_CALLBACK_MEMBER(timer_c_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_a_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_2hz_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_64hz_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_2khz_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_spi_tx);

	u8 *m_spiregion;
	u32 m_spisize;

	u16 m_ioa_buffer;
	u16 m_iob_buffer;
	u16 m_interrupt_ctrl;
	u16 m_interrupt2_ctrl;
	u16 m_timer_ctrl;
	u16 m_io_ctrl;
	u16 m_ioa_attribute;
	u16 m_iob_attribute;
	u16 m_ioc_attribute;
	u16 m_ioa_direction;
	u16 m_iob_direction;
	u16 m_ioc_direction;
	u16 m_cache_ctrl;
	u16 m_spi_auto_ctrl;
	u16 m_spi_bank;
	u16 m_fiq_sel;
	u16 m_fiq2_sel;
	u16 m_spi2_ctrl;
	u16 m_spi_ctrl;

	u16 m_pwm0_ctrl;
	u16 m_pwm1_ctrl;
	u16 m_pwm2_ctrl;
	u16 m_pwm3_ctrl;

	u32 m_interrupt_status;

	u8 m_spi_rx_fifo[8];
	u8 m_spi_tx_fifo[8];
	u8 m_rx_fifo_entries;
	u8 m_tx_fifo_entries;

	devcb_read16 m_porta_in;
	devcb_read16 m_portb_in;
	devcb_read16 m_portc_in;

	devcb_write16 m_porta_out;
	devcb_write16 m_portb_out;
	devcb_write16 m_portc_out;

	devcb_write8 m_spi2_out;
	devcb_write8 m_spi_out;
	devcb_write8 m_spi_reset;

	required_device<dac_word_device_base> m_dac;
	required_device<timer_device> m_spi_tx_timer;
};

DECLARE_DEVICE_TYPE(GPCE4, generalplus_gpce4_soc_device)

#endif // MAME_MACHINE_GENERALPLUS_GPCE4_SOC_H
