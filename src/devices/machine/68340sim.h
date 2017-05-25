// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_68340SIM_H
#define MAME_MACHINE_68340SIM_H

#pragma once


class m68340_sim
{
public:
	uint32_t m_am[4];
	uint32_t m_ba[4];

	void reset();
};

#endif // MAME_MACHINE_68340SIM_H
