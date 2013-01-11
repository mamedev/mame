/* Sega CD / Mega CD */

#include "machine/lc89510.h"
#include "machine/megacdcd.h"


#define RAM_MODE_2MEG (0)
#define RAM_MODE_1MEG (2)

#define DMA_PCM  (0x0400)
#define DMA_PRG  (0x0500)
#define DMA_WRAM (0x0700)

// irq3 timer
#define CHECK_SCD_LV3_INTERRUPT \
	if (lc89510_temp->get_segacd_irq_mask() & 0x08) \
	{ \
		machine().device(":segacd:segacd_68k")->execute().set_input_line(3, HOLD_LINE); \
	}
// from master
#define CHECK_SCD_LV2_INTERRUPT \
	if (lc89510_temp->get_segacd_irq_mask() & 0x04) \
	{ \
		machine.device(":segacd:segacd_68k")->execute().set_input_line(2, HOLD_LINE); \
	}

// gfx convert
#define CHECK_SCD_LV1_INTERRUPT \
	if (lc89510_temp->get_segacd_irq_mask() & 0x02) \
	{ \
		machine().device(":segacd:segacd_68k")->execute().set_input_line(1, HOLD_LINE); \
	}

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
	{ 0,1,2,3 },
#define _16x16_END \
		8*128 \
};
#define _32x32_START \
{ \
	32,32, \
	SEGACD_NUM_TILES32, \
	4, \
	{ 0,1,2,3 },

#define _32x32_END \
	8*512 \
};


#define _32x32_SEQUENCE_1 \
	{ 8,12,0,4,24,28,16,20, \
	1024+8, 1024+12, 1024+0, 1024+4, 1024+24, 1024+28, 1024+16, 1024+20, \
	2048+8, 2048+12, 2048+0, 2048+4, 2048+24, 2048+28, 2048+16, 2048+20, \
	3072+8, 3072+12, 3072+0, 3072+4, 3072+24, 3072+28, 3072+16, 3072+20  \
	},
#define _32x32_SEQUENCE_1_FLIP \
{ 3072+20, 3072+16, 3072+28, 3072+24, 3072+4, 3072+0, 3072+12, 3072+8, \
	2048+20, 2048+16, 2048+28, 2048+24, 2048+4, 2048+0, 2048+12, 2048+8, \
	1024+20, 1024+16, 1024+28, 1024+24, 1024+4, 1024+0, 1024+12, 1024+8, \
	20, 16, 28, 24, 4, 0, 12, 8},

#define _32x32_SEQUENCE_2 \
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, \
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32, \
		16*32,17*32,18*32,19*32,20*32,21*32,22*32,23*32, \
		24*32,25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32},
#define _32x32_SEQUENCE_2_FLIP \
{ 31*32, 30*32, 29*32, 28*32, 27*32, 26*32, 25*32, 24*32, \
	23*32, 22*32, 21*32, 20*32, 19*32, 18*32, 17*32, 16*32, \
	15*32, 14*32, 13*32, 12*32, 11*32, 10*32, 9*32 , 8*32 , \
	7*32 , 6*32 , 5*32 , 4*32 , 3*32 , 2*32 , 1*32 , 0*32},

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
	lc89510_temp_device *lc89510_temp;

	UINT16 *segacd_backupram;
	timer_device *stopwatch_timer;
	UINT8 segacd_font_color;
	UINT16* segacd_font_bits;
	UINT16 scd_rammode;
	UINT32 scd_mode_dmna_ret_flags ;

	timer_device *segacd_gfx_conversion_timer;
	timer_device *segacd_irq3_timer;
	//timer_device *segacd_hock_timer;

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


	TIMER_DEVICE_CALLBACK_MEMBER( segacd_irq3_timer_callback );
	TIMER_DEVICE_CALLBACK_MEMBER( segacd_gfx_conversion_timer_callback );

	UINT16 handle_segacd_sub_int_callback(int irqline);

	inline void write_pixel(running_machine& machine, UINT8 pix, int pixeloffset );
	UINT16 segacd_1meg_mode_word_read(int offset, UINT16 mem_mask);
	void segacd_1meg_mode_word_write(running_machine& machine, int offset, UINT16 data, UINT16 mem_mask, int use_pm);

	DECLARE_READ16_MEMBER( segacd_dmaaddr_r );
	DECLARE_WRITE16_MEMBER( segacd_dmaaddr_w );
	UINT16 m_dmaaddr;



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



	READ16_MEMBER( segacd_main_dataram_part1_r );
	WRITE16_MEMBER( segacd_main_dataram_part1_w );

	READ16_MEMBER( scd_hint_vector_r );
	READ16_MEMBER( scd_a12006_hint_register_r );
	WRITE16_MEMBER( scd_a12006_hint_register_w );


	WRITE16_MEMBER( segacd_stopwatch_timer_w );
	READ16_MEMBER( segacd_stopwatch_timer_r );
	READ16_MEMBER( segacd_sub_led_ready_r );
	WRITE16_MEMBER( segacd_sub_led_ready_w );
	READ16_MEMBER( segacd_sub_dataram_part1_r );
	WRITE16_MEMBER( segacd_sub_dataram_part1_w );
	READ16_MEMBER( segacd_sub_dataram_part2_r );
	WRITE16_MEMBER( segacd_sub_dataram_part2_w );


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
	READ16_MEMBER( segacd_backupram_r );
	WRITE16_MEMBER( segacd_backupram_w );
	READ16_MEMBER( segacd_font_color_r );
	WRITE16_MEMBER( segacd_font_color_w );
	READ16_MEMBER( segacd_font_converted_r );
	TIMER_DEVICE_CALLBACK_MEMBER( scd_dma_timer_callback );

	void SegaCD_CDC_Do_DMA( int &dmacount, UINT8 *CDC_BUFFER, UINT16 &dma_addrc, UINT16 &destination );
	timer_device* scd_dma_timer;

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
