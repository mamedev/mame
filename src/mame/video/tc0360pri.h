// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef _TC0360PRI_H_
#define _TC0360PRI_H_

class tc0360pri_device : public device_t
{
public:
	tc0360pri_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tc0360pri_device() {}

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	uint8_t   m_regs[16];
};

extern const device_type TC0360PRI;

#define MCFG_TC0360PRI_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TC0360PRI, 0)

#endif
