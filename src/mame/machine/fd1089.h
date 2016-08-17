// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Andreas Naive, Charles MacDonald
/***************************************************************************

    Hitachi FD1089A/FD1089B encryption emulation

****************************************************************************

    Copyright Nicola Salmoria, Andreas Naive, and Charles MacDonald.

    All rights reserved.

***************************************************************************/

#ifndef __FD1089_H__
#define __FD1089_H__

#include "cpu/m68000/m68000.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// device type definition
extern const device_type FD1089A;
extern const device_type FD1089B;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fd1089_base_device

// base device, shared implementation between A and B variants
class fd1089_base_device : public m68000_device
{
public:
	// construction/destruction
	fd1089_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// explicit decryption helpers
	void decrypt(offs_t baseaddr, UINT32 size, offs_t regionoffs, UINT16 *opcodesptr, UINT16 *dataptr) { decrypt(baseaddr, size, &m_plaintext[regionoffs/2], opcodesptr, dataptr); }

protected:
	// device overrides
	virtual void device_start() override;

	// internal helpers
	UINT8 rearrange_key(UINT8 table, bool opcode);
	virtual UINT8 decode(UINT8 val, UINT8 key, bool opcode) = 0;
	UINT16 decrypt_one(offs_t addr, UINT16 val, const UINT8 *key, bool opcode);
	void decrypt(offs_t baseaddr, UINT32 size, const UINT16 *srcptr, UINT16 *opcodesptr, UINT16 *dataptr);

	// internal state
	required_memory_region       m_region;
	required_region_ptr<UINT8>   m_key;
	std::vector<UINT16>          m_plaintext;
	required_shared_ptr<UINT16>  m_decrypted_opcodes;

	// internal types
	struct decrypt_parameters
	{
		UINT8 xorval;
		UINT8 s7,s6,s5,s4,s3,s2,s1,s0;
	};

	// static tables
	static const UINT8 s_basetable_fd1089[0x100];
	static const decrypt_parameters s_addr_params[16];
	static const decrypt_parameters s_data_params_a[16];
};


// ======================> fd1089a_device

// FD1089A variant
class fd1089a_device : public fd1089_base_device
{
public:
	// construction/destruction
	fd1089a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual UINT8 decode(UINT8 val, UINT8 key, bool opcode) override;
};


// ======================> fd1089b_device

// FD1089B variant
class fd1089b_device : public fd1089_base_device
{
public:
	// construction/destruction
	fd1089b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual UINT8 decode(UINT8 val, UINT8 key, bool opcode) override;
};


#endif
