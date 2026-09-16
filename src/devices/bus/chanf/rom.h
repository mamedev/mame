// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_CHANF_ROM_H
#define MAME_BUS_CHANF_ROM_H

#include "slot.h"


// ======================> chanf_rom_device

class chanf_rom_device : public device_t,
						public device_channelf_cart_interface
{
public:
	// construction/destruction
	chanf_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	uint8_t common_read_2102(uint32_t offset);
	uint8_t common_read_3853(uint32_t offset);
	void common_write_2102(uint32_t offset, uint8_t data);
	void common_write_3853(uint32_t offset, uint8_t data);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;

protected:
	chanf_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// used for RAM chip in Hangman & Maze
	uint8_t m_latch[2];       // PORT A & PORT B
	uint16_t m_addr_latch, m_addr;
	int m_read_write, m_data0;
};

// ======================> chanf_maze_device

class chanf_maze_device : public chanf_rom_device
{
public:
	// construction/destruction
	chanf_maze_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// reading and writing
	virtual uint8_t read_ram(offs_t offset) override { return common_read_2102(offset); }
	virtual void write_ram(offs_t offset, uint8_t data) override { common_write_2102(offset, data); }
};


// ======================> chanf_hangman_device

class chanf_hangman_device : public chanf_rom_device
{
public:
	// construction/destruction
	chanf_hangman_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// reading and writing
	virtual uint8_t read_ram(offs_t offset) override { return common_read_2102(offset); }
	virtual void write_ram(offs_t offset, uint8_t data) override { common_write_2102(offset, data); }
};


// ======================> chanf_chess_device

class chanf_chess_device : public chanf_rom_device
{
public:
	// construction/destruction
	chanf_chess_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_ram(offs_t offset) override { return common_read_3853(offset); }
	virtual void write_ram(offs_t offset, uint8_t data) override { common_write_3853(offset, data); }
};


// ======================> chanf_multi_old_device

class chanf_multi_old_device : public chanf_rom_device
{
public:
	// construction/destruction
	chanf_multi_old_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override { return common_read_3853(offset); }
	virtual void write_ram(offs_t offset, uint8_t data) override { common_write_3853(offset, data); }
	virtual void write_bank(uint8_t data) override;

private:
	int m_base_bank;
};


// ======================> chanf_multi_final_device

class chanf_multi_final_device : public chanf_rom_device
{
public:
	// construction/destruction
	chanf_multi_final_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override { return common_read_3853(offset); }
	virtual void write_ram(offs_t offset, uint8_t data) override { common_write_3853(offset, data); }
	virtual void write_bank(uint8_t data) override;

private:
	int m_base_bank, m_half_bank;
};


// device type definition
DECLARE_DEVICE_TYPE(CHANF_ROM_STD,         chanf_rom_device)
DECLARE_DEVICE_TYPE(CHANF_ROM_MAZE,        chanf_maze_device)
DECLARE_DEVICE_TYPE(CHANF_ROM_HANGMAN,     chanf_hangman_device)
DECLARE_DEVICE_TYPE(CHANF_ROM_CHESS,       chanf_chess_device)
DECLARE_DEVICE_TYPE(CHANF_ROM_MULTI_OLD,   chanf_multi_old_device)
DECLARE_DEVICE_TYPE(CHANF_ROM_MULTI_FINAL, chanf_multi_final_device)


#endif // MAME_BUS_CHANF_ROM_H
