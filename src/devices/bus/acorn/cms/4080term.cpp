// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    CMS 40/80 Video Terminal Card

    Part No. CMS 0010-3

**********************************************************************/


#include "emu.h"
#include "4080term.h"
#include "screen.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CMS_4080TERM, cms_4080term_device, "cms_4080term", "CMS 40/80 Video Terminal Card")


//-------------------------------------------------
//  gfx_layout cms_4080term_charlayout
//-------------------------------------------------

static const gfx_layout cms_4080term_charlayout =
{
	8, 10,                  /* 8 x 10 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0, 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8, 9 * 8 },
	8 * 16                  /* every char takes 16 bytes */
};

//-------------------------------------------------
//  GFXDECODE( gfx_cms_4080term )
//-------------------------------------------------

static GFXDECODE_START(gfx_cms_4080term)
	GFXDECODE_ENTRY("ef9345", 0x2000, cms_4080term_charlayout, 0, 4)
GFXDECODE_END

//-------------------------------------------------
//  MACHINE_DRIVER( cms_4080term )
//-------------------------------------------------

ROM_START(cms_4080term)
	ROM_REGION(0x4000, "ef9345", 0)
	ROM_LOAD("charset.rom", 0x0000, 0x2000, BAD_DUMP CRC(b2f49eb3) SHA1(d0ef530be33bfc296314e7152302d95fdf9520fc))            // from dcvg5k
ROM_END


static DEVICE_INPUT_DEFAULTS_START(keyboard)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_1200)
	DEVICE_INPUT_DEFAULTS("RS232_STARTBITS", 0xff, RS232_STARTBITS_1)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_2)
DEVICE_INPUT_DEFAULTS_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cms_4080term_device::device_add_mconfig(machine_config &config)
{
	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_size(768, 312);
	m_screen->set_visarea(0, 492 - 1, 0, 270 - 1);
	m_screen->set_screen_update("ef9345", FUNC(ef9345_device::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_cms_4080term);
	PALETTE(config, "palette").set_entries(8);

	EF9345(config, m_ef9345, 0);
	m_ef9345->set_screen("screen");
	m_ef9345->set_palette_tag("palette");

	TIMER(config, "scantimer").configure_scanline(FUNC(cms_4080term_device::update_scanline), "screen", 0, 10);

	VIA6522(config, m_via, 1_MHz_XTAL);
	m_via->writepa_handler().set("cent_data_out", FUNC(output_latch_device::bus_w));
	m_via->ca2_handler().set(m_centronics, FUNC(centronics_device::write_strobe));
	m_via->irq_handler().set(FUNC(cms_4080term_device::bus_irq_w));

	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_acia->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_acia->irq_handler().set(FUNC(cms_4080term_device::bus_irq_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, "keyboard");
	m_rs232->set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));
	m_rs232->rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	m_rs232->dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	m_rs232->dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	m_rs232->cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_via, FUNC(via6522_device::write_ca1));
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);
}


const tiny_rom_entry *cms_4080term_device::device_rom_region() const
{
	return ROM_NAME( cms_4080term );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cms_4080term_device - constructor
//-------------------------------------------------

cms_4080term_device::cms_4080term_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CMS_4080TERM, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_via(*this, "via")
	, m_acia(*this, "acia")
	, m_screen(*this, "screen")
	, m_ef9345(*this, "ef9345")
	, m_rs232(*this, "rs232")
	, m_centronics(*this, "centronics")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cms_4080term_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xfd20, 0xfd2f, read8sm_delegate(*m_ef9345, FUNC(ef9345_device::data_r)), write8sm_delegate(*m_ef9345, FUNC(ef9345_device::data_w)));
	space.install_device(0xfd30, 0xfd3f, *m_via, &via6522_device::map);
	space.install_readwrite_handler(0xfd40, 0xfd4f, read8sm_delegate(*m_acia, FUNC(mos6551_device::read)), write8sm_delegate(*m_acia, FUNC(mos6551_device::write)));

	uint8_t *FNT = memregion("ef9345")->base();
	uint16_t dest = 0x2000;

	/* Unscramble the chargen rom as the format is too complex for gfxdecode to handle unaided */
	for (uint16_t a = 0; a < 8192; a += 4096)
		for (uint16_t b = 0; b < 2048; b += 64)
			for (uint16_t c = 0; c < 4; c++)
				for (uint16_t d = 0; d < 64; d += 4)
					FNT[dest++] = FNT[a | b | c | d];
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_DEVICE_CALLBACK_MEMBER(cms_4080term_device::update_scanline)
{
	m_ef9345->update_scanline((uint16_t)param);
}

WRITE_LINE_MEMBER(cms_4080term_device::bus_irq_w)
{
	m_bus->irq_w(state);
}
