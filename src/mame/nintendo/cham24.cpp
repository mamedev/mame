// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*
    Chameleon 24

    driver by Mariusz Wojcieszek
    uses NES emulation by Brad Olivier

    Notes:
    - NES hardware is probably implemented on FPGA
    - Atmel mcu probably controls coins and timer - since these are not emulated
      game is marked as 'not working'
    - 72-in-1 mapper 225 (found on NES pirate carts) is used for bank switching
    - code at 0x0f8000 in 24-2.u2 contains English version of menu, code at 0x0fc000 contains
    other version (Asian language), is this controlled by mcu?

PCB is small and newly manufactured. There's 24 games which can be chosen
from a text menu after coin-up.
The games appear to be old NES games (i.e. very poor quality for an arcade product)
Screenshots can be found here....
http://www.coinopexpress.com/products/pcbs/pcb/Chameleon_24_2839.html

PCB Layout
----------


|------------------------------------|
|       LM380    --------            |
|                |NTA0002|           |
|                |(QFP80)|   24-1.U1 |
|                --------            |
|  ULN2003A    -----------           |
|              |LATTICE  |           |
|      DSW1    |PLSI 1016|           |
|J             |(PLCC44) |  24-2.U2  |
|A    AT89C51  -----------           |
|M                                   |
|M    SW1   21.4771MHz               |
|A                                   |
| GW6582  LS02                       |
|          |-----------| MC14040BCP  |
|  74HC245 |Philips    | MC14040BCP  |
|          |SAA71111AH2|             |
|          |20505650   |             |
|          |bP0219     | 24-3.U3     |
| 24.576MHz|-----------|             |
|             (QFP64)                |
|------------------------------------|

Notes:
       All components are listed.
       DSW1 has 2 switches only
       SW1 is a push button switch
       U1 is 27C040 EPROM
       U2 is 27C080 EPROM
       U3 is 27C512 EPROM
*/

#include "emu.h"

#include "cpu/m6502/rp2a03.h"
#include "video/ppu2c0x.h"

#include "screen.h"
#include "speaker.h"


namespace {

class cham24_state : public driver_device
{
public:
	cham24_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppu(*this, "ppu")
		, m_nt_page(*this, "nt_page%u", 0U)
		, m_prg_banks(*this, "prg%u", 0U)
		, m_chr_bank(*this, "chr")
		, m_in(*this, "P%u", 1U)
	{ }

	void cham24(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<rp2a03_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;

	required_memory_bank_array<4> m_nt_page;
	required_memory_bank_array<2> m_prg_banks;
	required_memory_bank m_chr_bank;

	required_ioport_array<2> m_in;

	std::unique_ptr<u8 []> m_nt_ram;
	u8 m_prg_chunks;
	u8 m_input_latch[2];
	u8 m_input_strobe;

	template <u8 Which> u8 cham24_in_r();
	void cham24_in0_w(u8 data);
	void cham24_mapper_w(offs_t offset, u8 data);
	void cham24_set_mirroring(int mirroring);
	void cham24_map(address_map &map) ATTR_COLD;
	void cham24_ppu_map(address_map &map) ATTR_COLD;
};



void cham24_state::cham24_set_mirroring(int mirroring)
{
	int bit = mirroring == PPU_MIRROR_HORZ;

	for (int i = 0; i < 4; i++)
		m_nt_page[i]->set_entry(BIT(i, bit));
}

template <u8 Which>
u8 cham24_state::cham24_in_r()
{
	if (m_input_strobe)
		m_input_latch[Which] = m_in[Which]->read();

	u8 ret = 0x40;

	ret |= m_input_latch[Which] & 1;
	if (!machine().side_effects_disabled())
		m_input_latch[Which] >>= 1;

	return ret;
}

void cham24_state::cham24_in0_w(u8 data)
{
	if (data & 0xfe)
	{
		//logerror("Unhandled cham24_in0_w write: data = %02X\n", data);
	}

	if (m_input_strobe & ~data & 1)
		for (int i = 0; i < 2; i++)
			m_input_latch[i] = m_in[i]->read();

	m_input_strobe = data & 1;
}

void cham24_state::cham24_mapper_w(offs_t offset, u8 data)
{
	// switch PRG bank
	u8 prg_bank = BIT(offset, 6, 6);
	u8 prg_mode = !BIT(offset, 12);
	m_prg_banks[0]->set_entry(prg_bank & ~prg_mode);
	m_prg_banks[1]->set_entry(prg_bank | prg_mode);

	// switch PPU VROM bank
	m_chr_bank->set_entry(offset & 0x3f);

	// set gfx mirroring
	cham24_set_mirroring(BIT(offset, 13) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}

void cham24_state::cham24_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram(); // NES RAM
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x4014, 0x4014).w(m_ppu, FUNC(ppu2c0x_device::spriteram_dma));
	map(0x4016, 0x4016).rw(FUNC(cham24_state::cham24_in_r<0>), FUNC(cham24_state::cham24_in0_w));            // IN0 - input port 1
	map(0x4017, 0x4017).r(FUNC(cham24_state::cham24_in_r<1>));    // IN1 - input port 2 / PSG second control register
	map(0x8000, 0xbfff).bankr(m_prg_banks[0]).w(FUNC(cham24_state::cham24_mapper_w));
	map(0xc000, 0xffff).bankr(m_prg_banks[1]).w(FUNC(cham24_state::cham24_mapper_w));
}

void cham24_state::cham24_ppu_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr(m_chr_bank);
	map(0x2000, 0x23ff).mirror(0x1000).bankrw(m_nt_page[0]);
	map(0x2400, 0x27ff).mirror(0x1000).bankrw(m_nt_page[1]);
	map(0x2800, 0x2bff).mirror(0x1000).bankrw(m_nt_page[2]);
	map(0x2c00, 0x2fff).mirror(0x1000).bankrw(m_nt_page[3]);
	map(0x3f00, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
}

static INPUT_PORTS_START( cham24 )
	PORT_START("P1") // IN0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)    // Select
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P2") // IN1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)    // Select
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
INPUT_PORTS_END


void cham24_state::machine_start()
{
	m_nt_ram = std::make_unique<u8[]>(0x800);
	for (auto &page : m_nt_page)
		page->configure_entries(0, 2, m_nt_ram.get(), 0x400);

	// set up code banking to be done in 16K chunks
	m_prg_chunks = memregion("user1")->bytes() / 0x4000;
	m_prg_banks[0]->configure_entries(0, m_prg_chunks, memregion("user1")->base(), 0x4000);
	m_prg_banks[1]->configure_entries(0, m_prg_chunks, memregion("user1")->base(), 0x4000);

	// gfx banking always done in 8K chunks
	m_chr_bank->configure_entries(0, memregion("gfx1")->bytes() / 0x2000, memregion("gfx1")->base(), 0x2000);

	save_item(NAME(m_input_latch));
	save_item(NAME(m_input_strobe));
	save_pointer(NAME(m_nt_ram), 0x800);
}

void cham24_state::machine_reset()
{
	// switch to main menu PRG and CHR
	m_prg_banks[0]->set_entry(m_prg_chunks - 2);
	m_prg_banks[1]->set_entry(m_prg_chunks - 2);
	m_chr_bank->set_entry(0);

	cham24_set_mirroring(PPU_MIRROR_VERT);
}


void cham24_state::cham24(machine_config &config)
{
	// basic machine hardware
	RP2A03G(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &cham24_state::cham24_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(32*8, 262);
	screen.set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	screen.set_screen_update(m_ppu, FUNC(ppu2c0x_device::screen_update));

	PPU_2C02(config, m_ppu);
	m_ppu->set_addrmap(0, &cham24_state::cham24_ppu_map);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START( cham24 )
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASE00)

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "at89c51", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION(0x100000, "user1", 0)
	ROM_LOAD( "24-2.u2", 0x000000, 0x100000, CRC(686e9d05) SHA1(a55b9850a4b47f1b4495710e71534ca0287b05ee) )

	ROM_REGION(0x080000, "gfx1", 0)
	ROM_LOAD( "24-1.u1", 0x000000, 0x080000, CRC(43c43d58) SHA1(3171befaca28acc80fb70226748d9abde76a1b56) )

	ROM_REGION(0x10000, "user2", 0)
	ROM_LOAD( "24-3.u3", 0x0000, 0x10000, CRC(e97955fa) SHA1(6d686c5d0967c9c2f40dbd8e6a0c0907606f2c7d) ) // unknown rom
ROM_END

} // Anonymous namespace


GAME( 2002, cham24, 0, cham24, cham24, cham24_state, empty_init, ROT0, "bootleg", "Chameleon 24", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
