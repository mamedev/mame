// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68307 SIM module */

#include "cpu/m68000/m68000.h"

class m68307cpu_device;


/* ports */
#define m68307SIM_PACNT (0x10)
#define m68307SIM_PADDR (0x12)
#define m68307SIM_PADAT (0x14)
#define m68307SIM_PBCNT (0x16)
#define m68307SIM_PBDDR (0x18)
#define m68307SIM_PBDAT (0x1a)


/* interrupt logic */
#define m68307SIM_LICR1 (0x20)
#define m68307SIM_LICR2 (0x22)
#define m68307SIM_PICR  (0x24)
#define m68307SIM_PIVR  (0x26)

/* used for the CS logic */
#define m68307SIM_BR0 (0x40)
#define m68307SIM_OR0 (0x42)
#define m68307SIM_BR1 (0x44)
#define m68307SIM_OR1 (0x46)
#define m68307SIM_BR2 (0x48)
#define m68307SIM_OR2 (0x4a)
#define m68307SIM_BR3 (0x4c)
#define m68307SIM_OR3 (0x4e)

class m68307_sim
{
	public:

	UINT16 m_pacnt; // 8-bit
	UINT16 m_paddr; // 8-bit
	UINT16 m_padat; // 8-bit

	UINT16 m_pbcnt;
	UINT16 m_pbddr;
	UINT16 m_pbdat;

	UINT16 m_pivr; // 8-bit

	UINT16 m_br[4];
	UINT16 m_or[4];
	UINT16 m_picr;
	UINT16 m_licr1;
	UINT16 m_licr2;


	void write_pacnt(UINT16 data, UINT16 mem_mask);
	void write_paddr(UINT16 data, UINT16 mem_mask);
	UINT16 read_padat(m68307cpu_device* m68k, address_space &space, UINT16 mem_mask);
	void write_padat(m68307cpu_device* m68k, address_space &space, UINT16 data, UINT16 mem_mask);

	void write_pbcnt(UINT16 data, UINT16 mem_mask);
	void write_pbddr(UINT16 data, UINT16 mem_mask);
	UINT16 read_pbdat(m68307cpu_device* m68k, address_space &space, UINT16 mem_mask);
	void write_pbdat(m68307cpu_device* m68k, address_space &space, UINT16 data, UINT16 mem_mask);



	void write_licr1(m68307cpu_device* m68k, UINT16 data, UINT16 mem_mask);
	void write_licr2(m68307cpu_device* m68k, UINT16 data, UINT16 mem_mask);
	void write_picr(m68307cpu_device* m68k, UINT16 data, UINT16 mem_mask);
	void write_pivr(m68307cpu_device* m68k, UINT16 data, UINT16 mem_mask);

	void reset(void);
};
