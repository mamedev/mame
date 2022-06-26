#ifndef MAME_MACHINE_PS2M80_H
#define MAME_MACHINE_PS2M80_H

#include "machine/ibmps2.h"

class ps2_m80_t1_mb_device : public ps2_mb_device
{
public:
	ps2_m80_t1_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint16_t sram_size, uint16_t pos_id)
		: ps2_mb_device(mconfig, tag, owner, clock, sram_size, pos_id, true)
	{
	}

	ps2_m80_t1_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: ps2_mb_device(mconfig, tag, owner, clock, 0x800, 0xfeff, true)
	{
	}

	virtual void map(address_map &map) override;

	void 	irq6_w(int state);
	
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
    virtual void device_config_complete() override;

    virtual uint8_t memory_encoding_r();
    virtual void    memory_encoding_w(uint8_t data);

    virtual uint8_t split_address_r();
    virtual void    split_address_w(uint8_t data);

	virtual void 	update_memory_split();

	virtual uint8_t planar_pos_r(offs_t offset) override;
	virtual void 	planar_pos_w(offs_t offset, uint8_t data) override;

    void    disable_memory_split();
    void    enable_memory_split();

	uint8_t m_split_address_reg;
	uint8_t m_memory_encoding_reg;

	bool m_memory_split_active;
	uint32_t m_memory_split_base;
	uint32_t m_split_size;

private:

};

class ps2_m80_t2_mb_device : public ps2_m80_t1_mb_device
{
public:
	ps2_m80_t2_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: ps2_m80_t1_mb_device(mconfig, tag, owner, clock, 0x800, 0xfdff)
	{
	}

	virtual void	map(address_map &map) override;

protected:
	virtual void 	device_start() override;
	virtual void 	device_reset() override;

	virtual uint8_t planar_pos_r(offs_t offset) override;
	virtual void 	planar_pos_w(offs_t offset, uint8_t data) override;

	virtual void 	update_memory_split() override;

    virtual uint8_t memory_encoding_r() override;
    virtual void    memory_encoding_w(uint8_t data) override;

    virtual uint8_t split_address_r() override;
    virtual void    split_address_w(uint8_t data) override;

private:
	uint8_t			shadow_bios_r(offs_t offset);
	void			shadow_bios_w(offs_t offset, uint8_t data);
};

class ps2_m80_t3_mb_device : public ps2_m80_t1_mb_device
{
public:
	ps2_m80_t3_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: ps2_m80_t1_mb_device(mconfig, tag, owner, clock, 0x2000, 0xfff9)
	{
	}
protected:

private:
};

DECLARE_DEVICE_TYPE(PS2_M80_T1_MB, ps2_m80_t1_mb_device)
DECLARE_DEVICE_TYPE(PS2_M80_T2_MB, ps2_m80_t2_mb_device)
DECLARE_DEVICE_TYPE(PS2_M80_T3_MB, ps2_m80_t3_mb_device)

#endif