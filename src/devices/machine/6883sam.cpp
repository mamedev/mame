// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    6883sam.cpp

    Motorola 6883 Synchronous Address Multiplexer

    The Motorola 6883 SAM has 16 bits worth of state, but the state is changed
    by writing into a 32 byte address space; odd bytes set bits and even bytes
    clear bits.  Here is the layout:

        31  Set     TY  Map Type            0: RAM/ROM  1: All RAM
        30  Clear   TY  Map Type
        29  Set     M1  Memory Size         00: 4K      10: 64K Dynamic
        28  Clear   M1  Memory Size         01: 16K     11: 64K Static
        27  Set     M0  Memory Size
        26  Clear   M0  Memory Size
        25  Set     R1  MPU Rate            00: Slow    10: Fast
        24  Clear   R1  MPU Rate            01: A.D.    11: Fast
        23  Set     R0  MPU Rate
        22  Clear   R0  MPU Rate
        21  Set     P1  Page #1             0: Low      1: High
        20  Clear   P1  Page #1
        19  Set     F6  Display Offset
        18  Clear   F6  Display Offset
        17  Set     F5  Display Offset
        16  Clear   F5  Display Offset
        15  Set     F4  Display Offset
        14  Clear   F4  Display Offset
        13  Set     F3  Display Offset
        12  Clear   F3  Display Offset
        11  Set     F2  Display Offset
        10  Clear   F2  Display Offset
         9  Set     F1  Display Offset
         8  Clear   F1  Display Offset
         7  Set     F0  Display Offset
         6  Clear   F0  Display Offset
         5  Set     V2  VDG Mode
         4  Clear   V2  VDG Mode
         3  Set     V1  VDG Mode
         2  Clear   V1  VDG Mode
         1  Set     V0  VDG Mode
         0  Clear   V0  VDG Mode

    The SAM installs the host's ROM and I/O maps directly in the CPU's
    program space, and maps RAM over $0000-$7FFF and $8000-$FEFF with memory
    views selected by TY, M1 and P1.

    All parts of the SAM are fully emulated except R1/R0 (the changes in the
    MPU rate are approximated; the loss of VDG and refresh cycles in fast
    mode is not emulated) and M1/M0 (CPU accesses do not alias or lose RAM
    when the memory size does not match the installed RAM)

***************************************************************************/


#include "emu.h"
#include "6883sam.h"

#include <algorithm>
#include <utility>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define LOG_FBITS   (1U << 1)
#define LOG_VBITS   (1U << 2)
#define LOG_PBITS   (1U << 3)
#define LOG_TBITS   (1U << 4)
#define LOG_MBITS   (1U << 5)
#define LOG_RBITS   (1U << 6)

#define VERBOSE (0)
// #define VERBOSE (LOG_FBITS)
// #define VERBOSE (LOG_FBITS | LOG_VBITS | LOG_PBITS | LOG_TBITS | LOG_MBITS | LOG_RBITS)

#include "logmacro.h"

#define LOGFBITS(...) LOGMASKED(LOG_FBITS, __VA_ARGS__)
#define LOGVBITS(...) LOGMASKED(LOG_VBITS, __VA_ARGS__)
#define LOGPBITS(...) LOGMASKED(LOG_PBITS, __VA_ARGS__)
#define LOGTBITS(...) LOGMASKED(LOG_TBITS, __VA_ARGS__)
#define LOGMBITS(...) LOGMASKED(LOG_MBITS, __VA_ARGS__)
#define LOGRBITS(...) LOGMASKED(LOG_RBITS, __VA_ARGS__)

DEFINE_DEVICE_TYPE(SAM6883, sam6883_device, "sam6883", "MC6883 SAM")



//**************************************************************************
//  DEVICE SETUP
//**************************************************************************

//-------------------------------------------------
//  constructor
//-------------------------------------------------

sam6883_device::sam6883_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SAM6883, tag, owner, clock)
	, sam6883_friend_device_interface(mconfig, *this, 4)
	, m_ram(*this, finder_base::DUMMY_TAG)
	, m_low_view(*this, "low_view")
	, m_high_view(*this, "high_view")
{
	std::fill(std::begin(m_ram_page_bank), std::end(m_ram_page_bank), -1);
}

sam6883_friend_device_interface::sam6883_friend_device_interface(const machine_config &mconfig, device_t &device, int divider)
	: device_interface(device, "sam6883")
	, m_cpu(device, finder_base::DUMMY_TAG)
	, m_sam_state(0x0000)
	, m_divider(divider)
	, m_video_flush_cb(device)
{
}


//-------------------------------------------------
//  set_addrmap - set one of the ROM or I/O maps
//-------------------------------------------------

void sam6883_device::set_addrmap(int select, address_map_constructor map)
{
	if ((select < 1) || (select > 7))
		throw emu_fatalerror("%s: SAM map %d does not exist\n", tag(), select);
	m_maps[select] = std::move(map);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam6883_device::device_start()
{
	if (m_ram.found() && !m_ram->started())
		throw device_missing_dependencies();

	// writes to $FFC0-$FFDF also reach the BOOT map
	m_cpu->space(AS_PROGRAM).install_write_tap(0xffc0, 0xffdf, "sam_control",
			[this] (offs_t offset, uint8_t &data, uint8_t mem_mask) { internal_write(offset & 0x1f, data); });

	if (m_ram.found())
		map_ram(0x0000, m_ram->size() - 1, m_ram->pointer(), m_ram->pointer());
	install_ram_views();

	// Install once: RAM remapping changes the page pointers, not this write handler.
	m_high_write_tap = m_high_view[0].install_write_tap(0x8000, 0xfeff, "sam_ram_write",
			[this] (offs_t offset, uint8_t &data, uint8_t mem_mask)
			{
				uint8_t *const page = m_ram_write[offset >> 8];
				if (page)
					page[offset & 0xff] = data;
			});

	// save state support
	save_item(NAME(m_sam_state));
	save_item(NAME(m_divider));
	save_item(NAME(m_counter_mask));
	save_item(NAME(m_counter));
	save_item(NAME(m_counter_xdiv));
	save_item(NAME(m_counter_ydiv));
	save_item(NAME(m_counter_bit4_input));
	save_item(NAME(m_counter_bit5_input));
	save_item(NAME(m_ad_base));
	save_item(NAME(m_ad_slow_end));
}


//-------------------------------------------------
//  device_stop - release view handlers while the
//  CPU's tap bookkeeping is still alive
//-------------------------------------------------

void sam6883_device::device_stop()
{
	m_ad_tap.remove();
	m_ad_tap_installed = false;
	m_low_view[0].unmap_readwrite(0x0000, 0x7fff);
	m_low_view[1].unmap_readwrite(0x0000, 0x7fff);
	m_high_view[0].unmap_readwrite(0x8000, 0xfeff);
}


//-------------------------------------------------
//  amap - the CPU's view of the SAM's address
//  space
//-------------------------------------------------

void sam6883_device::amap(address_map &map)
{
	// configured maps are bound relative to the SAM's owner
	auto const host_map = [this, &map] (int select, offs_t start, offs_t end)
	{
		if (!m_maps[select].isnull())
			map(start, end).m(owner(), m_maps[select]);
	};

	// ROM1 also supplies the vectors at $FFE0-$FFFF (ROM1 offsets $1FE0-$1FFF);
	// unmap the rest so gaps in the other select windows stay unmapped
	host_map(2, 0xe000, 0xffff);
	map(0xe000, 0xffdf).unmaprw();
	host_map(1, 0x8000, 0x9fff);
	host_map(2, 0xa000, 0xbfff);
	host_map(3, 0xc000, 0xfeff);
	host_map(4, 0xff00, 0xff1f);
	host_map(5, 0xff20, 0xff3f);
	host_map(6, 0xff40, 0xff5f);
	host_map(7, 0xff60, 0xffdf);

	// RAM is installed in the views at start; map type 0 leaves the ROM maps
	// visible in $8000-$FEFF
	map(0x0000, 0x7fff).view(m_low_view);
	m_low_view[0](0x0000, 0x7fff).noprw();
	m_low_view[1](0x0000, 0x7fff).noprw();
	map(0x8000, 0xfeff).view(m_high_view);
	m_high_view[0](0x8000, 0xfeff).nopr();
}


//-------------------------------------------------
//  map_ram - point physical RAM at memory
//-------------------------------------------------

void sam6883_device::map_ram(offs_t start, offs_t end, uint8_t *read, uint8_t *write)
{
	check_ram_range(start, end);
	set_ram_pages(start, end, read, write);

	if (started())
		install_ram_views();
}


//-------------------------------------------------
//  configure_ram_bank - set up switchable
//  physical RAM, installed once as memory banks
//-------------------------------------------------

void sam6883_device::configure_ram_bank(offs_t start, offs_t end, std::vector<std::pair<uint8_t *, uint8_t *> > entries)
{
	check_ram_range(start, end);
	if (entries.empty() || (m_ram_banks.size() >= 127))
		throw emu_fatalerror("%s: bad RAM bank $%04X-$%04X\n", tag(), start, end);
	for (auto const &entry : entries)
	{
		if (!entry.first || !entry.second)
			throw emu_fatalerror("%s: RAM bank $%04X-$%04X has a null entry\n", tag(), start, end);
	}

	int const index = m_ram_banks.size();
	ram_bank &bank = m_ram_banks.emplace_back();
	bank.start = start;
	bank.end = end;
	bank.read = machine().memory().bank_alloc(*this, util::string_format("rambank%d_r", index));
	bank.write = machine().memory().bank_alloc(*this, util::string_format("rambank%d_w", index));
	for (int i = 0; i < entries.size(); i++)
	{
		bank.read->configure_entry(i, entries[i].first);
		bank.write->configure_entry(i, entries[i].second);
	}
	bank.entries = std::move(entries);

	for (offs_t page = start >> 8; page <= (end >> 8); page++)
		m_ram_page_bank[page] = index;
	select_ram_bank(start, 0);

	if (started())
		install_ram_views();
}


//-------------------------------------------------
//  select_ram_bank - switch a configured range
//  of physical RAM
//-------------------------------------------------

void sam6883_device::select_ram_bank(offs_t start, int entry)
{
	int const index = m_ram_page_bank[(start >> 8) & 0xff];
	if ((index < 0) || (m_ram_banks[index].start != start) || (entry < 0) || (entry >= m_ram_banks[index].entries.size()))
		throw emu_fatalerror("%s: RAM bank $%04X entry %d does not exist\n", tag(), start, entry);

	ram_bank &bank = m_ram_banks[index];
	bank.read->set_entry(entry);
	bank.write->set_entry(entry);
	set_ram_pages(bank.start, bank.end, bank.entries[entry].first, bank.entries[entry].second);
}


//-------------------------------------------------
//  check_ram_range - validate a physical RAM
//  range
//-------------------------------------------------

void sam6883_device::check_ram_range(offs_t start, offs_t end)
{
	if ((start & 0xff) || ((end & 0xff) != 0xff) || (end < start) || (end > 0xffff))
		throw emu_fatalerror("%s: RAM range $%04X-$%04X is not 256 byte aligned\n", tag(), start, end);

	for (offs_t page = start >> 8; page <= (end >> 8); page++)
	{
		if (m_ram_page_bank[page] >= 0)
			throw emu_fatalerror("%s: RAM range $%04X-$%04X overlaps a RAM bank\n", tag(), start, end);
	}
}


//-------------------------------------------------
//  set_ram_pages - update the page tables used
//  by the display and RAM write tap
//-------------------------------------------------

void sam6883_device::set_ram_pages(offs_t start, offs_t end, uint8_t *read, uint8_t *write)
{
	for (offs_t page = start >> 8; page <= (end >> 8); page++)
	{
		offs_t const offset = (page << 8) - start;
		m_ram_read[page] = read ? (read + offset) : nullptr;
		m_ram_write[page] = write ? (write + offset) : nullptr;
	}
}


//-------------------------------------------------
//  install_ram - install physical RAM in a view
//  entry, merging contiguous pages
//-------------------------------------------------

void sam6883_device::install_ram(memory_view::memory_view_entry &entry, offs_t start, offs_t end, offs_t physical, bool writable)
{
	for (int write = 0; write < (writable ? 2 : 1); write++)
	{
		uint8_t *const *const pages = write ? m_ram_write : m_ram_read;
		offs_t address = start;
		while (address <= end)
		{
			offs_t const page = (physical + address - start) >> 8;
			int const bank_index = m_ram_page_bank[page];
			if (bank_index >= 0)
			{
				// switchable RAM: install the whole range as a bank
				ram_bank const &bank = m_ram_banks[bank_index];
				offs_t const last = address + (bank.end - bank.start);
				if (((page << 8) != bank.start) || (last > end))
					throw emu_fatalerror("%s: RAM bank $%04X-$%04X straddles a view\n", tag(), bank.start, bank.end);
				if (write)
					entry.install_write_bank(address, last, bank.write);
				else
					entry.install_read_bank(address, last, bank.read);
				address = last + 1;
				continue;
			}

			uint8_t *const base = pages[page];
			offs_t last = std::min<offs_t>(address | 0xff, end);
			while ((last < end)
					&& (m_ram_page_bank[(physical + last + 1 - start) >> 8] < 0)
					&& (pages[(physical + last + 1 - start) >> 8] == (base ? (base + (last + 1 - address)) : nullptr)))
				last = std::min<offs_t>(last + 0x100, end);

			if (write)
			{
				if (base)
					entry.install_writeonly(address, last, base);
				else
					entry.nop_write(address, last);
			}
			else
			{
				if (base)
					entry.install_rom(address, last, base);
				else
					entry.nop_read(address, last);
			}
			address = last + 1;
		}
	}
}


//-------------------------------------------------
//  install_ram_views - (re)install physical RAM
//  in the CPU's program space
//-------------------------------------------------

void sam6883_device::install_ram_views()
{
	// $0000-$7FFF, with P1 standing in for A15 in map type 0
	install_ram(m_low_view[0], 0x0000, 0x7fff, 0x0000, true);
	install_ram(m_low_view[1], 0x0000, 0x7fff, 0x8000, true);

	// $8000-$FEFF in map type 1: RAM reads, and writes reach both RAM and the
	// ROM maps (S is 1-3 for writes)
	install_ram(m_high_view[0], 0x8000, 0xfeff, 0x8000, false);

	// taps over the views do not survive reinstalling their contents
	if (m_ad_tap_installed)
	{
		m_ad_tap.remove();
		m_ad_tap_installed = false;
		update_ad_tap();
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sam6883_device::device_reset()
{
	counter_reset(0);
	m_sam_state = 0x0000;
	update_state();
}



//-------------------------------------------------
//  device_post_load - device-specific post load
//-------------------------------------------------

void sam6883_device::device_post_load()
{
	device_t::device_post_load();
	update_state();
}



//-------------------------------------------------
//  update_state
//-------------------------------------------------

void sam6883_device::update_state()
{
	update_memory();
	update_cpu_clock();
	update_ad_tap();
}



//-------------------------------------------------
//  update_memory
//-------------------------------------------------

void sam6883_device::update_memory()
{
	// Memory size - allowed restricting memory accesses to something less than
	// 32k
	//
	// This was a SAM switch that occupied 4 addresses:
	//
	//      $FFDD   (set)   R1
	//      $FFDC   (clear) R1
	//      $FFDB   (set)   R0
	//      $FFDA   (clear) R0
	//
	// R1:R0 formed the following states:
	//      00  - 4k
	//      01  - 16k
	//      10  - 64k
	//      11  - static RAM (??)
	//
	// If something less than 64k was set, the low RAM would be smaller and
	// mirror the other parts of the RAM
	//
	// TODO:  Find out what "static RAM" is
	// TODO:  This should affect _all_ memory accesses, not just video ram
	// TODO:  Verify that the CoCo 3 ignored this

	// In the 64K sizes, map type 1 maps RAM over $8000-$FEFF and map type 0
	// uses P1 in place of A15
	bool const mode_64k = (m_sam_state & SAM_STATE_M1) == SAM_STATE_M1;
	m_low_view.select((mode_64k && ((m_sam_state & (SAM_STATE_TY | SAM_STATE_P1)) == SAM_STATE_P1)) ? 1 : 0);
	if (mode_64k && (m_sam_state & SAM_STATE_TY))
		m_high_view.select(0);
	else
		m_high_view.disable();

	// switch depending on the M1/M0 variables
	switch(m_sam_state & (SAM_STATE_M1|SAM_STATE_M0))
	{
		case 0:
			// 4K mode
			m_counter_mask = 0x0FFF;
			break;

		case SAM_STATE_M0:
			// 16K mode
			m_counter_mask = 0x3FFF;
			break;

		case SAM_STATE_M1:
			// 64k mode (dynamic)
		case SAM_STATE_M1|SAM_STATE_M0:
			// 64k mode (static)
			// full 64k RAM or ROM/RAM
			// CoCo Max requires these two be treated the same
			m_counter_mask = 0xfFFF;
			break;
	}
}



//-------------------------------------------------
//  update_cpu_clock - adjusts the speed of the CPU
//  clock
//-------------------------------------------------

void sam6883_friend_device_interface::update_cpu_clock()
{
	// The infamous speed up poke.
	//
	// This was a SAM switch that occupied 4 addresses:
	//
	//      $FFD9   (set)   R1
	//      $FFD8   (clear) R1
	//      $FFD7   (set)   R0
	//      $FFD6   (clear) R0
	//
	// R1:R0 formed the following states:
	//      00  - slow          0.89 MHz (crystal / 16)
	//      01  - address dependent
	//      1x  - fast          1.78 MHz (crystal / 8)
	//
	// In address dependent mode, $0000-$7FFF and $FF00-$FF1F are accessed
	// at the slow rate and all other addresses at the fast rate.  The CPU
	// is clocked at the fast rate and the MC6883 stretches slow accesses
	// (see sam6883_device::address_dependent_cycle).
	//
	// Fast mode steals the time normally used for VDG and refresh access.
	// On pre-CoCo 3 machines, setting R1 caused the screen to display
	// garbage because the M6847 is not given access to RAM.

	int speed = (m_sam_state & (SAM_STATE_R1|SAM_STATE_R0)) / SAM_STATE_R0;
	m_cpu->owner()->set_unscaled_clock(device().clock() / (m_divider * (speed ? 2 : 4)));
}



//-------------------------------------------------
//  address_dependent_cycle - time one MPU bus
//  cycle in address dependent rate mode
//-------------------------------------------------

// The CPU is clocked at the fast rate (8 oscillator periods per cycle), and
// the MPU calls here once per bus cycle, dead cycles included, after the
// cycle has been counted.  Measured on a CoCo 1 (MC6883) and a CoCo 2B
// (SN74LS785):
//  - a slow access starts on a slow cycle boundary (16 oscillator periods)
//  - the cycle following a slow access also lasts a full slow period

void sam6883_device::address_dependent_cycle(bool slow)
{
	uint64_t start = m_cpu->total_cycles() - 1;
	int extra = 0;

	if (slow)
	{
		if ((start - m_ad_base) & 1)
			extra++;
		extra++;
		m_ad_slow_end = start + extra + 1;
	}
	else if (start == m_ad_slow_end)
	{
		extra++;
	}

	if (extra)
		m_cpu->adjust_icount(-extra);
}



//-------------------------------------------------
//  update_ad_tap - time every access while the
//  address dependent rate is selected
//-------------------------------------------------

void sam6883_device::update_ad_tap()
{
	bool const rate_ad = (m_sam_state & (SAM_STATE_R1|SAM_STATE_R0)) == SAM_STATE_R0;
	if (rate_ad && !m_ad_tap_installed)
	{
		auto const tap = [this] (offs_t offset, uint8_t &data, uint8_t mem_mask) { address_dependent_rate(offset); };
		m_ad_tap = m_cpu->space(AS_PROGRAM).install_readwrite_tap(0x0000, 0xffff, "sam_ad_rate", tap, tap, &m_ad_tap);
		m_ad_tap_installed = true;
	}
	else if (!rate_ad && m_ad_tap_installed)
	{
		m_ad_tap.remove();
		m_ad_tap_installed = false;
	}
}



//-------------------------------------------------
//  internal_write
//-------------------------------------------------

void sam6883_device::internal_write(offs_t offset, uint8_t data)
{
	// data is ignored
	(void)data;

	// Record elapsed video with the old V bits, even for same-value writes.
	if (offset < 6)
		m_video_flush_cb(1);

	// alter the SAM state
	uint8_t old_mode = m_sam_state & 7;
	uint16_t xorval = alter_sam_state(offset);
	if (xorval & (SAM_STATE_V0|SAM_STATE_V1|SAM_STATE_V2))
		counter_mode_changed(old_mode);

	// based on the mask, apply effects
	if (xorval & (SAM_STATE_TY|SAM_STATE_M1|SAM_STATE_M0|SAM_STATE_P1))
		update_memory();
	if (xorval & (SAM_STATE_R1|SAM_STATE_R0))
	{
		update_cpu_clock();
		update_ad_tap();

		// this write ends on a slow cycle boundary
		m_ad_base = m_cpu->total_cycles();
		m_ad_slow_end = ~uint64_t(0);
	}

	if (xorval & (SAM_STATE_F6|SAM_STATE_F5|SAM_STATE_F4|SAM_STATE_F3|SAM_STATE_F2|SAM_STATE_F1|SAM_STATE_F0))
	{
		LOGFBITS("%s: SAM F Address: $%04x\n",
			machine().describe_context(),
			display_offset());
	}

	if (xorval & (SAM_STATE_V0|SAM_STATE_V1|SAM_STATE_V2))
	{
		LOGVBITS("%s: SAM V Bits: $%02x\n",
			machine().describe_context(),
			(m_sam_state & (SAM_STATE_V0|SAM_STATE_V1|SAM_STATE_V2)));
	}

	if (xorval & (SAM_STATE_P1))
	{
		LOGPBITS("%s: SAM P1 Bit: $%02x\n",
			machine().describe_context(),
			(m_sam_state & (SAM_STATE_P1)) >> 10);
	}

	if (xorval & (SAM_STATE_TY))
	{
		LOGTBITS("%s: SAM TY Bit: $%02x\n",
			machine().describe_context(),
			(m_sam_state & (SAM_STATE_TY)) >> 15);
	}

	if (xorval & (SAM_STATE_M0|SAM_STATE_M1))
	{
		LOGMBITS("%s: SAM M Bits: $%02x\n",
			machine().describe_context(),
			(m_sam_state & (SAM_STATE_M0|SAM_STATE_M1)) >> 13);
	}

	if (xorval & (SAM_STATE_R0|SAM_STATE_R1))
	{
		LOGRBITS("%s: SAM R Bits: $%02x\n",
			machine().describe_context(),
			(m_sam_state & (SAM_STATE_R0|SAM_STATE_R1)) >> 11);
	}
}



//-------------------------------------------------
//  display counter divider network
//-------------------------------------------------

// The dividers run even when their outputs are not selected. Mode writes
// reconnect clock inputs, so a high-to-low transition can advance the address
// immediately. See the hardware investigations by Ciaran Anscomb and Stewart
// Orchard: https://www.6809.org.uk/dev/tm002/ (and its linked counter model).

void sam6883_device::counter_reset(uint16_t address)
{
	m_counter = address;
	m_counter_xdiv[0] = m_counter_xdiv[1] = 0;
	m_counter_ydiv[0] = m_counter_ydiv[1] = m_counter_ydiv[2] = 0;
	m_counter_bit4_input = false;
	m_counter_bit5_input = false;
}


bool sam6883_device::counter_x_output() const
{
	switch (m_sam_state & 7)
	{
	case 1: return BIT(m_counter_xdiv[1], 1); // /3
	case 3: return BIT(m_counter_xdiv[0], 0); // /2
	default: return BIT(m_counter, 3);
	}
}


bool sam6883_device::counter_y_output() const
{
	switch (m_sam_state & 7)
	{
	case 0: return BIT(m_counter_ydiv[2], 1); // /3 followed by /4
	case 2: return BIT(m_counter_ydiv[1], 1); // /3
	case 4: return BIT(m_counter_ydiv[0], 0); // /2
	default: return BIT(m_counter, 4);
	}
}


void sam6883_device::counter_update_bit3(bool falling)
{
	if (falling)
	{
		m_counter_xdiv[0] ^= 1;
		m_counter_xdiv[1] = (m_counter_xdiv[1] + 1) % 3;
	}
	counter_set_bit4_input(counter_x_output());
}


void sam6883_device::counter_update_bit4(bool falling)
{
	if (falling)
	{
		m_counter_ydiv[0] ^= 1;
		bool old_div3_output = BIT(m_counter_ydiv[1], 1);
		m_counter_ydiv[1] = (m_counter_ydiv[1] + 1) % 3;
		if (old_div3_output && !BIT(m_counter_ydiv[1], 1))
			m_counter_ydiv[2] = (m_counter_ydiv[2] + 1) & 3;
	}
	counter_set_bit5_input(counter_y_output());
}


void sam6883_device::counter_set_bit4_input(bool level)
{
	bool falling = m_counter_bit4_input && !level;
	m_counter_bit4_input = level;
	if (falling)
	{
		m_counter ^= 0x0010;
		counter_update_bit4(!BIT(m_counter, 4));
	}
}


void sam6883_device::counter_set_bit5_input(bool level)
{
	bool falling = m_counter_bit5_input && !level;
	m_counter_bit5_input = level;
	if (falling)
		m_counter += 0x0020;
}


void sam6883_device::counter_mode_changed(uint8_t old_mode)
{
	static constexpr uint8_t x_divisors[8] = { 1, 3, 1, 2, 1, 1, 1, 1 };
	static constexpr uint8_t y_divisors[8] = { 12, 1, 3, 1, 2, 1, 1, 1 };
	uint8_t new_mode = m_sam_state & 7;
	uint8_t old_y = y_divisors[old_mode];
	uint8_t new_y = y_divisors[new_mode];
	uint8_t old_x = x_divisors[old_mode];
	uint8_t new_x = x_divisors[new_mode];

	// The Y selector settles first. Switching between /12 and /3 briefly
	// selects low; switching between /12 and /2 briefly selects raw B4.
	if (old_y != new_y)
	{
		if ((old_y == 12 && new_y == 3) || (old_y == 3 && new_y == 12))
			counter_set_bit5_input(false);
		else if ((old_y == 12 && new_y == 2) || (old_y == 2 && new_y == 12))
			counter_set_bit5_input(BIT(m_counter, 4));
		counter_set_bit5_input(counter_y_output());
	}

	// Changing between the two divided X clocks briefly selects low.
	if (old_x != new_x)
	{
		if ((old_x == 2 && new_x == 3) || (old_x == 3 && new_x == 2))
			counter_set_bit4_input(false);
		counter_set_bit4_input(counter_x_output());
	}
}


//-------------------------------------------------
//  horizontal_sync
//-------------------------------------------------

void sam6883_device::horizontal_sync()
{
	// When horizontal sync occurs, bits B1-B3 or B1-B4 may be cleared (except in DMA mode).  The catch
	// is that the SAM's counter is a chain of flip-flops.  Clearing the counter can cause carries to
	// occur just as they can when the counter is bumped.
	//
	// This is critical in getting certain semigraphics modes to work correctly.  Guardian uses this
	// mode (see bug #1153).  Special thanks to Ciaran Anscomb and Phill Harvey-Smith for figuring this
	// out
	switch((m_sam_state & (SAM_STATE_V2|SAM_STATE_V1|SAM_STATE_V0)) / SAM_STATE_V0)
	{
		case 0x01:
		case 0x03:
		case 0x05:
			// these SAM modes clear bits B1-B3
		{
			bool falling = BIT(m_counter, 3);
			m_counter &= ~0x000F;
			counter_update_bit3(falling);
			break;
		}

		case 0x00:
		case 0x02:
		case 0x04:
		case 0x06:
			// clear bits B1-B4
		{
			bool falling = BIT(m_counter, 4);
			m_counter &= ~0x001F;
			// B3 clearing is not clocked through X in these modes.
			m_counter_bit4_input = false;
			counter_update_bit4(falling);
			break;
		}

		case 0x07:
			// DMA mode - do nothing
			break;

		default:
			fatalerror("Should not get here\n");
	}
}



//-------------------------------------------------
//  hs_w
//-------------------------------------------------

void sam6883_device::hs_w(int state)
{
	if (state)
	{
		horizontal_sync();
	}
}
