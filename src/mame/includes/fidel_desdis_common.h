// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************
*
*  Fidelity Electronics Designer Display common class
*
******************************************************************************/

#pragma once

#ifndef MAME_INCLUDES_DESDIS_COMMON_H
#define MAME_INCLUDES_DESDIS_COMMON_H

#include "includes/fidelbase.h"

class desdis_common_state : public fidelbase_state
{
public:
	desdis_common_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag)
	{ }

protected:
	// I/O handlers
	virtual DECLARE_WRITE8_MEMBER(control_w);
	virtual DECLARE_WRITE8_MEMBER(lcd_w);
	virtual DECLARE_READ8_MEMBER(input_r);
};


INPUT_PORTS_EXTERN( desdis );

#endif // MAME_INCLUDES_DESDIS_COMMON_H
