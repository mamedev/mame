// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef _INCLUDES_N64_H_
#define _INCLUDES_N64_H_

#include "cpu/rsp/rsp.h"
#include "sound/dmadac.h"

/*----------- forward decls -----------*/

/*----------- driver state -----------*/

class n64_rdp;

class n64_state : public driver_device
{
public:
	n64_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	/* video-related */
	n64_rdp *m_rdp;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void n64_machine_stop();

	UINT32 screen_update_n64(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_n64(screen_device &screen, bool state);
	required_device<cpu_device> m_maincpu;
};

/*----------- devices -----------*/

#define MCFG_N64_PERIPHS_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, N64PERIPH, 0)

#define AUDIO_DMA_DEPTH     2

struct n64_savable_data_t
{
	UINT8 sram[0x20000];
	UINT8 eeprom[2048];
	UINT8 mempak[2][0x8000];
};

class n64_periphs : public device_t,
					public device_video_interface
{
private:
	struct AUDIO_DMA
	{
		UINT32 address;
		UINT32 length;
	};

public:
	// construction/destruction
	n64_periphs(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ32_MEMBER( is64_r );
	DECLARE_WRITE32_MEMBER( is64_w );
	DECLARE_READ32_MEMBER( open_r );
	DECLARE_WRITE32_MEMBER( open_w );
	DECLARE_READ32_MEMBER( rdram_reg_r );
	DECLARE_WRITE32_MEMBER( rdram_reg_w );
	DECLARE_READ32_MEMBER( mi_reg_r );
	DECLARE_WRITE32_MEMBER( mi_reg_w );
	DECLARE_READ32_MEMBER( vi_reg_r );
	DECLARE_WRITE32_MEMBER( vi_reg_w );
	DECLARE_READ32_MEMBER( ai_reg_r );
	DECLARE_WRITE32_MEMBER( ai_reg_w );
	DECLARE_READ32_MEMBER( pi_reg_r );
	DECLARE_WRITE32_MEMBER( pi_reg_w );
	DECLARE_READ32_MEMBER( ri_reg_r );
	DECLARE_WRITE32_MEMBER( ri_reg_w );
	DECLARE_READ32_MEMBER( si_reg_r );
	DECLARE_WRITE32_MEMBER( si_reg_w );
	DECLARE_READ32_MEMBER( dd_reg_r );
	DECLARE_WRITE32_MEMBER( dd_reg_w );
	DECLARE_READ32_MEMBER( pif_ram_r );
	DECLARE_WRITE32_MEMBER( pif_ram_w );
	TIMER_CALLBACK_MEMBER(reset_timer_callback);
	TIMER_CALLBACK_MEMBER(vi_scanline_callback);
	TIMER_CALLBACK_MEMBER(ai_timer_callback);
	TIMER_CALLBACK_MEMBER(pi_dma_callback);
	TIMER_CALLBACK_MEMBER(si_dma_callback);
	DECLARE_READ32_MEMBER( dp_reg_r );
	DECLARE_WRITE32_MEMBER( dp_reg_w );
	DECLARE_READ32_MEMBER( sp_reg_r );
	DECLARE_WRITE32_MEMBER( sp_reg_w );
	DECLARE_WRITE32_MEMBER(sp_set_status);
	void signal_rcp_interrupt(int interrupt);
	void check_interrupts();

	void ai_timer_tick();
	void pi_dma_tick();
	void si_dma_tick();
	void vi_scanline_tick();
	void reset_tick();
	void video_update(bitmap_rgb32 &bitmap);

	// Video Interface (VI) registers
	UINT32 vi_width;
	UINT32 vi_origin;
	UINT32 vi_control;
	UINT32 vi_blank;
	UINT32 vi_hstart;
	UINT32 vi_vstart;
	UINT32 vi_xscale;
	UINT32 vi_yscale;
	UINT32 vi_burst;
	UINT32 vi_vsync;
	UINT32 vi_hsync;
	UINT32 vi_leap;
	UINT32 vi_intr;
	UINT32 vi_vburst;
	UINT8 field;

	/* nvram-specific for MESS */
	device_t *m_nvram_image;

	n64_savable_data_t m_save_data;

	UINT32 cart_length;

	bool dd_present;
	bool disk_present;
	bool cart_present;

	// Mouse X2/Y2 for delta position
	int mouse_x2[4];
	int mouse_y2[4];

	void poll_reset_button(bool button);

	UINT32 dp_clock;
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	address_space *mem_map;
	device_t *maincpu;
	device_t *rspcpu;

	void clear_rcp_interrupt(int interrupt);

	bool reset_held;
	emu_timer *reset_timer;

	UINT8 is64_buffer[0x10000];

	// Video interface (VI) registers and functions
	emu_timer *vi_scanline_timer;

	// Audio Interface (AI) registers and functions
	void ai_dma();
	AUDIO_DMA *ai_fifo_get_top();
	void ai_fifo_push(UINT32 address, UINT32 length);
	void ai_fifo_pop();

	dmadac_sound_device *ai_dac[2];
	UINT32 ai_dram_addr;
	UINT32 ai_len;
	UINT32 ai_control;
	int ai_dacrate;
	int ai_bitrate;
	UINT32 ai_status;

	emu_timer *ai_timer;

	AUDIO_DMA ai_fifo[AUDIO_DMA_DEPTH];
	int ai_fifo_wpos;
	int ai_fifo_rpos;
	int ai_fifo_num;

	// Memory Interface (MI) registers
	UINT32 mi_version;
	UINT32 mi_interrupt;
	UINT32 mi_intr_mask;
	UINT32 mi_mode;

	// RDRAM Interface (RI) registers
	UINT32 rdram_regs[10];
	UINT32 ri_regs[8];

	// RSP Interface (SP) registers
	void sp_dma(int direction);

	UINT32 sp_mem_addr;
	UINT32 sp_dram_addr;
	int sp_dma_length;
	int sp_dma_count;
	int sp_dma_skip;
	UINT32 sp_semaphore;

	// Disk Drive (DD) registers and functions
	void dd_set_zone_and_track_offset();
	void dd_update_bm();
	void dd_write_sector();
	void dd_read_sector();
	void dd_read_C2();
	UINT32 dd_buffer[256];
	UINT32 dd_sector_data[64];
	UINT32 dd_ram_seq_data[16];
	UINT32 dd_data_reg;
	UINT32 dd_status_reg;
	UINT32 dd_track_reg;
	UINT32 dd_buf_status_reg;
	UINT32 dd_sector_err_reg;
	UINT32 dd_seq_status_reg;
	UINT32 dd_seq_ctrl_reg;
	UINT32 dd_sector_reg;
	UINT32 dd_reset_reg;
	UINT32 dd_current_reg;
	bool dd_bm_reset_held;
	bool dd_write;
	UINT8 dd_int;
	UINT8 dd_start_block;
	UINT8 dd_start_sector;
	UINT8 dd_sectors_per_block;
	UINT8 dd_sector_size;
	UINT8 dd_zone;
	UINT32 dd_track_offset;

	// Peripheral Interface (PI) registers and functions
	void pi_dma();
	emu_timer *pi_dma_timer;
	UINT32 pi_dram_addr;
	UINT32 pi_cart_addr;
	UINT32 pi_rd_len;
	UINT32 pi_wr_len;
	UINT32 pi_status;
	UINT32 pi_bsd_dom1_lat;
	UINT32 pi_bsd_dom1_pwd;
	UINT32 pi_bsd_dom1_pgs;
	UINT32 pi_bsd_dom1_rls;
	UINT32 pi_bsd_dom2_lat;
	UINT32 pi_bsd_dom2_pwd;
	UINT32 pi_bsd_dom2_pgs;
	UINT32 pi_bsd_dom2_rls;
	UINT32 pi_dma_dir;

	// Serial Interface (SI) registers and functions
	emu_timer *si_dma_timer;
	void pif_dma(int direction);
	void handle_pif();
	int pif_channel_handle_command(int channel, int slength, UINT8 *sdata, int rlength, UINT8 *rdata);
	UINT8 calc_mempak_crc(UINT8 *buffer, int length);
	UINT8 pif_ram[0x40];
	UINT8 pif_cmd[0x40];
	UINT32 si_dram_addr;
	UINT32 si_pif_addr;
	UINT32 si_pif_addr_rd64b;
	UINT32 si_pif_addr_wr64b;
	UINT32 si_status;
	UINT32 cic_status;
	int cic_type;

	n64_savable_data_t savable_data;

	// Video Interface (VI) functions
	void vi_recalculate_resolution();
	void video_update16(bitmap_rgb32 &bitmap);
	void video_update32(bitmap_rgb32 &bitmap);
	UINT8 random_seed;        // %HACK%, adds 19 each time it's read and is more or less random
	UINT8 get_random() { return random_seed += 0x13; }

	INT32 m_gamma_table[256];
	INT32 m_gamma_dither_table[0x4000];

};

// device type definition
extern const device_type N64PERIPH;

/*----------- defined in video/n64.c -----------*/

#define DACRATE_NTSC    (48681812)
#define DACRATE_PAL (49656530)
#define DACRATE_MPAL    (48628316)

/*----------- defined in machine/n64.c -----------*/

#define SP_INTERRUPT    0x1
#define SI_INTERRUPT    0x2
#define AI_INTERRUPT    0x4
#define VI_INTERRUPT    0x8
#define PI_INTERRUPT    0x10
#define DP_INTERRUPT    0x20

#define SP_STATUS_HALT          0x0001
#define SP_STATUS_BROKE         0x0002
#define SP_STATUS_DMABUSY       0x0004
#define SP_STATUS_DMAFULL       0x0008
#define SP_STATUS_IOFULL        0x0010
#define SP_STATUS_SSTEP         0x0020
#define SP_STATUS_INTR_BREAK    0x0040
#define SP_STATUS_SIGNAL0       0x0080
#define SP_STATUS_SIGNAL1       0x0100
#define SP_STATUS_SIGNAL2       0x0200
#define SP_STATUS_SIGNAL3       0x0400
#define SP_STATUS_SIGNAL4       0x0800
#define SP_STATUS_SIGNAL5       0x1000
#define SP_STATUS_SIGNAL6       0x2000
#define SP_STATUS_SIGNAL7       0x4000

#define DP_STATUS_XBUS_DMA      0x01
#define DP_STATUS_FREEZE        0x02
#define DP_STATUS_FLUSH         0x04
#define DP_STATUS_START_VALID   0x400

#define DD_ASIC_STATUS_DISK_CHANGE   0x00010000
#define DD_ASIC_STATUS_MECHA_ERR     0x00020000
#define DD_ASIC_STATUS_WRPROTECT_ERR 0x00040000
#define DD_ASIC_STATUS_HEAD_RETRACT  0x00080000
#define DD_ASIC_STATUS_MOTOR_OFF     0x00100000
#define DD_ASIC_STATUS_RESET         0x00400000
#define DD_ASIC_STATUS_BUSY          0x00800000
#define DD_ASIC_STATUS_DISK          0x01000000
#define DD_ASIC_STATUS_MECHA_INT     0x02000000
#define DD_ASIC_STATUS_BM_INT        0x04000000
#define DD_ASIC_STATUS_BM_ERROR      0x08000000
#define DD_ASIC_STATUS_C2_XFER       0x10000000
#define DD_ASIC_STATUS_DREQ          0x40000000

#define DD_TRACK_INDEX_LOCK        0x60000000

#define DD_BM_MECHA_INT_RESET 0x01000000
#define DD_BM_XFERBLOCKS      0x02000000
#define DD_BM_DISABLE_C1      0x04000000
#define DD_BM_DISABLE_OR_CHK  0x08000000
#define DD_BM_RESET           0x10000000
#define DD_BM_INT_MASK        0x20000000
#define DD_BM_MODE            0x40000000
#define DD_BM_START           0x80000000

#define DD_BMST_RUNNING       0x80000000
#define DD_BMST_ERROR         0x04000000
#define DD_BMST_MICRO_STATUS  0x02000000
#define DD_BMST_BLOCKS        0x01000000
#define DD_BMST_C1_CORRECT    0x00800000
#define DD_BMST_C1_DOUBLE     0x00400000
#define DD_BMST_C1_SINGLE     0x00200000
#define DD_BMST_C1_ERROR      0x00010000

#define DD_ASIC_ERR_AM_FAIL      0x80000000
#define DD_ASIC_ERR_MICRO_FAIL   0x40000000
#define DD_ASIC_ERR_SPINDLE_FAIL 0x20000000
#define DD_ASIC_ERR_OVER_RUN     0x10000000
#define DD_ASIC_ERR_OFFTRACK     0x08000000
#define DD_ASIC_ERR_NO_DISK      0x04000000
#define DD_ASIC_ERR_CLOCK_UNLOCK 0x02000000
#define DD_ASIC_ERR_SELF_STOP    0x01000000

#define DD_SEQ_MICRO_INT_MASK    0x80000000
#define DD_SEQ_MICRO_PC_ENABLE   0x40000000

#define SECTORS_PER_BLOCK   85
#define BLOCKS_PER_TRACK    2

const unsigned int ddZoneSecSize[16] = {232,216,208,192,176,160,144,128,
										216,208,192,176,160,144,128,112};
const unsigned int ddZoneTrackSize[16] = {158,158,149,149,149,149,149,114,
											158,158,149,149,149,149,149,114};
const unsigned int ddStartOffset[16] =
	{0x0,0x5F15E0,0xB79D00,0x10801A0,0x1523720,0x1963D80,0x1D414C0,0x20BBCE0,
		0x23196E0,0x28A1E00,0x2DF5DC0,0x3299340,0x36D99A0,0x3AB70E0,0x3E31900,0x4149200};



extern UINT32 *n64_sram;
extern UINT32 *rdram;
extern UINT32 *rsp_imem;
extern UINT32 *rsp_dmem;

extern void dp_full_sync(running_machine &machine);

#endif
