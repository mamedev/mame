// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldstub.h

    Laserdisc player stubs.

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
		: laserdisc_device(mconfig, SONY_LDP1450, "Sony LDP-1450", tag, owner, clock, "ldp1450", __FILE__) { }

	// input/output
	UINT8 data_available_r() { return CLEAR_LINE; }
	UINT8 data_r() { return 0; }
	void data_w(UINT8 data) { }

protected:
	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override { }
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override { return fieldnum; }
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }
};


// ======================> pioneer_pr7820_device

class pioneer_pr7820_device : public laserdisc_device
{
public:
	// construction/destruction
	pioneer_pr7820_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: laserdisc_device(mconfig, PIONEER_PR7820, "Pioneer PR-7820", tag, owner, clock, "pr7820", __FILE__) { }

	// input/output
	UINT8 data_available_r() { return CLEAR_LINE; }
	UINT8 ready_r() { return ASSERT_LINE; }
	UINT8 data_r() { return 0; }
	void data_w(UINT8 data) { }
	void enter_w(UINT8 data) { }

protected:
	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override { }
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override { return fieldnum; }
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }
};


// ======================> phillips_22vp932_device

class phillips_22vp932_device : public laserdisc_device
{
public:
	// construction/destruction
	phillips_22vp932_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: laserdisc_device(mconfig, PHILLIPS_22VP932, "Phillips 22VP932", tag, owner, clock, "22vp932", __FILE__) { }

	// input/output
	UINT8 data_r() { return 0; }
	void data_w(UINT8 data) { }
	void enter_w(UINT8 data) { }

protected:
	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override { }
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override { return fieldnum; }
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }
};


#endif
