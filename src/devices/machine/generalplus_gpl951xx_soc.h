// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_GENERALPLUS_GPL951XX_SOC_H
#define MAME_MACHINE_GENERALPLUS_GPL951XX_SOC_H

#pragma once


#include "cpu/unsp/unsp.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "screen.h"
#include "emupal.h"

#include "generalplus_gpl_chx.h"
#include "generalplus_gpl_dma.h"
#include "generalplus_gpl_timebase.h"
#include "generalplus_gpl162xx_soc_video.h"
#include "generalplus_gpl951xx_rtc.h"
#include "spg2xx_audio.h"


class generalplus_gpl951xx_device : public unsp_20_device, public device_mixer_interface
{
public:
	template <typename T>
	generalplus_gpl951xx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag) :
		generalplus_gpl951xx_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
	}

	generalplus_gpl951xx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_spi_romregion(u8 *region, u32 size) { m_spiregion = region; m_spisize = size; }

	auto spi_out() { return m_spi_out.bind(); }
	auto spi_out_cmd() { return m_spi_out_cmd.bind(); }
	auto spi_reset() { return m_spi_reset.bind(); }

	auto i80_cmd_out() { return m_i80_cmd_out.bind(); }
	auto i80_data_out() { return m_i80_data_out.bind(); }

	auto porta_in() { return m_port_in[0].bind(); }
	auto portb_in() { return m_port_in[1].bind(); }
	auto portc_in() { return m_port_in[2].bind(); }
	auto portd_in() { return m_port_in[3].bind(); }
	auto porte_in() { return m_port_in[4].bind(); }
	auto portf_in() { return m_port_in[5].bind(); }

	auto porta_out() { return m_port_out[0].bind(); }
	auto portb_out() { return m_port_out[1].bind(); }
	auto portc_out() { return m_port_out[2].bind(); }
	auto portd_out() { return m_port_out[3].bind(); }
	auto porte_out() { return m_port_out[4].bind(); }
	auto portf_out() { return m_port_out[5].bind(); }

	auto adc0_in() { return m_adc_in[0].bind(); }
	auto adc1_in() { return m_adc_in[1].bind(); }
	auto adc2_in() { return m_adc_in[2].bind(); }
	auto adc3_in() { return m_adc_in[3].bind(); }
	auto adc4_in() { return m_adc_in[4].bind(); }
	auto adc5_in() { return m_adc_in[5].bind(); }

	void recieve_spi_fifo_data(u8 data);

	IRQ_CALLBACK_MEMBER(irq_vector_cb);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return m_spg_video->screen_update(screen, bitmap, cliprect); }
	void vblank(int state) { m_spg_video->vblank(state); }

protected:
	generalplus_gpl951xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void gpspi_direct_internal_map(address_map &map) ATTR_COLD;
	template<int Port> void add_port(address_map& map, u32 base) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// SPIFC interface
	u16 spifc_ctrl_r();
	void spifc_ctrl_w(u16 data);
	u16 spifc_cmd_r();
	void spifc_cmd_w(u16 data);
	u16 spifc_para_r();
	void spifc_para_w(u16 data);
	u16 spifc_addrl_r();
	void spifc_addrl_w(u16 data);
	u16 spifc_addrh_r();
	void spifc_addrh_w(u16 data);
	u16 spifc_txdat_r();
	void spifc_txdat_w(u16 data);
	u16 spifc_rxdat_r();
	void spifc_rxdat_w(u16 data);
	u16 spifc_tx_bc_r();
	void spifc_tx_bc_w(u16 data);
	u16 spifc_rx_bc_r();
	void spifc_rx_bc_w(u16 data);
	u16 spifc_timing_r();
	void spifc_timing_w(u16 data);
	u16 spifc_ctrl2_r();
	void spifc_ctrl2_w(u16 data);

	u16 spi_improve_r();
	void spi_improve_w(u16 data);

	void pm_ctrl_w(u16 data);

	u16 tft_status_r();
	void tft_ctrl_w(u16 data);
	void tft_memmode_wcmd_w(u16 data);

	u16 pllsel_r();
	void pllsel_w(u16 data);

	// Byte swap etc.
	u16 byte_swap_r();
	void byte_swap_w(u16 data);

	// Timers (different compared to GPL162xx)
	template<int Timer> u16 timer_preload_r();
	template<int Timer> void timer_preload_w(u16 data);
	template<int Timer> u16 timer_ctrl_r();
	template<int Timer> void timer_ctrl_w(u16 data);
	template<int Timer> void timer_ccpb_ctrl_w(u16 data);
	template<int Timer> u16 timer_upcount_r();

	u16 i2c_ctrl_r();
	u16 i2c_status_r();

	void dac_0_w(uint16_t data);
	void dac_1_w(uint16_t data);

	template<int Port> u16 io_data_r();
	template<int Port> void io_data_w(u16 data);
	template<int Port> u16 io_buffer_r();
	template<int Port> void io_buffer_w(u16 data);
	template<int Port> u16 io_dir_r();
	template<int Port> void io_dir_w(u16 data);
	template<int Port> u16 io_attrib_r();
	template<int Port> void io_attrib_w(u16 data);
	template<int Port> u16 io_drv_r();
	template<int Port> void io_drv_w(u16 data);
	template<int Port> u16 io_mux_r();
	template<int Port> void io_mux_w(u16 data);
	template<int Port> u16 io_latch_r();
	template<int Port> void io_latch_w(u16 data);
	template<int Port> u16 io_keyen_r();
	template<int Port> void io_keyen_w(u16 data);

	u16 spi_bank_r();
	void spi_bank_w(u16 data);

	u16 spi_direct_r(offs_t offset);
	u16 spi_direct_bank_r(offs_t offset);

	u8 get_byte_from_rx_fifo();

	u16 sys_ctrl_r();
	void sys_ctrl_w(u16 data);

	void clock_ctrl_w(u16 data);
	u16 clk_ctrl0_r();
	void clk_ctrl0_w(u16 data);

	u16 power_state_r();
	void watchdog_ctrl_w(u16 data);

	u16 pllclkwait_r();
	void pllclkwait_w(u16 data);

	u16 cache_ctrl_r();
	void cache_ctrl_w(u16 data);

	void int_status1_w(u16 data);
	void int_status2_w(u16 data);
	void int_status3_w(u16 data);
	u16 int_status1_r();
	u16 int_status2_r();
	u16 int_status3_r();
	void int_priority_1_w(u16 data);
	void int_priority_2_w(u16 data);
	void int_priority_3_w(u16 data);
	u16 int_priority_1_r();
	u16 int_priority_2_r();
	u16 int_priority_3_r();

	void mint_ctrl_w(u16 data);

	void update_interrupts(int state);

	void audioirq_w(int state);
	void videoirq_w(int state);

	u16 madc_ctrl_r();
	void madc_ctrl_w(u16 data);
	u16 madc_data_r();

	u16 tft_rgb_ctrl_r();
	void tft_rgb_ctrl_w(u16 data);

	void tft_v_width_w(u16 data);
	void tft_vsync_setup_w(u16 data);
	void tft_v_start_w(u16 data);
	void tft_v_end_w(u16 data);
	void tft_h_width_w(u16 data);
	void tft_hsync_setup_w(u16 data);
	void tft_h_start_w(u16 data);
	void tft_h_end_w(u16 data);
	void tft_v_show_start_w(u16 data);
	void tft_v_show_end_w(u16 data);
	void tft_h_show_start_w(u16 data);
	void tft_h_show_end_w(u16 data);
	void free_height_w(u16 data);
	void free_width_w(u16 data);

	inline u16 read_space(offs_t offset);
	inline void write_space(offs_t offset, u16 data);

	template<int Timer> TIMER_DEVICE_CALLBACK_MEMBER(timer_cb);

	TIMER_DEVICE_CALLBACK_MEMBER(adc_timer_cb);

	u16 m_byteswap;

	u16 m_timer_preload[8];
	u16 m_timer_ctrl[8];

	u16 m_spifc_ctrl;
	u16 m_spifc_ctrl2;
	u32 m_spifc_addr;
	u16 m_spifc_cmd;
	u16 m_spifc_para;
	u16 m_spifc_rx_bc;
	u16 m_spifc_tx_bc;
	u16 m_spifc_timing;
	u8 m_bytes_in_spifc_rx_fifo;
	u8 m_spifc_rx_fifo[16 * 2];
	u16 m_spifc_rx_read_latch;

	u16 m_io_dir[6];
	u16 m_io_attrib[6];

	u16 m_sys_ctrl;
	u16 m_clock_ctrl;
	u16 m_cache_ctrl;

	u16 m_int_priority_1;
	u16 m_int_priority_2;
	u16 m_int_priority_3;

	u16 m_misc_int_ctrl;

	u16 m_pllchange;

	u16 m_spi_bank;

	u16 m_memmode_wcmd;

	u16 m_tft_rgb_ctrl;

	u16 m_madc_ctrl;
	u16 m_madc_data;

	// config
	u8 *m_spiregion;
	u32 m_spisize;

	devcb_write8 m_spi_out;
	devcb_write8 m_spi_out_cmd;
	devcb_write8 m_spi_reset;

	devcb_write16 m_i80_cmd_out;
	devcb_write16 m_i80_data_out;

	devcb_read16::array<6> m_port_in;
	devcb_write16::array<6> m_port_out;

	devcb_read16::array<6> m_adc_in;

	// devices
	required_device_array<timer_device, 8> m_timer;
	required_device<timer_device> m_adc_timer;
	required_device<gpl951xx_rtc_device> m_rtc;
	required_device<gpl_chx_device> m_gpl_chx;
	required_device<dac_16bit_r2r_twos_complement_device> m_dac0;
	required_device<dac_16bit_r2r_twos_complement_device> m_dac1;
	required_device<gpl_dma_device> m_gpl_dma;
	required_device<gpl_timebase_device> m_gpl_timebase;
	required_device<screen_device> m_screen;
	required_device<gcm394_video_device> m_spg_video;
	required_device<sunplus_gcm394_audio_device> m_spg_audio;
	required_shared_ptr<u16> m_mainram;
};

DECLARE_DEVICE_TYPE(GPL951XX, generalplus_gpl951xx_device)

#endif // MAME_MACHINE_GENERALPLUS_GPL951XX_SOC_H
