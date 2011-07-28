/***************************************************************************

    cothread.h

    Class wrapper around byuu's cothread library.

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

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __COTHREAD_H__
#define __COTHREAD_H__

#include "cothread/libco.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// delegate that points to a thread's entry
typedef delegate<void ()> cothread_entry_delegate;


// ======================> cothread

class cothread
{
public:
	// construction/destruction
	cothread(cothread_t existing_thread);
	cothread(cothread_entry_delegate entry, size_t stack = 1048576);
	~cothread() { if (m_creator_cothread != NULL) co_delete(m_cothread); }

	// switching
	void make_active() { co_switch(m_cothread); }

private:
	// internal helpers
	static void cothread_entry();

	// internal state
	cothread_t			m_cothread;
	cothread_t			m_creator_cothread;
	cothread_entry_delegate m_entry;

	// static state
	static osd_lock *	s_create_lock;
	static cothread *	s_create_cothread;
};


#endif	// __COTHREAD_H__
