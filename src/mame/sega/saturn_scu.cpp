// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Sega Saturn System Control Unit (c) 1995 Sega/Yamaha

TODO:
- implement penalties for attached devices when DMA-ing from/to;
- implement additional DMA rulesets
\- can't access same bus in direct mode;
\- indirect mode can actually access same bus with some quirks, specifics TBD;
\- avoid cross country DMA-ing from one region to the other, i.e. gunblaze;
\- Road Blaster shifted 1-byte quirk, cfr. currently unused function (intentionally broken);
- Verify Timer 1 (seems unaffected even after rewriting it?)
- A-Bus external interrupts;
- A-Bus waitstates;
- PAD irq signal from SMPC (lightgun and some mice);

===================================================================================================

Interesting use cases (i.e. non-sloppy programming plaguing this system):
- 3dlemminj: title screen "3d" logo going fast and glitchy (wants VDP1 timing down everything else)
- burningrj: transfers FMV in VDP2 (cached) in display area (delayed one frame and done later?
  Current performance is quite bad right now)

A-Bus: $0200'0000 - $058f'ffff
B-Bus: $0590'0000 - $05ff'ffff
C-Bus: $0600'0000 - $07ff'ffff (Work RAM-H, mirrored)

**************************************************************************************************/

#include "emu.h"
#include "saturn_scu.h"

#define LOG_DMA_MOVE     (1 << 1) // log the initial values prior to a DMA WAIT -> MOVE
#define LOG_DMA_END      (1 << 2) // log the values at end of DMA
#define LOG_DMA_STATE    (1 << 3) // log state changes
#define LOG_DMA_MODE     (1 << 4) // log accepted transfer state
#define LOG_DMA_INDIRECT (1 << 5) // log indirect fetches (verbose)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(SATURN_SCU, saturn_scu_device, "saturn_scu", "Sega Saturn System Control Unit (Yamaha FH3007 315-5688)")

//-------------------------------------------------
//  saturn_scu_device - constructor
//-------------------------------------------------

saturn_scu_device::saturn_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SATURN_SCU, tag, owner, clock)
	, m_scudsp(*this, "scudsp")
	, m_hostcpu(*this, finder_base::DUMMY_TAG)
	, m_bbus_sound_dtack_cb(*this)
	, m_cbus_dtack_cb(*this)
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

template <unsigned level> void saturn_scu_device::dma_map(address_map &map)
{
	map(0x00, 0x03).lrw32(
		NAME([this] (offs_t offset) {
			return m_dma[level].src;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_dma[level].src);
			m_dma[level].src &= 0x27ff'ffff;
		})
	);
	map(0x04, 0x07).lrw32(
		NAME([this] (offs_t offset) {
			return m_dma[level].dst;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_dma[level].dst);
			m_dma[level].dst &= 0x27ff'ffff;
		})
	);
	map(0x08, 0x0b).lrw32(
		NAME([this] (offs_t offset) {
			return m_dma[level].size;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_dma[level].size);
			m_dma[level].size &= ((level == 0) ? 0x000fffff : 0xfff);
		})
	);
	// everything else is write only
	map(0x0c, 0x17).nopr();
	// DxAD: add values
	map(0x0c, 0x0f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_8_15)
				m_dma[level].src_add = BIT(data, 8) * 4;
			if (ACCESSING_BITS_0_7)
			{
				m_dma[level].dst_add = 1 << (data & 7);
				if(m_dma[level].dst_add == 1) { m_dma[level].dst_add = 0; }
			}
		})
	);
	// DxEN / DxGO: enable and trigger
	map(0x10, 0x13).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_8_15)
				m_dma[level].enable_mask = BIT(data, 8);

			// check if DxGO is enabled for start factor = 7
			if(ACCESSING_BITS_0_7 && m_dma[level].enable_mask == true && BIT(data, 0) && m_dma[level].start_factor == DMA_EVENT_TRIGGER)
			{
				if(m_dma[level].indirect_mode == true)
					trigger_dma_indirect(level);
				else
					trigger_dma_direct(level);
			}
		})
	);
	// DxMOD / DxRUP / DxWUP / DxFT: indirect mode, RUP, WUP, start factor
	map(0x14, 0x17).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_24_31)
				m_dma[level].indirect_mode = BIT(data, 24);
			if (ACCESSING_BITS_16_23)
				m_dma[level].rup = BIT(data, 16);
			if (ACCESSING_BITS_8_15)
				m_dma[level].wup = BIT(data, 8);
			if (ACCESSING_BITS_0_7)
				m_dma[level].start_factor = data & 7;
		})
	);
}

// Instantiate DMA maps
template void saturn_scu_device::dma_map<0>(address_map &map);
template void saturn_scu_device::dma_map<1>(address_map &map);
template void saturn_scu_device::dma_map<2>(address_map &map);


void saturn_scu_device::regs_map(address_map &map)
{
	map(0x0000, 0x0017).m(*this, FUNC(saturn_scu_device::dma_map<0>));
	map(0x0020, 0x0037).m(*this, FUNC(saturn_scu_device::dma_map<1>));
	map(0x0040, 0x0057).m(*this, FUNC(saturn_scu_device::dma_map<2>));
	// stv:smleague and shinmtaz reads from $005c (undocumented), DMA status mirror?
	map(0x005c, 0x005f).r(FUNC(saturn_scu_device::dma_status_r));
//  map(0x0060, 0x0063).w(FUNC(saturn_scu_device::dma_force_stop_w));
	map(0x007c, 0x007f).r(FUNC(saturn_scu_device::dma_status_r));
	map(0x0080, 0x0083).rw(m_scudsp, FUNC(scudsp_cpu_device::program_control_r), FUNC(scudsp_cpu_device::program_control_w));
	map(0x0084, 0x0087).w(m_scudsp, FUNC(scudsp_cpu_device::program_w));
	map(0x0088, 0x008b).w(m_scudsp, FUNC(scudsp_cpu_device::ram_address_control_w));
	map(0x008c, 0x008f).rw(m_scudsp, FUNC(scudsp_cpu_device::ram_address_r), FUNC(scudsp_cpu_device::ram_address_w));
	map(0x0090, 0x0093).w(FUNC(saturn_scu_device::t0_compare_w));
	map(0x0094, 0x0097).w(FUNC(saturn_scu_device::t1_setdata_w));
	map(0x009a, 0x009b).w(FUNC(saturn_scu_device::t1_mode_w));
	map(0x00a0, 0x00a3).rw(FUNC(saturn_scu_device::irq_mask_r), FUNC(saturn_scu_device::irq_mask_w));
	map(0x00a4, 0x00a7).rw(FUNC(saturn_scu_device::irq_status_r), FUNC(saturn_scu_device::irq_status_w));
//  map(0x00a8, 0x00ab).w(FUNC(saturn_scu_device::abus_irqack_w));
//  map(0x00b0, 0x00b7).rw(FUNC(saturn_scu_device::abus_set_r), FUNC(saturn_scu_device::abus_set_w));
//  map(0x00b8, 0x00bb).rw(FUNC(saturn_scu_device::abus_refresh_r), FUNC(saturn_scu_device::abus_refresh_w));
//  map(0x00c4, 0x00c7).rw(FUNC(saturn_scu_device::sdram_r), FUNC(saturn_scu_device::sdram_w));
	map(0x00c8, 0x00cb).r(FUNC(saturn_scu_device::version_r));
}


//-------------------------------------------------
//  add_device_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

uint16_t saturn_scu_device::scudsp_dma_r(offs_t offset, uint16_t mem_mask)
{
	//address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = offset & 0x07ff'ffff;

//	printf("%08x\n", offset);

	return m_hostspace->read_word(addr,mem_mask);
}


void saturn_scu_device::scudsp_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = offset & 0x07ff'ffff;

//  printf("%08x %02x\n",offset,data);

	m_hostspace->write_word(addr, data,mem_mask);
}

void saturn_scu_device::device_add_mconfig(machine_config &config)
{
	SCUDSP(config, m_scudsp, XTAL(57'272'727) / 4); // 14 MHz
	m_scudsp->out_irq_callback().set(DEVICE_SELF, FUNC(saturn_scu_device::scudsp_end_w));
	m_scudsp->in_dma_callback().set(FUNC(saturn_scu_device::scudsp_dma_r));
	m_scudsp->out_dma_callback().set(FUNC(saturn_scu_device::scudsp_dma_w));
	m_scudsp->out_ddwt_callback().set([this] (int state) {
		if (state)
			m_dma_status |= DMA_DSP_WAIT;
		else
			m_dma_status &= ~(DMA_DSP_WAIT);
	});
	m_scudsp->out_ddmv_callback().set([this] (int state) {
		if (state)
			m_dma_status |= DMA_DSP_MOVE;
		else
			m_dma_status &= ~(DMA_DSP_MOVE);
	});

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saturn_scu_device::device_start()
{
	save_item(NAME(m_ist));
	save_item(NAME(m_ism));
	save_item(NAME(m_t0c));
	save_item(NAME(m_t1s));
	save_item(NAME(m_t1md));

	save_item(NAME(m_dma[0].src));
	save_item(NAME(m_dma[0].dst));
	save_item(NAME(m_dma[0].src_add));
	save_item(NAME(m_dma[0].dst_add));
	save_item(NAME(m_dma[0].size));
	save_item(NAME(m_dma[0].index));
	save_item(NAME(m_dma[0].start_factor));
	save_item(NAME(m_dma[0].enable_mask));
	save_item(NAME(m_dma[0].indirect_mode));
	save_item(NAME(m_dma[0].indirect_fetch_phase));
	save_item(NAME(m_dma[0].rup));
	save_item(NAME(m_dma[0].wup));
	save_item(NAME(m_dma[0].mode));
	save_item(NAME(m_dma[0].done));
	save_item(NAME(m_dma[0].live_src));
	save_item(NAME(m_dma[0].live_dst));
	save_item(NAME(m_dma[0].live_size));
	save_item(NAME(m_dma[0].live_count));
	save_item(NAME(m_dma[0].cbus_cache_through));
	save_item(NAME(m_dma[0].bbus_sound_access));

	save_item(NAME(m_dma[1].src));
	save_item(NAME(m_dma[1].dst));
	save_item(NAME(m_dma[1].src_add));
	save_item(NAME(m_dma[1].dst_add));
	save_item(NAME(m_dma[1].size));
	save_item(NAME(m_dma[1].index));
	save_item(NAME(m_dma[1].start_factor));
	save_item(NAME(m_dma[1].enable_mask));
	save_item(NAME(m_dma[1].indirect_mode));
	save_item(NAME(m_dma[1].indirect_fetch_phase));
	save_item(NAME(m_dma[1].rup));
	save_item(NAME(m_dma[1].wup));
	save_item(NAME(m_dma[1].mode));
	save_item(NAME(m_dma[1].done));
	save_item(NAME(m_dma[1].live_src));
	save_item(NAME(m_dma[1].live_dst));
	save_item(NAME(m_dma[1].live_size));
	save_item(NAME(m_dma[1].live_count));
	save_item(NAME(m_dma[1].cbus_cache_through));
	save_item(NAME(m_dma[1].bbus_sound_access));

	save_item(NAME(m_dma[2].src));
	save_item(NAME(m_dma[2].dst));
	save_item(NAME(m_dma[2].src_add));
	save_item(NAME(m_dma[2].dst_add));
	save_item(NAME(m_dma[2].size));
	save_item(NAME(m_dma[2].index));
	save_item(NAME(m_dma[2].start_factor));
	save_item(NAME(m_dma[2].enable_mask));
	save_item(NAME(m_dma[2].indirect_mode));
	save_item(NAME(m_dma[2].indirect_fetch_phase));
	save_item(NAME(m_dma[2].rup));
	save_item(NAME(m_dma[2].wup));
	save_item(NAME(m_dma[2].mode));
	save_item(NAME(m_dma[2].done));
	save_item(NAME(m_dma[2].live_src));
	save_item(NAME(m_dma[2].live_dst));
	save_item(NAME(m_dma[2].live_size));
	save_item(NAME(m_dma[2].live_count));
	save_item(NAME(m_dma[2].cbus_cache_through));
	save_item(NAME(m_dma[2].bbus_sound_access));

	save_item(NAME(m_current_irq_level));

	m_hostspace = &m_hostcpu->space(AS_PROGRAM);

	m_dma_tick_timer = timer_alloc(FUNC(saturn_scu_device::dma_tick_cb), this);
	m_timer1 = timer_alloc(FUNC(saturn_scu_device::timer1_irq_cb), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void saturn_scu_device::device_reset()
{
	m_ism = 0xbfff;
	m_ist = 0;

	for(int i = 0; i < 3; i++)
	{
		m_dma[i].src_add = 4;
		m_dma[i].dst_add = 2;
		m_dma[i].start_factor = DMA_EVENT_TRIGGER;
		m_dma[i].enable_mask = false;
		m_dma[i].done = false;
		m_dma[i].cbus_cache_through = false;
		m_dma[i].bbus_sound_access = false;
		m_dma[i].mode = DMA_MODE_RESET;
	}

	m_dma_tick_timer->adjust(attotime::never);
	m_dma_status = 0;
	m_current_irq_level = 0;

	// Nope until we have a proper DTACK instead of an HALT,
	// SMPC triggers this thru dotsel (2 credits meme ...)
	//m_bbus_sound_dtack_cb(0);
	//m_cbus_dtack_cb(0);

	m_tenb = false;
	m_t1md = false;
	m_timer0_counter = 0;
	m_timer1->adjust(attotime::never);
}

void saturn_scu_device::device_clock_changed()
{
	m_scudsp->set_unscaled_clock(this->clock() / 4);
	// FIXME: should be /4 but saturn BIOS already disagrees
	// (or /2 if the doc claims 70 nsec in dword unit)
	m_dma_clock_ref = this->clock() / 1;
	LOG("New ref DMA clock %u\n", m_dma_clock_ref);
}

//-------------------------------------------------
//  device_reset_after_children
//-------------------------------------------------

void saturn_scu_device::device_reset_after_children()
{
	m_scudsp->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

//**************************************************************************
//  DMA logic
//**************************************************************************

inline void saturn_scu_device::update_dma_status(int level, dma_state_t new_state)
{
	const std::string status_names[] = { "IDLE", "WAIT", "MOVE", "????" };
	const int log_shifts[] = { 4, 8, 12 };
	assert(level >= 0);

	LOGMASKED(LOG_DMA_STATE, "DMA%d state change %s -> ", level, status_names[(m_dma_status >> log_shifts[level]) & 0x3]);

	m_dma_status &= ~(0x30 << 4 * level);
	m_dma_status |= (new_state << 4 * level);

	LOGMASKED(LOG_DMA_STATE, "%s (%08x)\n", status_names[(m_dma_status >> log_shifts[level]) & 0x3], m_dma_status);
}

void saturn_scu_device::trigger_dma_direct(uint8_t level)
{
	// registers are DxR, DxW etc.
	LOGMASKED(LOG_DMA_MOVE, "DMA%d direct R %08x W %08x C %08x RA %d WA %d %s %s\n",
		level, m_dma[level].src, m_dma[level].dst, m_dma[level].size,
		m_dma[level].src_add, m_dma[level].dst_add, m_dma[level].rup ? "RUP" : "", m_dma[level].wup ? "WUP" : "");

	// stv:vmahjong loads game IPL twice at startup, one with cache the other without.
	if (m_dma_status & 0x30 << level)
	{
		LOG("In-flight DMA%d attempt!\n", level);
		return;
	}

	// gamebas, wc98 and batmanfu trips this
	// according to the docs the SCU can't transfer from BIOS area
	// (can't communicate from/to that bus)
	if((m_dma[level].src & 0x07f00000) == 0)
	{
		LOG("Attempted an illegal DMA at R %08x\n", m_dma[level].src);
		m_ist |= IST_DMAILL;
		test_pending_irqs();
		return;
	}

	/* max size */
	if(m_dma[level].size == 0) { m_dma[level].size = (level == 0) ? 0x00100000 : 0x1000; }

	// gunblaze: during startup tries to do a max sized DMA transfer to VDP1 that would eventually hit fb/regs
	if ((m_dma[level].dst & 0x07f0'0000) == 0x05c0'0000 && m_dma[level].size >= 0x80000)
		m_dma[level].size = 0x80000 - (m_dma[level].dst & 0x7fffe);

	// stv:colmns97 & stv:wwshin, which doesn't work right
	// (timing more likely, also ST-V has more soundram)
//  if ((m_dma[level].dst & 0x07f0'0000) == 0x05a0'0000 && m_dma[level].size >= 0x80000)
//      m_dma[level].size = 0x80000 - (m_dma[level].dst & 0x7ffff);

	m_dma[level].mode = DMA_MODE_RESET;

	// CD transfers are special even without the hack below
	if (m_dma[level].src_add == 0 && (m_dma[level].src & 0x07ff'ffff) == 0x0581'8000)
	{
		LOGMASKED(LOG_DMA_MODE, "Mode select: CD\n");
		m_dma[level].mode |= DMA_MODE_CD;
	}

	// if target is Work RAM H, the add value is fixed.
	// behaviour confirmed by astrass, fromanc2, stv:vmahjong and burningr*
	if ((m_dma[level].dst & 0x0700'0000) == 0x0600'0000)
	{
		LOGMASKED(LOG_DMA_MODE, "Mode select: C-Bus Write\n");
		m_dma[level].mode |= DMA_MODE_CBUS_WRITE;
	}

	m_dma[level].bbus_sound_access = (m_dma[level].src & 0x07f0'0000) == 0x05a0'0000 || (m_dma[level].dst & 0x07f0'0000) == 0x05a0'0000;
	// - sonicjamj Sonic 1 (at least) does VDP2s back-to-back writes, several failing without a guard here.
	const bool vdp2_access = (m_dma[level].dst & 0x05e0'0000) == 0x05e0'0000;

	// - saturn BIOS chains several cache through DMAs back-to-back with no status check
	//   clearly expect that the host CPU shouldn't do anything around the time the DMA goes.
	// - stv:gaxeduel also does two back-to-back sound DMAs from A-Bus, failing the second one if
	//   SH-2s aren't slowed down to a crawl
	// TODO: latter really needs bus grants, interruptible SH-2 and .before_delay.
	m_dma[level].cbus_cache_through = m_dma[level].bbus_sound_access || vdp2_access || (m_dma[level].src & 0x2700'0000) == 0x2600'0000 || (m_dma[level].dst & 0x2700'0000) == 0x2600'0000;

	m_dma[level].live_src = m_dma[level].src;
	m_dma[level].live_dst = m_dma[level].dst;
	m_dma[level].live_size = m_dma[level].size;
	m_dma[level].live_count = 0;
	m_dma[level].done = false;

	update_dma_status(level, DMA_STATE_WAIT);

	m_dma_tick_timer->adjust(attotime::from_ticks(1, m_dma_clock_ref));
}

void saturn_scu_device::trigger_dma_indirect(uint8_t level)
{
	LOGMASKED(LOG_DMA_MOVE, "DMA%d indirect W %08x RA %d WA %d\n",
		level, m_dma[level].dst, m_dma[level].src_add, m_dma[level].dst_add);

	if (m_dma_status & 0x30 << level)
	{
		LOG("In-flight DMA%d attempt!\n", level);
		return;
	}

	// aligned in dword units
	// TODO: check if other buses can be used
	m_dma[level].index = m_dma[level].dst & 0x07ff'fffc;
	m_dma[level].done = false;
	m_dma[level].mode = DMA_MODE_INDIRECT;
	m_dma[level].indirect_fetch_phase = true;
	m_dma[level].indirect_end_flag = false;

	update_dma_status(level, DMA_STATE_WAIT);

	m_dma_tick_timer->adjust(attotime::from_ticks(1, m_dma_clock_ref));
}

// TODO: reimplement me
inline void saturn_scu_device::dma_single_transfer(uint32_t src, uint32_t dst,uint8_t *src_shift)
{
	uint32_t src_data;

	if(src & 1)
	{
		// tstrmrbl:cdrom2 (Road Blaster) does a work ram h to color ram with offsetted source address, do some data rotation
		src_data = ((m_hostspace->read_dword(src & 0x07fffffc) & 0x00ffffff)<<8);
		src_data |= ((m_hostspace->read_dword((src & 0x07fffffc)+4) & 0xff000000) >> 24);
		src_data >>= (*src_shift)*16;
	}
	else
		src_data = m_hostspace->read_dword(src & 0x07fffffc) >> (*src_shift)*16;

	m_hostspace->write_word(dst,src_data);

	*src_shift ^= 1;
}

std::tuple<int, int> saturn_scu_device::check_dma_level_round_robin()
{
	int move_level = -1, wait_level = -1;
	// this returns the highest move/wait level currently set
	for (int level = 0; level < 3; level ++)
	{
		if (m_dma_status & (0x10 << (level * 4)))
			move_level = level;
		if (m_dma_status & (0x20 << (level * 4)))
			wait_level = level;
	}

	return std::make_tuple(move_level, wait_level);
}

TIMER_CALLBACK_MEMBER(saturn_scu_device::dma_tick_cb)
{
	// guess: yield until DSP do its thing
	if (m_dma_status & DMA_DSP_MOVE)
	{
		m_dma_tick_timer->adjust(attotime::from_ticks(1, m_dma_clock_ref));
		return;
	}

	auto [level, wait_level] = check_dma_level_round_robin();

	//printf("%d %d\n", level, wait_level);

	if (level != -1)
	{
		if (m_dma[level].done)
		{
			// burningru doesn't want to zero existing size
			//  m_scu.size[dma_ch] = 0;

			m_dma[level].done = false;
			m_dma[level].live_count = 0;
			m_cbus_dtack_cb(0);
			m_bbus_sound_dtack_cb(0);

			const uint16_t irqmask = 1 << (11 - level);

			m_ist |= irqmask;
			test_pending_irqs();

			update_dma_status(level, DMA_STATE_IDLE);
			if (wait_level != -1)
			{
				update_dma_status(wait_level, DMA_STATE_MOVE);

				LOGMASKED(LOG_DMA_STATE, "Push DMA%d in foreground\n", wait_level);
				if (wait_level == 1)
					m_dma_status &= ~(DMA_LV1_BK);
				else
					m_dma_status &= ~(DMA_LV0_BK);
				m_dma_tick_timer->adjust(attotime::from_ticks(1, m_dma_clock_ref));
			}
			else
				m_dma_tick_timer->adjust(attotime::never);
			return;
		}

		if (m_dma[level].mode & DMA_MODE_INDIRECT)
		{
			if (m_dma[level].indirect_fetch_phase)
			{
				u32 indirect_src, indirect_dst, indirect_size;
				indirect_size = m_hostspace->read_dword(m_dma[level].index);
				indirect_dst  = m_hostspace->read_dword(m_dma[level].index + 4);
				indirect_src  = m_hostspace->read_dword(m_dma[level].index + 8);
				m_dma[level].indirect_end_flag = BIT(indirect_src, 31);

				LOGMASKED(LOG_DMA_INDIRECT, "DMA%d indirect entry %08x: R %08x W %08x C %08x %s\n",
					level, m_dma[level].index, indirect_src, indirect_dst, indirect_size,
					m_dma[level].indirect_end_flag ? "END" : "");

				m_dma[level].bbus_sound_access = (indirect_src & 0x07e0'0000) == 0x05a0'0000 || (indirect_dst & 0x07e0'0000) == 0x05a0'0000;
				const bool vdp2_access = (indirect_dst & 0x05e0'0000) == 0x05e0'0000;
				m_dma[level].cbus_cache_through = m_dma[level].bbus_sound_access || vdp2_access || (indirect_src & 0x2700'0000) == 0x2600'0000 || (indirect_dst & 0x2700'0000) == 0x2600'0000;

				m_dma[level].live_src = indirect_src & 0x07ff'ffff;
				m_dma[level].live_dst = indirect_dst & 0x07ff'ffff;
				//TODO: why guardherj sets up a 0x23000 transfer for the FMV?
				m_dma[level].live_size = indirect_size & ((level == 0) ? 0xf'ffff : 0x3'ffff);
				m_dma[level].live_count = 0;

				m_dma[level].mode = DMA_MODE_INDIRECT;

				// TODO: other rules still applies
				if ((indirect_dst & 0x0700'0000) == 0x0600'0000)
				{
					LOGMASKED(LOG_DMA_MODE, "Mode select: C-Bus Write\n");
					m_dma[level].mode |= DMA_MODE_CBUS_WRITE;
				}

				m_dma[level].index += 0x0c;
				m_dma[level].indirect_fetch_phase = false;
				// yield 3 clock cycles out of fetching the new data and get out
				m_dma_tick_timer->adjust(attotime::from_ticks(3, m_dma_clock_ref));
				return;
			}

			(this->*dma_transfer_table[m_dma[level].mode & 3])(m_dma[level]);

			if (m_dma[level].wup)
				m_dma[level].dst = m_dma[level].index;

			if (m_dma[level].live_count >= m_dma[level].live_size)
			{
				LOGMASKED(LOG_DMA_END, "DMA%d indirect ended at %08x %08x\n", level, m_dma[level].live_src, m_dma[level].live_dst);

				if (m_dma[level].indirect_end_flag)
					m_dma[level].done = true;
				else
					m_dma[level].indirect_fetch_phase = true;
			}
		}
		else
		{
			(this->*dma_transfer_table[m_dma[level].mode & 3])(m_dma[level]);

			if (m_dma[level].rup)
				m_dma[level].src = m_dma[level].live_src;

			if (m_dma[level].wup)
				m_dma[level].dst = m_dma[level].live_dst;

			if (m_dma[level].live_count >= m_dma[level].live_size)
			{
				LOGMASKED(LOG_DMA_END, "DMA%d direct ended at %08x %08x (RUP %d WUP %d)\n", level, m_dma[level].live_src, m_dma[level].live_dst, m_dma[level].rup, m_dma[level].wup);
				m_dma[level].done = true;
			}
		}

	}

	if (wait_level > level)
	{
		// clear wait, set move
		update_dma_status(wait_level, DMA_STATE_MOVE);

		if (level != -1)
		{
			// set wait (allegedly) and interrupt for the last transfer, clear move
			update_dma_status(wait_level, DMA_STATE_WAIT);
			LOGMASKED(LOG_DMA_STATE, "Push DMA%d in background\n", level);

			m_dma_status |= (1 << (16 + level));
		}
	}

	m_dma_tick_timer->adjust(attotime::from_ticks(1, m_dma_clock_ref));
}

// CD transfers needs to be in dword unit for now (need the xfertype32 branch)
// we will also need a proper DRDY later on ...
const saturn_scu_device::dma_transfer_func saturn_scu_device::dma_transfer_table[4] =
{
	&saturn_scu_device::dma_transfer_direct_default,
	&saturn_scu_device::dma_transfer_direct_cbus_write,
	&saturn_scu_device::dma_transfer_direct_cd,
	&saturn_scu_device::dma_transfer_direct_cd_cbus_write
};

void saturn_scu_device::dma_transfer_direct_default(dma_channel_t &ch)
{
	//dma_single_transfer(m_dma[level].src, m_dma[level].dst, &src_shift);
	const u32 src_address = ch.live_src & 0x07ff'fffe;
	const u32 dst_address = ch.live_dst & 0x07ff'fffe;

	uint32_t src_data = m_hostspace->read_word(src_address);

	m_hostspace->write_word(dst_address, src_data);
	m_cbus_dtack_cb(ch.cbus_cache_through);
	m_bbus_sound_dtack_cb(ch.bbus_sound_access);

	ch.live_src += 2;
	// TODO: reimplement me
// if(src_shift)
//	dma_params.src+= dma_params.src_add;
//
	ch.live_dst += ch.dst_add;

	ch.live_count += 2;
}

void saturn_scu_device::dma_transfer_direct_cbus_write(dma_channel_t &ch)
{
	//dma_single_transfer(m_dma[level].src, m_dma[level].dst, &src_shift);
	const u32 src_address = ch.live_src & 0x07ff'fffe;
	const u32 dst_address = ch.live_dst & 0x07ff'fffe;

	uint32_t src_data = m_hostspace->read_word(src_address);

	m_hostspace->write_word(dst_address, src_data);
	m_cbus_dtack_cb(ch.cbus_cache_through);
	m_bbus_sound_dtack_cb(ch.bbus_sound_access);

	ch.live_src += 2;
	// TODO: reimplement me
// if(src_shift)
//	dma_params.src+= dma_params.src_add;
//
	ch.live_dst += 2;

	ch.live_count += 2;
}

void saturn_scu_device::dma_transfer_direct_cd(dma_channel_t &ch)
{
	const u32 dst_add = ch.dst_add << 1;

	const u32 src_address = ch.live_src & 0x07ff'fffc;
	const u32 dst_address = ch.live_dst & 0x07ff'fffc;

	m_hostspace->write_dword(dst_address, m_hostspace->read_dword(src_address));
	if(dst_add == 8)
		m_hostspace->write_dword(dst_address + 4, m_hostspace->read_dword(src_address));

	m_cbus_dtack_cb(ch.cbus_cache_through);
	m_bbus_sound_dtack_cb(ch.bbus_sound_access);

	ch.live_src += ch.src_add;
	ch.live_dst += dst_add;
	ch.live_count += dst_add;
}

void saturn_scu_device::dma_transfer_direct_cd_cbus_write(dma_channel_t &ch)
{
	const u32 src_address = ch.live_src & 0x07ff'fffc;
	const u32 dst_address = ch.live_dst & 0x07ff'fffc;

	m_hostspace->write_dword(dst_address, m_hostspace->read_dword(src_address));
	if(ch.dst_add == 8)
		m_hostspace->write_dword(dst_address + 4, m_hostspace->read_dword(src_address));

	m_cbus_dtack_cb(ch.cbus_cache_through);
	m_bbus_sound_dtack_cb(ch.bbus_sound_access);

	ch.live_src += ch.src_add;
	ch.live_dst += 4;
	ch.live_count += 4;
}


inline void saturn_scu_device::dma_start_factor_ack(dma_event_id_t event)
{
	for(int i = 0; i < 3; i++)
	{
		if(m_dma[i].enable_mask == true && m_dma[i].start_factor == event)
		{
			if(m_dma[i].indirect_mode == true) { trigger_dma_indirect(i); }
			else                               { trigger_dma_direct(i); }
		}
	}
}

uint32_t saturn_scu_device::dma_status_r()
{
	return m_dma_status;
}

//**************************************************************************
// Timers
//**************************************************************************

void saturn_scu_device::t0_compare_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_t0c);
	m_t0c &= 0x3ff;
}

void saturn_scu_device::t1_setdata_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_t1s);
	m_t1s &= 0x1ff;
}

/*
 * ---- ---x ---- ---- T1MD Timer 1 mode (0=each line, 1=only at timer 0 lines)
 * ---- ---- ---- ---x TENB Timers enable
 */
void saturn_scu_device::t1_mode_w(uint16_t data)
{
	m_t1md = BIT(data, 8);
	m_tenb = BIT(data, 0);
	if (!m_tenb)
	{
		m_timer0_counter = 0;
		m_timer1->adjust(attotime::never);
	}
}

TIMER_CALLBACK_MEMBER(saturn_scu_device::timer1_irq_cb)
{
	dma_start_factor_ack(DMA_EVENT_TIMER1);

	m_ist |= IST_TIMER_1;
	test_pending_irqs();
}


//**************************************************************************
//  Interrupt
//**************************************************************************

uint32_t saturn_scu_device::irq_mask_r()
{
	return m_ism;
}

uint32_t saturn_scu_device::irq_status_r()
{
	return m_ist;
}

void saturn_scu_device::irq_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ism);
	test_pending_irqs();
}

void saturn_scu_device::irq_status_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// burningr*
//	if(mem_mask != 0xffffffff)
//		LOG("%s IST write %08x with %08x\n", this->tag(), data, mem_mask);

	m_ist &= data;
	test_pending_irqs();
}

void saturn_scu_device::test_pending_irqs()
{
	// ignore if current irq still serviced
	if (m_current_irq_level != 0)
		return;

	// 15: vblank-in
	// 14: vblank-out
	// 13: hblank-in
	// 12: timer 0
	// 11: timer 1
	// 10: DSP end
	//  9: SCSP
	//  8: SMPC & PAD
	//  6: DMA end LV2 & LV1
	//  5: DMA end LV0
	//  3: DMA illegal
	//  2: VDP1 draw end
	const int irq_level[32] = { 0xf, 0xe, 0xd, 0xc,
								0xb, 0xa, 0x9, 0x8,
								0x8, 0x6, 0x6, 0x5,
								0x3, 0x2,   0,   0,
								0x7, 0x7, 0x7, 0x7,
								0x4, 0x4, 0x4, 0x4,
								0x1, 0x1, 0x1, 0x1,
								0x1, 0x1, 0x1, 0x1  };

	// TODO: skip A-Bus for now
	for(int i = 0; i < 14; i++)
	{
		if (!(BIT(m_ism, i)) && BIT(m_ist, i))
		{
			m_current_irq_level = irq_level[i];
			m_current_vector = 0x40 + i;
			m_hostcpu->set_input_line(m_current_irq_level, ASSERT_LINE);
			m_ist &= ~(1 << i);
			return;
		}
	}
}

IRQ_CALLBACK_MEMBER(saturn_scu_device::irq_ack_cb)
{
	m_hostcpu->set_input_line(irqline, CLEAR_LINE);
	m_current_irq_level = 0;
	return m_current_vector;
}


void saturn_scu_device::vblank_out_w(int state)
{
	if(!state)
		return;

	dma_start_factor_ack(DMA_EVENT_VBLANKOUT);

	m_ist |= IST_VBLANK_OUT;
	test_pending_irqs();
	m_timer0_counter = 0;
}

void saturn_scu_device::vblank_in_w(int state)
{
	if(!state)
		return;

	dma_start_factor_ack(DMA_EVENT_VBLANKIN);

	m_ist |= IST_VBLANK_IN;
	test_pending_irqs();
}

void saturn_scu_device::hblank_in_w(int state)
{
	if(!state)
		return;

	dma_start_factor_ack(DMA_EVENT_HBLANKIN);
	m_ist |= IST_HBLANK_IN;

	// check if timer enabled first (diehard cares for sound, sets T0C = 0)
	if (m_tenb)
	{
		const bool timer0_hit = m_timer0_counter == m_t0c;
		if (timer0_hit)
		{
			dma_start_factor_ack(DMA_EVENT_TIMER0);
			m_ist |= IST_TIMER_0;
		}

		// Timer 1 conditions
		// - Mode is 0 (all scanlines)
		// - Mode is 1 and timer 0 is hit
		const bool timer1_hit = (timer0_hit || !m_t1md);
		if (timer1_hit)
		{
			m_timer1->adjust(attotime::from_ticks(m_t1s, this->clock() / 8));
		}
	}
	// NOTE: the counter still runs, it's the irq that fires if timer is enabled
	// also that this never fires if t0c & 0x200
	m_timer0_counter ++;
	m_timer0_counter &= 0x1ff;

	test_pending_irqs();
}

void saturn_scu_device::vdp1_end_w(int state)
{
	if(!state)
		return;

	dma_start_factor_ack(DMA_EVENT_VDP1);

	m_ist |= IST_VDP1_END;
	test_pending_irqs();
}

void saturn_scu_device::sound_req_w(int state)
{
	if(!state)
		return;

	dma_start_factor_ack(DMA_EVENT_SCSP);

	m_ist |= IST_SOUND_REQ;
	test_pending_irqs();
}

void saturn_scu_device::smpc_irq_w(int state)
{
	if(!state)
		return;

	m_ist |= IST_SMPC;
	test_pending_irqs();
}

void saturn_scu_device::scudsp_end_w(int state)
{
	if(!state)
		return;

	m_ist |= IST_DSP_END;
	test_pending_irqs();
}

//**************************************************************************
//  Miscellanea
//**************************************************************************

uint32_t saturn_scu_device::version_r()
{
	return 4; // correct for stock Saturn at least
}
