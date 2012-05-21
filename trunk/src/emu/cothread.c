/***************************************************************************

    cothread.c

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

#include "emu.h"
#include "cothread/libco.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// static members of the cothread class
osd_lock *cothread::s_create_lock = osd_lock_alloc();
cothread *cothread::s_create_cothread;



//**************************************************************************
//  COTHREAD
//**************************************************************************

//-------------------------------------------------
//  cothread - constructor
//-------------------------------------------------

cothread::cothread(cothread_t existing_thread)
	: m_cothread(existing_thread),
	  m_creator_cothread(NULL)
{
}

cothread::cothread(cothread_entry_delegate entry, size_t stack)
	: m_cothread(NULL),
	  m_creator_cothread(co_active()),
	  m_entry(entry)
{
	// due to the lack of input parameter to the entry function,
	// all cothread creation is explicitly serialized
	osd_lock_acquire(s_create_lock);
	s_create_cothread = this;
	m_cothread = co_create(stack, &cothread_entry);
	co_switch(m_cothread);
	osd_lock_release(s_create_lock);
}


//-------------------------------------------------
//  cothread_entry - static entry point
//-------------------------------------------------

void cothread::cothread_entry()
{
	// on first call, retrieve the static pointer and return
	cothread *thread = s_create_cothread;
	co_switch(thread->m_creator_cothread);

	// when actually swapped in later, call the entry point
	thread->m_entry();
}
