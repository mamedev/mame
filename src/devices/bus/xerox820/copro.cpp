// license:BSD-3-Clause
// copyright-holders:Dave Rand
/**********************************************************************

    Xerox 820-II coprocessor slot + the 16/8 8086 board

**********************************************************************/

#include "emu.h"
#include "copro.h"

#include "cpu/i86/i86.h"
#include "machine/wd1002_05.h"


namespace {

//**************************************************************************
//  16/8 8086 BOARD
//**************************************************************************

#define I8086_TAG "i8086"

// ======================> xerox820_16_8_device (implementation private to this file)

class xerox820_16_8_device : public device_t, public device_xerox820_copro_card_interface
{
public:
	xerox820_16_8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual uint8_t shared_ram_r(offs_t offset) override;
	virtual void shared_ram_w(offs_t offset, uint8_t data) override;

protected:
	xerox820_16_8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock); // for derived cards (the EM-II)

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t stop_r();          // IN  A0: stop = assert/hold 8086 RESET
	void lock_w(uint8_t data); // OUT A0, D7=1: lock = hold 8086 in RESET
	uint8_t start_r();         // IN  A1: start = release RESET -> fresh POST
	void dbell_w(uint8_t data);// OUT A1: Z80->8086 doorbell (counted latch)
	IRQ_CALLBACK_MEMBER(dbell_inta);

	required_device<i8086_cpu_device> m_i8086;
	memory_access<20, 1, 0, ENDIANNESS_LITTLE>::specific m_prog; // 8086 program space

	uint16_t m_dbell_count = 0; // outstanding (un-acknowledged) Z80->8086 doorbell rings

	// Z80 shared-RAM RMW reservation (see shared_ram_r/w)
	offs_t m_rmw_addr = ~offs_t(0);
	uint8_t m_rmw_val = 0;
	uint64_t m_rmw_cycles = 0;
};


xerox820_16_8_device::xerox820_16_8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: xerox820_16_8_device(mconfig, XEROX820_16_8, tag, owner, clock)
{
}

xerox820_16_8_device::xerox820_16_8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_xerox820_copro_card_interface(mconfig, *this)
	, m_i8086(*this, I8086_TAG)
{
}


//-------------------------------------------------
//  ROM / address map / machine config
//-------------------------------------------------

ROM_START( xerox820_16_8 )
	ROM_REGION( 0x1000, I8086_TAG, 0 )
	ROM_LOAD( "8086.u33", 0x0000, 0x1000, CRC(ee49e3dc) SHA1(a5f20c74fc53f9d695d8894534ab69a39e2c38d8) )
ROM_END

const tiny_rom_entry *xerox820_16_8_device::device_rom_region() const
{
	return ROM_NAME( xerox820_16_8 );
}

void xerox820_16_8_device::mem_map(address_map &map)
{
	// Per the 16/8 Technical Reference: the 8086 board has 128K RAM, expandable to
	// 256K with a daughter board.  Model the 256K maximum.
	map(0x00000, 0x3ffff).ram();                 // 256K TPA / program RAM (128K base + 128K daughter)
	map(0xf0000, 0xfefff).ram();                 // resident OS + shared-mailbox RAM (8086 runs CS=0xF400); the Z80 windows 0xF4000+
	map(0xff000, 0xfffff).rom().region(I8086_TAG, 0); // 8086 boot ROM (signature 0x0909 at 0xFFFFC; LOAD86 reads it at Z80 window 0xBFFC)
}

void xerox820_16_8_device::device_add_mconfig(machine_config &config)
{
	I8086(config, m_i8086, 4770000);
	m_i8086->set_addrmap(AS_PROGRAM, &xerox820_16_8_device::mem_map);
	m_i8086->set_irq_acknowledge_callback(FUNC(xerox820_16_8_device::dbell_inta));
}


//-------------------------------------------------
//  device_start / device_reset
//-------------------------------------------------

void xerox820_16_8_device::device_start()
{
	m_i8086->space(AS_PROGRAM).specific(m_prog);

	// The Z80 controls the coprocessor through I/O ports A0/A1, which the board
	// installs into the host Z80 I/O space (Tech Ref p165):
	//   IN  A0        -> Stop 8086   (LOAD86 quiesces it before its shared-RAM test)
	//   IN  A1        -> Start 8086  (LOAD86 restarts it after the test)
	//   OUT A0, D7=1  -> Lock 8086
	//   OUT A1        -> Z80->8086 doorbell (counted INT, see dbell_w)
	// A2/A3 are defined but unused by the loader/monitor; A4-AF are reserved.
	address_space &io = m_slot->maincpu().space(AS_IO);
	io.install_read_handler(0xa0, 0xa0, 0, 0xff00, 0, emu::rw_delegate(*this, FUNC(xerox820_16_8_device::stop_r)));
	io.install_write_handler(0xa0, 0xa0, 0, 0xff00, 0, emu::rw_delegate(*this, FUNC(xerox820_16_8_device::lock_w)));
	io.install_read_handler(0xa1, 0xa1, 0, 0xff00, 0, emu::rw_delegate(*this, FUNC(xerox820_16_8_device::start_r)));
	io.install_write_handler(0xa1, 0xa1, 0, 0xff00, 0, emu::rw_delegate(*this, FUNC(xerox820_16_8_device::dbell_w)));

	save_item(NAME(m_dbell_count));
	save_item(NAME(m_rmw_addr));
	save_item(NAME(m_rmw_val));
	save_item(NAME(m_rmw_cycles));
}

void xerox820_16_8_device::device_reset()
{
	// The 8086 is held in RESET until the Z80 releases it (IN A1 = Start).  Each
	// Start runs it fresh from 0xFFFF0 (a full POST), so LOAD86 can clobber the
	// shared RAM with its comm-test + stage commands while the 8086 is held, then
	// Start it to re-init its dispatcher TCBs cleanly.
	m_i8086->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_i8086->set_input_line(0, CLEAR_LINE); // empty the doorbell latch
	m_dbell_count = 0;
	m_rmw_addr = ~offs_t(0);
}


//-------------------------------------------------
//  A0/A1 control ports
//-------------------------------------------------

uint8_t xerox820_16_8_device::stop_r()
{
	m_i8086->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // Stop = assert/hold RESET
	return 0xff;
}

void xerox820_16_8_device::lock_w(uint8_t data)
{
	if (BIT(data, 7))
		m_i8086->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // Lock = hold in RESET
}

uint8_t xerox820_16_8_device::start_r()
{
	// Start = release RESET -> fresh POST from 0xFFFF0.  The POST writes its status
	// word at 0xF861E within microseconds; LOAD86's presence test relies on winning
	// that race (its first poll ~77us after the IN A1 must see a post-restart value).
	// Under MAME's coarse default quantum the Z80 can run a whole timeslice ahead and
	// read stale pre-restart contents, mis-classifying the 8086 as absent.  End the
	// Z80's slice at the release and interleave finely while the POST runs.
	m_i8086->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	machine().scheduler().add_quantum(attotime::from_usec(10), attotime::from_msec(100));
	m_slot->maincpu().abort_timeslice();
	return 0xff;
}

// 16/8 Z80->8086 doorbell.  Each Z80 OUT (A1) is latched as one interrupt request:
// INTR asserts (no PIC; the INTA cycle reads the floating bus = vector 0xFF -> the
// boot ROM's handler at 0xFFCEF, "inc word [4612]"), and the 8086's INTA pops one
// ring off the latch, releasing INTR only when no rings remain.  The count matters:
// the mailbox protocol counts doorbells in [4612], so two rings must deliver two INTs
// even when the second arrives while the 8086 still has IF=0 from the first (a plain
// held line would merge them - a lost wakeup that hangs the 86CON console mid-output).
void xerox820_16_8_device::dbell_w(uint8_t data)
{
	m_dbell_count++;
	m_i8086->set_input_line(0, ASSERT_LINE);
}

IRQ_CALLBACK_MEMBER(xerox820_16_8_device::dbell_inta)
{
	if (m_dbell_count)
		m_dbell_count--;
	if (m_dbell_count == 0)
		m_i8086->set_input_line(0, CLEAR_LINE);
	return 0xff; // floating bus during INTA
}


//-------------------------------------------------
//  shared RAM window + bus arbiter
//-------------------------------------------------

// The mailbox protocol acquires its lock bytes (mutex 0xF8000, per-device slot bytes
// 0xF8050/0xF8100/...) with an *unlocked* Z80 "sla (hl)" racing the 8086's "lock shl"
// (acquire) and "lock mov [si],80h" (release) -- sound on hardware because the
// shared-RAM arbiter keeps the 8086 off the bus between the read and write halves of
// the Z80's read-modify-write.  MAME's Z80 core is resumable mid-instruction, so a
// timeslice can end between those halves and the 8086 can release a slot in the gap;
// the Z80's stale write-back of 0x00 then orphans the lock -- both CPUs spin forever.
//
// Modelled as a load-locked/store-conditional reservation, measured in Z80 cycles (a
// Z80 parked mid-instruction accrues none, so the test is immune to scheduling):
// shared_ram_r records {addr, value}; if the matching write of the same RMW
// instruction (same addr, <= 8 Z80 T-states later) finds the location was changed by
// the 8086 in between, the Z80's RMW is serialized *before* the intervening 8086
// store: the stale write-back is dropped and the 8086's newer value stands.
uint8_t xerox820_16_8_device::shared_ram_r(offs_t offset)
{
	uint8_t const data = m_prog.read_byte(0xf8000 + offset);

	if (!machine().side_effects_disabled())
	{
		m_rmw_addr = offset;
		m_rmw_val = data;
		m_rmw_cycles = m_slot->maincpu().total_cycles();
	}

	return data;
}

void xerox820_16_8_device::shared_ram_w(offs_t offset, uint8_t data)
{
	if (offset == m_rmw_addr && uint64_t(m_slot->maincpu().total_cycles()) - m_rmw_cycles <= 8)
	{
		m_rmw_addr = ~offs_t(0); // reservation consumed
		if (m_prog.read_byte(0xf8000 + offset) != m_rmw_val)
			return; // torn RMW: an 8086 store slipped between the halves; Z80 serializes first
	}

	m_prog.write_byte(0xf8000 + offset, data);
}


//**************************************************************************
//  EM-II / DISK EXPANSION MODULE (WD1002-05) CARD
//**************************************************************************
//
//  The 16/8 5.25" "rigid disk unit" (DEM / "Expansion Box II") plugs into the
//  same coprocessor slot.  It IS the 16/8 8086 board plus a Western Digital
//  WD1002-05 disk controller (WD1010 Winchester+floppy combo) and the box's own
//  boot ROM (537p3682).  So the card extends the 8086 board and installs, into
//  the host Z80 I/O space at start, the disk side:
//    IN  A6      -> box id  (0x21 = rgd5: one ST-506 rigid + one 5.25" floppy;
//                            0x20 = flpy5: floppy-only)
//    A8-AF       -> the WD1002-05 task file (A8 data, A9 error, AA sec-count,
//                   AB sec-number, AC/AD cyl, AE SDH size/head/drive-select,
//                   AF status(r)/command(w))
//    IN  B0-BF   -> the box ROM, paged 256 bytes per port (B register = index)
//  The v5.0 monitor's ddskld reads A6, pages the driver out of B0-BF, validates
//  its 55AA header, copies it to $F380 and runs it; the driver then talks
//  register-level to A8-AF.  SELECT dispatches drive units 0-3 -> floppy, 4-7 ->
//  ST-506 rigid (SDH drive-select bits 3-4 == 11 => the 5.25" floppy).

#define EMII_TAG "emii"

class xerox820_emii_device : public xerox820_16_8_device
{
protected:
	xerox820_emii_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t box_id);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	uint8_t box_id_r();              // IN  A6    : expansion-box id
	uint8_t rom_r(offs_t offset);    // IN  B0-BF : paged box ROM (537p3682)

	required_device<wd1002_05_device> m_wdc; // the WD1002-05 controller (host task file at A8-AF)
	required_region_ptr<uint8_t> m_box_rom;  // the 537p3682 box ROM, paged at B0-BF

	const uint8_t m_box_id;          // 0x21 rgd5 (rigid present) / 0x20 flpy5 (floppy only)
};

// the two box complements: one box ROM (537p3682), the id at A6 only records whether
// a rigid is present (the v5.0 monitor's ddskld loads the driver identically for both).
class xerox820_emii_rgd5_device : public xerox820_emii_device
{
public:
	xerox820_emii_rgd5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)
		: xerox820_emii_device(mconfig, XEROX820_EMII_RGD5, tag, owner, clock, 0x21) { }
};

class xerox820_emii_flpy5_device : public xerox820_emii_device
{
public:
	xerox820_emii_flpy5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)
		: xerox820_emii_device(mconfig, XEROX820_EMII_FLPY5, tag, owner, clock, 0x20) { }
};


xerox820_emii_device::xerox820_emii_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t box_id)
	: xerox820_16_8_device(mconfig, type, tag, owner, clock)
	, m_wdc(*this, "wdc")
	, m_box_rom(*this, EMII_TAG)
	, m_box_id(box_id)
{
}


ROM_START( xerox820_emii )
	ROM_REGION( 0x1000, I8086_TAG, 0 )
	ROM_LOAD( "8086.u33", 0x0000, 0x1000, CRC(ee49e3dc) SHA1(a5f20c74fc53f9d695d8894534ab69a39e2c38d8) )

	ROM_REGION( 0x1000, EMII_TAG, 0 ) // the genuine EM-II (WD1002-05) box ROM
	ROM_LOAD( "537p3682.bin", 0x0000, 0x1000, CRC(88f30a00) SHA1(54d205e5a7ed80feb7cd06eddc863110fc655dde) )
ROM_END

const tiny_rom_entry *xerox820_emii_device::device_rom_region() const
{
	return ROM_NAME( xerox820_emii );
}

void xerox820_emii_device::device_add_mconfig(machine_config &config)
{
	xerox820_16_8_device::device_add_mconfig(config); // the 8086 + its memory map
	WD1002_05(config, m_wdc); // the WD1002-05 Winchester/Floppy controller (ST-506 rigid CHD + 5.25" floppy)
}

void xerox820_emii_device::device_start()
{
	xerox820_16_8_device::device_start(); // the 8086 + its A0/A1 control ports

	// install the disk side into the host Z80 I/O space (A6 id, A8-AF task file, B0-BF box ROM)
	address_space &io = m_slot->maincpu().space(AS_IO);
	io.install_read_handler(0xa6, 0xa6, 0, 0xff00, 0, emu::rw_delegate(*this, FUNC(xerox820_emii_device::box_id_r)));
	io.install_readwrite_handler(0xa8, 0xaf, 0, 0xff00, 0,
			emu::rw_delegate(*m_wdc, FUNC(wd1002_05_device::read)),
			emu::rw_delegate(*m_wdc, FUNC(wd1002_05_device::write)));
	io.install_read_handler(0xb0, 0xbf, 0, 0, 0xff00, emu::rw_delegate(*this, FUNC(xerox820_emii_device::rom_r)));
}

uint8_t xerox820_emii_device::box_id_r()
{
	return m_box_id; // expbx: 0x21 rgd5 / 0x20 flpy5
}

// box ROM paged at B0-BF: the port low nibble selects one of 16 256-byte blocks,
// the B register (high byte of the I/O address, via .select) walks the offset.
uint8_t xerox820_emii_device::rom_r(offs_t offset)
{
	const uint8_t low = 0xb0 + (offset & 0x0f); // port low byte ($B0-$BF)
	const uint8_t b   = (offset >> 8) & 0xff;    // B register = the ROM index ddskld walks
	const unsigned idx = (low - 0xb0) * 256 + b;
	return (idx < m_box_rom.bytes()) ? m_box_rom[idx] : 0xff;
}

// The host task file ($A8-$AF) is the WD1002-05 controller itself (machine/wd1002_05) --
// the card just maps the Z80 I/O ports onto m_wdc->read/write.  The rigid (ST-506 CHD) and
// the 5.25" floppy both hang off that device; the SDH register selects which.

} // anonymous namespace


//**************************************************************************
//  SLOT
//**************************************************************************

DEFINE_DEVICE_TYPE(XEROX820_COPRO_SLOT, xerox820_copro_slot_device, "xerox820_copro_slot", "Xerox 820-II coprocessor slot")

device_xerox820_copro_card_interface::device_xerox820_copro_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "xerox820copro")
	, m_slot(dynamic_cast<xerox820_copro_slot_device *>(device.owner()))
{
}

xerox820_copro_slot_device::xerox820_copro_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XEROX820_COPRO_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_xerox820_copro_card_interface>(mconfig, *this)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_card(nullptr)
{
}

void xerox820_copro_slot_device::device_start()
{
	m_card = get_card_device();
}


DEFINE_DEVICE_TYPE_PRIVATE(XEROX820_16_8, device_xerox820_copro_card_interface, xerox820_16_8_device, "xerox820_16_8", "Xerox 16/8 8086 coprocessor board")

DEFINE_DEVICE_TYPE_PRIVATE(XEROX820_EMII_RGD5,  device_xerox820_copro_card_interface, xerox820_emii_rgd5_device,  "xerox820_emii_rgd5",  "Xerox 16/8 EM-II, rigid+floppy (8086 + WD1002-05)")
DEFINE_DEVICE_TYPE_PRIVATE(XEROX820_EMII_FLPY5, device_xerox820_copro_card_interface, xerox820_emii_flpy5_device, "xerox820_emii_flpy5", "Xerox 16/8 EM-II, floppy only (8086 + WD1002-05)")


//**************************************************************************
//  SLOT OPTIONS
//**************************************************************************

void xerox820_copro_cards(device_slot_interface &device)
{
	device.option_add("16_8", XEROX820_16_8);
	device.option_add("emii_rgd5",  XEROX820_EMII_RGD5);  // the 16/8 5.25" rigid-disk unit (DEM): 8086 + WD1002-05 + ST-506 rigid
	device.option_add("emii_flpy5", XEROX820_EMII_FLPY5); // the 16/8 5.25" floppy-only DEM:        8086 + WD1002-05
}
