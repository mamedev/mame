
/* Kaneko Sprites */

/* berlwall, blazeon etc. */
#define MCFG_DEVICE_ADD_VU002_SPRITES \
	MCFG_DEVICE_ADD("kan_spr", KANEKO_VU002_SPRITE, 0)
/* gtmr, gtmr2, bloodwar etc. */
#define MCFG_DEVICE_ADD_KC002_SPRITES \
	MCFG_DEVICE_ADD("kan_spr", KANEKO_KC002_SPRITE, 0)


struct kaneko16_priority_t
{
	int sprite[4];
};

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
	kaneko16_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock,  device_type type);

	static void set_altspacing(device_t &device, int spacing);
	static void set_fliptype(device_t &device, int fliptype);
	static void set_offsets(device_t &device, int xoffs, int yoffs);
	static void set_priorities(device_t &device, int pri0, int pri1, int pri2, int pri3);

	// (legacy) used in the bitmap clear functions
	virtual int get_sprite_type(void) =0;

	void kaneko16_render_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram16, int spriteram16_bytes);


	DECLARE_READ16_MEMBER(kaneko16_sprites_regs_r);
	DECLARE_WRITE16_MEMBER(kaneko16_sprites_regs_w);

protected:
	virtual void device_start();
	virtual void device_reset();

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

	// pure virtual function for getting the attributes on sprites, the two different chip types have
	// them in a different order
	virtual void get_sprite_attributes(struct tempsprite *s, UINT16 attr) =0;


private:
	// registers
	UINT16 m_sprite_flipx;
	UINT16 m_sprite_flipy;
	UINT16* m_sprites_regs;

	struct tempsprite *m_first_sprite;
	int m_keep_sprites;
	bitmap_ind16 m_sprites_bitmap;

	void kaneko16_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram16, int spriteram16_bytes);

	void kaneko16_draw_sprites_custom(bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
			UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
			int priority);

	int kaneko16_parse_sprite_type012(running_machine &machine, int i, struct tempsprite *s, UINT16* spriteram16, int spriteram16_bytes);




};

//extern const device_type KANEKO16_SPRITE;


class kaneko_vu002_sprite_device : public kaneko16_sprite_device
{
public:
	kaneko_vu002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void get_sprite_attributes(struct tempsprite *s, UINT16 attr);
	int get_sprite_type(void){ return 0; };
};

extern const device_type KANEKO_VU002_SPRITE;

class kaneko_kc002_sprite_device : public kaneko16_sprite_device
{
public:
	kaneko_kc002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void get_sprite_attributes(struct tempsprite *s, UINT16 attr);
	int get_sprite_type(void){ return 1; };
};

extern const device_type KANEKO_KC002_SPRITE;
