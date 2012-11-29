/***************************************************************************

    Copyright Olivier Galibert
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

/****************************************************f***********************

    Amiga floppy disk controller emulation

***************************************************************************/


#include "emu.h"
#include "includes/amiga.h"
#include "formats/ami_dsk.h"
#include "amigafdc.h"
#include "machine/6526cia.h"

const device_type AMIGA_FDC = &device_creator<amiga_fdc>;

FLOPPY_FORMATS_MEMBER( amiga_fdc::floppy_formats )
	FLOPPY_ADF_FORMAT
FLOPPY_FORMATS_END

amiga_fdc::amiga_fdc(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, AMIGA_FDC, "Amiga FDC", tag, owner, clock)
{
}

void amiga_fdc::device_start()
{
	static const char *names[] = { "0", "1", "2", "3" };
	for(int i=0; i != 4; i++) {
		floppy_connector *con = subdevice<floppy_connector>(names[i]);
		if(con)
			floppy_devices[i] = con->get_device();
		else
			floppy_devices[i] = 0;
	}

	floppy = 0;

	t_gen = timer_alloc(0);
}


void amiga_fdc::device_reset()
{
	floppy = 0;
	dsklen = 0x4000;
	dsksync = 0x4489;
	adkcon = 0;
	dmacon = 0;
	dskpt = 0;
	pre_dsklen = 0x4000;
	dma_value = 0;
	dma_state = DMA_IDLE;

	live_abort();
}

void amiga_fdc::dma_done()
{
	dma_state = DMA_IDLE;
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	amiga_custom_w(space, REG_INTREQ, 0x8000 | INTENA_DSKBLK, 0xffff);
}

void amiga_fdc::dma_write(UINT16 value)
{
	amiga_state *state = machine().driver_data<amiga_state>();
	(*state->m_chip_ram_w)(state, dskpt, value);

	dskpt += 2;
	dsklen--;
	if(dsklen & 0x3fff)
		dma_state = DMA_RUNNING_BYTE_0;
	else
		dma_done();
}

void amiga_fdc::live_start()
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

void amiga_fdc::checkpoint()
{
	checkpoint_live = cur_live;
}

void amiga_fdc::rollback()
{
	cur_live = checkpoint_live;
}

void amiga_fdc::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		t_gen->adjust(cur_live.tm - machine().time());
}

void amiga_fdc::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			rollback();
			live_run(machine().time());
		}
		if(cur_live.tm == machine().time()) {
			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE)
				cur_live.tm = attotime::never;
		}
		checkpoint();
	}
}

void amiga_fdc::live_abort()
{
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}

void amiga_fdc::live_run(attotime limit)
{
	if(cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	for(;;) {
		switch(cur_live.state) {
		case RUNNING: {
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
				fatalerror("amiga_fdc::live_run - cur_live.bit_counter > 8\n");

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
			break;
		}

		case RUNNING_SYNCPOINT: {
			if(cur_live.shift_reg == dsksync) {
				if(adkcon & 0x0400) {
					if(dma_state == DMA_WAIT_START) {
						cur_live.bit_counter = 0;

						if(!(dsklen & 0x3fff))
							dma_done();
						else
							dma_write(dsksync);

					} else if(dma_state != DMA_IDLE) {
						dma_write(dsksync);
						cur_live.bit_counter = 0;

					} else if(cur_live.bit_counter != 8)
						cur_live.bit_counter = 0;
				}
				dskbyt |= 0x1000;
				address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);
				amiga_custom_w(space, REG_INTREQ, 0x8000 | INTENA_DSKSYN, 0xffff);
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

				case DMA_RUNNING_BYTE_1: {
					dma_value |= cur_live.shift_reg & 0xff;
					dma_write(dma_value);
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

bool amiga_fdc::dma_enabled()
{
	return (dsklen & 0x8000) && ((dmacon & 0x0210) == 0x0210);
}

void amiga_fdc::dma_check()
{
	if(dma_enabled()) {
		if(dma_state == IDLE) {
			dma_state = adkcon & 0x0400 ? DMA_WAIT_START : DMA_RUNNING_BYTE_0;
			if(dma_state == DMA_RUNNING_BYTE_0 && !(dsklen & 0x3fff))
				dma_done();
		}
	} else
		dma_state = IDLE;
}

void amiga_fdc::adkcon_set(UINT16 data)
{
	live_sync();
	adkcon = data;
	live_run();
}

void amiga_fdc::dsklen_w(UINT16 data)
{
	live_sync();
	if(!(data & 0x8000) || (data == pre_dsklen)) {
		dsklen = pre_dsklen = data;
		dma_check();

		dskbyt = dskbyt & 0x9fff;
		if(data & 0x4000)
			dskbyt |= 0x2000;
		if(dma_state != DMA_IDLE)
			dskbyt |= 0x4000;
	} else
		pre_dsklen = data;
	live_run();
}

void amiga_fdc::dskpth_w(UINT16 data)
{
	live_sync();
	dskpt = (dskpt & 0xffff) | (data << 16);
	live_run();
}

void amiga_fdc::dskptl_w(UINT16 data)
{
	live_sync();
	dskpt = (dskpt & 0xffff0000) | data;
	live_run();
}

UINT16 amiga_fdc::dskpth_r()
{
	return dskpt >> 16;
}

UINT16 amiga_fdc::dskptl_r()
{
	return dskpt;
}

void amiga_fdc::dsksync_w(UINT16 data)
{
	live_sync();
	dsksync = data;
	live_run();
}

void amiga_fdc::dmacon_set(UINT16 data)
{
	live_sync();
	dmacon = data;
	dma_check();
	dskbyt = dskbyt & 0xbfff;
	if(dma_state != DMA_IDLE)
		dskbyt |= 0x4000;
	live_run();
}

UINT16 amiga_fdc::dskbytr_r()
{
	UINT16 res = dskbyt;
	dskbyt &= 0x7fff;
	return res;
}

void amiga_fdc::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	live_sync();
	live_run();
}

void amiga_fdc::setup_leds()
{
	if(floppy) {
		int drive =
			floppy == floppy_devices[0] ? 0 :
			floppy == floppy_devices[1] ? 1 :
			floppy == floppy_devices[2] ? 2 :
			3;

		output_set_value("drive_0_led", drive == 0);
		output_set_value("drive_1_led", drive == 1);
		output_set_value("drive_2_led", drive == 2);
		output_set_value("drive_3_led", drive == 3);

		set_led_status(machine(), 1, drive == 0); /* update internal drive led */
		set_led_status(machine(), 2, drive == 1); /* update external drive led */
	}
}

WRITE8_MEMBER( amiga_fdc::ciaaprb_w )
{
	floppy_image_device *old_floppy = floppy;

	live_sync();

	if(!(data & 0x08))
		floppy = floppy_devices[0];
	else if(!(data & 0x10))
		floppy = floppy_devices[1];
	else if(!(data & 0x20))
		floppy = floppy_devices[2];
	else if(!(data & 0x40))
		floppy = floppy_devices[3];
	else
		floppy = 0;

	if(old_floppy != floppy) {
		if(old_floppy)
			old_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());
		if(floppy)
			floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(FUNC(amiga_fdc::index_callback), this));
	}

	if(floppy) {
		floppy->ss_w(!((data >> 2) & 1));
		floppy->dir_w((data >> 1) & 1);
		floppy->stp_w(data & 1);
		floppy->mon_w((data >> 7) & 1);
		output_set_value("fdc_led",data & 0x80); // LED directly connected to FDC motor
	}

	if(floppy) {
		if(cur_live.state == IDLE)
			live_start();
	} else
		live_abort();

	setup_leds();
	live_run();
}

UINT8 amiga_fdc::ciaapra_r()
{
	UINT8 ret = 0x3c;
	if(floppy) {
		// fixit
		ret &= ~0x20;

		if(!floppy->trk00_r())
			ret &= ~0x10;
		if(!floppy->wpt_r())
			ret &= ~0x08;
		if(!floppy->dskchg_r())
			ret &= ~0x04;
	}
	return ret;
}

void amiga_fdc::index_callback(floppy_image_device *floppy, int state)
{
	/* Issue a index pulse when a disk revolution completes */
	device_t *cia = machine().device("cia_1");
	mos6526_flag_w(cia, !state);
}

void amiga_fdc::pll_t::set_clock(attotime period)
{
	for(int i=0; i<38; i++)
		delays[i] = period*(i+1);
}

void amiga_fdc::pll_t::reset(attotime when)
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

int amiga_fdc::pll_t::get_next_bit(attotime &tm, floppy_image_device *floppy, attotime limit)
{
	attotime when = floppy ? floppy->get_next_transition(ctime) : attotime::never;

	for(;;) {
		attotime etime = ctime+delays[slot];
		if(etime > limit)
			return -1;
		if(transition_time == 0xffff && !when.is_never() && etime >= when)
			transition_time = counter;
		if(slot < 8) {
			UINT8 mask = 1 << slot;
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
		static const UINT8 pha[8] = { 0xf, 0x7, 0x3, 0x1, 0, 0, 0, 0 };
		static const UINT8 phs[8] = { 0, 0, 0, 0, 0x1, 0x3, 0x7, 0xf };
		static const UINT8 freqa[4][8] = {
			{ 0xf, 0x7, 0x3, 0x1, 0, 0, 0, 0 },
			{ 0x7, 0x3, 0x1, 0, 0, 0, 0, 0 },
			{ 0x7, 0x3, 0x1, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0 }
		};
		static const UINT8 freqs[4][8] = {
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
