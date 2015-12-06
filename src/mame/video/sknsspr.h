// license:BSD-3-Clause
// copyright-holders:David Haywood

class sknsspr_device : public device_t,
							public device_video_interface
{
public:
	sknsspr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void skns_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT32* spriteram_source, size_t spriteram_size, UINT8* gfx_source, size_t gfx_length, UINT32* sprite_regs);
	void skns_sprite_kludge(int x, int y);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	int sprite_kludge_x, sprite_kludge_y;
	#define SUPRNOVA_DECODE_BUFFER_SIZE 0x2000
	UINT8 decodebuffer[SUPRNOVA_DECODE_BUFFER_SIZE];
	int skns_rle_decode ( int romoffset, int size, UINT8*gfx_source, size_t gfx_length );
};

extern const device_type SKNS_SPRITE;
