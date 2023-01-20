// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

uint32_t arcompact_device::handleop32_ASLS(uint32_t op)  { return arcompact_handle04_helper(op, "ASLS", 0,0); }
uint32_t arcompact_device::handleop32_ASRS(uint32_t op)  { return arcompact_handle04_helper(op, "ASRS", 0,0); }

uint32_t arcompact_device::handleop32_ADDSDW(uint32_t op)  { return arcompact_handle04_helper(op, "ADDSDW", 0,0); }
uint32_t arcompact_device::handleop32_SUBSDW(uint32_t op)  { return arcompact_handle04_helper(op, "SUBSDW", 0,0); }

