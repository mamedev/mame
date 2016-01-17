// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 2040 floppy disk controller emulation

**********************************************************************/

/*

    TODO:

    - write protect
    - separate read/write methods

*/

#include "c2040fdc.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define GCR_DECODE(_e, _i) \
	((BIT(_e, 6) << 7) | (BIT(_i, 7) << 6) | (_e & 0x33) | (BIT(_e, 2) << 3) | (_i & 0x04))

#define GCR_ENCODE(_e, _i) \
	((_e & 0xc0) << 2 | (_i & 0x80) | (_e & 0x3c) << 1 | (_i & 0x04) | (_e & 0x03))



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C2040_FDC = &device_creator<c2040_fdc_t>;


//-------------------------------------------------
//  ROM( c2040_fdc )
//-------------------------------------------------

ROM_START( c2040_fdc )
	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467.uk6", 0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c2040_fdc_t::device_rom_region() const
{
	return ROM_NAME( c2040_fdc );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c2040_fdc_t - constructor
//-------------------------------------------------

c2040_fdc_t::c2040_fdc_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C2040_FDC, "C2040 FDC", tag, owner, clock, "c2040fdc", __FILE__),
	m_write_sync(*this),
	m_write_ready(*this),
	m_write_error(*this),
	m_gcr_rom(*this, "gcr"),
	m_floppy0(nullptr),
	m_floppy1(nullptr),
	m_mtr0(1),
	m_mtr1(1),
	m_stp0(0),
	m_stp1(0),
	m_ds(0),
	m_ds0(0),
	m_ds1(0),
	m_drv_sel(0),
	m_mode_sel(0),
	m_rw_sel(0), m_odd_hd(0), m_pi(0),
	m_period(attotime::from_hz(clock)), t_gen(nullptr)
{
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
	cur_live.write_position = 0;
	cur_live.write_start_time = attotime::never;
	cur_live.drv_sel = m_drv_sel;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c2040_fdc_t::device_start()
{
	// resolve callbacks
	m_write_sync.resolve_safe();
	m_write_ready.resolve_safe();
	m_write_error.resolve_safe();

	// allocate timer
	t_gen = timer_alloc(0);

	// register for state saving
	save_item(NAME(m_mtr0));
	save_item(NAME(m_mtr1));
	save_item(NAME(m_stp0));
	save_item(NAME(m_stp1));
	save_item(NAME(m_ds));
	save_item(NAME(m_ds0));
	save_item(NAME(m_ds1));
	save_item(NAME(m_drv_sel));
	save_item(NAME(m_mode_sel));
	save_item(NAME(m_rw_sel));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c2040_fdc_t::device_reset()
{
	live_abort();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void c2040_fdc_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	live_sync();
	live_run();
}

floppy_image_device* c2040_fdc_t::get_floppy()
{
	return cur_live.drv_sel ? m_floppy1 : m_floppy0;
}

void c2040_fdc_t::live_start()
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
	cur_live.drv_sel = m_drv_sel;
	cur_live.mode_sel = m_mode_sel;
	cur_live.rw_sel = m_rw_sel;
	cur_live.pi = m_pi;

	checkpoint_live = cur_live;

	live_run();
}

void c2040_fdc_t::checkpoint()
{
	get_next_edge(machine().time());
	checkpoint_live = cur_live;
}

void c2040_fdc_t::rollback()
{
	cur_live = checkpoint_live;
	get_next_edge(cur_live.tm);
}

void c2040_fdc_t::start_writing(const attotime &tm)
{
	cur_live.write_start_time = tm;
	cur_live.write_position = 0;
}

void c2040_fdc_t::stop_writing(const attotime &tm)
{
	commit(tm);
	cur_live.write_start_time = attotime::never;
}

bool c2040_fdc_t::write_next_bit(bool bit, const attotime &limit)
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

void c2040_fdc_t::commit(const attotime &tm)
{
	if(cur_live.write_start_time.is_never() || tm == cur_live.write_start_time || !cur_live.write_position)
		return;

	if (LOG) logerror("%s committing %u transitions since %s\n", tm.as_string(), cur_live.write_position, cur_live.write_start_time.as_string());

	if(get_floppy())
		get_floppy()->write_flux(cur_live.write_start_time, tm, cur_live.write_position, cur_live.write_buffer);

	cur_live.write_start_time = tm;
	cur_live.write_position = 0;
}

void c2040_fdc_t::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		t_gen->adjust(cur_live.tm - machine().time());
	else
		live_sync();
}

void c2040_fdc_t::live_sync()
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

void c2040_fdc_t::live_abort()
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

	cur_live.ready = 1;
	cur_live.sync = 1;
	cur_live.error = 1;
}

void c2040_fdc_t::live_run(const attotime &limit)
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

				if (LOG) logerror("%s read bit %u (%u) >> %03x, rw=%u mode=%u\n", cur_live.tm.as_string(), cur_live.bit_counter,
					!(BIT(cur_live.cell_counter, 3) || BIT(cur_live.cell_counter, 2)), cur_live.shift_reg, cur_live.rw_sel, cur_live.mode_sel);

				// write bit
				if (!cur_live.rw_sel) { // TODO WPS
					write_next_bit(BIT(cur_live.shift_reg_write, 9), limit);
				}

				syncpoint = true;
			}

			int sync = !((cur_live.shift_reg == 0x3ff) && cur_live.rw_sel);

			if (!sync) {
				cur_live.bit_counter = 0;
			} else if (!BIT(cell_counter, 1) && BIT(cur_live.cell_counter, 1) && cur_live.sync) {
				cur_live.bit_counter++;
				if (cur_live.bit_counter == 10) {
					cur_live.bit_counter = 0;
				}
			}

			// update GCR
			if (cur_live.rw_sel) {
				cur_live.i = (cur_live.rw_sel << 10) | cur_live.shift_reg;
			} else {
				cur_live.i = (cur_live.rw_sel << 10) | ((cur_live.pi & 0xf0) << 1) | (cur_live.mode_sel << 4) | (cur_live.pi & 0x0f);
			}

			cur_live.e = m_gcr_rom->base()[cur_live.i];

			int ready = !(BIT(cell_counter, 1) && !BIT(cur_live.cell_counter, 1) && (cur_live.bit_counter == 9));

			if (!ready) {
				// load write shift register
				cur_live.shift_reg_write = GCR_ENCODE(cur_live.e, cur_live.i);

				if (LOG) logerror("%s load write shift register %03x\n",cur_live.tm.as_string(),cur_live.shift_reg_write);
			} else if (BIT(cell_counter, 1) && !BIT(cur_live.cell_counter, 1)) {
				// clock write shift register
				cur_live.shift_reg_write <<= 1;
				cur_live.shift_reg_write &= 0x3ff;

				if (LOG) logerror("%s write shift << %03x\n",cur_live.tm.as_string(),cur_live.shift_reg_write);
			}

			int error = !(BIT(cur_live.e, 3) || ready);

			if (ready != cur_live.ready) {
				if (LOG) logerror("%s READY %u\n", cur_live.tm.as_string(),ready);
				cur_live.ready = ready;
				syncpoint = true;
			}

			if (sync != cur_live.sync) {
				if (LOG) logerror("%s SYNC %u\n", cur_live.tm.as_string(),sync);
				cur_live.sync = sync;
				syncpoint = true;
			}

			if (error != cur_live.error) {
				cur_live.error = error;
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
			m_write_ready(cur_live.ready);
			m_write_sync(cur_live.sync);
			m_write_error(cur_live.error);

			cur_live.state = RUNNING;
			checkpoint();
			break;
		}
		}
	}
}

void c2040_fdc_t::get_next_edge(const attotime &when)
{
	floppy_image_device *floppy = get_floppy();

	cur_live.edge = floppy ? floppy->get_next_transition(when) : attotime::never;
}

int c2040_fdc_t::get_next_bit(attotime &tm, const attotime &limit)
{
	attotime next = tm + m_period;

	int bit = (cur_live.edge.is_never() || cur_live.edge >= next) ? 0 : 1;

	if (bit) {
		get_next_edge(next);
	}

	return bit && cur_live.rw_sel;
}

READ8_MEMBER( c2040_fdc_t::read )
{
	UINT8 e = checkpoint_live.e;
	offs_t i = checkpoint_live.i;

	UINT8 data = GCR_DECODE(e, i);

	if (LOG) logerror("%s %s VIA reads data %02x (%03x)\n", machine().time().as_string(), machine().describe_context(), data, checkpoint_live.shift_reg);

	return data;
}

WRITE8_MEMBER( c2040_fdc_t::write )
{
	if (m_pi != data)
	{
		live_sync();
		m_pi = cur_live.pi = data;
		checkpoint();
		if (LOG) logerror("%s %s PI %02x\n", machine().time().as_string(), machine().describe_context(), data);
		live_run();
	}
}

WRITE_LINE_MEMBER( c2040_fdc_t::ds0_w )
{
	m_ds0 = state;
}

WRITE_LINE_MEMBER( c2040_fdc_t::ds1_w )
{
	m_ds1 = state;

	ds_w(m_ds1 << 1 | m_ds0);
}

WRITE_LINE_MEMBER( c2040_fdc_t::drv_sel_w )
{
	if (m_drv_sel != state)
	{
		live_sync();
		m_drv_sel = cur_live.drv_sel = state;
		checkpoint();
		if (LOG) logerror("%s %s DRV SEL %u\n", machine().time().as_string(), machine().describe_context(), state);
		live_run();
	}
}

WRITE_LINE_MEMBER( c2040_fdc_t::mode_sel_w )
{
	if (m_mode_sel != state)
	{
		live_sync();
		m_mode_sel = cur_live.mode_sel = state;
		checkpoint();
		if (LOG) logerror("%s %s MODE SEL %u\n", machine().time().as_string(), machine().describe_context(), state);
		live_run();
	}
}

WRITE_LINE_MEMBER( c2040_fdc_t::rw_sel_w )
{
	if (m_rw_sel != state)
	{
		live_sync();
		m_rw_sel = cur_live.rw_sel = state;
		checkpoint();
		if (LOG) logerror("%s %s RW SEL %u\n", machine().time().as_string(), machine().describe_context(), state);
		if (m_rw_sel) {
			stop_writing(machine().time());
		} else {
			start_writing(machine().time());
		}
		live_run();
	}
}

WRITE_LINE_MEMBER( c2040_fdc_t::mtr0_w )
{
	if (m_mtr0 != state)
	{
		live_sync();
		m_mtr0 = state;
		if (LOG) logerror("%s %s MTR0 %u\n", machine().time().as_string(), machine().describe_context(), state);
		m_floppy0->mon_w(state);
		checkpoint();

		if (!m_mtr0 || !m_mtr1) {
			if(cur_live.state == IDLE) {
				live_start();
			}
		} else {
			live_abort();
		}

		live_run();
	}
}

WRITE_LINE_MEMBER( c2040_fdc_t::mtr1_w )
{
	if (m_mtr1 != state)
	{
		live_sync();
		m_mtr1 = state;
		if (LOG) logerror("%s %s MTR1 %u\n", machine().time().as_string(), machine().describe_context(), state);
		if (m_floppy1) m_floppy1->mon_w(state);
		checkpoint();

		if (!m_mtr0 || !m_mtr1) {
			if(cur_live.state == IDLE) {
				live_start();
			}
		} else {
			live_abort();
		}

		live_run();
	}
}

void c2040_fdc_t::stp_w(floppy_image_device *floppy, int mtr, int &old_stp, int stp)
{
	if (mtr) return;

	int tracks = 0;

	switch (old_stp)
	{
	case 0: if (stp == 1) tracks++; else if (stp == 3) tracks--; break;
	case 1: if (stp == 2) tracks++; else if (stp == 0) tracks--; break;
	case 2: if (stp == 3) tracks++; else if (stp == 1) tracks--; break;
	case 3: if (stp == 0) tracks++; else if (stp == 2) tracks--; break;
	}

	if (tracks == -1)
	{
		floppy->dir_w(1);
		floppy->stp_w(1);
		floppy->stp_w(0);
	}
	else if (tracks == 1)
	{
		floppy->dir_w(0);
		floppy->stp_w(1);
		floppy->stp_w(0);
	}

	old_stp = stp;
}

void c2040_fdc_t::stp0_w(int stp)
{
	if (m_stp0 != stp)
	{
		live_sync();
		this->stp_w(m_floppy0, m_mtr0, m_stp0, stp);
		checkpoint();
		live_run();
	}
}

void c2040_fdc_t::stp1_w(int stp)
{
	if (m_stp1 != stp)
	{
		live_sync();
		if (m_floppy1) this->stp_w(m_floppy1, m_mtr1, m_stp1, stp);
		checkpoint();
		live_run();
	}
}

void c2040_fdc_t::ds_w(int ds)
{
	if (m_ds != ds)
	{
		live_sync();
		m_ds = cur_live.ds = ds;
		if (LOG) logerror("%s %s DS %u\n", machine().time().as_string(), machine().describe_context(), ds);
		checkpoint();
		live_run();
	}
}

void c2040_fdc_t::set_floppy(floppy_image_device *floppy0, floppy_image_device *floppy1)
{
	m_floppy0 = floppy0;
	m_floppy1 = floppy1;
}
