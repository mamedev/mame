// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont
/****************************************************************************

    machine/mac.c

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
         - Mac IIvx/IIvi        030             SWIM    Egret ADB  n/a      Internal "VASP" type
         - Mac LC               020             SWIM    Egret ADB  n/a      Internal "V8" type
         - Mac LC II            030             SWIM    Egret ADB  n/a      Internal "V8" type
         - Mac LC III           030             SWIM    Egret ADB  n/a      Internal "Sonora" type
         - Mac Classic II       030             SWIM    Egret ADB  n/a      Internal "Eagle" type (V8 clone)
         - Mac Color Classic    030             SWIM    Cuda ADB   n/a      Internal "Spice" type (V8 clone)

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

     Egret version spotting:
     341S0850 - 0x???? (1.01, earlier) - LC, LC II
     341S0851 - 0x0101 (1.01, later) - Classic II, IIsi, IIvx/IIvi, LC III
     344S0100 - 0x0100 (1.00) - Some (early production?) IIsi

     Cuda version spotting:
     341S0262 - 0x0003f200 (3.02) - some PMac 6500, Bondi blue iMac
     341S0285 - No version (x.xx) - PMac 4400 + Mac clones ("Cuda Lite" with 768 bytes more ROM + PS/2 keyboard/mouse support)
     341S0060 - 0x00020028 (2.40) - Performa/Quadra 6xx, PMac 6200, x400, some x500, Pippin, "Gossamer" G3, others?
                                    (verified found in PMac 5500-225, G3-333)
     341S0788 - 0x00020025 (2.37) - LC 475/575/Quadra 605, Quadra 660AV/840AV, PMac 7200
     341S0417 - 0x00020023 (2.35) - Color Classic

     Caboose version spotting:
     341S0853 - 0x0100 (1.00) - Quadra 950

     PG&E (68HC05 PMU) version spotting:
     (find the text "BORG" in the system ROM, the next 32768 bytes are the PG&E image.
      offset +4 in the image is the version byte).
     01 - PowerBook Duo 210/230/250
     02 - PowerBook 540c, PBDuo 270C, PBDuo 280/280C
     03 - PowerBook 150
     08 - PB190cs, PowerBook 540c PPC update, all PowerPC PowerBooks through WallStreet G3s

****************************************************************************/

#include "emu.h"
#include "mac.h"

#define INTS_RBV    ((m_model >= MODEL_MAC_IICI) && (m_model <= MODEL_MAC_IIVI)) || ((m_model >= MODEL_MAC_LC) && (m_model <= MODEL_MAC_LC_580))

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

void mac_state::v8_resize()
{
	offs_t memory_size;
	uint8_t *memory_data;
	int is_rom;

	is_rom = (m_overlay) ? 1 : 0;

	// get what memory we're going to map
	if (is_rom)
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

//    printf("mac_v8_resize: memory_size = %x, ctrl bits %02x (overlay %d = %s)\n", memory_size, m_rbv_regs[1] & 0xe0, m_overlay, is_rom ? "ROM" : "RAM");

	if (is_rom)
	{
		mac_install_memory(0x00000000, memory_size-1, memory_size, memory_data, is_rom);

		// install catcher in place of ROM that will detect the first access to ROM in its real location
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xa00000, 0xafffff, read32sm_delegate(*this, FUNC(mac_state::rom_switch_r)), 0xffffffff);
	}
	else
	{
		address_space& space = m_maincpu->space(AS_PROGRAM);
		uint32_t onboard_amt, simm_amt, simm_size;
		static const uint32_t simm_sizes[4] = { 0, 2*1024*1024, 4*1024*1024, 8*1024*1024 };

		// re-install ROM in its normal place
		size_t rom_mirror = 0xfffff ^ (m_rom_size - 1);
		m_maincpu->space(AS_PROGRAM).install_rom(0xa00000, 0xafffff, rom_mirror, m_rom_ptr);

		// force unmap of entire RAM region
		space.unmap_write(0, 0x9fffff);

		// LC and Classic II have 2 MB built-in, all other V8-style machines have 4 MB
		// we reserve the first 2 or 4 MB of mess_ram for the onboard,
		// RAM above that mark is the SIMM
		onboard_amt = ((m_model == MODEL_MAC_LC) || (m_model == MODEL_MAC_CLASSIC_II)) ? 2*1024*1024 : 4*1024*1024;
		simm_amt = (m_rbv_regs[1]>>6) & 3;  // size of SIMM RAM window
		simm_size = memory_size - onboard_amt;  // actual amount of RAM available for SIMMs

		// installing SIMM RAM?
		if (simm_amt != 0)
		{
//            printf("mac_v8_resize: SIMM region size is %x, SIMM size is %x, onboard size is %x\n", simm_sizes[simm_amt], simm_size, onboard_amt);

			if ((simm_amt > 0) && (simm_size > 0))
			{
//              mac_install_memory(0x000000, simm_sizes[simm_amt]-1, simm_sizes[simm_amt], memory_data + onboard_amt, is_rom);
				mac_install_memory(0x000000, simm_size-1, simm_size, memory_data + onboard_amt, is_rom);
			}

			// onboard RAM sits immediately above the SIMM, if any
			if (simm_sizes[simm_amt] + onboard_amt <= 0x800000)
			{
				mac_install_memory(simm_sizes[simm_amt], simm_sizes[simm_amt] + onboard_amt - 1, onboard_amt, memory_data, is_rom);
			}

			// a mirror of the first 2 MB of on board RAM always lives at 0x800000
			mac_install_memory(0x800000, 0x9fffff, 0x200000, memory_data, is_rom);
		}
		else
		{
//          printf("mac_v8_resize: SIMM off, mobo RAM at 0 and top\n");

			mac_install_memory(0x000000, onboard_amt-1, onboard_amt, memory_data, is_rom);
			mac_install_memory(0x900000, 0x9fffff, 0x200000, memory_data+0x100000, is_rom);
		}
	}
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
		if (((m_model >= MODEL_MAC_LC) && (m_model <= MODEL_MAC_COLOR_CLASSIC) && ((m_model != MODEL_MAC_LC_III) && (m_model != MODEL_MAC_LC_III_PLUS))) || (m_model == MODEL_MAC_CLASSIC_II))
		{
			m_overlay = overlay;
			v8_resize();
		}
		else if ((m_model == MODEL_MAC_IICI) || (m_model == MODEL_MAC_IISI))
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
		else if ((m_model >= MODEL_MAC_II) && (m_model <= MODEL_MAC_SE30) && (m_model != MODEL_MAC_IIVX) && (m_model != MODEL_MAC_IIVI))
		{
			mac_install_memory(0x00000000, 0x3fffffff, memory_size, memory_data, is_rom);
		}
		else if ((m_model == MODEL_MAC_IIVX) || (m_model == MODEL_MAC_IIVI) || (m_model == MODEL_MAC_LC_III) || (m_model == MODEL_MAC_LC_III_PLUS) || (m_model >= MODEL_MAC_LC_475 && m_model <= MODEL_MAC_LC_580))   // up to 36 MB
		{
			mac_install_memory(0x00000000, memory_size-1, memory_size, memory_data, is_rom);

			if (is_rom)
			{
				m_maincpu->space(AS_PROGRAM).install_read_handler(0x40000000, 0x4fffffff, read32sm_delegate(*this, FUNC(mac_state::rom_switch_r)), 0xffffffff);
			}
			else
			{
				size_t rom_mirror = 0xfffffff ^ (m_rom_size - 1);
				m_maincpu->space(AS_PROGRAM).install_rom(0x40000000, 0x4fffffff & ~rom_mirror, rom_mirror, m_rom_ptr);
			}
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
/*  mac_state *mac = machine.driver_data<mac_state>();

    if ((mac->m_scsiirq_enable) && ((mac->m_model == MODEL_MAC_SE) || (mac->m_model == MODEL_MAC_CLASSIC)))
    {
        mac->m_scsi_interrupt = state;
        mac->field_interrupts();
    }*/
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
	else if (ADB_IS_CUDA)
	{
		m_cuda->set_via_data(state & 1);
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

		case MODEL_MAC_LC:
		case MODEL_MAC_LC_II:
		case MODEL_MAC_IIVX:
		case MODEL_MAC_IIVI:
			return 0x81 | PA6 | PA4 | PA2;

		case MODEL_MAC_IICI:
			return 0x81 | PA6 | PA2 | PA1;

		case MODEL_MAC_IISI:
			return 0x81 | PA4 | PA2 | PA1;

		case MODEL_MAC_IIFX:
			return 0x81 | PA6 | PA4 | PA1;

		case MODEL_MAC_IICX:
			return 0x81 | PA6;

		case MODEL_MAC_CLASSIC_II:
		case MODEL_MAC_QUADRA_800:
			return 0x81 | PA4 | PA1;

		case MODEL_MAC_QUADRA_900:
			return 0x81 | PA6 | PA4;

		case MODEL_MAC_COLOR_CLASSIC:
			return 0x81 | PA1;

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
	else if (ADB_IS_CUDA)
	{
		logerror("%s cuda treq %d\n", machine().time().to_string(), m_cuda->get_treq());
		val |= m_cuda->get_treq()<<3;
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
	else if (ADB_IS_CUDA)
	{
		val |= m_cuda->get_treq()<<3;
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

void mac_state::mac_via_out_b_cdadb(uint8_t data)
{
//  printf("%s VIA1 OUT B: %02x\n", machine().describe_context().c_str(), data);

	#if LOG_ADB
	printf("%s 68K: New Cuda state: TIP %d BYTEACK %d\n", machine().describe_context().c_str(), (data>>5)&1, (data>>4)&1);
	#endif
	m_cuda->set_byteack((data&0x10) ? 1 : 0);
	m_cuda->set_tip((data&0x20) ? 1 : 0);
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
	uint8_t result;

	if ((m_model == MODEL_MAC_QUADRA_900) || (m_model == MODEL_MAC_QUADRA_950))
	{
		result = 0x80 | m_nubus_irq_state;
	}
	else
	{
		result = 0xc0 | m_nubus_irq_state;
	}

	return result;
}

uint8_t mac_state::mac_via2_in_b()
{
//  logerror("%s VIA2 IN B\n", machine().describe_context());

	if ((m_model == MODEL_MAC_LC) || (m_model == MODEL_MAC_LC_II) || (m_model == MODEL_MAC_CLASSIC_II))
	{
		return 0x4f;
	}

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
	save_item(NAME(irq_count));
	save_item(NAME(ca1_data));
	save_item(NAME(ca2_data));
	save_item(NAME(m_rbv_regs));
	save_item(NAME(m_rbv_ier));
	save_item(NAME(m_rbv_ifr));
	save_item(NAME(m_rbv_colors));
	save_item(NAME(m_rbv_count));
	save_item(NAME(m_rbv_clutoffs));
	save_item(NAME(m_rbv_palette));
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
	if ((ADB_IS_EGRET) || (ADB_IS_CUDA))
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}

	// stop 60.15 Hz timer
	m_6015_timer->adjust(attotime::never);

	m_rbv_vbltime = 0;

	// start 60.15 Hz timer for most systems
	if (((m_model >= MODEL_MAC_IICI) && (m_model <= MODEL_MAC_IIFX)) || (m_model >= MODEL_MAC_LC))
	{
		m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));
	}

	// default to 32-bit mode on LC
	if (m_model == MODEL_MAC_LC)
	{
		m68000_base_device *m68k = downcast<m68000_base_device *>(m_maincpu.target());
		m68k->set_hmmu_enable(M68K_HMMU_DISABLE);
	}

	m_last_taken_interrupt = -1;

	/* setup the memory overlay */
	m_overlay = -1; // insure no match
	this->set_memory_overlay(1);

	if (m_overlay_timeout != (emu_timer *)nullptr)
	{
		if ((m_model == MODEL_MAC_LC_III) || (m_model == MODEL_MAC_LC_III_PLUS) || (m_model >= MODEL_MAC_LC_475 && m_model <= MODEL_MAC_LC_580))   // up to 36 MB
		{
			m_overlay_timeout->adjust(attotime::never);
		}
		else if (((m_model >= MODEL_MAC_LC) && (m_model <= MODEL_MAC_COLOR_CLASSIC) && ((m_model != MODEL_MAC_LC_III) && (m_model != MODEL_MAC_LC_III_PLUS))) || (m_model == MODEL_MAC_CLASSIC_II))
		{
			m_overlay_timeout->adjust(attotime::never);
		}
		else if ((m_model >= MODEL_MAC_IIVX) && (m_model <= MODEL_MAC_IIVI))
		{
			m_overlay_timeout->adjust(attotime::never);
		}
		else
		{
			m_overlay_timeout->adjust(m_maincpu->cycles_to_attotime(8));
		}
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

WRITE_LINE_MEMBER(mac_state::cuda_reset_w)
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

uint32_t mac_state::mac_read_id()
{
//    printf("Mac read ID reg @ PC=%x\n", m_maincpu->pc());

	switch (m_model)
	{
		case MODEL_MAC_LC_III:
			return 0xa55a0001;  // 25 MHz LC III

		case MODEL_MAC_LC_III_PLUS:
			return 0xa55a0003;  // 33 MHz LC III+

		case MODEL_MAC_LC_475:
			return 0xa55a2221;

		case MODEL_MAC_LC_520:
			return 0xa55a0100;

		case MODEL_MAC_LC_550:
			return 0xa55a0101;

		case MODEL_MAC_LC_575:
			return 0xa55a222e;

		case MODEL_MAC_PBDUO_210:
			return 0xa55a1004;

		case MODEL_MAC_PBDUO_230:
			return 0xa55a1005;

		case MODEL_MAC_PBDUO_250:
			return 0xa55a1006;

		case MODEL_MAC_QUADRA_605:
			return 0xa55a2225;

		case MODEL_MAC_QUADRA_610:
		case MODEL_MAC_QUADRA_650:
		case MODEL_MAC_QUADRA_800:
			return 0xa55a2bad;

		case MODEL_MAC_QUADRA_660AV:
		case MODEL_MAC_QUADRA_840AV:
			return 0xa55a2830;

		case MODEL_MAC_IIVX:
			return 0xa55a2015;

		default:
			return 0;
	}
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

	if ((model == MODEL_MAC_CLASSIC_II) || (model == MODEL_MAC_LC) || (model == MODEL_MAC_COLOR_CLASSIC) || (model >= MODEL_MAC_LC_475 && model <= MODEL_MAC_LC_580) ||
		(model == MODEL_MAC_LC_II) || (model == MODEL_MAC_LC_III) || (model == MODEL_MAC_LC_III_PLUS) || ((m_model >= MODEL_MAC_II) && (m_model <= MODEL_MAC_SE30)))
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

MAC_DRIVER_INIT(maclc, MODEL_MAC_LC)
MAC_DRIVER_INIT(maclc2, MODEL_MAC_LC_II)
MAC_DRIVER_INIT(maclc3, MODEL_MAC_LC_III)
MAC_DRIVER_INIT(maclc3plus, MODEL_MAC_LC_III_PLUS)
MAC_DRIVER_INIT(maciici, MODEL_MAC_IICI)
MAC_DRIVER_INIT(maciisi, MODEL_MAC_IISI)
MAC_DRIVER_INIT(macii, MODEL_MAC_II)
MAC_DRIVER_INIT(macse30, MODEL_MAC_SE30)
MAC_DRIVER_INIT(macclassic2, MODEL_MAC_CLASSIC_II)
MAC_DRIVER_INIT(maclrcclassic, MODEL_MAC_COLOR_CLASSIC)
MAC_DRIVER_INIT(maciivx, MODEL_MAC_IIVX)
MAC_DRIVER_INIT(maciivi, MODEL_MAC_IIVI)
MAC_DRIVER_INIT(maciifx, MODEL_MAC_IIFX)
MAC_DRIVER_INIT(maciicx, MODEL_MAC_IICX)
MAC_DRIVER_INIT(maciifdhd, MODEL_MAC_II_FDHD)
MAC_DRIVER_INIT(maciix, MODEL_MAC_IIX)
MAC_DRIVER_INIT(maclc520, MODEL_MAC_LC_520)

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

	if (++irq_count == 60)
	{
		irq_count = 0;

		ca2_data ^= 1;
		/* signal 1 Hz irq on CA2 input on the VIA */
		m_via1->write_ca2(ca2_data);
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

/* *************************************************************************
 * Trap Tracing
 *
 * This is debug code that will output diagnostics regarding OS traps called
 * *************************************************************************/

const char *lookup_trap(uint16_t opcode)
{
	static const struct
	{
		uint16_t trap;
		const char *name;
	} traps[] =
	{
		{ 0xA000, "_Open" },
		{ 0xA001, "_Close" },
		{ 0xA002, "_Read" },
		{ 0xA003, "_Write" },
		{ 0xA004, "_Control" },
		{ 0xA005, "_Status" },
		{ 0xA006, "_KillIO" },
		{ 0xA007, "_GetVolInfo" },
		{ 0xA008, "_Create" },
		{ 0xA009, "_Delete" },
		{ 0xA00A, "_OpenRF" },
		{ 0xA00B, "_Rename" },
		{ 0xA00C, "_GetFileInfo" },
		{ 0xA00D, "_SetFileInfo" },
		{ 0xA00E, "_UnmountVol" },
		{ 0xA00F, "_MountVol" },
		{ 0xA010, "_Allocate" },
		{ 0xA011, "_GetEOF" },
		{ 0xA012, "_SetEOF" },
		{ 0xA013, "_FlushVol" },
		{ 0xA014, "_GetVol" },
		{ 0xA015, "_SetVol" },
		{ 0xA016, "_FInitQueue" },
		{ 0xA017, "_Eject" },
		{ 0xA018, "_GetFPos" },
		{ 0xA019, "_InitZone" },
		{ 0xA01B, "_SetZone" },
		{ 0xA01C, "_FreeMem" },
		{ 0xA01F, "_DisposePtr" },
		{ 0xA020, "_SetPtrSize" },
		{ 0xA021, "_GetPtrSize" },
		{ 0xA023, "_DisposeHandle" },
		{ 0xA024, "_SetHandleSize" },
		{ 0xA025, "_GetHandleSize" },
		{ 0xA027, "_ReallocHandle" },
		{ 0xA029, "_HLock" },
		{ 0xA02A, "_HUnlock" },
		{ 0xA02B, "_EmptyHandle" },
		{ 0xA02C, "_InitApplZone" },
		{ 0xA02D, "_SetApplLimit" },
		{ 0xA02E, "_BlockMove" },
		{ 0xA02F, "_PostEvent" },
		{ 0xA030, "_OSEventAvail" },
		{ 0xA031, "_GetOSEvent" },
		{ 0xA032, "_FlushEvents" },
		{ 0xA033, "_VInstall" },
		{ 0xA034, "_VRemove" },
		{ 0xA035, "_OffLine" },
		{ 0xA036, "_MoreMasters" },
		{ 0xA038, "_WriteParam" },
		{ 0xA039, "_ReadDateTime" },
		{ 0xA03A, "_SetDateTime" },
		{ 0xA03B, "_Delay" },
		{ 0xA03C, "_CmpString" },
		{ 0xA03D, "_DrvrInstall" },
		{ 0xA03E, "_DrvrRemove" },
		{ 0xA03F, "_InitUtil" },
		{ 0xA040, "_ResrvMem" },
		{ 0xA041, "_SetFilLock" },
		{ 0xA042, "_RstFilLock" },
		{ 0xA043, "_SetFilType" },
		{ 0xA044, "_SetFPos" },
		{ 0xA045, "_FlushFile" },
		{ 0xA047, "_SetTrapAddress" },
		{ 0xA049, "_HPurge" },
		{ 0xA04A, "_HNoPurge" },
		{ 0xA04B, "_SetGrowZone" },
		{ 0xA04C, "_CompactMem" },
		{ 0xA04D, "_PurgeMem" },
		{ 0xA04E, "_AddDrive" },
		{ 0xA04F, "_RDrvrInstall" },
		{ 0xA050, "_CompareString" },
		{ 0xA051, "_ReadXPRam" },
		{ 0xA052, "_WriteXPRam" },
		{ 0xA054, "_UprString" },
		{ 0xA055, "_StripAddress" },
		{ 0xA056, "_LowerText" },
		{ 0xA057, "_SetAppBase" },
		{ 0xA058, "_InsTime" },
		{ 0xA059, "_RmvTime" },
		{ 0xA05A, "_PrimeTime" },
		{ 0xA05B, "_PowerOff" },
		{ 0xA05C, "_MemoryDispatch" },
		{ 0xA05D, "_SwapMMUMode" },
		{ 0xA05E, "_NMInstall" },
		{ 0xA05F, "_NMRemove" },
		{ 0xA060, "_FSDispatch" },
		{ 0xA061, "_MaxBlock" },
		{ 0xA063, "_MaxApplZone" },
		{ 0xA064, "_MoveHHi" },
		{ 0xA065, "_StackSpace" },
		{ 0xA067, "_HSetRBit" },
		{ 0xA068, "_HClrRBit" },
		{ 0xA069, "_HGetState" },
		{ 0xA06A, "_HSetState" },
		{ 0xA06C, "_InitFS" },
		{ 0xA06D, "_InitEvents" },
		{ 0xA06E, "_SlotManager" },
		{ 0xA06F, "_SlotVInstall" },
		{ 0xA070, "_SlotVRemove" },
		{ 0xA071, "_AttachVBL" },
		{ 0xA072, "_DoVBLTask" },
		{ 0xA075, "_SIntInstall" },
		{ 0xA076, "_SIntRemove" },
		{ 0xA077, "_CountADBs" },
		{ 0xA078, "_GetIndADB" },
		{ 0xA079, "_GetADBInfo" },
		{ 0xA07A, "_SetADBInfo" },
		{ 0xA07B, "_ADBReInit" },
		{ 0xA07C, "_ADBOp" },
		{ 0xA07D, "_GetDefaultStartup" },
		{ 0xA07E, "_SetDefaultStartup" },
		{ 0xA07F, "_InternalWait" },
		{ 0xA080, "_GetVideoDefault" },
		{ 0xA081, "_SetVideoDefault" },
		{ 0xA082, "_DTInstall" },
		{ 0xA083, "_SetOSDefault" },
		{ 0xA084, "_GetOSDefault" },
		{ 0xA085, "_PMgrOp" },
		{ 0xA086, "_IOPInfoAccess" },
		{ 0xA087, "_IOPMsgRequest" },
		{ 0xA088, "_IOPMoveData" },
		{ 0xA089, "_SCSIAtomic" },
		{ 0xA08A, "_Sleep" },
		{ 0xA08B, "_CommToolboxDispatch" },
		{ 0xA08D, "_DebugUtil" },
		{ 0xA08F, "_DeferUserFn" },
		{ 0xA090, "_SysEnvirons" },
		{ 0xA092, "_EgretDispatch" },
		{ 0xA094, "_ServerDispatch" },
		{ 0xA09E, "_PowerMgrDispatch" },
		{ 0xA09F, "_PowerDispatch" },
		{ 0xA0A4, "_HeapDispatch" },
		{ 0xA0AC, "_FSMDispatch" },
		{ 0xA0AE, "_VADBProc" },
		{ 0xA0DD, "_PPC" },
		{ 0xA0FE, "_TEFindWord" },
		{ 0xA0FF, "_TEFindLine" },
		{ 0xA11A, "_GetZone" },
		{ 0xA11D, "_MaxMem" },
		{ 0xA11E, "_NewPtr" },
		{ 0xA122, "_NewHandle" },
		{ 0xA126, "_HandleZone" },
		{ 0xA128, "_RecoverHandle" },
		{ 0xA12F, "_PPostEvent" },
		{ 0xA146, "_GetTrapAddress" },
		{ 0xA148, "_PtrZone" },
		{ 0xA162, "_PurgeSpace" },
		{ 0xA166, "_NewEmptyHandle" },
		{ 0xA193, "_Microseconds" },
		{ 0xA198, "_HWPriv" },
		{ 0xA1AD, "_Gestalt" },
		{ 0xA200, "_HOpen" },
		{ 0xA207, "_HGetVInfo" },
		{ 0xA208, "_HCreate" },
		{ 0xA209, "_HDelete" },
		{ 0xA20A, "_HOpenRF" },
		{ 0xA20B, "_HRename" },
		{ 0xA20C, "_HGetFileInfo" },
		{ 0xA20D, "_HSetFileInfo" },
		{ 0xA20E, "_HUnmountVol" },
		{ 0xA210, "_AllocContig" },
		{ 0xA214, "_HGetVol" },
		{ 0xA215, "_HSetVol" },
		{ 0xA22E, "_BlockMoveData" },
		{ 0xA241, "_HSetFLock" },
		{ 0xA242, "_HRstFLock" },
		{ 0xA247, "_SetOSTrapAddress" },
		{ 0xA256, "_StripText" },
		{ 0xA260, "_HFSDispatch" },
		{ 0xA285, "_IdleUpdate" },
		{ 0xA28A, "_SlpQInstall" },
		{ 0xA31E, "_NewPtrClear" },
		{ 0xA322, "_NewHandleClear" },
		{ 0xA346, "_GetOSTrapAddress" },
		{ 0xA3AD, "_NewGestalt" },
		{ 0xA456, "_UpperText" },
		{ 0xA458, "_InsXTime" },
		{ 0xA485, "_IdleState" },
		{ 0xA48A, "_SlpQRemove" },
		{ 0xA51E, "_NewPtrSys" },
		{ 0xA522, "_NewHandleSys" },
		{ 0xA562, "_PurgeSpaceSys" },
		{ 0xA5AD, "_ReplaceGestalt" },
		{ 0xA647, "_SetToolBoxTrapAddress" },
		{ 0xA656, "_StripUpperText" },
		{ 0xA685, "_SerialPower" },
		{ 0xA71E, "_NewPtrSysClear" },
		{ 0xA722, "_NewHandleSysClear" },
		{ 0xA746, "_GetToolBoxTrapAddress" },
		{ 0xA7AD, "_GetGestaltProcPtr" },
		{ 0xA800, "_SoundDispatch" },
		{ 0xA801, "_SndDisposeChannel" },
		{ 0xA802, "_SndAddModifier" },
		{ 0xA803, "_SndDoCommand" },
		{ 0xA804, "_SndDoImmediate" },
		{ 0xA805, "_SndPlay" },
		{ 0xA806, "_SndControl" },
		{ 0xA807, "_SndNewChannel" },
		{ 0xA808, "_InitProcMenu" },
		{ 0xA809, "_GetControlVariant" },
		{ 0xA80A, "_GetWVariant" },
		{ 0xA80B, "_PopUpMenuSelect" },
		{ 0xA80C, "_RGetResource" },
		{ 0xA811, "_TESelView" },
		{ 0xA812, "_TEPinScroll" },
		{ 0xA813, "_TEAutoView" },
		{ 0xA814, "_SetFractEnable" },
		{ 0xA815, "_SCSIDispatch" },
		{ 0xA817, "_CopyMask" },
		{ 0xA819, "_XMunger" },
		{ 0xA81A, "_HOpenResFile" },
		{ 0xA81B, "_HCreateResFile" },
		{ 0xA81D, "_InvalMenuBar" },
		{ 0xA821, "_MaxSizeRsrc" },
		{ 0xA822, "_ResourceDispatch" },
		{ 0xA823, "_AliasDispatch" },
		{ 0xA824, "_HFSUtilDispatch" },
		{ 0xA825, "_MenuDispatch" },
		{ 0xA826, "_InsertMenuItem" },
		{ 0xA827, "_HideDialogItem" },
		{ 0xA828, "_ShowDialogItem" },
		{ 0xA82A, "_ComponentDispatch" },
		{ 0xA833, "_ScrnBitMap" },
		{ 0xA834, "_SetFScaleDisable" },
		{ 0xA835, "_FontMetrics" },
		{ 0xA836, "_GetMaskTable" },
		{ 0xA837, "_MeasureText" },
		{ 0xA838, "_CalcMask" },
		{ 0xA839, "_SeedFill" },
		{ 0xA83A, "_ZoomWindow" },
		{ 0xA83B, "_TrackBox" },
		{ 0xA83C, "_TEGetOffset" },
		{ 0xA83D, "_TEDispatch" },
		{ 0xA83E, "_TEStyleNew" },
		{ 0xA847, "_FracCos" },
		{ 0xA848, "_FracSin" },
		{ 0xA849, "_FracSqrt" },
		{ 0xA84A, "_FracMul" },
		{ 0xA84B, "_FracDiv" },
		{ 0xA84D, "_FixDiv" },
		{ 0xA84E, "_GetItemCmd" },
		{ 0xA84F, "_SetItemCmd" },
		{ 0xA850, "_InitCursor" },
		{ 0xA851, "_SetCursor" },
		{ 0xA852, "_HideCursor" },
		{ 0xA853, "_ShowCursor" },
		{ 0xA854, "_FontDispatch" },
		{ 0xA855, "_ShieldCursor" },
		{ 0xA856, "_ObscureCursor" },
		{ 0xA858, "_BitAnd" },
		{ 0xA859, "_BitXOr" },
		{ 0xA85A, "_BitNot" },
		{ 0xA85B, "_BitOr" },
		{ 0xA85C, "_BitShift" },
		{ 0xA85D, "_BitTst" },
		{ 0xA85E, "_BitSet" },
		{ 0xA85F, "_BitClr" },
		{ 0xA860, "_WaitNextEvent" },
		{ 0xA861, "_Random" },
		{ 0xA862, "_ForeColor" },
		{ 0xA863, "_BackColor" },
		{ 0xA864, "_ColorBit" },
		{ 0xA865, "_GetPixel" },
		{ 0xA866, "_StuffHex" },
		{ 0xA867, "_LongMul" },
		{ 0xA868, "_FixMul" },
		{ 0xA869, "_FixRatio" },
		{ 0xA86A, "_HiWord" },
		{ 0xA86B, "_LoWord" },
		{ 0xA86C, "_FixRound" },
		{ 0xA86D, "_InitPort" },
		{ 0xA86E, "_InitGraf" },
		{ 0xA86F, "_OpenPort" },
		{ 0xA870, "_LocalToGlobal" },
		{ 0xA871, "_GlobalToLocal" },
		{ 0xA872, "_GrafDevice" },
		{ 0xA873, "_SetPort" },
		{ 0xA874, "_GetPort" },
		{ 0xA875, "_SetPBits" },
		{ 0xA876, "_PortSize" },
		{ 0xA877, "_MovePortTo" },
		{ 0xA878, "_SetOrigin" },
		{ 0xA879, "_SetClip" },
		{ 0xA87A, "_GetClip" },
		{ 0xA87B, "_ClipRect" },
		{ 0xA87C, "_BackPat" },
		{ 0xA87D, "_ClosePort" },
		{ 0xA87E, "_AddPt" },
		{ 0xA87F, "_SubPt" },
		{ 0xA880, "_SetPt" },
		{ 0xA881, "_EqualPt" },
		{ 0xA882, "_StdText" },
		{ 0xA883, "_DrawChar" },
		{ 0xA884, "_DrawString" },
		{ 0xA885, "_DrawText" },
		{ 0xA886, "_TextWidth" },
		{ 0xA887, "_TextFont" },
		{ 0xA888, "_TextFace" },
		{ 0xA889, "_TextMode" },
		{ 0xA88A, "_TextSize" },
		{ 0xA88B, "_GetFontInfo" },
		{ 0xA88C, "_StringWidth" },
		{ 0xA88D, "_CharWidth" },
		{ 0xA88E, "_SpaceExtra" },
		{ 0xA88F, "_OSDispatch" },
		{ 0xA890, "_StdLine" },
		{ 0xA891, "_LineTo" },
		{ 0xA892, "_Line" },
		{ 0xA893, "_MoveTo" },
		{ 0xA894, "_Move" },
		{ 0xA895, "_ShutDown" },
		{ 0xA896, "_HidePen" },
		{ 0xA897, "_ShowPen" },
		{ 0xA898, "_GetPenState" },
		{ 0xA899, "_SetPenState" },
		{ 0xA89A, "_GetPen" },
		{ 0xA89B, "_PenSize" },
		{ 0xA89C, "_PenMode" },
		{ 0xA89D, "_PenPat" },
		{ 0xA89E, "_PenNormal" },
		{ 0xA89F, "_Moof" },
		{ 0xA8A0, "_StdRect" },
		{ 0xA8A1, "_FrameRect" },
		{ 0xA8A2, "_PaintRect" },
		{ 0xA8A3, "_EraseRect" },
		{ 0xA8A4, "_InverRect" },
		{ 0xA8A5, "_FillRect" },
		{ 0xA8A6, "_EqualRect" },
		{ 0xA8A7, "_SetRect" },
		{ 0xA8A8, "_OffsetRect" },
		{ 0xA8A9, "_InsetRect" },
		{ 0xA8AA, "_SectRect" },
		{ 0xA8AB, "_UnionRect" },
		{ 0xA8AD, "_PtInRect" },
		{ 0xA8AE, "_EmptyRect" },
		{ 0xA8AF, "_StdRRect" },
		{ 0xA8B0, "_FrameRoundRect" },
		{ 0xA8B1, "_PaintRoundRect" },
		{ 0xA8B2, "_EraseRoundRect" },
		{ 0xA8B3, "_InverRoundRect" },
		{ 0xA8B4, "_FillRoundRect" },
		{ 0xA8B5, "_ScriptUtil" },
		{ 0xA8B6, "_StdOval" },
		{ 0xA8B7, "_FrameOval" },
		{ 0xA8B8, "_PaintOval" },
		{ 0xA8B9, "_EraseOval" },
		{ 0xA8BA, "_InvertOval" },
		{ 0xA8BB, "_FillOval" },
		{ 0xA8BC, "_SlopeFromAngle" },
		{ 0xA8BD, "_StdArc" },
		{ 0xA8BE, "_FrameArc" },
		{ 0xA8BF, "_PaintArc" },
		{ 0xA8C0, "_EraseArc" },
		{ 0xA8C1, "_InvertArc" },
		{ 0xA8C2, "_FillArc" },
		{ 0xA8C3, "_PtToAngle" },
		{ 0xA8C4, "_AngleFromSlope" },
		{ 0xA8C5, "_StdPoly" },
		{ 0xA8C6, "_FramePoly" },
		{ 0xA8C7, "_PaintPoly" },
		{ 0xA8C8, "_ErasePoly" },
		{ 0xA8C9, "_InvertPoly" },
		{ 0xA8CA, "_FillPoly" },
		{ 0xA8CB, "_OpenPoly" },
		{ 0xA8CC, "_ClosePoly" },
		{ 0xA8CD, "_KillPoly" },
		{ 0xA8CE, "_OffsetPoly" },
		{ 0xA8CF, "_PackBits" },
		{ 0xA8D0, "_UnpackBits" },
		{ 0xA8D1, "_StdRgn" },
		{ 0xA8D2, "_FrameRgn" },
		{ 0xA8D3, "_PaintRgn" },
		{ 0xA8D4, "_EraseRgn" },
		{ 0xA8D5, "_InverRgn" },
		{ 0xA8D6, "_FillRgn" },
		{ 0xA8D7, "_BitMapToRegion" },
		{ 0xA8D8, "_NewRgn" },
		{ 0xA8D9, "_DisposeRgn" },
		{ 0xA8DA, "_OpenRgn" },
		{ 0xA8DB, "_CloseRgn" },
		{ 0xA8DC, "_CopyRgn" },
		{ 0xA8DD, "_SetEmptyRgn" },
		{ 0xA8DE, "_SetRecRgn" },
		{ 0xA8DF, "_RectRgn" },
		{ 0xA8E0, "_OffsetRgn" },
		{ 0xA8E1, "_InsetRgn" },
		{ 0xA8E2, "_EmptyRgn" },
		{ 0xA8E3, "_EqualRgn" },
		{ 0xA8E4, "_SectRgn" },
		{ 0xA8E5, "_UnionRgn" },
		{ 0xA8E6, "_DiffRgn" },
		{ 0xA8E7, "_XOrRgn" },
		{ 0xA8E8, "_PtInRgn" },
		{ 0xA8E9, "_RectInRgn" },
		{ 0xA8EA, "_SetStdProcs" },
		{ 0xA8EB, "_StdBits" },
		{ 0xA8EC, "_CopyBits" },
		{ 0xA8ED, "_StdTxMeas" },
		{ 0xA8EE, "_StdGetPic" },
		{ 0xA8EF, "_ScrollRect" },
		{ 0xA8F0, "_StdPutPic" },
		{ 0xA8F1, "_StdComment" },
		{ 0xA8F2, "_PicComment" },
		{ 0xA8F3, "_OpenPicture" },
		{ 0xA8F4, "_ClosePicture" },
		{ 0xA8F5, "_KillPicture" },
		{ 0xA8F6, "_DrawPicture" },
		{ 0xA8F7, "_Layout" },
		{ 0xA8F8, "_ScalePt" },
		{ 0xA8F9, "_MapPt" },
		{ 0xA8FA, "_MapRect" },
		{ 0xA8FB, "_MapRgn" },
		{ 0xA8FC, "_MapPoly" },
		{ 0xA8FD, "_PrGlue" },
		{ 0xA8FE, "_InitFonts" },
		{ 0xA8FF, "_GetFName" },
		{ 0xA900, "_GetFNum" },
		{ 0xA901, "_FMSwapFont" },
		{ 0xA902, "_RealFont" },
		{ 0xA903, "_SetFontLock" },
		{ 0xA904, "_DrawGrowIcon" },
		{ 0xA905, "_DragGrayRgn" },
		{ 0xA906, "_NewString" },
		{ 0xA907, "_SetString" },
		{ 0xA908, "_ShowHide" },
		{ 0xA909, "_CalcVis" },
		{ 0xA90A, "_CalcVBehind" },
		{ 0xA90B, "_ClipAbove" },
		{ 0xA90C, "_PaintOne" },
		{ 0xA90D, "_PaintBehind" },
		{ 0xA90E, "_SaveOld" },
		{ 0xA90F, "_DrawNew" },
		{ 0xA910, "_GetWMgrPort" },
		{ 0xA911, "_CheckUpDate" },
		{ 0xA912, "_InitWindows" },
		{ 0xA913, "_NewWindow" },
		{ 0xA914, "_DisposeWindow" },
		{ 0xA915, "_ShowWindow" },
		{ 0xA916, "_HideWindow" },
		{ 0xA917, "_GetWRefCon" },
		{ 0xA918, "_SetWRefCon" },
		{ 0xA919, "_GetWTitle" },
		{ 0xA91A, "_SetWTitle" },
		{ 0xA91B, "_MoveWindow" },
		{ 0xA91C, "_HiliteWindow" },
		{ 0xA91D, "_SizeWindow" },
		{ 0xA91E, "_TrackGoAway" },
		{ 0xA91F, "_SelectWindow" },
		{ 0xA920, "_BringToFront" },
		{ 0xA921, "_SendBehind" },
		{ 0xA922, "_BeginUpDate" },
		{ 0xA923, "_EndUpDate" },
		{ 0xA924, "_FrontWindow" },
		{ 0xA925, "_DragWindow" },
		{ 0xA926, "_DragTheRgn" },
		{ 0xA927, "_InvalRgn" },
		{ 0xA928, "_InvalRect" },
		{ 0xA929, "_ValidRgn" },
		{ 0xA92A, "_ValidRect" },
		{ 0xA92B, "_GrowWindow" },
		{ 0xA92C, "_FindWindow" },
		{ 0xA92D, "_CloseWindow" },
		{ 0xA92E, "_SetWindowPic" },
		{ 0xA92F, "_GetWindowPic" },
		{ 0xA930, "_InitMenus" },
		{ 0xA931, "_NewMenu" },
		{ 0xA932, "_DisposeMenu" },
		{ 0xA933, "_AppendMenu" },
		{ 0xA934, "_ClearMenuBar" },
		{ 0xA935, "_InsertMenu" },
		{ 0xA936, "_DeleteMenu" },
		{ 0xA937, "_DrawMenuBar" },
		{ 0xA938, "_HiliteMenu" },
		{ 0xA939, "_EnableItem" },
		{ 0xA93A, "_DisableItem" },
		{ 0xA93B, "_GetMenuBar" },
		{ 0xA93C, "_SetMenuBar" },
		{ 0xA93D, "_MenuSelect" },
		{ 0xA93E, "_MenuKey" },
		{ 0xA93F, "_GetItmIcon" },
		{ 0xA940, "_SetItmIcon" },
		{ 0xA941, "_GetItmStyle" },
		{ 0xA942, "_SetItmStyle" },
		{ 0xA943, "_GetItmMark" },
		{ 0xA944, "_SetItmMark" },
		{ 0xA945, "_CheckItem" },
		{ 0xA946, "_GetMenuItemText" },
		{ 0xA947, "_SetMenuItemText" },
		{ 0xA948, "_CalcMenuSize" },
		{ 0xA949, "_GetMenuHandle" },
		{ 0xA94A, "_SetMFlash" },
		{ 0xA94B, "_PlotIcon" },
		{ 0xA94C, "_FlashMenuBar" },
		{ 0xA94D, "_AppendResMenu" },
		{ 0xA94E, "_PinRect" },
		{ 0xA94F, "_DeltaPoint" },
		{ 0xA950, "_CountMItems" },
		{ 0xA951, "_InsertResMenu" },
		{ 0xA952, "_DeleteMenuItem" },
		{ 0xA953, "_UpdtControl" },
		{ 0xA954, "_NewControl" },
		{ 0xA955, "_DisposeControl" },
		{ 0xA956, "_KillControls" },
		{ 0xA957, "_ShowControl" },
		{ 0xA958, "_HideControl" },
		{ 0xA959, "_MoveControl" },
		{ 0xA95A, "_GetControlReference" },
		{ 0xA95B, "_SetControlReference" },
		{ 0xA95C, "_SizeControl" },
		{ 0xA95D, "_HiliteControl" },
		{ 0xA95E, "_GetControlTitle" },
		{ 0xA95F, "_SetControlTitle" },
		{ 0xA960, "_GetControlValue" },
		{ 0xA961, "_GetControlMinimum" },
		{ 0xA962, "_GetControlMaximum" },
		{ 0xA963, "_SetControlValue" },
		{ 0xA964, "_SetControlMinimum" },
		{ 0xA965, "_SetControlMaximum" },
		{ 0xA966, "_TestControl" },
		{ 0xA967, "_DragControl" },
		{ 0xA968, "_TrackControl" },
		{ 0xA969, "_DrawControls" },
		{ 0xA96A, "_GetControlAction" },
		{ 0xA96B, "_SetControlAction" },
		{ 0xA96C, "_FindControl" },
		{ 0xA96E, "_Dequeue" },
		{ 0xA96F, "_Enqueue" },
		{ 0xA970, "_GetNextEvent" },
		{ 0xA971, "_EventAvail" },
		{ 0xA972, "_GetMouse" },
		{ 0xA973, "_StillDown" },
		{ 0xA974, "_Button" },
		{ 0xA975, "_TickCount" },
		{ 0xA976, "_GetKeys" },
		{ 0xA977, "_WaitMouseUp" },
		{ 0xA978, "_UpdtDialog" },
		{ 0xA97B, "_InitDialogs" },
		{ 0xA97C, "_GetNewDialog" },
		{ 0xA97D, "_NewDialog" },
		{ 0xA97E, "_SelectDialogItemText" },
		{ 0xA97F, "_IsDialogEvent" },
		{ 0xA980, "_DialogSelect" },
		{ 0xA981, "_DrawDialog" },
		{ 0xA982, "_CloseDialog" },
		{ 0xA983, "_DisposeDialog" },
		{ 0xA984, "_FindDialogItem" },
		{ 0xA985, "_Alert" },
		{ 0xA986, "_StopAlert" },
		{ 0xA987, "_NoteAlert" },
		{ 0xA988, "_CautionAlert" },
		{ 0xA98B, "_ParamText" },
		{ 0xA98C, "_ErrorSound" },
		{ 0xA98D, "_GetDialogItem" },
		{ 0xA98E, "_SetDialogItem" },
		{ 0xA98F, "_SetDialogItemText" },
		{ 0xA990, "_GetDialogItemText" },
		{ 0xA991, "_ModalDialog" },
		{ 0xA992, "_DetachResource" },
		{ 0xA993, "_SetResPurge" },
		{ 0xA994, "_CurResFile" },
		{ 0xA995, "_InitResources" },
		{ 0xA996, "_RsrcZoneInit" },
		{ 0xA997, "_OpenResFile" },
		{ 0xA998, "_UseResFile" },
		{ 0xA999, "_UpdateResFile" },
		{ 0xA99A, "_CloseResFile" },
		{ 0xA99B, "_SetResLoad" },
		{ 0xA99C, "_CountResources" },
		{ 0xA99D, "_GetIndResource" },
		{ 0xA99E, "_CountTypes" },
		{ 0xA99F, "_GetIndType" },
		{ 0xA9A0, "_GetResource" },
		{ 0xA9A1, "_GetNamedResource" },
		{ 0xA9A2, "_LoadResource" },
		{ 0xA9A3, "_ReleaseResource" },
		{ 0xA9A4, "_HomeResFile" },
		{ 0xA9A5, "_SizeRsrc" },
		{ 0xA9A6, "_GetResAttrs" },
		{ 0xA9A7, "_SetResAttrs" },
		{ 0xA9A8, "_GetResInfo" },
		{ 0xA9A9, "_SetResInfo" },
		{ 0xA9AA, "_ChangedResource" },
		{ 0xA9AB, "_AddResource" },
		{ 0xA9AC, "_AddReference" },
		{ 0xA9AD, "_RmveResource" },
		{ 0xA9AE, "_RmveReference" },
		{ 0xA9AF, "_ResError" },
		{ 0xA9B0, "_WriteResource" },
		{ 0xA9B1, "_CreateResFile" },
		{ 0xA9B2, "_SystemEvent" },
		{ 0xA9B3, "_SystemClick" },
		{ 0xA9B4, "_SystemTask" },
		{ 0xA9B5, "_SystemMenu" },
		{ 0xA9B6, "_OpenDeskAcc" },
		{ 0xA9B7, "_CloseDeskAcc" },
		{ 0xA9B8, "_GetPattern" },
		{ 0xA9B9, "_GetCursor" },
		{ 0xA9BA, "_GetString" },
		{ 0xA9BB, "_GetIcon" },
		{ 0xA9BC, "_GetPicture" },
		{ 0xA9BD, "_GetNewWindow" },
		{ 0xA9BE, "_GetNewControl" },
		{ 0xA9BF, "_GetRMenu" },
		{ 0xA9C0, "_GetNewMBar" },
		{ 0xA9C1, "_UniqueID" },
		{ 0xA9C2, "_SysEdit" },
		{ 0xA9C3, "_KeyTranslate" },
		{ 0xA9C4, "_OpenRFPerm" },
		{ 0xA9C5, "_RsrcMapEntry" },
		{ 0xA9C6, "_SecondsToDate" },
		{ 0xA9C7, "_DateToSeconds" },
		{ 0xA9C8, "_SysBeep" },
		{ 0xA9C9, "_SysError" },
		{ 0xA9CA, "_PutIcon" },
		{ 0xA9CB, "_TEGetText" },
		{ 0xA9CC, "_TEInit" },
		{ 0xA9CD, "_TEDispose" },
		{ 0xA9CE, "_TETextBox" },
		{ 0xA9CF, "_TESetText" },
		{ 0xA9D0, "_TECalText" },
		{ 0xA9D1, "_TESetSelect" },
		{ 0xA9D2, "_TENew" },
		{ 0xA9D3, "_TEUpdate" },
		{ 0xA9D4, "_TEClick" },
		{ 0xA9D5, "_TECopy" },
		{ 0xA9D6, "_TECut" },
		{ 0xA9D7, "_TEDelete" },
		{ 0xA9D8, "_TEActivate" },
		{ 0xA9D9, "_TEDeactivate" },
		{ 0xA9DA, "_TEIdle" },
		{ 0xA9DB, "_TEPaste" },
		{ 0xA9DC, "_TEKey" },
		{ 0xA9DD, "_TEScroll" },
		{ 0xA9DE, "_TEInsert" },
		{ 0xA9DF, "_TESetAlignment" },
		{ 0xA9E0, "_Munger" },
		{ 0xA9E1, "_HandToHand" },
		{ 0xA9E2, "_PtrToXHand" },
		{ 0xA9E3, "_PtrToHand" },
		{ 0xA9E4, "_HandAndHand" },
		{ 0xA9E5, "_InitPack" },
		{ 0xA9E6, "_InitAllPacks" },
		{ 0xA9EF, "_PtrAndHand" },
		{ 0xA9F0, "_LoadSeg" },
		{ 0xA9F1, "_UnLoadSeg" },
		{ 0xA9F2, "_Launch" },
		{ 0xA9F3, "_Chain" },
		{ 0xA9F4, "_ExitToShell" },
		{ 0xA9F5, "_GetAppParms" },
		{ 0xA9F6, "_GetResFileAttrs" },
		{ 0xA9F7, "_SetResFileAttrs" },
		{ 0xA9F8, "_MethodDispatch" },
		{ 0xA9F9, "_InfoScrap" },
		{ 0xA9FA, "_UnloadScrap" },
		{ 0xA9FB, "_LoadScrap" },
		{ 0xA9FC, "_ZeroScrap" },
		{ 0xA9FD, "_GetScrap" },
		{ 0xA9FE, "_PutScrap" },
		{ 0xA9FF, "_Debugger" },
		{ 0xAA00, "_OpenCPort" },
		{ 0xAA01, "_InitCPort" },
		{ 0xAA02, "_CloseCPort" },
		{ 0xAA03, "_NewPixMap" },
		{ 0xAA04, "_DisposePixMap" },
		{ 0xAA05, "_CopyPixMap" },
		{ 0xAA06, "_SetPortPix" },
		{ 0xAA07, "_NewPixPat" },
		{ 0xAA08, "_DisposePixPat" },
		{ 0xAA09, "_CopyPixPat" },
		{ 0xAA0A, "_PenPixPat" },
		{ 0xAA0B, "_BackPixPat" },
		{ 0xAA0C, "_GetPixPat" },
		{ 0xAA0D, "_MakeRGBPat" },
		{ 0xAA0E, "_FillCRect" },
		{ 0xAA0F, "_FillCOval" },
		{ 0xAA10, "_FillCRoundRect" },
		{ 0xAA11, "_FillCArc" },
		{ 0xAA12, "_FillCRgn" },
		{ 0xAA13, "_FillCPoly" },
		{ 0xAA14, "_RGBForeColor" },
		{ 0xAA15, "_RGBBackColor" },
		{ 0xAA16, "_SetCPixel" },
		{ 0xAA17, "_GetCPixel" },
		{ 0xAA18, "_GetCTable" },
		{ 0xAA19, "_GetForeColor" },
		{ 0xAA1A, "_GetBackColor" },
		{ 0xAA1B, "_GetCCursor" },
		{ 0xAA1C, "_SetCCursor" },
		{ 0xAA1D, "_AllocCursor" },
		{ 0xAA1E, "_GetCIcon" },
		{ 0xAA1F, "_PlotCIcon" },
		{ 0xAA20, "_OpenCPicture" },
		{ 0xAA21, "_OpColor" },
		{ 0xAA22, "_HiliteColor" },
		{ 0xAA23, "_CharExtra" },
		{ 0xAA24, "_DisposeCTable" },
		{ 0xAA25, "_DisposeCIcon" },
		{ 0xAA26, "_DisposeCCursor" },
		{ 0xAA27, "_GetMaxDevice" },
		{ 0xAA28, "_GetCTSeed" },
		{ 0xAA29, "_GetDeviceList" },
		{ 0xAA2A, "_GetMainDevice" },
		{ 0xAA2B, "_GetNextDevice" },
		{ 0xAA2C, "_TestDeviceAttribute" },
		{ 0xAA2D, "_SetDeviceAttribute" },
		{ 0xAA2E, "_InitGDevice" },
		{ 0xAA2F, "_NewGDevice" },
		{ 0xAA30, "_DisposeGDevice" },
		{ 0xAA31, "_SetGDevice" },
		{ 0xAA32, "_GetGDevice" },
		{ 0xAA35, "_InvertColor" },
		{ 0xAA36, "_RealColor" },
		{ 0xAA37, "_GetSubTable" },
		{ 0xAA38, "_UpdatePixMap" },
		{ 0xAA39, "_MakeITable" },
		{ 0xAA3A, "_AddSearch" },
		{ 0xAA3B, "_AddComp" },
		{ 0xAA3C, "_SetClientID" },
		{ 0xAA3D, "_ProtectEntry" },
		{ 0xAA3E, "_ReserveEntry" },
		{ 0xAA3F, "_SetEntries" },
		{ 0xAA40, "_QDError" },
		{ 0xAA41, "_SetWinColor" },
		{ 0xAA42, "_GetAuxWin" },
		{ 0xAA43, "_SetControlColor" },
		{ 0xAA44, "_GetAuxiliaryControlRecord" },
		{ 0xAA45, "_NewCWindow" },
		{ 0xAA46, "_GetNewCWindow" },
		{ 0xAA47, "_SetDeskCPat" },
		{ 0xAA48, "_GetCWMgrPort" },
		{ 0xAA49, "_SaveEntries" },
		{ 0xAA4A, "_RestoreEntries" },
		{ 0xAA4B, "_NewColorDialog" },
		{ 0xAA4C, "_DelSearch" },
		{ 0xAA4D, "_DelComp" },
		{ 0xAA4E, "_SetStdCProcs" },
		{ 0xAA4F, "_CalcCMask" },
		{ 0xAA50, "_SeedCFill" },
		{ 0xAA51, "_CopyDeepMask" },
		{ 0xAA52, "_HFSPinaforeDispatch" },
		{ 0xAA53, "_DictionaryDispatch" },
		{ 0xAA54, "_TextServicesDispatch" },
		{ 0xAA56, "_SpeechRecognitionDispatch" },
		{ 0xAA57, "_DockingDispatch" },
		{ 0xAA59, "_MixedModeDispatch" },
		{ 0xAA5A, "_CodeFragmentDispatch" },
		{ 0xAA5C, "_OCEUtils" },
		{ 0xAA5D, "_DigitalSignature" },
		{ 0xAA5E, "_TBDispatch" },
		{ 0xAA60, "_DeleteMCEntries" },
		{ 0xAA61, "_GetMCInfo" },
		{ 0xAA62, "_SetMCInfo" },
		{ 0xAA63, "_DisposeMCInfo" },
		{ 0xAA64, "_GetMCEntry" },
		{ 0xAA65, "_SetMCEntries" },
		{ 0xAA66, "_MenuChoice" },
		{ 0xAA67, "_ModalDialogMenuSetup" },
		{ 0xAA68, "_DialogDispatch" },
		{ 0xAA73, "_ControlDispatch" },
		{ 0xAA74, "_AppearanceDispatch" },
		{ 0xAA7E, "_SysDebugDispatch" },
		{ 0xAA80, "_AVLTreeDispatch" },
		{ 0xAA81, "_FileMappingDispatch" },
		{ 0xAA90, "_InitPalettes" },
		{ 0xAA91, "_NewPalette" },
		{ 0xAA92, "_GetNewPalette" },
		{ 0xAA93, "_DisposePalette" },
		{ 0xAA94, "_ActivatePalette" },
		{ 0xAA95, "_NSetPalette" },
		{ 0xAA96, "_GetPalette" },
		{ 0xAA97, "_PmForeColor" },
		{ 0xAA98, "_PmBackColor" },
		{ 0xAA99, "_AnimateEntry" },
		{ 0xAA9A, "_AnimatePalette" },
		{ 0xAA9B, "_GetEntryColor" },
		{ 0xAA9C, "_SetEntryColor" },
		{ 0xAA9D, "_GetEntryUsage" },
		{ 0xAA9E, "_SetEntryUsage" },
		{ 0xAAA1, "_CopyPalette" },
		{ 0xAAA2, "_PaletteDispatch" },
		{ 0xAAA3, "_CodecDispatch" },
		{ 0xAAA4, "_ALMDispatch" },
		{ 0xAADB, "_CursorDeviceDispatch" },
		{ 0xAAF2, "_ControlStripDispatch" },
		{ 0xAAF3, "_ExpansionManager" },
		{ 0xAB1D, "_QDExtensions" },
		{ 0xABC3, "_NQDMisc" },
		{ 0xABC9, "_IconDispatch" },
		{ 0xABCA, "_DeviceLoop" },
		{ 0xABEB, "_DisplayDispatch" },
		{ 0xABED, "_DragDispatch" },
		{ 0xABF1, "_GestaltValueDispatch" },
		{ 0xABF2, "_ThreadDispatch" },
		{ 0xABF6, "_CollectionMgr" },
		{ 0xABF8, "_StdOpcodeProc" },
		{ 0xABFC, "_TranslationDispatch" },
		{ 0xABFF, "_DebugStr" }
	};

	int i;

	for (i = 0; i < std::size(traps); i++)
	{
		if (traps[i].trap == opcode)
			return traps[i].name;
	}
	return nullptr;
}



offs_t mac_state::mac_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	uint16_t opcode;
	unsigned result = 0;
	const char *trap;

	opcode = opcodes.r16(pc);
	if ((opcode & 0xF000) == 0xA000)
	{
		trap = lookup_trap(opcode);
		if (trap != nullptr)
		{
			stream << trap;
			result = 2 | util::disasm_interface::SUPPORTED;
		}
	}
	return result;
}



#ifdef MAC_TRACETRAP
void mac_state::mac_tracetrap(const char *cpu_name_local, int addr, int trap)
{
	struct sonycscodeentry
	{
		int csCode;
		const char *name;
	};

	static const sonycscodeentry cscodes[] =
	{
		{ 1, "KillIO" },
		{ 5, "VerifyDisk" },
		{ 6, "FormatDisk" },
		{ 7, "EjectDisk" },
		{ 8, "SetTagBuffer" },
		{ 9, "TrackCacheControl" },
		{ 23, "ReturnDriveInfo" }
	};

	static const char *const scsisels[] =
	{
		"SCSIReset",    /* $00 */
		"SCSIGet",      /* $01 */
		"SCSISelect",   /* $02 */
		"SCSICmd",      /* $03 */
		"SCSIComplete", /* $04 */
		"SCSIRead",     /* $05 */
		"SCSIWrite",    /* $06 */
		nullptr,           /* $07 */
		"SCSIRBlind",   /* $08 */
		"SCSIWBlind",   /* $09 */
		"SCSIStat",     /* $0A */
		"SCSISelAtn",   /* $0B */
		"SCSIMsgIn",    /* $0C */
		"SCSIMsgOut",   /* $0D */
	};

	int i, a0, a7, d0, d1;
	int csCode, ioVRefNum, ioRefNum, ioCRefNum, ioCompletion, ioBuffer, ioReqCount, ioPosOffset;
	char *s;
	unsigned char *mem;
	char buf[256];
	const char *trapstr;

	trapstr = lookup_trap(trap);
	if (trapstr)
		strcpy(buf, trapstr);
	else
		sprintf(buf, "Trap $%04x", trap);

	s = &buf[strlen(buf)];
	mem = mac_ram_ptr;
	a0 = M68K_A0);
	a7 = cpu_get_reg(M68K_A7);
	d0 = cpu_get_reg(M68K_D0);
	d1 = cpu_get_reg(M68K_D1);

	switch(trap)
	{
	case 0xa004:    /* _Control */
		ioVRefNum = *((int16_t*) (mem + a0 + 22));
		ioCRefNum = *((int16_t*) (mem + a0 + 24));
		csCode = *((uint16_t*) (mem + a0 + 26));
		sprintf(s->state().state_int(" ioVRefNum=%i ioCRefNum=%i csCode=%i", ioVRefNum, ioCRefNum, csCode);

		for (i = 0; i < std::size(cscodes); i++)
		{
			if (cscodes[i].csCode == csCode)
			{
				strcat(s, "=");
				strcat(s, cscodes[i].name);
				break;
			}
		}
		break;

	case 0xa002:    /* _Read */
		ioCompletion = (*((int16_t*) (mem + a0 + 12)) << 16) + *((int16_t*) (mem + a0 + 14));
		ioVRefNum = *((int16_t*) (mem + a0 + 22));
		ioRefNum = *((int16_t*) (mem + a0 + 24));
		ioBuffer = (*((int16_t*) (mem + a0 + 32)) << 16) + *((int16_t*) (mem + a0 + 34));
		ioReqCount = (*((int16_t*) (mem + a0 + 36)) << 16) + *((int16_t*) (mem + a0 + 38));
		ioPosOffset = (*((int16_t*) (mem + a0 + 46)) << 16) + *((int16_t*) (mem + a0 + 48));
		sprintf(s, " ioCompletion=0x%08x ioVRefNum=%i ioRefNum=%i ioBuffer=0x%08x ioReqCount=%i ioPosOffset=%i",
			ioCompletion, ioVRefNum, ioRefNum, ioBuffer, ioReqCount, ioPosOffset);
		break;

	case 0xa04e:    /* _AddDrive */
		sprintf(s, " drvrRefNum=%i drvNum=%i qEl=0x%08x", (int) (int16_t) d0, (int) (int16_t) d1, a0);
		break;

	case 0xa9a0:    /* _GetResource */
		/* HACKHACK - the 'type' output assumes that the host is little endian
		 * since this is just trace code it isn't much of an issue
		 */
		sprintf(s, " type='%c%c%c%c' id=%i", (char) mem[a7+3], (char) mem[a7+2],
			(char) mem[a7+5], (char) mem[a7+4], *((int16_t*) (mem + a7)));
		break;

	case 0xa815:    /* _SCSIDispatch */
		i = *((uint16_t*) (mem + a7));
		if (i < std::size(scsisels))
			if (scsisels[i])
				sprintf(s, " (%s)", scsisels[i]);
		break;
	}

	logerror("mac_trace_trap: %s at 0x%08x: %s\n",cpu_name_local, addr, buf);
}
#endif

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
