// license:BSD-3-Clause
// copyright-holders:Philip Bennett, R. Belmont
/*
  Konami Gambling Games ("Tasman" hardware)
  System GX derivative

  68EC020 @ 25 MHz
  K056832 + K058143 : GX tilemaps
  K055673(x2) + K053246A : (extended?) GX sprites
  K055555 : GX mixer/blender

  68000 @ 16 MHz + YMZ280B for audio

  Thanks to palindrome for PCB scans.

    0x38606

  TODO:
  - dual port sound RAM;
  - interrupt sources / irq masking;
*/

#include "emu.h"
#include "konami_helper.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "machine/k053252.h"
#include "machine/timer.h"
#include "k053246_k053247_k055673.h"
#include "k054156_k054157_k056832.h"
#include "k055555.h"
#include "machine/eepromser.h"
#include "emupal.h"
#include "speaker.h"


namespace {

#define CUSTOM_DRAW 1

class kongambl_state : public driver_device
{
public:
	kongambl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_k053252(*this, "k053252"),
		m_k055673(*this, "k055673"),
		m_k055555(*this, "k055555"),
		m_k056832(*this, "k056832"),
#if CUSTOM_DRAW
		m_gfxdecode(*this, "gfxdecode"),
#endif
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram")
	{ }

	void kongambl(machine_config &config);

	void init_kingtut();

protected:
	virtual void machine_reset() override { m_irq_mask = 0; };
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<k053252_device> m_k053252;
	required_device<k055673_device> m_k055673;
	required_device<k055555_device> m_k055555;
	required_device<k056832_device> m_k056832;
#if CUSTOM_DRAW
	required_device<gfxdecode_device> m_gfxdecode;
#endif
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint32_t> m_vram;
	uint32_t eeprom_r(offs_t offset, uint32_t mem_mask = ~0);
	void eeprom_w(offs_t offset, uint8_t data);
	void kongambl_ff_w(uint8_t data);
	uint32_t test_r();
	// uint32_t rng_r();

	uint8_t m_irq_mask = 0;

	uint32_t screen_update_kongambl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq_ack_w(int state);
	void hblank_irq_ack_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(kongambl_vblank);
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);

	void kongamaud_map(address_map &map) ATTR_COLD;
	void kongambl_map(address_map &map) ATTR_COLD;
};


void kongambl_state::video_start()
{
	#if CUSTOM_DRAW

	#else
	m_k056832->set_layer_association(0);
	m_k056832->set_layer_offs(0, -2, 0);
	m_k056832->set_layer_offs(1,  2, 0);
	m_k056832->set_layer_offs(2,  4, 0);
	m_k056832->set_layer_offs(3,  6, 0);
	#endif
}

uint32_t kongambl_state::screen_update_kongambl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	#if CUSTOM_DRAW
	gfx_element *gfx = m_gfxdecode->gfx(0);
	uint32_t count;

	count = 0;

	for (int y=0;y<64;y++)
	{
		for (int x=0;x<128;x++)
		{
			uint32_t tile = m_vram[count] & 0xffff;

			if(m_screen->visible_area().contains(x*8, y*8))
				gfx->opaque(bitmap,cliprect,tile,0,0,0,x*8,y*8);

			count++;
		}
	}

	count = 0x8000/4;

	for (int y=0;y<64;y++)
	{
		for (int x=0;x<128;x++)
		{
			uint32_t tile = m_vram[count] & 0xffff;

			if(m_screen->visible_area().contains(x*8, y*8))
				gfx->transpen(bitmap,cliprect,tile,0,0,0,x*8,y*8,0);

			count++;
		}
	}


	#else
	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 3, 0, 0);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);
	#endif
	return 0;
}

uint32_t kongambl_state::eeprom_r(offs_t offset, uint32_t mem_mask)
{
	//return machine().rand();
	uint32_t retval = 0;

	if (ACCESSING_BITS_24_31)
		retval |= ioport("IN0")->read() << 24; // bit 0 freezes the system if 1

	if (ACCESSING_BITS_16_23)
		retval |= ioport("EXT_PCB")->read() << 16; // ???

	if (ACCESSING_BITS_8_15)
		retval |= ioport("IN2")->read() << 8; // ???

	if (ACCESSING_BITS_0_7)
		retval |= (ioport("SYSTEM")->read());


//  printf("%08x\n",mem_mask);

	return retval;
}
void kongambl_state::eeprom_w(offs_t offset, uint8_t data)
{
	// offset == 3 seems mux writes (active low)

	if (offset == 2)
	{
		ioport("EEPROMOUT")->write(data&0x7, 0xff);
		if(data & 0xf8)
			printf("Unused EEPROM bits %02x %02x\n",offset,data);
	}
	else
		printf("%02x %02x\n",offset,data);

	if(offset == 0)
		m_irq_mask = data;
}

uint32_t kongambl_state::test_r()
{
	return -1;//machine().rand();
}

/*
 uint32_t kongambl_state::rng_r()
{
    return machine().rand();
}
*/

void kongambl_state::kongambl_ff_w(uint8_t data)
{
	/* enables thru 0->1 */
	/* ---- x--- (related to OBJ ROM) */
	/* ---- -x-- k056832 upper ROM bank */
	/* ---- --x- k056832 related (enabled when testing ROM area) */
//  printf("%02x\n",data);
}

void kongambl_state::kongambl_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom(); // main program
	map(0x100000, 0x11ffff).ram(); // work RAM

	map(0x200000, 0x207fff).ram(); // backup RAM 24F

	map(0x300000, 0x307fff).ram(); // backup RAM 24H

	// override konami chips with custom areas until that code is removed
	map(0x420000, 0x43ffff).ram().share("vram");
	//map(0x480000, 0x48003f).ram(); // vregs

	//0x400000 0x400001 "13M" even addresses
	//0x400002,0x400003 "13J" odd addresses
	map(0x400000, 0x401fff).r(m_k056832, FUNC(k056832_device::rom_word_r));
//  map(0x420000, 0x43ffff).rw(m_k056832, FUNC(k056832_device::unpaged_ram_word_r), FUNC(k056832_device::unpaged_ram_word_w));
	map(0x480000, 0x48003f).w(m_k056832, FUNC(k056832_device::word_w));



	map(0x440000, 0x443fff).ram(); // OBJ RAM

	map(0x460000, 0x47ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");

	map(0x4b0000, 0x4b001f).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write)).umask32(0xff00ff00);

	map(0x4c0000, 0x4c0007).w(m_k055673, FUNC(k055673_device::k053246_w));
	//map(0x4c4000, 0x4c4003).nopw();
	//map(0x4c4004, 0x4c4007).nopw();
	//map(0x4c801c, 0x4c801f).nopw();
	//map(0x4cc01c, 0x4cc01f).nopw();

	map(0x4cc000, 0x4cc00f).r(m_k055673, FUNC(k055673_device::k055673_rom_word_r));

	map(0x4d0000, 0x4d0000).w(FUNC(kongambl_state::kongambl_ff_w));

	map(0x500000, 0x5007ff).ram();
	map(0x500380, 0x500383).r(FUNC(kongambl_state::test_r));
//  map(0x500400, 0x500403).noprw(); //dual port?
//  map(0x500420, 0x500423).noprw(); //dual port?
//  map(0x500500, 0x500503).noprw(); // reads sound ROM in here, polled from m68k?
	map(0x580000, 0x580007).r(FUNC(kongambl_state::test_r));

	map(0x600000, 0x60000f).r(FUNC(kongambl_state::test_r));

	map(0x700000, 0x700003).r(FUNC(kongambl_state::eeprom_r));
	map(0x700004, 0x700007).portr("IN1");
	map(0x700008, 0x70000b).portr("IN3");
	map(0x780000, 0x780003).w(FUNC(kongambl_state::eeprom_w));
	//map(0x780004, 0x780007).nopw();
}

void kongambl_state::kongamaud_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom(); // main program (mirrored?)
	map(0x100000, 0x10ffff).ram(); // work RAM
	map(0x180000, 0x180001).nopw();
	map(0x190000, 0x190001).nopw();
	map(0x1a0000, 0x1a0001).nopw();
	map(0x1b0000, 0x1b0001).nopr();
	map(0x1c0000, 0x1c0001).nopr();
	map(0x200000, 0x2000ff).ram(); // unknown (YMZ280b?  Shared with 68020?)
	map(0x280000, 0x2800ff).ram();
	map(0x300000, 0x3007ff).ram();
}



static INPUT_PORTS_START( kongambl )
	PORT_START( "SYSTEM" )
	PORT_DIPNAME( 0x01, 0x00, "SYSTEM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START( "IN0" )
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Reset Backup RAM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPNAME( 0x08, 0x08, "Reset Signal" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START( "EXT_PCB" )
	PORT_DIPNAME( 0x01, 0x00, "SYSB" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "No Battery - 1" ) // "NO BATTERY"
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "No Battery - 2" ) // "NO BATTERY"
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "IFU signal" ) // IFU
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "LCU signal" ) // LCU
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "VSB signal" ) // VSB
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START( "IN1" )
	PORT_DIPNAME( 0x01, 0x00, "SYSC" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, "SYSD" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )
	PORT_DIPNAME( 0x010000, 0x000000, "SYSE" )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x010000, DEF_STR( On ) )
	PORT_DIPNAME( 0x020000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x020000, DEF_STR( On ) )
	PORT_DIPNAME( 0x040000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x040000, DEF_STR( On ) )
	PORT_DIPNAME( 0x080000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x080000, DEF_STR( On ) )
	PORT_DIPNAME( 0x100000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x100000, DEF_STR( On ) )
	PORT_DIPNAME( 0x200000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x200000, DEF_STR( On ) )
	PORT_DIPNAME( 0x400000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x400000, DEF_STR( On ) )
	PORT_DIPNAME( 0x800000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x800000, DEF_STR( On ) )
	PORT_DIPNAME( 0x01000000, 0x00000000, "SYSF" )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x04000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x80000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80000000, DEF_STR( On ) )

	PORT_START( "IN2" )
	PORT_DIPNAME( 0x01, 0x00, "SYSG" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START( "IN3" )
	PORT_DIPNAME( 0x01, 0x01, "SYSC" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, "SYSD" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )
	PORT_DIPNAME( 0x010000, 0x000000, "SYSE" )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x010000, DEF_STR( On ) )
	PORT_DIPNAME( 0x020000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x020000, DEF_STR( On ) )
	PORT_DIPNAME( 0x040000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x040000, DEF_STR( On ) )
	PORT_DIPNAME( 0x080000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x080000, DEF_STR( On ) )
	PORT_DIPNAME( 0x100000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x100000, DEF_STR( On ) )
	PORT_DIPNAME( 0x200000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x200000, DEF_STR( On ) )
	PORT_DIPNAME( 0x400000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x400000, DEF_STR( On ) )
	PORT_DIPNAME( 0x800000, 0x000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x800000, DEF_STR( On ) )
	PORT_DIPNAME( 0x01000000, 0x00000000, "SYSF" )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x04000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x10000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x80000000, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80000000, DEF_STR( On ) )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
INPUT_PORTS_END


K053246_CB_MEMBER(kongambl_state::sprite_callback)
{
}

K056832_CB_MEMBER(kongambl_state::tile_callback)
{
}


static const gfx_layout charlayout8_tasman =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	// 0,8,16,24,32,40,48,56
	{ 56,24,40,8,48,16,32,0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64},
	8*64
};

static GFXDECODE_START( gfx_tasman )
	GFXDECODE_ENTRY( "k056832", 0, charlayout8_tasman, 0, 0x8000/(1 << 8) )
GFXDECODE_END


void kongambl_state::vblank_irq_ack_w(int state)
{
	m_maincpu->set_input_line(1, CLEAR_LINE);
}

void kongambl_state::hblank_irq_ack_w(int state)
{
	m_maincpu->set_input_line(2, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(kongambl_state::kongambl_vblank)
{
	int scanline = param;

	// disabled for now since it interferes with the ROM tests
	if(scanline == 384 && m_irq_mask & 1)
		m_maincpu->set_input_line(1, HOLD_LINE); // vblank?

	//if(scanline == 256 && m_irq_mask & 2)
	//  m_maincpu->set_input_line(2, HOLD_LINE); // unknown (jumps to work RAM via a branch or returns lv 2 exception error, extension board?)

	if(scanline == 0 && m_irq_mask & 4)
		m_maincpu->set_input_line(3, HOLD_LINE); // sprite irq?

	if(scanline == 128 && m_irq_mask & 8)
		m_maincpu->set_input_line(4, HOLD_LINE); // sound irq

}

void kongambl_state::kongambl(machine_config &config)
{
	M68EC020(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &kongambl_state::kongambl_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(kongambl_state::kongambl_vblank), "screen", 0, 1);

	m68000_device &sndcpu(M68000(config, "sndcpu", 16000000));
	sndcpu.set_addrmap(AS_PROGRAM, &kongambl_state::kongamaud_map);
	sndcpu.set_periodic_int(FUNC(kongambl_state::irq2_line_hold), attotime::from_hz(480));

	K053252(config, m_k053252, 25000000);
	m_k053252->set_offsets(0, 16); // TBD
	m_k053252->int1_ack().set(FUNC(kongambl_state::vblank_irq_ack_w));
	m_k053252->int2_ack().set(FUNC(kongambl_state::hblank_irq_ack_w));
	m_k053252->set_screen(m_screen);

	EEPROM_93C46_16BIT(config, "eeprom");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(25000000, 288+16+32+48, 0, 287, 224+16+8+16, 0, 223); // fake, they'll be changed by CCU anyway, TBD
	m_screen->set_screen_update(FUNC(kongambl_state::screen_update_kongambl));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 32768);

	K055555(config, m_k055555, 0);

	K055673(config, m_k055673, 0);
	m_k055673->set_sprite_callback(FUNC(kongambl_state::sprite_callback));
	m_k055673->set_config(K055673_LAYOUT_LE2, -48+1, -23);
	m_k055673->set_palette(m_palette);

#if CUSTOM_DRAW
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tasman);
#endif

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(kongambl_state::tile_callback));
	m_k056832->set_config(K056832_BPP_8TASMAN, 0, 0);
	m_k056832->set_palette(m_palette);

	SPEAKER(config, "speaker", 2).front();
}


ROM_START( kingtut )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68EC020 Code */
	ROM_LOAD32_WORD_SWAP( "kitp1b37_l.02", 0x000002, 0x40000, CRC(95c6da28) SHA1(3ef33f5d0748c80be82d33c21f0f8bb71909884e) )
	ROM_RELOAD(0x080002, 0x40000)
	ROM_LOAD32_WORD_SWAP( "kitp1b37_h.01", 0x000000, 0x40000, CRC(16709625) SHA1(6b818a85724f87fed23a26978dd26b079f814134) )
	ROM_RELOAD(0x080000, 0x40000)

	ROM_REGION( 0x80000, "sndcpu", 0 ) /* 68000 sound program */
	ROM_LOAD16_WORD_SWAP( "n12prog_ifu.41", 0x00000, 0x08000, CRC(dbb8a7e8) SHA1(9662b34e9332385d20e17ee1c92fd91935d4c3b2) )

	ROM_REGION( 0x100000, "k056832", 0 ) // 8x8x8 tiles
	ROM_LOAD16_BYTE( "kit11_l1_vrm.21", 0x000000, 0x80000, CRC(431eb89f) SHA1(377c96f615b4b76314aeecad4e868edb66c72f33) )
	ROM_LOAD16_BYTE( "kit11_h1_vrm.23", 0x000001, 0x80000, CRC(7aa2f1bc) SHA1(d8aead9dedcc83d3dc574122103aaa2074011197) )

	ROM_REGION( 0x200000, "k055673", 0 ) // 16x16x8 sprites
	ROM_LOAD64_WORD( "kit11ll1_obj.17", 0x000000, 0x80000, CRC(a19338b8) SHA1(1aa68596e5bf493cb360495f1174dc1323086ad2) )
	ROM_LOAD64_WORD( "kit11lm1_obj.15", 0x000002, 0x80000, CRC(1aea3f4d) SHA1(52fd1a7ffeeb3acce176ad3812a2ca146e02c324) )
	ROM_LOAD64_WORD( "kit11hm1_obj.13", 0x000004, 0x80000, CRC(21cc4e40) SHA1(9e3735fc8cd53f7e831dc76697911216bd8bbc70) )
	ROM_LOAD64_WORD( "kit11hh1_obj.11", 0x000006, 0x80000, CRC(a64d2382) SHA1(bb745a26ef6c076f3aa3ec476589a95915b359ed) )

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "snd12sd1_snd.31", 0x000000, 0x80000, CRC(f4121baa) SHA1(723c6d96ecef5ef510d085f443d44bad07aa19e5) )
	ROM_LOAD( "kit11sd2_snd.32", 0x080000, 0x80000, CRC(647c6e2e) SHA1(e013239a73553e2993adabeda103f5b1cfee0f6c) )
ROM_END

ROM_START( moneybnk )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68EC020 Code */
	ROM_LOAD32_WORD_SWAP( "mobn6l29_l.02", 0x000002, 0x40000, CRC(9cd2754a) SHA1(2eb695cb4abab4a448711b8acf3f5b1bb169eb6f) )
	ROM_RELOAD(0x080002, 0x40000)
	ROM_LOAD32_WORD_SWAP( "mobn6l29_h.01", 0x000000, 0x40000, CRC(952c376b) SHA1(0fc0499f5570b920c600ddd6a15751d72345c83e) )
	ROM_RELOAD(0x080000, 0x40000)

	ROM_REGION( 0x80000, "sndcpu", 0 ) /* 68000 sound program */
	ROM_LOAD16_WORD_SWAP( "n12prog_ifu.41", 0x00000, 0x08000, CRC(dbb8a7e8) SHA1(9662b34e9332385d20e17ee1c92fd91935d4c3b2) ) // some kind of bios? same on both games

	ROM_REGION( 0x100000, "k056832", 0 ) // 8x8x8 tiles
	ROM_LOAD16_BYTE( "mob11_l1_vrm.21", 0x000000, 0x80000, CRC(926fbd3b) SHA1(4f85ea63faff1508d5abf0ca0ebd16e802f8f45c) )
	ROM_LOAD16_BYTE( "mob11_h1_vrm.23", 0x000001, 0x80000, CRC(a119feaa) SHA1(567e319dfddb9ec04b9302af782e9baccab4f5a6) )

	ROM_REGION( 0x200000, "k055673", 0 ) // 16x16x8 sprites
	ROM_LOAD64_WORD( "mob11ll1_obj.17", 0x000000, 0x80000, CRC(5c5959a3) SHA1(1eea6bf4c34aa05f45b2737eb6035f2762277cfb) )
	ROM_LOAD64_WORD( "mob11lm1_obj.15", 0x000002, 0x80000, CRC(0b0e4e9b) SHA1(cbbbde7470f96e9f93fa848371e19ebfeea7fe4d) )
	ROM_LOAD64_WORD( "mob11hm1_obj.13", 0x000004, 0x80000, CRC(6f84c287) SHA1(edccefa96d97c6f67a9cd02f70cf61385d70daae) )
	ROM_LOAD64_WORD( "mob11hh1_obj.11", 0x000006, 0x80000, CRC(fc2ebc0a) SHA1(7c61d05ae1644a2aafc2f81725658b29ce69a091) )

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "snd11sd1_snd.31", 0x000000, 0x80000, CRC(cce53e79) SHA1(970507fcef309c6c81f7e1a8e90afa64f3f6e2ae) )
	ROM_LOAD( "mob11sd2_snd.32", 0x080000, 0x80000, CRC(71ecc441) SHA1(4c94fa3a4ab872b2b841d98b73da89eaec0f46f0) )
ROM_END



ROM_START( dragsphr )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68EC020 Code */
	ROM_LOAD32_WORD_SWAP( "u2.bin", 0x00002, 0x080000, CRC(1fec9ead) SHA1(55c1420b22781ee985ce5369186a236e235c55d1) )
	ROM_LOAD32_WORD_SWAP( "u1.bin", 0x00000, 0x080000, CRC(581acba9) SHA1(157157130c009ab5c4329e4f0dad7419176ff51a) )

	ROM_REGION( 0x80000, "sndcpu", 0 ) /* 68000 sound program */
	ROM_LOAD16_WORD_SWAP( "u41_c06chex", 0x0000, 0x020000, CRC(adac17b1) SHA1(8e92dfd112f15ee0dbca215e265f479fb19d4be4) )

	ROM_REGION( 0x100000, "k056832", 0 ) // 8x8x8 tiles
	ROM_LOAD16_BYTE( "u21.bin", 0x00000, 0x080000, CRC(83fc3afe) SHA1(09cc89567b985685ed206b273915157fc46212f9) )
	ROM_LOAD16_BYTE( "u23.bin", 0x00001, 0x080000, CRC(a29a777f) SHA1(1ca37e468f31246cbcbd2e1799e5a0137d19d0b9) )

	ROM_REGION( 0x200000, "k055673", 0 ) // 16x16x8 sprites
	ROM_LOAD64_WORD( "u17.bin", 0x000000, 0x080000, CRC(9352f279) SHA1(1795df2331fde6de06b7d910d74a3fde69379943) )
	ROM_LOAD64_WORD( "u15.bin", 0x000002, 0x080000, CRC(4a7bc71a) SHA1(7b6bfc2b83ea6189a629b64cae295071b52c5fab) )
	ROM_LOAD64_WORD( "u13.bin", 0x000004, 0x080000, CRC(a4a60822) SHA1(6f49ae6b40185a0b0dc796b32cdbd048bfcbd3de) )
	ROM_LOAD64_WORD( "u11.bin", 0x000006, 0x080000, CRC(97efac6c) SHA1(e317834e3e9b32fb8a8343e58c047a427b3111f0) )

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "snd11sd1_snd.31", 0x000000, 0x80000, CRC(cce53e79) SHA1(970507fcef309c6c81f7e1a8e90afa64f3f6e2ae) ) // same as moneybnk
	/* no rom 32? missing or unused? */
ROM_END



ROM_START( ivorytsk )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68EC020 Code */
	ROM_LOAD32_WORD_SWAP( "u2_5ff4hex", 0x00002, 0x080000, CRC(0af976ba) SHA1(23dbaa6d8eaa501436aecc4f4d2875b3cf1ce4d9) )
	ROM_LOAD32_WORD_SWAP( "u1_a3d6hex", 0x00000, 0x080000, CRC(4e37c7dc) SHA1(52afb1989cb720b4757c8adb12240b493165c145) )

	ROM_REGION( 0x80000, "sndcpu", 0 ) /* 68000 sound program */
	ROM_LOAD16_WORD_SWAP( "u41_c06chex", 0x0000, 0x020000, CRC(adac17b1) SHA1(8e92dfd112f15ee0dbca215e265f479fb19d4be4) )

	ROM_REGION( 0x100000, "k056832", 0 ) // 8x8x8 tiles
	ROM_LOAD16_BYTE( "u21_ba6dhex", 0x00000, 0x080000, CRC(d14efb82) SHA1(420bf5d807d59e6d17ee113125046b979e1d12f4) )
	ROM_LOAD16_BYTE( "u23_9297hex", 0x00001, 0x080000, CRC(5e36ff5f) SHA1(9be65015217affc1e28d9ce855cd22f9cb147258) )

	ROM_REGION( 0x200000, "k055673", 0 ) // 16x16x8 sprites
	ROM_LOAD64_WORD( "u17_cof8hex", 0x000000, 0x080000, CRC(1ace8891) SHA1(91115680b50d6e31cdbac81ae439eeacb7a5f812) )
	ROM_LOAD64_WORD( "u15_8e23hex", 0x000002, 0x080000, CRC(174114cb) SHA1(3f9151e5785482aebfcb6787ddd63d32e0225ad2) )
	ROM_LOAD64_WORD( "u13_29fbhex", 0x000004, 0x080000, CRC(8f21cbb9) SHA1(a0e82e9f29f9eedabcd79a72db7187180e64a076) )
	ROM_LOAD64_WORD( "u11_17fbhex", 0x000006, 0x080000, CRC(82e8e69b) SHA1(9aab64be470b07340d4f39de04b3b790821b3ce7) )

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "snd11sd1_snd.31", 0x000000, 0x80000, CRC(cce53e79) SHA1(970507fcef309c6c81f7e1a8e90afa64f3f6e2ae) ) // same as moneybnk
	ROM_LOAD( "u32_c20fbin.hex", 0x080000, 0x080000, CRC(38a50800) SHA1(a7a70638d021a039070c9173a42095f7603b57c2) )
ROM_END



ROM_START( vikingt )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68EC020 Code */
	ROM_LOAD32_WORD_SWAP( "u2.bin", 0x00002, 0x080000, CRC(09a14cb1) SHA1(f09338b43e89cb265c136965b01625a3458f3e41) )
	ROM_LOAD32_WORD_SWAP( "u1.bin", 0x00000, 0x080000, CRC(90b07cb4) SHA1(e9eb1601956fa6f5bfa3c4c9b7fccf6eab08dc09) )

	ROM_REGION( 0x80000, "sndcpu", 0 ) /* 68000 sound program */
	ROM_LOAD16_WORD_SWAP( "u41.bin", 0x0000, 0x020000, CRC(adac17b1) SHA1(8e92dfd112f15ee0dbca215e265f479fb19d4be4) )

	ROM_REGION( 0x100000, "k056832", 0 ) // 8x8x8 tiles
	ROM_LOAD16_BYTE( "u21.bin", 0x00000, 0x080000, CRC(789d7c41) SHA1(a04b7e8c894e08e9210c630fabd878b8389ee82c) )
	ROM_LOAD16_BYTE( "u23.bin", 0x00001, 0x080000, CRC(56ba968e) SHA1(100edc40748067683172480fc2b7d48f4dc89da7) )

	ROM_REGION( 0x200000, "k055673", 0 ) // 16x16x8 sprites
	ROM_LOAD64_WORD( "u17.bin", 0x000000, 0x080000, CRC(83e7f568) SHA1(0f82eadb3badb7074338099ff9f4d73216a1d5c7) )
	ROM_LOAD64_WORD( "u15.bin", 0x000002, 0x080000, CRC(f349b72b) SHA1(d8abc42bbc607e36004a76e45dd88b581db60d09) )
	ROM_LOAD64_WORD( "u13.bin", 0x000004, 0x080000, CRC(2cbda923) SHA1(888b3ef9fe91843b59b03b9dabc3fd32fb7fac20) )
	ROM_LOAD64_WORD( "u11.bin", 0x000006, 0x080000, CRC(c0bf7510) SHA1(aa0a6d8109452ddf6915a9bd33b7cbb5fbda2386) )

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "snd12sd1_snd.31", 0x000000, 0x80000, CRC(f4121baa) SHA1(723c6d96ecef5ef510d085f443d44bad07aa19e5) ) // same as King Tut
	ROM_LOAD( "u32.bin",         0x080000, 0x080000, CRC(b162ecc7) SHA1(2d1bcbe692a579ed4b582472228021839fd5dab0) )
ROM_END



ROM_START( thequest ) // all SUMs on labels match but for ROM position 17 which has SUM 3949 on label but dump has 5dcb. However it passes the boot checks.
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "qstn2a20.l.2", 0x00002, 0x080000, CRC(2ae75e07) SHA1(5137ecde0fba3011854a34642b6f7dc8a2464612) ) // 1st and 2nd half identical, but SUM matches and passes boot check
	ROM_LOAD32_WORD_SWAP( "qstn2a20.h.1", 0x00000, 0x080000, CRC(03121033) SHA1(fe8ad20a35eb604c8a4c280e66d8d77b3539c7a7) ) // 1st and 2nd half identical, but SUM matches and passes boot check

	ROM_REGION( 0x80000, "sndcpu", 0 ) // BADADDR       --xxxxxxxxxxxxxxx, but SUM matches and passes boot check
	ROM_LOAD16_WORD_SWAP( "n12prog.ifu.41", 0x0000, 0x020000, CRC(adac17b1) SHA1(8e92dfd112f15ee0dbca215e265f479fb19d4be4) ) // same as dragsphr, horses4c, ivorytsk, vikingt

	ROM_REGION( 0x100000, "k056832", 0 ) // 8x8x8 tiles
	ROM_LOAD16_BYTE( "qst11_l1.vrm.21", 0x00000, 0x080000, CRC(bc336662) SHA1(145fc5725bbdce1f25f276221edf52324684876c) )
	ROM_LOAD16_BYTE( "qst11_h1.vrm.23", 0x00001, 0x080000, CRC(cca0bb57) SHA1(291fd4bb87ca3417da2dd5eb987f836420d75683) )

	ROM_REGION( 0x200000, "k055673", 0 ) // 16x16x8 sprites
	ROM_LOAD64_WORD( "qst11ll1.obj.17", 0x000000, 0x080000, CRC(bc2b43be) SHA1(dfa48d42d3d80849d6c3dad8e153fee650a32b1a) )
	ROM_LOAD64_WORD( "qst11lm1.obj.15", 0x000002, 0x080000, CRC(0e036d7b) SHA1(c39ae7180d7e5562cb0305ce551425dbabd42429) )
	ROM_LOAD64_WORD( "qst11hm1.obj.13", 0x000004, 0x080000, CRC(3e3a490c) SHA1(1f3c25bbca0d0ae1f7ce8c47d3f75e311fc68e22) )
	ROM_LOAD64_WORD( "qst11hh1.obj.11", 0x000006, 0x080000, CRC(d9fab8e3) SHA1(8912278c3fb38cdb7b9f3d1ee57cd890dca23d9a) )

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "snd11sd1.snd.31", 0x000000, 0x80000, CRC(cce53e79) SHA1(970507fcef309c6c81f7e1a8e90afa64f3f6e2ae) ) // same as dragsphr, horses4c, ivorytsk, moneybnk
	ROM_LOAD( "qst11sd2.snd.32", 0x080000, 0x80000, CRC(f4dd8f52) SHA1(1bf3358fcbe2b30064be0d232d57dde19792199f) )
ROM_END



ROM_START( horses4c ) // no labels available
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "l.2", 0x00002, 0x080000, CRC(b0db53e7) SHA1(54b262fc963140325283b22ba35b7966ea8c3677) ) // 1st and 2nd half identical, but passes boot check
	ROM_LOAD32_WORD_SWAP( "h.1", 0x00000, 0x080000, CRC(deade3a1) SHA1(bdf4a1259528f0afa70b543fea432975985bae6e) ) // 1st and 2nd half identical, but passes boot check

	ROM_REGION( 0x80000, "sndcpu", 0 ) // BADADDR       --xxxxxxxxxxxxxxx, but passes boot check
	ROM_LOAD16_WORD_SWAP( "n12prog.ifu.41", 0x0000, 0x020000, CRC(adac17b1) SHA1(8e92dfd112f15ee0dbca215e265f479fb19d4be4) ) // same as dragsphr, horses4c, ivorytsk, vikingt

	ROM_REGION( 0x100000, "k056832", 0 ) // 8x8x8 tiles
	ROM_LOAD16_BYTE( "vrm.21", 0x00000, 0x080000, CRC(8e3acb65) SHA1(d7e90f1d4653a1590dc283f7012a7aa4d3e6e07f) )
	ROM_LOAD16_BYTE( "vrm.23", 0x00001, 0x080000, CRC(05c1845c) SHA1(1a3036e9cb961fdd771782c83c1ed5fab5857b6c) )

	ROM_REGION( 0x400000, "k055673", 0 ) // 16x16x8 sprites
	ROM_LOAD64_WORD( "obj.17", 0x000000, 0x080000, CRC(2c8a45e8) SHA1(dc6bb416da67abdc0d0b24ea548207fc53fb4d54) )
	ROM_LOAD64_WORD( "obj.15", 0x000002, 0x080000, CRC(a99c7a48) SHA1(298f67d5ceedf03152ebb92ae88d44792a9e7fcc) )
	ROM_LOAD64_WORD( "obj.13", 0x000004, 0x080000, CRC(ca11684d) SHA1(7c93118fcc7e44a7d927f12c0b04f6d169fe8f86) )
	ROM_LOAD64_WORD( "obj.11", 0x000006, 0x080000, CRC(97356ece) SHA1(ad45e6a8aae9683e06421e8a6bf076008abca86a) )
	ROM_LOAD64_WORD( "obj.18", 0x200000, 0x080000, CRC(b0717c2e) SHA1(5e49969b86e2a0784ac3e71d8bc62e5d9536f79b) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF, but passes boot check
	ROM_LOAD64_WORD( "obj.16", 0x200002, 0x080000, CRC(e3536f44) SHA1(da676c5d3753b49c5827dca05450b6e067081148) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF, but passes boot check
	ROM_LOAD64_WORD( "obj.14", 0x200004, 0x080000, CRC(993e09ed) SHA1(40d38444e5896814df2096ead9507bdfe9961ca6) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF, but passes boot check
	ROM_LOAD64_WORD( "obj.12", 0x200006, 0x080000, CRC(dbb7e613) SHA1(5ae0b0a756b997318c7fc60a161ce2c55ef04a4f) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF, but passes boot check

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "snd11sd1.snd.31", 0x000000, 0x80000, CRC(cce53e79) SHA1(970507fcef309c6c81f7e1a8e90afa64f3f6e2ae) ) // same as dragsphr, horses4c, ivorytsk, moneybnk
	ROM_LOAD( "sd2.snd.32",      0x080000, 0x80000, CRC(bacb43cb) SHA1(04519a811d3d997bd126be6a5eba375c7b285df0) )
ROM_END


void kongambl_state::init_kingtut()
{
	//uint32_t *rom = (uint32_t*)memregion("maincpu")->base();

	//rom[0x3986c/4] = (rom[0x3986c/4] & 0xffff0000) | 0x600e; // patch ROM check
	//rom[0x2bfc8/4] = (rom[0x2bfc8/4] & 0xffff0000) | 0x6612; // patch VRAM ROM checks
	//rom[0x2acd0/4] = (rom[0x2acd0/4] & 0xffff) | 0x6612<<16; // patch OBJ ROM checks
	//rom[0x55e40/4] = (rom[0x55e40/4] & 0xffff0000) | 0x4e71; // goes away from the POST
}

} // Anonymous namespace


GAME( 199?, kingtut,    0,        kongambl,    kongambl, kongambl_state, init_kingtut, ROT0,  "Konami", "King Tut (NSW, Australia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, moneybnk,   0,        kongambl,    kongambl, kongambl_state, empty_init,   ROT0,  "Konami", "Money In The Bank (NSW, Australia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, dragsphr,   0,        kongambl,    kongambl, kongambl_state, empty_init,   ROT0,  "Konami", "Dragon Sphere", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, ivorytsk,   0,        kongambl,    kongambl, kongambl_state, empty_init,   ROT0,  "Konami", "Ivory Tusk", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, vikingt,    0,        kongambl,    kongambl, kongambl_state, empty_init,   ROT0,  "Konami", "Viking Treasure", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, thequest,   0,        kongambl,    kongambl, kongambl_state, empty_init,   ROT0,  "Konami", "The Quest (NSW, Australia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2000, horses4c,   0,        kongambl,    kongambl, kongambl_state, empty_init,   ROT0,  "Konami", "Horses For Courses (NSW, Australia)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
