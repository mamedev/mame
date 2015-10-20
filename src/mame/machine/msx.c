// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
 * msx.c: MSX emulation
 *
 * Copyright (C) 2004 Sean Young
 *
 * Todo:
 *
 * - fix mouse support
 * - cassette support doesn't work
 * - Ensure changing cartridge after boot works
 * - wd2793, nms8255
 */

#include "includes/msx.h"

#define VERBOSE 0


void msx_state::msx_irq_source(int source, int level)
{
	assert(source >= 0 && source < ARRAY_LENGTH(m_irq_state));

	m_irq_state[source] = level;
	check_irq();
}


void msx_state::check_irq()
{
	int state = CLEAR_LINE;

	for (int i = 0; i < ARRAY_LENGTH(m_irq_state); i++)
	{
		if (m_irq_state[i] != CLEAR_LINE)
		{
			state = ASSERT_LINE;
		}
	}

	m_maincpu->set_input_line(0, state);
}


void msx_state::machine_reset()
{
	msx_memory_reset ();
	msx_memory_map_all ();
	for (int i = 0; i < ARRAY_LENGTH(m_irq_state); i++)
	{
		m_irq_state[i] = CLEAR_LINE;
	}
	check_irq();
}


void msx_state::machine_start()
{
	m_port_c_old = 0xff;
}


static const UINT8 cc_op[0x100] = {
	4+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1, 4+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	8+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,12+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	7+1,10+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1, 7+1,11+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	7+1,10+1,13+1, 6+1,11+1,11+1,10+1, 4+1, 7+1,11+1,13+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	7+1, 7+1, 7+1, 7+1, 7+1, 7+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	5+1,10+1,10+1,10+1,10+1,11+1, 7+1,11+1, 5+1,10+1,10+1, 0  ,10+1,17+1, 7+1,11+1,
	5+1,10+1,10+1,11+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1,11+1,10+1, 0  , 7+1,11+1,
	5+1,10+1,10+1,19+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1, 4+1,10+1, 0  , 7+1,11+1,
	5+1,10+1,10+1, 4+1,10+1,11+1, 7+1,11+1, 5+1, 6+1,10+1, 4+1,10+1, 0  , 7+1,11+1
};

static const UINT8 cc_cb[0x100] = {
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2
};

static const UINT8 cc_ed[0x100] = {
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2,18+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2,18+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 8+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,
16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
	8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2
};

static const UINT8 cc_xy[0x100] = {
	4+4+2,10+4+2, 7+4+2, 6+4+2, 4+4+2, 4+4+2, 7+4+2, 4+4+2, 4+4+2,11+4+2, 7+4+2, 6+4+2, 4+4+2, 4+4+2, 7+4+2, 4+4+2,
	8+4+2,10+4+2, 7+4+2, 6+4+2, 4+4+2, 4+4+2, 7+4+2, 4+4+2,12+4+2,11+4+2, 7+4+2, 6+4+2, 4+4+2, 4+4+2, 7+4+2, 4+4+2,
	7+4+2,10+4+2,16+4+2, 6+4+2, 4+4+2, 4+4+2, 7+4+2, 4+4+2, 7+4+2,11+4+2,16+4+2, 6+4+2, 4+4+2, 4+4+2, 7+4+2, 4+4+2,
	7+4+2,10+4+2,13+4+2, 6+4+2,23  +2,23  +2,19  +2, 4+4+2, 7+4+2,11+4+2,13+4+2, 6+4+2, 4+4+2, 4+4+2, 7+4+2, 4+4+2,
	4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2,
	4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2,
	4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2,
19  +2,19  +2,19  +2,19  +2,19  +2,19  +2, 4+4+2,19  +2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2,
	4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2,
	4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2,
	4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2,
	4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2, 4+4+2,19  +2, 4+4+2,
	5+4+2,10+4+2,10+4+2,10+4+2,10+4+2,11+4+2, 7+4+2,11+4+2, 5+4+2,10+4+2,10+4+2, 0    ,10+4+2,17+4+2, 7+4+2,11+4+2,
	5+4+2,10+4+2,10+4+2,11+4+2,10+4+2,11+4+2, 7+4+2,11+4+2, 5+4+2, 4+4+2,10+4+2,11+4+2,10+4+2, 4  +1, 7+4+2,11+4+2,
	5+4+2,10+4+2,10+4+2,19+4+2,10+4+2,11+4+2, 7+4+2,11+4+2, 5+4+2, 4+4+2,10+4+2, 4+4+2,10+4+2, 4  +1, 7+4+2,11+4+2,
	5+4+2,10+4+2,10+4+2, 4+4+2,10+4+2,11+4+2, 7+4+2,11+4+2, 5+4+2, 6+4+2,10+4+2, 4+4+2,10+4+2, 4  +1, 7+4+2,11+4+2
};

static const UINT8 cc_xycb[0x100] = {
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2
};

/* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
static const UINT8 cc_ex[0x100] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* DJNZ */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NZ/JR Z */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NC/JR C */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0, /* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2+1
};


void msx_state::driver_start()
{
	m_maincpu->set_input_line_vector(0, 0xff);

	msx_memory_init();

	m_maincpu->z80_set_cycle_tables( cc_op, cc_cb, cc_ed, cc_xy, cc_xycb, cc_ex );

	save_item(NAME(m_psg_b));
	save_item(NAME(m_mouse));
	save_item(NAME(m_mouse_stat));
	save_item(NAME(m_rtc_latch));
	save_item(NAME(m_kanji_latch));
	save_item(NAME(m_slot_expanded));
	save_item(NAME(m_primary_slot));
	save_item(NAME(m_secondary_slot));
	save_item(NAME(m_port_c_old));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_current_switched_device));
	save_item(NAME(m_irq_state));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_state::post_load), this));
}

void msx_state::post_load()
{
	for (int page = 0; page < 4; page++)
	{
		int slot_primary = (m_primary_slot >> (page * 2)) & 3;
		int slot_secondary = (m_secondary_slot[slot_primary] >> (page * 2)) & 3;

		m_current_page[page] = m_all_slots[slot_primary][slot_secondary][page];
	}
}

INTERRUPT_GEN_MEMBER(msx_state::msx_interrupt)
{
	m_mouse[0] = m_io_mouse0->read();
	m_mouse_stat[0] = -1;
	m_mouse[1] = m_io_mouse1->read();
	m_mouse_stat[1] = -1;
}

/*
** The I/O functions
*/


READ8_MEMBER(msx_state::msx_psg_port_a_r)
{
	UINT8 data;

	data = (m_cassette->input() > 0.0038 ? 0x80 : 0);

	if ( (m_psg_b ^ m_io_dsw->read() ) & 0x40)
	{
		/* game port 2 */
		UINT8 inp = m_io_joy1->read();
		if ( !(inp & 0x80) )
		{
			/* joystick */
			data |= ( inp & 0x7f );
		}
		else
		{
			/* mouse */
			data |= ( inp & 0x70 );
			if (m_mouse_stat[1] < 0)
				data |= 0xf;
			else
				data |= ~(m_mouse[1] >> (4*m_mouse_stat[1]) ) & 15;
		}
	}
	else
	{
		/* game port 1 */
		UINT8 inp = m_io_joy0->read();
		if ( !(inp & 0x80) )
		{
			/* joystick */
			data |= ( inp & 0x7f );
		}
		else
		{
			/* mouse */
			data |= ( inp & 0x70 );
			if (m_mouse_stat[0] < 0)
				data |= 0xf;
			else
				data |= ~(m_mouse[0] >> (4*m_mouse_stat[0]) ) & 15;
		}
	}

	return data;
}

READ8_MEMBER(msx_state::msx_psg_port_b_r)
{
	return m_psg_b;
}

WRITE8_MEMBER(msx_state::msx_psg_port_a_w)
{
}

WRITE8_MEMBER(msx_state::msx_psg_port_b_w)
{
	/* Arabic or kana mode led */
	if ( (data ^ m_psg_b) & 0x80)
		set_led_status (machine(), 2, !(data & 0x80) );

	if ( (m_psg_b ^ data) & 0x10)
	{
		if (++m_mouse_stat[0] > 3) m_mouse_stat[0] = -1;
	}
	if ( (m_psg_b ^ data) & 0x20)
	{
		if (++m_mouse_stat[1] > 3) m_mouse_stat[1] = -1;
	}

	m_psg_b = data;
}


/*
** RTC functions
*/

WRITE8_MEMBER( msx_state::msx_rtc_latch_w )
{
	m_rtc_latch = data & 15;
}

WRITE8_MEMBER( msx_state::msx_rtc_reg_w )
{
	m_rtc->write(space, m_rtc_latch, data);
}

READ8_MEMBER( msx_state::msx_rtc_reg_r )
{
	return m_rtc->read(space, m_rtc_latch);
}


/*
** The PPI functions
*/

WRITE8_MEMBER( msx_state::msx_ppi_port_a_w )
{
	m_primary_slot = data;

	if (VERBOSE)
		logerror ("write to primary slot select: %02x\n", m_primary_slot);
	msx_memory_map_all ();
}

WRITE8_MEMBER( msx_state::msx_ppi_port_c_w )
{
	m_keylatch = data & 0x0f;

	/* caps lock */
	if ( BIT(m_port_c_old ^ data, 6) )
		set_led_status (machine(), 1, !BIT(data, 6) );

	/* key click */
	if ( BIT(m_port_c_old ^ data, 7) )
		m_dac->write_signed8(BIT(data, 7) ? 0x7f : 0);

	/* cassette motor on/off */
	if ( BIT(m_port_c_old ^ data, 4) )
		m_cassette->change_state(BIT(data, 4) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

	/* cassette signal write */
	if ( BIT(m_port_c_old ^ data, 5) )
		m_cassette->output(BIT(data, 5) ? -1.0 : 1.0);

	m_port_c_old = data;
}

READ8_MEMBER( msx_state::msx_ppi_port_b_r )
{
	UINT8 result = 0xff;
	int row, data;
	ioport_port *keynames[] = { m_io_key0, m_io_key1, m_io_key2, m_io_key3, m_io_key4, m_io_key5 };

	row = m_keylatch;
	if (row <= 10)
	{
		data = keynames[row / 2]->read();

		if (BIT(row, 0))
			data >>= 8;
		result = data & 0xff;
	}
	return result;
}

/************************************************************************
 *
 * New memory emulation !!
 *
 ***********************************************************************/

void msx_state::install_slot_pages(device_t &owner, UINT8 prim, UINT8 sec, UINT8 page, UINT8 numpages, device_t *device)
{
	msx_state &msx = downcast<msx_state &>(owner);
	msx_internal_slot_interface *internal_slot = dynamic_cast<msx_internal_slot_interface *>(device);

	for ( int i = page; i < MIN(page + numpages, 4); i++ )
	{
		msx.m_all_slots[prim][sec][i] = internal_slot;
	}
	if ( sec )
	{
		msx.m_slot_expanded[prim] = true;
	}
}

void msx_state::msx_memory_init()
{
	int count_populated_pages = 0;

	// Populate all unpopulated slots with the dummy interface
	for ( int prim = 0; prim < 4; prim++ )
	{
		for ( int sec = 0; sec < 4; sec++ )
		{
			for ( int page = 0; page < 4; page++ )
			{
				if ( m_all_slots[prim][sec][page] == NULL )
				{
					m_all_slots[prim][sec][page] = &m_empty_slot;
				}
				else
				{
					count_populated_pages++;
				}
			}
		}
	}

	if ( count_populated_pages == 0 ) {
		fatalerror("No msx slot layout defined for this system!\n");
	}
}

void msx_state::msx_memory_reset ()
{
	m_primary_slot = 0;

	for (int prim=0; prim<4; prim++)
	{
		m_secondary_slot[prim] = 0;
	}
}

void msx_state::msx_memory_map_page (UINT8 page)
{
	int slot_primary = (m_primary_slot >> (page * 2)) & 3;
	int slot_secondary = (m_secondary_slot[slot_primary] >> (page * 2)) & 3;

	m_current_page[page] = m_all_slots[slot_primary][slot_secondary][page];
}

void msx_state::msx_memory_map_all ()
{
	for (UINT8 i=0; i<4; i++)
		msx_memory_map_page (i);
}

READ8_MEMBER( msx_state::msx_mem_read )
{
	return m_current_page[offset >> 14]->read(space, offset);
}

WRITE8_MEMBER( msx_state::msx_mem_write )
{
	m_current_page[offset >> 14]->write(space, offset, data);
}

WRITE8_MEMBER( msx_state::msx_sec_slot_w )
{
	int slot = m_primary_slot >> 6;
	if (m_slot_expanded[slot])
	{
		if (VERBOSE)
			logerror ("write to secondary slot %d select: %02x\n", slot, data);

		m_secondary_slot[slot] = data;
		msx_memory_map_all ();
	}
	else
		m_current_page[3]->write(space, 0xffff, data);
}

READ8_MEMBER( msx_state::msx_sec_slot_r )
{
	int slot = m_primary_slot >> 6;

	if (m_slot_expanded[slot])
	{
		return ~m_secondary_slot[slot];
	}
	else
	{
		return m_current_page[3]->read(space, 0xffff);
	}
}

READ8_MEMBER( msx_state::msx_kanji_r )
{
	UINT8 result = 0xff;

	if (offset && m_region_kanji)
	{
		int latch = m_kanji_latch;
		result = m_region_kanji->u8(latch++);

		m_kanji_latch &= ~0x1f;
		m_kanji_latch |= latch & 0x1f;
	}
	return result;
}

WRITE8_MEMBER( msx_state::msx_kanji_w )
{
	if (offset)
		m_kanji_latch = (m_kanji_latch & 0x007E0) | ((data & 0x3f) << 11);
	else
		m_kanji_latch = (m_kanji_latch & 0x1f800) | ((data & 0x3f) << 5);
}

READ8_MEMBER( msx_state::msx_switched_r )
{
	// Read from selected switched device
	return this->space().read_byte( (m_current_switched_device << 8) | offset );
}

WRITE8_MEMBER( msx_state::msx_switched_w )
{
	if (offset == 0)
	{
		// Select switched device
		m_current_switched_device = data;
	}
	else
	{
		// Write to selected switched device
		this->space().write_byte( (m_current_switched_device << 8) | offset, data );
	}
}
