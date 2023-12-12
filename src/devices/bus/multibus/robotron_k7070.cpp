// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

/*
 * Robotron K7070 and K7072 combo
 *
 * K7070    KGS - graphics terminal (K7070.10 model)
 * K7072    ABG - dumb grayscale framebuffer
 *
 * Reference: http://www.tiffe.de/Robotron/MMS16/
 * Internal test of KGS -- in KGS-K7070.pdf, pp. 19-23
 *
 * To do:
 * - palette
 * - vertical raster split and text mode
 */

#include "emu.h"
#include "robotron_k7070.h"

#define VERBOSE 0
#include "logmacro.h"

enum kgs_st : u8
{
	KGS_ST_OBF = 0x01,
	KGS_ST_IBF = 0x02,
	KGS_ST_INT = 0x04,
	KGS_ST_ERR = 0x80,
};

DEFINE_DEVICE_TYPE(ROBOTRON_K7070, robotron_k7070_device, "robotron_k7070", "Robotron K7070 KGS")

robotron_k7070_device::robotron_k7070_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ROBOTRON_K7070, tag, owner, clock)
	, device_multibus_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_ctc(*this, "ctc")
	, m_sio(*this, "sio")
	, m_serial(*this, "serial%u", 0U)
	, m_palette(*this, "palette")
	, m_ram(*this, "vram0")
	, m_dsel(*this, "DSEL%u", 0U)
	, m_view_lo(*this, "view_lo")
	, m_view_hi(*this, "view_hi")
{
	m_abg_msel = m_kgs_iml = 0;
	m_abg_func = m_abg_split = 0;
	m_abg_addr = 0;
	m_kgs_datao = m_kgs_datai = m_kgs_ctrl = 0;
	m_nmi_enable = 0;
}

ROM_START(robotron_k7070)
	ROM_REGION(0x2000, "eprom", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("152")

	ROM_SYSTEM_BIOS(0, "152", "Version 152") // ROM from A7100
	ROMX_LOAD("kgs7070-152.bin", 0x0000, 0x2000, CRC(403f4235) SHA1(d07ccd40f8b600651d513f588bcf1ea4f15ed094), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "153", "Version 153")
	ROMX_LOAD("kgs7070-153.rom", 0x0000, 0x2000, CRC(a72fe820) SHA1(4b77ab2b59ea8c3632986847ff359df26b16196b), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "154", "Version 154")
	ROMX_LOAD("kgs7070-154.rom", 0x0000, 0x2000, CRC(2995ade0) SHA1(62516f2e1cb62698445f80fd823d39a1a78a7807), ROM_BIOS(2))
ROM_END

static INPUT_PORTS_START(robotron_k7070)
	PORT_START("DSEL0")
	PORT_DIPNAME(0x01, 0x01, "Codepoint 0x24")
	PORT_DIPSETTING(0x00, "Currency sign" )
	PORT_DIPSETTING(0x01, "Dollar sign" )
	PORT_DIPNAME(0x02, 0x02, "Perform I/O test")
	PORT_DIPSETTING(0x00, DEF_STR(No) )
	PORT_DIPSETTING(0x02, DEF_STR(Yes) )
	PORT_DIPNAME(0x04, 0x00, "Perform VRAM test")
	PORT_DIPSETTING(0x00, DEF_STR(Yes) )
	PORT_DIPSETTING(0x04, DEF_STR(No) )

	PORT_START("DSEL1")
	PORT_DIPNAME(0x03, 0x02, "V.24 Parity")
	PORT_DIPSETTING(0x00, "No parity" )
	PORT_DIPSETTING(0x01, "Odd" )
	PORT_DIPSETTING(0x02, "No parity" )
	PORT_DIPSETTING(0x03, "Even" )
	PORT_DIPNAME(0x04, 0x04, "V.24 Character size")
	PORT_DIPSETTING(0x00, "7 bits")
	PORT_DIPSETTING(0x04, "8 bits")
	PORT_DIPNAME(0x38, 0x38, "V.24 Baud rate")
	PORT_DIPSETTING(0x38, "19200")
	PORT_DIPSETTING(0x30, "9600")
	PORT_DIPSETTING(0x28, "4800")
	PORT_DIPSETTING(0x20, "2400")
	PORT_DIPSETTING(0x18, "1200")
	PORT_DIPSETTING(0x10, "600")
	PORT_DIPSETTING(0x08, "300")
	PORT_DIPNAME(0x40, 0x40, "IFSS Parity")
	PORT_DIPSETTING(0x00, "Odd" )
	PORT_DIPSETTING(0x40, "Even" )
	PORT_DIPNAME(0x80, 0x80, "IFSS Baud rate")
	PORT_DIPSETTING(0x00, "9600")
	PORT_DIPSETTING(0x80, "Same as V.24")
INPUT_PORTS_END

const tiny_rom_entry *robotron_k7070_device::device_rom_region() const
{
	return ROM_NAME(robotron_k7070);
}

ioport_constructor robotron_k7070_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(robotron_k7070);
}

void robotron_k7070_device::device_start()
{
	save_item(NAME(m_start));
	save_item(NAME(m_abg_msel));
	save_item(NAME(m_kgs_iml));
	save_item(NAME(m_kgs_datao));
	save_item(NAME(m_kgs_datai));
	save_item(NAME(m_kgs_ctrl));
	save_item(NAME(m_nmi_enable));

	m_bus->space(AS_IO).install_readwrite_handler(0x200, 0x203, read16s_delegate(*this, FUNC(robotron_k7070_device::io_r)), write16s_delegate(*this, FUNC(robotron_k7070_device::io_w)));
}

void robotron_k7070_device::device_reset()
{
	m_start = 0;
	m_kgs_ctrl = KGS_ST_IBF | KGS_ST_OBF;
	m_kgs_datao = m_kgs_datai = 0;
	m_kgs_iml = m_abg_msel = 0;
	m_nmi_enable = false;
	kgs_memory_remap();
}

static const z80_daisy_config k7070_daisy_chain[] =
{
	{ "sio" },
	{ "ctc" },
	{ nullptr }
};

void robotron_k7070_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, XTAL(16'000'000) / 4);
	m_cpu->set_addrmap(AS_PROGRAM, &robotron_k7070_device::cpu_mem);
	m_cpu->set_addrmap(AS_IO, &robotron_k7070_device::cpu_pio);
	m_cpu->set_daisy_config(k7070_daisy_chain);

	Z80CTC(config, m_ctc, 16_MHz_XTAL / 3);
	m_ctc->intr_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(1230750);
	m_ctc->set_clk<1>(1230750);
	m_ctc->set_clk<2>(1230750);
	m_ctc->set_clk<3>(1230750);
	m_ctc->zc_callback<0>().set(m_sio, FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<0>().append(m_sio, FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<1>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	Z80SIO(config, m_sio, XTAL(16'000'000) / 4);
	m_sio->out_int_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	m_sio->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set([this] (bool state) { m_kgs_iml = !state; kgs_memory_remap(); });

	// V.24 port for tablet
	RS232_PORT(config, m_serial[0], default_rs232_devices, "loopback");
	m_serial[0]->rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	m_serial[0]->dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));
	m_serial[0]->cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));

	// IFSS (current loop) port for keyboard (unused on A7150)
	RS232_PORT(config, m_serial[1], default_rs232_devices, "loopback");
	m_serial[1]->rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(XTAL(16'000'000), 737, 0, 640, 431, 0, 400);
	screen.set_screen_update(FUNC(robotron_k7070_device::screen_update_k7072));
	screen.set_palette(m_palette);
	screen.screen_vblank().set([this] (bool state) { if (m_nmi_enable) m_cpu->set_input_line(INPUT_LINE_NMI, state); });

	PALETTE(config, m_palette, palette_device::MONOCHROME);
}

void robotron_k7070_device::cpu_mem(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x1fff).view(m_view_lo);
	m_view_lo[0](0x0000, 0x1fff).rom().region("eprom", 0);
	m_view_lo[1](0x0000, 0x1fff).ram().share("kgs_ram0");

	map(0x2000, 0x7fff).ram().share("kgs_ram1");

// FIXME handle IML=0 and MSEL=1 (no access to VRAM)
	map(0x8000, 0xffff).view(m_view_hi);
	m_view_hi[0](0x8000, 0xffff).ram().share("kgs_ram2");
	m_view_hi[1](0x8000, 0xffff).ram().share("vram0");
}

void robotron_k7070_device::cpu_pio(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	map(0x0000, 0x0003).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0008, 0x000b).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x0010, 0x0017).rw(FUNC(robotron_k7070_device::kgs_host_r), FUNC(robotron_k7070_device::kgs_host_w)); // p. 11 of KGS-K7070.pdf

	// p. 6 of ABG-K7072.pdf
	map(0x0020, 0x0021).w(FUNC(robotron_k7070_device::abg_addr_w));
	map(0x0022, 0x0022).w(FUNC(robotron_k7070_device::abg_func_w));
	map(0x0023, 0x0023).w(FUNC(robotron_k7070_device::abg_split_w));
	map(0x0030, 0x003f).noprw(); // palette register

	map(0x0080, 0x0080).w(FUNC(robotron_k7070_device::abg_misc_w));
}

void robotron_k7070_device::palette_init(palette_device &palette)
{
	// 0=black, 1=dim, 2=normal, 3=bold

	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0x80, 0x80, 0x80));
	palette.set_pen_color(2, rgb_t(0xc0, 0xc0, 0xc0));
	palette.set_pen_color(3, rgb_t::white());
}

uint16_t robotron_k7070_device::io_r(offs_t offset, uint16_t mem_mask)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_kgs_ctrl;
		break;

	case 1:
		data = m_kgs_datai;
		if (!machine().side_effects_disabled())
			m_kgs_ctrl &= ~KGS_ST_OBF;
		break;
	}

	if (offset && !machine().side_effects_disabled())
	{
		LOG("%s: KGS %d == %02x '%c'\n", machine().describe_context(),
				offset, data, (data > 0x1f && data < 0x7f) ? data : 0x20);
	}

	return data;
}

void robotron_k7070_device::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset)
	{
		LOG("%s: KGS %d <- %02x '%c', ctrl %02x\n", machine().describe_context(),
				offset, data, (data > 0x1f && data < 0x7f) ? data : 0x20, m_kgs_ctrl);
	}

	switch (offset)
	{
	case 0:
		m_kgs_ctrl &= ~(KGS_ST_ERR | KGS_ST_INT);
		int_w(7, CLEAR_LINE);
		break;

	case 1:
		if (m_kgs_ctrl & KGS_ST_IBF)
		{
			m_kgs_ctrl |= KGS_ST_ERR;
		}
		else
		{
			m_kgs_datao = data;
			m_kgs_ctrl |= KGS_ST_IBF;
		}
		break;
	}
}

uint32_t robotron_k7070_device::screen_update_k7072(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int addr = m_start;

	for (int y = 0; y < 400; y++)
	{
		int horpos = 0;
		for (int x = 0; x < 80; x++)
		{
			uint8_t code = m_ram[addr++ % 32768];
			for (int b = 0; b < 8; b++)
			{
				bitmap.pix(y, horpos++) = BIT(code, 7 - b);
			}
		}
	}

	return 0;
}


uint8_t robotron_k7070_device::kgs_host_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_kgs_datao;
		if (!machine().side_effects_disabled())
			m_kgs_ctrl &= ~(KGS_ST_ERR | KGS_ST_IBF);
		break;

	case 2:
		data = m_kgs_ctrl;
		break;

	case 6:
		data = m_dsel[0]->read();
		break;

	case 7:
		data = m_dsel[1]->read();
		break;

	default:
		break;
	}

	if (offset != 2 && offset != 5 && !machine().side_effects_disabled())
	{
		LOG("%s: kgs %d == %02x '%c'\n", machine().describe_context(),
				offset, data, (data > 0x1f && data < 0x7f) ? data : ' ');
	}

	return data;
}

void robotron_k7070_device::kgs_host_w(offs_t offset, uint8_t data)
{
	if (offset != 2 && offset != 5)
	{
		LOG("%s: kgs %d <- %02x '%c', ctrl %02x\n", machine().describe_context(),
				offset, data, (data > 0x1f && data < 0x7f) ? data : ' ', m_kgs_ctrl);
	}

	switch (offset)
	{
	case 1:
		if (m_kgs_ctrl & KGS_ST_OBF)
		{
			m_kgs_ctrl |= KGS_ST_ERR;
		}
		else
		{
			m_kgs_datai = data;
			m_kgs_ctrl |= KGS_ST_OBF;
		}
		break;

	case 3:
		m_kgs_ctrl |= KGS_ST_INT;
		int_w(7, ASSERT_LINE);
		break;

	case 4:
		m_kgs_ctrl |= KGS_ST_ERR;
		break;

	case 5:
		m_abg_msel = data;
		kgs_memory_remap();
		break;
	}
}

void robotron_k7070_device::abg_addr_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
		m_start |= ((data & 127) << 8);
	else
		m_start = data;
}

/*
 * bit 0:  GRAF  This bit must be set if both video levels are to be output in graphics display mode or if the palette registers are to be programmed.
 * bit 1: /PALP  This bit must be set to program the palette register.
 * bit 2: /KGSP  If this bit is reset, the frame buffer can be accessed asynchronously.
 * bit 3:  BLNK  Switching this bit causes the points (byte by byte) in which the attribute bit of video level 2 (bit 1) is set to flash in alphanumeric display mode.
 */
void robotron_k7070_device::abg_func_w(offs_t offset, uint8_t data)
{
	LOG("%s: abg func <- %02x\n", machine().describe_context(), data);
	m_abg_func = data;
}

void robotron_k7070_device::abg_split_w(offs_t offset, uint8_t data)
{
	LOG("%s: abg split <- %02x (%d -> %d)\n", machine().describe_context(), data, data, data*2);
	m_abg_split = data;
}

void robotron_k7070_device::abg_misc_w(offs_t offset, uint8_t data)
{
	LOG("%s: abg misc <- %02x\n", machine().describe_context(), data);
	m_nmi_enable = BIT(data, 7);
}

void robotron_k7070_device::kgs_memory_remap()
{
	LOG("%s: kgs memory: iml %d msel %d\n", machine().describe_context(), m_kgs_iml, m_abg_msel);

	m_view_lo.select(m_kgs_iml);
	m_view_hi.select(m_abg_msel != 0);
}
