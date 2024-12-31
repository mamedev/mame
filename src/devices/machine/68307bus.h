// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_68307BUS_H
#define MAME_MACHINE_68307BUS_H

#pragma once

#include "68307.h"


class m68307_cpu_device::m68307_mbus
{
public:
	uint16_t m_MFCR;

	bool m_busy;
	bool m_intpend;

	void reset();
};

#endif // MAME_MACHINE_68307BUS_H
