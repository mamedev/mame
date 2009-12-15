/*************************************************************************

    Driver for Atari polygon racer games

**************************************************************************/

#include "cpu/tms34010/tms34010.h"
#include "machine/atarigen.h"

typedef struct _harddriv_state harddriv_state;
struct _harddriv_state
{
	atarigen_state			atarigen;

	const device_config *	maincpu;
	const device_config *	gsp;
	const device_config *	msp;
	const device_config *	adsp;
	const device_config *	soundcpu;
	const device_config *	sounddsp;
	const device_config *	jsacpu;
	const device_config *	dsp32;

	UINT8 					hd34010_host_access;
	UINT8 					dsk_pio_access;

	UINT16 *				msp_ram;
	UINT16 *				dsk_ram;
	UINT16 *				dsk_rom;
	UINT16 *				dsk_zram;
	UINT16 *				m68k_slapstic_base;
	UINT16 *				m68k_sloop_alt_base;

	UINT16 *				adsp_data_memory;
	UINT32 *				adsp_pgm_memory;

	UINT16 *				gsp_protection;
	UINT16 *				stmsp_sync[3];

	UINT16 *				gsp_speedup_addr[2];
	offs_t 					gsp_speedup_pc;

	UINT16 *				msp_speedup_addr;
	offs_t 					msp_speedup_pc;

	UINT16 *				ds3_speedup_addr;
	offs_t 					ds3_speedup_pc;
	offs_t 					ds3_transfer_pc;

	UINT32 *				rddsp32_sync[2];

	UINT32 					gsp_speedup_count[4];
	UINT32 					msp_speedup_count[4];
	UINT32 					adsp_speedup_count[4];

	UINT16 *				sounddsp_ram;

	UINT8 					gsp_multisync;
	UINT8 *					gsp_vram;
	UINT16 *				gsp_control_lo;
	UINT16 *				gsp_control_hi;
	UINT16 *				gsp_paletteram_lo;
	UINT16 *				gsp_paletteram_hi;
	size_t 					gsp_vram_size;

	/* driver state */
	UINT32 *				rddsp32_speedup;
	offs_t 					rddsp32_speedup_pc;

	/* machine state */
	UINT8 					irq_state;
	UINT8 					gsp_irq_state;
	UINT8 					msp_irq_state;
	UINT8 					adsp_irq_state;
	UINT8 					duart_irq_state;

	UINT8 					duart_read_data[16];
	UINT8 					duart_write_data[16];
	UINT8 					duart_output_port;
	const device_config *	duart_timer;

	UINT8 					last_gsp_shiftreg;

	UINT8 					m68k_zp1;
	UINT8					m68k_zp2;
	UINT8 					m68k_adsp_buffer_bank;

	UINT8 					adsp_halt;
	UINT8					adsp_br;
	UINT8 					adsp_xflag;
	UINT16 					adsp_sim_address;
	UINT16 					adsp_som_address;
	UINT32 					adsp_eprom_base;

	UINT16 *				sim_memory;
	UINT32 					sim_memory_size;
	UINT16 					som_memory[0x8000/2];
	UINT16 *				adsp_pgm_memory_word;

	UINT8 					ds3_gcmd;
	UINT8					ds3_gflag;
	UINT8					ds3_g68irqs;
	UINT8					ds3_gfirqs;
	UINT8					ds3_g68flag;
	UINT8					ds3_send;
	UINT8					ds3_reset;
	UINT16 					ds3_gdata;
	UINT16					ds3_g68data;
	UINT32 					ds3_sim_address;

	UINT16 					adc_control;
	UINT8 					adc8_select;
	UINT8 					adc8_data;
	UINT8 					adc12_select;
	UINT8 					adc12_byte;
	UINT16 					adc12_data;

	UINT16 					hdc68k_last_wheel;
	UINT16 					hdc68k_last_port1;
	UINT8 					hdc68k_wheel_edge;
	UINT8 					hdc68k_shifter_state;

	UINT8 					st68k_sloop_bank;
	offs_t 					st68k_last_alt_sloop_offset;

	#define MAX_MSP_SYNC	16
	UINT32 *				dataptr[MAX_MSP_SYNC];
	UINT32 					dataval[MAX_MSP_SYNC];
	int 					next_msp_sync;

	/* audio state */
	UINT8 					soundflag;
	UINT8 					mainflag;
	UINT16 					sounddata;
	UINT16 					maindata;

	UINT8 					dacmute;
	UINT8 					cramen;
	UINT8 					irq68k;

	offs_t 					sound_rom_offs;

	UINT8 *					rombase;
	UINT32 					romsize;
	UINT16 					comram[0x400/2];
	UINT64 					last_bio_cycles;

	/* video state */
	offs_t 					vram_mask;

	UINT8 					shiftreg_enable;

	UINT32 					mask_table[65536 * 4];
	UINT8 *					gsp_shiftreg_source;

	INT8 					gfx_finescroll;
	UINT8	 				gfx_palettebank;
};


/*----------- defined in machine/harddriv.c -----------*/

/* Driver/Multisync board */
MACHINE_START( harddriv );
MACHINE_RESET( harddriv );

INTERRUPT_GEN( hd68k_irq_gen );
WRITE16_HANDLER( hd68k_irq_ack_w );
void hdgsp_irq_gen(const device_config *device, int state);
void hdmsp_irq_gen(const device_config *device, int state);

READ16_HANDLER( hd68k_gsp_io_r );
WRITE16_HANDLER( hd68k_gsp_io_w );

READ16_HANDLER( hd68k_msp_io_r );
WRITE16_HANDLER( hd68k_msp_io_w );

READ16_HANDLER( hd68k_port0_r );
READ16_HANDLER( hd68k_adc8_r );
READ16_HANDLER( hd68k_adc12_r );
READ16_HANDLER( hdc68k_port1_r );
READ16_HANDLER( hda68k_port1_r );
READ16_HANDLER( hdc68k_wheel_r );
READ16_HANDLER( hd68k_sound_reset_r );

WRITE16_HANDLER( hd68k_adc_control_w );
WRITE16_HANDLER( hd68k_wr0_write );
WRITE16_HANDLER( hd68k_wr1_write );
WRITE16_HANDLER( hd68k_wr2_write );
WRITE16_HANDLER( hd68k_nwr_w );
WRITE16_HANDLER( hdc68k_wheel_edge_reset_w );

READ16_HANDLER( hd68k_zram_r );
WRITE16_HANDLER( hd68k_zram_w );

TIMER_DEVICE_CALLBACK( hd68k_duart_callback );
READ16_HANDLER( hd68k_duart_r );
WRITE16_HANDLER( hd68k_duart_w );

WRITE16_HANDLER( hdgsp_io_w );

WRITE16_HANDLER( hdgsp_protection_w );

WRITE16_HANDLER( stmsp_sync0_w );
WRITE16_HANDLER( stmsp_sync1_w );
WRITE16_HANDLER( stmsp_sync2_w );

/* ADSP board */
READ16_HANDLER( hd68k_adsp_program_r );
WRITE16_HANDLER( hd68k_adsp_program_w );

READ16_HANDLER( hd68k_adsp_data_r );
WRITE16_HANDLER( hd68k_adsp_data_w );

READ16_HANDLER( hd68k_adsp_buffer_r );
WRITE16_HANDLER( hd68k_adsp_buffer_w );

WRITE16_HANDLER( hd68k_adsp_control_w );
WRITE16_HANDLER( hd68k_adsp_irq_clear_w );
READ16_HANDLER( hd68k_adsp_irq_state_r );

READ16_HANDLER( hdadsp_special_r );
WRITE16_HANDLER( hdadsp_special_w );

/* DS III board */
WRITE16_HANDLER( hd68k_ds3_control_w );
READ16_HANDLER( hd68k_ds3_girq_state_r );
READ16_HANDLER( hd68k_ds3_sirq_state_r );
READ16_HANDLER( hd68k_ds3_gdata_r );
WRITE16_HANDLER( hd68k_ds3_gdata_w );
READ16_HANDLER( hd68k_ds3_sdata_r );
WRITE16_HANDLER( hd68k_ds3_sdata_w );

READ16_HANDLER( hdds3_special_r );
WRITE16_HANDLER( hdds3_special_w );
READ16_HANDLER( hdds3_control_r );
WRITE16_HANDLER( hdds3_control_w );

READ16_HANDLER( hd68k_ds3_program_r );
WRITE16_HANDLER( hd68k_ds3_program_w );

/* DSK board */
void hddsk_update_pif(const device_config *device, UINT32 pins);
WRITE16_HANDLER( hd68k_dsk_control_w );
READ16_HANDLER( hd68k_dsk_ram_r );
WRITE16_HANDLER( hd68k_dsk_ram_w );
READ16_HANDLER( hd68k_dsk_zram_r );
WRITE16_HANDLER( hd68k_dsk_zram_w );
READ16_HANDLER( hd68k_dsk_small_rom_r );
READ16_HANDLER( hd68k_dsk_rom_r );
WRITE16_HANDLER( hd68k_dsk_dsp32_w );
READ16_HANDLER( hd68k_dsk_dsp32_r );
WRITE32_HANDLER( rddsp32_sync0_w );
WRITE32_HANDLER( rddsp32_sync1_w );

/* DSPCOM board */
WRITE16_HANDLER( hddspcom_control_w );

WRITE16_HANDLER( rd68k_slapstic_w );
READ16_HANDLER( rd68k_slapstic_r );

/* Game-specific protection */
WRITE16_HANDLER( st68k_sloop_w );
READ16_HANDLER( st68k_sloop_r );
READ16_HANDLER( st68k_sloop_alt_r );
WRITE16_HANDLER( st68k_protosloop_w );
READ16_HANDLER( st68k_protosloop_r );

/* GSP optimizations */
READ16_HANDLER( hdgsp_speedup_r );
WRITE16_HANDLER( hdgsp_speedup1_w );
WRITE16_HANDLER( hdgsp_speedup2_w );
READ16_HANDLER( rdgsp_speedup1_r );
WRITE16_HANDLER( rdgsp_speedup1_w );

/* MSP optimizations */
READ16_HANDLER( hdmsp_speedup_r );
WRITE16_HANDLER( hdmsp_speedup_w );
READ16_HANDLER( stmsp_speedup_r );

/* ADSP optimizations */
READ16_HANDLER( hdadsp_speedup_r );
READ16_HANDLER( hdds3_speedup_r );


/*----------- defined in audio/harddriv.c -----------*/

void hdsnd_init(running_machine *machine);

READ16_HANDLER( hd68k_snd_data_r );
READ16_HANDLER( hd68k_snd_status_r );
WRITE16_HANDLER( hd68k_snd_data_w );
WRITE16_HANDLER( hd68k_snd_reset_w );

READ16_HANDLER( hdsnd68k_data_r );
WRITE16_HANDLER( hdsnd68k_data_w );

READ16_HANDLER( hdsnd68k_switches_r );
READ16_HANDLER( hdsnd68k_320port_r );
READ16_HANDLER( hdsnd68k_status_r );

WRITE16_HANDLER( hdsnd68k_latches_w );
WRITE16_HANDLER( hdsnd68k_speech_w );
WRITE16_HANDLER( hdsnd68k_irqclr_w );

READ16_HANDLER( hdsnd68k_320ram_r );
WRITE16_HANDLER( hdsnd68k_320ram_w );
READ16_HANDLER( hdsnd68k_320ports_r );
WRITE16_HANDLER( hdsnd68k_320ports_w );
READ16_HANDLER( hdsnd68k_320com_r );
WRITE16_HANDLER( hdsnd68k_320com_w );

READ16_HANDLER( hdsnddsp_get_bio );

WRITE16_DEVICE_HANDLER( hdsnddsp_dac_w );
WRITE16_HANDLER( hdsnddsp_comport_w );
WRITE16_HANDLER( hdsnddsp_mute_w );
WRITE16_HANDLER( hdsnddsp_gen68kirq_w );
WRITE16_HANDLER( hdsnddsp_soundaddr_w );

READ16_HANDLER( hdsnddsp_rom_r );
READ16_HANDLER( hdsnddsp_comram_r );
READ16_HANDLER( hdsnddsp_compare_r );


/*----------- defined in video/harddriv.c -----------*/

VIDEO_START( harddriv );
void hdgsp_write_to_shiftreg(const address_space *space, UINT32 address, UINT16 *shiftreg);
void hdgsp_read_from_shiftreg(const address_space *space, UINT32 address, UINT16 *shiftreg);

READ16_HANDLER( hdgsp_control_lo_r );
WRITE16_HANDLER( hdgsp_control_lo_w );
READ16_HANDLER( hdgsp_control_hi_r );
WRITE16_HANDLER( hdgsp_control_hi_w );

READ16_HANDLER( hdgsp_vram_2bpp_r );
WRITE16_HANDLER( hdgsp_vram_1bpp_w );
WRITE16_HANDLER( hdgsp_vram_2bpp_w );

READ16_HANDLER( hdgsp_paletteram_lo_r );
WRITE16_HANDLER( hdgsp_paletteram_lo_w );
READ16_HANDLER( hdgsp_paletteram_hi_r );
WRITE16_HANDLER( hdgsp_paletteram_hi_w );

void harddriv_scanline_driver(const device_config *screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);
void harddriv_scanline_multisync(const device_config *screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);
