// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom Advanced Video Card

***************************************************************************/

#include "avc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NASCOM_AVC = &device_creator<nascom_avc_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( nascom_avc )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(16250000, 1024, 0, 768, 320, 0, 256)
	MCFG_SCREEN_UPDATE_DEVICE("mc6845", mc6845_device, screen_update)

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	MCFG_MC6845_ADD("mc6845", MC6845, "screen", XTAL_16MHz / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(6)
	MCFG_MC6845_UPDATE_ROW_CB(nascom_avc_device, crtc_update_row)
MACHINE_CONFIG_END

machine_config_constructor nascom_avc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( nascom_avc );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nascom_avc_device - constructor
//-------------------------------------------------

nascom_avc_device::nascom_avc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, NASCOM_AVC, "Nascom Advanced Video Card", tag, owner, clock, "nascom_avc", __FILE__),
	device_nasbus_card_interface(mconfig, *this),
	m_crtc(*this, "mc6845"),
	m_palette(*this, "palette"),
	m_control(0x80)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nascom_avc_device::device_start()
{
	// allocate memory
	m_r_ram.resize(0x4000);
	m_g_ram.resize(0x4000);
	m_b_ram.resize(0x4000);

	save_item(NAME(m_r_ram));
	save_item(NAME(m_g_ram));
	save_item(NAME(m_b_ram));
	save_item(NAME(m_control));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nascom_avc_device::device_reset()
{
	m_nasbus->m_io->install_write_handler(0xb0, 0xb0, write8_delegate(FUNC(mc6845_device::address_w), m_crtc.target()));
	m_nasbus->m_io->install_readwrite_handler(0xb1, 0xb1, read8_delegate(FUNC(mc6845_device::register_r), m_crtc.target()), write8_delegate(FUNC(mc6845_device::register_w), m_crtc.target()));
	m_nasbus->m_io->install_write_handler(0xb2, 0xb2, write8_delegate(FUNC(nascom_avc_device::control_w), this));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

MC6845_UPDATE_ROW( nascom_avc_device::crtc_update_row )
{
	offs_t base_addr = (ma << 1 | ra << 6) + 2; // y * 64 + 2

	for (int x = 0; x < x_count * 6; x++)
	{
		// addr of source byte
		offs_t addr = base_addr + (x / 16);

		// msb first
		int bl = 7 - ((x / 2) & 7);
		int bh = 7 - ((x / 1) & 7);

		int r, g, b;

		// double density)
		if (BIT(m_control, 3))
		{
			// red disabled, blue low density, red/green combined to green
			r = 0;
			b = BIT(m_b_ram[addr], bl);
			g = (x & 8) ? BIT(m_r_ram[addr], bh) : BIT(m_g_ram[addr], bh);
		}
		else
		{
			// rgb color
			r = BIT(m_r_ram[addr], bl);
			g = BIT(m_g_ram[addr], bl);
			b = BIT(m_b_ram[addr], bl);
		}

		// plot the pixel
		bitmap.pix32(y, x) = m_palette->pen_color((b << 2) | (g << 1) | (r << 0));
	}
}

WRITE8_MEMBER( nascom_avc_device::control_w )
{
	logerror("nascom_avc_device::control_w: 0x%02x\n", data);

	// page video ram in?
	if (((m_control & 0x07) == 0) && (data & 0x07))
	{
		m_nasbus->ram_disable_w(0);
		m_nasbus->m_program->install_readwrite_handler(0x8000, 0xbfff, read8_delegate(FUNC(nascom_avc_device::vram_r), this), write8_delegate(FUNC(nascom_avc_device::vram_w), this));
	}
	else if ((data & 0x07) == 0)
	{
		m_nasbus->m_program->unmap_readwrite(0x8000, 0xbfff);
		m_nasbus->ram_disable_w(1);
	}

	m_control = data;
}

READ8_MEMBER( nascom_avc_device::vram_r )
{
	// manual says only one plane can be read, i assume this is the order
	if (BIT(m_control, 0)) return m_r_ram[offset];
	if (BIT(m_control, 1)) return m_g_ram[offset];
	if (BIT(m_control, 2)) return m_b_ram[offset];

	// should never happen
	return 0xff;
}

WRITE8_MEMBER( nascom_avc_device::vram_w )
{
	// all planes can be written at the same time
	if (BIT(m_control, 0)) m_r_ram[offset] = data;
	if (BIT(m_control, 1)) m_g_ram[offset] = data;
	if (BIT(m_control, 2)) m_b_ram[offset] = data;
}
