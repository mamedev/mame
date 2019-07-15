// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MACHINE_MSX_SWITCHED_H
#define MAME_MACHINE_MSX_SWITCHED_H

#pragma once


class msx_switched_interface
{
public:
	virtual DECLARE_READ8_MEMBER(switched_read) = 0;
	virtual DECLARE_WRITE8_MEMBER(switched_write) = 0;
};

#endif // MAME_MACHINE_MSX_SWITCHED_H
