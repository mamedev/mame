// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    MTX SDX Controller

**********************************************************************/


#include "emu.h"
#include "sdx.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MTX_SDXBAS, mtx_sdxbas_device, "mtx_sdxbas", "MTX SDX Controller (BASIC)")
DEFINE_DEVICE_TYPE(MTX_SDXCPM, mtx_sdxcpm_device, "mtx_sdxcpm", "MTX SDX Controller (CP/M)")


//-------------------------------------------------
//  SLOT_INTERFACE( sdx_floppies )
//-------------------------------------------------

static void sdx_floppies(device_slot_interface &device)
{
	device.option_add("35dd",  FLOPPY_35_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}

FLOPPY_FORMATS_MEMBER(mtx_sdx_device::floppy_formats)
	FLOPPY_MTX_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  ROM( sdx )
//-------------------------------------------------

ROM_START( sdxbas )
	ROM_REGION(0x2000, "sdx_rom", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("sdx07")
	ROM_SYSTEM_BIOS(0, "sdx07", "Type 07")
	ROMX_LOAD("sdxbas07.rom", 0x0000, 0x2000, CRC(db88b245) SHA1(05c89db8e39ec3165b4620432f48e1d59abe10dd), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "sdx03", "Type 03")
	ROMX_LOAD("sdxbas03.rom", 0x0000, 0x2000, CRC(2fc46a46) SHA1(f08e6a8cffbb3ca39633be6e9958bec85d1e5981), ROM_BIOS(1))
ROM_END

ROM_START( sdxcpm )
	ROM_REGION(0x2000, "sdx_rom", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("sdx07")
	ROM_SYSTEM_BIOS(0, "sdx07", "SDX07 CP/M")
	ROMX_LOAD("sdxcpm07.rom", 0x0000, 0x2000, CRC(622a04ea) SHA1(c633ce1054b45afda53116e0c6e272a1ae6a2155), ROM_BIOS(0))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("80z_7a.bin", 0x0000, 0x1000, CRC(ea6fe865) SHA1(f84883f79bed34501e5828336894fad929bddbb5)) // alpha
	ROM_LOAD("80z_9a.bin", 0x1000, 0x1000, NO_DUMP) // graphic
ROM_END

//-------------------------------------------------
//  INPUT_PORTS( sdx )
//-------------------------------------------------

INPUT_PORTS_START( sdx )
	PORT_START("DSW0")
	PORT_DIPNAME(0x01, 0x00, "Drive A: Head-load solenoid present") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x01, DEF_STR(No))
	PORT_DIPNAME(0x02, 0x00, "Drive A: Double-sided drive") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x02, DEF_STR(No))
	PORT_DIPNAME(0x04, 0x00, "Drive A: 96 TPI drive") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x04, DEF_STR(No))
	PORT_DIPNAME(0x08, 0x00, "Drive A: Stepping rate") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x00, "Stepping rate 6ms")
	PORT_DIPSETTING(0x08, "Stepping rate 12ms")

	PORT_START("DSW1")
	PORT_DIPNAME(0x01, 0x00, "Drive B: Head-load solenoid present") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x01, DEF_STR(No))
	PORT_DIPNAME(0x02, 0x00, "Drive B: Double-sided drive") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x02, DEF_STR(No))
	PORT_DIPNAME(0x04, 0x00, "Drive B: 96 TPI drive") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x00, DEF_STR(Yes))
	PORT_DIPSETTING(0x04, DEF_STR(No))
	PORT_DIPNAME(0x08, 0x00, "Drive B: Stepping rate") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x00, "Stepping rate 6ms")
	PORT_DIPSETTING(0x08, "Stepping rate 12ms")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor mtx_sdx_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sdx);
}

//-------------------------------------------------
//  gfx_layout mtx_sdx_charlayout
//-------------------------------------------------

static const gfx_layout mtx_sdx_charlayout =
{
	8, 10,                  /* 8 x 10 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0 * 8,  1 * 8,  2 * 8,  3 * 8,  4 * 8,  5 * 8,  6 * 8,  7 * 8, 8 * 8,  9 * 8 },
	8 * 16                  /* every char takes 16 bytes */
};

//-------------------------------------------------
//  GFXDECODE( gfx_mtx_sdx )
//-------------------------------------------------

static GFXDECODE_START(gfx_mtx_sdx)
	GFXDECODE_ENTRY("chargen", 0, mtx_sdx_charlayout, 0, 8)
GFXDECODE_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mtx_sdxbas_device::device_add_mconfig(machine_config &config)
{
	/* fdc */
	MB8877(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->hld_wr_callback().set(FUNC(mtx_sdx_device::motor_w));

	FLOPPY_CONNECTOR(config, "fdc:0", sdx_floppies, "525qd", mtx_sdx_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", sdx_floppies, "525qd", mtx_sdx_device::floppy_formats).enable_sound(true);
}

void mtx_sdxcpm_device::device_add_mconfig(machine_config &config)
{
	/* fdc */
	MB8877(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->hld_wr_callback().set(FUNC(mtx_sdx_device::motor_w));

	FLOPPY_CONNECTOR(config, "fdc:0", sdx_floppies, "525qd", mtx_sdx_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", sdx_floppies, "525qd", mtx_sdx_device::floppy_formats).enable_sound(true);

	/* 80 column video card - required to be installed in MTX internally */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_refresh_hz(50);
	m_screen->set_size(960, 313);
	m_screen->set_visarea(00, 640 - 1, 0, 240 - 1);
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_mtx_sdx);
	PALETTE(config, "palette", palette_device::RGB_3BIT);

	MC6845(config, m_crtc, 15_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(mtx_sdxcpm_device::crtc_update_row));
}


const tiny_rom_entry *mtx_sdxbas_device::device_rom_region() const
{
	return ROM_NAME( sdxbas );
}

const tiny_rom_entry *mtx_sdxcpm_device::device_rom_region() const
{
	return ROM_NAME( sdxcpm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mtx_sdx_device - constructor
//-------------------------------------------------

mtx_sdx_device::mtx_sdx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mtx_exp_interface(mconfig, *this)
	, m_sdx_rom(*this, "sdx_rom")
	, m_fdc(*this, "fdc")
	, m_floppy0(*this, "fdc:0")
	, m_floppy1(*this, "fdc:1")
	, m_dsw(*this, "DSW%u", 0)
{
}

mtx_sdxbas_device::mtx_sdxbas_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mtx_sdx_device(mconfig, MTX_SDXBAS, tag, owner, clock)
{
}

mtx_sdxcpm_device::mtx_sdxcpm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mtx_sdx_device(mconfig, MTX_SDXCPM, tag, owner, clock)
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
	, m_crtc(*this, "crtc")
	, m_char_rom(*this, "chargen")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mtx_sdxbas_device::device_start()
{
	save_item(NAME(m_control));
}

void mtx_sdxcpm_device::device_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_80col_ascii));
	save_item(NAME(m_80col_attr));
	save_item(NAME(m_80col_addr));
	save_item(NAME(m_80col_char_ram));
	save_item(NAME(m_80col_attr_ram));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mtx_sdxbas_device::device_reset()
{
	machine().root_device().membank("rommap_bank1")->configure_entry(3, m_sdx_rom->base());

	/* SDX FDC */
	io_space().install_readwrite_handler(0x10, 0x13, read8sm_delegate(*m_fdc, FUNC(mb8877_device::read)), write8sm_delegate(*m_fdc, FUNC(mb8877_device::write)));
	io_space().install_readwrite_handler(0x14, 0x14, read8_delegate(*this, FUNC(mtx_sdx_device::sdx_status_r)), write8_delegate(*this, FUNC(mtx_sdx_device::sdx_control_w)));
}

void mtx_sdxcpm_device::device_reset()
{
	machine().root_device().membank("rommap_bank1")->configure_entry(3, m_sdx_rom->base());

	/* SDX FDC */
	io_space().install_readwrite_handler(0x10, 0x13, read8sm_delegate(*m_fdc, FUNC(mb8877_device::read)), write8sm_delegate(*m_fdc, FUNC(mb8877_device::write)));
	io_space().install_readwrite_handler(0x14, 0x14, read8_delegate(*this, FUNC(mtx_sdx_device::sdx_status_r)), write8_delegate(*this, FUNC(mtx_sdx_device::sdx_control_w)));

	/* 80 column */
	io_space().install_readwrite_handler(0x30, 0x33, read8_delegate(*this, FUNC(mtx_sdxcpm_device::mtx_80col_r)), write8_delegate(*this, FUNC(mtx_sdxcpm_device::mtx_80col_w)));
	io_space().install_readwrite_handler(0x38, 0x38, read8smo_delegate(*m_crtc, FUNC(mc6845_device::status_r)), write8smo_delegate(*m_crtc, FUNC(mc6845_device::address_w)));
	io_space().install_readwrite_handler(0x39, 0x39, read8smo_delegate(*m_crtc, FUNC(mc6845_device::register_r)), write8smo_delegate(*m_crtc, FUNC(mc6845_device::register_w)));

	memset(m_80col_char_ram, 0, sizeof(m_80col_char_ram));
	memset(m_80col_attr_ram, 0, sizeof(m_80col_attr_ram));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(mtx_sdx_device::sdx_status_r)
{
	/*
	bit     description
	0       Head load: 1 - head load on drive
	1       Double-sided: 1 if drive double-sided
	2       TPI: 0 - 48 TPI drive. 1 - 96 TPI drive
	3       Track stepping rate: 0 - 12ms, 1 - 6ms
	4       No. of drives: 0 - 1 drive, 1 - 2 drives
	5       Ready: 1 - drive ready
	6       Interrupt: 1 - FDC interrupt request
	7       Data request: 1 - FDC data request
	*/

	uint8_t data = 0x00;

	data |= m_dsw[BIT(m_control, 0)].read_safe(0x0f) & 0x0f;

	data |= (m_floppy0->get_device() && m_floppy1->get_device()) ? 0x10 : 0x00;

	if (m_floppy)
		data |= m_floppy->ready_r() ? 0x00 : 0x20;

	data |= m_fdc->intrq_r() ? 0x40 : 0x00;
	data |= m_fdc->drq_r() ? 0x80 : 0x00;

	return data;
}

WRITE8_MEMBER(mtx_sdx_device::sdx_control_w)
{
	/*
	bit     description
	0       Drive select: 0 - drive A, 1 - drive B
	1       Side select: 0 - side 0, 1 - side 1
	2       Motor on: 1 - turns drive motor on
	3       Motor ready: 1 - drive motor ready
	4       Density: 0 - FM, 1 - MFM
	*/

	m_control = data;

	/* bit 0: drive select */
	m_floppy = BIT(data, 0) ? m_floppy1->get_device() : m_floppy0->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		/* bit 1: side select */
		m_floppy->ss_w(BIT(data, 1));
		logerror("motor on %d\n", BIT(data, 2));
		/* bit 2: motor on */
		m_floppy->mon_w(!(BIT(data, 2) || m_fdc->hld_r()));
		logerror("head load %d\n", m_fdc->hld_r());
		/* bit 3: motor ready */
		//if (BIT(data, 3))
			//m_floppy->mon_w(!BIT(data, 2));
			//m_floppy->mon_w(!BIT(data, 3));
		logerror("motor ready %d\n", BIT(data, 3));
	}

	/* bit 4: density */
	m_fdc->dden_w(!BIT(data, 4));
}

WRITE_LINE_MEMBER(mtx_sdx_device::motor_w)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(0);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(0);
}

//-------------------------------------------------
//  80 column video board
//-------------------------------------------------

READ8_MEMBER(mtx_sdxcpm_device::mtx_80col_r)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0:
		/* ring the bell */
		break;
	case 2:
		if (!BIT(m_80col_addr, 15))
			data = m_80col_char_ram[m_80col_addr & 0x07ff];
		break;
	case 3:
		if (!BIT(m_80col_addr, 15))
			data = m_80col_attr_ram[m_80col_addr & 0x07ff];
		break;
	}
	return data;
}

WRITE8_MEMBER(mtx_sdxcpm_device::mtx_80col_w)
{
	switch (offset)
	{
	case 0:
		m_80col_addr = (m_80col_addr & 0xff00) | data;
		/* write to ram */
		if (BIT(m_80col_addr, 15))
		{
			/* write enable ascii ram */
			if (BIT(m_80col_addr, 14))
				m_80col_char_ram[m_80col_addr & 0x07ff] = m_80col_ascii;

			/* write enable attribute ram */
			if (BIT(m_80col_addr, 13))
				m_80col_attr_ram[m_80col_addr & 0x07ff] = m_80col_attr;
		}
		break;
	case 1:
		m_80col_addr = (data << 8) | (m_80col_addr & 0x00ff);
		break;
	case 2:
		m_80col_ascii = data;
		break;
	case 3:
		m_80col_attr = data;
		break;
	}
}

MC6845_UPDATE_ROW(mtx_sdxcpm_device::crtc_update_row)
{
	const pen_t *pen = m_palette->pens();

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_80col_char_ram[(ma + column) & 0x7ff];
		uint8_t attr = m_80col_attr_ram[(ma + column) & 0x7ff];
		offs_t addr = (code << 4) | (ra & 0x0f);
		uint8_t data = m_char_rom->base()[addr];

		if (column == cursor_x)
		{
			data = 0xff;
			attr = 0x07;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			int x = (column * 8) + bit;
			int fg = attr & 0x07;
			int bg = attr & 0x38;

			int color = BIT(data, 7) ? fg : bg;

			bitmap.pix32(y, x) = pen[de ? color : 0];

			data <<= 1;
		}
	}
}
