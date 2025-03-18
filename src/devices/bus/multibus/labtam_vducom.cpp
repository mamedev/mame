// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Labtam 3000 8086 VDU COMM card.
 *
 * Sources:
 *  - https://arvutimuuseum.ut.ee/index.php?m=eksponaadid&id=223
 *
 * TODO:
 *  - keyboard click/bell
 *  - light pen
 *  - parallel ports
 */

#include "emu.h"
#include "labtam_vducom.h"

#include "machine/input_merger.h"

#define VERBOSE 0
#include "logmacro.h"

enum u7_mask : u8
{
	U7_PWRFAIL = 0x01, // enable multibus access
	U7_WINDOW  = 0x02, // 0=lower 512k, 1=upper 512k
	U7_SWAP    = 0x04, // swap data & display segments
	U7_VDOUBLE = 0x08, // enable double vertical resolution
	U7_HALF    = 0x10, // enable half intensity
	U7_CLICK   = 0x20, // disable key click
	U7_BLANK   = 0x80, // disable blanked screen
};

DEFINE_DEVICE_TYPE(LABTAM_8086CPU, labtam_8086cpu_device, "labtam_8086cpu", "Labtam 8086 CPU")
DEFINE_DEVICE_TYPE(LABTAM_VDUCOM,  labtam_vducom_device,  "labtam_vducom",  "Labtam 8086 VDU COMM")

labtam_vducom_device_base::labtam_vducom_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_multibus_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_pic(*this, "pic")
	, m_ctc(*this, "ctc%u", 0U)
	, m_com(*this, "com%u", 0U)
	, m_nvram(*this, "nvram%u", 0U)
	, m_serial(*this, "serial%u", 0U)
	, m_e4(*this, "E4")
	, m_mbus(*this, "mbus")
	, m_installed(false)
{
}

labtam_8086cpu_device::labtam_8086cpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: labtam_vducom_device_base(mconfig, LABTAM_8086CPU, tag, owner, clock)
{
}

labtam_vducom_device::labtam_vducom_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: labtam_vducom_device_base(mconfig, LABTAM_VDUCOM, tag, owner, clock)
	, m_crtc(*this, "crtc")
	, m_palette(*this, "palette")
	, m_screen(*this, "screen")
	, m_ram(*this, "ram%u", 0U)
{
}

ROM_START(labtam_8086cpu)
	ROM_REGION16_LE(0x10000, "eprom", 0)

	ROM_SYSTEM_BIOS(0, "v0", "v0")
	ROMX_LOAD("slave_1__0.u34", 0xc000, 0x2000, CRC(f5007dd0) SHA1(2362852d88bf32ce605ad394244dc6e21a6df5c6), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("slave_1__1.u44", 0xc001, 0x2000, CRC(2de2e667) SHA1(c575d5d53f2fe99569af002c636754dedca09e3b), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

ROM_START(labtam_vducom)
	ROM_REGION16_LE(0x10000, "eprom", 0)

	ROM_SYSTEM_BIOS(0, "k84", "Version K84 23/11/84") // date not certain
	ROMX_LOAD("vdu_com__k84.1_0.u26", 0x0000, 0x4000, CRC(ac27697e) SHA1(eeec9cff06181dffe95a37aefb2b3789a0959b4f), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("vdu_com__k84.1_1.u39", 0x0001, 0x4000, CRC(8b3e567d) SHA1(5ce821004155d02b87ed7b6d14989a9fffc38c6a), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("vdu_com__k84.1_2.u34", 0x8000, 0x4000, CRC(70a2c615) SHA1(67d265be3101dfe33fa705447e1394410e55f518), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("vdu_com__k84.1_3.u44", 0x8001, 0x4000, CRC(dcd7ce03) SHA1(727e5fa72d6088ddf9e8613656d713754d628538), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "l83", "Version L83 15/12/83") // date not certain
	ROMX_LOAD("vdu_com__l83.3_0.u26", 0x0000, 0x4000, CRC(2ff53d1d) SHA1(034a89f25dd03e43bdeae710f55d948483624dbe), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("vdu_com__l83.3_1.u39", 0x0001, 0x4000, CRC(5c425046) SHA1(baad928d3189ed6112ed1849511ce230ddb66166), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("vdu_com__l83.3_2.u34", 0x8000, 0x4000, CRC(2ed84e20) SHA1(36fdf5e83b1e8f9b706a221930cec1aa2c587a96), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("vdu_com__l83.3_3.u44", 0x8001, 0x4000, CRC(c601c308) SHA1(61503b5109183a5814be3fcaa393fde8f5bd5359), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

static INPUT_PORTS_START(labtam_vducom)
	PORT_START("E4")
	PORT_DIPNAME(0x08, 0x08, "Keyboard Select 1") PORT_DIPLOCATION("E4:1")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x08, DEF_STR(No))
	PORT_DIPNAME(0x10, 0x10, "Disable Graphics") PORT_DIPLOCATION("E4:2")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x10, DEF_STR(No))
	PORT_DIPNAME(0x04, 0x04, "Keyboard Select 0") PORT_DIPLOCATION("E4:3")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x04, DEF_STR(No))
	PORT_DIPNAME(0x20, 0x20, "Killer Enable") PORT_DIPLOCATION("E4:4")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x20, DEF_STR(No))
	PORT_DIPNAME(0x02, 0x02, "Init NVRAM") PORT_DIPLOCATION("E4:5")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x02, DEF_STR(No))
	PORT_DIPNAME(0x01, 0x01, "Terminal Mode") PORT_DIPLOCATION("E4:7")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x01, DEF_STR(No))
	PORT_DIPUNUSED_DIPLOC(0xc0, 0xc0, "E4:6,8")
INPUT_PORTS_END

static INPUT_PORTS_START(labtam_8086cpu)
	PORT_START("E4")
	PORT_DIPNAME(0x08, 0x08, "Keyboard Select 1") PORT_DIPLOCATION("E4:1")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x08, DEF_STR(No))
	PORT_DIPNAME(0x10, 0x00, "Disable Graphics") PORT_DIPLOCATION("E4:2")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x10, DEF_STR(No))
	PORT_DIPNAME(0x04, 0x04, "Keyboard Select 0") PORT_DIPLOCATION("E4:3")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x04, DEF_STR(No))
	PORT_DIPNAME(0x20, 0x20, "Killer Enable") PORT_DIPLOCATION("E4:4")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x20, DEF_STR(No))
	PORT_DIPNAME(0x02, 0x02, "Init NVRAM") PORT_DIPLOCATION("E4:5")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x02, DEF_STR(No))
	PORT_DIPNAME(0x01, 0x01, "Terminal Mode") PORT_DIPLOCATION("E4:7")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x01, DEF_STR(No))
	PORT_DIPUNUSED_DIPLOC(0xc0, 0xc0, "E4:6,8")
INPUT_PORTS_END

static const gfx_layout vducom_char_set_1 =
{
	8, 12, 64, 1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8, 9 * 8, 10 * 8, 11 * 8 },
	8 * 12
};

static const gfx_layout vducom_char_set_2 =
{
	8, 12, 96, 1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8, 9 * 8, 10 * 8, 11 * 8 },
	8 * 12
};

static GFXDECODE_START(labtam_vducom)
	GFXDECODE_ENTRY("eprom", 0xbc80, vducom_char_set_1, 0, 1)
	GFXDECODE_ENTRY("eprom", 0xc100, vducom_char_set_2, 0, 1)
GFXDECODE_END

const tiny_rom_entry *labtam_vducom_device::device_rom_region() const
{
	return ROM_NAME(labtam_vducom);
}

const tiny_rom_entry *labtam_8086cpu_device::device_rom_region() const
{
	return ROM_NAME(labtam_8086cpu);
}

ioport_constructor labtam_vducom_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(labtam_vducom);
}

ioport_constructor labtam_8086cpu_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(labtam_8086cpu);
}

void labtam_vducom_device_base::device_start()
{
	save_item(NAME(m_start));
	save_item(NAME(m_u7));
}

void labtam_vducom_device_base::device_reset()
{
	if (!m_installed)
		m_installed = true;

	m_u7 = 0;
	m_start = 0;

	m_nvram[0]->recall(1);
	m_nvram[1]->recall(1);

	m_mbus.select(0);

	m_ctc[0]->gate1_w(1);
	m_ctc[0]->gate2_w(1);
	m_ctc[0]->gate3_w(1);
	m_ctc[0]->gate4_w(1);
	m_ctc[0]->gate5_w(1);

	m_ctc[0]->source1_w(1);
	m_ctc[0]->source2_w(1);
	m_ctc[0]->source3_w(1);
	m_ctc[0]->source4_w(1);
	m_ctc[0]->source5_w(1);

	m_ctc[1]->gate4_w(1);
	m_ctc[1]->source5_w(1);
}

void labtam_vducom_device_base::device_add_mconfig(machine_config &config)
{
	I8086(config, m_cpu, 16_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &labtam_vducom_device_base::cpu_mem);
	m_cpu->set_addrmap(AS_IO, &labtam_vducom_device_base::cpu_pio);
	m_cpu->set_irq_acknowledge_callback(m_pic, FUNC(pic8259_device::inta_cb));

	AM9513(config, m_ctc[1], 16_MHz_XTAL / 4);
	m_ctc[1]->out4_cb().set(m_ctc[1], FUNC(am9513_device::gate5_w));
	m_ctc[1]->out5_cb().set(m_ctc[1], FUNC(am9513_device::source4_w)); // TODO: also keyboard bell

	AM9513(config, m_ctc[0], 16_MHz_XTAL / 4);
	m_ctc[0]->out1_cb().set(m_com[1], FUNC(upd7201_device::txcb_w));
	m_ctc[0]->out2_cb().set(m_com[1], FUNC(upd7201_device::rxcb_w));
	m_ctc[0]->out3_cb().set(m_com[1], FUNC(upd7201_device::rxca_w));
	m_ctc[0]->out3_cb().append(m_com[1], FUNC(upd7201_device::txca_w));
	m_ctc[0]->out4_cb().set(m_com[0], FUNC(upd7201_device::rxcb_w));
	m_ctc[0]->out4_cb().append(m_com[0], FUNC(upd7201_device::txcb_w));
	m_ctc[0]->out5_cb().set(m_com[0], FUNC(upd7201_device::rxca_w));
	m_ctc[0]->out5_cb().append(m_com[0], FUNC(upd7201_device::txca_w));

	PIC8259(config, m_pic);
	/*
	 * irq  source
	 *  0   crtc vsync
	 *  1   light pen strobe
	 *  2   com1
	 *  3   com0
	 *  4   parallel input strobe
	 *  5   parallel printer acknowledge
	 *  6   parallel printer busy
	 *  7   not connected
	 */
	m_pic->out_int_callback().set_inputline(m_cpu, INPUT_LINE_INT0);
	int_callback<3>().set(m_pic, FUNC(pic8259_device::ir6_w));

	UPD7201(config, m_com[0], 16_MHz_XTAL / 4);
	m_com[0]->out_int_callback().set(m_pic, FUNC(pic8259_device::ir3_w));

	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);
	input_merger_any_high_device &com0dcda(INPUT_MERGER_ANY_HIGH(config, "com0dcda"));
	input_merger_any_high_device &com0ctsa(INPUT_MERGER_ANY_HIGH(config, "com0ctsa"));
	m_serial[0]->cts_handler().set(com0ctsa, FUNC(input_merger_any_high_device::in_w<0>));
	m_serial[0]->dsr_handler().set(com0ctsa, FUNC(input_merger_any_high_device::in_w<1>));
	com0ctsa.output_handler().set(m_com[0], FUNC(upd7201_device::ctsa_w));
	m_serial[0]->dcd_handler().set(com0dcda, FUNC(input_merger_any_high_device::in_w<0>));
	m_serial[0]->dsr_handler().append(com0dcda, FUNC(input_merger_any_high_device::in_w<1>));
	com0dcda.output_handler().set(m_com[0], FUNC(upd7201_device::dcda_w));
	m_serial[0]->rxd_handler().set(m_com[0], FUNC(upd7201_device::rxa_w));
	m_com[0]->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
	m_com[0]->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_com[0]->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	input_merger_any_high_device &com0dcdb(INPUT_MERGER_ANY_HIGH(config, "com0dcdb"));
	input_merger_any_high_device &com0ctsb(INPUT_MERGER_ANY_HIGH(config, "com0ctsb"));
	m_serial[1]->cts_handler().set(com0ctsb, FUNC(input_merger_any_high_device::in_w<0>));
	m_serial[1]->dsr_handler().set(com0ctsb, FUNC(input_merger_any_high_device::in_w<1>));
	com0ctsb.output_handler().set(m_com[0], FUNC(upd7201_device::ctsb_w));
	m_serial[1]->dcd_handler().set(com0dcdb, FUNC(input_merger_any_high_device::in_w<0>));
	m_serial[1]->dsr_handler().append(com0dcdb, FUNC(input_merger_any_high_device::in_w<1>));
	com0dcdb.output_handler().set(m_com[0], FUNC(upd7201_device::dcdb_w));
	m_serial[1]->rxd_handler().set(m_com[0], FUNC(upd7201_device::rxb_w));
	m_com[0]->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_com[0]->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_com[0]->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));

	UPD7201(config, m_com[1], 16_MHz_XTAL / 4);
	m_com[1]->out_int_callback().set(m_pic, FUNC(pic8259_device::ir2_w));

	RS232_PORT(config, m_serial[2], default_rs232_devices, nullptr);
	input_merger_any_high_device &com1dcda(INPUT_MERGER_ANY_HIGH(config, "com1dcda"));
	input_merger_any_high_device &com1ctsa(INPUT_MERGER_ANY_HIGH(config, "com1ctsa"));
	m_serial[2]->cts_handler().set(com1ctsa, FUNC(input_merger_any_high_device::in_w<0>));
	m_serial[2]->dsr_handler().set(com1ctsa, FUNC(input_merger_any_high_device::in_w<1>));
	com1ctsa.output_handler().set(m_com[1], FUNC(upd7201_device::ctsa_w));
	m_serial[2]->dcd_handler().set(com1dcda, FUNC(input_merger_any_high_device::in_w<0>));
	m_serial[2]->dsr_handler().append(com1dcda, FUNC(input_merger_any_high_device::in_w<1>));
	com1dcda.output_handler().set(m_com[1], FUNC(upd7201_device::dcda_w));
	m_serial[2]->rxd_handler().set(m_com[1], FUNC(upd7201_device::rxa_w));
	m_com[1]->out_dtra_callback().set(m_serial[2], FUNC(rs232_port_device::write_dtr));
	m_com[1]->out_rtsa_callback().set(m_serial[2], FUNC(rs232_port_device::write_rts));
	m_com[1]->out_txda_callback().set(m_serial[2], FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_serial[3], default_rs232_devices, nullptr);
	m_serial[3]->dcd_handler().set(m_com[1], FUNC(upd7201_device::dcdb_w));
	m_serial[3]->cts_handler().set(m_com[1], FUNC(upd7201_device::ctsb_w));
	m_serial[3]->rxd_handler().set(m_com[1], FUNC(upd7201_device::rxb_w));
	m_com[1]->out_dtrb_callback().set(m_serial[3], FUNC(rs232_port_device::write_dtr));
	m_com[1]->out_rtsb_callback().set(m_serial[3], FUNC(rs232_port_device::write_rts));
	m_com[1]->out_txdb_callback().set(m_serial[3], FUNC(rs232_port_device::write_txd));

	X2212(config, m_nvram[0]);
	X2212(config, m_nvram[1]);
}

static DEVICE_INPUT_DEFAULTS_START(keyboard_defaults)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_110)
DEVICE_INPUT_DEFAULTS_END

void labtam_vducom_device::device_add_mconfig(machine_config &config)
{
	labtam_vducom_device_base::device_add_mconfig(config);

	m_serial[3]->set_default_option("keyboard");
	m_serial[3]->set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard_defaults));

	MC6845(config, m_crtc, 16_MHz_XTAL / 16);
	m_crtc->set_show_border_area(false);
	m_crtc->set_hpixels_per_column(16);
	m_crtc->out_vsync_callback().set("pic", FUNC(pic8259_device::ir0_w));

	m_crtc->set_update_row_callback(FUNC(labtam_vducom_device::update_row));
	m_crtc->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(labtam_vducom_device::palette_init), 4);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL / 16, 62 * 16, 2 * 16, 52 * 16, 78 * 4, 3 * 4, 75 * 4);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	// gfxdecode is only to show the font data in the tile viewer
	GFXDECODE(config, "gfx", m_palette, labtam_vducom);
}

void labtam_vducom_device_base::cpu_mem(address_map &map)
{
	map(0x00000, 0x0ffff).ram().share("ram1");
	map(0x10000, 0x1ffff).ram().share("ram0");
	map(0x20000, 0x2ffff).rom().region("eprom", 0).mirror(0x50000);

	map(0x80000, 0xfffff).view(m_mbus);
	m_mbus[0](0x80000, 0x8ffff).rom().region("eprom", 0).mirror(0x70000);
	m_mbus[1](0x80000, 0xfffff).rw(FUNC(labtam_vducom_device_base::bus_mem_r), FUNC(labtam_vducom_device_base::bus_mem_w));
}

void labtam_vducom_device_base::cpu_pio(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(labtam_vducom_device_base::bus_pio_r), FUNC(labtam_vducom_device_base::bus_pio_w));

	map(0xe400, 0xe403).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0xe600, 0xe607).rw(m_com[0], FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask16(0x00ff);
	map(0xe800, 0xe807).rw(m_com[1], FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask16(0x00ff);
	map(0xea00, 0xea03).rw(m_ctc[0], FUNC(am9513_device::read8), FUNC(am9513_device::write8)).umask16(0x00ff);
	map(0xec00, 0xec03).rw(m_ctc[1], FUNC(am9513_device::read8), FUNC(am9513_device::write8)).umask16(0x00ff);
	map(0xee00, 0xefff).rw(FUNC(labtam_vducom_device_base::nvram_r), FUNC(labtam_vducom_device_base::nvram_w)).umask16(0x00ff);

	map(0xf000, 0xf00f).rw(FUNC(labtam_vducom_device_base::u15_r), FUNC(labtam_vducom_device_base::u7_w)).umask16(0x00ff);
	map(0xf200, 0xf200).lw8([this](u8 data) { m_start = (m_start & 0xff00) | u16(data) << 0; }, "start_lo");
	map(0xf400, 0xf400).lw8([this](u8 data) { m_start = (m_start & 0x00ff) | u16(data) << 8; }, "start_hi");
	//map(0xf600, 0xf600); // r:parallel data input, w:joystick start pulse
	//map(0xf800, 0xf800); // r:parallel data acknowledge, w:parallel printer output
	//map(0xfa00, 0xfa00); // w:parallel printer strobe
	map(0xfe00, 0xfe00).rw(FUNC(labtam_vducom_device_base::nvram_recall), FUNC(labtam_vducom_device_base::nvram_store));
}

void labtam_vducom_device::cpu_pio(address_map &map)
{
	labtam_vducom_device_base::cpu_pio(map);

	map(0xe000, 0xe000).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xe002, 0xe002).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

void labtam_vducom_device::palette_init(palette_device &palette)
{
	// 0=black, 1=dim, 2=normal, 3=bold

	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0x80, 0x80, 0x80));
	palette.set_pen_color(2, rgb_t(0xc0, 0xc0, 0xc0));
	palette.set_pen_color(3, rgb_t::white());
}

// non-interlace: two planes 800x300, each 32k, 2bpp -> 4 levels (black, dim, normal, bold)
// interlace: one plane 800x600, 64k, (black, normal)
MC6845_UPDATE_ROW(labtam_vducom_device::update_row)
{
	required_shared_ptr<u16> const ram = m_ram[BIT(u7(), 2)];
	offs_t const offset = (start() >> 1) + ma * 4 + ra * 50;

	for (unsigned x = 0; x < x_count; x++)
	{
		u16 const data0 = ram[(offset + x)];
		u16 const data1 = ram[(offset + x) ^ 0x4000];

		for (unsigned b = 0; b < 16; b++)
		{
			unsigned c = BIT(data0, b) << 1;
			if (u7() & U7_HALF)
				c |= BIT(data1, b);

			bitmap.pix(y, x * 16 + b) = m_palette->pen_color(c);
		}
	}
}

void labtam_vducom_device_base::u7_w(offs_t offset, u8 data)
{
	LOG("u7_w offset %d data %d (%s)\n", offset, data, machine().describe_context());

	if (BIT(data, 0))
		m_u7 |= 1U << offset;
	else
		m_u7 &= ~(1U << offset);

	m_mbus.select(BIT(m_u7, 0));
}

void labtam_vducom_device_base::bus_mem_w(offs_t offset, u16 data, u16 mem_mask)
{
	offs_t const address = (BIT(m_u7, 1) ? 0x80000 : 0) | (offset << 1);

	m_bus->space(AS_PROGRAM).write_word(address, data, mem_mask);
}

u16 labtam_vducom_device_base::bus_mem_r(offs_t offset, u16 mem_mask)
{
	offs_t const address = (BIT(m_u7, 1) ? 0x80000 : 0) | (offset << 1);

	return m_bus->space(AS_PROGRAM).read_word(address, mem_mask);
}

u8 labtam_vducom_device_base::nvram_recall()
{
	m_nvram[0]->recall(1);
	m_nvram[1]->recall(1);

	m_nvram[0]->recall(0);
	m_nvram[1]->recall(0);

	return 0;
}

void labtam_vducom_device_base::nvram_store(u8 data)
{
	m_nvram[0]->store(1);
	m_nvram[1]->store(1);

	m_nvram[0]->store(0);
	m_nvram[1]->store(0);
}
