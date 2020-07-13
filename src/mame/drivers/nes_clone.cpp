// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    NES clones that don't fit anywhere else / Plug & Play (non-VT)
*/

#include "emu.h"
#include "cpu/m6502/n2a03.h"
#include "video/ppu2c0x.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class nes_clone_state : public driver_device
{
public:
	nes_clone_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_io0(*this, "IO0"),
		m_io1(*this, "IO1"),
		m_ppu(*this, "ppu")
	{ }

	void nes_clone(machine_config &config);
	void nes_clone_pal(machine_config &config);

	void init_nes_clone();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void sprite_dma_w(address_space &space, uint8_t data);

	virtual uint8_t in0_r();
	virtual uint8_t in1_r();
	virtual void in0_w(uint8_t data);

	void nes_clone_basemap(address_map &map);

	uint8_t* m_mainrom;
	int m_mainromsize;

	required_device<n2a03_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_ioport m_io0;
	optional_ioport m_io1;
	uint8_t m_latch0;
	uint8_t m_latch1;

private:
	required_device<ppu2c0x_device> m_ppu;

	void nes_clone_map(address_map &map);
};

class nes_clone_dnce2000_state : public nes_clone_state
{
public:
	nes_clone_dnce2000_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_clone_state(mconfig, type, tag)
	{ }
	void nes_clone_dnce2000(machine_config& config);

private:
	void nes_clone_dnce2000_map(address_map& map);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint8_t rom_r(offs_t offset);
	void bank_w(uint8_t data);
	int m_rombase;
};

class nes_clone_vtvppong_state : public nes_clone_state
{
public:
	nes_clone_vtvppong_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_clone_state(mconfig, type, tag)
	{ }
	void nes_clone_vtvppong(machine_config& config);

	void init_vtvppong();

private:
	void nes_clone_vtvppong_map(address_map& map);
};

class nes_clone_suduko_state : public nes_clone_state
{
public:
	nes_clone_suduko_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_clone_state(mconfig, type, tag)
	{ }

	void init_sudoku();

	void nes_clone_suduko(machine_config& config);

private:
	void nes_clone_suduko_map(address_map& map);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint8_t rom_r(offs_t offset);
	void bank_w(uint8_t data);
	int m_rombase;
};

void nes_clone_state::sprite_dma_w(address_space &space, uint8_t data)
{
	int source = (data & 7);
	m_ppu->spriteram_dma(space, source);
}

// Standard NES style inputs (not using bus device as there are no real NES controller ports etc. these are all-in-one units and can be custom
uint8_t nes_clone_state::in0_r()
{
	//logerror("%s: in0_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch0 & 1;
	m_latch0 >>= 1;
	return ret;
}

uint8_t nes_clone_state::in1_r()
{
	//logerror("%s: in1_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch1 & 1;
	m_latch1 >>= 1;
	return ret;
}

void nes_clone_state::in0_w(uint8_t data)
{
	//logerror("%s: in0_w %02x\n", machine().describe_context(), data);
	if (data & 0x01)
		return;

	m_latch0 = m_io0->read();
	m_latch1 = m_io1->read();
}


void nes_clone_state::nes_clone_basemap(address_map& map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));

	map(0x4016, 0x4016).rw(FUNC(nes_clone_state::in0_r), FUNC(nes_clone_state::in0_w));
	map(0x4017, 0x4017).r(FUNC(nes_clone_state::in1_r));

	map(0x4014, 0x4014).w(FUNC(nes_clone_state::sprite_dma_w));
}

void nes_clone_state::nes_clone_map(address_map& map)
{
	nes_clone_basemap(map);
	map(0x8000, 0xffff).rom();
}

static INPUT_PORTS_START( nes_clone )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY

	PORT_START("IO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
INPUT_PORTS_END


static INPUT_PORTS_START( papsudok )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Auto Play")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Exit / Menu")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Start / Enter") // this is actually 2 buttons that map to the same input

	PORT_START("IO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_PLAYER(1) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_PLAYER(1) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_PLAYER(1) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
INPUT_PORTS_END


static INPUT_PORTS_START( dnce2000 )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Down-Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Down-Right")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1) PORT_NAME("Up-Left / Select")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1) PORT_NAME("Up-Right / Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_16WAY // NOT A JOYSTICK!!
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_16WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_16WAY

	PORT_START("IO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void nes_clone_state::video_start()
{
}

void nes_clone_state::machine_reset()
{
	m_latch0 = 0;
	m_latch1 = 0;
}


void nes_clone_state::machine_start()
{
	m_mainrom = memregion("maincpu")->base();
	m_mainromsize = memregion("maincpu")->bytes();

	save_item(NAME(m_latch0));
	save_item(NAME(m_latch1));
}

//  m_nt_ram = std::make_unique<uint8_t[]>(0x1000);
//  m_nt_page[0] = m_nt_ram.get();
//  m_nt_page[1] = m_nt_ram.get() + 0x400;
//  m_nt_page[2] = m_nt_ram.get() + 0x800;
//  m_nt_page[3] = m_nt_ram.get() + 0xc00;

//  m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8_delegate(*this, FUNC(nes_clone_state::nes_clone_nt_r)), write8_delegate(*this, FUNC(nes_clone_state::nes_clone_nt_w)));
//  m_ppu->space(AS_PROGRAM).install_read_bank(0x0000, 0x1fff, "bank1");
//  membank("bank1")->set_base(memregion("gfx1")->base());

void nes_clone_state::nes_clone(machine_config &config)
{
	/* basic machine hardware */
	N2A03(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_clone_state::nes_clone_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.0988);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(NTSC_APU_CLOCK.dvalue()/1000000)) * (ppu2c0x_device::VBLANK_LAST_SCANLINE_NTSC-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(m_ppu, FUNC(ppu2c0x_device::screen_update));

	PPU_2C02(config, m_ppu);
	m_ppu->set_cpu_tag("maincpu");
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void nes_clone_state::nes_clone_pal(machine_config &config)
{
	/* basic machine hardware */
	N2A03(config, m_maincpu, PALC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_clone_state::nes_clone_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66 / (PALC_APU_CLOCK.dvalue() / 1000000)) * (ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL - ppu2c0x_device::VBLANK_FIRST_SCANLINE_PALC + 1 + 2)));
	m_screen->set_size(32 * 8, 312);
	m_screen->set_visarea(0 * 8, 32 * 8 - 1, 0 * 8, 30 * 8 - 1);
	m_screen->set_screen_update(m_ppu, FUNC(ppu2c0x_device::screen_update));

	PPU_PALC(config, m_ppu);
	m_ppu->set_cpu_tag("maincpu");
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void nes_clone_dnce2000_state::nes_clone_dnce2000(machine_config& config)
{
	nes_clone_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_clone_dnce2000_state::nes_clone_dnce2000_map);
}

void nes_clone_dnce2000_state::nes_clone_dnce2000_map(address_map& map)
{
	nes_clone_basemap(map);
	map(0x8000, 0xffff).rw(FUNC(nes_clone_dnce2000_state::rom_r), FUNC(nes_clone_dnce2000_state::bank_w));
}

void nes_clone_dnce2000_state::machine_reset()
{
	nes_clone_state::machine_reset();
	m_rombase = 0;
}

void nes_clone_dnce2000_state::machine_start()
{
	nes_clone_state::machine_start();
	save_item(NAME(m_rombase));
}

uint8_t nes_clone_dnce2000_state::rom_r(offs_t offset)
{
	return m_mainrom[(offset + (m_rombase * 0x8000)) & (m_mainromsize - 1)];
}

void nes_clone_dnce2000_state::bank_w(uint8_t data)
{
	m_rombase = data;
}

void nes_clone_vtvppong_state::nes_clone_vtvppong(machine_config& config)
{
	nes_clone_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_clone_vtvppong_state::nes_clone_vtvppong_map);
}

void nes_clone_vtvppong_state::nes_clone_vtvppong_map(address_map& map)
{
	nes_clone_basemap(map);
	map(0x8000, 0xffff).rom().region("maincpu", 0x38000);
}


void nes_clone_suduko_state::machine_reset()
{
	nes_clone_state::machine_reset();
	m_rombase = 0;
}

void nes_clone_suduko_state::machine_start()
{
	nes_clone_state::machine_start();
	save_item(NAME(m_rombase));
}

void nes_clone_suduko_state::nes_clone_suduko(machine_config& config)
{
	nes_clone(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_clone_suduko_state::nes_clone_suduko_map);
}

void nes_clone_suduko_state::nes_clone_suduko_map(address_map& map)
{
	nes_clone_basemap(map);
	map(0x8000, 0xffff).rw(FUNC(nes_clone_suduko_state::rom_r), FUNC(nes_clone_suduko_state::bank_w));
}

uint8_t nes_clone_suduko_state::rom_r(offs_t offset)
{
	return m_mainrom[(offset + (m_rombase * 0x8000)) & (m_mainromsize - 1)];
}

void nes_clone_suduko_state::bank_w(uint8_t data)
{
	logerror("%04x: bank_w %02x\n", machine().describe_context(), data);
	m_rombase = data;
}


void nes_clone_suduko_state::init_sudoku()
{
	u8 *src = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();

	std::vector<u8> buffer(len);
	{
		for (int i = 0; i < len; i += 8)
		{
			buffer[i+0] = src[i+5];
			buffer[i+1] = src[i+4];
			buffer[i+2] = src[i+7];
			buffer[i+3] = src[i+6];
			buffer[i+4] = src[i+1];
			buffer[i+5] = src[i+0];
			buffer[i+6] = src[i+3];
			buffer[i+7] = src[i+2];
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}


}

void nes_clone_vtvppong_state::init_vtvppong()
{
	u8 *src = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();

	std::vector<u8> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			int newaddr = bitswap<18>(i, 17, 16, 15, 13, 14, 12, 	
				11, 10, 9, 8, 
				7, 6, 5, 4, 		
				3, 2, 1, 0);
		
			buffer[i] = src[newaddr];
		}
		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}

	if (1)
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine().system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(&src[0], len, 1, fp);
			fclose(fp);
		}
	}

}


ROM_START( papsudok )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sudoku2.bin", 0x00000, 0x80000, CRC(d1ffcc1e) SHA1(2010e60933a08d0b9271ef37f338758aacba6d2d) )
ROM_END

ROM_START( pjoypj001 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "powerjoy_pj001_lh28f008sc_89a6.bin", 0x00000, 0x100000, CRC(e655e0aa) SHA1(c96d3422e26451c366fee2151fedccb95014cbc7) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "powerjoy_pj001_te28f400ceb_00894471.bin", 0x00000, 0x80000, CRC(edca9b66) SHA1(f2f6d9043f524748282065b2fa0ca323ddd7d008) )
ROM_END

ROM_START( afbm7800 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "atariflashbackmini7800.bin", 0x00000, 0x100000, CRC(da4d9483) SHA1(c04465ff5bd5ca7abf088fe771b8e71c157afb89) )
ROM_END

ROM_START( dnce2000 ) // use Mapper 241 if you want to run this in a NES emulator
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "dance.bin", 0x00000, 0x40000, CRC(0982bb50) SHA1(bd608159d7e624ea345f2a188de51cb1aa116421) )
ROM_END

ROM_START( vtvppong )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 ) // high bit is never set in the first 0x28000 bytes of this ROM, probably 7-bit sound data? code might need opcode bits swapping
	ROM_LOAD( "vtvpongcpu.bin", 0x00000, 0x40000, CRC(52df95fa) SHA1(3015bcc90eee862b3568f122b402c9defa566aab) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "vtvpongppu.bin", 0x00000, 0x20000, CRC(474dfc0c) SHA1(4d0afab111e40172ae0b31e94f1b74b73a18385f) )
ROM_END

void nes_clone_state::init_nes_clone()
{
}

CONS( 200?, pjoypj001, 0, 0, nes_clone, nes_clone, nes_clone_state, init_nes_clone, "Trump Grand", "PowerJoy (PJ001, NES based plug & play)", MACHINE_NOT_WORKING )

// "Flashback Mini 7800 uses normal NES-style hardware, together with a mapper chipset similar to the Waixing kk33xx cartridges (NES 2.0 Mapper 534)"
CONS( 2004, afbm7800,  0,  0,  nes_clone,    nes_clone, nes_clone_state, init_nes_clone, "Atari", "Atari Flashback Mini 7800", MACHINE_NOT_WORKING )

CONS( 200?, vtvppong,  0,  0,  nes_clone_vtvppong,    nes_clone, nes_clone_vtvppong_state, init_vtvppong, "<unknown>", "Virtual TV Ping Pong", MACHINE_NOT_WORKING )


CONS( 200?, dnce2000, 0, 0, nes_clone_dnce2000, dnce2000, nes_clone_dnce2000_state, init_nes_clone, "Shenzhen Soyin Electric Appliance Ind. Co., Ltd.", "Dance 2000 / Hot 2000 (Jin Bao TV Dancing Carpet, SY-2000-04)", 0 )

// Black pad marked 'SUDOKU' with tails on the S and U characters looping over the logo.  Box says "Plug and Play Sudoku"
// Has 2 sets of 4 buttons in circular 'direction pad' layouts (on the left for directions, on the right for functions) and 9 red numbered buttons with red power LED on left of them, and reset button on right
// Alt. version was released with 'New York Times' titlescreen
CONS( 200?, papsudok,     0,  0,  nes_clone_suduko, papsudok, nes_clone_suduko_state, init_sudoku, "Nice Code", "Plug and Play Sudoku Game (NES based)", 0 ) // plays, but unclear how 'save' feature is meant to work, is it meant to save after shutdown or not? no obvious writes
