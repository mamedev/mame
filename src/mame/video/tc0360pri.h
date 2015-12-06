// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef _TC0360PRI_H_
#define _TC0360PRI_H_

class tc0360pri_device : public device_t
{
public:
	tc0360pri_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0360pri_device() {}

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	UINT8   m_regs[16];
};

extern const device_type TC0360PRI;

#define MCFG_TC0360PRI_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TC0360PRI, 0)

#endif
