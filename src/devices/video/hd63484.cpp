// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Sandro Ronco
/***************************************************************************

    HD63484 ACRTC

    TODO:
    - DMA handshake (DREQ/DACK/DONE), character screen mode, cursors,
      blink, video attribute outputs, interlaced rendering;

***************************************************************************/

#include "emu.h"
#include "hd63484.h"

#include "screen.h"

#include <numbers>
#include <unordered_set>
#include <utility>
#include <vector>


#define LOG 0
#define FIFO_LOG 0
#define CMD_LOG 0



enum
{
	STREAM_NONE = 0,
	STREAM_RPTN,
	STREAM_DRD
};

//-------------------------------------------------
//  hd63484_device - constructor
//-------------------------------------------------

hd63484_device::hd63484_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HD63484, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_display_cb(*this),
	m_character_cb(*this),
	m_irq_cb(*this),
	m_dreq_cb(*this),
	m_done_cb(*this),
	m_auto_configure_screen(true),
	m_external_skew(0),
	m_ar(0),
	m_sr(0),
	m_irq_state(CLEAR_LINE),
	m_dreq_state(CLEAR_LINE),
	m_fifo_ptr(-1),
	m_fifo_r_ptr(-1),
	m_cr(0),
	m_param_ptr(0),
	m_rwp_dn(0),
	m_org_dpa(0),
	m_org_dn(0),
	m_org_dpd(0),
	m_cl0(0),
	m_cl1(0), m_ccmp(0), m_mask(0), m_cpx(0),
	m_stream(STREAM_NONE),
	m_dcr(0),
	m_space_config("videoram", ENDIANNESS_BIG, 16, 20, -1),
	m_char_space_config("charram", ENDIANNESS_BIG, 16, 16, -1),
	m_char_space(nullptr)
{
}

enum
{
	FIFO_READ = 0,
	FIFO_WRITE
};

enum
{
	FIFO_EMPTY = -1,
	FIFO_PARAMETER,
	FIFO_COMMAND
};

#define HD63484_SR_CER 0x80 // Command Error
#define HD63484_SR_ARD 0x40 // Area Detect
#define HD63484_SR_CED 0x20 // Command End
#define HD63484_SR_LPD 0x10 // Light Pen Strobe Detect
#define HD63484_SR_RFF 0x08 // Read FIFO Full
#define HD63484_SR_RFR 0x04 // Read FIFO Ready
#define HD63484_SR_WFR 0x02 // Write FIFO Ready
#define HD63484_SR_WFE 0x01 // Write FIFO Empty


static const char *const acrtc_regnames[0x100/2] =
{
	"FIFO Entry", // 0x00
	"Command Control (CCR)", // 0x02
	"Operation Mode (OMR)", // 0x04
	"Display Control (DCR)", // 0x06
	"(Undefined)", // 0x08
	"(Undefined)", // 0x0a
	"(Undefined)", // 0x0c
	"(Undefined)", // 0x0e
	"(Undefined)", // 0x10
	"(Undefined)", // 0x12
	"(Undefined)", // 0x14
	"(Undefined)", // 0x16
	"(Undefined)", // 0x18
	"(Undefined)", // 0x1a
	"(Undefined)", // 0x1c
	"(Undefined)", // 0x1e
	"(Undefined)", // 0x20
	"(Undefined)", // 0x22
	"(Undefined)", // 0x24
	"(Undefined)", // 0x26
	"(Undefined)", // 0x28
	"(Undefined)", // 0x2a
	"(Undefined)", // 0x2c
	"(Undefined)", // 0x2e
	"(Undefined)", // 0x30
	"(Undefined)", // 0x32
	"(Undefined)", // 0x34
	"(Undefined)", // 0x36
	"(Undefined)", // 0x38
	"(Undefined)", // 0x3a
	"(Undefined)", // 0x3c
	"(Undefined)", // 0x3e
	"(Undefined)", // 0x40
	"(Undefined)", // 0x42
	"(Undefined)", // 0x44
	"(Undefined)", // 0x46
	"(Undefined)", // 0x48
	"(Undefined)", // 0x4a
	"(Undefined)", // 0x4c
	"(Undefined)", // 0x4e
	"(Undefined)", // 0x50
	"(Undefined)", // 0x52
	"(Undefined)", // 0x54
	"(Undefined)", // 0x56
	"(Undefined)", // 0x58
	"(Undefined)", // 0x5a
	"(Undefined)", // 0x5c
	"(Undefined)", // 0x5e
	"(Undefined)", // 0x60
	"(Undefined)", // 0x62
	"(Undefined)", // 0x64
	"(Undefined)", // 0x66
	"(Undefined)", // 0x68
	"(Undefined)", // 0x6a
	"(Undefined)", // 0x6c
	"(Undefined)", // 0x6e
	"(Undefined)", // 0x70
	"(Undefined)", // 0x72
	"(Undefined)", // 0x74
	"(Undefined)", // 0x76
	"(Undefined)", // 0x78
	"(Undefined)", // 0x7a
	"(Undefined)", // 0x7c
	"(Undefined)", // 0x7e
	"Raster Count (RCR)", // 0x80
	"Horizontal Sync (HSR)", // 0x82
	"Horizontal Display (HDR)", // 0x84
	"Vertical Sync (VSR)", // 0x86
	"Vertical Display (VDR)", // 0x88
	"Split Screen Width (SSW) 0x8a", // 0x8a
	"Split Screen Width (SSW) 0x8c", // 0x8c
	"Split Screen Width (SSW) 0x8e", // 0x8e
	"Blink Control (BCR)", // 0x90
	"Horizontal Window Display (HWR)", // 0x92
	"Vertical Window Display (VWR) 0x94", // 0x94
	"Vertical Window Display (VWR) 0x96", // 0x96
	"Graphic Cursor (GCR0)", // 0x98
	"Graphic Cursor (GCR1)", // 0x9a
	"Graphic Cursor (GCR2)", // 0x9c
	"(Undefined)", // 0x9e
	"(Undefined)", // 0xa0
	"(Undefined)", // 0xa2
	"(Undefined)", // 0xa4
	"(Undefined)", // 0xa6
	"(Undefined)", // 0xa8
	"(Undefined)", // 0xaa
	"(Undefined)", // 0xac
	"(Undefined)", // 0xae
	"(Undefined)", // 0xb0
	"(Undefined)", // 0xb2
	"(Undefined)", // 0xb4
	"(Undefined)", // 0xb6
	"(Undefined)", // 0xb8
	"(Undefined)", // 0xba
	"(Undefined)", // 0xbc
	"(Undefined)", // 0xbe
	// upper screen
	"Raster Address 0 (RAR0)", // 0xc0
	"Memory Width 0 (MWR0)", // 0xc2
	"Start Address 0 (SAR0) 0xc4", // 0xc4
	"Start Address 0 (SAR0) 0xc6", // 0xc6
	// base screen
	"Raster Address 1 (RAR1)", // 0xc8
	"Memory Width 1 (MWR1)", // 0xca
	"Start Address 1 (SAR1) 0xcc", // 0xcc
	"Start Address 1 (SAR1) 0xce", // 0xce
	// lower screen
	"Raster Address 2 (RAR2)", // 0xd0
	"Memory Width 2 (MWR2)", // 0xd2
	"Start Address 2 (SAR2) 0xd4", // 0xd4
	"Start Address 2 (SAR2) 0xd6", // 0xd6
	// window screen
	"Raster Address 3 (RAR3)", // 0xd8
	"Memory Width 3 (MWR3)", // 0xda
	"Start Address 3 (SAR3) 0xdc", // 0xdc
	"Start Address 3 (SAR3) 0xde", // 0xde
	// block cursor 1
	"Block Cursor 1 (BCUR1) 0xe0",
	"Block Cursor 1 (BCUR1) 0xe2",
	// block cursor 2
	"Block Cursor 2 (BCUR2) 0xe4",
	"Block Cursor 2 (BCUR2) 0xe6",
	"Cursor Definition (CDR)",
	"Zoom Factor (ZFR)",
	"Lightpen Address (LPAR) 0xec",
	"Lightpen Address (LPAR) 0xee",
	"(Undefined)", // 0xf0
	"(Undefined)", // 0xf2
	"(Undefined)", // 0xf4
	"(Undefined)", // 0xf6
	"(Undefined)", // 0xf8
	"(Undefined)", // 0xfa
	"(Undefined)", // 0xfc
	"(Undefined)"  // 0xfe
};

static const char *const wpr_regnames[0x20] =
{
	"Color 0 (CL0)",
	"Color 1 (CL1)",
	"Color Comparison (COMP)",
	"Edge Color (EDG)",
	"Mask (MASK)",
	"Pattern RAM Control (PRC) 0x05",
	"Pattern RAM Control (PRC) 0x06",
	"Pattern RAM Control (PRC) 0x07",
	"Area Definition (ADR) 0x08",
	"Area Definition (ADR) 0x09",
	"Area Definition (ADR) 0x0a",
	"Area Definition (ADR) 0x0b",
	"Read Write Pointer (RWP) H",
	"Read Write Pointer (RWP) L",
	"(Undefined)",
	"(Undefined)",
	"Drawing Pointer (DP) H",
	"Drawing Pointer (DP) L",
	"Current Pointer (CP) H",
	"Current Pointer (CP) L",
	"(Undefined)", // 0x14
	"(Undefined)",
	"(Undefined)", // 0x16
	"(Undefined)",
	"(Undefined)", // 0x18
	"(Undefined)",
	"(Undefined)", // 0x1a
	"(Undefined)",
	"(Undefined)", // 0x1c
	"(Undefined)",
	"(Undefined)", // 0x1e
	"(Undefined)"
};

enum
{
	COMMAND_INVALID = -1,
	COMMAND_ORG,
	COMMAND_WPR,
	COMMAND_RPR,
	COMMAND_WPTN,
	COMMAND_RPTN,
	COMMAND_DRD,
	COMMAND_DWT,
	COMMAND_DMOD,
	COMMAND_RD,
	COMMAND_WT,
	COMMAND_MOD,
	COMMAND_CLR,
	COMMAND_SCLR,
	COMMAND_CPY,
	COMMAND_SCPY,
	COMMAND_AMOVE,
	COMMAND_RMOVE,
	COMMAND_ALINE,
	COMMAND_RLINE,
	COMMAND_ARCT,
	COMMAND_RRCT,
	COMMAND_APLL,
	COMMAND_RPLL,
	COMMAND_APLG,
	COMMAND_RPLG,
	COMMAND_CRCL,
	COMMAND_ELPS,
	COMMAND_AARC,
	COMMAND_RARC,
	COMMAND_AEARC,
	COMMAND_REARC,
	COMMAND_AFRCT,
	COMMAND_RFRCT,
	COMMAND_PAINT,
	COMMAND_DOT,
	COMMAND_PTN,
	COMMAND_AGCPY,
	COMMAND_RGCPY
};

#define HD63484_COMMAND_ORG      0x0400  //              p: 2
#define HD63484_COMMAND_WPR      0x0800  // & ~0x1f      p: 1
#define HD63484_COMMAND_RPR      0x0c00  // & ~0x1f      p: 0
#define HD63484_COMMAND_WPTN     0x1800  // & ~0xf       p: 1 + n
#define HD63484_COMMAND_RPTN     0x1c00  // & ~0xf       p: 1
#define HD63484_COMMAND_DRD      0x2400  //              p: 2
#define HD63484_COMMAND_DWT      0x2800  //              p: 2
#define HD63484_COMMAND_DMOD     0x2c00  // & ~3         p: 2
#define HD63484_COMMAND_RD       0x4400  //              p: 0
#define HD63484_COMMAND_WT       0x4800  //              p: 1
#define HD63484_COMMAND_MOD      0x4c00  // & ~3         p: 1
#define HD63484_COMMAND_CLR      0x5800  //              p: 3
#define HD63484_COMMAND_SCLR     0x5c00  // & ~3         p: 3
#define HD63484_COMMAND_CPY      0x6000  // & ~0x0f03    p: 4
#define HD63484_COMMAND_SCPY     0x7000  // & ~0x0f03    p: 4
#define HD63484_COMMAND_AMOVE    0x8000  //              p: 2
#define HD63484_COMMAND_RMOVE    0x8400  //              p: 2
#define HD63484_COMMAND_ALINE    0x8800  // & ~0x00ff    p: 2
#define HD63484_COMMAND_RLINE    0x8c00  // & ~0x00ff    p: 2
#define HD63484_COMMAND_ARCT     0x9000  // & ~0x00ff    p: 2
#define HD63484_COMMAND_RRCT     0x9400  // & ~0x00ff    p: 2
#define HD63484_COMMAND_APLL     0x9800  // & ~0x00ff    p: 1 + n
#define HD63484_COMMAND_RPLL     0x9c00  // & ~0x00ff    p: 1 + n
#define HD63484_COMMAND_APLG     0xa000  // & ~0x00ff    p: 1 + n
#define HD63484_COMMAND_RPLG     0xa400  // & ~0x00ff    p: 1 + n
#define HD63484_COMMAND_CRCL     0xa800  // & ~0x01ff    p: 1
#define HD63484_COMMAND_ELPS     0xac00  // & ~0x01ff    p: 3
#define HD63484_COMMAND_AARC     0xb000  // & ~0x01ff    p: 4
#define HD63484_COMMAND_RARC     0xb400  // & ~0x01ff    p: 4
#define HD63484_COMMAND_AEARC    0xb800  // & ~0x01ff    p: 6
#define HD63484_COMMAND_REARC    0xbc00  // & ~0x01ff    p: 6
#define HD63484_COMMAND_AFRCT    0xc000  // & ~0x00ff    p: 2
#define HD63484_COMMAND_RFRCT    0xc400  // & ~0x00ff    p: 2
#define HD63484_COMMAND_PAINT    0xc800  // & ~0x01ff    p: 0
#define HD63484_COMMAND_DOT      0xcc00  // & ~0x00ff    p: 0
#define HD63484_COMMAND_PTN      0xd000  // & ~0x0fff    p: 1
#define HD63484_COMMAND_AGCPY    0xe000  // & ~0x0fff    p: 4
#define HD63484_COMMAND_RGCPY    0xf000  // & ~0x0fff    p: 4


/*-------------------------------------------------
    ROM( hd63484 )
-------------------------------------------------*/

// devices
DEFINE_DEVICE_TYPE(HD63484, hd63484_device, "hd63484", "Hitachi HD63484 ACRTC")


ROM_START( hd63484 )
	ROM_REGION( 0x100, "hd63484", 0 )
	ROM_LOAD( "hd63484.bin", 0x000, 0x100, NO_DUMP ) /* internal control ROM */
ROM_END

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector hd63484_device::memory_space_config() const
{
	// the character frame buffer space only exists when the machine maps it
	space_config_vector v { std::make_pair(AS_GRAPHIC, &m_space_config) };
	if (has_configured_map(AS_CHARACTER))
		v.emplace_back(AS_CHARACTER, &m_char_space_config);
	return v;
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *hd63484_device::device_rom_region() const
{
	return ROM_NAME( hd63484 );
}

//-------------------------------------------------
//  readword - read a word at the given address
//-------------------------------------------------

inline uint16_t hd63484_device::readword(offs_t address, int screen)
{
	// each screen's CHR bit selects which of the two frame buffers it
	// resides in; addresses wrap at the space limit (1M/64K words)
	if (m_char_space != nullptr && m_mwr_chr[screen & 3])
		return m_char_space->read_word(address & 0xffff);

	return space().read_word(address & 0xfffff);
}


//-------------------------------------------------
//  writeword - write a word at the given address
//-------------------------------------------------

inline void hd63484_device::writeword(offs_t address, uint16_t data, int screen)
{
	if (m_char_space != nullptr && m_mwr_chr[screen & 3])
		m_char_space->write_word(address & 0xffff, data);
	else
		space().write_word(address & 0xfffff, data);
}


inline void hd63484_device::inc_ar(int value)
{
	if(m_ar & 0x80)
	{
		if (m_ar + value > 0xff)    // TODO: what happens if it overflows?
			logerror("HD63484 '%s': Address Register overflows 0x%02x\n", tag(), m_ar + value);

		m_ar = (m_ar + value) & 0xff;
	}
}

inline void hd63484_device::update_irq()
{
	int const state = (m_sr & m_ccr & 0xff) ? ASSERT_LINE : CLEAR_LINE;
	if (state != m_irq_state)
	{
		m_irq_state = state;
		m_irq_cb(state);
	}
}

inline void hd63484_device::fifo_w_clear()
{
	int i;

	for (i = 0; i < 16; i++)
		m_fifo[i] = 0;

	m_fifo_ptr = -1;

	m_sr |= HD63484_SR_WFR;
	m_sr |= HD63484_SR_WFE;

	update_irq();
}

inline void hd63484_device::queue_w(uint8_t data)
{
	if (m_fifo_ptr < 15)
	{
		m_fifo_ptr++;

		m_fifo[m_fifo_ptr] = data;

		// ready until the FIFO is completely full (8 words/16 bytes)
		if (m_fifo_ptr == 15)
			m_sr &= ~HD63484_SR_WFR;

		m_sr &= ~HD63484_SR_WFE;

		update_irq();
	}
	else
	{
		// TODO what happen? somebody set us up the bomb
		logerror("FIFO?\n");
	}
}

inline void hd63484_device::dequeue_w(uint8_t *data)
{
	int i;

	*data = m_fifo[0];

	if (m_fifo_ptr > -1)
	{
		for (i = 0; i < 15; i++)
			m_fifo[i] = m_fifo[i + 1];

		m_fifo[15] = 0;

		m_fifo_ptr--;

		m_sr |= HD63484_SR_WFR;

		if (m_fifo_ptr == -1)
			m_sr |= HD63484_SR_WFE;

		update_irq();
	}
}

inline void hd63484_device::fifo_r_clear()
{
	int i;

	for (i = 0; i < 16; i++)
		m_fifo_r[i] = 0;

	m_fifo_r_ptr = -1;

	m_sr &= ~HD63484_SR_RFR;
	m_sr &= ~HD63484_SR_RFF;

	update_irq();
}

inline void hd63484_device::queue_r(uint8_t data)
{
	if (m_fifo_r_ptr < 15)
	{
		m_fifo_r_ptr++;

		m_fifo_r[m_fifo_r_ptr] = data;

		if (m_fifo_r_ptr == 15)
			m_sr |= HD63484_SR_RFF;

		m_sr |= HD63484_SR_RFR;

		update_irq();
	}
	else
	{
		// TODO what happen? somebody set us up the bomb
		logerror("FIFO?\n");
	}
}

inline void hd63484_device::dequeue_r(uint8_t *data)
{
	int i;

	*data = m_fifo_r[0];

	if (m_fifo_r_ptr > -1)
	{
		for (i = 0; i < 15; i++)
			m_fifo_r[i] = m_fifo_r[i + 1];

		m_fifo_r[15] = 0;

		m_fifo_r_ptr--;

		m_sr &= ~HD63484_SR_RFF;

		if (m_fifo_r_ptr == -1)
			m_sr &= ~HD63484_SR_RFR;

		if (m_stream != STREAM_NONE)
			stream_fill();

		update_irq();
	}
}

// resume a data transfer command that is still streaming words into the
// read FIFO, as far as the available FIFO space allows
void hd63484_device::stream_fill()
{
	while (m_stream != STREAM_NONE && m_fifo_r_ptr < 14)
	{
		uint16_t data = 0;
		bool const was_rptn = m_stream == STREAM_RPTN;

		switch (m_stream)
		{
			case STREAM_RPTN:
				data = m_pram[(m_dma_d0 + (m_cr & 0x0f)) & 0x0f];
				m_dma_d0++;
				if (m_dma_d0 >= m_dn)
					m_stream = STREAM_NONE;
				break;

			case STREAM_DRD:
			{
				int16_t const ax = (int16_t)m_pr[0];
				int16_t const ay = (int16_t)m_pr[1];

				data = readword((m_rwp[m_rwp_dn] - m_dma_d1 * m_mwr[m_rwp_dn] + m_dma_d0) & 0xfffff, m_rwp_dn);

				if (m_dma_d0 != ax)
					m_dma_d0 += (ax < 0) ? -1 : 1;
				else
				{
					m_dma_d0 = 0;
					if (m_dma_d1 != ay)
						m_dma_d1 += (ay < 0) ? -1 : 1;
					else
					{
						int const d1_inc = (ay < 0) ? -1 : 1;
						m_rwp[m_rwp_dn] -= (ay + d1_inc) * m_mwr[m_rwp_dn];
						m_rwp[m_rwp_dn] &= 0xfffff;
						m_stream = STREAM_NONE;
					}
				}
				break;
			}
		}

		queue_r((data >> 8) & 0xff);
		queue_r((data >> 0) & 0xff);

		if (m_stream == STREAM_NONE)
		{
			if (was_rptn)
				command_end_seq(4 * m_dn + 10);
			else
			{
				uint32_t const x = abs((int16_t)m_pr[0]) + 1;
				uint32_t const yc = abs((int16_t)m_pr[1]) + 1;
				command_end_seq((4 * x + 8) * yc + 12 * ((x * yc + 7) / 8) + 65);
			}
		}
	}
}

int hd63484_device::get_ppmc()
{
	int gai = (m_omr >> 4) & 0x07;
	int acm = (m_omr & 0x08) ? 2 : 1;
	int ppw = 16 / get_bpp();

	switch (gai)
	{
		case 0: case 1: case 2: case 3: // address incremented by 1, 2, 4 or 8 words
			return (ppw << gai) / acm;
		case 4:                         // address incremented by 16 words
			return (ppw << 4) / acm;
		case 6: case 7:                 // address incremented by 1 every two display cycles
			return std::max(ppw / 2 / acm, 1);
		default:                        // 5: address not incremented
			return ppw / acm;
	}
}

//-------------------------------------------------
//  recompute_parameters -
//-------------------------------------------------

inline void hd63484_device::recompute_parameters()
{
	if (!m_auto_configure_screen)
		return;

	if (LOG)
	{
		logerror("HC %d HSW %d HDS %d HDW %d HWS %d HWW %d\n",m_hc,m_hsw,m_hds,m_hdw,m_hws,m_hww);
		logerror("VC %d VDS %d VSW %d VWS %d VWW %d\n",m_vc,m_vds,m_vsw,m_vws,m_vww);
		logerror("SP0 %d SP1 %d SP2 %d\n",m_sp[0],m_sp[1],m_sp[2]);
	}

	int ppmc = get_ppmc();
	int htotal = m_hc * ppmc;
	int hbend = (m_hsw + m_hds + m_external_skew) * ppmc;
	int hbstart = hbend + m_hdw * ppmc;
	int vbstart = m_vds + m_sp[1];
	if (BIT(m_dcr, 13)) vbstart += m_sp[0];
	if (BIT(m_dcr, 11)) vbstart += m_sp[2];

	// bail out if screen params aren't valid
	if (m_hc == 0 || m_vc == 0 || hbstart > htotal || vbstart <= m_vds)
		return;

	rectangle visarea = screen().visible_area();
	visarea.set(hbend, hbstart - 1, m_vds, vbstart - 1);
	attotime frame_period = screen().frame_period();
	if (clock() != 0)
	{
		// a memory cycle is two 2CLK periods in all access modes
		attotime const line = clocks_to_attotime(2 * m_hc);
		switch (m_omr & 0x03)
		{
			case 2: // interlace sync: dummy raster adds half a line per field
				frame_period = line * m_vc + line / 2;
				break;
			case 3: // interlace sync & video: VC counts both fields
				frame_period = line * m_vc / 2;
				break;
			default: // non-interlace
				frame_period = line * m_vc;
				break;
		}
	}         
	screen().configure(htotal, m_vc, visarea, frame_period);
	if (LOG)
		logerror("ACRTC: full %dx%d vis (%d, %d)-(%d, %d)\n", htotal, m_vc, visarea.left(), visarea.top(), visarea.right(), visarea.bottom());
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

int hd63484_device::translate_command(uint16_t data)
{
	switch (data & 0xf000) // using top 4 bits for decoding
	{
		case HD63484_COMMAND_CPY:    return COMMAND_CPY;
		case HD63484_COMMAND_SCPY:   return COMMAND_SCPY;
		case HD63484_COMMAND_PTN:    return COMMAND_PTN;
		case HD63484_COMMAND_AGCPY:  return COMMAND_AGCPY;
		case HD63484_COMMAND_RGCPY:  return COMMAND_RGCPY;
	}

	switch (data & 0xfc00) // using top 6 bits for decoding
	{
		case HD63484_COMMAND_ORG:    return COMMAND_ORG;
		case HD63484_COMMAND_WPR:    return COMMAND_WPR;
		case HD63484_COMMAND_RPR:    return COMMAND_RPR;
		case HD63484_COMMAND_WPTN:   return COMMAND_WPTN;
		case HD63484_COMMAND_RPTN:   return COMMAND_RPTN;
		case HD63484_COMMAND_DRD:    return COMMAND_DRD;
		case HD63484_COMMAND_DWT:    return COMMAND_DWT;
		case HD63484_COMMAND_DMOD:   return COMMAND_DMOD;
		case HD63484_COMMAND_RD:     return COMMAND_RD;
		case HD63484_COMMAND_WT:     return COMMAND_WT;
		case HD63484_COMMAND_MOD:    return COMMAND_MOD;
		case HD63484_COMMAND_CLR:    return COMMAND_CLR;
		case HD63484_COMMAND_SCLR:   return COMMAND_SCLR;
		case HD63484_COMMAND_AMOVE:  return COMMAND_AMOVE;
		case HD63484_COMMAND_RMOVE:  return COMMAND_RMOVE;
		case HD63484_COMMAND_ALINE:  return COMMAND_ALINE;
		case HD63484_COMMAND_RLINE:  return COMMAND_RLINE;
		case HD63484_COMMAND_ARCT:   return COMMAND_ARCT;
		case HD63484_COMMAND_RRCT:   return COMMAND_RRCT;
		case HD63484_COMMAND_APLL:   return COMMAND_APLL;
		case HD63484_COMMAND_RPLL:   return COMMAND_RPLL;
		case HD63484_COMMAND_APLG:   return COMMAND_APLG;
		case HD63484_COMMAND_RPLG:   return COMMAND_RPLG;
		case HD63484_COMMAND_CRCL:   return COMMAND_CRCL;
		case HD63484_COMMAND_ELPS:   return COMMAND_ELPS;
		case HD63484_COMMAND_AARC:   return COMMAND_AARC;
		case HD63484_COMMAND_RARC:   return COMMAND_RARC;
		case HD63484_COMMAND_AEARC:  return COMMAND_AEARC;
		case HD63484_COMMAND_REARC:  return COMMAND_REARC;
		case HD63484_COMMAND_AFRCT:  return COMMAND_AFRCT;
		case HD63484_COMMAND_RFRCT:  return COMMAND_RFRCT;
		case HD63484_COMMAND_PAINT:  return COMMAND_PAINT;
		case HD63484_COMMAND_DOT:    return COMMAND_DOT;
	}

	return COMMAND_INVALID;
}

inline void hd63484_device::command_end_seq(uint32_t cycles)
{
	// command execution is signalled complete after its documented duration
	// (2CLK cycles); devices instantiated without a clock complete instantly
	if (cycles && clock() != 0)
	{
		m_busy = true;
		m_cmd_timer->adjust(clocks_to_attotime(cycles));
	}
	else
	{
		m_sr |= HD63484_SR_CED;
		update_irq();
	}

	/* TODO: we might need to be more aggressive and clear the params in there */
}

TIMER_CALLBACK_MEMBER(hd63484_device::cmd_exec_tick)
{
	m_busy = false;
	m_sr |= HD63484_SR_CED;
	update_irq();

	// resume processing of anything queued behind the finished command
	while (!m_busy && !BIT(m_ccr, 14) && m_fifo_ptr >= 1)
		process_fifo();

	update_dreq();
}

int hd63484_device::get_bpp()
{
	int gbm = (m_ccr >> 8) & 0x07;

	if (gbm <= 4)
		return 1 << gbm;

	//logerror ("Invalid Graphic Bit Mode (%d)\n", gbm);
	return 1;
}

void hd63484_device::calc_offset(int16_t x, int16_t y, uint32_t &offset, uint8_t &bit_pos)
{
	int bpp = get_bpp();
	int ppw = 16 / bpp;
	int gbm = (m_ccr >> 8) & 0x07;
	x += (m_org_dpd >> gbm);
	if (x >= 0)
	{
		offset = x / ppw;
		bit_pos = x % ppw;
	}
	else
	{
		offset = x / ppw;
		bit_pos= (-x) % ppw;
		if (bit_pos != 0)
		{
			offset--;
			bit_pos = ppw - bit_pos;
		}
	}

	offset += m_org_dpa - y * m_mwr[m_org_dn];
	bit_pos *= bpp;
}

uint16_t hd63484_device::get_dot(int16_t x, int16_t y)
{
	uint8_t bpp = get_bpp();
	uint32_t offset = 0;
	uint8_t bit_pos = 0;

	calc_offset(x, y, offset, bit_pos);

	return (readword(offset, m_org_dn) >> bit_pos) & ((1 << bpp) - 1);
}

// pattern RAM scan position for a drawing-relative coordinate; returns the
// pattern cell in the low byte (x | y<<4) and the pattern bit in bit 8
int hd63484_device::pattern_bit(int16_t px, int16_t py)
{
	int xs = m_pex - m_psx + 1;
	int ys = m_pey - m_psy + 1;
	int zx = m_pzx + 1;
	int zy = m_pzy + 1;
	int xp = m_psx + ((((m_ppx - m_psx) * zx + m_pzcx + px) % (xs * zx)) / zx);
	int yp = m_psy + ((((m_ppy - m_psy) * zy + m_pzcy + py) % (ys * zy)) / zy);

	if (xp < m_psx)
		xp = (m_pex + 1) - (m_psx - xp);

	if (yp < m_psy)
		yp = (m_pey + 1) - (m_psy - yp);

	int const pix = (m_pram[yp & 0x0f] >> (xp & 0x0f)) & 0x01;
	return (pix << 8) | ((yp & 0x0f) << 4) | (xp & 0x0f);
}

bool hd63484_device::set_dot(int16_t x, int16_t y, int16_t px, int16_t py)
{
	int const pat = pattern_bit(px, py);
	int const pix = BIT(pat, 8);
	int const xp = pat & 0x0f;
	int const yp = (pat >> 4) & 0x0f;

	uint8_t col = (m_cr >> 3) & 0x03;
	uint8_t bpp = get_bpp();
	uint16_t mask = (1 << bpp) - 1;
	uint16_t xmask = (16 / bpp) - 1;
	uint16_t cl0 = (m_cl0 >> ((x & xmask) * bpp)) & mask;
	uint16_t cl1 = (m_cl1 >> ((x & xmask) * bpp)) & mask;

	switch (col)
	{
		case 0x00:
			return set_dot(x, y, pix ? cl1 : cl0);
		case 0x01:
			if (pix)
				return set_dot(x, y, cl1);
			break;
		case 0x02:
			if (!pix)
				return set_dot(x, y, cl0);
			break;
		case 0x03:
			// pattern RAM direct: the pattern RAM is remapped as a 4x4 grid of
			// logical pixel color cells, each cell one 16-bit word of color data
			return set_dot(x, y, (m_pram[((yp & 0x03) << 2) | (xp & 0x03)] >> ((x & xmask) * bpp)) & mask);
	}

	return false;
}

bool hd63484_device::set_dot(int16_t x, int16_t y, uint16_t color)
{
	m_dot_count++;

	uint8_t bpp = get_bpp();
	uint32_t offset = 0;
	uint8_t bit_pos = 0;
	uint8_t opm = m_cr & 0x07;
	uint8_t area = (m_cr >> 5) & 0x07;

	if (area != 0 && area != 4)
	{
		bool const inside = x >= int16_t(m_xmin) && x <= int16_t(m_xmax) && y >= int16_t(m_ymin) && y <= int16_t(m_ymax);

		switch (area)
		{
			case 1: // command terminated when attempting to exit the area
				if (!inside)
				{
					m_sr |= HD63484_SR_ARD;
					update_irq();
					return true;
				}
				break;
			case 2: // drawing suppressed outside the area
			case 3: // as 2, with the suppression flagged
				if (!inside)
				{
					if (area == 3)
					{
						m_sr |= HD63484_SR_ARD;
						update_irq();
					}
					return false;
				}
				break;
			case 5: // command terminated when attempting to enter the area
				if (inside)
				{
					m_sr |= HD63484_SR_ARD;
					update_irq();
					return true;
				}
				break;
			case 6: // drawing suppressed inside the area
			case 7: // as 6, with the suppression flagged
				if (inside)
				{
					if (area == 7)
					{
						m_sr |= HD63484_SR_ARD;
						update_irq();
					}
					return false;
				}
				break;
		}
	}

	calc_offset(x, y, offset, bit_pos);

	uint16_t mask = ((1 << bpp) - 1) << bit_pos;
	uint16_t color_shifted = (color << bit_pos) & mask;

	uint16_t data = readword(offset, m_org_dn);
	uint16_t res = data;

	switch (opm)
	{
		case 0:
			res = (data & ~mask) | color_shifted;
			break;
		case 1:
			res = (data & ~mask) | ((data & mask) | color_shifted);
			break;
		case 2:
			res = (data & ~mask) | ((data & mask) & color_shifted);
			break;
		case 3:
			res = (data & ~mask) | ((data & mask) ^ color_shifted);
			break;
		case 4:
			if (get_dot(x, y) == ((m_ccmp & mask) >> bit_pos))
				res = (data & ~mask) | color_shifted;
			break;
		case 5:
			if (get_dot(x, y) != ((m_ccmp & mask) >> bit_pos))
				res = (data & ~mask) | color_shifted;
			break;
		case 6:
			if (get_dot(x, y) < (color_shifted >> bit_pos))
				res = (data & ~mask) | color_shifted;
			break;
		case 7:
			if (get_dot(x, y) > (color_shifted >> bit_pos))
				res = (data & ~mask) | color_shifted;
			break;
	}

	writeword(offset, res, m_org_dn);

	return false;
}

bool hd63484_device::draw_line(int16_t sx, int16_t sy, int16_t ex, int16_t ey)
{
	uint16_t delta_x = abs(ex - sx) * 2;
	uint16_t delta_y = abs(ey - sy) * 2;
	int dir_x = (ex < sx) ? -1 : ((ex > sx) ? +1 : 0);
	int dir_y = (ey < sy) ? -1 : ((ey > sy) ? +1 : 0);
	int pram_pos = 0;

	if(delta_x > delta_y)
	{
		int delta = delta_y - delta_x / 2;
		while(sx != ex)
		{
			if (set_dot(sx, sy, pram_pos, 0))
				return true;

			if(delta >= 0)
			{
				sy += dir_y;
				delta -= delta_x;
			}
			pram_pos++;
			sx += dir_x;
			delta += delta_y;
		}
	}
	else
	{
		int delta = delta_x - delta_y / 2;
		while(sy != ey)
		{
			if (set_dot(sx, sy, pram_pos, 0))
				return true;

			if(delta >= 0)
			{
				sx += dir_x;
				delta -= delta_y;
			}
			pram_pos++;
			sy += dir_y;
			delta += delta_x;
		}
	}

	return false;
}

void hd63484_device::draw_ellipse(int16_t cx, int16_t cy, double dx, double dy, double s_angol, double e_angol, bool c)
{
	double inc = 1.0 / (std::max(dx, dy) * 100);
	for (double angol = s_angol; fabs(angol - e_angol) >= inc*2; angol += inc * (c ? -1 : +1))
	{
		if (angol > 2.0 * std::numbers::pi) angol -= 2.0 * std::numbers::pi;
		if (angol < 0.0)                    angol += 2.0 * std::numbers::pi;

		double px = cos(angol) * dx;
		double py = sin(angol) * dy;
		if (set_dot(cx + round(px), cy + round(py), 0, 0))
			return;
	}
}

uint32_t hd63484_device::paint(int16_t sx, int16_t sy)
{
/*
    Complex figure handling is approximated: real hardware holds four
    unpaintable-area coordinates on an internal stack and pushes further ones
    into the read FIFO as 3-word records (CPx, CPy, pattern pointer), halting
    while the FIFO is full so the host can drain it and re-issue PAINT per
    spilled point. Painting everything in one pass instead keeps such programs
    working: they see command end with an empty read FIFO, i.e. 'completely
    painted', which mid-command suspension would be needed to improve on.
*/
	uint8_t e = BIT(m_cr, 8);
	uint8_t bpp = get_bpp();
	uint16_t mask = (1 << bpp) - 1;
	uint16_t xmask = (16 / bpp) - 1;

	const auto paintable = [this, e, bpp, mask, xmask] (int32_t px, int32_t py)
	{
		uint16_t dot = get_dot(px, py);
		uint16_t edg = (m_edg >> (px & xmask) * bpp) & mask;
		uint16_t cl0 = (m_cl0 >> (px & xmask) * bpp) & mask;
		uint16_t cl1 = (m_cl1 >> (px & xmask) * bpp) & mask;
		return !((e && dot != edg) || (!e && dot == edg) || dot == cl0 || dot == cl1);
	};

	// In replace operation mode a painted dot reads back as the written
	// value; one that does not, although the pattern selected an actual
	// write, sits in unwritable space (ROM or unpopulated memory, reachable
	// since large y coordinates wrap the 20 bit address). Such dots are
	// scanned over without counting as painted work or seeding, and a span
	// start is never queued twice, so a fill that strays outside the frame
	// buffer RAM visits the region once and terminates instead of
	// re-scanning it until the budget is gone - and a fill seeded entirely
	// in unwritable space leaves CP at its seed.
	uint8_t const colm = (m_cr >> 3) & 0x03;
	bool const scanover = (m_cr & 0x07) == 0;

	std::vector<std::pair<int16_t, int16_t>> seeds;
	std::unordered_set<uint32_t> seen;
	seeds.emplace_back(sx, sy);

	// CP ends at Pe, the end of the last line of the sweep, which finishes
	// on the top-most painted line (the sweep runs down first, then up)
	int32_t pe_x = sx;
	int32_t pe_y = sy;

	uint32_t rows = 0;

	// failsafe for fills whose painted dots stay paintable (conditional
	// operation modes), which would otherwise never terminate
	uint32_t budget = 1U << 20;

	while (!seeds.empty() && budget)
	{
		auto [x, y] = seeds.back();
		seeds.pop_back();

		if (!paintable(x, y))
			continue;

		if (!seen.emplace((uint32_t(uint16_t(y)) << 16) | uint16_t(x)).second)
			continue;

		rows++;

		// coordinates live on the hardware's 16 bit two's complement ring,
		// so the span scan wraps there instead of walking an int32 line into
		// aliased reads of the same dots
		uint32_t dl = 0;
		while (dl < 0x8000 && paintable(int16_t(x - dl - 1), y))
			dl++;
		uint32_t dr = 0;
		while (dl + dr < 0xffff && paintable(int16_t(x + dr + 1), y))
			dr++;

		bool up = false, dn = false;
		for (uint32_t d = 0; d <= dl + dr && budget; d++, budget--)
		{
			int16_t const px = int16_t(x - dl + d);
			if (set_dot(px, y, px - m_cpx, y - m_cpy))
				return rows;

			bool stuck = true;
			if (scanover && paintable(px, y))
			{
				// distinguish unwritable memory from a pattern-suppressed dot
				int const pix = BIT(pattern_bit(px - m_cpx, y - m_cpy), 8);
				if (colm == 0 || colm == 3 || (colm == 1 && pix) || (colm == 2 && !pix))
					stuck = false; // write attempted and did not stick
			}

			// the end point tracks every visited dot and the sweep continues
			// through unwritable territory, as the hardware does not verify
			// its writes - only the span start dedup keeps the traversal
			// finite there; an unwritable dot just is not painted work
			if (y > pe_y || (y == pe_y && px > pe_x))
			{
				pe_x = px;
				pe_y = y;
			}

			if (!stuck)
			{
				if (paintable(px, y + 1)) { if (!up) { seeds.emplace_back(px, y + 1); up = true; } }
				else up = false;
				if (paintable(px, y - 1)) { if (!dn) { seeds.emplace_back(px, y - 1); dn = true; } }
				else dn = false;
				continue;
			}

			// drop one seed per paintable run of the adjacent rows
			if (paintable(px, y + 1)) { if (!up) { seeds.emplace_back(px, y + 1); up = true; } }
			else up = false;
			if (paintable(px, y - 1)) { if (!dn) { seeds.emplace_back(px, y - 1); dn = true; } }
			else dn = false;
		}
	}

	if (!budget)
		logerror("HD63484 '%s': PAINT dot budget exhausted, fill aborted\n", tag());

	m_cpx = pe_x;
	m_cpy = pe_y;

	return rows;
}

uint16_t hd63484_device::command_rpr_exec()
{
	switch(m_cr & 0x1f)
	{
		case 0x00: // color 0
			return m_cl0;
		case 0x01: // color 1
			return m_cl1;
		case 0x02: // color comparison
			return m_ccmp;
		case 0x03: // edge color
			return m_edg;
		case 0x04: // mask
			return m_mask;
		case 0x05: // Pattern RAM Control 1
			return (m_ppy << 12) | (m_pzcy << 8) | (m_ppx << 4) | m_pzcx;
		case 0x06: // Pattern RAM Control 2
			return (m_psy << 12) | (m_psx << 4);
		case 0x07: // Pattern RAM Control 3
			return (m_pey << 12) | (m_pzy << 8) | (m_pex << 4) | m_pzx;
		case 0x08: // Area Definition XMIN
			return m_xmin;
		case 0x09: // Area Definition YMIN
			return m_ymin;
		case 0x0a: // Area Definition XMAX
			return m_xmax;
		case 0x0b: // Area Definition YMAX
			return m_ymax;
		case 0x0c: // Read Write Pointer H
			return (m_rwp_dn << 14) | ((m_rwp[m_rwp_dn] >> 12) & 0xff);
		case 0x0d: // Read Write Pointer L
			return (m_rwp[m_rwp_dn] & 0x0fff) << 4;
		case 0x10: // Drawing Pointer H
		case 0x11: // Drawing Pointer L
		{
			uint32_t offset = 0;
			uint8_t bit_pos = 0;
			calc_offset(m_cpx, m_cpy, offset, bit_pos);
			if ((m_cr & 0x1f) == 0x10)
				return (m_org_dn << 14) | ((offset >> 12) & 0xff);
			else
				return ((offset & 0x0fff) << 4) | ((bit_pos / get_bpp()) & 0x0f);
		}
		case 0x12: // Current Pointer X
			return m_cpx;
		case 0x13: // Current Pointer Y
			return m_cpy;
		default:
			if(LOG) logerror("Read %sx\n", wpr_regnames[m_cr & 0x1f]);
			return 0;
	}
}

void hd63484_device::command_wpr_exec()
{
	switch(m_cr & 0x1f)
	{
		case 0x00: // color 0
			m_cl0 = m_pr[0];
			break;
		case 0x01: // color 1
			m_cl1 = m_pr[0];
			break;
		case 0x02: // color comparison
			m_ccmp = m_pr[0];
			break;
		case 0x03: // edge color
			m_edg = m_pr[0];
			break;
		case 0x04: // mask
			m_mask = m_pr[0];
			break;
		case 0x05: // Pattern RAM Control 1
			m_pzcx = (m_pr[0] >> 0) & 0x0f;
			m_ppx = (m_pr[0] >> 4) & 0x0f;
			m_pzcy = (m_pr[0] >> 8) & 0x0f;
			m_ppy = (m_pr[0] >> 12) & 0x0f;
			break;
		case 0x06: // Pattern RAM Control 2
			m_psx = (m_pr[0] >> 4) & 0x0f;
			m_psy = (m_pr[0] >> 12) & 0x0f;
			break;
		case 0x07: // Pattern RAM Control 3
			m_pzx = (m_pr[0] >> 0) & 0x0f;
			m_pex = (m_pr[0] >> 4) & 0x0f;
			m_pzy = (m_pr[0] >> 8) & 0x0f;
			m_pey = (m_pr[0] >> 12) & 0x0f;
			break;
		case 0x08: // Area Definition XMIN
			m_xmin = m_pr[0];
			break;
		case 0x09: // Area Definition YMIN
			m_ymin = m_pr[0];
			break;
		case 0x0a: // Area Definition XMAX
			m_xmax = m_pr[0];
			break;
		case 0x0b: // Area Definition YMAX
			m_ymax = m_pr[0];
			break;
		case 0x0c: // Read Write Pointer H
			m_rwp_dn = (m_pr[0] & 0xc000) >> 14;
			m_rwp[m_rwp_dn] = (m_rwp[m_rwp_dn] & 0x00fff) | ((m_pr[0] & 0x00ff) << 12);
			break;
		case 0x0d: // Read Write Pointer L
			m_rwp[m_rwp_dn] = (m_rwp[m_rwp_dn] & 0xff000) | ((m_pr[0] & 0xfff0) >> 4);
			break;
		default:
			if(LOG) logerror("%s -> %02x\n",wpr_regnames[m_cr & 0x1f],m_pr[0]);
			break;
	}
}

void hd63484_device::command_clr_exec()
{
	uint8_t mm = m_cr & 0x03;
	uint16_t d = m_pr[0];
	int16_t ax = (int16_t)m_pr[1];
	int16_t ay = (int16_t)m_pr[2];

	int d0_inc = (ax < 0) ? -1 : 1;
	int d1_inc = (ay < 0) ? -1 : 1;

	for(int16_t d1=0; d1!=ay+d1_inc; d1+=d1_inc)
	{
		for(int16_t d0=0; d0!=ax+d0_inc; d0+=d0_inc)
		{
			uint32_t offset = m_rwp[m_rwp_dn] - d1 * m_mwr[m_rwp_dn] + d0;
			uint16_t data = readword(offset, m_rwp_dn);
			uint16_t res = 0;

			if (BIT(m_cr, 10))
			{
				switch(mm)
				{
					case 0: // replace
						res = (data & ~m_mask) | (d & m_mask);
						break;
					case 1: // OR
						res = (data & ~m_mask) | ((data | d) & m_mask);
						break;
					case 2: // AND
						res = (data & ~m_mask) | ((data & d) & m_mask);
						break;
					case 3: // EOR
						res = (data & ~m_mask) | ((data ^ d) & m_mask);
						break;
				}
			}
			else
				res = d;

			writeword(offset, res, m_rwp_dn);
		}
	}

	m_rwp[m_rwp_dn] -= (ay + d1_inc) * m_mwr[m_rwp_dn];
	m_rwp[m_rwp_dn] &= 0xfffff;
}

bool hd63484_device::command_dma_write_exec(uint16_t data, bool modify)
{
	int16_t const ax = (int16_t)m_pr[0];
	int16_t const ay = (int16_t)m_pr[1];
	int const d0_inc = (ax < 0) ? -1 : 1;
	int const d1_inc = (ay < 0) ? -1 : 1;
	uint32_t const offset = (m_rwp[m_rwp_dn] - m_dma_d1 * m_mwr[m_rwp_dn] + m_dma_d0) & 0xfffff;

	if (modify)
	{
		uint16_t const fb = readword(offset, m_rwp_dn);
		switch (m_cr & 0x03)
		{
			case 0: // replace
				data = (fb & ~m_mask) | (data & m_mask);
				break;
			case 1: // OR
				data = (fb & ~m_mask) | ((fb | data) & m_mask);
				break;
			case 2: // AND
				data = (fb & ~m_mask) | ((fb & data) & m_mask);
				break;
			case 3: // XOR
				data = (fb & ~m_mask) | ((fb ^ data) & m_mask);
				break;
		}
	}

	writeword(offset, data, m_rwp_dn);

	if (m_dma_d0 != ax)
	{
		m_dma_d0 += d0_inc;
		return false;
	}

	m_dma_d0 = 0;
	if (m_dma_d1 != ay)
	{
		m_dma_d1 += d1_inc;
		return false;
	}

	m_rwp[m_rwp_dn] -= (ay + d1_inc) * m_mwr[m_rwp_dn];
	m_rwp[m_rwp_dn] &= 0xfffff;
	return true;
}

void hd63484_device::command_cpy_exec()
{
	uint8_t mm = m_cr & 0x03;
	uint8_t dsd = (m_cr >> 8) & 0x07;
	uint8_t s = BIT(m_cr, 11);
	uint32_t SA = ((m_pr[0] & 0xff) << 12) | ((m_pr[1]&0xfff0) >> 4);
	int16_t DX = (int16_t)m_pr[s ? 3 : 2];
	int16_t DY = (int16_t)m_pr[s ? 2 : 3];

	int sd0_inc = (DX < 0) ? -1 : 1;
	int sd1_inc = (DY < 0) ? -1 : 1;
	int dd0_inc, dd1_inc;
	if (dsd & 0x04)
	{
		dd0_inc = dsd & 0x01 ? -1 : +1;
		dd1_inc = dsd & 0x02 ? -1 : +1;
	}
	else
	{
		dd0_inc = dsd & 0x02 ? -1 : +1;
		dd1_inc = dsd & 0x01 ? -1 : +1;
	}

	for(int16_t sd1=0, dd1=0; sd1!=DY+sd1_inc; sd1+=sd1_inc, dd1+=dd1_inc)
	{
		for(int16_t sd0=0, dd0=0; sd0!=DX+sd0_inc; sd0+=sd0_inc, dd0+=dd0_inc)
		{
			uint32_t src_offset, dst_offset;

			if (s)
				src_offset = SA + sd1 - sd0 * m_mwr[m_rwp_dn];
			else
				src_offset = SA + sd0 - sd1 * m_mwr[m_rwp_dn];

			if (BIT(dsd, 2))
				dst_offset = m_rwp[m_rwp_dn] + dd1 - dd0 * m_mwr[m_rwp_dn];
			else
				dst_offset = m_rwp[m_rwp_dn] + dd0 - dd1 * m_mwr[m_rwp_dn];

			uint16_t src_data = readword(src_offset, m_rwp_dn);
			uint16_t dst_data = readword(dst_offset, m_rwp_dn);

			if (BIT(m_cr, 12))
			{
				switch(mm)
				{
					case 0: // replace
						dst_data = (dst_data & ~m_mask) | (src_data & m_mask);
						break;
					case 1: // OR
						dst_data = (dst_data & ~m_mask) | ((dst_data | src_data) & m_mask);
						break;
					case 2: // AND
						dst_data = (dst_data & ~m_mask) | ((dst_data & src_data) & m_mask);
						break;
					case 3: // EOR
						dst_data = (dst_data & ~m_mask) | ((dst_data ^ src_data) & m_mask);
						break;
				}
			}
			else
				dst_data = src_data;

			writeword(dst_offset, dst_data, m_rwp_dn);
		}
	}

	m_rwp[m_rwp_dn] += dsd & 0x04 ? (DY + dd1_inc) : (-(DY + dd1_inc) * m_mwr[m_rwp_dn]);
	m_rwp[m_rwp_dn] &= 0xfffff;
}

void hd63484_device::command_line_exec()
{
	int16_t x = (int16_t)m_pr[0];
	int16_t y = (int16_t)m_pr[1];

	if (BIT(m_cr, 10))
	{
		x += m_cpx;
		y += m_cpy;
	}

	draw_line(m_cpx, m_cpy, x, y);

	m_cpx = x;
	m_cpy = y;
}

void hd63484_device::command_rct_exec()
{
	int16_t dX = m_pr[0];
	int16_t dY = m_pr[1];

	if (BIT(m_cr, 10))  // relative (RRCT)
	{
		dX += m_cpx;
		dY += m_cpy;
	}

	/*
	3<-2
	|  ^
	v  |
	0->1
	*/

	/* 0 -> 1 */
	if (draw_line(m_cpx, m_cpy, dX, m_cpy))
		return;

	/* 1 -> 2 */
	if (draw_line(dX, m_cpy, dX, dY))
		return;

	/* 2 -> 3 */
	if (draw_line(dX, dY, m_cpx, dY))
		return;

	/* 3 -> 4 */
	draw_line(m_cpx, dY, m_cpx, m_cpy);
}

void hd63484_device::command_gcpy_exec()
{
	uint8_t dsd = (m_cr >> 8) & 0x07;
	uint8_t s = BIT(m_cr, 11);
	int16_t Xs = (int16_t)m_pr[0];
	int16_t Ys = (int16_t)m_pr[1];
	int16_t DX = (int16_t)m_pr[s ? 3 : 2];
	int16_t DY = (int16_t)m_pr[s ? 2 : 3];

	if (BIT(m_cr, 12))  // relative (RGCPY)
	{
		Xs += m_cpx;
		Ys += m_cpy;
	}

	int sd0_inc = (DX < 0) ? -1 : 1;
	int sd1_inc = (DY < 0) ? -1 : 1;
	int dd0_inc, dd1_inc;
	if (dsd & 0x04)
	{
		dd0_inc = dsd & 0x01 ? -1 : +1;
		dd1_inc = dsd & 0x02 ? -1 : +1;
	}
	else
	{
		dd0_inc = dsd & 0x02 ? -1 : +1;
		dd1_inc = dsd & 0x01 ? -1 : +1;
	}

	for(int16_t sd1=0, dd1=0; sd1!=DY+sd1_inc; sd1+=sd1_inc, dd1+=dd1_inc)
	{
		for(int16_t sd0=0, dd0=0; sd0!=DX+sd0_inc; sd0+=sd0_inc, dd0+=dd0_inc)
		{
			uint16_t color;
			if (s)
				color = get_dot(Xs + sd1, Ys + sd0);
			else
				color = get_dot(Xs + sd0, Ys + sd1);

			bool stop;
			if (BIT(dsd, 2))
				stop = set_dot(m_cpx + dd1, m_cpy + dd0, color);
			else
				stop = set_dot(m_cpx + dd0, m_cpy + dd1, color);
			if (stop)
				return;
		}
	}

	if (dsd & 0x04)
		m_cpx += DY + dd1_inc;
	else
		m_cpy += DY + dd1_inc;

}

void hd63484_device::command_frct_exec()
{
	int16_t X = (int16_t)m_pr[0];
	int16_t Y = (int16_t)m_pr[1];

	if (!BIT(m_cr, 10))
	{
		X -= m_cpx;
		Y -= m_cpy;
	}

	int d0_inc = (X < 0) ? -1 : 1;
	int d1_inc = (Y < 0) ? -1 : 1;

	for(int16_t d1=0; d1!=Y+d1_inc; d1+=d1_inc)
	{
		for(int16_t d0=0; d0!=X+d0_inc; d0+=d0_inc)
		{
			if (set_dot(m_cpx + d0, m_cpy + d1, d0, d1))
				return;
		}
	}

	m_cpy += (Y + d1_inc);
}

void hd63484_device::command_ptn_exec()
{
	int16_t szx = ((m_pr[0] >> 0) & 0xff);
	int16_t szy = ((m_pr[0] >> 8) & 0xff);
	uint8_t sl_sd = (m_cr >> 8) & 0x0f;
	int16_t px = 0;
	int16_t py = 0;

	for(int16_t d1=0; d1!=szy+1; d1++)
	{
		switch (sl_sd)
		{
			case 0x00: px = 0;    py = d1;   break;
			case 0x01: px = -d1;  py = d1;   break;
			case 0x02: py = 0;    px = -d1;  break;
			case 0x03: px = -d1;  py = -d1;  break;
			case 0x04: px = 0;    py = -d1;  break;
			case 0x05: px = d1;   py = -d1;  break;
			case 0x06: py = 0;    px = d1;   break;
			case 0x07: px = d1;   py = d1;   break;
			case 0x08: px = d1;   py = d1;   break;
			case 0x09: px = 0;    py = d1;   break;
			case 0x0a: px = -d1;  py = d1;   break;
			case 0x0b: px = -d1;  py = 0;    break;
			case 0x0c: px = -d1;  py = -d1;  break;
			case 0x0d: px = 0;    py = -d1;  break;
			case 0x0e: px = +d1;  py = -d1;  break;
			case 0x0f: px = +d1;  py = 0;    break;
		}

		for(int16_t d0=0; d0!=szx+1; d0++)
		{
			if (set_dot(m_cpx + px, m_cpy + py, d0, d1))
				return;

			switch (sl_sd)
			{
				case 0x00: px++;        break;
				case 0x01: px++; py++;  break;
				case 0x02: py++;        break;
				case 0x03: px--; py++;  break;
				case 0x04: px--;        break;
				case 0x05: px--; py--;  break;
				case 0x06: py--;        break;
				case 0x07: px++; py--;  break;
				case 0x08: px++;        break;
				case 0x09: px++; py++;  break;
				case 0x0a: py++;        break;
				case 0x0b: px--; py++;  break;
				case 0x0c: px--;        break;
				case 0x0d: px--; py--;  break;
				case 0x0e: py--;        break;
				case 0x0f: px++; py--;  break;
			}
		}
	}

	switch (sl_sd)
	{
		case 0x00: m_cpy += (szy + 1); break;
		case 0x01: m_cpx -= (szy + 1); m_cpy += (szy + 1); break;
		case 0x02: m_cpx -= (szy + 1); break;
		case 0x03: m_cpx -= (szy + 1); m_cpy -= (szy + 1); break;
		case 0x04: m_cpy -= (szy + 1); break;
		case 0x05: m_cpx += (szy + 1); m_cpy -= (szy + 1); break;
		case 0x06: m_cpx += (szy + 1); break;
		case 0x07: m_cpx += (szy + 1); m_cpy += (szy + 1); break;
		case 0x08: m_cpx += (szy + 1); m_cpy += (szy + 1); break;
		case 0x09: m_cpy += (szy + 1); break;
		case 0x0a: m_cpx -= (szy + 1); m_cpy += (szy + 1); break;
		case 0x0b: m_cpx -= (szy + 1); break;
		case 0x0c: m_cpx -= (szy + 1); m_cpy -= (szy + 1); break;
		case 0x0d: m_cpy -= (szy + 1); break;
		case 0x0e: m_cpx += (szy + 1); m_cpy -= (szy + 1); break;
		case 0x0f: m_cpx += (szy + 1); break;
	}
}

void hd63484_device::command_plg_exec()
{
	int sx = m_cpx;
	int sy = m_cpy;
	int ex=0;
	int ey=0;

	for (int i = 0; i < m_dn; i++)
	{
		if (BIT(m_cr, 10))
		{
			ex = sx + (int16_t)m_pr[1 + i * 2];
			ey = sy + (int16_t)m_pr[1 + i * 2 + 1];
		}
		else
		{
			ex = (int16_t)m_pr[1 + i * 2];
			ey = (int16_t)m_pr[1 + i * 2 + 1];
		}

		if (draw_line(sx, sy, ex, ey))
			break;

		sx = ex;
		sy = ey;
	}

	if (m_cr & 0x2000)
	{
		// APLG/RPLG
		draw_line(sx, sy, m_cpx, m_cpy);
	}
	else
	{
		// APLL/RPLL
		m_cpx = ex;
		m_cpy = ey;
	}
}

void hd63484_device::command_arc_exec()
{
	int16_t xc = (int16_t)m_pr[0];
	int16_t yc = (int16_t)m_pr[1];
	int16_t xe = (int16_t)m_pr[2];
	int16_t ye = (int16_t)m_pr[3];

	if (BIT(m_cr, 10))
	{
		xc += m_cpx;
		yc += m_cpy;
		xe += m_cpx;
		ye += m_cpy;
	}

	double r = sqrt(pow((double)(xc - m_cpx), 2) + pow((double)(yc - m_cpy), 2));
	double s_angol = atan2((double)(m_cpy - yc), (double)(m_cpx - xc));
	double e_angol = atan2((double)(ye - yc), (double)(xe - xc));
	if (s_angol < 0)    s_angol += 2.0 * std::numbers::pi;
	if (e_angol < 0)    e_angol += 2.0 * std::numbers::pi;

	draw_ellipse(xc, yc, r, r, s_angol, e_angol, BIT(m_cr, 8));

	m_cpx = xe;
	m_cpy = ye;
}

void hd63484_device::command_earc_exec()
{
	uint16_t a = m_pr[0];
	uint16_t b = m_pr[1];
	int16_t xc = (int16_t)m_pr[2];
	int16_t yc = (int16_t)m_pr[3];
	int16_t xe = (int16_t)m_pr[4];
	int16_t ye = (int16_t)m_pr[5];

	if (BIT(m_cr, 10))
	{
		xc += m_cpx;
		yc += m_cpy;
		xe += m_cpx;
		ye += m_cpy;
	}

	double r = sqrt(pow((double)(xc - m_cpx), 2) / a + pow((double)(yc - m_cpy), 2) / b);
	double dx = sqrt((double)a);
	double dy = sqrt((double)b);
	double s_angol = atan2((double)(m_cpy - yc) / dy, (double)(m_cpx - xc) / dx);
	double e_angol = atan2((double)(ye - yc) / dy, (double)(xe - xc) / dx);
	if (s_angol < 0)    s_angol += 2.0 * std::numbers::pi;
	if (e_angol < 0)    e_angol += 2.0 * std::numbers::pi;

	draw_ellipse(xc, yc, r * dx, r * dy, s_angol, e_angol, BIT(m_cr, 8));

	m_cpx = xe;
	m_cpy = ye;
}

// DREQ is re-evaluated only at bus-level boundaries, where no command decode
// is in flight: with CDM set (command/parameter DMA) it requests while the
// write FIFO has word space; with DDM set (data DMA) it requests while a DMA
// read has data in the read FIFO, or while an in-progress DWT/DMOD can take
// a data word. With neither mode enabled it never asserts. DRC only shapes
// the DREQ pulses on the pin, which a level callback abstracts.
void hd63484_device::update_dreq()
{
	int state = CLEAR_LINE;

	if (BIT(m_ccr, 12)) // CDM
	{
		if (m_fifo_ptr <= 13)
			state = ASSERT_LINE;
	}
	else if (BIT(m_ccr, 13)) // DDM
	{
		if (m_fifo_r_ptr >= 1)
			state = ASSERT_LINE;
		else if (!(m_sr & HD63484_SR_CED) && !m_busy)
		{
			int const cmd = translate_command(m_cr);
			if ((cmd == COMMAND_DWT || cmd == COMMAND_DMOD) && m_fifo_ptr <= 13)
				state = ASSERT_LINE;
		}
	}

	if (state != m_dreq_state)
	{
		m_dreq_state = state;
		m_dreq_cb(state);
	}
}

void hd63484_device::process_fifo()
{
	uint8_t data;

	dequeue_w(&data);

	if (m_sr & HD63484_SR_CED)
	{
		m_cr = (data & 0xff) << 8;
		dequeue_w(&data);
		m_cr |= data & 0xff;
		m_param_ptr = 0;
		m_sr &= ~HD63484_SR_CED;
		update_irq();
	}
	else
	{
		m_pr[m_param_ptr] = (data & 0xff) << 8;
		dequeue_w(&data);
		m_pr[m_param_ptr] |= (data & 0xff);
		m_param_ptr++;
	}

	m_dot_count = 0;
	int const opm_p = ((m_cr & 0x07) < 4) ? 4 : 6;

	switch (translate_command(m_cr))
	{
		case COMMAND_INVALID:
			if (CMD_LOG)    logerror("HD63484 '%s': <invalid %04x>\n", tag(), m_cr);
			logerror("HD63484 '%s' Invalid Command Byte %02x\n", tag(), m_cr);
			m_sr |= HD63484_SR_CER; // command error
			command_end_seq(0);
			break;

		case COMMAND_ORG:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': ORG 0x%04x, 0x%04x\n", tag(), m_pr[0], m_pr[1]);
				m_org_dn = (m_pr[0] & 0xc000) >> 14;
				m_org_dpa = ((m_pr[0] & 0xff) << 12) | ((m_pr[1] & 0xfff0) >> 4);
				m_org_dpd = (m_pr[1] & 0xf);
				m_cpx = m_cpy = 0;
				command_end_seq(8);
			}
			break;

		case COMMAND_WPR: // 0x0800 & ~0x1f
			if (m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': WPR (%d) 0x%04x\n", tag(), m_cr & 0x1f, m_pr[0]);
				command_wpr_exec();
				command_end_seq(6);
			}
			break;

		case COMMAND_RPR:
			if (m_param_ptr == 0)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': RPR (%d)\n", tag(), m_cr & 0x1f);
				m_sr &= ~HD63484_SR_ARD;
				update_irq();
				uint16_t data = command_rpr_exec();
				queue_r((data >> 8) & 0xff);
				queue_r((data >> 0) & 0xff);
				command_end_seq(6);
			}
			break;

		case COMMAND_WPTN:
			if(m_param_ptr == 1)
			{
				m_dn = m_pr[0]; // number of param words

				//if(m_dn > 0x10 || m_dn == 0)
				//  fatalerror("stop!\n");
			}

			if(m_param_ptr == (1 + m_dn))
			{
				if (CMD_LOG)    logerror("HD63484 '%s': WPTN (%d) %d", tag(), m_cr & 0x0f, m_pr[0]);

				int pra = m_cr & 0xf;
				for(int i=0; i<m_dn; i++)
				{
					if (CMD_LOG)    logerror(", 0x%04x", m_pr[1 + i]);
					m_pram[(i + pra) & 0xf] = m_pr[1 + i];
				}

				if (CMD_LOG)    logerror("\n");
				command_end_seq(4 * m_dn + 8);
			}
			break;

		case COMMAND_RPTN:
			if(m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': RPTN (%d) %d\n", tag(), m_cr & 0x0f, m_pr[0]);
				m_dn = m_pr[0]; // number of words to read
				m_dma_d0 = 0;

				if (m_dn)
				{
					m_stream = STREAM_RPTN;
					stream_fill();
				}
				else
					command_end_seq(10);
			}
			break;

		case COMMAND_DRD:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': DRD %d, %d\n", tag(), (int16_t)m_pr[0], (int16_t)m_pr[1]);
				m_dma_d0 = m_dma_d1 = 0;
				m_stream = STREAM_DRD;
				stream_fill();
			}
			break;

		case COMMAND_DWT:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': DWT %d, %d\n", tag(), (int16_t)m_pr[0], (int16_t)m_pr[1]);
				m_dma_d0 = m_dma_d1 = 0;
			}
			else if (m_param_ptr == 3)
			{
				// data words follow the parameters, written by CPU
				m_param_ptr = 2;
				if (command_dma_write_exec(m_pr[2], false))
				{
					uint32_t const x = abs((int16_t)m_pr[0]) + 1;
					uint32_t const yc = abs((int16_t)m_pr[1]) + 1;
					command_end_seq((4 * x + 8) * yc + 16 * ((x * yc + 7) / 8) + 34);

					if (BIT(m_ccr, 13)) // data DMA transfer complete
					{
						m_done_cb(ASSERT_LINE);
						m_done_cb(CLEAR_LINE);
					}
				}
			}
			break;

		case COMMAND_DMOD:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': DMOD (%d) %d, %d\n", tag(), m_cr & 0x03, (int16_t)m_pr[0], (int16_t)m_pr[1]);
				m_dma_d0 = m_dma_d1 = 0;
			}
			else if (m_param_ptr == 3)
			{
				// as DWT, each data word is combined with the frame buffer under MASK
				m_param_ptr = 2;
				if (command_dma_write_exec(m_pr[2], true))
				{
					uint32_t const x = abs((int16_t)m_pr[0]) + 1;
					uint32_t const yc = abs((int16_t)m_pr[1]) + 1;
					command_end_seq((4 * x + 8) * yc + 16 * ((x * yc + 7) / 8) + 34);

					if (BIT(m_ccr, 13)) // data DMA transfer complete
					{
						m_done_cb(ASSERT_LINE);
						m_done_cb(CLEAR_LINE);
					}
				}
			}
			break;

		case COMMAND_RD:
			if (m_param_ptr == 0)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': RD\n", tag());
				uint16_t data = readword(m_rwp[m_rwp_dn], m_rwp_dn);
				queue_r((data >> 8) & 0xff);
				queue_r((data >> 0) & 0xff);
				m_rwp[m_rwp_dn]+=1;
				m_rwp[m_rwp_dn]&=0xfffff;
				command_end_seq(12);
			}
			break;

		case COMMAND_WT:
			if (m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': WT 0x%04x\n", tag(), m_pr[0]);
				writeword(m_rwp[m_rwp_dn], m_pr[0], m_rwp_dn);
				m_rwp[m_rwp_dn]+=1;
				m_rwp[m_rwp_dn]&=0xfffff;
				command_end_seq(8);
			}
			break;

		case COMMAND_MOD:
			if(m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': MOD (%d) 0x%04x\n", tag(), m_cr & 0x03, m_pr[0]);
				uint16_t d = m_pr[0];
				uint16_t data = readword(m_rwp[m_rwp_dn], m_rwp_dn);
				uint16_t res = 0;

				switch(m_cr & 0x03)
				{
					case 0: // replace
						res = (data & ~m_mask) | (d & m_mask);
						break;
					case 1: // OR
						res = (data & ~m_mask) | ((data | d) & m_mask);
						break;
					case 2: // AND
						res = (data & ~m_mask) | ((data & d) & m_mask);
						break;
					case 3: // EOR
						res = (data & ~m_mask) | ((data ^ d) & m_mask);
						break;
				}

				writeword(m_rwp[m_rwp_dn], res, m_rwp_dn);
				command_end_seq(8);
			}
			break;

		case COMMAND_CLR:
		case COMMAND_SCLR:
			if (m_param_ptr == 3)
			{
				if (CMD_LOG)
				{
					if (BIT(m_cr, 10))
						logerror("HD63484 '%s': SCLR (%d) 0x%04x,  %d, %d\n", tag(), m_cr & 0x03, m_pr[0], (int16_t)m_pr[1], (int16_t)m_pr[2]);
					else
						logerror("HD63484 '%s': CLR 0x%04x, %d, %d\n", tag(), m_pr[0], (int16_t)m_pr[1], (int16_t)m_pr[2]);
				}

				{
					uint32_t const x = abs((int16_t)m_pr[1]) + 1;
					uint32_t const yc = abs((int16_t)m_pr[2]) + 1;
					command_clr_exec();
					command_end_seq(BIT(m_cr, 10) ? ((4 * x + 6) * yc + 12) : ((2 * x + 8) * yc + 12));
				}
			}
			break;

		case COMMAND_CPY:
		case COMMAND_SCPY:
			if (m_param_ptr == 4)
			{
				if (CMD_LOG)
				{
					if (BIT(m_cr, 12))
						logerror("HD63484 '%s': SCPY (%d, %d, %d) 0x%x, 0x%x, %d, %d\n", tag(), BIT(m_cr, 11), (m_cr >> 8) & 0x07, m_cr & 0x07, m_pr[0] & 0xff, (m_pr[1]&0xfff0) >> 4, (int16_t)m_pr[2], (int16_t)m_pr[3]);
					else
						logerror("HD63484 '%s': CPY (%d, %d) 0x%x, 0x%x, %d, %d\n", tag(), BIT(m_cr, 11), (m_cr >> 8) & 0x07, m_pr[0] & 0xff, (m_pr[1]&0xfff0) >> 4, (int16_t)m_pr[2], (int16_t)m_pr[3]);
				}

				{
					uint8_t const sw = BIT(m_cr, 11);
					uint32_t const x = abs((int16_t)m_pr[sw ? 3 : 2]) + 1;
					uint32_t const yc = abs((int16_t)m_pr[sw ? 2 : 3]) + 1;
					command_cpy_exec();
					command_end_seq((6 * x + 10) * yc + 12);
				}
			}
			break;

		case COMMAND_AMOVE:
		case COMMAND_RMOVE:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cMOVE %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', (int16_t)m_pr[0], (int16_t)m_pr[1]);
				if (BIT(m_cr, 10))
				{
					m_cpx += (int16_t)m_pr[0];
					m_cpy += (int16_t)m_pr[1];
				}
				else
				{
					m_cpx = (int16_t)m_pr[0];
					m_cpy = (int16_t)m_pr[1];
				}
				command_end_seq(56);
			}
			break;

		case COMMAND_RRCT:
		case COMMAND_ARCT:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cRTC (%d, %d, %d) %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, (int16_t)m_pr[0], (int16_t)m_pr[1]);
				command_rct_exec();
				command_end_seq(opm_p * m_dot_count + 54);
			}
			break;

		case COMMAND_RLINE:
		case COMMAND_ALINE:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cLINE (%d, %d, %d) %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, (int16_t)m_pr[0], (int16_t)m_pr[1]);
				command_line_exec();
				command_end_seq(opm_p * m_dot_count + 18);
			}
			break;

		case COMMAND_APLG:
		case COMMAND_RPLG:
		case COMMAND_APLL:
		case COMMAND_RPLL:
			if(m_param_ptr == 1)
				m_dn = m_pr[0]; // number of param words

			if(m_param_ptr == (1 + m_dn*2))
			{
				if (CMD_LOG)
				{
					logerror("HD63484 '%s': %cPL%c (%d, %d, %d) %d", tag(), BIT(m_cr, 10) ? 'R' : 'A', m_cr & 0x2000 ? 'G' : 'L', (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, m_pr[0]);
					for (int i=0; i<m_dn; i++)
						logerror(", %d, %d", (int16_t)m_pr[1 + i * 2], (int16_t)m_pr[1 + i * 2 + 1]);
					logerror("\n");
				}

				command_plg_exec();
				command_end_seq(opm_p * m_dot_count + 16 * m_dn + ((m_cr & 0x2000) ? 20 : 8));
			}
			break;

		case COMMAND_CRCL:
			if(m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': CRCL (%d, %d, %d, %d) %d\n", tag(), BIT(m_cr, 8), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, m_pr[0]);
				uint16_t r = m_pr[0] & 0x1fff;
				draw_ellipse(m_cpx, m_cpy, r, r, 0.0, 2.0 * std::numbers::pi, BIT(m_cr, 8));
				command_end_seq(8 * m_dot_count + 66);
			}
			break;

		case COMMAND_ELPS:
			if(m_param_ptr == 3)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': ELPS (%d, %d, %d, %d) %d, %d, %d\n", tag(), BIT(m_cr, 8), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, m_pr[0], m_pr[1], m_pr[2]);
				double dx = (double)m_pr[2];
				double dy = sqrt(pow(dx, 2) / ((double)m_pr[0] / m_pr[1]));
				draw_ellipse(m_cpx, m_cpy, dx, dy, 0.0, 2.0 * std::numbers::pi, BIT(m_cr, 8));
				command_end_seq(10 * m_dot_count + 90);
			}
			break;

		case COMMAND_AARC:
		case COMMAND_RARC:
			if(m_param_ptr == 4)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cARC (%d, %d, %d, %d) %d, %d, %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', BIT(m_cr, 8), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, (int16_t)m_pr[0], (int16_t)m_pr[1], (int16_t)m_pr[2], (int16_t)m_pr[3]);
				command_arc_exec();
				command_end_seq(8 * m_dot_count + 18);
			}
			break;

		case COMMAND_AEARC:
		case COMMAND_REARC:
			if(m_param_ptr == 6)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cEARC (%d, %d, %d, %d) %d, %d, %d, %d, %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', BIT(m_cr, 8), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, m_pr[0], m_pr[1], m_pr[2], m_pr[3], m_pr[4], m_pr[5]);
				command_earc_exec();
				command_end_seq(10 * m_dot_count + 96);
			}
			break;

		case COMMAND_AFRCT:
		case COMMAND_RFRCT:
			if (m_param_ptr == 2)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cFRCT (%d, %d, %d) %d, %d\n", tag(), BIT(m_cr, 10) ? 'R' : 'A', (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, (int16_t)m_pr[0], (int16_t)m_pr[1]);

				{
					uint32_t const rows = abs(BIT(m_cr, 10) ? (int16_t)m_pr[1] : (int16_t)(m_pr[1] - m_cpy)) + 1;
					command_frct_exec();
					command_end_seq(opm_p * m_dot_count + 8 * rows + 18);
				}
			}
			break;

		case COMMAND_PAINT:
			if (m_param_ptr == 0)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': PAINT (%d, %d, %d, %d)\n", tag(), BIT(m_cr, 8), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07);
				{
					uint32_t const rows = paint(m_cpx, m_cpy);
					uint32_t const cycles = 18 * m_dot_count + 102 * rows;
					command_end_seq(cycles > 58 ? cycles - 58 : cycles);
				}
			}
			break;

		case COMMAND_DOT:
			if (m_param_ptr == 0)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': DOT (%d, %d, %d)\n", tag(), (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07);
				set_dot(m_cpx, m_cpy, 0, 0);
				command_end_seq(8);
			}
			break;

		case COMMAND_PTN:
			if (m_param_ptr == 1)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': PTN (%d, %d, %d, %d, %d) 0x%04x\n", tag(), (m_cr >> 11) & 0x01, (m_cr >> 8) & 0x07, (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, m_pr[0]);
				{
					uint32_t const rows = ((m_pr[0] >> 8) & 0xff) + 1;
					command_ptn_exec();
					command_end_seq(opm_p * m_dot_count + 10 * rows + 20);
				}
			}
			break;

		case COMMAND_RGCPY:
		case COMMAND_AGCPY:
			if (m_param_ptr == 4)
			{
				if (CMD_LOG)    logerror("HD63484 '%s': %cGCPY (%d, %d, %d, %d, %d) %d, %d, %d, %d\n", tag(), BIT(m_cr, 12) ? 'R' : 'A', (m_cr >> 11) & 0x01, (m_cr >> 8) & 0x07, (m_cr >> 5) & 0x07, (m_cr >> 3) & 0x03, (m_cr >> 0) & 0x07, (int16_t)m_pr[0], (int16_t)m_pr[1], (int16_t)m_pr[2], (int16_t)m_pr[3]);

				{
					uint32_t const rows = abs((int16_t)m_pr[BIT(m_cr, 11) ? 2 : 3]) + 1;
					command_gcpy_exec();
					command_end_seq((opm_p + 2) * m_dot_count + 10 * rows + 70);
				}
			}
			break;

		default:
			logerror("%04x\n",m_cr);
			fatalerror("stop!\n");
	}
}

void hd63484_device::exec_abort_sequence()
{
	m_busy = false;
	m_cmd_timer->adjust(attotime::never);
	m_stream = STREAM_NONE;
	fifo_w_clear();
	fifo_r_clear();
	m_sr = HD63484_SR_WFR | HD63484_SR_WFE | HD63484_SR_CED; // hard-set to 0x23

	update_irq();
	update_dreq();
}

// DMA acknowledged data port: the FIFOs are selected regardless of the
// address register contents
uint16_t hd63484_device::dack_r()
{
	uint8_t hi = 0, lo = 0;
	dequeue_r(&hi);
	dequeue_r(&lo);
	update_dreq();
	return (hi << 8) | lo;
}

void hd63484_device::dack_w(uint16_t data)
{
	queue_w((data >> 8) & 0xff);
	queue_w(data & 0xff);
	if (!BIT(m_ccr, 14) && !m_busy)
		process_fifo();
	update_dreq();
}

// DONE input: terminates DMA transfer, resetting CDM (and data DMA mode)
void hd63484_device::done_w(int state)
{
	if (state)
	{
		m_ccr &= ~0x3000;
		update_dreq();
	}
}

void hd63484_device::lpstb_w(int state)
{
	if (state && !m_lpstb_state)
	{
		// latch the display refresh address of the background screen under the
		// beam (approximation: reconstructed from the beam coordinates)
		int const ppmc = get_ppmc();
		int col = screen().hpos() / ppmc - (m_hsw + m_hds + m_external_skew);
		if (col < 0)
			col = 0;

		int const y = screen().vpos();
		int const l0 = m_vds + (BIT(m_dcr, 13) ? m_sp[0] : 0);
		int const l1 = l0 + m_sp[1];
		int const l2 = l1 + (BIT(m_dcr, 11) ? m_sp[2] : 0);

		int layer = 1;
		int row = 0;
		if (BIT(m_dcr, 13) && y >= m_vds && y < l0)
		{
			layer = 0;
			row = y - m_vds;
		}
		else if (y >= l0 && y < l1)
		{
			layer = 1;
			row = (y - l0) / (m_vzf + 1);
		}
		else if (BIT(m_dcr, 11) && y >= l1 && y < l2)
		{
			layer = 2;
			row = y - l1;
		}

		m_lpar = (m_sar[layer] + row * m_mwr[layer] + col) & 0xfffff;
		m_lpar_chr = m_mwr_chr[layer];

		m_sr |= HD63484_SR_LPD;
		update_irq();
	}

	m_lpstb_state = state;
}

uint16_t hd63484_device::video_registers_r(int offset)
{
	uint16_t res = (m_vreg[offset] << 8) | (m_vreg[offset+1] & 0xff);

	switch(offset)
	{
		case 0x06:
			res = m_dcr;
			break;

		case 0x80:
			res = screen().vpos() & 0xfff; // Raster Count
			break;

		case 0xec: // Light Pen Address high
			res = (m_lpar_chr << 7) | ((m_lpar >> 16) & 0x0f);
			break;

		case 0xee: // Light Pen Address low; reading it ends the latch sequence
			res = m_lpar & 0xffff;
			m_sr &= ~HD63484_SR_LPD;
			update_irq();
			break;

		default:
			if(LOG) logerror("%s R\n",acrtc_regnames[m_ar/2]);
			break;
	}

	return res;
}

void hd63484_device::video_registers_w(int offset)
{
	uint16_t vreg_data;

	vreg_data = (m_vreg[offset]<<8)|(m_vreg[offset+1]&0xff);

	switch(offset)
	{
		case 0x00: // FIFO entry
			queue_w((vreg_data & 0xff00) >> 8);
			queue_w((vreg_data & 0x00ff) >> 0);
			if(FIFO_LOG) logerror("%s -> %04x\n",acrtc_regnames[m_ar/2],vreg_data);
			if (!BIT(m_ccr, 14) && !m_busy)
				process_fifo();
			update_dreq();
			break;

		case 0x02: // Command Entry

			if(vreg_data & 0x8000) // abort sequence (ABT)
				exec_abort_sequence();

			/*
			x--- ---- ---- ---- ABorT
			-x-- ---- ---- ---- PauSE
			...
			---- -xxx ---- ---- Graphic Bit Mode (bpp)
			---- ---- xxxx xxxx irq mask, directly correlated to sr
			*/
			{
				bool const resume = BIT(m_ccr, 14) && !BIT(vreg_data, 14) && !(vreg_data & 0x8000);
				m_ccr = vreg_data;
				update_irq();

				// leaving pause: process everything accumulated in the write FIFO
				if (resume)
					while (!m_busy && m_fifo_ptr >= 1)
						process_fifo();

				update_dreq();
			}
			break;

		case 0x04:
			logerror("OMR: %04x\n", vreg_data);
			m_omr = vreg_data;
			break;

		case 0x06:
			m_dcr = vreg_data;
			recompute_parameters();
			break;

		case 0x82: // Horizontal Sync Register
			m_hc = ((vreg_data & 0xff00) >> 8) + 1;
			m_hsw = vreg_data & 0x1f;
			recompute_parameters();
			break;
		case 0x84: // Horizontal Display Register
			m_hds = ((vreg_data & 0xff00) >> 8) + 1;
			m_hdw = ((vreg_data & 0x00ff) >> 0) + 1;
			recompute_parameters();
			break;
		case 0x92: // Horizontal Window Register
			m_hws = ((vreg_data & 0xff00) >> 8) + 1;
			m_hww = ((vreg_data & 0x00ff) >> 0) + 1;
			recompute_parameters();
			break;

		case 0x86: // Vertical Sync Register
			m_vc = (vreg_data & 0xfff);
			recompute_parameters();
			break;
		case 0x88: // Vertical Display Register
			m_vds = ((vreg_data & 0xff00) >> 8) + 1;
			m_vsw = (vreg_data & 0x1f);
			recompute_parameters();
			break;
		case 0x8a:  // Split Screen Width 1
			m_sp[1] = vreg_data & 0x0fff;
			recompute_parameters();
			break;
		case 0x8c:  // Split Screen Width 0
			m_sp[0] = vreg_data & 0x0fff;
			recompute_parameters();
			break;
		case 0x8e:  // Split Screen Width 2
			m_sp[2] = vreg_data & 0x0fff;
			recompute_parameters();
			break;
		case 0xea: // Zoom Factor Register
			m_hzf = (vreg_data >> 12) & 0x0f;
			m_vzf = (vreg_data >> 8) & 0x0f;
			break;

		case 0x94: // Vertical Window Register A
			m_vws = (vreg_data & 0xfff) + 1;
			recompute_parameters();
			break;
		case 0x96: // Vertical Window Register B
			m_vww = (vreg_data & 0xfff);
			recompute_parameters();
			break;

		case 0x90: // Blink Control Register
			m_bon[0] = (vreg_data >> 12) & 0x0f;
			m_boff[0] = (vreg_data >> 8) & 0x0f;
			m_bon[1] = (vreg_data >> 4) & 0x0f;
			m_boff[1] = vreg_data & 0x0f;
			break;

		case 0x98: // Graphic Cursor: X end / X start (memory cycles from HSYNC)
			m_cxe = (vreg_data >> 8) & 0xff;
			m_cxs = vreg_data & 0xff;
			break;
		case 0x9a: // Graphic Cursor: Y start (rasters from VSYNC)
			m_cys = vreg_data & 0xfff;
			break;
		case 0x9c: // Graphic Cursor: Y end
			m_cye = vreg_data & 0xfff;
			break;

		case 0xe0: // Block Cursor address
		case 0xe4:
			m_bca[(offset >> 2) & 1] = vreg_data;
			break;
		case 0xe2: // Block Cursor width / start / end raster
		case 0xe6:
			m_bcw[(offset >> 2) & 1] = (vreg_data >> 13) & 0x07;
			m_bcsr[(offset >> 2) & 1] = (vreg_data >> 8) & 0x1f;
			m_bcer[(offset >> 2) & 1] = vreg_data & 0x1f;
			break;

		case 0xe8: // Cursor Definition Register
			m_cm = (vreg_data >> 14) & 0x03;
			m_con[0] = (vreg_data >> 11) & 0x07;
			m_coff[0] = (vreg_data >> 8) & 0x07;
			m_con[1] = (vreg_data >> 3) & 0x07;
			m_coff[1] = vreg_data & 0x07;
			break;

		case 0xc0: // Raster Address Register
		case 0xc8:
		case 0xd0:
		case 0xd8:
			m_lra[(offset & 0x18) >> 3] = (vreg_data >> 8) & 0x1f;
			m_fra[(offset & 0x18) >> 3] = vreg_data & 0x1f;
			break;

		case 0xc2: // Memory Width Register
		case 0xca:
		case 0xd2:
		case 0xda:
			m_mwr[(offset & 0x18) >> 3] = vreg_data & 0xfff; // pitch
			m_mwr_chr[(offset & 0x18) >> 3] = (vreg_data & 0x8000) >> 15;
			break;

		case 0xc4: // Start Address Register
		case 0xcc:
		case 0xd4:
		case 0xdc:
			m_sar[(offset & 0x18) >> 3] = ((vreg_data & 0xf) << 16) | (m_sar[(offset & 0x18) >> 3] & 0xffff);
			m_sda[(offset & 0x18) >> 3] = (vreg_data & 0x0f00) >> 8;
			break;

		case 0xc6: // Start Address Register
		case 0xce:
		case 0xd6:
		case 0xde:
			m_sar[(offset & 0x18) >> 3] = (vreg_data & 0xffff) | (m_sar[(offset & 0x18) >> 3] & 0xf0000);
			break;

		default:
			if(LOG) logerror("%s -> %04x\n",acrtc_regnames[m_ar/2],vreg_data);
			break;
	}
}

uint16_t hd63484_device::read16(offs_t offset)
{
	if (BIT(offset, 0))
	{
		// Read control register
		uint16_t res;

		if(m_ar == 0) // FIFO read
		{
			uint8_t data;

			dequeue_r(&data);
			res = (data & 0xff) << 8;
			dequeue_r(&data);
			res |= data & 0xff;
			update_dreq();
		}
		else
			res = video_registers_r(m_ar);

		inc_ar(2);

		return res;
	}
	else
	{
		// Read status register
		// kothello is coded so that upper byte of this should be 0xff (tests with jc opcode). Maybe it's just open bus?
		return m_sr | 0xff00;
	}
}

void hd63484_device::write16(offs_t offset, uint16_t data)
{
	if (BIT(offset, 0))
	{
		// Write control register
		m_vreg[m_ar] = (data & 0xff00) >> 8;
		m_vreg[m_ar+1] = (data & 0xff);

		video_registers_w(m_ar);

		inc_ar(2);
	}
	else
	{
		// Write address register
		m_ar = data & 0xfe;
	}
}

uint8_t hd63484_device::read8(offs_t offset)
{
	if (BIT(offset, 0))
	{
		// Read control register
		uint8_t res = 0xff;

		if(m_ar < 2) // FIFO read
		{
			dequeue_r(&res);
			update_dreq();
		}
		else
			res = video_registers_r(m_ar & 0xfe) >> (m_ar & 1 ? 0 : 8);

		inc_ar(1);

		return res;
	}
	else
	{
		// Read status register
		return m_sr;
	}
}

void hd63484_device::write8(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
	{
		// Write control register
		m_vreg[m_ar] = data;

		if(m_ar < 2) // FIFO write
		{
			queue_w(data);
			if ((m_ar & 1) && !BIT(m_ccr, 14) && !m_busy)
			{
				process_fifo();
				update_dreq();
			}

			m_ar ^= 1;
		}
		else
			video_registers_w(m_ar & 0xfe);

		inc_ar(1);
	}
	else
	{
		// Write address register
		m_ar = data;
	}
}

void hd63484_device::device_start()
{
	m_display_cb.resolve();
	m_character_cb.resolve();

	m_char_space = has_configured_map(AS_CHARACTER) ? &space(AS_CHARACTER) : nullptr;

	m_cmd_timer = timer_alloc(FUNC(hd63484_device::cmd_exec_tick), this);

	register_save_state();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hd63484_device::device_reset()
{
	m_sr = HD63484_SR_CED | HD63484_SR_WFR | HD63484_SR_WFE;
	m_ccr = m_omr = m_edg = m_dcr = m_hsw = 0;
	m_hc = m_hds = m_hdw = m_hws = m_hww = 0;
	m_vc = m_vws = m_vww = m_vds = m_vsw = 0;
	m_sp[0] = m_sp[1] = m_sp[2] = 0;
	m_cl0 = m_cl1 = 0;
	m_xmin = m_ymin = m_xmax = m_ymax = 0;
	m_ppx = m_pzcx = m_psx = m_pzx = m_pex = 0;
	m_ppy = m_pzcy=  m_psy = m_pzy = m_pey = 0;
	m_ar = m_cr = 0;
	m_param_ptr = 0;
	m_rwp_dn = 0;
	m_org_dpa = 0;
	m_org_dn = 0;
	m_org_dpd = 0;
	m_ccmp = 0;
	m_mask = -1;
	m_cpx = m_cpy = 0;
	m_dn = 0;
	m_dma_d0 = m_dma_d1 = 0;
	m_stream = STREAM_NONE;
	m_irq_state = CLEAR_LINE;
	m_hzf = m_vzf = 0;
	m_lpstb_state = 0;
	m_lpar = 0;
	m_lpar_chr = 0;
	m_busy = false;
	m_dot_count = 0;
	m_cmd_timer->adjust(attotime::never);
	m_dreq_state = CLEAR_LINE;

	memset(m_vreg, 0, sizeof(m_vreg));
	memset(m_fifo, 0, sizeof(m_fifo));
	memset(m_fifo_r, 0, sizeof(m_fifo_r));
	memset(m_pr, 0, sizeof(m_pr));
	memset(m_rwp, 0, sizeof(m_rwp));
	memset(m_mwr, 0, sizeof(m_mwr));
	memset(m_mwr_chr, 0, sizeof(m_mwr_chr));
	memset(m_fra, 0, sizeof(m_fra));
	memset(m_lra, 0, sizeof(m_lra));
	memset(m_bca, 0, sizeof(m_bca));
	memset(m_bcw, 0, sizeof(m_bcw));
	memset(m_bcsr, 0, sizeof(m_bcsr));
	memset(m_bcer, 0, sizeof(m_bcer));
	m_cm = 0;
	m_con[0] = m_con[1] = m_coff[0] = m_coff[1] = 0;
	m_bon[0] = m_bon[1] = m_boff[0] = m_boff[1] = 0;
	m_cxs = m_cxe = 0;
	m_cys = m_cye = 0;
	memset(m_sar, 0, sizeof(m_sar));
	memset(m_sda, 0, sizeof(m_sda));
	memset(m_pram, 0, sizeof(m_pram));
}

//-------------------------------------------------
//  draw_graphics_line -
//-------------------------------------------------

void hd63484_device::draw_graphics_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int vs, int y, int layer_n, bool active, bool ins_window)
{
	int bpp = get_bpp();
	int ppw = 16 / bpp;
	uint32_t mask = (1 << bpp) - 1;
	// zooming applies to the base screen only: the same raster is scanned
	// (VZF+1) times and the same display address is output (HZF+1) times
	int row = (layer_n == 1) ? (y - vs) / (m_vzf + 1) : (y - vs);
	int const hzoom = (layer_n == 1) ? m_hzf + 1 : 1;
	int hcnt = 0;
	uint32_t base_offs = m_sar[layer_n] + row * m_mwr[layer_n] + m_external_skew;
	uint32_t wind_offs = m_sar[3] + (y - m_vws) * m_mwr[3] + m_external_skew;
	int step = (m_omr & 0x08) ? 2 : 1;
	int ppmc = get_ppmc();
	int ws = m_hsw + m_hws + m_external_skew;

	if (m_omr & 0x08)
	{
		/*
		    According to the datasheet, in interleaved and superimposed modes:
		    - HDW and HWW must be even
		    - the relation between HDS and HWS must be even/even or odd/odd
		*/

		if (m_hww & 1)
			ws += step;

		if ((m_hws & 1) ^ (m_hds & 1))
			wind_offs++;
	}

	// start dot address: the screen's display is shifted left by SDA dots for
	// horizontal smooth scroll (on hardware SDA is output as the HSD video
	// attribute to condition the external shift register load). WSS in the
	// OMR selects whether the shift applies to the window or the background
	// screens on this raster.
	bool const wss = BIT(m_omr, 12);
	int const sda_bg = wss ? 0 : (m_sda[layer_n] & (ppw - 1));
	int const sda_win = wss ? (m_sda[3] & (ppw - 1)) : 0;

	if (!sda_bg && !sda_win)
	{
		for(int x=cliprect.left(); x<=cliprect.right(); x+=ppw)
		{
			uint16_t data = 0;
			int screen_n = layer_n;

			if (ins_window && x >= ws * ppmc && x < (ws + m_hww) * ppmc)
			{
				data = readword(wind_offs, 3);
				wind_offs++;
				screen_n = 3;
			}
			else if (active)
				data = readword(base_offs, layer_n);

			for (int b=0; b<ppw; b++)
			{
				int px = x + b;
				if (!m_display_cb.isnull())
					m_display_cb(bitmap, cliprect, y, px, data & mask, screen_n);
				else if (cliprect.contains(px, y))
					bitmap.pix(y, px) = data & mask;

				data >>= bpp;
			}

			if (++hcnt >= hzoom)
			{
				hcnt = 0;
				base_offs++;
			}
		}
	}
	else
	{
		// one extra word feeds the dots shifted in from the right
		for(int x=cliprect.left(); x<=cliprect.right() + ppw; x+=ppw)
		{
			uint16_t data = 0;
			int screen_n = layer_n;
			int sda = sda_bg;

			if (ins_window && x >= ws * ppmc && x < (ws + m_hww) * ppmc)
			{
				data = readword(wind_offs, 3);
				wind_offs++;
				screen_n = 3;
				sda = sda_win;
			}
			else if (active)
				data = readword(base_offs, layer_n);

			for (int b=0; b<ppw; b++)
			{
				int px = x + b - sda;
				if (px >= cliprect.left() && px <= cliprect.right())
				{
					if (!m_display_cb.isnull())
						m_display_cb(bitmap, cliprect, y, px, data & mask, screen_n);
					else if (cliprect.contains(px, y))
						bitmap.pix(y, px) = data & mask;
				}

				data >>= bpp;
			}

			if (++hcnt >= hzoom)
			{
				hcnt = 0;
				base_offs++;
			}
		}
	}
}

// cursor blink phase from the CDR on/off times (units of 4 field periods;
// on time 0 means steady off, otherwise off time 0 means steady on)
bool hd63484_device::cursor_phase(int which)
{
	if (m_con[which] == 0)
		return false;
	if (m_coff[which] == 0)
		return true;

	uint32_t const on = 4 * m_con[which] + 4;
	uint32_t const off = 4 * m_coff[which] + 4;
	return (screen().frame_number() % (on + off)) < on;
}

// BLINK1/BLINK2 video attribute phase from the BCR (same encoding, 4-bit)
bool hd63484_device::blink_state(int which)
{
	which &= 1;
	if (m_bon[which] == 0)
		return false;
	if (m_boff[which] == 0)
		return true;

	uint32_t const on = 4 * m_bon[which] + 4;
	uint32_t const off = 4 * m_boff[which] + 4;
	return (screen().frame_number() % (on + off)) < on;
}

// block cursor raster range, evaluated in character row sequence order so
// that it follows the FRA..LRA wraparound
bool hd63484_device::cursor_raster(int which, int layer_n, int raster)
{
	uint8_t const idx = (raster - m_fra[layer_n]) & 0x1f;
	uint8_t const s = (m_bcsr[which] - m_fra[layer_n]) & 0x1f;
	uint8_t const e = (m_bcer[which] - m_fra[layer_n]) & 0x1f;

	return (s <= e) ? (idx >= s && idx <= e) : (idx >= s || idx <= e);
}

// graphic (CM=10, CUD1) and crosshair (CM=11, CUD1 horizontal / CUD2
// vertical element) cursor state at a bitmap position; coordinates are
// memory cycles from HSYNC and rasters from VSYNC, which match the
// full-screen bitmap coordinates this device configures
int hd63484_device::cursor_dot(int x, int y)
{
	int const ppmc = get_ppmc();
	int res = 0;

	if (m_cm == 2)
	{
		if (cursor_phase(0) && y >= m_cys && y <= m_cye && x >= m_cxs * ppmc && x < (m_cxe + 1) * ppmc)
			res |= 1;
	}
	else if (m_cm == 3)
	{
		if (cursor_phase(0))
		{
			if (y >= m_cys && y <= m_cye)
				res |= 1;
			if (x >= m_cxs * ppmc && x < (m_cxe + 1) * ppmc)
				res |= 2;
		}
	}

	return res;
}

//-------------------------------------------------
//  draw_char_line - character screen scanline:
//  one character code per memory cycle, rendered
//  by the board's character generator callback
//-------------------------------------------------

void hd63484_device::draw_char_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int vs, int y, int layer_n)
{
	if (m_character_cb.isnull())
		return;

	uint8_t const fra = m_fra[layer_n];
	uint8_t const lra = m_lra[layer_n];
	uint8_t const sra = m_sda[layer_n] & 0x1f; // SAR low: start raster address for character screens
	int const height = ((lra - fra) & 0x1f) + 1;        // full row: FRA..LRA with 5-bit wrap
	int const first = ((lra - sra) & 0x1f) + 1;         // first displayed (partial) row: SRA..LRA

	int const n = y - vs;
	int row, raster;
	if (n < first)
	{
		row = 0;
		raster = (sra + n) & 0x1f;
	}
	else
	{
		row = 1 + (n - first) / height;
		raster = (fra + (n - first) % height) & 0x1f;
	}

	int const cellw = get_ppmc();
	uint32_t addr = (m_sar[layer_n] + row * m_mwr[layer_n]) & 0xffff;

	for (int x = cliprect.left(); x <= cliprect.right(); x += cellw)
	{
		uint16_t const code = readword(addr, layer_n);

		// the block cursors exist only on character screens: in block mode
		// they drive CUD1/CUD2 separately, in graphic cursor mode both are
		// combined onto CUD2, in crosshair mode they are unused
		int cursor = 0;
		if (m_cm < 3)
		{
			for (int c = 0; c < 2; c++)
			{
				uint16_t const span = uint16_t(addr - m_bca[c]);
				if (span <= m_bcw[c] && cursor_raster(c, layer_n, raster) && cursor_phase((m_cm == 2) ? 1 : c))
					cursor |= (m_cm == 2) ? 2 : (1 << c);
			}
		}

		m_character_cb(bitmap, cliprect, y, x, cellw, code, raster, layer_n, cursor);
		addr = (addr + 1) & 0xffff;
	}
}

//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

uint32_t hd63484_device::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int l0 = cliprect.top() + (BIT(m_dcr, 13) ? m_sp[0] : 0);
	int l1 = l0 + m_sp[1];
	int l2 = l1 + (BIT(m_dcr, 11) ? m_sp[2] : 0);

	if(m_omr & 0x4000)
	{
		for(int y=cliprect.top(); y<=cliprect.bottom(); y++)
		{
			bool ins_window = BIT(m_dcr, 9) && y >= m_vws && y < m_vws+m_vww;

			if (BIT(m_dcr, 13) && y >= cliprect.top() && y < l0)
			{
				if (m_mwr_chr[0])
					draw_char_line(bitmap, cliprect, cliprect.top(), y, 0);
				else
					draw_graphics_line(bitmap, cliprect, cliprect.top(), y, 0, BIT(m_dcr, 12), ins_window);
			}
			else if (y >= l0 && y < l1)
			{
				if (m_mwr_chr[1])
					draw_char_line(bitmap, cliprect, l0, y, 1);
				else
					draw_graphics_line(bitmap, cliprect, l0, y, 1, BIT(m_dcr, 14), ins_window);
			}
			else if (BIT(m_dcr, 11) && y >= l1 && y < l2)
			{
				if (m_mwr_chr[2])
					draw_char_line(bitmap, cliprect, l1, y, 2);
				else
					draw_graphics_line(bitmap, cliprect, l1, y, 2, BIT(m_dcr, 10), ins_window);
			}
		}
	}
	return 0;
}

void hd63484_device::register_save_state()
{
	save_item(NAME(m_ar));
	save_item(NAME(m_vreg));
	save_item(NAME(m_sr));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_ptr));
	save_item(NAME(m_fifo_r));
	save_item(NAME(m_fifo_r_ptr));
	save_item(NAME(m_cr));
	save_item(NAME(m_pr));
	save_item(NAME(m_param_ptr));
	save_item(NAME(m_rwp));
	save_item(NAME(m_dma_d0));
	save_item(NAME(m_dma_d1));
	save_item(NAME(m_rwp_dn));
	save_item(NAME(m_org_dpa));
	save_item(NAME(m_org_dn));
	save_item(NAME(m_org_dpd));
	save_item(NAME(m_cl0));
	save_item(NAME(m_cl1));
	save_item(NAME(m_ccmp));
	save_item(NAME(m_mask));
	save_item(NAME(m_cpx));
	save_item(NAME(m_cpy));
	save_item(NAME(m_mwr));
	save_item(NAME(m_mwr_chr));
	save_item(NAME(m_fra));
	save_item(NAME(m_lra));
	save_item(NAME(m_bca));
	save_item(NAME(m_bcw));
	save_item(NAME(m_bcsr));
	save_item(NAME(m_bcer));
	save_item(NAME(m_cm));
	save_item(NAME(m_con));
	save_item(NAME(m_coff));
	save_item(NAME(m_bon));
	save_item(NAME(m_boff));
	save_item(NAME(m_cxs));
	save_item(NAME(m_cxe));
	save_item(NAME(m_cys));
	save_item(NAME(m_cye));
	save_item(NAME(m_sar));
	save_item(NAME(m_sda));
	save_item(NAME(m_pram));
	save_item(NAME(m_dn));
	save_item(NAME(m_stream));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_hzf));
	save_item(NAME(m_vzf));
	save_item(NAME(m_lpstb_state));
	save_item(NAME(m_lpar));
	save_item(NAME(m_lpar_chr));
	save_item(NAME(m_busy));
	save_item(NAME(m_dot_count));
	save_item(NAME(m_dreq_state));
	save_item(NAME(m_ccr));
	save_item(NAME(m_omr));
	save_item(NAME(m_edg));
	save_item(NAME(m_dcr));
	save_item(NAME(m_hc));
	save_item(NAME(m_hds));
	save_item(NAME(m_hdw));
	save_item(NAME(m_hws));
	save_item(NAME(m_hww));
	save_item(NAME(m_sp));
	save_item(NAME(m_hsw));
	save_item(NAME(m_vc));
	save_item(NAME(m_vws));
	save_item(NAME(m_vww));
	save_item(NAME(m_vds));
	save_item(NAME(m_vsw));
	save_item(NAME(m_ppy));
	save_item(NAME(m_pzcy));
	save_item(NAME(m_ppx));
	save_item(NAME(m_pzcx));
	save_item(NAME(m_psx));
	save_item(NAME(m_pex));
	save_item(NAME(m_pzx));
	save_item(NAME(m_psy));
	save_item(NAME(m_pzy));
	save_item(NAME(m_pey));
	save_item(NAME(m_xmin));
	save_item(NAME(m_ymin));
	save_item(NAME(m_xmax));
	save_item(NAME(m_ymax));
}
