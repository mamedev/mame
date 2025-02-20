// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Andreas Naive, Charles MacDonald
/***************************************************************************

    Hitachi FD1094 encryption emulation

****************************************************************************

    Copyright Nicola Salmoria, Andreas Naive, and Charles MacDonald.

***************************************************************************/

#ifndef MAME_SEGA_FD1094_H
#define MAME_SEGA_FD1094_H

#pragma once

#include "cpu/m68000/m68000.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(FD1094, fd1094_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fd1094_decryption_cache

class fd1094_decryption_cache
{
public:
	// construction/destruction
	fd1094_decryption_cache(fd1094_device &fd1094);

	// getters
	fd1094_device &fd1094() const { return m_fd1094; }
	uint16_t *decrypted_opcodes(uint8_t state);

	// operations
	void reset();
	void configure(offs_t baseaddress, uint32_t size, offs_t rgnoffset);

protected:
	// internal state
	fd1094_device &         m_fd1094;
	uint32_t                  m_baseaddress;
	uint32_t                  m_size;
	uint32_t                  m_rgnoffset;
	std::vector<uint16_t>     m_decrypted_opcodes[256];
};


// ======================> fd1094_device

// base device, shared implementation between A and B variants
class fd1094_device : public m68000_device
{
public:
	typedef delegate<void (uint8_t)> state_change_delegate;

	// construction/destruction
	fd1094_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// explicit decryption helpers
	void decrypt(offs_t baseaddr, uint32_t size, offs_t regionoffs, uint16_t *opcodesptr, uint8_t state) { decrypt(baseaddr, size, &m_srcbase[regionoffs/2], opcodesptr, state); }

	// live state management
	uint8_t state() const { return m_irqmode ? m_key[0] : m_state; }
	void change_state(int newstate = -1);
	void notify_state_change(state_change_delegate callback) { m_state_change = callback; }

	// constants
	enum
	{
		STATE_RESET = 0x100,
		STATE_IRQ = 0x200,
		STATE_RTE = 0x300
	};

	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
protected:
	required_memory_bank m_decrypted_opcodes_bank;

	// device overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_postload();

	// internal helpers
	uint16_t decrypt_one(offs_t address, uint16_t val, const uint8_t *main_key, uint8_t state, bool vector_fetch);
	void decrypt(offs_t baseaddr, uint32_t size, const uint16_t *srcptr, uint16_t *opcodesptr, uint8_t state);
	void default_state_change(uint8_t state);
	IRQ_CALLBACK_MEMBER( irq_callback );

	// static helpers
	void cmp_callback(offs_t offset, uint32_t data);
	void rte_callback(int state);

	// internal state
	uint8_t                   m_state;
	bool                    m_irqmode;
	state_change_delegate   m_state_change;
	fd1094_decryption_cache m_cache;
	uint16_t const            *m_srcbase;
	optional_region_ptr<uint16_t> m_rom;
	required_region_ptr<uint8_t> m_key;
	uint8_t                   m_masked_opcodes_lookup[2][65536/8/2];

	// static tables
	static const uint16_t s_masked_opcodes[];
};

struct fd1094_constraint
{
	offs_t  pc;
	uint16_t  state;
	uint16_t  value;
	uint16_t  mask;
};

#endif // MAME_SEGA_FD1094_H
