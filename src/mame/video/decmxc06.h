/* MXC06 */


class deco_mxc06_device_config : public device_config
{
	friend class deco_mxc06_device;
	deco_mxc06_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
public:
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;
	static void set_gfx_region(device_config *device, int region);

protected:
	UINT8 m_gfxregion;
};

class deco_mxc06_device : public device_t
{
	friend class deco_mxc06_device_config;
	deco_mxc06_device(running_machine &_machine, const deco_mxc06_device_config &config);
public:
	void set_gfxregion(int region) { m_gfxregion = region; };
	void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16* spriteram16, int pri_mask, int pri_val, int col_mask );
	void set_pri_type( int type ) { m_priority_type = type; }
protected:
	virtual void device_start();
	virtual void device_reset();
	const deco_mxc06_device_config &m_config;

	UINT8 m_gfxregion;
	int m_priority_type; // just so we can support the existing drivers without converting everything to pdrawgfx just yet

private:


};

const device_type deco_mxc06_ = deco_mxc06_device_config::static_alloc_device_config;


