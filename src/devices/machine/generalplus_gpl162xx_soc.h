// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

  SunPlus "GCM394" (based on die pictures)

**********************************************************************/

#ifndef MAME_MACHINE_GENERALPLUS_GPL162XX_SOC_H
#define MAME_MACHINE_GENERALPLUS_GPL162XX_SOC_H

#pragma once

#include "generalplus_gpl_dma.h"
#include "generalplus_gpl_timebase.h"
#include "generalplus_gpl162xx_soc_video.h"
#include "spg2xx_audio.h"

#include "cpu/unsp/unsp.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"


class sunplus_gcm394_base_device : public unsp_20_device, public device_mixer_interface
{
public:
	sunplus_gcm394_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
		sunplus_gcm394_base_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(sunplus_gcm394_base_device::gcm394_internal_map), this))
	{
	}

	auto porta_in() { return m_porta_in.bind(); }
	auto portb_in() { return m_portb_in.bind(); }
	auto portc_in() { return m_portc_in.bind(); }
	auto portd_in() { return m_portd_in.bind(); }

	auto porta_out() { return m_porta_out.bind(); }
	auto portb_out() { return m_portb_out.bind(); }
	auto portc_out() { return m_portc_out.bind(); }
	auto portd_out() { return m_portd_out.bind(); }

	auto space_read_callback() { return m_space_read_cb.bind(); }
	auto space_write_callback() { return m_space_write_cb.bind(); }
	auto dma_complete_callback() { return m_dma_complete_cb.bind(); }

	auto nand_read_callback() { return m_nand_read_cb.bind(); }

	// hack for beijuehh / bornkidh
	void disable_timebase_interrupts() { m_disable_timebase_interrupts = true; }

	template <typename... T> void set_cs_config_callback(T &&... args) { m_cs_callback.set(std::forward<T>(args)...); }
	template <typename T> void set_cs_space(T &&tag, int no)
	{
		m_cs_space.set_tag(tag, no);
		m_spg_video.lookup()->set_cs_video_space(tag, no, m_csbase);
	}

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return m_spg_video->screen_update(screen, bitmap, cliprect); }

	void vblank(int state) { m_spg_video->vblank(state); }

	void set_bootmode(int mode) { m_boot_mode = mode; }

	IRQ_CALLBACK_MEMBER(irq_vector_cb);
	void default_cs_callback(u16 cs0, u16 cs1, u16 cs2, u16 cs3, u16 cs4 );

	void set_legacy_video_mode() { m_spg_video->set_legacy_video_mode(); }
	void set_disallow_resolution_control() { m_spg_video->set_disallow_resolution_control(); }

	void set_romtype(int romtype) { m_romtype = romtype; }

	u16 get_ram_addr(u32 addr) { return m_mainram[addr]; }

protected:
	using cs_callback_delegate = device_delegate<void (u16, u16, u16, u16, u16)>;

	sunplus_gcm394_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	void gcm394_internal_map(address_map &map) ATTR_COLD;
	void base_internal_map(address_map &map) ATTR_COLD;

	required_device<screen_device> m_screen;
	required_device<gcm394_video_device> m_spg_video;
	required_device<sunplus_gcm394_audio_device> m_spg_audio;
	optional_memory_region m_internalrom;
	required_shared_ptr<u16> m_mainram;

	devcb_read16 m_porta_in;
	devcb_read16 m_portb_in;
	devcb_read16 m_portc_in;
	devcb_read16 m_portd_in;

	devcb_write16 m_porta_out;
	devcb_write16 m_portb_out;
	devcb_write16 m_portc_out;
	devcb_write16 m_portd_out;

	devcb_read16 m_nand_read_cb;
	optional_address_space m_cs_space;
	u32 m_csbase;

	// unk 78xx
	u16 m_sys_ctrl;

	u16 m_clock_ctrl;

	u16 m_membankswitch_7810;

	u16 m_7816;
	u16 m_pllchange;

	u16 m_cache_ctrl;

	u16 m_782x[5];

	u16 m_782d;

	u16 m_7835;

	u16 m_7862_porta_direction;
	u16 m_7863_porta_attribute;

	u16 m_786a_portb_direction;
	u16 m_786b_portb_attribute;

	u16 m_7872_portc_direction;
	u16 m_7873_portc_attribute;

	u16 m_787a_portd_direction;
	u16 m_787b_portd_attribute;

	u16 m_ioc_data;

	u16 m_ioe_dir;
	u16 m_ioe_attrib;

	u16 m_int_status1;

	u16 m_int_priority_1;
	u16 m_int_priority_2;
	u16 m_int_priority_3;

	u16 m_misc_int_ctrl;

	u16 m_cha_ctrl;

	u16 m_dac_pga;

	// unk 79xx
	u16 m_rtc_ctrl;
	u16 m_rtc_int_status;
	u16 m_rtc_int_ctrl;

	u16 m_adc_setup;
	u16 m_madc_ctrl;

	u16 m_timera_ctrl;
	u16 m_timerb_ctrl;


	u16 internalrom_lower32_r(offs_t offset);

	u16 cs_space_r(offs_t offset);
	void cs_space_w(offs_t offset, u16 data);
	u16 cs_bank_space_r(offs_t offset);
	void cs_bank_space_w(offs_t offset, u16 data);
	int m_romtype;

protected:
	devcb_read16 m_space_read_cb;
	devcb_write16 m_space_write_cb;
	devcb_write_line m_dma_complete_cb;

	TIMER_DEVICE_CALLBACK_MEMBER(timer_a_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_b_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_c_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_d_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_e_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_f_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(scheduler_cb);

	u16 unk_r(offs_t offset);
	void unk_w(offs_t offset, u16 data);


	u16 power_state_r();
	u16 dac_pga_r();

	u16 sys_ctrl_r();
	void sys_ctrl_w(u16 data);

	void clock_ctrl_w(u16 data);

	u16 clk_ctrl0_r();
	void clk_ctrl0_w(u16 data);

	void waitmode_enter_780c_w(u16 data);

	u16 membankswitch_7810_r();
	void membankswitch_7810_w(u16 data);

	void unkarea_7816_w(u16 data);
	void pllchange_w(u16 data);

	u16 pllclkwait_r();
	void pllclkwait_w(u16 data);

	void watchdog_ctrl_w(u16 data);

	u16 cache_ctrl_r();
	void cache_ctrl_w(u16 data);

	void chipselect_csx_memory_device_control_w(offs_t offset, u16 data);

	void mcs0_page_w(u16 data);

	u16 raw_war_r();
	void raw_war_w(u16 data);

	// Port A
	u16 ioa_data_r();
	void ioa_data_w(u16 data);
	u16 ioa_buffer_r();
	void ioa_buffer_w(u16 data);
	u16 ioa_dir_r();
	void ioa_dir_w(u16 data);
	u16 ioa_attrib_r();
	void ioa_attrib_w(u16 data);

	// Port B
	u16 iob_data_r();
	void iob_data_w(u16 data);
	u16 iob_buffer_r();
	void iob_buffer_w(u16 data);
	u16 iob_dir_r();
	void iob_dir_w(u16 data);
	u16 iob_attrib_r();
	void iob_attrib_w(u16 data);

	// Port C
	u16 ioc_data_r();
	void ioc_data_w(u16 data);
	u16 ioc_buffer_r();
	void ioc_buffer_w(u16 data);
	u16 ioc_dir_r();
	void ioc_dir_w(u16 data);
	u16 ioc_attrib_r();
	void ioc_attrib_w(u16 data);

	// Port D
	u16 iod_data_r();
	void iod_data_w(u16 data);
	u16 iod_buffer_r();
	void iod_buffer_w(u16 data);
	u16 iod_dir_r();
	void iod_dir_w(u16 data);
	u16 iod_attib_r();
	void iod_attib_w(u16 data);
	u16 iod_drv_r();
	void iod_drv_w(u16 data);
	u16 iod_mux_r();
	void iod_mux_w(u16 data);

	u16 ioe_buffer_r();
	void ioe_buffer_w(u16 data);
	u16 ioe_dir_r();
	void ioe_dir_w(u16 data);
	u16 ioe_attrib_r();
	void ioe_attrib_w(u16 data);

	void int_status1_w(u16 data);
	void int_status2_w(u16 data);
	void int_status3_w(u16 data);

	u16 int_status1_r();
	u16 int_status2_r();
	u16 int_status3_r();

	void int_priority_1_w(u16 data);
	void int_priority_2_w(u16 data);
	void int_priority_3_w(u16 data);

	void mint_ctrl_w(u16 data);

	virtual void update_interrupts(int state);

	u16 timera_ctrl_r();
	void timera_ctrl_w(u16 data);

	u16 timerb_ctrl_r();
	void timerb_ctrl_w(u16 data);

	u16 timerc_ctrl_r();
	u16 timerd_ctrl_r();

	u16 cha_ctrl_r();
	void cha_ctrl_w(u16 data);

	u16 uart_status_r();

	u16 rtc_ctrl_r();
	void rtc_ctrl_w(u16 data);

	u16 rtc_int_status_r();
	void rtc_int_status_w(u16 data);

	u16 rtc_int_ctrl_r();
	void rtc_int_ctrl_w(u16 data);

	u16 spi_7944_rxdata_r();
	u16 spi_7945_misc_control_reg_r();
	void spi_7942_txdata_w(u16 data);

	void adc_setup_w(u16 data);
	u16 madc_ctrl_r();
	void madc_ctrl_w(u16 data);
	u16 madc_data_r();

	void videoirq_w(int state);
	void audioirq_w(int state);

	u16 usb_7a35_r();
	u16 usb_7a37_r();
	u16 usb_7a39_r();
	u16 usb_7a3a_r();
	u16 usb_7a46_r();
	u16 usb_7a54_r();

	inline u16 read_space(offs_t offset);
	inline void write_space(offs_t offset, u16 data);

	// config registers (external pins)
	int m_boot_mode; // 2 pins determine boot mode, likely only read at power-on
	cs_callback_delegate m_cs_callback;

	required_device<timer_device> m_timer_a;
	required_device<timer_device> m_timer_b;
	required_device<timer_device> m_timer_c;
	required_device<timer_device> m_timer_d;
	required_device<timer_device> m_timer_e;
	required_device<timer_device> m_timer_f;
	required_device<timer_device> m_scheduler;

	required_device<gpl_dma_device> m_gpl_dma;
	required_device<gpl_timebase_device> m_gpl_timebase;

	// config/hacks
	bool m_disable_timebase_interrupts;

private:
	void dma_complete(int state);

};



class sunplus_gcm394_device : public sunplus_gcm394_base_device
{
public:
	template <typename T>
	sunplus_gcm394_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag) :
		sunplus_gcm394_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
	}

	sunplus_gcm394_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};





DECLARE_DEVICE_TYPE(GCM394, sunplus_gcm394_device)



#endif // MAME_MACHINE_GENERALPLUS_GPL162XX_SOC_H
