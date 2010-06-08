/***************************************************************************

    didisasm.c

    Device disasm interfaces.

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


//**************************************************************************
//  DEVICE CONFIG DISASM INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_config_disasm_interface - constructor
//-------------------------------------------------

device_config_disasm_interface::device_config_disasm_interface(const machine_config &mconfig, device_config &devconfig)
	: device_config_interface(mconfig, devconfig)
{
}


//-------------------------------------------------
//  ~device_config_disasm_interface - destructor
//-------------------------------------------------

device_config_disasm_interface::~device_config_disasm_interface()
{
}



//**************************************************************************
//  DEVICE DISASM INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_disasm_interface - constructor
//-------------------------------------------------

device_disasm_interface::device_disasm_interface(running_machine &machine, const device_config &config, device_t &device)
	: device_interface(machine, config, device),
	  m_disasm_config(dynamic_cast<const device_config_disasm_interface &>(config))
{
}


//-------------------------------------------------
//  ~device_disasm_interface - destructor
//-------------------------------------------------

device_disasm_interface::~device_disasm_interface()
{
}
