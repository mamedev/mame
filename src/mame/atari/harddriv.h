// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Atari polygon racer games

**************************************************************************/
#ifndef MAME_ATARI_HARDDRIV_H
#define MAME_ATARI_HARDDRIV_H

#pragma once

#include "atarijsa.h"
#include "slapstic.h"

#include "bus/rs232/rs232.h"

#include "cpu/adsp2100/adsp2100.h"
#include "cpu/dsp32/dsp32.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68010.h"
#include "cpu/tms32010/tms32010.h"
#include "cpu/tms34010/tms34010.h"

#include "machine/74259.h"
#include "machine/adc0808.h"
#include "asic65.h"
#include "machine/eeprompar.h"
#include "machine/mc68681.h"
#include "machine/timekpr.h"
#include "machine/timer.h"

#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"

#define HARDDRIV_MASTER_CLOCK   XTAL(32'000'000)
#define HARDDRIV_GSP_CLOCK      XTAL(48'000'000)

DECLARE_DEVICE_TYPE(HARDDRIV_BOARD,               harddriv_board_device_state)
DECLARE_DEVICE_TYPE(HARDDRIVC_BOARD,              harddrivc_board_device_state)
DECLARE_DEVICE_TYPE(RACEDRIV_BOARD,               racedriv_board_device_state)
DECLARE_DEVICE_TYPE(RACEDRIVB1_BOARD,             racedrivb1_board_device_state)
DECLARE_DEVICE_TYPE(RACEDRIVC_BOARD,              racedrivc_board_device_state)
DECLARE_DEVICE_TYPE(RACEDRIVC1_BOARD,             racedrivc1_board_device_state)
DECLARE_DEVICE_TYPE(RACEDRIVC_PANORAMA_SIDE_BOARD,racedrivc_panorama_side_board_device_state)
DECLARE_DEVICE_TYPE(STUNRUN_BOARD,                stunrun_board_device_state)
DECLARE_DEVICE_TYPE(STEELTAL_BOARD,               steeltal_board_device_state)
DECLARE_DEVICE_TYPE(STEELTAL1_BOARD,              steeltal1_board_device_state)
DECLARE_DEVICE_TYPE(STEELTALP_BOARD,              steeltalp_board_device_state)
DECLARE_DEVICE_TYPE(STRTDRIV_BOARD,               strtdriv_board_device_state)
DECLARE_DEVICE_TYPE(HDRIVAIR_BOARD,               hdrivair_board_device_state)
DECLARE_DEVICE_TYPE(HDRIVAIRP_BOARD,              hdrivairp_board_device_state)
DECLARE_DEVICE_TYPE(HARDDRIV_SOUND_BOARD,         harddriv_sound_board_device)


class harddriv_state : public device_t
{
public:
	harddriv_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void driver_msp(machine_config &config);
	void driver_nomsp(machine_config &config);
	void multisync_msp(machine_config &config);
	void multisync_nomsp(machine_config &config);
	void dsk(machine_config &config);
	void dsk2(machine_config &config);
	void ds3(machine_config &config);
	void multisync2(machine_config &config);
	void adsp(machine_config &config);

	void init_strtdriv(void);

	void init_harddriv(void);

	void init_harddrivc(void);

	void init_racedriv(void);
	void init_racedrivb1(void);

	void init_racedrivc(void);
	void init_racedrivc1(void);

	void init_hdrivair(void);
	void init_hdrivairp(void);

	void init_steeltal(void);
	void init_steeltal1(void);
	void init_steeltalp(void);

	void init_stunrun(void);
	void init_racedrivc_panorama_side();

	mc68681_device* get_duart() { return m_duartn68681; }
	screen_device* get_screen() { return m_screen; }

	void video_int_write_line(int state);
	void sound_int_write_line(int state);

protected:
	void init_video();
	INTERRUPT_GEN_MEMBER(hd68k_irq_gen);
	TIMER_CALLBACK_MEMBER(deferred_adsp_bank_switch);
	TIMER_CALLBACK_MEMBER(rddsp32_sync_cb);

	/*----------- defined in machine/harddriv.cpp -----------*/

	/* Driver/Multisync board */
	void hd68k_irq_ack_w(uint16_t data);

	void harddriv_duart_irq_handler(int state);

	uint16_t hd68k_gsp_io_r(offs_t offset);
	void hd68k_gsp_io_w(offs_t offset, uint16_t data);

	uint16_t hd68k_msp_io_r(offs_t offset);
	void hd68k_msp_io_w(offs_t offset, uint16_t data);

	uint16_t hd68k_a80000_r();
	uint16_t hd68k_port0_r();
	uint16_t hd68k_adc12_r();
	uint16_t hdc68k_port1_r();
	uint16_t hda68k_port1_r();
	uint16_t hdc68k_wheel_r();
	uint16_t hd68k_sound_reset_r();

	void hd68k_adc_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void hd68k_wr0_write(offs_t offset, uint16_t data);
	void hd68k_wr1_write(offs_t offset, uint16_t data);
	void hd68k_wr2_write(offs_t offset, uint16_t data);
	void hd68k_nwr_w(offs_t offset, uint16_t data);
	void hdc68k_wheel_edge_reset_w(uint16_t data);

	uint16_t hd68k_zram_r(address_space &space, offs_t offset, uint16_t mem_mask = ~0);
	void hd68k_zram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void hdgsp_io_w(offs_t offset, u16 data, u16 mem_mask = ~u16(0));

	void hdgsp_protection_w(uint16_t data);

	void hdgsp_irq_gen(int state);
	void hdmsp_irq_gen(int state);

	/* ADSP board */
	uint16_t hd68k_adsp_program_r(offs_t offset);
	void hd68k_adsp_program_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t hd68k_adsp_data_r(offs_t offset);
	void hd68k_adsp_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t hd68k_adsp_buffer_r(offs_t offset);
	void hd68k_adsp_buffer_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void hd68k_adsp_control_w(offs_t offset, uint16_t data);
	void hd68k_adsp_irq_clear_w(uint16_t data);
	uint16_t hd68k_adsp_irq_state_r();

	uint16_t hdadsp_special_r(offs_t offset);
	void hdadsp_special_w(offs_t offset, uint16_t data);

	uint16_t steeltal_dummy_r();
	uint32_t rddsp_unmap_r();

	/* DS III/IV board */
	void hd68k_ds3_control_w(offs_t offset, uint16_t data);
	uint16_t hd68k_ds3_girq_state_r();

	uint16_t hd68k_ds3_gdata_r();
	void hd68k_ds3_gdata_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t hdds3_special_r(offs_t offset);
	void hdds3_special_w(offs_t offset, uint16_t data);
	uint16_t hdds3_control_r(offs_t offset);
	void hdds3_control_w(offs_t offset, uint16_t data);

	uint16_t hd68k_ds3_program_r(offs_t offset);
	void hd68k_ds3_program_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t hd68k_ds3_sdata_r();
	void hd68k_ds3_sdata_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void hd68k_ds3_sirq_clear_w(uint16_t data);
	uint16_t hd68k_ds3_sirq_state_r();

	uint16_t hdds3_sdsp_special_r(offs_t offset);
	void hdds3_sdsp_special_w(offs_t offset, uint16_t data);

	uint16_t hdds3_sdsp_control_r(offs_t offset);
	void hdds3_sdsp_control_w(offs_t offset, uint16_t data);
	uint16_t hdds3_xdsp_control_r(offs_t offset);
	void hdds3_xdsp_control_w(offs_t offset, uint16_t data);

	TIMER_CALLBACK_MEMBER( xdsp_sport1_irq_off_callback );

	uint16_t hdgsp_control_lo_r(offs_t offset);
	void hdgsp_control_lo_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t hdgsp_control_hi_r(offs_t offset);
	void hdgsp_control_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t hdgsp_vram_2bpp_r();
	void hdgsp_vram_1bpp_w(offs_t offset, uint16_t data);
	void hdgsp_vram_2bpp_w(offs_t offset, uint16_t data);

	uint16_t hdgsp_paletteram_lo_r(offs_t offset);
	void hdgsp_paletteram_lo_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t hdgsp_paletteram_hi_r(offs_t offset);
	void hdgsp_paletteram_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	/* DSK board */
	void hddsk_update_pif(uint32_t data);

	/* DS III/IV board */
	TIMER_DEVICE_CALLBACK_MEMBER( ds3sdsp_internal_timer_callback );
	TIMER_DEVICE_CALLBACK_MEMBER( ds3xdsp_internal_timer_callback );

	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_driver);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_multisync);

	/*----------- defined in video/harddriv.cpp -----------*/

	TMS340X0_TO_SHIFTREG_CB_MEMBER(hdgsp_write_to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(hdgsp_read_from_shiftreg);

	/* DSK board */
	void hd68k_dsk_control_w(offs_t offset, uint16_t data);
	uint16_t hd68k_dsk_ram_r(offs_t offset);
	void hd68k_dsk_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t hd68k_dsk_small_rom_r(offs_t offset);
	uint16_t hd68k_dsk_rom_r(offs_t offset);
	void hd68k_dsk_dsp32_w(offs_t offset, uint16_t data);
	uint16_t hd68k_dsk_dsp32_r(offs_t offset);
	void rddsp32_sync0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void rddsp32_sync1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	/* DSPCOM board */
	void hddspcom_control_w(offs_t offset, uint16_t data);

	/* Game-specific protection */
	void st68k_sloop_w(offs_t offset, uint16_t data);
	uint16_t st68k_sloop_r(offs_t offset);
	uint16_t st68k_sloop_alt_r(offs_t offset);
	void st68k_protosloop_w(offs_t offset, uint16_t data);
	uint16_t st68k_protosloop_r(offs_t offset);

	/* GSP optimizations */
	uint16_t hdgsp_speedup_r(offs_t offset);
	void hdgsp_speedup1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void hdgsp_speedup2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t rdgsp_speedup1_r(offs_t offset);
	void rdgsp_speedup1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	/* MSP optimizations */
	uint16_t hdmsp_speedup_r(offs_t offset);
	void hdmsp_speedup_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	/* ADSP optimizations */
	uint16_t hdadsp_speedup_r();
	uint16_t hdds3_speedup_r();

	void display_speedups();

	void hdds3sdsp_timer_enable_callback(int state);
	void hdds3sdsp_serial_tx_callback(uint32_t data);
	uint32_t hdds3sdsp_serial_rx_callback();

	void hdds3xdsp_timer_enable_callback(int state);
	void hdds3xdsp_serial_tx_callback(uint32_t data);
	uint32_t hdds3xdsp_serial_rx_callback();

	void adsp_data_map(address_map &map) ATTR_COLD;
	void adsp_program_map(address_map &map) ATTR_COLD;
	void driver_68k_map(address_map &map) ATTR_COLD;
	void driver_gsp_map(address_map &map) ATTR_COLD;
	void driver_msp_map(address_map &map) ATTR_COLD;
	void ds3_data_map(address_map &map) ATTR_COLD;
	void ds3_program_map(address_map &map) ATTR_COLD;
	void ds3sdsp_data_map(address_map &map) ATTR_COLD;
	void ds3sdsp_program_map(address_map &map) ATTR_COLD;
	void ds3xdsp_data_map(address_map &map) ATTR_COLD;
	void ds3xdsp_program_map(address_map &map) ATTR_COLD;
	void dsk2_dsp32_map(address_map &map) ATTR_COLD;
	void dsk_dsp32_map(address_map &map) ATTR_COLD;
	void multisync2_68k_map(address_map &map) ATTR_COLD;
	void multisync2_gsp_map(address_map &map) ATTR_COLD;
	void multisync_68k_map(address_map &map) ATTR_COLD;
	void multisync_gsp_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<tms34010_device> m_gsp;
	optional_device<tms34010_device> m_msp;
	required_device<adsp21xx_device> m_adsp;
	optional_device<cpu_device> m_jsacpu;
	optional_device<dsp32c_device> m_dsp32;
	optional_device<adsp2105_device> m_ds3sdsp;
	optional_device<adsp2105_device> m_ds3xdsp;
	optional_memory_region m_ds3sdsp_region;
	optional_memory_region m_ds3xdsp_region;
	optional_device<dac_word_interface> m_ldac;
	optional_device<dac_word_interface> m_rdac;
	optional_device<harddriv_sound_board_device> m_harddriv_sound;
	optional_device<atari_jsa_base_device> m_jsa;
	optional_device<screen_device> m_screen;
	optional_device<mc68681_device> m_duartn68681;
	required_device<adc0808_device> m_adc8;
	output_finder<2> m_lamps;
	output_finder<4> m_sel;
	output_finder<> m_wheel;

	uint8_t                 m_hd34010_host_access = 0;

	optional_shared_ptr<uint16_t> m_msp_ram;

	uint16_t *              m_dsk_ram = nullptr;
	uint16_t *              m_dsk_rom = nullptr;
	optional_device<eeprom_parallel_28xx_device> m_dsk_10c;
	optional_device<eeprom_parallel_28xx_device> m_dsk_30c;
	uint8_t                 m_dsk_pio_access = 0;

	uint16_t *              m_m68k_sloop_base = nullptr;
	uint16_t *              m_m68k_sloop_alt_base = nullptr;

	required_device<timekeeper_device> m_200e;
	required_device<eeprom_parallel_28xx_device> m_210e;

	optional_shared_ptr<uint16_t> m_adsp_data_memory;
	optional_shared_ptr<uint32_t> m_adsp_pgm_memory;

	optional_shared_ptr<uint16_t> m_ds3sdsp_data_memory;
	optional_shared_ptr<uint32_t> m_ds3sdsp_pgm_memory;
	optional_shared_ptr<uint32_t> m_ds3xdsp_pgm_memory;

	optional_shared_ptr<uint32_t> m_dsp32_ram;

	uint16_t *              m_gsp_protection = nullptr;

	uint16_t *              m_gsp_speedup_addr[2]{};
	offs_t                  m_gsp_speedup_pc = 0;

	uint16_t *              m_msp_speedup_addr = nullptr;
	offs_t                  m_msp_speedup_pc = 0;

	uint16_t *              m_ds3_speedup_addr = nullptr;
	offs_t                  m_ds3_speedup_pc = 0;
	offs_t                  m_ds3_transfer_pc = 0;

	uint32_t *              m_rddsp32_sync[2]{};

	uint32_t                m_gsp_speedup_count[4]{};
	uint32_t                m_msp_speedup_count[4]{};
	uint32_t                m_adsp_speedup_count[4]{};

	uint8_t                 m_gsp_multisync = 0;
	optional_shared_ptr<uint16_t>  m_gsp_vram;
	optional_shared_ptr<uint16_t>  m_gsp_control_lo;
	optional_shared_ptr<uint16_t>  m_gsp_control_hi;
	memory_share_creator<uint16_t> m_gsp_paletteram_lo;
	memory_share_creator<uint16_t> m_gsp_paletteram_hi;

	required_ioport m_in0;
	optional_ioport m_sw1;
	required_ioport m_a80000;
	optional_ioport_array<4> m_12badc;

	/* machine state */
	uint8_t                 m_irq_state = 0;
	uint8_t                 m_gsp_irq_state = 0;
	uint8_t                 m_msp_irq_state = 0;
	uint8_t                 m_adsp_irq_state = 0;
	uint8_t                 m_ds3sdsp_irq_state = 0;
	uint8_t                 m_duart_irq_state = 0;

	uint8_t                 m_last_gsp_shiftreg = 0;

	uint8_t                 m_m68k_zp1 = 0;
	uint8_t                 m_m68k_zp2 = 0;
	uint8_t                 m_m68k_adsp_buffer_bank = 0;

	uint8_t                 m_adsp_halt = 0;
	uint8_t                 m_adsp_br = 0;
	uint8_t                 m_adsp_xflag = 0;
	uint16_t                m_adsp_sim_address = 0;
	uint16_t                m_adsp_som_address = 0;
	uint32_t                m_adsp_eprom_base = 0;

	required_region_ptr<uint16_t> m_sim_memory;
	uint16_t                m_som_memory[0x8000/2]{};
	uint16_t *              m_adsp_pgm_memory_word = 0;

	uint16_t *              m_ds3_sdata_memory = 0;
	uint32_t                m_ds3_sdata_memory_size = 0;

	uint8_t                 m_ds3_gcmd = 0;
	uint8_t                 m_ds3_gflag = 0;
	uint8_t                 m_ds3_g68irqs = 0;
	uint8_t                 m_ds3_gfirqs = 0;
	uint8_t                 m_ds3_g68flag = 0;
	uint8_t                 m_ds3_send = 0;
	uint8_t                 m_ds3_reset = 0;
	uint16_t                m_ds3_gdata = 0;
	uint16_t                m_ds3_g68data = 0;
	uint32_t                m_ds3_sim_address = 0;

	uint8_t                 m_ds3_scmd = 0;
	uint8_t                 m_ds3_sflag = 0;
	uint8_t                 m_ds3_s68irqs = 0;
	uint8_t                 m_ds3_sfirqs = 0;
	uint8_t                 m_ds3_s68flag = 0;
	uint8_t                 m_ds3_sreset = 0;
	uint16_t                m_ds3_sdata = 0;
	uint16_t                m_ds3_s68data = 0;
	uint32_t                m_ds3_sdata_address = 0;
	uint16_t                m_ds3sdsp_regs[32]{};
	uint16_t                m_ds3sdsp_timer_en = 0;
	uint16_t                m_ds3sdsp_sdata = 0;
	optional_device<timer_device> m_ds3sdsp_internal_timer;

	uint16_t                m_ds3xdsp_regs[32]{};
	uint16_t                m_ds3xdsp_timer_en = 0;
	uint16_t                m_ds3xdsp_sdata = 0;
	optional_device<timer_device> m_ds3xdsp_internal_timer;
	emu_timer *             m_xdsp_serial_irq_off_timer = nullptr;

	uint16_t                m_adc_control = 0;
	uint8_t                 m_adc12_select = 0;
	uint8_t                 m_adc12_byte = 0;
	uint16_t                m_adc12_data = 0;

	uint16_t                m_hdc68k_last_wheel = 0;
	uint16_t                m_hdc68k_last_port1 = 0;
	uint8_t                 m_hdc68k_wheel_edge = 0;
	uint8_t                 m_hdc68k_shifter_state = 0;

	uint8_t                 m_st68k_sloop_bank = 0;
	offs_t                  m_st68k_last_alt_sloop_offset = 0;

	uint8_t                 m_sel_select = 0;
	uint8_t                 m_sel1_data = 0;
	uint8_t                 m_sel2_data = 0;
	uint8_t                 m_sel3_data = 0;
	uint8_t                 m_sel4_data = 0;

	#define MAX_MSP_SYNC    16
	uint32_t *              m_dataptr[MAX_MSP_SYNC]{};
	uint32_t                m_dataval[MAX_MSP_SYNC]{};
	int                     m_next_msp_sync = 0;

	/* video state */
	offs_t                  m_vram_mask = 0;

	uint8_t                 m_shiftreg_enable = 0;

	uint32_t                m_mask_table[65536 * 4]{};
	uint16_t *              m_gsp_shiftreg_source = 0;

	int8_t                  m_gfx_finescroll = 0;
	uint8_t                 m_gfx_palettebank = 0;
	virtual void update_interrupts();
	void init_driver();
	void init_multisync(int compact_inputs);
	void init_adsp();
	void init_ds3();
	void init_dsk();
	void init_dsk2();
	void init_dspcom();
	void init_driver_sound();
	void racedrivc_init_common(offs_t gsp_protection);
	void steeltal_init_common(offs_t ds3_transfer_pc, int proto_sloop);

	required_device<mc68681_device> m_duart;
	optional_device<asic65_device> m_asic65;

	/* DS III/IV board */
	void update_ds3_irq();
	void update_ds3_sirq();

	void hdds3sdsp_reset_timer();
	void hdds3xdsp_reset_timer();

	/* Game-specific protection */
	int st68k_sloop_tweak(offs_t offset);
	int st68k_protosloop_tweak(offs_t offset);

	/*----------- defined in video/harddriv.c -----------*/

	void update_palette_bank(int newbank);

	inline void gsp_palette_change(int offset);

	uint8_t               m_sound_int_state = 0;
	uint8_t               m_video_int_state = 0;

	optional_device<palette_device> m_palette;
	int get_hblank(screen_device &screen) const { return (screen.hpos() > (screen.width() * 9 / 10)); }
	optional_device<atari_slapstic_device> m_slapstic;
	memory_bank_creator m_slapstic_bank;

	optional_device<rs232_port_device> m_rs232;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class harddriv_sound_board_device :  public device_t
{
public:
	// construction/destruction
	harddriv_sound_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~harddriv_sound_board_device() {}

	uint16_t hd68k_snd_data_r();
	uint16_t hd68k_snd_status_r();
	void hd68k_snd_data_w(uint16_t data);
	void hd68k_snd_reset_w(uint16_t data);

private:
	uint16_t hdsnd68k_data_r();
	void hdsnd68k_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t hdsnd68k_switches_r(offs_t offset);
	uint16_t hdsnd68k_320port_r(offs_t offset);
	uint16_t hdsnd68k_status_r();
	void hdsnd68k_latches_w(offs_t offset, uint16_t data);
	void speech_write_w(int state);
	void speech_reset_w(int state);
	void speech_rate_w(int state);
	void cram_enable_w(int state);
	void led_w(int state);
	void hdsnd68k_speech_w(offs_t offset, uint16_t data);
	void hdsnd68k_irqclr_w(uint16_t data);
	uint16_t hdsnd68k_320ram_r(offs_t offset);
	void hdsnd68k_320ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t hdsnd68k_320ports_r(offs_t offset);
	void hdsnd68k_320ports_w(offs_t offset, uint16_t data);
	uint16_t hdsnd68k_320com_r(offs_t offset);
	void hdsnd68k_320com_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void hdsnddsp_dac_w(uint16_t data);
	void hdsnddsp_comport_w(uint16_t data);
	void hdsnddsp_mute_w(uint16_t data);
	void hdsnddsp_gen68kirq_w(uint16_t data);
	void hdsnddsp_soundaddr_w(offs_t offset, uint16_t data);
	uint16_t hdsnddsp_rom_r();
	uint16_t hdsnddsp_comram_r();
	uint16_t hdsnddsp_compare_r(offs_t offset);

	void driversnd_68k_map(address_map &map) ATTR_COLD;
	void driversnd_dsp_io_map(address_map &map) ATTR_COLD;
	void driversnd_dsp_program_map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<cpu_device> m_soundcpu;
	required_device<ls259_device> m_latch;
	required_device<dac_word_interface> m_dac;
	required_device<tms32010_device> m_sounddsp;
	required_shared_ptr<uint16_t> m_sounddsp_ram;
	required_region_ptr<uint8_t>  m_sound_rom;

	uint8_t                   m_soundflag = 0;
	uint8_t                   m_mainflag = 0;
	uint16_t                  m_sounddata = 0;
	uint16_t                  m_maindata = 0;

	uint8_t                   m_cramen = 0;
	uint8_t                   m_irq68k = 0;

	offs_t                  m_sound_rom_offs = 0;

	uint16_t                  m_comram[0x400/2]{};
	uint64_t                  m_last_bio_cycles = 0;

	void update_68k_interrupts();
	TIMER_CALLBACK_MEMBER( delayed_68k_w );

	int hdsnddsp_get_bio();
};


/* Hard Drivin' */

class harddriv_board_device_state :  public harddriv_state
{
public:
	harddriv_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
//  virtual void device_reset();
};


/* Hard Drivin' Compact */

class harddrivc_board_device_state :  public harddriv_state
{
public:
	harddrivc_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
//  virtual void device_reset();
};


/* Race Drivin' */

class racedriv_board_device_state :  public harddriv_state
{
public:
	racedriv_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	racedriv_board_device_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
//  virtual void device_reset();
};

class racedrivb1_board_device_state :  public racedriv_board_device_state
{
public:
	racedrivb1_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


/* Race Drivin' Compact */

class racedrivc_board_device_state :  public harddriv_state
{
public:
	racedrivc_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	racedrivc_board_device_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
//  virtual void device_reset();
};

class racedrivc1_board_device_state :  public racedrivc_board_device_state
{
public:
	racedrivc1_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class racedrivc_panorama_side_board_device_state :  public racedrivc_board_device_state
{
public:
	racedrivc_panorama_side_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};


/* Stun Runner */

class stunrun_board_device_state :  public harddriv_state
{
public:
	stunrun_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
//  virtual void device_reset();
};


/* Steel Talons */

class steeltal_board_device_state :  public harddriv_state
{
public:
	steeltal_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	steeltal_board_device_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
//  virtual void device_reset();
};

class steeltal1_board_device_state :  public steeltal_board_device_state
{
public:
	steeltal1_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class steeltalp_board_device_state :  public steeltal_board_device_state
{
public:
	steeltalp_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


/* Street Drivin' */

class strtdriv_board_device_state :  public harddriv_state
{
public:
	strtdriv_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
//  virtual void device_reset();
};


/* Hard Drivin' Airbourne */

class hdrivair_board_device_state :  public harddriv_state
{
public:
	hdrivair_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	hdrivair_board_device_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
//  virtual void device_reset();
};

class hdrivairp_board_device_state :  public hdrivair_board_device_state
{
public:
	hdrivairp_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	virtual void device_start() override ATTR_COLD;
};

#endif // MAME_ATARI_HARDDRIV_H
