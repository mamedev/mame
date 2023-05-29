// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    divtlb.h

    Generic virtual TLB implementation.

***************************************************************************/

#ifndef MAME_EMU_DIVTLB_H
#define MAME_EMU_DIVTLB_H

#pragma once

class device_vtlb_interface : public device_interface
{
public:
	using vtlb_entry = u32;

	// Translation type flags
	// TR_READ/WRITE/FETCH come from device_memory_interface
	enum {
		TR_UREAD  = 4,
		TR_UWRITE = 5,
		TR_UFETCH = 6,

		TR_TYPE   = 3,
		TR_USER   = 4
	};

	enum {
		FLAGS_MASK           = 0xff,

		READ_ALLOWED         = 0x01,     /* (1 << TR_READ) */
		WRITE_ALLOWED        = 0x02,     /* (1 << TR_WRITE) */
		FETCH_ALLOWED        = 0x04,     /* (1 << TR_FETCH) */
		FLAG_VALID           = 0x08,
		USER_READ_ALLOWED    = 0x10,     /* (1 << TR_UREAD) */
		USER_WRITE_ALLOWED   = 0x20,     /* (1 << TR_UWRITE) */
		USER_FETCH_ALLOWED   = 0x40,     /* (1 << TR_UFETCH) */
		FLAG_FIXED           = 0x80
	};

	// construction/destruction
	device_vtlb_interface(const machine_config &mconfig, device_t &device, int space);
	virtual ~device_vtlb_interface();

	// configuration helpers
	void set_vtlb_dynamic_entries(int entries) { m_dynamic = entries; }
	void set_vtlb_fixed_entries(int entries) { m_fixed = entries; }

	// filling
	bool vtlb_fill(offs_t address, offs_t taddress, int intention);
	void vtlb_load(int entrynum, int numpages, offs_t address, vtlb_entry value);
	void vtlb_dynload(u32 index, offs_t address, vtlb_entry value);

	// flushing
	void vtlb_flush_dynamic();
	void vtlb_flush_address(offs_t address);

	// accessors
	const vtlb_entry *vtlb_table() const;

protected:
	// interface-level overrides
	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void interface_pre_reset() override;

private:
	// private state
	int    m_space;            // address space
	int                 m_dynamic;          // number of dynamic entries
	int                 m_fixed;            // number of fixed entries
	int                 m_dynindex;         // index of next dynamic entry
	int                 m_pageshift;        // bits to shift to get page index
	int                 m_addrwidth;        // logical address bus width
	std::vector<offs_t> m_live;             // array of live entries by table index
	std::vector<int>    m_fixedpages;       // number of pages each fixed entry covers
	std::vector<vtlb_entry> m_table;        // table of entries by address
	std::vector<offs_t> m_refcnt;           // table of entry reference counts by address
	vtlb_entry          *m_table_base;      // pointer to m_table[0]
};


#endif /* MAME_EMU_DIVTLB_H */
