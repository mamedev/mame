
class sknsspr_device_config : public device_config
{
	friend class sknsspr_device;
	sknsspr_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
public:
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;
protected:
};

class sknsspr_device : public device_t
{
	friend class sknsspr_device_config;
	sknsspr_device(running_machine &_machine, const sknsspr_device_config &config);
public:
	void skns_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT32* spriteram_source, size_t spriteram_size, UINT8* gfx_source, size_t gfx_length, UINT32* sprite_regs);
	void skns_sprite_kludge(int x, int y);

protected:
	virtual void device_start();
	virtual void device_reset();
	const sknsspr_device_config &m_config;
private:
	int sprite_kludge_x, sprite_kludge_y;
	#define SUPRNOVA_DECODE_BUFFER_SIZE 0x2000
	UINT8 decodebuffer[SUPRNOVA_DECODE_BUFFER_SIZE];
	int skns_rle_decode ( running_machine *machine, int romoffset, int size, UINT8*gfx_source, size_t gfx_length );
};

const device_type sknsspr_ = sknsspr_device_config::static_alloc_device_config;





