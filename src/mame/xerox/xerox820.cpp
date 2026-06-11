// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

        Xerox 820

        12/05/2009 Skeleton driver.

****************************************************************************/

/*

    TODO:

    - Xerox 820
        - floppy format has 3xcd at the end of track data
            :u109: write track 0
            :u109: track description 16xff ... 109xff 3xcd
        - keyboard conflicts with optional serial terminal
    - Xerox 820-II
        - floppy (read/write to FDC triggers Z80 WAIT)
        - Winchester
            - Shugart SA1004 (chs=256,4,40 ss=256)
            - Shugart SA606 (chs=160,6, ss=256)
            - Shugart SA1403D controller
    - Xerox 16/8
    - Emerald Microware X120 board
    - type in Monitor v1.0 from manual
    - ASCII keyboard
    - low-profile keyboard

    http://www.vintagesbc.it/?page_id=233
    http://mccworkshop.com/computers/comphistory7.htm
    http://bitsavers.org/bits/Xerox/820/
    http://bitsavers.org/bits/Xerox/820-II/
    http://www.classiccmp.org/dunfield/img54306/system.htm

    Note:
    - MK-82 have same roms as original Big Board
    - MK-83 have 256K of RAM

    8-inch formats
    77 tracks, 1 head, 26 sectors, 128 bytes sector length, first sector id 1
    77 tracks, 1 head, 26 sectors, 256 bytes sector length, first sector id 1

    5.25-inch formats
    40 tracks, 1 head, 18 sectors, 128 bytes sector length, first sector id 1
    40 tracks, 2 heads, 18 sectors, 128 bytes sector length, first sector id 1

    SmartROM and Plus2 ROM both come for 2.5MHz or 4MHz systems, and there is another distinction between variants for generic or Xerox keyboards
    http://www.microcodeconsulting.com/z80/plus2.htm
    http://www.microcodeconsulting.com/z80/smartrom.htm

*/


#include "emu.h"
#include "xerox820.h"

#include "bus/nscsi/devices.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "formats/flopimg.h"
#include "formats/imd_dsk.h"


/* Read/Write Handlers */

uint8_t xerox820_state::fdc_r(offs_t offset)
{
	// The first-gen 820's FD1771 data bus is inverted (m_fdc_xor = 0xff); the 820-II's
	// FD1797 is not (m_fdc_xor = 0x00).  With the 0xff inversion the WD status bit 7
	// (not-ready) reads back inverted, so the boot ROM's media-detect (selunt, F65D
	// jp m,F5F6) misreads a ready drive as not-ready and gives up.
	return m_fdc->read(offset) ^ m_fdc_xor;
}

void xerox820_state::fdc_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset, data ^ m_fdc_xor);
}

void xerox820_state::scroll_w(offs_t offset, uint8_t data)
{
	m_scroll = (offset >> 8) & 0x1f;
}

#ifdef UNUSED_CODE
void xerox820_state::x120_system_w(uint8_t data)
{
	/*

	    bit     signal      description

	    0       DSEL0       drive select bit 0 (01=A, 10=B, 00=C, 11=D)
	    1       DSEL1       drive select bit 1
	    2       SIDE        side select
	    3       VATT        video attribute (0=inverse, 1=blinking)
	    4       BELL        bell trigger
	    5       DENSITY     density (0=double, 1=single)
	    6       _MOTOR      disk motor (0=on, 1=off)
	    7       BANK        memory bank switch (0=RAM, 1=ROM/video)

	*/
}
#endif

void xerox820ii_state::bell_w(offs_t offset, uint8_t data)
{
	m_speaker->level_w(offset);
}

void xerox820ii_state::slden_w(offs_t offset, uint8_t data)
{
	// Port 0x30 = Select Single Density (FM), port 0x31 = Select Double Density (MFM)
	// (Technical Reference, I/O Port Assignments).  wd_fdc dden_w() is active for FM,
	// so the offset must be inverted: 0x30 -> FM (dden=1), 0x31 -> MFM (dden=0).
	m_fdc->dden_w(!offset);
}

void xerox820ii_state::chrom_w(offs_t offset, uint8_t data)
{
	m_chrom = offset;
}

void xerox820ii_state::lowlite_w(uint8_t data)
{
	m_lowlite = data;
}

void xerox820ii_state::sync_w(offs_t offset, uint8_t data)
{
	if (offset)
	{
		/* set external clocks for synchronous sio A */
	}
	else
	{
		/* set internal clocks for asynchronous sio A */
	}
}

/* Memory Maps */

void xerox820_state::xerox820_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).view(m_view);
	m_view[0](0x0000, 0x3fff).ram();
	m_view[1](0x0000, 0x0fff).rom().region(Z80_TAG, 0);
	m_view[1](0x3000, 0x3fff).ram().share("video_ram");
	map(0x4000, 0xffff).ram();
}

void xerox820_state::xerox820_io(address_map &map)
{
	map(0x00, 0x00).mirror(0xff03).w(COM8116_TAG, FUNC(com8116_device::str_w));
	map(0x04, 0x07).mirror(0xff00).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x08, 0x0b).mirror(0xff00).rw(Z80PIO_GP_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x0c, 0x0c).mirror(0xff03).w(COM8116_TAG, FUNC(com8116_device::stt_w));
	map(0x10, 0x13).mirror(0xff00).rw(FUNC(xerox820_state::fdc_r), FUNC(xerox820_state::fdc_w));
	map(0x14, 0x14).mirror(0x0003).select(0xff00).w(FUNC(xerox820_state::scroll_w));
	map(0x18, 0x1b).mirror(0xff00).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x1c, 0x1f).mirror(0xff00).rw(m_kbpio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}

void xerox820ii_state::xerox820ii_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xbfff).view(m_view);
	m_view[0](0x0000, 0xbfff).ram();
	m_view[1](0x0000, 0x1fff).rom().region(Z80_TAG, 0);
	m_view[1](0x3000, 0x3fff).ram().share("video_ram");
	// 16/8: in the banked-ROM view the 0x4000-0xBFFF window reaches the 8086's resident
	// RAM (Z80 0x4000 -> 8086 0xF8000, i.e. 8086 addr = Z80 offset + 0xF4000), so the
	// monitor's F033 block-move can read/write the 8086 mailbox (0x4600) + ROM signature
	// (0x8FF5).  Harmless on a plain 820-II (no 8086 -> reads 0xFF).
	// shared_ram_r/w also model the bus arbiter (see their comment).
	m_view[1](0x4000, 0xbfff).rw(FUNC(xerox820ii_state::shared_ram_r), FUNC(xerox820ii_state::shared_ram_w));
	map(0xc000, 0xffff).ram();
}

// 16/8 Z80->8086 doorbell.  Each Z80 OUT (A1) is latched as one interrupt request:
// INTR asserts (no PIC; the INTA cycle reads the floating bus = vector 0xFF -> the
// boot ROM's handler at 0xFFCEF, "inc word [4612]"), and the 8086's INTA pops one
// ring off the latch, releasing INTR only when no rings remain.  The count matters:
// the mailbox protocol counts doorbells in [4612], so two rings must deliver two
// INTs even when the second arrives while the 8086 still has IF=0 from the first
// (a plain held line would merge them - a lost wakeup that hangs the 86CON console
// mid-output).  On hardware the 8086 acknowledges within microseconds, faster than
// the Z80 can ring twice; under MAME's scheduler the Z80 can ring many times in one
// timeslice, so the latch must hold the count.
void xerox820ii_state::dbell_w(u8 data)
{
	if (!m_i8086)
		return;

	m_dbell_count++;
	m_i8086->set_input_line(0, ASSERT_LINE);
}

IRQ_CALLBACK_MEMBER(xerox820ii_state::i8086_dbell_inta)
{
	if (m_dbell_count)
		m_dbell_count--;
	if (m_dbell_count == 0)
		m_i8086->set_input_line(0, CLEAR_LINE);
	return 0xff; // floating bus during INTA
}

// 16/8 shared-RAM window.  The mailbox protocol acquires its lock bytes (mutex
// 0xF8000, per-device slot bytes 0xF8050/0xF8100/...) with an *unlocked* Z80
// "sla (hl)" racing the 8086's "lock shl" (acquire) and "lock mov [si],80h"
// (release) -- sound on hardware because the shared-RAM arbiter keeps the 8086
// off the bus between the read and write halves of the Z80's read-modify-write.
// MAME's Z80 core is resumable mid-instruction, so a timeslice can end between
// those halves and the 8086 can release a slot (write 0x80) in the gap; the
// Z80's stale write-back of 0x00 then orphans the lock -- both CPUs spin forever
// on a slot nobody owns and the 86CON console hangs (originally reproduced by
// typing while DIR output finishes: the Z80 kbd task spins on the slot exactly
// as the 8086 conin thread completes and releases it).
//
// Modelled as a load-locked/store-conditional reservation, measured in Z80
// cycles (a Z80 parked mid-instruction accrues none, so the test is immune to
// scheduling): shared_ram_r records {addr, value}; if the matching write of the
// same RMW instruction (same addr, <= 8 Z80 T-states later -- distinct
// instructions are always further apart) finds the location was changed by the
// 8086 in between, the Z80's RMW is serialized *before* the intervening 8086
// store: the stale write-back is dropped and the 8086's newer value stands.
// That is exactly the outcome of an arbiter granting the Z80 both halves
// back-to-back (Z80 sees the old value, fails its acquire, retries; the 8086's
// release lands last).  Suspending the 8086 for the gap instead was tried and
// rejected: the resume timer can fire while the Z80 is still parked
// mid-instruction (recreating the race), and the bus-hold churn from read-heavy
// Z80 loops starves the 8086 into a conout livelock.
u8 xerox820ii_state::shared_ram_r(offs_t offset)
{
	if (!m_i8086)
		return 0xff;

	u8 const data = m_i8086->space(AS_PROGRAM).read_byte(0xf8000 + offset);

	if (!machine().side_effects_disabled())
	{
		m_rmw_addr = offset;
		m_rmw_val = data;
		m_rmw_cycles = m_maincpu->total_cycles();
	}

	return data;
}

void xerox820ii_state::shared_ram_w(offs_t offset, u8 data)
{
	if (!m_i8086)
		return;

	if (offset == m_rmw_addr && u64(m_maincpu->total_cycles()) - m_rmw_cycles <= 8)
	{
		m_rmw_addr = ~offs_t(0); // reservation consumed
		if (m_i8086->space(AS_PROGRAM).read_byte(0xf8000 + offset) != m_rmw_val)
			return; // torn RMW: an 8086 store slipped between the halves; Z80 serializes first
	}

	m_i8086->space(AS_PROGRAM).write_byte(0xf8000 + offset, data);
}

// 820-II with the main-board FD1797 floppy controller (5.25" or 8" drives).
void xerox820ii_state::xerox820ii_io(address_map &map)
{
	xerox820_io(map); // FD1797 floppy controller stays mapped at 0x10-0x13
	map(0x28, 0x29).mirror(0xff00).w(FUNC(xerox820ii_state::bell_w));
	map(0x30, 0x31).mirror(0xff00).w(FUNC(xerox820ii_state::slden_w));
	map(0x34, 0x35).mirror(0xff00).w(FUNC(xerox820ii_state::chrom_w));
	map(0x36, 0x36).mirror(0xff00).w(FUNC(xerox820ii_state::lowlite_w));
	map(0x68, 0x69).mirror(0xff00).w(FUNC(xerox820ii_state::sync_w));
	// 16/8 8086 coprocessor control (Tech Ref p165).  The Z80 stops/starts the 8086
	// through read-strobes on A0/A1 (the data value is irrelevant on those):
	//   IN  A0        -> Stop 8086   (LOAD86 does this at 0x3CF, just before its
	//                                 shared-RAM test, so the test runs on a quiescent
	//                                 coprocessor -- otherwise the 8086 dispatcher keeps
	//                                 writing [4600]/[4616]/[461E] inside the verified
	//                                 window and LOAD86 reports "Shared RAM failure")
	//   IN  A1        -> Start 8086  (LOAD86 restarts it after the test, 0x4A0)
	//   OUT A0, D7=1  -> Lock 8086
	// OUT A1 is the Z80->8086 doorbell (the monitor's ring @E197 and 86CON's @DCB5/
	// DE3F/DE52: di / out A1 / touch the mailbox / ei); each ring must deliver one
	// INT 0xFF on the 8086 -- see dbell_w() above for the counted latch.
	// A2/A3 are defined but unused by the loader/monitor; A4-AF are reserved.
	map(0xa0, 0xa0).mirror(0xff00)
	    .lr8(NAME([this]() -> u8 { if (m_i8086) m_i8086->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); return 0xff; }))   // IN A0 = Stop = assert/hold RESET
	    .lw8(NAME([this](u8 data) { if (m_i8086 && BIT(data, 7)) m_i8086->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); })); // OUT A0,D7=1 = Lock (hold in reset)
	// IN A1 = Start = release RESET -> fresh POST from 0xFFFF0.  The POST writes its
	// status word at 0xF861E within microseconds of the release, and LOAD86's
	// presence test relies on winning that race: its first poll (an F033 window
	// read ~77us after the IN A1) must see a post-restart value (high byte 0 =
	// POST in progress, keep polling; 0xFFFF = present) -- on hardware the 8086
	// always wins.  Under MAME's coarse default quantum the Z80 can run a whole
	// timeslice ahead and read the stale pre-restart contents (LOAD86's own
	// shared-RAM test pattern), mis-classifying the 8086 as a missing option
	// ("16/8 PC <garbage> not present." and no co-processor CP/M).  End the Z80's
	// slice at the release and interleave finely while the POST runs.
	map(0xa1, 0xa1).mirror(0xff00)
	    .lr8(NAME([this]() -> u8 {
			if (m_i8086)
			{
				m_i8086->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				machine().scheduler().add_quantum(attotime::from_usec(10), attotime::from_msec(100));
				m_maincpu->abort_timeslice();
			}
			return 0xff;
		}))
	    .lw8(NAME([this](u8 data) { dbell_w(data); })); // OUT A1 = Z80->8086 doorbell (see dbell_w)
}

// 820-II with the Shugart SASI host adapter: the floppy + rigid disk hang off the
// SA1403D, reached through the "u8" Z80PIO (pio_rd) instead of the FD1797.  Port A
// (0x10 data / 0x11 ctrl) is the SASI data bus, port B (0x12 / 0x13) the SASI control
// and status lines.  The boot ROM programs it via write_alt ordering (out 11h = port-A
// mode, out 13h = port-B mode) and runs SELECT -> 6-byte CDB -> data -> status.
void xerox820ii_state::xerox820iis_io(address_map &map)
{
	xerox820ii_io(map);
	map(0x10, 0x13).mirror(0xff00).rw(Z80PIO_RD_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}

// --- RX024 5.25" floppy disk-controller ROM (Part 1) -----------------------
// The low-profile 16/8's 5.25" controller ("RX v024") presents a small ROM in
// the Z80 I/O space (read at port 0xBF, addressed by the B register; the box id
// at port 0xA6) that the v5.0 monitor's ddskld validates ($55AA) and copies a
// floppy driver out of, into RAM at $F360.  The controller ROM is undumped; the
// image below reconstructs the EPROM wrapper (header / diagnostic / checksum,
// per the ddskld load contract in the v5.0 monitor source xr.mac) around the
// *recovered* Balcones WDVR driver (wd1797.mac+seltab+fivdpb+dphdpb, dev disk
// B23D13).  The loaded driver then drives the main-board FD1797 at 0x10-0x13,
// so nothing else of the 5.25" hardware changes.  Only the 5.25" 16/8 (x1685)
// carries this controller.
static const uint8_t RX024_IMAGE[0x3a3] =
{
	0x01, 0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x03, 0x01, 0x04, 0x01, 0x05, 0x01, 0x06, 0x01, 0x07,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x2a, 0xf4, 0xb0, 0xf4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xfc, 0x00, 0x00, 0x00, 0xfd, 0x80, 0xfd,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xfc, 0x00, 0x00, 0x20, 0xfd, 0xa0, 0xfd,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xfc, 0x00, 0x00, 0x40, 0xfd, 0xc0, 0xfd,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xfc, 0x00, 0x00, 0x60, 0xfd, 0xe0, 0xfd,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xfc, 0x70, 0xf4, 0x00, 0x00, 0x00, 0xfe,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xfc, 0x80, 0xf4, 0x00, 0x00, 0x80, 0xfe,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xfc, 0x90, 0xf4, 0x00, 0x00, 0xc0, 0xfe,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xfc, 0xa0, 0xf4, 0x00, 0x00, 0xe0, 0xfe,
	0x01, 0x07, 0x0d, 0x13, 0x19, 0x05, 0x0b, 0x11, 0x17, 0x03, 0x09, 0x0f, 0x15, 0x02, 0x08, 0x0e,
	0x14, 0x1a, 0x06, 0x0c, 0x12, 0x18, 0x04, 0x0a, 0x10, 0x16, 0x21, 0x00, 0x00, 0xf6, 0xff, 0xc9,
	0x1a, 0x00, 0x03, 0x07, 0x00, 0xf2, 0x00, 0x3f, 0x00, 0xc0, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00,
	0x1a, 0x00, 0x04, 0x0f, 0x01, 0xf6, 0x00, 0x7f, 0x00, 0xc0, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00,
	0x34, 0x00, 0x04, 0x0f, 0x01, 0xf2, 0x00, 0x7f, 0x00, 0xc0, 0x00, 0x20, 0x00, 0x02, 0x00, 0x81,
	0x34, 0x00, 0x05, 0x1f, 0x03, 0xf6, 0x00, 0x7f, 0x00, 0xc0, 0x00, 0x20, 0x00, 0x02, 0x00, 0x81,
	0x12, 0x00, 0x03, 0x07, 0x00, 0x52, 0x00, 0x1f, 0x00, 0x80, 0x00, 0x08, 0x00, 0x03, 0x00, 0x00,
	0x12, 0x00, 0x03, 0x07, 0x00, 0xac, 0x00, 0x1f, 0x00, 0x80, 0x00, 0x08, 0x00, 0x03, 0x00, 0x00,
	0x22, 0x00, 0x03, 0x07, 0x00, 0x9c, 0x00, 0x3f, 0x00, 0xc0, 0x00, 0x10, 0x00, 0x03, 0x00, 0x81,
	0x22, 0x00, 0x04, 0x0f, 0x01, 0xa2, 0x00, 0x3f, 0x00, 0xc0, 0x00, 0x10, 0x00, 0x03, 0x00, 0x81,
	0xcd, 0x47, 0xf6, 0x7e, 0x23, 0x32, 0xe7, 0xf4, 0x3c, 0x28, 0x55, 0x06, 0x0a, 0xc5, 0xe5, 0x7e,
	0xcd, 0x44, 0xf5, 0xfa, 0x06, 0xf5, 0x23, 0x23, 0x4e, 0xcd, 0xa3, 0xf5, 0x4e, 0x20, 0x37, 0x23,
	0x23, 0x13, 0x1a, 0xe6, 0x18, 0x7e, 0x20, 0x06, 0x79, 0xb7, 0x7e, 0x28, 0x01, 0x3c, 0xd3, 0x12,
	0x23, 0x23, 0x5e, 0x23, 0x56, 0xeb, 0x3e, 0x00, 0xb7, 0x0e, 0xa8, 0x3e, 0xa3, 0x28, 0x03, 0x0e,
	0x88, 0x3d, 0x32, 0xfe, 0xf4, 0x3e, 0x00, 0x81, 0x4f, 0xcd, 0x1d, 0xf6, 0x76, 0xed, 0xa2, 0x20,
	0xfb, 0xcd, 0x39, 0xf6, 0xe6, 0xdf, 0xe1, 0xc1, 0xc8, 0xd8, 0xcd, 0x69, 0xf0, 0x10, 0xae, 0xc9,
	0x7e, 0xfe, 0x04, 0xd2, 0x2a, 0xf4, 0x26, 0x00, 0xcd, 0x39, 0xf0, 0x2b, 0x22, 0x59, 0xf5, 0x2b,
	0x22, 0x6e, 0xf6, 0x26, 0x00, 0x6f, 0x29, 0x29, 0x29, 0x29, 0x11, 0x90, 0xf3, 0x19, 0xe5, 0xcd,
	0x5a, 0xf6, 0xe1, 0xca, 0x2a, 0xf4, 0x71, 0x23, 0x70, 0x01, 0x0a, 0x00, 0x09, 0x72, 0x2b, 0x73,
	0xed, 0x42, 0xaf, 0xc9, 0x4f, 0xeb, 0x21, 0x00, 0xf7, 0x06, 0x00, 0x09, 0x09, 0x22, 0xd1, 0xf6,
	0xeb, 0x1a, 0x1b, 0xcd, 0x98, 0xf5, 0x3e, 0x06, 0x32, 0x59, 0xf5, 0xfb, 0xcb, 0x89, 0xdb, 0x1c,
	0x47, 0xe6, 0xf8, 0xb1, 0x3c, 0xd3, 0x1c, 0xa8, 0xe6, 0x03, 0x28, 0x25, 0x3e, 0xff, 0x12, 0xcb,
	0x60, 0x20, 0x1e, 0xcd, 0x47, 0xf6, 0x06, 0x08, 0xe5, 0x2a, 0x59, 0xf5, 0x7e, 0xe1, 0xd6, 0x04,
	0xd8, 0xdb, 0x10, 0xe6, 0x02, 0x28, 0xf1, 0x3a, 0x85, 0xf5, 0xee, 0x08, 0x32, 0x85, 0xf5, 0x10,
	0xe7, 0xdb, 0x10, 0xe6, 0x80, 0xc9, 0x3e, 0x18, 0x32, 0x32, 0xf6, 0xe6, 0x18, 0xd3, 0x31, 0xc8,
	0xd3, 0x30, 0xc9, 0x79, 0xb7, 0xcc, 0x96, 0xf5, 0x13, 0x1a, 0x1b, 0xe6, 0x01, 0x28, 0x15, 0xdb,
	0x1c, 0xcb, 0x67, 0x06, 0x4d, 0x20, 0x02, 0x06, 0x28, 0x79, 0xb8, 0x3e, 0x00, 0x38, 0x05, 0x79,
	0x90, 0x4f, 0x3e, 0x02, 0x32, 0xf6, 0xf4, 0x87, 0x47, 0xf3, 0xdb, 0x1c, 0xcb, 0x97, 0xb0, 0xfb,
	0xd3, 0x1c, 0x1a, 0xd3, 0x11, 0xb9, 0x28, 0x17, 0x3c, 0xcc, 0xf8, 0xf5, 0x28, 0x0d, 0x79, 0xd3,
	0x13, 0x3e, 0x1c, 0xcd, 0x43, 0xf6, 0xe6, 0x98, 0x79, 0x28, 0x02, 0xf6, 0xff, 0x12, 0xc9, 0xcd,
	0x47, 0xf6, 0xe6, 0x20, 0x28, 0xe8, 0xaf, 0xc9, 0xc5, 0xcd, 0x05, 0xf6, 0x20, 0x05, 0xcd, 0x41,
	0xf6, 0xe6, 0x04, 0xc1, 0xc9, 0x0e, 0xc4, 0xcd, 0x1d, 0xf6, 0x76, 0xed, 0x40, 0x76, 0xed, 0x48,
	0xcd, 0x39, 0xf6, 0xe6, 0x98, 0x20, 0x04, 0x78, 0xd3, 0x11, 0xf6, 0xaf, 0xc9, 0xf3, 0x3a, 0x66,
	0x00, 0x32, 0x3a, 0xf6, 0x3e, 0xc9, 0x32, 0x66, 0x00, 0x79, 0x01, 0x13, 0x14, 0xd3, 0x10, 0x10,
	0xfe, 0x3e, 0x00, 0xe6, 0x18, 0xc8, 0x06, 0x80, 0xc9, 0x3e, 0x00, 0x32, 0x66, 0x00, 0xfb, 0x18,
	0x0a, 0xaf, 0x12, 0xf6, 0x01, 0x18, 0x02, 0x3e, 0xd0, 0xd3, 0x10, 0x3e, 0x14, 0x3d, 0x20, 0xfd,
	0xcd, 0x66, 0xf0, 0xdb, 0x10, 0xcb, 0x47, 0x20, 0xf7, 0xc9, 0xcd, 0x44, 0xf5, 0xfa, 0xf6, 0xf5,
	0x21, 0xd5, 0xf6, 0x36, 0xa0, 0xdb, 0x1c, 0xcb, 0x67, 0x20, 0x02, 0xcb, 0xf6, 0x3a, 0x6e, 0xf6,
	0xe6, 0x03, 0x32, 0x44, 0xf6, 0xcd, 0x41, 0xf6, 0xe6, 0x84, 0xc8, 0xfa, 0xf6, 0xf5, 0xd3, 0x31,
	0x3e, 0xff, 0x12, 0x3e, 0x02, 0xd3, 0x13, 0x3e, 0x18, 0xcd, 0x43, 0xf6, 0x3e, 0x1c, 0xd3, 0x10,
	0x01, 0x00, 0x00, 0x10, 0xfe, 0xdb, 0x10, 0xcb, 0x47, 0x28, 0x08, 0x0d, 0x20, 0xf5, 0xcd, 0x47,
	0xf6, 0x3e, 0x18, 0xe6, 0x18, 0x13, 0x12, 0x1b, 0x28, 0x14, 0xd3, 0x30, 0x3e, 0x1c, 0xcd, 0x43,
	0xf6, 0xe6, 0x18, 0x28, 0x07, 0xcb, 0x7e, 0xcb, 0xbe, 0x20, 0xba, 0xc9, 0xcb, 0xae, 0xcb, 0xbe,
	0xdb, 0x1c, 0xcb, 0xd7, 0xd3, 0x1c, 0xcd, 0x05, 0xf6, 0x28, 0x09, 0x0d, 0x20, 0x06, 0xcb, 0xe6,
	0x21, 0xd1, 0xf6, 0x34, 0x21, 0x00, 0x00, 0x7d, 0x4c, 0x44, 0x11, 0x30, 0xf4, 0x19, 0xeb, 0xcb,
	0x6f, 0xc0, 0x01, 0xed, 0xf6, 0xcb, 0x77, 0xc0, 0x01, 0x10, 0xf4, 0x3c, 0xc9, 0x01, 0x06, 0x0b,
	0x10, 0x03, 0x08, 0x0d, 0x12, 0x05, 0x0a, 0x0f, 0x02, 0x07, 0x0c, 0x11, 0x04, 0x09, 0x0e, 0xf6,
	0xff, 0xc9, 0xb0,
};

uint8_t xerox820ii_state::rx024_rom_r(offs_t offset)
{
	const uint8_t low = 0xb0 + (offset & 0x0f); // port low byte (mapped 0xB0-0xBF)
	const uint8_t b   = (offset >> 8) & 0xff;    // B register = the ROM byte/index pointer ddskld walks down from 0xFF
	if (low == 0xbf)
	{
		switch (b) // ROM header (read at port 0xBF, indexed by B)
		{
		case 0xff: return 0x55;  case 0xfe: return 0xaa; // valnum 55AA
		case 0xfc: return 0x03;  case 0xfb: return 0xa3; // driver length 03A3
		case 0xfa: return 0xf3;  case 0xf9: return 0x60; // RAM load address F360
		case 0xf8: return 0xf6;  case 0xf7: return 0xff; // diagnostic entry F6FF (returns NZ = controller OK)
		default:   return 0xff;
		}
	}
	if (low <= 0xb3) // driver body: 4 blocks of 256 bytes (ports 0xB0-0xB3), indexed by B
	{
		const unsigned idx = (low - 0xb0) * 256 + b;
		return idx < sizeof(RX024_IMAGE) ? RX024_IMAGE[idx] : 0xff;
	}
	return 0xff;
}

// --- rgd5 5.25" rigid-disk-unit controller ROM ------------------------------
// The 16/8's 5.25" rigid disk unit (expansion box id 0x21) presents the same
// port-mapped controller ROM as the RX024 floppy box, carrying the loadable
// SASI driver (sdvr = seltab+dphdpb+rigdpb+sa1403, linked at F360).  The box
// ROM itself is undumped; this image is the sdvr driver rebuilt from the
// recovered Balcones v5.0 source (B23D13) with the 5.25" drive geometry
// (40 tracks, 17 sectors -- the shipped B23D13 binary carries the 8" unit's
// 77/26 constants; the source assembles byte-identical to it, so only the
// four geometry literals were changed).  1040 bytes at F360.
static const uint8_t SDVR_IMAGE[0x411] =
{
	0x01,0x00,0x01,0x01,0x01,0x02,0x01,0x03,0x01,0x04,0x01,0x05,0x01,0x06,0x01,0x07,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x2a,0xf4,0xb0,0xf4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xfc,0x00,0x00,0x00,0xfd,0x80,0xfd,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xfc,0x00,0x00,0x20,0xfd,0xa0,0xfd,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xfc,0x00,0x00,0x40,0xfd,0xc0,0xfd,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xfc,0x00,0x00,0x60,0xfd,0xe0,0xfd,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xfc,0x70,0xf4,0x00,0x00,0x00,0xfe,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xfc,0x80,0xf4,0x00,0x00,0x80,0xfe,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xfc,0x90,0xf4,0x00,0x00,0xc0,0xfe,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xfc,0xa0,0xf4,0x00,0x00,0xe0,0xfe,
	0x01,0x07,0x0d,0x13,0x19,0x05,0x0b,0x11,0x17,0x03,0x09,0x0f,0x15,0x02,0x08,0x0e,
	0x14,0x1a,0x06,0x0c,0x12,0x18,0x04,0x0a,0x10,0x16,0x21,0x00,0x00,0xf6,0xff,0xc9,
	0x1a,0x00,0x03,0x07,0x00,0xf2,0x00,0x3f,0x00,0xc0,0x00,0x10,0x00,0x02,0x00,0x00,
	0x1a,0x00,0x04,0x0f,0x01,0xf6,0x00,0x7f,0x00,0xc0,0x00,0x10,0x00,0x02,0x00,0x00,
	0x34,0x00,0x04,0x0f,0x01,0xf2,0x00,0x7f,0x00,0xc0,0x00,0x20,0x00,0x02,0x00,0x81,
	0x34,0x00,0x05,0x1f,0x03,0xf6,0x00,0x7f,0x00,0xc0,0x00,0x20,0x00,0x02,0x00,0x81,
	0x00,0x02,0x05,0x1f,0x01,0xef,0x03,0xff,0x01,0xff,0x00,0x00,0x00,0x01,0x00,0x01,
	0x00,0x02,0x05,0x1f,0x01,0xef,0x01,0xff,0x01,0xff,0x00,0x00,0x00,0x41,0x00,0x01,
	0x00,0x02,0x05,0x1f,0x03,0xef,0x00,0xff,0x01,0xff,0x00,0x00,0x00,0x61,0x00,0x01,
	0x00,0x02,0x05,0x1f,0x03,0xef,0x00,0xff,0x01,0xff,0x00,0x00,0x00,0x71,0x00,0x01,
	0x06,0x06,0x7e,0x23,0x3c,0x28,0x49,0xe5,0xc5,0x3d,0x3e,0x0a,0x28,0x02,0x3e,0x08,
	0x32,0xf0,0xf6,0x7e,0xe5,0xcd,0xaf,0xf5,0xe1,0x23,0x23,0x56,0x23,0x23,0x5e,0xcd,
	0xd7,0xf6,0x20,0x22,0xe5,0xcd,0xc2,0xf5,0x21,0xf0,0xf6,0xcd,0x43,0xf6,0xe1,0x23,
	0x23,0x5e,0x23,0x56,0xeb,0x06,0x00,0x3a,0xf0,0xf6,0xfe,0x0a,0x28,0x05,0xcd,0x5f,
	0xf6,0x18,0x03,0xcd,0x56,0xf6,0xc1,0xe1,0x2b,0xc8,0xcd,0x69,0xf0,0x10,0xb3,0xc9,
	0x7e,0xfe,0x08,0x3f,0xd4,0x0e,0xf5,0xd0,0x21,0x00,0x00,0xf6,0xff,0xc9,0xcd,0xf6,
	0xf6,0xcd,0xaf,0xf5,0xeb,0x7d,0x29,0x29,0x29,0x29,0x11,0x90,0xf3,0x19,0xfe,0x04,
	0xd0,0xe5,0x3e,0x80,0x32,0xf5,0xf6,0x32,0xf1,0xf5,0x0a,0xf6,0x01,0x02,0x3e,0x07,
	0x32,0x3d,0xf5,0xc5,0xcd,0x7a,0xf5,0xc1,0x60,0x69,0x28,0x14,0x3e,0x00,0xd6,0x01,
	0x38,0x31,0x35,0xf2,0x48,0xf5,0x36,0x07,0xcb,0x4e,0x20,0xe4,0xcb,0x96,0x18,0xe0,
	0x57,0x5a,0xcb,0x4e,0x20,0x03,0x11,0x10,0xf4,0xe1,0xe5,0x73,0x23,0x72,0x11,0x09,
	0x00,0x19,0x0a,0xe6,0x03,0xeb,0x6f,0x29,0x29,0x29,0x29,0x01,0x30,0xf4,0x09,0xeb,
	0x73,0x23,0x72,0xe1,0x3e,0x00,0x32,0xf5,0xf6,0xc9,0x0a,0xe6,0x01,0x11,0x01,0x02,
	0x28,0x02,0x16,0x2a,0xcd,0xc2,0xf5,0x21,0xf0,0xf6,0x36,0x08,0xcd,0x43,0xf6,0xcd,
	0xce,0xf6,0xcd,0x87,0xf6,0x20,0x04,0xed,0x78,0x18,0xf7,0xcd,0x69,0xf6,0xc9,0x00,
	0x00,0x20,0x01,0x00,0x06,0x40,0x07,0x60,0x80,0x60,0x80,0x60,0x80,0x60,0x80,0x21,
	0x9f,0xf5,0x16,0x00,0x5f,0x19,0x19,0x7e,0x32,0xf1,0xf6,0x32,0xeb,0xf6,0x23,0x44,
	0x4d,0xc9,0x21,0xe6,0xf4,0x36,0x00,0xeb,0x0a,0xfe,0x80,0x28,0x46,0xfe,0x06,0x38,
	0x06,0x2c,0x7c,0xb7,0x20,0x04,0x2d,0x3e,0x80,0x12,0x0a,0xcb,0x47,0x28,0x0a,0x7c,
	0xfe,0x28,0x38,0x02,0xd6,0x28,0x3f,0x8f,0x67,0xe5,0x0a,0x2a,0xeb,0xf6,0x67,0x11,
	0xff,0xff,0x22,0xf0,0xf5,0xb7,0xed,0x52,0x28,0x0c,0x32,0xef,0xf6,0x21,0xea,0xf6,
	0xcd,0x43,0xf6,0xcd,0x69,0xf6,0xe1,0x44,0x11,0x11,0x00,0x62,0x37,0xed,0x52,0x04,
	0x19,0x10,0xfd,0x7c,0x65,0x6f,0x22,0xf2,0xf6,0xc9,0xcd,0xad,0xf6,0x7e,0xfe,0x01,
	0x3e,0x0a,0x28,0x02,0x3e,0x03,0x32,0x27,0xf6,0xcd,0xd2,0xf6,0x3e,0x01,0xd3,0x10,
	0x3e,0x20,0xd3,0x12,0xdb,0x12,0x0f,0x38,0x06,0xcd,0xa5,0xf6,0xf2,0x34,0xf6,0xaf,
	0xd3,0x12,0xc9,0x7e,0xfe,0x04,0xc8,0xcd,0x1a,0xf6,0x01,0x10,0x06,0xcd,0x87,0xf6,
	0xc8,0xed,0xa3,0x20,0xf8,0xc9,0xcd,0x87,0xf6,0x20,0x0e,0xed,0xb3,0x18,0x0a,0xcd,
	0xce,0xf6,0xcd,0x87,0xf6,0x20,0x02,0xed,0xb2,0xcd,0xce,0xf6,0xcd,0x87,0xf6,0x28,
	0x2a,0xed,0x78,0xe6,0x03,0x47,0xcd,0x87,0xf6,0x28,0x20,0xdb,0x12,0xcb,0x4f,0x28,
	0x1a,0xed,0x78,0x20,0x16,0xb0,0xc9,0xcd,0xa5,0xf6,0xfa,0x9a,0xf6,0xdb,0x12,0xcb,
	0x5f,0x28,0xf4,0xcb,0x77,0x20,0x03,0xe6,0x04,0xc9,0xf1,0xaf,0x32,0xad,0xf6,0xf6,
	0xff,0x32,0xf0,0xf5,0xc9,0xcd,0x66,0xf0,0x3a,0x00,0x00,0xb7,0xc9,0x00,0x3e,0xcf,
	0xd3,0x13,0x3e,0x5f,0xd3,0x13,0x3e,0x80,0xd3,0x12,0xaf,0xd3,0x12,0x3e,0xc9,0x32,
	0xad,0xf6,0xe5,0x21,0xe8,0xf6,0xcd,0x43,0xf6,0xcd,0x69,0xf6,0xe1,0xc9,0x3e,0x4f,
	0x18,0x02,0x3e,0x0f,0xd3,0x11,0xc9,0x0a,0xe6,0x80,0xc8,0x3e,0x00,0xb7,0xc8,0x7a,
	0xb7,0xc8,0x3a,0xf0,0xf6,0xd6,0x08,0xc9,0x01,0x60,0xc0,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x01,0x00,0xf5,0x3e,0xc9,0x32,0xf6,0xf6,0x26,0x00,0xcd,0x39,
	0xf0,0x2b,0x22,0xa9,0xf6,0x22,0x27,0xf6,0x21,0x67,0xf7,0xcd,0xb0,0xf4,0xb7,0xcc,
	0x23,0xf7,0x28,0x0d,0x21,0x6c,0xf7,0x7e,0xc6,0x20,0x77,0x20,0xeb,0x2f,0x32,0xdc,
	0xf6,0xf1,0xc9,0x21,0x00,0xee,0x3a,0x6c,0xf7,0xfe,0x20,0x20,0x04,0x7e,0xfe,0xe5,
	0xc8,0x11,0x0f,0x00,0x06,0x04,0x7e,0xb7,0xc0,0x19,0x7e,0xe6,0x87,0xcb,0x2f,0xcb,
	0x2f,0xc0,0x23,0x10,0xf1,0x2b,0x11,0xaf,0xf4,0x01,0x40,0x00,0xed,0xb8,0x01,0x10,
	0x00,0x11,0xde,0xf3,0x21,0x7d,0xf4,0x3e,0x04,0xf5,0x7e,0x3d,0x87,0x12,0x09,0xeb,
	0x09,0xeb,0xf1,0x3d,0x20,0xf3,0xc8,0x01,0x04,0x00,0x00,0x00,0x20,0x00,0x00,0xee,
	0x8b, // checksum: ddskld's ccs sums the header-declared length to zero
};


uint8_t xerox820ii_state::rgd5_rom_r(offs_t offset)
{
	const uint8_t low = 0xb0 + (offset & 0x0f); // port low byte (mapped 0xB0-0xBF)
	const uint8_t b   = (offset >> 8) & 0xff;   // B register = the index pointer
	if (low == 0xbf)
	{
		switch (b) // ROM header
		{
		case 0xff: return 0x55;  case 0xfe: return 0xaa; // valnum 55AA
		case 0xfc: return 0x04;  case 0xfb: return 0x11; // driver length 0411 (driver + trailing checksum byte)
		case 0xfa: return 0xf3;  case 0xf9: return 0x60; // RAM load address F360
		case 0xf8: return 0xf4;  case 0xf7: return 0x2a; // diagnostic entry F42A (selerr: returns NZ = controller OK)
		default:   return 0xff;
		}
	}
	if (low < 0xbf) // driver body: 256-byte blocks at ports 0xB0+, indexed by B
	{
		const unsigned idx = (low - 0xb0) * 256 + b;
		return idx < sizeof(SDVR_IMAGE) ? SDVR_IMAGE[idx] : 0xff;
	}
	return 0xff;
}

void xerox820ii_state::rx024_select_w(uint8_t data)
{
	// On the 16/8 the system PIO keeps D0-D2 configured as inputs (the
	// monitor's "turn around d0,1,2" reprogram runs only in the 820-II FDC
	// signon path, which the 16/8's RX/ddskld flow never takes), so the PIO
	// cannot drive drive-select or side -- and MAME's z80pio masks those bits
	// out of the port-A callback.  On the real machine the RX024 box's own
	// select latch listens to the I/O bus write directly.  Model the latch:
	// forward the write to the PIO (bank/charset live there), then apply the
	// raw byte to the box's floppy.  The monitor's keyboard-ROM bank toggles
	// (reads back 3F/BF: input bits read as 1s) are filtered by bits 4-5.
	m_kbpio->write_alt(0, data);
	if (m_fdc)
	{
		floppy_image_device *floppy = m_floppy0->get_device();
		m_fdc->set_floppy(floppy);
		if (floppy)
		{
			floppy->mon_w(0);
			if ((data & 0x30) != 0x30)
				floppy->ss_w(BIT(data, 2));
		}
	}
}


void xerox820ii_state::xerox1685_io(address_map &map)
{
	xerox820ii_io(map); // the FD1797 floppy controller + the 8086 A0/A1 ports, unchanged
	map(0xa6, 0xa6).mirror(0xff00).lr8(NAME([]() -> u8 { return 0x20; })); // expbx id = flpy5 (5.25" floppy box)
	map(0xb0, 0xbf).select(0xff00).r(FUNC(xerox820ii_state::rx024_rom_r)); // the reconstructed RX024 controller ROM
	map(0x1c, 0x1c).mirror(0xff00).w(FUNC(xerox820ii_state::rx024_select_w)); // the box latch sees the raw bus write
}

void xerox820ii_state::xerox168_mem(address_map &map)
{
	// Per the 16/8 Technical Reference: the 8086 board has 128K RAM, expandable to
	// 256K with a daughter board.  Model the 256K maximum.
	map(0x00000, 0x3ffff).ram();                 // 256K TPA / program RAM (128K base + 128K daughter)
	map(0xf0000, 0xfefff).ram();                 // resident OS + shared-mailbox RAM (8086 runs CS=0xF400); the Z80 windows 0xF4000+
	map(0xff000, 0xfffff).rom().region(I8086_TAG, 0); // 8086 boot ROM (signature 0x0909 at 0xFFFFC; LOAD86 reads it at Z80 window 0xBFFC)
}

// MK-83: 256K DRAM = fixed top 32K + a 0x0000-0x7FFF window holding one of
// eight 32K pages, selected by the 3-bit bank code {PA7,PA6,P14.5} (see the
// mk83_state comment in xerox820.h).  Bank code 6 overlays the 4K monitor
// EPROM at 0x0000 and the video RAM at 0x7000 (the trampoline at 0xFB25
// switches to code 6 around every call into the EPROM-resident routines);
// bank code 7 decodes to the same DRAM half-row as the fixed top page.
void mk83_state::mk83_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).view(m_view);
	m_view[0](0x0000, 0x7fff).ram();
	m_view[1](0x0000, 0x7fff).ram();
	m_view[2](0x0000, 0x7fff).ram();
	m_view[3](0x0000, 0x7fff).ram();
	m_view[4](0x0000, 0x7fff).ram();
	m_view[5](0x0000, 0x7fff).ram();
	m_view[6](0x0000, 0x7fff).ram();
	m_view[6](0x0000, 0x0fff).rom().region(Z80_TAG, 0);
	m_view[6](0x7000, 0x7fff).ram().share("video_ram");
	m_view[7](0x0000, 0x7fff).ram().share("mainram");
	map(0x8000, 0xffff).ram().share("mainram");
}

// I/O decode per the U86 74LS138 (schematic sheet 2): 00 BAUDA, 04 SIO,
// 08 GP PIO, 0C BAUDB (the SIO-B console baud), 10 FD1797, 14 SCROLL/bank
// latch, 18 CTC, 1C KBD PIO.  Same layout as the Xerox 820 except that the
// scroll latch takes the DATA byte (plus bank bit B2) rather than sampling
// the address lines.
void mk83_state::mk83_io(address_map &map)
{
	map(0x00, 0x00).mirror(0xff03).w(COM8116_TAG, FUNC(com8116_device::str_w));
	map(0x04, 0x07).mirror(0xff00).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x08, 0x0b).mirror(0xff00).rw(Z80PIO_GP_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x0c, 0x0c).mirror(0xff03).w(COM8116_TAG, FUNC(com8116_device::stt_w));
	map(0x10, 0x13).mirror(0xff00).rw(FUNC(mk83_state::fdc_r), FUNC(mk83_state::fdc_w));
	map(0x14, 0x14).mirror(0xff03).w(FUNC(mk83_state::scroll_bank_w));
	map(0x18, 0x1b).mirror(0xff00).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x1c, 0x1f).mirror(0xff00).rw(m_kbpio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}

void mk83_state::update_bank()
{
	m_view.select((m_bank_hi << 1) | m_bank_lo);
}

void mk83_state::scroll_bank_w(uint8_t data)
{
	// 0x14-0x17 write latch: bits 0-4 = CRT scroll row, bit 5 = bank bit B2.
	// The monitor also mirrors the bank code into bits 6-7 here, but on the
	// hardware those bank bits come from the PIO-A pins (bits 6-7 of this
	// latch are unconnected), which is what makes the power-on state safe:
	// the EPROM's first OUT (14h) is the video-clear's scroll value 0x17 with
	// bank bits 0, and only the PA6/PA7 pull-ups keep the EPROM mapped.
	m_scroll = data & 0x1f;
	m_bank_lo = BIT(data, 5);
	update_bank();
}

uint8_t mk83_state::mk83_kbpio_pa_r()
{
	/*

	    bit     signal      description

	    3       PBRDY       keyboard PIO-B ready
	    6       B0          bank (pull-up; reads 1 while programmed as input)
	    7       B1          bank (pull-up; reads 1 while programmed as input)

	*/
	return 0xc0 | (m_kbpio->rdy_b() << 3);
}

void mk83_state::mk83_kbpio_pa_w(uint8_t data)
{
	/*

	    bit     signal      description

	    0       DVSEL0      drive select unit # bit 0 (binary, decoded to DVSEL1-3 + J7)
	    1       DVSEL1      drive select unit # bit 1
	    2       MOTOR       1 = drive motor off (the 8 Hz tick ISR sets it after ~3 ticks)
	    3       PBRDY       (input)
	    4       SIZE        drive size per the FF6B geometry config (1 = 8")  [TODO: verify against media]
	    5       DENS        density per the geometry config (1 = MFM)         [TODO: verify against media]
	    6       B0          bank code bit 1
	    7       B1          bank code bit 2

	    MAME's z80pio passes mode-3 input bits to this callback as 1, which
	    models the PA6/PA7 pull-ups: from power-on until the monitor's F7F7
	    re-programs the I/O mask (CF 08), bits 6-7 are inputs and the bank
	    code stays {1,1,B2} = EPROM mapped, regardless of the values the init
	    code writes through here.

	*/
	floppy_image_device *floppy = nullptr;

	switch (data & 0x03)
	{
	case 0: floppy = m_floppy0->get_device(); break;
	case 1: floppy = m_floppy1->get_device(); break;
	}

	m_fdc->set_floppy(floppy);

	if (floppy)
		floppy->mon_w(BIT(data, 2));

	// bits 4-5 feed the FDC9229BT data-separator config; model size as the
	// FD1797 clock and density as DDEN until real media pins them down
	m_fdc->set_unscaled_clock(BIT(data, 4) ? 16_MHz_XTAL / 8 : 16_MHz_XTAL / 16);
	m_fdc->dden_w(!BIT(data, 5));

	m_bank_hi = data >> 6;
	update_bank();
}

uint8_t mk83_state::mk83_kbpio_pb_r()
{
	return m_kbdata ^ 0xff;     // ASCII keyboard, inverted (the ISR at 03B0 CPLs it back)
}

void mk83_state::kb_put(u8 data)
{
	// latch the byte and pulse the PIO-B strobe: the falling edge samples
	// pb_r, the rising edge raises the PIO interrupt (vector 0x1A -> the
	// type-ahead FIFO at FF30)
	m_kbdata = data;
	m_kbpio->strobe_b(0);
	m_kbpio->strobe_b(1);
}

// FD1797 DRQ drives /NMI directly (no /HALT gate on this board: the monitor
// installs an EX AF/EXX/INI/EXX/EX AF/RETN handler at 0x0066 and polls the
// busy bit while DRQ-NMIs move the bytes).  INTRQ is not wired to /NMI here:
// the command end is detected by the busy poll, and an INTRQ-NMI would inject
// a spurious INI after the last byte.
void mk83_state::mk83_fdc_drq_w(int state)
{
	m_fdc_drq = bool(state);
	m_maincpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}


/* Input Ports */

static INPUT_PORTS_START( xerox820 )
	// inputs defined in machine/keyboard.c
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(bigboard_state::beep_timer)
{
	m_beeper->set_state(0);
}

/* Z80 PIO */

uint8_t xerox820_state::kbpio_pa_r()
{
	/*

	    bit     signal          description

	    0
	    1
	    2
	    3       PBRDY           keyboard data available
	    4       8/N5            8"/5.25" disk select (0=5.25", 1=8")
	    5       400/460         double sided disk detect (only on Etch 2 PCB) (0=SS, 1=DS)
	    6
	    7

	*/

	uint8_t data = 0;

	// keyboard
	data |= m_kbpio->rdy_b() << 3;

	// floppy
	data |= m_8n5 << 4;
	data |= m_400_460 << 5;

	return data;
}

uint8_t xerox820ii_state::kbpio_pa_r()
{
	// The 820-II disk daughterboard reports its personality on PA bits 0-1, which the
	// ROSR 'signon' routine reads (port 0x1C):
	//   bit 0 = 1: main-board FD1797 floppy controller (WD1797 driver path)
	//           0: Shugart SASI host adapter (installs the SA1403D driver)
	//   bit 1 = A/E swap gate (signon FC95 `and 2; jr nz`): 1 = no swap, so drive A is
	//           the floppy (boot floppy-first); 0 = swap A<->E (A is the rigid disk).
	// A SASI machine boots the floppy first, so it reports no swap.
	uint8_t data = xerox820_state::kbpio_pa_r();
	data |= (m_sasi_board ? 0 : 1) << 0;
	data |= (m_sasi_board ? 1 : 0) << 1;
	return data;
}

void xerox820_state::kbpio_pa_w(uint8_t data)
{
	/*

	    bit     signal          description

	    0       _DVSEL1         drive select 1
	    1       _DVSEL2         drive select 2
	    2       SIDE            side select
	    3
	    4
	    5
	    6       NCSET2          display character set (inverted and connected to chargen A10)
	    7       BANK            bank switching (0=RAM, 1=ROM/videoram)

	*/

	/* drive select (no main-board floppy path on the SASI personality: the
	   FD1797 add-in board is replaced by the SASI host adapter) */
	if (m_fdc)
	{
		floppy_image_device *floppy = nullptr;

		if (m_drvsel_binary)
		{
			// Big Board: the low 2 bits are a binary drive-unit number (drive 0 = 00),
			// demuxed to one select line.  The Xerox 820 instead uses one bit per drive.
			switch (data & 0x03)
			{
			case 0: floppy = m_floppy0->get_device(); break;
			case 1: floppy = m_floppy1->get_device(); break;
			}
		}
		else
		{
			if (BIT(data, 0)) floppy = m_floppy0->get_device();
			if (BIT(data, 1)) floppy = m_floppy1->get_device();
		}

		// x1685: the 5.25" drive lives in the RX024 expansion box and is permanently
		// cabled to the FD1797; the box's own controller routes the drive, so the
		// system-PIO drive-select bits 0-1 do NOT mux it (the v5.0 monitor in fact
		// drives those bits with unrelated traffic - e.g. its keyboard-ROM banking
		// writes 0x3F/0xBF, whose bits 0-1=11 would otherwise select the empty drive
		// 1).  And the loadable WDVR driver samples READY (in wdsr; and 80h) *before*
		// it issues its own drive-select, so the drive must already be attached and
		// spinning or the FD1797 reads not-ready (status 0x84) and the driver aborts
		// with "A:Load error".  So pin the FDC to floppy0 (motor kept on below).
		if (m_fdc_single_floppy)
			floppy = m_floppy0->get_device();

		m_fdc->set_floppy(floppy);

		if (floppy)
		{
			int _8n5 = (floppy->get_form_factor() == floppy_image::FF_8);

			if (m_8n5 != _8n5)
			{
				m_8n5 = _8n5;

				m_fdc->set_unscaled_clock(m_8n5 ? 20_MHz_XTAL / 10 : 20_MHz_XTAL / 20);
			}

			m_400_460 = !floppy->twosid_r();

			floppy->mon_w(0);

			// On x1685 the WDVR driver DOES select the side via bit 2 (0x09 = side 0 for
			// reads, 0x0D = side 1 for its single/double-sided probe in smf).  But the
			// v5.0 monitor's keyboard-ROM banking also writes this port with garbage in
			// the low bits (0x3F/0xBF, bits 4 & 5 both set) that would spuriously flip the
			// head to the empty side 1 of a single-sided disk.  Honor the driver's side
			// select and ignore the banking writes (bits 4 & 5 set); the floppy keeps its
			// last driver-set side across the banking traffic.  Pinning side 0 outright
			// (an earlier fix) broke smf's side-1 probe, so it mis-detected two-sided and
			// looped loading CCP+BDOS.  Other machines select the side normally.
			if (m_fdc_single_floppy)
			{
				if ((data & 0x30) != 0x30)
					floppy->ss_w(BIT(data, 2));
			}
			else
				floppy->ss_w(BIT(data, 2));
		}
	}

	/* display character set */
	m_ncset2 = !BIT(data, 6);

	/* bank switching */
	m_view.select(BIT(data, 7));
}

void bigboard_state::kbpio_pa_w(uint8_t data)
{
	xerox820_state::kbpio_pa_w(data);

	/* beeper on bigboard */
	if (BIT(data, 5) & (!m_bit5))
	{
		m_beep_timer->adjust(attotime::from_msec(40));
		m_beeper->set_state(1);
	}
	m_bit5 = BIT(data, 5);
}

uint8_t xerox820_state::kbpio_pb_r()
{
	/*

	    bit     description

	    0       KB0
	    1       KB1
	    2       KB2
	    3       KB3
	    4       KB4
	    5       KB5
	    6       KB6
	    7       KB7

	*/

	return (m_lpk ? m_lpk->read() : m_kb->read()) ^ 0xff;
}

// ============================================================================
//  x820_sasi_host_device - the 820-II SASI host adapter (9R80758 daughterboard)
//
//  Line-level bridge between the "u8" Z80PIO and the nscsi bus.  Port A is
//  the SASI data bus; PARDY clocks the U11 74LS74 on either a port-A read or
//  write, generating the per-byte ACK pulse (modelled as a timed ACK inside
//  data_r/data_w, the bigbord2 host pattern).  Port B carries the control
//  lines, non-inverted, in the layout the boot ROM polls.
// ============================================================================

DEFINE_DEVICE_TYPE(X820_SASI_HOST, x820_sasi_host_device, "x820_sasi_host", "Xerox 820-II SASI host adapter")

x820_sasi_host_device::x820_sasi_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, X820_SASI_HOST, tag, owner, clock)
	, nscsi_device_interface(mconfig, *this)
{
}

void x820_sasi_host_device::device_start()
{
	m_ack_timer = timer_alloc(FUNC(x820_sasi_host_device::ack_off), this);
}

void x820_sasi_host_device::device_reset()
{
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, nscsi_device_interface::S_ALL);
	m_scsi_bus->data_w(m_scsi_refid, 0);
	// register for target-driven line changes so scsi_ctrl_changed() fires
	// (the bus only dispatches to devices whose wait mask overlaps)
	constexpr u32 target_mask =
		nscsi_device_interface::S_BSY |
		nscsi_device_interface::S_REQ |
		nscsi_device_interface::S_MSG |
		nscsi_device_interface::S_CTL |
		nscsi_device_interface::S_INP;
	m_scsi_bus->ctrl_wait(m_scsi_refid, target_mask, target_mask);
}

void x820_sasi_host_device::scsi_ctrl_changed()
{
	// the boot ROM polls port B; no edge-triggered host logic to drive
}

TIMER_CALLBACK_MEMBER(x820_sasi_host_device::ack_off)
{
	// drop ACK and release the data bus (essential on target-driven phases:
	// a stale host byte would OR with the target's data)
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, nscsi_device_interface::S_ACK);
	m_scsi_bus->data_w(m_scsi_refid, 0);
}

uint8_t x820_sasi_host_device::data_r()
{
	const uint8_t v = m_scsi_bus->data_r();
	// PARDY pulses U11 on a port-A read too: ACK each byte taken during a
	// connected transfer phase (never during selection)
	if (!machine().side_effects_disabled() && (m_scsi_bus->ctrl_r() & S_BSY))
	{
		m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
		m_ack_timer->adjust(SASI_PULSE);
	}
	return v;
}

void x820_sasi_host_device::data_w(uint8_t data)
{
	m_scsi_bus->data_w(m_scsi_refid, data);
	if (m_scsi_bus->ctrl_r() & S_BSY)
	{
		m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
		m_ack_timer->adjust(SASI_PULSE);
	}
}

uint8_t x820_sasi_host_device::ctrl_r()
{
	/*

	    bit     description

	    0       NBSY
	    1       NMSG
	    2       NC/D
	    3       NREQ
	    4       NI/O
	    5
	    6       LS74 Q (U11; 0 = no parity error, the boot's f693 gate)
	    7

	*/

	const u32 ctrl = m_scsi_bus->ctrl_r();
	uint8_t data = 0;
	if (ctrl & S_BSY) data |= 0x01;
	if (ctrl & S_MSG) data |= 0x02;
	if (ctrl & S_CTL) data |= 0x04;
	if (ctrl & S_REQ) data |= 0x08;
	if (ctrl & S_INP) data |= 0x10;
	return data;
}

void x820_sasi_host_device::ctrl_w(uint8_t data)
{
	/*

	    bit     description

	    0-4
	    5       NSEL
	    7       NRST (init writes 0x80 then 0x00 = RST pulse)

	*/

	m_scsi_bus->ctrl_w(m_scsi_refid,
			(BIT(data, 5) ? S_SEL : 0) | (BIT(data, 7) ? S_RST : 0),
			S_SEL | S_RST);
	// release the data bus when SEL drops so the target can drive COMMAND
	if (!BIT(data, 5))
		m_scsi_bus->data_w(m_scsi_refid, 0);
}

/* Z80 CTC */

TIMER_DEVICE_CALLBACK_MEMBER( xerox820_state::ctc_tick )
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);
}

/* Z80 Daisy Chain */

static const z80_daisy_config xerox820_daisy_chain[] =
{
	{ Z80SIO_TAG },
	{ Z80PIO_KB_TAG },
	{ Z80PIO_GP_TAG },
	{ Z80CTC_TAG },
	{ nullptr }
};

// The 820-II adds the SASI "read" PIO (u8), which also drives IRQ0; it must be
// in the IM2 daisy chain or its interrupts mis-vector and starve the keyboard PIO.
// Floppy daughterboard: the FD1797 interrupts via /NMI, so no disk device joins the
// Z80 interrupt daisy chain.
static const z80_daisy_config xerox820ii_daisy_chain[] =
{
	{ Z80SIO_TAG },
	{ Z80PIO_KB_TAG },
	{ Z80PIO_GP_TAG },
	{ Z80CTC_TAG },
	{ nullptr }
};

// SASI host-adapter daughterboard: the "u8" Z80PIO (pio_rd) sits in the daisy chain.
static const z80_daisy_config xerox820iis_daisy_chain[] =
{
	{ Z80SIO_TAG },
	{ Z80PIO_KB_TAG },
	{ Z80PIO_GP_TAG },
	{ Z80PIO_RD_TAG },
	{ Z80CTC_TAG },
	{ nullptr }
};



/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER(xerox820_state::quickload_cb)
{
	m_view.select(0);

	address_space &prog_space = m_maincpu->space(AS_PROGRAM);

	// Avoid loading a program if CP/M-80 is not in memory
	if ((prog_space.read_byte(0) != 0xc3) || (prog_space.read_byte(5) != 0xc3))
	{
		machine_reset();
		return std::make_pair(image_error::UNSUPPORTED, "CP/M must already be running");
	}

	const int mem_avail = 256 * prog_space.read_byte(7) + prog_space.read_byte(6) - 512;
	if (mem_avail < image.length())
		return std::make_pair(image_error::UNSPECIFIED, "Insufficient memory available");

	// Load image to the TPA (Transient Program Area)
	uint16_t quickload_size = image.length();
	for (uint16_t i = 0; i < quickload_size; i++)
	{
		uint8_t data;
		if (image.fread(&data, 1) != 1)
			return std::make_pair(image_error::UNSPECIFIED, "Problem reading the image at offset " + std::to_string(i));
		prog_space.write_byte(i + 0x100, data);
	}

	// clear out command tail
	prog_space.write_byte(0x80, 0);
	prog_space.write_byte(0x81, 0);

	// Roughly set SP basing on the BDOS position
	m_maincpu->set_state_int(Z80_SP, mem_avail + 384);
	m_maincpu->set_pc(0x100); // start program

	return std::make_pair(std::error_condition(), std::string());
}



/* WD1771 Interface */

static void xerox820_floppies(device_slot_interface &device)
{
	device.option_add("sa400", FLOPPY_525_SSSD_35T); // Shugart SA-400, 35 trk drive
	device.option_add("sa400l", FLOPPY_525_SSSD); // Shugart SA-400, 40 trk drive
	device.option_add("sa450", FLOPPY_525_DD); // Shugart SA-450
	device.option_add("sa800", FLOPPY_8_SSDD); // Shugart SA-800
	device.option_add("sa850", FLOPPY_8_DSDD); // Shugart SA-850
}

// Big Board 8" CP/M disks ship as IMD images.
static void bigboard_floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_IMD_FORMAT);
}

// FDC INTRQ and DRQ are OR'd and gated by the Z80 /HALT line to drive /NMI.
// The BIOS issues an FDC command, executes HALT, and each DRQ (and the final
// INTRQ) fires an NMI that moves one byte / completes the command.  The gate
// must be evaluated on BOTH of its inputs -- the FDC DRQ/INTRQ edges and the
// Z80 /HALT edge -- otherwise the case where the next DRQ asserts just before
// the CPU reaches HALT is lost and the CPU hangs in HALT forever.  m_cpu_halted
// is tracked via the Z80 halt_cb so /NMI tracks (/HALT & (DRQ|INTRQ)).
void xerox820_state::update_nmi()
{
	int state = (m_cpu_halted && (m_fdc_irq || m_fdc_drq)) ? ASSERT_LINE : CLEAR_LINE;

	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

void xerox820_state::cpu_halt_w(int state)
{
	m_cpu_halted = bool(state);

	update_nmi();
}

void xerox820_state::fdc_intrq_w(int state)
{
	m_fdc_irq = state;

	update_nmi();
}

void xerox820_state::fdc_drq_w(int state)
{
	m_fdc_drq = state;

	update_nmi();
}

/* Video */

uint32_t xerox820_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=(m_scroll + 1) * 0x80;
	pen_t const *const pen=m_palette->pens();

	m_framecnt++;

	for (uint8_t y = 0; y < 24; y++)
	{
		if (ma > 0xb80) ma = 0;

		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint32_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 80; x++)
			{
				uint8_t gfx;
				if (ra < 8)
				{
					uint8_t chr = m_video_ram[x & XEROX820_VIDEORAM_MASK] ^ 0x80;

					/* Take care of flashing characters */
					if ((chr < 0x80) && (m_framecnt & 0x08))
						chr |= 0x80;

					/* get pattern of pixels for that character scanline */
					gfx = m_char_rom->base()[(m_ncset2 << 10) | (chr<<3) | ra ];
				}
				else
					gfx = 0xff;

				/* Display a scanline of a character (7 pixels) */
				*p++ = pen[0];
				*p++ = pen[BIT(gfx, 4) ^ 1];
				*p++ = pen[BIT(gfx, 3) ^ 1];
				*p++ = pen[BIT(gfx, 2) ^ 1];
				*p++ = pen[BIT(gfx, 1) ^ 1];
				*p++ = pen[BIT(gfx, 0) ^ 1];
				*p++ = pen[0];
			}
		}
		ma+=128;
	}
	return 0;
}

/* Machine Initialization */

void xerox820_state::machine_start()
{
	// state saving
	save_item(NAME(m_scroll));
	save_item(NAME(m_ncset2));
	save_item(NAME(m_vatt));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_8n5));
	save_item(NAME(m_400_460));
	save_item(NAME(m_cpu_halted));

	m_ncset2 = 0;
}

void xerox820_state::machine_reset()
{
	m_view.select(1);

	m_fdc->reset();
}

void bigboard_state::machine_reset()
{
	m_view.select(1);

	/* bigboard has a one-pulse output to drive a user-supplied beeper */
	m_beeper->set_state(0);

	m_fdc->reset();

	// The Big Board's monitor selects drives by a binary unit number in the low
	// bits of the system PIO port (drive 0 = 0), not the Xerox 820's bit-per-drive.
	m_drvsel_binary = true;

	// 8" Shugart drives spin continuously (no motor-control line); force the
	// motor on so the WD1771 sees READY for the boot ROM's RESTORE.
	if (floppy_image_device *fd = m_floppy0->get_device()) fd->mon_w(0);
	if (floppy_image_device *fd = m_floppy1->get_device()) fd->mon_w(0);
}

void xerox820ii_state::machine_start()
{
	xerox820_state::machine_start();

	save_item(NAME(m_dbell_count));
	save_item(NAME(m_rmw_addr));
	save_item(NAME(m_rmw_val));
	save_item(NAME(m_rmw_cycles));
}

void xerox820ii_state::machine_reset()
{
	m_view.select(1);

	// The 16/8 add-in 8086 is held in RESET until the Z80 releases it (IN A1 = Start).
	// IN A0 (Stop) re-asserts reset.  Because each Start runs the 8086 fresh from 0xFFFF0
	// (a full POST), LOAD86 can clobber the shared RAM with its comm-test + stage commands
	// while the 8086 is held, then Start it to re-init its dispatcher TCBs cleanly.
	if (m_i8086)
	{
		m_i8086->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_i8086->set_input_line(0, CLEAR_LINE); // empty the doorbell latch
	}
	m_dbell_count = 0;
	m_rmw_addr = ~offs_t(0); // clear any shared-RAM RMW reservation

	if (m_fdc)
		m_fdc->reset();

	// The FD1797 data rate must match the installed drive size, but kbpio_pa_w only
	// updates the clock when m_8n5 *changes* (and m_8n5 starts at 0 = 5.25", so a 5.25"
	// drive never triggers it) -> the controller would stay at the 8" rate and a 5.25"
	// RESTORE/read would seek-error.  Prime the rate + m_8n5 from the connected drive's
	// form factor here (8" = 2 MHz, 5.25" = 1 MHz).
	m_fdc_xor = 0x00; // 820-II FD1797 data bus is NOT inverted (unlike the first-gen FD1771)
	if (floppy_image_device *fd = m_floppy0 ? m_floppy0->get_device() : nullptr)
	{
		bool const eight = (fd->get_form_factor() == floppy_image::FF_8);
		m_8n5 = eight ? 1 : 0;
		m_fdc->set_unscaled_clock(eight ? 16_MHz_XTAL / 8 : 16_MHz_XTAL / 16);
		fd->mon_w(0); // floppy spindle motor on (drive ready for the boot's RESTORE)
	}

	m_sio->synca_w(1);
	m_sio->syncb_w(1);
}


/* F4 Character Displayer */
static const gfx_layout xerox820_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*8                 /* every char takes 8 bytes */
};

static const gfx_layout xerox820_gfxlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_xerox820 )
	GFXDECODE_ENTRY( "chargen", 0x0000, xerox820_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_xerox820ii )
	GFXDECODE_ENTRY( "chargen", 0x0000, xerox820_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "chargen", 0x0800, xerox820_gfxlayout, 0, 1 )
GFXDECODE_END

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_ODD )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

/* Machine Drivers */

void xerox820_state::xerox820(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 20_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &xerox820_state::xerox820_mem);
	m_maincpu->set_addrmap(AS_IO, &xerox820_state::xerox820_io);
	m_maincpu->set_daisy_config(xerox820_daisy_chain);

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(xerox820_state::screen_update));
	screen.set_raw(10.69425_MHz_XTAL, 700, 0, 560, 260, 0, 240);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_xerox820);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* devices */
	Z80PIO(config, m_kbpio, 20_MHz_XTAL / 8);
	m_kbpio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_kbpio->in_pa_callback().set(FUNC(xerox820_state::kbpio_pa_r));
	m_kbpio->out_pa_callback().set(FUNC(xerox820_state::kbpio_pa_w));
	m_kbpio->in_pb_callback().set(FUNC(xerox820_state::kbpio_pb_r));

	z80pio_device& pio_gp(Z80PIO(config, Z80PIO_GP_TAG, 20_MHz_XTAL / 8));
	pio_gp.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc, 20_MHz_XTAL / 8);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));
	//TIMER(config, "ctc").configure_periodic(FUNC(xerox820_state::ctc_tick), attotime::from_hz(20_MHz_XTAL / 8));

	m_maincpu->halt_cb().set(FUNC(xerox820_state::cpu_halt_w)); // FDC DRQ/INTRQ -> /NMI is /HALT-gated

	FD1771(config, m_fdc, 20_MHz_XTAL / 20);
	m_fdc->intrq_wr_callback().set(FUNC(xerox820_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(xerox820_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, FD1771_TAG":0", xerox820_floppies, "sa400l", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1771_TAG":1", xerox820_floppies, "sa400l", floppy_image_device::default_mfm_floppy_formats);

	Z80SIO(config, m_sio, 20_MHz_XTAL / 8); // MK3884 (SIO/0)
	m_sio->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.rxd_handler().append(m_sio, FUNC(z80sio_device::synca_w));
	rs232a.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	rs232a.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	rs232b.rxd_handler().append(m_sio, FUNC(z80sio_device::syncb_w));
	rs232b.cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));
	rs232b.dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));
	rs232b.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	com8116_device &dbrg(COM8116(config, COM8116_TAG, 5.0688_MHz_XTAL));
	dbrg.fr_handler().set(m_sio, FUNC(z80sio_device::rxca_w));
	dbrg.fr_handler().append(m_sio, FUNC(z80sio_device::txca_w));
	dbrg.ft_handler().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	XEROX_820_KEYBOARD(config, m_kb);
	m_kb->kbstb_wr_callback().set(m_kbpio, FUNC(z80pio_device::strobe_b));

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("xerox820");
	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(xerox820_state::quickload_cb));
}

void bigboard_state::bigboard(machine_config &config)
{
	xerox820(config);

	// The Big Board uses 8" SSSD drives (Shugart SA-800) read by the WD1771 in FM
	// at the 8" 2 MHz clock, not the Xerox 820's 5.25" SA-400 at 1 MHz; its disks
	// ship as IMD.
	m_fdc->set_clock(20_MHz_XTAL / 10);
	FLOPPY_CONNECTOR(config.replace(), FD1771_TAG":0", xerox820_floppies, "sa800", bigboard_floppy_formats);
	FLOPPY_CONNECTOR(config.replace(), FD1771_TAG":1", xerox820_floppies, "sa800", bigboard_floppy_formats);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 950).add_route(ALL_OUTPUTS, "mono", 1.00); /* bigboard only */
	TIMER(config, m_beep_timer).configure_generic(FUNC(bigboard_state::beep_timer));
}

void bigboard_state::bigboard5(machine_config &config)
{
	xerox820(config);

	// A common field modification ran the Big Board from 5.25" SA-400 drives in
	// place of the stock 8" SA-800.  The WD1771 (FM/single density) stays at its
	// 5.25" 1 MHz clock (the Xerox 820 default), and the monitor auto-selects the
	// 5.25" disk parameters from the drive's form factor (kbpio_pa_r bit 4).
	FLOPPY_CONNECTOR(config.replace(), FD1771_TAG":0", xerox820_floppies, "sa400l", bigboard_floppy_formats);
	FLOPPY_CONNECTOR(config.replace(), FD1771_TAG":1", xerox820_floppies, "sa400l", bigboard_floppy_formats);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 950).add_route(ALL_OUTPUTS, "mono", 1.00); /* bigboard only */
	TIMER(config, m_beep_timer).configure_generic(FUNC(bigboard_state::beep_timer));
}

// Shared 820-II hardware, independent of the disk daughterboard.  The disk drives and
// the SASI host adapter are added by the per-personality configs below.
void xerox820ii_state::xerox820ii_common(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &xerox820ii_state::xerox820ii_mem);
	m_maincpu->set_addrmap(AS_IO, &xerox820ii_state::xerox820ii_io); // FD1797 floppy map; the SASI personality overrides it
	m_maincpu->set_daisy_config(xerox820ii_daisy_chain);             // floppy chain; the SASI personality overrides it

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(xerox820ii_state::screen_update));
	screen.set_raw(10.69425_MHz_XTAL, 700, 0, 560, 260, 0, 240);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_xerox820ii);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	Z80PIO(config, m_kbpio, 16_MHz_XTAL / 4);
	m_kbpio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_kbpio->in_pa_callback().set(FUNC(xerox820ii_state::kbpio_pa_r)); // adds the disk-board personality bits
	m_kbpio->out_pa_callback().set(FUNC(xerox820_state::kbpio_pa_w));
	m_kbpio->in_pb_callback().set(FUNC(xerox820_state::kbpio_pb_r));

	z80pio_device& pio_gp(Z80PIO(config, Z80PIO_GP_TAG, 16_MHz_XTAL / 4));
	pio_gp.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc, 16_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));
	//TIMER(config, "ctc").configure_periodic(FUNC(xerox820_state::ctc_tick), attotime::from_hz(16_MHz_XTAL / 4));

	m_maincpu->halt_cb().set(FUNC(xerox820_state::cpu_halt_w)); // FDC DRQ/INTRQ -> /NMI is /HALT-gated

	// the FD1797 floppy add-in board is added by the floppy personalities;
	// the SASI personality replaces it with the SASI host adapter

	Z80SIO(config, m_sio, 16_MHz_XTAL / 4); // MK3884 (SIO/0)
	m_sio->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	rs232a.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	rs232b.cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));
	rs232b.dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));

	com8116_device &dbrg(COM8116(config, COM8116_TAG, 5.0688_MHz_XTAL));
	dbrg.fr_handler().set(m_sio, FUNC(z80sio_device::rxca_w));
	dbrg.fr_handler().append(m_sio, FUNC(z80sio_device::txca_w));
	dbrg.ft_handler().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	XEROX_820_KEYBOARD(config, m_kb);
	m_kb->set_xerox820ii(true); // 820-II keyboard announces "available" (high-bit first byte sets monitor $F977)
	m_kb->kbstb_wr_callback().set(m_kbpio, FUNC(z80pio_device::strobe_b));

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("xerox820ii");
	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(xerox820_state::quickload_cb));
}

// Shugart SASI host-adapter daughterboard: the "u8" Z80PIO (pio_rd) talks to an SA1403D
// that carries the 8" floppies (LUN 0-2) and an ST-506 rigid disk (LUN 3) over SASI.
namespace {

// The 16/8 5.25" rigid disk unit's controller: the same SA1403D with the
// 5.25" drive complement (SA450 floppies on LUN 0-2; the advertised 10 MB
// rigid -- exact drive model undetermined -- on LUN 3, presenting the
// SA1004-compatible logical space the sdvr driver addresses).
class sa1403d_rgd5_device : public nscsi_sa1403d_device
{
public:
	sa1403d_rgd5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DEFINE_DEVICE_TYPE_PRIVATE(SA1403D_RGD5, nscsi_sa1403d_device, sa1403d_rgd5_device, "sa1403d_rgd5", "Shugart SA1403D (Xerox 5.25\" rigid disk unit)")

sa1403d_rgd5_device::sa1403d_rgd5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nscsi_sa1403d_device(mconfig, SA1403D_RGD5, tag, owner, clock)
{
	set_drive_type(0, SA450);
	set_drive_type(1, SA450);
	set_drive_type(2, SA450);   // D: -- connector defaults empty (two-drive units)
	set_drive_type(3, RIGID5);
}

} // anonymous namespace

void xerox820ii_state::xerox820ii_sasi(machine_config &config, bool rgd5)
{
	m_sasi_board = 1;
	m_maincpu->set_addrmap(AS_IO, &xerox820ii_state::xerox820iis_io); // pio_rd at 0x10-0x13
	m_maincpu->set_daisy_config(xerox820iis_daisy_chain);             // u8 joins the daisy chain

	z80pio_device& pio_rd(Z80PIO(config, Z80PIO_RD_TAG, 20_MHz_XTAL / 8));
	pio_rd.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio_rd.in_pa_callback().set(m_sasi_host, FUNC(x820_sasi_host_device::data_r));
	pio_rd.out_pa_callback().set(m_sasi_host, FUNC(x820_sasi_host_device::data_w));
	pio_rd.in_pb_callback().set(m_sasi_host, FUNC(x820_sasi_host_device::ctrl_r));
	pio_rd.out_pb_callback().set(m_sasi_host, FUNC(x820_sasi_host_device::ctrl_w));

	// SASI bus: SA1403D controller at target id 0 (SEL with DB0 = controller
	// id), host adapter bridged through the u8 PIO.  Floppies on LUN 0-2 and
	// the rigid disk (SA1004) on LUN 3, per the 9R80758 unit wiring.
	NSCSI_BUS(config, m_sasibus);
	if (rgd5)
		NSCSI_CONNECTOR(config, SASIBUS_TAG ":0",
				[](device_slot_interface &d) { d.option_add("sa1403d_rgd5", SA1403D_RGD5); }, "sa1403d_rgd5");
	else
		NSCSI_CONNECTOR(config, SASIBUS_TAG ":0", default_scsi_devices, "sa1403d");
	X820_SASI_HOST(config, m_sasi_host);
	m_sasibus->set_external_device(7, m_sasi_host);
}

// 820-II, main-board FD1797 floppy controller, 8" drives.
void xerox820ii_state::xerox820ii(machine_config &config)
{
	xerox820ii_common(config);
	FD1797(config, m_fdc, 16_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(FUNC(xerox820_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(xerox820_state::fdc_drq_w));
	m_fdc->sso_wr_callback().set_nop(); // SSO pin unconnected; the side select is PIO-driven (system port bit 2)
	FLOPPY_CONNECTOR(config, FD1797_TAG":0", xerox820_floppies, "sa850", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1797_TAG":1", xerox820_floppies, "sa850", floppy_image_device::default_mfm_floppy_formats);
}

// 820-II, main-board FD1797 floppy controller, 5.25" drives.
void xerox820ii_state::xerox820ii5(machine_config &config)
{
	xerox820ii_common(config);
	FD1797(config, m_fdc, 16_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(FUNC(xerox820_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(xerox820_state::fdc_drq_w));
	m_fdc->sso_wr_callback().set_nop(); // SSO pin unconnected; the side select is PIO-driven (system port bit 2)
	FLOPPY_CONNECTOR(config, FD1797_TAG":0", xerox820_floppies, "sa450", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1797_TAG":1", xerox820_floppies, "sa450", floppy_image_device::default_mfm_floppy_formats);
}

// 820-II, Shugart SASI host adapter: 8" floppies + ST-506 rigid disk on the SA1403D.
void xerox820ii_state::xerox820iis(machine_config &config)
{
	xerox820ii_common(config);
	xerox820ii_sasi(config);    // replaces the FD1797 add-in board; all drives live on the SA1403D
}

// The 16/8 option board: an 8086 (16-bit) processor + RAM sharing the 820-II.
void xerox820ii_state::add_8086(machine_config &config)
{
	i8086_cpu_device &i8086(I8086(config, I8086_TAG, 4770000));
	i8086.set_addrmap(AS_PROGRAM, &xerox820ii_state::xerox168_mem);
	i8086.set_irq_acknowledge_callback(FUNC(xerox820ii_state::i8086_dbell_inta));

	/* internal ram */
	RAM(config, m_ram).set_default_size("192K").set_extra_options("320K");

	// The 16/8 ("RX") monitor expects the position-encoded Low Profile Keyboard
	// instead of the standard ASCII keyboard the 820-II uses.  Replace it at the
	// shared "kb" tag (m_kb resolves null, m_lpk resolves) and re-route the strobe.
	config.device_remove(KEYBOARD_TAG);
	xerox_lpk_device &lpk = XEROX_LPK(config, m_lpk);
	lpk.kbstb_wr_callback().set(m_kbpio, FUNC(z80pio_device::strobe_b));
}

// 820-II with the low-profile keyboard (for the LPK-family u36 ROMs, e.g. v016/v018)
void xerox820ii_state::xerox820iilp(machine_config &config)
{
	xerox820ii(config);
	config.device_remove(KEYBOARD_TAG);
	xerox_lpk_device &lpk = XEROX_LPK(config, m_lpk);
	lpk.kbstb_wr_callback().set(m_kbpio, FUNC(z80pio_device::strobe_b));
}

void xerox820ii_state::xerox1685s_io(address_map &map)
{
	xerox820iis_io(map); // u8 PIO (SASI) at 0x10-0x13 + the 8086 A0/A1 ports
	map(0xa6, 0xa6).mirror(0xff00).lr8(NAME([]() -> u8 { return 0x21; })); // expbx id = rgd5 (5.25" rigid disk unit)
	map(0xb0, 0xbf).select(0xff00).r(FUNC(xerox820ii_state::rgd5_rom_r)); // the unit's controller ROM (sdvr driver)
}

void xerox820ii_state::xerox168(machine_config &config)  { xerox820ii(config);  add_8086(config); } // 16/8, 8" floppy
void xerox820ii_state::xerox1685(machine_config &config) { xerox820ii5(config); add_8086(config); m_maincpu->set_addrmap(AS_IO, &xerox820ii_state::xerox1685_io); m_fdc_single_floppy = true; } // 16/8, 5.25" floppy + RX024 controller ROM (drive permanently cabled to FD1797)
void xerox820ii_state::xerox168s(machine_config &config) { xerox820iis(config); add_8086(config); } // 16/8, SASI
void xerox820ii_state::xerox1685s(machine_config &config) // 16/8 low profile, 5.25" rigid disk unit (rgd5 box: SASI rigid + floppies)
{
	xerox820ii_common(config);
	xerox820ii_sasi(config, true);
	add_8086(config);
	m_maincpu->set_addrmap(AS_IO, &xerox820ii_state::xerox1685s_io);
}

void mk83_state::machine_start()
{
	xerox820_state::machine_start();

	save_item(NAME(m_kbdata));
	save_item(NAME(m_bank_hi));
	save_item(NAME(m_bank_lo));
}

void mk83_state::machine_reset()
{
	// power-on bank code = {1,1,0} = 6 (EPROM mapped): PA6/PA7 pull-ups with
	// the PIO reset to inputs, port-0x14 latch bit 5 clear
	m_bank_hi = 3;
	m_bank_lo = 0;
	update_bank();

	m_fdc_xor = 0x00;   // the MK-83's FD1797 data bus is not inverted (unlike the Big Board FD1771)
	m_fdc->reset();

	// /SYNC idles inactive (RxD mark): the boot's console select polls RR0
	// bit 4 for a serial start bit and would otherwise mis-detect one
	m_sio->synca_w(1);
	m_sio->syncb_w(1);
}

// Drives shipped with the MK3000: BASF 6106 5.25" 40-track SS/DD and Shugart
// SA465 5.25" 80-track DS/DD; the board also carries a 50-pin 8" connector
// (the monitor's default FF6B geometry is in fact 8" SSSD 77x26x128).
static void mk83_floppies(device_slot_interface &device)
{
	device.option_add("basf6106", FLOPPY_525_SSDD);
	device.option_add("sa465", FLOPPY_525_QD);
	device.option_add("sa800", FLOPPY_8_SSDD);
	device.option_add("sa850", FLOPPY_8_DSDD);
}

static void mk83_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();    // includes IMD, the format the surviving Big Board family disks ship in
}

void mk83_state::mk83(machine_config &config)
{
	/* basic machine hardware: Z80A at 2.5 MHz (20 MHz crystal / 8, schematic sheet 2) */
	Z80(config, m_maincpu, 20_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &mk83_state::mk83_mem);
	m_maincpu->set_addrmap(AS_IO, &mk83_state::mk83_io);
	m_maincpu->set_daisy_config(xerox820_daisy_chain);

	/* video hardware: 14.318 MHz dot clock (sheet 3), 80x24, 7x10 cells, 5x7 glyphs */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(mk83_state::screen_update));
	screen.set_raw(14.318181_MHz_XTAL, 910, 0, 560, 262, 0, 240);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_xerox820);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* keyboard PIO (also drive select, motor, FDC9229 config and the bank bits) */
	Z80PIO(config, m_kbpio, 20_MHz_XTAL / 8);
	m_kbpio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_kbpio->in_pa_callback().set(FUNC(mk83_state::mk83_kbpio_pa_r));
	m_kbpio->out_pa_callback().set(FUNC(mk83_state::mk83_kbpio_pa_w));
	m_kbpio->in_pb_callback().set(FUNC(mk83_state::mk83_kbpio_pb_r));

	/* general purpose (printer) PIO */
	z80pio_device& pio_gp(Z80PIO(config, Z80PIO_GP_TAG, 20_MHz_XTAL / 8));
	pio_gp.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	/* CTC: ch2 timer (/16, TC 210) cascades into ch3 counter (TC 93) for the
	   8 Hz tick that runs the monitor's H/K clock and motor timeout; the disk
	   code also reads the ch3 down-counter (port 0x1B) as its spin-up timer */
	Z80CTC(config, m_ctc, 20_MHz_XTAL / 8);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	/* FD1797 + FDC9229BT; 16 MHz xtal on sheet 5 (1 MHz FDC clock for 5.25", 2 MHz for 8") */
	FD1797(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->drq_wr_callback().set(FUNC(mk83_state::mk83_fdc_drq_w));
	FLOPPY_CONNECTOR(config, FD1771_TAG":0", mk83_floppies, "sa465", mk83_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1771_TAG":1", mk83_floppies, "basf6106", mk83_floppy_formats);

	/* SIO: channel B (data 0x05 / control 0x07) is the serial console option */
	Z80SIO(config, m_sio, 20_MHz_XTAL / 8);
	m_sio->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// the boot console select watches the /SYNC pin (RR0 bit 4) for a start
	// bit to autobaud a serial console, so RxD also feeds /SYNC
	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.rxd_handler().append(m_sio, FUNC(z80sio_device::synca_w));
	rs232a.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	rs232a.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	rs232b.rxd_handler().append(m_sio, FUNC(z80sio_device::syncb_w));
	rs232b.cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));
	rs232b.dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));

	com8116_device &dbrg(COM8116(config, COM8116_TAG, 5.0688_MHz_XTAL));
	dbrg.fr_handler().set(m_sio, FUNC(z80sio_device::rxca_w));
	dbrg.fr_handler().append(m_sio, FUNC(z80sio_device::txca_w));
	dbrg.ft_handler().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	/* ASCII keyboard -> inverted data on PIO-B with a strobe per byte */
	GENERIC_KEYBOARD(config, m_gkb, 0);
	m_gkb->set_keyboard_callback(FUNC(mk83_state::kb_put));
}

/* ROMs */

ROM_START( bigboard )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "bigboard.u67", 0x0000, 0x0800, CRC(5a85a228) SHA1(d51a2cbd0aae80315bda9530275aabfe8305364e) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "bigboard.u73", 0x0000, 0x0800, CRC(10bf0d81) SHA1(7ec73670a4d9d6421a5d6a4c4edc8b7c87923f6c) )
ROM_END

#define rom_mk82 rom_bigboard
#define rom_bigboard5 rom_bigboard

ROM_START( x820 )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v20" )
	ROM_SYSTEM_BIOS( 0, "v10", "Xerox Monitor v1.0" )
	ROMX_LOAD( "x820v10.u64", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "x820v10.u63", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v20", "Xerox Monitor v2.0" )
	ROMX_LOAD( "x820v20.u64", 0x0000, 0x0800, CRC(2fc227e2) SHA1(b4ea0ae23d281a687956e8a514cb364a1372678e), ROM_BIOS(1) )
	ROMX_LOAD( "x820v20.u63", 0x0800, 0x0800, CRC(bc11f834) SHA1(4fd2b209a6e6ff9b0c41800eb5228c34a0d7f7ef), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "smart23", "MICROCode SmartROM v2.3" )
	ROMX_LOAD( "mxkx25a.u64", 0x0000, 0x0800, CRC(7ec5f100) SHA1(5d0ff35a51aa18afc0d9c20ef99ff5d9d3f2075b), ROM_BIOS(2) )
	ROMX_LOAD( "mxkx25b.u63", 0x0800, 0x0800, CRC(a7543798) SHA1(886e617e1003d13f86f33085cbd49391b77291a3), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "plus2", "MICROCode Plus2 v0.2a" )
	ROMX_LOAD( "p2x25a.u64",  0x0000, 0x0800, CRC(3ccd7a8f) SHA1(6e46c88f03fc7289595dd6bec95e23bb13969525), ROM_BIOS(3) )
	ROMX_LOAD( "p2x25b.u63",  0x0800, 0x0800, CRC(1e580391) SHA1(e91f8ce82586df33c0d6d02eb005e8079f4de67d), ROM_BIOS(3) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "x820.u92", 0x0000, 0x0800, CRC(b823fa98) SHA1(ad0ea346aa257a53ad5701f4201896a2b3a0f928) )
ROM_END

// The base 820-II boot ROM set, shared by the FD1797 floppy (5.25"/8") and the SASI
// disk-board variants (the ROM auto-detects the daughterboard via the PA bits).
#define ROM_X820II_CONTENTS \
	ROM_REGION( 0x2000, Z80_TAG, 0 ) \
	ROM_DEFAULT_BIOS( "v50" ) /* the standard keyboard and the in-ROM SASI driver both live in u33-u35 (the missing v500.u36 is not needed for floppy or SASI operation).  The v50v018 "Typewriter" build gates keyboard input differently (no echo from the standard keyboard). */ \
	/* v4.00/v4.01 u33-u35 and v4.04 u36 are not chip reads: they are ROM images recovered from Balcones/Xerox distribution master disks in the Don Maslin 820-II archive (bitsavers 820ii_images) and functionally validated by booting.  Images + recovered source: https://github.com/davidlrand/mame-system-media (Xerox-820-line-roms) */ \
	ROM_SYSTEM_BIOS( 0, "v400", "Balcones Operating System v4.00" ) /* u33-u35 split from ROM400.COM, master disk B16D35 */ \
	ROMX_LOAD( "v400.u33", 0x0000, 0x0800, CRC(0d1bcaa8) SHA1(a6ac83f8584d19f7a08e666cb5d4b62620d7d3c0), ROM_BIOS(0) ) \
	ROMX_LOAD( "v400.u34", 0x0800, 0x0800, CRC(f1df9e29) SHA1(79f38880c3aed9ddf2ccba4ddb11128586dc9c25), ROM_BIOS(0) ) \
	ROMX_LOAD( "v400.u35", 0x1000, 0x0800, CRC(68357ed7) SHA1(78498542f4d8506edf5f2b3b9ed0fde3fd72f85b), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "v401", "Balcones Operating System v4.01" ) /* u33-u35 decoded from U3x.HEX, "820-II ROM IMAGES MASTER" disk B17D7 */ \
	ROMX_LOAD( "v401.u33", 0x0000, 0x0800, CRC(fe9fa596) SHA1(194162b3d063b2d1bcad03d0bee51dabce2d1985), ROM_BIOS(1) ) \
	ROMX_LOAD( "v401.u34", 0x0800, 0x0800, CRC(d3137de3) SHA1(d1de4e11f29799b2024af0412415a07984e58f3a), ROM_BIOS(1) ) \
	ROMX_LOAD( "v401.u35", 0x1000, 0x0800, CRC(bf3096fb) SHA1(136f6b1cf2cd93f0b908688394675ef69883f47b), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 2, "v402", "Balcones Operating System v4.02" ) \
	ROMX_LOAD( "u33.4.02.rom", 0x0000, 0x0800, CRC(d9eb668e) SHA1(6acbef96e4e6526c58e068b7849fb9cce2ea2a10), ROM_BIOS(2) ) \
	ROMX_LOAD( "u34.4.02.rom", 0x0800, 0x0800, CRC(62181209) SHA1(2238aec096d19af9307bb294532f66f53dd7dfc3), ROM_BIOS(2) ) \
	ROMX_LOAD( "u35.4.02.rom", 0x1000, 0x0800, CRC(e22fbf6d) SHA1(6c162f79d42611176b0f1c0e8a4eeb07492beca1), ROM_BIOS(2) ) \
	ROMX_LOAD( "u36.rx11.4.02.rom", 0x1800, 0x0800, CRC(b6a239ce) SHA1(330d28fa8ec006d48d948b1c5e714ffced88fe90), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 3, "v403", "Balcones Operating System v4.03" ) \
	ROMX_LOAD( "v403.u33", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(3) ) \
	ROMX_LOAD( "v403.u34", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(3) ) \
	ROMX_LOAD( "v403.u35", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(3) ) \
	ROMX_LOAD( "v403.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(3) ) \
	ROM_SYSTEM_BIOS( 4, "v404", "Balcones Operating System v4.04" ) \
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(4) ) \
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(4) ) \
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(4) ) \
	ROMX_LOAD( "v404.u36", 0x1800, 0x0800, CRC(97047d38) SHA1(f36506635653736b8d754d2c04f608180602b5a2), ROM_BIOS(4) ) /* RX ver 016 (27-Sep-83), best-available pair for v4.04, functionally validated; decoded from U36.ROM, master disk B17D7 */ \
	ROM_SYSTEM_BIOS( 5, "v50", "Balcones Operating System v5.0" ) \
	ROMX_LOAD( "u33.5.0_537p10828.bin", 0x0000, 0x0800, CRC(a17af0f1) SHA1(b1d9a151ed4558f49b3cdc1adbf348b54da48877), ROM_BIOS(5) ) \
	ROMX_LOAD( "u34.5.0_537p10829.bin", 0x0800, 0x0800, CRC(c9f5182e) SHA1(ac830848614cea984c849a42687ea2944d6765d9), ROM_BIOS(5) ) \
	ROMX_LOAD( "u35.5.0_537p10830.bin", 0x1000, 0x0800, CRC(278fa75f) SHA1(f47cf9eb30366211280f93a8460523fcc53eebe9), ROM_BIOS(5) ) \
	ROMX_LOAD( "v500.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(5) ) \
	ROM_SYSTEM_BIOS( 6, "v50v018", "Balcones Operating System v5.0 v018" ) \
	ROMX_LOAD( "537p10828.u33.5.0.bin", 0x0000, 0x0800, CRC(a17af0f1) SHA1(b1d9a151ed4558f49b3cdc1adbf348b54da48877), ROM_BIOS(6) ) \
	ROMX_LOAD( "537p10829.u34.5.0.bin", 0x0800, 0x0800, CRC(c9f5182e) SHA1(ac830848614cea984c849a42687ea2944d6765d9), ROM_BIOS(6) ) \
	ROMX_LOAD( "u35.5.0_537p10830.bin", 0x1000, 0x0800, CRC(278fa75f) SHA1(f47cf9eb30366211280f93a8460523fcc53eebe9), ROM_BIOS(6) ) /* good dump from Balcones v5.0 source disk B23D13; was BAD_DUMP cc4e1c2b */ \
	ROMX_LOAD( "537p10831.u36.5.0.bin", 0x1800, 0x0800, CRC(cda7f598) SHA1(08ffd18959e1708136076c82486b8d121a04fa23), ROM_BIOS(6) ) \
	ROM_REGION( 0x1000, "chargen", 0 ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(0) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(0) ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(1) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(1) ) \
	ROMX_LOAD( "u57.04.north.rom", 0x0000, 0x0800, CRC(eda727a2) SHA1(292cd8a0dc6699c3a2091b20c0fc63d97a266fbf), ROM_BIOS(2) ) \
	ROMX_LOAD( "u58.03.north.rom", 0x0800, 0x0800, CRC(a2e514f3) SHA1(8ac22dd0cf0324a857718adf67b41912864893a3), ROM_BIOS(2)  ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(3) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(3) ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(4) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(4) ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(5) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(5) ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(6) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(6) )

ROM_START( x820ii )  ROM_X820II_CONTENTS ROM_END // 820-II, 8" floppy
ROM_START( x820ii5 ) ROM_X820II_CONTENTS ROM_END // 820-II, 5.25" floppy
ROM_START( x820iis ) ROM_X820II_CONTENTS ROM_END // 820-II, SASI hard disk
ROM_START( x820iilp ) ROM_X820II_CONTENTS ROM_END // 820-II, low-profile keyboard

ROM_START( x168 )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v50" ) // 16/8 ships with the Low Profile Keyboard, which only the v5.0 "RX" monitor decodes (v4.04 can't, and echoes "what?")
	ROM_SYSTEM_BIOS( 0, "v404", "Balcones Operating System v4.04" ) // Changes sign-on message from Xerox 820-II to Xerox
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(0) )
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(0) )
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(0) )
	ROMX_LOAD( "v404.u36", 0x1800, 0x0800, CRC(97047d38) SHA1(f36506635653736b8d754d2c04f608180602b5a2), ROM_BIOS(0) ) // fitted for low-profile keyboard only; RX ver 016, recovered from master disk B17D7 (see x820ii)

	ROM_SYSTEM_BIOS( 1, "v50", "Balcones Operating System v5.0" ) // Operating system modifications for DEM and new 5.25" disk controller (4 new boot ROMs)
	ROMX_LOAD( "l5.u33.rom", 0x0000, 0x0800, CRC(a17af0f1) SHA1(b1d9a151ed4558f49b3cdc1adbf348b54da48877), ROM_BIOS(1) )
	ROMX_LOAD( "l5.u34.rom", 0x0800, 0x0800, CRC(c9f5182e) SHA1(ac830848614cea984c849a42687ea2944d6765d9), ROM_BIOS(1) )
	ROMX_LOAD( "u35.5.0_537p10830.bin", 0x1000, 0x0800, CRC(278fa75f) SHA1(f47cf9eb30366211280f93a8460523fcc53eebe9), ROM_BIOS(1) ) // was BAD_DUMP l5.u35.rom (44c8dbf8): same ROM as 537p10830 but with data bit 7 stuck high
	ROMX_LOAD( "u36.rx024.rom", 0x1800, 0x0800, CRC(a7f1d677) SHA1(8c2a442f3a691f2e181a640d65f767ce3b51d711), ROM_BIOS(1) ) // fitted for low-profile keyboard only

	ROM_REGION( 0x1000, I8086_TAG, 0 )
	ROM_LOAD( "8086.u33", 0x0000, 0x1000, CRC(ee49e3dc) SHA1(a5f20c74fc53f9d695d8894534ab69a39e2c38d8) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(0) )
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(0) )

	ROMX_LOAD( "u57.04.north.rom", 0x0000, 0x0800, CRC(eda727a2) SHA1(292cd8a0dc6699c3a2091b20c0fc63d97a266fbf), ROM_BIOS(1) )
	ROMX_LOAD( "u58.03.north.rom", 0x0800, 0x0800, CRC(a2e514f3) SHA1(8ac22dd0cf0324a857718adf67b41912864893a3), ROM_BIOS(1)  )
ROM_END

ROM_START( mk83 )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "2732mk83.bin", 0x0000, 0x1000, CRC(a845c7e1) SHA1(3ccf629c5cd384953794ac4a1d2b45678bd40e92) ) // "SCOMAR MULTIPROG.   REL. 2.00" system monitor (U67; sockets U68-U70 for option ROMs are empty)

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "2716mk83.bin", 0x0000, 0x0800, CRC(10bf0d81) SHA1(7ec73670a4d9d6421a5d6a4c4edc8b7c87923f6c) ) // character generator (U73), same chip as the Big Board's
ROM_END

ROM_START( mojmikro )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_LOAD( "mikro-s.u67", 0x0000, 0x0800, CRC(56a329a8) SHA1(22a5d6bef121d14eddc0c25e85b8a73f6ca6a65f))
	ROM_REGION( 0x0800, "chargen", ROMREGION_ERASEFF ) // MMSCHAR YU 8.1.1987
	ROM_LOAD( "mmschar-yu.u73", 0x0000, 0x0800, CRC(ebcc72d3) SHA1(1c3f90b1d2e57586dcd32385d0aaa09e56662e32))
ROM_END

// The 16/8 variants carry the 16/8 boot ROM set (which adds the 8086 ROM and differs
// from the plain 820-II); like x168 they group under x820ii.  Same ROMs across the three
// disk personalities, so they re-list x168's set (MAME de-duplicates the files by hash).
#define ROM_X168_CONTENTS \
	ROM_REGION( 0x2000, Z80_TAG, 0 ) \
	ROM_DEFAULT_BIOS( "v50" ) \
	ROM_SYSTEM_BIOS( 0, "v404", "Balcones Operating System v4.04" ) \
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(0) ) \
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(0) ) \
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(0) ) \
	ROMX_LOAD( "v404.u36", 0x1800, 0x0800, CRC(97047d38) SHA1(f36506635653736b8d754d2c04f608180602b5a2), ROM_BIOS(0) ) /* RX ver 016, recovered from master disk B17D7 (see x820ii) */ \
	ROM_SYSTEM_BIOS( 1, "v50", "Balcones Operating System v5.0" ) \
	ROMX_LOAD( "l5.u33.rom", 0x0000, 0x0800, CRC(a17af0f1) SHA1(b1d9a151ed4558f49b3cdc1adbf348b54da48877), ROM_BIOS(1) ) \
	ROMX_LOAD( "l5.u34.rom", 0x0800, 0x0800, CRC(c9f5182e) SHA1(ac830848614cea984c849a42687ea2944d6765d9), ROM_BIOS(1) ) \
	ROMX_LOAD( "u35.5.0_537p10830.bin", 0x1000, 0x0800, CRC(278fa75f) SHA1(f47cf9eb30366211280f93a8460523fcc53eebe9), ROM_BIOS(1) ) \
	ROMX_LOAD( "u36.rx024.rom", 0x1800, 0x0800, CRC(a7f1d677) SHA1(8c2a442f3a691f2e181a640d65f767ce3b51d711), ROM_BIOS(1) ) \
	ROM_REGION( 0x1000, I8086_TAG, 0 ) \
	ROM_LOAD( "8086.u33", 0x0000, 0x1000, CRC(ee49e3dc) SHA1(a5f20c74fc53f9d695d8894534ab69a39e2c38d8) ) \
	ROM_REGION( 0x1000, "chargen", 0 ) \
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(0) ) \
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(0) ) \
	ROMX_LOAD( "u57.04.north.rom", 0x0000, 0x0800, CRC(eda727a2) SHA1(292cd8a0dc6699c3a2091b20c0fc63d97a266fbf), ROM_BIOS(1) ) \
	ROMX_LOAD( "u58.03.north.rom", 0x0800, 0x0800, CRC(a2e514f3) SHA1(8ac22dd0cf0324a857718adf67b41912864893a3), ROM_BIOS(1) )

ROM_START( x1685 ) ROM_X168_CONTENTS ROM_END // 16/8, 5.25" floppy
ROM_START( x1685s ) ROM_X168_CONTENTS ROM_END // 16/8, 5.25" rigid disk unit (rgd5 box)
ROM_START( x168s ) ROM_X168_CONTENTS ROM_END // 16/8, SASI hard disk

/* System Drivers */

//    YEAR  NAME      PARENT    COMPAT  MACHINE      INPUT     CLASS             INIT        COMPANY                       FULLNAME                          FLAGS
COMP( 1980, bigboard, 0,        0,      bigboard,    xerox820, bigboard_state,   empty_init, "Digital Research Computers", "Big Board",                      0 )
COMP( 1980, bigboard5,bigboard, 0,      bigboard5,   xerox820, bigboard_state,   empty_init, "Digital Research Computers", "Big Board (5.25\" drives)",      MACHINE_NOT_WORKING )
COMP( 1981, x820,     bigboard, 0,      xerox820,    xerox820, xerox820_state,   empty_init, "Xerox",                      "Xerox 820",                      MACHINE_NO_SOUND_HW )
COMP( 1982, mk82,     bigboard, 0,      bigboard,    xerox820, bigboard_state,   empty_init, "Scomar",                     "MK-82",                          0 )
COMP( 1983, x820ii,   0,        0,      xerox820ii,  xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 820-II (8\" floppy)",      0 )
COMP( 1983, x820iilp, x820ii,   0,      xerox820iilp, xerox820, xerox820ii_state, empty_init, "Xerox",                     "Xerox 820-II (low profile keyboard)", 0 )
COMP( 1983, x820ii5,  x820ii,   0,      xerox820ii5, xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 820-II (5.25\" floppy)",   0 )
COMP( 1983, x820iis,  x820ii,   0,      xerox820iis, xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 820-II (SASI hard disk)",  0 )
COMP( 1983, x168,     x820ii,   0,      xerox168,    xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 16/8 (8\" floppy)",        0 )
COMP( 1983, x1685,    x820ii,   0,      xerox1685,   xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 16/8 (5.25\" floppy)",     0 )
COMP( 1983, x1685s,   x820ii,   0,      xerox1685s,  xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 16/8 (5.25\" rigid disk unit)", 0 ) // rigid controller is a documented reconstruction (real unit, likely a WD1002-05, is undumped)
COMP( 1983, x168s,    x820ii,   0,      xerox168s,   xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 16/8 (SASI hard disk)",    0 )
COMP( 1983, mk83,     bigboard, 0,      mk83,        xerox820, mk83_state,       empty_init, "ADE Elettronica",            "MK-83",                          MACHINE_NO_SOUND_HW ) // fitted firmware is Scomar's MULTIPROG REL. 2.00; no original MK3000 media survives - boots a CP/M 2.2 disk reconstructed from the period format tables
COMP( 1985, mojmikro, bigboard, 0,      bigboard,    xerox820, bigboard_state,   empty_init, "<unknown>",                  "Moj mikro Slovenija",            0 )
