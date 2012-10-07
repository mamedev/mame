/* Sega CD / Mega CD */

#include "machine/lc89510.h"




#define READ_MAIN (0x0200)
#define READ_SUB  (0x0300)
#define DMA_PCM  (0x0400)
#define DMA_PRG  (0x0500)
#define DMA_WRAM (0x0700)

#define REG_W_SBOUT  (0x0)
#define REG_W_IFCTRL (0x1)
#define REG_W_DBCL   (0x2)
#define REG_W_DBCH   (0x3)
#define REG_W_DACL   (0x4)
#define REG_W_DACH   (0x5)
#define REG_W_DTTRG  (0x6)
#define REG_W_DTACK  (0x7)
#define REG_W_WAL    (0x8)
#define REG_W_WAH    (0x9)
#define REG_W_CTRL0  (0xA)
#define REG_W_CTRL1  (0xB)
#define REG_W_PTL    (0xC)
#define REG_W_PTH    (0xD)
#define REG_W_CTRL2  (0xE)
#define REG_W_RESET  (0xF)

#define REG_R_COMIN  (0x0)
#define REG_R_IFSTAT (0x1)
#define REG_R_DBCL   (0x2)
#define REG_R_DBCH   (0x3)
#define REG_R_HEAD0  (0x4)
#define REG_R_HEAD1  (0x5)
#define REG_R_HEAD2  (0x6)
#define REG_R_HEAD3  (0x7)
#define REG_R_PTL    (0x8)
#define REG_R_PTH    (0x9)
#define REG_R_WAL    (0xa)
#define REG_R_WAH    (0xb)
#define REG_R_STAT0  (0xc)
#define REG_R_STAT1  (0xd)
#define REG_R_STAT2  (0xe)
#define REG_R_STAT3  (0xf)

#define CMD_STATUS   (0x0)
#define CMD_STOPALL  (0x1)
#define CMD_GETTOC   (0x2)
#define CMD_READ     (0x3)
#define CMD_SEEK     (0x4)
//                   (0x5)
#define CMD_STOP     (0x6)
#define CMD_RESUME   (0x7)
#define CMD_FF       (0x8)
#define CMD_RW       (0x9)
#define CMD_INIT     (0xa)
//                   (0xb)
#define CMD_CLOSE    (0xc)
#define CMD_OPEN     (0xd)
//                   (0xe)
//                   (0xf)


#define TOCCMD_CURPOS    (0x0)
#define TOCCMD_TRKPOS	 (0x1)
#define TOCCMD_CURTRK    (0x2)
#define TOCCMD_LENGTH    (0x3)
#define TOCCMD_FIRSTLAST (0x4)
#define TOCCMD_TRACKADDR (0x5)

struct segacd_t
{
	cdrom_file	*cd;
	const cdrom_toc   *toc;
	UINT32 current_frame;
};



#define SECTOR_SIZE (2352)

#define SET_CDD_DATA_MODE \
	CDD_CONTROL |= 0x0100; \

#define SET_CDD_AUDIO_MODE \
	CDD_CONTROL &= ~0x0100; \

#define STOP_CDC_READ \
	SCD_STATUS_CDC &= ~0x01; \

#define SET_CDC_READ \
	SCD_STATUS_CDC |= 0x01; \

#define SET_CDC_DMA \
	SCD_STATUS_CDC |= 0x08; \

#define STOP_CDC_DMA \
	SCD_STATUS_CDC &= ~0x08; \

#define SCD_READ_ENABLED \
	(SCD_STATUS_CDC & 1)

#define SCD_DMA_ENABLED \
	(SCD_STATUS_CDC & 0x08)

#define CLEAR_CDD_RESULT \
	CDD_MIN = CDD_SEC = CDD_FRAME = CDD_EXT = 0; \

#define CHECK_SCD_LV5_INTERRUPT \
	if (segacd_irq_mask & 0x20) \
	{ \
		machine.device(":segacd:segacd_68k")->execute().set_input_line(5, HOLD_LINE); \
	} \

#define CHECK_SCD_LV4_INTERRUPT \
	if (segacd_irq_mask & 0x10) \
	{ \
		machine.device(":segacd:segacd_68k")->execute().set_input_line(4, HOLD_LINE); \
	} \



// from master
#define CHECK_SCD_LV2_INTERRUPT \
	if (segacd_irq_mask & 0x04) \
	{ \
		machine.device(":segacd:segacd_68k")->execute().set_input_line(2, HOLD_LINE); \
	} \


// irq3 timer
#define CHECK_SCD_LV3_INTERRUPT \
	if (segacd_irq_mask & 0x08) \
	{ \
		machine().device(":segacd:segacd_68k")->execute().set_input_line(3, HOLD_LINE); \
	} \

// gfx convert
#define CHECK_SCD_LV1_INTERRUPT \
	if (segacd_irq_mask & 0x02) \
	{ \
		machine().device(":segacd:segacd_68k")->execute().set_input_line(1, HOLD_LINE); \
	} \

#define CURRENT_TRACK_IS_DATA \
	(segacd.toc->tracks[SCD_CURTRK - 1].trktype != CD_TRACK_AUDIO) \


#define RAM_MODE_2MEG (0)
#define RAM_MODE_1MEG (2)


#define CDD_PLAYINGCDDA	0x0100
#define CDD_READY		0x0400
#define CDD_STOPPED		0x0900

#define SEGACD_IRQ3_TIMER_SPEED (attotime::from_nsec(segacd_irq3_timer_reg*30720))



// the tiles in RAM are 8x8 tiles
// they are referenced in the cell look-up map as either 16x16 or 32x32 tiles (made of 4 / 16 8x8 tiles)

#define SEGACD_BYTES_PER_TILE16 (128)
#define SEGACD_BYTES_PER_TILE32 (512)

#define SEGACD_NUM_TILES16 (0x40000/SEGACD_BYTES_PER_TILE16)
#define SEGACD_NUM_TILES32 (0x40000/SEGACD_BYTES_PER_TILE32)

#define _16x16_SEQUENCE_1  { 8,12,0,4,24,28,16,20, 512+8, 512+12, 512+0, 512+4, 512+24, 512+28, 512+16, 512+20 },
#define _16x16_SEQUENCE_1_FLIP  { 512+20,512+16,512+28,512+24,512+4,512+0, 512+12,512+8, 20,16,28,24,4,0,12,8 },

#define _16x16_SEQUENCE_2  { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32,10*32,11*32,12*32,13*32,14*32,15*32 },
#define _16x16_SEQUENCE_2_FLIP  { 15*32, 14*32, 13*32, 12*32, 11*32, 10*32, 9*32, 8*32, 7*32, 6*32, 5*32, 4*32, 3*32, 2*32, 1*32, 0*32 },


#define _16x16_START \
{ \
	16,16, \
	SEGACD_NUM_TILES16, \
	4, \
	{ 0,1,2,3 }, \

#define _16x16_END \
		8*128 \
}; \

#define _32x32_START \
{ \
	32,32, \
	SEGACD_NUM_TILES32, \
	4, \
	{ 0,1,2,3 }, \


#define _32x32_END \
	8*512 \
}; \



#define _32x32_SEQUENCE_1 \
	{ 8,12,0,4,24,28,16,20, \
	1024+8, 1024+12, 1024+0, 1024+4, 1024+24, 1024+28, 1024+16, 1024+20, \
	2048+8, 2048+12, 2048+0, 2048+4, 2048+24, 2048+28, 2048+16, 2048+20, \
	3072+8, 3072+12, 3072+0, 3072+4, 3072+24, 3072+28, 3072+16, 3072+20  \
	}, \

#define _32x32_SEQUENCE_1_FLIP \
{ 3072+20, 3072+16, 3072+28, 3072+24, 3072+4, 3072+0, 3072+12, 3072+8, \
  2048+20, 2048+16, 2048+28, 2048+24, 2048+4, 2048+0, 2048+12, 2048+8, \
  1024+20, 1024+16, 1024+28, 1024+24, 1024+4, 1024+0, 1024+12, 1024+8, \
  20, 16, 28, 24, 4, 0, 12, 8}, \


#define _32x32_SEQUENCE_2 \
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, \
    	8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32, \
	 16*32,17*32,18*32,19*32,20*32,21*32,22*32,23*32, \
	 24*32,25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32}, \

#define _32x32_SEQUENCE_2_FLIP \
{ 31*32, 30*32, 29*32, 28*32, 27*32, 26*32, 25*32, 24*32, \
  23*32, 22*32, 21*32, 20*32, 19*32, 18*32, 17*32, 16*32, \
  15*32, 14*32, 13*32, 12*32, 11*32, 10*32, 9*32 , 8*32 , \
  7*32 , 6*32 , 5*32 , 4*32 , 3*32 , 2*32 , 1*32 , 0*32}, \


/* 16x16 decodes */
static const gfx_layout sega_16x16_r00_f0_layout =
_16x16_START
	_16x16_SEQUENCE_1
	_16x16_SEQUENCE_2
_16x16_END

static const gfx_layout sega_16x16_r01_f0_layout =
_16x16_START
	_16x16_SEQUENCE_2
	_16x16_SEQUENCE_1_FLIP
_16x16_END

static const gfx_layout sega_16x16_r10_f0_layout =
_16x16_START
	_16x16_SEQUENCE_1_FLIP
	_16x16_SEQUENCE_2_FLIP
_16x16_END

static const gfx_layout sega_16x16_r11_f0_layout =
_16x16_START
	_16x16_SEQUENCE_2_FLIP
	_16x16_SEQUENCE_1
_16x16_END

static const gfx_layout sega_16x16_r00_f1_layout =
_16x16_START
	_16x16_SEQUENCE_1_FLIP
	_16x16_SEQUENCE_2
_16x16_END

static const gfx_layout sega_16x16_r01_f1_layout =
_16x16_START
	_16x16_SEQUENCE_2
	_16x16_SEQUENCE_1
_16x16_END

static const gfx_layout sega_16x16_r10_f1_layout =
_16x16_START
	_16x16_SEQUENCE_1
	_16x16_SEQUENCE_2_FLIP
_16x16_END

static const gfx_layout sega_16x16_r11_f1_layout =
_16x16_START
	_16x16_SEQUENCE_2_FLIP
	_16x16_SEQUENCE_1_FLIP
_16x16_END

/* 32x32 decodes */
static const gfx_layout sega_32x32_r00_f0_layout =
_32x32_START
	_32x32_SEQUENCE_1
	_32x32_SEQUENCE_2
_32x32_END

static const gfx_layout sega_32x32_r01_f0_layout =
_32x32_START
	_32x32_SEQUENCE_2
	_32x32_SEQUENCE_1_FLIP
_32x32_END

static const gfx_layout sega_32x32_r10_f0_layout =
_32x32_START
	_32x32_SEQUENCE_1_FLIP
	_32x32_SEQUENCE_2_FLIP
_32x32_END

static const gfx_layout sega_32x32_r11_f0_layout =
_32x32_START
	_32x32_SEQUENCE_2_FLIP
	_32x32_SEQUENCE_1
_32x32_END

static const gfx_layout sega_32x32_r00_f1_layout =
_32x32_START
	_32x32_SEQUENCE_1_FLIP
	_32x32_SEQUENCE_2
_32x32_END

static const gfx_layout sega_32x32_r01_f1_layout =
_32x32_START
	_32x32_SEQUENCE_2
	_32x32_SEQUENCE_1
_32x32_END

static const gfx_layout sega_32x32_r10_f1_layout =
_32x32_START
	_32x32_SEQUENCE_1
	_32x32_SEQUENCE_2_FLIP
_32x32_END

static const gfx_layout sega_32x32_r11_f1_layout =
_32x32_START
	_32x32_SEQUENCE_2_FLIP
	_32x32_SEQUENCE_1_FLIP
_32x32_END

extern UINT16 a12000_halt_reset_reg;

class sega_segacd_device : public device_t
{
public:
	sega_segacd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, device_type type);

	cpu_device *_segacd_68k_cpu;


	UINT16 segacd_irq_mask;
	UINT16 *segacd_backupram;
	timer_device *stopwatch_timer;
	UINT8 segacd_font_color;
	UINT16* segacd_font_bits;
	UINT16 scd_rammode;
	UINT32 scd_mode_dmna_ret_flags ;

	timer_device *segacd_gfx_conversion_timer;
	timer_device *segacd_irq3_timer;
	//timer_device *segacd_hock_timer;
	timer_device* scd_dma_timer;

	UINT16* segacd_4meg_prgram;  // pointer to SubCPU PrgRAM
	UINT16* segacd_dataram;
	UINT16* segacd_dataram2;
	tilemap_t    *segacd_stampmap[4];


	UINT8 segacd_ram_writeprotect_bits;
	int segacd_4meg_prgbank;// = 0; // which bank the MainCPU can see of the SubCPU PrgRAM
	int segacd_memory_priority_mode;// = 0;
	int segacd_stampsize;

	UINT16 segacd_hint_register;
	UINT16 segacd_imagebuffer_vdot_size;
	UINT16 segacd_imagebuffer_vcell_size;
	UINT16 segacd_imagebuffer_hdot_size;

	int segacd_conversion_active;// = 0;
	UINT16 segacd_stampmap_base_address;
	UINT16 segacd_imagebuffer_start_address;
	UINT16 segacd_imagebuffer_offset;


	UINT16 segacd_comms_flags;// = 0x0000;
	UINT16 segacd_comms_part1[0x8];
	UINT16 segacd_comms_part2[0x8];

	int segacd_redled;// = 0;
	int segacd_greenled;// = 0;
	int segacd_ready;// = 1; // actually set 100ms after startup?
	UINT16 segacd_irq3_timer_reg;

	segacd_t segacd;

	UINT8    SCD_BUFFER[2560];
	UINT32   SCD_STATUS;
	UINT32   SCD_STATUS_CDC;
	INT32    SCD_CURLBA;
	UINT8    SCD_CURTRK;

	UINT16 CDC_DECODE;
	INT16 CDC_DMACNT; // can go negative
	UINT16 CDC_DMA_ADDRC;
	UINT16 CDC_PT;
	UINT16 CDC_WA;
	UINT16 CDC_REG0;
	UINT16 CDC_REG1;
	UINT16 CDC_DMA_ADDR;
	UINT16 CDC_IFSTAT;
	UINT8 CDC_HEADB0;
	UINT8 CDC_HEADB1;
	UINT8 CDC_HEADB2;
	UINT8 CDC_HEADB3;
	UINT8 CDC_STATB0;
	UINT8 CDC_STATB1;
	UINT8 CDC_STATB2;
	UINT8 CDC_STATB3;
	UINT16 CDC_SBOUT;
	UINT16 CDC_IFCTRL;
	UINT8 CDC_CTRLB0;
	UINT8 CDC_CTRLB1;
	UINT8 CDC_CTRLB2;
	UINT8 CDC_BUFFER[(32 * 1024 * 2) + SECTOR_SIZE];

	UINT32 CDD_STATUS;
	UINT32 CDD_MIN;
	UINT32 CDD_SEC;

	UINT8 CDD_RX[10];
	UINT8 CDD_TX[10];
	UINT32 CDD_FRAME;
	UINT32 CDD_EXT;
	UINT16 CDD_CONTROL;
	INT16  CDD_DONE;

	TIMER_DEVICE_CALLBACK_MEMBER( segacd_irq3_timer_callback );
	TIMER_DEVICE_CALLBACK_MEMBER( scd_dma_timer_callback );
	TIMER_DEVICE_CALLBACK_MEMBER( segacd_access_timer_callback );
	TIMER_DEVICE_CALLBACK_MEMBER( segacd_gfx_conversion_timer_callback );

	UINT16 handle_segacd_sub_int_callback(int irqline);

	inline int to_bcd(int val, bool byte);
	inline void write_pixel(running_machine& machine, UINT8 pix, int pixeloffset );
	UINT16 segacd_1meg_mode_word_read(int offset, UINT16 mem_mask);
	void segacd_1meg_mode_word_write(running_machine& machine, int offset, UINT16 data, UINT16 mem_mask, int use_pm);
	void set_data_audio_mode(void);
	void CDD_DoChecksum(void);
	void CDD_Export(void);
	void CDC_UpdateHEAD(void);
	void scd_ctrl_checks(running_machine& machine);
	void scd_advance_current_readpos(void);
	int Read_LBA_To_Buffer(running_machine& machine);
	void CheckCommand(running_machine& machine);
	void CDD_GetStatus(void);
	void CDD_Stop(running_machine &machine);
	void CDD_GetPos(void);
	void CDD_GetTrackPos(void);
	void CDD_GetTrack(void);
	void CDD_Length(void);
	void CDD_FirstLast(void);
	void CDD_GetTrackAdr(void);
	UINT32 getmsf_from_regs(void);
	void CDD_Play(running_machine &machine);
	void CDD_Seek(void);
	void CDD_Pause(running_machine &machine);
	void CDD_Resume(running_machine &machine);
	void CDD_FF(running_machine &machine);
	void CDD_RW(running_machine &machine);
	void CDD_Open(void);
	void CDD_Close(void);
	void CDD_Init(void);
	void CDD_Default(void);
	void CDD_Reset(void);
	void CDC_Reset(void);
	void lc89510_Reset(void);
	void CDC_End_Transfer(running_machine& machine);
	void CDC_Do_DMA(running_machine& machine, int rate);
	UINT16 CDC_Host_r(running_machine& machine, UINT16 type);
	UINT8 CDC_Reg_r(void);
	void CDC_Reg_w(UINT8 data);
	void CDD_Process(running_machine& machine, int reason);
	void CDD_Handle_TOC_Commands(void);
	void CDD_Import(running_machine& machine);

	void segacd_mark_tiles_dirty(running_machine& machine, int offset);
	int segacd_get_active_stampmap_tilemap(void);

	void SCD_GET_TILE_INFO_16x16_1x1( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_32x32_1x1( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_16x16_16x16( int& tile_region, int& tileno, int tile_index );
	void SCD_GET_TILE_INFO_32x32_16x16( int& tile_region, int& tileno, int tile_index );

	TILE_GET_INFO_MEMBER( get_stampmap_16x16_1x1_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_32x32_1x1_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_16x16_16x16_tile_info );
	TILE_GET_INFO_MEMBER( get_stampmap_32x32_16x16_tile_info );

	UINT8 get_stampmap_16x16_1x1_tile_info_pixel(running_machine& machine, int xpos, int ypos);
	UINT8 get_stampmap_32x32_1x1_tile_info_pixel(running_machine& machine, int xpos, int ypos);
	UINT8 get_stampmap_16x16_16x16_tile_info_pixel(running_machine& machine, int xpos, int ypos);
	UINT8 get_stampmap_32x32_16x16_tile_info_pixel(running_machine& machine, int xpos, int ypos);

	WRITE16_MEMBER( scd_a12000_halt_reset_w );
	READ16_MEMBER( scd_a12000_halt_reset_r );
	READ16_MEMBER( scd_a12002_memory_mode_r );
	WRITE8_MEMBER( scd_a12002_memory_mode_w_8_15 );
	WRITE8_MEMBER( scd_a12002_memory_mode_w_0_7 );
	WRITE16_MEMBER( scd_a12002_memory_mode_w );
	READ16_MEMBER( segacd_sub_memory_mode_r );
	WRITE8_MEMBER( segacd_sub_memory_mode_w_8_15 );
	WRITE8_MEMBER( segacd_sub_memory_mode_w_0_7 );
	WRITE16_MEMBER( segacd_sub_memory_mode_w );

	READ16_MEMBER( segacd_comms_flags_r );
	WRITE16_MEMBER( segacd_comms_flags_subcpu_w );
	WRITE16_MEMBER( segacd_comms_flags_maincpu_w );
	READ16_MEMBER( scd_4m_prgbank_ram_r );
	WRITE16_MEMBER( scd_4m_prgbank_ram_w );
	READ16_MEMBER( segacd_comms_main_part1_r );
	WRITE16_MEMBER( segacd_comms_main_part1_w );
	READ16_MEMBER( segacd_comms_main_part2_r );
	WRITE16_MEMBER( segacd_comms_main_part2_w );
	READ16_MEMBER( segacd_comms_sub_part1_r );
	WRITE16_MEMBER( segacd_comms_sub_part1_w );
	READ16_MEMBER( segacd_comms_sub_part2_r );
	WRITE16_MEMBER( segacd_comms_sub_part2_w );

	WRITE16_MEMBER( segacd_cdc_mode_address_w );
	READ16_MEMBER( segacd_cdc_mode_address_r );
	WRITE16_MEMBER( segacd_cdc_data_w );
	READ16_MEMBER( segacd_cdc_data_r );

	READ16_MEMBER( segacd_main_dataram_part1_r );
	WRITE16_MEMBER( segacd_main_dataram_part1_w );

	READ16_MEMBER( scd_hint_vector_r );
	READ16_MEMBER( scd_a12006_hint_register_r );
	WRITE16_MEMBER( scd_a12006_hint_register_w );

	READ16_MEMBER( cdc_data_sub_r );
	READ16_MEMBER( cdc_data_main_r );
	WRITE16_MEMBER( segacd_stopwatch_timer_w );
	READ16_MEMBER( segacd_stopwatch_timer_r );
	READ16_MEMBER( segacd_sub_led_ready_r );
	WRITE16_MEMBER( segacd_sub_led_ready_w );
	READ16_MEMBER( segacd_sub_dataram_part1_r );
	WRITE16_MEMBER( segacd_sub_dataram_part1_w );
	READ16_MEMBER( segacd_sub_dataram_part2_r );
	WRITE16_MEMBER( segacd_sub_dataram_part2_w );
	READ16_MEMBER( segacd_irq_mask_r );
	WRITE16_MEMBER( segacd_irq_mask_w );
	READ16_MEMBER( segacd_cdd_ctrl_r );
	WRITE16_MEMBER( segacd_cdd_ctrl_w );
	READ8_MEMBER( segacd_cdd_rx_r );
	WRITE8_MEMBER( segacd_cdd_tx_w );
	READ16_MEMBER( segacd_stampsize_r );
	WRITE16_MEMBER( segacd_stampsize_w );

	UINT8 read_pixel_from_stampmap( running_machine& machine, bitmap_ind16* srcbitmap, int x, int y);

	WRITE16_MEMBER( segacd_trace_vector_base_address_w );
	READ16_MEMBER( segacd_imagebuffer_vdot_size_r );
	WRITE16_MEMBER( segacd_imagebuffer_vdot_size_w );
	READ16_MEMBER( segacd_stampmap_base_address_r );
	WRITE16_MEMBER( segacd_stampmap_base_address_w );
	READ16_MEMBER( segacd_imagebuffer_start_address_r );
	WRITE16_MEMBER( segacd_imagebuffer_start_address_w );
	READ16_MEMBER( segacd_imagebuffer_offset_r );
	WRITE16_MEMBER( segacd_imagebuffer_offset_w );
	READ16_MEMBER( segacd_imagebuffer_vcell_size_r );
	WRITE16_MEMBER( segacd_imagebuffer_vcell_size_w );
	READ16_MEMBER( segacd_imagebuffer_hdot_size_r );
	WRITE16_MEMBER( segacd_imagebuffer_hdot_size_w );
	READ16_MEMBER( segacd_irq3timer_r );
	WRITE16_MEMBER( segacd_irq3timer_w );
	READ16_MEMBER( cdc_dmaaddr_r );
	WRITE16_MEMBER( cdc_dmaaddr_w );
	READ16_MEMBER( segacd_cdfader_r );
	WRITE16_MEMBER( segacd_cdfader_w );
	READ16_MEMBER( segacd_backupram_r );
	WRITE16_MEMBER( segacd_backupram_w );
	READ16_MEMBER( segacd_font_color_r );
	WRITE16_MEMBER( segacd_font_color_w );
	READ16_MEMBER( segacd_font_converted_r );

protected:
	virtual void device_start();
	virtual void device_reset();

	// optional information overrides
//  virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
private:
//  virtual void device_config_complete();

};


class sega_segacd_us_device : public sega_segacd_device
{
	public:
		sega_segacd_us_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	protected:

};

class sega_segacd_japan_device : public sega_segacd_device
{
	public:
		sega_segacd_japan_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	protected:
//      virtual machine_config_constructor device_mconfig_additions() const;
};

class sega_segacd_europe_device : public sega_segacd_device
{
	public:
		sega_segacd_europe_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	protected:
//      virtual machine_config_constructor device_mconfig_additions() const;
};


extern const device_type SEGA_SEGACD_US;
extern const device_type SEGA_SEGACD_JAPAN;
extern const device_type SEGA_SEGACD_EUROPE;
