#pragma once
#ifndef __K001006_H__
#define __K001006_H__





struct k001006_interface
{
	const char     *m_gfx_region;
};



class k001006_device : public device_t,
						public k001006_interface
{
public:
	k001006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k001006_device() {}

	UINT32 get_palette(int index);

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT16 *     m_pal_ram;
	UINT16 *     m_unknown_ram;
	UINT32       m_addr;
	int          m_device_sel;

	UINT32 *     m_palette;
};


extern const device_type K001006;

#define MCFG_K001006_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K001006, 0) \
	MCFG_DEVICE_CONFIG(_interface)




#endif
