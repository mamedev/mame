// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari vector hardware

***************************************************************************/

#ifndef __ATARIVGEAROM_H__
#define __ATARIVGEAROM_H__

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_ATARIVGEAROM_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ATARIVGEAROM, 0)


#define EAROM_SIZE  0x40

// ======================> atari_vg_earom_device

class atari_vg_earom_device :   public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	atari_vg_earom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;
public:
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
private:
	int m_old_ctrl;
	int m_state;
	int m_in_offset;
	int m_in_data;
	int m_out_data;
	char m_rom[EAROM_SIZE];
};

// device type definition
extern const device_type ATARIVGEAROM;

#endif
