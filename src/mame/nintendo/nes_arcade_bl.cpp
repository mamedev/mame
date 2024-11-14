// license:BSD-3-Clause
// copyright-holders:

/*
  Bootleg PCB with 2 sub PCBs.

  Main PCB components:
  1x Z80 (timer CPU?)
  1x ROM
  1x RP2A03E (NTSC NES main CPU)
  1x RP2C02E-0 (NES PPU)
  3x GM76C28-10 RAMs
  1x 8-dip bank
  various TTL

  ROM PCB components:
  6x ROMs
  1x UM6264 RAM
  1x PAL
  various TTL

  NTSC PCB components:
  1x NEC uPC1352C chrominance and luminance processor for NTSC color TV
  1x 3'579'545 XTAL

  The Z80 ROM is really strange:
  * 1st half is 0xff filled;
  * 2nd half contains 8x 0x800 programs
  * 6 of them are identical
  * 2 others (0x4000 - 0x47ff and 0x6000 - 0x67ff) differ but are identical between themselves

  SMB3 bootleg ROMs appear to be hacks of the first Japanese release. CHR data
  is identical. PRG data differs by only 46 bytes from the mapper 106 bootleg,
  which similarly uses TTL in place of an MMC3. The mapper 106 bootleg adds a
  Chinese title screen and gives the player 20 lives, suggesting it is a hack
  of this original bootleg?

  TODO:
  - machine is NOT WORKING since Z80 side isn't implemented
  - coins, DIPs, etc
*/


#include "emu.h"

#include "cpu/m6502/rp2a03.h"
#include "cpu/z80/z80.h"
#include "video/ppu2c0x.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class nes_arcade_bl_state : public driver_device
{
public:
	nes_arcade_bl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppu(*this, "ppu"),
		m_in(*this, "IN%u", 0U),
		m_nt_page(*this, "nt_page%u", 0U),
		m_prg_bank(*this, "prg%u", 0U),
		m_chr_bank(*this, "chr%u", 0U)
	{ }

	void smb3bl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<rp2a03_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;
	required_ioport_array<2> m_in;
	required_memory_bank_array<4> m_nt_page;
	required_memory_bank_array<4> m_prg_bank;
	required_memory_bank_array<8> m_chr_bank;

	std::unique_ptr<u8[]> m_nt_ram;
	u8 m_prg_chunks;
	u8 m_input_latch[2];
	u8 m_input_strobe;
	u16 m_irq_count;
	bool m_irq_enable;
	emu_timer* m_irq_timer;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);
	template <u8 Which> u8 in_r();
	void in0_w(u8 data);
	void set_mirroring(int mirroring);
	void reg_w(offs_t offset, u8 data);
	void nes_cpu_map(address_map &map) ATTR_COLD;
	void nes_ppu_map(address_map &map) ATTR_COLD;
	void timer_prg_map(address_map &map) ATTR_COLD;
	void timer_io_map(address_map &map) ATTR_COLD;
};


template <u8 Which>
u8 nes_arcade_bl_state::in_r()
{
	if (m_input_strobe)
		m_input_latch[Which] = m_in[Which]->read();

	u8 ret = 0x40; // open bus - fake it til ya make it

	ret |= m_input_latch[Which] & 1;
	m_input_latch[Which] >>= 1;

	return ret;
}

void nes_arcade_bl_state::in0_w(u8 data)
{
	if (m_input_strobe & ~data & 1)
		for (int i = 0; i < 2; i++)
			m_input_latch[i] = m_in[i]->read();

	m_input_strobe = data & 1;
}

void nes_arcade_bl_state::set_mirroring(int mirroring)
{
	int bit = mirroring == PPU_MIRROR_HORZ;

	for (int i = 0; i < 4; i++)
		m_nt_page[i]->set_entry(BIT(i, bit));
}

void nes_arcade_bl_state::reg_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
			m_chr_bank[offset & 0x07]->set_entry((data & 0x7e) | (offset & 1));
			break;
		case 0x04: case 0x05: case 0x06: case 0x07:
			m_chr_bank[offset & 0x07]->set_entry(data & 0x7f);
			break;
		case 0x08:
		case 0x0b:
			m_prg_bank[offset & 0x03]->set_entry((data & 0x0f) | 0x10);
			break;
		case 0x09:
		case 0x0a:
			m_prg_bank[offset & 0x03]->set_entry(data & 0x1f);
			break;
		case 0x0c:
			set_mirroring(data & 1 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
		case 0x0d:
			m_irq_count = 0;
			m_irq_enable = false;
			m_maincpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);
			break;
		case 0x0e:
			m_irq_count = (m_irq_count & 0xff00) | data;
			break;
		case 0x0f:
			m_irq_count = (m_irq_count & 0x00ff) | (data << 8);
			m_irq_enable = true;
			break;
	}
}

TIMER_CALLBACK_MEMBER(nes_arcade_bl_state::irq_timer_tick)
{
	// counter does not stop when interrupts are disabled
	if (m_irq_count != 0xffff)
		m_irq_count++;
	if (m_irq_enable && m_irq_count == 0xffff)
		m_maincpu->set_input_line(m6502_device::IRQ_LINE, ASSERT_LINE);
}


void nes_arcade_bl_state::nes_cpu_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x4014, 0x4014).w(m_ppu, FUNC(ppu2c0x_device::spriteram_dma));
	map(0x4016, 0x4016).rw(FUNC(nes_arcade_bl_state::in_r<0>), FUNC(nes_arcade_bl_state::in0_w)); // IN0 - input port 1
	map(0x4017, 0x4017).r(FUNC(nes_arcade_bl_state::in_r<1>));     // IN1 - input port 2 / PSG second control register

	map(0x6000, 0x7fff).ram();
	map(0x8000, 0x800f).mirror(0x7ff0).w(FUNC(nes_arcade_bl_state::reg_w));
	map(0x8000, 0x9fff).bankr(m_prg_bank[0]);
	map(0xa000, 0xbfff).bankr(m_prg_bank[1]);
	map(0xc000, 0xdfff).bankr(m_prg_bank[2]);
	map(0xe000, 0xffff).bankr(m_prg_bank[3]);
}

void nes_arcade_bl_state::nes_ppu_map(address_map &map)
{
	map(0x0000, 0x03ff).bankr(m_chr_bank[0]);
	map(0x0400, 0x07ff).bankr(m_chr_bank[1]);
	map(0x0800, 0x0bff).bankr(m_chr_bank[2]);
	map(0x0c00, 0x0fff).bankr(m_chr_bank[3]);
	map(0x1000, 0x13ff).bankr(m_chr_bank[4]);
	map(0x1400, 0x17ff).bankr(m_chr_bank[5]);
	map(0x1800, 0x1bff).bankr(m_chr_bank[6]);
	map(0x1c00, 0x1fff).bankr(m_chr_bank[7]);
	map(0x2000, 0x23ff).mirror(0x1000).bankrw(m_nt_page[0]);
	map(0x2400, 0x27ff).mirror(0x1000).bankrw(m_nt_page[1]);
	map(0x2800, 0x2bff).mirror(0x1000).bankrw(m_nt_page[2]);
	map(0x2c00, 0x2fff).mirror(0x1000).bankrw(m_nt_page[3]);
	map(0x3f00, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
}

void nes_arcade_bl_state::timer_prg_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x8000, 0x87ff).ram();
}

void nes_arcade_bl_state::timer_io_map(address_map &map)
{
}


static INPUT_PORTS_START( smb3bl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)    // Select
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)    // Select
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


void nes_arcade_bl_state::machine_start()
{
	m_nt_ram = std::make_unique<u8[]>(0x800);
	for (auto &page : m_nt_page)
		page->configure_entries(0, 2, m_nt_ram.get(), 0x400);

	m_prg_chunks = memregion("maincpu")->bytes() / 0x2000;
	u8 *base = memregion("maincpu")->base();
	for (auto &bank : m_prg_bank)
		bank->configure_entries(0, m_prg_chunks, base, 0x2000);

	int chr_chunks = memregion("gfx")->bytes() / 0x400;
	base = memregion("gfx")->base();
	for (auto &bank : m_chr_bank)
		bank->configure_entries(0, chr_chunks, base, 0x400);

	m_irq_timer = timer_alloc(FUNC(nes_arcade_bl_state::irq_timer_tick), this);
	m_irq_timer->adjust(attotime::zero, 0, m_maincpu->clocks_to_attotime(1));
}

void nes_arcade_bl_state::machine_reset()
{
	for (int i = 0; i < 4; i++)
		m_prg_bank[i]->set_entry(m_prg_chunks - 4 + i); // last 32K

	for (int i = 0; i < 8; i++)
		m_chr_bank[i]->set_entry(i);

	set_mirroring(PPU_MIRROR_VERT);
	m_input_latch[0] = 0;
	m_input_latch[1] = 0;
	m_input_strobe = 0;
	m_irq_count = 0;
	m_irq_enable = false;
}

void nes_arcade_bl_state::smb3bl(machine_config &config)
{
	RP2A03G(config, m_maincpu, 3.579545_MHz_XTAL / 2); // TODO: verify divider, really RP2A03E
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_arcade_bl_state::nes_cpu_map);

	z80_device &timercpu(Z80(config, "timercpu", 3.579545_MHz_XTAL));
	timercpu.set_addrmap(AS_PROGRAM, &nes_arcade_bl_state::timer_prg_map);
	timercpu.set_addrmap(AS_IO, &nes_arcade_bl_state::timer_io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(32*8, 262);
	screen.set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	screen.set_screen_update(m_ppu, FUNC(ppu2c0x_device::screen_update));

	PPU_2C02(config, m_ppu);
	m_ppu->set_addrmap(0, &nes_arcade_bl_state::nes_ppu_map);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( smb3bl )
	ROM_REGION( 0x40000, "maincpu", 0 ) // extremely similar to smb3h in nes.xml once split
	ROM_LOAD( "mario_3-1.bin", 0x00000, 0x10000, CRC(d28e54ee) SHA1(25b842aa091dd3e497b1ff34ef8cf4cda5025dc5) ) // 5601.rom-2 [1/2]      IDENTICAL
	ROM_LOAD( "mario_3-2.bin", 0x10000, 0x10000, CRC(1ed05c8d) SHA1(f78cfc827b5cde86f2aa4f9a9df82b923835a7a6) ) // 5601.rom-2 [2/2]      IDENTICAL
	ROM_LOAD( "mario_3-3.bin", 0x20000, 0x10000, CRC(0b2f4356) SHA1(bffb329b9d7387ededf779cf40c84906fc26cf05) ) // 5602.rom-1 [1/2]      IDENTICAL
	ROM_LOAD( "mario_3-4.bin", 0x30000, 0x10000, CRC(2abda7cc) SHA1(6e6c9b1f28a0d6eb7dc43dfeca1da458b7ddb89e) ) // 5602.rom-1 [2/2]      99.929810% (changes seem legit / not a result of a bad dump)

	ROM_REGION( 0x20000, "gfx", 0 ) // matches smb3j in nes.xml once split
	ROM_LOAD( "mario_3-5.bin", 0x00000, 0x10000, CRC(48d6ddce) SHA1(686793fcb9c3ba9d7280b40c9afdbd75860a290a) ) // hvc-um-0 chr [1/2]      IDENTICAL
	ROM_LOAD( "mario_3-6.bin", 0x10000, 0x10000, CRC(a88664e0) SHA1(327d246f198713f20adc7764ee539d18eb0b82ad) ) // hvc-um-0 chr [2/2]      IDENTICAL

	ROM_REGION( 0x8000, "timercpu", 0 )
	/* 2KiB content repeated 8 times to fill the upper half of the ROM (lower half is empty).
	   Also seen using a 27C128 (16K*8) with the 2KiB content repeated 8 times to fill the entire ROM, with CRC(986fb6b3) and
	   SHA1(fa205601adf15947bb073afd5fbd57cd971bff7d) */
	ROM_LOAD( "nes_jamma_base.bin", 0x0000, 0x4000, CRC(ea276bdd) SHA1(1cd5916e9a6ea9e40526a4fe55b846ca1818fd5f) ) // BADADDR x-xxxxxxxxxxxxx
	ROM_CONTINUE(                   0x0000, 0x4000 )
ROM_END

} // anonymous namespace


GAME( 1987, smb3bl, 0, smb3bl, smb3bl, nes_arcade_bl_state, empty_init, ROT0, "Sang Ho Soft", "Super Mario Bros. 3 (NES bootleg)", MACHINE_NOT_WORKING ) // 1987.10.01 in Z80 ROM
