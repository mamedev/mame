// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Angelo Salese
/**************************************************************************************************

    Amiga Copper

    "Co-processor" contained inside Agnus,
    it's a finite-state machine that either wait to a h/v video beam position or direct writes to
    the Amiga chipset registers via program lists, ranging from "simple" video mode/color changes
    to override sprite structures and beyond ...

    TODO:
    - Current handling is horizontally offset by several pixels, also cfr. amiga video notes
      (screen geometry slightly incorrect?);
    - Fix Bitplane offset corruption in some games (e.g. exile, zoola, AGA games).
      Same as above?
    - Fix missing/corrupt sprites in known nasty examples
      (e.g. zoola status bar, parasol score layer on top, riskyw backgrounds);
    - Find & verify cdang examples (especially for ECS/AGA);
    - Find & verify examples that uses this non-canonically,
      i.e. anything that may use this for controlling Paula, FDC or Blitter;
    - Add debugger command for printing the current disassembler structure
      (current live logging is painfully slow in places, cfr. lweapon);

**************************************************************************************************/

#include "emu.h"
#include "agnus_copper.h"

#define LOG_WARN    (1U << 1)   // Show warnings
#define LOG_COPINS  (1U << 2)   // Show instruction fetches thru COPINS
#define LOG_INST    (1U << 3)   // Show live instruction fetches
#define LOG_PC      (1U << 4)   // Show PC fetches
#define LOG_CHIPSET (1U << 5)   // Show custom chipset writes

#define VERBOSE (LOG_WARN)
//#define VERBOSE (LOG_WARN | LOG_CHIPSET | LOG_PC | LOG_INST)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGWARN(...)     LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGCOPINS(...)   LOGMASKED(LOG_COPINS, __VA_ARGS__)
#define LOGINST(...)     LOGMASKED(LOG_INST, __VA_ARGS__)
#define LOGPC(...)       LOGMASKED(LOG_PC, __VA_ARGS__)
#define LOGCHIPSET(...)  LOGMASKED(LOG_CHIPSET, __VA_ARGS__)

// TODO: legacy inheritance, to be verified
#define COPPER_CYCLES_TO_PIXELS(x)      (4 * (x))

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


// device type definition
DEFINE_DEVICE_TYPE(AGNUS_COPPER, agnus_copper_device, "agnus_copper", "Amiga Agnus Copper")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************


//-------------------------------------------------
//  agnus_copper_device - constructor
//-------------------------------------------------


agnus_copper_device::agnus_copper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AGNUS_COPPER, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_chipmem_r(*this, 0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------


void agnus_copper_device::device_start()
{
	m_host_space = &m_host_cpu->space(AS_PROGRAM);

	save_item(NAME(m_cdang_setting));
	save_item(NAME(m_cdang_min_reg));
	save_item(NAME(m_dma_master_enable));
	save_item(NAME(m_dma_copen));
	save_pointer(NAME(m_lc), 2);
	save_item(NAME(m_pc));
	save_item(NAME(m_state_waiting));
	save_item(NAME(m_state_waitblit));
	save_item(NAME(m_state_skipping));
	save_item(NAME(m_waitval));
	save_item(NAME(m_waitmask));
	save_item(NAME(m_pending_data));
	save_item(NAME(m_pending_offset));
	save_item(NAME(m_xpos_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------


void agnus_copper_device::device_reset()
{
	m_cdang_setting = 0x40;
	m_dma_master_enable = false;
	m_dma_copen = false;
	// TODO: latches states on soft reset
}


//**************************************************************************
//  I/Os
//**************************************************************************

// $dff080-8d memory map
void agnus_copper_device::regs_map(address_map &map)
{
	// TODO: location addresses belongs to Agnus
	map(0x00, 0x01).w(FUNC(agnus_copper_device::copxlch_w<0>));
	map(0x02, 0x03).w(FUNC(agnus_copper_device::copxlcl_w<0>));
	map(0x04, 0x05).w(FUNC(agnus_copper_device::copxlch_w<1>));
	map(0x06, 0x07).w(FUNC(agnus_copper_device::copxlcl_w<1>));
	map(0x08, 0x09).rw(FUNC(agnus_copper_device::copjmpx_r<0>), FUNC(agnus_copper_device::copjmpx_w<0>));
	map(0x0a, 0x0b).rw(FUNC(agnus_copper_device::copjmpx_r<1>), FUNC(agnus_copper_device::copjmpx_w<1>));
//  map(0x0c, 0x0d).w(FUNC(agnus_copper_device::copins_w));
}

void agnus_copper_device::dmacon_set(u16 data)
{
	m_dma_master_enable = bool(BIT(data, 9));
	m_dma_copen = bool(BIT(data, 7));
}

/*
 * COPCON 02E W A Copper Control Register
 *
 * ---- ---- ---- --x- CDANG (Copper Danger Mode) setting
 *
 * Documentation is quite contradictory here.
 * This bit is supposed to allow Copper access of the
 * lowest registers. OriginaL HRM states that it gives
 * access to the blitter HW therefore $dff080 is the
 * minimum for non-cdang and $dff040 for cdang mode.
 *
 * In ECS and AGA the latter limitation is lifted so Copper
 * can access $dff000-$dff03f too, which basically means the
 * possibility of accessing disk block regs.
 * (i.e. the other regs are either r/o or wouldn't have much
 *       sense to write via Copper).
 *
 */
void agnus_copper_device::copcon_w(u16 data)
{
	bool cdang = bool(BIT(data, 1));

	// TODO: is min_reg working even with BPLCON0 bit 0 unset?
	m_cdang_setting = cdang ? m_cdang_min_reg : 0x40;
	if (cdang)
		LOGWARN("%s: cdang enabled\n", machine().describe_context());
	if (data & 0xfffd)
		LOGWARN("%s: COPCON undocumented setting write %04x\n", machine().describe_context(), data);
}

template <u8 ch> void agnus_copper_device::copxlch_w(u16 data)
{
	// TODO: chipmem mask
	m_lc[ch] = (m_lc[ch] & 0x0000ffff) | ((data & 0x001f) << 16);
}

template <u8 ch> void agnus_copper_device::copxlcl_w(u16 data)
{
	m_lc[ch] = (m_lc[ch] & 0xffff0000) | ((data & 0xfffe) <<  0);
}

/*
 * COPJMP1 088 S A Copper restart at first location
 * COPJMP2 08a S A Copper restart at second location
 *
 * Strobe register that loads the new PC into Copper.
 * NB: Copper can use this to control its program flow and even
 *     do conditional branching by clever use of the skip opcode.
 *
 */
template <u8 ch> void agnus_copper_device::copjmpx_w(u16 data)
{
	set_pc(ch, false);
}

template <u8 ch> u16 agnus_copper_device::copjmpx_r()
{
	if (!machine().side_effects_disabled())
		set_pc(ch, false);
	return m_host_space->unmap();
}

inline void agnus_copper_device::set_pc(u8 ch, bool is_sync)
{
	m_pc = m_lc[ch];
	m_state_waiting = false;
	m_state_skipping = false;
	LOGPC("%s: COPJMP%d new PC = %08x%s\n"
		, machine().describe_context()
		, ch + 1
		, m_pc
		, is_sync ? " (resync)" : ""
	);
}

/*
 * COPINS 08c W A Copper instruction fetch identify
 *
 * Apparently this register is pinged every time that
 * the Copper internally decodes an instruction value.
 * Shouldn't have any implementation detail connected,
 * de-facto it's a mailbox that can be listened by external HW
 * (such as our debugger ;=).
 *
 */
void agnus_copper_device::copins_w(u16 data)
{
	LOGCOPINS("%s: COPINS %04x\n", machine().describe_context(), data);
}

//**************************************************************************
//  Implementation getters/setters
//**************************************************************************

// executed on scanline == 0
void agnus_copper_device::vblank_sync(bool state)
{
	if (state)
	{
		set_pc(0, true);
		m_xpos_state = 0;
	}
	m_vertical_blank = state;
}

// check current copper cycle at end of scanline
// - auntaadv (gameplay), WAITs with $xxd9
// - gunbee WAITs with $xxe1 at beginning of copper lists, before setting fmode.
//   Will desync scrolling by a whole lot if we don't add a +6 here
// cfr. https://eab.abime.net/showpost.php?p=627136&postcount=59
void agnus_copper_device::suspend_offset(int xpos, int hblank_width)
{
	m_xpos_state = (xpos == 511) ? 0 : xpos - hblank_width;
	// TODO: commented out, causes issues in too many places
	//m_xpos_state += 6;
//  assert(m_xpos_state > 0);
}

// restore at start
int agnus_copper_device::restore_offset()
{
	return m_xpos_state;
}

// TODO: h/vblank checks against xpos/vpos
int agnus_copper_device::execute_next(int xpos, int ypos, bool is_blitter_busy, int num_planes)
{
	int word0, word1;

	/* bail if not enabled */
	if (!m_dma_master_enable || !m_dma_copen)
		return 511;

	/* flush any pending writes */
	if (m_pending_offset)
	{
		//LOGCHIPSET("%02X.%02X: Write to %s = %04x\n", ypos, xpos / 2, s_custom_reg_names[m_copper_pending_offset & 0xff], m_copper_pending_data);
		LOGCHIPSET("%02X.%02X: MOVE $dff%03x = %04x\n",
			ypos,
			xpos / 2,
			(m_pending_offset << 1),
			m_pending_data
		);
		m_host_space->write_word(0xdff000 | (m_pending_offset << 1), m_pending_data);
		m_pending_offset = 0;
	}

	/* if we're waiting, check for a breakthrough */
	if (m_state_waiting)
	{
		int curpos = (ypos << 8) | (xpos >> 1);

		/* if we're past the wait time, stop it and hold up 2 cycles */
		if ((curpos & m_waitmask) >= (m_waitval & m_waitmask) &&
			(!m_state_waitblit || !(is_blitter_busy)))
		{
			m_state_waiting = false;
			return xpos + COPPER_CYCLES_TO_PIXELS(1);
		}

		/* otherwise, see if this line is even a possibility; if not, punt */
		if (((curpos | 0xff) & m_waitmask) < (m_waitval & m_waitmask))
			return 511;

		/* else just advance another pixel */
		xpos += COPPER_CYCLES_TO_PIXELS(1);
		return xpos;
	}

	/* fetch the first data word */
	// TODO: swap between ir0 and ir1 is controlled thru a selins latch
	// (which can't be this instant too)
	word0 = m_chipmem_r(m_pc);
	m_host_space->write_word(0xdff08c, word0);
	m_pc += 2;
	xpos += COPPER_CYCLES_TO_PIXELS(1);

	/* fetch the second data word */
	word1 = m_chipmem_r(m_pc);
	m_host_space->write_word(0xdff08c, word1);
	m_pc += 2;
	xpos += COPPER_CYCLES_TO_PIXELS(1);

	LOGINST("%02X.%02X: Copper inst @ %06x = %04x %04x\n",
		ypos,
		xpos / 2,
		m_pc - 4,
		word0,
		word1
	);

	/* handle a move */
	if ((word0 & 1) == 0)
	{
		/* do the write if we're allowed */
		word0 = (word0 >> 1) & 0xff;
		if (word0 >= m_cdang_setting)
		{
			// SKIP applies to valid MOVEs only
			// - apocalyps (gameplay, chain of SKIPs & WAIT)
			// - rbisland (loading screen at least, tries to SKIP a CDANG MOVE)
			if (m_state_skipping)
			{
				LOGINST("  (Ignored)\n");
				m_state_skipping = false;
				// TODO: verify timings
				// may depend on num of planes enabled (move_offset) or opcode fetch above is enough.
				xpos += COPPER_CYCLES_TO_PIXELS(2);
				return xpos;
			}
			// delay write to the next available DMA slot if not in blanking area
			// - suprfrog & abreed (bottom playfield rows).
			// - beast, biochall and cd32 bios wants this to be 0x5c
			const bool horizontal_blank = xpos <= 0x5c;
			const int move_offset = horizontal_blank || m_vertical_blank ? 0 : std::max(num_planes - 4, 0);

			m_pending_offset = word0;
			m_pending_data = word1;
			xpos += COPPER_CYCLES_TO_PIXELS(move_offset);
		}

		/* illegal writes suspend until next frame */
		else
		{
			LOGWARN("%02X.%02X: Aborting copper on illegal write\n", ypos, xpos / 2);

			m_waitval = 0xffff;
			m_waitmask = 0xffff;
			m_state_waitblit = false;
			m_state_waiting = true;
			m_state_skipping = false;

			return 511;
		}
	}
	else
	{
		/* extract common wait/skip values */
		m_waitval = word0 & 0xfffe;

		m_waitmask = word1 | 0x8001;
		m_state_waitblit = (~word1 >> 15) & 1;

		/* handle a wait */
		if ((word1 & 1) == 0)
		{
			const bool horizontal_blank = xpos <= 0x5c;
			const int wait_offset = horizontal_blank || m_vertical_blank ? 0 : std::max(num_planes - 4, 0) + 1;

			LOGINST("  WAIT %04x & %04x (currently %04x, num planes %d +%d)\n",
				m_waitval,
				m_waitmask,
				(ypos << 8) | (xpos >> 1),
				num_planes,
				wait_offset
			);

			m_state_skipping = false;
			m_state_waiting = true;
			xpos += COPPER_CYCLES_TO_PIXELS(wait_offset);
		}

		/* handle a skip */
		else
		{
			int curpos = (ypos << 8) | (xpos >> 1);
			m_state_skipping = ((curpos & m_waitmask) >= (m_waitval & m_waitmask)
				&& (!m_state_waitblit || !(is_blitter_busy)));

			LOGINST("  SKIP %04x & %04x (currently %04x) - %s\n",
				m_waitval,
				m_waitmask,
				(ypos << 8) | (xpos >> 1),
				m_state_skipping ? "Skipping" : "Not skipped"
			);
		}
	}

	/* advance and consume 8 cycles */
	return xpos;
}
