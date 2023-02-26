// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MSX_MSX_SWITCHED_H
#define MAME_MSX_MSX_SWITCHED_H

#pragma once


class msx_switched_interface
{
public:
	virtual u8 switched_read(offs_t offset) = 0;
	virtual void switched_write(offs_t offset, u8 data) = 0;
};

#endif // MAME_MSX_MSX_SWITCHED_H
