// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 8050 floppy disk controller emulation

**********************************************************************/

/*

    TODO:

    - write protect
    - 75,format speed error,01,00,0

*/

#include "c8050fdc.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0
#define LOG_MORE 0
#define LOG_BITS 0

#define GCR_DECODE(_e, _i) \
	((BIT(_e, 6) << 7) | (BIT(_i, 7) << 6) | (_e & 0x33) | (BIT(_e, 2) << 3) | (_i & 0x04))

#define GCR_ENCODE(_e, _i) \
	((_e & 0xc0) << 2 | (_i & 0x80) | (_e & 0x3c) << 1 | (_i & 0x04) | (_e & 0x03))



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C8050_FDC = &device_creator<c8050_fdc_t>;


//-------------------------------------------------
//  ROM( c8050_fdc )
//-------------------------------------------------

ROM_START( c8050_fdc )
	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467.uk6", 0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c8050_fdc_t::device_rom_region() const
{
	return ROM_NAME( c8050_fdc );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c8050_fdc_t - constructor
//-------------------------------------------------

c8050_fdc_t::c8050_fdc_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C8050_FDC, "Commodore 8050 FDC", tag, owner, clock, "c8050fdc", __FILE__),
	m_write_sync(*this),
	m_write_ready(*this),
	m_write_brdy(*this),
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
	m_rw_sel(1), m_odd_hd(0), m_pi(0), t_gen(nullptr)
{
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
	cur_live.drv_sel = m_drv_sel;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c8050_fdc_t::device_start()
{
	// resolve callbacks
	m_write_sync.resolve_safe();
	m_write_ready.resolve_safe();
	m_write_brdy.resolve_safe();
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
	save_item(NAME(m_odd_hd));
	save_item(NAME(m_pi));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c8050_fdc_t::device_reset()
{
	live_abort();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void c8050_fdc_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	live_sync();
	live_run();
}

floppy_image_device* c8050_fdc_t::get_floppy()
{
	return cur_live.drv_sel ? m_floppy1 : m_floppy0;
}

void c8050_fdc_t::stp_w(floppy_image_device *floppy, int mtr, int &old_stp, int stp)
{
	if (mtr) return;

	int tracks = 0;

	switch (old_stp)
	{
	case 0: if (stp == 1) tracks++; else if (stp == 2) tracks--; break;
	case 1: if (stp == 3) tracks++; else if (stp == 0) tracks--; break;
	case 2: if (stp == 0) tracks++; else if (stp == 3) tracks--; break;
	case 3: if (stp == 2) tracks++; else if (stp == 1) tracks--; break;
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

void c8050_fdc_t::stp0_w(int stp)
{
	if (m_stp0 != stp)
	{
		live_sync();
		stp_w(m_floppy0, m_mtr0, m_stp0, stp);
		checkpoint();
		live_run();
	}
}

void c8050_fdc_t::stp1_w(int stp)
{
	if (m_stp1 != stp)
	{
		live_sync();
		if (m_floppy1) stp_w(m_floppy1, m_mtr1, m_stp1, stp);
		checkpoint();
		live_run();
	}
}

void c8050_fdc_t::ds_w(int ds)
{
	if (m_ds != ds)
	{
		live_sync();
		m_ds = cur_live.ds = ds;
		pll_reset(cur_live.tm);
		if (LOG) logerror("%s %s DS %u\n", machine().time().as_string(), machine().describe_context(), ds);
		checkpoint();
		live_run();
	}
}

void c8050_fdc_t::set_floppy(floppy_connector *floppy0, floppy_connector *floppy1)
{
	m_floppy0 = floppy0->get_device();

	if (floppy1) {
		m_floppy1 = floppy1->get_device();
	}
}

void c8050_fdc_t::live_start()
{
	cur_live.tm = machine().time();
	cur_live.state = RUNNING;
	cur_live.next_state = -1;

	cur_live.shift_reg = 0;
	cur_live.shift_reg_write = 0;
	cur_live.bit_counter = 0;
	cur_live.ds = m_ds;
	cur_live.drv_sel = m_drv_sel;
	cur_live.mode_sel = m_mode_sel;
	cur_live.rw_sel = m_rw_sel;
	cur_live.pi = m_pi;

	pll_reset(cur_live.tm);
	checkpoint_live = cur_live;
	pll_save_checkpoint();

	live_run();
}

void c8050_fdc_t::pll_reset(const attotime &when)
{
	cur_pll.reset(when);
	cur_pll.set_clock(attotime::from_hz(clock() / (16 - m_ds)));
}

void c8050_fdc_t::pll_start_writing(const attotime &tm)
{
	cur_pll.start_writing(tm);
	pll_reset(cur_live.tm);
}

void c8050_fdc_t::pll_commit(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.commit(floppy, tm);
}

void c8050_fdc_t::pll_stop_writing(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.stop_writing(floppy, tm);
	pll_reset(cur_live.tm);
}

void c8050_fdc_t::pll_save_checkpoint()
{
	checkpoint_pll = cur_pll;
}

void c8050_fdc_t::pll_retrieve_checkpoint()
{
	cur_pll = checkpoint_pll;
}

int c8050_fdc_t::pll_get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.get_next_bit(tm, floppy, limit);
}

bool c8050_fdc_t::pll_write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.write_next_bit(bit, tm, floppy, limit);
}

void c8050_fdc_t::checkpoint()
{
	pll_commit(get_floppy(), cur_live.tm);
	checkpoint_live = cur_live;
	pll_save_checkpoint();
}

void c8050_fdc_t::rollback()
{
	cur_live = checkpoint_live;
	pll_retrieve_checkpoint();
}

void c8050_fdc_t::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		t_gen->adjust(cur_live.tm - machine().time());
	else
		live_sync();
}

void c8050_fdc_t::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			rollback();
			live_run(machine().time());
			pll_commit(get_floppy(), cur_live.tm);
		} else {
			pll_commit(get_floppy(), cur_live.tm);
			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE) {
				pll_stop_writing(get_floppy(), cur_live.tm);
				cur_live.tm = attotime::never;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void c8050_fdc_t::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time()) {
		rollback();
		live_run(machine().time());
	}

	pll_stop_writing(get_floppy(), cur_live.tm);

	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;

	cur_live.ready = 1;
	cur_live.brdy = 1;
	cur_live.sync = 1;
	cur_live.error = 1;
}

void c8050_fdc_t::live_run(const attotime &limit)
{
	if(cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	for(;;) {
		switch(cur_live.state) {
		case RUNNING: {
			bool syncpoint = false;

			if (cur_live.tm > limit)
				return;

			// read bit
			int bit = 0;
			if (cur_live.rw_sel) {
				bit = pll_get_next_bit(cur_live.tm, get_floppy(), limit);
				if(bit < 0)
					return;
			}

			// write bit
			int write_bit = BIT(cur_live.shift_reg_write, 9);
			if (!cur_live.rw_sel) { // TODO WPS
				/*
				write precompensation

				UA5.A = UM6.Qc
				UA5.B = !(!(!BRDY && UM6.Qa) && !(BRDY && E7))
				UA5.C0 = UA4.Qb = bit clock delayed 333ns
				UA5.C1 = UA4.Qa = bit clock delayed 166ns
				UA5.C2 = UA4.Qc = bit clock delayed 499ns
				UA5.C3 = UA5.Qb = bit clock delayed 333ns

				DATA OUT = !(!BITCLK || !(UA5.Y && !(WRITE_ENABLE && !UM6.Qb)))
				*/
				if (pll_write_next_bit(write_bit, cur_live.tm, get_floppy(), limit))
					return;
			}

			// clock read shift register
			cur_live.shift_reg <<= 1;
			cur_live.shift_reg |= bit;
			cur_live.shift_reg &= 0x3ff;

			// sync
			int sync = !((cur_live.shift_reg == 0x3ff) && cur_live.rw_sel);

			// bit counter
			if (!sync) {
				cur_live.bit_counter = 0;
			} else if (cur_live.sync) {
				cur_live.bit_counter++;
				if (cur_live.bit_counter == 10) {
					cur_live.bit_counter = 0;
				}
			}

			// GCR decoder
			if (cur_live.rw_sel) {
				cur_live.i = (cur_live.rw_sel << 10) | cur_live.shift_reg;
			} else {
				cur_live.i = (cur_live.rw_sel << 10) | ((cur_live.pi & 0xf0) << 1) | (cur_live.mode_sel << 4) | (cur_live.pi & 0x0f);
			}

			cur_live.e = m_gcr_rom->base()[cur_live.i];

			// byte ready
			int ready = !(cur_live.bit_counter == 9); // 74190 _RC, should be triggered on the falling edge of the clock
			int brdy = ready; // 74190 TC

			// GCR error
			int error = !(ready || BIT(cur_live.e, 3));

			if (LOG_BITS) {
				if (cur_live.rw_sel) {
					logerror("%s cyl %u bit %u sync %u bc %u sr %03x i %03x e %02x\n",cur_live.tm.as_string(),get_floppy()->get_cyl(),bit,sync,cur_live.bit_counter,cur_live.shift_reg,cur_live.i,cur_live.e);
				} else {
					logerror("%s cyl %u writing bit %u bc %u sr %03x i %03x e %02x\n",cur_live.tm.as_string(),get_floppy()->get_cyl(),write_bit,cur_live.bit_counter,cur_live.shift_reg_write,cur_live.i,cur_live.e);
				}
			}

			if (!ready) {
				// load write shift register
				cur_live.shift_reg_write = GCR_ENCODE(cur_live.e, cur_live.i);

				if (LOG_BITS) logerror("%s load write shift register %03x\n",cur_live.tm.as_string(),cur_live.shift_reg_write);
			} else {
				// clock write shift register
				cur_live.shift_reg_write <<= 1;
				cur_live.shift_reg_write &= 0x3ff;
			}

			if (ready != cur_live.ready) {
				if (cur_live.rw_sel && !ready)
					if (LOG) logerror("%s READY %u : %02x\n", cur_live.tm.as_string(),ready,GCR_DECODE(cur_live.e, cur_live.i));
				cur_live.ready = ready;
				syncpoint = true;
			}

			if (brdy != cur_live.brdy) {
				if (LOG_MORE) logerror("%s BRDY %u\n", cur_live.tm.as_string(), brdy);
				cur_live.brdy = brdy;
				syncpoint = true;
			}

			if (sync != cur_live.sync) {
				if (LOG) logerror("%s SYNC %u\n", cur_live.tm.as_string(), sync);
				cur_live.sync = sync;
				syncpoint = true;
			}

			if (error != cur_live.error) {
				if (LOG_MORE) logerror("%s ERROR %u\n", cur_live.tm.as_string(), error);
				cur_live.error = error;
				syncpoint = true;
			}

			if (syncpoint) {
				live_delay(RUNNING_SYNCPOINT);
				return;
			}
			break;
		}

		case RUNNING_SYNCPOINT: {
			m_write_ready(cur_live.ready);
			m_write_brdy(cur_live.brdy);
			m_write_sync(cur_live.sync);
			m_write_error(cur_live.error);

			cur_live.state = RUNNING;
			checkpoint();
			break;
		}
		}
	}
}

READ8_MEMBER( c8050_fdc_t::read )
{
	UINT8 e = checkpoint_live.e;
	offs_t i = checkpoint_live.i;

	return GCR_DECODE(e, i);
}

WRITE8_MEMBER( c8050_fdc_t::write )
{
	if (LOG) logerror("%s %s PI %02x\n", machine().time().as_string(), machine().describe_context(), data);

	if (m_pi != data)
	{
		live_sync();
		m_pi = cur_live.pi = data;
		checkpoint();
		live_run();
	}
}

WRITE_LINE_MEMBER( c8050_fdc_t::ds0_w )
{
	m_ds0 = state;
}

WRITE_LINE_MEMBER( c8050_fdc_t::ds1_w )
{
	m_ds1 = state;

	ds_w(m_ds1 << 1 | m_ds0);
}

WRITE_LINE_MEMBER( c8050_fdc_t::drv_sel_w )
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

WRITE_LINE_MEMBER( c8050_fdc_t::mode_sel_w )
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

WRITE_LINE_MEMBER( c8050_fdc_t::rw_sel_w )
{
	if (m_rw_sel != state)
	{
		live_sync();
		m_rw_sel = cur_live.rw_sel = state;
		checkpoint();
		if (LOG) logerror("%s %s RW SEL %u\n", machine().time().as_string(), machine().describe_context(), state);
		if (m_rw_sel) {
			pll_stop_writing(get_floppy(), cur_live.tm);
		} else {
			pll_start_writing(cur_live.tm);
		}
		live_run();
	}
}

WRITE_LINE_MEMBER( c8050_fdc_t::mtr0_w )
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

WRITE_LINE_MEMBER( c8050_fdc_t::mtr1_w )
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

WRITE_LINE_MEMBER( c8050_fdc_t::odd_hd_w )
{
	if (m_odd_hd != state)
	{
		live_sync();
		m_odd_hd = cur_live.odd_hd = state;
		if (LOG) logerror("%s %s ODD HD %u\n", machine().time().as_string(), machine().describe_context(), state);
		m_floppy0->ss_w(!state);
		if (m_floppy1) m_floppy1->ss_w(!state);
		checkpoint();
		live_run();
	}
}

WRITE_LINE_MEMBER( c8050_fdc_t::pull_sync_w )
{
	if (LOG_MORE) logerror("%s %s PULL SYNC %u\n", machine().time().as_string(), machine().describe_context(), state);
}
