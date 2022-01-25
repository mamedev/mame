// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/**************************************************************************************************

    Sega Dreamcast G2 System Bus I/F

    TODO:
    - Single-step instead of transfering in one go;
    - Abort DMA if suspend mode is triggered;
    - Time Out mechanism thru DS# & TR# signals;
    - External pin enable in trigger select;
    - Create a pure abstract interface shared with PVR-DMA I/F
      (one channel, different max size, simpler tsel, no suspend,
       different security code & area protection);
    - DMA starts should send DDT requests and being notified back to use this i/f implementation
      anyway. Exact purpose is unknown, maybe it's for granting use of the bus?

**************************************************************************************************/

#include "emu.h"
#include "dc_g2if.h"

#define LOG_WARN    (1U << 1)
#define LOG_DMA     (1U << 2) // log DMA starts with CPU triggers (.tsel bit 1 == 0)
#define LOG_HWTRIG  (1U << 3) // log DMA starts with HW triggers (.tsel bit 1 == 1)
#define LOG_DMAEND  (1U << 4) // log DMA event ends
#define LOG_ILLEGAL (1U << 5) // log illegal/malformed addresses

#define VERBOSE (LOG_WARN | LOG_DMA | LOG_HWTRIG | LOG_DMAEND | LOG_ILLEGAL)
//#define LOG_OUTPUT_STREAM std::cout
#include "logmacro.h"

#define LOGWARN(...)      LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGDMA(...)       LOGMASKED(LOG_DMA, __VA_ARGS__)
#define LOGHWTRIG(...)    LOGMASKED(LOG_HWTRIG, __VA_ARGS__)
#define LOGDMAEND(...)    LOGMASKED(LOG_DMAEND, __VA_ARGS__)
#define LOGILLEGAL(...)   LOGMASKED(LOG_ILLEGAL, __VA_ARGS__)

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


// device type definition
DEFINE_DEVICE_TYPE(DC_G2IF, dc_g2if_device, "dc_g2if", "Sega Dreamcast G2 I/F System Bus")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************


//-------------------------------------------------
//  dc_g2if_device - constructor
//-------------------------------------------------


dc_g2if_device::dc_g2if_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DC_G2IF, tag, owner, clock)
	, m_host_space(*this, finder_base::DUMMY_TAG, -1)
	, m_int_w(*this)
	, m_error_ia_w(*this)
	, m_error_ov_w(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------


void dc_g2if_device::device_start()
{
	for (int i = 0; i < 4; i++)
	{
		m_dma[i].end_timer = timer_alloc(i);
	}

	m_int_w.resolve();
	m_error_ia_w.resolve();
	m_error_ov_w.resolve();

	save_item(STRUCT_MEMBER(m_dma, g2_addr));
	save_item(STRUCT_MEMBER(m_dma, root_addr));
	save_item(STRUCT_MEMBER(m_dma, len));
	save_item(STRUCT_MEMBER(m_dma, size));
	save_item(STRUCT_MEMBER(m_dma, mode));
	save_item(STRUCT_MEMBER(m_dma, dir));
	save_item(STRUCT_MEMBER(m_dma, enable));
	save_item(STRUCT_MEMBER(m_dma, in_progress));
	save_item(STRUCT_MEMBER(m_dma, start));
	save_item(STRUCT_MEMBER(m_dma, tsel));
	save_item(STRUCT_MEMBER(m_dma, hw_trigger));
	save_item(NAME(m_g2apro.top_addr));
	save_item(NAME(m_g2apro.bottom_addr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------


void dc_g2if_device::device_reset()
{
	for (int ch = 0; ch < 4; ch ++)
	{
		m_dma[ch].g2_addr = 0;
		m_dma[ch].root_addr = 0;
		m_dma[ch].len = 0;
		m_dma[ch].size = 0;
		m_dma[ch].mode = false;
		m_dma[ch].dir = false;
		m_dma[ch].enable = false;
		m_dma[ch].in_progress = false;
		m_dma[ch].start = false;
		m_dma[ch].tsel = 0;
		m_dma[ch].hw_trigger = false;
		m_dma[ch].end_timer->adjust(attotime::never);
	}
}

void dc_g2if_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	u8 channel = (u8)id;
	bool dma_result = (param == 1);
	m_dma[channel].in_progress = false;
	m_dma[channel].start = false;
	LOGDMAEND("DMA%d %s\n", id, dma_result ? "normal end" : "overflow error");

	if (dma_result)
		m_int_w(channel, 1);
	else
		m_error_ov_w(channel, 1);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

template <u8 Channel> void dc_g2if_device::channel_map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(dc_g2if_device::stag_r<Channel>), FUNC(dc_g2if_device::stag_w<Channel>));
	map(0x04, 0x07).rw(FUNC(dc_g2if_device::star_r<Channel>), FUNC(dc_g2if_device::star_w<Channel>));
	map(0x08, 0x0b).rw(FUNC(dc_g2if_device::len_r<Channel>), FUNC(dc_g2if_device::len_w<Channel>));
	map(0x0c, 0x0f).rw(FUNC(dc_g2if_device::dir_r<Channel>), FUNC(dc_g2if_device::dir_w<Channel>));
	map(0x10, 0x13).rw(FUNC(dc_g2if_device::tsel_r<Channel>), FUNC(dc_g2if_device::tsel_w<Channel>));
	map(0x14, 0x17).rw(FUNC(dc_g2if_device::en_r<Channel>), FUNC(dc_g2if_device::en_w<Channel>));
	map(0x18, 0x1b).rw(FUNC(dc_g2if_device::st_r<Channel>), FUNC(dc_g2if_device::st_w<Channel>));
	map(0x1c, 0x1f).rw(FUNC(dc_g2if_device::susp_r<Channel>), FUNC(dc_g2if_device::susp_w<Channel>));
}

// Instantiate channel maps
template void dc_g2if_device::channel_map<0>(address_map &map);
template void dc_g2if_device::channel_map<1>(address_map &map);
template void dc_g2if_device::channel_map<2>(address_map &map);
template void dc_g2if_device::channel_map<3>(address_map &map);

void dc_g2if_device::amap(address_map &map)
{
	// 0x5f7800-ff
	// SB_AD*
	map(0x00, 0x1f).m(FUNC(dc_g2if_device::channel_map<0>));
	// SB_E1*
	map(0x20, 0x3f).m(FUNC(dc_g2if_device::channel_map<1>));
	// SB_E2*
	map(0x40, 0x5f).m(FUNC(dc_g2if_device::channel_map<2>));
	// SB_DD*
	map(0x60, 0x7f).m(FUNC(dc_g2if_device::channel_map<3>));

	map(0x80, 0x83).r(FUNC(dc_g2if_device::g2id_r));

//  map(0x90, 0x93).rw SB_G2DSTO #DS timeout
//  map(0x94, 0x97).rw SB_G2TRTO #TR timeout
//  map(0x98, 0x9b).rw SB_G2MDMTO modem wait timeout
//  map(0x9c, 0x9f).rw SB_G2MDMW modem wait time

	map(0xbc, 0xbf).w(FUNC(dc_g2if_device::g2apro_w));

//  map(0xc0, 0xcb).r SB_AD*D live register reads (STAG, STAR, LEN)
//  map(0xd0, 0xdb).r SB_E1*D live register reads
//  map(0xe0, 0xeb).r SB_E2*D live register reads
//  map(0xf0, 0xfb).r SB_DD*D live register reads
}

template <u8 Channel> u32 dc_g2if_device::stag_r()
{
	return m_dma[Channel].g2_addr;
}

// SB_**STAG
// G2 bus start address
template <u8 Channel> void dc_g2if_device::stag_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma[Channel].g2_addr);

	if (!g2_address_check(m_dma[Channel].g2_addr))
	{
		LOGILLEGAL("%s: G2 illegal Address trap %08x (%08x)\n", machine().describe_context(), data, mem_mask);
		m_error_ia_w(Channel, 1);
	}
}

template <u8 Channel> u32 dc_g2if_device::star_r()
{
	return m_dma[Channel].root_addr;
}

// SB_**STAR
// root bus (SH4) start address
template <u8 Channel> void dc_g2if_device::star_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma[Channel].root_addr);

	if (!root_address_check(m_dma[Channel].root_addr))
	{
		LOGILLEGAL("%s: root illegal Address trap %08x (%08x)\n", machine().describe_context(), data, mem_mask);
		m_error_ia_w(Channel, 1);
	}
}

template <u8 Channel> u32 dc_g2if_device::len_r()
{
	return m_dma[Channel].len;
}

/*
 * SB_**LEN
 * x--- ---- ---- ---- ---- ---- ---- ---- DMA transfer mode
 *                                         (0) Restart
 *                                         (1) End (enable register clears to '0')
 * ---- ---x xxxx xxxx xxxx xxxx xxx- ---- DMA transfer length
 *                                         (all buses?)
 */
template <u8 Channel> void dc_g2if_device::len_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma[Channel].len);
	// log an attempt if any of the reserved bits 30-25 and 4-0 are set
	if (m_dma[Channel].len & 0x7fe0001f)
		LOGWARN("%s: DMA%d LEN setup %08x (mask=%08x)!\n", machine().describe_context(), data, mem_mask);
//  m_dma[Channel].size = m_dma[Channel].len & 0x7fffffff;
	m_dma[Channel].size = m_dma[Channel].len & 0x001fffe0;
	m_dma[Channel].mode = bool(BIT(m_dma[Channel].len, 31));
}

// TODO: following regs are supposedly single byte, but HW still accesses them as dword, is it a liability?

template <u8 Channel> u32 dc_g2if_device::dir_r()
{
	return m_dma[Channel].dir;
}

/*
 * SB_**DIR (transfer direction)
 * ---x (0) root -> G2 device RAM
 *      (1) root <- G2 device RAM
 */
template <u8 Channel> void dc_g2if_device::dir_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_dma[Channel].dir = bool(BIT(data, 0));
}

template <u8 Channel> u32 dc_g2if_device::tsel_r()
{
	return m_dma[Channel].tsel;
}

/*
 * SB_**TSEL (trigger select)
 * -x-- SUSPend enable
 * --x- (0) CPU trigger (along with st_w '1'),
 *      (1) HW trigger (with external pin/irq mechanism)
 * ---x External pin enable
 */
template <u8 Channel> void dc_g2if_device::tsel_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_dma[Channel].tsel = data & 7;
		m_dma[Channel].hw_trigger = bool(BIT(m_dma[Channel].tsel, 1));
	}
}


template <u8 Channel> u32 dc_g2if_device::en_r()
{
	return m_dma[Channel].enable;
}

/*
 * SB_**EN
 * ---x DMA enable
 *      (0) mask
 *      (1) enabled
 * Note: DMA transfer is aborted if this is written with a 0.
 */
template <u8 Channel> void dc_g2if_device::en_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_dma[Channel].enable = bool(BIT(data, 0));
		// TODO: suppresses an in-progress DMA if this is disabled
	}
}

template <u8 Channel> u32 dc_g2if_device::st_r()
{
	return m_dma[Channel].in_progress & 1;
}

/*
 * SB_**ST
 * ---x DMA start/status
 *      (r) (0) DMA isn't running (1) DMA is in-progress
 *      (w) (1) starts a DMA (if hw_trigger is '0')
 */
template <u8 Channel> void dc_g2if_device::st_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if (m_dma[Channel].start == true)
		{
			LOGWARN("%s: DMA%d attempt to start an in-flight\n", machine().describe_context(), Channel);
			return;
		}

		m_dma[Channel].start = bool(BIT(data, 0));

		if (m_dma[Channel].enable && m_dma[Channel].start && m_dma[Channel].hw_trigger == false)
		{
			LOGDMA("%s: DMA%d root=%08x g2=%08x dir=G2%sroot (%d)\n    size=%08x (len=%08x) mode=DMA %s\n",
				machine().describe_context(), Channel,
				m_dma[Channel].root_addr, m_dma[Channel].g2_addr, m_dma[Channel].dir ? "->" : "<-",
				m_dma[Channel].dir,
				m_dma[Channel].size, m_dma[Channel].len, m_dma[Channel].mode ? "end" : "restart"
			);
			dma_execute(Channel);
		}
	}
}

// --x- ---- (r/o) DMA request input state (from external bus?)
// ---x ---- (r/o) DMA suspend/stop status (active low)
// ---- ---x (w) DMA suspend request
template <u8 Channel> u32 dc_g2if_device::susp_r()
{
	return (m_dma[Channel].in_progress == false) << 4;
}

template <u8 Channel> void dc_g2if_device::susp_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		// TODO: unemulated suspend mode
		if (data & 1)
		{
			LOGWARN("%s: DMA%d suspend write %08x %08x\n",
				machine().describe_context(),
				Channel, data, mem_mask
			);
			// ...
		}
	}
}

//**************************************************************************
//  Misc. registers
//**************************************************************************

// SB_G2ID
// 0001 ---- Holly v1.0
// ---- 0011 G2 version
u32 dc_g2if_device::g2id_r()
{
	LOGWARN("%s: read ID\n", machine().describe_context());
	return 0x12;
}

// SB_G2APRO
// xxxx xxxx xxxx xxxx ---- ---- ---- ---- Unlock register (must be == 0x4659)
// ---- ---- ---- ---- -xxx xxxx ---- ---- Top range (start address)
// ---- ---- ---- ---- ---- ---- -xxx xxxx Bottom range (end address)
// all channels follows this ruleset
void dc_g2if_device::g2apro_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask != 0xffffffff)
	{
		LOGWARN("%s: g2apro_w attempt to write %08x with a non-dword (mem_mask=%08x)\n", machine().describe_context(), data, mem_mask);
		return;
	}
	const u16 security_code = data >> 16;

	if (security_code != 0x4659)
	{
		LOGWARN("%s: g2apro_w attempt to write %08x without satisfying security code condition\n", machine().describe_context(), data);
		return;
	}

	const u16 top_range = (data & 0x7f00) >> 8;
	const u16 bottom_range = (data & 0x7f);

	m_g2apro.top_addr = (top_range << 20) | 0x08000000;
	m_g2apro.bottom_addr = (bottom_range << 20) | 0x080fffff;

	LOGILLEGAL("%s: g2apro_w set top=%08x bottom=%08x (%08x)\n",
		machine().describe_context(),
		m_g2apro.top_addr, m_g2apro.bottom_addr, data
	);
}

//**************************************************************************
//  DMA implementation
//**************************************************************************

inline bool dc_g2if_device::root_address_check(u32 offset)
{
	const u8 area = (offset >> 26) & 7;
	// root iA is generated by accessing outside System RAM or texture/framebuffer RAM
	return area == 1 || area == 3;
}

inline bool dc_g2if_device::g2_address_check(u32 offset)
{
	const u8 area = (offset >> 26) & 7;
	// g2 iA is generated by accessing outside:
	// - area == 0 for AD/E1/E2 buses
	// - area == 5 for DD
	return area == 0 || area == 5;
}

inline bool dc_g2if_device::root_overflow_check(u32 offset, u8 channel)
{
	bool result = offset >= m_g2apro.top_addr && offset <= m_g2apro.bottom_addr;
	if (result == false)
		LOGILLEGAL("DMA%d overflow abort root=%08x\n",  channel, offset);

	return result;
}

void dc_g2if_device::dma_execute(u8 channel)
{
	u32 src, dst, index, transfer_size;
	dst = m_dma[channel].g2_addr;
	src = m_dma[channel].root_addr;

	// Punt if attempts to go beyond the allocated buses
	// TODO: should require two extra cycles for fetching addresses first
	if (!root_address_check(src) || !g2_address_check(dst))
	{
		LOGILLEGAL("%s: DMA%d illegal address attempt root=%08x g2=%08x\n",
			machine().describe_context(),
			channel, src, dst
		);
		m_dma[channel].in_progress = false;
		m_dma[channel].start = false;
		m_dma[channel].enable = false;
		m_error_ia_w(channel, 1);
		return;
	}

	index = 0;

	transfer_size = m_dma[channel].size;
	/* 0 rounding size = 32 Mbytes */
	if (transfer_size == 0) { transfer_size = 0x200000; }

	if (m_dma[channel].dir == 1)
		std::swap(src, dst);

	// notify that a DMA is in progress
	// ofc this should rather transfer one word at a time,
	// we currently don't do that for performance reasons ...
	m_dma[channel].in_progress = true;

	bool dma_result = true;

	for (; index < transfer_size; index += 2)
	{
		// assert that root address is inside the g2apro range
		if (!root_overflow_check(m_dma[channel].dir ? dst : src, channel))
		{
			dma_result = false;
			break;
		}
		// TODO: raise debug signals if SB_G2DSTO / SB_G2TRTO aren't respected
		// Shouldn't matter for AICA RAM,
		// it does in loopchk g2 test 0304 when it tries to write to
		// expansion bus (where nothing lies on stock DC)
		m_host_space->write_word(dst, m_host_space->read_word(src));
		src += 2;
		dst += 2;
	}

	// update the params
	// Note: if you trigger an instant DMA IRQ trigger, sfz3ugd doesn't play any BGM.
	// G2 bus is 16 bits @ 25 MHz according to Fig. 2-1
	// TODO: reported limit output for AICA DMA is set at 11.3MB/s while the others at 24.0/26.0
	// bus contention ftw ...
	const attotime dma_time = attotime::from_ticks(index / 2, clock());

	m_dma[channel].g2_addr = dst;
	m_dma[channel].root_addr = src;
	// TODO: how len copes with updates?
	m_dma[channel].len = 0;
	// clear mask flag if the DMA transfer mode is in End mode
	// (Restart mode leaves this set to true)
	if (m_dma[channel].mode == true)
		m_dma[channel].enable = false;

	m_dma[channel].end_timer->adjust(dma_time, dma_result);
}

/*
 * normal_ist: SB_G2DTNRM & SB_ISTNRM
 *             (triggers a DMA if selected irq in former gets triggered)
 * ext_ist: SB_G2DTEXT & SB_ISTEXT
 *          (triggers a DMA if external pin is triggered)
 */
void dc_g2if_device::hw_irq_trigger_hs(u32 normal_ist, u32 ext_ist)
{
	// TODO: is latter requiring .tsel bit 0 == 1?
	bool hw_ist_enable = normal_ist || ext_ist;

	if (hw_ist_enable == false)
		return;

	for (int ch = 0; ch < 4; ch++)
	{
		if (m_dma[ch].hw_trigger & m_dma[ch].enable)
		{
			LOGHWTRIG("HW trigger channel %d (ISTNRM=%08x ISTEXT=%08x)\n", ch, normal_ist, ext_ist);
			dma_execute(ch);
		}
	}
}
