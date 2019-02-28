// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldstub.h

    Laserdisc player stubs.

*************************************************************************/

#ifndef MAME_MACHINE_LDSTUB_H
#define MAME_MACHINE_LDSTUB_H

#pragma once

#include "laserdsc.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(PIONEER_PR7820,  pioneer_pr7820_device)
DECLARE_DEVICE_TYPE(PHILIPS_22VP932, philips_22vp932_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pioneer_pr7820_device

class pioneer_pr7820_device : public laserdisc_device
{
public:
	// construction/destruction
	pioneer_pr7820_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// input/output
	uint8_t data_available_r() { return CLEAR_LINE; }
	uint8_t ready_r() { return ASSERT_LINE; }
	uint8_t data_r() { return 0; }
	void data_w(uint8_t data) { }
	void enter_w(uint8_t data) { }

protected:
	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override { }
	virtual int32_t player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override { return fieldnum; }
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }
};


// ======================> philips_22vp932_device

class philips_22vp932_device : public laserdisc_device
{
public:
	// construction/destruction
	philips_22vp932_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// input/output
	uint8_t data_r() { return 0; }
	void data_w(uint8_t data) { }
	void enter_w(uint8_t data) { }

protected:
	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override { }
	virtual int32_t player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override { return fieldnum; }
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }
};


#endif // MAME_MACHINE_LDSTUB_H
