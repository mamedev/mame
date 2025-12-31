// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

https://segaretro.org/Magistr_16

a.k.a. Super Kombat KO16

MegaDrive SECAM clone with ISA bus glued together thru expansion bus

Notes:
- error #100A: generic Magistr16 HW error, missing "BIOS" header at $400000
  (what would return when run on stock MD)
- error #1004: flash type error PC=fc07da
- error #1008: GPIO1 Super I/O device at $100 AT90S1200
- error #1009: <seen in the same path as #1008>
- error #100C: checksum error (on main cart);

TODO:
- accomodate YM7101 to use PAL semantics and V30 modes
- Fix current #1008 error (bp fc1b84,1,{D0=0;g})
- Alias for flash type device ID 0x46 (at PC=fc0794/PC=fc07b2)
- Eventually return "ILLEGAL COPY !!!" at PC=FC3900

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa.h"
#include "bus/megadrive/cart/options.h"
#include "bus/megadrive/cart/slot.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/sms_ctrl/controllers.h"
#include "bus/sms_ctrl/smsctrl.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/intelfsh.h"
#include "machine/ram.h"
#include "machine/sega_md_ioport.h"
#include "machine/w83977tf.h"
#include "sound/spkrdev.h"
#include "sound/ymopn.h"
#include "video/ym7101.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {

class magistr16_state : public driver_device
{
public:
	magistr16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_md68kcpu(*this, "md68kcpu")
		, m_mdz80cpu(*this, "mdz80cpu")
		, m_mdscreen(*this, "mdscreen")
		, m_md_cart(*this, "md_cart")
		, m_md_vdp(*this, "md_vdp")
		, m_opn(*this, "opn")
		, m_md_68k_sound_view(*this, "md_68k_sound_view")
		, m_md_ctrl_ports(*this, { "md_ctrl1", "md_ctrl2", "md_exp" })
		, m_md_ioports(*this, "md_ioport%u", 1U)
		, m_isabus(*this, "isabus")
		, m_flash(*this, "flash")
	{ }

	void magistr16(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	void md_68k_map(address_map &map) ATTR_COLD;
	void md_cpu_space_map(address_map &map);
	void md_68k_z80_map(address_map &map) ATTR_COLD;
	void md_z80_map(address_map &map) ATTR_COLD;
	void md_z80_io(address_map &map) ATTR_COLD;
	void md_ioctrl_map(address_map &map) ATTR_COLD;
private:
	required_device<m68000_device> m_md68kcpu;
	required_device<z80_device> m_mdz80cpu;
	required_device<screen_device> m_mdscreen;
	required_device<megadrive_cart_slot_device> m_md_cart;
	required_device<ym7101_device> m_md_vdp;
	required_device<ym_generic_device> m_opn;
	memory_view m_md_68k_sound_view;
	required_device_array<sms_control_port_device, 3> m_md_ctrl_ports;
	required_device_array<megadrive_io_port_device, 3> m_md_ioports;
	required_device<isa16_device> m_isabus;
	required_device<winbond_w29c020c_device> m_flash;

	std::unique_ptr<u8[]> m_sound_program;

	bool m_z80_reset = false;
	bool m_z80_busrq = false;
	u32 m_z80_main_address = 0;

	void flush_z80_state();

	static void winbond_superio_config(device_t *device);

	u8 gpio1_r();
	void gpio1_w(u8 data);
};


void magistr16_state::md_68k_map(address_map &map)
{
	map.unmap_value_high();
	// assume it can't access expansion bus area
	map(0x000000, 0x3fffff).rw(m_md_cart, FUNC(megadrive_cart_slot_device::base_r), FUNC(megadrive_cart_slot_device::base_w));
	map(0x400000, 0x43ffff).rw(m_flash, FUNC(winbond_w29c020c_device::read), FUNC(winbond_w29c020c_device::write));

	map(0x7e0000, 0x7e07ff).rw(m_isabus, FUNC(isa16_device::io_r), FUNC(isa16_device::io_w)).umask16(0x00ff);

//  map(0x800000, 0x9fffff) unmapped or 32X
	map(0xa00000, 0xa07fff).view(m_md_68k_sound_view);
	m_md_68k_sound_view[0](0xa00000, 0xa07fff).before_delay(NAME([](offs_t) { return 1; })).m(*this, FUNC(magistr16_state::md_68k_z80_map));
//  map(0xa07f00, 0xa07fff) Z80 VDP space (freezes machine if accessed from 68k)
//  map(0xa08000, 0xa0ffff) Z80 68k window (assume no DTACK), or just mirror of above according to TD HW notes?
//  map(0xa10000, 0xa100ff) I/O
	map(0xa10000, 0xa100ff).m(*this, FUNC(magistr16_state::md_ioctrl_map));

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
	map(0xa13000, 0xa130ff).rw(m_md_cart, FUNC(megadrive_cart_slot_device::time_r), FUNC(megadrive_cart_slot_device::time_w));
//  map(0xa14000, 0xa14003) TMSS lock
//  map(0xa15100, 0xa153ff) 32X registers if present, <unmapped> otherwise
//  map(0xc00000, 0xdfffff) VDP and PSG (with mirrors and holes)
//  $d00000 alias required by earthdef
	map(0xc00000, 0xc0001f).mirror(0x100000).m(m_md_vdp, FUNC(ym7101_device::if16_map));
	map(0xe00000, 0xe3ffff).mirror(0x1c0000).ram(); // more work RAM compared to stock MD
}

// $a10000 base
void magistr16_state::md_ioctrl_map(address_map &map)
{
	// version unknown, expansion bus should be on.
	// Assume overseas (bit 7) and PAL set (bit 6)
	map(0x00, 0x01).lr8(NAME([] () { return 1 << 7 | 1 << 6 | 0 << 5; }));
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

void magistr16_state::md_cpu_space_map(address_map &map)
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


void magistr16_state::flush_z80_state()
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

void magistr16_state::md_68k_z80_map(address_map &map)
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

void magistr16_state::md_z80_map(address_map &map)
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

void magistr16_state::md_z80_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	// juuouki reads $bf in irq service (SMS VDP left-over?)
	map(0x00, 0xff).nopr();
	// TODO: SMS mode thru cart
}

static INPUT_PORTS_START( magistr16 )
INPUT_PORTS_END

void magistr16_state::machine_start()
{
	m_sound_program = std::make_unique<u8[]>(0x2000);

	save_pointer(NAME(m_sound_program), 0x2000);

	save_item(NAME(m_z80_reset));
	save_item(NAME(m_z80_busrq));
	save_item(NAME(m_z80_main_address));
}

void magistr16_state::machine_reset()
{
	m_z80_reset = true;
	flush_z80_state();
}

/*
 *
 * Super I/O semantics
 *
 */

// AT90S1200 comms, also from $300000 in cart space for "music"
/*
 * ---- --x- Atmel PB6/MISO
 */
u8 magistr16_state::gpio1_r()
{
	return 0;
}

/*
 * x--- ---- Atmel PB5/MOSI
 * -x-- ---- Atmel PB7/SCK
 * --xx ---- <used on $300000 checks> PB0~1/AIN0~1?
 */
void magistr16_state::gpio1_w(u8 data)
{
	if (data & 0x3f)
		logerror("gpio1_w: unknown bit set %02x\n",data);
}

static void isa_internal_devices(device_slot_interface &device)
{
	// TODO: Winbond w83977f
	device.option_add("w83977tf", W83977TF);
}

void magistr16_state::winbond_superio_config(device_t *device)
{
	auto *state = device->subdevice<magistr16_state>(":");

	w83977tf_device &fdc = *downcast<w83977tf_device *>(device);
	fdc.gpio1_read().set(*state, FUNC(magistr16_state::gpio1_r));
	fdc.gpio1_write().set(*state, FUNC(magistr16_state::gpio1_w));

//  fdc.set_sysopt_pin(1);
//  fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
//  fdc.gp25_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
//  fdc.irq1().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq1_w));
//  fdc.irq8().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq8n_w));
//  fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
//  fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}


void magistr16_state::magistr16(machine_config &config)
{
	// PAL clock
	constexpr XTAL md_master_xtal(53.203424_MHz_XTAL);

	M68000(config, m_md68kcpu, md_master_xtal / 7);
	m_md68kcpu->set_addrmap(AS_PROGRAM, &magistr16_state::md_68k_map);
	m_md68kcpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &magistr16_state::md_cpu_space_map);
	// disallow TAS (gargoyle, cliffh, exmutant)
	m_md68kcpu->set_tas_write_callback(NAME([] (offs_t offset, u8 data) { }));

	Z80(config, m_mdz80cpu, md_master_xtal / 15);
	m_mdz80cpu->set_addrmap(AS_PROGRAM, &magistr16_state::md_z80_map);
	m_mdz80cpu->set_addrmap(AS_IO, &magistr16_state::md_z80_io);

	SCREEN(config, m_mdscreen, SCREEN_TYPE_RASTER);
	m_mdscreen->set_raw(md_master_xtal / 8, 423, 0, 320, 312, 0, 240);
	m_mdscreen->set_screen_update(m_md_vdp, FUNC(ym7101_device::screen_update));

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

	MEGADRIVE_CART_SLOT(config, m_md_cart, md_master_xtal / 7, megadrive_cart_options, nullptr).set_must_be_loaded(true);
	m_md_cart->vres_cb().set([this](int state) {
		if (state)
		{
			m_md68kcpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
			for (auto &port : m_md_ioports)
				port->reset();
		}
	});

	SPEAKER(config, "md_speaker", 2).front();

	YM2612(config, m_opn, md_master_xtal / 7);
	m_opn->add_route(0, "md_speaker", 0.50, 0);
	m_opn->add_route(1, "md_speaker", 0.50, 1);

	// TODO: either split specific magistr16 carts or make it a filter option
	SOFTWARE_LIST(config, "cart_list").set_original("megadriv").set_filter("PAL");

	// expansion bus overlays
	WINBOND_W29C020C(config, "flash");

	// FIXME: determine ISA bus clock
	ISA16(config, m_isabus, 0);
	m_isabus->set_custom_spaces();
	ISA16_SLOT(config, "board1", 0, "isabus", isa_internal_devices, "w83977tf", true).set_option_machine_config("w83977tf", winbond_superio_config);
}


ROM_START( magistr16 )
	ROM_REGION16_BE( 0x40000, "flash", ROMREGION_ERASE00 )
	// both looks incomplete, to be checked if any vital data is missing here.
	ROM_SYSTEM_BIOS(0, "212a", "2.12a")
	ROMX_LOAD( "bios-dump_2.12a-core.bin", 0x00000, 0x4800, BAD_DUMP CRC(ed7b2d61) SHA1(89aa09023262e5daae7f62c2c46b0a6b2228113f), ROM_BIOS(0) | ROM_GROUPWORD)
	ROM_SYSTEM_BIOS(1, "209i", "2.09i")
	ROMX_LOAD( "bios-dump_2.09i-core.bin", 0x00000, 0x4c00, BAD_DUMP CRC(c63d54b4) SHA1(8dad327fd5464341c55a9c12225b46c440c46507), ROM_BIOS(1) | ROM_GROUPWORD)
ROM_END

} // anonymous namespace

CONS( 2001, magistr16, 0, 0, magistr16, magistr16, magistr16_state, empty_init, "New Game", "Magistr16 (Russia)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )

