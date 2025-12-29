// license:BSD-3-Clause
// copyright-holders:Carl, D. Donohoe
/****************************************************************************

    AT&T 6300 Plus emulation

    Although the 6300 Plus was manufactured by Olivetti, the motherboard
    is an AT&T design, and is very different from the original 6300
    motherboard (80286 CPU, custom virtualization hardware etc.).
    The rest of the machine is pretty much identical to the 6300, and
    like the original 6300, is not an AT clone.

    The main unique feature of this system is its virtualization hardware,
    which allows DOS to run under the control of Unix.  Hardware can prevent
    the realmode/legacy OS from writing to regions of memory, with access
    control on a 1K page basis.  Additionally, IO accesses can be virtualized,
    on a per-port basis, allowing device ownership and access control, and
    even the implemenation of virtual devices.  In effect, this system was the
    first to support a hardware virtual 8086 mode.

    TODO: while an attempt has been made to emulate the virtualization
    hardware, it is mostly unverified, and will almost certainly not work.
    In order to verify it, someone needs to unearth a copy of the AT&T Unix
    release that was developed and sold specifically for the 6300+, in
    addition to the OS Merge (AKA Simul-Task) software that's needed to run
    Unix and DOS concurrently.  There are seven disks labelled "UNIX System V
    Release 2.0 Foundation Set" and one labelled "UNIX System V Release 2.0
    Simul-Task OS Merge Disk".  All disks are also labelled with "AT&T
    Personal Computer 6300 Plus".  Anyone with a disk set to donate or sell
    is kindly requested to contact donohoe00 on the VCFed forum or github.

    Some of the enhanced functionality, such as switching in and out of
    protected mode, has been verified (since the BIOS does this in order to
    test memory above 640K).  This involves external logic to reset the CPU
    (the only way to return to Real Mode) and to enable/disable address
    lines A20-A23.

    Sources:

    https://archive.org/details/att_hardware_reference_manual_att_personal_computer_6300_plus/page/n41/mode/2up
    https://bitsavers.org/pdf/att/6300/ATT_Personal_Computer_6300_Plus_-_ROM-BIOS_Listing_-_Rel_2.05_Issue_1_-_1986.pdf

****************************************************************************/

#include "emu.h"

#include "att6300p_mmu.h"
#include "m24_kbd.h"

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "cpu/i86/i286.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/z80ctc.h"
#include "machine/mm58274c.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "formats/naslite_dsk.h"
#include "formats/m20_dsk.h"

#include "softlist_dev.h"


namespace {

class att6300p_state : public driver_device
{
public:
	att6300p_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_isabus(*this, "isabus"),
		m_dmac(*this, "dmac"),
		m_pic(*this, "pic"),
		m_pit(*this, "pit"),
		m_speaker(*this, "speaker"),
		m_kbc(*this, "kbc"),
		m_keyboard(*this, "keyboard"),
		m_ctc(*this, "ctc"),
		m_mmu(*this, "mmu"),
		m_dsw(*this, "DSW"),
		m_map_rom(*this, "addrmap"),
		m_nmi_enable(false)
	{ }

	void att6300p(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	/*
	 * Virtualization Register definitions
	 *
	 * 3f00-3f1f: RESETCS
	 *
	 *   Reset Chip Select.  Any access in this range will reset the CPU
	 *   (returning it to real mode), and automatically gate A20-A23.
	 *
	 * 3f20-3f3f: PROTECTEN
	 *
	 *   Write-only virtualization configuration latch
	 *
	 *     0: IOSETUP   : Set to enable writing of the IO access table.
	 *                    The table entries must be written to the ROM area,
	 *                    with A0-A9 providing the table index.
	 *     1: MEMSETUP  : Set to enable writing of the memory access table.
	 *                    Each entry is a 1-bit enable/disable.  Entries must
	 *                    be written one bit at a time (D0 provides the value
	 *                    to write).  Since A10-A19 are used to index the
	 *                    table, the 1024 entries of the table must
	 *                    (apparently) be written via an unused 1M region of
	 *                    physical address space, with a stride of 1K.
	 *                    Therefore it only makes sense to write the table when
	 *                    in protected mode (accesses via the lower 1M range
	 *                    likely don't even work, precluding it from being
	 *                    written in Real Mode).
	 *     3: IORDTRAP  : Enable readout of data/address/flags
	 *                    pertaining to trapped IO access(es) via TRAPCE.
	 *     4: _PWRUP    : Indicates type of reset to the Boot ROM.  The value
	 *                  : set here will be reflected in the BITREAD register.
	 *                     0: Perform power-on initialization sequence
	 *                     1: Reset was triggered to return from protected mode
	 *     5: IOWRTRAP  : Enable writing of IO trap registers via TRAPCE.
	 *     6: MEMFENCE  : Enable inhibition of memory writes in accordance
	 *                  : with contents of memory access table.
	 *     7: PROTECTEN : When set, enables A20-A23 address lines and disables
	 *                    IO port virtualization.  This setting is
	 *                    automatically disabled/cleared on reset (including
	 *                    soft reset).
	 *
	 * 3f40-3f5f: TIMESLICE
	 *
	 *   Z8430 CTC register access
	 *
	 *   Any access to a CTC register clears _SANITYNMI in the BITREAD
	 *   register, and deasserts the timeslice interrupt, clearing the TS
	 *   bit in BITREAD.
	 *
	 * 3f60-3f7f: TRAPCE
	 *
	 *   Access registers at the current index which store data/address/flags
	 *   pertaining to trapped IO access(es).  Register index automatically
	 *   advances on write.  Any access will clear the _TRAPIO signal (???),
	 *   in order to deassert _NMI.
	 *
	 *   BIT(regaddr, 0): Must be zero???
	 *   BIT(regaddr, 1): Chip Select (S2L/S2R)
	 *   BIT(regaddr, 2): 4-bit register file select
	 *
	 *   3f60: XXXX|LA11:LA8
	 *   3f62: Data 7:0
	 *   3f64: XXXX|LA7:LA4
	 *   3f66: LA0|_LBHE|_IORC|0|LA3:1|A0*
	 *
	 * 3f80-3f9f: VXLATEN
	 *
	 *   Address Mapping Select
	 *
	 *   The address translation PROM allows memory regions below 1M to be
	 *   remapped in 32K chunks.  Bits 3:0 are inputs to the PROM, allowing
	 *   software to select different mappings.  While translation via the PROM
	 *   cannot be disabled, some of the mappings in the PROM are 1:1,
	 *   effectively disabling mapping when selected.  While in theory any
	 *   chunk could be mapped to any 32K aligned region in the 16M address
	 *   space, in practice the PROM is programmed only to map regions of
	 *   (video) memory in the a0000-bffff range to the first meg of RAM (which
	 *   is split between 0-0x9ffff and fa0000-ffffff).
	 *
	 *   NOTE: Address translation occurs before Write Inhibit check.
	 *
	 * 3fa0-3fbf: BITREAD
	 *
	 *    Virtualization status
	 *
	 *      0: PROTECTEN  : Reflects the value last written to the
	 *                      PROTECTEN bit in the PROTECTEN register,
	 *                      unless a subsequent soft reset occured (in which
	 *                      case it will read as 0).
	 *      1: _SANITYNMI : Sanity timer fired.  Despite the name, NMI is not
	 *                      asserted when the timer fires: rather the CPU is
	 *                      reset, which you could argue is a type of
	 *                      non-maskable interrupt.
	 *      2: _TRAPIO    : One or more IO operations were trapped.
	 *      3: NMI        : NMI is asserted
	 *      4: TS         : Time Slice timer expired
	 *      5: _PWRUP     : Reflects the value last written to the
	 *                      _PWRUP bit in the PROTECTEN register.
	 *
	 * 3fc0-3fdf: ADADV
	 *
	 *    Address Advance - Read advances trap register index
	 *
	 * 3fe0-3fff: CLTRAP
	 *
	 *   Reset the trap register index to 0
	 *
	 */

	enum {
		// PROTECTEN register bits
		B_VIRT_PE_IOSETUP	= 0x01,
		B_VIRT_PE_MEMSETUP 	= 0x02,
		B_VIRT_PE_IORDTRAP 	= 0x08,
		B_VIRT_PE_PWRUP		= 0x10,
		B_VIRT_PE_IOWRTRAP	= 0x20,
		B_VIRT_PE_MEMFENCE	= 0x40,
		B_VIRT_PE_PROTECTEN	= 0x80,

		// BITREAD register bits
		B_VIRT_BR_PROTECTEN	= 0x01,
		B_VIRT_BR_SANITYNMI	= 0x02,
		B_VIRT_BR_TRAPIO	= 0x04,
		B_VIRT_BR_NMI		= 0x08,
		B_VIRT_BR_TS		= 0x10,
		B_VIRT_BR_PWRUP		= 0x20,
	};

	void dma_segment_w(offs_t offset, uint8_t data);
	void dma_hrq_w(int state);
	uint8_t dma_memory_read(offs_t offset);
	void dma_memory_write(offs_t offset, uint8_t data);
	template <int Channel> uint8_t dma_io_read(offs_t offset);
	template <int Channel> void dma_io_write(offs_t offset, uint8_t data);
	template <int Channel> void dma_dack_w(int state);
	void dma_tc_w(int state);
	void dreq0_ck_w(int state);
	void speaker_ck_w(int state);
	void update_speaker();
	void trapio_cb(uint32_t data);

	uint8_t keyboard_data_r();
	uint8_t keyboard_status_r();

	void ctrlport_a_w(uint8_t data);
	uint8_t ctrlport_a_r();
	uint8_t ctrlport_b_r();

	void zc01_cb(int state);
	void zc02_cb(int state);

	void chck_w(int state);
	void nmi_enable_w(uint8_t data);
	void update_nmi();

	void set_protected_mode(bool enabled);

	required_device<i80286_cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<isa8_device> m_isabus;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<pit8254_device> m_pit;
	required_device<speaker_sound_device> m_speaker;
	required_device<i8741a_device> m_kbc;
	required_device<m24_keyboard_device> m_keyboard;
	required_device<z80ctc_device> m_ctc;
	required_device<att6300p_mmu_device> m_mmu;
	required_ioport m_dsw;
	required_memory_region m_map_rom;

	uint8_t m_dma_segment[4];
	uint8_t m_dma_active;
	bool m_tc;
	bool m_dreq0_ck;

	uint8_t m_ctrlport_a;
	uint8_t m_ctrlport_b;

	bool m_nmi_enable;
	bool m_nmi_active;
	bool m_chck_active;
	bool m_trapio_active;

	bool m_sanity_active;
	bool m_ts_int;

	uint8_t m_p2_out;
	bool m_kbdata;
	bool m_kb_int;

	uint8_t kbc_p1_r();
	void kbc_p2_w(uint8_t data);
	int kbc_t0_r();
	int kbc_t1_r();
	uint8_t kbcdata_r();
	void kbcdata_w(uint8_t data);
	void kbcin_w(int state);

	void update_ir1();

	static void floppy_formats(format_registration &fr);

	void soft_reset();
	void configure_mapping(int sel);

	uint8_t reset_r(offs_t offset);
	void reset_w(offs_t offset, uint8_t data);
	void protecten_w(offs_t offset, uint8_t data);
	uint8_t timeslice_r(offs_t offset);
	void timeslice_w(offs_t offset, uint8_t data);
	uint8_t trapce_r(offs_t offset);
	void trapce_w(offs_t offset, uint8_t data);
	uint8_t vxlaten_r(offs_t offset);
	void vxlaten_w(offs_t offset, uint8_t data);
	uint8_t bitread_r(offs_t offset);
	uint8_t adadv_r(offs_t offset);
	void adadv_w(offs_t offset, uint8_t data);
	uint8_t cltrap_r(offs_t offset);
	void cltrap_w(offs_t offset, uint8_t data);

	bool m_protected;
	bool m_mapping_sel;
	bool m_warm_reset;
	bool m_trapio_read_enabled;
	bool m_trapio_write_enabled;

	uint8_t m_trapio_reg_idx;
	uint16_t m_trapio_reg[4][4];

	static void cfg_m20_format(device_t *device);
	void kbc_map(address_map &map) ATTR_COLD;

	void att6300p_io_map(address_map &map) ATTR_COLD;
	void att6300p_mem_map(address_map &map) ATTR_COLD;

	void att6300p_vmem_map(address_map &map) ATTR_COLD;
	void att6300p_vio_map(address_map &map) ATTR_COLD;
};

void att6300p_state::machine_start()
{
	offs_t rsize = m_ram->size();

	// Conventional RAM
	m_mmu->space(AS_PROGRAM).install_ram(0, (rsize > 640*1024 ? 640*1024 : rsize) - 1, m_ram->pointer());

	if (rsize > 640*1024)
	{
		// Upper 384K of the first megabyte - if installed - shows up at
		// 16M-384K, where it can only be accessed in protected mode.
		m_mmu->space(AS_PROGRAM).install_ram(0xfa0000, 0xffffff, m_ram->pointer() + 640*1024);
	}

	if (rsize > 1024*1024)
	{
		m_mmu->space(AS_PROGRAM).install_ram(0x100000, rsize-1, m_ram->pointer() + 1024*1024);
	}

	std::fill_n(&m_dma_segment[0], 4, 0);
	m_dma_active = 0;
	m_tc = false;
	m_dreq0_ck = true;

	m_ctrlport_a = 0;
	m_ctrlport_b = 0;
	m_kb_int = false;

	m_nmi_active = false;
	m_chck_active = false;
	m_sanity_active = false;
	m_ts_int = false;
	m_trapio_active = false;
	m_nmi_enable = false;

	m_warm_reset = false;
	m_trapio_read_enabled = false;
	m_trapio_write_enabled = false;

	m_trapio_reg_idx = 0;

	save_item(NAME(m_dma_segment));
	save_item(NAME(m_dma_active));
	save_item(NAME(m_tc));
	save_item(NAME(m_dreq0_ck));
	save_item(NAME(m_ctrlport_a));
	save_item(NAME(m_ctrlport_b));
	save_item(NAME(m_nmi_active));
	save_item(NAME(m_chck_active));
	save_item(NAME(m_sanity_active));
	save_item(NAME(m_ts_int));
	save_item(NAME(m_trapio_active));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_p2_out));
	save_item(NAME(m_kbdata));
	save_item(NAME(m_kb_int));
	save_item(NAME(m_protected));
	save_item(NAME(m_warm_reset));
	save_item(NAME(m_trapio_read_enabled));
	save_item(NAME(m_trapio_write_enabled));
	save_item(NAME(m_trapio_reg_idx));
	save_item(NAME(m_trapio_reg));
}

void att6300p_state::machine_reset()
{
	set_protected_mode(false);
	configure_mapping(0);

	ctrlport_a_w(0);
	nmi_enable_w(0);
	m_p2_out = 0;
	m_kbdata = true;
}

void att6300p_state::trapio_cb(uint32_t data)
{
	uint16_t addr = data & 0x0fff;
	uint8_t val = (data>>16) & 0xff;
	uint8_t flags = (data>>24) & 0xf;

	m_trapio_reg[m_trapio_reg_idx][0] = (addr >> 8);
	m_trapio_reg[m_trapio_reg_idx][1] = val & 0xff;
	m_trapio_reg[m_trapio_reg_idx][2] = (addr>>4) & 0xf;
	m_trapio_reg[m_trapio_reg_idx][3] = flags << 4 | (addr & 0xf);

	m_trapio_reg_idx = (m_trapio_reg_idx + 1) & 3;

	m_trapio_active = true;
}

void att6300p_state::dma_segment_w(offs_t offset, uint8_t data)
{
	m_dma_segment[offset] = data & 0x0f;
}

void att6300p_state::dma_hrq_w(int state)
{
	/* Assert HLDA */
	m_dmac->hack_w(state);
}

uint8_t att6300p_state::dma_memory_read(offs_t offset)
{
	const int seg = (BIT(m_dma_active, 2) ? 0 : 2) | (BIT(m_dma_active, 3) ? 0 : 1);
	return m_mmu->space(AS_PROGRAM).read_byte(offset | u32(m_dma_segment[seg]) << 16);
}

void att6300p_state::dma_memory_write(offs_t offset, uint8_t data)
{
	const int seg = (BIT(m_dma_active, 2) ? 0 : 2) | (BIT(m_dma_active, 3) ? 0 : 1);
	m_mmu->space(AS_PROGRAM).write_byte(offset | u32(m_dma_segment[seg]) << 16, data);
}

template <int Channel>
uint8_t att6300p_state::dma_io_read(offs_t offset)
{
	return m_isabus->dack_r(Channel);
}

template <int Channel>
void att6300p_state::dma_io_write(offs_t offset, uint8_t data)
{
	m_isabus->dack_w(Channel, data);
}

template <int Channel>
void att6300p_state::dma_dack_w(int state)
{
	m_isabus->dack_line_w(Channel, state);

	if (!state)
	{
		m_dma_active |= 1 << Channel;
		if (Channel == 0)
			m_dmac->dreq0_w(0);
		if (m_tc)
			m_isabus->eop_w(Channel, ASSERT_LINE);
	}
	else
	{
		m_dma_active &= ~(1 << Channel);
		if (m_tc)
			m_isabus->eop_w(Channel, CLEAR_LINE);
	}
}

void att6300p_state::dma_tc_w(int state)
{
	m_tc = (state == ASSERT_LINE);
	for (int channel = 0; channel < 4; channel++)
		if (BIT(m_dma_active, channel))
			m_isabus->eop_w(channel, state);
}

void att6300p_state::dreq0_ck_w(int state)
{
	if (state && !m_dreq0_ck && !BIT(m_dma_active, 0))
		m_dmac->dreq0_w(1);

	m_dreq0_ck = state;
}

void att6300p_state::speaker_ck_w(int state)
{
	if (state)
		m_ctrlport_b |= 0x20;
	else
		m_ctrlport_b &= 0xdf;

	update_speaker();
}

void att6300p_state::update_speaker()
{
	if (BIT(m_ctrlport_a, 1) && BIT(m_ctrlport_b, 5))
	{
		m_speaker->level_w(1);
		m_ctrlport_b &= 0xef;
	}
	else
	{
		m_speaker->level_w(0);
		m_ctrlport_b |= 0x10;
	}
}

void att6300p_state::update_ir1()
{
	// TimeSlice interrupt is shared with KBD interrupt
	m_pic->ir1_w((m_kb_int || m_ts_int) ? 1 : 0);
}

int att6300p_state::kbc_t0_r()
{
	return 1;
}

int att6300p_state::kbc_t1_r()
{
	return BIT(m_ctrlport_a, 7);	// Keyboard Enable
}

uint8_t att6300p_state::keyboard_data_r()
{
	m_kb_int = false;
	update_ir1();
	return m_kbc->upi41_master_r(0);
}

uint8_t att6300p_state::keyboard_status_r()
{
	return m_kbc->upi41_master_r(1);
}

void att6300p_state::ctrlport_a_w(uint8_t data)
{
	const bool spkrdata_en_dis = BIT(data ^ m_ctrlport_a, 1);
	const bool iochk_en_dis = BIT(data ^ m_ctrlport_a, 4);

	m_pit->write_gate2(BIT(data, 0));

	if (BIT(m_ctrlport_a, 4) && !m_chck_active)
		m_ctrlport_b &= 0xbf;

	m_ctrlport_a = data;

	if (spkrdata_en_dis)
		update_speaker();
	if (iochk_en_dis)
		update_nmi();
}

uint8_t att6300p_state::ctrlport_a_r()
{
	return m_ctrlport_a;
}

uint8_t att6300p_state::ctrlport_b_r()
{
	// Bit 0 = NC
	// Bit 1 = DSW2-5 (80287 present)
	// Bit 2 = ~RI1
	// Bit 3 = ~DSR1
	// Bit 4 = ~OUT2 (8254)
	// Bit 5 = OUT2
	// Bit 6 = IOCHK
	// Bit 7 = MBMERR (MRD parity check)

	if (BIT(m_dsw->read(), 12))
		m_ctrlport_b |= 0x02;
	else
		m_ctrlport_b &= 0xfd;

	return m_ctrlport_b;
}

void att6300p_state::chck_w(int state)
{
	m_chck_active = (state == 0);
	if (m_chck_active)
	{
		if (!BIT(m_ctrlport_b, 6))
		{
			m_ctrlport_b |= 0x40;
			update_nmi();
		}
	}
	else if (BIT(m_ctrlport_a, 4))
		m_ctrlport_b &= 0xbf;
}

void att6300p_state::nmi_enable_w(uint8_t data)
{
	m_nmi_enable = BIT(data, 7);
	update_nmi();
}

void att6300p_state::update_nmi()
{
	m_nmi_active = (m_trapio_active ||
	  (m_nmi_enable && BIT(m_ctrlport_b, 6) && !BIT(m_ctrlport_a, 4)));

	if (m_nmi_active)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	else
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

uint8_t att6300p_state::kbc_p1_r()
{
	return (~m_p2_out & (m_kbdata ? 3 : 1)) | BIT(m_ctrlport_a, 6) << 2;
}

void att6300p_state::kbc_p2_w(uint8_t data)
{
	m_p2_out = data;

	m_keyboard->clock_w(!BIT(data, 0));
	m_keyboard->data_w(!BIT(data, 1));

	// Needed to ensure the keyboard MCU gets scheduled to respond to
	// the clock change and output the data in time.
	machine().scheduler().perfect_quantum(attotime::from_usec(50));

	if (m_kb_int != BIT(data, 4))
	{
		m_kb_int = BIT(data, 4);
		update_ir1();
	}
}

void att6300p_state::kbcin_w(int state)
{
	m_kbdata = state;
}

void att6300p_state::zc01_cb(int state)
{
	m_ctc->trg2(state);

	if (state)
	{
		// Timeslice interrupt
		m_ts_int = true;
		update_ir1();
	}
}

void att6300p_state::zc02_cb(int state)
{
	if (state)
	{
		// Sanity timer expired.
		m_sanity_active = true;
		soft_reset();
	}
}

void att6300p_state::soft_reset()
{
	set_protected_mode(false);
	m_maincpu->reset();
}

void att6300p_state::configure_mapping(int sel)
{
	static const uint8_t reverse_bits[32] = {
		0x00, 0x10, 0x08, 0x18, 0x04, 0x14, 0x0c, 0x1c,
		0x02, 0x12, 0x0a, 0x1a, 0x06, 0x16, 0x0e, 0x1e,
		0x01, 0x11, 0x09, 0x19, 0x05, 0x15, 0x0d, 0x1d,
		0x03, 0x13, 0x0b, 0x1b, 0x07, 0x17, 0x0f, 0x1f,
	};

	bool rom_upper = (sel & 0x8) == 0 || m_protected;
	int rom_region = (sel & 7) + (rom_upper ? 8 : 0);

	uint8_t *rom = m_map_rom->base() + rom_region*32;

	uint32_t table[32];
	for (int i = 0; i < 32; i++)
	{
		// AA19:AA15 inputs to the PROM are reversed
		int idx = reverse_bits[i];

		// Redirect to a 32K region below 1M
		table[i] = (rom[idx] >> 3)*32*1024;

		if (rom[idx] & 0x04)
		{
			// Redirect to top area of RAM
			table[i] |= 0xf00000;
		}
	}

	m_mmu->set_mem_mapping(table);

	m_mapping_sel = sel;
}

void att6300p_state::set_protected_mode(bool enabled)
{
	if (m_protected != enabled)
	{
		m_protected = enabled;
		configure_mapping(m_mapping_sel);
		m_mmu->set_protected_mode_enabled(enabled);
	}
}

uint8_t att6300p_state::reset_r(offs_t offset)
{
	soft_reset();

	return 0xff;
}

void att6300p_state::reset_w(offs_t offset, uint8_t data)
{
	soft_reset();
}

uint8_t att6300p_state::timeslice_r(offs_t offset)
{
	m_sanity_active = false;
	m_ts_int = false;
	update_ir1();

	return m_ctc->read(offset & 3);
}

void att6300p_state::timeslice_w(offs_t offset, uint8_t data)
{
	m_sanity_active = false;
	m_ts_int = false;
	update_ir1();

	m_ctc->write(offset & 3, data);
}

void att6300p_state::protecten_w(offs_t offset, uint8_t data)
{
	set_protected_mode(data & B_VIRT_PE_PROTECTEN);

	m_mmu->set_mem_setup_enabled(data & B_VIRT_PE_MEMSETUP);
	m_mmu->set_io_setup_enabled(data & B_VIRT_PE_IOSETUP);
	m_mmu->set_memprot_enabled(data & B_VIRT_PE_MEMFENCE);

	m_trapio_read_enabled = data & B_VIRT_PE_IORDTRAP;
	m_trapio_write_enabled = data & B_VIRT_PE_IOWRTRAP;
	m_warm_reset = data & B_VIRT_PE_PWRUP;
}

uint8_t att6300p_state::trapce_r(offs_t offset)
{
	m_trapio_active = false;
	update_nmi();

	// Surely IORDTRAP has some other more useful purpose?
	if (m_trapio_read_enabled)
	{
		return m_trapio_reg[m_trapio_reg_idx][(offset>>1) & 0x3];
	}

	return 0xff;
}

void att6300p_state::trapce_w(offs_t offset, uint8_t data)
{
	m_trapio_active = false;
	update_nmi();

	if (m_trapio_write_enabled)
	{
		m_trapio_reg[m_trapio_reg_idx][(offset>>1) & 0x3] = data;

		// Should the advance be conditional on register address bit 0???
		m_trapio_reg_idx = (m_trapio_reg_idx + 1) & 3;
	}
}

uint8_t att6300p_state::vxlaten_r(offs_t offset)
{
	return m_mapping_sel;
}

void att6300p_state::vxlaten_w(offs_t offset, uint8_t data)
{
	configure_mapping(data & 0xf);
}

uint8_t att6300p_state::bitread_r(offs_t offset)
{
	  return (m_protected ? B_VIRT_BR_PROTECTEN : 0) |
		(m_sanity_active ? 0 : B_VIRT_BR_SANITYNMI) |
		(m_trapio_active ? 0 : B_VIRT_BR_TRAPIO) |
		(m_nmi_active ? B_VIRT_BR_NMI: 0) |
		(m_ts_int ? B_VIRT_BR_TS : 0) |
		(m_warm_reset ? B_VIRT_BR_PWRUP : 0);
}

uint8_t att6300p_state::adadv_r(offs_t offset)
{
	m_trapio_reg_idx = (m_trapio_reg_idx + 1) & 3;

	return 0xff;
}

void att6300p_state::adadv_w(offs_t offset, uint8_t data)
{
	m_trapio_reg_idx = (m_trapio_reg_idx + 1) & 3;
}

uint8_t att6300p_state::cltrap_r(offs_t offset)
{
	m_trapio_reg_idx = 0;

	return 0xff;
}

void att6300p_state::cltrap_w(offs_t offset, uint8_t data)
{
	m_trapio_reg_idx = 0;
}

void att6300p_state::att6300p_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf8000, 0xfffff).rom().region("bios", 0);
}

void att6300p_state::att6300p_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x000f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).mirror(0xe).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x0043).mirror(0xc).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0060).r(FUNC(att6300p_state::keyboard_data_r)).w(m_kbc, FUNC(i8741a_device::upi41_master_w));
	map(0x0061, 0x0061).rw(FUNC(att6300p_state::ctrlport_a_r), FUNC(att6300p_state::ctrlport_a_w));
	map(0x0062, 0x0062).r(FUNC(att6300p_state::ctrlport_b_r));
	map(0x0064, 0x0064).r(FUNC(att6300p_state::keyboard_status_r));
	map(0x0066, 0x0067).portr("DSW");
	map(0x0070, 0x007f).rw("mm58274", FUNC(mm58274c_device::read), FUNC(mm58274c_device::write));
	map(0x0080, 0x0083).mirror(0xc).w(FUNC(att6300p_state::dma_segment_w));
	map(0x00a0, 0x00a1).mirror(0xe).w(FUNC(att6300p_state::nmi_enable_w));
}

void att6300p_state::att6300p_vmem_map(address_map &map)
{
	map(0x000000, 0xffffff).rw(m_mmu, FUNC(att6300p_mmu_device::mem_r), FUNC(att6300p_mmu_device::mem_w));
}

void att6300p_state::att6300p_vio_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).mirror(0xc000).rw(m_mmu, FUNC(att6300p_mmu_device::io_r), FUNC(att6300p_mmu_device::io_w));
	map(0x3f00, 0x3f00).mirror(0x001f).rw(FUNC(att6300p_state::reset_r), FUNC(att6300p_state::reset_w));
	map(0x3f20, 0x3f20).mirror(0x001f).w(FUNC(att6300p_state::protecten_w));
	map(0x3f40, 0x3f40).mirror(0x001f).rw(FUNC(att6300p_state::timeslice_r), FUNC(att6300p_state::timeslice_w));
	map(0x3f60, 0x3f7f).rw(FUNC(att6300p_state::trapce_r), FUNC(att6300p_state::trapce_w));
	map(0x3f80, 0x3f80).mirror(0x001f).rw(FUNC(att6300p_state::vxlaten_r), FUNC(att6300p_state::vxlaten_w));
	map(0x3fa0, 0x3fa0).mirror(0x001f).r(FUNC(att6300p_state::bitread_r));
	map(0x3fc0, 0x3fc0).mirror(0x001f).rw(FUNC(att6300p_state::adadv_r), FUNC(att6300p_state::adadv_w));
	map(0x3fe0, 0x3fe0).mirror(0x001f).rw(FUNC(att6300p_state::cltrap_r), FUNC(att6300p_state::cltrap_w));
}

static INPUT_PORTS_START( att6300p )
	PORT_START("DSW")

	// DSW1
	PORT_DIPNAME( 0x0001, 0x0000, "Drive B Type")
	PORT_DIPSETTING(      0x0000, "96 TPI" )
	PORT_DIPSETTING(      0x0001, "48 TPI" )
	PORT_DIPNAME( 0x0002, 0x0000, "Drive A Type")
	PORT_DIPSETTING(      0x0000, "96 TPI" )
	PORT_DIPSETTING(      0x0002, "48 TPI" )
	PORT_DIPNAME( 0x000c, 0x0000, "Hard Disk Type")
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0040, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPNAME( 0x0030, 0x0010, "Display Type")
	PORT_DIPSETTING(      0x0000, "Monochrome" )
	PORT_DIPSETTING(      0x0010, "Color 80x25" )
	PORT_DIPSETTING(      0x0020, "Color 40x25" )
	PORT_DIPNAME( 0x00c0, 0x0080, "Number of floppy drives")
	PORT_DIPSETTING(      0x00c0, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x0040, "3" )

	// DSW2
	PORT_DIPNAME( 0x0f00, 0x0400, "Motherboard RAM banks")
	PORT_DIPSETTING(      0x0e00, "128K - 128/0")
	PORT_DIPSETTING(      0x0d00, "256K - 128/128")
	PORT_DIPSETTING(      0x0700, "512K - 512/0")
	PORT_DIPSETTING(      0x0600, "640K - 128/512")
	PORT_DIPSETTING(      0x0500, "640K - 512/128")
	PORT_DIPSETTING(      0x0400, "1M - 512/512")
	PORT_DIPNAME( 0x1000, 0x0000, "80287 installed")
	PORT_DIPSETTING(      0x0000, DEF_STR(No) )
	PORT_DIPSETTING(      0x1000, DEF_STR(Yes) )
	PORT_DIPNAME( 0x4000, 0x4000, "HDD ROM")
	PORT_DIPSETTING(      0x0000, "External" )
	PORT_DIPSETTING(      0x4000, "Internal" )
	PORT_DIPNAME( 0x8000, 0x00, "EPROM Size")
	PORT_DIPSETTING(      0x0000, "32K" )
	PORT_DIPSETTING(      0x8000, "64K" )
INPUT_PORTS_END

void att6300p_state::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
	fr.add(FLOPPY_M20_FORMAT);
}

void att6300p_state::cfg_m20_format(device_t *device)
{
	device->subdevice<floppy_connector>("fdc:0")->set_formats(att6300p_state::floppy_formats);
	device->subdevice<floppy_connector>("fdc:1")->set_formats(att6300p_state::floppy_formats);
}

void att6300p_state::att6300p(machine_config &config)
{
	I80286(config, m_maincpu, 12_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &att6300p_state::att6300p_vmem_map);
	m_maincpu->set_addrmap(AS_IO, &att6300p_state::att6300p_vio_map);
	m_maincpu->set_irq_acknowledge_callback("pic", FUNC(pic8259_device::inta_cb));

	ATT6300P_MMU(config, m_mmu, 0);
	m_mmu->set_addrmap(AS_PROGRAM, &att6300p_state::att6300p_mem_map);
	m_mmu->set_addrmap(AS_IO, &att6300p_state::att6300p_io_map);
	m_mmu->trapio_callback().set(*this, FUNC(att6300p_state::trapio_cb));

	AM9517A(config, m_dmac, 14.318181_MHz_XTAL / 2); // 8237A
	m_dmac->out_hreq_callback().set(FUNC(att6300p_state::dma_hrq_w));
	m_dmac->in_memr_callback().set(FUNC(att6300p_state::dma_memory_read));
	m_dmac->out_memw_callback().set(FUNC(att6300p_state::dma_memory_write));
	m_dmac->in_ior_callback<1>().set(FUNC(att6300p_state::dma_io_read<1>));
	m_dmac->in_ior_callback<2>().set(FUNC(att6300p_state::dma_io_read<2>));
	m_dmac->in_ior_callback<3>().set(FUNC(att6300p_state::dma_io_read<3>));
	m_dmac->out_iow_callback<1>().set(FUNC(att6300p_state::dma_io_write<1>));
	m_dmac->out_iow_callback<2>().set(FUNC(att6300p_state::dma_io_write<2>));
	m_dmac->out_iow_callback<3>().set(FUNC(att6300p_state::dma_io_write<3>));
	m_dmac->out_dack_callback<0>().set(FUNC(att6300p_state::dma_dack_w<0>));
	m_dmac->out_dack_callback<1>().set(FUNC(att6300p_state::dma_dack_w<1>));
	m_dmac->out_dack_callback<2>().set(FUNC(att6300p_state::dma_dack_w<2>));
	m_dmac->out_dack_callback<3>().set(FUNC(att6300p_state::dma_dack_w<3>));
	m_dmac->out_eop_callback().set(FUNC(att6300p_state::dma_tc_w));

	PIC8259(config, m_pic);
	m_pic->in_sp_callback().set_constant(1);
    m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	PIT8254(config, m_pit);
	m_pit->set_clk<0>(14.318181_MHz_XTAL / 12);		// IOCLK/4
	m_pit->set_clk<1>(14.318181_MHz_XTAL / 12);
	m_pit->set_clk<2>(14.318181_MHz_XTAL / 12);
	m_pit->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_pit->out_handler<1>().set(FUNC(att6300p_state::dreq0_ck_w));
	m_pit->out_handler<2>().set(FUNC(att6300p_state::speaker_ck_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	ISA8(config, m_isabus, 14.318181_MHz_XTAL / 3);	// IOCLK
	m_isabus->set_memspace(m_mmu, AS_PROGRAM);
	m_isabus->set_iospace(m_mmu, AS_IO);
	m_isabus->irq2_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	m_isabus->irq3_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	m_isabus->irq4_callback().set(m_pic, FUNC(pic8259_device::ir4_w));
	m_isabus->irq5_callback().set(m_pic, FUNC(pic8259_device::ir5_w));
	m_isabus->irq6_callback().set(m_pic, FUNC(pic8259_device::ir6_w));
	m_isabus->irq7_callback().set(m_pic, FUNC(pic8259_device::ir7_w));
	m_isabus->drq1_callback().set(m_dmac, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dmac, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dmac, FUNC(am9517a_device::dreq3_w));
	m_isabus->iochck_callback().set(FUNC(att6300p_state::chck_w));

	ISA8_SLOT(config, "mb1", 0, m_isabus, pc_isa8_cards, "cga_m24", true);
	ISA8_SLOT(config, "mb2", 0, m_isabus, pc_isa8_cards, "fdc_xt", true).set_option_machine_config("fdc_xt", cfg_m20_format);
	ISA8_SLOT(config, "mb3", 0, m_isabus, pc_isa8_cards, "lpt", true);
	ISA8_SLOT(config, "mb4", 0, m_isabus, pc_isa8_cards, "com", true);

	ISA8_SLOT(config, "isa1", 0, m_isabus, pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa2", 0, m_isabus, pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa3", 0, m_isabus, pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa4", 0, m_isabus, pc_isa8_cards, nullptr, false);

	// The following three expansion slots can take Olivetti 16-bit boards in
	// addition to standard 8-bit ISA boards, but they cannot take standard ISA
	// 16-bit boards.  The 512KB or 2MB memory expansion boards from AT&T can
	// work only in these slots.  In order for the memory on the expansion
	// boards to be usable under DOS, the expansion boards would have to be
	// emulated properly, with EMS-style windowing/paging functionality.  AT&T
	// Unix on the other hand runs in protected mode, and therefore is able to
	// access all of the memory without needing to muck with any paging
	// registers.
	//
	// TODO: Emulate AT&T/Olivetti memory expansion cards such that they work
	// with AEMM.SYS and AEX.SYS.
	//
	// We behave as if otherwise-unused 16-bit slots have non-EMS memory cards
	// in them, making up to 7M RAM in total possible in protected mode.  The
	// BIOS can see memory above 640K during its power-on test, since it
	// switches to protected mode in order to test it.
	//
	// TODO: For each 16-bit slot that gets populated with something else, the
	// amount of possible memory should be reduced by 2M (from a max of 7M)
	// in order to keep the system configuration legitimate.
	//
	ISA8_SLOT(config, "isa5", 0, m_isabus, pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa6", 0, m_isabus, pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa7", 0, m_isabus, pc_isa8_cards, nullptr, false);

	RAM(config, m_ram).set_default_size("1M").set_extra_options("128K, 256K, 512K, 640K, 3M, 5M, 7M");

	I8741A(config, m_kbc, 4_MHz_XTAL);
	m_kbc->p1_in_cb().set(FUNC(att6300p_state::kbc_p1_r));
	m_kbc->p2_out_cb().set(FUNC(att6300p_state::kbc_p2_w));
	m_kbc->t0_in_cb().set(FUNC(att6300p_state::kbc_t0_r));
	m_kbc->t1_in_cb().set(FUNC(att6300p_state::kbc_t1_r));

	M24_KEYBOARD(config, m_keyboard, 0);
	m_keyboard->out_data_handler().set(FUNC(att6300p_state::kbcin_w));

	MM58274C(config, "mm58274", 32.768_kHz_XTAL);

	Z80CTC(config, m_ctc, 12_MHz_XTAL / 4);		// PCLK/2
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_ctc->zc_callback<1>().set(FUNC(att6300p_state::zc01_cb));
	m_ctc->zc_callback<2>().set(FUNC(att6300p_state::zc02_cb));

	/* software lists */
	SOFTWARE_LIST(config, "disk_list").set_original("ibm5150");
}

ROM_START( att6300p )
	ROM_REGION16_LE(0x8000, "bios", 0)

	ROM_LOAD16_BYTE("att6300p_bios_6j_v1.04_lo.bin", 0x0000, 0x4000, CRC(dd0b9335) SHA1(56783b84f34d900b3cc73d1f7f1291a4211271ec))
	ROM_LOAD16_BYTE("att6300p_bios_6k_v1.04_hi.bin", 0x0001, 0x4000, CRC(208d84ab) SHA1(16ac79071a39b31ff002231f3fa564ac2d91b30c))

	ROM_REGION16_LE(0x200, "addrmap", 0)
	ROMX_LOAD("addrmap_4f_r4.0.bin", 0, 0x200, CRC(ea70d65d) SHA1(aed2295e0cb747b0a1ce5e78df4f99ac6ec692ed), 0)

	ROM_REGION(0x400, "kbc", 0)
	ROM_LOAD("d8741a_kbc_8w_v2.1.bin", 0x000, 0x400, CRC(e5a95ace) SHA1(f87f83ae1be790cde227514168f48bd7f806b2fa))
ROM_END

} // Anonymous namespace


//    YEAR  NAME       PARENT   COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY     FULLNAME      FLAGS
COMP( 1985, att6300p,  0,       0,      att6300p, att6300p, att6300p_state, empty_init, "AT&T",     "6300 Plus",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
