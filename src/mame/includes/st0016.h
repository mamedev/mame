
class st0016_state : public driver_device
{
public:
	st0016_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	int mux_port;
	required_device<cpu_device> m_maincpu;
};

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

/*----------- defined in drivers/st0016.c -----------*/

extern UINT32 st0016_rom_bank;
WRITE8_HANDLER	(st0016_rom_bank_w);


/*----------- defined in video/st0016.c -----------*/

extern UINT8 macs_cart_slot;
extern UINT32 st0016_game;
extern UINT8 *st0016_charram;

READ8_HANDLER(st0016_dma_r);
WRITE8_HANDLER	(st0016_sprite_bank_w);
WRITE8_HANDLER	(st0016_palette_bank_w);
WRITE8_HANDLER	(st0016_character_bank_w);
READ8_HANDLER	(st0016_sprite_ram_r);
WRITE8_HANDLER	(st0016_sprite_ram_w);
READ8_HANDLER	(st0016_sprite2_ram_r);
WRITE8_HANDLER	(st0016_sprite2_ram_w);
READ8_HANDLER	(st0016_palette_ram_r);
WRITE8_HANDLER	(st0016_palette_ram_w);
READ8_HANDLER	(st0016_character_ram_r);
WRITE8_HANDLER	(st0016_character_ram_w);
READ8_HANDLER	(st0016_vregs_r);
WRITE8_HANDLER	(st0016_vregs_w);

void st0016_draw_screen(screen_device *screen, bitmap_t *bitmap, const rectangle *cliprect);
VIDEO_START(st0016);
SCREEN_UPDATE(st0016);
