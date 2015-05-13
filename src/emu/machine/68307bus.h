// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "cpu/m68000/m68000.h"

#define m68307BUS_MADR (0x01)
#define m68307BUS_MFDR (0x03)
#define m68307BUS_MBCR (0x05)
#define m68307BUS_MBSR (0x07)
#define m68307BUS_MBDR (0x09)


class m68307_mbus
{
	public:

	UINT16 m_MFCR;

	bool m_busy;
	bool m_intpend;

	void reset(void);
};
