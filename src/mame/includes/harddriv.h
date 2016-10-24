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
#include "machine/mc68681.h"
#include "machine/asic65.h"
#include "machine/timekpr.h"

#define HARDDRIV_MASTER_CLOCK   XTAL_32MHz
#define HARDDRIV_GSP_CLOCK      XTAL_48MHz

extern const device_type HARDDRIV_DEVICE;
extern const device_type HARDDRIV_BOARD_DEVICE;
extern const device_type HARDDRIVC_BOARD_DEVICE;
extern const device_type RACEDRIV_BOARD_DEVICE;
extern const device_type RACEDRIVC_BOARD_DEVICE;
extern const device_type RACEDRIVC1_BOARD_DEVICE;
extern const device_type RACEDRIVB1_BOARD_DEVICE;
extern const device_type RACEDRIVC_PANORAMA_SIDE_BOARD_DEVICE;
extern const device_type STUNRUN_BOARD_DEVICE;
extern const device_type STEELTAL_BOARD_DEVICE;
extern const device_type STEELTAL1_BOARD_DEVICE;
extern const device_type STEELTALP_BOARD_DEVICE;
extern const device_type STRTDRIV_BOARD_DEVICE;
extern const device_type HDRIVAIR_BOARD_DEVICE;
extern const device_type HDRIVAIRP_BOARD_DEVICE;
extern const device_type HARDDRIV_SOUND_BOARD_DEVICE;


class harddriv_sound_board_device;

class harddriv_state :  public device_t
	/* public device_video_interface */
{
public:
	harddriv_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

	void init_video();
	void hd68k_irq_gen(device_t &device);
	void deferred_adsp_bank_switch(void *ptr, int32_t param);
	void rddsp32_sync_cb(void *ptr, int32_t param);

	/*----------- defined in machine/harddriv.c -----------*/

	/* Driver/Multisync board */
	void hd68k_irq_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void harddriv_duart_irq_handler(int state);

	uint16_t hd68k_gsp_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_gsp_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hd68k_msp_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_msp_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hd68k_a80000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_port0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_adc8_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_adc12_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hdc68k_port1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hda68k_port1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hdc68k_wheel_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_sound_reset_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void hd68k_adc_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hd68k_wr0_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hd68k_wr1_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hd68k_wr2_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hd68k_nwr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hdc68k_wheel_edge_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hd68k_zram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_zram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void hdgsp_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void hdgsp_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void hdgsp_irq_gen(int state);
	void hdmsp_irq_gen(int state);

	/* ADSP board */
	uint16_t hd68k_adsp_program_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_adsp_program_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hd68k_adsp_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_adsp_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hd68k_adsp_buffer_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_adsp_buffer_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void hd68k_adsp_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hd68k_adsp_irq_clear_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_adsp_irq_state_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	uint16_t hdadsp_special_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdadsp_special_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t steeltal_dummy_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t rddsp_unmap_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

	/* DS III/IV board */
	void hd68k_ds3_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_ds3_girq_state_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	uint16_t hd68k_ds3_gdata_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_ds3_gdata_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hdds3_special_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdds3_special_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hdds3_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdds3_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hd68k_ds3_program_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_ds3_program_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hd68k_ds3_sdata_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_ds3_sdata_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hd68k_ds3_sirq_clear_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_ds3_sirq_state_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	uint16_t hdds3_sdsp_special_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdds3_sdsp_special_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hdds3_sdsp_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdds3_sdsp_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hdds3_xdsp_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdds3_xdsp_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void xsdp_sport1_irq_off_callback(void *ptr, int32_t param);

	uint16_t hdgsp_control_lo_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdgsp_control_lo_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hdgsp_control_hi_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdgsp_control_hi_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hdgsp_vram_2bpp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdgsp_vram_1bpp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hdgsp_vram_2bpp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hdgsp_paletteram_lo_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdgsp_paletteram_lo_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hdgsp_paletteram_hi_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdgsp_paletteram_hi_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	/* DSK board */
	void hddsk_update_pif(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	/* DS III/IV board */
	void ds3sdsp_internal_timer_callback(timer_device &timer, void *ptr, int32_t param);
	void ds3xdsp_internal_timer_callback(timer_device &timer, void *ptr, int32_t param);

	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_driver);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_multisync);

	/*----------- defined in video/harddriv.c -----------*/

	TMS340X0_TO_SHIFTREG_CB_MEMBER(hdgsp_write_to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(hdgsp_read_from_shiftreg);

	void video_int_gen(device_t &device);
	void sound_int_write_line(int state);


	/* DSK board */
	void hd68k_dsk_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_dsk_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_dsk_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_dsk_small_rom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_dsk_rom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_dsk_dsp32_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_dsk_dsp32_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void rddsp32_sync0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void rddsp32_sync1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	/* DSPCOM board */
	void hddspcom_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void rd68k_slapstic_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t rd68k_slapstic_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	/* Game-specific protection */
	void st68k_sloop_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t st68k_sloop_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t st68k_sloop_alt_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void st68k_protosloop_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t st68k_protosloop_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	/* GSP optimizations */
	uint16_t hdgsp_speedup_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdgsp_speedup1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hdgsp_speedup2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t rdgsp_speedup1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void rdgsp_speedup1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	/* MSP optimizations */
	uint16_t hdmsp_speedup_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdmsp_speedup_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	/* ADSP optimizations */
	uint16_t hdadsp_speedup_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hdds3_speedup_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);


	void hdds3sdsp_timer_enable_callback(int state);
	void hdds3sdsp_serial_tx_callback(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t hdds3sdsp_serial_rx_callback(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

	void hdds3xdsp_timer_enable_callback(int state);
	void hdds3xdsp_serial_tx_callback(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t hdds3xdsp_serial_rx_callback(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

protected:
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
	optional_ioport_array<8> m_8badc;
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

	optional_region_ptr<uint16_t> m_ds3_sdata_memory;
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
	uint8_t                   m_adc8_select;
	uint8_t                   m_adc8_data;
	uint8_t                   m_adc12_select;
	uint8_t                   m_adc12_byte;
	uint16_t                  m_adc12_data;

	uint16_t                  m_hdc68k_last_wheel;
	uint16_t                  m_hdc68k_last_port1;
	uint8_t                   m_hdc68k_wheel_edge;
	uint8_t                   m_hdc68k_shifter_state;

	uint8_t                   m_st68k_sloop_bank;
	offs_t                  m_st68k_last_alt_sloop_offset;

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

protected:
	//virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start() override;
	virtual void device_reset() override;
};

class harddriv_sound_board_device :  public device_t
{
public:
	// construction/destruction
	harddriv_sound_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~harddriv_sound_board_device() {}

	uint16_t hd68k_snd_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hd68k_snd_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hd68k_snd_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hd68k_snd_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t hdsnd68k_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdsnd68k_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hdsnd68k_switches_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hdsnd68k_320port_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hdsnd68k_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdsnd68k_latches_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hdsnd68k_speech_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hdsnd68k_irqclr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hdsnd68k_320ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdsnd68k_320ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hdsnd68k_320ports_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdsnd68k_320ports_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hdsnd68k_320com_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hdsnd68k_320com_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	int hdsnddsp_get_bio();

	void hdsnddsp_dac_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hdsnddsp_comport_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hdsnddsp_mute_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hdsnddsp_gen68kirq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hdsnddsp_soundaddr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t hdsnddsp_rom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hdsnddsp_comram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hdsnddsp_compare_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<cpu_device> m_soundcpu;
	required_device<dac_word_interface> m_dac;
	required_device<cpu_device> m_sounddsp;
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
	void delayed_68k_w(void *ptr, int32_t param);
};

/* Hard Drivin' */

class harddriv_board_device_state :  public harddriv_state
{
public:
	harddriv_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
//  virtual void device_reset();
};

/* Hard Drivin' Compact */

class harddrivc_board_device_state :  public harddriv_state
{
public:
	harddrivc_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
//  virtual void device_reset();
};

/* Race Drivin' */

class racedriv_board_device_state :  public harddriv_state
{
public:
	racedriv_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
//  virtual void device_reset();
};

class racedrivb1_board_device_state :  public racedriv_board_device_state
{
public:
	racedrivb1_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		racedriv_board_device_state(mconfig, tag, owner, clock)
		{};

protected:
	virtual void device_start() override;
};

/* Race Drivin' Compact */

class racedrivc_board_device_state :  public harddriv_state
{
public:
	racedrivc_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
//  virtual void device_reset();
};

class racedrivc1_board_device_state :  public racedrivc_board_device_state
{
public:
	racedrivc1_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		racedrivc_board_device_state(mconfig, tag, owner, clock)
		{};
protected:
	virtual void device_start() override;
};

class racedrivc_panorama_side_board_device_state :  public racedrivc_board_device_state
{
public:
	racedrivc_panorama_side_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		racedrivc_board_device_state(mconfig, tag, owner, clock)
		{};
protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
};


/* Stun Runner */

class stunrun_board_device_state :  public harddriv_state
{
public:
	stunrun_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
//  virtual void device_reset();
};

/* Steel Talons */

class steeltal_board_device_state :  public harddriv_state
{
public:
	steeltal_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
//  virtual void device_reset();
};

class steeltal1_board_device_state :  public steeltal_board_device_state
{
public:
	steeltal1_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		steeltal_board_device_state(mconfig, tag, owner, clock)
		{};

protected:
	virtual void device_start() override;
};

class steeltalp_board_device_state :  public steeltal_board_device_state
{
public:
	steeltalp_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		steeltal_board_device_state(mconfig, tag, owner, clock)
		{};

protected:
	virtual void device_start() override;
};



/* Street Drivin' */

class strtdriv_board_device_state :  public harddriv_state
{
public:
	strtdriv_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
//  virtual void device_reset();
};

/* Hard Drivin' Airbourne */

class hdrivair_board_device_state :  public harddriv_state
{
public:
	hdrivair_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
//  virtual void device_reset();
};

class hdrivairp_board_device_state :  public hdrivair_board_device_state
{
public:
	hdrivairp_board_device_state(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		hdrivair_board_device_state(mconfig, tag, owner, clock)
		{};

protected:
	virtual void device_start() override;
};
