/*************************************************************************

    ldstub.h

    Laserdisc player stubs.

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

*************************************************************************/

#pragma once

#ifndef __LDSTUB_H__
#define __LDSTUB_H__

#include "laserdsc.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LASERDISC_LDP1450_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SONY_LDP1450, 0)
#define MCFG_LASERDISC_PR7820_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PIONEER_PR7820, 0)
#define MCFG_LASERDISC_22VP932_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PHILLIPS_22VP932, 0)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type SONY_LDP1450;
extern const device_type PIONEER_PR7820;
extern const device_type PHILLIPS_22VP932;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sony_ldp1450_device

class sony_ldp1450_device : public laserdisc_device
{
public:
	// construction/destruction
	sony_ldp1450_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: laserdisc_device(mconfig, SONY_LDP1450, "Sony LDP-1450", "ldp1450", tag, owner, clock) { }

	// input/output
	UINT8 data_available_r() { return CLEAR_LINE; }
	UINT8 data_r() { return 0; }
	void data_w(UINT8 data) { }

protected:
	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, attotime curtime) { }
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, attotime curtime) { return fieldnum; }
	virtual void player_overlay(bitmap_yuy16 &bitmap) { }
};


// ======================> pioneer_pr7820_device

class pioneer_pr7820_device : public laserdisc_device
{
public:
	// construction/destruction
	pioneer_pr7820_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: laserdisc_device(mconfig, PIONEER_PR7820, "Pioneer PR-7820", "pr7820", tag, owner, clock) { }

	// input/output
	UINT8 data_available_r() { return CLEAR_LINE; }
	UINT8 ready_r() { return ASSERT_LINE; }
	UINT8 data_r() { return 0; }
	void data_w(UINT8 data) { }
	void enter_w(UINT8 data) { }

protected:
	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, attotime curtime) { }
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, attotime curtime) { return fieldnum; }
	virtual void player_overlay(bitmap_yuy16 &bitmap) { }
};


// ======================> phillips_22vp932_device

class phillips_22vp932_device : public laserdisc_device
{
public:
	// construction/destruction
	phillips_22vp932_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: laserdisc_device(mconfig, PHILLIPS_22VP932, "Phillips 22VP932", "22vp932", tag, owner, clock) { }

	// input/output
	UINT8 data_r() { return 0; }
	void data_w(UINT8 data) { }
	void enter_w(UINT8 data) { }

protected:
	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, attotime curtime) { }
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, attotime curtime) { return fieldnum; }
	virtual void player_overlay(bitmap_yuy16 &bitmap) { }
};


#endif
