// license:BSD-3-Clause
// copyright-holders:rfka01
/***************************************************************************

    NCR 9010

    8085A-based business system (about 1981), 256 KB RAM behind a page MMU,
    8" flexible disk drives, optional fixed disk, character video 80x24.
    Typical operating system: SB-80 (Lifeboat Associates, CP/M 2.2 compatible).

    Boards (from photos of a machine at Schroeder):
      M210-02  Common Control Module: P8085A-2, 27.648 MHz oscillator,
               2716 EPROMs "1023" (ROM 1, J1) and "1024" (ROM 2, F1),
               empty third socket G1, 93419 64x9 bipolar RAM (MMU map),
               CCM indicator board (8 LEDs "17".."10" = bits 7-0, RUN LED)
      M217-02  memory board, 36 x 64Kx1 DRAM (4 of 8 rows fitted, parity)
      M215-01  discrete DMA board
      M223-06  video board, no CRTC (TTL timing), Intel D2608 "6-1043026" and
               EPROM "9709" (character generator?), MMI 6331 PROM, 18 x 1Kx1 SRAM
      I/O Processor: microcoded, 2 x Am2901, 5 x MMI 6341 PROMs (1117..1120, 1122)
      Flex Disk IF: discrete drive interface, PROMs "502" and "177"

    The two boot EPROMs are stored with inverted data lines.

    Front panel (right of the screen, NCR GmbH Augsburg, class 3274,
    model 1201-4301; the drive cabinet is class 7646, model 0201-2390):
    FLEX DRIVE STATUS with a green READY and a red BUSY lamp per drive 0-3,
    a MONIT key, COMM and POWER lamps, power switch and brightness wheel.
    READY follows the ready line of the drive, BUSY lights while the flex controller
    works on the drive and for a short head unload delay afterwards.  The
    COMM lamp stays dark (no communication hardware is emulated); RUN on
    the CCM indicator is lit while the machine is on.

    MONIT: the key sets bit 6 of port 05 and pulls TRAP.  The boot ROM has
    no RAM vector for TRAP (SB-80 leaves 0024h alone), so the TRAP
    acknowledge is assumed to switch the boot ROM in, which the emulation
    does.  The ROM handler (0024h -> 0742h) saves the registers to 3FE1h-
    3FEAh; with bit 6 set it acknowledges with OUT 05 and enters the ROM
    monitor ("MONITOR?", commands M R G I E F C T L Z), with bit 7 set it
    shows 18h on the CCM LEDs and stops (power fail?), otherwise it
    restarts.  The monitor keeps its command table and work area at
    3F80h-3FFFh, which is only set up at power on: before an operating
    system is loaded it works; after SB-80 has used that memory it may not.

    Memory map as seen by the 8085 (derived from the boot ROM and SBIOS 1.8):
      0000-0FFF  boot ROM while bit 0 of port 04 is set (writes go to RAM;
                 port 05 bit 3 reports the state)
      0000-FFFF  RAM through the MMU (16 x 4 KB pages, 64 physical pages)
      FC00-FCFF  memory mapped I/O, mirrors I/O ports 00-FF
      FD00-FDFF  flexible disk controller (I/O Processor)

    MMU: writing port 04 with bit 2 set arms a map write; the next memory
    write cycle does not reach memory.  Its address supplies the data:
      A15-A12 logical page, A11-A10 map set, A9-A8 = 01 (page register),
      A7-A2 = inverted physical page.  Writes with A9-A8 != 01 are control
      writes of unknown meaning (the ROM writes address 0000 after loading
      the maps).  Only map set 0 is ever used by the ROM and SB-80, so map
      set selection is not emulated.

    I/O ports (boot ROM, SB-80 and the NCR I-8100 Series pocket reference,
    whose port map the 9010 shares):
      00   R  interrupt status (bits 5-3 internal level 4/5/6, bit 6 maskable
              interrupts disabled, bit 7 internal interrupt waiting)
      01  RW  CCM indicator LEDs (POST step / firmware error code); also the
              data port of the byte link to a DMV (data inverted, see below).
              The 8100 has its indicator lights on 06 instead.
      02  RW  byte link status: bits 1-0 = 3 byte received, 2 ready to send
      03  RW  interval timer count (RST 5 at 0028h reads it)
      04   W  01 enable on-board memory (boot ROM), 00 disable, bit 2 arm
              MMU write (8100: "enable memory address translator control")
      05  RW  nonmaskable interrupt (TRAP) status: bit 0 transfer error,
              bit 1 parity error, bit 2 time-out, bit 3 on-board memory
              enabled, bit 6 MONIT key, bit 7 power down (error code 18);
              writing 00 resets it
      12   W  written 00 at reset
      21   R  workstation status / mode switches (bit 7 = 0 ROM test modes)
      23   R  configuration switches SW1 (bits 3-0) and SW2 (bits 7-4)
      27   W  workstation interface reset
      28-2F   keyboard: 28 data, 29 status (10 data, 20 entry not accepted,
              40 break) / clear ENA, 2A/2B enable/disable interrupt, 2C reset
              service request, 2D diagnostic clear, 2E bell, 2F reset
      30-37   parallel printer 1 (NCR 6441): 30 data, 31 status (bits C6 =
              error), 32 enable interrupt / read data turnaround, 33 disable
              interrupt, 37 reset; SBIOS 1.8 polls it, LST: = P1 by default
      41-47   data cartridge, load device "2" (41 command: 00 rewind,
              01 unload, 02 tapemark, 07 write, 08 read, 09 retension,
              0A-3A track, 0E/1E unit; 42 diagnostic status, 43/46 int,
              45 functional status, 47 reset service request; not emulated)
      61-67   NCR 6560 fixed disk controller, load device "1" (not emulated,
              reports "not ready", see below)
      8C-8F   flex disk DMA      \
      94-97   data cartridge DMA  | byte 0 A7-A0, byte 1 A15-A8, byte 2 count
      9C-9F   cassette DMA        | F7-F0, byte 3 C1 C0 A17 A16 F11-F8 with
      A4-A7   fixed disk DMA     /  C1C0 = 10 write memory, 11 read memory.
              The DMA addresses physical memory (18 bits) past the MMU.
      C0-C4   CRT: cursor X, cursor Y, display char (auto increment),
              control character / attribute, display enable
    8100 options at FDxx, not known on the 9010 and not emulated: FDA0-FDA7
    message level adapter M250-05 (synchronous BSC, V.24 modem control;
    probably the 9010 COMM option), FDD0-FDD7 auxiliary printer interface
    (SBIOS printer P2, NCR 4504 visual record printer), FDE0-FDE3 time of
    day clock, FCE0-FCF9 cassette.

    Flex disk controller (FD01 command / sense 1, FD02 sense 2 / int on,
    FD03 int off, FD04 reset service request, FD07 reset):
      the CPU writes four bytes: function, unit (bits 2-1) / side (bit 0) /
      double density (bit 7), cylinder, sector.  Functions: 0 restore,
      1 write format, 2 read format (IDs), 3 write data, 4 read data,
      5 write deleted sector, 6 format defective track, 7 verify, 8 DMA
      byte out, 9 DMA byte in, A byte turnaround, B data turnaround; the
      high nibble selects ASCII/EBCDIC conversion (8, C).  0-4 and 7 are
      emulated.  Data moves by DMA for the full count; the controller
      continues with the next sector and the next cylinder (same head)
      until the count is exhausted.  Sense 1: bit 7 busy, bit 6 service
      request, bit 5 deleted data mark, bits 3-0 result (0 diagnostics
      failed, 1 soft error, 2 ok, 3 invalid instruction, 4 not ready,
      5 media changed since the last command (not generated; the ROM
      restores again), 7 verify error, 8 beyond track 73 sector 26).
      Sense 2: 40 DMA parity, 20 CRC, 10 DMA time-out, 08 data mark not
      found, 04 sector not found, 02 seek error, 01 write fault.  SB-80
      only tests for 2.  The I/O Processor PROMs are not dumped, so the
      controller is a self-contained high-level emulation that works on
      the flux transitions of the drives (FM/MFM coding, IBM track
      layout); it is meant to be replaced once the PROMs are dumped.

    Disk format (SB-80): cylinder 0 head 0 FM 26 x 128 ("NCR FMT-" ID
    block), all other tracks MFM 26 x 256.  Logical track 1 = cylinder 72,
    logical track 2 = cylinder 73 (boot sector, BIOS, DOS); the boot ROM
    reads cylinder 72 sectors 1-26 and cylinder 73 sectors 1-6 to 0100h and
    requires a zero 8-bit sum over 0100h-20FFh.

    Fixed disk controller (SBIOS 1.8 / 1.7, boot ROM load device "1"):
      NCR 6560 cartridge disk (8100 series: one fixed and one removable
      4.9 MB disk, 35 ms average access, 312 KB/s per Datapro 1980; these
      figures match a CDC 9427H Hawk, not verified).  FC61 command /
      sense 1, FC62 sense 2 / int on, FC63 int off, FC64 reset service
      request, FC67 reset.
      The CPU writes four bytes, waiting for bit 7 (busy) to clear between
      them: command (00 restore, 01 seek, 02 write format, 03 read format,
      04 write, 05 read, 06 time disk, 07 seek RT, 08 fill buffer,
      09 dump buffer, 0A data / 0B byte turnaround), unit (software unit
      0-7 = drive 1-4, disk 0/1; SB-80 uses 0 and 1 as C: and D:), block
      number low, block number high (00-3F).  Sense 1: 80 busy, 40 poll,
      10 write protected, 08 not ready, 04 invalid instruction, 02 ok,
      01 soft error; sense 2: 80 alternate track used, 40 DMA parity,
      20 CRC, 10 missing clock, 08 no data sync, 04 sector not found,
      02 drive fault, 01 seek error.  Blocks are 512 bytes, addressed
      linearly (BIOS: track * 24 + sector - 1), so the controller handles
      the drive geometry itself.  Completion sets bit 6 (service request),
      result = status & 1F (02 ok, 08 not ready).  DMA on FCA4-FCA7 with
      the same layout as the flex disk DMA.  SB-80 drives C: and D: are one
      unit each: 24 x 512 bytes per track, 4 KB blocks, 1200 blocks
      (4.8 MB), 512 directory entries, 1 reserved track.  Block 11 holds
      an alternate track table (6 entries, lookup disabled in SBIOS 1.8).
      The boot ROM loads blocks 12-19 to 0100h and 20-27 to 1100h (same
      checksum rule as the flex disk); CONFIG bit 7 selects the unit.
      The controller board is not in the photos.

    DMV link (SB-80 "Fix Disk Version 1.7 WITH INTERRUPT"): no hardware
    interrupts; the BIOS hooks the BDOS entry and the console wait loops
    and polls port 02.  When a byte is pending (status 3) it receives a
    disk request block over port 01 (inverted data), runs it on the local
    fixed disk and transfers 512 byte sectors over the link, making the
    9010 a fixed disk server for a DMV (see SEND9010/EMPF9010 and
    SENDDMV/EMPFDMV, DMV side on ports C0-C3).  The link is reported idle.
    The link software writes and reads port 01 (OUT 1 / IN 1, data
    complemented) with OUT 2 / IN 2 as handshake, so it is a byte wide
    parallel link, presumably on the CCM header J1 that carries the port 01
    latch to the indicator board.  Neither the boot ROM nor SB-80 executes
    SIM, and RIM only appears in the TRAP handler (IE recovery), so the
    8085 SID/SOD pins are unused by the known software.

    TODO:
    - dump character generator, video PROMs, IOP microcode, Flex IF PROMs
      (the character generator is a substitute, see ROM_START)
    - real video timing, character cell, attributes, cursor shape
    - keyboard interface is HLE (key matrix scanned by the driver, codes from
      the I-8100 pocket reference; real scan hardware and timing unknown)
    - DMV byte link (ports 01/02)
    - interrupts (keyboard, disk), serial port, fixed disk,
      load device 2, third ROM socket ("EXTFW" extension firmware at 1000h)

****************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "imagedev/floppy.h"
#include "imagedev/printer.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/imd_dsk.h"

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "ncr9010.lh"

#define LOG_MMU     (1U << 1)
#define LOG_FLEX    (1U << 2)
#define LOG_IO      (1U << 3)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGMMU(...)  LOGMASKED(LOG_MMU,  __VA_ARGS__)
#define LOGFLEX(...) LOGMASKED(LOG_FLEX, __VA_ARGS__)
#define LOGIO(...)   LOGMASKED(LOG_IO,   __VA_ARGS__)


namespace {

/***************************************************************************
    FLEX DISK HELPERS (track cells, FM/MFM coding)
***************************************************************************/

u16 fx_crc(u16 crc, u8 data)
{
	// CRC-16-CCITT as used by IBM style disk formats
	crc ^= u16(data) << 8;
	for (int i = 0; i < 8; i++)
		crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
	return crc;
}

// a track as a sequence of bit cells (8": FM 2 us, MFM 1 us)
class fx_track
{
public:
	fx_track(bool dd) : m_dd(dd), m_prev(false) { }

	attotime cell_time() const { return attotime::from_nsec(m_dd ? 1000 : 2000); }
	std::vector<bool> &cells() { return m_cells; }
	std::vector<bool> const &cells() const { return m_cells; }

	// cells read from a disk keep the time of each flux transition, so
	// positions can be found again across phase jumps (write splices)
	void clear() { m_cells.clear(); m_times.clear(); }
	void flux(const attotime &when)
	{
		attoseconds_t const cell = cell_time().as_attoseconds();
		size_t pos = 0;
		if (!m_times.empty())
			pos = m_times.back().first + std::max<attoseconds_t>(((when - m_times.back().second).as_attoseconds() + cell / 2) / cell, 1);
		m_cells.resize(pos + 1, false);
		m_cells[pos] = true;
		m_times.emplace_back(pos, when);
	}
	void pad(const attotime &until)
	{
		if (!m_times.empty())
			m_cells.resize(m_times.back().first + (until - m_times.back().second).as_attoseconds() / cell_time().as_attoseconds() + 1, false);
	}
	attotime time_of(size_t pos) const
	{
		auto it = std::upper_bound(m_times.begin(), m_times.end(), pos, [] (size_t p, const auto &e) { return p < e.first; });
		if (it != m_times.begin())
			--it;
		return it->second + cell_time() * u32(pos - it->first);
	}

	void raw(u16 val)
	{
		for (int i = 15; i >= 0; i--)
			m_cells.push_back(BIT(val, i));
		m_prev = BIT(val, 0);
	}

	void byte(u8 val, u8 clock = 0xff)
	{
		for (int i = 7; i >= 0; i--)
		{
			bool const d = BIT(val, i);
			m_cells.push_back(m_dd ? (!m_prev && !d) : BIT(clock, i));
			m_cells.push_back(d);
			m_prev = d;
		}
	}

	void bytes(u8 val, unsigned int count)
	{
		for (unsigned int i = 0; i < count; i++)
			byte(val);
	}

	// address mark: MFM 3 x A1 (or C2 for the index mark) with a missing
	// clock, FM a mark with clock C7 (D7 for the index mark); returns the CRC
	u16 mark(u8 val)
	{
		u16 crc = 0xffff;
		if (m_dd)
		{
			for (int i = 0; i < 3; i++)
			{
				raw((val == 0xfc) ? 0x5224 : 0x4489);
				crc = fx_crc(crc, (val == 0xfc) ? 0xc2 : 0xa1);
			}
			byte(val);
		}
		else
		{
			byte(val, (val == 0xfc) ? 0xd7 : 0xc7);
		}
		return fx_crc(crc, val);
	}

	// data byte whose first (clock) cell is at pos
	u8 get_byte(unsigned int pos) const
	{
		u8 val = 0;
		for (unsigned int i = 0; i < 8; i++)
			val = (val << 1) | ((pos + i * 2 + 1 < m_cells.size()) && m_cells[pos + i * 2 + 1]);
		return val;
	}

private:
	bool m_dd;
	bool m_prev;
	std::vector<bool> m_cells;
	std::vector<std::pair<size_t, attotime> > m_times;
};

struct fx_sector
{
	u8 c, h, r, n;
	bool id_ok;
	int id_pos;     // first cell of the ID mark (MFM: of the first A1)
	int data_pos;   // first cell of the data mark, -1 if none
	u8 data_mark;
};

// find the ID and data fields of a track
void fx_decode(const fx_track &trk, bool dd, std::vector<fx_sector> &sectors)
{
	sectors.clear();
	std::vector<bool> const &cells = trk.cells();

	u64 shift = 0;
	for (unsigned int i = 0; i < cells.size(); i++)
	{
		shift = (shift << 1) | (cells[i] ? 1 : 0);
		int pos;
		u8 mark;
		u16 crc = 0xffff;
		if (dd)
		{
			if ((i < 47) || ((shift & 0xffff'ffff'ffffU) != 0x4489'4489'4489U))
				continue;
			pos = i - 47;
			mark = trk.get_byte(i + 1);
			for (int j = 0; j < 3; j++)
				crc = fx_crc(crc, 0xa1);
		}
		else
		{
			// clock C7 interleaved with data FE, FB or F8
			if (i < 15)
				continue;
			u8 clk = 0, dat = 0;
			for (int j = 0; j < 8; j++)
			{
				clk = (clk << 1) | BIT(shift, 15 - j * 2);
				dat = (dat << 1) | BIT(shift, 14 - j * 2);
			}
			if ((clk != 0xc7) || ((dat != 0xfe) && (dat != 0xfb) && (dat != 0xf8)))
				continue;
			pos = i - 15;
			mark = dat;
		}
		crc = fx_crc(crc, mark);
		int const body = pos + (dd ? 64 : 16);

		if (mark == 0xfe)
		{
			fx_sector s;
			s.c = trk.get_byte(body);
			s.h = trk.get_byte(body + 16);
			s.r = trk.get_byte(body + 32);
			s.n = trk.get_byte(body + 48);
			for (int j = 0; j < 6; j++)
				crc = fx_crc(crc, trk.get_byte(body + j * 16));
			s.id_ok = !crc;
			s.id_pos = pos;
			s.data_pos = -1;
			s.data_mark = 0;
			sectors.push_back(s);
		}
		else if ((mark == 0xfb) || (mark == 0xf8))
		{
			// a data field belongs to the ID right before it (within gap 2)
			if (!sectors.empty() && (sectors.back().data_pos < 0) && (pos - sectors.back().id_pos < (dd ? 64 : 40) * 16))
			{
				sectors.back().data_pos = pos;
				sectors.back().data_mark = mark;
			}
		}
	}
}

fx_sector const *fx_find(const std::vector<fx_sector> &sectors, u8 c, u8 h, u8 r)
{
	for (fx_sector const &s : sectors)
	{
		if (s.id_ok && (s.c == c) && (s.h == h) && (s.r == r))
			return &s;
	}
	return nullptr;
}


class ncr9010_state : public driver_device
{
public:
	ncr9010_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_floppy(*this, "flop%u", 0U)
		, m_printer(*this, "printer")
		, m_bell(*this, "bell")
		, m_palette(*this, "palette")
		, m_rom(*this, "maincpu")
		, m_chargen(*this, "chargen")
		, m_ram(*this, "ram", 0x40000, ENDIANNESS_LITTLE)
		, m_bank(*this, "bank%u", 0U)
		, m_rom_view(*this, "rom_view")
		, m_mmu_view(*this, "mmu_view")
		, m_mode(*this, "MODE")
		, m_config(*this, "CONFIG")
		, m_keys(*this, "KEY%u", 0U)
		, m_kbmods(*this, "KBMODS")
		, m_ccm_leds(*this, "ccm_led%u", 0U)
		, m_ccm_run(*this, "ccm_run")
		, m_ready_leds(*this, "flex_ready%u", 0U)
		, m_busy_leds(*this, "flex_busy%u", 0U)
		, m_comm_led(*this, "comm_led")
		, m_power_led(*this, "power_led")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(monit_pressed);

	void ncr9010(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr unsigned int SECTORS_PER_TRACK = 26;
	static constexpr attotime FX_REV_TIME = attotime(0, HZ_TO_ATTOSECONDS(6)); // 360 rpm
	static constexpr attotime FX_STEP_TIME = attotime::from_msec(3); // not verified
	static constexpr attotime FX_SETTLE_TIME = attotime::from_msec(15); // not verified
	static constexpr attotime FX_CMD_TIME = attotime::from_usec(100); // not verified

	// primary status (FD01 bits 3-0) and secondary status (FD02) after the I-8100 pocket reference
	enum : u8
	{
		RES_SOFT     = 0x01,
		RES_OK       = 0x02,
		RES_BADCMD   = 0x03,
		RES_NOTREADY = 0x04,
		RES_CHANGED  = 0x05, // media changed since the last command (not generated)
		RES_VERIFY   = 0x07,
		RES_BEYOND   = 0x08  // read/write beyond track 73 sector 26
	};

	enum : u8
	{
		ST2_WFAULT   = 0x01,
		ST2_SEEK     = 0x02,
		ST2_RNF      = 0x04,
		ST2_NOMARK   = 0x08,
		ST2_DMATIME  = 0x10,
		ST2_CRC      = 0x20,
		ST2_PARITY   = 0x40
	};

	required_device<i8085a_cpu_device> m_maincpu;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<printer_image_device> m_printer;
	required_device<beep_device> m_bell;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_rom;
	required_region_ptr<u8> m_chargen;
	memory_share_creator<u8> m_ram;
	memory_bank_array_creator<16> m_bank;
	memory_view m_rom_view;
	memory_view m_mmu_view;
	required_ioport m_mode;
	required_ioport m_config;
	required_ioport_array<11> m_keys;
	required_ioport m_kbmods;
	output_finder<8> m_ccm_leds;
	output_finder<> m_ccm_run;
	output_finder<4> m_ready_leds;
	output_finder<4> m_busy_leds;
	output_finder<> m_comm_led;
	output_finder<> m_power_led;

	u8 m_map[4][16];
	u8 m_memctrl;
	bool m_map_arm;
	u8 m_trap_cause;

	// video
	u8 m_vram[2048];
	u8 m_vattr[2048];
	u8 m_cur_x, m_cur_y;
	u8 m_crt_ctrl;

	// keyboard
	u8 m_kbd_fifo[16];
	u8 m_kbd_head, m_kbd_count;
	u8 m_kbd_prev[11];
	u8 m_kbd_last;     // last key (row * 8 + bit) for REP
	u8 m_kbd_reptime;
	emu_timer *m_kbd_timer;

	// flex disk
	u8 m_fx_cmdbuf[4];
	u8 m_fx_cmdcnt;
	u8 m_fx_status, m_fx_status2;
	u8 m_fx_cmd, m_fx_unit, m_fx_head, m_fx_cyl, m_fx_sec;
	bool m_fx_dd;
	u8 m_fx_dmareg[4];
	u32 m_fx_addr;
	u16 m_fx_count;
	bool m_fx_inten;
	u8 m_fx_result, m_fx_result2; // reported when the command completes
	emu_timer *m_fx_timer;
	emu_timer *m_hld_timer;
	emu_timer *m_bell_timer;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	void mmu_w(offs_t offset, u8 data);
	void update_views();
	u8 io_r(offs_t offset);
	void io_w(offs_t offset, u8 data);

	// the DMA board addresses physical memory with 18 bits and bypasses the MMU
	u8 dma_r(u32 addr) { return m_ram[addr & 0x3ffff]; }
	void dma_w(u32 addr, u8 data) { m_ram[addr & 0x3ffff] = data; }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void kbd_put(u8 data);
	TIMER_CALLBACK_MEMBER(kbd_scan);

	u8 flex_r(offs_t offset);
	void flex_w(offs_t offset, u8 data);
	void flex_command();
	void flex_error(u8 result, u8 status2 = 0);
	attotime flex_seek(floppy_image_device &floppy, int cyl);
	void flex_read_track(floppy_image_device &floppy, const attotime &start, fx_track &trk);
	void flex_write_track(floppy_image_device &floppy, const attotime &start, const fx_track &trk);
	void flex_transfer(floppy_image_device &floppy, attotime &done);
	void flex_read_ids(floppy_image_device &floppy, attotime &done);
	void flex_format(floppy_image_device &floppy, attotime &done);
	void floppy_ready(floppy_image_device *floppy, int state);
	TIMER_CALLBACK_MEMBER(flex_done);
	TIMER_CALLBACK_MEMBER(head_unload);
	TIMER_CALLBACK_MEMBER(bell_off) { m_bell->set_state(0); }
	IRQ_CALLBACK_MEMBER(irq_ack);
};


/***************************************************************************
    MEMORY
***************************************************************************/

// MMU map write: armed by port 04 bit 2, the next memory write cycle does not reach memory
void ncr9010_state::mmu_w(offs_t offset, u8 data)
{
	m_map_arm = false;
	update_views();
	if (BIT(offset, 8, 2) == 1)
	{
		u8 const set = BIT(offset, 10, 2);
		u8 const page = BIT(offset, 12, 4);
		m_map[set][page] = ~offset >> 2 & 0x3f;
		if (!set)
			m_bank[page]->set_entry(m_map[0][page]);
		LOGMMU("map set %u page %X -> %02X\n", set, page, m_map[set][page]);
	}
	else
	{
		LOGMMU("%s: MMU control write %04X = %02X\n", machine().describe_context(), offset, data);
	}
}

void ncr9010_state::update_views()
{
	if (BIT(m_memctrl, 0))
		m_rom_view.select(0);
	else
		m_rom_view.disable();

	if (m_map_arm)
		m_mmu_view.select(0);
	else
		m_mmu_view.disable();
}

void ncr9010_state::mem_map(address_map &map)
{
	// 16 x 4 KB pages through map set 0 of the MMU (map sets 1-3 are never used)
	for (int i = 0; i < 16; i++)
		map(i << 12, (i << 12) | 0x0fff).bankrw(m_bank[i]);
	map(0xfc00, 0xfcff).rw(FUNC(ncr9010_state::io_r), FUNC(ncr9010_state::io_w));
	map(0xfd00, 0xfdff).rw(FUNC(ncr9010_state::flex_r), FUNC(ncr9010_state::flex_w));

	// boot ROM overlay: reads only, writes still go to RAM
	map(0x0000, 0x0fff).view(m_rom_view);
	m_rom_view[0](0x0000, 0x0fff).lr8(NAME([this] (offs_t offset) { return m_rom[offset]; }));

	// armed MMU: the next write cycle loads a map register
	map(0x0000, 0xffff).view(m_mmu_view);
	m_mmu_view[0](0x0000, 0xffff).w(FUNC(ncr9010_state::mmu_w));
}

void ncr9010_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(ncr9010_state::io_r), FUNC(ncr9010_state::io_w));
}


/***************************************************************************
    I/O
***************************************************************************/

u8 ncr9010_state::io_r(offs_t offset)
{
	switch (offset)
	{
	case 0x05: // TRAP cause, bit 3 = on-board memory (boot ROM) enabled
		return m_trap_cause | (BIT(m_memctrl, 0) << 3);

	case 0x21:
		return m_mode->read();

	case 0x23:
		return m_config->read();

	case 0x28: // keyboard data
		if (m_kbd_count)
		{
			u8 const data = m_kbd_fifo[m_kbd_head];
			if (!machine().side_effects_disabled())
			{
				m_kbd_head = (m_kbd_head + 1) % std::size(m_kbd_fifo);
				m_kbd_count--;
			}
			return data;
		}
		return 0x00;

	case 0x29: // keyboard status
		return m_kbd_count ? 0x10 : 0x00;

	case 0x31: // printer 1 status, 0 = no error
		return 0x00;

	case 0x02: // DMV link status: idle (nothing received, not ready to send)
		return 0x00;

	case 0x41: // load device 2 status
		return 0x00;

	case 0x61: // fixed disk: service request + "not ready"
		return 0x44;

	case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		return m_fx_dmareg[offset & 3];

	case 0xc0:
		return m_cur_x;
	case 0xc1:
		return m_cur_y;
	case 0xc2:
		return m_vram[(m_cur_y * 80 + m_cur_x) & 0x7ff];
	case 0xc3:
		return m_vattr[(m_cur_y * 80 + m_cur_x) & 0x7ff];

	default:
		if (!machine().side_effects_disabled())
			LOGIO("%s: unknown port read %02X\n", machine().describe_context(), offset);
		return 0xff;
	}
}

void ncr9010_state::io_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0x01: // CCM indicator
		for (int i = 0; i < 8; i++)
			m_ccm_leds[i] = BIT(data, i);
		break;

	case 0x05: // acknowledge TRAP
		m_trap_cause = 0;
		m_maincpu->set_input_line(I8085_TRAP_LINE, CLEAR_LINE);
		break;

	case 0x04: // memory control
		m_memctrl = data;
		m_map_arm = BIT(data, 2);
		update_views();
		break;

	case 0x2e: // keyboard bell
		m_bell->set_state(1);
		m_bell_timer->adjust(attotime::from_msec(150)); // length not verified
		break;

	case 0x2f: // keyboard reset
		m_kbd_head = m_kbd_count = 0;
		break;

	case 0x28: case 0x29: case 0x2a: case 0x2b:
	case 0x2c: case 0x2d: // keyboard data/control
		break;

	case 0x30: // printer 1 data
		m_printer->output(data);
		break;

	case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		m_fx_dmareg[offset & 3] = data;
		break;

	case 0xc0:
		m_cur_x = data;
		break;
	case 0xc1:
		m_cur_y = data;
		break;
	case 0xc2:
		m_vram[(m_cur_y * 80 + m_cur_x) & 0x7ff] = data;
		if (++m_cur_x >= 80)
		{
			m_cur_x = 0;
			if (++m_cur_y >= 24)
				m_cur_y = 0;
		}
		break;
	case 0xc3:
		m_vattr[(m_cur_y * 80 + m_cur_x) & 0x7ff] = data;
		break;
	case 0xc4:
		m_crt_ctrl = data;
		break;

	default:
		LOGIO("%s: port write %02X = %02X\n", machine().describe_context(), offset, data);
		break;
	}
}


/***************************************************************************
    KEYBOARD
***************************************************************************/

/*
    NCR 4503 keyboard, codes from the "Latin keyboard code chart" of the
    I-8100 pocket reference.  Every key has a control, a shift and an
    unshift code (and a fourth one for non-Latin keyboards, not used here).
    Unshifted letters give UPPER case, shifted ones lower case; SB-80 can
    swap the case of its translation table with control + the top left
    keypad key (code 90h).  FF = no code.  SB-80 translates 80 ESC, 81 ^R,
    82 ^S, 83 ^X, 84 ^E, 85 ^D, 86 TAB, 88 CR, 89 ",", 8A ".", 8B BS,
    8C ^C.  REP repeats the held key.  The lit key left of A is taken to be
    a shift lock (assumption).
*/
struct kb_code { u8 ctrl, shift, unshift; };

constexpr kb_code KB_TABLE[11][8] =
{
	// _/DEL                1                     2                     3                     4                     5                     6                     7
	{ { 0x1f, 0x7f, 0x5f }, { 0xaa, 0x21, 0x31 }, { 0xab, 0x22, 0x32 }, { 0xa3, 0x23, 0x33 }, { 0xa4, 0x24, 0x34 }, { 0xa5, 0x25, 0x35 }, { 0xac, 0x26, 0x36 }, { 0xa7, 0x27, 0x37 } },
	// 8                    9                     0 `                   - =                   ^ ~                   backslash |           backspace
	{ { 0xa8, 0x28, 0x38 }, { 0xa9, 0x29, 0x39 }, { 0xad, 0x60, 0x30 }, { 0x9e, 0x3d, 0x2d }, { 0x1e, 0x7e, 0x5e }, { 0x1c, 0x7c, 0x5c }, { 0x9b, 0x8b, 0x8b }, { 0xff, 0xff, 0xff } },
	// Q                    W                     E                     R                     T                     Y                     U                     I
	{ { 0x11, 0x71, 0x51 }, { 0x17, 0x77, 0x57 }, { 0x05, 0x65, 0x45 }, { 0x12, 0x72, 0x52 }, { 0x14, 0x74, 0x54 }, { 0x19, 0x79, 0x59 }, { 0x15, 0x75, 0x55 }, { 0x09, 0x69, 0x49 } },
	// O                    P                     @ `                   [ {
	{ { 0x0f, 0x6f, 0x4f }, { 0x10, 0x70, 0x50 }, { 0x00, 0x60, 0x40 }, { 0x1b, 0x7b, 0x5b }, { 0xff, 0xff, 0xff }, { 0xff, 0xff, 0xff }, { 0xff, 0xff, 0xff }, { 0xff, 0xff, 0xff } },
	// A                    S                     D                     F                     G                     H                     J                     K
	{ { 0x01, 0x61, 0x41 }, { 0x13, 0x73, 0x53 }, { 0x04, 0x64, 0x44 }, { 0x06, 0x66, 0x46 }, { 0x07, 0x67, 0x47 }, { 0x08, 0x68, 0x48 }, { 0x0a, 0x6a, 0x4a }, { 0x0b, 0x6b, 0x4b } },
	// L                    ; +                   : *                   ] }
	{ { 0x0c, 0x6c, 0x4c }, { 0x2b, 0x2b, 0x3b }, { 0x2a, 0x2a, 0x3a }, { 0x1d, 0x7d, 0x5d }, { 0xff, 0xff, 0xff }, { 0xff, 0xff, 0xff }, { 0xff, 0xff, 0xff }, { 0xff, 0xff, 0xff } },
	// < >                  Z                     X                     C                     V                     B                     N                     M
	{ { 0xff, 0x3e, 0x3c }, { 0x1a, 0x7a, 0x5a }, { 0x18, 0x78, 0x58 }, { 0x03, 0x63, 0x43 }, { 0x16, 0x76, 0x56 }, { 0x02, 0x62, 0x42 }, { 0x0e, 0x6e, 0x4e }, { 0x0d, 0x6d, 0x4d } },
	// ,                    .                     / ?                   space                 FT                    BREAK
	{ { 0x3c, 0x2c, 0x2c }, { 0x3e, 0x2e, 0x2e }, { 0x9f, 0x3f, 0x2f }, { 0x20, 0x20, 0x20 }, { 0x98, 0x88, 0x88 }, { 0x9c, 0x8c, 0x8c }, { 0xff, 0xff, 0xff }, { 0xff, 0xff, 0xff } },
	// keypad: clear        diagonal              left                  down                  up                    right                 C                     7
	{ { 0x90, 0x80, 0x80 }, { 0x91, 0x81, 0x81 }, { 0x92, 0x82, 0x82 }, { 0x93, 0x83, 0x83 }, { 0x94, 0x84, 0x84 }, { 0x95, 0x85, 0x85 }, { 0x96, 0x86, 0x86 }, { 0xa7, 0x37, 0x37 } },
	// 8                    9                     R                     4                     5                     6                     (blank)               1
	{ { 0xa8, 0x38, 0x38 }, { 0xa9, 0x39, 0x39 }, { 0xff, 0x8e, 0x8e }, { 0xa4, 0x34, 0x34 }, { 0xa5, 0x35, 0x35 }, { 0xac, 0x36, 0x36 }, { 0xff, 0x8f, 0x8f }, { 0xaa, 0x31, 0x31 } },
	// 2                    3                     0                     00                    .                     FT (keypad)
	{ { 0xab, 0x32, 0x32 }, { 0xa3, 0x33, 0x33 }, { 0xad, 0x30, 0x30 }, { 0x99, 0x89, 0x89 }, { 0x8a, 0x8a, 0x8a }, { 0x98, 0x88, 0x88 }, { 0xff, 0xff, 0xff }, { 0xff, 0xff, 0xff } }
};

TIMER_CALLBACK_MEMBER(ncr9010_state::kbd_scan)
{
	u8 const mods = m_kbmods->read();
	bool const shift = BIT(mods, 0) || BIT(mods, 3);
	bool const ctrl = BIT(mods, 1);
	bool const rep = BIT(mods, 2);

	auto code_of = [ctrl, shift] (unsigned int key) -> u8
	{
		kb_code const &k = KB_TABLE[key >> 3][key & 7];
		return ctrl ? k.ctrl : shift ? k.shift : k.unshift;
	};

	bool held_last = false;
	for (unsigned int row = 0; row < std::size(m_keys); row++)
	{
		u8 const state = m_keys[row]->read();
		u8 const pressed = state & ~m_kbd_prev[row];
		m_kbd_prev[row] = state;
		for (unsigned int bit = 0; bit < 8; bit++)
		{
			unsigned int const key = row * 8 + bit;
			if (BIT(pressed, bit))
			{
				u8 const code = code_of(key);
				if (code != 0xff)
					kbd_put(code);
				m_kbd_last = key;
				m_kbd_reptime = 0;
			}
			if (BIT(state, bit) && (key == m_kbd_last))
				held_last = true;
		}
	}

	// REP: repeat the last key while both are held (rate not verified)
	if (rep && held_last && (++m_kbd_reptime >= 6))
	{
		m_kbd_reptime = 0;
		u8 const code = code_of(m_kbd_last);
		if ((code != 0xff) && !m_kbd_count)
			kbd_put(code);
	}
}

void ncr9010_state::kbd_put(u8 data)
{
	if (m_kbd_count < std::size(m_kbd_fifo))
	{
		m_kbd_fifo[(m_kbd_head + m_kbd_count) % std::size(m_kbd_fifo)] = data;
		m_kbd_count++;
	}
}


/***************************************************************************
    VIDEO
***************************************************************************/

u32 ncr9010_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const pens = m_palette->pens();
	bool const blink = BIT(screen.frame_number(), 4);

	bitmap.fill(pens[0], cliprect);
	if (!BIT(m_crt_ctrl, 2))
		return 0;

	for (int y = cliprect.min_y / 12; y <= std::min(cliprect.max_y / 12, 23); y++)
	{
		for (int x = 0; x < 80; x++)
		{
			unsigned int const pos = y * 80 + x;
			u8 const chr = m_vram[pos];
			bool const inv = BIT(chr, 7) || (m_vattr[pos] & 0x01);
			bool const cursor = (x == m_cur_x) && (y == m_cur_y) && blink;
			// substitute character generator: 16 lines per character, leftmost dot in bit 0,
			// glyphs in lines 3-14 (see ROM_START)
			u8 const *const gfx = &m_chargen[(chr & 0x7f) * 16 + 3];

			for (int ra = 0; ra < 12; ra++)
			{
				u8 bits = gfx[ra];
				if (inv)
					bits = ~bits;
				if (cursor && (ra == 11))
					bits = 0xff;

				u32 *const p = &bitmap.pix(y * 12 + ra, x * 8);
				for (int b = 0; b < 8; b++)
					p[b] = pens[BIT(bits, b)];
			}
		}
	}

	return 0;
}


/***************************************************************************
    FLEX DISK CONTROLLER

    High level emulation of the I/O Processor and the Flex Disk IF; see
    the notes at the top.  Commands are carried out at once on the flux
    transitions of the floppy device: the HLE steps the drive, reads a
    track, decodes the FM or MFM fields itself and writes data fields or
    whole formatted tracks back.  The controller then stays busy until
    the time the last field has passed under the head.
***************************************************************************/

u8 ncr9010_state::flex_r(offs_t offset)
{
	switch (offset)
	{
	case 0x01:
		return m_fx_status;
	case 0x02:
		return m_fx_status2;
	default:
		if (!machine().side_effects_disabled())
			LOGFLEX("%s: flex read %02X\n", machine().describe_context(), offset);
		return 0xff;
	}
}

void ncr9010_state::flex_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0x01: // command byte
		if (BIT(m_fx_status, 7))
			break;
		m_fx_cmdbuf[m_fx_cmdcnt++] = data;
		if (m_fx_cmdcnt == 4)
		{
			m_fx_cmdcnt = 0;
			flex_command();
		}
		break;

	case 0x02: // interrupt on
		m_fx_inten = true;
		break;

	case 0x03: // interrupt off
		m_fx_inten = false;
		break;

	case 0x04: // reset service request
		m_fx_status &= ~0x40;
		break;

	case 0x07: // reset controller
		m_fx_cmdcnt = 0;
		m_fx_timer->adjust(attotime::never);
		m_fx_status = 0x00;
		break;

	default:
		LOGFLEX("%s: flex write %02X = %02X\n", machine().describe_context(), offset, data);
		break;
	}
}

// step the drive to a cylinder (-1: restore), returns the time needed
attotime ncr9010_state::flex_seek(floppy_image_device &floppy, int cyl)
{
	int steps = 0;
	if (cyl < 0)
	{
		floppy.dir_w(1);
		while (floppy.trk00_r() && (steps < 100))
		{
			floppy.stp_w(0);
			floppy.stp_w(1);
			steps++;
		}
	}
	else
	{
		floppy.dir_w((cyl < floppy.get_cyl()) ? 1 : 0);
		while ((floppy.get_cyl() != cyl) && (steps < 100))
		{
			floppy.stp_w(0);
			floppy.stp_w(1);
			steps++;
		}
	}
	return steps ? (FX_STEP_TIME * steps + FX_SETTLE_TIME) : attotime::zero;
}

// read a little more than one revolution from start
void ncr9010_state::flex_read_track(floppy_image_device &floppy, const attotime &start, fx_track &trk)
{
	attotime const end = start + FX_REV_TIME * 6 / 5;
	trk.clear();
	for (attotime t = floppy.get_next_transition(start); !t.is_never() && (t < end); t = floppy.get_next_transition(t))
		trk.flux(t);
	trk.pad(end);
}

// write cells as flux transitions, cell 0 at time start
void ncr9010_state::flex_write_track(floppy_image_device &floppy, const attotime &start, const fx_track &trk)
{
	attotime const cell = trk.cell_time();
	std::vector<bool> const &cells = trk.cells();
	floppy.write_start(start);
	for (size_t i = 0; i < cells.size(); i++)
	{
		if (cells[i])
			floppy.write_flux_change(start + cell * i);
	}
	floppy.write_end(start + cell * cells.size());
}

void ncr9010_state::flex_command()
{
	m_fx_cmd = m_fx_cmdbuf[0] & 0x0f; // high nibble: ASCII/EBCDIC conversion (not emulated)
	m_fx_unit = BIT(m_fx_cmdbuf[1], 1, 2);
	m_fx_head = BIT(m_fx_cmdbuf[1], 0);
	m_fx_dd = BIT(m_fx_cmdbuf[1], 7);
	m_fx_cyl = m_fx_cmdbuf[2];
	m_fx_sec = m_fx_cmdbuf[3];
	m_fx_addr = m_fx_dmareg[0] | (m_fx_dmareg[1] << 8) | (u32(BIT(m_fx_dmareg[3], 4, 2)) << 16);
	m_fx_count = m_fx_dmareg[2] | ((m_fx_dmareg[3] & 0x0f) << 8);
	if (!m_fx_count)
		m_fx_count = 0x1000;

	LOGFLEX("flex cmd %02X unit %u head %u %s cyl %u sec %u dma %05X count %03X dir %02X\n",
			m_fx_cmd, m_fx_unit, m_fx_head, m_fx_dd ? "MFM" : "FM", m_fx_cyl, m_fx_sec,
			m_fx_addr, m_fx_count, m_fx_dmareg[3] & 0xc0);

	m_fx_status = 0x80;
	m_fx_status2 = 0x00;
	m_hld_timer->adjust(attotime::never);

	floppy_image_device *const floppy = m_floppy[m_fx_unit]->get_device();
	bool const ready = floppy && floppy->exists();
	for (unsigned int i = 0; i < 4; i++)
		m_busy_leds[i] = (i == m_fx_unit) && ready;

	m_fx_result = RES_OK;
	m_fx_result2 = 0;
	attotime done = FX_CMD_TIME;
	if (!ready)
	{
		m_fx_result = RES_NOTREADY;
	}
	else
	{
		floppy->ss_w(m_fx_head);
		switch (m_fx_cmd)
		{
		case 0x00: // restore
			done += flex_seek(*floppy, -1);
			break;

		case 0x01: // write format
			done += flex_seek(*floppy, m_fx_cyl);
			flex_format(*floppy, done);
			break;

		case 0x02: // read format (IDs)
			done += flex_seek(*floppy, m_fx_cyl);
			flex_read_ids(*floppy, done);
			break;

		case 0x03: // write data
		case 0x04: // read data
		case 0x07: // verify
			flex_transfer(*floppy, done);
			break;

		default:
			logerror("flex: unsupported command %02X\n", m_fx_cmd);
			m_fx_result = RES_BADCMD;
			break;
		}
	}

	m_fx_timer->adjust(done);
}

// read, write or verify data; continues with the next sector and the next
// cylinder (same head) until the DMA count is exhausted
void ncr9010_state::flex_transfer(floppy_image_device &floppy, attotime &done)
{
	bool const write = m_fx_cmd == 0x03;
	if (write && floppy.wpt_r())
	{
		flex_error(RES_SOFT, ST2_WFAULT);
		return;
	}

	fx_track trk(m_fx_dd);
	std::vector<fx_sector> sectors;
	bool have_track = false;

	while (m_fx_count)
	{
		if (!have_track)
		{
			done += flex_seek(floppy, m_fx_cyl);
			flex_read_track(floppy, machine().time() + done, trk);
			fx_decode(trk, m_fx_dd, sectors);
			have_track = true;
		}

		fx_sector const *const s = fx_find(sectors, m_fx_cyl, m_fx_head, m_fx_sec);
		if (!s || (s->data_pos < 0))
		{
			flex_error(RES_SOFT, s ? ST2_NOMARK : ST2_RNF);
			done += FX_REV_TIME * 2; // the controller gives up after two revolutions
			return;
		}

		// wait until the sector comes round (more turns for an interleaved track)
		attotime wait = attotime::zero;
		while (trk.time_of(s->id_pos) + wait < machine().time() + done)
			wait += FX_REV_TIME;

		unsigned int const size = 128 << (s->n & 3);
		unsigned int const n = std::min<unsigned int>(size, m_fx_count);
		int const body = s->data_pos + (m_fx_dd ? 64 : 16);

		if (write)
		{
			// rewrite the data field from the zero bytes before the mark on
			fx_track field(m_fx_dd);
			int const from = s->data_pos - (m_fx_dd ? 12 : 6) * 16;
			field.bytes(0x00, m_fx_dd ? 12 : 6);
			u16 crc = field.mark(0xfb);
			for (unsigned int i = 0; i < size; i++)
			{
				u8 const data = (i < n) ? dma_r(m_fx_addr + i) : 0x00;
				field.byte(data);
				crc = fx_crc(crc, data);
			}
			field.byte(crc >> 8);
			field.byte(crc & 0xff);
			field.bytes(m_fx_dd ? 0x4e : 0xff, 1);
			flex_write_track(floppy, trk.time_of(from) + wait, field);
		}
		else
		{
			u16 crc = 0xffff;
			if (m_fx_dd)
			{
				for (int i = 0; i < 3; i++)
					crc = fx_crc(crc, 0xa1);
			}
			crc = fx_crc(crc, s->data_mark);
			for (unsigned int i = 0; i < size + 2; i++)
			{
				u8 const data = trk.get_byte(body + i * 16);
				if ((i < n) && (m_fx_cmd == 0x04))
					dma_w(m_fx_addr + i, data);
				crc = fx_crc(crc, data);
			}
			if (s->data_mark == 0xf8)
				m_fx_status |= 0x20; // deleted data mark
			if (crc)
			{
				flex_error((m_fx_cmd == 0x07) ? RES_VERIFY : RES_SOFT, ST2_CRC);
				return;
			}
		}

		// busy until the end of this field
		done = trk.time_of(body + (size + 2) * 16) + wait - machine().time();

		m_fx_addr += n;
		m_fx_count -= n;
		if (!m_fx_count)
			break;

		if (++m_fx_sec > SECTORS_PER_TRACK)
		{
			if (m_fx_cyl >= 73)
			{
				// the controller stops at track 73 sector 26
				flex_error(RES_BEYOND);
				return;
			}
			m_fx_sec = 1;
			m_fx_cyl++;
			have_track = false;
		}
	}
}

// DMA receives C, H, R, N of the sectors in the order they pass the head after the index
void ncr9010_state::flex_read_ids(floppy_image_device &floppy, attotime &done)
{
	attotime start = floppy.time_next_index();
	if (start.is_never())
	{
		flex_error(RES_NOTREADY);
		return;
	}
	while (start < machine().time() + done)
		start += FX_REV_TIME;

	fx_track trk(m_fx_dd);
	std::vector<fx_sector> sectors;
	flex_read_track(floppy, start, trk);
	fx_decode(trk, m_fx_dd, sectors);

	unsigned int found = 0;
	for (fx_sector const &s : sectors)
	{
		if (!m_fx_count || (found == SECTORS_PER_TRACK))
			break;
		if (!s.id_ok)
			continue;
		u8 const id[4] = { s.c, s.h, s.r, s.n };
		for (unsigned int i = 0; (i < 4) && m_fx_count; i++, m_fx_count--)
			dma_w(m_fx_addr++, id[i]);
		found++;
	}

	if (!found)
		flex_error(RES_SOFT, ST2_RNF);
	done = start + FX_REV_TIME - machine().time();
}

// write a whole track from the index on; DMA supplies C, H, R, N of the 26 sectors
void ncr9010_state::flex_format(floppy_image_device &floppy, attotime &done)
{
	if (floppy.wpt_r())
	{
		flex_error(RES_SOFT, ST2_WFAULT);
		return;
	}

	attotime start = floppy.time_next_index();
	if (start.is_never())
	{
		flex_error(RES_NOTREADY);
		return;
	}
	while (start < machine().time() + done)
		start += FX_REV_TIME;

	u8 ids[SECTORS_PER_TRACK * 4];
	for (unsigned int i = 0; i < std::size(ids); i++)
		ids[i] = dma_r(m_fx_addr + i);
	m_fx_count = 0;

	// IBM System 34 (MFM) or 3740 (FM) layout
	bool const dd = m_fx_dd;
	u8 const gap = dd ? 0x4e : 0xff;
	fx_track trk(dd);
	trk.bytes(gap, dd ? 80 : 40);
	trk.bytes(0x00, dd ? 12 : 6);
	trk.mark(0xfc);
	trk.bytes(gap, dd ? 50 : 26);
	for (unsigned int s = 0; s < SECTORS_PER_TRACK; s++)
	{
		u8 const *const id = &ids[s * 4];
		trk.bytes(0x00, dd ? 12 : 6);
		u16 crc = trk.mark(0xfe);
		for (int i = 0; i < 4; i++)
		{
			trk.byte(id[i]);
			crc = fx_crc(crc, id[i]);
		}
		trk.byte(crc >> 8);
		trk.byte(crc & 0xff);
		trk.bytes(gap, dd ? 22 : 11);
		trk.bytes(0x00, dd ? 12 : 6);
		crc = trk.mark(0xfb);
		for (unsigned int i = 0; i < (128U << (id[3] & 3)); i++)
		{
			trk.byte(0xe5);
			crc = fx_crc(crc, 0xe5);
		}
		trk.byte(crc >> 8);
		trk.byte(crc & 0xff);
		trk.bytes(gap, dd ? 54 : 27);
	}

	// gap 4b up to just before the next index (a full turn would be an empty span)
	size_t const total = FX_REV_TIME.as_attoseconds() / trk.cell_time().as_attoseconds() - 16;
	while (trk.cells().size() < total)
		trk.byte(gap);
	trk.cells().resize(total);

	flex_write_track(floppy, start, trk);
	done = start + FX_REV_TIME - machine().time();
}

void ncr9010_state::flex_error(u8 result, u8 status2)
{
	m_fx_result = result;
	m_fx_result2 = status2;
}

TIMER_CALLBACK_MEMBER(ncr9010_state::flex_done)
{
	LOGFLEX("flex result %02X/%02X\n", m_fx_result, m_fx_result2);
	m_fx_status2 = m_fx_result2;
	m_fx_status = (m_fx_status & 0x20) | 0x40 | m_fx_result;
	m_hld_timer->adjust(attotime::from_msec(300)); // head unload delay
}

TIMER_CALLBACK_MEMBER(ncr9010_state::head_unload)
{
	if (!BIT(m_fx_status, 7))
	{
		for (auto &led : m_busy_leds)
			led = 0;
	}
}


/***************************************************************************
    FRONT PANEL
***************************************************************************/

// READY lamps follow the drives' ready lines (also for disks created at run time)
void ncr9010_state::floppy_ready(floppy_image_device *floppy, int state)
{
	for (unsigned int i = 0; i < 4; i++)
	{
		floppy_image_device *const f = m_floppy[i]->get_device();
		m_ready_leds[i] = f && !f->ready_r();
	}
}

INPUT_CHANGED_MEMBER(ncr9010_state::monit_pressed)
{
	if (newval && !BIT(m_trap_cause, 6))
	{
		m_trap_cause |= 0x40;
		m_maincpu->set_input_line(I8085_TRAP_LINE, ASSERT_LINE);
	}
}

IRQ_CALLBACK_MEMBER(ncr9010_state::irq_ack)
{
	// assumed: taking TRAP switches the boot ROM in (its handler is at 0024h)
	if (irqline == I8085_TRAP_LINE)
		m_memctrl |= 0x01;
	return 0xff;
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( ncr9010 )
	// NCR 4503 keyboard (see KB_TABLE), PC keys chosen by position
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('_') PORT_CHAR(0x7f)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("FT (Return)") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BREAK") PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad Clear (ESC)") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad Diagonal Arrow") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad C (TAB)") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad R") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad (blank)") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))

	PORT_START("KEY10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 00") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad FT") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBMODS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("REP") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MODE")
	PORT_DIPNAME( 0x80, 0x80, "Mode" )
	PORT_DIPSETTING(    0x80, "Normal" )
	PORT_DIPSETTING(    0x00, "ROM test (selected by CONFIG)" )
	PORT_DIPNAME( 0x04, 0x00, "Start" )
	PORT_DIPSETTING(    0x00, "Memory test, keep memory" )
	PORT_DIPSETTING(    0x04, "Clear memory / RESTORE? prompt" )
	PORT_BIT( 0x7b, 0x00, IPT_UNUSED )

	PORT_START("PANEL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("MONIT") PORT_CODE(KEYCODE_MENU) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ncr9010_state::monit_pressed), 0)

	// configuration switches, names after the I-8100 pocket reference
	PORT_START("CONFIG")
	PORT_DIPNAME( 0x01, 0x01, "CRT" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "9-inch (32 x 16)" )
	PORT_DIPSETTING(    0x01, "12-inch (80 x 24)" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW1:2" )
	PORT_DIPNAME( 0x04, 0x04, "Flexible disk is primary load device" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Fixed disk is primary load device" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "Data cartridge is primary load device" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Battery back-up for CCM memory" ) PORT_DIPLOCATION("SW2:2") // enables the RESTORE? prompt
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:3" )
	PORT_DIPNAME( 0x80, 0x00, "Fixed disk unit (fixed disk load device only)" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, "Disk 0" )
	PORT_DIPSETTING(    0x80, "Disk 1" )
INPUT_PORTS_END


/***************************************************************************
    MACHINE
***************************************************************************/

void ncr9010_state::machine_start()
{
	for (int i = 0; i < 16; i++)
		m_bank[i]->configure_entries(0, 64, &m_ram[0], 0x1000);

	m_fx_timer = timer_alloc(FUNC(ncr9010_state::flex_done), this);
	m_hld_timer = timer_alloc(FUNC(ncr9010_state::head_unload), this);
	m_bell_timer = timer_alloc(FUNC(ncr9010_state::bell_off), this);
	m_kbd_timer = timer_alloc(FUNC(ncr9010_state::kbd_scan), this);
	m_kbd_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));

	for (auto &f : m_floppy)
	{
		if (f->get_device())
		{
			f->get_device()->setup_ready_cb(floppy_image_device::ready_cb(&ncr9010_state::floppy_ready, this));
		}
	}

	m_power_led = 1;
	m_ccm_run = 1;
	m_comm_led = 0;

	std::fill(std::begin(m_vram), std::end(m_vram), 0x20);
	std::fill(std::begin(m_vattr), std::end(m_vattr), 0x00);

	// flex disk controller state (also set up again by machine_reset)
	std::fill(std::begin(m_fx_cmdbuf), std::end(m_fx_cmdbuf), 0);
	m_fx_cmdcnt = 0;
	m_fx_status = m_fx_status2 = 0;
	m_fx_cmd = m_fx_unit = m_fx_head = m_fx_cyl = m_fx_sec = 0;
	m_fx_dd = false;
	std::fill(std::begin(m_fx_dmareg), std::end(m_fx_dmareg), 0);
	m_fx_addr = 0;
	m_fx_count = 0;
	m_fx_inten = false;
	m_fx_result = m_fx_result2 = 0;

	save_item(NAME(m_map));
	save_item(NAME(m_memctrl));
	save_item(NAME(m_map_arm));
	save_item(NAME(m_trap_cause));
	save_item(NAME(m_vram));
	save_item(NAME(m_vattr));
	save_item(NAME(m_cur_x));
	save_item(NAME(m_cur_y));
	save_item(NAME(m_crt_ctrl));
	save_item(NAME(m_kbd_fifo));
	save_item(NAME(m_kbd_head));
	save_item(NAME(m_kbd_count));
	save_item(NAME(m_kbd_prev));
	save_item(NAME(m_kbd_last));
	save_item(NAME(m_kbd_reptime));
	save_item(NAME(m_fx_cmdbuf));
	save_item(NAME(m_fx_cmdcnt));
	save_item(NAME(m_fx_status));
	save_item(NAME(m_fx_status2));
	save_item(NAME(m_fx_cmd));
	save_item(NAME(m_fx_unit));
	save_item(NAME(m_fx_head));
	save_item(NAME(m_fx_cyl));
	save_item(NAME(m_fx_sec));
	save_item(NAME(m_fx_dd));
	save_item(NAME(m_fx_dmareg));
	save_item(NAME(m_fx_addr));
	save_item(NAME(m_fx_count));
	save_item(NAME(m_fx_inten));
	save_item(NAME(m_fx_result));
	save_item(NAME(m_fx_result2));
}

void ncr9010_state::machine_reset()
{
	// MMU maps come up as identity (the ROM uses 3F00h-3FFFh before loading them)
	for (auto &set : m_map)
	{
		for (unsigned int i = 0; i < 16; i++)
			set[i] = i;
	}
	for (int i = 0; i < 16; i++)
		m_bank[i]->set_entry(i);
	m_memctrl = 0x01;
	m_map_arm = false;
	update_views();
	m_trap_cause = 0;
	m_maincpu->set_input_line(I8085_TRAP_LINE, CLEAR_LINE);
	for (auto &led : m_busy_leds)
		led = 0;

	m_cur_x = m_cur_y = 0;
	m_crt_ctrl = 0;
	m_kbd_head = m_kbd_count = 0;
	std::fill(std::begin(m_kbd_prev), std::end(m_kbd_prev), 0);
	m_kbd_last = 0xff;
	m_kbd_reptime = 0;

	m_fx_cmdcnt = 0;
	m_fx_status = 0x00;
	m_fx_status2 = 0x00;
	m_fx_inten = false;
	m_fx_timer->adjust(attotime::never);
	std::fill(std::begin(m_fx_dmareg), std::end(m_fx_dmareg), 0);

	floppy_ready(nullptr, 0);
}

void ncr9010_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_IMD_FORMAT);
}

static void ncr9010_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

void ncr9010_state::ncr9010(machine_config &config)
{
	I8085A(config, m_maincpu, 27.648_MHz_XTAL / 4); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &ncr9010_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ncr9010_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(ncr9010_state::irq_ack));

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_color(rgb_t::green());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(640, 288);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(ncr9010_state::screen_update));
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// I/O Processor + Flex Disk IF: HLE in this driver
	FLOPPY_CONNECTOR(config, m_floppy[0], ncr9010_floppies, "8dsdd", ncr9010_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], ncr9010_floppies, "8dsdd", ncr9010_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[2], ncr9010_floppies, nullptr, ncr9010_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[3], ncr9010_floppies, nullptr, ncr9010_state::floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_original("ncr9010_flop");

	PRINTER(config, m_printer, 0);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_bell, 1000).add_route(ALL_OUTPUTS, "mono", 0.5); // frequency not verified

	config.set_default_layout(layout_ncr9010);
}


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( ncr9010 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_INVERT ) // data lines inverted
	ROM_LOAD( "1023.j1", 0x0000, 0x0800, CRC(072a6f95) SHA1(8502f68640a8ecd7529632ca369f78181b3cf35a) )
	ROM_LOAD( "1024.f1", 0x0800, 0x0800, CRC(1f336b9a) SHA1(0c36d83bf27e99fef25f573c2df41dad370e9a08) )

	// The real character generator (Intel D2608 mask ROM "6-1043026", 1 KB) is not dumped,
	// neither its layout nor its glyphs are known.  As a substitute this loads the character
	// generator of the NCR Decision Mate V (76161, also in the DMV boot ROM at 1000h), the
	// internal successor of the 9010: 128 characters x 16 lines, leftmost dot in bit 0.
	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "6-1043026.q5", 0x000, 0x800, BAD_DUMP CRC(6e4df4f9) SHA1(20ff4fc48e55eaf5131f6573fff93e7f97d2f45d) ) // substitute: DMV 76161

	ROM_REGION( 0x400, "chargen2", 0 ) // video board, purpose unknown (second character set?)
	ROM_LOAD( "9709.r5", 0x000, 0x400, NO_DUMP ) // type hidden under label, size unknown

	ROM_REGION( 0x20, "vprom", 0 )
	ROM_LOAD( "6331.d2", 0x00, 0x20, NO_DUMP )

	ROM_REGION( 0xa00, "iop", 0 ) // I/O Processor microcode, MMI 6341 (512 x 8), locations read from photos
	ROM_LOAD( "1117.j1", 0x000, 0x200, NO_DUMP )
	ROM_LOAD( "1118.k1", 0x200, 0x200, NO_DUMP )
	ROM_LOAD( "1119.l1", 0x400, 0x200, NO_DUMP )
	ROM_LOAD( "1120.m1", 0x600, 0x200, NO_DUMP )
	ROM_LOAD( "1122.h1", 0x800, 0x200, NO_DUMP )

	ROM_REGION( 0x400, "flexif", 0 ) // Flex Disk IF, type and size unknown
	ROM_LOAD( "502.c3", 0x000, 0x200, NO_DUMP )
	ROM_LOAD( "177.h4", 0x200, 0x100, NO_DUMP )
ROM_END

} // anonymous namespace


//    YEAR   NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME     FLAGS
COMP( 1981?, ncr9010, 0,      0,      ncr9010, ncr9010, ncr9010_state, empty_init, "NCR",   "NCR 9010",  MACHINE_NOT_WORKING )
