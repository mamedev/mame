// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Atari polygon racer games

**************************************************************************/

#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/tms32010/tms32010.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/dsp32/dsp32.h"
#include "audio/atarijsa.h"
#include "sound/dac.h"
#include "machine/atarigen.h"
#include "machine/mc68681.h"
#include "machine/asic65.h"
#include "machine/timekpr.h"

#define HARDDRIV_MASTER_CLOCK   XTAL_32MHz
#define HARDDRIV_GSP_CLOCK      XTAL_48MHz

class harddriv_state : public atarigen_state
{
public:
	harddriv_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_gsp(*this, "gsp"),
			m_msp(*this, "msp"),
			m_adsp(*this, "adsp"),
			m_soundcpu(*this, "soundcpu"),
			m_sounddsp(*this, "sounddsp"),
			m_jsacpu(*this, "jsacpu"),
			m_dsp32(*this, "dsp32"),
			m_ds3sdsp(*this, "ds3sdsp"),
			m_ds3xdsp(*this, "ds3xdsp"),
			m_ds3dac1(*this, "ds3dac1"),
			m_ds3dac2(*this, "ds3dac2"),
			m_jsa(*this, "jsa"),
			m_msp_ram(*this, "msp_ram"),
			m_dsk_10c(*this, "dsk_10c"),
			m_dsk_30c(*this, "dsk_30c"),
			m_200e(*this, "200e"),
			m_210e(*this, "210e"),
			m_adsp_data_memory(*this, "adsp_data"),
			m_adsp_pgm_memory(*this, "adsp_pgm_memory"),
			m_ds3sdsp_data_memory(*this, "ds3sdsp_data"),
			m_ds3sdsp_pgm_memory(*this, "ds3sdsp_pgm"),
			m_ds3xdsp_pgm_memory(*this, "ds3xdsp_pgm"),
			m_sounddsp_ram(*this, "sounddsp_ram"),
			m_gsp_vram(*this, "gsp_vram", 16),
			m_gsp_control_lo(*this, "gsp_control_lo"),
			m_gsp_control_hi(*this, "gsp_control_hi"),
			m_gsp_paletteram_lo(*this, "gsp_palram_lo"),
			m_gsp_paletteram_hi(*this, "gsp_palram_hi"),
			m_ds3sdsp_internal_timer(*this, "ds3sdsp_timer"),
			m_ds3xdsp_internal_timer(*this, "ds3xdsp_timer"),
			m_dac(*this, "dac"),
			m_duart(*this, "duartn68681"),
			m_asic65(*this, "asic65"),
			m_slapstic_device(*this, "slapstic")
			{}

	required_device<cpu_device> m_maincpu;
	required_device<tms34010_device> m_gsp;
	optional_device<tms34010_device> m_msp;
	required_device<adsp21xx_device> m_adsp;
	optional_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_sounddsp;
	optional_device<cpu_device> m_jsacpu;
	optional_device<dsp32c_device> m_dsp32;
	optional_device<adsp2105_device> m_ds3sdsp;
	optional_device<adsp2105_device> m_ds3xdsp;
	optional_device<dac_device> m_ds3dac1;
	optional_device<dac_device> m_ds3dac2;
	optional_device<atari_jsa_base_device> m_jsa;

	UINT8                   m_hd34010_host_access;
	UINT8                   m_dsk_pio_access;

	optional_shared_ptr<UINT16> m_msp_ram;
	UINT16 *                m_dsk_ram;
	UINT16 *                m_dsk_rom;
	optional_device<eeprom_parallel_28xx_device> m_dsk_10c;
	optional_device<eeprom_parallel_28xx_device> m_dsk_30c;
	UINT16 *                m_m68k_slapstic_base;
	UINT16 *                m_m68k_sloop_alt_base;

	required_device<timekeeper_device> m_200e;
	required_device<eeprom_parallel_28xx_device> m_210e;

	optional_shared_ptr<UINT16> m_adsp_data_memory;
	optional_shared_ptr<UINT32> m_adsp_pgm_memory;

	optional_shared_ptr<UINT16> m_ds3sdsp_data_memory;
	optional_shared_ptr<UINT32> m_ds3sdsp_pgm_memory;
	optional_shared_ptr<UINT32> m_ds3xdsp_pgm_memory;

	UINT16 *                m_gsp_protection;

	UINT16 *                m_gsp_speedup_addr[2];
	offs_t                  m_gsp_speedup_pc;

	UINT16 *                m_msp_speedup_addr;
	offs_t                  m_msp_speedup_pc;

	UINT16 *                m_ds3_speedup_addr;
	offs_t                  m_ds3_speedup_pc;
	offs_t                  m_ds3_transfer_pc;

	UINT32 *                m_rddsp32_sync[2];

	UINT32                  m_gsp_speedup_count[4];
	UINT32                  m_msp_speedup_count[4];
	UINT32                  m_adsp_speedup_count[4];

	optional_shared_ptr<UINT16> m_sounddsp_ram;

	UINT8                   m_gsp_multisync;
	optional_shared_ptr<UINT8> m_gsp_vram;
	optional_shared_ptr<UINT16> m_gsp_control_lo;
	optional_shared_ptr<UINT16> m_gsp_control_hi;
	optional_shared_ptr<UINT16> m_gsp_paletteram_lo;
	optional_shared_ptr<UINT16> m_gsp_paletteram_hi;

	/* machine state */
	UINT8                   m_irq_state;
	UINT8                   m_gsp_irq_state;
	UINT8                   m_msp_irq_state;
	UINT8                   m_adsp_irq_state;
	UINT8                   m_ds3sdsp_irq_state;
	UINT8                   m_duart_irq_state;

	UINT8                   m_last_gsp_shiftreg;

	UINT8                   m_m68k_zp1;
	UINT8                   m_m68k_zp2;
	UINT8                   m_m68k_adsp_buffer_bank;

	UINT8                   m_adsp_halt;
	UINT8                   m_adsp_br;
	UINT8                   m_adsp_xflag;
	UINT16                  m_adsp_sim_address;
	UINT16                  m_adsp_som_address;
	UINT32                  m_adsp_eprom_base;

	UINT16 *                m_sim_memory;
	UINT32                  m_sim_memory_size;
	UINT16                  m_som_memory[0x8000/2];
	UINT16 *                m_adsp_pgm_memory_word;

	UINT16 *                m_ds3_sdata_memory;
	UINT32                  m_ds3_sdata_memory_size;

	UINT8                   m_ds3_gcmd;
	UINT8                   m_ds3_gflag;
	UINT8                   m_ds3_g68irqs;
	UINT8                   m_ds3_gfirqs;
	UINT8                   m_ds3_g68flag;
	UINT8                   m_ds3_send;
	UINT8                   m_ds3_reset;
	UINT16                  m_ds3_gdata;
	UINT16                  m_ds3_g68data;
	UINT32                  m_ds3_sim_address;

	UINT8                   m_ds3_scmd;
	UINT8                   m_ds3_sflag;
	UINT8                   m_ds3_s68irqs;
	UINT8                   m_ds3_sfirqs;
	UINT8                   m_ds3_s68flag;
	UINT8                   m_ds3_sreset;
	UINT16                  m_ds3_sdata;
	UINT16                  m_ds3_s68data;
	UINT32                  m_ds3_sdata_address;
	UINT16                  m_ds3sdsp_regs[32];
	UINT16                  m_ds3sdsp_timer_en;
	UINT16                  m_ds3sdsp_sdata;
	optional_device<timer_device> m_ds3sdsp_internal_timer;

	UINT16                  m_ds3xdsp_regs[32];
	UINT16                  m_ds3xdsp_timer_en;
	UINT16                  m_ds3xdsp_sdata;
	optional_device<timer_device> m_ds3xdsp_internal_timer;

	UINT16                  m_adc_control;
	UINT8                   m_adc8_select;
	UINT8                   m_adc8_data;
	UINT8                   m_adc12_select;
	UINT8                   m_adc12_byte;
	UINT16                  m_adc12_data;

	UINT16                  m_hdc68k_last_wheel;
	UINT16                  m_hdc68k_last_port1;
	UINT8                   m_hdc68k_wheel_edge;
	UINT8                   m_hdc68k_shifter_state;

	UINT8                   m_st68k_sloop_bank;
	offs_t                  m_st68k_last_alt_sloop_offset;

	#define MAX_MSP_SYNC    16
	UINT32 *                m_dataptr[MAX_MSP_SYNC];
	UINT32                  m_dataval[MAX_MSP_SYNC];
	int                     m_next_msp_sync;

	/* audio state */
	UINT8                   m_soundflag;
	UINT8                   m_mainflag;
	UINT16                  m_sounddata;
	UINT16                  m_maindata;

	UINT8                   m_dacmute;
	UINT8                   m_cramen;
	UINT8                   m_irq68k;

	offs_t                  m_sound_rom_offs;

	UINT8 *                 m_rombase;
	UINT32                  m_romsize;
	UINT16                  m_comram[0x400/2];
	UINT64                  m_last_bio_cycles;

	/* video state */
	offs_t                  m_vram_mask;

	UINT8                   m_shiftreg_enable;

	UINT32                  m_mask_table[65536 * 4];
	UINT8 *                 m_gsp_shiftreg_source;

	INT8                    m_gfx_finescroll;
	UINT8                   m_gfx_palettebank;
	virtual void update_interrupts();
	DECLARE_READ16_MEMBER(steeltal_dummy_r);
	DECLARE_READ32_MEMBER(rddsp_unmap_r);
	DECLARE_READ16_MEMBER(hd68k_snd_data_r);
	DECLARE_READ16_MEMBER(hd68k_snd_status_r);
	DECLARE_WRITE16_MEMBER(hd68k_snd_data_w);
	DECLARE_WRITE16_MEMBER(hd68k_snd_reset_w);
	DECLARE_READ16_MEMBER(hdsnd68k_data_r);
	DECLARE_WRITE16_MEMBER(hdsnd68k_data_w);
	DECLARE_READ16_MEMBER(hdsnd68k_switches_r);
	DECLARE_READ16_MEMBER(hdsnd68k_320port_r);
	DECLARE_READ16_MEMBER(hdsnd68k_status_r);
	DECLARE_WRITE16_MEMBER(hdsnd68k_latches_w);
	DECLARE_WRITE16_MEMBER(hdsnd68k_speech_w);
	DECLARE_WRITE16_MEMBER(hdsnd68k_irqclr_w);
	DECLARE_READ16_MEMBER(hdsnd68k_320ram_r);
	DECLARE_WRITE16_MEMBER(hdsnd68k_320ram_w);
	DECLARE_READ16_MEMBER(hdsnd68k_320ports_r);
	DECLARE_WRITE16_MEMBER(hdsnd68k_320ports_w);
	DECLARE_READ16_MEMBER(hdsnd68k_320com_r);
	DECLARE_WRITE16_MEMBER(hdsnd68k_320com_w);
	DECLARE_READ16_MEMBER(hdsnddsp_get_bio);
	DECLARE_WRITE16_MEMBER(hdsnddsp_comport_w);
	DECLARE_WRITE16_MEMBER(hdsnddsp_mute_w);
	DECLARE_WRITE16_MEMBER(hdsnddsp_gen68kirq_w);
	DECLARE_WRITE16_MEMBER(hdsnddsp_soundaddr_w);
	DECLARE_READ16_MEMBER(hdsnddsp_rom_r);
	DECLARE_READ16_MEMBER(hdsnddsp_comram_r);
	DECLARE_READ16_MEMBER(hdsnddsp_compare_r);
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
	DECLARE_DRIVER_INIT(strtdriv);
	DECLARE_DRIVER_INIT(harddrivc);
	DECLARE_DRIVER_INIT(hdrivairp);
	DECLARE_DRIVER_INIT(racedriv);
	DECLARE_DRIVER_INIT(hdrivair);
	DECLARE_DRIVER_INIT(steeltal1);
	DECLARE_DRIVER_INIT(racedrivc);
	DECLARE_DRIVER_INIT(steeltal);
	DECLARE_DRIVER_INIT(racedrivc1);
	DECLARE_DRIVER_INIT(racedrivb1);
	DECLARE_DRIVER_INIT(harddriv);
	DECLARE_DRIVER_INIT(steeltalp);
	DECLARE_DRIVER_INIT(stunrun);
	DECLARE_MACHINE_START(harddriv);
	DECLARE_MACHINE_RESET(harddriv);
	DECLARE_VIDEO_START(harddriv);
	INTERRUPT_GEN_MEMBER(hd68k_irq_gen);
	TIMER_CALLBACK_MEMBER(deferred_adsp_bank_switch);
	TIMER_CALLBACK_MEMBER(rddsp32_sync_cb);
	DECLARE_WRITE16_MEMBER(hdsnddsp_dac_w);
	optional_device<dac_device> m_dac;
	required_device<mc68681_device> m_duart;
	optional_device<asic65_device> m_asic65;
	DECLARE_WRITE_LINE_MEMBER(harddriv_duart_irq_handler);

	/*----------- defined in audio/harddriv.c -----------*/

	void hdsnd_init();
	void update_68k_interrupts();
	TIMER_CALLBACK_MEMBER( delayed_68k_w );

	/*----------- defined in machine/harddriv.c -----------*/

	/* Driver/Multisync board */
	DECLARE_WRITE16_MEMBER( hd68k_irq_ack_w );

	DECLARE_READ16_MEMBER( hd68k_gsp_io_r );
	DECLARE_WRITE16_MEMBER( hd68k_gsp_io_w );

	DECLARE_READ16_MEMBER( hd68k_msp_io_r );
	DECLARE_WRITE16_MEMBER( hd68k_msp_io_w );

	DECLARE_READ16_MEMBER( hd68k_port0_r );
	DECLARE_READ16_MEMBER( hd68k_adc8_r );
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

	DECLARE_WRITE16_MEMBER( hdgsp_io_w );

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

	/* DS III/IV board */
	void update_ds3_irq();
	void update_ds3_sirq();

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

	void hdds3sdsp_reset_timer();
	void hdds3xdsp_reset_timer();

	TIMER_CALLBACK_MEMBER( xsdp_sport1_irq_off_callback );

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
	int st68k_sloop_tweak(offs_t offset);
	DECLARE_WRITE16_MEMBER( st68k_sloop_w );
	DECLARE_READ16_MEMBER( st68k_sloop_r );
	DECLARE_READ16_MEMBER( st68k_sloop_alt_r );
	int st68k_protosloop_tweak(offs_t offset);
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

	/*----------- defined in video/harddriv.c -----------*/
	TMS340X0_TO_SHIFTREG_CB_MEMBER(hdgsp_write_to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(hdgsp_read_from_shiftreg);

	void update_palette_bank(int newbank);

	DECLARE_READ16_MEMBER( hdgsp_control_lo_r );
	DECLARE_WRITE16_MEMBER( hdgsp_control_lo_w );
	DECLARE_READ16_MEMBER( hdgsp_control_hi_r );
	DECLARE_WRITE16_MEMBER( hdgsp_control_hi_w );

	DECLARE_READ16_MEMBER( hdgsp_vram_2bpp_r );
	DECLARE_WRITE16_MEMBER( hdgsp_vram_1bpp_w );
	DECLARE_WRITE16_MEMBER( hdgsp_vram_2bpp_w );

	inline void gsp_palette_change(int offset);

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

	optional_device<atari_slapstic_device> m_slapstic_device;
};
