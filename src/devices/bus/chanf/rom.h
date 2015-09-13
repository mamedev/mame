// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __CHANF_ROM_H
#define __CHANF_ROM_H

#include "slot.h"


// ======================> chanf_rom_device

class chanf_rom_device : public device_t,
						public device_channelf_cart_interface
{
public:
	// construction/destruction
	chanf_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	chanf_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() {}
	virtual void device_reset() {}

	UINT8 common_read_2102(UINT32 offset);
	UINT8 common_read_3853(UINT32 offset);
	void common_write_2102(UINT32 offset, UINT8 data);
	void common_write_3853(UINT32 offset, UINT8 data);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);


protected:
	// used for RAM chip in Hangman & Maze
	UINT8 m_latch[2];       // PORT A & PORT B
	UINT16 m_addr_latch, m_addr;
	int m_read_write, m_data0;
};

// ======================> chanf_maze_device

class chanf_maze_device : public chanf_rom_device
{
public:
	// construction/destruction
	chanf_maze_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_ram) { return common_read_2102(offset); }
	virtual DECLARE_WRITE8_MEMBER(write_ram) { common_write_2102(offset, data); }
};


// ======================> chanf_hangman_device

class chanf_hangman_device : public chanf_rom_device
{
public:
	// construction/destruction
	chanf_hangman_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_ram) { return common_read_2102(offset); }
	virtual DECLARE_WRITE8_MEMBER(write_ram) { common_write_2102(offset, data); }
};


// ======================> chanf_chess_device

class chanf_chess_device : public chanf_rom_device
{
public:
	// construction/destruction
	chanf_chess_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_ram) { return common_read_3853(offset); }
	virtual DECLARE_WRITE8_MEMBER(write_ram) { common_write_3853(offset, data); }
};


// ======================> chanf_multi_old_device

class chanf_multi_old_device : public chanf_rom_device
{
public:
	// construction/destruction
	chanf_multi_old_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_READ8_MEMBER(read_ram) { return common_read_3853(offset); }
	virtual DECLARE_WRITE8_MEMBER(write_ram) { common_write_3853(offset, data); }
	virtual DECLARE_WRITE8_MEMBER(write_bank);

private:
	int m_base_bank;
};


// ======================> chanf_multi_final_device

class chanf_multi_final_device : public chanf_rom_device
{
public:
	// construction/destruction
	chanf_multi_final_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_READ8_MEMBER(read_ram) { return common_read_3853(offset); }
	virtual DECLARE_WRITE8_MEMBER(write_ram) { common_write_3853(offset, data); }
	virtual DECLARE_WRITE8_MEMBER(write_bank);

private:
	int m_base_bank, m_half_bank;
};


// device type definition
extern const device_type CHANF_ROM_STD;
extern const device_type CHANF_ROM_MAZE;
extern const device_type CHANF_ROM_HANGMAN;
extern const device_type CHANF_ROM_CHESS;
extern const device_type CHANF_ROM_MULTI_OLD;
extern const device_type CHANF_ROM_MULTI_FINAL;


#endif
