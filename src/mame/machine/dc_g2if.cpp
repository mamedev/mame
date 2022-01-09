// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/**************************************************************************************************

    Sega Dreamcast G2 System Bus I/F

**************************************************************************************************/

#include "emu.h"
#include "dc_g2if.h"


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
{
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------


void dc_g2if_device::device_add_mconfig(machine_config &config)
{
	//DEVICE(config, ...);
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
	u8 channel = param;
	m_dma[channel].in_progress = false;
	m_dma[channel].start = false;
	//printf("%02x\n", channel);
	m_int_w(channel, 1);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void dc_g2if_device::amap(address_map &map)
{
	// SB_AD*
	map(0x00, 0x03).rw(FUNC(dc_g2if_device::stag_r<0>), FUNC(dc_g2if_device::stag_w<0>));
	map(0x04, 0x07).rw(FUNC(dc_g2if_device::star_r<0>), FUNC(dc_g2if_device::star_w<0>));
	map(0x08, 0x0b).rw(FUNC(dc_g2if_device::len_r<0>), FUNC(dc_g2if_device::len_w<0>));
	map(0x0c, 0x0f).rw(FUNC(dc_g2if_device::dir_r<0>), FUNC(dc_g2if_device::dir_w<0>));
	map(0x10, 0x13).rw(FUNC(dc_g2if_device::tsel_r<0>), FUNC(dc_g2if_device::tsel_w<0>));
	map(0x14, 0x17).rw(FUNC(dc_g2if_device::en_r<0>), FUNC(dc_g2if_device::en_w<0>));
	map(0x18, 0x1b).rw(FUNC(dc_g2if_device::st_r<0>), FUNC(dc_g2if_device::st_w<0>));
	map(0x1c, 0x1f).rw(FUNC(dc_g2if_device::susp_r<0>), FUNC(dc_g2if_device::susp_w<0>));

	// SB_E1*

	// SB_E2*

	// SB_DD*
}

template <u8 ch> u32 dc_g2if_device::stag_r()
{
	return m_dma[ch].g2_addr;
}

// SB_**STAG
// G2 bus start address
template <u8 ch> void dc_g2if_device::stag_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma[ch].g2_addr);
}

template <u8 ch> u32 dc_g2if_device::star_r()
{
	return m_dma[ch].root_addr;
}

// SB_**STAR
// root bus (SH4) start address
template <u8 ch> void dc_g2if_device::star_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma[ch].root_addr);
}

template <u8 ch> u32 dc_g2if_device::len_r()
{
	return m_dma[ch].len;
}

/*
 * SB_**LEN
 * x--- ---- ---- ---- ---- ---- ---- ---- DMA transfer mode
 *                                         (0) Restart
 *                                         (1) End (enable clears to '0')
 * ---- ---x xxxx xxxx xxxx xxxx xxx- ---- DMA transfer length
 *                                         (all buses?)
 */
template <u8 ch> void dc_g2if_device::len_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma[ch].len);
	// TODO: throw log warning if reserved bits 30-25 and 4-0 are set
//  m_dma[ch].size = m_dma[ch].len & 0x7fffffff;
	m_dma[ch].size = m_dma[ch].len & 0x001fffe0;
	m_dma[ch].mode = bool(BIT(m_dma[ch].len, 31));
}

// TODO: following regs are supposedly single byte, but HW still accesses them as dword, is it a liability?

template <u8 ch> u32 dc_g2if_device::dir_r()
{
	return m_dma[ch].dir;
}

/*
 * SB_**DIR (transfer direction)
 * ---x (0) root -> G2 device RAM
 *      (1) root <- G2 device RAM
 */
template <u8 ch> void dc_g2if_device::dir_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_dma[ch].dir = bool(BIT(data, 0));
}

template <u8 ch> u32 dc_g2if_device::tsel_r()
{
	return m_dma[ch].tsel;
}

/*
 * SB_**TSEL (trigger select)
 * -x-- SUSPend enable
 * --x- (0) CPU trigger (along with st_w '1'),
 *      (1) HW trigger (with external pin/irq mechanism)
 * ---x External pin enable
 */
template <u8 ch> void dc_g2if_device::tsel_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_dma[ch].tsel = data & 7;
		m_dma[ch].hw_trigger = bool(BIT(m_dma[ch].tsel, 1));
	}
}


template <u8 ch> u32 dc_g2if_device::en_r()
{
	return m_dma[ch].enable;
}

/*
 * SB_**EN
 * ---x DMA enable
 *      (0) mask
 *      (1) enabled
 * Note: DMA transfer is aborted if this is written with a 0.
 */
template <u8 ch> void dc_g2if_device::en_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_dma[ch].enable = bool(BIT(data, 0));
		// TODO: suppress an in-progress DMA if this is disabled
	}
}

template <u8 ch> u32 dc_g2if_device::st_r()
{
	return m_dma[ch].in_progress & 1;
}

/*
 * SB_**ST
 * ---x DMA start/status
 *      (r) (0) DMA isn't running (1) DMA is in-progress
 *      (w) (1) starts a DMA (if hw_trigger is '0')
 */
template <u8 ch> void dc_g2if_device::st_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if (m_dma[ch].start == true)
		{
			// TODO: log warning for an in-flight attempt
			return;
		}

		m_dma[ch].start = bool(BIT(data, 0));

		if (m_dma[ch].enable && m_dma[ch].start && m_dma[ch].hw_trigger == false)
			dma_execute(ch);
	}
}

template <u8 ch> u32 dc_g2if_device::susp_r()
{
	return (m_dma[ch].in_progress == false) << 4;
}

template <u8 ch> void dc_g2if_device::susp_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		// TODO: log a suspend request (bit 0 off)
		// ...
	}
}

void dc_g2if_device::dma_execute(u8 channel)
{
	u32 src, dst, index, transfer_size;
	dst = m_dma[channel].g2_addr;
	src = m_dma[channel].root_addr;

	index = 0;

	transfer_size = m_dma[channel].size;
	/* 0 rounding size = 32 Mbytes */
	if (transfer_size == 0) { transfer_size = 0x200000; }

	if (m_dma[channel].dir == 1)
		std::swap(src, dst);

	//printf("%08x %08x %08x\n", src, dst, transfer_size);

	// TODO: punt with exception if SB_G2APRO isn't asserted along the way
	// TODO: raise debug signals if SB_G2DSTO / SB_G2TRTO aren't respected
	//       (shouldn't matter for AICA RAM?)
	for (; index < transfer_size; index += 2)
	{
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
		m_dma[channel].enable = 0;

	m_dma[channel].in_progress = true;

	m_dma[channel].end_timer->adjust(dma_time, channel);
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
			logerror("G2 I/F HW trigger channel %d (ISTNRM=%08x ISTEXT=%08x)\n", ch, normal_ist, ext_ist);
			m_g2if->dma_execute(ch);
		}
	}
}
