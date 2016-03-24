// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64H156 Gate Array emulation

**********************************************************************/

/*

    TODO:

    http://personalpages.tds.net/~rcarlsen/cbm/1541/1541%20EARLY/1540-2.GIF

    - write protect
    - separate read/write methods
    - cycle exact VIA
    - get these running and we're golden
        - Bounty Bob Strikes Back (aligned halftracks)
        - Quiwi (speed change within track)
        - Defender of the Crown (V-MAX! v2, density checks)
        - Test Drive / Cabal (HLS, sub-cycle jitter)
        - Galaxian (?, needs 100% accurate VIA)

*/

#include "64h156.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define CYCLES_UNTIL_ANALOG_DESYNC      288 // 18 us



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64H156 = &device_creator<c64h156_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64h156_device - constructor
//-------------------------------------------------

c64h156_device::c64h156_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64H156, "64H156", tag, owner, clock, "c64h156", __FILE__),
	m_write_atn(*this),
	m_write_sync(*this),
	m_write_byte(*this),
	m_floppy(nullptr),
	m_mtr(1),
	m_accl(0),
	m_stp(0),
	m_ds(0),
	m_soe(0),
	m_oe(1),
	m_ted(0),
	m_yb(0),
	m_atni(0),
	m_atna(0),
	m_period(attotime::from_hz(clock))
{
	memset(&cur_live, 0x00, sizeof(cur_live));
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
	cur_live.write_start_time = attotime::never;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64h156_device::device_start()
{
	// resolve callbacks
	m_write_atn.resolve_safe();
	m_write_sync.resolve_safe();
	m_write_byte.resolve_safe();

	// allocate timer
	t_gen = timer_alloc(0);

	// register for state saving
	save_item(NAME(m_mtr));
	save_item(NAME(m_accl));
	save_item(NAME(m_stp));
	save_item(NAME(m_ds));
	save_item(NAME(m_soe));
	save_item(NAME(m_oe));
	save_item(NAME(m_ted));
	save_item(NAME(m_yb));
	save_item(NAME(m_atni));
	save_item(NAME(m_atna));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64h156_device::device_reset()
{
	live_abort();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void c64h156_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	live_sync();
	live_run();
}

void c64h156_device::live_start()
{
	cur_live.tm = machine().time();
	cur_live.state = RUNNING;
	cur_live.next_state = -1;

	cur_live.shift_reg = 0;
	cur_live.shift_reg_write = 0;
	cur_live.cycle_counter = 0;
	cur_live.cell_counter = 0;
	cur_live.bit_counter = 0;
	cur_live.ds = m_ds;
	cur_live.oe = m_oe;
	cur_live.soe = m_soe;
	cur_live.accl = m_accl;
	cur_live.zero_counter = 0;
	cur_live.cycles_until_random_flux = (rand() % 31) + 289;

	checkpoint_live = cur_live;

	live_run();
}

void c64h156_device::checkpoint()
{
	get_next_edge(machine().time());
	checkpoint_live = cur_live;
}

void c64h156_device::rollback()
{
	cur_live = checkpoint_live;
	get_next_edge(cur_live.tm);
}

void c64h156_device::start_writing(const attotime &tm)
{
	cur_live.write_start_time = tm;
	cur_live.write_position = 0;
}

void c64h156_device::stop_writing(const attotime &tm)
{
	commit(tm);
	cur_live.write_start_time = attotime::never;
}

bool c64h156_device::write_next_bit(bool bit, const attotime &limit)
{
	if(cur_live.write_start_time.is_never()) {
		cur_live.write_start_time = cur_live.tm;
		cur_live.write_position = 0;
	}

	attotime etime = cur_live.tm + m_period;
	if(etime > limit)
		return true;

	if(bit && cur_live.write_position < ARRAY_LENGTH(cur_live.write_buffer))
		cur_live.write_buffer[cur_live.write_position++] = cur_live.tm - m_period;

	if (LOG) logerror("%s write bit %u (%u)\n", cur_live.tm.as_string(), cur_live.bit_counter, bit);

	return false;
}

void c64h156_device::commit(const attotime &tm)
{
	if(cur_live.write_start_time.is_never() || tm == cur_live.write_start_time || !cur_live.write_position)
		return;

	if (LOG) logerror("%s committing %u transitions since %s\n", tm.as_string(), cur_live.write_position, cur_live.write_start_time.as_string());

	m_floppy->write_flux(cur_live.write_start_time, tm, cur_live.write_position, cur_live.write_buffer);

	cur_live.write_start_time = tm;
	cur_live.write_position = 0;
}

void c64h156_device::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		t_gen->adjust(cur_live.tm - machine().time());
	else
		live_sync();
}

void c64h156_device::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			rollback();
			live_run(machine().time());
			commit(cur_live.tm);
		} else {
			commit(cur_live.tm);
			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE) {
				stop_writing(cur_live.tm);
				cur_live.tm = attotime::never;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void c64h156_device::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time()) {
		rollback();
		live_run(machine().time());
	}

	stop_writing(cur_live.tm);

	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
	cur_live.write_position = 0;
	cur_live.write_start_time = attotime::never;

	cur_live.sync = 1;
	cur_live.byte = 1;
}

void c64h156_device::live_run(const attotime &limit)
{
	if(cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	for(;;) {
		switch(cur_live.state) {
		case RUNNING: {
			bool syncpoint = false;

			if (cur_live.tm > limit)
				return;

			int bit = get_next_bit(cur_live.tm, limit);
			if(bit < 0)
				return;

			int cell_counter = cur_live.cell_counter;

			if (bit) {
				cur_live.cycle_counter = cur_live.ds;
				cur_live.cell_counter = 0;
			} else {
				cur_live.cycle_counter++;
			}

			if (cur_live.cycle_counter == 16) {
				cur_live.cycle_counter = cur_live.ds;

				cur_live.cell_counter++;
				cur_live.cell_counter &= 0xf;
			}

			if (!BIT(cell_counter, 1) && BIT(cur_live.cell_counter, 1)) {
				// read bit
				cur_live.shift_reg <<= 1;
				cur_live.shift_reg |= !(BIT(cur_live.cell_counter, 3) || BIT(cur_live.cell_counter, 2));
				cur_live.shift_reg &= 0x3ff;

				if (LOG) logerror("%s read bit %u (%u) >> %03x, oe=%u soe=%u sync=%u byte=%u\n", cur_live.tm.as_string(), cur_live.bit_counter,
					!(BIT(cur_live.cell_counter, 3) || BIT(cur_live.cell_counter, 2)), cur_live.shift_reg, cur_live.oe, cur_live.soe, cur_live.sync, cur_live.byte);

				syncpoint = true;
			}

			if (BIT(cell_counter, 1) && !BIT(cur_live.cell_counter, 1) && !cur_live.oe) { // TODO WPS
				write_next_bit(BIT(cur_live.shift_reg_write, 7), limit);
			}

			int sync = !((cur_live.shift_reg == 0x3ff) && cur_live.oe);

			if (!sync) {
				cur_live.bit_counter = 8;
			} else if (!BIT(cell_counter, 1) && BIT(cur_live.cell_counter, 1) && cur_live.sync) {
				cur_live.bit_counter++;
				cur_live.bit_counter &= 0xf;
			}

			int byte = !(((cur_live.bit_counter & 7) == 7) && cur_live.soe && !(cur_live.cell_counter & 2));
			int load = !(((cur_live.bit_counter & 7) == 7) && ((cur_live.cell_counter & 3) == 3));

			if (!load) {
				if (cur_live.oe) {
					cur_live.shift_reg_write = cur_live.shift_reg;
					if (LOG) logerror("%s load write shift register from read shift register %02x\n",cur_live.tm.as_string(),cur_live.shift_reg_write);
				} else {
					cur_live.shift_reg_write = cur_live.yb;
					if (LOG) logerror("%s load write shift register from YB %02x\n",cur_live.tm.as_string(),cur_live.shift_reg_write);
				}
			} else if (!BIT(cell_counter, 1) && BIT(cur_live.cell_counter, 1)) {
				cur_live.shift_reg_write <<= 1;
				cur_live.shift_reg_write &= 0xff;
				if (LOG) logerror("%s shift write register << %02x\n", cur_live.tm.as_string(), cur_live.shift_reg_write);
			}

			// update signals
			if (byte != cur_live.byte) {
				if (!byte || !cur_live.accl) {
					if (LOG) logerror("%s BYTE %02x\n", cur_live.tm.as_string(), cur_live.shift_reg & 0xff);
					cur_live.byte = byte;
					syncpoint = true;
				}
				if (!byte) {
					cur_live.accl_yb = cur_live.shift_reg & 0xff;
				}
			}

			if (sync != cur_live.sync) {
				if (LOG) logerror("%s SYNC %u\n", cur_live.tm.as_string(),sync);
				cur_live.sync = sync;
				syncpoint = true;
			}

			if (syncpoint) {
				commit(cur_live.tm);

				cur_live.tm += m_period;
				live_delay(RUNNING_SYNCPOINT);
				return;
			}

			cur_live.tm += m_period;
			break;
		}

		case RUNNING_SYNCPOINT: {
			m_write_sync(cur_live.sync);
			m_write_byte(cur_live.byte);

			cur_live.state = RUNNING;
			checkpoint();
			break;
		}
		}
	}
}

void c64h156_device::get_next_edge(const attotime &when)
{
	cur_live.edge = m_floppy->get_next_transition(when);
}

int c64h156_device::get_next_bit(attotime &tm, const attotime &limit)
{
	int bit = 0;
	if (!cur_live.edge.is_never())
	{
		attotime next = tm + m_period;
		if (cur_live.edge < next)
		{
			bit = 1;

			cur_live.zero_counter = 0;
			cur_live.cycles_until_random_flux = (rand() % 31) + 289;

			get_next_edge(next);
		}
	}

	if (cur_live.zero_counter >= cur_live.cycles_until_random_flux) {
		cur_live.zero_counter = 0;
		cur_live.cycles_until_random_flux = (rand() % 367) + 33;

		bit = 1;
	}

	return bit && cur_live.oe;
}


//-------------------------------------------------
//  yb_r -
//-------------------------------------------------

READ8_MEMBER( c64h156_device::yb_r )
{
	if (checkpoint_live.accl) {
		return checkpoint_live.accl_yb;
	} else {
		return checkpoint_live.shift_reg;
	}
}


//-------------------------------------------------
//  yb_w -
//-------------------------------------------------

WRITE8_MEMBER( c64h156_device::yb_w )
{
	if (m_yb != data)
	{
		live_sync();
		m_yb = cur_live.yb = data;
		checkpoint();
		if (LOG) logerror("%s YB %02x\n", machine().time().as_string(), data);
		live_run();
	}

}


//-------------------------------------------------
//  test_w - test write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::test_w )
{
}


//-------------------------------------------------
//  accl_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::accl_w )
{
	if (m_accl != state)
	{
		live_sync();
		m_accl = cur_live.accl = state;
		checkpoint();
		if (LOG) logerror("%s ACCL %u\n", machine().time().as_string(), state);
		live_run();
	}
}


//-------------------------------------------------
//  ted_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::ted_w )
{
	if (m_ted != state)
	{
		live_sync();
		if (m_ted && !state && cur_live.accl && !cur_live.byte) {
			cur_live.byte = 1;
			m_write_byte(cur_live.byte);
		}
		m_ted = state;
		checkpoint();
		if (LOG) logerror("%s TED %u\n", machine().time().as_string(), state);
		live_run();
	}
}


//-------------------------------------------------
//  mtr_w - motor write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::mtr_w )
{
	if (m_mtr != state)
	{
		live_sync();
		m_mtr = state;
		if (LOG) logerror("%s MTR %u\n", machine().time().as_string(), state);
		m_floppy->mon_w(!state);
		checkpoint();

		if (m_mtr) {
			if(cur_live.state == IDLE) {
				live_start();
			}
		} else {
			live_abort();
		}

		live_run();
	}
}


//-------------------------------------------------
//  oe_w - output enable write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::oe_w )
{
	if (m_oe != state)
	{
		live_sync();
		m_oe = cur_live.oe = state;
		if (m_oe) {
			stop_writing(machine().time());
		} else {
			start_writing(machine().time());
		}
		checkpoint();
		if (LOG) logerror("%s OE %u\n", machine().time().as_string(), state);
		live_run();
	}
}


//-------------------------------------------------
//  soe_w - SO enable write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::soe_w )
{
	if (m_soe != state)
	{
		live_sync();
		m_soe = cur_live.soe = state;
		checkpoint();
		if (LOG) logerror("%s SOE %u\n", machine().time().as_string(), state);
		live_run();
	}
}


//-------------------------------------------------
//  atni_w - serial attention input write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::atni_w )
{
	if (LOG) logerror("ATNI %u\n", state);

	m_atni = state;

	m_write_atn(m_atni ^ m_atna);
}


//-------------------------------------------------
//  atna_w - serial attention acknowledge write
//-------------------------------------------------

WRITE_LINE_MEMBER( c64h156_device::atna_w )
{
	if (LOG) logerror("ATNA %u\n", state);

	m_atna = state;

	m_write_atn(m_atni ^ m_atna);
}


//-------------------------------------------------
//  set_floppy -
//-------------------------------------------------

void c64h156_device::set_floppy(floppy_image_device *floppy)
{
	m_floppy = floppy;
}


//-------------------------------------------------
//  stp_w -
//-------------------------------------------------

void c64h156_device::stp_w(int stp)
{
	if (m_stp != stp)
	{
		live_sync();

		if (m_mtr)
		{
			int tracks = 0;

			switch (m_stp)
			{
			case 0: if (stp == 1) tracks++; else if (stp == 3) tracks--; break;
			case 1: if (stp == 2) tracks++; else if (stp == 0) tracks--; break;
			case 2: if (stp == 3) tracks++; else if (stp == 1) tracks--; break;
			case 3: if (stp == 0) tracks++; else if (stp == 2) tracks--; break;
			}

			if (tracks == -1)
			{
				m_floppy->dir_w(1);
				m_floppy->stp_w(1);
				m_floppy->stp_w(0);
			}
			else if (tracks == 1)
			{
				m_floppy->dir_w(0);
				m_floppy->stp_w(1);
				m_floppy->stp_w(0);
			}

			m_stp = stp;
		}

		checkpoint();
		live_run();
	}
}


//-------------------------------------------------
//  ds_w - density select
//-------------------------------------------------

void c64h156_device::ds_w(int ds)
{
	if (m_ds != ds)
	{
		live_sync();
		m_ds = cur_live.ds = ds;
		checkpoint();
		if (LOG) logerror("%s DS %u\n", machine().time().as_string(), ds);
		live_run();
	}
}
