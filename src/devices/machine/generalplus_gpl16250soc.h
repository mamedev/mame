// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

  SunPlus "GCM394" (based on die pictures)

**********************************************************************/

#ifndef MAME_MACHINE_GENERALPLUS_GPL16250SOC_H
#define MAME_MACHINE_GENERALPLUS_GPL16250SOC_H

#pragma once

#include "cpu/unsp/unsp.h"
#include "screen.h"
#include "emupal.h"
#include "generalplus_gpl16250soc_video.h"
#include "spg2xx_audio.h"

typedef device_delegate<void (uint16_t, uint16_t, uint16_t, uint16_t, uint16_t)> sunplus_gcm394_cs_callback_device;

class sunplus_gcm394_base_device : public unsp_20_device, public device_mixer_interface
{
public:
	sunplus_gcm394_base_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
		sunplus_gcm394_base_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(sunplus_gcm394_base_device::gcm394_internal_map), this))
	{
	}

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return m_spg_video->screen_update(screen, bitmap, cliprect); }

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

	auto nand_read_callback() { return m_nand_read_cb.bind(); }

	void vblank(int state) { m_spg_video->vblank(state); }

	virtual void device_add_mconfig(machine_config& config) override;

	void set_bootmode(int mode) { m_boot_mode = mode; }

	IRQ_CALLBACK_MEMBER(irq_vector_cb);
	template <typename... T> void set_cs_config_callback(T &&... args) { m_cs_callback.set(std::forward<T>(args)...); }
	void default_cs_callback(uint16_t cs0, uint16_t cs1, uint16_t cs2, uint16_t cs3, uint16_t cs4 );

	void set_cs_space(address_space* csspace) { m_cs_space = csspace; }

	//void set_paldisplaybank_high_hack(int pal_displaybank_high) { m_spg_video->set_paldisplaybank_high(pal_displaybank_high); }
	void set_alt_tile_addressing_hack(int alt_tile_addressing) { m_spg_video->set_alt_tile_addressing(alt_tile_addressing); }
	//void set_pal_sprites_hack(int pal_sprites) { m_spg_video->set_pal_sprites(pal_sprites); }
	//void set_pal_back_hack(int pal_back) { m_spg_video->set_pal_back(pal_back); }
	void set_alt_extrasprite_hack(int alt_extrasprite_hack) { m_spg_video->set_alt_extrasprite(alt_extrasprite_hack); }
	void set_legacy_video_mode() { m_spg_video->set_legacy_video_mode(); }
	void set_disallow_resolution_control() { m_spg_video->set_disallow_resolution_control(); }

	void set_romtype(int romtype) { m_romtype = romtype; }

	inline uint16_t get_ram_addr(uint32_t addr) { return m_mainram[addr]; }

protected:
	sunplus_gcm394_base_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock, address_map_constructor internal);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_post_load() override;

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

	uint16_t m_dma_params[8][4];
	bool m_dma_latched[4];

	// unk 78xx
	uint16_t m_7803;

	uint16_t m_7807;

	uint16_t m_membankswitch_7810;

	uint16_t m_7816;
	uint16_t m_7817;

	uint16_t m_7819;

	uint16_t m_782x[5];

	uint16_t m_782d;

	uint16_t m_7835;

	uint16_t m_7860;

	uint16_t m_7861;

	uint16_t m_7862_porta_direction;
	uint16_t m_7863_porta_attribute;

	uint16_t m_786a_portb_direction;
	uint16_t m_786b_portb_attribute;

	uint16_t m_7872_portc_direction;
	uint16_t m_7873_portc_attribute;

	uint16_t m_787a_portd_direction;
	uint16_t m_787b_portd_attribute;

	uint16_t m_7870;

	//uint16_t m_7871;


	uint16_t m_7882;
	uint16_t m_7883;

	uint16_t m_78a0;

	uint16_t m_78a4;
	uint16_t m_78a5;
	uint16_t m_78a6;

	uint16_t m_78a8;

	uint16_t m_78b0;
	uint16_t m_78b1;
	uint16_t m_78b2;

	uint16_t m_78b8;

	uint16_t m_78f0;

	uint16_t m_78fb;

	// unk 79xx
	uint16_t m_7934;
	uint16_t m_7935;
	uint16_t m_7936;

	uint16_t m_7960;
	uint16_t m_7961;

	uint16_t m_system_dma_memtype;

	devcb_read16 m_nand_read_cb;
	uint32_t m_csbase;

	uint16_t internalrom_lower32_r(offs_t offset);

	address_space* m_cs_space;

	uint16_t cs_space_r(offs_t offset);
	void cs_space_w(offs_t offset, uint16_t data);
	uint16_t cs_bank_space_r(offs_t offset);
	void cs_bank_space_w(offs_t offset, uint16_t data);
	int m_romtype;

private:
	devcb_read16 m_space_read_cb;
	devcb_write16 m_space_write_cb;

	uint16_t unk_r(offs_t offset);
	void unk_w(offs_t offset, uint16_t data);

	void write_dma_params(int channel, int offset, uint16_t data);
	uint16_t read_dma_params(int channel, int offset);
	void trigger_systemm_dma(int channel);

	uint16_t system_dma_params_channel0_r(offs_t offset);
	void system_dma_params_channel0_w(offs_t offset, uint16_t data);
	uint16_t system_dma_params_channel1_r(offs_t offset);
	void system_dma_params_channel1_w(offs_t offset, uint16_t data);
	uint16_t system_dma_params_channel2_r(offs_t offset);
	void system_dma_params_channel2_w(offs_t offset, uint16_t data);
	uint16_t system_dma_params_channel3_r(offs_t offset);
	void system_dma_params_channel3_w(offs_t offset, uint16_t data);
	uint16_t system_dma_status_r();
	void system_dma_7abf_unk_w(uint16_t data);
	uint16_t system_dma_memtype_r();
	void system_dma_memtype_w(uint16_t data);

	uint16_t unkarea_780f_status_r();
	uint16_t unkarea_78fb_status_r();

	uint16_t unkarea_7803_r();
	void unkarea_7803_w(uint16_t data);

	void unkarea_7807_w(uint16_t data);

	void waitmode_enter_780c_w(uint16_t data);

	uint16_t membankswitch_7810_r();
	void membankswitch_7810_w(uint16_t data);

	void unkarea_7816_w(uint16_t data);
	void unkarea_7817_w(uint16_t data);

	uint16_t unkarea_7819_r();
	void unkarea_7819_w(uint16_t data);

	void chipselect_csx_memory_device_control_w(offs_t offset, uint16_t data);

	void unkarea_7835_w(uint16_t data);

	uint16_t unkarea_782d_r();
	void unkarea_782d_w(uint16_t data);

	// Port A
	uint16_t ioarea_7860_porta_r();
	void ioarea_7860_porta_w(uint16_t data);
	uint16_t ioarea_7861_porta_buffer_r();
	void ioarea_7861_porta_buffer_w(uint16_t data);
	uint16_t ioarea_7862_porta_direction_r();
	void ioarea_7862_porta_direction_w(uint16_t data);
	uint16_t ioarea_7863_porta_attribute_r();
	void ioarea_7863_porta_attribute_w(uint16_t data);

	// Port B
	uint16_t ioarea_7868_portb_r();
	void ioarea_7868_portb_w(uint16_t data);
	uint16_t ioarea_7869_portb_buffer_r();
	void ioarea_7869_portb_buffer_w(uint16_t data);
	uint16_t ioarea_786a_portb_direction_r();
	void ioarea_786a_portb_direction_w(uint16_t data);
	uint16_t ioarea_786b_portb_attribute_r();
	void ioarea_786b_portb_attribute_w(uint16_t data);

	// Port C
	uint16_t ioarea_7870_portc_r();
	void ioarea_7870_portc_w(uint16_t data);
	uint16_t ioarea_7871_portc_buffer_r();
	void ioarea_7871_portc_buffer_w(uint16_t data);
	uint16_t ioarea_7872_portc_direction_r();
	void ioarea_7872_portc_direction_w(uint16_t data);
	uint16_t ioarea_7873_portc_attribute_r();
	void ioarea_7873_portc_attribute_w(uint16_t data);

	// Port D
	uint16_t ioarea_7878_portd_r();
	void ioarea_7878_portd_w(uint16_t data);
	uint16_t ioarea_7879_portd_buffer_r();
	void ioarea_7879_portd_buffer_w(uint16_t data);
	uint16_t ioarea_787a_portd_direction_r();
	void ioarea_787a_portd_direction_w(uint16_t data);
	uint16_t ioarea_787b_portd_attribute_r();
	void ioarea_787b_portd_attribute_w(uint16_t data);

	uint16_t unkarea_7882_r();
	void unkarea_7882_w(uint16_t data);
	uint16_t unkarea_7883_r();
	void unkarea_7883_w(uint16_t data);

	void unkarea_78a0_w(uint16_t data);

	uint16_t unkarea_78a0_r();
	uint16_t unkarea_78a1_r();

	void unkarea_78a4_w(uint16_t data);
	void unkarea_78a5_w(uint16_t data);
	void unkarea_78a6_w(uint16_t data);

	void unkarea_78a8_w(uint16_t data);

	void unkarea_78b0_w(uint16_t data);
	void unkarea_78b1_w(uint16_t data);

	uint16_t unkarea_78b2_r();
	void unkarea_78b2_w(uint16_t data);

	void unkarea_78b8_w(uint16_t data);

	uint16_t unkarea_78c0_r();

	uint16_t unkarea_78d0_r();
	uint16_t unkarea_78d8_r();

	void unkarea_78f0_w(uint16_t data);

	uint16_t unkarea_7904_r();

	uint16_t unkarea_7934_r();
	void unkarea_7934_w(uint16_t data);

	uint16_t unkarea_7935_r();
	void unkarea_7935_w(uint16_t data);

	uint16_t unkarea_7936_r();
	void unkarea_7936_w(uint16_t data);

	uint16_t spi_7944_rxdata_r();
	uint16_t spi_7945_misc_control_reg_r();
	void spi_7942_txdata_w(uint16_t data);

	void unkarea_7960_w(uint16_t data);
	uint16_t unkarea_7961_r();
	void unkarea_7961_w(uint16_t data);


	void videoirq_w(int state);
	void audioirq_w(int state);

	uint16_t system_7a35_r();
	uint16_t system_7a37_r();
	uint16_t system_7a39_r();
	uint16_t system_7a3a_r();
	uint16_t system_7a46_r();
	uint16_t system_7a54_r();

	void checkirq6();

	emu_timer *m_unk_timer;

	TIMER_CALLBACK_MEMBER(unknown_tick);

	inline uint16_t read_space(uint32_t offset);
	inline void write_space(uint32_t offset, uint16_t data);

	// config registers (external pins)
	int m_boot_mode; // 2 pins determine boot mode, likely only read at power-on
	sunplus_gcm394_cs_callback_device m_cs_callback;
};



class sunplus_gcm394_device : public sunplus_gcm394_base_device
{
public:
	template <typename T>
	sunplus_gcm394_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag) :
		sunplus_gcm394_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
	}

	sunplus_gcm394_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class generalplus_gpac800_device : public sunplus_gcm394_base_device
{
public:
	template <typename T>
	generalplus_gpac800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag) :
		generalplus_gpac800_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
		m_csbase = 0x30000;
	}

	generalplus_gpac800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	void gpac800_internal_map(address_map &map) ATTR_COLD;

	//virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void recalculate_calculate_effective_nand_address();

	uint16_t nand_7850_status_r();
	uint16_t nand_7854_r();
	void nand_dma_ctrl_w(uint16_t data);
	void nand_7850_w(uint16_t data);
	void nand_command_w(uint16_t data);
	void nand_addr_low_w(uint16_t data);
	void nand_addr_high_w(uint16_t data);
	uint16_t nand_ecc_low_byte_error_flag_1_r();
	void nand_7856_type_w(uint16_t data);
	void nand_7857_w(uint16_t data);
	void nand_785b_w(uint16_t data);
	void nand_785c_w(uint16_t data);
	void nand_785d_w(uint16_t data);
	uint16_t nand_785e_r();

	uint16_t m_nandcommand;

	uint16_t m_nand_addr_low;
	uint16_t m_nand_addr_high;

	uint16_t m_nand_dma_ctrl;
	uint16_t m_nand_7850;
	uint16_t m_nand_785d;
	uint16_t m_nand_785c;
	uint16_t m_nand_785b;
	uint16_t m_nand_7856;
	uint16_t m_nand_7857;

	int m_curblockaddr;
	uint32_t m_effectiveaddress;
};


class generalplus_gpspispi_device : public sunplus_gcm394_base_device
{
public:
	template <typename T>
	generalplus_gpspispi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag) :
		generalplus_gpspispi_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
		m_csbase = 0x30000;
	}

	generalplus_gpspispi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	void gpspispi_internal_map(address_map &map) ATTR_COLD;

	//virtual void device_start() override ATTR_COLD;
	//virtual void device_reset() override ATTR_COLD;

private:
	uint16_t spi_unk_7943_r();
};

class generalplus_gpspi_direct_device : public sunplus_gcm394_base_device
{
public:
	template <typename T>
	generalplus_gpspi_direct_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag) :
		generalplus_gpspi_direct_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
		//m_csbase = 0x30000;
		// TODO: is cs_space even used by this type?
		m_csbase = 0xffffffff;
	}

	generalplus_gpspi_direct_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	void gpspi_direct_internal_map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t ramread_r(offs_t offset);
	void ramwrite_w(offs_t offset, uint16_t data);
	uint16_t spi_direct_7b40_r();
	uint16_t spi_direct_7b46_r();
	uint16_t spi_direct_7af0_r();
	void spi_direct_7af0_w(uint16_t data);
	uint16_t spi_direct_79f5_r();
	uint16_t spi_direct_78e8_r();
	void spi_direct_78e8_w(uint16_t data);
	uint16_t spi_direct_79f4_r();

	uint16_t m_7af0;
};



DECLARE_DEVICE_TYPE(GCM394, sunplus_gcm394_device)
DECLARE_DEVICE_TYPE(GPAC800, generalplus_gpac800_device)
DECLARE_DEVICE_TYPE(GP_SPISPI, generalplus_gpspispi_device)
DECLARE_DEVICE_TYPE(GP_SPI_DIRECT, generalplus_gpspi_direct_device)

#endif // MAME_MACHINE_GENERALPLUS_GPL16250SOC_H
