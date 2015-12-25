// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbec.h

    Interpreted C core back-end for the universal machine language.

***************************************************************************/

#pragma once

#ifndef __DRCBEC_H__
#define __DRCBEC_H__

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
	drcbe_c(drcuml_state &drcuml, device_t &device, drc_cache &cache, UINT32 flags, int modes, int addrbits, int ignorebits);
	virtual ~drcbe_c();

	// required overrides
	virtual void reset() override;
	virtual int execute(uml::code_handle &entry) override;
	virtual void generate(drcuml_block &block, const uml::instruction *instlist, UINT32 numinst) override;
	virtual bool hash_exists(UINT32 mode, UINT32 pc) override;
	virtual void get_info(drcbe_info &info) override;

private:
	// helpers
	void output_parameter(drcbec_instruction **dstptr, void **immedptr, int size, const uml::parameter &param);
	void fixup_label(void *parameter, drccodeptr labelcodeptr);
	int dmulu(UINT64 &dstlo, UINT64 &dsthi, UINT64 src1, UINT64 src2, int flags);
	int dmuls(UINT64 &dstlo, UINT64 &dsthi, INT64 src1, INT64 src2, int flags);

	// internal state
	drc_hash_table          m_hash;                 // hash table state
	drc_map_variables       m_map;                  // code map
	drc_label_list          m_labels;               // label list
	drc_label_fixup_delegate m_fixup_delegate;      // precomputed delegate

	static const UINT32     s_condition_map[32];
	static UINT64           s_immediate_zero;
};


#endif /* __DRCBEC_H__ */
