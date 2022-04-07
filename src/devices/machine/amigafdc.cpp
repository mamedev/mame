// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/**************************************************************************************************

    Amiga floppy disk controller emulation "Trackdisk"

    Contained inside MOS 8364 Paula device

    TODO:
    - Some games currently writes 2+ dsksync to the buffer (marked as "[FDC] dsksync" in SW list):
      Current workaround:
      1. comment out dma_write in DMA_WAIT_START handling and change the dma_state *only*;
      2. remove all of the non-DMA_WAIT_START phase inside the dsksync sub-section;
      NB: according to documentation syncing doesn't really write anything on the bus,
      so technically this "workaround" is more correct.
      However it unfortunately causes other SW regressions, most notably in Workbench.
    - Other games trashes memory or refuses to boot, in a few instances randomly
      (marked as "[FDC] with adkcon=1100", implies dsksync disabled):
      they often uses the AmigaDOS trackdisk BIOS functions, which may be expecting a
      different timing. May be worth testing this out with the SDK;
    - "[FDC] format" or in general writing to disks doesn't work properly.
      i.e. formatting a disk in any Workbench version will cause a system crash once it completes.
    - Fix ready line read handling;
    - FDC LED output callback;

**************************************************************************************************/


#include "emu.h"
#include "formats/ami_dsk.h"
#include "formats/ipf_dsk.h"
#include "amigafdc.h"

#define LOG_WARN    (1U << 1)   // Show warnings
#define LOG_DMA     (1U << 2)   // Show DMA setups
#define LOG_SYNC    (1U << 3)   // Show sync block setups

#define VERBOSE (LOG_WARN | LOG_DMA | LOG_SYNC)

#include "logmacro.h"

#define LOGWARN(...)     LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGDMA(...)      LOGMASKED(LOG_DMA, __VA_ARGS__)
#define LOGSYNC(...)     LOGMASKED(LOG_SYNC, __VA_ARGS__)

DEFINE_DEVICE_TYPE(AMIGA_FDC, amiga_fdc_device, "amiga_fdc", "Amiga \"Trackdisk\" FDC")

void amiga_fdc_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ADF_FORMAT);
	fr.add(FLOPPY_IPF_FORMAT);
}

amiga_fdc_device::amiga_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AMIGA_FDC, tag, owner, clock)
	, m_write_index(*this)
	, m_read_dma(*this)
	, m_write_dma(*this)
	, m_write_dskblk(*this)
	, m_write_dsksyn(*this)
	, m_leds(*this, "led%u", 1U)
	, m_fdc_led(*this, "fdc_led")
	, floppy(nullptr), t_gen(nullptr), dsklen(0), pre_dsklen(0), dsksync(0), dskbyt(0), adkcon(0), dmacon(0), dskpt(0), dma_value(0), dma_state(0)
{
}

void amiga_fdc_device::device_start()
{
	m_write_index.resolve_safe();
	m_read_dma.resolve_safe(0);
	m_write_dma.resolve_safe();
	m_write_dskblk.resolve_safe();
	m_write_dsksyn.resolve_safe();
	m_leds.resolve();
	m_fdc_led.resolve();

	static char const *const names[] = { "0", "1", "2", "3" };
	for(int i=0; i != 4; i++) {
		floppy_connector *con = subdevice<floppy_connector>(names[i]);
		if(con)
			floppy_devices[i] = con->get_device();
		else
			floppy_devices[i] = nullptr;
	}

	floppy = nullptr;

	t_gen = timer_alloc(0);
}

void amiga_fdc_device::device_reset()
{
	floppy = nullptr;
	dsklen = 0x4000;
	dsksync = 0x4489;
	adkcon = 0;
	dmacon = 0;
	dskpt = 0;
	dskbyt = 0;
	pre_dsklen = 0x4000;
	dma_value = 0;
	dma_state = DMA_IDLE;

	live_abort();
}

void amiga_fdc_device::dma_done()
{
	if(dskbyt & 0x2000) {
		dskbyt &= ~0x2000;
		cur_live.pll.stop_writing(floppy, cur_live.tm);
	}

	dma_state = DMA_IDLE;
	m_write_dskblk(1);
}

void amiga_fdc_device::dma_write(uint16_t value)
{
	m_write_dma(dskpt, value, 0xffff);

	dskpt += 2;
	dsklen--;

	if(dsklen & 0x3fff)
		dma_state = DMA_RUNNING_BYTE_0;
	else
		dma_done();
}

uint16_t amiga_fdc_device::dma_read()
{
	uint16_t res = m_read_dma(dskpt, 0xffff);

	dskpt += 2;
	dsklen--;

	// This loses the last word.  So does the real hardware.
	if(dsklen & 0x3fff)
		dma_state = DMA_RUNNING_BYTE_0;
	else
		dma_done();

	return res;
}

void amiga_fdc_device::live_start()
{
	cur_live.tm = machine().time();
	cur_live.state = RUNNING;
	cur_live.next_state = -1;
	cur_live.shift_reg = 0;
	cur_live.bit_counter = 0;
	cur_live.pll.reset(cur_live.tm);
	cur_live.pll.set_clock(clocks_to_attotime(1));
	checkpoint_live = cur_live;

	live_run();
}

void amiga_fdc_device::checkpoint()
{
	cur_live.pll.commit(floppy, cur_live.tm);
	checkpoint_live = cur_live;
}

void amiga_fdc_device::rollback()
{
	cur_live = checkpoint_live;
}

void amiga_fdc_device::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		t_gen->adjust(cur_live.tm - machine().time());
}

void amiga_fdc_device::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			rollback();
			live_run(machine().time());
			cur_live.pll.commit(floppy, cur_live.tm);

		} else {
			cur_live.pll.commit(floppy, cur_live.tm);

			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE) {
				cur_live.pll.stop_writing(floppy, cur_live.tm);
				cur_live.tm = attotime::never;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void amiga_fdc_device::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time()) {
		rollback();
		live_run(machine().time());
	}

	cur_live.pll.stop_writing(floppy, cur_live.tm);
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}

void amiga_fdc_device::live_run(const attotime &limit)
{
	if(cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	for(;;) {
		switch(cur_live.state) {
		case RUNNING: {
			if(!(dskbyt & 0x2000)) {
				int bit = cur_live.pll.get_next_bit(cur_live.tm, floppy, limit);
				if(bit < 0)
					return;

				cur_live.shift_reg = (cur_live.shift_reg << 1) | bit;
				cur_live.bit_counter++;

				if((adkcon & 0x0200) && !(cur_live.shift_reg & 0x80)) {
					cur_live.bit_counter--;

					// Avoid any risk of livelock
					live_delay(RUNNING_SYNCPOINT);
					return;
				}

				if(cur_live.bit_counter > 8)
				{
					// CHECKME: abreed, ghoulsvf Ghouls'n Goblins and lastnin2 at very least throws this
					// is it a side effect of something else not happening at the right time or the assumption is right?
					cur_live.bit_counter = 0;
					LOGWARN("%s: live_run - cur_live.bit_counter > 8\n", machine().describe_context());
				}

				if(cur_live.bit_counter == 8) {
					live_delay(RUNNING_SYNCPOINT);
					return;
				}
				if(dskbyt & 0x1000) {
					if(cur_live.shift_reg != dsksync) {
						live_delay(RUNNING_SYNCPOINT);
						return;
					}
				} else {
					if(cur_live.shift_reg == dsksync) {
						live_delay(RUNNING_SYNCPOINT);
						return;
					}
				}
			} else {
				int bit = (dma_state == DMA_RUNNING_BYTE_0 ? 15 : 7) - cur_live.bit_counter;
				if(cur_live.pll.write_next_bit((dma_value >> bit) & 1, cur_live.tm, floppy, limit))
					return;
				cur_live.bit_counter++;
				if(cur_live.bit_counter > 8)
				{
					cur_live.bit_counter = 0;
					LOGWARN("%s: live_run - cur_live.bit_counter > 8\n", machine().describe_context());
				}

				if(cur_live.bit_counter == 8) {
					live_delay(RUNNING_SYNCPOINT);
					return;
				}
			}
			break;
		}

		case RUNNING_SYNCPOINT: {
			if(!(dskbyt & 0x2000)) {
				if(cur_live.shift_reg == dsksync) {
					if(adkcon & 0x0400) {
						// FIXME: exact dsksync behaviour, cfr. note at top
						if(dma_state == DMA_WAIT_START) {
							cur_live.bit_counter = 0;

							if(!(dsklen & 0x3fff))
								dma_done();
							else if(dsklen & 0x4000) {
								dskbyt |= 0x2000;
								cur_live.bit_counter = 0;
								dma_value = dma_read();

							} else {
								LOGSYNC("%s: DSKSYNC on %06x %d\n", this->tag(), dskpt, dma_state);
								dma_write(dsksync);
							}

						} else if(dma_state != DMA_IDLE) {
							LOGSYNC("%s: DSKSYNC on %06x %d\n", this->tag(), dskpt, dma_state);
							dma_write(dsksync);
							cur_live.bit_counter = 0;

						} else if(cur_live.bit_counter != 8)
							cur_live.bit_counter = 0;
					}

					dskbyt |= 0x1000;
					m_write_dsksyn(1);
				} else
					dskbyt &= ~0x1000;

				if(cur_live.bit_counter == 8) {
					dskbyt = (dskbyt & 0xff00) | 0x8000 | (cur_live.shift_reg & 0xff);
					cur_live.bit_counter = 0;

					switch(dma_state) {
					case DMA_IDLE:
					case DMA_WAIT_START:
						break;

					case DMA_RUNNING_BYTE_0:
						dma_value = (cur_live.shift_reg & 0xff) << 8;
						dma_state = DMA_RUNNING_BYTE_1;
						break;

					case DMA_RUNNING_BYTE_1:
						dma_value |= cur_live.shift_reg & 0xff;
						dma_write(dma_value);
						break;
					}
				}
			} else {
				if(cur_live.bit_counter != 8)
					fatalerror("amiga_fdc_device::live_run - cur_live.bit_counter != 8\n");
				cur_live.bit_counter = 0;

				switch(dma_state) {
				case DMA_IDLE:
				case DMA_WAIT_START:
					break;

				case DMA_RUNNING_BYTE_0:
					dma_state = DMA_RUNNING_BYTE_1;
					break;

				case DMA_RUNNING_BYTE_1: {
					dma_value = dma_read();
					break;
				}
				}
			}

			cur_live.state = RUNNING;
			checkpoint();
			break;
		}
		}
	}
}

bool amiga_fdc_device::dma_enabled()
{
	return (dsklen & 0x8000) && ((dmacon & 0x0210) == 0x0210);
}

void amiga_fdc_device::dma_check()
{
	bool was_writing = dskbyt & 0x2000;
	dskbyt &= 0x9fff;
	if(dma_enabled()) {
		LOGDMA("%s: DMA start dskpt=%08x dsklen=%04x dir=%s adkcon=%04x dsksync=%04x state=%d\n",
			machine().describe_context(),
			dskpt, dsklen & 0x3fff, BIT(dsklen, 14) ? "RAM->disk" : "disk->RAM", adkcon, dsksync, dma_state
		);

		if(dma_state == IDLE) {
			dma_state = adkcon & 0x0400 ? DMA_WAIT_START : DMA_RUNNING_BYTE_0;
			if(dma_state == DMA_RUNNING_BYTE_0) {
				if(!(dsklen & 0x3fff))
					dma_done();
				else if(dsklen & 0x4000) {
					dskbyt |= 0x2000;
					cur_live.bit_counter = 0;
					dma_value = dma_read();
				}
			}
		} else {
			if(dsklen & 0x4000)
				dskbyt |= 0x2000;
		}
	} else
		dma_state = IDLE;

	if(was_writing && !(dskbyt & 0x2000))
		cur_live.pll.stop_writing(floppy, cur_live.tm);
	if(!was_writing && (dskbyt & 0x2000))
		cur_live.pll.start_writing(cur_live.tm);
}

void amiga_fdc_device::adkcon_set(uint16_t data)
{
	live_sync();
	adkcon = data;
	live_run();
}

uint16_t amiga_fdc_device::adkcon_r(void)
{
	return adkcon;
}

void amiga_fdc_device::dsklen_w(uint16_t data)
{
	live_sync();
	if(!(data & 0x8000) || (data == pre_dsklen)) {
		dsklen = pre_dsklen = data;
		dma_check();

	} else
		pre_dsklen = data;
	live_run();
}

void amiga_fdc_device::dskpth_w(uint16_t data)
{
	live_sync();
	dskpt = (dskpt & 0xffff) | (data << 16);
	live_run();
}

void amiga_fdc_device::dskptl_w(uint16_t data)
{
	live_sync();
	dskpt = (dskpt & 0xffff0000) | data;
	live_run();
}

uint16_t amiga_fdc_device::dskpth_r()
{
	return dskpt >> 16;
}

uint16_t amiga_fdc_device::dskptl_r()
{
	return dskpt;
}

void amiga_fdc_device::dsksync_w(uint16_t data)
{
	live_sync();
	LOGSYNC("%s: DSKSYNC %04x\n", machine().describe_context(), data);
	dsksync = data;
	live_run();
}

void amiga_fdc_device::dmacon_set(uint16_t data)
{
	live_sync();
	// log changes only
	// FIXME: needs better boilerplate code on top level
	if ((data & 0x210) != (dmacon & 0x210))
		LOGDMA("%s: DMACON set DSKEN %d DMAEN %d (%04x)\n", machine().describe_context(), BIT(data, 4), BIT(data, 9), data);
	dmacon = data;
	dma_check();
	live_run();
}

uint16_t amiga_fdc_device::dskbytr_r()
{
	uint16_t res = (dskbyt & ~0x4000);
	// check if DMA is on
	// logica2 diagnostic BIOS floppy test requires this
	bool dmaon = (dma_state != DMA_IDLE) && ((dmacon & 0x0210) == 0x0210);
	res |= dmaon << 14;

	// reset DSKBYT ready on read
	if (!machine().side_effects_disabled())
		dskbyt &= 0x7fff;
	return res;
}

void amiga_fdc_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	live_sync();
	live_run();
}

void amiga_fdc_device::setup_leds()
{
	if(floppy) {
		int drive =
			floppy == floppy_devices[0] ? 0 :
			floppy == floppy_devices[1] ? 1 :
			floppy == floppy_devices[2] ? 2 :
			3;

		m_leds[0] = drive == 0 ? 1 : 0; // update internal drive led
		m_leds[1] = drive == 1 ? 1 : 0;  // update external drive led
	}
}

void amiga_fdc_device::ciaaprb_w(uint8_t data)
{
	floppy_image_device *old_floppy = floppy;

	live_sync();

	// FIXME: several sources claims that multiple drive selects is really possible
	if(!(data & 0x08))
		floppy = floppy_devices[0];
	else if(!(data & 0x10))
		floppy = floppy_devices[1];
	else if(!(data & 0x20))
		floppy = floppy_devices[2];
	else if(!(data & 0x40))
		floppy = floppy_devices[3];
	else
		floppy = nullptr;

	if(old_floppy != floppy) {
		if(old_floppy)
			old_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());
		if(floppy)
			floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&amiga_fdc_device::index_callback, this));
	}

	if(floppy) {
		floppy->ss_w(!(BIT(data, 2)));
		floppy->dir_w(BIT(data, 1));
		floppy->stp_w(BIT(data, 0));
		floppy->mon_w(BIT(data, 7));
		m_fdc_led = BIT(data, 7); // LED directly connected to FDC motor
	}

	if(floppy) {
		if(cur_live.state == IDLE)
			live_start();
	} else
		live_abort();

	setup_leds();
	live_run();
}

uint8_t amiga_fdc_device::ciaapra_r()
{
	uint8_t ret = 0x3c;
	if(floppy) {
		// FIXME: seems to not work well with multiple disk drives
		//if(!floppy->ready_r())
			ret &= ~0x20;
		if(!floppy->trk00_r())
			ret &= ~0x10;
		if(floppy->wpt_r())
			ret &= ~0x08;
		if(!floppy->dskchg_r())
			ret &= ~0x04;
	}

	return ret;
}

void amiga_fdc_device::index_callback(floppy_image_device *floppy, int state)
{
	/* Issue a index pulse when a disk revolution completes */
	m_write_index(!state);
}

void amiga_fdc_device::pll_t::set_clock(const attotime &period)
{
	for(int i=0; i<38; i++)
		delays[i] = period*(i+1);
}

void amiga_fdc_device::pll_t::reset(const attotime &when)
{
	counter = 0;
	increment = 146;
	transition_time = 0xffff;
	history = 0x80;
	slot = 0;
	ctime = when;
	phase_add = 0x00;
	phase_sub = 0x00;
	freq_add  = 0x00;
	freq_sub  = 0x00;
}

int amiga_fdc_device::pll_t::get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	attotime when = floppy ? floppy->get_next_transition(ctime) : attotime::never;

	for(;;) {
		attotime etime = ctime+delays[slot];
		if(etime > limit)
			return -1;
		if(transition_time == 0xffff && !when.is_never() && etime >= when)
			transition_time = counter;
		if(slot < 8) {
			uint8_t mask = 1 << slot;
			if(phase_add & mask)
				counter += 258;
			else if(phase_sub & mask)
				counter += 34;
			else
				counter += increment;

			if((freq_add & mask) && increment < 159)
				increment++;
			else if((freq_sub & mask) && increment > 134)
				increment--;
		} else
			counter += increment;

		slot++;
		tm = etime;
		if(counter & 0x800)
			break;
	}

	int bit = transition_time != 0xffff;

	if(transition_time != 0xffff) {
		static uint8_t const pha[8] = { 0xf, 0x7, 0x3, 0x1, 0, 0, 0, 0 };
		static uint8_t const phs[8] = { 0, 0, 0, 0, 0x1, 0x3, 0x7, 0xf };
		static uint8_t const freqa[4][8] = {
			{ 0xf, 0x7, 0x3, 0x1, 0, 0, 0, 0 },
			{ 0x7, 0x3, 0x1, 0, 0, 0, 0, 0 },
			{ 0x7, 0x3, 0x1, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0 }
		};
		static uint8_t const freqs[4][8] = {
			{ 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0x1, 0x3, 0x7 },
			{ 0, 0, 0, 0, 0, 0x1, 0x3, 0x7 },
			{ 0, 0, 0, 0, 0x1, 0x3, 0x7, 0xf },
		};

		int cslot = transition_time >> 8;
		phase_add = pha[cslot];
		phase_sub = phs[cslot];
		int way = transition_time & 0x400 ? 1 : 0;
		if(history & 0x80)
			history = way ? 0x80 : 0x83;
		else if(history & 0x40)
			history = way ? history & 2 : (history & 2) | 1;
		freq_add = freqa[history & 3][cslot];
		freq_sub = freqs[history & 3][cslot];
		history = way ? (history >> 1) | 2 : history >> 1;

	} else
		phase_add = phase_sub = freq_add = freq_sub = 0;

	counter &= 0x7ff;

	ctime = tm;
	transition_time = 0xffff;
	slot = 0;

	return bit;
}

void amiga_fdc_device::pll_t::start_writing(const attotime & tm)
{
	write_start_time = tm;
	write_position = 0;
}

void amiga_fdc_device::pll_t::stop_writing(floppy_image_device *floppy, const attotime &tm)
{
	commit(floppy, tm);
	write_start_time = attotime::never;
}

bool amiga_fdc_device::pll_t::write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	if(write_start_time.is_never()) {
		write_start_time = ctime;
		write_position = 0;
	}

	for(;;) {
		attotime etime = ctime+delays[slot];
		if(etime > limit)
			return true;
		uint16_t pre_counter = counter;
		counter += increment;
		if(bit && !(pre_counter & 0x400) && (counter & 0x400))
			if(write_position < std::size(write_buffer))
				write_buffer[write_position++] = etime;
		slot++;
		tm = etime;
		if(counter & 0x800)
			break;
	}

	counter &= 0x7ff;

	ctime = tm;
	slot = 0;

	return false;
}


void amiga_fdc_device::pll_t::commit(floppy_image_device *floppy, const attotime &tm)
{
	if(write_start_time.is_never() || tm == write_start_time)
		return;

	if(floppy)
		floppy->write_flux(write_start_time, tm, write_position, write_buffer);
	write_start_time = tm;
	write_position = 0;
}
