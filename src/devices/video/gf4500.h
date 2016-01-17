// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    NVIDIA GoForce 4500

    (c) 2010 Tim Schuerewegen

*/

#ifndef __GF4500_H__
#define __GF4500_H__


class gf4500_device : public device_t
{
public:
	gf4500_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~gf4500_device() {}


	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );

	UINT32 screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state

	void vram_write16(UINT16 data);

	std::unique_ptr<UINT32[]> m_data;
	int m_screen_x;
	int m_screen_y;
	int m_screen_x_max;
	int m_screen_y_max;
	int m_screen_x_min;
	int m_screen_y_min;
};



#define MCFG_GF4500_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GF4500, 0)


extern const device_type GF4500;


#endif /* __GF4500_H__ */
