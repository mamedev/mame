// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    SH6578 NES clone hardware
    enhanced NES, different to VT / OneBus systems

	video rendering is changed significantly compared to NES so not using NES PPU device
	has 256x256 pixel pages, attributes are stored next to tile numbers (not in their own table after them) etc.
	
*/

#include "emu.h"
#include "cpu/m6502/n2a03.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/bankdev.h"
#include "machine/timer.h"

class nes_sh6578_state : public driver_device
{
public:
	nes_sh6578_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bank(*this, "cartbank"),
		m_fullrom(*this, "fullrom"),
		m_vram(*this, "vram"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }
	
	void nes_sh6578(machine_config& config);

	void init_nes_sh6578();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	WRITE8_MEMBER(sprite_dma_w);

private:
	required_device<n2a03_device> m_maincpu;
	required_memory_bank m_bank;
	required_device<address_map_bank_device> m_fullrom;
	required_device<address_map_bank_device> m_vram;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	rgb_t nespal_to_RGB(int color_intensity, int color_num);

	DECLARE_READ8_MEMBER(bankswitch_r);
	DECLARE_WRITE8_MEMBER(bankswitch_w);

	DECLARE_READ8_MEMBER(dma_r);
	DECLARE_WRITE8_MEMBER(dma_w);

	uint8_t bank_r(int bank, uint16_t offset);
	void bank_w(int bank, uint16_t offset, uint8_t data);

	DECLARE_READ8_MEMBER(bank0_r) { return bank_r(0, offset); }
	DECLARE_WRITE8_MEMBER(bank0_w) { bank_w(0, offset, data); }
	DECLARE_READ8_MEMBER(bank1_r) { return bank_r(1, offset); }
	DECLARE_WRITE8_MEMBER(bank1_w) { bank_w(1, offset, data); }
	DECLARE_READ8_MEMBER(bank2_r) { return bank_r(2, offset); }
	DECLARE_WRITE8_MEMBER(bank2_w) { bank_w(2, offset, data); }
	DECLARE_READ8_MEMBER(bank3_r) { return bank_r(3, offset); }
	DECLARE_WRITE8_MEMBER(bank3_w) { bank_w(3, offset, data); }
	DECLARE_READ8_MEMBER(bank4_r) { return bank_r(4, offset); }
	DECLARE_WRITE8_MEMBER(bank4_w) { bank_w(4, offset, data); }
	DECLARE_READ8_MEMBER(bank5_r) { return bank_r(5, offset); }
	DECLARE_WRITE8_MEMBER(bank5_w) { bank_w(5, offset, data); }
	DECLARE_READ8_MEMBER(bank6_r) { return bank_r(6, offset); }
	DECLARE_WRITE8_MEMBER(bank6_w) { bank_w(6, offset, data); }
	DECLARE_READ8_MEMBER(bank7_r) { return bank_r(7, offset); }
	DECLARE_WRITE8_MEMBER(bank7_w) { bank_w(7, offset, data); }

	DECLARE_WRITE8_MEMBER(timing_setting_control_w);
	DECLARE_WRITE8_MEMBER(initial_startup_w);

	DECLARE_WRITE8_MEMBER(write_ppu);
	DECLARE_READ8_MEMBER(read_ppu);
	DECLARE_WRITE8_MEMBER(write_palette);
	DECLARE_READ8_MEMBER(read_palette);

	int m_iniital_startup_state;
	std::vector<uint8_t> m_palette_ram;

	uint8_t m_bankswitch[8];

	uint8_t m_dma_control;
	uint8_t m_dma_bank;
	uint8_t m_dma_source[2];
	uint8_t m_dma_dest[2];
	uint8_t m_dma_length[2];

	uint8_t m_2000;
	uint8_t m_colsel_pntstart;
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void do_dma();

	void rom_map(address_map& map);
	void vram_map(address_map& map);
	void nes_sh6578_map(address_map& map);

	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);
};

TIMER_DEVICE_CALLBACK_MEMBER(nes_sh6578_state::scanline)
{
	int scanline = param;

	if (scanline == 240)
	{
		if (m_2000 & 0x80)
		{
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		}
	}
}


uint8_t nes_sh6578_state::bank_r(int bank, uint16_t offset)
{
	uint32_t address;
	address = offset & 0x00fff;                   // 0x00fff part of address
	address |= (m_bankswitch[bank] & 0xff) << 12; // 0xff000 part of address
	return m_fullrom->read8(address);
}

void nes_sh6578_state::bank_w(int bank, uint16_t offset, uint8_t data)
{
	uint32_t address;
	address = offset & 0x00fff;                   // 0x00fff part of address
	address |= (m_bankswitch[bank] & 0xff) << 12; // 0xff000 part of address
	return m_fullrom->write8(address, data);
}

WRITE8_MEMBER(nes_sh6578_state::sprite_dma_w)
{
	//	int source = (data & 7);
}

READ8_MEMBER(nes_sh6578_state::bankswitch_r)
{
	return m_bankswitch[offset];
}

WRITE8_MEMBER(nes_sh6578_state::bankswitch_w)
{
	m_bankswitch[offset] = data;
}

READ8_MEMBER(nes_sh6578_state::dma_r)
{
	switch (offset)
	{
	case 0x00: logerror("%s: nes_sh6578_state::dma_r offset %01x : DMA Control :  %02x\n", machine().describe_context(), offset); return m_dma_control;
	case 0x01: logerror("%s: nes_sh6578_state::dma_r offset %01x : DMA Bank Select : %02x\n", machine().describe_context(), offset); return m_dma_bank;
	case 0x02: logerror("%s: nes_sh6578_state::dma_r offset %01x : DMA Source Address Low : %02x\n", machine().describe_context(), offset); return m_dma_source[0];
	case 0x03: logerror("%s: nes_sh6578_state::dma_r offset %01x : DMA Source Address High : %02x\n", machine().describe_context(), offset); return m_dma_source[1];
	case 0x04: logerror("%s: nes_sh6578_state::dma_r offset %01x : DMA Destination Address Low : %02x\n", machine().describe_context(), offset); return m_dma_dest[0];
	case 0x05: logerror("%s: nes_sh6578_state::dma_r offset %01x : DMA Destination Address High : %02x\n", machine().describe_context(), offset); return m_dma_dest[1];
	case 0x06: logerror("%s: nes_sh6578_state::dma_r offset %01x : DMA Length Low : %02x\n", machine().describe_context(), offset); return m_dma_length[0];
	case 0x07: logerror("%s: nes_sh6578_state::dma_r offset %01x : DMA Length High : %02x\n", machine().describe_context(), offset); return m_dma_length[1];
	}
	return 0x00;
}

void nes_sh6578_state::do_dma()
{
	if (m_dma_control & 0x80)
	{
		uint16_t dma_source = m_dma_source[0] | (m_dma_source[1] << 8);
		uint16_t dma_dest = m_dma_dest[0] | (m_dma_dest[1] << 8);
		uint16_t dma_length = m_dma_length[0] | (m_dma_length[1] << 8);

		logerror("Doing DMA :%02x bank:%02x: source:%04x dest:%04x length:%04x\n", m_dma_control, m_dma_bank, dma_source, dma_dest, dma_length);

		uint16_t realsourceaddress = dma_source;
		uint16_t realdestaddress = dma_dest;

		for (int i = 0; i <= dma_length; i++)
		{
			uint8_t readdat = 0x00;
			if (realsourceaddress & 0x8000)
			{
				// reading from ROM?
				uint32_t trueaddress = (realsourceaddress & 0x7fff) | ((m_dma_bank & 0x1f) * 0x8000);

				readdat = m_fullrom->read8(trueaddress);
			}
			else
			{
				uint32_t trueaddress = (realsourceaddress & 0x7fff);

				readdat = m_maincpu->space(AS_PROGRAM).read_byte(trueaddress);
				//logerror("reading from system area %04x\n", realsourceaddress);
			}

			if (m_dma_control & 0x20)
			{
				// write to work RAM
				logerror("writing to WORK RAM %04x %02x\n", realdestaddress, readdat);
			}
			else
			{
				m_vram->write8(realdestaddress, readdat);
			}

			realsourceaddress++;
			realdestaddress++;
		}

	}
}

WRITE8_MEMBER(nes_sh6578_state::dma_w)
{
	switch (offset)
	{
	case 0x0:
		logerror("%s: nes_sh6578_state::dma_w offset %01x : DMA Control : %02x\n", machine().describe_context(), offset, data);
		m_dma_control = data;
		do_dma();
		break;

	case 0x1:
		logerror("%s: nes_sh6578_state::dma_w offset %01x : DMA Bank Select : %02x\n", machine().describe_context(), offset, data);
		m_dma_bank = data;
		break;

	case 0x2:
		logerror("%s: nes_sh6578_state::dma_w offset %01x : DMA Source Address Low : %02x\n", machine().describe_context(), offset, data);
		m_dma_source[0] = data;
		break;

	case 0x3:
		logerror("%s: nes_sh6578_state::dma_w offset %01x : DMA Source Address High : %02x\n", machine().describe_context(), offset, data);
		m_dma_source[1] = data;
		break;

	case 0x4:
		logerror("%s: nes_sh6578_state::dma_w offset %01x : DMA Destination Address Low : %02x\n", machine().describe_context(), offset, data);
		m_dma_dest[0] = data;
		break;

	case 0x5:
		logerror("%s: nes_sh6578_state::dma_w offset %01x : DMA Destination Address High : %02x\n", machine().describe_context(), offset, data);
		m_dma_dest[1] = data;
		break;

	case 0x6:
		logerror("%s: nes_sh6578_state::dma_w offset %01x : DMA Length Low : %02x\n", machine().describe_context(), offset, data);
		m_dma_length[0] = data;
		break;

	case 0x7:
		logerror("%s: nes_sh6578_state::dma_w offset %01x : DMA Length High : %02x\n", machine().describe_context(), offset, data);
		m_dma_length[1] = data;
		break;
	}
}


WRITE8_MEMBER(nes_sh6578_state::initial_startup_w)
{
	// there is also a timeframe in which this must happen
	// if the writes are not correct the system does not operate
	if (m_iniital_startup_state == 0)
	{
		if (data == 0x65)
		{
			logerror("initial_startup_w VALID first write (0x65)\n");
			m_iniital_startup_state = 1;
		}
		else
		{
			logerror("initial_startup_w invalid first write (not 0x65)\n");
			m_iniital_startup_state = -1;
		}
	}
	else if (m_iniital_startup_state == 1)
	{
		if (data == 0x76)
		{
			logerror("initial_startup_w VALID second write (0x76)\n");
			m_iniital_startup_state = 2;
		}
		else
		{
			logerror("initial_startup_w invalid second write (not 0x76)\n");
			m_iniital_startup_state = -1;
		}
	}
	else if (m_iniital_startup_state == 2)
	{
		logerror("initial_startup_w invalid write (already passed) (%02x)\n", data);
	}
	else if (m_iniital_startup_state == -1)
	{
		logerror("initial_startup_w invalid write (already failed) (%02x)\n", data);
	}
}

WRITE8_MEMBER(nes_sh6578_state::timing_setting_control_w)
{
	logerror("%s: nes_sh6578_state::timing_setting_control_w : %02x\n", machine().describe_context(), data);
}

// borrowed from ppu2c0x.cpp, doesn't currently handle color emphasis!
rgb_t nes_sh6578_state::nespal_to_RGB(int color_intensity, int color_num)
{
	const double tint = 0.22; /* adjust to taste */
	const double hue = 287.0;

	const double Kr = 0.2989;
	const double Kb = 0.1145;
	const double Ku = 2.029;
	const double Kv = 1.140;

	static const double brightness[3][4] =
	{
		{ 0.50, 0.75, 1.0, 1.0 },
		{ 0.29, 0.45, 0.73, 0.9 },
		{ 0, 0.24, 0.47, 0.77 }
	};

	double sat;
	double y, u, v;
	double rad;

	switch (color_num)
	{
	case 0:
		sat = 0; rad = 0;
		y = brightness[0][color_intensity];
		break;

	case 13:
		sat = 0; rad = 0;
		y = brightness[2][color_intensity];
		break;

	case 14:
	case 15:
		sat = 0; rad = 0; y = 0;
		break;

	default:
		sat = tint;
		rad = M_PI * ((color_num * 30 + hue) / 180.0);
		y = brightness[1][color_intensity];
		break;
	}

	u = sat * cos(rad);
	v = sat * sin(rad);

	/* Transform to RGB */
	double R = (y + Kv * v) * 255.0;
	double G = (y - (Kb * Ku * u + Kr * Kv * v) / (1 - Kb - Kr)) * 255.0;
	double B = (y + Ku * u) * 255.0;

	/* Clipping, in case of saturation */
	if (R < 0)
		R = 0;
	if (R > 255)
		R = 255;
	if (G < 0)
		G = 0;
	if (G > 255)
		G = 255;
	if (B < 0)
		B = 0;
	if (B > 255)
		B = 255;

	return rgb_t(floor(R + .5), floor(G + .5), floor(B + .5));
}


READ8_MEMBER(nes_sh6578_state::read_palette)
{
	logerror("%s: nes_sh6578_state::read_ppu : Palette Entry %02x\n", machine().describe_context(), offset);
	return m_palette_ram[offset];
}

WRITE8_MEMBER(nes_sh6578_state::write_palette)
{
	logerror("%s: nes_sh6578_state::write_ppu : Palette Entry %02x : %02x\n", machine().describe_context(), offset, data);
	m_palette_ram[offset] = data;

	rgb_t col = nespal_to_RGB((data & 0x30) >> 4, data & 0x0f);
	m_palette->set_pen_color(offset, col);
}


READ8_MEMBER(nes_sh6578_state::read_ppu)
{
	switch (offset)
	{
	case 0x08:
		logerror("%s: nes_sh6578_state::read_ppu : Color Select & PNT Start Address\n", machine().describe_context());
		return m_colsel_pntstart;

	case 0x00:
		return m_2000;

	case 0x02:
	{
		uint8_t ret = 0x00;
		int vblank = m_screen->vpos() > 239 ? 1 : 0;

		if (vblank) ret |= 0x80;

		return ret;
	}

	default:
		//return ppu2c0x_device::read(space, offset);
		return 0x00;
	}
}

WRITE8_MEMBER(nes_sh6578_state::write_ppu)
{
	switch (offset)
	{
	case 0x08:
		logerror("%s: nes_sh6578_state::write_ppu : Color Select & PNT Start Address : %02x\n", machine().describe_context(), data);
		m_colsel_pntstart = data;
		break;

	case 0x00:
		m_2000 = data;
		break;

	default:
		break;
		//ppu2c0x_device::write(space, offset, data);
	}
}


void nes_sh6578_state::nes_sh6578_map(address_map& map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2008).rw(FUNC(nes_sh6578_state::read_ppu), FUNC(nes_sh6578_state::write_ppu));
	
	map(0x2040, 0x207f).rw(FUNC(nes_sh6578_state::read_palette), FUNC(nes_sh6578_state::write_palette));
	
	map(0x4014, 0x4014).w(FUNC(nes_sh6578_state::sprite_dma_w));

	map(0x4020, 0x4020).w(FUNC(nes_sh6578_state::timing_setting_control_w));

	map(0x4031, 0x4031).w(FUNC(nes_sh6578_state::initial_startup_w));

	map(0x4040, 0x4047).rw(FUNC(nes_sh6578_state::bankswitch_r), FUNC(nes_sh6578_state::bankswitch_w));

	map(0x4048, 0x404f).rw(FUNC(nes_sh6578_state::dma_r), FUNC(nes_sh6578_state::dma_w));


	map(0x5000, 0x57ff).ram();
	map(0x8000, 0x8fff).rw(FUNC(nes_sh6578_state::bank0_r), FUNC(nes_sh6578_state::bank0_w));
	map(0x9000, 0x9fff).rw(FUNC(nes_sh6578_state::bank1_r), FUNC(nes_sh6578_state::bank1_w));
	map(0xa000, 0xafff).rw(FUNC(nes_sh6578_state::bank2_r), FUNC(nes_sh6578_state::bank2_w));
	map(0xb000, 0xbfff).rw(FUNC(nes_sh6578_state::bank3_r), FUNC(nes_sh6578_state::bank3_w));
	map(0xc000, 0xcfff).rw(FUNC(nes_sh6578_state::bank4_r), FUNC(nes_sh6578_state::bank4_w));
	map(0xd000, 0xdfff).rw(FUNC(nes_sh6578_state::bank5_r), FUNC(nes_sh6578_state::bank5_w));
	map(0xe000, 0xefff).rw(FUNC(nes_sh6578_state::bank6_r), FUNC(nes_sh6578_state::bank6_w));
	map(0xf000, 0xffff).rw(FUNC(nes_sh6578_state::bank7_r), FUNC(nes_sh6578_state::bank7_w));
}

static INPUT_PORTS_START(nes_sh6578)
INPUT_PORTS_END

void nes_sh6578_state::video_start()
{
}

void nes_sh6578_state::machine_reset()
{
	for (int i = 0; i < 8; i++)
		m_bankswitch[i] = i;

	m_palette_ram.resize(0x40);

	for (int i = 0; i < 0x40; i++)
		m_palette_ram[i] = 0x00;

	m_iniital_startup_state = 0;
}

void nes_sh6578_state::machine_start()
{
	m_bank->configure_entry(0, memregion("maincpu")->base() + 0x0000000);
	m_bank->set_entry(0);
}

// SH6578 can address 20-bit address space (1MB of ROM)
void nes_sh6578_state::rom_map(address_map& map)
{
	map(0x00000, 0xfffff).bankr("cartbank");
}

void nes_sh6578_state::vram_map(address_map& map)
{
	map(0x0000, 0x27ff).ram();
	map(0x8000, 0xffff).ram();
}


uint32_t nes_sh6578_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 32; x++)
		{


		}
	}

	return 0;
}

void nes_sh6578_state::nes_sh6578(machine_config& config)
{
	/* basic machine hardware */
	N2A03(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_sh6578_state::nes_sh6578_map);

	ADDRESS_MAP_BANK(config, m_fullrom).set_map(&nes_sh6578_state::rom_map).set_options(ENDIANNESS_NATIVE, 8, 20, 0x100000);

	ADDRESS_MAP_BANK(config, m_vram).set_map(&nes_sh6578_state::vram_map).set_options(ENDIANNESS_NATIVE, 8, 16, 0x10000);

	TIMER(config, "scantimer").configure_scanline(FUNC(nes_sh6578_state::scanline), "screen", 0, 1);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(32 * 8, 262);
	m_screen->set_visarea(0 * 8, 32 * 8 - 1, 0 * 8, 30 * 8 - 1);
	m_screen->set_screen_update(FUNC(nes_sh6578_state::screen_update));

	PALETTE(config, m_palette).set_entries(0x40);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0.50);
}



void nes_sh6578_state::init_nes_sh6578()
{
}



ROM_START( bandgpad )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "gamepad.bin", 0x00000, 0x100000, CRC(e2fbb532) SHA1(e9170a7739a8355acbf263fe2b1d291951dc07f0) )
ROM_END

ROM_START( ts_handy11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tvplaypowercontroller.bin", 0x00000, 0x100000, CRC(9c7fe9ff) SHA1(c872e91ca835b66c9dd3b380e8374b51f12bcae0) ) // 29LV008B
ROM_END

ROM_START( cpatrolm )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "citypatrolman.bin", 0x00000, 0x100000, CRC(4b139c67) SHA1(a5b03f472a94ee879f58bbff201b671fbf4f1ea1) )
ROM_END

ROM_START( ablwikid )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "mx29f1610atc.u2", 0x00000, 0x200000, CRC(f16abf79) SHA1(aeccbb40d7fdd451ba8e5cca20464da2cf116461) )
ROM_END


CONS( 1997, bandgpad,  0,  0,  nes_sh6578,    nes_sh6578, nes_sh6578_state, init_nes_sh6578, "Bandai", "Multi Game Player Gamepad", MACHINE_NOT_WORKING )

// possibly newer than 2001
CONS( 2001, ts_handy11,  0,  0,  nes_sh6578,    nes_sh6578, nes_sh6578_state, init_nes_sh6578, "Techno Source", "Handy Boy 11-in-1 (TV Play Power)", MACHINE_NOT_WORKING )

CONS( 200?, cpatrolm,  0,  0,  nes_sh6578,    nes_sh6578, nes_sh6578_state, init_nes_sh6578, "<unknown>", "City Patrolman", MACHINE_NOT_WORKING )

// this writes the 0x65 0x76 startup code to 0x4031, but the ROM is larger than expected, extra banking?
CONS( 200?, ablwikid,   0,        0,  nes_sh6578, nes_sh6578, nes_sh6578_state, init_nes_sh6578, "Advance Bright Ltd.",         "Wikid Joystick", MACHINE_NOT_WORKING ) // or Wik!d Joystick
