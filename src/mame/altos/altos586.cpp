// license:BSD-2-Clause
// copyright-holders:Lubomir Rintel

/***************************************************************************

    Altos 586 computer emulation

    Work in progress. The current goal is to iron our the overall flaws
    caused by my inexperience with C++ and MAME. The ultimate goal is to
    make this complete enough for XENIX to run. I guess at that point
    it would imply CP/M and OASIS working too.

    At this point I've not tried to boot anything but the diags floppy,
    and it's unlikely to work.

    The tests on the diags floppy work well, including serial, clock,
    memory management, hard disk & floppy diags and maintenance.
    What the diags disk doesn't exercise are interrupts, access control of
    bus access from IOP/HDC and system call machinery.

    Literature:

    [1] 586T/986T System Reference Manual, P/N 690-15813-002, April 1985
        This describes a slightly different system.
        690-15813-002_Altos_586T_986T_System_Reference_Apr85.pdf

    [2] Notes on the Altos 586 Computer & Firmware disassembly
        https://github.com/lkundrak/altos586/

***************************************************************************/

#include "emu.h"

#include "altos586_hdc.h"

#include "bus/rs232/rs232.h"

#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"

#include "imagedev/floppy.h"

#include "machine/clock.h"
#include "machine/mm58167.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"

// TODO: Should this be a separate device? It is on a same board.
// I've split this so that I've got two device_memory_interface-s --
// the board object provides the maps and unmanaged memory and I/O
// spaces, while the MMU provides the managed ones for the bus
// peripherals (IOP and HDC) via its AS_PROGRAM/AS_IO spaces and
// cpu_{mem,io}_{r,w} routines for the access from the main CPU.
// Could there be a better way to structure this?
class altos586_mmu_device : public device_t, public device_memory_interface
{
public:
	using violation_delegate = device_delegate<void ()>;

	altos586_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	altos586_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&board_tag)
		: altos586_mmu_device(mconfig, tag, owner, clock)
	{
		m_board.set_tag(std::forward<T>(board_tag));
	}

	template <typename... T> void set_violation_callback(T &&... args) { m_violation_callback.set(std::forward<T>(args)...); }
	auto syscall_handler() { return m_syscall_handler.bind(); }

	// Managed access from the main board CPU
	u16 cpu_io_r(offs_t offset, u16 mem_mask);
	void cpu_io_w(offs_t offset, u16 data, u16 mem_mask);
	u16 cpu_mem_r(offs_t offset, u16 mem_mask);
	void cpu_mem_w(offs_t offset, u16 data, u16 mem_mask);

	// MMU control registers
	u16 err_addr1_r(offs_t offset) { return m_err_addr1; }
	u16 err_addr2_r(offs_t offset) { return m_err_addr2; }
	u16 clr_violation_r(offs_t offset) { return m_violation = 0xffff; }
	void clr_violation_w(offs_t offset, u16 data) { clr_violation_r(offset); }
	u16 violation_r(offs_t offset) { return m_violation; /* | jumpers */ }

	// Page table SRAM (should this be a ram device instead?
	u16 map_ram_r(offs_t offset) { return m_map_ram[offset & 0xff]; }
	void map_ram_w(offs_t offset, u16 data, u16 mem_mask) { m_map_ram[offset & 0xff] = data; }

	// Control/Mode
	void set_system_mode();
	void check_user_mode();
	void clr_syscall_w(offs_t offset, u8 data);
	u16 control_r(offs_t offset);
	void control_w(offs_t offset, u16 data);
	void cpu_if_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

private:
	// Managed memory access from the bus (IOP and HDC)
	void bus_mem(address_map &map) ATTR_COLD;
	u16 bus_mem_r(offs_t offset, u16 mem_mask);
	void bus_mem_w(offs_t offset, u16 data, u16 mem_mask);

	// Managed I/O access from the bus (IOP and HDC)
	void bus_io(address_map &map) ATTR_COLD;
	u16 bus_io_r(offs_t offset, u16 mem_mask);
	void bus_io_w(offs_t offset, u16 data, u16 mem_mask);

	// Memory translation and address checking
	offs_t phys_mem_addr(offs_t offset);
	void signal_violation(u16 violation_bits);
	bool check_mem_violation(offs_t offset, int access_bit, int access_bit_set, u16 violation_bits);

	violation_delegate m_violation_callback;
	devcb_write_line m_syscall_handler;

	// Access to board's unmanaged address spaces
	required_device<device_memory_interface> m_board;
	memory_access<20, 1, 0, ENDIANNESS_LITTLE>::specific m_mem;
	memory_access<16, 1, 0, ENDIANNESS_LITTLE>::specific m_io;

	// Configuration for managed address spaces we provide
	address_space_config m_program_config;
	address_space_config m_io_config;

	// User or System mode
	bool m_user;
	bool m_cpu_if;
	u16 m_control;

	u16 m_err_addr1;
	u16 m_err_addr2;

	enum : u16 {
		INVALID_INSN        = 0x0001,   // Invalid Instruction
		END_OF_STACK        = 0x0008,   // End of Stack Warning
		SYS_W_VIOLATION     = 0x0010,   // System write Violation
		USER_W_VIOLATION    = 0x0080,   // User Mode Write Violation
		IOP_W_VIOLATION     = 0x0400,   // I/O Processor Write Violation
		USER_ACC_VIOLATION  = 0x0800,   // User Mode Access Violation
	};
	u16 m_violation;

	enum {
		IOP_W           = 11,       // Allow I/O Processor Write
		SYS_W           = 12,       // Allow System Write
		STACK_BOUND     = 13,       // Stack Boundary Page
		USER_ACC        = 14,       // Allow User Access
		USER_W          = 15,       // Allow User Write
	};
	u16 m_map_ram[256];
};

DEFINE_DEVICE_TYPE(ALTOS586_MMU, altos586_mmu_device, "altos586_mmu", "ALTOS586 MMU")

altos586_mmu_device::altos586_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ALTOS586_MMU, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_violation_callback(*this)
	, m_syscall_handler(*this)
	, m_board(*this, finder_base::DUMMY_TAG)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 20, 0, address_map_constructor(FUNC(altos586_mmu_device::bus_mem), this))
	, m_io_config("io", ENDIANNESS_LITTLE, 16, 16, 0, address_map_constructor(FUNC(altos586_mmu_device::bus_io), this))
{
}

u16 altos586_mmu_device::cpu_mem_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled() && m_user && check_mem_violation(offset, USER_ACC, 1, USER_ACC_VIOLATION)) {
		return 0xffff;
	} else {
		return m_mem.read_word(phys_mem_addr(offset), mem_mask);
	}
}

void altos586_mmu_device::cpu_mem_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (m_user) {
		if (check_mem_violation(offset, USER_W, 1, USER_W_VIOLATION))
			return;
		if (check_mem_violation(offset, USER_ACC, 1, USER_ACC_VIOLATION))
			return;
	} else {
		if (check_mem_violation(offset, SYS_W, 1, SYS_W_VIOLATION))
			return;
	}

	if ((offset & 0x7ff) < 0x40) {
		// Check the stack boundary watermark so that XENIX could map
		// some more memory, but don't abort the write cycle.
		check_mem_violation(offset, STACK_BOUND, 0, END_OF_STACK);
	}

	m_mem.write_word(phys_mem_addr(offset), data, mem_mask);
}

u16 altos586_mmu_device::cpu_io_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled() && m_user) {
		m_syscall_handler(ASSERT_LINE);
		return 0xffff;
	} else {
		return m_io.read_word(offset << 1, mem_mask);
	}
}

void altos586_mmu_device::cpu_io_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (m_user) {
		// XENIX user programs do outb 8800 to trigger the interrupt
		// TODO: I have not tested if I got syscall handling right.
		m_syscall_handler(ASSERT_LINE);
	} else {
		m_io.write_word(offset << 1, data, mem_mask);
	}
}

void altos586_mmu_device::bus_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(altos586_mmu_device::bus_mem_r), FUNC(altos586_mmu_device::bus_mem_w));
}

u16 altos586_mmu_device::bus_mem_r(offs_t offset, u16 mem_mask)
{
	return m_mem.read_word(phys_mem_addr(offset), mem_mask);
}

void altos586_mmu_device::bus_mem_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (check_mem_violation(offset, IOP_W, 1, IOP_W_VIOLATION)) {
		return;
	} else {
		m_mem.write_word(phys_mem_addr(offset), data, mem_mask);
	}
}

void altos586_mmu_device::bus_io(address_map &map)
{
	// As per the manual, I/O addresses up to 0x3ff are available only to CPU, not other bus masters
	map(0x0400, 0xffff).rw(FUNC(altos586_mmu_device::bus_io_r), FUNC(altos586_mmu_device::bus_io_w));
}

u16 altos586_mmu_device::bus_io_r(offs_t offset, u16 mem_mask)
{
	return m_io.read_word(offset, mem_mask);
}

void altos586_mmu_device::bus_io_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_io.write_word(offset, data, mem_mask);
}

void altos586_mmu_device::device_start()
{
	set_system_mode();
	m_cpu_if = false;

	m_board->space(AS_PROGRAM).specific(m_mem);
	m_board->space(AS_IO).specific(m_io);

	m_violation_callback.resolve_safe();

	save_item(NAME(m_user));
	save_item(NAME(m_cpu_if));
	save_item(NAME(m_control));
	save_item(NAME(m_err_addr1));
	save_item(NAME(m_err_addr2));
	save_item(NAME(m_violation));
	save_item(NAME(m_map_ram));
}

device_memory_interface::space_config_vector altos586_mmu_device::memory_space_config() const
{
	return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO, &m_io_config) };
}

offs_t altos586_mmu_device::phys_mem_addr(offs_t offset)
{
	return ((m_map_ram[offset >> 11] & 0xff) << 12) | ((offset & 0x7ff) << 1);
}

void altos586_mmu_device::signal_violation(u16 violation_bits)
{
	u16 old_violation = m_violation;
	m_violation &= ~violation_bits;
	if (old_violation == 0xffff && BIT(m_control, 2)) {
		// TODO: Is this (user mode request) bit actually cleared on real hw?
		// Is this the right place to do that?
		m_control &= ~1;
		m_violation_callback();
	}
}

bool altos586_mmu_device::check_mem_violation(offs_t offset, int access_bit, int access_bit_set, u16 violation_bits)
{
	u16 acc = m_map_ram[offset >> 11];

	if (BIT(acc, access_bit) == access_bit_set) {
		return false;
	} else {
		if (m_violation == 0xffff) {
			// TODO: Would it be less clumsy to move setting
			// of m_err_addr* into signal_violation()?
			m_err_addr1 = offset << 1;
			m_err_addr2 = (offset >> 3) & 0xf000;

			if (m_user)
				m_err_addr2 |= 0x0100;
			// TODO: Warm start bit. When is it set?
			m_err_addr2 |= 0x0200;
			if (BIT(m_control, 2))
				m_err_addr2 |= 0x0800; // NMI enabled
		}
		signal_violation(violation_bits);
		return true;
	}
}

void altos586_mmu_device::set_system_mode()
{
	m_user = false;
}

void altos586_mmu_device::check_user_mode()
{
	if (m_cpu_if && BIT(m_control, 0))
		m_user = true;
}

void altos586_mmu_device::clr_syscall_w(offs_t offset, u8 data)
{
	m_syscall_handler(CLEAR_LINE);
}

u16 altos586_mmu_device::control_r(offs_t offset)
{
	// TODO: Is this register actually readable?
	return m_control;
}

void altos586_mmu_device::control_w(offs_t offset, u16 data)
{
	m_control = data;
	check_user_mode();
}

void altos586_mmu_device::cpu_if_w(int state)
{
	if (state == ASSERT_LINE) {
		m_cpu_if = true;
		check_user_mode();
	} else if (m_user) {
		if (m_violation == 0xffff) {
			m_err_addr1 = 0;
			m_err_addr2 = 0;
		}
		signal_violation(INVALID_INSN);
	}
}

namespace {

class altos586_state : public driver_device, public device_memory_interface {
public:
	altos586_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, device_memory_interface(mconfig, *this)
		, m_program_config("program", ENDIANNESS_LITTLE, 16, 20, 0, address_map_constructor(FUNC(altos586_state::mem_map), this))
		, m_io_config("io", ENDIANNESS_LITTLE, 16, 16, 0, address_map_constructor(FUNC(altos586_state::io_map), this))
		, m_cpu(*this, "cpu")
		, m_mmu(*this, "mmu")
		, m_ram(*this, RAM_TAG)
		, m_pic(*this, "pic8259")
		, m_pit(*this, "pit")
		, m_iop(*this, "iop")
		, m_fdc(*this, "fd1797")
		, m_floppy(*this, "fd1797:%u", 0)
	{
	}

	void altos586(machine_config &config);

protected:
	// driver_device implementation
	virtual void machine_start() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

private:
	void mmu_violation();

	// IOP interfacing.
	void iop_attn_w(offs_t offset, u16 data);
	u8 hostram_r(offs_t offset);
	void hostram_w(offs_t offset, u8 data);
	void hiaddr_w(u8 data);
	u8 pio_pa_r(offs_t offset);
	void pio_pa_w(offs_t offset, u8 data);
	void pio_pb_w(offs_t offset, u8 data);

	IRQ_CALLBACK_MEMBER(inta_cb);

	// Maps for the 8086 & Z80 processors
	void cpu_mem(address_map &map) ATTR_COLD;
	void cpu_io(address_map &map) ATTR_COLD;
	void iop_mem(address_map &map) ATTR_COLD;
	void iop_io(address_map &map) ATTR_COLD;

	// Maps and physical spaces used by the MMU
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	address_space_config m_program_config;
	address_space_config m_io_config;

	required_device<i8086_cpu_device> m_cpu;
	required_device<altos586_mmu_device> m_mmu;
	required_device<ram_device> m_ram;
	required_device<pic8259_device> m_pic;
	required_device<pit8254_device> m_pit;
	required_device<z80_device> m_iop;
	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	u8 m_hiaddr;

	memory_access<20, 1, 0, ENDIANNESS_LITTLE>::specific m_mmu_mem;
};

void altos586_state::mem_map(address_map &map)
{
	// ROM needs to be there for IOP bootup
	map(0xfc000, 0xfffff).rom().region("bios", 0);
}

void altos586_state::io_map(address_map &map)
{
	map(0x0040, 0x0047).w(m_mmu, FUNC(altos586_mmu_device::clr_syscall_w));
	// TODO: Manual reads: 0048H to 004FH Channel attention 0
	// (reserved for future bus master channel attentions)
	map(0x0050, 0x0057).w(FUNC(altos586_state::iop_attn_w));
	map(0x0058, 0x005f).rw(m_mmu, FUNC(altos586_mmu_device::control_r), FUNC(altos586_mmu_device::control_w));
	map(0x0060, 0x0067).r(m_mmu, FUNC(altos586_mmu_device::err_addr2_r));
	map(0x0068, 0x006F).r(m_mmu, FUNC(altos586_mmu_device::err_addr1_r));
	map(0x0070, 0x0077).rw(m_mmu, FUNC(altos586_mmu_device::clr_violation_r), FUNC(altos586_mmu_device::clr_violation_w));
	map(0x0078, 0x007f).r(m_mmu, FUNC(altos586_mmu_device::violation_r));

	// These are wired funnily
	map(0x0080, 0x00ff).lrw16(
			NAME([this] (offs_t offset) { return m_pic->read(~offset & 1); }),
			NAME([this] (offs_t offset, u16 data) { m_pic->write(~offset & 1, data); }));
	map(0x0100, 0x01ff).lrw16(
			NAME([this] (offs_t offset) { return m_pit->read(~offset) << 8; }),
			NAME([this] (offs_t offset, u16 data) { m_pit->write(~offset, data >> 8); }));

	map(0x0200, 0x03ff).rw(m_mmu, FUNC(altos586_mmu_device::map_ram_r), FUNC(altos586_mmu_device::map_ram_w));

	// Addresses 0400H to FFFFH are, unlike the above, accessible from
	// other bus masters than the main CPU. Peripherals are expected to
	// hook there. Documented as "Reserved for system bus I/O"

	// Bus peripherals
	map(0xff00, 0xff01).w("hdc", FUNC(altos586_hdc_device::attn_w));
}

void altos586_state::cpu_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(m_mmu, FUNC(altos586_mmu_device::cpu_mem_r), FUNC(altos586_mmu_device::cpu_mem_w));
	// ROM is always mapped at these addresses
	map(0xfc000, 0xfffff).rom().region("bios", 0);
}

void altos586_state::cpu_io(address_map &map)
{
	map(0x0000, 0xffff).rw(m_mmu, FUNC(altos586_mmu_device::cpu_io_r), FUNC(altos586_mmu_device::cpu_io_w));
}

void altos586_state::iop_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("iop", 0);
	map(0x2000, 0x27ff).ram();
	map(0x8000, 0xffff).rw(FUNC(altos586_state::hostram_r), FUNC(altos586_state::hostram_w));
}

void altos586_state::iop_io(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x00).w(FUNC(altos586_state::hiaddr_w));                      // 0x00 Address Latch
	map(0x20, 0x23).rw("iop_pit0", FUNC(pit8254_device::read), FUNC(pit8254_device::write));    // 0x20 PIT 0
	map(0x24, 0x27).rw("iop_pit1", FUNC(pit8254_device::read), FUNC(pit8254_device::write));    // 0x24 PIT 1
	map(0x28, 0x2b).rw("iop_sio0", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)); // 0x28 SIO 0
	map(0x2c, 0x2f).rw("iop_sio1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)); // 0x2C SIO 1
	map(0x30, 0x33).rw("iop_sio2", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)); // 0x30 SIO 2
	map(0x34, 0x37).rw("iop_pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));   // 0x34 PIO
	map(0x38, 0x3b).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));       // 0x38 FDC
	map(0x3c, 0x3f).rw("iop_dma", FUNC(z80dma_device::read), FUNC(z80dma_device::write));       // 0x3C DMA
	map(0x40, 0x40).noprw();                                    // 0x40 DMA - Clear carrier sense and parity error bit
	map(0x80, 0x9f).rw("iop_rtc", FUNC(mm58167_device::read), FUNC(mm58167_device::write));     // 0x80 RTC - Counter - thousandths of seconds
													// 0x60 586T Generate MULTIBUS interrupt
}

void altos586_state::iop_attn_w(offs_t offset, u16 data)
{
	m_iop->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_iop->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

u8 altos586_state::hostram_r(offs_t offset)
{
	return m_mmu_mem.read_byte((m_hiaddr << 15) | (offset & 0x7fff));
}

void altos586_state::hostram_w(offs_t offset, u8 data)
{
	m_mmu_mem.write_byte((m_hiaddr << 15) | (offset & 0x7fff), data);
}

void altos586_state::hiaddr_w(u8 data)
{
	m_hiaddr = data;
}

u8 altos586_state::pio_pa_r(offs_t offset)
{
	u8 data = 0x00;

	data |= 0 << 0; // Positions 2-4 in E1 jumper (present on my machine, pulling to GND)
	data |= 1 << 1; // Positions 1-2 in E1 jumper (absent, pull-up resistor)
	data |= 1 << 2; // Positions 7-8 in E1 jumper (absent, pull-up resistor)
	data |= 0 << 3; // Pulled to GND
	data |= 1 << 4; // Pin 18 of 74LS74 at position G9
	data |= 0 << 5; // TODO: pin 22 (IR4) of 8259 PIC. Remember what we set it to.
	data |= 1 << 6; // Pin 5 of 74LS38 at position H9
	data |= m_fdc->intrq_r() << 7;
	return data;
}

void altos586_state::pio_pa_w(offs_t offset, u8 data)
{
	// TODO: Is there an interrerrupt ack pin somewhere? PA4 or PA6 perhaps?
	m_pic->ir4_w(BIT(data, 5));
}

void altos586_state::pio_pb_w(offs_t offset, u8 data)
{
	floppy_image_device *floppy;

	if (!BIT(data, 3))
		floppy = m_floppy[0]->get_device();
	else if (!BIT(data, 4))
		floppy = m_floppy[1]->get_device();
	else
		floppy = nullptr;

	if (floppy) {
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 5));
	}

	m_fdc->set_floppy(floppy);
	m_fdc->dden_w(BIT(data, 6));
}

IRQ_CALLBACK_MEMBER(altos586_state::inta_cb)
{
	m_mmu->set_system_mode();
	return m_pic->acknowledge();
}

void altos586_state::machine_start()
{
	u8 *romdata = memregion("bios")->base();
	int romlen = memregion("bios")->bytes();

	space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());

	// The address lines to the ROMs are reversed
	std::reverse(romdata, romdata + romlen);

	m_mmu->space(AS_PROGRAM).specific(m_mmu_mem);

	save_item(NAME(m_hiaddr));
}

device_memory_interface::space_config_vector altos586_state::memory_space_config() const
{
	// Spaces we provide the MMU
	return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO, &m_io_config) };
}

static void altos586_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

static DEVICE_INPUT_DEFAULTS_START(altos586_terminal)
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void altos586_state::mmu_violation()
{
	m_mmu->set_system_mode();
	m_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void altos586_state::altos586(machine_config &config)
{
	ALTOS586_MMU(config, m_mmu, 0, *this);
	m_mmu->set_violation_callback(FUNC(altos586_state::mmu_violation));
	m_mmu->syscall_handler().set(m_pic, FUNC(pic8259_device::ir0_w));

	I8086(config, m_cpu, 30_MHz_XTAL / 3);
	m_cpu->set_addrmap(AS_PROGRAM, &altos586_state::cpu_mem);
	m_cpu->set_addrmap(AS_IO, &altos586_state::cpu_io);
	m_cpu->set_irq_acknowledge_callback(FUNC(altos586_state::inta_cb));
	m_cpu->if_handler().set(m_mmu, FUNC(altos586_mmu_device::cpu_if_w));

	RAM(config, RAM_TAG).set_default_size("512K").set_extra_options("1M");

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);

	PIT8254(config, m_pit);
	m_pit->set_clk<0>(30_MHz_XTAL/6);
	m_pit->out_handler<0>().set("iop_sio2", FUNC(z80sio_device::rxcb_w));
	m_pit->out_handler<0>().append("iop_sio2", FUNC(z80sio_device::txcb_w));
	m_pit->set_clk<1>(30_MHz_XTAL/6);
	m_pit->set_clk<2>(62'500);
	m_pit->out_handler<2>().set(m_pic, FUNC(pic8259_device::ir1_w));

	Z80(config, m_iop, 8_MHz_XTAL / 2);
	m_iop->set_addrmap(AS_PROGRAM, &altos586_state::iop_mem);
	m_iop->set_addrmap(AS_IO, &altos586_state::iop_io);

	pit8254_device &pit0(PIT8254(config, "iop_pit0", 0));
	pit0.set_clk<0>(30_MHz_XTAL);
	pit0.out_handler<0>().set("iop_sio0", FUNC(z80sio_device::rxca_w));
	pit0.out_handler<0>().append("iop_sio0", FUNC(z80sio_device::txca_w));
	pit0.set_clk<1>(30_MHz_XTAL/6);
	pit0.out_handler<1>().set("iop_sio0", FUNC(z80sio_device::rxcb_w));
	pit0.out_handler<1>().append("iop_sio0", FUNC(z80sio_device::txcb_w));
	pit0.set_clk<2>(30_MHz_XTAL/6);
	pit0.out_handler<2>().set("iop_sio1", FUNC(z80sio_device::rxca_w));
	pit0.out_handler<2>().append("iop_sio1", FUNC(z80sio_device::txca_w));

	pit8254_device &pit1(PIT8254(config, "iop_pit1", 0));
	pit1.set_clk<0>(30_MHz_XTAL/6);
	pit1.out_handler<0>().set("iop_sio1", FUNC(z80sio_device::rxcb_w));
	pit1.out_handler<0>().append("iop_sio1", FUNC(z80sio_device::txcb_w));
	pit1.set_clk<1>(30_MHz_XTAL/6);
	pit1.out_handler<1>().set("iop_sio2", FUNC(z80sio_device::rxca_w));
	pit1.out_handler<1>().append("iop_sio2", FUNC(z80sio_device::txca_w));

	z80sio_device &sio0(Z80SIO(config, "iop_sio0", 8_MHz_XTAL / 2));
	sio0.out_txda_callback().set("rs232_port3", FUNC(rs232_port_device::write_txd));
	sio0.out_dtra_callback().set("rs232_port3", FUNC(rs232_port_device::write_dtr));
	sio0.out_rtsa_callback().set("rs232_port3", FUNC(rs232_port_device::write_rts));
	sio0.out_txdb_callback().set("rs232_port4", FUNC(rs232_port_device::write_txd));
	sio0.out_dtrb_callback().set("rs232_port4", FUNC(rs232_port_device::write_dtr));
	sio0.out_rtsb_callback().set("rs232_port4", FUNC(rs232_port_device::write_rts));
	sio0.out_int_callback().set_inputline(m_iop, INPUT_LINE_IRQ0);
	sio0.set_cputag(m_iop);

	rs232_port_device &rs232_port3(RS232_PORT(config, "rs232_port3", default_rs232_devices, nullptr));
	rs232_port3.rxd_handler().set("iop_sio0", FUNC(z80sio_device::rxa_w));
	rs232_port3.dcd_handler().set("iop_sio0", FUNC(z80sio_device::dcda_w));
	rs232_port3.cts_handler().set("iop_sio0", FUNC(z80sio_device::ctsa_w));

	rs232_port_device &rs232_port4(RS232_PORT(config, "rs232_port4", default_rs232_devices, nullptr));
	rs232_port4.rxd_handler().set("iop_sio0", FUNC(z80sio_device::rxb_w));
	rs232_port4.dcd_handler().set("iop_sio0", FUNC(z80sio_device::dcdb_w));
	rs232_port4.cts_handler().set("iop_sio0", FUNC(z80sio_device::ctsb_w));

	z80sio_device &sio1(Z80SIO(config, "iop_sio1", 8_MHz_XTAL / 2));
	sio1.out_txda_callback().set("rs232_port1", FUNC(rs232_port_device::write_txd));
	sio1.out_dtra_callback().set("rs232_port1", FUNC(rs232_port_device::write_dtr));
	sio1.out_rtsa_callback().set("rs232_port1", FUNC(rs232_port_device::write_rts));
	sio1.out_txdb_callback().set("rs232_port2", FUNC(rs232_port_device::write_txd));
	sio1.out_dtrb_callback().set("rs232_port2", FUNC(rs232_port_device::write_dtr));
	sio1.out_rtsb_callback().set("rs232_port2", FUNC(rs232_port_device::write_rts));
	sio1.out_int_callback().set_inputline(m_iop, INPUT_LINE_IRQ0);
	sio1.set_cputag(m_iop);

	rs232_port_device &rs232_port1(RS232_PORT(config, "rs232_port1", default_rs232_devices, "terminal"));
	rs232_port1.rxd_handler().set("iop_sio1", FUNC(z80sio_device::rxa_w));
	rs232_port1.dcd_handler().set("iop_sio1", FUNC(z80sio_device::dcda_w));
	rs232_port1.cts_handler().set("iop_sio1", FUNC(z80sio_device::ctsa_w));
	rs232_port1.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(altos586_terminal));

	rs232_port_device &rs232_port2(RS232_PORT(config, "rs232_port2", default_rs232_devices, nullptr));
	rs232_port2.rxd_handler().set("iop_sio1", FUNC(z80sio_device::rxb_w));
	rs232_port2.dcd_handler().set("iop_sio1", FUNC(z80sio_device::dcdb_w));
	rs232_port2.cts_handler().set("iop_sio1", FUNC(z80sio_device::ctsb_w));

	z80sio_device &sio2(Z80SIO(config, "iop_sio2", 8_MHz_XTAL/2));
	sio2.out_txda_callback().set("rs232_port5", FUNC(rs232_port_device::write_txd));
	sio2.out_dtra_callback().set("rs232_port5", FUNC(rs232_port_device::write_dtr));
	sio2.out_rtsa_callback().set("rs232_port5", FUNC(rs232_port_device::write_rts));
	sio2.out_txdb_callback().set("rs232_port6", FUNC(rs232_port_device::write_txd));
	sio2.out_dtrb_callback().set("rs232_port6", FUNC(rs232_port_device::write_dtr));
	sio2.out_rtsb_callback().set("rs232_port6", FUNC(rs232_port_device::write_rts));
	sio2.set_cputag(m_iop);

	rs232_port_device &rs232_port5(RS232_PORT(config, "rs232_port5", default_rs232_devices, nullptr));
	rs232_port5.rxd_handler().set("iop_sio2", FUNC(z80sio_device::rxa_w));
	rs232_port5.dcd_handler().set("iop_sio2", FUNC(z80sio_device::dcda_w));
	rs232_port5.cts_handler().set("iop_sio2", FUNC(z80sio_device::ctsa_w));

	rs232_port_device &rs232_port6(RS232_PORT(config, "rs232_port6", default_rs232_devices, nullptr));
	rs232_port6.rxd_handler().set("iop_sio2", FUNC(z80sio_device::rxb_w));
	rs232_port6.dcd_handler().set("iop_sio2", FUNC(z80sio_device::dcdb_w));
	rs232_port6.cts_handler().set("iop_sio2", FUNC(z80sio_device::ctsb_w));

	z80pio_device &pio(Z80PIO(config, "iop_pio", 8_MHz_XTAL / 2));
	pio.in_pa_callback().set(FUNC(altos586_state::pio_pa_r));
	pio.out_pa_callback().set(FUNC(altos586_state::pio_pa_w));
	pio.out_pb_callback().set(FUNC(altos586_state::pio_pb_w));

	FD1797(config, m_fdc, 1'000'000); // TODO: Check clock
	m_fdc->drq_wr_callback().set("iop_dma", FUNC(z80dma_device::rdy_w)).invert();
	FLOPPY_CONNECTOR(config, m_floppy[0], altos586_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats); // TODO: Sound?
	FLOPPY_CONNECTOR(config, m_floppy[1], altos586_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats);

	z80dma_device &dma(Z80DMA(config, "iop_dma", 8_MHz_XTAL / 2));
	dma.in_mreq_callback().set([this] (offs_t offset) { return m_iop->space(AS_PROGRAM).read_byte(offset); });
	dma.out_mreq_callback().set([this] (offs_t offset, u8 data) { m_iop->space(AS_PROGRAM).write_byte(offset, data); });
	dma.in_iorq_callback().set([this] (offs_t offset) { return m_iop->space(AS_IO).read_byte(offset); });
	dma.out_iorq_callback().set([this] (offs_t offset, u8 data) { m_iop->space(AS_IO).write_byte(offset, data); });

	// TODO: The RTC seems to run approx. 2 times slower. Why?
	MM58167(config, "iop_rtc", 32.768_kHz_XTAL);

	// TODO: Could a multibus-like bus interface be implemented?
	// This sits on a such bus, but the "backplane" are two 50-pin
	// ribbon cables w/o a fixed number of slots.
	ALTOS586_HDC(config, "hdc", 30_MHz_XTAL / 3, m_mmu);

	SOFTWARE_LIST(config, "flop_list").set_original("altos586");
}

ROM_START(altos586)
	ROM_REGION16_LE(0x4000, "bios", 0)
	ROMX_LOAD("altos586-v13-g1.rom", 0x0001, 0x1000, CRC(29fdcb40) SHA1(34700603d6034f0ccc599d74432cef332459987b) , ROM_SKIP(1))
	ROMX_LOAD("altos586-v13-g2.rom", 0x0000, 0x1000, CRC(de22003a) SHA1(ce25a45cd4fb0ff63e5064fb74347b978c3a78f1) , ROM_SKIP(1))
	ROM_COPY("bios", 0, 0x2000, 0x2000) // v1.3 Firmware is 2x4K, but the board accepts 2x8K

	ROM_REGION(0x2000, "iop", 0)
	ROM_LOAD("altos586-v56-iop.rom", 0x0000, 0x2000, CRC(411ca183) SHA1(f2af03d361dddbf6aae055e69210c994ead281d8))
ROM_END

} // anonymous namespace

COMP( 1984, altos586, 0, 0, altos586, 0, altos586_state, empty_init, "Altos Computer Systems", "ACS586", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
