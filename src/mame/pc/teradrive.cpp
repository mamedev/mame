// license:BSD-3-Clause
// copyright-holders: Angelo Salese
// thanks-to: Mask of Destiny, Nemesis, Sik
/**************************************************************************************************

Sega Teradrive

IBM PS/2 Model 30 (PS/55 5510Z) + Japanese Sega Mega Drive (TMSS MD1 VA3 or VA4)

References (Teradrive specifics):
- https://www.retrodev.com/blastem/trac/wiki/TeradriveHardwareNotes
- https://plutiedev.com/mirror/teradrive-hardware-info
- https://github.com/RetroSwimAU/TeradriveCode
- https://www.youtube.com/watch?v=yjg3gmTo4WA
- https://www.asahi-net.or.jp/~ds6k-mtng/tera.htm

References (generic MD):
- https://plutiedev.com/cartridge-slot
- https://plutiedev.com/vdp-registers
- https://segaretro.org/Sega_Mega_Drive/VDP_general_usage
- https://segaretro.org/Sega_Mega_Drive/Technical_specifications
- https://gendev.spritesmind.net/forum/viewtopic.php?p=37011#p37011

NOTES (PC side):
- F1 at POST will bring a setup menu;
- F2 (allegedly at DOS/V boot) will dual boot the machine;
- a program named VVCHG switches back and forth between MD and PC, and setup 68k to 10 MHz mode;

NOTES (MD side):
- 16 KiB of Z80 RAM (vs. 8 of stock)
- 128 KiB of VDP RAM (vs. 64)
- 68k can switch between native 7.67 MHz or 10 MHz
- has discrete YM3438 in place of YM2612
- Mega CD expansion port working with DIY extension cable, 32x needs at least a passive cart adapter
- focus 3 in debugger is the current default for MD side
- MAME inability of handling differing refresh rates causes visible tearing in MD screen
  (cfr. koteteik intro). A partial workaround is to use Video mode = composite, so that
  VGA will downclock to ~60 Hz instead.

TODO:
- RAM size always gets detected as 2560K even when it's not (from chipset?);
- Quadtel EMM driver fails recognizing WD76C10 chipset with drv4;
- Cannot HDD format with floppy insthdd.bat, cannot boot from HDD (needs floppy first).
  Attached disk is a WDL-330PS with no geometry info available;
- MD side cart slot, expansion bay and VDP rewrites (WIP);
- TMSS unlock and respective x86<->MD bus grants are sketchy;
- SEGA TERADRIVE テラドライブ ユーザーズマニュアル known to exist (not scanned yet)
- "TIMER FAIL" when exiting from F1 setup menu (keyboard? reset from chipset?);

**************************************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/isa/svga_paradise.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/sms_ctrl/controllers.h"
#include "bus/sms_ctrl/smsctrl.h"
#include "cpu/i86/i286.h"
//#include "cpu/i386/i386.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/ram.h"
#include "machine/sega_md_ioport.h"
#include "machine/wd7600.h"
#include "sound/spkrdev.h"
#include "sound/ymopn.h"
#include "video/ym7101.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

/*
 * ISA16 IBM 79F2661 "bus switch"
 *
 * Motherboard resource also shared with undumped 5510Z Japanese DOS/V
 *
 */

class isa16_ibm_79f2661 : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_ibm_79f2661(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_romdisk_tag(T &&tag) { m_romdisk.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_md_space(T &&tag, int index) { m_md_space.set_tag(std::forward<T>(tag), index); }
	auto tmss_bank_callback() { return m_tmss_bank_cb.bind(); }
	auto reg_1163_read_callback() { return m_reg_1163_read_cb.bind(); }
	auto reg_1163_write_callback() { return m_reg_1163_write_cb.bind(); }
	auto reg_1164_callback() { return m_reg_1164_cb.bind(); }
	auto system_in_callback() { return m_system_in_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_memory_region m_romdisk;
	memory_bank_creator m_rom_window_bank;
	// TODO: may be optional for 5510Z
	required_address_space m_md_space;

	void io_map(address_map &map) ATTR_COLD;
	void md_mem_map(address_map &map) ATTR_COLD;

	void remap(int space_id, offs_t start, offs_t end) override;

	u8 m_rom_bank = 0;
	u8 m_rom_address = 0x0e;
	u8 m_reg_1163 = 0;
	u8 m_reg_1164 = 0;
	u16 m_68k_address = 0;
	bool m_68k_view = false;

	devcb_write8 m_tmss_bank_cb;
	devcb_read8 m_reg_1163_read_cb;
	devcb_write8 m_reg_1163_write_cb;
	devcb_write8 m_reg_1164_cb;
	devcb_read8 m_system_in_cb;
};

DEFINE_DEVICE_TYPE(ISA16_IBM_79F2661, isa16_ibm_79f2661, "isa16_ibm_79f2661", "ISA16 IBM 79F2661 \"bus switch\"")

isa16_ibm_79f2661::isa16_ibm_79f2661(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_IBM_79F2661, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_romdisk(*this, finder_base::DUMMY_TAG)
	, m_rom_window_bank(*this, "rom_window_bank")
	, m_md_space(*this, finder_base::DUMMY_TAG, -1)
	, m_tmss_bank_cb(*this)
	, m_reg_1163_read_cb(*this, 0)
	, m_reg_1163_write_cb(*this)
	, m_reg_1164_cb(*this)
	, m_system_in_cb(*this, 0)
{
}

void isa16_ibm_79f2661::device_start()
{
	set_isa_device();
	m_rom_window_bank->configure_entries(0, 0x100, m_romdisk->base(), 0x2000);

	save_item(NAME(m_rom_bank));
	save_item(NAME(m_rom_address));
	save_item(NAME(m_reg_1163));
	save_item(NAME(m_reg_1164));
	save_item(NAME(m_68k_address));
	save_item(NAME(m_68k_view));
}

void isa16_ibm_79f2661::device_reset()
{
	m_rom_bank = 0;
	m_rom_address = 0x0e;
	m_reg_1163 = 0;
	m_68k_view = false;
	m_reg_1164 = 0;
	m_68k_address = 0;
	m_reg_1163_write_cb(0);
	m_reg_1164_cb(0);
	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0xffff);
}

// TODO: prettier method for ISA16 memory swap
void isa16_ibm_79f2661::md_mem_map(address_map &map)
{
	map(0x0000, 0x1fff).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			const u32 base_address = (offset << 1) + (m_68k_address << 12);
			if (!ACCESSING_BITS_0_7)
			{
				u8 res = m_md_space->read_byte(base_address ^ 1);
				return (u16)(res | (res << 8));
			}
			else if (!ACCESSING_BITS_8_15)
			{
				u8 res = m_md_space->read_byte(base_address);
				return (u16)(res | (res << 8));
			}

			return swapendian_int16(m_md_space->read_word(base_address, mem_mask));
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			const u32 base_address = (offset << 1) + (m_68k_address << 12);
			if (!ACCESSING_BITS_0_7)
				m_md_space->write_byte(base_address ^ 1, data >> 8);
			else if (!ACCESSING_BITS_8_15)
				m_md_space->write_byte(base_address, data);
			else
				m_md_space->write_word(base_address, swapendian_int16(data), mem_mask);
		})
	);
}

void isa16_ibm_79f2661::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		const u32 base_addr = (m_rom_address << 12) | 0xc0000;

		if (m_68k_view)
		{
			m_isa->install_memory(base_addr, base_addr | 0x1fff, *this, &isa16_ibm_79f2661::md_mem_map, 0xffff);
		}
		else
		{
			//printf("%08x %02x\n", base_addr, m_rom_bank);
			m_isa->install_bank(base_addr, base_addr | 0x1fff, m_rom_window_bank);
		}
	}
	if (space_id == AS_IO)
	{
		m_isa->install_device(0x1160, 0x1167, *this, &isa16_ibm_79f2661::io_map);
	}
}

// +$1160 base
//  map(0x1160, 0x1160) romdisk bank * 0x2000, r/w
//  map(0x1161, 0x1161) <undefined>?
//  map(0x1162, 0x1162) romdisk segment start in ISA space (val & 0x1e | 0xc0)
//  map(0x1163, 0x1163) comms and misc handshake bits, partially reflected on 68k $ae0001 register
//  map(0x1164, 0x1164) boot control
//  map(0x1165, 0x1165) switches/bus timeout/TMSS unlock (r/o)
//  map(0x1166, 0x1167) 68k address space select & 0xffffe * 0x2000, r/w
void isa16_ibm_79f2661::io_map(address_map &map)
{
	map(0x00, 0x00).lrw8(
		NAME([this] (offs_t offset) {
			return m_rom_bank;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_rom_bank = data;
			m_rom_window_bank->set_entry(m_rom_bank);
			remap(AS_PROGRAM, 0, 0xfffff);
			m_tmss_bank_cb(data);
		})
	);
	map(0x01, 0x01).lr8(
		NAME([this] (offs_t offset) {
			logerror("%s: $1161 undefined access\n", machine().describe_context());
			return 0xff;
		})
	);

	map(0x02, 0x02).lrw8(
		NAME([this] (offs_t offset) {
			return m_rom_address | 0xc0;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_rom_address = data & 0x1e;
			remap(AS_PROGRAM, 0, 0xfffff);
		})
	);
/*
 * xx-- ---- 68k $ae0001 bits 3-2 (writes from this side)
 * --xx ---- 68k $ae0001 bits 1-0 (reads from this side)
 * ---- xx-- <unknown>
 * ---- --x- Enable 286 ISA memory window (and disables TMSS from 68k)
 * ---- ---x Enable auxiliary TMSS ROM (on 68k space?)
 */
	map(0x03, 0x03).lrw8(
		NAME([this] (offs_t offset) {
			return (m_reg_1163 & 0xcf) | (m_reg_1163_read_cb() & 0x30);
		}),
		NAME([this] (offs_t offset, u8 data) {
			//logerror("$1163: %02x\n", data);
			if (BIT(data, 1) != BIT(m_reg_1163, 1))
			{
				m_68k_view = !!BIT(data, 1);
				remap(AS_PROGRAM, 0, 0xfffff);
			}

			m_reg_1163 = data;
			m_reg_1163_write_cb(data);
		})
	);
/*
 * x--- ---- handshake bit? Clearable by 68k, not by 286
 * -x-- ---- Set on TMSS failure
 * --x- ---- <unknown>
 * ---x ---- <unknown>, if 1 then $1165 returns a stream of bytes
 * ---- x--- dual boot bit
 * ---- -x-- video switch
 * ---- --x- (0) Teradrive HW at 68k $0 (1) cart space at $0
 * ---- ---x (0) 68k assert RESET/286 clear HALT (1) 68k clear RESET/286 assert HALT
 */
	map(0x04, 0x04).lrw8(
		NAME([this] (offs_t offset) {
			return m_reg_1164;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_reg_1164 = data;
			m_reg_1164_cb(data);
		})
	);
/*
 * xx-- ---- <unknown, other bus error?>
 * --x- ---- TMSS unlocked
 * ---- x--- bus timeout on 68k space access from 286
 * ---- -x-- video switch (0) "video" (1) RGB
 * ---- ---x MD/PC switch (0) MD boot (1) PC boot
 */
	map(0x05, 0x05).lr8(
		NAME([this] (offs_t offset) {
			// HACK: MD TMSS never writes `SEGA` from $a14000
			return (1 << 5) | (m_system_in_cb() & 5);
		})
	);
	map(0x06, 0x07).lrw16(
		NAME([this] (offs_t offset) {
			return m_68k_address;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_68k_address);
			m_68k_address &= 0xffffe;
			//if (m_68k_view)
			//  remap(AS_PROGRAM, 0, 0xfffff);
		})
	);
}

/*
 * ISA16 WD90C10
 *
 * On motherboard, VGA ROM is in BIOS region and unmapped on ISA memory
 *
 */

// TODO: really WD90C10
class isa16_wd90c10_romless_device : public isa16_wd90c11_lr_device
{
public:
	isa16_wd90c10_romless_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DEFINE_DEVICE_TYPE(ISA16_WD90C10_ROMLESS,  isa16_wd90c10_romless_device,  "wd90c10_romless",  "Western Digital WD90C10 ROM-less VGA")

// NOTE: it will still try to map a ROM during setup mode
ROM_START( wd90c10_romless )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASE00)
ROM_END

const tiny_rom_entry *isa16_wd90c10_romless_device::device_rom_region() const
{
	return ROM_NAME( wd90c10_romless );
}

isa16_wd90c10_romless_device::isa16_wd90c10_romless_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: isa16_wd90c11_lr_device(mconfig, ISA16_WD90C10_ROMLESS, tag, owner, clock)
{
}

void isa16_wd90c10_romless_device::device_add_mconfig(machine_config &config)
{
	isa16_wd90c11_lr_device::device_add_mconfig(config);
	// unknown source, assume standard NTSC (divided internally)
	// tested in Video mode
	m_vga->set_vclk2(14'318'181);
}


namespace {

class teradrive_state : public driver_device
{
public:
	teradrive_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_x86cpu(*this, "x86cpu")
		, m_chipset(*this, "chipset")
		, m_ram(*this, RAM_TAG)
		, m_isabus(*this, "isabus")
		, m_speaker(*this, "speaker")
		, m_system_in(*this, "SYSTEM_IN")
		, m_md68kcpu(*this, "md68kcpu")
		, m_mdz80cpu(*this, "mdz80cpu")
		, m_mdscreen(*this, "mdscreen")
		, m_tmss_bank(*this, "tmss_bank")
		, m_tmss_view(*this, "tmss_view")
		, m_md_cart(*this, "md_cart")
		, m_md_vdp(*this, "md_vdp")
		, m_opn(*this, "opn")
		, m_md_68k_sound_view(*this, "md_68k_sound_view")
		, m_md_ctrl_ports(*this, { "md_ctrl1", "md_ctrl2", "md_exp" })
		, m_md_ioports(*this, "md_ioport%u", 1U)
	{ }

	void teradrive(machine_config &config);
	void at_softlists(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	void x86_io(address_map &map) ATTR_COLD;
	void x86_map(address_map &map) ATTR_COLD;

	void md_68k_map(address_map &map) ATTR_COLD;
	void md_cpu_space_map(address_map &map);
	void md_68k_z80_map(address_map &map) ATTR_COLD;
	void md_z80_map(address_map &map) ATTR_COLD;
	void md_z80_io(address_map &map) ATTR_COLD;
	void md_ioctrl_map(address_map &map) ATTR_COLD;
private:
	required_device<i80286_cpu_device> m_x86cpu;
	required_device<wd7600_device> m_chipset;
	required_device<ram_device> m_ram;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	u16 wd7600_ior(offs_t offset)
	{
		if (offset < 4)
			return m_isabus->dack_r(offset);
		else
			return m_isabus->dack16_r(offset);
	}
	void wd7600_iow(offs_t offset, u16 data)
	{
		if (offset < 4)
			m_isabus->dack_w(offset, data);
		else
			m_isabus->dack16_w(offset, data);
	}
	void wd7600_hold(int state)
	{
		// halt cpu
		m_x86cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

		// and acknowledge hold
		m_chipset->hlda_w(state);
	}
	void wd7600_tc(offs_t offset, u8 data) { m_isabus->eop_w(offset, data); }
	void wd7600_spkr(int state) { m_speaker->level_w(state); }

	u16 m_heartbeat = 0;

	// bus switch semantics
	required_ioport m_system_in;

	static void romdisk_config(device_t *device);

	void tmss_bank_w(u8 data);
	u8 reg_1163_r();
	void reg_1163_w(u8 data);
	void reg_1164_w(u8 data);
	u8 system_in_r();

	// MD side
	required_device<m68000_device> m_md68kcpu;
	required_device<z80_device> m_mdz80cpu;
	required_device<screen_device> m_mdscreen;
	required_memory_bank m_tmss_bank;
	memory_view m_tmss_view;
	required_device<generic_slot_device> m_md_cart;
	required_device<ym7101_device> m_md_vdp;
	required_device<ym_generic_device> m_opn;
	memory_view m_md_68k_sound_view;
	required_device_array<sms_control_port_device, 3> m_md_ctrl_ports;
	required_device_array<megadrive_io_port_device, 3> m_md_ioports;

	// TODO: main PC screen can also swap the VGA with this
	// (roughly #5801 and #11343 league)
	u32 md_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 m_isa_address_bank = 0;
	u8 m_68k_hs = 0;
	std::unique_ptr<u8[]> m_sound_program;

	bool m_z80_reset = false;
	bool m_z80_busrq = false;
	u32 m_z80_main_address = 0;

	void flush_z80_state();
};

void teradrive_state::x86_map(address_map &map)
{
	map.unmap_value_high();
}

void teradrive_state::x86_io(address_map &map)
{
	map.unmap_value_high();
	// TODO: belongs to chipset
	map(0xfc72, 0xfc73).lr16(
		NAME([this] () {
			u16 res = m_heartbeat & 0x8000;
			if (!machine().side_effects_disabled())
				m_heartbeat ^= 0x8000;
			// other bits read
			return 0x7fff | res;
		})
	);
}

/*
 *
 * MD specifics
 *
 */

u32 teradrive_state::md_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_md_vdp->screen_update(screen, bitmap, cliprect);
	return 0;
}


void teradrive_state::md_68k_map(address_map &map)
{
	map.unmap_value_high();
//  map(0x000000, 0x7fffff).view(m_cart_view);
	// when /CART pin is low
//  m_cart_view[0](0x000000, 0x3fffff).m(m_cart, FUNC(...::cart_map));
//  m_cart_view[0](0x000000, 0x000fff).view(m_tmss_view);
//  m_tmss_view[0](0x000000, 0x000fff).rom().region("tmss", 0);
//  m_cart_view[0](0x400000, 0x7fffff).m(m_exp, FUNC(...::expansion_map));

	// /CART high (matters for MCD SRAM at very least)
//  m_cart_view[1](0x000000, 0x3fffff).m(m_exp, FUNC(...::expansion_map));
//  m_cart_view[1](0x400000, 0x7fffff).m(m_cart, FUNC(...::cart_map));
//  m_cart_view[1](0x400000, 0x400fff).view(m_tmss_view);
//  m_tmss_view[0](0x400000, 0x400fff).rom().region("tmss", 0);

	map(0x000000, 0x3fffff).r(m_md_cart, FUNC(generic_slot_device::read_rom));
	map(0x000000, 0x000fff).view(m_tmss_view);
	m_tmss_view[0](0x000000, 0x000fff).bankr(m_tmss_bank);

//  map(0x800000, 0x9fffff) unmapped or 32X
	map(0xa00000, 0xa07fff).view(m_md_68k_sound_view);
	m_md_68k_sound_view[0](0xa00000, 0xa07fff).before_delay(NAME([](offs_t) { return 1; })).m(*this, FUNC(teradrive_state::md_68k_z80_map));
//  map(0xa07f00, 0xa07fff) Z80 VDP space (freezes machine if accessed from 68k)
//  map(0xa08000, 0xa0ffff) Z80 68k window (assume no DTACK), or just mirror of above according to TD HW notes?
//  map(0xa10000, 0xa100ff) I/O
	map(0xa10000, 0xa100ff).m(*this, FUNC(teradrive_state::md_ioctrl_map));

//  map(0xa11000, 0xa110ff) memory mode register
//  map(0xa11100, 0xa111ff) Z80 BUSREQ/BUSACK
	map(0xa11100, 0xa11101).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			address_space &space = m_md68kcpu->space(AS_PROGRAM);
			// TODO: enough for all edge cases but timekill
			u16 open_bus = space.read_word(m_md68kcpu->pc() - 2) & 0xfefe;
			// printf("%06x -> %04x\n", m_md68kcpu->pc() - 2, open_bus);
			u16 res = (!m_z80_busrq || m_z80_reset) ^ 1;
			return (res << 8) | (res) | open_bus;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			//printf("%04x %04x\n", data, mem_mask);
			if (!ACCESSING_BITS_0_7)
			{
				m_z80_busrq = !!BIT(~data, 8);
			}
			else if (!ACCESSING_BITS_8_15)
			{
				m_z80_busrq = !!BIT(~data, 0);
			}
			else // word access
			{
				m_z80_busrq = !!BIT(~data, 8);
			}
			flush_z80_state();
		})
	);
//  map(0xa11200, 0xa112ff) Z80 RESET
	map(0xa11200, 0xa11201).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (!ACCESSING_BITS_0_7)
			{
				m_z80_reset = !!BIT(~data, 8);
			}
			else if (!ACCESSING_BITS_8_15)
			{
				m_z80_reset = !!BIT(~data, 0);
			}
			else // word access
			{
				m_z80_reset = !!BIT(~data, 8);
			}
			flush_z80_state();
		})
	);
//  map(0xa11300, 0xa113ff) <open bus>

//  map(0xa11400, 0xa1dfff) <unmapped> (no DTACK generation, freezes machine without additional HW)
//  map(0xa12000, 0xa120ff).m(m_exp, FUNC(...::fdc_map));
//  map(0xa13000, 0xa130ff).m(m_cart, FUNC(...::time_map));
//  map(0xa14000, 0xa14003) TMSS lock
//  map(0xa15100, 0xa153ff) 32X registers if present, <unmapped> otherwise
//  map(0xae0000, 0xae0003) Teradrive bus switch registers
	map(0xae0001, 0xae0001).lrw8(
		NAME([this] (offs_t offset) {
			return m_68k_hs;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_68k_hs = (data & 0xf3) | (m_68k_hs & 0x0c);
		})
	);
	map(0xae0003, 0xae0003).lrw8(
		NAME([this] (offs_t offset) {
			return m_isa_address_bank;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_isa_address_bank = data;
		})
	);
	map(0xaf0000, 0xafffff).rw(m_isabus, FUNC(isa16_device::io16_swap_r), FUNC(isa16_device::io16_swap_w));
	// NOTE: actually bank selectable from $ae0003 in 1 MiB units
	// pzlcnst loads from here (i.e. RAM)
	// TODO: verify requiring swapped endianness
	map(0xb00000, 0xbfffff).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			return m_isabus->mem16_swap_r(offset | (m_isa_address_bank * 0x80000), mem_mask);
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			m_isabus->mem16_swap_w(offset | (m_isa_address_bank * 0x80000), data, mem_mask);
		})
	);
//  map(0xc00000, 0xdfffff) VDP and PSG (with mirrors and holes)
//	$d00000 alias required by earthdef
	map(0xc00000, 0xc0001f).mirror(0x100000).m(m_md_vdp, FUNC(ym7101_device::if16_map));
	map(0xe00000, 0xe0ffff).mirror(0x1f0000).ram(); // Work RAM, usually accessed at $ff0000
}

// $a10000 base
void teradrive_state::md_ioctrl_map(address_map &map)
{
	// version, should be 0 for Teradrive, bit 5 for expansion bus not connected yet
	map(0x00, 0x01).lr8(NAME([] () { return 1 << 5; }));
	map(0x02, 0x03).rw(m_md_ioports[0], FUNC(megadrive_io_port_device::data_r), FUNC(megadrive_io_port_device::data_w));
	map(0x04, 0x05).rw(m_md_ioports[1], FUNC(megadrive_io_port_device::data_r), FUNC(megadrive_io_port_device::data_w));
	map(0x06, 0x07).rw(m_md_ioports[2], FUNC(megadrive_io_port_device::data_r), FUNC(megadrive_io_port_device::data_w));
	map(0x08, 0x09).rw(m_md_ioports[0], FUNC(megadrive_io_port_device::ctrl_r), FUNC(megadrive_io_port_device::ctrl_w));
	map(0x0a, 0x0b).rw(m_md_ioports[1], FUNC(megadrive_io_port_device::ctrl_r), FUNC(megadrive_io_port_device::ctrl_w));
	map(0x0c, 0x0d).rw(m_md_ioports[2], FUNC(megadrive_io_port_device::ctrl_r), FUNC(megadrive_io_port_device::ctrl_w));
	map(0x0e, 0x0f).rw(m_md_ioports[0], FUNC(megadrive_io_port_device::txdata_r), FUNC(megadrive_io_port_device::txdata_w));
	map(0x10, 0x11).r(m_md_ioports[0], FUNC(megadrive_io_port_device::rxdata_r));
	map(0x12, 0x13).rw(m_md_ioports[0], FUNC(megadrive_io_port_device::s_ctrl_r), FUNC(megadrive_io_port_device::s_ctrl_w));
	map(0x14, 0x15).rw(m_md_ioports[1], FUNC(megadrive_io_port_device::txdata_r), FUNC(megadrive_io_port_device::txdata_w));
	map(0x16, 0x17).r(m_md_ioports[1], FUNC(megadrive_io_port_device::rxdata_r));
	map(0x18, 0x19).rw(m_md_ioports[1], FUNC(megadrive_io_port_device::s_ctrl_r), FUNC(megadrive_io_port_device::s_ctrl_w));
	map(0x1a, 0x1b).rw(m_md_ioports[2], FUNC(megadrive_io_port_device::txdata_r), FUNC(megadrive_io_port_device::txdata_w));
	map(0x1c, 0x1d).r(m_md_ioports[2], FUNC(megadrive_io_port_device::rxdata_r));
	map(0x1e, 0x1f).rw(m_md_ioports[2], FUNC(megadrive_io_port_device::s_ctrl_r), FUNC(megadrive_io_port_device::s_ctrl_w));
}

void teradrive_state::md_cpu_space_map(address_map &map)
{
	map(0xfffff3, 0xfffff3).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 25; }));
	// TODO: IPL0 (external irq tied to VDP IE2)
	map(0xfffff5, 0xfffff5).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 26; }));
	map(0xfffff7, 0xfffff7).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 27; }));
	map(0xfffff9, 0xfffff9).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([this] () -> u8 {
		m_md_vdp->irq_ack();
		return 28;
	}));
	map(0xfffffb, 0xfffffb).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 29; }));
	map(0xfffffd, 0xfffffd).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([this] () -> u8 {
		m_md_vdp->irq_ack();
		return 30;
	}));
	map(0xffffff, 0xffffff).before_time(m_md68kcpu, FUNC(m68000_device::vpa_sync)).after_delay(m_md68kcpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 31; }));
}


void teradrive_state::flush_z80_state()
{
	m_mdz80cpu->set_input_line(INPUT_LINE_RESET, m_z80_reset ? ASSERT_LINE : CLEAR_LINE);
	m_mdz80cpu->set_input_line(Z80_INPUT_LINE_BUSRQ, m_z80_busrq ? CLEAR_LINE : ASSERT_LINE);
	if (m_z80_reset)
		m_opn->reset();
	if (m_z80_reset || !m_z80_busrq)
		m_md_68k_sound_view.select(0);
	else
		m_md_68k_sound_view.disable();
}

void teradrive_state::md_68k_z80_map(address_map &map)
{
	map(0x0000, 0x3fff).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			u16 res = m_sound_program[(offset << 1) ^ 1] | (m_sound_program[offset << 1] << 8);
			return res;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (!ACCESSING_BITS_0_7)
			{
				m_sound_program[(offset<<1)] = (data & 0xff00) >> 8;
			}
			else if (!ACCESSING_BITS_8_15)
			{
				m_sound_program[(offset<<1) ^ 1] = (data & 0x00ff);
			}
			else
			{
				m_sound_program[(offset<<1)] = (data & 0xff00) >> 8;
			}
		})
	);
	map(0x4000, 0x4003).mirror(0x1ffc).rw(m_opn, FUNC(ym_generic_device::read), FUNC(ym_generic_device::write));
}

void teradrive_state::md_z80_map(address_map &map)
{
	map(0x0000, 0x3fff).lrw8(
		NAME([this] (offs_t offset) {
			return m_sound_program[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_sound_program[offset] = data;
		})
	);
	map(0x4000, 0x4003).mirror(0x1ffc).rw(m_opn, FUNC(ym_generic_device::read), FUNC(ym_generic_device::write));
	map(0x6000, 0x60ff).lw8(NAME([this] (offs_t offset, u8 data) {
		m_z80_main_address = ((m_z80_main_address >> 1) | ((data & 1) << 23)) & 0xff8000;
	}));
	map(0x7f00, 0x7f1f).m(m_md_vdp, FUNC(ym7101_device::if8_map));
	map(0x8000, 0xffff).lrw8(
		NAME([this] (offs_t offset) {
			address_space &space68k = m_md68kcpu->space();
			u8 ret = space68k.read_byte(m_z80_main_address + offset);
			return ret;

		}),
		NAME([this] (offs_t offset, u8 data) {
			address_space &space68k = m_md68kcpu->space();
			space68k.write_byte(m_z80_main_address + offset, data);
		})
	);
}

void teradrive_state::md_z80_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	// juuouki reads $bf in irq service (SMS VDP left-over?)
	map(0x00, 0xff).nopr();
	// TODO: SMS mode thru cart
}

/*
 *
 * Bus Switch interface
 *
 */

void teradrive_state::tmss_bank_w(u8 data)
{
	// NOTE: half the granularity than 286 version, and the lower portion of it
	m_tmss_bank->set_entry((data * 2) + 1);
}

u8 teradrive_state::reg_1163_r()
{
	return (m_68k_hs & 3) << 4;
}

void teradrive_state::reg_1163_w(u8 data)
{
	m_68k_hs &= ~0x0c;
	m_68k_hs = (data & 0xc0) >> 4;
}

void teradrive_state::reg_1164_w(u8 data)
{
	if (BIT(data, 0))
	{
		m_x86cpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_md68kcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
	else
	{
		m_x86cpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_md68kcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}

	if (BIT(data, 1))
	{
		m_tmss_view.disable();
	}
	else
	{
		m_tmss_view.select(0);
	}
}

u8 teradrive_state::system_in_r()
{
	return m_system_in->read();
}

void teradrive_state::romdisk_config(device_t *device)
{
	auto *state = device->subdevice<teradrive_state>(":");
	isa16_ibm_79f2661 &bus_switch = *downcast<isa16_ibm_79f2661 *>(device);
	bus_switch.set_romdisk_tag("romdisk");
	bus_switch.set_md_space(":md68kcpu", AS_PROGRAM);
	bus_switch.tmss_bank_callback().set(*state, FUNC(teradrive_state::tmss_bank_w));
	bus_switch.reg_1163_read_callback().set(*state, FUNC(teradrive_state::reg_1163_r));
	bus_switch.reg_1163_write_callback().set(*state, FUNC(teradrive_state::reg_1163_w));
	bus_switch.reg_1164_callback().set(*state, FUNC(teradrive_state::reg_1164_w));
	// TODO: using set_ioport doesn't work, will try to map from bus_switch device anyway
//  bus_switch.system_in_callback().set_ioport("SYSTEM_IN");
	bus_switch.system_in_callback().set(*state, FUNC(teradrive_state::system_in_r));
}

/*
 *
 * Machine generics
 *
 */

static INPUT_PORTS_START( teradrive )
	PORT_START("SYSTEM_IN")
	PORT_DIPNAME( 0x01, 0x01, "Boot mode" )
	PORT_DIPSETTING(    0x00, "MD boot" )
	PORT_DIPSETTING(    0x01, "PC boot" )
	PORT_DIPNAME( 0x04, 0x04, "Video mode" )
	PORT_DIPSETTING(    0x00, "Video" ) // composite
	PORT_DIPSETTING(    0x04, "RGB" )
INPUT_PORTS_END

void teradrive_state::machine_start()
{
	m_tmss_bank->configure_entries(0, 0x200, memregion("tmss")->base(), 0x1000);
	// doubled in space
	m_sound_program = std::make_unique<u8[]>(0x4000);

	save_item(NAME(m_heartbeat));

	save_item(NAME(m_isa_address_bank));
	save_item(NAME(m_68k_hs));
	save_pointer(NAME(m_sound_program), 0x4000);

	save_item(NAME(m_z80_reset));
	save_item(NAME(m_z80_busrq));
	save_item(NAME(m_z80_main_address));
}

void teradrive_state::machine_reset()
{
	m_68k_hs = 0;
	m_z80_reset = true;
	flush_z80_state();
}

void teradrive_isa_cards(device_slot_interface &device)
{
	device.option_add_internal("bus_switch", ISA16_IBM_79F2661);
	device.option_add_internal("wd90c10_romless", ISA16_WD90C10_ROMLESS);
}

void teradrive_state::at_softlists(machine_config &config)
{
	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
//  SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");
//  SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");

//  TODO: MD portion
	SOFTWARE_LIST(config, "cart_list").set_original("megadriv").set_filter("NTSC-J");
	SOFTWARE_LIST(config, "td_disk_list").set_original("teradrive_flop");
//  TODO: Teradrive HDD SW list
}

void teradrive_state::teradrive(machine_config &config)
{
	I80286(config, m_x86cpu, XTAL(10'000'000));
	m_x86cpu->set_addrmap(AS_PROGRAM, &teradrive_state::x86_map);
	m_x86cpu->set_addrmap(AS_IO, &teradrive_state::x86_io);
	m_x86cpu->set_irq_acknowledge_callback("chipset", FUNC(wd7600_device::intack_cb));

	// WD76C10LP system controller
	// WD76C30 peripheral controller
	WD7600(config, m_chipset, 50_MHz_XTAL / 2);
	m_chipset->set_cputag(m_x86cpu);
	m_chipset->set_ramtag(m_ram);
	m_chipset->set_biostag("bios");
	m_chipset->set_keybctag("keybc");
	m_chipset->hold_callback().set(FUNC(teradrive_state::wd7600_hold));
	m_chipset->nmi_callback().set_inputline(m_x86cpu, INPUT_LINE_NMI);
	m_chipset->intr_callback().set_inputline(m_x86cpu, INPUT_LINE_IRQ0);
	m_chipset->cpureset_callback().set_inputline(m_x86cpu, INPUT_LINE_RESET);
	m_chipset->a20m_callback().set_inputline(m_x86cpu, INPUT_LINE_A20);
	// isa dma
	m_chipset->ior_callback().set(FUNC(teradrive_state::wd7600_ior));
	m_chipset->iow_callback().set(FUNC(teradrive_state::wd7600_iow));
	m_chipset->tc_callback().set(FUNC(teradrive_state::wd7600_tc));
	// speaker
	m_chipset->spkr_callback().set(FUNC(teradrive_state::wd7600_spkr));

	// on board devices
	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace(m_x86cpu, AS_PROGRAM);
	m_isabus->set_iospace(m_x86cpu, AS_IO);
	m_isabus->iochck_callback().set(m_chipset, FUNC(wd7600_device::iochck_w));
	m_isabus->irq2_callback().set(m_chipset, FUNC(wd7600_device::irq09_w));
	m_isabus->irq3_callback().set(m_chipset, FUNC(wd7600_device::irq03_w));
	m_isabus->irq4_callback().set(m_chipset, FUNC(wd7600_device::irq04_w));
	m_isabus->irq5_callback().set(m_chipset, FUNC(wd7600_device::irq05_w));
	m_isabus->irq6_callback().set(m_chipset, FUNC(wd7600_device::irq06_w));
	m_isabus->irq7_callback().set(m_chipset, FUNC(wd7600_device::irq07_w));
	m_isabus->irq10_callback().set(m_chipset, FUNC(wd7600_device::irq10_w));
	m_isabus->irq11_callback().set(m_chipset, FUNC(wd7600_device::irq11_w));
	m_isabus->irq12_callback().set(m_chipset, FUNC(wd7600_device::irq12_w));
	m_isabus->irq14_callback().set(m_chipset, FUNC(wd7600_device::irq14_w));
	m_isabus->irq15_callback().set(m_chipset, FUNC(wd7600_device::irq15_w));
	m_isabus->drq0_callback().set(m_chipset, FUNC(wd7600_device::dreq0_w));
	m_isabus->drq1_callback().set(m_chipset, FUNC(wd7600_device::dreq1_w));
	m_isabus->drq2_callback().set(m_chipset, FUNC(wd7600_device::dreq2_w));
	m_isabus->drq3_callback().set(m_chipset, FUNC(wd7600_device::dreq3_w));
	m_isabus->drq5_callback().set(m_chipset, FUNC(wd7600_device::dreq5_w));
	m_isabus->drq6_callback().set(m_chipset, FUNC(wd7600_device::dreq6_w));
	m_isabus->drq7_callback().set(m_chipset, FUNC(wd7600_device::dreq7_w));

	// NOTE: wants IBM BIOS over IBM keyboard only
	ps2_keyboard_controller_device &keybc(PS2_KEYBOARD_CONTROLLER(config, "keybc", 12_MHz_XTAL));
	keybc.hot_res().set("chipset", FUNC(wd7600_device::kbrst_w));
	keybc.gate_a20().set("chipset", FUNC(wd7600_device::gatea20_w));
	keybc.kbd_irq().set("chipset", FUNC(wd7600_device::irq01_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_84));
	pc_kbdc.out_clock_cb().set("keybc", FUNC(ps2_keyboard_controller_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set("keybc", FUNC(ps2_keyboard_controller_device::kbd_data_w));

	pc_kbdc_device &aux_con(PC_KBDC(config, "aux", ps2_mice, STR_HLE_PS2_MOUSE));
	aux_con.out_clock_cb().set("keybc", FUNC(ps2_keyboard_controller_device::aux_clk_w));
	aux_con.out_data_cb().set("keybc", FUNC(ps2_keyboard_controller_device::aux_data_w));

	// FIXME: determine ISA bus clock, unverified configuration
	// WD76C20
	ISA16_SLOT(config, "board1", 0, "isabus", pc_isa16_cards, "fdc_smc", true);
	ISA16_SLOT(config, "board2", 0, "isabus", pc_isa16_cards, "comat", true);
	// TODO: should be ESDI built-in interface on riser with IBM WDL-330PS 3.5" HDD, not IDE
	ISA16_SLOT(config, "board3", 0, "isabus", pc_isa16_cards, "side116", true);
	ISA16_SLOT(config, "board4", 0, "isabus", pc_isa16_cards, "lpt", true);
	ISA16_SLOT(config, "board5", 0, "isabus", teradrive_isa_cards, "wd90c10_romless", true);
	ISA16_SLOT(config, "board6", 0, "isabus", teradrive_isa_cards, "bus_switch", true).set_option_machine_config("bus_switch", romdisk_config);
	ISA16_SLOT(config, "isa1",   0, "isabus", pc_isa16_cards, nullptr, false);

	// 2.5MB is the max allowed by the BIOS (even if WD chipset can do more)
	// TODO: pcdos5v garbles font loading with 1664K, which should be the actual default
	RAM(config, RAM_TAG).set_default_size("2688K").set_extra_options("640K,1664K");

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	at_softlists(config);

	// MD section (NTSC)
	constexpr XTAL md_master_xtal(53.693175_MHz_XTAL);

	M68000(config, m_md68kcpu, md_master_xtal / 7);
	m_md68kcpu->set_addrmap(AS_PROGRAM, &teradrive_state::md_68k_map);
	m_md68kcpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &teradrive_state::md_cpu_space_map);
	// disallow TAS (gargoyle, cliffh, exmutant)
	m_md68kcpu->set_tas_write_callback(NAME([] (offs_t offset, u8 data) { }));

	Z80(config, m_mdz80cpu, md_master_xtal / 15);
	m_mdz80cpu->set_addrmap(AS_PROGRAM, &teradrive_state::md_z80_map);
	m_mdz80cpu->set_addrmap(AS_IO, &teradrive_state::md_z80_io);

	SCREEN(config, m_mdscreen, SCREEN_TYPE_RASTER);
	// NOTE: PAL is 423x312
	m_mdscreen->set_raw(md_master_xtal / 8, 427, 0, 320, 262, 0, 224);
	m_mdscreen->set_screen_update(FUNC(teradrive_state::md_screen_update));

	YM7101(config, m_md_vdp, md_master_xtal / 4);
	m_md_vdp->set_mclk(md_master_xtal);
	m_md_vdp->set_screen(m_mdscreen);
	// TODO: actual DTACK
	// TODO: accessing 68k bus from x86, defers access?
	m_md_vdp->dtack_cb().set_inputline(m_md68kcpu, INPUT_LINE_HALT);
	m_md_vdp->mreq_cb().set([this] (offs_t offset) {
		// TODO: can't read outside of base cart and RAM
		//printf("%06x\n", offset);
		address_space &space68k = m_md68kcpu->space();
		u16 ret = space68k.read_word(offset, 0xffff);

		return ret;
	});
	m_md_vdp->vint_cb().set_inputline(m_md68kcpu, 6);
	m_md_vdp->hint_cb().set_inputline(m_md68kcpu, 4);
	m_md_vdp->sint_cb().set_inputline(m_mdz80cpu, INPUT_LINE_IRQ0);
	m_md_vdp->add_route(ALL_OUTPUTS, "md_speaker", 0.50, 0);
	m_md_vdp->add_route(ALL_OUTPUTS, "md_speaker", 0.50, 1);

	auto &hl(INPUT_MERGER_ANY_HIGH(config, "hl"));
	// TODO: gated thru VDP
	hl.output_handler().set_inputline(m_md68kcpu, 2);

	MEGADRIVE_IO_PORT(config, m_md_ioports[0], 0);
	m_md_ioports[0]->hl_handler().set("hl", FUNC(input_merger_device::in_w<0>));

	MEGADRIVE_IO_PORT(config, m_md_ioports[1], 0);
	m_md_ioports[1]->hl_handler().set("hl", FUNC(input_merger_device::in_w<1>));

	MEGADRIVE_IO_PORT(config, m_md_ioports[2], 0);
	m_md_ioports[2]->hl_handler().set("hl", FUNC(input_merger_device::in_w<2>));

	for (int N = 0; N < 3; N++)
	{
		SMS_CONTROL_PORT(config, m_md_ctrl_ports[N], sms_control_port_devices, N != 2 ? SMS_CTRL_OPTION_MD_PAD : nullptr);
		m_md_ctrl_ports[N]->th_handler().set(m_md_ioports[N], FUNC(megadrive_io_port_device::th_w));
		m_md_ctrl_ports[N]->set_screen(m_mdscreen);

		m_md_ioports[N]->set_in_handler(m_md_ctrl_ports[N], FUNC(sms_control_port_device::in_r));
		m_md_ioports[N]->set_out_handler(m_md_ctrl_ports[N], FUNC(sms_control_port_device::out_w));
	}

	// TODO: vestigial
	GENERIC_CARTSLOT(config, m_md_cart, generic_plain_slot, "megadriv_cart");
	m_md_cart->set_width(GENERIC_ROM16_WIDTH);
	// TODO: generic_cartslot has issues with softlisted endianness (use loose for now)
	m_md_cart->set_endian(ENDIANNESS_BIG);

	SPEAKER(config, "md_speaker", 2).front();

	YM2612(config, m_opn, md_master_xtal / 7);
	m_opn->add_route(0, "md_speaker", 0.50, 0);
	m_opn->add_route(1, "md_speaker", 0.50, 1);
}

ROM_START( teradrive )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD( "bios-27c010.bin", 0x00000, 0x20000, CRC(32642518) SHA1(6bb6d0325b8e4150c4258fd16f3a870b92e88f75))

	ROM_REGION16_LE(0x200000, "board6:romdisk", ROMREGION_ERASEFF)
	// contains bootable PC-DOS 3.x + a MENU.EXE
	// 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "tru-27c800.bin", 0x00000, 0x100000,  CRC(c2fe9c9e) SHA1(06ec0461dab425f41fb5c3892d9beaa8fa53bbf1))

	// firmware for 68k side, need to ROM_COPY to match endianness
	ROM_REGION16_BE(0x200000, "tmss", ROMREGION_ERASEFF)
	ROM_COPY("board6:romdisk", 0x00000, 0x00000, 0x200000 )
ROM_END

ROM_START( teradrive3 )
	// TODO: may just be a BIOS dump that needs to be trimmed
	ROM_REGION(0x80000, "rawbios", 0)
	ROM_LOAD( "model3.bin", 0x00000, 0x80000, CRC(dc757cb3) SHA1(c1489cc2d554fc62f986464604f7f7fbb219b438))

	ROM_REGION(0x20000, "bios", 0)
	ROM_COPY("rawbios", 0x60000, 0x00000, 0x20000)

	ROM_REGION16_LE(0x200000, "board6:romdisk", ROMREGION_ERASEFF)
	// contains bootable PC-DOS 3.x + a MENU.EXE
	// 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "tru-27c800.bin", 0x00000, 0x100000,  CRC(c2fe9c9e) SHA1(06ec0461dab425f41fb5c3892d9beaa8fa53bbf1))

	ROM_REGION16_BE(0x200000, "tmss", ROMREGION_ERASEFF)
	ROM_COPY("board6:romdisk", 0x00000, 0x00000, 0x200000 )
ROM_END


} // anonymous namespace

COMP( 1991, teradrive,  0,         0,       teradrive, teradrive, teradrive_state, empty_init, "Sega / International Business Machines", "Teradrive (Japan, Model 2)", MACHINE_NOT_WORKING )
COMP( 1991, teradrive3, teradrive, 0,       teradrive, teradrive, teradrive_state, empty_init, "Sega / International Business Machines", "Teradrive (Japan, Model 3)", MACHINE_NOT_WORKING )
