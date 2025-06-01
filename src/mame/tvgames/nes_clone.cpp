// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    NES clones that don't fit anywhere else / Plug & Play (non-VT)
*/

#include "emu.h"
#include "cpu/m6502/rp2a03.h"
#include "video/ppu2c0x.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/bankdev.h"


namespace {

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
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual uint8_t in0_r();
	virtual uint8_t in1_r();
	virtual void in0_w(uint8_t data);

	void nes_clone_basemap(address_map &map) ATTR_COLD;

	uint8_t* m_mainrom;
	int m_mainromsize;

	required_device<rp2a03_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_ioport m_io0;
	optional_ioport m_io1;
	uint8_t m_latch0;
	uint8_t m_latch1;
	uint8_t m_previous_port0;

	required_device<ppu2c0x_device> m_ppu;

private:

	void nes_clone_map(address_map &map) ATTR_COLD;
};


class nes_clone_dancexpt_state : public nes_clone_state
{
public:
	nes_clone_dancexpt_state(const machine_config &mconfig, device_type type, const char *tag) :
		nes_clone_state(mconfig, type, tag),
		m_nametables(*this, "nametable%u", 0),
		m_prgrom(*this, "prgrom"),
		m_gfxrom(*this, "gfxrom"),
		m_mainrom(*this, "maincpu")
	{ }
	void nes_clone_dancexpt(machine_config &config);

private:
	void nes_clone_dancexpt_map(address_map &map) ATTR_COLD;
	memory_bank_array_creator<4> m_nametables;
	required_memory_bank m_prgrom;
	memory_bank_creator m_gfxrom;
	required_region_ptr<uint8_t> m_mainrom;

	std::unique_ptr<u8[]> m_nt_ram;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mapper_5000_w(offs_t offset, uint8_t data);
	void mapper_5100_w(offs_t offset, uint8_t data);
	void mapper_5200_w(offs_t offset, uint8_t data);

	virtual uint8_t in0_r() override;
	virtual uint8_t in1_r() override;
	virtual void in0_w(uint8_t data) override;

	void update_video_bank();

	uint8_t m_5000_val;
	uint8_t m_5100_val;
	uint8_t m_5200_val;
};


class nes_clone_dnce2000_state : public nes_clone_state
{
public:
	nes_clone_dnce2000_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_clone_state(mconfig, type, tag)
	{ }
	void nes_clone_dnce2000(machine_config& config);

private:
	void nes_clone_dnce2000_map(address_map &map) ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint8_t rom_r(offs_t offset);
	void bank_w(uint8_t data);
	uint8_t m_rombase = 0;
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
	void nes_clone_vtvppong_map(address_map &map) ATTR_COLD;
};

class nes_clone_sudoku_state : public nes_clone_state
{
public:
	nes_clone_sudoku_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_clone_state(mconfig, type, tag)
	{ }

	void init_sudoku();

	void nes_clone_sudoku(machine_config& config);

private:
	void nes_clone_sudoku_map(address_map &map) ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint8_t rom_r(offs_t offset);
	void bank_w(uint8_t data);
	uint8_t m_rombase = 0;
};

class nes_clone_vtvsocr_state : public nes_clone_state
{
public:
	nes_clone_vtvsocr_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_clone_state(mconfig, type, tag)
	{ }

	void nes_clone_vtvsocr(machine_config& config);

private:
	void nes_clone_vtvsocr_map(address_map &map) ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint8_t rom_r(offs_t offset);
	void bank_w(offs_t offset, uint8_t data);
	uint8_t m_bankregs[4];
};



class nes_clone_afbm7800_state : public nes_clone_state
{
public:
	nes_clone_afbm7800_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_clone_state(mconfig, type, tag),
		m_prgbank(*this, "prgbank%u", 0),
		m_cbank(*this, "cbank%u", 0),
		m_nametables(*this, "nametable%u", 0),
		m_charbank(*this, "charbank"),
		m_rom_solderpad_bank(*this, "rom_sldpad_bank")
	{ }
	void nes_clone_afbm7800(machine_config& config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

protected:
	// configured at startup
	uint16_t m_maxchrbank = 0;

	void nes_clone_afbm7800_map(address_map &map) ATTR_COLD;

	void mapper_6000_w(uint8_t data);
	void mapper_6001_w(uint8_t data);
	void mapper_6002_w(uint8_t data);
	void mapper_6003_w(uint8_t data);

	void mapper_8000_w(uint8_t data);
	void mapper_8001_w(uint8_t data);
	void mapper_a000_w(uint8_t data);
	void mapper_a001_w(uint8_t data);
	void mapper_c000_w(uint8_t data);
	void mapper_c001_w(uint8_t data);
	void mapper_e000_w(uint8_t data);
	void mapper_e001_w(uint8_t data);

	uint8_t solderpad_r(offs_t offset);

	uint8_t vram_r(offs_t offset);
	virtual void vram_w(offs_t offset, uint8_t data);

	uint8_t m_banksel = 0;
	uint8_t m_bankregs[8];
	uint8_t m_extraregs[4];

	void common_start();
	void handle_stdchr_banks(uint16_t* selected_chrbanks);
	virtual void handle_mmc3chr_banks(uint16_t* selected_chrbanks);
	void update_prg_banks();
	void update_banks();
	void mmc3_scanline_cb(int scanline, bool vblank, bool blanked);

	int16_t m_mmc3_scanline_counter = 0;
	uint8_t m_mmc3_scanline_latch = 0;
	uint8_t m_mmc3_irq_enable = 0;

	std::vector<u8> m_vram;

	uint8_t m_ntmirror = 0;
	uint8_t m_ramprot = 0;

	void update_nt_mirroring();
	std::vector<u8> m_nt_ram;

	void vram_map(address_map &map) ATTR_COLD;
	void ntram_map(address_map &map) ATTR_COLD;
	void romarea_map(address_map &map) ATTR_COLD;

	required_memory_bank_array<4> m_prgbank;
	required_memory_bank_array<6> m_cbank;
	memory_bank_array_creator<4> m_nametables;

	required_device<address_map_bank_device> m_charbank;
	required_device<address_map_bank_device> m_rom_solderpad_bank;
};

class nes_clone_taikee_new_state : public nes_clone_afbm7800_state
{
public:
	nes_clone_taikee_new_state(const machine_config &mconfig, device_type type, const char *tag) :
		nes_clone_afbm7800_state(mconfig, type, tag)
	{ }

protected:
	virtual void handle_mmc3chr_banks(uint16_t* selected_chrbanks) override;

	virtual void vram_w(offs_t offset, uint8_t data) override;

private:
	virtual void machine_start() override ATTR_COLD;
};

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
	if ((data & 0x01) != (m_previous_port0 & 0x01))
	{
		if (data & 0x01)
		{
			m_latch0 = m_io0->read();
			m_latch1 = m_io1->read();
		}
	}

	m_previous_port0 = data;
}


void nes_clone_state::nes_clone_basemap(address_map& map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));

	map(0x4014, 0x4014).w(m_ppu, FUNC(ppu2c0x_device::spriteram_dma));

	map(0x4016, 0x4016).rw(FUNC(nes_clone_state::in0_r), FUNC(nes_clone_state::in0_w));
	map(0x4017, 0x4017).r(FUNC(nes_clone_state::in1_r));
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



static INPUT_PORTS_START( dancexpt )
	PORT_START("IO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_16WAY // NOT A JOYSTICK!!
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_16WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)
INPUT_PORTS_END

void nes_clone_state::video_start()
{
}

void nes_clone_state::machine_reset()
{
	m_latch0 = 0;
	m_latch1 = 0;
	m_previous_port0 = 0;
}


void nes_clone_state::machine_start()
{
	m_mainrom = memregion("maincpu")->base();
	m_mainromsize = memregion("maincpu")->bytes();

	save_item(NAME(m_latch0));
	save_item(NAME(m_latch1));
}


void nes_clone_state::nes_clone(machine_config &config)
{
	/* basic machine hardware */
	RP2A03G(config, m_maincpu, NTSC_APU_CLOCK);
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
	RP2A03G(config, m_maincpu, PALC_APU_CLOCK);
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

/**************************************************
 Dancing Expert specifics
**************************************************/

void nes_clone_dancexpt_state::machine_start()
{
	nes_clone_state::machine_start();

	m_nt_ram = std::make_unique<u8[]>(0x800);

	m_prgrom->configure_entries(0, 0x08, &m_mainrom[0], 0x4000);
	m_gfxrom->configure_entries(0, 0x20, memregion("gfx1")->base(), 0x2000);

	m_ppu->space(AS_PROGRAM).install_read_bank(0x0000, 0x1fff, m_gfxrom);

	for (int i = 0; i < 4; i++)
		m_nametables[i]->configure_entries(0, 2, &m_nt_ram[0], 0x400);

	save_pointer(NAME(m_nt_ram), 0x800);

	m_nametables[0]->set_entry(0);
	m_nametables[1]->set_entry(1);
	m_nametables[2]->set_entry(0);
	m_nametables[3]->set_entry(1);

	m_ppu->space(AS_PROGRAM).install_readwrite_bank(0x2000,0x23ff,m_nametables[0]);
	m_ppu->space(AS_PROGRAM).install_readwrite_bank(0x2400,0x27ff,m_nametables[1]);
	m_ppu->space(AS_PROGRAM).install_readwrite_bank(0x2800,0x2bff,m_nametables[2]);
	m_ppu->space(AS_PROGRAM).install_readwrite_bank(0x2c00,0x2fff,m_nametables[3]);
}


uint8_t nes_clone_dancexpt_state::in0_r()
{
	// polled frequently during game, music data? from some other device?
	return 0xfe; // bit 0x01 exits the song?
}

uint8_t nes_clone_dancexpt_state::in1_r()
{
	// read directly, not through shifter here
	return m_io1->read();
}

void nes_clone_dancexpt_state::machine_reset()
{
	nes_clone_state::machine_reset();
	m_5000_val = 0x6;
}

void nes_clone_dancexpt_state::update_video_bank()
{
	int bank = (m_previous_port0 & 0x7) * 4;
	bank += m_5100_val & 0x3;

	m_gfxrom->set_entry(bank);
}

void nes_clone_dancexpt_state::mapper_5000_w(offs_t offset, uint8_t data)
{
	// xxxx -bbb  b = CPU bank at 8000-bfff  x = unknown (sometimes 2, sometimes d)

	if (data & ~0x27)
		logerror("%s: mapper_5000_w %02x\n", machine().describe_context(), data);

	m_5000_val = data;

	m_prgrom->set_entry(m_5000_val & 0x7);
}

void nes_clone_dancexpt_state::mapper_5100_w(offs_t offset, uint8_t data)
{
	// ---- --cc   c = lower character ROM bank

	if (data & 0xfc)
		logerror("%s: mapper_5100_w %02x\n", machine().describe_context(), data);

	m_5100_val = data;
	update_video_bank();
}

void nes_clone_dancexpt_state::in0_w(uint8_t data)
{
	// ---- -CCC   C = upper character ROM bank

	// instead of input related writes, the video banking is here?!
	m_previous_port0 = data;
	update_video_bank();
}

void nes_clone_dancexpt_state::mapper_5200_w(offs_t offset, uint8_t data)
{
	// ---- ----  unknown

	logerror("%s: mapper_5200_w %02x\n", machine().describe_context(), data);
	m_5200_val = data;
	//update_video_bank();
}

void nes_clone_dancexpt_state::nes_clone_dancexpt(machine_config &config)
{
	nes_clone(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_clone_dancexpt_state::nes_clone_dancexpt_map);
}

void nes_clone_dancexpt_state::nes_clone_dancexpt_map(address_map &map)
{
	nes_clone_basemap(map);

	map(0x5000, 0x5000).w(FUNC(nes_clone_dancexpt_state::mapper_5000_w));
	map(0x5100, 0x5100).w(FUNC(nes_clone_dancexpt_state::mapper_5100_w));
	map(0x5200, 0x5200).w(FUNC(nes_clone_dancexpt_state::mapper_5200_w));

	map(0x8000, 0xbfff).bankr("prgrom");
	map(0xc000, 0xffff).rom().region("maincpu", 0x1c000);
}


/**************************************************
 Dance 2000 Specifics
**************************************************/

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

/**************************************************
 Virtual Ping Pong Specifics
**************************************************/

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

/**************************************************
 Atari Flashback Specifics
**************************************************/

void nes_clone_afbm7800_state::nes_clone_afbm7800(machine_config& config)
{
	nes_clone_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_clone_afbm7800_state::nes_clone_afbm7800_map);

	ADDRESS_MAP_BANK(config, "charbank").set_map(&nes_clone_afbm7800_state::vram_map).set_options(ENDIANNESS_NATIVE, 8, 14, 0x2000);
	ADDRESS_MAP_BANK(config, "ntbank").set_map(&nes_clone_afbm7800_state::ntram_map).set_options(ENDIANNESS_NATIVE, 8, 12, 0x1000);
	ADDRESS_MAP_BANK(config, "rom_sldpad_bank").set_map(&nes_clone_afbm7800_state::romarea_map).set_options(ENDIANNESS_NATIVE, 8, 16, 0x8000);

}

void nes_clone_afbm7800_state::update_prg_banks()
{
	uint8_t innerbankmask;
	uint8_t outerbank;

	uint8_t selected_banks[4] = { 0x00, 0x00, 0x00, 0x00 };

	if (m_extraregs[1] & 1) // ROM or solder pad select
	{
		m_rom_solderpad_bank->set_bank(1);
	}
	else
	{
		m_rom_solderpad_bank->set_bank(0);
	}

	if (m_extraregs[0] & 0x40)
	{
		innerbankmask = 0x0f;
		outerbank = (m_extraregs[0] & 0x07) << 4;
	}
	else
	{
		innerbankmask = 0x1f;
		outerbank = (m_extraregs[0] & 0x06) << 4;
	}

	if ((m_extraregs[3] & 0x03) == 0x00) // MMC3 style mode
	{
		selected_banks[1] = outerbank | (m_bankregs[7] & innerbankmask);
		selected_banks[3] = outerbank | (0x1f & innerbankmask);

		if (m_banksel & 0x40)
		{
			selected_banks[0] = outerbank | (0x1e & innerbankmask);
			selected_banks[2] = outerbank | (m_bankregs[6] & innerbankmask);

		}
		else
		{
			selected_banks[0] = outerbank | (m_bankregs[6] & innerbankmask);
			selected_banks[2] = outerbank | (0x1e & innerbankmask);
		}
	}
	else if ((m_extraregs[3] & 0x03) == 0x03)
	{
		int basebank = (m_bankregs[6] & innerbankmask);

		selected_banks[0] = outerbank | (basebank + 0);
		selected_banks[1] = outerbank | (basebank + 1);
		selected_banks[2] = outerbank | (basebank + 2);
		selected_banks[3] = outerbank | (basebank + 3);
	}
	else // 01 and 02 cases
	{
		int basebank = (m_bankregs[6] & innerbankmask);

		selected_banks[0] = outerbank | (basebank + 0);
		selected_banks[1] = outerbank | (basebank + 1);
		selected_banks[2] = outerbank | (basebank + 0);
		selected_banks[3] = outerbank | (basebank + 1);
	}

	m_prgbank[0]->set_entry(selected_banks[0]);
	m_prgbank[1]->set_entry(selected_banks[1]);
	m_prgbank[2]->set_entry(selected_banks[2]);
	m_prgbank[3]->set_entry(selected_banks[3]);
}

void nes_clone_afbm7800_state::handle_stdchr_banks(uint16_t* selected_chrbanks)
{
	int outerchrbank = m_extraregs[2] & 0xf;
	m_charbank->set_bank(0);

	// should also have m_extraregs[0] & 0x38) applied?

	selected_chrbanks[0] = (outerchrbank << 3) | 0x0;
	selected_chrbanks[1] = (outerchrbank << 3) | 0x2;
	selected_chrbanks[2] = (outerchrbank << 3) | 0x4;
	selected_chrbanks[3] = (outerchrbank << 3) | 0x5;
	selected_chrbanks[4] = (outerchrbank << 3) | 0x6;
	selected_chrbanks[5] = (outerchrbank << 3) | 0x7;
}

void nes_clone_afbm7800_state::handle_mmc3chr_banks(uint16_t* selected_chrbanks)
{
	int bankmask;
	int outerchrbank;

	/* not correct? desert falcon
	if (m_extraregs[0] & 0x80)
	{
	    bankmask = 0x0f;
	    outerchrbank = (m_extraregs[0] & 0x38) << 1;
	}
	else
	*/
	{
		bankmask = 0x1f;
		outerchrbank = (m_extraregs[0] & 0x30) << 1;
	}

	if (m_banksel & 0x80)
		m_charbank->set_bank(1);
	else
		m_charbank->set_bank(0);

	selected_chrbanks[0] = (outerchrbank | (m_bankregs[0] & bankmask));
	selected_chrbanks[1] = (outerchrbank | (m_bankregs[1] & bankmask));
	selected_chrbanks[2] = (outerchrbank | (m_bankregs[2] & bankmask));
	selected_chrbanks[3] = (outerchrbank | (m_bankregs[3] & bankmask));
	selected_chrbanks[4] = (outerchrbank | (m_bankregs[4] & bankmask));
	selected_chrbanks[5] = (outerchrbank | (m_bankregs[5] & bankmask));
}

void nes_clone_afbm7800_state::update_banks()
{
	update_prg_banks();
	// chrbank stuff

	uint16_t selected_chrbanks[6] = { 0x00, 0x02, 0x04, 0x05, 0x06, 0x07 };

	if (m_extraregs[3] & 0x10)
	{
		handle_stdchr_banks(selected_chrbanks);
	}
	else // MMC3 mode
	{
		handle_mmc3chr_banks(selected_chrbanks);
	}

	// have to mask with m_maxchrbank because otherwise banks are set at 0x40 (would lower banks atually be chrrom?)
	m_cbank[0]->set_entry((selected_chrbanks[0] & m_maxchrbank)>>1);
	m_cbank[1]->set_entry((selected_chrbanks[1] & m_maxchrbank)>>1);
	m_cbank[2]->set_entry((selected_chrbanks[2] & m_maxchrbank));
	m_cbank[3]->set_entry((selected_chrbanks[3] & m_maxchrbank));
	m_cbank[4]->set_entry((selected_chrbanks[4] & m_maxchrbank));
	m_cbank[5]->set_entry((selected_chrbanks[5] & m_maxchrbank));
}

void nes_clone_taikee_new_state::handle_mmc3chr_banks(uint16_t* selected_chrbanks)
{
	int bankmask;
	int outerchrbank;

	bankmask = 0x3f;
	outerchrbank = 0x00;

	// is this more complex here, or is the CHR ROM just incorrectly loaded?

	// 1010 0000
	if (m_extraregs[0] == 0xa0)
		outerchrbank = 0x00;
	// 1110 0000
	else if (m_extraregs[0] == 0xe0)
		outerchrbank = 0x20; // with 3f mask for space car?
	// 1110 1000
	else if (m_extraregs[0] == 0xe8)
		outerchrbank = 0x80; // 1f mask, but no sprites?? (hot racing)
	// 1111 0010
	else if (m_extraregs[0] == 0xf2)
		outerchrbank = 0x100; // (winter race)
	// 1111 1011
	else if (m_extraregs[0] == 0xfb)
		outerchrbank = 0x180; // with 1f mask (power boat)

	if (m_banksel & 0x80)
		m_charbank->set_bank(1);
	else
		m_charbank->set_bank(0);

	selected_chrbanks[0] = (outerchrbank | (m_bankregs[0] & bankmask));
	selected_chrbanks[1] = (outerchrbank | (m_bankregs[1] & bankmask));
	selected_chrbanks[2] = (outerchrbank | (m_bankregs[2] & bankmask));
	selected_chrbanks[3] = (outerchrbank | (m_bankregs[3] & bankmask));
	selected_chrbanks[4] = (outerchrbank | (m_bankregs[4] & bankmask));
	selected_chrbanks[5] = (outerchrbank | (m_bankregs[5] & bankmask));
}

void nes_clone_afbm7800_state::update_nt_mirroring()
{
	if (m_ntmirror & 1)
	{
		m_nametables[0]->set_entry(0);
		m_nametables[1]->set_entry(0);
		m_nametables[2]->set_entry(1);
		m_nametables[3]->set_entry(1);
	}
	else
	{
		m_nametables[0]->set_entry(0);
		m_nametables[1]->set_entry(1);
		m_nametables[2]->set_entry(0);
		m_nametables[3]->set_entry(1);
	}
}

void nes_clone_afbm7800_state::mapper_8000_w(uint8_t data)
{
	m_banksel = data;
	update_banks();
}

void nes_clone_afbm7800_state::mapper_8001_w(uint8_t data)
{
	m_bankregs[m_banksel & 0x7] = data;
	update_banks();
}

void nes_clone_afbm7800_state::mapper_a000_w(uint8_t data)
{
	// nametable mirroring
	m_ntmirror = data;
	update_nt_mirroring();
}

void nes_clone_afbm7800_state::mapper_a001_w(uint8_t data)
{
	// ram protect
	m_ramprot = data;
}

void nes_clone_afbm7800_state::mapper_c000_w(uint8_t data)
{
	// irq latch
	m_mmc3_scanline_counter = data ^ 0xff;
}

void nes_clone_afbm7800_state::mapper_c001_w(uint8_t data)
{
	// irq reload
	m_mmc3_scanline_latch = data;
}

void nes_clone_afbm7800_state::mapper_e000_w(uint8_t data)
{
	// irq disable
	m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	m_mmc3_irq_enable = 0;
}

void nes_clone_afbm7800_state::mapper_e001_w(uint8_t data)
{
	// irq enable
	m_mmc3_irq_enable = 1;
}



void nes_clone_afbm7800_state::mmc3_scanline_cb(int scanline, bool vblank, bool blanked)
{
	if (m_mmc3_irq_enable)
	{
		if (!vblank && !blanked)
		{
			if (--m_mmc3_scanline_counter == -1)
			{
				m_mmc3_scanline_counter = m_mmc3_scanline_latch;
				m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
			}
		}
	}
}



void nes_clone_afbm7800_state::mapper_6000_w(uint8_t data)
{
	if (m_ramprot & 0x80)
	{
		if (!(m_extraregs[3] & 0x80))
		{
			m_extraregs[0] = data;
			update_banks();
		}
	}
}

void nes_clone_afbm7800_state::mapper_6001_w(uint8_t data)
{
	if (m_ramprot & 0x80)
	{
		m_extraregs[1] = data;
		update_banks();
	}
}

void nes_clone_afbm7800_state::mapper_6002_w(uint8_t data)
{
	if (m_ramprot & 0x80)
	{
		m_extraregs[2] = data;
		update_banks();
	}
}

void nes_clone_afbm7800_state::mapper_6003_w(uint8_t data)
{
	if (m_ramprot & 0x80)
	{
		if (!(m_extraregs[3] & 0x80))
		{
			m_extraregs[3] = data;
			update_banks();
		}
	}
}

void nes_clone_afbm7800_state::nes_clone_afbm7800_map(address_map &map)
{
	nes_clone_basemap(map);

	map(0x6000, 0x6000).mirror(0x1ffc).w(FUNC(nes_clone_afbm7800_state::mapper_6000_w));
	map(0x6001, 0x6001).mirror(0x1ffc).w(FUNC(nes_clone_afbm7800_state::mapper_6001_w));
	map(0x6002, 0x6002).mirror(0x1ffc).w(FUNC(nes_clone_afbm7800_state::mapper_6002_w));
	map(0x6003, 0x6003).mirror(0x1ffc).w(FUNC(nes_clone_afbm7800_state::mapper_6003_w));

	map(0x8000, 0xffff).m(m_rom_solderpad_bank, FUNC(address_map_bank_device::amap8));

	map(0x8000, 0x8000).mirror(0x1ffe).w(FUNC(nes_clone_afbm7800_state::mapper_8000_w));
	map(0x8001, 0x8001).mirror(0x1ffe).w(FUNC(nes_clone_afbm7800_state::mapper_8001_w));
	map(0xa000, 0xa000).mirror(0x1ffe).w(FUNC(nes_clone_afbm7800_state::mapper_a000_w));
	map(0xa001, 0xa001).mirror(0x1ffe).w(FUNC(nes_clone_afbm7800_state::mapper_a001_w));
	map(0xc000, 0xc000).mirror(0x1ffe).w(FUNC(nes_clone_afbm7800_state::mapper_c000_w));
	map(0xc001, 0xc001).mirror(0x1ffe).w(FUNC(nes_clone_afbm7800_state::mapper_c001_w));
	map(0xe000, 0xe000).mirror(0x1ffe).w(FUNC(nes_clone_afbm7800_state::mapper_e000_w));
	map(0xe001, 0xe001).mirror(0x1ffe).w(FUNC(nes_clone_afbm7800_state::mapper_e001_w));
}

uint8_t nes_clone_afbm7800_state::solderpad_r(offs_t offset)
{
	logerror("%s: solderpad read\n", machine().describe_context());
	return 0x00;
}

void nes_clone_afbm7800_state::romarea_map(address_map &map)
{
	// when (m_extraregs[1] & 1) == 1
	map(0x0000, 0x1fff).bankr("prgbank0");
	map(0x2000, 0x3fff).bankr("prgbank1");
	map(0x4000, 0x5fff).bankr("prgbank2");
	map(0x6000, 0x7fff).bankr("prgbank3");

	// when (m_extraregs[1] & 1) == 1

	map(0x8000, 0xffff).r(FUNC(nes_clone_afbm7800_state::solderpad_r));
}

void nes_clone_afbm7800_state::vram_map(address_map &map)
{
	// in MMC3 mode when (m_banksel & 0x80) == 0x00 or when not in MMC3 mode
	map(0x0000, 0x07ff).bankrw("cbank0");
	map(0x0800, 0x0fff).bankrw("cbank1");
	map(0x1000, 0x13ff).bankrw("cbank2");
	map(0x1400, 0x17ff).bankrw("cbank3");
	map(0x1800, 0x1bff).bankrw("cbank4");
	map(0x1c00, 0x1fff).bankrw("cbank5");

	// in MMC3 mode when (m_banksel & 0x80) == 0x80
	map(0x2000, 0x23ff).bankrw("cbank2");
	map(0x2400, 0x27ff).bankrw("cbank3");
	map(0x2800, 0x2bff).bankrw("cbank4");
	map(0x2c00, 0x2fff).bankrw("cbank5");
	map(0x3000, 0x37ff).bankrw("cbank0");
	map(0x3800, 0x3fff).bankrw("cbank1");
}

void nes_clone_afbm7800_state::ntram_map(address_map& map)
{
	map(0x0000, 0x03ff).bankrw("nametable0");
	map(0x0400, 0x07ff).bankrw("nametable1");
	map(0x0800, 0x0bff).bankrw("nametable2");
	map(0x0c00, 0x0fff).bankrw("nametable3");
}

void nes_clone_afbm7800_state::machine_reset()
{
	nes_clone_state::machine_reset();
	m_banksel = 0;

	m_bankregs[0] = 0x00;
	m_bankregs[1] = 0x00;
	m_bankregs[2] = 0x00;
	m_bankregs[3] = 0x00;
	m_bankregs[4] = 0x00;
	m_bankregs[5] = 0x00;
	m_bankregs[6] = 0x00;
	m_bankregs[7] = 0x00;

	m_extraregs[0] = 0x00;
	m_extraregs[1] = 0x00;
	m_extraregs[2] = 0x00;
	m_extraregs[3] = 0x00;

	m_ntmirror = 0;
	m_ramprot = 0;

	update_banks();
	update_nt_mirroring();

	m_mmc3_scanline_counter = 0;
	m_mmc3_scanline_latch = 0;
	m_mmc3_irq_enable = 0;
}

uint8_t nes_clone_afbm7800_state::vram_r(offs_t offset)
{
	return m_charbank->read8(offset);
}

void nes_clone_afbm7800_state::vram_w(offs_t offset, uint8_t data)
{
	m_charbank->write8(offset, data);
}

void nes_clone_afbm7800_state::common_start()
{
	uint8_t* ROM = memregion("maincpu")->base();

	for (int i = 0; i < 4; i++)
		m_prgbank[i]->configure_entries(0, memregion("maincpu")->bytes() / 0x2000, &ROM[0x00000], 0x02000);

	m_nt_ram.resize(0x800);

	for (int i = 0; i < 4; i++)
		m_nametables[i]->configure_entries(0, 0x800 / 0x400, &m_nt_ram[0], 0x400);

	m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x0000, 0x1fff, read8sm_delegate(*this, FUNC(nes_clone_afbm7800_state::vram_r)), write8sm_delegate(*this, FUNC(nes_clone_afbm7800_state::vram_w)));
	m_ppu->set_scanline_callback(*this, FUNC(nes_clone_afbm7800_state::mmc3_scanline_cb));

	m_ppu->space(AS_PROGRAM).install_readwrite_bank(0x2000,0x23ff,m_nametables[0]);
	m_ppu->space(AS_PROGRAM).install_readwrite_bank(0x2400,0x27ff,m_nametables[1]);
	m_ppu->space(AS_PROGRAM).install_readwrite_bank(0x2800,0x2bff,m_nametables[2]);
	m_ppu->space(AS_PROGRAM).install_readwrite_bank(0x2c00,0x2fff,m_nametables[3]);

	save_item(NAME(m_nt_ram));

	save_item(NAME(m_mmc3_scanline_counter));
	save_item(NAME(m_mmc3_scanline_latch));
	save_item(NAME(m_mmc3_irq_enable));

	save_item(NAME(m_ntmirror));
	save_item(NAME(m_ramprot));

	save_item(NAME(m_banksel));
	save_item(NAME(m_bankregs));
	save_item(NAME(m_extraregs));
}

void nes_clone_afbm7800_state::machine_start()
{
	nes_clone_state::machine_start();
	common_start();

	m_vram.resize(0x8000);

	m_maxchrbank = (0x8000/0x400)-1;

	for (int i = 0; i < 2; i++)
		m_cbank[i]->configure_entries(0, 0x8000 / 0x800, &m_vram[0], 0x800);

	for (int i = 2; i < 6; i++)
		m_cbank[i]->configure_entries(0, 0x8000 / 0x400, &m_vram[0], 0x400);

	save_item(NAME(m_vram));
}

void nes_clone_taikee_new_state::vram_w(offs_t offset, uint8_t data)
{
	// hot racing attempts to write this, but it's all ROM(!)
	//m_charbank->write8(offset, data);
}

void nes_clone_taikee_new_state::machine_start()
{
	nes_clone_state::machine_start();

	common_start();

	m_maxchrbank = (0x80000/0x400)-1;

	for (int i = 0; i < 2; i++)
		m_cbank[i]->configure_entries(0, 0x80000 / 0x800, memregion("gfx1")->base(), 0x800);

	for (int i = 2; i < 6; i++)
		m_cbank[i]->configure_entries(0, 0x80000 / 0x400, memregion("gfx1")->base(), 0x400);
}

/**************************************************
 Sudoku Specifics
**************************************************/

void nes_clone_sudoku_state::machine_reset()
{
	nes_clone_state::machine_reset();
	m_rombase = 0;
}

void nes_clone_sudoku_state::machine_start()
{
	nes_clone_state::machine_start();
	save_item(NAME(m_rombase));
}

void nes_clone_sudoku_state::nes_clone_sudoku(machine_config& config)
{
	nes_clone(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_clone_sudoku_state::nes_clone_sudoku_map);
}

void nes_clone_sudoku_state::nes_clone_sudoku_map(address_map& map)
{
	nes_clone_basemap(map);
	map(0x8000, 0xffff).rw(FUNC(nes_clone_sudoku_state::rom_r), FUNC(nes_clone_sudoku_state::bank_w));
}

uint8_t nes_clone_sudoku_state::rom_r(offs_t offset)
{
	return m_mainrom[(offset + (m_rombase * 0x8000)) & (m_mainromsize - 1)];
}

void nes_clone_sudoku_state::bank_w(uint8_t data)
{
	logerror("%04x: bank_w %02x\n", machine().describe_context(), data);
	m_rombase = data;
}

void nes_clone_sudoku_state::init_sudoku()
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

/**************************************************
 Soccer Specifics
 INES Mapper 178 / NES 2.0 Mapper 551 clone or similar?
**************************************************/

void nes_clone_vtvsocr_state::machine_reset()
{
	nes_clone_state::machine_reset();
	m_bankregs[0] = 0;
	m_bankregs[1] = 0;
	m_bankregs[2] = 0;
	m_bankregs[3] = 0;
}

void nes_clone_vtvsocr_state::machine_start()
{
	nes_clone_state::machine_start();
	save_item(NAME(m_bankregs));
}

void nes_clone_vtvsocr_state::nes_clone_vtvsocr(machine_config& config)
{
	nes_clone(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_clone_vtvsocr_state::nes_clone_vtvsocr_map);
}

void nes_clone_vtvsocr_state::nes_clone_vtvsocr_map(address_map& map)
{
	nes_clone_basemap(map);
	map(0x4800, 0x4803).w(FUNC(nes_clone_vtvsocr_state::bank_w));
	map(0x8000, 0xffff).r(FUNC(nes_clone_vtvsocr_state::rom_r));
}

uint8_t nes_clone_vtvsocr_state::rom_r(offs_t offset)
{
	int bank = m_bankregs[1] >> 1;
	return m_mainrom[(offset + (bank * 0x8000)) & (m_mainromsize - 1)];
}

void nes_clone_vtvsocr_state::bank_w(offs_t offset, uint8_t data)
{
	logerror("%04x: bank_w %02x %02x\n", machine().describe_context(), offset, data);
	m_bankregs[offset] = data;
}

/**************************************************
 Ping Pong Specifics
**************************************************/


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

#if 0
	FILE *fp;
	char filename[256];
	sprintf(filename,"decrypted_%s", machine().system().name);
	fp=fopen(filename, "w+b");
	if (fp)
	{
		fwrite(&src[0], len, 1, fp);
		fclose(fp);
	}
#endif
}


ROM_START( papsudok )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sudoku2.bin", 0x00000, 0x80000, CRC(d1ffcc1e) SHA1(2010e60933a08d0b9271ef37f338758aacba6d2d) )
ROM_END

ROM_START( nytsudo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "nytsudoku.bin", 0x00000, 0x80000, CRC(9aab1977) SHA1(6948f34d67204287418cdf79f1ca0fa0c26670e3) )
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

ROM_START( croaky )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "croakykaraoke_16in1.bin", 0x00000, 0x80000, CRC(5f939fd6) SHA1(9dd56b1ee5f27a7d9b42c2638c6a06fac5554c9b) )
ROM_END

ROM_START( vtvppong )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 ) // high bit is never set in the first 0x28000 bytes of this ROM, probably 7-bit sound data? code might need opcode bits swapping
	ROM_LOAD( "vtvpongcpu.bin", 0x00000, 0x40000, CRC(52df95fa) SHA1(3015bcc90eee862b3568f122b402c9defa566aab) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "vtvpongppu.bin", 0x00000, 0x20000, CRC(474dfc0c) SHA1(4d0afab111e40172ae0b31e94f1b74b73a18385f) )
ROM_END

ROM_START( vtvsocr )
	ROM_REGION( 0x40000, "maincpu", 0 )
	// 8-bit ROM, but byteswapped for encryption?
	ROM_LOAD16_WORD_SWAP( "virtualtvsoccer.bin", 0x00000, 0x40000, CRC(2cfe42aa) SHA1(c2cafdbd5cc6491c94efd3f1be4b70c9de737b46) )
ROM_END

ROM_START( hs36red )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "mx29lv160cb.u3", 0x00000, 0x200000, CRC(318a81bb) SHA1(8b207e6a5fca53cbf383b79ff570fcdb89639fa3) )
ROM_END

ROM_START( hs36blk )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "mx29lv160cbtc.u3", 0x00000, 0x200000, CRC(b5cf91a0) SHA1(399f015fb0580c90928e7f3d73810cc4b6cc70d9) )
ROM_END

ROM_START( dancexpt )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "dancingexpert27c010.bin", 0x0000, 0x20000, CRC(658ba2ea) SHA1(c08f6a2735b3c7383cba3a5fa3af905d5af6926f) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "dancingexpert27c020.bin", 0x00000, 0x40000, CRC(b653c40c) SHA1(9b74e56768ea5b5309df9761fb442b9be0be46e9) )
ROM_END

ROM_START( digezlg )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "15-in-1_digitalezlg.bin", 0x00000, 0x80000, CRC(11944bf2) SHA1(7c3744926cd1be9d7d81d29333b1839be201fefc) )
ROM_END

ROM_START( racechl8 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "pro 400 0517.u4", 0x00000, 0x100000, CRC(13f24783) SHA1(a3b8ceb8954495a7471dfe4d6cdaa15c9945fcc5) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "ppu 401 0517.u5", 0x00000, 0x80000, CRC(51c6d44b) SHA1(6a48ea1185cf0d2d0bcf9a1b2a8cc881e318d978) )
ROM_END

void nes_clone_state::init_nes_clone()
{
}

} // anonymous namespace


// aka 'Atari Flashback' or 'Atari Flashback 1'
// "Flashback Mini 7800 uses normal NES-style hardware, together with a mapper chipset similar to the Waixing kk33xx cartridges (NES 2.0 Mapper 534)"
CONS( 2004, afbm7800,  0,  0,  nes_clone_afbm7800,    nes_clone, nes_clone_afbm7800_state, init_nes_clone, "Atari / Nice Code", "Atari Flashback Mini 7800", 0 )

CONS( 200?, dnce2000, 0, 0, nes_clone_dnce2000, dnce2000, nes_clone_dnce2000_state, init_nes_clone, "Shenzhen Soyin Electric Appliance Ind. Co., Ltd.", "Dance 2000 / Hot 2000 (Jin Bao TV Dancing Carpet, SY-2000-04)", 0 )

CONS( 2002, croaky,   0, 0, nes_clone_dnce2000, nes_clone, nes_clone_dnce2000_state, init_nes_clone, "<unknown>", "Croaky Karaoke 16-in-1", MACHINE_NOT_WORKING ) // no inputs

// Black pad marked 'SUDOKU' with tails on the S and U characters looping over the logo.  Box says "Plug and Play Sudoku"
// Has 2 sets of 4 buttons in circular 'direction pad' layouts (on the left for directions, on the right for functions) and 9 red numbered buttons with red power LED on left of them, and reset button on right
// Alt. version was released with 'New York Times' titlescreen
CONS( 200?, papsudok,     0,  0,  nes_clone_sudoku, papsudok, nes_clone_sudoku_state, init_sudoku, "Nice Code", "Plug and Play Sudoku Game (NES based)", 0 ) // plays, but unclear how 'save' feature is meant to work, is it meant to save after shutdown or not? no obvious writes

CONS( 200?, nytsudo,      0,  0,  nes_clone_sudoku, papsudok, nes_clone_sudoku_state, init_sudoku, "Excalibur Electronics / Nice Code", "The New York Times Sudoku", 0 ) // based on the above

CONS( 200?, vtvppong,  0,  0,  nes_clone_vtvppong,    nes_clone, nes_clone_vtvppong_state, init_vtvppong, "<unknown>", "Virtual TV Ping Pong", MACHINE_NOT_WORKING )

CONS( 200?, pjoypj001, 0, 0, nes_clone, nes_clone, nes_clone_state, init_nes_clone, "Trump Grand", "PowerJoy (PJ001, NES based plug & play)", MACHINE_NOT_WORKING )

//

/*
    Dancing Export by Daidaixing (aka TimeTop)

    (notes from Sean Riddle regarding missing sound)
    There are two globs on the main PCB, the bigger one next to a label that says NT6561.
    Also two 32-pin COBs, one marked 27C020 and the other 27C010 (both dumped)

    Finally, a daughterboard with 1 glob.
    The daughterboard has 10 traces going to it; power, ground, and 7 from the smaller glob.
    The 10th trace looks like the audio output.

    I assume the daughterboard has a microcontroller for sound
    I'm not sure what the smaller glob is, but it looks like it sends commands to the daughterboard.
*/
CONS( 200?, dancexpt, 0, 0, nes_clone_dancexpt, dancexpt, nes_clone_dancexpt_state, init_nes_clone, "Daidaixing", "Dancing Expert", MACHINE_NOT_WORKING )

CONS( 200?, vtvsocr,     0,  0,  nes_clone_vtvsocr, nes_clone, nes_clone_vtvsocr_state, init_nes_clone, "<unknown>", "Virtual TV Soccer", MACHINE_NOT_WORKING )

// might be VT02 hardware, needs decrypting, possibly designed by wellminds (M350 etc.)
CONS( 2010, hs36red, 0, 0, nes_clone, nes_clone, nes_clone_state, init_nes_clone, "HengSheng", "HengSheng 36-in-1 (Red pad)", MACHINE_NOT_WORKING )
CONS( 2010, hs36blk, 0, 0, nes_clone, nes_clone, nes_clone_state, init_nes_clone, "HengSheng", "HengSheng 36-in-1 (Black pad)", MACHINE_NOT_WORKING )


// in early 2000s LG TVs
CONS( 200?, digezlg, 0, 0, nes_clone, nes_clone, nes_clone_state, init_nes_clone, "LG", "Digital ez LG", MACHINE_NOT_WORKING )

// 2005-04-03 date on PCB
CONS( 2005, racechl8, 0, 0, nes_clone_afbm7800, nes_clone, nes_clone_taikee_new_state, init_nes_clone, "Play Vision / Taikee", "Racing Challenge - 8 Games In 1", 0 )
