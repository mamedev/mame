
typedef UINT16 (*decospr_priority_callback_func)(UINT16 pri);

class decospr_device_config : public device_config
{
	friend class decospr_device;
	decospr_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
public:
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;
	static void set_gfx_region(device_config *device, int gfxregion);
	static void set_pri_callback(device_config *device, decospr_priority_callback_func callback);

protected:
	UINT8 m_gfxregion;
	decospr_priority_callback_func m_pricallback;
};

class decospr_device : public device_t
{
	friend class decospr_device_config;
	decospr_device(running_machine &_machine, const decospr_device_config &config);
public:
	//void decospr_sprite_kludge(int x, int y);
	void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16* spriteram, int sizewords, bool invert_flip = false );
	void set_pri_callback(decospr_priority_callback_func callback);
	void set_gfxregion(int region) { m_gfxregion = region; };

protected:
	virtual void device_start();
	virtual void device_reset();
	const decospr_device_config &m_config;
	UINT8						m_gfxregion;
	decospr_priority_callback_func m_pricallback;

private:

};

const device_type decospr_ = decospr_device_config::static_alloc_device_config;





