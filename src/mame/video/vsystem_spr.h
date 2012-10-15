// Video System Sprites


typedef delegate<UINT32 (UINT32)> vsystem_tile_indirection_delegate;

/*** CG10103 **********************************************/

class vsystem_spr_device : public device_t
{
public:
	vsystem_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_offsets(device_t &device, int xoffs, int yoffs);
	static void set_pdraw(device_t &device, bool pdraw);
	static void set_tile_indirect_callback(device_t &device,vsystem_tile_indirection_delegate newtilecb);

	UINT32 tile_callback_noindirect(UINT32 tile);
	vsystem_tile_indirection_delegate m_newtilecb;

	int m_xoffs, m_yoffs;
	bool m_pdraw;

	UINT16* m_vram;
	UINT16 m_pal_base;
	UINT8 m_gfx_region;
	UINT8 m_transpen;

	struct vsystem_sprite_attributes
	{
		int ox;
		int xsize;
		int zoomx;
		int oy;
		int ysize;
		int zoomy;
		int flipx;
		int flipy;
		int color;
		int pri;
		UINT32 map;
	} curr_sprite;

	void get_sprite_attributes(UINT16* ram);
	void common_sprite_drawgfx(int gfxrgn, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void draw_sprites_inufuku(  UINT16* spriteram, int spriteram_bytes, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites_suprslam( UINT16* spriteram, int spriteram_bytes, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprite_taotaido( UINT16* spriteram, int spriteram_bytes,  running_machine &machine, UINT16 spriteno, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites_taotaido( UINT16* spriteram, int spriteram_bytes, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites_crshrace( UINT16* spriteram, int spriteram_bytes, running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect, int flipscreen);
	void draw_sprites_aerofght( UINT16* spriteram3, int spriteram_bytes, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	void f1gp2_draw_sprites(UINT16* spritelist, int flipscreen, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect );

	void CG10103_draw_sprite(running_machine &machine, bitmap_ind16& screen, const rectangle &cliprect, UINT16* spr, int drawpri);
	void CG10103_draw(running_machine &machine, int numchip, bitmap_ind16& screen, const rectangle &cliprect, int priority);
	void CG10103_set_pal_base(int pal_base);
	void CG10103_set_gfx_region(int gfx_region);
	void CG10103_set_transpen(int transpen);
	void CG10103_set_ram(UINT16* vram);

protected:
	virtual void device_start();
	virtual void device_reset();

private:


};


extern const device_type VSYSTEM_SPR;



