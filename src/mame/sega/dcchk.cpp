// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

https://segaretro.org/Dreamcast_Controller_Function_Checker

Dreamcast Maple bus checker running on Megadrive-based HW

Maple board marked "VMS CHECKER MAIN BOARD-3", with an Altera Flex FPGA
(and assuming it's same for the dumped BIOS)

TODO:
- Maple bus interactions;

**************************************************************************************************/

#include "emu.h"

//#include "bus/megadrive/cart/options.h"
//#include "bus/megadrive/cart/slot.h"
//#include "bus/sms_ctrl/controllers.h"
//#include "bus/sms_ctrl/smsctrl.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
//#include "machine/input_merger.h"
//#include "machine/sega_md_ioport.h"
#include "sound/spkrdev.h"
#include "sound/ymopn.h"
#include "video/ym7101.h"

#include "screen.h"
#include "speaker.h"

namespace {

class dcchk_state : public driver_device
{
public:
	dcchk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_md68kcpu(*this, "md68kcpu")
		, m_mdz80cpu(*this, "mdz80cpu")
		, m_mdscreen(*this, "mdscreen")
		//, m_md_cart(*this, "md_cart")
		, m_md_vdp(*this, "md_vdp")
		, m_opn(*this, "opn")
		, m_md_68k_sound_view(*this, "md_68k_sound_view")
		//, m_md_ctrl_ports(*this, { "md_ctrl1", "md_ctrl2", "md_exp" })
		//, m_md_ioports(*this, "md_ioport%u", 1U)
	{ }

	void dcchk(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	void md_68k_map(address_map &map) ATTR_COLD;
	void md_cpu_space_map(address_map &map);
	void md_68k_z80_map(address_map &map) ATTR_COLD;
	void md_z80_map(address_map &map) ATTR_COLD;
	void md_z80_io(address_map &map) ATTR_COLD;
	//void md_ioctrl_map(address_map &map) ATTR_COLD;
private:
	required_device<m68000_device> m_md68kcpu;
	required_device<z80_device> m_mdz80cpu;
	required_device<screen_device> m_mdscreen;
	//required_device<megadrive_cart_slot_device> m_md_cart;
	required_device<ym7101_device> m_md_vdp;
	required_device<ym_generic_device> m_opn;
	memory_view m_md_68k_sound_view;
	//required_device_array<sms_control_port_device, 3> m_md_ctrl_ports;
	//required_device_array<megadrive_io_port_device, 3> m_md_ioports;

	std::unique_ptr<u8[]> m_sound_program;

	bool m_z80_reset = false;
	bool m_z80_busrq = false;
	u32 m_z80_main_address = 0;

	void flush_z80_state();
};


void dcchk_state::md_68k_map(address_map &map)
{
	map.unmap_value_high();
//  map(0x000000, 0x3fffff).rw(m_md_cart, FUNC(megadrive_cart_slot_device::base_r), FUNC(megadrive_cart_slot_device::base_w));
	map(0x000000, 0x07ffff).rom().region("bios", 0);
	// $400000 maple bus interactions

	map(0xa00000, 0xa07fff).view(m_md_68k_sound_view);
	m_md_68k_sound_view[0](0xa00000, 0xa07fff).before_delay(NAME([](offs_t) { return 1; })).m(*this, FUNC(dcchk_state::md_68k_z80_map));
//  map(0xa07f00, 0xa07fff) Z80 VDP space (freezes machine if accessed from 68k)
//  map(0xa08000, 0xa0ffff) Z80 68k window (assume no DTACK), or just mirror of above according to TD HW notes?

	// TODO: checked at startup, still used for Maple interactions?
//  map(0xa10000, 0xa100ff) I/O
//  map(0xa10000, 0xa100ff).m(*this, FUNC(dcchk_state::md_ioctrl_map));

//  map(0xa11000, 0xa110ff) memory mode register
//  map(0xa11100, 0xa111ff) Z80 BUSREQ/BUSACK
	map(0xa11100, 0xa11101).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			address_space &space = m_md68kcpu->space(AS_PROGRAM);
			// TODO: as per teradrive
			u16 open_bus = space.read_word(m_md68kcpu->pc() - 2) & 0xfefe;
			// printf("%06x -> %04x\n", m_md68kcpu->pc() - 2, open_bus);
			u16 res = (m_mdz80cpu->busack_r() && !m_z80_reset) ^ 1;
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
//  map(0xa13000, 0xa130ff).rw(m_md_cart, FUNC(megadrive_cart_slot_device::time_r), FUNC(megadrive_cart_slot_device::time_w));
//  map(0xa14000, 0xa14003) TMSS lock
//  map(0xa15100, 0xa153ff) 32X registers if present, <unmapped> otherwise
//  map(0xc00000, 0xdfffff) VDP and PSG (with mirrors and holes)
//  $d00000 alias required by earthdef
	map(0xc00000, 0xc0001f).mirror(0x100000).m(m_md_vdp, FUNC(ym7101_device::if16_map));
	map(0xe00000, 0xe3ffff).mirror(0x1c0000).ram(); // more work RAM compared to stock MD
}

void dcchk_state::md_cpu_space_map(address_map &map)
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


void dcchk_state::flush_z80_state()
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

void dcchk_state::md_68k_z80_map(address_map &map)
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

void dcchk_state::md_z80_map(address_map &map)
{
	map(0x0000, 0x1fff).lrw8(
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

void dcchk_state::md_z80_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0xff).nopr();
}

static INPUT_PORTS_START( dcchk )
INPUT_PORTS_END

void dcchk_state::machine_start()
{
	m_sound_program = std::make_unique<u8[]>(0x2000);

	save_pointer(NAME(m_sound_program), 0x2000);

	save_item(NAME(m_z80_reset));
	save_item(NAME(m_z80_busrq));
	save_item(NAME(m_z80_main_address));
}

void dcchk_state::machine_reset()
{
	m_z80_reset = true;
	flush_z80_state();
}

void dcchk_state::dcchk(machine_config &config)
{
	// MD section (NTSC, assumed)
	constexpr XTAL md_master_xtal(53.693175_MHz_XTAL);

	M68000(config, m_md68kcpu, md_master_xtal / 7);
	m_md68kcpu->set_addrmap(AS_PROGRAM, &dcchk_state::md_68k_map);
	m_md68kcpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &dcchk_state::md_cpu_space_map);
	m_md68kcpu->set_tas_write_callback(NAME([] (offs_t offset, u8 data) { }));

	Z80(config, m_mdz80cpu, md_master_xtal / 15);
	m_mdz80cpu->set_addrmap(AS_PROGRAM, &dcchk_state::md_z80_map);
	m_mdz80cpu->set_addrmap(AS_IO, &dcchk_state::md_z80_io);

	SCREEN(config, m_mdscreen, SCREEN_TYPE_RASTER);
	m_mdscreen->set_raw(md_master_xtal / 8, 427, 0, 320, 262, 0, 224);
	m_mdscreen->set_screen_update(m_md_vdp, FUNC(ym7101_device::screen_update));

	YM7101(config, m_md_vdp, md_master_xtal / 4);
	m_md_vdp->set_mclk(md_master_xtal);
	m_md_vdp->set_screen(m_mdscreen);
	// TODO: actual DTACK
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

	SPEAKER(config, "md_speaker", 2).front();

	YM2612(config, m_opn, md_master_xtal / 7);
	m_opn->add_route(0, "md_speaker", 0.50, 0);
	m_opn->add_route(1, "md_speaker", 0.50, 1);

	// TODO: Maple bus here
}


ROM_START( dcacchk )
	ROM_REGION16_BE( 0x80000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "1998_08_dcc_rom.bin", 0x00000, 0x80000, CRC(989d4ded) SHA1(a5e7bdba2465241a21010d1fd8ab5deb4e10d8b2))
ROM_END

} // anonymous namespace

CONS( 1998, dcacchk, 0, 0, dcchk, dcchk, dcchk_state, empty_init, "Sega", "Dreamcast Arcade Stick Checker (v2.00)", MACHINE_NOT_WORKING )

