// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/*****************************************************************************

    Casio Loopy (c) 1995 Casio

    skeleton driver

    TODO:
    - Identify what actually is the NEC CDT-109 CPU, it should contain a program
      controller for the thermal printer device

    Note:
    - just a placeholder for any HW discovery, until we decap/trojan the BIOS,
      the idea is to understand the HW enough to extract the SH-1 internal BIOS
      data via a trojan;

    ASM notes:
    - first vector is almost certainly VBR value.
    - [VBR + 0x2c] irq for i/o?
    - [VBR + 0x140] points to an internal BIOS routine, at 0x6238
      (Nigaoe Artist has a direct 0x648c instead)
    - 0x0604 is probably a BRA -2 / NOP (some games puts that as a null irq vector)
    - Nigaoe Artist jumps to 0x668 at some point.


===============================================================================

Casio Loopy PCB Layout
----------------------

JCM631-MA1M C
|---------------------------------------------------------|
|    CB    CC              CD         CE       CF      CG |
|--|                                                      |
   |                                        BA10339F      |
|--| 15218  |--|     CXA1645M                           CH|
|           |  |                A1603C                    |
|    15218  |  |                                          |
|           |  |                                          |
|BIOS.LSI352|  |                                          |
|           |  |                      21MHz               |
| |--------||  |   |------|                 SW1           |
| |NEC     ||  |   |SH7021|      |----------|             |
| |CDT109  ||CA|   |      |      |          |             |
| |        ||  |   |------|      |CASIO     |             |
| |--------||  |                 |RH-7500   |             |
|           |  |                 |5C315     |          |--|
| |-------| |  |                 |          |          |
| |CASIO  | |  |                 |----------|          |--|
| |RH-7501| |  |  HM514260                                |
| |5C350  | |  |                               HM62256    |
| |-------| |  |                                          |
| 6379      |--|    SW301                      HM62256    /
|--------|                        HM538123               /
         |                                              /
         |                                             /
         |--------------------------------------------/

Notes:
      Connectors
      ----------
      CA - Cartridge connector
      CB - Power Input connector
      CC - Composite Video and Audio Out connector
      CD - Printer Cassette Motor connector
      CE - Printer Data connector
      CF - Printer Head connector
      CG - Paper Sensor connector
      CH - Joystick connector
      Connectors on the back of the main unit include RCA audio (left/right), RCA composite video,
      24V DC power input and contrast knob.
      On top of the main unit, there is a reset button, on/off slide switch, a big eject button, a
      button to cut off stickers after they're printed, a button to open the hatch where the sticker
      cassette is located and a red LED for power.

      IC's
      ----
      BIOS2.LSI352- Hitachi HN62434 512k x8 (4MBit) maskROM (SOP40)
      CDT-109     - NEC CDT109 (QFP120). This is some kind of CPU, the package looks a bit
                    like a V60. The BIOS is tied directly to it.
      RH-7500     - Casio RH-7500 5C315 (QFP208). This is the graphics generator chip.
      RH-7501     - Casio RH-7501 5C350 (QFP64). This is probably the sound chip.
      SH7021      - Hitachi HD6437021TE20 SuperH RISC Engine SH-1 CPU with 32k internal maskROM (TQFP100)
                    The internal ROM (BIOS1) is not dumped.
      CXA1645M    - Sony CXA1645M RGB Encoder (RGB -> Composite Video) (SOIC24)
      A1603C      - NEC uPA1603C Compound Field Effect Power Transistor Array (DIP16)
      HM514260    - Hitachi HM514260 256k x 16 DRAM (SOJ40)
      HM538123    - Hitachi HM538123 128k x8 multi-port Video DRAM with 256-word x8 serial access memory (SOJ40)
      HM62256     - Hitachi HM62256 32k x8 SRAM (SOP28)
      BA10339F    - Rohm BA10339F Quad Comparitor (SOIC14)
      6379        - NEC uPD6379 2-channel 16-bit D/A convertor for digital audio signal demodulation (SOIC8)
      15218       - Rohm BA15218 Dual High Slew Rate, Low Noise Operational Amplifier (SOIC8)

      Other
      -----
      SW1        - Reset Switch
      SW301      - ON/OFF Slide Switch


Carts
-----
There are reports of 11 existing carts.
Only 6 are available so far.

XK-401: Anime Land
XK-402: HARIHARI Seal Paradise
XK-403: Dream Change
XK-404: Nigaoe Artist
XK-501: Wanwan Aijou Monogatari
XK-502: PC Collection

The rest are not dumped yet.....


Lupiton's Wonder Palette
Magical Shop
Chakra-kun no Omajinai Paradise
XK-503: Little Romance
XK-504: I want a room in Loopy Town



Inside the carts
----------------

Carts 401 - 404:
PCB 'JCM632-AN1M C'
1x 16M maskROM (SOP44)
1x 8k x8 SRAM (SOP28)
1x 3V coin battery (CR2032)

Cart 501:
PCB 'Z544-1 A240427-1'
1x 16M maskROM (SOP44)
1x 8k x8 SRAM (SOP28)
1x OKI MSM6653A Voice Synthesis IC with 544Kbits internal maskROM (SOP24)
1x Rohm BA15218 High Slew Rate, Low Noise, Dual Operational Amplifier (SOIC8)
1x 74HC273 logic chip
1x 3V coin battery (CR2032)

Cart 502:
PCB 'Z545-1 A240570-1'
1x 16M maskROM (SOP44)
1x 32k x8 SRAM (SOP28)
1x 74HC00 logic chip
1x 3V coin battery (CR2032)

******************************************************************************/

#include "emu.h"
#include "cpu/sh/sh2.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

class casloopy_state : public driver_device
{
public:
	casloopy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bios_rom(*this, "bios_rom"),
		m_vregs(*this, "vregs"),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen") { }

	void casloopy(machine_config &config);

	void init_casloopy();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_shared_ptr<uint32_t> m_bios_rom;
	required_shared_ptr<uint32_t> m_vregs;
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	std::unique_ptr<uint16_t[]> m_paletteram;
	std::unique_ptr<uint8_t[]> m_vram;
	std::unique_ptr<uint8_t[]> m_bitmap_vram;
	uint16_t sh7021_regs[0x100];
	int m_gfx_index;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint16_t vregs_r(offs_t offset);
	void vregs_w(offs_t offset, uint16_t data);
	uint16_t pal_r(offs_t offset);
	void pal_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	uint16_t sh7021_r(offs_t offset);
	void sh7021_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t bitmap_r(offs_t offset);
	void bitmap_w(offs_t offset, uint8_t data);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void casloopy_map(address_map &map);
	void casloopy_sub_map(address_map &map);
};


static const gfx_layout casloopy_4bpp_layout =
{
	8,8,
	0x10000/32,
	4,
	{ STEP4(0, 1) },
	{ STEP8(0, 4) },
	{ STEP8(0, 4*8) },
	4*8*8
};

static const gfx_layout casloopy_8bpp_layout =
{
	8,8,
	0x10000/64,
	8,
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	{ STEP8(0, 8*8) },
	8*8*8
};

void casloopy_state::video_start()
{
	/* TODO: proper sizes */
	m_paletteram = make_unique_clear<uint16_t[]>(0x1000);
	m_vram = make_unique_clear<uint8_t[]>(0x10000);
	m_bitmap_vram = make_unique_clear<uint8_t[]>(0x20000);

	for (m_gfx_index = 0; m_gfx_index < MAX_GFX_ELEMENTS; m_gfx_index++)
		if (m_gfxdecode->gfx(m_gfx_index) == nullptr)
			break;

	for(int i=0;i<0x10000;i++)
		m_vram[i] = i & 0xff;

	m_gfxdecode->set_gfx(m_gfx_index, std::make_unique<gfx_element>(m_palette, casloopy_4bpp_layout, m_vram.get(), 0, 0x10, 0));
	m_gfxdecode->set_gfx(m_gfx_index+1, std::make_unique<gfx_element>(m_palette, casloopy_8bpp_layout, m_vram.get(), 0, 1, 0));
}

uint32_t casloopy_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(m_gfx_index);
	int count;

	static int test;

	if(machine().input().code_pressed(KEYCODE_Z))
		test+=0x100;

	if(machine().input().code_pressed(KEYCODE_X))
		test-=0x100;

	//popmessage("%08x",test);

	#if 0
	int r,g,b;

	r = pal5bit((m_vregs[0x4/4] >> 10) & 0x1f);
	g = pal5bit((m_vregs[0x4/4] >> 5) & 0x1f);
	b = pal5bit((m_vregs[0x4/4] >> 0) & 0x1f);
	m_palette->set_pen_color(0x100,rgb_t(r^0xff,g^0xff,b^0xff));
	bitmap.fill( 0x100 ,cliprect);
	#endif

	count = test;
	for (int y=0;y<32;y++)
	{
		for (int x=0;x<32;x++)
		{
			uint16_t tile = (m_vram[count+1])|(m_vram[count]<<8);

			tile &= 0x7ff; //???

			gfx->transpen(bitmap,cliprect,tile,7,0,0,x*8,y*8,0xffffffff);

			count+=2;
		}
	}

	count = test;

	for (int y=cliprect.top(); y<cliprect.bottom(); y++) // FIXME: off-by-one?
	{
		for(int x=0;x<256;x++)
		{
			uint8_t pix = m_bitmap_vram[count];
			if(pix)
				bitmap.pix(y, x) = pix + 0x100;

			count++;
		}
	}

	return 0;
}

uint16_t casloopy_state::vregs_r(offs_t offset)
{
	if(offset == 4/2)
	{
		return (m_screen->vblank() << 8) | (machine().rand() & 0xff); // vblank + vpos?
	}

	if(offset == 2/2)
		return machine().rand();/*(m_screen->hblank() << 8) | (m_screen->hpos() & 0xff);*/ // hblank + hpos?

	if(offset == 0/2)
		return machine().rand(); // pccllect

	printf("%08x\n",offset*2);

	return 0xffff;
}

void casloopy_state::vregs_w(offs_t offset, uint16_t data)
{
//  if(offset != 6/2)
//      printf("%08x %08x\n",offset*2,data);
}

uint16_t casloopy_state::pal_r(offs_t offset)
{
	return m_paletteram[offset];
}

void casloopy_state::pal_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int r,g,b;
	COMBINE_DATA(&m_paletteram[offset]);

	b = ((m_paletteram[offset])&0x001f)>>0;
	g = ((m_paletteram[offset])&0x03e0)>>5;
	r = ((m_paletteram[offset])&0x7c00)>>10;

	m_palette->set_pen_color(offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

uint8_t casloopy_state::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void casloopy_state::vram_w(offs_t offset, uint8_t data)
{
	m_vram[offset] = data;

	m_gfxdecode->gfx(m_gfx_index)->mark_dirty(offset/32);
	m_gfxdecode->gfx(m_gfx_index+1)->mark_dirty(offset/64);
}

/* TODO: all of this should be internal to the SH core, this is just to check what it enables. */
uint16_t casloopy_state::sh7021_r(offs_t offset)
{
	return sh7021_regs[offset];
}

void casloopy_state::sh7021_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&sh7021_regs[offset]);

	if(offset == 0x4e/2)
	{
		uint32_t src,dst,size,type;

		src = (sh7021_regs[0x40/2]<<16)|(sh7021_regs[0x42/2]&0xffff);
		dst = (sh7021_regs[0x44/2]<<16)|(sh7021_regs[0x46/2]&0xffff);
		size = (sh7021_regs[0x4a/2]&0xffff);
		type = (sh7021_regs[0x4e/2]&0xffff);

		printf("0 %08x %08x %04x %04x\n",src & 0x7ffffff,dst & 0x7ffffff,size,type);

		sh7021_regs[0x4e/2]&=0xfffe;
	}

	if(offset == 0x7e/2)
	{
		uint32_t src,dst,size,type;

		src = (sh7021_regs[0x70/2]<<16)|(sh7021_regs[0x72/2]&0xffff);
		dst = (sh7021_regs[0x74/2]<<16)|(sh7021_regs[0x76/2]&0xffff);
		size = (sh7021_regs[0x7a/2]&0xffff);
		type = (sh7021_regs[0x7e/2]&0xffff);

		printf("3 %08x %08x %04x %04x\n",src & 0x7ffffff,dst & 0x7ffffff,size,type);

		sh7021_regs[0x7e/2]&=0xfffe;
	}

//  printf("%08x %04x\n",sh7021_regs[offset],0x05ffff00+offset*2);
}

uint8_t casloopy_state::bitmap_r(offs_t offset)
{
	return m_bitmap_vram[offset];
}

void casloopy_state::bitmap_w(offs_t offset, uint8_t data)
{
	m_bitmap_vram[offset] = data;
}

void casloopy_state::casloopy_map(address_map &map)
{
	map(0x00000000, 0x00007fff).ram().share("bios_rom");
	map(0x01000000, 0x0107ffff).ram().share("wram");// stack pointer points here
	map(0x04000000, 0x0401ffff).rw(FUNC(casloopy_state::bitmap_r), FUNC(casloopy_state::bitmap_w));
	map(0x04040000, 0x0404ffff).rw(FUNC(casloopy_state::vram_r), FUNC(casloopy_state::vram_w)); // tilemap + PCG
	map(0x04050000, 0x040503ff).ram(); // ???
	map(0x04051000, 0x040511ff).rw(FUNC(casloopy_state::pal_r), FUNC(casloopy_state::pal_w));
	map(0x04058000, 0x04058007).rw(FUNC(casloopy_state::vregs_r), FUNC(casloopy_state::vregs_w));
	map(0x0405b000, 0x0405b00f).ram().share("vregs"); // RGB555 brightness control plus scrolling
//  map(0x05ffff00, 0x05ffffff).rw(FUNC(casloopy_state::sh7021_r), FUNC(casloopy_state::sh7021_w));
//  map(0x05ffff00, 0x05ffffff) - SH7021 internal i/o
	map(0x06000000, 0x062fffff).r(m_cart, FUNC(generic_cartslot_device::read32_rom));
	//map(0x07000000, 0x070003ff).ram();// area 7 (CS7), NOT on-chip RAM mirror
	map(0x09000000, 0x0907ffff).ram().share("wram");
	map(0x0e000000, 0x0e2fffff).r(m_cart, FUNC(generic_cartslot_device::read32_rom));
	map(0x0f000000, 0x0f0003ff).ram().share("oram");
}

#if 0
void casloopy_state::casloopy_sub_map(address_map &map)
{
	map(0xf80000, 0xffffff).rom().region("subcpu",0);
}
#endif

static INPUT_PORTS_START( casloopy )
INPUT_PORTS_END

void casloopy_state::machine_start()
{
}

void casloopy_state::machine_reset()
{
	//m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE); //halt the CPU until we find enough data to proceed

}

#if 0
static const gfx_layout casloopy_4bpp_layoutROM =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0, 1) },
	{ STEP8(0, 4) },
	{ STEP8(0, 4*8) },
	4*8*8
};


static const gfx_layout casloopy_8bpp_layoutROM =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	{ STEP8(0, 8*8) },
	8*8*8
};
#endif


DEVICE_IMAGE_LOAD_MEMBER( casloopy_state::cart_load )
{
	uint32_t size = m_cart->common_get_size("rom");
	uint8_t *SRC, *DST;
	std::vector<uint8_t> temp;
	temp.resize(size);

	m_cart->rom_alloc(size, GENERIC_ROM32_WIDTH, ENDIANNESS_LITTLE);

	SRC = &temp[0];
	DST = m_cart->get_rom_base();
	m_cart->common_load_rom(&temp[0], size, "rom");

	// fix endianness
	for (int i = 0; i < size; i += 4)
	{
		uint8_t tempa = SRC[i + 0];
		uint8_t tempb = SRC[i + 1];
		DST[i + 0] = SRC[i + 2];
		DST[i + 1] = SRC[i + 3];
		DST[i + 2] = tempa;
		DST[i + 3] = tempb;
	}

	return image_init_result::PASS;
}

void casloopy_state::casloopy(machine_config &config)
{
	/* basic machine hardware */
	SH2A(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &casloopy_state::casloopy_map);

//  v60_device &subcpu(V60(config, "subcpu", 8000000));
//  subcpu.set_addrmap(AS_PROGRAM, &casloopy_state::casloopy_sub_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(8000000, 444, 0, 256, 263, 0, 224);
//  m_screen->set_refresh_hz(60);
//  m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
//  m_screen->set_size(444, 263);
//  m_screen->set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	m_screen->set_screen_update(FUNC(casloopy_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(512);

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "loopy_cart", "bin,ic1"));
	cartslot.set_width(GENERIC_ROM32_WIDTH);
	cartslot.set_endian(ENDIANNESS_LITTLE);
	cartslot.set_must_be_loaded(true);
	cartslot.set_device_load(FUNC(casloopy_state::cart_load));

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("casloopy");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( casloopy )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bios1", 0x0000, 0x8000, NO_DUMP ) // SH7021 uses 32 KB

	ROM_REGION( 0x80000, "subcpu", 0) //NEC CDT-109
	ROM_LOAD( "bios2.lsi352", 0x0000, 0x80000, CRC(8f51fa17) SHA1(99f50be06b083fdb07e08f30b0b26d9037afc869) )
ROM_END

void casloopy_state::init_casloopy()
{
	/* load hand made bios data*/
	m_bios_rom[0/4] = 0x6000480;//0x600af3c;//0x6000964; //SPC
	m_bios_rom[4/4] = 0x0000000; //SSP

	for(int i = 0x400/4; i < 0x8000/4; i++)
		m_bios_rom[i] = 0x000b0009; // RTS + NOP
}

CONS( 1995, casloopy, 0, 0, casloopy, casloopy, casloopy_state, init_casloopy, "Casio", "Loopy", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_NODEVICE_PRINTER )
