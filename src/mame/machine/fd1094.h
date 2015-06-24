// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Andreas Naive, Charles MacDonald
/***************************************************************************

    Hitachi FD1094 encryption emulation

****************************************************************************

    Copyright Nicola Salmoria, Andreas Naive, and Charles MacDonald.

    All rights reserved.

***************************************************************************/

#ifndef __FD1094_H__
#define __FD1094_H__

#include "cpu/m68000/m68000.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// device type definition
extern const device_type FD1094;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class fd1094_device;


// ======================> fd1094_decryption_cache

class fd1094_decryption_cache
{
public:
	// construction/destruction
	fd1094_decryption_cache(fd1094_device &fd1094);

	// getters
	fd1094_device &fd1094() const { return m_fd1094; }
	UINT16 *decrypted_opcodes(UINT8 state);

	// operations
	void reset();
	void configure(offs_t baseaddress, UINT32 size, offs_t rgnoffset);

protected:
	// internal state
	fd1094_device &         m_fd1094;
	UINT32                  m_baseaddress;
	UINT32                  m_size;
	UINT32                  m_rgnoffset;
	std::vector<UINT16>          m_decrypted_opcodes[256];
};


// ======================> fd1094_device

// base device, shared implementation between A and B variants
class fd1094_device : public m68000_device
{
public:
	typedef delegate<void (UINT8)> state_change_delegate;

	// construction/destruction
	fd1094_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// explicit decryption helpers
	void decrypt(offs_t baseaddr, UINT32 size, offs_t regionoffs, UINT16 *opcodesptr, UINT8 state) { decrypt(baseaddr, size, m_srcbase + regionoffs/2, opcodesptr, state); }

	// live state management
	UINT8 state() const { return m_irqmode ? m_key[0] : m_state; }
	void change_state(int newstate = -1);
	void notify_state_change(state_change_delegate callback) { m_state_change = callback; }

	// constants
	enum
	{
		STATE_RESET = 0x100,
		STATE_IRQ = 0x200,
		STATE_RTE = 0x300
	};

protected:
	required_memory_bank m_decrypted_opcodes_bank;

	// device overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_postload();

	// internal helpers
	UINT16 decrypt_one(offs_t address, UINT16 val, const UINT8 *main_key, UINT8 state, bool vector_fetch);
	void decrypt(offs_t baseaddr, UINT32 size, const UINT16 *srcptr, UINT16 *opcodesptr, UINT8 state);
	void default_state_change(UINT8 state);
	IRQ_CALLBACK_MEMBER( irq_callback );

	// static helpers
	DECLARE_WRITE32_MEMBER(cmp_callback);
	DECLARE_WRITE_LINE_MEMBER(rte_callback);

	// internal state
	UINT8                   m_state;
	bool                    m_irqmode;
	state_change_delegate   m_state_change;
	fd1094_decryption_cache m_cache;
	UINT16 *                m_srcbase;
	UINT32                  m_srcbytes;
	const UINT8 *           m_key;
	UINT8                   m_masked_opcodes_lookup[2][65536/8/2];

	// static tables
	static const UINT16 s_masked_opcodes[];
};

struct fd1094_constraint
{
	offs_t  pc;
	UINT16  state;
	UINT16  value;
	UINT16  mask;
};

#endif
