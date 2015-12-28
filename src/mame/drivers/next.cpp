// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    NeXT

    TODO:

    - Hook up the sound output, it is shared with the keyboard port

    - Implement more of the scc and its dma interactions so that the
      start up test passes, but not before sound out is done (if the scc
      test passes all the other test pass up to sound out which
      infloops)

    - Really implement the MO, it's only faking it for the startup test right now

    - Fix the networking

    - Find out why netbsd goes to hell even before loading the kernel

    Memory map and other bits can be found here:
    http://fxr.watson.org/fxr/source/arch/next68k/include/cpu.h?v=NETBSD5#L366

****************************************************************************/


#include "includes/next.h"
#include "formats/pc_dsk.h"
#include "formats/mfi_dsk.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"
#include "softlist.h"

UINT32 next_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// We don't handle partial updates, but we don't generate them either :-)
	if(cliprect.min_x || cliprect.min_y || cliprect.max_x+1 != screen_sx || cliprect.max_y+1 != screen_sy)
		return 0;

	// 0,1,2,4=0b000000, 3=2c000000, 5,8,9=0c000000
	// 1152x832 (0,1,2,3c,4)
	// 1120x832 (8)
	// 832x624 (5c,9c)
	if(screen_color) {
		const UINT32 *vr = vram;
		for(int y=0; y<screen_sy; y++) {
			UINT32 *pix = reinterpret_cast<UINT32 *>(bitmap.raw_pixptr(y));
			for(int x=0; x<screen_sx; x+=2) {
				UINT32 v = *vr++;
				for(int xi=0; xi<2; xi++) {
					UINT16 pen = (v >> (16-(xi*16))) & 0xffff;
					UINT32 r = (pen & 0xf000) >> 12;
					UINT32 g = (pen & 0x0f00) >> 8;
					UINT32 b = (pen & 0x00f0) >> 4;
					UINT32 col = (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
					*pix++ = col;
				}
			}
			vr += screen_skip;
		}
	} else {
		static UINT32 colors[4] = { 0xffffff, 0xaaaaaa, 0x555555, 0x000000 };
		const UINT32 *vr = vram;
		for(int y=0; y<screen_sy; y++) {
			UINT32 *pix = reinterpret_cast<UINT32 *>(bitmap.raw_pixptr(y));
			for(int x=0; x<screen_sx; x+=16) {
				UINT32 v = *vr++;
				for(int xi=0; xi<16; xi++)
					*pix++ = colors[(v >> (30-(xi*2))) & 0x3];
			}
			vr += screen_skip;
		}
	}

	return 0;
}

/* Dummy catcher for any unknown r/w */
READ8_MEMBER( next_state::io_r )
{
	if(!space.debugger_access())
		printf("io_r %08x (%08x)\n",offset+0x02000000, space.device().safe_pc());

	if(offset == 0xc0)
		return 0;

	return 0xff;
}

WRITE8_MEMBER( next_state::io_w )
{
	if(!space.debugger_access())
		printf("io_w %08x, %02x (%08x)\n",offset+0x02000000,data, space.device().safe_pc());
}

/* map ROM at 0x01000000-0x0101ffff? */
READ32_MEMBER( next_state::rom_map_r )
{
	if(0 && !space.debugger_access())
		printf("%08x ROM MAP?\n",space.device().safe_pc());
	return 0x01000000;
}

READ32_MEMBER( next_state::scr2_r )
{
	if(0 && !space.debugger_access())
		printf("%08x\n",space.device().safe_pc());
	/*
	x--- ---- ---- ---- ---- ---- ---- ---- dsp reset
	-x-- ---- ---- ---- ---- ---- ---- ---- dsp block end
	--x- ---- ---- ---- ---- ---- ---- ---- dsp unpacked
	---x ---- ---- ---- ---- ---- ---- ---- dsp mode b
	---- x--- ---- ---- ---- ---- ---- ---- dsp mode a
	---- -x-- ---- ---- ---- ---- ---- ---- remote int
	---- ---x ---- ---- ---- ---- ---- ---- local int
	---- ---- ---x ---- ---- ---- ---- ---- dram 256k
	---- ---- ---- ---x ---- ---- ---- ---- dram 1m
	---- ---- ---- ---- x--- ---- ---- ---- "timer on ipl7"
	---- ---- ---- ---- -xxx ---- ---- ---- rom waitstates
	---- ---- ---- ---- ---- x--- ---- ---- ROM 1M
	---- ---- ---- ---- ---- -x-- ---- ---- MCS1850 rtdata
	---- ---- ---- ---- ---- --x- ---- ---- MCS1850 rtclk
	---- ---- ---- ---- ---- ---x ---- ---- MCS1850 rtce
	---- ---- ---- ---- ---- ---- x--- ---- rom overlay
	---- ---- ---- ---- ---- ---- -x-- ---- dsp ie
	---- ---- ---- ---- ---- ---- --x- ---- mem en
	---- ---- ---- ---- ---- ---- ---- ---x led

	68040-25, 100ns, 32M: 00000c80
	68040-25, 100ns, 20M: 00ff0c80

	*/

	UINT32 data = scr2 & 0xfffffbff;

	data |= rtc->sdo_r() << 10;

	return data;
}

WRITE32_MEMBER( next_state::scr2_w )
{
	if(0 && !space.debugger_access())
		printf("scr2_w %08x (%08x)\n", data, space.device().safe_pc());
	COMBINE_DATA(&scr2);

	rtc->ce_w(BIT(scr2, 8));
	rtc->sdi_w(BIT(scr2, 10));
	rtc->sck_w(BIT(scr2, 9));
	irq_set(0, scr2 & 0x01000000);
	irq_set(1, scr2 & 0x02000000);
}

READ32_MEMBER( next_state::scr1_r )
{
	/*
	    xxxx ---- ---- ---- ---- ---- ---- ---- slot ID
	    ---- ---- xxxx xxxx ---- ---- ---- ---- DMA type
	    ---- ---- ---- ---- xxxx ---- ---- ---- machine type
	    ---- ---- ---- ---- ---- xxxx ---- ---- board revision
	    ---- ---- ---- ---- ---- ---- -xx- ---- video mem speed
	    ---- ---- ---- ---- ---- ---- ---x x--- mem speed
	    ---- ---- ---- ---- ---- ---- ---- -xxx cpu speed 16/20/25/33/40/50/66/80

	machine types:
	0 NeXT_CUBE
	1 NeXT_WARP9
	2 NeXT_X15
	3 NeXT_WARP9C
	4 NeXT_Turbo
	5 NeXT_TurboC
	6 Unknown
	7 Unknown
	8 NeXT_TurboCube
	9 NeXT_TurboCubeC

	    68040-25: 00011102
	    68040-25: 00013002 (non-turbo, color)
	*/

	return scr1;
}

// Interrupt subsystem
// source    bit   level
// nmi       31    7 *
// pfail     30    7
// timer     29    6 *
// enetxdma  28    6 *
// enetrdma  27    6 *
// scsidma   26    6 *
// diskdma   25    6
// prndma    24    6 *
// sndoutdma 23    6
// sndindma  22    6
// sccdma    21    6
// dspdma    20    6
// m2rdma    19    6
// r2mdma    18    6
// scc       17    5
// remote    16    5 *
// bus       15    5 *
// dsp4      14    4
// disk/cvid 13    3
// scsi      12    3 *
// printer   11    3
// enetx     10    3 *
// enetr      9    3
// soundovr   8    3 *
// phone      7    3 * -- floppy
// dsp3       6    3
// video      5    3
// monitor    4    3
// kbdmouse   3    3 *
// power      2    3 *
// softint1   1    2 *
// softint0   0    1 *

void next_state::irq_set(int id, bool raise)
{
	UINT32 mask = 1U << id;
	UINT32 old_status = irq_status;
	if(raise)
		irq_status |= mask;
	else
		irq_status &= ~mask;
	if(old_status != irq_status)
		irq_check();
}


READ32_MEMBER( next_state::irq_status_r )
{
	return irq_status;
}

READ32_MEMBER( next_state::irq_mask_r )
{
	return irq_mask;
}

WRITE32_MEMBER( next_state::irq_mask_w )
{
	COMBINE_DATA(&irq_mask);
	irq_check();
}

void next_state::irq_check()
{
	UINT32 act = irq_status & (irq_mask | 0x80000000);
	int bit;
	for(bit=31; bit >= 0 && !(act & (1U << bit)); bit--);

	int level;
	if     (bit <  0) level = 0;
	else if(bit <  1) level = 1;
	else if(bit <  2) level = 2;
	else if(bit < 14) level = 3;
	else if(bit < 15) level = 4;
	else if(bit < 18) level = 5;
	else if(bit < 30) level = 6;
	else              level = 7;

	logerror("IRQ info %08x/%08x - %d\n", irq_status, irq_mask, level);

	if(level != irq_level) {
		maincpu->set_input_line(irq_level, CLEAR_LINE);
		maincpu->set_input_line_and_vector(level, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
		irq_level = level;
	}
}

const char *next_state::dma_targets[0x20] = {
	nullptr, "scsi", nullptr, nullptr, "soundout", "disk", nullptr, nullptr,
	"soundin", "printer", nullptr, nullptr, "scc", "dsp", nullptr, nullptr,
	"s-enetx", "enetx", nullptr, nullptr, "s-enetr", "enetr", nullptr, nullptr,
	"video", nullptr, nullptr, nullptr, "r2m", "m2r", nullptr, nullptr
};

const int next_state::dma_irqs[0x20] = {
	-1, 26, -1, -1, 23, 25, -1, -1, 22, 24, -1, -1, 21, 20, -1, -1,
	-1, 28, -1, -1, -1, 27, -1, -1,  5, -1, -1, -1, 18, 19, -1, -1
};

const bool next_state::dma_has_saved[0x20] = {
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, true,  false, false, false, true,  false, false,
	false, false, false, false, false, false, false, false,
};

const char *next_state::dma_name(int slot)
{
	static char buf[32];
	if(dma_targets[slot])
		return dma_targets[slot];
	sprintf(buf, "<%02x>", slot);
	return buf;
}

void next_state::dma_drq_w(int slot, bool state)
{
	//  fprintf(stderr, "DMA drq_w %d, %d\n", slot, state);
	dma_slot &ds = dma_slots[slot];
	ds.drq = state;
	if(state && (ds.state & DMA_ENABLE)) {
		address_space &space = maincpu->space(AS_PROGRAM);
		if(ds.state & DMA_READ) {
			while(ds.drq) {
				dma_check_update(slot);
				UINT8 val;
				bool eof;
				bool err;
				dma_read(slot, val, eof, err);
				if(err) {
					ds.state = (ds.state & ~DMA_ENABLE) | DMA_BUSEXC;
					logerror("DMA: bus error on read slot %d\n", slot);
					return;
				}
				space.write_byte(ds.current++, val);
				dma_check_end(slot, eof);
				if(!(ds.state & DMA_ENABLE))
					return;
			}
		} else {
			while(ds.drq) {
				dma_check_update(slot);
				UINT8 val = space.read_byte(ds.current++);
				bool eof = ds.current == (ds.limit & 0x7fffffff) && (ds.limit & 0x80000000);
				bool err;
				dma_write(slot, val, eof, err);
				if(err) {
					ds.state = (ds.state & ~DMA_ENABLE) | DMA_BUSEXC;
					logerror("DMA: bus error on write slot %d\n", slot);
					return;
				}
				dma_check_end(slot, false);
				if(!(ds.state & DMA_ENABLE))
					return;
			}
		}
	}
}

void next_state::dma_read(int slot, UINT8 &val, bool &eof, bool &err)
{
	err = false;
	eof = false;
	switch(slot) {
	case 1:
		if(fdc && fdc->get_drq()) {
			val = fdc->dma_r();
			if(eof) {
				fdc->tc_w(true);
				fdc->tc_w(false);
			}
		} else
			val = scsi->dma_r();
		break;

	case 5:
		val = mo->dma_r();
		break;

	case 21:
		net->rx_dma_r(val, eof);
		logerror("dma read net %02x %s\n", val, eof ? "eof" : "");
		break;

	default:
		err = true;
		val = 0;
		break;
	}
}

void next_state::dma_write(int slot, UINT8 data, bool eof, bool &err)
{
	err = false;
	switch(slot) {
	case 1:
		if(fdc && fdc->get_drq()) {
			fdc->dma_w(data);
			if(eof) {
				fdc->tc_w(true);
				fdc->tc_w(false);
			}
		} else
			scsi->dma_w(data);
		break;

	case 4:
		break;

	case 5:
		mo->dma_w(data);
		break;

	case 17:
		net->tx_dma_w(data, eof);
		break;

	default:
		err = true;
		break;
	}
}

void next_state::dma_check_update(int slot)
{
	dma_slot &ds = dma_slots[slot];
	if(ds.restart) {
		ds.current = ds.start;
		ds.restart = false;
	}
}

void next_state::dma_end(int slot)
{
	dma_slot &ds = dma_slots[slot];
	if(dma_has_saved[slot]) {
		dma_slot &ds1 = dma_slots[(slot-1) & 31];
		ds1.current = ds.start;
		ds1.limit = ds.current;
	}

	if(!ds.supdate)
		ds.state &= ~DMA_ENABLE;
	else {
		ds.start = ds.chain_start;
		ds.limit = ds.chain_limit;
		ds.restart = true;
		ds.supdate = false;
		ds.state &= ~DMA_SUPDATE;
	}
	ds.state |= DMA_COMPLETE;
	logerror("dma end slot %d irq %d\n", slot, dma_irqs[slot]);
	if(dma_irqs[slot] >= 0)
		irq_set(dma_irqs[slot], true);
}

void next_state::dma_check_end(int slot, bool eof)
{
	dma_slot &ds = dma_slots[slot];
	if(eof || ds.current == (ds.limit & 0x7fffffff))
		dma_end(slot);
}

READ32_MEMBER( next_state::dma_regs_r)
{
	int slot = offset >> 2;
	int reg = offset & 3;

	UINT32 res;

	switch(reg) {
	case 0:
		res = dma_slots[slot].current;
		break;
	case 1:
		res = dma_slots[slot].limit;
		break;
	case 2:
		res = dma_slots[slot].chain_start;
		break;
	case 3: default:
		res = dma_slots[slot].chain_limit;
		break;
	}

	const char *name = dma_name(slot);
	logerror("dma_regs_r %s:%d %08x (%08x)\n", name, reg, res, space.device().safe_pc());

	return res;
}

WRITE32_MEMBER( next_state::dma_regs_w)
{
	int slot = offset >> 2;
	int reg = offset & 3;

	const char *name = dma_name(slot);

	logerror("dma_regs_w %s:%d %08x (%08x)\n", name, reg, data, space.device().safe_pc());
	switch(reg) {
	case 0:
		dma_slots[slot].start = data;
		dma_slots[slot].current = data;
		break;
	case 1:
		dma_slots[slot].limit = data;
		break;
	case 2:
		dma_slots[slot].chain_start = data;
		break;
	case 3:
		dma_slots[slot].chain_limit = data;
		break;
	}
}

READ32_MEMBER( next_state::dma_ctrl_r)
{
	int slot = offset >> 2;
	int reg = offset & 3;

	const char *name = dma_name(slot);

	if(space.device().safe_pc() != 0x409bb4e)
		logerror("dma_ctrl_r %s:%d %02x (%08x)\n", name, reg, dma_slots[slot].state, space.device().safe_pc());

	return reg ? 0 : dma_slots[slot].state << 24;
}

WRITE32_MEMBER( next_state::dma_ctrl_w)
{
	int slot = offset >> 2;
	int reg = offset & 3;
	const char *name = dma_name(slot);
	logerror("dma_ctrl_w %s:%d %08x @ %08x (%08x)\n", name, reg, data, mem_mask, space.device().safe_pc());
	if(!reg) {
		if(ACCESSING_BITS_16_23)
			dma_do_ctrl_w(slot, data >> 16);
		else if(ACCESSING_BITS_24_31)
			dma_do_ctrl_w(slot, data >> 24);
	}
}

void next_state::dma_do_ctrl_w(int slot, UINT8 data)
{
	const char *name = dma_name(slot);
#if 0
	fprintf(stderr, "dma_ctrl_w %s %02x (%08x)\n", name, data, maincpu->pc());

	fprintf(stderr, "  ->%s%s%s%s%s%s%s\n",
			data & DMA_SETENABLE ? " enable" : "",
			data & DMA_SETSUPDATE ? " supdate" : "",
			data & DMA_SETREAD ? " read" : "",
			data & DMA_CLRCOMPLETE ? " complete" : "",
			data & DMA_RESET ? " reset" : "",
			data & DMA_INITBUF ? " initbuf" : "",
			data & DMA_INITBUFTURBO ? " initbufturbo" : "");
#endif
	if(data & DMA_SETENABLE)
		logerror("dma enable %s %s %08x (%08x)\n", name, data & DMA_SETREAD ? "read" : "write", (dma_slots[slot].limit-dma_slots[slot].start) & 0x7fffffff, maincpu->pc());

	dma_slot &ds = dma_slots[slot];
	if(data & (DMA_RESET|DMA_INITBUF|DMA_INITBUFTURBO)) {
		ds.state = 0;
		if(dma_irqs[slot] >= 0)
			irq_set(dma_irqs[slot], false);
	}
	if(data & DMA_SETSUPDATE) {
		ds.state |= DMA_SUPDATE;
		ds.supdate = true;
	}
	if(data & DMA_SETREAD)
		ds.state |= DMA_READ;
	if(data & DMA_CLRCOMPLETE) {
		ds.state &= ~DMA_COMPLETE;
		if(dma_irqs[slot] >= 0)
			irq_set(dma_irqs[slot], false);
	}
	if(data & DMA_SETENABLE) {
		ds.state |= DMA_ENABLE;
		//      fprintf(stderr, "dma slot %d drq=%s\n", slot, ds.drq ? "on" : "off");
		if(ds.drq)
			dma_drq_w(slot, ds.drq);
	}
}

const int next_state::scsi_clocks[4] = { 10000000, 12000000, 20000000, 16000000 };

READ32_MEMBER( next_state::scsictrl_r )
{
	UINT32 res = (scsictrl << 24) | (scsistat << 16);
	logerror("scsictrl_read %08x @ %08x (%08x)\n", res, mem_mask, space.device().safe_pc());
	return res;
}

WRITE32_MEMBER( next_state::scsictrl_w )
{
	if(ACCESSING_BITS_24_31) {
		scsictrl = data >> 24;
		if(scsictrl & 0x02)
			scsi->reset();
		device_t::static_set_clock(*scsi, scsi_clocks[scsictrl >> 6]);

		logerror("SCSIctrl %dMHz int=%s dma=%s dmadir=%s%s%s dest=%s (%08x)\n",
				scsi_clocks[scsictrl >> 6]/1000000,
				scsictrl & 0x20 ? "on" : "off",
				scsictrl & 0x10 ? "on" : "off",
				scsictrl & 0x08 ? "read" : "write",
				scsictrl & 0x04 ? " flush" : "",
				scsictrl & 0x02 ? " reset" : "",
				scsictrl & 0x01 ? "wd3392" : "ncr5390",
				space.device().safe_pc());
	}
	if(ACCESSING_BITS_16_23) {
		scsistat = data >> 16;
		logerror("SCSIstat %02x (%08x)\n", data, space.device().safe_pc());
	}
}

READ32_MEMBER( next_state::event_counter_r)
{
	// Event counters, around that time, are usually fixed-frequency counters.
	// This one being 1MHz seems to make sense

	// The v74 rom seems pretty convinced that it's 20 bits only.

	if(ACCESSING_BITS_24_31)
		eventc_latch = machine().time().as_ticks(1000000) & 0xfffff;
	return eventc_latch;
}

READ32_MEMBER( next_state::dsp_r)
{
	return 0x7fffffff;
}

WRITE32_MEMBER( next_state::fdc_control_w )
{
	logerror("FDC write %02x (%08x)\n", data >> 24, space.device().safe_pc());
}

READ32_MEMBER( next_state::fdc_control_r )
{
	// Type of floppy present
	// 0 = no floppy in drive
	// 1 = ed
	// 2 = hd
	// 3 = dd

	// The rom strangely can't boot on anything else than ED, it has
	// code for the other densities but forces ED for some mysterious
	// reason.  The kernel otoh behaves as expected.

	if(fdc) {
		floppy_image_device *fdev = machine().device<floppy_connector>(":fdc:0")->get_device();
		if(fdev->exists()) {
			UINT32 variant = fdev->get_variant();
			switch(variant) {
			case floppy_image::SSSD:
			case floppy_image::SSDD:
			case floppy_image::DSDD:
				return 3 << 24;

			case floppy_image::DSHD:
				return 2 << 24;

			case floppy_image::DSED:
				return 1 << 24;
			}
		}
	}

	return 0 << 24;
}

READ32_MEMBER( next_state::phy_r )
{
	logerror("phy_r %d %08x (%08x)\n", offset, phy[offset], space.device().safe_pc());
	return phy[offset] | (0 << 24);
}

WRITE32_MEMBER( next_state::phy_w )
{
	COMBINE_DATA(phy+offset);
	logerror("phy_w %d %08x (%08x)\n", offset, phy[offset], space.device().safe_pc());
}

void next_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	irq_set(29, true);
	timer_data = timer_next_data;
	if(timer_ctrl & 0x40000000)
		timer_start();
	else
		timer_ctrl &= 0x7fffffff;
}

READ32_MEMBER( next_state::timer_data_r )
{
	if(timer_ctrl & 0x80000000)
		timer_update();
	return timer_data;
}

WRITE32_MEMBER( next_state::timer_data_w )
{
	if(timer_ctrl & 0x80000000) {
		COMBINE_DATA(&timer_next_data);
		timer_next_data &= 0xffff0000;
	} else {
		COMBINE_DATA(&timer_data);
		timer_data &= 0xffff0000;
	}
}

READ32_MEMBER( next_state::timer_ctrl_r )
{
	irq_set(29, false);
	return timer_ctrl;
}

WRITE32_MEMBER( next_state::timer_ctrl_w )
{
	bool oldact = timer_ctrl & 0x80000000;
	COMBINE_DATA(&timer_ctrl);
	bool newact = timer_ctrl & 0x80000000;
	if(oldact != newact) {
		if(oldact) {
			timer_update();
			irq_set(29, false);
		} else {
			timer_next_data = timer_data;
			timer_start();
		}
	}
}

void next_state::timer_update()
{
	int delta = timer_vbase - (machine().time() - timer_tbase).as_ticks(1000000);
	if(delta < 0)
		delta = 0;
	timer_data = delta << 16;
}

void next_state::timer_start()
{
	timer_tbase = machine().time();
	timer_vbase = timer_data >> 16;
	timer_tm->adjust(attotime::from_usec(timer_vbase));
}

WRITE_LINE_MEMBER(next_state::scc_irq)
{
	irq_set(17, state);
}

WRITE_LINE_MEMBER(next_state::keyboard_irq)
{
	irq_set(3, state);
}

WRITE_LINE_MEMBER(next_state::power_irq)
{
	irq_set(2, state);
}

WRITE_LINE_MEMBER(next_state::nmi_irq)
{
	irq_set(31, state);
}

WRITE_LINE_MEMBER(next_state::fdc_irq)
{
	irq_set(7, state);
}

WRITE_LINE_MEMBER(next_state::fdc_drq)
{
	dma_drq_w(1, state);
}

WRITE_LINE_MEMBER(next_state::net_tx_irq)
{
	irq_set(10, state);
}

WRITE_LINE_MEMBER(next_state::net_rx_irq)
{
	irq_set(9, state);
}

WRITE_LINE_MEMBER(next_state::net_tx_drq)
{
	dma_drq_w(17, state);
}

WRITE_LINE_MEMBER(next_state::net_rx_drq)
{
	dma_drq_w(21, state);
}

WRITE_LINE_MEMBER(next_state::mo_irq)
{
	irq_set(13, state);
}

WRITE_LINE_MEMBER(next_state::mo_drq)
{
	dma_drq_w(5, state);
}

WRITE_LINE_MEMBER(next_state::scsi_irq)
{
	irq_set(12, state);
}

WRITE_LINE_MEMBER(next_state::scsi_drq)
{
	dma_drq_w(1, state);
}

WRITE8_MEMBER(next_state::ramdac_w)
{
	switch(offset) {
	case 0:
		switch(data) {
		case 0x05:
			if(screen_color)
				irq_set(13, false);
			else
				irq_set(5, false);
			vbl_enabled = false;
			break;

		case 0x06:
			vbl_enabled = true;
			break;

		default:
			fprintf(stderr, "ramdac_w %d, %02x\n", offset, data);
			break;
		}
		break;

	default:
		fprintf(stderr, "ramdac_w %d, %02x\n", offset, data);
		break;
	}
}

void next_state::setup(UINT32 _scr1, int size_x, int size_y, int skip, bool color)
{
	scr1 = _scr1;
	screen_sx = size_x;
	screen_sy = size_y;
	screen_skip = skip;
	screen_color = color;
}

void next_state::machine_start()
{
	save_item(NAME(scr2));
	save_item(NAME(irq_status));
	save_item(NAME(irq_mask));
	save_item(NAME(irq_level));
	save_item(NAME(phy));
	save_item(NAME(scsictrl));
	save_item(NAME(scsistat));
	save_item(NAME(timer_tbase));
	save_item(NAME(timer_vbase));
	save_item(NAME(timer_data));
	save_item(NAME(timer_next_data));
	save_item(NAME(timer_ctrl));
	save_item(NAME(eventc_latch));
	save_item(NAME(esp));

	for(int i=0; i<0x20; i++) {
		save_item(NAME(dma_slots[i].start), i);
		save_item(NAME(dma_slots[i].limit), i);
		save_item(NAME(dma_slots[i].chain_start), i);
		save_item(NAME(dma_slots[i].chain_limit), i);
		save_item(NAME(dma_slots[i].current), i);
		save_item(NAME(dma_slots[i].state), i);
		save_item(NAME(dma_slots[i].supdate), i);
		save_item(NAME(dma_slots[i].restart), i);
		save_item(NAME(dma_slots[i].drq), i);
	}

	timer_tm = timer_alloc(0);

	system_time systime;
	machine().base_datetime(systime);
	rtc->set_counter(systime.time);
}

void next_state::machine_reset()
{
	scr2 = 0;
	irq_status = 0;
	irq_mask = 0;
	irq_level = 0;
	esp = 0;
	scsictrl = 0;
	scsistat = 0;
	phy[0] = phy[1] = 0;
	eventc_latch = 0;
	timer_vbase = 0;
	timer_data = 0;
	timer_next_data = 0;
	timer_ctrl = 0;
	vbl_enabled = true;
	dma_drq_w(4, true); // soundout
}

void next_state::vblank_w(screen_device &screen, bool vblank_state)
{
	if(vbl_enabled) {
		if(screen_color)
			irq_set(13, vblank_state);
		else
			irq_set(5, vblank_state);
	}
}

static ADDRESS_MAP_START( next_mem, AS_PROGRAM, 32, next_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x01000000, 0x0101ffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x02000000, 0x020001ff) AM_MIRROR(0x300200) AM_READWRITE(dma_ctrl_r, dma_ctrl_w)
	AM_RANGE(0x02004000, 0x020041ff) AM_MIRROR(0x300200) AM_READWRITE(dma_regs_r, dma_regs_w)
	AM_RANGE(0x02006000, 0x0200600f) AM_MIRROR(0x300000) AM_DEVICE8("net", mb8795_device, map, 0xffffffff)
//  AM_RANGE(0x02006010, 0x02006013) AM_MIRROR(0x300000) memory timing
	AM_RANGE(0x02007000, 0x02007003) AM_MIRROR(0x300000) AM_READ(irq_status_r)
	AM_RANGE(0x02007800, 0x02007803) AM_MIRROR(0x300000) AM_READWRITE(irq_mask_r,irq_mask_w)
	AM_RANGE(0x02008000, 0x02008003) AM_MIRROR(0x300000) AM_READ(dsp_r)
	AM_RANGE(0x0200c000, 0x0200c003) AM_MIRROR(0x300000) AM_READ(scr1_r)
	AM_RANGE(0x0200c800, 0x0200c803) AM_MIRROR(0x300000) AM_READ(rom_map_r)
	AM_RANGE(0x0200d000, 0x0200d003) AM_MIRROR(0x300000) AM_READWRITE(scr2_r,scr2_w)
//  AM_RANGE(0x0200d800, 0x0200d803) AM_MIRROR(0x300000) RMTINT
	AM_RANGE(0x0200e000, 0x0200e00b) AM_MIRROR(0x300000) AM_DEVICE("keyboard", nextkbd_device, amap)
//  AM_RANGE(0x0200f000, 0x0200f003) AM_MIRROR(0x300000) printer
//  AM_RANGE(0x02010000, 0x02010003) AM_MIRROR(0x300000) brightness
	AM_RANGE(0x02012000, 0x0201201f) AM_MIRROR(0x300000) AM_DEVICE8("mo", nextmo_device, map, 0xffffffff)
	AM_RANGE(0x02014000, 0x0201400f) AM_MIRROR(0x300000) AM_DEVICE8("scsibus:7:ncr5390", ncr5390_device, map, 0xffffffff)
	AM_RANGE(0x02014020, 0x02014023) AM_MIRROR(0x300000) AM_READWRITE(scsictrl_r, scsictrl_w)
	AM_RANGE(0x02016000, 0x02016003) AM_MIRROR(0x300000) AM_READWRITE(timer_data_r, timer_data_w)
	AM_RANGE(0x02016004, 0x02016007) AM_MIRROR(0x300000) AM_READWRITE(timer_ctrl_r, timer_ctrl_w)
	AM_RANGE(0x02018000, 0x02018003) AM_MIRROR(0x300000) AM_DEVREADWRITE8("scc", scc8530_t, reg_r, reg_w, 0xffffffff)
//  AM_RANGE(0x02018004, 0x02018007) AM_MIRROR(0x300000) SCC CLK
//  AM_RANGE(0x02018190, 0x02018197) AM_MIRROR(0x300000) warp 9c DRAM timing
//  AM_RANGE(0x02018198, 0x0201819f) AM_MIRROR(0x300000) warp 9c VRAM timing
	AM_RANGE(0x0201a000, 0x0201a003) AM_MIRROR(0x300000) AM_READ(event_counter_r) // EVENTC
//  AM_RANGE(0x020c0000, 0x020c0004) AM_MIRROR(0x300000) BMAP
	AM_RANGE(0x020c0030, 0x020c0037) AM_MIRROR(0x300000) AM_READWRITE(phy_r, phy_w)
	AM_RANGE(0x04000000, 0x07ffffff) AM_RAM //work ram
//  AM_RANGE(0x0c000000, 0x0c03ffff) video RAM w A+B-AB function
//  AM_RANGE(0x0d000000, 0x0d03ffff) video RAM w (1-A)B function
//  AM_RANGE(0x0e000000, 0x0e03ffff) video RAM w ceil(A+B) function
//  AM_RANGE(0x0f000000, 0x0f03ffff) video RAM w AB function
//  AM_RANGE(0x10000000, 0x1003ffff) main RAM w A+B-AB function
//  AM_RANGE(0x14000000, 0x1403ffff) main RAM w (1-A)B function
//  AM_RANGE(0x18000000, 0x1803ffff) main RAM w ceil(A+B) function
//  AM_RANGE(0x1c000000, 0x1c03ffff) main RAM w AB function
ADDRESS_MAP_END

static ADDRESS_MAP_START( next_0b_m_nofdc_mem, AS_PROGRAM, 32, next_state )
	AM_RANGE(0x0b000000, 0x0b03ffff) AM_RAM AM_SHARE("vram")

	AM_IMPORT_FROM(next_mem)
ADDRESS_MAP_END

static ADDRESS_MAP_START( next_fdc_mem, AS_PROGRAM, 32, next_state )
	AM_RANGE(0x02014100, 0x02014107) AM_MIRROR(0x300000) AM_DEVICE8("fdc", n82077aa_device, map, 0xffffffff)
	AM_RANGE(0x02014108, 0x0201410b) AM_MIRROR(0x300000) AM_READWRITE(fdc_control_r, fdc_control_w)

	AM_IMPORT_FROM(next_mem)
ADDRESS_MAP_END

static ADDRESS_MAP_START( next_0b_m_mem, AS_PROGRAM, 32, next_state )
	AM_RANGE(0x0b000000, 0x0b03ffff) AM_RAM AM_SHARE("vram")

	AM_IMPORT_FROM(next_fdc_mem)
ADDRESS_MAP_END

static ADDRESS_MAP_START( next_0c_m_mem, AS_PROGRAM, 32, next_state )
	AM_RANGE(0x0c000000, 0x0c1fffff) AM_RAM AM_SHARE("vram")

	AM_IMPORT_FROM(next_fdc_mem)
ADDRESS_MAP_END

static ADDRESS_MAP_START( next_0c_c_mem, AS_PROGRAM, 32, next_state )
	AM_RANGE(0x0c000000, 0x0c1fffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x02018180, 0x02018183) AM_MIRROR(0x300000) AM_WRITE8(ramdac_w, 0xffffffff)

	AM_IMPORT_FROM(next_fdc_mem)
ADDRESS_MAP_END

static ADDRESS_MAP_START( next_2c_c_mem, AS_PROGRAM, 32, next_state )
	AM_RANGE(0x2c000000, 0x2c1fffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x02018180, 0x02018183) AM_MIRROR(0x300000) AM_WRITE8(ramdac_w, 0xffffffff)

	AM_IMPORT_FROM(next_fdc_mem)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( next )
INPUT_PORTS_END

FLOPPY_FORMATS_MEMBER( next_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( next_floppies )
	SLOT_INTERFACE( "35ed", FLOPPY_35_ED )
SLOT_INTERFACE_END

static SLOT_INTERFACE_START( next_scsi_devices )
	SLOT_INTERFACE("cdrom", NSCSI_CDROM)
	SLOT_INTERFACE("harddisk", NSCSI_HARDDISK)
	SLOT_INTERFACE_INTERNAL("ncr5390", NCR5390)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( ncr5390 )
	MCFG_DEVICE_CLOCK(10000000)
	MCFG_NCR5390_IRQ_HANDLER(DEVWRITELINE(":", next_state, scsi_irq))
	MCFG_NCR5390_DRQ_HANDLER(DEVWRITELINE(":", next_state, scsi_drq))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( next_base, next_state )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(next_state, screen_update)
	MCFG_SCREEN_SIZE(1120, 900)
	MCFG_SCREEN_VISIBLE_AREA(0, 1120-1, 0, 832-1)
	MCFG_SCREEN_VBLANK_DRIVER(next_state, vblank_w)

	// devices
	MCFG_NSCSI_BUS_ADD("scsibus")
	MCFG_DEVICE_ADD("rtc", MCCS1850, XTAL_32_768kHz)
	MCFG_DEVICE_ADD("scc", SCC8530, XTAL_25MHz)
	MCFG_Z8530_INTRQ_CALLBACK(WRITELINE(next_state, scc_irq))
	MCFG_DEVICE_ADD("keyboard", NEXTKBD, 0)
	MCFG_NEXTKBD_INT_CHANGE_CALLBACK(WRITELINE(next_state, keyboard_irq))
	MCFG_NEXTKBD_INT_POWER_CALLBACK(WRITELINE(next_state, power_irq))
	MCFG_NEXTKBD_INT_NMI_CALLBACK(WRITELINE(next_state, nmi_irq))
	MCFG_NSCSI_ADD("scsibus:0", next_scsi_devices, "harddisk", false)
	MCFG_NSCSI_ADD("scsibus:1", next_scsi_devices, "cdrom", false)
	MCFG_NSCSI_ADD("scsibus:2", next_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:3", next_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:4", next_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:5", next_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:6", next_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:7", next_scsi_devices, "ncr5390", true)
	MCFG_DEVICE_CARD_MACHINE_CONFIG("ncr5390", ncr5390)

	MCFG_DEVICE_ADD("net", MB8795, 0)
	MCFG_MB8795_TX_IRQ_CALLBACK(WRITELINE(next_state, net_tx_irq))
	MCFG_MB8795_RX_IRQ_CALLBACK(WRITELINE(next_state, net_rx_irq))
	MCFG_MB8795_TX_DRQ_CALLBACK(WRITELINE(next_state, net_tx_drq))
	MCFG_MB8795_RX_DRQ_CALLBACK(WRITELINE(next_state, net_rx_drq))

	MCFG_DEVICE_ADD("mo", NEXTMO, 0)
	MCFG_NEXTMO_IRQ_CALLBACK(WRITELINE(next_state, mo_irq))
	MCFG_NEXTMO_DRQ_CALLBACK(WRITELINE(next_state, mo_drq))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( next, next_base )
	MCFG_CPU_ADD("maincpu", M68030, XTAL_25MHz)
	MCFG_CPU_PROGRAM_MAP(next_0b_m_nofdc_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( next_fdc_base, next_base )
	MCFG_N82077AA_ADD("fdc", n82077aa_device::MODE_PS2)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(next_state, fdc_irq))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(next_state, fdc_drq))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", next_floppies, "35ed", next_state::floppy_formats)

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "next")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nexts, next_fdc_base )
	MCFG_CPU_ADD("maincpu", M68040, XTAL_25MHz)
	MCFG_CPU_PROGRAM_MAP(next_0b_m_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nexts2, next_fdc_base )
	MCFG_CPU_ADD("maincpu", M68040, XTAL_25MHz)
	MCFG_CPU_PROGRAM_MAP(next_0b_m_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nextsc, next_fdc_base )
	MCFG_CPU_ADD("maincpu", M68040, XTAL_25MHz)
	MCFG_CPU_PROGRAM_MAP(next_2c_c_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nextst, next_fdc_base )
	MCFG_CPU_ADD("maincpu", M68040, XTAL_33MHz)
	MCFG_CPU_PROGRAM_MAP(next_0b_m_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nextstc, next_fdc_base )
	MCFG_CPU_ADD("maincpu", M68040, XTAL_33MHz)
	MCFG_CPU_PROGRAM_MAP(next_0c_c_mem)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 832-1, 0, 624-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nextct, next_fdc_base )
	MCFG_CPU_ADD("maincpu", M68040, XTAL_33MHz)
	MCFG_CPU_PROGRAM_MAP(next_0c_m_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nextctc, next_fdc_base )
	MCFG_CPU_ADD("maincpu", M68040, XTAL_33MHz)
	MCFG_CPU_PROGRAM_MAP(next_0c_c_mem)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 832-1, 0, 624-1)
MACHINE_CONFIG_END

/* ROM definition */
#define ROM_NEXT_V1 \
	ROM_REGION32_BE( 0x20000, "user1", ROMREGION_ERASEFF ) \
	ROM_SYSTEM_BIOS( 0, "v12", "v1.2" ) /* MAC address/serial number word at 0xC: 005AD0 */ \
	ROMX_LOAD( "rev_1.2.bin",     0x0000, 0x10000, CRC(7070bd78) SHA1(e34418423da61545157e36b084e2068ad41c9e24), ROM_BIOS(1)) /* Label: "(C) 1990 NeXT, Inc. // All Rights Reserved. // Release 1.2 // 1142.02", underlabel exists but unknown */ \
	ROM_SYSTEM_BIOS( 1, "v10", "v1.0 v41" ) /* MAC address/serial number word at 0xC: 003090 */ \
	ROMX_LOAD( "rev_1.0_v41.bin", 0x0000, 0x10000, CRC(54df32b9) SHA1(06e3ecf09ab67a571186efd870e6b44028612371), ROM_BIOS(2)) /* Label: "(C) 1989 NeXT, Inc. // All Rights Reserved. // Release 1.0 // 1142.00", underlabel: "MYF // 1.0.41 // 0D5C" */ \
	ROM_SYSTEM_BIOS( 2, "v10p", "v1.0 v41 alternate" ) /* MAC address/serial number word at 0xC: 0023D9 */ \
	ROMX_LOAD( "rev_1.0_proto.bin", 0x0000, 0x10000, CRC(f44974f9) SHA1(09eaf9f5d47e379cfa0e4dc377758a97d2869ddc), ROM_BIOS(3)) /* Label: "(C) 1989 NeXT, Inc. // All Rights Reserved. // Release 1.0 // 1142.00", no underlabel */

#define ROM_NEXT_V2 \
	ROM_REGION32_BE( 0x20000, "user1", ROMREGION_ERASEFF ) \
	ROM_SYSTEM_BIOS( 0, "v25", "v2.5 v66" ) /* MAC address/serial number word at 0xC: 00F302 */ \
	ROMX_LOAD( "rev_2.5_v66.bin", 0x0000, 0x20000, CRC(f47e0bfe) SHA1(b3534796abae238a0111299fc406a9349f7fee24), ROM_BIOS(1)) \
	ROM_SYSTEM_BIOS( 1, "v24", "v2.4 v65" ) /* MAC address/serial number word at 0xC: 00A634 */ \
	ROMX_LOAD( "rev_2.4_v65.bin", 0x0000, 0x20000, CRC(74e9e541) SHA1(67d195351288e90818336c3a84d55e6a070960d2), ROM_BIOS(2)) \
	ROM_SYSTEM_BIOS( 2, "v22", "v2.2 v63" ) /* MAC address/serial number word at 0xC: 00894C */ \
	ROMX_LOAD( "rev_2.2_v63.bin", 0x0000, 0x20000, CRC(739d7c07) SHA1(48ffe54cf2038782a92a0850337c5c6213c98571), ROM_BIOS(3)) /* Label: "(C) 1990 NeXT Computer, Inc. // All Rights Reserved. // Release 2.1 // 2918.AB" */ \
	ROM_SYSTEM_BIOS( 3, "v21", "v2.1 v59" ) /* MAC address/serial number word at 0xC: 0072FE */ \
	ROMX_LOAD( "rev_2.1_v59.bin", 0x0000, 0x20000, CRC(f20ef956) SHA1(09586c6de1ca73995f8c9b99870ee3cc9990933a), ROM_BIOS(4)) \
	ROM_SYSTEM_BIOS( 4, "v12", "v1.2 v58" ) /* MAC address/serial number word at 0xC: 006372 */ \
	ROMX_LOAD( "rev_1.2_v58.bin", 0x0000, 0x20000, CRC(b815b6a4) SHA1(97d8b09d03616e1487e69d26609487486db28090), ROM_BIOS(5)) /* Label: "V58 // (C) 1990 NeXT, Inc. // All Rights Reserved // Release 1.2 // 1142.02" */

#define ROM_NEXT_V3 \
	ROM_REGION32_BE( 0x20000, "user1", ROMREGION_ERASEFF ) \
	ROM_SYSTEM_BIOS( 0, "v33", "v3.3 v74" ) /* MAC address/serial number word at 0xC: 123456 */ \
	ROMX_LOAD( "rev_3.3_v74.bin", 0x0000, 0x20000, CRC(fbc3a2cd) SHA1(a9bef655f26f97562de366e4a33bb462e764c929), ROM_BIOS(1)) \
	ROM_SYSTEM_BIOS( 1, "v32", "v3.2 v72" ) /* MAC address/serial number word at 0xC: 012f31 */ \
	ROMX_LOAD( "rev_3.2_v72.bin", 0x0000, 0x20000, CRC(e750184f) SHA1(ccebf03ed090a79c36f761265ead6cd66fb04329), ROM_BIOS(2)) \
	ROM_SYSTEM_BIOS( 2, "v30", "v3.0 v70" ) /* MAC address/serial number word at 0xC: 0106e8 */ \
	ROMX_LOAD( "rev_3.0_v70.bin", 0x0000, 0x20000, CRC(37250453) SHA1(a7e42bd6a25c61903c8ca113d0b9a624325ee6cf), ROM_BIOS(3))


ROM_START(next)
	ROM_NEXT_V1
ROM_END

ROM_START(nexts)
	ROM_NEXT_V2
ROM_END

ROM_START(nexts2)
	ROM_NEXT_V2
ROM_END

ROM_START(nextsc)
	ROM_NEXT_V2
ROM_END

ROM_START(nextst)
	ROM_NEXT_V3
ROM_END

ROM_START(nextstc)
	ROM_NEXT_V3
ROM_END

ROM_START(nextct)
	ROM_NEXT_V3
ROM_END

ROM_START(nextctc)
	ROM_NEXT_V3
ROM_END

DRIVER_INIT_MEMBER(next_state,next)
{
	setup(0x00010002, 1120, 832, 2, false);
}

DRIVER_INIT_MEMBER(next_state,nexts)
{
	setup(0x00011002, 1120, 832, 2, false);
}

DRIVER_INIT_MEMBER(next_state,nexts2)
{
	setup(0x00012102, 1120, 832, 2, false);
}

DRIVER_INIT_MEMBER(next_state,nextsc)
{
	setup(0x00013102, 1120, 832, 16, true);
}

DRIVER_INIT_MEMBER(next_state,nextst)
{
	setup(0x00014103, 1120, 832, 2, false);
}

DRIVER_INIT_MEMBER(next_state,nextstc)
{
	setup(0x00015103,  832, 624, 0, true);
}

DRIVER_INIT_MEMBER(next_state,nextct)
{
	setup(0x00018103, 1120, 832, 0, false);
}

DRIVER_INIT_MEMBER(next_state,nextctc)
{
	setup(0x00019103,  832, 624, 0, true);
}

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                 FULLNAME                      FLAGS */
COMP( 1987, next,    0,      0,       next,      next, next_state,    next,    "Next Software Inc",   "NeXT Cube",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1990, nexts,   0,      0,       nexts,     next, next_state,    nexts,   "Next Software Inc",   "NeXTstation",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1990, nexts2,  nexts,  0,       nexts2,    next, next_state,    nexts2,  "Next Software Inc",   "NeXTstation (X15 variant)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1990, nextsc,  nexts,  0,       nextsc,    next, next_state,    nextsc,  "Next Software Inc",   "NeXTstation color",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1990, nextst,  0,      0,       nextst,    next, next_state,    nextst,  "Next Software Inc",   "NeXTstation turbo",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1990, nextstc, nextst, 0,       nextstc,   next, next_state,    nextstc, "Next Software Inc",   "NeXTstation turbo color",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( ????, nextct,  nextst, 0,       nextct,    next, next_state,    nextct,  "Next Software Inc",   "NeXT Cube turbo",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( ????, nextctc, nextst, 0,       nextctc,   next, next_state,    nextctc, "Next Software Inc",   "NeXT Cube turbo color",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
