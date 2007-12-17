
#define ISMACS  (st0016_game&0x80)
#define ISMACS1 (((st0016_game&0x180)==0x180))
#define ISMACS2 (((st0016_game&0x180)==0x080))


#define ST0016_MAX_SPR_BANK   0x10
#define ST0016_MAX_CHAR_BANK  0x10000
#define ST0016_MAX_PAL_BANK   4

#define ST0016_SPR_BANK_SIZE  0x1000
#define ST0016_CHAR_BANK_SIZE 0x20
#define ST0016_PAL_BANK_SIZE  0x200

#define UNUSED_PEN 1024

#define ST0016_SPR_BANK_MASK  (ST0016_MAX_SPR_BANK-1)
#define ST0016_CHAR_BANK_MASK (ST0016_MAX_CHAR_BANK-1)
#define ST0016_PAL_BANK_MASK  (ST0016_MAX_PAL_BANK-1)

/*----------- defined in drivers/macs.c -----------*/

extern UINT8 macs_mux_data;


/*----------- defined in video/st0016.c -----------*/

extern UINT8 *st0016_charram,*st0016_spriteram,*st0016_paletteram;

extern UINT8 *macs_ram1,*macs_ram2;

READ8_HANDLER(st0016_dma_r);
WRITE8_HANDLER 	(st0016_sprite_bank_w);
WRITE8_HANDLER 	(st0016_palette_bank_w);
WRITE8_HANDLER 	(st0016_character_bank_w);
READ8_HANDLER  	(st0016_sprite_ram_r);
WRITE8_HANDLER 	(st0016_sprite_ram_w);
READ8_HANDLER  	(st0016_sprite2_ram_r);
WRITE8_HANDLER 	(st0016_sprite2_ram_w);
READ8_HANDLER  	(st0016_palette_ram_r);
WRITE8_HANDLER 	(st0016_palette_ram_w);
READ8_HANDLER  	(st0016_character_ram_r);
WRITE8_HANDLER 	(st0016_character_ram_w);
READ8_HANDLER	(st0016_vregs_r);
WRITE8_HANDLER	(st0016_vregs_w);
WRITE8_HANDLER	(st0016_rom_bank_w);

VIDEO_START(st0016);
VIDEO_UPDATE(st0016);

extern UINT32 st0016_game;

extern UINT32 st0016_rom_bank;

void st0016_save_init(void);


