/**********************************************************************

    Amstrad PC1640 Integrated Graphics Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

    This display controller is integrated on the PC1640 motherboard 
    but wired to the ISA bus, and can be disabled with a DIP switch.

    WD Paradise PEGA 1A 38304B 2116-002 8745AAA JAPAN (84 pin PLCC)

    Single chip multimode EGA video controller with 
	integral 6845 CRTC. Provides 100% IBM EGA, CGA, 
	MDA, Hercules graphics and Plantronics COLORPLUS* 
	compatibility

**********************************************************************/

#include "isa_pc1640_iga.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define SCREEN_TAG      "screen"
#define PEGA1A_TAG    	"ic910"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ISA8_PC1640_IGA = &device_creator<isa8_pc1640_iga_device>;


//-------------------------------------------------
//  ROM( pc1640_iga )
//-------------------------------------------------

ROM_START( pc1640_iga )
	ROM_REGION16_LE( 0x8000, "iga", 0)
	ROM_LOAD( "40100.ic913", 0x0000, 0x8000, CRC(d2d1f1ae) SHA1(98302006ee38a17c09bd75504cc18c0649174e33) ) // 8736 E
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_pc1640_iga_device::device_rom_region() const
{
	return ROM_NAME( pc1640_iga );
}


//-------------------------------------------------
//  mc6845_interface crtc_intf
//-------------------------------------------------

static MC6845_UPDATE_ROW( pc1640_update_row )
{
}

static MC6845_INTERFACE( crtc_intf )
{
	SCREEN_TAG,
	false,
	8,
	NULL,
	pc1640_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};


//-------------------------------------------------
//  SCREEN_UPDATE( pc1640_iga )
//-------------------------------------------------

UINT32 isa8_pc1640_iga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


//-------------------------------------------------
//  MACHINE_DRIVER( pc1640_iga )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( pc1640_iga )
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, isa8_pc1640_iga_device, screen_update)
	MCFG_SCREEN_SIZE(80*8, 25*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 25*8-1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_REFRESH_RATE(60)

	MCFG_PALETTE_LENGTH(64)

	MCFG_MC6845_ADD(PEGA1A_TAG, AMS40041, XTAL_28_63636MHz/32, crtc_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_pc1640_iga_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pc1640_iga );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_pc1640_iga_device - constructor
//-------------------------------------------------

isa8_pc1640_iga_device::isa8_pc1640_iga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ISA8_PC1640_IGA, "Amstrad PC1640 IGA", tag, owner, clock, "pc1640_iga", __FILE__),
		device_isa8_card_interface(mconfig, *this),
		m_vdu(*this, PEGA1A_TAG),
		m_video_ram(*this, "video_ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_pc1640_iga_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "iga", "iga");
	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, READ8_DELEGATE(isa8_pc1640_iga_device, video_ram_r), WRITE8_DELEGATE(isa8_pc1640_iga_device, video_ram_w));
	m_isa->install_device(0x3b0, 0x3df, 0, 0, READ8_DELEGATE(isa8_pc1640_iga_device, iga_r), WRITE8_DELEGATE(isa8_pc1640_iga_device, iga_w));

	// allocate memory
	m_video_ram.allocate(0x20000);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_pc1640_iga_device::device_reset()
{
}


//-------------------------------------------------
//  video_ram_r -
//-------------------------------------------------

READ8_MEMBER( isa8_pc1640_iga_device::video_ram_r )
{
	UINT8 data = 0;

	if (BIT(m_egc_ctrl, 1))
	{
		data = m_video_ram[offset];
	}

	return data;
}


//-------------------------------------------------
//  video_ram_w -
//-------------------------------------------------

WRITE8_MEMBER( isa8_pc1640_iga_device::video_ram_w )
{
	if (BIT(m_egc_ctrl, 1))
	{
		m_video_ram[offset] = data;
	}
}


//-------------------------------------------------
//  iga_r -
//-------------------------------------------------

READ8_MEMBER( isa8_pc1640_iga_device::iga_r )
{
	UINT8 data = 0;

	//logerror("IGA read %03x\n", offset+0x3b0);

	switch (offset)
	{
	case 0x01:
		data = m_vdu->register_r(space, 0);
		break;

	case 0x05: // Mono CRT Controller Data
		if (!BIT(m_egc_ctrl, 0))
		{
			data = m_vdu->register_r(space, 0);
		}
		break;

	case 0x08: // Mono Extended Mode Control Protection Register
		if (!BIT(m_egc_ctrl, 0))
		{
			m_emcrp++;
		}
		break;

	case 0x0a: // Mono Status Register
		/*

		    bit     description

		    0       Display Enable
		    1       Light Pen Strobe
		    2       Light Pen Switch (-LPSW)
		    3       Mono Video
		    4       Color Diagnostic (MUX)
		    5       Color Diagnostic (MUX)
		    6       EGA Mode
		    7       -VSYNC

		*/

		if (!BIT(m_egc_ctrl, 0))
		{
			data |= m_vdu->de_r();
			data |= m_lpen << 1;
			data |= 0x04;
			data |= !m_vdu->vsync_r() << 7;
		}
		break;

	case 0x12: // EGC Status Register
		/*

		    bit     description

		    0
		    1
		    2
		    3
		    4       Switch Sense
		    5
		    6
		    7       VSYNC Interrupt Active

		*/

		// switch sense
		//data |= BIT(m_sw->read(), ((m_egc_ctrl >> 2) & 0x03) ^ 0x03);
		break;

	case 0x15: // Sequencer Data Register
		break;

	case 0x25: // Color CRT Controller Data
		if (BIT(m_egc_ctrl, 0))
		{
			data = m_vdu->register_r(space, 0);
		}
		break;

	case 0x28: // Color Extended Mode Control Protection Register
		if (BIT(m_egc_ctrl, 0))
		{
			m_emcrp++;
		}
		break;

	case 0x2a: // Color Status Register
		/*

		    bit     description

		    0       Display Enable
		    1       Light Pen Strobe
		    2       Light Pen Switch (-LPSW)
		    3       -VSYNC
		    4       Color Diagnostic (MUX)
		    5       Color Diagnostic (MUX)
		    6       EGA Mode
		    7       1

		*/

		if (BIT(m_egc_ctrl, 0))
		{
			data |= m_vdu->de_r();
			data |= m_lpen << 1;
			data |= 0x04;
			data |= !m_vdu->vsync_r() << 3;
			data |= 0x80;
		}
		break;
	}

	return data;
}


//-------------------------------------------------
//  iga_w -
//-------------------------------------------------

WRITE8_MEMBER( isa8_pc1640_iga_device::iga_w )
{
	//logerror("IGA write %03x:%02x\n", offset+0x3b0, data);

	switch (offset)
	{
	case 0x00:
		m_vdu->address_w(space, 0, data);
		break;

	case 0x01:
		m_vdu->register_w(space, 0, data);
		break;

	case 0x04: // Mono CRT Controller Address
		if (!BIT(m_egc_ctrl, 0))
		{
			m_vdu->address_w(space, 0, data);
		}
		break;

	case 0x05: // Mono CRT Controller Data
		if (!BIT(m_egc_ctrl, 0))
		{
		}
		break;

	case 0x08: // HMGA Mode Control Register
		break;

	case 0x0b: // Mono Extended Mode Control Register
		/*

		    bit     description

		    0       Enable Color Simulation Modes
		    1       Enable 132 Character Mode
		    2       Disable Blanking
		    3       Enable Alternate Character Sets on plane 3
		    4       Lock CRTC Timing Registers
		    5       Disable Palette and Overscan Registers
		    6       Enable Special Modes
		    7       Vsync Polarity, Border Blanking

		*/

		if (!BIT(m_egc_ctrl, 0) && (m_emcrp > 1))
		{
			m_emcrp = 0;
			m_emcr = data;
		}
		break;

	case 0x0f: // Hercules Mode Register
		break;

	case 0x10: // EGA Mode Control Register
		break;

	case 0x12: // EGC Control Register
		/*

		    bit     description

		    0       CRTC 3BX/3DX I/O Address Select
		    1       Display RAM Enable
		    2       Clock Rate Select / Switch Sense Select bit 0
		    3       Clock Rate Select / Switch Sense Select bit 1
		    4       External Video Enable
		    5       Alternate (64K) Text page Select
		    6       HSYNC Polarity
		    7       VSYNC Polarity

		*/

		m_egc_ctrl = data;
		break;

	case 0x14: // Sequencer Address Register
		m_sar = data;
		break;

	case 0x15: // Sequencer Data Register
		m_sdr[m_sar & 0x07] = data;
		break;

	case 0x1e: // Graphics Controller Address
		m_gcar = data;
		break;

	case 0x1f: // Graphics Controller Data
		m_gcdr[m_gcar & 0x0f] = data;
		break;

	case 0x24: // Color CRT Controller Address
		if (BIT(m_egc_ctrl, 0))
		{
			m_vdu->address_w(space, 0, data);
		}
		break;

	case 0x25: // Color CRT Controller Data
		if (BIT(m_egc_ctrl, 0))
		{
			m_vdu->register_w(space, 0, data);
		}
		break;

	case 0x28: // CGA Mode Control Register
		break;

	case 0x2b: // Color Extended Mode Control Register
		/*

		    bit     description

		    0       Enable Color Simulation Modes
		    1       Enable 132 Character Mode
		    2       Disable Blanking
		    3       Enable Alternate Character Sets on plane 3
		    4       Lock CRTC Timing Registers
		    5       Disable Palette and Overscan Registers
		    6       Enable Special Modes
		    7       Vsync Polarity, Border Blanking

		*/

		if (BIT(m_egc_ctrl, 0) && (m_emcrp > 1))
		{
			m_emcrp = 0;
			m_emcr = data;
		}
		break;

	case 0x2d: // Plantronics Mode Register
		/*

		    bit     description

		    0
		    1
		    2
		    3
		    4       Enable Extended color palette 2
		    5       Enable Extended color palette 1
		    6       Color Plane 0/1 Position
		    7

		*/

		m_plr = data;
		break;
	}
}
