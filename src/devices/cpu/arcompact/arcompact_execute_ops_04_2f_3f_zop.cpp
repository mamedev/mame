// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


uint32_t arcompact_device::handleop32_SLEEP(uint32_t op)  { arcompact_log("SLEEP (%08x)", op); return m_pc + (4 >> 0);}
uint32_t arcompact_device::handleop32_SWI(uint32_t op)  { arcompact_log("SWI / TRAP0 (%08x)", op); return m_pc + (4 >> 0);}
uint32_t arcompact_device::handleop32_SYNC(uint32_t op)  { arcompact_log("SYNC (%08x)", op); return m_pc + (4 >> 0);}
uint32_t arcompact_device::handleop32_RTIE(uint32_t op)  { arcompact_log("RTIE (%08x)", op); return m_pc + (4 >> 0);}
uint32_t arcompact_device::handleop32_BRK(uint32_t op)  { arcompact_log("BRK (%08x)", op); return m_pc + (4 >> 0);}

