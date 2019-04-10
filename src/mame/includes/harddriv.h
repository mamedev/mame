// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Atari polygon racer games

**************************************************************************/
#ifndef MAME_INCLUDES_HARDDRIV_H
#define MAME_INCLUDES_HARDDRIV_H

#pragma once

#include "audio/atarijsa.h"

#include "bus/rs232/rs232.h"

#include "cpu/adsp2100/adsp2100.h"
#include "cpu/dsp32/dsp32.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "cpu/tms34010/tms34010.h"

#include "machine/74259.h"
#include "machine/adc0808.h"
#include "machine/asic65.h"
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

	DECLARE_WRITE_LINE_MEMBER(video_int_write_line);
	DECLARE_WRITE_LINE_MEMBER(sound_int_write_line);

protected:

	void init_video();
	INTERRUPT_GEN_MEMBER(hd68k_irq_gen);
	TIMER_CALLBACK_MEMBER(deferred_adsp_bank_switch);
	TIMER_CALLBACK_MEMBER(rddsp32_sync_cb);

	/*----------- defined in machine/harddriv.c -----------*/

	/* Driver/Multisync board */
	DECLARE_WRITE16_MEMBER( hd68k_irq_ack_w );

	DECLARE_WRITE_LINE_MEMBER(harddriv_duart_irq_handler);

	DECLARE_READ16_MEMBER( hd68k_gsp_io_r );
	DECLARE_WRITE16_MEMBER( hd68k_gsp_io_w );

	DECLARE_READ16_MEMBER( hd68k_msp_io_r );
	DECLARE_WRITE16_MEMBER( hd68k_msp_io_w );

	DECLARE_READ16_MEMBER( hd68k_a80000_r );
	DECLARE_READ16_MEMBER( hd68k_port0_r );
	DECLARE_READ16_MEMBER( hd68k_adc12_r );
	DECLARE_READ16_MEMBER( hdc68k_port1_r );
	DECLARE_READ16_MEMBER( hda68k_port1_r );
	DECLARE_READ16_MEMBER( hdc68k_wheel_r );
	DECLARE_READ16_MEMBER( hd68k_sound_reset_r );

	DECLARE_WRITE16_MEMBER( hd68k_adc_control_w );
	DECLARE_WRITE16_MEMBER( hd68k_wr0_write );
	DECLARE_WRITE16_MEMBER( hd68k_wr1_write );
	DECLARE_WRITE16_MEMBER( hd68k_wr2_write );
	DECLARE_WRITE16_MEMBER( hd68k_nwr_w );
	DECLARE_WRITE16_MEMBER( hdc68k_wheel_edge_reset_w );

	DECLARE_READ16_MEMBER( hd68k_zram_r );
	DECLARE_WRITE16_MEMBER( hd68k_zram_w );

	void hdgsp_io_w(offs_t offset, u16 data, u16 mem_mask = ~u16(0));

	DECLARE_WRITE16_MEMBER( hdgsp_protection_w );

	DECLARE_WRITE_LINE_MEMBER( hdgsp_irq_gen );
	DECLARE_WRITE_LINE_MEMBER( hdmsp_irq_gen );

	/* ADSP board */
	DECLARE_READ16_MEMBER( hd68k_adsp_program_r );
	DECLARE_WRITE16_MEMBER( hd68k_adsp_program_w );

	DECLARE_READ16_MEMBER( hd68k_adsp_data_r );
	DECLARE_WRITE16_MEMBER( hd68k_adsp_data_w );

	DECLARE_READ16_MEMBER( hd68k_adsp_buffer_r );
	DECLARE_WRITE16_MEMBER( hd68k_adsp_buffer_w );

	DECLARE_WRITE16_MEMBER( hd68k_adsp_control_w );
	DECLARE_WRITE16_MEMBER( hd68k_adsp_irq_clear_w );
	DECLARE_READ16_MEMBER( hd68k_adsp_irq_state_r );

	DECLARE_READ16_MEMBER( hdadsp_special_r );
	DECLARE_WRITE16_MEMBER( hdadsp_special_w );

	DECLARE_READ16_MEMBER(steeltal_dummy_r);
	DECLARE_READ32_MEMBER(rddsp_unmap_r);

	/* DS III/IV board */
	DECLARE_WRITE16_MEMBER( hd68k_ds3_control_w );
	DECLARE_READ16_MEMBER( hd68k_ds3_girq_state_r );

	DECLARE_READ16_MEMBER( hd68k_ds3_gdata_r );
	DECLARE_WRITE16_MEMBER( hd68k_ds3_gdata_w );

	DECLARE_READ16_MEMBER( hdds3_special_r );
	DECLARE_WRITE16_MEMBER( hdds3_special_w );
	DECLARE_READ16_MEMBER( hdds3_control_r );
	DECLARE_WRITE16_MEMBER( hdds3_control_w );

	DECLARE_READ16_MEMBER( hd68k_ds3_program_r );
	DECLARE_WRITE16_MEMBER( hd68k_ds3_program_w );

	DECLARE_READ16_MEMBER( hd68k_ds3_sdata_r );
	DECLARE_WRITE16_MEMBER( hd68k_ds3_sdata_w );
	DECLARE_WRITE16_MEMBER( hd68k_ds3_sirq_clear_w );
	DECLARE_READ16_MEMBER( hd68k_ds3_sirq_state_r );

	DECLARE_READ16_MEMBER( hdds3_sdsp_special_r );
	DECLARE_WRITE16_MEMBER( hdds3_sdsp_special_w );

	DECLARE_READ16_MEMBER( hdds3_sdsp_control_r );
	DECLARE_WRITE16_MEMBER( hdds3_sdsp_control_w );
	DECLARE_READ16_MEMBER( hdds3_xdsp_control_r );
	DECLARE_WRITE16_MEMBER( hdds3_xdsp_control_w );

	TIMER_CALLBACK_MEMBER( xsdp_sport1_irq_off_callback );

	DECLARE_READ16_MEMBER( hdgsp_control_lo_r );
	DECLARE_WRITE16_MEMBER( hdgsp_control_lo_w );
	DECLARE_READ16_MEMBER( hdgsp_control_hi_r );
	DECLARE_WRITE16_MEMBER( hdgsp_control_hi_w );

	DECLARE_READ16_MEMBER( hdgsp_vram_2bpp_r );
	DECLARE_WRITE16_MEMBER( hdgsp_vram_1bpp_w );
	DECLARE_WRITE16_MEMBER( hdgsp_vram_2bpp_w );

	DECLARE_READ16_MEMBER( hdgsp_paletteram_lo_r );
	DECLARE_WRITE16_MEMBER( hdgsp_paletteram_lo_w );
	DECLARE_READ16_MEMBER( hdgsp_paletteram_hi_r );
	DECLARE_WRITE16_MEMBER( hdgsp_paletteram_hi_w );

	/* DSK board */
	DECLARE_WRITE32_MEMBER(hddsk_update_pif);

	/* DS III/IV board */
	TIMER_DEVICE_CALLBACK_MEMBER( ds3sdsp_internal_timer_callback );
	TIMER_DEVICE_CALLBACK_MEMBER( ds3xdsp_internal_timer_callback );

	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_driver);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_multisync);

	/*----------- defined in video/harddriv.c -----------*/

	TMS340X0_TO_SHIFTREG_CB_MEMBER(hdgsp_write_to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(hdgsp_read_from_shiftreg);

	/* DSK board */
	DECLARE_WRITE16_MEMBER( hd68k_dsk_control_w );
	DECLARE_READ16_MEMBER( hd68k_dsk_ram_r );
	DECLARE_WRITE16_MEMBER( hd68k_dsk_ram_w );
	DECLARE_READ16_MEMBER( hd68k_dsk_small_rom_r );
	DECLARE_READ16_MEMBER( hd68k_dsk_rom_r );
	DECLARE_WRITE16_MEMBER( hd68k_dsk_dsp32_w );
	DECLARE_READ16_MEMBER( hd68k_dsk_dsp32_r );
	DECLARE_WRITE32_MEMBER( rddsp32_sync0_w );
	DECLARE_WRITE32_MEMBER( rddsp32_sync1_w );

	/* DSPCOM board */
	DECLARE_WRITE16_MEMBER( hddspcom_control_w );

	DECLARE_WRITE16_MEMBER( rd68k_slapstic_w );
	DECLARE_READ16_MEMBER( rd68k_slapstic_r );

	/* Game-specific protection */
	DECLARE_WRITE16_MEMBER( st68k_sloop_w );
	DECLARE_READ16_MEMBER( st68k_sloop_r );
	DECLARE_READ16_MEMBER( st68k_sloop_alt_r );
	DECLARE_WRITE16_MEMBER( st68k_protosloop_w );
	DECLARE_READ16_MEMBER( st68k_protosloop_r );

	/* GSP optimizations */
	DECLARE_READ16_MEMBER( hdgsp_speedup_r );
	DECLARE_WRITE16_MEMBER( hdgsp_speedup1_w );
	DECLARE_WRITE16_MEMBER( hdgsp_speedup2_w );
	DECLARE_READ16_MEMBER( rdgsp_speedup1_r );
	DECLARE_WRITE16_MEMBER( rdgsp_speedup1_w );

	/* MSP optimizations */
	DECLARE_READ16_MEMBER( hdmsp_speedup_r );
	DECLARE_WRITE16_MEMBER( hdmsp_speedup_w );

	/* ADSP optimizations */
	DECLARE_READ16_MEMBER( hdadsp_speedup_r );
	DECLARE_READ16_MEMBER( hdds3_speedup_r );


	DECLARE_WRITE_LINE_MEMBER(hdds3sdsp_timer_enable_callback);
	DECLARE_WRITE32_MEMBER(hdds3sdsp_serial_tx_callback);
	DECLARE_READ32_MEMBER(hdds3sdsp_serial_rx_callback);

	DECLARE_WRITE_LINE_MEMBER(hdds3xdsp_timer_enable_callback);
	DECLARE_WRITE32_MEMBER(hdds3xdsp_serial_tx_callback);
	DECLARE_READ32_MEMBER(hdds3xdsp_serial_rx_callback);


	void adsp_data_map(address_map &map);
	void adsp_program_map(address_map &map);
	void driver_68k_map(address_map &map);
	void driver_gsp_map(address_map &map);
	void driver_msp_map(address_map &map);
	void ds3_data_map(address_map &map);
	void ds3_program_map(address_map &map);
	void ds3sdsp_data_map(address_map &map);
	void ds3sdsp_program_map(address_map &map);
	void ds3xdsp_data_map(address_map &map);
	void ds3xdsp_program_map(address_map &map);
	void dsk2_dsp32_map(address_map &map);
	void dsk_dsp32_map(address_map &map);
	void multisync2_68k_map(address_map &map);
	void multisync2_gsp_map(address_map &map);
	void multisync_68k_map(address_map &map);
	void multisync_gsp_map(address_map &map);

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

	uint8_t                   m_hd34010_host_access;

	optional_shared_ptr<uint16_t> m_msp_ram;

	uint16_t *                m_dsk_ram;
	uint16_t *                m_dsk_rom;
	optional_device<eeprom_parallel_28xx_device> m_dsk_10c;
	optional_device<eeprom_parallel_28xx_device> m_dsk_30c;
	uint8_t                   m_dsk_pio_access;

	uint16_t *                m_m68k_slapstic_base;
	uint16_t *                m_m68k_sloop_alt_base;

	required_device<timekeeper_device> m_200e;
	required_device<eeprom_parallel_28xx_device> m_210e;

	optional_shared_ptr<uint16_t> m_adsp_data_memory;
	optional_shared_ptr<uint32_t> m_adsp_pgm_memory;

	optional_shared_ptr<uint16_t> m_ds3sdsp_data_memory;
	optional_shared_ptr<uint32_t> m_ds3sdsp_pgm_memory;
	optional_shared_ptr<uint32_t> m_ds3xdsp_pgm_memory;

	optional_shared_ptr<uint32_t> m_dsp32_ram;

	uint16_t *                m_gsp_protection;

	uint16_t *                m_gsp_speedup_addr[2];
	offs_t                  m_gsp_speedup_pc;

	uint16_t *                m_msp_speedup_addr;
	offs_t                  m_msp_speedup_pc;

	uint16_t *                m_ds3_speedup_addr;
	offs_t                  m_ds3_speedup_pc;
	offs_t                  m_ds3_transfer_pc;

	uint32_t *                m_rddsp32_sync[2];

	uint32_t                  m_gsp_speedup_count[4];
	uint32_t                  m_msp_speedup_count[4];
	uint32_t                  m_adsp_speedup_count[4];

	uint8_t                   m_gsp_multisync;
	optional_shared_ptr<uint8_t>  m_gsp_vram;
	optional_shared_ptr<uint16_t> m_gsp_control_lo;
	optional_shared_ptr<uint16_t> m_gsp_control_hi;
	optional_shared_ptr<uint16_t> m_gsp_paletteram_lo;
	optional_shared_ptr<uint16_t> m_gsp_paletteram_hi;

	required_ioport m_in0;
	optional_ioport m_sw1;
	required_ioport m_a80000;
	optional_ioport_array<4> m_12badc;

	/* machine state */
	uint8_t                   m_irq_state;
	uint8_t                   m_gsp_irq_state;
	uint8_t                   m_msp_irq_state;
	uint8_t                   m_adsp_irq_state;
	uint8_t                   m_ds3sdsp_irq_state;
	uint8_t                   m_duart_irq_state;

	uint8_t                   m_last_gsp_shiftreg;

	uint8_t                   m_m68k_zp1;
	uint8_t                   m_m68k_zp2;
	uint8_t                   m_m68k_adsp_buffer_bank;

	uint8_t                   m_adsp_halt;
	uint8_t                   m_adsp_br;
	uint8_t                   m_adsp_xflag;
	uint16_t                  m_adsp_sim_address;
	uint16_t                  m_adsp_som_address;
	uint32_t                  m_adsp_eprom_base;

	required_region_ptr<uint16_t> m_sim_memory;
	uint16_t                  m_som_memory[0x8000/2];
	uint16_t *                m_adsp_pgm_memory_word;

	uint16_t *                m_ds3_sdata_memory;
	uint32_t                  m_ds3_sdata_memory_size;

	uint8_t                   m_ds3_gcmd;
	uint8_t                   m_ds3_gflag;
	uint8_t                   m_ds3_g68irqs;
	uint8_t                   m_ds3_gfirqs;
	uint8_t                   m_ds3_g68flag;
	uint8_t                   m_ds3_send;
	uint8_t                   m_ds3_reset;
	uint16_t                  m_ds3_gdata;
	uint16_t                  m_ds3_g68data;
	uint32_t                  m_ds3_sim_address;

	uint8_t                   m_ds3_scmd;
	uint8_t                   m_ds3_sflag;
	uint8_t                   m_ds3_s68irqs;
	uint8_t                   m_ds3_sfirqs;
	uint8_t                   m_ds3_s68flag;
	uint8_t                   m_ds3_sreset;
	uint16_t                  m_ds3_sdata;
	uint16_t                  m_ds3_s68data;
	uint32_t                  m_ds3_sdata_address;
	uint16_t                  m_ds3sdsp_regs[32];
	uint16_t                  m_ds3sdsp_timer_en;
	uint16_t                  m_ds3sdsp_sdata;
	optional_device<timer_device> m_ds3sdsp_internal_timer;

	uint16_t                  m_ds3xdsp_regs[32];
	uint16_t                  m_ds3xdsp_timer_en;
	uint16_t                  m_ds3xdsp_sdata;
	optional_device<timer_device> m_ds3xdsp_internal_timer;

	uint16_t                  m_adc_control;
	uint8_t                   m_adc12_select;
	uint8_t                   m_adc12_byte;
	uint16_t                  m_adc12_data;

	uint16_t                  m_hdc68k_last_wheel;
	uint16_t                  m_hdc68k_last_port1;
	uint8_t                   m_hdc68k_wheel_edge;
	uint8_t                   m_hdc68k_shifter_state;

	uint8_t                   m_st68k_sloop_bank;
	offs_t                  m_st68k_last_alt_sloop_offset;

	uint8_t                   m_sel_select;
	uint8_t                   m_sel1_data;
	uint8_t                   m_sel2_data;
	uint8_t                   m_sel3_data;
	uint8_t                   m_sel4_data;

	#define MAX_MSP_SYNC    16
	uint32_t *                m_dataptr[MAX_MSP_SYNC];
	uint32_t                  m_dataval[MAX_MSP_SYNC];
	int                     m_next_msp_sync;

	/* video state */
	offs_t                  m_vram_mask;

	uint8_t                   m_shiftreg_enable;

	uint32_t                  m_mask_table[65536 * 4];
	uint8_t *                 m_gsp_shiftreg_source;

	int8_t                    m_gfx_finescroll;
	uint8_t                   m_gfx_palettebank;
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

	uint8_t               m_sound_int_state;
	uint8_t               m_video_int_state;

	optional_device<palette_device> m_palette;
	int get_hblank(screen_device &screen) const { return (screen.hpos() > (screen.width() * 9 / 10)); }
	optional_device<atari_slapstic_device> m_slapstic_device;

	optional_device<rs232_port_device> m_rs232;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

class harddriv_sound_board_device :  public device_t
{
public:
	// construction/destruction
	harddriv_sound_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~harddriv_sound_board_device() {}

	DECLARE_READ16_MEMBER(hd68k_snd_data_r);
	DECLARE_READ16_MEMBER(hd68k_snd_status_r);
	DECLARE_WRITE16_MEMBER(hd68k_snd_data_w);
	DECLARE_WRITE16_MEMBER(hd68k_snd_reset_w);

private:

	DECLARE_READ16_MEMBER(hdsnd68k_data_r);
	DECLARE_WRITE16_MEMBER(hdsnd68k_data_w);
	DECLARE_READ16_MEMBER(hdsnd68k_switches_r);
	DECLARE_READ16_MEMBER(hdsnd68k_320port_r);
	DECLARE_READ16_MEMBER(hdsnd68k_status_r);
	DECLARE_WRITE16_MEMBER(hdsnd68k_latches_w);
	DECLARE_WRITE_LINE_MEMBER(speech_write_w);
	DECLARE_WRITE_LINE_MEMBER(speech_reset_w);
	DECLARE_WRITE_LINE_MEMBER(speech_rate_w);
	DECLARE_WRITE_LINE_MEMBER(cram_enable_w);
	DECLARE_WRITE_LINE_MEMBER(led_w);
	DECLARE_WRITE16_MEMBER(hdsnd68k_speech_w);
	DECLARE_WRITE16_MEMBER(hdsnd68k_irqclr_w);
	DECLARE_READ16_MEMBER(hdsnd68k_320ram_r);
	DECLARE_WRITE16_MEMBER(hdsnd68k_320ram_w);
	DECLARE_READ16_MEMBER(hdsnd68k_320ports_r);
	DECLARE_WRITE16_MEMBER(hdsnd68k_320ports_w);
	DECLARE_READ16_MEMBER(hdsnd68k_320com_r);
	DECLARE_WRITE16_MEMBER(hdsnd68k_320com_w);

	DECLARE_WRITE16_MEMBER(hdsnddsp_dac_w);
	DECLARE_WRITE16_MEMBER(hdsnddsp_comport_w);
	DECLARE_WRITE16_MEMBER(hdsnddsp_mute_w);
	DECLARE_WRITE16_MEMBER(hdsnddsp_gen68kirq_w);
	DECLARE_WRITE16_MEMBER(hdsnddsp_soundaddr_w);
	DECLARE_READ16_MEMBER(hdsnddsp_rom_r);
	DECLARE_READ16_MEMBER(hdsnddsp_comram_r);
	DECLARE_READ16_MEMBER(hdsnddsp_compare_r);

	void driversnd_68k_map(address_map &map);
	void driversnd_dsp_io_map(address_map &map);
	void driversnd_dsp_program_map(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<cpu_device> m_soundcpu;
	required_device<ls259_device> m_latch;
	required_device<dac_word_interface> m_dac;
	required_device<tms32010_device> m_sounddsp;
	required_shared_ptr<uint16_t> m_sounddsp_ram;
	required_region_ptr<uint8_t>  m_sound_rom;

	uint8_t                   m_soundflag;
	uint8_t                   m_mainflag;
	uint16_t                  m_sounddata;
	uint16_t                  m_maindata;

	uint8_t                   m_cramen;
	uint8_t                   m_irq68k;

	offs_t                  m_sound_rom_offs;

	uint16_t                  m_comram[0x400/2];
	uint64_t                  m_last_bio_cycles;

	void update_68k_interrupts();
	TIMER_CALLBACK_MEMBER( delayed_68k_w );

	DECLARE_READ_LINE_MEMBER(hdsnddsp_get_bio);
};

/* Hard Drivin' */

class harddriv_board_device_state :  public harddriv_state
{
public:
	harddriv_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
//  virtual void device_reset();
};

/* Hard Drivin' Compact */

class harddrivc_board_device_state :  public harddriv_state
{
public:
	harddrivc_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
//  virtual void device_reset();
};

/* Race Drivin' */

class racedriv_board_device_state :  public harddriv_state
{
public:
	racedriv_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	racedriv_board_device_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
//  virtual void device_reset();
};

class racedrivb1_board_device_state :  public racedriv_board_device_state
{
public:
	racedrivb1_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};

/* Race Drivin' Compact */

class racedrivc_board_device_state :  public harddriv_state
{
public:
	racedrivc_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	racedrivc_board_device_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
//  virtual void device_reset();
};

class racedrivc1_board_device_state :  public racedrivc_board_device_state
{
public:
	racedrivc1_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};

class racedrivc_panorama_side_board_device_state :  public racedrivc_board_device_state
{
public:
	racedrivc_panorama_side_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
};


/* Stun Runner */

class stunrun_board_device_state :  public harddriv_state
{
public:
	stunrun_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
//  virtual void device_reset();
};

/* Steel Talons */

class steeltal_board_device_state :  public harddriv_state
{
public:
	steeltal_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	steeltal_board_device_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
//  virtual void device_reset();
};

class steeltal1_board_device_state :  public steeltal_board_device_state
{
public:
	steeltal1_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};

class steeltalp_board_device_state :  public steeltal_board_device_state
{
public:
	steeltalp_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};



/* Street Drivin' */

class strtdriv_board_device_state :  public harddriv_state
{
public:
	strtdriv_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
//  virtual void device_reset();
};

/* Hard Drivin' Airbourne */

class hdrivair_board_device_state :  public harddriv_state
{
public:
	hdrivair_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	hdrivair_board_device_state(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
//  virtual void device_reset();
};

class hdrivairp_board_device_state :  public hdrivair_board_device_state
{
public:
	hdrivairp_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	virtual void device_start() override;
};

#endif // MAME_INCLUDES_HARDDRIV_H
