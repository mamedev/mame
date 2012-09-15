/*****************************************************************************
 *
 * includes/gb.h
 *
 ****************************************************************************/

#ifndef GB_H_
#define GB_H_



/* Interrupts */
#define VBL_INT               0       /* V-Blank    */
#define LCD_INT               1       /* LCD Status */
#define TIM_INT               2       /* Timer      */
#define SIO_INT               3       /* Serial I/O */
#define EXT_INT               4       /* Joypad     */

#ifdef TIMER
#undef TIMER
#endif

/* Cartridge types */
#define CART_RAM	0x01	/* Cartridge has RAM                             */
#define BATTERY		0x02	/* Cartridge has a battery to save RAM           */
#define TIMER		0x04	/* Cartridge has a real-time-clock (MBC3 only)   */
#define RUMBLE		0x08	/* Cartridge has a rumble motor (MBC5 only)      */
#define SRAM		0x10	/* Cartridge has SRAM                            */
#define UNKNOWN		0x80	/* Cartridge is of an unknown type               */

#define DMG_FRAMES_PER_SECOND	59.732155
#define SGB_FRAMES_PER_SECOND	61.17


#define MAX_ROMBANK 512
#define MAX_RAMBANK 256


#define _NR_GB_VID_REGS		0x40

struct layer_struct {
	UINT8  enabled;
	UINT8  *bg_tiles;
	UINT8  *bg_map;
	UINT8  xindex;
	UINT8  xshift;
	UINT8  xstart;
	UINT8  xend;
	/* GBC specific */
	UINT8  *gbc_map;
	INT16  bgline;
};

struct gb_lcd_t {
	int	window_lines_drawn;

	UINT8	gb_vid_regs[_NR_GB_VID_REGS];
	UINT8	bg_zbuf[160];

	UINT16	cgb_bpal[32];	/* CGB current background palette table */
	UINT16	cgb_spal[32];	/* CGB current sprite palette table */

	UINT8	gb_bpal[4];		/* Background palette */
	UINT8	gb_spal0[4];	/* Sprite 0 palette */
	UINT8	gb_spal1[4];	/* Sprite 1 palette */

	/* Things used to render current line */
	int current_line;		/* Current line */
	int cmp_line;			/* Compare line */
	int sprCount;			/* Number of sprites on current line */
	int sprite[10];			/* References to sprites to draw on current line */
	int previous_line;		/* Previous line we've drawn in */
	int start_x;			/* Pixel to start drawing from (inclusive) */
	int end_x;				/* Pixel to end drawing (exclusive) */
	int mode;				/* Keep track of internal STAT mode */
	int state;				/* Current state of the video state machine */
	int lcd_irq_line;
	int triggering_line_irq;
	int line_irq;
	int triggering_mode_irq;
	int mode_irq;
	int delayed_line_irq;
	int sprite_cycles;
	int scrollx_adjust;
	int oam_locked;
	int vram_locked;
	int pal_locked;
	int hdma_enabled;
	int hdma_possible;
	struct layer_struct	layer[2];
	emu_timer *lcd_timer;
	int gbc_mode;

	memory_region *gb_vram;		/* Pointer to VRAM */
	memory_region *gb_oam;		/* Pointer to OAM memory */
	UINT8	*gb_vram_ptr;
	UINT8	*gb_chrgen;		/* Character generator */
	UINT8	*gb_bgdtab;		/* Background character table */
	UINT8	*gb_wndtab;		/* Window character table */
	UINT8	gb_tile_no_mod;
	UINT8	*gbc_chrgen;	/* CGB Character generator */
	UINT8	*gbc_bgdtab;	/* CGB Background character table */
	UINT8	*gbc_wndtab;	/* CGB Window character table */
};



class gb_state : public driver_device
{
public:
	gb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT16 m_sgb_pal_data[4096];
	UINT8 m_sgb_pal_map[20][18];
	UINT16 m_sgb_pal[128];
	UINT8 *m_sgb_tile_data;
	UINT8 m_sgb_tile_map[2048];
	UINT8 m_sgb_window_mask;
	UINT8 m_sgb_hack;
	//gb_state driver_data;
	UINT8		m_gb_io[0x10];

	/* Timer related */
	UINT16		m_divcount;
	UINT8		m_shift;
	UINT16		m_shift_cycles;
	UINT8		m_triggering_irq;
	UINT8		m_reloading;

	/* Serial I/O related */
	UINT32		m_SIOCount;				/* Serial I/O counter */
	emu_timer	*m_gb_serial_timer;

	/* SGB variables */
	UINT8		m_sgb_atf_data[4050];		/* (SGB) Attributes files */
	INT8 m_sgb_packets;
	UINT8 m_sgb_bitcount;
	UINT8 m_sgb_bytecount;
	UINT8 m_sgb_start;
	UINT8 m_sgb_rest;
	UINT8 m_sgb_controller_no;
	UINT8 m_sgb_controller_mode;
	UINT8 m_sgb_data[112];
	UINT32 m_sgb_atf;

	/* Cartridge/mapper */
	UINT16		m_MBCType;				/* MBC type: 0 for none */
	UINT8		m_CartType;				/* Cartridge type (battery, ram, rtc, etc) */
	UINT8		*m_ROMMap[MAX_ROMBANK];	/* Addresses of ROM banks */
	UINT16		m_ROMBank;				/* Index of ROM bank currently used at 4000-7fff */
	UINT16		m_ROMBank00;				/* Index of ROM bank currently used at 0000-3fff */
	UINT8		m_ROMMask;				/* Mask for the ROM bank number */
	UINT16		m_ROMBanks;				/* Total number of ROM banks */
	UINT8		*m_RAMMap[MAX_RAMBANK];	/* Addresses of RAM banks */
	UINT8		m_RAMBank;				/* Number of RAM bank currently used */
	UINT8		m_RAMMask;				/* Mask for the RAM bank number */
	UINT8		m_RAMBanks;				/* Total number of RAM banks */
	UINT8		m_MBC1Mode;				/* MBC1 ROM/RAM mode */
	UINT8		*m_MBC3RTCData;			/* MBC3 RTC data */
	UINT8		m_MBC3RTCMap[5];			/* MBC3 RTC banks */
	UINT8		m_MBC3RTCBank;			/* MBC3 RTC bank */
	UINT8		*m_GBC_RAMMap[8];			/* (CGB) Addresses of internal RAM banks */
	UINT8		m_GBC_RAMBank;			/* (CGB) Current CGB RAM bank */
	UINT8		m_gbTama5Memory[32];
	UINT8		m_gbTama5Byte;
	UINT8		m_gbTama5Address;
	UINT8		m_gbLastTama5Command;
	UINT8		*m_gb_cart;
	UINT8		*m_gb_cart_ram;
	UINT8		*m_gb_dummy_rom_bank;
	UINT8		*m_gb_dummy_ram_bank;

	UINT8 m_mmm01_bank_offset;
	UINT8 m_mmm01_reg1;
	UINT8 m_mmm01_bank;
	UINT8 m_mmm01_bank_mask;
	gb_lcd_t m_lcd;
	void (*update_scanline)( running_machine &machine );

	bitmap_ind16 m_bitmap;
	DECLARE_WRITE8_MEMBER(gb_rom_bank_select_mbc1);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_select_mbc2);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_select_mbc3);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_select_mbc5);
	DECLARE_WRITE8_MEMBER(gb_ram_bank_select_mbc6);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_select_mbc6_1);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_select_mbc6_2);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_select_mbc7);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_unknown_mbc7);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_select_wisdom);
	DECLARE_WRITE8_MEMBER(gb_ram_bank_select_mbc1);
	DECLARE_WRITE8_MEMBER(gb_ram_bank_select_mbc3);
	DECLARE_WRITE8_MEMBER(gb_ram_bank_select_mbc5);
	DECLARE_WRITE8_MEMBER(gb_ram_enable);
	DECLARE_WRITE8_MEMBER(gb_mem_mode_select_mbc1);
	DECLARE_WRITE8_MEMBER(gb_mem_mode_select_mbc3);
	DECLARE_WRITE8_MEMBER(gb_ram_tama5);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_mmm01_0000_w);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_mmm01_2000_w);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_mmm01_4000_w);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_mmm01_6000_w);
	DECLARE_WRITE8_MEMBER(gb_rom_bank_select_mbc1_kor);
	DECLARE_WRITE8_MEMBER(gb_ram_bank_select_mbc1_kor);
	DECLARE_WRITE8_MEMBER(gb_mem_mode_select_mbc1_kor);
	DECLARE_WRITE8_MEMBER(gb_io_w);
	DECLARE_WRITE8_MEMBER(gb_io2_w);
	DECLARE_WRITE8_MEMBER(sgb_io_w);
	DECLARE_READ8_MEMBER(gb_ie_r);
	DECLARE_WRITE8_MEMBER(gb_ie_w);
	DECLARE_READ8_MEMBER(gb_io_r);
	DECLARE_WRITE8_MEMBER(gbc_io2_w);
	DECLARE_READ8_MEMBER(gbc_io2_r);
	DECLARE_READ8_MEMBER(megaduck_video_r);
	DECLARE_WRITE8_MEMBER(megaduck_video_w);
	DECLARE_WRITE8_MEMBER(megaduck_sound_w1);
	DECLARE_READ8_MEMBER(megaduck_sound_r1);
	DECLARE_WRITE8_MEMBER(megaduck_sound_w2);
	DECLARE_READ8_MEMBER(megaduck_sound_r2);
	DECLARE_WRITE8_MEMBER(megaduck_rom_bank_select_type1);
	DECLARE_WRITE8_MEMBER(megaduck_rom_bank_select_type2);
	DECLARE_READ8_MEMBER(gb_video_r);
	DECLARE_READ8_MEMBER(gb_vram_r);
	DECLARE_WRITE8_MEMBER(gb_vram_w);
	DECLARE_READ8_MEMBER(gb_oam_r);
	DECLARE_WRITE8_MEMBER(gb_oam_w);
	DECLARE_WRITE8_MEMBER(gb_video_w);
	DECLARE_READ8_MEMBER(gbc_video_r);
	DECLARE_WRITE8_MEMBER(gbc_video_w);
	DECLARE_MACHINE_START(gb);
	DECLARE_MACHINE_RESET(gb);
	DECLARE_PALETTE_INIT(gb);
	DECLARE_MACHINE_START(megaduck);
	DECLARE_MACHINE_RESET(megaduck);
	DECLARE_PALETTE_INIT(megaduck);
	DECLARE_MACHINE_START(sgb);
	DECLARE_MACHINE_RESET(sgb);
	DECLARE_PALETTE_INIT(sgb);
	DECLARE_MACHINE_RESET(gbpocket);
	DECLARE_PALETTE_INIT(gbp);
	DECLARE_MACHINE_START(gbc);
	DECLARE_MACHINE_RESET(gbc);	
	DECLARE_PALETTE_INIT(gbc);
	DECLARE_MACHINE_START(gb_video);
	DECLARE_MACHINE_START(gbc_video);
};


/*----------- defined in machine/gb.c -----------*/

DEVICE_START(gb_cart);
DEVICE_IMAGE_LOAD(gb_cart);
INTERRUPT_GEN( gb_scanline_interrupt );
void gb_timer_callback(lr35902_cpu_device *device, int cycles);




/* -- Super Game Boy specific -- */
#define SGB_BORDER_PAL_OFFSET	64	/* Border colours stored from pal 4-7   */
#define SGB_XOFFSET				48	/* GB screen starts at column 48        */
#define SGB_YOFFSET				40	/* GB screen starts at row 40           */


/* -- Megaduck specific -- */
extern DEVICE_IMAGE_LOAD(megaduck_cart);



/*----------- defined in video/gb.c -----------*/



enum
{
	GB_VIDEO_DMG = 1,
	GB_VIDEO_MGB,
	GB_VIDEO_SGB,
	GB_VIDEO_CGB
};









void gb_video_reset( running_machine &machine, int mode );
UINT8 *gb_get_vram_ptr(running_machine &machine);


#endif /* GB_H_ */
