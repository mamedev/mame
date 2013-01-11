/***************************************************************************

    drcbec.h

    Interpreted C core back-end for the universal machine language.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
	virtual void reset();
	virtual int execute(uml::code_handle &entry);
	virtual void generate(drcuml_block &block, const uml::instruction *instlist, UINT32 numinst);
	virtual bool hash_exists(UINT32 mode, UINT32 pc);
	virtual void get_info(drcbe_info &info);

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
