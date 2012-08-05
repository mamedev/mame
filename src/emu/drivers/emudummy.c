/***************************************************************************

    emudummy.c

    Dummy driver file that references CPU devices which are in turn
    referenced by devices in libemu.a.

    The reason we need this is due to link ordering issues with gcc
    if the actual drivers being linked don't reference these CPU
    devices. Since we link libcpu first, if libemu needs stuff from
    libcpu that wasn't previously referenced, it will fail the link.

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
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"


MACHINE_CONFIG_START( __dummy, driver_device )
	MCFG_CPU_ADD("dummy1", I8049, 1000000)
	MCFG_CPU_ADD("dummy2", I8748, 1000000)
	MCFG_CPU_ADD("dummy3", Z80, 1000000)
MACHINE_CONFIG_END


ROM_START( __dummy )
	ROM_REGION( 0x1000, "dummy1", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000, "dummy2", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000, "dummy3", ROMREGION_ERASEFF )
ROM_END


GAME( 1900, __dummy, 0, __dummy, 0, driver_device, 0, ROT0, "(none)", "Dummy", GAME_NO_SOUND )


