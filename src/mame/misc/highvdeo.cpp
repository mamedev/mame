// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni, David Haywood, Angelo Salese, Roberto Fresca
/*************************************************************************************************

High Video Tour 4000

Driver by Mirko Buffoni.
Original brasil.c by David Haywood & Angelo Salese.
Additional work by Roberto Fresca.


Memory layout:

000000-0003FF   Interrupt table
000400-003FFF   NVRAM and main ram
040000-04FFFF   VGA 320x200 address space
080000-0BFFFF   Banked ROM
0C0000-0FFFFF   Program ROM


Port layout:

0x0008 R    Input port -> Keyboard
0x000A R    Input port -> Coin and Service
0x000C R    Input port -> Reset

0x0030 R    Read continuously... maybe watchdog?

0x0000 W    Keyboard Lights control port
0x0002 W    \ Hopper or ticket related
0x0004 W    /
0x0006 W    OKI6395 (6376?)ADPCM command:  need to be latched
0x0010 W    Like 0x3c8 in VGA
0x0014 W    Like 0x3c9 in VGA

0x0030 W    Bankswitch select

----

INT 2 (NMI) called every Vblank

----

Interesting locations.  255 = YES

3E23-24     Valore Moneta   (5)  (1,5,10)
3E25-26     Valore Gettone  (5)  (1-20)
3E27-28     Valore Servizio (10) (5-500)
3E29-2A     Banconote 1     (5)  (5-500)

3E33        Replay          (255) (0,255)
3E34        Double          (0)   (0,255)
3E35        BloccaBanconote (255) (0,255)
3E36        Accumulo        (0)   (0,255)
3E37        Vincita 10      (255) (0,255)
3E38        Numeroni        (255) (0,255)
3E39        Palline         (255) (0,255)
3E3B        Lattine         (255) (0,255)
3E3D        Premio          (10)  (X,10)
3E3E        Bet Max Credit  (20)  (1-50)
3E3F        Bet Max Points  (20)  (1-50)

3E40-41     Blocco Getton.  (100) (10-1000)
3E42        Cambio Carte    (0)   (Veloce=0, Normale=1, Lento=2)
3E45-46     Valore ticket   (100) (1-500)
3E4B        Bet Min Gioco   (1)   (1-10)
3E4C        Bet Min Fever   (1)   (1-10)

3E59        Tickets         (10)  (Tutti=0, 10=F, 1=FF)

----

Initial High Video releases have roms named 'vcf'...
They have low resolution 320x200x256 colors.
Game is V30 based, with rom banking

Next, they released new board with roms named 'ncf'...
Same resolution, but different mapping for memory and input ports, plus a check for vblank (protection?)
Game is V30 based, without banking

Newer boards instead have roms named 'tcf'...
Resolution is higher as 400x300x256 colors, and graphic is fancier.
There is a simple protection check, tied on an input port.
Game is V30 based, with rom banking (2Mb)

*************************************************************************************************

  Game notes....

  * New York Joker:

  The game needs default NVRAM, otherwise the game parameters will be totally wrong and the game
  can't work properly. To switch between pins/cards, after insert some credits (before bet on the
  game), press HOLD3 to get the graphics option, and use HOLD2 and HOLD4 to choose the wished set.


*************************************************************************************************/

#include "emu.h"

#include "cpu/i86/i186.h"
#include "cpu/nec/nec.h"
#include "machine/nvram.h"
#include "sound/okim6376.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "fashion.lh"


namespace {

class highvdeo_state : public driver_device
{
public:
	highvdeo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_blit_ram(*this, "blit_ram")
		, m_mainbank(*this, "mainbank")
		, m_lamps(*this, "lamp%u", 1U)
		, m_maincpu(*this, "maincpu")
		, m_okim6376(*this, "oki")
		, m_palette(*this, "palette")
		, m_inputs(*this, "IN%u", 0U)
	{ }

	void grancapi(machine_config &config);
	void tv_ncf(machine_config &config);
	void ciclone(machine_config &config);
	void nyjoker(machine_config &config);
	void magicbom(machine_config &config);
	void brasil(machine_config &config);
	void fashion(machine_config &config);
	void newmcard(machine_config &config);
	void record(machine_config &config);
	void tv_tcf(machine_config &config);
	void tv_vcf(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_shared_ptr<uint16_t> m_blit_ram;
	optional_memory_bank m_mainbank;

	output_finder<6> m_lamps;
	uint16_t m_vblank_bit;
	uint16_t m_brasil_prot_latch;
	uint16_t m_grancapi_prot_latch;
	uint16_t read0_r();
	uint16_t read1_r();
	uint16_t read2_r();
	uint8_t read2_nmi_clear_r();
	template<int Mask> void bankselect_w(uint8_t data);
	void write1_w(uint16_t data);
	uint16_t tv_ncf_read1_r();
	uint16_t newmcard_status_r(offs_t offset);
	uint16_t newmcard_vblank_r();
	void newmcard_vblank_w(uint16_t data);
	void write2_w(uint16_t data);
	void nyj_write2_w(uint16_t data);
	uint16_t brasil_status_r(offs_t offset);
	void brasil_status_w(uint16_t data);
	uint16_t ciclone_status_r(offs_t offset);
	uint16_t grancapi_status_r(offs_t offset);
	void grancapi_status_w(uint16_t data);
	uint16_t magicbom_status_r(offs_t offset);
	uint16_t record_status_r(offs_t offset);
	void fashion_output_w(uint16_t data);
	void tv_oki6376_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t tv_oki6376_r();
	void tv_ncf_oki6376_st_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t nmi_clear_r();
	void nmi_clear_w(uint8_t data);
	uint32_t screen_update_tourvisn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_brasil(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<okim6376_device> m_okim6376;
	required_device<palette_device> m_palette;
	required_ioport_array<3> m_inputs;

	void brasil_io(address_map &map);
	void brasil_map(address_map &map);
	void ciclone_io(address_map &map);
	void fashion_io(address_map &map);
	void grancapi_io(address_map &map);
	void magicbom_io(address_map &map);
	void newmcard_io_base(address_map &map);
	void newmcard_io(address_map &map);
	void newmcard_map(address_map &map);
	void nyjoker_io(address_map &map);
	void nyjoker_map(address_map &map);
	void ramdac_map(address_map &map);
	void record_io(address_map &map);
	void tv_ncf_io(address_map &map);
	void tv_ncf_map(address_map &map);
	void tv_tcf_io(address_map &map);
	void tv_tcf_map(address_map &map);
	void tv_vcf_io(address_map &map);
	void tv_vcf_map(address_map &map);
};

void highvdeo_state::machine_start()
{
	m_lamps.resolve();

	if (m_mainbank.found()) /*bankaddress might be incorrect at brasil/fasion/grancapi/magicbom*/
	{
		uint32_t max = memregion("maincpu")->bytes() / 0x40000;
		m_mainbank->configure_entries(0, max, memregion("maincpu")->base(), 0x40000);
	}
}


uint32_t highvdeo_state::screen_update_tourvisn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for(int y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		int count = ((y * (screen.visible_area().max_x+1)) + cliprect.min_x) >> 1;
		for(int x=(cliprect.min_x>>1);x<=(cliprect.max_x>>1);x++)
		{
			uint32_t color;

			color = ((m_blit_ram[count]) & 0x00ff)>>0;

			if(cliprect.contains((x*2)+0, y))
				bitmap.pix(y, (x*2)+0) = m_palette->pen(color);

			color = ((m_blit_ram[count]) & 0xff00)>>8;

			if(cliprect.contains((x*2)+1, y))
				bitmap.pix(y, (x*2)+1) = m_palette->pen(color);

			count++;
		}
	}

	return 0;
}

/*Later HW, RGB565 instead of RAM-based pens (+ ramdac).*/
uint32_t highvdeo_state::screen_update_brasil(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const rgb = m_palette->pens(); // 16 bit RGB

	for(int y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		int count = (y * 400) + cliprect.min_x;
		for(int x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			bitmap.pix(y, x) = rgb[m_blit_ram[count++]];
		}
	}

	return 0;
}



uint16_t highvdeo_state::read0_r()
{
	return m_inputs[0]->read();
}

uint16_t highvdeo_state::read1_r()
{
	return m_inputs[1]->read();
}

uint16_t highvdeo_state::read2_r()
{
	return m_inputs[2]->read();
}

uint8_t highvdeo_state::read2_nmi_clear_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return m_inputs[2]->read();
}

template<int Mask>
void highvdeo_state::bankselect_w(uint8_t data)
{
	m_mainbank->set_entry(data & Mask);
}


void highvdeo_state::tv_oki6376_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	static int okidata;
	if (ACCESSING_BITS_0_7 && okidata != data)
	{
		okidata = data;
		m_okim6376->write(data & ~0x80);
		m_okim6376->st_w(data & 0x80);
	}
}

uint8_t highvdeo_state::tv_oki6376_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	return m_okim6376->busy_r();
}

uint8_t highvdeo_state::nmi_clear_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	return 0xff;
}

void highvdeo_state::nmi_clear_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void highvdeo_state::write1_w(uint16_t data)
{
/*
    - Lbits -
    7654 3210
    =========
    ---- ---x  Hold1 lamp.
    ---- --x-  Hold2 lamp.
    ---- -x--  Hold3 lamp.
    ---- x---  Hold4 lamp.
    ---x ----  Hold5 lamp.
    --x- ----  Start lamp.
*/
	for (int i = 0; i < 6; i++)
	{
		m_lamps[i] = BIT(data, i);
	}

//  popmessage("%04x %04x",t1,t3);
}

void highvdeo_state::tv_vcf_map(address_map &map)
{
	map(0x00000, 0x003ff).ram(); /*irq vector area*/
	map(0x00400, 0x03fff).ram().share("nvram");
	map(0x40000, 0x4ffff).ram().share("blit_ram"); /*blitter ram*/
	map(0x80000, 0xbffff).bankr("mainbank");
	map(0xc0000, 0xfffff).rom().region("maincpu", 0xc0000);
}

void highvdeo_state::tv_vcf_io(address_map &map)
{
	map(0x0000, 0x0001).w(FUNC(highvdeo_state::write1_w)); // lamps
	map(0x0006, 0x0007).w(FUNC(highvdeo_state::tv_oki6376_w));
	map(0x0008, 0x0009).r(FUNC(highvdeo_state::read0_r));
	map(0x000a, 0x000b).r(FUNC(highvdeo_state::read1_r));
	map(0x000c, 0x000d).r(FUNC(highvdeo_state::read2_r));
	map(0x0010, 0x0010).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0012, 0x0012).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x0014, 0x0014).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x0030, 0x0030).w(FUNC(highvdeo_state::bankselect_w<0x03>)).r(FUNC(highvdeo_state::tv_oki6376_r));
}


uint16_t highvdeo_state::tv_ncf_read1_r()
{
	static int resetpulse = 0;

	// Bit 6 of port 1 is connected to clock impulse, as heartbeat.  If impulse cease
	// machine resets itself.
	resetpulse ^= 0x40;

	return (m_inputs[1]->read() & 0xbf) | resetpulse;
}

void highvdeo_state::tv_ncf_oki6376_st_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_okim6376->st_w( (data & 0x80) );
	}
}

void highvdeo_state::tv_ncf_map(address_map &map)
{
	map(0x00000, 0x003ff).ram(); /*irq vector area*/
	map(0x00400, 0x03fff).ram().share("nvram");
	map(0x20000, 0x2ffff).ram().share("blit_ram"); /*blitter ram*/
	map(0x40000, 0xfffff).rom().region("maincpu", 0x40000);
}

void highvdeo_state::tv_ncf_io(address_map &map)
{
	map(0x0000, 0x0001).w(FUNC(highvdeo_state::write1_w)); // lamps
	map(0x0008, 0x0008).w(m_okim6376, FUNC(okim6376_device::write));
	map(0x000a, 0x000b).w(FUNC(highvdeo_state::tv_ncf_oki6376_st_w));
	map(0x000c, 0x000d).r(FUNC(highvdeo_state::read0_r));
	map(0x0010, 0x0011).r(FUNC(highvdeo_state::tv_ncf_read1_r));
	map(0x0012, 0x0013).r(FUNC(highvdeo_state::read2_r));
	map(0x0020, 0x0020).w(FUNC(highvdeo_state::nmi_clear_w));
	map(0x0030, 0x0030).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0032, 0x0032).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x0034, 0x0034).w("ramdac", FUNC(ramdac_device::pal_w));
}


void highvdeo_state::nyjoker_map(address_map &map)
{
	map(0x00000, 0x003ff).ram(); /*irq vector area*/
	map(0x00400, 0x03fff).ram().share("nvram");
	map(0x10000, 0x1ffff).ram().share("blit_ram"); /*blitter ram*/
	map(0x40000, 0xfffff).rom().region("maincpu", 0x40000);
}

void highvdeo_state::nyjoker_io(address_map &map)
{
	map(0x0000, 0x0001).w(FUNC(highvdeo_state::write1_w));    // lamps
	map(0x0002, 0x0003).nopw();            // alternate coin counter (bits 0 and 2)
	map(0x0004, 0x0005).w(FUNC(highvdeo_state::nyj_write2_w)); // coin and note counter
//  map(0x0006, 0x0007).nopw();
	map(0x0008, 0x0008).w(m_okim6376, FUNC(okim6376_device::write));
	map(0x000a, 0x000b).w(FUNC(highvdeo_state::tv_ncf_oki6376_st_w));
	map(0x000c, 0x000d).portr("IN0");
	map(0x000e, 0x000f).portr("DSW");
	map(0x0010, 0x0011).portr("IN2");
	map(0x0012, 0x0013).portr("IN3");
	map(0x0014, 0x0015).r(FUNC(highvdeo_state::tv_ncf_read1_r));
	map(0x0020, 0x0020).w(FUNC(highvdeo_state::nmi_clear_w));
	map(0x0030, 0x0030).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0032, 0x0032).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x0034, 0x0034).w("ramdac", FUNC(ramdac_device::pal_w));
}


void highvdeo_state::nyj_write2_w(uint16_t data)
{
/*
    7654 3210
    =========
    ---- xxxx  Coin counter (all mixed).
    ---x ----  Note counter.
    xxx- ----  Unknown.
*/
//  popmessage("%04x",data);
	machine().bookkeeping().coin_counter_w(0, ~data & 0x0f); // Coins (all)
	machine().bookkeeping().coin_counter_w(1, ~data & 0x10); // Notes (all)
}

void highvdeo_state::tv_tcf_map(address_map &map)
{
	map(0x00000, 0x003ff).ram(); /*irq vector area*/
	map(0x00400, 0x03fff).ram().share("nvram");
	map(0x40000, 0x5d4bf).ram().share("blit_ram"); /*blitter ram*/
	map(0x7fe00, 0x7ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x80000, 0xbffff).bankr("mainbank");
	map(0xc0000, 0xfffff).rom().region("maincpu", 0x1c0000);
}

void highvdeo_state::tv_tcf_io(address_map &map)
{
	map(0x0000, 0x0001).w(FUNC(highvdeo_state::write1_w)); // lamps
	map(0x0006, 0x0007).w(FUNC(highvdeo_state::tv_oki6376_w));
	map(0x0008, 0x0009).r(FUNC(highvdeo_state::read0_r));
	map(0x000a, 0x000b).r(FUNC(highvdeo_state::read1_r));
	map(0x000e, 0x000e).rw(FUNC(highvdeo_state::nmi_clear_r), FUNC(highvdeo_state::nmi_clear_w));
	map(0x0030, 0x0031).r(FUNC(highvdeo_state::read2_r));
	map(0x0030, 0x0030).w(FUNC(highvdeo_state::bankselect_w<0x07>));
}

/****************************
*
* Ciclone
*
****************************/

uint16_t highvdeo_state::ciclone_status_r(offs_t offset)
{
	static uint16_t resetpulse;
	switch(offset*2)
	{
		case 0:
		resetpulse^=0x10;
		return 0 | resetpulse;
		case 2: return 0x15; //and 0x3f
	}

	return 0;
}

void highvdeo_state::ciclone_io(address_map &map)
{
	tv_tcf_io(map);
	map(0x0030, 0x0033).r(FUNC(highvdeo_state::ciclone_status_r));
}

/****************************
*
* New Magic Card
*
****************************/

uint16_t highvdeo_state::newmcard_status_r(offs_t offset)
{
	switch(offset*2)
	{
	case 0:
		if (!machine().side_effects_disabled())
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		return 2; //and $7
	case 2:
		return 2; //and $7
	}
	return 0;
}

uint16_t highvdeo_state::record_status_r(offs_t offset)
{
	static uint16_t resetpulse;
	switch(offset*2)
	{
	case 0:
		if (!machine().side_effects_disabled())
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		resetpulse^=0x15;       // and 0x07, cmp with 0x05
		return 0 | resetpulse;
	case 2:
		return 0x15;    // unknown
	}

	return 0;
}


uint16_t highvdeo_state::newmcard_vblank_r()
{
	return m_vblank_bit; //0x80
}

void highvdeo_state::newmcard_vblank_w(uint16_t data)
{
	m_vblank_bit = data;
}

void highvdeo_state::write2_w(uint16_t data)
{
	int i;

//  popmessage("%04x",data);

	for(i=0;i<4;i++)
	{
		machine().bookkeeping().coin_counter_w(i,data & 0x20);
		machine().bookkeeping().coin_lockout_w(i,~data & 0x08);
	}
}

void highvdeo_state::newmcard_map(address_map &map)
{
	map(0x00000, 0x003ff).ram(); /*irq vector area*/
	map(0x00400, 0x0ffff).ram().share("nvram");
	map(0x40000, 0x7ffff).ram().share("blit_ram"); /*blitter ram*/
	map(0x80000, 0xbffff).bankr("mainbank");
	map(0xc0000, 0xfffff).rom().region("maincpu", 0xc0000);
}

void highvdeo_state::newmcard_io_base(address_map &map)
{
	map(0x0000, 0x0001).w(FUNC(highvdeo_state::write1_w)); // lamps
	map(0x0002, 0x0003).w(FUNC(highvdeo_state::write2_w)); // coin counter & coin lockout
	map(0x0004, 0x0005).w(FUNC(highvdeo_state::newmcard_vblank_w));
	map(0x0006, 0x0007).w(FUNC(highvdeo_state::tv_oki6376_w));
	map(0x0008, 0x0009).r(FUNC(highvdeo_state::read0_r));
	map(0x000a, 0x000b).r(FUNC(highvdeo_state::read1_r));
	map(0x000c, 0x000d).r(FUNC(highvdeo_state::newmcard_vblank_r));
	map(0x000e, 0x000f).r(FUNC(highvdeo_state::read2_r));
	map(0x0010, 0x0010).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0012, 0x0012).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x0014, 0x0014).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x0030, 0x0030).w(FUNC(highvdeo_state::bankselect_w<0x07>));
}

void highvdeo_state::newmcard_io(address_map &map)
{
	newmcard_io_base(map);
	map(0x0030, 0x0033).r(FUNC(highvdeo_state::newmcard_status_r));
}

void highvdeo_state::record_io(address_map &map)
{
	newmcard_io_base(map);
	map(0x0030, 0x0033).r(FUNC(highvdeo_state::record_status_r));
}

/****************************
*
* Bra$il
*
****************************/


uint16_t highvdeo_state::brasil_status_r(offs_t offset)
{
	static uint16_t resetpulse;

	switch(offset*2)
	{
		case 0:
		resetpulse^=0x10;

		return 3 | resetpulse;
		case 2: return (m_brasil_prot_latch & 3); //and 0x3f
	}

	return 0;
}

/*bankaddress might be incorrect.*/
void highvdeo_state::brasil_status_w(uint16_t data)
{
	switch(data & 3) //data & 7?
	{
		case 0: m_brasil_prot_latch = 1; break;
		case 1: m_brasil_prot_latch = 0; break;
		case 2: m_brasil_prot_latch = 2; break;
	}

	m_mainbank->set_entry(data & 0x07);

//  popmessage("%04x",data);
}

uint16_t highvdeo_state::grancapi_status_r(offs_t offset)
{
	static uint16_t resetpulse;

	switch(offset*2)
	{
		case 0:
		resetpulse^=0x20;

		return 3 | resetpulse;
		case 2: return (m_grancapi_prot_latch & 3)|0x17; //and 0x3f  // This will always return 0x17: Coverity 316053
	}

	return 0;
}

/*bankaddress might be incorrect.*/
void highvdeo_state::grancapi_status_w(uint16_t data)
{
	switch(data & 3) //data & 7?
	{
		case 0: m_grancapi_prot_latch = 1; break;
		case 1: m_grancapi_prot_latch = 0; break;
		case 2: m_grancapi_prot_latch = 2; break;
	}

	m_mainbank->set_entry(data & 0x07);

//  popmessage("%04x",data);
}

uint16_t highvdeo_state::magicbom_status_r(offs_t offset)
{
	static uint16_t resetpulse;

	switch(offset*2)
	{
		case 0:
		resetpulse^=0x20;

		return  resetpulse;
		case 2: return (m_grancapi_prot_latch & 3)|0x0b; //and 0x3f
	}

	return 0;
}


void highvdeo_state::brasil_map(address_map &map)
{
	map(0x00000, 0x003ff).ram(); /*irq vector area*/
	map(0x00400, 0x0ffff).ram().share("nvram");
	map(0x40000, 0x7ffff).ram().share("blit_ram"); /*blitter ram*/
	map(0x80000, 0xbffff).bankr("mainbank");
	map(0xc0000, 0xfffff).rom().region("maincpu", 0x1c0000);
}

void highvdeo_state::brasil_io(address_map &map)
{
	map(0x0030, 0x0033).r(FUNC(highvdeo_state::brasil_status_r));
	map(0x0030, 0x0031).w(FUNC(highvdeo_state::brasil_status_w));
	map(0x0000, 0x0001).w(FUNC(highvdeo_state::write1_w)); // lamps
	map(0x0002, 0x0003).w(FUNC(highvdeo_state::write2_w)); // coin counter & coin lockout
	map(0x0006, 0x0007).w(FUNC(highvdeo_state::tv_oki6376_w));
	map(0x0008, 0x0009).r(FUNC(highvdeo_state::read0_r));
	map(0x000a, 0x000b).r(FUNC(highvdeo_state::read1_r));
	map(0x000e, 0x000e).rw(FUNC(highvdeo_state::read2_nmi_clear_r), FUNC(highvdeo_state::nmi_clear_w));
//  map(0xffa2, 0xffa3).w(FUNC(highvdeo_state::));
}

void highvdeo_state::fashion_output_w(uint16_t data)
{
	int i;

//  popmessage("%04x",data);

	for(i=0;i<4;i++)
	{
		machine().bookkeeping().coin_counter_w(i,data & 0x20);
		machine().bookkeeping().coin_lockout_w(i,~data & 0x01);
	}
}

void highvdeo_state::fashion_io(address_map &map)
{
	brasil_io(map);
	map(0x0002, 0x0003).w(FUNC(highvdeo_state::fashion_output_w));
}

void highvdeo_state::grancapi_io(address_map &map)
{
	map(0x0030, 0x0033).r(FUNC(highvdeo_state::grancapi_status_r));
	map(0x000e, 0x000f).w(FUNC(highvdeo_state::grancapi_status_w));
	map(0x0000, 0x0001).w(FUNC(highvdeo_state::write1_w)); // lamps
	map(0x0002, 0x0003).w(FUNC(highvdeo_state::write2_w)); // coin counter & coin lockout
	map(0x0006, 0x0007).w(FUNC(highvdeo_state::tv_oki6376_w));
	map(0x0008, 0x0009).r(FUNC(highvdeo_state::read0_r));
	map(0x000a, 0x000b).r(FUNC(highvdeo_state::read1_r));
	map(0x000e, 0x000e).rw(FUNC(highvdeo_state::read2_nmi_clear_r), FUNC(highvdeo_state::nmi_clear_w));
//  map(0xffa2, 0xffa3).w(FUNC(highvdeo_state::));
}

void highvdeo_state::magicbom_io(address_map &map)
{
	map(0x0002, 0x0003).w(FUNC(highvdeo_state::fashion_output_w));
	map(0x0030, 0x0033).r(FUNC(highvdeo_state::magicbom_status_r));
	map(0x000e, 0x000f).w(FUNC(highvdeo_state::grancapi_status_w));
	map(0x0000, 0x0001).w(FUNC(highvdeo_state::write1_w)); // lamps
	map(0x0002, 0x0003).w(FUNC(highvdeo_state::write2_w)); // coin counter & coin lockout
	map(0x0006, 0x0007).w(FUNC(highvdeo_state::tv_oki6376_w));
	map(0x0008, 0x0009).r(FUNC(highvdeo_state::read0_r));
	map(0x000a, 0x000b).r(FUNC(highvdeo_state::read1_r));
	map(0x000e, 0x000e).rw(FUNC(highvdeo_state::read2_nmi_clear_r), FUNC(highvdeo_state::nmi_clear_w));
//  map(0xffa2, 0xffa3).w(FUNC(highvdeo_state::));
}


static INPUT_PORTS_START( tv_vcf )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Button")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Risk Button")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) // Note 1
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Ticket")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Diagnostics") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Key")
	PORT_DIPNAME( 0x0002, 0x0000, "Reset NVRAM" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tv_ncf )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Button")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Risk Button")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) // Note 1
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER )        /* connected to the clock signal, to signal heartbeat */
	PORT_DIPNAME( 0x0080, 0x0000, "Reset NVRAM" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Diagnostics") PORT_CODE(KEYCODE_F2)
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END

static INPUT_PORTS_START( nyjoker )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Risk")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Choose")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Magic")

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER )        /* connected to the clock signal, to signal heartbeat */
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) // Coin 1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) // Coin 2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 ) // Coin 3
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) // Coin 4 <--- This one has non-timed pulse, so maybe was designed to be KEY IN.
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Note 1") PORT_CODE(KEYCODE_1_PAD)   // Note 1
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN6 ) PORT_NAME("Note 2") PORT_CODE(KEYCODE_2_PAD)   // Note 2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN7 ) PORT_NAME("Note 3") PORT_CODE(KEYCODE_3_PAD)   // Note 3
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN8 ) PORT_NAME("Note 4") PORT_CODE(KEYCODE_4_PAD)   // Note 4

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )  PORT_NAME("Bookkeeping")                    // Account
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Key")                            // Key
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Ticket")   PORT_CODE(KEYCODE_T)  // Ticket
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Unknown")  PORT_CODE(KEYCODE_U)  // Unknown
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Hopper 1") PORT_CODE(KEYCODE_H)  // Hopper 1
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Hopper 2") PORT_CODE(KEYCODE_J)  // Hopper 2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Level 2")  PORT_CODE(KEYCODE_K)  // Level 2
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Level 1")  PORT_CODE(KEYCODE_L)  // Level 1

	PORT_START("DSW")   // DIP switches bank
	PORT_DIPNAME( 0x0001, 0x0000, "DSW 8" )             PORT_DIPLOCATION("DSW:!8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Test Mode" )         PORT_DIPLOCATION("DSW:!7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "DSW 6" )             PORT_DIPLOCATION("DSW:!6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "DSW 5" )             PORT_DIPLOCATION("DSW:!5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "DSW 4" )             PORT_DIPLOCATION("DSW:!4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "DSW 3" )             PORT_DIPLOCATION("DSW:!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "DSW 2" )             PORT_DIPLOCATION("DSW:!2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "DSW 1" )             PORT_DIPLOCATION("DSW:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( tv_tcf )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Button")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Risk Button")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) // Note 1
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Ticket")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Diagnostics") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x0003, 0x0002, IPT_OTHER ) // Protection
	PORT_DIPNAME( 0x0004, 0x0000, "Reset NVRAM" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( brasil )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Button")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Risk Button")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) // note
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) //ticket
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0020, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) //hopper
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/*Slightly different inputs*/
static INPUT_PORTS_START( fashion )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Button")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stock 2")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Risk Button") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stock 3 / Note") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Stock 1 / Ticket") PORT_CODE(KEYCODE_Q)
	PORT_SERVICE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Stock 5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Stock 4") PORT_CODE(KEYCODE_R)
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

void highvdeo_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


void highvdeo_state::tv_vcf(machine_config &config)
{
	V30(config, m_maincpu, XTAL(12'000'000)/2); // ?
	m_maincpu->set_addrmap(AS_PROGRAM, &highvdeo_state::tv_vcf_map);
	m_maincpu->set_addrmap(AS_IO, &highvdeo_state::tv_vcf_io);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(400, 300);
	screen.set_visarea(0, 320-1, 0, 200-1);
	screen.set_screen_update(FUNC(highvdeo_state::screen_update_tourvisn));
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI, ASSERT_LINE);

	PALETTE(config, m_palette).set_entries(0x100);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &highvdeo_state::ramdac_map);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	//OkiM6376
	OKIM6376(config, m_okim6376, XTAL(12'000'000)/2/2/20).add_route(ALL_OUTPUTS, "mono", 1.0); //Guess, gives approx. same sample rate as previous emulation
}

void highvdeo_state::tv_ncf(machine_config &config)
{
	tv_vcf(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &highvdeo_state::tv_ncf_map);
	m_maincpu->set_addrmap(AS_IO, &highvdeo_state::tv_ncf_io);
}

void highvdeo_state::nyjoker(machine_config &config)
{
	tv_vcf(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &highvdeo_state::nyjoker_map);
	m_maincpu->set_addrmap(AS_IO, &highvdeo_state::nyjoker_io);
}

void highvdeo_state::tv_tcf(machine_config &config)
{
	tv_vcf(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &highvdeo_state::tv_tcf_map);
	m_maincpu->set_addrmap(AS_IO, &highvdeo_state::tv_tcf_io);

	subdevice<screen_device>("screen")->set_visarea(0, 400-1, 0, 300-1);

	m_palette->set_format(palette_device::RGB_565, 0x100);
}

void highvdeo_state::newmcard(machine_config &config)
{
	tv_tcf(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &highvdeo_state::newmcard_map);
	m_maincpu->set_addrmap(AS_IO, &highvdeo_state::newmcard_io);

	subdevice<screen_device>("screen")->set_visarea(0, 320-1, 0, 200-1);
}

void highvdeo_state::record(machine_config &config)
{
	newmcard(config);

	m_maincpu->set_addrmap(AS_IO, &highvdeo_state::record_io);
}

void highvdeo_state::ciclone(machine_config &config)
{
	tv_tcf(config);

	I80186(config.replace(), m_maincpu, 20000000);    // ?
	m_maincpu->set_addrmap(AS_PROGRAM, &highvdeo_state::tv_tcf_map);
	m_maincpu->set_addrmap(AS_IO, &highvdeo_state::ciclone_io);
}

void highvdeo_state::brasil(machine_config &config)
{
	I80186(config, m_maincpu, 20000000);  // fashion doesn't like 20/2 Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &highvdeo_state::brasil_map);
	m_maincpu->set_addrmap(AS_IO, &highvdeo_state::brasil_io);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(400, 300);
	screen.set_visarea(0, 400-1, 0, 300-1);
	screen.set_screen_update(FUNC(highvdeo_state::screen_update_brasil));
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI, ASSERT_LINE);

	PALETTE(config, m_palette, palette_device::RGB_565);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6376(config, m_okim6376, XTAL(12'000'000)/2/2/20).add_route(ALL_OUTPUTS, "mono", 1.0); //Guess, gives same sample rate as previous emulation
}

void highvdeo_state::fashion(machine_config &config)
{
	brasil(config);

	m_maincpu->set_addrmap(AS_IO, &highvdeo_state::fashion_io);
}

void highvdeo_state::grancapi(machine_config &config)
{
	I80186(config, m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &highvdeo_state::brasil_map);
	m_maincpu->set_addrmap(AS_IO, &highvdeo_state::grancapi_io);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(400, 300);
	screen.set_visarea(0, 400-1, 0, 300-1);
	screen.set_screen_update(FUNC(highvdeo_state::screen_update_brasil));
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI, ASSERT_LINE);

	PALETTE(config, m_palette, palette_device::RGB_565);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6376(config, m_okim6376, XTAL(12'000'000)/2/2/20).add_route(ALL_OUTPUTS, "mono", 1.0); //Guess, gives same sample rate as previous emulation
}

void highvdeo_state::magicbom(machine_config &config)
{
	I80186(config, m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &highvdeo_state::brasil_map);
	m_maincpu->set_addrmap(AS_IO, &highvdeo_state::magicbom_io);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(400, 300);
	screen.set_visarea(0, 400-1, 0, 300-1);
	screen.set_screen_update(FUNC(highvdeo_state::screen_update_brasil));
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI, ASSERT_LINE);

	PALETTE(config, m_palette, palette_device::RGB_565);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6376(config, m_okim6376, XTAL(12'000'000)/2/2/20).add_route(ALL_OUTPUTS, "mono", 1.0); //Guess, gives same sample rate as previous emulation
}


ROM_START( tour4000 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "vcfi18.bin", 0x00001, 0x80000, CRC(8c83cd34) SHA1(a94bdfdb74d047ac3851f2aef295a37c93b091f2) )
	ROM_LOAD16_BYTE( "vcfi17.bin", 0x00000, 0x80000, CRC(bcae57ed) SHA1(13c02cae59ed5cc0847a7827a315902066b03190) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( cfever40 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "vcfi48.bin", 0x00001, 0x80000, CRC(5a86a642) SHA1(fd927bc393242ff0aca87a0e3c2127f6f1df09cd) )
	ROM_LOAD16_BYTE( "vcfi47.bin", 0x00000, 0x80000, CRC(e7adc4d8) SHA1(862041c2c5d260727e525ab85fde18994484db16) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( cfever50 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "vcfi58.bin", 0x00001, 0x80000, CRC(c3464934) SHA1(1672c34d9ca250769973f7bc739137f153552eb9) )
	ROM_LOAD16_BYTE( "vcfi57.bin", 0x00000, 0x80000, CRC(2b789acb) SHA1(782ad3a6e0eacbf9adec4afd20a309215913e505) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( tour4010 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "ncfi18.bin", 0x00001, 0x80000, CRC(294929d9) SHA1(712926cf1f78197fce838a4e76d70082182214eb) )
	ROM_LOAD16_BYTE( "ncfi17.bin", 0x00000, 0x80000, CRC(4a8ac279) SHA1(41b0de4444466700ef2de2e926c8fa6f0bda280d) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( cfever51 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "ncfi58.bin", 0x00001, 0x80000, CRC(cdf9c2f0) SHA1(94f9cf6b1856becd74971022ded6db5ae927fb54) )
	ROM_LOAD16_BYTE( "ncfi57.bin", 0x00000, 0x80000, CRC(5005cf2b) SHA1(468ccd27fcb8bdb7d6ccf423542e1d4773930b88) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( cfever61 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "ncfi68.bin", 0x00001, 0x80000, CRC(51fe839f) SHA1(e4d9bce4a995cb407faaf36b2c1e10409a2e94da) )
	ROM_LOAD16_BYTE( "ncfi67.bin", 0x00000, 0x80000, CRC(d889d6b6) SHA1(791d9b9fc2d0a128ab07a9ae18a32f2838a5ea3f) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

/*
  New York Joker...

  pcb made in spain video/mpu-5

  CPU d70116c 8
  adv476kp35 CMOS Monolithic 256x18 Color Palette RAM-DAC
  Xtal 16Mhz
  oki m6376
  Lattice   isplsi1032e

  model TV
  vers 2.0
  date 2/99

  1 empty socket
  ny2.ic25 is audio

*/

ROM_START( nyjoker )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "ni8-2-1.ic8", 0x00001, 0x80000, CRC(81416871) SHA1(c5519b1fcf0131710a8972d9016b8af5f8ac75a1) )
	ROM_LOAD16_BYTE( "ni7-2-1.ic7", 0x00000, 0x80000, CRC(835b8606) SHA1(a036f8568f0e41eb1f4db7fa41a9cd4b92d41514) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ny2.ic25", 0x00000, 0x80000, CRC(eeea7f4d) SHA1(2afc498792f848fd45be4d3eb3e6607edb5dd9df) )

	ROM_REGION( 0x3c00, "nvram", 0 )    /* default NVRAM (to check bounds) */
	ROM_LOAD( "nyjoker_nvram", 0x0000, 0x3c00, CRC(5ed3d184) SHA1(043ac9ea33676529d02e340891f8447c4497e73e) )
ROM_END

ROM_START( cfever1k )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "tcfi28.bin", 0x00001, 0x100000, CRC(e38d115a) SHA1(7fec94ddcdb07e483ed2f0d7d667c35ceb7a1f44) )
	ROM_LOAD16_BYTE( "tcfi27.bin", 0x00000, 0x100000, CRC(32f884e6) SHA1(cc74a4c6313654bbd363a89fe7757a05c74de45b) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "ic25.bin", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END


ROM_START( girotutt )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "tcfi18.bin", 0x00001, 0x100000, CRC(822ab6a1) SHA1(04f4254da46cf67ea17587fde4a0fdd39c658b3b) )
	ROM_LOAD16_BYTE( "tcfi17.bin", 0x00000, 0x100000, CRC(b326a0ee) SHA1(c96b7578c112a97ba1d8de4d3d0ae68fef846cad) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "t41.bin", 0x00000, 0x80000, CRC(6f694406) SHA1(ec8b8baba0ee1bfe8986ce978412ee4de06f1906) )
ROM_END


ROM_START( galeone )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "model.a8-vers.1.0.ic8", 0x00001, 0x40000, CRC(b9e1e7ce) SHA1(4e036285b26dc0a313e76259b7e67c8d55322c84) )
	ROM_RELOAD(0x80001,0x40000)
	ROM_LOAD16_BYTE( "model.a7-vers.1.0.ic7", 0x00000, 0x40000, CRC(1940b738) SHA1(81e40de8df4dc838342bf966658110989a233731) )
	ROM_RELOAD(0x80000,0x40000)

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "model.ga-vers.1.ic25", 0x00000, 0x80000, CRC(4c2c2cc1) SHA1(20da29b2f1dd1f86ec23d9dbdaa9470878e900e2) )
ROM_END

/*
CPU

1x NEC 9145N5-V30-D70116C-8 (main)
1x OKI M6376 (sound)
1x ispLSI2032-80LJ-H013J05 (main)
1x ispLSI1032E-70LJ-E013S09 (main)
1x ADV476KP35-9948-F112720.1 (GFX)
1x oscillator 16.000MHz

ROMs
1x M27C2001 (ic31)
2x M27C4001 (ic32,ic33)

Note

1x 28x2 edge connector (not JAMMA)
1x 8 legs connector
1x 3 legs jumper
1x pushbutton
1x battery
1x trimmer (volume)

PCB markings: "V150500 CE type 001/v0"
PCB n. E178247

*/

ROM_START( newmcard )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF ) /* V30 Code; Fix for extra bank bit */
	ROM_LOAD16_BYTE( "mc32.ic4", 0x00000, 0x80000, CRC(d9817f48) SHA1(c523a8248b487081ea2e0e326dcc660b051c23c1) )
	ROM_LOAD16_BYTE( "mc33.ic5", 0x00001, 0x80000, CRC(83a855ab) SHA1(7f9384c875b951d17caa91f8a7365edaf7f9afe1) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "mc31.ic15", 0x00000, 0x40000, CRC(8b72ffec) SHA1(fca5cf2594325e0c9fe446ddf2330c669f7f37a9) )
ROM_END

/*
1x N80C186XL25 PLCC68 (main)(u1)
1x M6376 (sound)(u17)
1x TDA1010A (sound)(u19)
1x oscillator 40.000 MHz
1x ispLSI2032 (PLCC44)(u13)
1x ispLSI1032E (PLCC84)(u18)
1x NE555 (u25)

1x 28x2 edge connector
1x 5 legs connector (cn2)
1x 8 legs connector (cn3)
1x trimmer (volume)
1x pushbutton (K1)
1x 3 legs jumper (s3)
1x battery 3.6V (b1)
*/

ROM_START( ciclone )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* N80C186XL25 Code */
	ROM_LOAD16_BYTE( "hclv1.u7", 0x000000, 0x100000, CRC(071c64f2) SHA1(5125c3caf77258260bfa4c24dd612cedf61fe7f2) )
	ROM_LOAD16_BYTE( "hclv1.u8", 0x000001, 0x100000, CRC(c2ed99b4) SHA1(a1a3bfa9a6ea53979c20d60ccd7eb1773c805fc8) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "hclv1.u16", 0x00000, 0x80000, CRC(45b2b53a) SHA1(983bcc5869d84938ba278f26339dd72c17ed1d00) )
ROM_END

/*
CPUs
N80C186XL25 (main)(u1)
1x ispLSI2032-80LJ (u13)(not dumped)
1x ispLSI1032E-70LJ (u18)(not dumped)
1x M6376 (sound)(u17)
1x oscillator 40.000MHz

ROMs
1x MX27C4000 (u16)
2x M27C801 (u7,u8)

Note

1x 28x2 edge connector (cn1)
1x 5 legs connector (cn2)
1x 8 legs connector (cn3)
1x trimmer (volume)
1x pushbutton (k1)
1x battery (b1)


cpu is 80186 based (with extras), see
http://media.digikey.com/pdf/Data%20Sheets/Intel%20PDFs/80C186XL,%2080C188XL.pdf

*/


ROM_START( brasil )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* N80C186XL25 Code */
	ROM_LOAD16_BYTE( "record_brasil_hrc7_vers.3.u7", 0x000000, 0x100000, CRC(627e0d58) SHA1(6ff8ba7b21e1ea5c88de3f02a057906c9a7cd808) )
	ROM_LOAD16_BYTE( "record_brasil_hrc8_vers.3.u8", 0x000001, 0x100000, CRC(47f7ba2a) SHA1(0add7bbf771fd0bf205a05e910cb388cf052b09f) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "sound_brasil_hbr_vers.1.u16", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END


ROM_START( fashion )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* N80C186XL25 Code */
	ROM_LOAD16_BYTE( "fashion1-hfs7v2.14.high-video8m.u7", 0x000000, 0x100000, CRC(20411b89) SHA1(3ed6336978e5046eeef26115614cb74e3ffe134a) )
	ROM_LOAD16_BYTE( "fashion1-hfs8v2.14.high-video8m.u8", 0x000001, 0x100000, CRC(521f34f3) SHA1(91edc90fcd895a096955ac031a42da04510df1e6) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "sound-fashion-v-1-memory4m.u16", 0x00000, 0x80000, CRC(2927c799) SHA1(f11cad096a23fee10bfdff5bf944c96e30f4a8b8) )
ROM_END


ROM_START( grancapi )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* N80C186XL25 Code */
	ROM_LOAD16_BYTE( "gran-capitan-hgcp-i38-vers3-high-video.ic7", 0x000000, 0x100000, CRC(8bb75c50) SHA1(3a54daaf57ff4ffd1ebea6bfa33d40dbfcfe8d8f) )
	ROM_LOAD16_BYTE( "gran-capitan-hgcp-i38-vers3-high-video.ic8", 0x000001, 0x100000, CRC(28ad57f1) SHA1(093b117ab315fca1c0905363e8f637dbf96c48ec) )

	ROM_REGION( 0x040000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "sound-gran-capitan-hgcp-vers1-memory-2m.ic16", 0x00000, 0x40000, CRC(3d19146e) SHA1(a0e45df231fa7513e294633cbdbe46bf9bd77c1b) )
ROM_END

ROM_START( magicbom )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* N80C186XL25 Code */
	ROM_LOAD16_BYTE( "magic-bomb-hmb-i17-vers1-high-video.ic7", 0x000000, 0x100000, CRC(d217ae33) SHA1(ca3a13d9d23809583733e7b1bdba096a50fe7488) )
	ROM_LOAD16_BYTE( "magic-bomb-hmb-i17-vers1-high-video.ic8", 0x000001, 0x100000, CRC(53a9c3d5) SHA1(72a2a07c0bd7b00566795042bcff1f6f324b964f) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "sound-magic-bomb-hmb-vers1-memory-4m.ic16", 0x00000, 0x80000, CRC(45b2b53a) SHA1(983bcc5869d84938ba278f26339dd72c17ed1d00) )
ROM_END

ROM_START( record ) // do checks and expect something... pc=e8044
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF ) /* V30 Code; Fix for extra bank bit */
	ROM_LOAD16_BYTE( "record-vrc-i17-vers1-video-map.ic7", 0x00000, 0x80000, CRC(d0e59a64) SHA1(5f51448a4cdefd335e19affa4b47df7b428b0e7c) )
	ROM_LOAD16_BYTE( "record-vrc-i17-vers1-video-map.ic8", 0x00001, 0x80000, CRC(823d1c25) SHA1(3104567b2b05708d1b5218f9f0e64bfa3d0df46b) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "sound-record-hrc-vers1-memory-2m.ic16", 0x00000, 0x40000, CRC(8b72ffec) SHA1(fca5cf2594325e0c9fe446ddf2330c669f7f37a9) )
ROM_END

ROM_START( cuncino )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* N80C186XL25 Code */
	ROM_LOAD16_BYTE( "capitan uncino-hcu i27 vers.2_high video ic7.u7", 0x000000, 0x100000, CRC(47501ad8) SHA1(1aca8427dbd88bf8dbab768361e9d3e359ade78e) )
	ROM_LOAD16_BYTE( "capitan uncino-hcu i28 vers.2_high video ic8.u8", 0x000001, 0x100000, CRC(81593809) SHA1(833b5226143d9d11f4e2db09ea76831f34e8751d) )

	ROM_REGION( 0x080000, "oki", 0 ) /* M6376 Samples */
	ROM_LOAD( "sound-capitan uncino-hcu vers.1.u16", 0x00000, 0x80000, CRC(de87edb0) SHA1(a54c17dd21a756b1dbaec94b144db797d25ab7d6) ) // 1ST AND 2ND HALF IDENTICAL, matches grancapi's one when split in half
ROM_END

ROM_START( unksmk ) // VIDEO/SMK-1 C PCB. V30 + OKI M6376. Feigns to be a NeoGeo multigame to cover gambling game? Needs investigation.
	ROM_REGION( 0x100000, "maincpu", 0 ) // no labels
	ROM_LOAD16_BYTE( "ic7", 0x00000, 0x40000, CRC(ce1f303c) SHA1(8563ba217357857d0bc49cc64426406fa12a2f9b) )
	ROM_RELOAD(             0x80000, 0x40000 )
	ROM_LOAD16_BYTE( "ic8", 0x00001, 0x40000, CRC(b85575e5) SHA1(4fcda845c8e1b6c843cfe0a1f9b06df806325a7d) )
	ROM_RELOAD(             0x80001, 0x40000 )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "ng 1.ic25", 0x00000, 0x80000, CRC(89291fe6) SHA1(f729e6b90a98151ff64baa1d852137a86bd0ef93) ) // handwritten label
ROM_END

ROM_START( newtiger ) // New High Video TYPE 001/V0 PCB. N80C186XL25 + OKIM6376 + Lattice ispLSI1032E + Lattice ispLSI2032. Also found on PCB with no markings, otherwise almost identical. These have a big black box covering part of the PCB which isn't present on other PCBs for these driver
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "new tiger pt hnt 7 v.-1.0 h.video 9.a 8m.u7", 0x000000, 0x100000, CRC(57c68716) SHA1(d6c6d285b63809864b7868b5fdfef31519eb94bf) )
	ROM_LOAD16_BYTE( "new tiger pt hnt 8 v.-1.0 h.video 9.a 8m.u8", 0x000001, 0x100000, CRC(e83bd175) SHA1(d454783f58038ca8938077fa6575b22494056f6f) )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "new tiger sound v.-1 memory 2m.u16", 0x00000, 0x40000, CRC(aa8444ec) SHA1(3663c01a7cf2fe334eb6ea974d6a0c0b6bb88354) )
ROM_END

} // Anonymous namespace


GAMEL( 2000, tour4000,  0,      tv_vcf,   tv_vcf,  highvdeo_state, empty_init, ROT0, "High Video",     "Tour 4000",                              0,                   layout_fashion )
GAMEL( 2000, cfever40,  0,      tv_vcf,   tv_vcf,  highvdeo_state, empty_init, ROT0, "High Video",     "Casino Fever 4.0",                       0,                   layout_fashion )
GAMEL( 2000, cfever50,  0,      tv_vcf,   tv_vcf,  highvdeo_state, empty_init, ROT0, "High Video",     "Casino Fever 5.0",                       0,                   layout_fashion )
GAMEL( 2000, tour4010,  0,      tv_ncf,   tv_ncf,  highvdeo_state, empty_init, ROT0, "High Video",     "Tour 4010",                              0,                   layout_fashion )
GAMEL( 2000, cfever51,  0,      tv_ncf,   tv_ncf,  highvdeo_state, empty_init, ROT0, "High Video",     "Casino Fever 5.1",                       0,                   layout_fashion )
GAMEL( 2000, cfever61,  0,      tv_ncf,   tv_ncf,  highvdeo_state, empty_init, ROT0, "High Video",     "Casino Fever 6.1",                       0,                   layout_fashion )
GAMEL( 2000, nyjoker,   0,      nyjoker,  nyjoker, highvdeo_state, empty_init, ROT0, "High Video",     "New York Joker",                         0,                   layout_fashion )
GAMEL( 2000, cfever1k,  0,      tv_tcf,   tv_tcf,  highvdeo_state, empty_init, ROT0, "High Video",     "Casino Fever 1k",                        0,                   layout_fashion )
GAMEL( 2000, girotutt,  0,      tv_tcf,   tv_tcf,  highvdeo_state, empty_init, ROT0, "High Video",     "GiroTutto",                              0,                   layout_fashion )
GAMEL( 2000, galeone,   0,      nyjoker,  nyjoker, highvdeo_state, empty_init, ROT0, "San Remo Games", "Il Galeone",                             0,                   layout_fashion )
GAMEL( 2000, ciclone,   0,      ciclone,  tv_tcf,  highvdeo_state, empty_init, ROT0, "High Video",     "Ciclone",                                0,                   layout_fashion )
GAMEL( 2000, newmcard,  0,      newmcard, tv_tcf,  highvdeo_state, empty_init, ROT0, "High Video",     "New Magic Card",                         0,                   layout_fashion )
GAMEL( 2000, brasil,    0,      brasil,   brasil,  highvdeo_state, empty_init, ROT0, "High Video",     "Bra$il (Version 3)",                     0,                   layout_fashion )
GAMEL( 2000, fashion,   brasil, fashion,  fashion, highvdeo_state, empty_init, ROT0, "High Video",     "Fashion (Version 2.14)",                 0,                   layout_fashion )
GAMEL( 2000, grancapi,  0,      grancapi, brasil,  highvdeo_state, empty_init, ROT0, "High Video",     "Gran Capitan (Version 3)",               MACHINE_NOT_WORKING, layout_fashion )
GAMEL( 2000, magicbom,  0,      magicbom, fashion, highvdeo_state, empty_init, ROT0, "High Video",     "Magic Bomb (Version 1)",                 MACHINE_NOT_WORKING, layout_fashion )
GAMEL( 2000, record,    0,      record,   tv_tcf,  highvdeo_state, empty_init, ROT0, "High Video",     "Record (Version 1)",                     0,                   layout_fashion )
GAMEL( 2000, cuncino,   0,      grancapi, brasil,  highvdeo_state, empty_init, ROT0, "High Video",     "Capitan Uncino (High Video, version 2)", MACHINE_NOT_WORKING, layout_fashion )
GAMEL( 200?, unksmk,    0,      nyjoker,  nyjoker, highvdeo_state, empty_init, ROT0, "High Video",     "unknown SMK game",                       MACHINE_NOT_WORKING, layout_fashion ) // needs correct inputs
GAMEL( 200?, newtiger,  0,      magicbom, fashion, highvdeo_state, empty_init, ROT0, "High Video",     "New Tiger",                              MACHINE_NOT_WORKING, layout_fashion ) // slightly differently protected
