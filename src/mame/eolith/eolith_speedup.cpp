// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina,Pierpaolo Prazzoli
#include "emu.h"
#include "eolith_speedup.h"


/* Eolith Speedup Handling */

/*
  This uses triggers and a scanline counter to speed up the eolith games a bit
  in some cases this results in a 100% speedup
  e.g hidden catch 25% -> 50% speed ingame

  this could probably be done a bit better using timers
*/

void eolith_e1_speedup_state_base::speedup_read()
{
	/* for debug */
	//if ((m_maincpu->pc()!=m_speedup_address) && (m_speedup_vblank!=1) )
	//    printf("%s:eolith speedup_read data %02x\n",machine().describe_context().c_str(), m_speedup_vblank);

	if (m_speedup_vblank==0 && m_speedup_scanline < m_speedup_resume_scanline)
	{
		int pc = m_maincpu->pc();

		if ((pc==m_speedup_address) || (pc==m_speedup_address2))
		{
			m_maincpu->spin_until_trigger(1000);
		}
	}
}

static const struct
{
	const char *s_name;
	int speedup_address;
	int speedup_address2;
	int speedup_resume_scanline;

} eolith_speedup_table[] =
{
	// eolith.cpp
	{ "linkypip", 0x4000825c, -1,/*0x4000abae,*/ 240 }, // 2nd address is used on the planet cutscene between but idle skipping between levels, but seems too aggressive
	{ "ironfort", 0x40020854, -1, 240 },
	{ "ironfortc",0x40020234, -1, 240 },
	{ "hidnctch", 0x4000bba0, -1, 240 },
	{ "hidnctcha",0x4000bba0, -1, 240 },
	{ "raccoon",  0x40008204, -1, 240 },
	{ "puzzlekg", 0x40029458, -1, 240 },
	{ "hidctch2", 0x40009524, -1, 240 },
	{ "hidctch2a",0x40029B58, -1, 240 },
	{ "landbrk",  0x40023574, -1, 240 },
	{ "landbrka", 0x4002446c, -1, 240 },
	{ "landbrkb", 0x40023B28, -1, 240 },
	{ "nhidctch", 0x40012778, -1, 240 },
	{ "hidctch3", 0x4001f6a0, -1, 240 },
	{ "fort2b",   0x000081e0, -1, 240 },
	{ "fort2ba",  0x000081e0, -1, 240 },
	{ "penfan",   0x4001FA66, -1, 240 },
	{ "penfana",  0x4001FAb6, -1, 240 },
	{ "candy",    0x4001990C, -1, 240 },
	{ "hidnc2k",  0x40016824, -1, 240 },
	// eolith16.cpp
	{ "klondkp",  0x0001a046, -1, 240 },
	// vegaeo.cpp
	{ "crazywar", 0x00008cf8, -1, 240 }
};


void eolith_e1_speedup_state_base::init_speedup()
{
	m_speedup_address = 0;
	m_speedup_address2 = 0;
	m_speedup_resume_scanline = 0;
	m_speedup_vblank = 0;
	m_speedup_scanline = 0;

	for (const auto &speedups : eolith_speedup_table)
	{
		if (strcmp(machine().system().name, speedups.s_name) == 0)
		{
			m_speedup_address = speedups.speedup_address;
			m_speedup_address2 = speedups.speedup_address2;
			m_speedup_resume_scanline = speedups.speedup_resume_scanline;
			break;
		}
	}

	save_item(NAME(m_speedup_vblank));
	save_item(NAME(m_speedup_scanline));
}

/* todo, use timers instead! */
TIMER_DEVICE_CALLBACK_MEMBER(eolith_e1_speedup_state_base::eolith_speedup)
{
	if (param == 0)
		m_speedup_vblank = 0;

	if (param == m_speedup_resume_scanline)
		machine().scheduler().trigger(1000);

	if (param == 240)
		m_speedup_vblank = 1;
}

int eolith_e1_speedup_state_base::speedup_vblank_r()
{
//  printf("%s:eolith speedup_read data %02x\n",machine().describe_context().c_str(), m_speedup_vblank);


	return (m_screen->vpos() >= 240);
}

// StealSee doesn't use interrupts, just the vblank
int eolith_e1_speedup_state_base::stealsee_speedup_vblank_r()
{
	int pc = m_maincpu->pc();

	if (pc == 0x400081ec)
		if (!m_speedup_vblank)
			m_maincpu->eat_cycles(500);

	return (m_screen->vpos() >= 240);
}
