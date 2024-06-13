// license:BSD-3-Clause
// copyright-holders: Nathan Woods, Raphael Nabet, R. Belmont, O. Galibert
/****************************************************************************

    mac.cpp
    Macintosh II ("Becks, Cabernet, Ikki, Little Big Mac, Paris, Reno, Uzi, Milwaukee")
    Macintosh II FDHD (Codenames not known)
    Macintosh IIx ("Spock, Stratos")
    Macintosh IIcx ("Atlantic, Cobra, Aurora")
    Macintosh SE/30 ("Fafnir, Green Jade")

    Driver by R. Belmont and O. Galibert, based on work by Nathan Woods, Ernesto Corvi, and Raphael Nabet

****************************************************************************/

#include "emu.h"

#include "adbmodem.h"
#include "macadb.h"
#include "macrtc.h"
#include "macscsi.h"
#include "mactoolbox.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/macpds/macpds.h"
#include "bus/nubus/nubus.h"
#include "bus/nubus/cards.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68020.h"
#include "cpu/m68000/m68030.h"
#include "cpu/m68000/m68kmusashi.h"
#include "machine/6522via.h"
#include "machine/applefdintf.h"
#include "machine/iwm.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/ram.h"
#include "machine/swim1.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "sound/asc.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/ap_dsk35.h"


namespace {

#define C15M (15.6672_MHz_XTAL)
#define C7M  (C15M/2)

// video parameters for classic Macs
#define MAC_H_VIS   (512)
#define MAC_V_VIS   (342)
#define MAC_H_TOTAL (704)  // (512+192)
#define MAC_V_TOTAL (370) // (342+28)

// Mac driver data

class mac_state:public driver_device
{
public:
	mac_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via1(*this, "via6522_0"),
		m_via2(*this, "via6522_1"),
		m_asc(*this, "asc"),
		m_adbmodem(*this, "adbmodem"),
		m_macadb(*this, "macadb"),
		m_ram(*this, RAM_TAG),
		m_scc(*this, "scc"),
		m_ncr5380(*this, "scsi:7:ncr5380"),
		m_scsihelp(*this, "scsihelp"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_rtc(*this, "rtc"),
		m_vram(*this,"vram"),
		m_via2_ca1_hack(0),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{
		m_rom_size = 0;
		m_rom_ptr = nullptr;
		m_cur_floppy = nullptr;
	}

	void macii(machine_config &config);
	void maciihmu(machine_config &config);
	void maciihd(machine_config &config);
	void maciix(machine_config &config);
	void maciicx(machine_config &config);
	void macse30(machine_config &config);

	void init_maciifdhd();
	void init_macse30();
	void init_macii();
	void init_maciix();
	void init_maciicx();

	/* tells which model is being emulated (set by macxxx_init) */
	enum model_t
	{
		MODEL_MAC_II,       // Mac II class 68020/030 machines
		MODEL_MAC_II_FDHD,
		MODEL_MAC_IIX,
		MODEL_MAC_IICX,
		MODEL_MAC_SE30
	};

	model_t m_model;

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<via6522_device> m_via2;
	required_device<asc_device> m_asc;
	optional_device<adbmodem_device> m_adbmodem;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;
	required_device<z80scc_device> m_scc;
	required_device<ncr53c80_device> m_ncr5380;
	required_device<mac_scsi_helper_device> m_scsihelp;
	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<rtc3430042_device> m_rtc;

	uint32_t m_overlay = 0;

	uint32_t m_via2_vbl = 0;
	uint32_t m_se30_vbl_enable = 0;
	uint8_t m_nubus_irq_state = 0;

	emu_timer *m_overlay_timeout = nullptr;
	TIMER_CALLBACK_MEMBER(overlay_timeout_func);

	int m_adb_irq_pending = 0;
	int m_screen_buffer = 0;
	int ca1_data = 0;

	// ADB refresh timer, independent of anything else going on
	emu_timer *m_adbupdate_timer = nullptr;

	void adb_irq_w(int state) { m_adb_irq_pending = state; }

	// this is shared among all video setups with vram
	optional_shared_ptr<uint32_t> m_vram;

	// interrupts
	int m_scc_interrupt = false, m_via_interrupt = false, m_via2_interrupt = false, m_scsi_interrupt = false, m_last_taken_interrupt = false;

	// defined in machine/mac.c
	void set_memory_overlay(int overlay);
	void scc_mouse_irq( int x, int y );
	void nubus_slot_interrupt(uint8_t slot, uint32_t state);
	void set_scc_interrupt(int state);
	void set_via_interrupt(int value);
	void set_via2_interrupt(int value);
	void field_interrupts();
	void vblank_irq();
	void update_volume();

	void mac_via_sync();
	uint16_t mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mac_via2_r(offs_t offset);
	void mac_via2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mac_iwm_r(offs_t offset, uint16_t mem_mask = ~0);
	void mac_iwm_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	u16 mac_scc_r(offs_t offset);
	void mac_scc_w(offs_t offset, u16 data);
	uint16_t macplus_scsi_r(offs_t offset, uint16_t mem_mask = ~0);
	void macii_scsi_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t macii_scsi_drq_r(offs_t offset, uint32_t mem_mask = ~0);
	void macii_scsi_drq_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void scsi_berr_w(uint8_t data);

	template <int Slot> void nubus_irq_w(int state);

	void mac_scsi_irq(int state);
	void mac_asc_irq(int state);

	void macii_map(address_map &map);
	void macse30_map(address_map &map);

	int m_via2_ca1_hack = 0;
	optional_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;

	uint32_t m_rom_size = 0;
	uint32_t *m_rom_ptr = nullptr;

	emu_timer *m_scanline_timer = nullptr;

	floppy_image_device *m_cur_floppy = nullptr;

	void phases_w(uint8_t phases);
	void devsel_w(uint8_t devsel);

	uint32_t screen_update_macse30(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(mac_adbrefresh_tick);
	TIMER_CALLBACK_MEMBER(mac_scanline_tick);
	uint8_t mac_via_in_a();
	uint8_t mac_via_in_b();
	void mac_via_out_a(uint8_t data);
	void mac_via_out_b(uint8_t data);
	uint8_t mac_via2_in_a();
	uint8_t mac_via2_in_b();
	void mac_via2_out_a(uint8_t data);
	void mac_via2_out_b(uint8_t data);
	void mac_state_load();
	void mac_via_irq(int state);
	void mac_via2_irq(int state);
	void mac_driver_init(model_t model);
	void mac_install_memory(offs_t memory_begin, offs_t memory_end, offs_t memory_size, void *memory_data, int is_rom);
};

uint32_t mac_state::screen_update_macse30(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint32_t const video_base = (m_screen_buffer ? 0x8000 : 0) + (MAC_H_VIS / 8);
	uint16_t const *const video_ram = (const uint16_t *)&m_vram[video_base / 4];

	for (int y = 0; y < MAC_V_VIS; y++)
	{
		uint16_t *const line = &bitmap.pix(y);

		for (int x = 0; x < MAC_H_VIS; x += 16)
		{
			uint16_t const word = video_ram[((y * MAC_H_VIS) / 16) + ((x / 16) ^ 1)];
			for (int b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}
	return 0;
}

void mac_state::mac_install_memory(offs_t memory_begin, offs_t memory_end,
								   offs_t memory_size, void *memory_data, int is_rom)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
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
}

/*
    Interrupt handling
*/

void mac_state::field_interrupts()
{
	int take_interrupt = -1;

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

void mac_state::set_scc_interrupt(int state)
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

void mac_state::mac_asc_irq(int state)
{
	m_via2->write_cb1(state ^ 1);
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
		mac_install_memory(0x00000000, 0x3fffffff, memory_size, memory_data, is_rom);

		m_overlay = overlay;
	}
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
	int reg = (offset >> 3) & 0xf;

	//  logerror("macplus_scsi_r: offset %x mask %x\n", offset, mem_mask);

	bool pseudo_dma = (reg == 6) && (offset == 0x130);

	return m_scsihelp->read_wrapper(pseudo_dma, reg) << 8;
}

uint32_t mac_state::macii_scsi_drq_r(offs_t offset, uint32_t mem_mask)
{
	switch (mem_mask)
	{
	case 0xff000000:
		return m_scsihelp->read_wrapper(true, 6) << 24;

	case 0xffff0000:
		return (m_scsihelp->read_wrapper(true, 6) << 24) | (m_scsihelp->read_wrapper(true, 6) << 16);

	case 0xffffffff:
		return (m_scsihelp->read_wrapper(true, 6) << 24) | (m_scsihelp->read_wrapper(true, 6) << 16) | (m_scsihelp->read_wrapper(true, 6) << 8) | m_scsihelp->read_wrapper(true, 6);

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
		m_scsihelp->write_wrapper(true, 0, data >> 24);
		break;

	case 0xffff0000:
		m_scsihelp->write_wrapper(true, 0, data >> 24);
		m_scsihelp->write_wrapper(true, 0, data >> 16);
		break;

	case 0xffffffff:
		m_scsihelp->write_wrapper(true, 0, data >> 24);
		m_scsihelp->write_wrapper(true, 0, data >> 16);
		m_scsihelp->write_wrapper(true, 0, data >> 8);
		m_scsihelp->write_wrapper(true, 0, data & 0xff);
		break;

	default:
		logerror("macii_scsi_drq_w: unknown mem_mask %08x\n", mem_mask);
		break;
	}
}

void mac_state::macii_scsi_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int reg = (offset >> 3) & 0xf;

	//  logerror("macplus_scsi_w: data %x offset %x mask %x (PC=%x)\n", data, offset, mem_mask, m_maincpu->pc());

	bool pseudo_dma = (reg == 0) && (offset == 0x100);

	m_scsihelp->write_wrapper(pseudo_dma, reg, data >> 8);
}

void mac_state::mac_scsi_irq(int state)
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
u16 mac_state::mac_scc_r(offs_t offset)
{
	u16 result = m_scc->dc_ab_r(offset);
	return (result << 8) | result;
}

void mac_state::mac_scc_w(offs_t offset, u16 data)
{
	m_scc->dc_ab_w(offset, data >> 8);
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

	return (result << 8) | result;
}

void mac_state::mac_iwm_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_fdc->write((offset >> 8), data & 0xff);
	else
		m_fdc->write((offset >> 8), data >> 8);

	if (!machine().side_effects_disabled())
		m_maincpu->adjust_icount(-5);
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
		return 0x81; // bit 0 must be set on most Macs to avoid attempting to boot from AppleTalk

	case MODEL_MAC_SE30:
		return 0x81 | PA6;

	case MODEL_MAC_IICX:
		return 0x81 | PA6;

	default:
		return 0x80;
	}
}

uint8_t mac_state::mac_via_in_b()
{
	int val = 0;

	if (!m_adb_irq_pending)
	{
		val |= 0x08;
	}

	val |= m_rtc->data_r();

	//  printf("%s VIA1 IN_B_II = %02x\n", machine().describe_context().c_str(), val);

	return val;
}

void mac_state::mac_via_out_a(uint8_t data)
{
	//  printf("%s VIA1 OUT A: %02x\n", machine().describe_context().c_str(), data);

	m_screen_buffer = (data & 0x40) >> 6;
	if (m_cur_floppy)
		m_cur_floppy->ss_w((data & 0x20) >> 5);
}

void mac_state::mac_via_out_b(uint8_t data)
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

	m_adbmodem->set_via_state((data & 0x30) >> 4);

	m_rtc->ce_w((data & 0x04) >> 2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

void mac_state::mac_via_irq(int state)
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
	u64 main_cycle = (via_cycle * 2 + 3) * m_maincpu->clock() / (2 * m_via1->clock()) + 1;

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

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void mac_state::mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	mac_via_sync();

	offset >>= 8;
	offset &= 0x0f;

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

/* *************************************************************************
 * VIA 2 (on Mac IIs, PowerBooks > 100, and PowerMacs)
 * *************************************************************************/

void mac_state::mac_via2_irq(int state)
{
	set_via2_interrupt(state);
}

uint16_t mac_state::mac_via2_r(offs_t offset)
{
	int data;

	if (!machine().side_effects_disabled())
		mac_via_sync();

	offset >>= 8;
	offset &= 0x0f;

	data = m_via2->read(offset);

	return (data & 0xff) | (data << 8);
}

void mac_state::mac_via2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	mac_via_sync();

	offset >>= 8;
	offset &= 0x0f;

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
	if ((m_model == MODEL_MAC_SE30) || (m_model == MODEL_MAC_IIX))
	{
		return 0x87; // bits 3 and 6 are tied low on SE/30
	}

	return 0xcf; // indicate no NuBus transaction error
}

void mac_state::mac_via2_out_a(uint8_t data)
{
}

void mac_state::mac_via2_out_b(uint8_t data)
{
	// chain 60.15 Hz to VIA1
	m_via1->write_ca1(data >> 7);

	if (m_model == MODEL_MAC_II)
	{
		m68000_musashi_device *m68k = downcast<m68000_musashi_device *>(m_maincpu.target());
		m68k->set_hmmu_enable((data & 0x8) ? M68K_HMMU_DISABLE : M68K_HMMU_ENABLE_II);
	}
}

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

	save_item(NAME(m_nubus_irq_state));
	save_item(NAME(m_se30_vbl_enable));
	save_item(NAME(m_adb_irq_pending));
	save_item(NAME(ca1_data));
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_via2_interrupt));
	save_item(NAME(m_scsi_interrupt));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_via2_ca1_hack));
}

void mac_state::machine_reset()
{
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

	m_via2_ca1_hack = 1;
	m_via2->write_ca1(1);
	m_via2->write_cb1(1);

	m_scsi_interrupt = 0;

	m_via2_vbl = 0;
	m_se30_vbl_enable = 0;
	m_nubus_irq_state = 0xff;
	m_last_taken_interrupt = 0;
}

void mac_state::mac_state_load()
{
	int overlay = m_overlay;
	m_overlay = -1;
	set_memory_overlay(overlay);
}

TIMER_CALLBACK_MEMBER(mac_state::overlay_timeout_func)
{
	if (m_overlay != -1)
	{
		set_memory_overlay(0); // kill the overlay
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

	m_overlay_timeout = timer_alloc(FUNC(mac_state::overlay_timeout_func), this);

	/* save state stuff */
	machine().save().register_postload(save_prepost_delegate(FUNC(mac_state::mac_state_load), this));
}

#define MAC_DRIVER_INIT(label, model) \
	void mac_state::init_##label()    \
	{                                 \
		mac_driver_init(model);       \
	}

MAC_DRIVER_INIT(macii, MODEL_MAC_II)
MAC_DRIVER_INIT(macse30, MODEL_MAC_SE30)
MAC_DRIVER_INIT(maciicx, MODEL_MAC_IICX)
MAC_DRIVER_INIT(maciifdhd, MODEL_MAC_II_FDHD)
MAC_DRIVER_INIT(maciix, MODEL_MAC_IIX)

void mac_state::nubus_slot_interrupt(uint8_t slot, uint32_t state)
{
	static const uint8_t masks[8] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
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

void mac_state::vblank_irq()
{
	/* handle ADB keyboard/mouse */
	m_macadb->adb_vblank();

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
	const int scanline = m_screen->vpos();
	if (scanline == 0)
	{
		vblank_irq();
	}
	const int next_scanline = (scanline + 1) % MAC_V_TOTAL;
	m_scanline_timer->adjust(m_screen->time_until_pos(next_scanline), next_scanline);
}

template <int Slot> void mac_state::nubus_irq_w(int state)
{
	nubus_slot_interrupt(Slot, state);
}

template void mac_state::nubus_irq_w<9>(int state);
template void mac_state::nubus_irq_w<0xa>(int state);
template void mac_state::nubus_irq_w<0xb>(int state);
template void mac_state::nubus_irq_w<0xc>(int state);
template void mac_state::nubus_irq_w<0xd>(int state);
template void mac_state::nubus_irq_w<0xe>(int state);

void mac_state::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void mac_state::devsel_w(uint8_t devsel)
{
	if (devsel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if (devsel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;
	m_fdc->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
		m_cur_floppy->ss_w((m_via1->read_pa() & 0x20) >> 5);
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void mac_state::macii_map(address_map &map)
{
	map(0x40000000, 0x4003ffff).rom().region("bootrom", 0).mirror(0x0ffc0000);

	// MMU remaps I/O without the F
	map(0x50000000, 0x50001fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
	map(0x50002000, 0x50003fff).rw(FUNC(mac_state::mac_via2_r), FUNC(mac_state::mac_via2_w)).mirror(0x00f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(mac_state::mac_scc_r), FUNC(mac_state::mac_scc_w)).mirror(0x00f00000);
	map(0x50006000, 0x50006003).w(FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x50006060, 0x50006063).r(FUNC(mac_state::macii_scsi_drq_r)).mirror(0x00f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(mac_state::macplus_scsi_r), FUNC(mac_state::macii_scsi_w)).mirror(0x00f00000);
	map(0x50012000, 0x50013fff).rw(FUNC(mac_state::macii_scsi_drq_r), FUNC(mac_state::macii_scsi_drq_w)).mirror(0x00f00000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(mac_state::mac_iwm_r), FUNC(mac_state::mac_iwm_w)).mirror(0x00f00000);
	map(0x50040000, 0x50041fff).rw(FUNC(mac_state::mac_via_r), FUNC(mac_state::mac_via_w)).mirror(0x00f00000);
}

void mac_state::macse30_map(address_map &map)
{
	macii_map(map);

	map(0xfe000000, 0xfe00ffff).ram().share("vram");
	map(0xfee00000, 0xfee0ffff).ram().share("vram").mirror(0x000f0000);
	map(0xfeffe000, 0xfeffffff).rom().region("se30vrom", 0x0);
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void mac_state::macii(machine_config &config)
{
	M68020PMMU(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macii_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_asc, C15M, asc_device::asc_type::ASC);
	m_asc->irqf_callback().set(FUNC(mac_state::mac_asc_irq));
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);

	RTC3430042(config, m_rtc, XTAL(32'768));
	m_rtc->cko_cb().set(m_via1, FUNC(via6522_device::write_ca2));

	IWM(config, m_fdc, C15M);
	m_fdc->devsel_cb().set(FUNC(mac_state::devsel_w));
	m_fdc->phases_cb().set(FUNC(mac_state::phases_w));
	applefdintf_device::add_35(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	SCC85C30(config, m_scc, C7M);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_int_callback().set(FUNC(mac_state::set_scc_interrupt));
	m_scc->out_txda_callback().set("printer", FUNC(rs232_port_device::write_txd));
	m_scc->out_txdb_callback().set("modem", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config([](device_t *device)
																							{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^lspeaker", 1.0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^rspeaker", 1.0); });
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR53C80).machine_config([this](device_t *device)
																					 {
		ncr53c80_device &adapter = downcast<ncr53c80_device &>(*device);
		adapter.irq_handler().set(*this, FUNC(mac_state::mac_scsi_irq));
		adapter.drq_handler().set(m_scsihelp, FUNC(mac_scsi_helper_device::drq_w)); });

	MAC_SCSI_HELPER(config, m_scsihelp);
	m_scsihelp->scsi_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::read));
	m_scsihelp->scsi_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::write));
	m_scsihelp->scsi_dma_read_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_r));
	m_scsihelp->scsi_dma_write_callback().set(m_ncr5380, FUNC(ncr53c80_device::dma_w));
	m_scsihelp->cpu_halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_scsihelp->timeout_error_callback().set(FUNC(mac_state::scsi_berr_w));

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68020");

	nubus_device &nubus(NUBUS(config, "nubus", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irq9_callback().set(FUNC(mac_state::nubus_irq_w<9>));
	nubus.out_irqa_callback().set(FUNC(mac_state::nubus_irq_w<0xa>));
	nubus.out_irqb_callback().set(FUNC(mac_state::nubus_irq_w<0xb>));
	nubus.out_irqc_callback().set(FUNC(mac_state::nubus_irq_w<0xc>));
	nubus.out_irqd_callback().set(FUNC(mac_state::nubus_irq_w<0xd>));
	nubus.out_irqe_callback().set(FUNC(mac_state::nubus_irq_w<0xe>));
	NUBUS_SLOT(config, "nb9", "nubus", mac_nubus_cards, "mdc824");
	NUBUS_SLOT(config, "nba", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbb", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbc", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbe", "nubus", mac_nubus_cards, nullptr);

	R65NC22(config, m_via1, C7M/10);
	m_via1->readpa_handler().set(FUNC(mac_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(mac_state::mac_via_in_b));
	m_via1->writepa_handler().set(FUNC(mac_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(mac_state::mac_via_out_b));
	m_via1->irq_handler().set(FUNC(mac_state::mac_via_irq));

	R65NC22(config, m_via2, C7M/10);
	m_via2->readpa_handler().set(FUNC(mac_state::mac_via2_in_a));
	m_via2->readpb_handler().set(FUNC(mac_state::mac_via2_in_b));
	m_via2->writepa_handler().set(FUNC(mac_state::mac_via2_out_a));
	m_via2->writepb_handler().set(FUNC(mac_state::mac_via2_out_b));
	m_via2->irq_handler().set(FUNC(mac_state::mac_via2_irq));

	ADBMODEM(config, m_adbmodem, C7M);
	m_adbmodem->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_adbmodem->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_adbmodem->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_adbmodem->irq_callback().set(FUNC(mac_state::adb_irq_w));
	m_via1->cb2_handler().set(m_adbmodem, FUNC(adbmodem_device::set_via_data));
	config.set_perfect_quantum(m_maincpu);

	MACADB(config, m_macadb, C15M);
	m_macadb->adb_data_callback().set(m_adbmodem, FUNC(adbmodem_device::set_adb_line));

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("8M,32M,64M,96M,128M");

	SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
	SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
}

void mac_state::maciihmu(machine_config &config)
{
	macii(config);

	M68020HMMU(config.replace(), m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macii_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");
}

void mac_state::maciihd(machine_config &config)
{
	macii(config);

	// Mac II FDHD = Mac II with a SWIM1 instead of IWM
	SWIM1(config.replace(), m_fdc, C15M);
	m_fdc->phases_cb().set(FUNC(mac_state::phases_w));
	m_fdc->devsel_cb().set(FUNC(mac_state::devsel_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_hd(config, m_floppy[1]);
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
}

void mac_state::maciix(machine_config &config)
{
	maciihd(config);

	// IIx = Mac II FDHD with a 68030 instead of the 020
	M68030(config.replace(), m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macii_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	SOFTWARE_LIST(config.replace(), "cd_list").set_original("mac_cdrom").set_filter("MC68030");
}

void mac_state::maciicx(machine_config &config)
{
	maciix(config);

	// IIcx = IIx with 3 fewer slots
	config.device_remove("nbc");
	config.device_remove("nbd");
	config.device_remove("nbe");
}

void mac_state::macse30(machine_config &config)
{
	maciicx(config);

	// SE/30 = IIx with no slots and built-in video
	m_maincpu->set_addrmap(AS_PROGRAM, &mac_state::macse30_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_refresh_hz(60.15);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1260));
	m_screen->set_size(MAC_H_TOTAL, MAC_V_TOTAL);
	m_screen->set_visarea(0, MAC_H_VIS-1, 0, MAC_V_VIS-1);
	m_screen->set_screen_update(FUNC(mac_state::screen_update_macse30));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);

	config.device_remove("nb9");
	config.device_remove("nba");
	config.device_remove("nbb");
	config.device_remove("nubus");

	nubus_device &nubus(NUBUS(config, "pds", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irq9_callback().set(FUNC(mac_state::nubus_irq_w<9>));
	nubus.out_irqa_callback().set(FUNC(mac_state::nubus_irq_w<0xa>));
	nubus.out_irqb_callback().set(FUNC(mac_state::nubus_irq_w<0xb>));
	nubus.out_irqc_callback().set(FUNC(mac_state::nubus_irq_w<0xc>));
	nubus.out_irqd_callback().set(FUNC(mac_state::nubus_irq_w<0xd>));
	nubus.out_irqe_callback().set(FUNC(mac_state::nubus_irq_w<0xe>));
	NUBUS_SLOT(config, "pds030", "pds", mac_pds030_cards, nullptr);
}

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

ROM_START( macii )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "default", "rev. B")
	ROMX_LOAD( "9779d2c4.rom", 0x000000, 0x040000, CRC(4df6d054) SHA1(db6b504744281369794e26ba71a6e385cf6227fa), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "original", "rev. A")
	ROMX_LOAD( "97851db6.rom", 0x000000, 0x040000, CRC(8c8b9d03) SHA1(5c264fe976f1e8495d364947c932a5e8309b4300), ROM_BIOS(1) )
ROM_END

ROM_START( maciihmu )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "default", "rev. B")
	ROMX_LOAD( "9779d2c4.rom", 0x000000, 0x040000, CRC(4df6d054) SHA1(db6b504744281369794e26ba71a6e385cf6227fa), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "original", "rev. A")
	ROMX_LOAD( "97851db6.rom", 0x000000, 0x040000, CRC(8c8b9d03) SHA1(5c264fe976f1e8495d364947c932a5e8309b4300), ROM_BIOS(1) )
ROM_END

ROM_START( mac2fdhd )   // same ROM for II FDHD, IIx, IIcx, and SE/30
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_LOAD( "97221136.rom", 0x000000, 0x040000, CRC(ce3b966f) SHA1(753b94351d94c369616c2c87b19d568dc5e2764e) )
ROM_END

ROM_START( maciix )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_LOAD( "97221136.rom", 0x000000, 0x040000, CRC(ce3b966f) SHA1(753b94351d94c369616c2c87b19d568dc5e2764e) )
ROM_END

ROM_START( maciicx )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_LOAD( "97221136.rom", 0x000000, 0x040000, CRC(ce3b966f) SHA1(753b94351d94c369616c2c87b19d568dc5e2764e) )
ROM_END

ROM_START( macse30 )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_LOAD( "97221136.rom", 0x000000, 0x040000, CRC(ce3b966f) SHA1(753b94351d94c369616c2c87b19d568dc5e2764e) )

	ROM_REGION32_BE(0x2000, "se30vrom", 0)
	ROM_LOAD( "se30vrom.uk6", 0x000000, 0x002000, CRC(b74c3463) SHA1(584201cc67d9452b2488f7aaaf91619ed8ce8f03) )
ROM_END

} // anonymous namespace

//    YEAR  NAME       PARENT    COMPAT  MACHINE   INPUT    CLASS      INIT                COMPANY           FULLNAME
COMP( 1987, macii,     0,        0,      macii,    macadb,  mac_state, init_macii,         "Apple Computer", "Macintosh II",                 MACHINE_SUPPORTS_SAVE )
COMP( 1987, maciihmu,  macii,    0,      maciihmu, macadb,  mac_state, init_macii,         "Apple Computer", "Macintosh II (w/o 68851 MMU)", MACHINE_SUPPORTS_SAVE )
COMP( 1988, mac2fdhd,  0,        0,      maciihd,  macadb,  mac_state, init_maciifdhd,     "Apple Computer", "Macintosh II (FDHD)",          MACHINE_SUPPORTS_SAVE )
COMP( 1988, maciix,    mac2fdhd, 0,      maciix,   macadb,  mac_state, init_maciix,        "Apple Computer", "Macintosh IIx",                MACHINE_SUPPORTS_SAVE )
COMP( 1989, macse30,   mac2fdhd, 0,      macse30,  macadb,  mac_state, init_macse30,       "Apple Computer", "Macintosh SE/30",              MACHINE_SUPPORTS_SAVE )
COMP( 1989, maciicx,   mac2fdhd, 0,      maciicx,  macadb,  mac_state, init_maciicx,       "Apple Computer", "Macintosh IIcx",               MACHINE_SUPPORTS_SAVE )
