// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

uint32_t arcompact_device::handleop32_ARC_EXT06(uint32_t op)
{
	fatalerror("op a,b,c (06 ARC ext) (%08x)", op);
	return m_pc + 4;
}

uint32_t arcompact_device::handleop32_USER_EXT07(uint32_t op)
{
	fatalerror("op a,b,c (07 User ext) (%08x)", op);
	return m_pc + 4;
}

uint32_t arcompact_device::handleop32_USER_EXT08(uint32_t op)
{
	fatalerror("op a,b,c (08 User ext) (%08x)", op);
	return m_pc + 4;
}

uint32_t arcompact_device::handleop32_MARKET_EXT09(uint32_t op)
{
	fatalerror("op a,b,c (09 Market ext) (%08x)", op);
	return m_pc + 4;
}

uint32_t arcompact_device::handleop32_MARKET_EXT0a(uint32_t op)
{
	fatalerror("op a,b,c (0a Market ext) (%08x)", op);
	return m_pc + 4;
}

uint32_t arcompact_device::handleop32_MARKET_EXT0b(uint32_t op)
{
	fatalerror("op a,b,c (0b Market ext) (%08x)", op);
	return m_pc + 4;
}
