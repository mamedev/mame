// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarivad.cpp

    Atari VAD video controller device.

***************************************************************************/

#include "emu.h"
#include "atarivad.h"
#include "atarimo.h"
#include "screen.h"


//**************************************************************************
//  VAD VIDEO CONTROLLER DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ATARI_VAD, atari_vad_device, "atarivad", "Atari VAD")

//-------------------------------------------------
//  atari_vad_device - constructor
//-------------------------------------------------

atari_vad_device::atari_vad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ATARI_VAD, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_scanline_int_cb(*this)
	, m_alpha_tilemap(*this, "alpha")
	, m_playfield_tilemap(*this, "playfield")
	, m_playfield2_tilemap(*this, "playfield2")
	, m_mob(*this, "mob")
	, m_eof_data(*this, "eof")
	, m_scanline_int_timer(nullptr)
	, m_tilerow_update_timer(nullptr)
	, m_eof_timer(nullptr)
	, m_palette_bank(0)
	, m_pf0_xscroll_raw(0)
	, m_pf0_yscroll(0)
	, m_pf1_xscroll_raw(0)
	, m_pf1_yscroll(0)
	, m_mo_xscroll(0)
	, m_mo_yscroll(0)
	, m_pf_xoffset(0)
	, m_pf2_xoffset(4)
{
}


//-------------------------------------------------
//  control_write: Does the bulk of the word for an I/O
//  write.
//-------------------------------------------------

void atari_vad_device::control_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t newword = m_control[offset];
	COMBINE_DATA(&newword);
	internal_control_write(offset, newword);
}


//-------------------------------------------------
//  control_read: Handles an I/O read from the video controller.
//-------------------------------------------------

uint16_t atari_vad_device::control_read(offs_t offset)
{
	if (!machine().side_effects_disabled())
		logerror("vc_r(%02X)\n", offset);

	// a read from offset 0 returns the current scanline
	// also sets bit 0x4000 if we're in VBLANK
	if (offset == 0)
	{
		int result = screen().vpos();
		if (result > 255)
			result = 255;
		if (result > screen().visible_area().bottom())
			result |= 0x4000;
		return result;
	}
	else
		return m_control[offset];
}


//-------------------------------------------------
//  alpha_w: Generic write handler for alpha RAM.
//-------------------------------------------------

void atari_vad_device::alpha_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_alpha_tilemap->write16(offset, data, mem_mask);
}


//-------------------------------------------------
//  playfield_upper_w: Generic write handler for
//  upper word of split playfield RAM.
//-------------------------------------------------

void atari_vad_device::playfield_upper_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_playfield_tilemap->write16_ext(offset, data, mem_mask);
	if (m_playfield2_tilemap != nullptr)
		m_playfield2_tilemap->write16_ext(offset, data, mem_mask);
}


//-------------------------------------------------
//  playfield_latched_lsb_w: Generic write handler for
//  lower word of playfield RAM with a latch in the LSB of the
//  upper word.
//-------------------------------------------------

void atari_vad_device::playfield_latched_lsb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_playfield_tilemap->write16(offset, data, mem_mask);
	if (BIT(m_control[0x0a], 7))
		m_playfield_tilemap->write16_ext(offset, m_control[0x1d], uint16_t(0x00ff));
}


//-------------------------------------------------
//  playfield_latched_msb_w: Generic write handler for
//  lower word of playfield RAM with a latch in the MSB of the
//  upper word.
//-------------------------------------------------

void atari_vad_device::playfield_latched_msb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_playfield_tilemap->write16(offset, data, mem_mask);
	if (BIT(m_control[0x0a], 7))
		m_playfield_tilemap->write16_ext(offset, m_control[0x1c], uint16_t(0xff00));
}


//-------------------------------------------------
//  playfield2_latched_msb_w: Generic write handler for
//  lower word of second playfield RAM with a latch in the MSB
//  of the upper word.
//-------------------------------------------------

void atari_vad_device::playfield2_latched_msb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_playfield2_tilemap->write16(offset, data, mem_mask);
	if (BIT(m_control[0x0a], 7))
		m_playfield2_tilemap->write16_ext(offset, m_control[0x1c], uint16_t(0xff00));
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void atari_vad_device::device_start()
{
	// verify configuration
	if (m_playfield_tilemap == nullptr)
		throw emu_fatalerror("Playfield tilemap not found!");
	if (m_eof_data == nullptr)
		throw emu_fatalerror("EOF data not found!");

	// allocate timers
	m_scanline_int_timer = timer_alloc(FUNC(atari_vad_device::scanline_int), this);
	m_tilerow_update_timer = timer_alloc(FUNC(atari_vad_device::update_tilerow), this);
	m_eof_timer = timer_alloc(FUNC(atari_vad_device::eof_update), this);

	// register for save states
	save_item(NAME(m_palette_bank));          // which palette bank is enabled
	save_item(NAME(m_pf0_xscroll_raw));       // playfield 1 xscroll raw value
	save_item(NAME(m_pf0_yscroll));           // playfield 1 yscroll
	save_item(NAME(m_pf1_xscroll_raw));       // playfield 2 xscroll raw value
	save_item(NAME(m_pf1_yscroll));           // playfield 2 yscroll
	save_item(NAME(m_mo_xscroll));            // sprite xscroll
	save_item(NAME(m_mo_yscroll));            // sprite xscroll
}


//-------------------------------------------------
//  device_reset: Handle a device reset by
//  clearing the interrupt lines and states
//-------------------------------------------------

void atari_vad_device::device_reset()
{
	// share extended memory between the two tilemaps
	if (m_playfield2_tilemap != nullptr)
		m_playfield2_tilemap->extmem().set(m_playfield_tilemap->extmem());

	// reset the state
	m_palette_bank = 0;
	m_pf0_xscroll_raw = m_pf1_xscroll_raw = 0;
	m_pf0_yscroll = m_pf1_yscroll = 0;
	m_mo_xscroll = m_mo_yscroll = 0;
	memset(m_control, 0, sizeof(m_control));

	// start the timers
	m_tilerow_update_timer->adjust(screen().time_until_pos(0));
	m_eof_timer->adjust(screen().time_until_pos(0));
}


//-------------------------------------------------
//  scanline_int - trigger a scanline interrupt
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(atari_vad_device::scanline_int)
{
	m_scanline_int_cb(ASSERT_LINE);
}



//-------------------------------------------------
//  internal_control_write: Handle writes to the
//  control registers and EOF updates
//-------------------------------------------------

void atari_vad_device::internal_control_write(offs_t offset, uint16_t newword)
{
	// switch off the offset
	uint16_t const oldword = m_control[offset];
	m_control[offset] = newword;
	switch (offset)
	{
		//
		//  VAD register map:
		//
		//      00 = HDW_ENABLE
		//              A000 = enable other VAD params
		//              E000 = enable other VAD params but disable update from ALPHA RAM
		//      01 = HDW_VSY_LD (standard = 0x795)
		//              0FF8 = V_SY_S
		//              0007 = V_SY_E
		//      02 = HDW_VBL_LD (standard = 0xAEF)
		//              FE00 = VLAST
		//              01FF = V_BL_S
		//      03 = HDW_VINT
		//      04 = HDW_HSY_LD (standard = 0x5EEF)
		//              FC00 = H_SY_E/2
		//              03FF = H_SY_S
		//      05 = HDW_HBL_LD (relief = 0x72AC, batman/thunderj = 0xBEB1, shuuz/offtwall = 0xCEB1)
		//              F000 = LB_CLR
		//              0800 = H_BL_E/16
		//              0400 = H_BL_E/1
		//              03FF = H_BL_S
		//      06 = HDW_SLIP_LD (relief/batman/shuuz = 0x6F05, shuuz/offtwall = 0x0F05)
		//              7F00 = SLIP_S
		//              0080 = SLIP_E_SE1
		//              007F = SLIP_E
		//      07 = HDW_APDMA_LD (relief/batman/shuuz = 0xBE84, shuuz = 0xB90E, offtwall = 0xB90D)
		//              C000 = OPFPIC
		//              3800 = PF_HRESET/16
		//              0780 = PFAT
		//              0070 = AL_HRESET (depending on ALHSIZ)
		//              000F = ALPHA_DMA
		//      08 = HDW_PMBASE_LD (relief/batman/shuuz = 0x0519, shuuz/offtwall = 0x49B4)
		//              E000 = P1BASE/0x2000
		//              1C00 = P2BASE/0x2000
		//              0380 = PABASE/0x2000
		//              0040 = PMASK
		//              003E = MOBASE/0x800
		//              0001 = MOMASK
		//      09 = HDW_ALBASE_LD (relief/batman/shuuz = 0x061F, shuuz/offtwall = 0x017F)
		//              0400 = OPMINUS1
		//              03C0 = ALBASE/0x1000
		//              003F = SLIPBASE/0x80
		//      0A = HDW_OPT_LD (relief/batman/shuuz = 0x4410, shuuz = 0x0250, offtwall = 0x0A10)
		//              8000 = OPFHSIZ - subtract 1 from PF2 attribute horizontal stamp address
		//              4000 = OPIUM - playfield 2 enable
		//              2000 = OLS_EN - linescroll enable
		//              1000 = OSLPINK - split links enable
		//              0800 = OSLORAM - slow MO DMA cycles
		//              0400 = O_DCDMA - enable double time color RAM DMA cycles
		//              0200 = O_DVDMA - enable double time video RAM DMA cycles
		//              0100 = OADROEN - tristate address bus
		//              0080 = OAS_EN - enable autostore of playfield attributes
		//              0040 = OCRDWE - enable 8-bit color RAM
		//              0020 = OVRDTACK - wait for VRAM DMA on writes
		//              0010 = OCRDTACK - wait for CRAM DMA on writes
		//              0008 = OALHSIZ - alpha horiz stamp size (0=8)
		//              0004 = OPFHSIZ - PF horiz stamp size (0=8)
		//              0002 = OPFVSIZ - PF vertical stamp size (0=8)
		//              0001 = OMOVSIZ - MO vertical stamp size (0=8)
		//      10 = HDW_MOCON (MOB chip MOB code)
		//      11 = HDW_PCON (MOB chip PF code)
		//      12 = HDW_GRCON (MOB chip GR code)
		//      13 = MO_hscroll (9)
		//      14 = PF1_hscroll (A)
		//      15 = PF2_hscroll (B)
		//      16 = MO_vscroll (D)
		//      17 = PF1_vscroll (E)
		//      18 = PF2_vscroll (F)
		//
		//  3efffe = reset to wrong configuration?
		//

		case 0:
//if (oldword != newword) printf("Word 0 = %04X\n", newword);
			break;

		// set the scanline interrupt here
		case 0x03:
			if (oldword != newword || !m_scanline_int_timer->enabled())
				m_scanline_int_timer->adjust(screen().time_until_pos(newword & 0x1ff));
			break;

		// latch enable
		case 0x0a:
			// check for palette banking
			if (m_palette_bank != BIT(~newword, 10))
			{
				screen().update_partial(screen().vpos());
				m_palette_bank = BIT(~newword, 10);
			}
//if ((oldword & ~0x0080) != (newword & ~0x0080)) printf("Latch control = %04X\n", newword);
			break;

		// indexed parameters
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
			update_parameter(newword);
			break;

		// scanline IRQ ack here
		case 0x1e:
			m_scanline_int_cb(CLEAR_LINE);
			break;

		// log anything else
		default:
			if (oldword != newword)
				logerror("vc_w(%02X, %04X) ** [prev=%04X]\n", offset, newword, oldword);
			break;
	}
}


//-------------------------------------------------
//  update_pf_xscrolls: Update the playfield
//  scroll values.
//-------------------------------------------------

inline void atari_vad_device::update_pf_xscrolls()
{
	m_playfield_tilemap->set_scrollx(0, m_pf0_xscroll_raw + ((m_pf1_xscroll_raw) & 7) + m_pf_xoffset);
	if (m_playfield2_tilemap != nullptr)
		m_playfield2_tilemap->set_scrollx(0, m_pf1_xscroll_raw + m_pf2_xoffset);
}

//-------------------------------------------------
//  update_parameter: Update parameters, shared
//  between end-of-frame, tilerow updates, and
//  direct control writes.
//-------------------------------------------------

void atari_vad_device::update_parameter(uint16_t newword)
{
	switch (newword & 15)
	{
		case 9:
			m_mo_xscroll = (newword >> 7) & 0x1ff;
			if (m_mob != nullptr)
				m_mob->set_xscroll(m_mo_xscroll);
			break;

		case 10:
			m_pf1_xscroll_raw = (newword >> 7) & 0x1ff;
			update_pf_xscrolls();
			break;

		case 11:
			m_pf0_xscroll_raw = (newword >> 7) & 0x1ff;
			update_pf_xscrolls();
			break;

		case 13:
			m_mo_yscroll = (newword >> 7) & 0x1ff;
			if (m_mob != nullptr)
				m_mob->set_yscroll(m_mo_yscroll);
			break;

		case 14:
			m_pf1_yscroll = (newword >> 7) & 0x1ff;
			if (m_playfield2_tilemap != nullptr)
				m_playfield2_tilemap->set_scrolly(0, m_pf1_yscroll);
			break;

		case 15:
			m_pf0_yscroll = (newword >> 7) & 0x1ff;
			m_playfield_tilemap->set_scrolly(0, m_pf0_yscroll);
			break;
	}
}


//-------------------------------------------------
//  update_tilerow: Fetch parameters stored at
//  the end of the current tilerow, which affect
//  rowscrolling.
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(atari_vad_device::update_tilerow)
{
	int scanline = param;

	// skip if out of bounds, or not enabled
	if (scanline <= screen().visible_area().bottom() && BIT(m_control[0x0a], 13) && m_alpha_tilemap != nullptr)
	{
		// iterate over non-visible alpha tiles in this row
		int offset = scanline / 8 * 64 + 48 + 2 * (scanline % 8);
		int data0 = m_alpha_tilemap->basemem_read(offset++);
		int data1 = m_alpha_tilemap->basemem_read(offset++);

		// force an update if we have data
		if (scanline > 0 && ((data0 | data1) & 15) != 0)
			screen().update_partial(scanline - 1);

		// write the data
		if ((data0 & 15) != 0)
			update_parameter(data0);
		if ((data1 & 15) != 0)
			update_parameter(data1);
	}

	// update the timer to go off at the start of the next row
	scanline += BIT(m_control[0x0a], 13) ? 1 : 8;
	if (scanline >= screen().height())
		scanline = 0;
	m_tilerow_update_timer->adjust(screen().time_until_pos(scanline), scanline);
}


//-------------------------------------------------
//  eof_update: Callback that slurps up data and
//  feeds it into the video controller registers
//  every refresh.
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(atari_vad_device::eof_update)
{
	// echo all the commands to the video controller
	for (int i = 0; i < 0x1c; i++)
		if (m_eof_data[i] != 0)
			internal_control_write(i, m_eof_data[i]);

	// update the scroll positions
/*  atarimo_set_xscroll(0, m_mo_xscroll);
    atarimo_set_yscroll(0, m_mo_yscroll);

    update_pf_xscrolls();

    m_playfield_tilemap->set_scrolly(0, m_pf0_yscroll);
    if (m_playfield2_tilemap != nullptr)
        m_playfield2_tilemap->set_scrolly(0, m_pf1_yscroll);*/
	m_eof_timer->adjust(screen().time_until_pos(0));

	// use this for debugging the video controller values
#if 0
	if (machine().input().code_pressed(KEYCODE_8))
	{
		static FILE *out;
		if (!out) out = fopen("scroll.log", "w");
		if (out)
		{
			for (i = 0; i < 64; i++)
				fprintf(out, "%04X ", data[i]);
			fprintf(out, "\n");
		}
	}
#endif
}
