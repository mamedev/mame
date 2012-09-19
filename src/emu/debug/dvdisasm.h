/*********************************************************************

    dvdisasm.h

    Disassembly debugger view.

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

#ifndef __DVDISASM_H__
#define __DVDISASM_H__

#include "debugvw.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// selections for what goes into the right-hand column
enum disasm_right_column
{
	DASM_RIGHTCOL_NONE,
	DASM_RIGHTCOL_RAW,
	DASM_RIGHTCOL_ENCRYPTED,
	DASM_RIGHTCOL_COMMENTS
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// a disassembly view_source
class debug_view_disasm_source : public debug_view_source
{
	friend class debug_view_disasm;

	// construction/destruction
	debug_view_disasm_source(const char *name, device_t &device);

public:
	// getters
	device_t &device() const { return m_device; }
	address_space &space() const { return m_space; }

private:
	// internal state
	device_t &			m_device;				// underlying device
	device_disasm_interface *m_disasmintf;		// disassembly interface
	address_space &		m_space;				// address space to display
};


// debug view for disassembly
class debug_view_disasm : public debug_view
{
	friend resource_pool_object<debug_view_disasm>::~resource_pool_object();
	friend class debug_view_manager;

	// construction/destruction
	debug_view_disasm(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_disasm();

public:
	// getters
	const char *expression() const { return m_expression.string(); }
	disasm_right_column right_column() const { return m_right_column; }
	UINT32 backward_steps() const { return m_backwards_steps; }
	UINT32 disasm_width() const { return m_dasm_width; }
	offs_t selected_address();

	// setters
	void set_expression(const char *expression);
	void set_right_column(disasm_right_column contents);
	void set_backward_steps(UINT32 steps);
	void set_disasm_width(UINT32 width);
	void set_selected_address(offs_t address);

protected:
	// view overrides
	virtual void view_update();
	virtual void view_notify(debug_view_notification type);
	virtual void view_char(int chval);

private:
	// internal helpers
	void enumerate_sources();
	offs_t find_pc_backwards(offs_t targetpc, int numinstrs);
	void generate_bytes(offs_t pcbyte, int numbytes, int minbytes, char *string, int maxchars, bool encrypted);
	bool recompute(offs_t pc, int startline, int lines);

	// internal state
	disasm_right_column	m_right_column;			// right column contents
	UINT32				m_backwards_steps;		// number of backwards steps
	UINT32				m_dasm_width;			// width of the disassembly area
	UINT8 *				m_last_direct_raw;		// last direct raw value
	UINT8 *				m_last_direct_decrypted;// last direct decrypted value
	UINT32				m_last_change_count;	// last comment change count
	offs_t				m_last_pcbyte;			// last PC byte value
	int					m_divider1, m_divider2;	// left and right divider columns
	int					m_divider3;				// comment divider column
	debug_view_expression m_expression;			// expression-related information
	debug_view_xy		m_allocated;			// allocated rows/columns
	offs_t *			m_byteaddress;			// addresses of the instructions
	char *				m_dasm;					// disassembled instructions

	// constants
	static const int DEFAULT_DASM_LINES = 1000;
	static const int DEFAULT_DASM_WIDTH = 50;
	static const int DASM_MAX_BYTES = 16;
};


#endif
