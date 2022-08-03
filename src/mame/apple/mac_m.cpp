// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont
/****************************************************************************

    mac_m.cpp

    Mac II series hardware

    Nate Woods
    Ernesto Corvi
    Raphael Nabet
    R. Belmont

    Mac Model Feature Summary:
                                CPU             FDC     Kbd/Mouse  PRAM     Video
         - Mac II               020             IWM     MacII ADB  ext      NuBus card
         - Mac IIx              030             SWIM    MacII ADB  ext      NuBus card
         - Mac IIfx             030             SWIM    IOP ADB    ext      NuBus card
         - Mac SE/30            030             SWIM    MacII ADB  ext      Internal fake NuBus card
         - Mac IIcx             030             SWIM    MacII ADB  ext      NuBus card
         - Mac IIci             030             SWIM    MacII ADB  ext      Internal "RBV" type
         - Mac IIsi             030             SWIM    Egret ADB  n/a      Internal "RBV" type

    Notes:
        - On the SE and most later Macs, the first access to ROM turns off the overlay.
          However, the Mac II/IIx/IIcx (and others?) have the old-style VIA overlay control bit!
        - The Mac II can have either a 68551 PMMU fitted or an Apple custom that handles 24 vs. 32
          bit addressing mode.  The ROM is *not* 32-bit clean so Mac OS normally runs in 24 bit mode,
          but A/UX can run 32.
        - There are 5 known kinds of host-side ADB hardware:
          * "Mac II ADB" used in the SE, II, IIx, IIcx, SE/30, IIci, Quadra 610, Quadra 650, Quadra 700,
             Quadra 800, Centris 610 and Centris 650.  This is a bit-banger using the VIA and a simple PIC.
          * "PMU ADB" used in the Mac Portable and all 680x0-based PowerBooks.
          * "Egret ADB" used in the IIsi, IIvi, IIvx, Classic II, LC, LC II, LC III, Performa 460,
             and Performa 600.  This is a 68HC05 with a different internal ROM than CUDA.
          * "IOP ADB" (ADB driven by a 6502 coprocessor, similar to Lisa) used in the IIfx,
            Quadra 900, and Quadra 950.
          * "Cuda ADB" (Apple's CUDA chip, which is a 68HC05 MCU) used in the Color Classic, LC 520,
            LC 55x, LC 57x, LC 58x, Quadra 630, Quadra 660AV, Quadra 840AV, PowerMac 6100/7100/8100,
            IIci, and PowerMac 5200.

     TODO:
        - Call the RTC timer

****************************************************************************/

#include "emu.h"
#include "mac.h"

#define INTS_RBV    ((m_model == MODEL_MAC_IICI) || (m_model == MODEL_MAC_IISI))

#ifdef MAME_DEBUG
#define LOG_ADB         0
#define LOG_VIA         0
#define LOG_MAC_IWM     0
#define LOG_GENERAL     0
#define LOG_KEYBOARD    0
#define LOG_MEMORY      0
#else
#define LOG_ADB         0
#define LOG_VIA         0
#define LOG_MAC_IWM     0
#define LOG_GENERAL     0
#define LOG_KEYBOARD    0
#define LOG_MEMORY      0
#endif

void mac_state::mac_install_memory(offs_t memory_begin, offs_t memory_end,
	offs_t memory_size, void *memory_data, int is_rom)
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	offs_t memory_mirror;

	memory_size = std::min(memory_size, (memory_end + 1 - memory_begin));
	memory_mirror = (memory_end - memory_begin) & ~(memory_size - 1);

	if (!is_rom)
	{
		space.install_ram(memory_begin, memory_end & ~memory_mirror, memory_mirror, memory_data);
	}
	else
	{
		space.unmap_write(memory_begin, memory_end);
		space.install_rom(memory_begin, memory_end & ~memory_mirror, memory_mirror, memory_data);
	}

	if (LOG_MEMORY)
	{
		printf("mac_install_memory(): range=[0x%06x...0x%06x] mirror=0x%06x ptr=0x%p\n",
			memory_begin, memory_end, memory_mirror, memory_data);
	}
}



/*
    Interrupt handling
*/

void mac_state::field_interrupts()
{
	int take_interrupt = -1;

	if (m_model != MODEL_MAC_IIFX)
	{
		if (m_scc_interrupt)
		{
			take_interrupt = 4;
		}
		else if (m_via2_interrupt)
		{
			take_interrupt = 2;
		}
		else if (m_via_interrupt)
		{
			take_interrupt = 1;
		}
	}
	else
	{
		return; // no interrupts for IIfx yet
	}

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
	}

	if (take_interrupt > -1)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
	}
}

WRITE_LINE_MEMBER(mac_state::set_scc_interrupt)
{
	m_scc_interrupt = state;
	this->field_interrupts();
}

void mac_state::set_via_interrupt(int value)
{
	m_via_interrupt = value;
	this->field_interrupts();
}


void mac_state::set_via2_interrupt(int value)
{
	m_via2_interrupt = value;
	this->field_interrupts();
}

WRITE_LINE_MEMBER(mac_state::mac_asc_irq)
{
	if (INTS_RBV)
	{
		if (state == ASSERT_LINE)
		{
			m_rbv_regs[3] |= 0x10; // any VIA 2 interrupt | sound interrupt
			rbv_recalc_irqs();
		}
		else
		{
			m_rbv_regs[3] &= ~0x10;
			rbv_recalc_irqs();
		}
	}
	else if (m_model != MODEL_MAC_IIFX)
	{
		m_via2->write_cb1(state^1);
	}
}

void mac_state::mac_autovector_w(offs_t offset, uint16_t data)
{
	if (LOG_GENERAL)
		logerror("mac_autovector_w: offset=0x%08x data=0x%04x\n", offset, data);

	/* This should throw an exception */

	/* Not yet implemented */
}

uint16_t mac_state::mac_autovector_r(offs_t offset)
{
	if (LOG_GENERAL)
		logerror("mac_autovector_r: offset=0x%08x\n", offset);

	/* This should throw an exception */

	/* Not yet implemented */
	return 0;
}

void mac_state::set_scc_waitrequest(int waitrequest)
{
	if (LOG_GENERAL)
		logerror("set_scc_waitrequest: waitrequest=%i\n", waitrequest);

	/* Not Yet Implemented */
}

void mac_state::set_memory_overlay(int overlay)
{
	offs_t memory_size;
	uint8_t *memory_data;
	int is_rom;

	/* normalize overlay */
	overlay = overlay ? true : false;

	if (overlay != m_overlay)
	{
		/* set up either main RAM area or ROM mirror at 0x000000-0x3fffff */
		if (overlay)
		{
			/* ROM mirror */
			memory_size = m_rom_size;
			memory_data = reinterpret_cast<uint8_t *>(m_rom_ptr);
			is_rom = true;
		}
		else
		{
			/* RAM */
			memory_size = m_ram->size();
			memory_data = m_ram->pointer();
			is_rom = false;
		}

		/* install the memory */
		if ((m_model == MODEL_MAC_IICI) || (m_model == MODEL_MAC_IISI))
		{
			// ROM is OK to flood to 3fffffff
			if (is_rom)
			{
				mac_install_memory(0x00000000, 0x3fffffff, memory_size, memory_data, is_rom);
			}
			else    // RAM: be careful not to populate ram B with a mirror or the ROM will get confused
			{
				mac_install_memory(0x00000000, memory_size-1, memory_size, memory_data, is_rom);
				// switch ROM region to direct access instead of through rom_switch_r
				mac_install_memory(0x40000000, 0x4007ffff, memory_size, memory_data, is_rom);
			}
		}
		else if (m_model == MODEL_MAC_IIFX)
		{
			address_space& space = m_maincpu->space(AS_PROGRAM);
			space.unmap_write(0x000000, 0x9fffff);
			mac_install_memory(0x000000, memory_size-1, memory_size, memory_data, is_rom);
		}
		else if ((m_model >= MODEL_MAC_II) && (m_model <= MODEL_MAC_SE30))
		{
			mac_install_memory(0x00000000, 0x3fffffff, memory_size, memory_data, is_rom);
		}
		else
		{
			mac_install_memory(0x00000000, 0x003fffff, memory_size, memory_data, is_rom);
		}

		m_overlay = overlay;

		if (LOG_GENERAL)
			logerror("mac_set_memory_overlay: overlay=%i\n", overlay);
	}
}

uint32_t mac_state::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay)
	{
		set_memory_overlay(0);
	}

	//printf("rom_switch_r: offset %08x ROM_size -1 = %08x, masked = %08x\n", offset, m_rom_size-1, offset & ((m_rom_size - 1)>>2));

	return m_rom_ptr[offset & ((m_rom_size - 1)>>2)];
}

/* *************************************************************************
 * SCSI
 * *************************************************************************/

/*

From http://www.mac.m68k-linux.org/devel/plushw.php

The address of each register is computed as follows:

  $580drn

  where r represents the register number (from 0 through 7),
  n determines whether it a read or write operation
  (0 for reads, or 1 for writes), and
  d determines whether the DACK signal to the NCR 5380 is asserted.
  (0 for not asserted, 1 is for asserted)

Here's an example of the address expressed in binary:

  0101 1000 0000 00d0 0rrr 000n

Note:  Asserting the DACK signal applies only to write operations to
       the output data register and read operations from the input
       data register.

  Symbolic            Memory
  Location            Location   NCR 5380 Internal Register

  scsiWr+sODR+dackWr  $580201    Output Data Register with DACK
  scsiWr+sODR         $580001    Output Data Register
  scsiWr+sICR         $580011    Initiator Command Register
  scsiWr+sMR          $580021    Mode Register
  scsiWr+sTCR         $580031    Target Command Register
  scsiWr+sSER         $580041    Select Enable Register
  scsiWr+sDMAtx       $580051    Start DMA Send
  scsiWr+sTDMArx      $580061    Start DMA Target Receive
  scsiWr+sIDMArx      $580071    Start DMA Initiator Receive

  scsiRd+sIDR+dackRd  $580260    Current SCSI Data with DACK
  scsiRd+sCDR         $580000    Current SCSI Data
  scsiRd+sICR         $580010    Initiator Command Register
  scsiRd+sMR          $580020    Mode Registor
  scsiRd+sTCR         $580030    Target Command Register
  scsiRd+sCSR         $580040    Current SCSI Bus Status
  scsiRd+sBSR         $580050    Bus and Status Register
  scsiRd+sIDR         $580060    Input Data Register
  scsiRd+sRESET       $580070    Reset Parity/Interrupt
             */

uint16_t mac_state::macplus_scsi_r(offs_t offset, uint16_t mem_mask)
{
	int reg = (offset>>3) & 0xf;

//  logerror("macplus_scsi_r: offset %x mask %x\n", offset, mem_mask);

	bool pseudo_dma = (reg == 6) && (offset == 0x130);

	return m_scsihelp->read_wrapper(pseudo_dma, reg)<<8;
}

uint32_t mac_state::macii_scsi_drq_r(offs_t offset, uint32_t mem_mask)
{
	switch (mem_mask)
	{
		case 0xff000000:
			return m_scsihelp->read_wrapper(true, 6)<<24;

		case 0xffff0000:
			return (m_scsihelp->read_wrapper(true, 6)<<24) | (m_scsihelp->read_wrapper(true, 6)<<16);

		case 0xffffffff:
			return (m_scsihelp->read_wrapper(true, 6)<<24) | (m_scsihelp->read_wrapper(true, 6)<<16) | (m_scsihelp->read_wrapper(true, 6)<<8) | m_scsihelp->read_wrapper(true, 6);

		default:
			logerror("macii_scsi_drq_r: unknown mem_mask %08x\n", mem_mask);
	}

	return 0;
}

void mac_state::macii_scsi_drq_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (mem_mask)
	{
		case 0xff000000:
			m_scsihelp->write_wrapper(true, 0, data>>24);
			break;

		case 0xffff0000:
			m_scsihelp->write_wrapper(true, 0, data>>24);
			m_scsihelp->write_wrapper(true, 0, data>>16);
			break;

		case 0xffffffff:
			m_scsihelp->write_wrapper(true, 0, data>>24);
			m_scsihelp->write_wrapper(true, 0, data>>16);
			m_scsihelp->write_wrapper(true, 0, data>>8);
			m_scsihelp->write_wrapper(true, 0, data&0xff);
			break;

		default:
			logerror("macii_scsi_drq_w: unknown mem_mask %08x\n", mem_mask);
			break;
	}
}

void mac_state::macplus_scsi_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int reg = (offset>>3) & 0xf;

//  logerror("macplus_scsi_w: data %x offset %x mask %x\n", data, offset, mem_mask);

	bool pseudo_dma = (reg == 0) && (offset == 0x100);

	m_scsihelp->write_wrapper(pseudo_dma, reg, data);
}

void mac_state::macii_scsi_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int reg = (offset>>3) & 0xf;

//  logerror("macplus_scsi_w: data %x offset %x mask %x (PC=%x)\n", data, offset, mem_mask, m_maincpu->pc());

	bool pseudo_dma = (reg == 0) && (offset == 0x100);

	m_scsihelp->write_wrapper(pseudo_dma, reg, data>>8);
}

WRITE_LINE_MEMBER(mac_state::mac_scsi_irq)
{
}

void mac_state::scsi_berr_w(uint8_t data)
{
	m_maincpu->pulse_input_line(M68K_LINE_BUSERROR, attotime::zero);
}

/* *************************************************************************
 * SCC
 *
 * Serial Communications Controller
 * *************************************************************************/
uint16_t mac_state::mac_scc_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		mac_via_sync();

	uint16_t result;

	result = m_scc->reg_r(offset);
	return (result << 8) | result;
}

void mac_state::mac_scc_w(offs_t offset, uint16_t data)
{
	mac_via_sync();
	m_scc->reg_w(offset, data);
}

void mac_state::mac_scc_2_w(offs_t offset, uint16_t data)
{
	mac_via_sync();
	m_scc->reg_w(offset, data >> 8);
}

/* ********************************** *
 * IWM Code specific to the Mac Plus  *
 * ********************************** */

uint16_t mac_state::mac_iwm_r(offs_t offset, uint16_t mem_mask)
{
	/* The first time this is called is in a floppy test, which goes from
	 * $400104 to $400126.  After that, all access to the floppy goes through
	 * the disk driver in the MacOS
	 *
	 * I just thought this would be on interest to someone trying to further
	 * this driver along
	 */

	uint16_t result = m_fdc->read(offset >> 8);

	if (!machine().side_effects_disabled())
		m_maincpu->adjust_icount(-5);

	if (LOG_MAC_IWM)
		printf("%s mac_iwm_r: offset=0x%08x mem_mask %04x = %02x\n", machine().describe_context().c_str(), offset, mem_mask, result);

	return (result << 8) | result;
}

void mac_state::mac_iwm_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (LOG_MAC_IWM)
		printf("mac_iwm_w: offset=0x%08x data=0x%04x mask %04x (PC=%x)\n", offset, data, mem_mask, m_maincpu->pc());

	if (ACCESSING_BITS_0_7)
		m_fdc->write((offset >> 8), data & 0xff);
	else
		m_fdc->write((offset >> 8), data>>8);

	if (!machine().side_effects_disabled())
		m_maincpu->adjust_icount(-5);
}

WRITE_LINE_MEMBER(mac_state::mac_adb_via_out_cb2)
{
	//printf("VIA OUT CB2 = %x\n", state);
	if (ADB_IS_EGRET)
	{
		m_egret->set_via_data(state & 1);
	}
	else
	{
		if (m_macadb)
		{
			m_macadb->adb_data_w(state);
		}
	}
}

/* *************************************************************************
 * VIA
 * *************************************************************************
 *
 *
 * PORT A
 *
 *  bit 7               R   SCC Wait/Request
 *  bit 6               W   Main/Alternate screen buffer select
 *  bit 5               W   Floppy Disk Line Selection
 *  bit 4               W   Overlay/Normal memory mapping
 *  bit 3               W   Main/Alternate sound buffer
 *  bit 2-0             W   Sound Volume
 *
 *
 * PORT B
 *
 *  bit 7               W   Sound enable
 *  bit 6               R   Video beam in display
 *  bit 5   (pre-ADB)   R   Mouse Y2
 *          (ADB)       W   ADB ST1
 *  bit 4   (pre-ADB)   R   Mouse X2
 *          (ADB)       W   ADB ST0
 *  bit 3   (pre-ADB)   R   Mouse button (active low)
 *          (ADB)       R   ADB INT
 *  bit 2               W   Real time clock enable
 *  bit 1               W   Real time clock data clock
 *  bit 0               RW  Real time clock data
 *
 */

#define PA6 0x40
#define PA4 0x10
#define PA2 0x04
#define PA1 0x02

uint8_t mac_state::mac_via_in_a()
{
//  printf("%s VIA1 IN_A\n", machine().describe_context().c_str());

	switch (m_model)
	{
		case MODEL_MAC_II:
		case MODEL_MAC_II_FDHD:
		case MODEL_MAC_IIX:
			return 0x81;        // bit 0 must be set on most Macs to avoid attempting to boot from AppleTalk

		case MODEL_MAC_SE30:
			return 0x81 | PA6;

		case MODEL_MAC_IICI:
			return 0x81 | PA6 | PA2 | PA1;

		case MODEL_MAC_IISI:
			return 0x81 | PA4 | PA2 | PA1;

		case MODEL_MAC_IIFX:
			return 0x81 | PA6 | PA4 | PA1;

		case MODEL_MAC_IICX:
			return 0x81 | PA6;

		default:
			return 0x80;
	}
}

uint8_t mac_state::mac_via_in_b()
{
	int val = 0;
	/* video beam in display (! VBLANK && ! HBLANK basically) */
	if (m_screen->vpos() >= MAC_V_VIS)
		val |= 0x40;

	if (ADB_IS_BITBANG_CLASS)
	{
		val |= m_macadb->get_adb_state()<<4;

		if (!m_adb_irq_pending)
		{
			val |= 0x08;
		}

		val |= m_rtc->data_r();
	}
	else if (ADB_IS_EGRET)
	{
		val |= m_egret->get_xcvr_session()<<3;
	}

//  printf("%s VIA1 IN_B = %02x\n", machine().describe_context().c_str(), val);

	return val;
}

uint8_t mac_state::mac_via_in_b_ii()
{
	int val = 0;

	if (ADB_IS_BITBANG_CLASS)
	{
		val |= m_macadb->get_adb_state()<<4;

		if (!m_adb_irq_pending)
		{
			val |= 0x08;
		}

		val |= m_rtc->data_r();
	}
	else if (ADB_IS_EGRET)
	{
		val |= m_egret->get_xcvr_session()<<3;
	}

//  printf("%s VIA1 IN_B_II = %02x\n", machine().describe_context().c_str(), val);

	return val;
}

void mac_state::mac_via_out_a(uint8_t data)
{
//  printf("%s VIA1 OUT A: %02x\n", machine().describe_context().c_str(), data);

	set_scc_waitrequest((data & 0x80) >> 7);
	m_screen_buffer = (data & 0x40) >> 6;
	if (m_cur_floppy)
		m_cur_floppy->ss_w((data & 0x20) >> 5);
}

void mac_state::mac_via_out_b(uint8_t data)
{
//  printf("%s VIA1 OUT B: %02x\n", machine().describe_context().c_str(), data);
	m_rtc->ce_w((data & 0x04)>>2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

void mac_state::mac_via_out_b_bbadb(uint8_t data)
{
//  printf("%s VIA1 OUT B: %02x\n", machine().describe_context().c_str(), data);

	if (m_model == MODEL_MAC_SE30)
	{
		// 0x40 = 0 means enable vblank on SE/30
		m_se30_vbl_enable = (data & 0x40) ? 0 : 1;

		// clear the interrupt if we disabled it
		if (!m_se30_vbl_enable)
		{
			nubus_slot_interrupt(0xe, 0);
		}
	}

	m_macadb->mac_adb_newaction((data & 0x30) >> 4);

	m_rtc->ce_w((data & 0x04)>>2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

void mac_state::mac_via_out_b_egadb(uint8_t data)
{
//  printf("%s VIA1 OUT B: %02x\n", machine().describe_context().c_str(), data);

	#if LOG_ADB
	printf("%s 68K: New Egret state: SS %d VF %d\n", machine().describe_context().c_str(), (data>>5)&1, (data>>4)&1);
	#endif
	m_egret->set_via_full((data&0x10) ? 1 : 0);
	m_egret->set_sys_session((data&0x20) ? 1 : 0);
}

WRITE_LINE_MEMBER(mac_state::mac_via_irq)
{
	/* interrupt the 68k (level 1) */
	set_via_interrupt(state);
}

void mac_state::mac_via_sync()
{
	// The via runs at 783.36KHz while the main cpu runs at 15MHz or
	// more, so we need to sync the access with the via clock.  Plus
	// the whole access takes half a (via) cycle and ends when synced
	// with the main cpu again.

	// Get the main cpu time
	u64 cycle = m_maincpu->total_cycles();

	// Get the number of the cycle the via is in at that time
	u64 via_cycle = cycle * m_via1->clock() / m_maincpu->clock();

	// The access is going to start at via_cycle+1 and end at
	// via_cycle+1.5, compute what that means in maincpu cycles (the
	// +1 rounds up, since the clocks are too different to ever be
	// synced).
	u64 main_cycle = (via_cycle*2+3) * m_maincpu->clock() / (2*m_via1->clock()) + 1;

	// Finally adjust the main cpu icount as needed.
	m_maincpu->adjust_icount(-int(main_cycle - cycle));
}

uint16_t mac_state::mac_via_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		mac_via_sync();

	uint16_t data;

	offset >>= 8;
	offset &= 0x0f;

	if (LOG_VIA)
		logerror("mac_via_r: offset=0x%02x\n", offset);
	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void mac_state::mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	mac_via_sync();

	offset >>= 8;
	offset &= 0x0f;

	if (LOG_VIA)
		logerror("mac_via_w: offset=0x%02x data=0x%08x\n", offset, data);

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

/* *************************************************************************
 * VIA 2 (on Mac IIs, PowerBooks > 100, and PowerMacs)
 * *************************************************************************/

WRITE_LINE_MEMBER(mac_state::mac_via2_irq)
{
	set_via2_interrupt(state);
}

uint16_t mac_state::mac_via2_r(offs_t offset)
{
	int data;

	offset >>= 8;
	offset &= 0x0f;

	data = m_via2->read(offset);

	if (LOG_VIA)
		logerror("mac_via2_r: offset=0x%02x = %02x (PC=%x)\n", offset*2, data, m_maincpu->pc());

	return (data & 0xff) | (data << 8);
}

void mac_state::mac_via2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	if (LOG_VIA)
		logerror("mac_via2_w: offset=%x data=0x%08x mask=%x (PC=%x)\n", offset, data, mem_mask, m_maincpu->pc());

	if (ACCESSING_BITS_0_7)
		m_via2->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via2->write(offset, (data >> 8) & 0xff);
}


uint8_t mac_state::mac_via2_in_a()
{
	return 0xc0 | m_nubus_irq_state;
}

uint8_t mac_state::mac_via2_in_b()
{
//  logerror("%s VIA2 IN B\n", machine().describe_context());

	if ((m_model == MODEL_MAC_SE30) || (m_model == MODEL_MAC_IIX))
	{
		return 0x87;    // bits 3 and 6 are tied low on SE/30
	}

	return 0xcf;        // indicate no NuBus transaction error
}

void mac_state::mac_via2_out_a(uint8_t data)
{
//  logerror("%s VIA2 OUT A: %02x\n", machine().describe_context(), data);
}

void mac_state::mac_via2_out_b(uint8_t data)
{
//  logerror("%s VIA2 OUT B: %02x\n", machine().describe_context(), data);

	// chain 60.15 Hz to VIA1
	m_via1->write_ca1(data>>7);

	if (m_model == MODEL_MAC_II)
	{
		m68000_base_device *m68k = downcast<m68000_base_device *>(m_maincpu.target());
		m68k->set_hmmu_enable((data & 0x8) ? M68K_HMMU_DISABLE : M68K_HMMU_ENABLE_II);
	}
}

// This signal is generated internally on RBV, V8, VASP, Eagle, etc.
TIMER_CALLBACK_MEMBER(mac_state::mac_6015_tick)
{
	m_via1->write_ca1(0);
	m_via1->write_ca1(1);
}

/* *************************************************************************
 * Main
 * *************************************************************************/

void mac_state::machine_start()
{
	if (m_screen)
	{
		this->m_scanline_timer = timer_alloc(FUNC(mac_state::mac_scanline_tick), this);
		this->m_scanline_timer->adjust(m_screen->time_until_pos(0, 0));
	}
	else
	{
		m_adbupdate_timer = timer_alloc(FUNC(mac_state::mac_adbrefresh_tick), this);
		m_adbupdate_timer->adjust(attotime::from_hz(70));
	}

	if (m_model != MODEL_MAC_IIFX)
		m_6015_timer = timer_alloc(FUNC(mac_state::mac_6015_tick), this);
	else
		m_6015_timer = timer_alloc(FUNC(mac_state::oss_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	save_item(NAME(m_nubus_irq_state));
	save_item(NAME(m_se30_vbl_enable));
	save_item(NAME(m_adb_irq_pending));
	save_item(NAME(ca1_data));
	if ((m_model == MODEL_MAC_IICI) || (m_model == MODEL_MAC_IISI))
	{
		save_item(NAME(m_rbv_regs));
		save_item(NAME(m_rbv_ier));
		save_item(NAME(m_rbv_ifr));
		save_item(NAME(m_rbv_colors));
		save_item(NAME(m_rbv_count));
		save_item(NAME(m_rbv_clutoffs));
		save_item(NAME(m_rbv_palette));
	}
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_via2_interrupt));
	save_item(NAME(m_scsi_interrupt));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_oss_regs));
	save_item(NAME(m_via2_ca1_hack));
}

void mac_state::machine_reset()
{
	if (ADB_IS_EGRET)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}

	// stop 60.15 Hz timer
	m_6015_timer->adjust(attotime::never);

	if ((m_model == MODEL_MAC_IICI) || (m_model == MODEL_MAC_IISI))
	{
		m_rbv_vbltime = 0;
		macrbv_reset();
	}

	// start 60.15 Hz timer for most systems
	if ((m_model >= MODEL_MAC_IICI) && (m_model <= MODEL_MAC_IIFX))
	{
		m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));
	}

	m_last_taken_interrupt = -1;

	/* setup the memory overlay */
	m_overlay = -1; // insure no match
	this->set_memory_overlay(1);

	if (m_overlay_timeout != (emu_timer *)nullptr)
	{
		m_overlay_timeout->adjust(m_maincpu->cycles_to_attotime(8));
	}

	/* setup videoram */
	this->m_screen_buffer = 1;

	if (m_model != MODEL_MAC_IIFX)  // prime CB1 for ASC and slot interrupts
	{
		m_via2_ca1_hack = 1;
		m_via2->write_ca1(1);
		m_via2->write_cb1(1);
	}

	m_scsi_interrupt = 0;

	m_via2_vbl = 0;
	m_se30_vbl_enable = 0;
	m_nubus_irq_state = 0xff;
	m_last_taken_interrupt = 0;
}

WRITE_LINE_MEMBER(mac_state::egret_reset_w)
{
	if (state == ASSERT_LINE)
	{
		set_memory_overlay(0);
		set_memory_overlay(1);
	}

	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}

void mac_state::mac_state_load()
{
	int overlay;
	overlay = m_overlay;
	m_overlay = -1;
	set_memory_overlay(overlay);
}


TIMER_CALLBACK_MEMBER(mac_state::overlay_timeout_func)
{
	if (m_overlay != -1)
	{
		set_memory_overlay(0);      // kill the overlay
	}

	// we're a one-time-only thing
	m_overlay_timeout->adjust(attotime::never);
}

void mac_state::mac_driver_init(model_t model)
{
	m_overlay = 1;
	m_scsi_interrupt = 0;
	m_model = model;
	m_scc_interrupt = 0;
	m_via_interrupt = 0;
	m_via2_interrupt = 0;

	m_rom_size = memregion("bootrom")->bytes();
	m_rom_ptr = reinterpret_cast<uint32_t *>(memregion("bootrom")->base());

	m_overlay = -1;
	set_memory_overlay(1);

	memset(m_ram->pointer(), 0, m_ram->size());

	if ((m_model >= MODEL_MAC_II) && (m_model <= MODEL_MAC_SE30))
	{
		m_overlay_timeout = timer_alloc(FUNC(mac_state::overlay_timeout_func), this);
	}
	else
	{
		m_overlay_timeout = (emu_timer *)nullptr;
	}

	/* save state stuff */
	machine().save().register_postload(save_prepost_delegate(FUNC(mac_state::mac_state_load), this));
}

#define MAC_DRIVER_INIT(label, model)   \
void mac_state::init_##label()     \
{   \
	mac_driver_init(model); \
}

MAC_DRIVER_INIT(maciici, MODEL_MAC_IICI)
MAC_DRIVER_INIT(maciisi, MODEL_MAC_IISI)
MAC_DRIVER_INIT(macii, MODEL_MAC_II)
MAC_DRIVER_INIT(macse30, MODEL_MAC_SE30)
MAC_DRIVER_INIT(maciifx, MODEL_MAC_IIFX)
MAC_DRIVER_INIT(maciicx, MODEL_MAC_IICX)
MAC_DRIVER_INIT(maciifdhd, MODEL_MAC_II_FDHD)
MAC_DRIVER_INIT(maciix, MODEL_MAC_IIX)

void mac_state::nubus_slot_interrupt(uint8_t slot, uint32_t state)
{
	static const uint8_t masks[8] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
	uint8_t mask = 0x3f;

	slot -= 9;

	if (state)
	{
		m_nubus_irq_state &= ~masks[slot];
	}
	else
	{
		m_nubus_irq_state |= masks[slot];
	}

	if ((m_model != MODEL_MAC_IIFX) && (!INTS_RBV))
	{
		if ((m_nubus_irq_state & mask) != mask)
		{
			// HACK: sometimes we miss an ack (possible misbehavior in the VIA?)
			if (m_via2_ca1_hack == 0)
			{
				m_via2->write_ca1(1);
			}
			m_via2_ca1_hack = 0;
			m_via2->write_ca1(0);
		}
		else
		{
			m_via2_ca1_hack = 1;
			m_via2->write_ca1(1);
		}
	}

	if (INTS_RBV)
	{
		m_rbv_regs[2] &= ~0x38;
		m_rbv_regs[2] |= (m_nubus_irq_state & 0x38);
		rbv_recalc_irqs();
	}
}

void mac_state::vblank_irq()
{
	/* handle ADB keyboard/mouse */
	if (m_macadb)
	{
		m_macadb->adb_vblank();
	}

	// handle SE/30 vblank IRQ
	if (m_model == MODEL_MAC_SE30)
	{
		if (m_se30_vbl_enable)
		{
			m_via2_vbl ^= 1;
			if (!m_via2_vbl)
			{
				this->nubus_slot_interrupt(0xe, 1);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(mac_state::mac_adbrefresh_tick)
{
	vblank_irq();
	m_adbupdate_timer->adjust(attotime::from_hz(70));
}

TIMER_CALLBACK_MEMBER(mac_state::mac_scanline_tick)
{
	int scanline;

	if (m_rbv_vbltime > 0)
	{
		m_rbv_vbltime--;

		if (m_rbv_vbltime == 0)
		{
			m_rbv_regs[2] |= 0x40;
			rbv_recalc_irqs();
		}
	}

	scanline = m_screen->vpos();
	if (scanline == MAC_V_VIS)
		vblank_irq();

	int next_scanline = (scanline+1) % MAC_V_TOTAL;
	m_scanline_timer->adjust(m_screen->time_until_pos(next_scanline), next_scanline);
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_9_w)
{
	nubus_slot_interrupt(9, state);
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_a_w)
{
	nubus_slot_interrupt(0xa, state);
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_b_w)
{
	nubus_slot_interrupt(0xb, state);
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_c_w)
{
	nubus_slot_interrupt(0xc, state);
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_d_w)
{
	nubus_slot_interrupt(0xd, state);
}

WRITE_LINE_MEMBER(mac_state::nubus_irq_e_w)
{
	nubus_slot_interrupt(0xe, state);
}

void mac_state::phases_w(uint8_t phases)
{
	if(m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void mac_state::sel35_w(int sel35)
{
	logerror("fdc mac sel35 %d\n", sel35);
}

void mac_state::devsel_w(uint8_t devsel)
{
	if(devsel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if(devsel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;
	m_fdc->set_floppy(m_cur_floppy);
	if(m_cur_floppy)
		m_cur_floppy->ss_w((m_via1->read_pa() & 0x20) >> 5);
}

void mac_state::hdsel_w(int hdsel)
{
}
