// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  sparc_intf.h - Sun SPARC MMU interfaces

***************************************************************************/

#ifndef MAME_CPU_SPARC_SPARC_INTF_H
#define MAME_CPU_SPARC_SPARC_INTF_H

#pragma once

class sparc_mmu_host_interface
{
public:
	virtual ~sparc_mmu_host_interface() { }
	virtual void set_mae() = 0;
};

class sparc_mmu_interface
{
public:
	virtual ~sparc_mmu_interface() { }
	virtual uint32_t fetch_insn(const bool supervisor, const uint32_t offset) = 0;
	virtual uint32_t read_asi(uint8_t asi, uint32_t offset, uint32_t mem_mask) = 0;
	virtual void write_asi(uint8_t asi, uint32_t offset, uint32_t data, uint32_t mem_mask) = 0;
	virtual void set_host(sparc_mmu_host_interface *host) = 0;
};

#endif // MAME_CPU_SPARC_SPARC_INTF_H
