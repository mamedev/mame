/* toaplan SCU */



class toaplan_scu_device : public device_t
{
public:
	toaplan_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void draw_sprites_to_tempbitmap(const rectangle &cliprect, UINT16* spriteram, UINT32 bytes );
	void copy_sprites_from_tempbitmap(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	void alloc_sprite_bitmap(screen_device &screen);
	void set_gfx_region(int region);

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	bitmap_ind16 m_temp_spritebitmap;
	int m_gfxregion;
};

extern const device_type TOAPLAN_SCU;

#define MCFG_TOAPLAN_SCU_ADD(_tag ) \
	MCFG_DEVICE_ADD(_tag, TOAPLAN_SCU, 0)
