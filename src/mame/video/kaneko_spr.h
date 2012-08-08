/* Kaneko Sprites */

/* berlwall, blazeon etc. */
#define MCFG_DEVICE_ADD_VU002_SPRITES \
	MCFG_DEVICE_ADD("kan_spr", KANEKO16_SPRITE, 0) \
	kaneko16_sprite_device::set_type(*device, 0); \

/* gtmr, gtmr2, bloodwar etc. */
#define MCFG_DEVICE_ADD_KC002_SPRITES \
	MCFG_DEVICE_ADD("kan_spr", KANEKO16_SPRITE, 0) \
	kaneko16_sprite_device::set_type(*device, 1); \



typedef struct
{
	int sprite[4];
} kaneko16_priority_t;

struct tempsprite
{
	int code,color;
	int x,y;
	int xoffs,yoffs;
	int flipx,flipy;
	int priority;
};



class kaneko16_sprite_device : public device_t
{
public:
	kaneko16_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	static void set_type(device_t &device, int type);
	static void set_altspacing(device_t &device, int spacing);
	static void set_fliptype(device_t &device, int fliptype);
	static void set_offsets(device_t &device, int xoffs, int yoffs);

	static void set_priorities(device_t &device, int pri0, int pri1, int pri2, int pri3);



	// sprite type: todo, different class instead (set when declaring device in MCFG)
	int m_sprite_type;

	// alt ram addressing (set when declaring device in MCFG)
	//  used on Berlin Wall.. it's the same sprite chip, so probably just a different RAM hookup on the PCB, maybe also
	//  related to the 'COPY BOARD' protection check on one set? investigate..
	int m_altspacing;

	// flip latching (set when declaring device in MCFG )  probably needs figuring out properly, only brapboys wants it?
	int m_sprite_fliptype;

	// offsets (set when declaring device in MCFG )
	UINT16 m_sprite_xoffs;
	UINT16 m_sprite_yoffs;

	// priority for mixing (set when declaring device in MCFG )
	kaneko16_priority_t m_priority;



	// used in the bitmap clear functions
	int get_sprite_type(void)
	{
		return m_sprite_type;
	}



	// registers
	UINT16 m_sprite_flipx;
	UINT16 m_sprite_flipy;
	UINT16* m_sprites_regs;

	struct tempsprite *m_first_sprite;
	int m_keep_sprites;
	bitmap_ind16 m_sprites_bitmap;


	void kaneko16_render_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram16, int spriteram16_bytes);
	void kaneko16_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram16, int spriteram16_bytes);

	void kaneko16_draw_sprites_custom(bitmap_ind16 &dest_bmp,const rectangle &clip,const gfx_element *gfx,
			UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
			int priority);

	int kaneko16_parse_sprite_type012(running_machine &machine, int i, struct tempsprite *s, UINT16* spriteram16, int spriteram16_bytes);


	DECLARE_READ16_MEMBER(kaneko16_sprites_regs_r);
	DECLARE_WRITE16_MEMBER(kaneko16_sprites_regs_w);


protected:
	virtual void device_start();
	virtual void device_reset();

private:

};

extern const device_type KANEKO16_SPRITE;

