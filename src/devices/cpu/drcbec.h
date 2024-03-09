// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbec.h

    Interpreted C core back-end for the universal machine language.

***************************************************************************/

#ifndef MAME_CPU_DRCBEC_H
#define MAME_CPU_DRCBEC_H

#pragma once

#include "drcuml.h"
#include "drcbeut.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

union drcbec_instruction;

class drcbe_c : public drcbe_interface
{
public:
	// construction/destruction
	drcbe_c(drcuml_state &drcuml, device_t &device, drc_cache &cache, uint32_t flags, int modes, int addrbits, int ignorebits);
	virtual ~drcbe_c();

	// required overrides
	virtual void reset() override;
	virtual int execute(uml::code_handle &entry) override;
	virtual void generate(drcuml_block &block, const uml::instruction *instlist, uint32_t numinst) override;
	virtual bool hash_exists(uint32_t mode, uint32_t pc) override;
	virtual void get_info(drcbe_info &info) override;

private:
	// helpers
	void output_parameter(drcbec_instruction **dstptr, void **immedptr, int size, const uml::parameter &param);
	void fixup_label(void *parameter, drccodeptr labelcodeptr);
	int dmulu(uint64_t &dstlo, uint64_t &dsthi, uint64_t src1, uint64_t src2, bool flags);
	int dmuls(uint64_t &dstlo, uint64_t &dsthi, int64_t src1, int64_t src2, bool flags);
	uint32_t tzcount32(uint32_t value);
	uint64_t tzcount64(uint64_t value);

	// internal state
	drc_hash_table          m_hash;                 // hash table state
	drc_map_variables       m_map;                  // code map
	drc_label_list          m_labels;               // label list
	drc_label_fixup_delegate m_fixup_delegate;      // precomputed delegate

	static const uint32_t     s_condition_map[32];
	static uint64_t           s_immediate_zero;
};


#endif // MAME_CPU_DRCBEC_H
