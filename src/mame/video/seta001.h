

class seta001_device : public device_t
{
public:
	seta001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	void setac_draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16* spriteram16_2, int flipxoffs, int noflipxoffs);
	void setac_eof( UINT16* spriteram2);

	void tnzs_draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8* objram, UINT8* bg_flag );
	void tnzs_eof( UINT8* objram);

	void srmp2_draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16* spriteram16_2, int color_bank);
	
	void srmp3_draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8* spriteram_2, UINT8* spriteram_3, int gfx_bank);
	
	void mjyuugi_draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16* spriteram16_2, int gfx_bank);

	UINT8 m_spritectrl[4];
	UINT8 m_spriteylow[0x300]; // 0x200 low y + 0x100 bg stuff

protected:
	virtual void device_start();
	virtual void device_reset();
	private:

private:
	
	void setac_draw_sprites_map(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16* spriteram16_2);
	
	void tnzs_draw_foreground( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 *char_pointer, UINT8 *x_pointer, UINT8 *y_pointer, UINT8 *ctrl_pointer, UINT8 *color_pointer, int screenflip);
	void tnzs_draw_background( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 *m, UINT8* scrollram, UINT8* bg_flag, int screenflip);
	
	void srmp3_draw_sprites_map(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8* spriteram_2, UINT8* spriteram_3);

	void mjyuugi_draw_sprites_map(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16* spriteram16_2);
};

READ16_DEVICE_HANDLER( spritectrl_r16 );
WRITE16_DEVICE_HANDLER( spritectrl_w16 );
READ8_DEVICE_HANDLER( spritectrl_r8 );
WRITE8_DEVICE_HANDLER( spritectrl_w8 );
WRITE8_DEVICE_HANDLER( spritectrl_w8_champbwl );

READ16_DEVICE_HANDLER( spriteylow_r16 );
WRITE16_DEVICE_HANDLER( spriteylow_w16 );
READ8_DEVICE_HANDLER( spriteylow_r8 );
WRITE8_DEVICE_HANDLER( spriteylow_w8 );

extern const device_type SETA001_SPRITE;






