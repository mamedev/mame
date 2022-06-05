// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Labtam 3000 8086 VDU COMM card.
 *
 * Sources:
 *  - https://arvutimuuseum.ut.ee/index.php?m=eksponaadid&id=223
 *
 * TODO:
 *  - skeleton only
 */

#include "emu.h"
#include "labtam_vducom.h"

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

DEFINE_DEVICE_TYPE(LABTAM_VDUCOM, labtam_vducom_device, "labtam_vducom", "Labtam 8086 VDU COMM")

labtam_vducom_device::labtam_vducom_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LABTAM_VDUCOM, tag, owner, clock)
	, device_multibus_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_pic(*this, "pic")
	, m_ctc(*this, "ctc%u", 0U)
	, m_com(*this, "com%u", 0U)
	, m_nvram(*this, "nvram%u", 0U)
	, m_crtc(*this, "crtc")
	, m_screen(*this, "screen")
	, m_ram(*this, "ram%u", 0U)
	, m_e4(*this, "E4")
	, m_mbus(*this, "mbus")
	, m_installed(false)
{
}

ROM_START(labtam_vducom)
	ROM_REGION16_LE(0x10000, "eprom", 0)
	ROM_LOAD16_BYTE("vdu_com__k84.1-0.u26", 0x0000, 0x4000, CRC(ac27697e) SHA1(eeec9cff06181dffe95a37aefb2b3789a0959b4f))
	ROM_LOAD16_BYTE("vdu_com__k84.1-1.u39", 0x0001, 0x4000, CRC(8b3e567d) SHA1(5ce821004155d02b87ed7b6d14989a9fffc38c6a))
	ROM_LOAD16_BYTE("vdu_com__k84.1-2.u34", 0x8000, 0x4000, CRC(70a2c615) SHA1(67d265be3101dfe33fa705447e1394410e55f518))
	ROM_LOAD16_BYTE("vdu_com__k84.1-3.u44", 0x8001, 0x4000, CRC(dcd7ce03) SHA1(727e5fa72d6088ddf9e8613656d713754d628538))
ROM_END

static INPUT_PORTS_START(labtam_vducom)
	PORT_START("E4")
	PORT_DIPNAME(0x01, 0x01, "Keyboard Select 1") PORT_DIPLOCATION("E4:1")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x01, DEF_STR(No))
	PORT_DIPNAME(0x02, 0x02, "Disable Graphics") PORT_DIPLOCATION("E4:2")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x02, DEF_STR(No))
	PORT_DIPNAME(0x04, 0x04, "Keyboard Select 0") PORT_DIPLOCATION("E4:3")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x04, DEF_STR(No))
	PORT_DIPNAME(0x08, 0x08, "Killer Enable") PORT_DIPLOCATION("E4:4")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x08, DEF_STR(No))
	PORT_DIPNAME(0x10, 0x10, "Init NVRAM") PORT_DIPLOCATION("E4:5")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x10, DEF_STR(No))
	PORT_DIPNAME(0x40, 0x40, "Terminal Mode") PORT_DIPLOCATION("E4:7")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x40, DEF_STR(No))
	PORT_DIPUNUSED_DIPLOC(0xa0, 0xa0, "E4:6,8")
INPUT_PORTS_END

const tiny_rom_entry *labtam_vducom_device::device_rom_region() const
{
	return ROM_NAME(labtam_vducom);
}

ioport_constructor labtam_vducom_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(labtam_vducom);
}

void labtam_vducom_device::device_resolve_objects()
{
	m_bus->int_callback<3>().set(m_pic, FUNC(pic8259_device::ir6_w));
}

void labtam_vducom_device::device_start()
{
	save_item(NAME(m_start));
	save_item(NAME(m_u7));
}

void labtam_vducom_device::device_reset()
{
	if (!m_installed)
		m_installed = true;

	m_u7 = 0;
	m_start = 0;

	m_nvram[0]->recall(1);
	m_nvram[1]->recall(1);

	m_mbus.select(0);
}

void labtam_vducom_device::device_add_mconfig(machine_config &config)
{
	I8086(config, m_cpu, 16_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &labtam_vducom_device::cpu_mem);
	m_cpu->set_addrmap(AS_IO, &labtam_vducom_device::cpu_pio);
	m_cpu->set_irq_acknowledge_callback(m_pic, FUNC(pic8259_device::inta_cb));

	AM9513(config, m_ctc[0], 4'000'000);
	//m_ctc[0]->out1_cb().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_ctc[0]->out4_cb().set(m_ctc[0], FUNC(am9513_device::gate5_w)); // ?
	m_ctc[0]->out5_cb().set(m_ctc[0], FUNC(am9513_device::source4_w)); // ?

	AM9513(config, m_ctc[1], 4'000'000);
	m_ctc[1]->out1_cb().set(m_com[1], FUNC(upd7201_device::txcb_w));
	m_ctc[1]->out2_cb().set(m_com[1], FUNC(upd7201_device::rxcb_w));
	m_ctc[1]->out3_cb().set(m_com[1], FUNC(upd7201_device::rxca_w));
	m_ctc[1]->out3_cb().append(m_com[1], FUNC(upd7201_device::txca_w));
	m_ctc[1]->out4_cb().set(m_com[0], FUNC(upd7201_device::rxcb_w));
	m_ctc[1]->out5_cb().set(m_com[0], FUNC(upd7201_device::rxca_w));
	m_ctc[1]->out5_cb().append(m_com[0], FUNC(upd7201_device::txca_w));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_cpu, INPUT_LINE_INT0);

	UPD7201(config, m_com[0], 4'000'000);
	m_com[0]->out_int_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	UPD7201(config, m_com[1], 4'000'000);
	m_com[1]->out_int_callback().set(m_pic, FUNC(pic8259_device::ir2_w));

	X2212(config, m_nvram[0]);
	X2212(config, m_nvram[1]);

	MC6845(config, m_crtc, 1'000'000);
	m_crtc->set_show_border_area(false);
	m_crtc->set_hpixels_per_column(16);
	m_crtc->set_update_row_callback(FUNC(labtam_vducom_device::update_row));
	m_crtc->set_screen(m_screen);
	m_crtc->out_vsync_callback().set(m_pic, FUNC(pic8259_device::ir0_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(1'000'000, 62*16, 2*16, 52*16, 78*4, 3*4, 75*4);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));
}

void labtam_vducom_device::cpu_mem(address_map &map)
{
	map(0x00000, 0x0ffff).ram().share("ram1");
	map(0x10000, 0x1ffff).ram().share("ram0");
	map(0x20000, 0x2ffff).rom().region("eprom", 0).mirror(0x50000);

	map(0x80000, 0xfffff).view(m_mbus);
	m_mbus[0](0x80000, 0x8ffff).rom().region("eprom", 0).mirror(0x70000);
	m_mbus[1](0x80000, 0xfffff).rw(FUNC(labtam_vducom_device::bus_mem_r), FUNC(labtam_vducom_device::bus_mem_w));
}

void labtam_vducom_device::cpu_pio(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(labtam_vducom_device::bus_pio_r), FUNC(labtam_vducom_device::bus_pio_w));

	map(0xe000, 0xe000).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xe002, 0xe002).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xe400, 0xe403).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0xe600, 0xe607).rw(m_com[0], FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask16(0x00ff);
	map(0xe800, 0xe807).rw(m_com[1], FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask16(0x00ff);
	map(0xea00, 0xea03).rw(m_ctc[0], FUNC(am9513_device::read8), FUNC(am9513_device::write8)).umask16(0x00ff);
	map(0xec00, 0xec03).rw(m_ctc[1], FUNC(am9513_device::read8), FUNC(am9513_device::write8)).umask16(0x00ff);
	map(0xee00, 0xefff).rw(FUNC(labtam_vducom_device::nvram_r), FUNC(labtam_vducom_device::nvram_w)).umask16(0x00ff);

	map(0xf000, 0xf00f).rw(FUNC(labtam_vducom_device::u7_r), FUNC(labtam_vducom_device::u7_w)).umask16(0x00ff);
	map(0xf200, 0xf200).lw8([this](u8 data) { m_start = (m_start & 0xff00) | u16(data) << 0; }, "start_lo");
	map(0xf400, 0xf400).lw8([this](u8 data) { m_start = (m_start & 0x00ff) | u16(data) << 8; }, "start_hi");
	map(0xfe00, 0xfe00).lw8([this](u8 data) { m_nvram[0]->store(1); m_nvram[1]->store(1); }, "store_array");
}

// non-interlace: two planes 800x300, each 32k, 2bpp -> 4 levels (black, dim, normal, bold)
// interlace: one plane 800x600, 64k, (black, normal)
MC6845_UPDATE_ROW(labtam_vducom_device::update_row)
{
	// 0=black, 1=dim, 2=normal, 3=bold
	static const rgb_t color[4] = { rgb_t::black(), rgb_t(0x80,0x80,0x80), rgb_t(0xc0,0xc0,0xc0), rgb_t::white() };

	required_shared_ptr<u16> const ram = m_ram[BIT(m_u7, 2)];
	offs_t const offset = (m_start >> 1) + ma * 4 + ra * 50;

	for (unsigned x = 0; x < x_count; x++)
	{
		u16 const data0 = ram[(offset + x)];
		u16 const data1 = ram[(offset + x) ^ 0x4000];

		for (unsigned b = 0; b < 16; b++)
		{
			unsigned c = BIT(data0, b) << 1;
			if (m_u7 & U7_HALF)
				c |= BIT(data1, b);

			bitmap.pix(y, x * 16 + b) = color[c];
		}
	}
}

void labtam_vducom_device::u7_w(offs_t offset, u8 data)
{
	LOG("u7_w offset %d data %d (%s)\n", offset, data, machine().describe_context());

	if (BIT(data, 0))
		m_u7 |= 1U << offset;
	else
		m_u7 &= ~(1U << offset);

	m_mbus.select(BIT(m_u7, 0));
}

void labtam_vducom_device::bus_mem_w(offs_t offset, u16 data, u16 mem_mask)
{
	offs_t const address = (BIT(m_u7, 1) ? 0x80000 : 0) | (offset << 1);

	m_bus->space(AS_PROGRAM).write_word(address, data, mem_mask);
}

u16 labtam_vducom_device::bus_mem_r(offs_t offset, u16 mem_mask)
{
	offs_t const address = (BIT(m_u7, 1) ? 0x80000 : 0) | (offset << 1);

	return m_bus->space(AS_PROGRAM).read_word(address, mem_mask);
}
