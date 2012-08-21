/***************************************************************************

  Radius Two Page Display (1280x960?)

  Fsx8000a = DAC data
  Fsx8000e = DAC control
  Fsx00000 = VRAM (offset 0x200, stride 0x1b0)

***************************************************************************/

#include "emu.h"
#include "video/nubus_radiustpd.h"

#define RADIUSTPD_SCREEN_NAME "tpd_screen"
#define RADIUSTPD_ROM_REGION  "tpd_rom"

#define VRAM_SIZE	(0x40000)	// 256k.  1152x880 1 bit per pixel fits nicely.

static SCREEN_UPDATE_RGB32( radiustpd );

MACHINE_CONFIG_FRAGMENT( radiustpd )
	MCFG_SCREEN_ADD( RADIUSTPD_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_STATIC(radiustpd)
	MCFG_SCREEN_SIZE(1280, 960)
	MCFG_SCREEN_REFRESH_RATE(70)
	MCFG_SCREEN_VISIBLE_AREA(0, 1152-1, 0, 880-1)
MACHINE_CONFIG_END

ROM_START( radiustpd )
	ROM_REGION(0x8000, RADIUSTPD_ROM_REGION, 0)
	ROM_LOAD( "tpd_v22.bin",  0x0000, 0x8000, CRC(7dc5ed05) SHA1(4abb64e49201e966c17a255a94b670564b229934) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NUBUS_RADIUSTPD = &device_creator<nubus_radiustpd_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nubus_radiustpd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( radiustpd );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *nubus_radiustpd_device::device_rom_region() const
{
	return ROM_NAME( radiustpd );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_radiustpd_device - constructor
//-------------------------------------------------

nubus_radiustpd_device::nubus_radiustpd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, NUBUS_RADIUSTPD, "Radius Two Page Display video card", tag, owner, clock),
		device_nubus_card_interface(mconfig, *this)
{
	m_shortname = "nb_rtpd";
}

nubus_radiustpd_device::nubus_radiustpd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, type, name, tag, owner, clock),
		device_nubus_card_interface(mconfig, *this)
{
	m_shortname = "nb_rtpd";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_radiustpd_device::device_start()
{
	UINT32 slotspace;

	// set_nubus_device makes m_slot valid
	set_nubus_device();
	install_declaration_rom(this, RADIUSTPD_ROM_REGION, true, true);

	slotspace = get_slotspace();

	printf("[radiustpd %p] slotspace = %x\n", this, slotspace);

	m_vram = auto_alloc_array(machine(), UINT8, VRAM_SIZE);
	m_vram32 = (UINT32 *)m_vram;

	m_nubus->install_device(slotspace, slotspace+VRAM_SIZE-1, read32_delegate(FUNC(nubus_radiustpd_device::vram_r), this), write32_delegate(FUNC(nubus_radiustpd_device::vram_w), this));
	m_nubus->install_device(slotspace+0x900000, slotspace+VRAM_SIZE-1+0x900000, read32_delegate(FUNC(nubus_radiustpd_device::vram_r), this), write32_delegate(FUNC(nubus_radiustpd_device::vram_w), this));
	m_nubus->install_device(slotspace+0x80000, slotspace+0xeffff, read32_delegate(FUNC(nubus_radiustpd_device::radiustpd_r), this), write32_delegate(FUNC(nubus_radiustpd_device::radiustpd_w), this));
	m_nubus->install_device(slotspace+0x980000, slotspace+0x9effff, read32_delegate(FUNC(nubus_radiustpd_device::radiustpd_r), this), write32_delegate(FUNC(nubus_radiustpd_device::radiustpd_w), this));

	m_timer = timer_alloc(0, NULL);
	m_screen = NULL;	// can we look this up now?
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_radiustpd_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	m_mode = 0;
	memset(m_vram, 0, VRAM_SIZE);
	memset(m_palette, 0, sizeof(m_palette));

	m_palette[1] = MAKE_RGB(255, 255, 255);
	m_palette[0] = MAKE_RGB(0, 0, 0);
}


void nubus_radiustpd_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(m_screen->time_until_pos(479, 0), 0);
}

/***************************************************************************

  Two Page Display section

***************************************************************************/

static SCREEN_UPDATE_RGB32( radiustpd )
{
	UINT32 *scanline;
	int x, y;
	nubus_radiustpd_device *card = downcast<nubus_radiustpd_device *>(screen.owner());
	UINT8 pixels, *vram;

	// first time?  kick off the VBL timer
	if (!card->m_screen)
	{
		card->m_screen = &screen;
		card->m_timer->adjust(card->m_screen->time_until_pos(479, 0), 0);
	}

	vram = card->m_vram + 0x200;

	for (y = 0; y < 880; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1152/8; x++)
		{
			pixels = vram[(y * (1152/8)) + (BYTE4_XOR_BE(x))];

			*scanline++ = card->m_palette[((pixels>>7)&0x1)];
			*scanline++ = card->m_palette[((pixels>>6)&0x1)];
			*scanline++ = card->m_palette[((pixels>>5)&0x1)];
			*scanline++ = card->m_palette[((pixels>>4)&0x1)];
			*scanline++ = card->m_palette[((pixels>>3)&0x1)];
			*scanline++ = card->m_palette[((pixels>>2)&0x1)];
			*scanline++ = card->m_palette[((pixels>>1)&0x1)];
			*scanline++ = card->m_palette[(pixels&1)];
		}
	}

	return 0;
}

WRITE32_MEMBER( nubus_radiustpd_device::radiustpd_w )
{
//  printf("TPD: write %08x to %x, mask %08x\n", data, offset, mem_mask);
}

READ32_MEMBER( nubus_radiustpd_device::radiustpd_r )
{
//  printf("TPD: read @ %x, mask %08x\n", offset, mem_mask);

	if (offset == 0)
	{
		lower_slot_irq();
		m_vbl_disable = true;
	}

	if (offset == 0x8000)
	{
		m_vbl_disable = false;
	}

	if (offset == 0x18000)
	{
		return 0xffffffff;
	}

	return 0;
}

WRITE32_MEMBER( nubus_radiustpd_device::vram_w )
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram32[offset]);
}

READ32_MEMBER( nubus_radiustpd_device::vram_r )
{
	return m_vram32[offset] ^ 0xffffffff;
}
