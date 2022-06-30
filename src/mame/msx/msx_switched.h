// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MACHINE_MSX_SWITCHED_H
#define MAME_MACHINE_MSX_SWITCHED_H

#pragma once


class msx_switched_interface
{
public:
	virtual uint8_t switched_read(offs_t offset) = 0;
	virtual void switched_write(offs_t offset, uint8_t data) = 0;
};

#endif // MAME_MACHINE_MSX_SWITCHED_H
