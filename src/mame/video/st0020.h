


class st0020_device : public device_t
{
public:
	st0020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	int m_gfx_index;
	int m_st0020_gfxram_bank;
	UINT16* m_st0020_gfxram;
	UINT16* m_st0020_spriteram;
	UINT16* m_st0020_blitram;

	void st0020_draw_zooming_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	void st0020_draw_all(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ16_MEMBER(st0020_gfxram_r);
	DECLARE_WRITE16_MEMBER(st0020_gfxram_w);
	DECLARE_READ16_MEMBER(st0020_blitram_r);
	DECLARE_WRITE16_MEMBER(st0020_blitram_w);
	DECLARE_READ16_MEMBER(st0020_sprram_r);
	DECLARE_WRITE16_MEMBER(st0020_sprram_w);

protected:
	virtual void device_start();
	virtual void device_reset();

private:



};


extern const device_type ST0020_SPRITES;

