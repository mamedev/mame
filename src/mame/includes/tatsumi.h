/*----------- defined in drivers/tatsumi.c -----------*/

extern UINT8* tatsumi_rom_sprite_lookup1;
extern UINT8* tatsumi_rom_sprite_lookup2;
extern UINT8* tatsumi_rom_clut0;
extern UINT8* tatsumi_rom_clut1;

extern UINT16 *roundup5_d0000_ram, *roundup5_e0000_ram;
extern UINT16 *roundup5_unknown0, *roundup5_unknown1, *roundup5_unknown2;

/*----------- defined in machine/tatsumi.c -----------*/

READ16_HANDLER( apache3_bank_r );
WRITE16_HANDLER( apache3_bank_w );
WRITE16_HANDLER( apache3_irq_ack_w );
READ16_HANDLER( apache3_v30_v20_r );
WRITE16_HANDLER( apache3_v30_v20_w );
READ16_HANDLER( roundup_v30_z80_r );
WRITE16_HANDLER( roundup_v30_z80_w );
READ16_HANDLER( tatsumi_v30_68000_r );
WRITE16_HANDLER( tatsumi_v30_68000_w ) ;
READ16_HANDLER(apache3_z80_r);
WRITE16_HANDLER(apache3_z80_w);
READ8_HANDLER( apache3_adc_r );
WRITE8_HANDLER( apache3_adc_w );
WRITE16_HANDLER(cyclwarr_control_w);
READ16_HANDLER(cyclwarr_control_r);
WRITE16_HANDLER( roundup5_control_w );
WRITE16_HANDLER( apache3_a0000_w );
WRITE16_HANDLER( roundup5_d0000_w );
WRITE16_HANDLER( roundup5_e0000_w );

READ8_HANDLER(tatsumi_hack_ym2151_r);
READ8_HANDLER(tatsumi_hack_oki_r);

extern UINT16 *tatsumi_68k_ram;
extern UINT8 *apache3_z80_ram;
extern UINT16 tatsumi_control_word;
extern UINT16 apache3_a0000[16];

void tatsumi_reset(void);

/*----------- defined in video/tatsumi.c -----------*/

WRITE16_HANDLER( roundup5_palette_w );
WRITE16_HANDLER( tatsumi_sprite_control_w );
WRITE16_HANDLER( roundup5_text_w );
WRITE16_HANDLER( roundup5_crt_w );
READ16_HANDLER( cyclwarr_videoram_r );
WRITE16_HANDLER( cyclwarr_videoram_w );
READ16_HANDLER( cyclwarr_videoram2_r );
WRITE16_HANDLER( cyclwarr_videoram2_w );
READ16_HANDLER(roundup5_vram_r);
WRITE16_HANDLER(roundup5_vram_w);
WRITE16_HANDLER(apache3_palette_w);

extern UINT16* tatsumi_sprite_control_ram;
extern UINT16 *cyclwarr_videoram, *cyclwarr_videoram2;
extern UINT16 *roundup_r_ram, *roundup_p_ram, *roundup_l_ram;

VIDEO_START( apache3 );
VIDEO_START( roundup5 );
VIDEO_START( cyclwarr );
VIDEO_UPDATE( roundup5 );
VIDEO_UPDATE( apache3 );
VIDEO_UPDATE( cyclwarr );

