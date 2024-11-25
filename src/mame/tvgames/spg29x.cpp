// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Mattel HyperScan

        08/17/2013 Skeleton driver by Sandro Ronco

        HyperScan TODO:
        - Various graphics glitches
        - Sound
        - USB

        Hyperscan has a hidden test menu that can be accessed with a specific inputs sequence:
        - During boot press and hold Select + Left Shoulder + Green until 'PLEASE WAIT' is shown on the screen
        - Press and release Red, Red, Green, Green, Yellow, Blue

****************************************************************************

        SPG290 Interrupt:

        Vector      Source
          63        SPU FIQ
          62        SPU Beatirq
          61        SPU Envirq
          60        CD servo
          59        ADC gain overflow / ADC recorder / FIFO overflow
          58        General purpose ADC
          57        Timer base
          56        Timer
          55        TV vblanking start
          54        LCD vblanking start
          53        PPU vblanking start
          52        TV
          51        Sensor frame end
          50        Sensor coordinate hit
          49        Sensor motion frame end
          48        Sensor capture done + sensor debug IRQ
          47        TV coordinate hit
          46        PPU coordinate hit
          45        USB host+device
          44        SIO
          43        SPI
          42        Uart (IrDA)
          41        NAND
          40        SD
          39        I2C master
          38        I2S slave
          37        APBDMA CH1
          36        APBDMA CH2
          35        LDM_DMA
          34        BLN_DMA
          33        APBDMA CH3
          32        APBDMA CH4
          31        Alarm + HMS
          30        MP4
          29        C3 (ECC module)
          28        GPIO
          27        Bufctl (for debug) + TV/PPU vblanking end (for debug)
          26        RESERVED1
          25        RESERVED2
          24        RESERVED3


-------

        CPU die markings on Big Buck Hunter "SunplusmM LU9001"


****************************************************************************/

#include "emu.h"
#include "cpu/score/score.h"
#include "imagedev/snapquik.h"
#include "machine/spg290_cdservo.h"
#include "machine/spg290_i2c.h"
#include "machine/spg290_ppu.h"
#include "machine/spg290_timer.h"
#include "hyperscan_card.h"
#include "hyperscan_ctrl.h"
#include "screen.h"
#include "softlist_dev.h"

#include "multibyte.h"


namespace {

class spg29x_game_state : public driver_device
{
public:
	spg29x_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_ppu(*this, "ppu"),
		m_i2c(*this, "i2c"),
		m_timers(*this, "timer%u", 0U),
		m_hyperscan_card(*this, "card"),
		m_hyperscan_ctrl(*this, "ctrl%u", 0U),
		m_leds(*this, "led%u", 0U)
	{ }

	void spg29x(machine_config &config);
	void hyperscan(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

	required_device<score7_cpu_device> m_maincpu;
private:

	virtual void machine_start() override ATTR_COLD;

	uint32_t spg290_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_hyper_exe);

	void spg290_mem(address_map &map) ATTR_COLD;
	void spg290_bios_mem(address_map &map) ATTR_COLD;

	void space_byte_w(offs_t offset, uint8_t data) { return m_maincpu->space(AS_PROGRAM).write_byte(offset, data); }
	uint32_t space_dword_r(offs_t offset)          { return m_maincpu->space(AS_PROGRAM).read_dword(offset); }

	uint16_t i2c_r(offs_t offset);

	required_device<screen_device> m_screen;
	required_device<spg290_ppu_device> m_ppu;
	required_device<spg290_i2c_device> m_i2c;
	required_device_array<spg290_timer_device, 6> m_timers;
	optional_device<hyperscan_card_device> m_hyperscan_card;
	optional_device_array<hyperscan_ctrl_device, 2> m_hyperscan_ctrl;
	output_finder<8> m_leds;

	void tve_control_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void gpio_out_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void timers_clk_sel_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint16_t m_tve_control = 0;
	uint8_t  m_tve_fade_offset = 0;
	uint16_t m_timers_clk_sel = 0;
	uint8_t  m_tve_buffer_ctrl = 3 ;
	uint32_t m_tv_start_addr[3] = { 0 };
	uint16_t m_gpio_out = 0;
};

class spg29x_nand_game_state : public spg29x_game_state
{
public:
	spg29x_nand_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg29x_game_state(mconfig, type, tag)
	{ }

	void nand_init(int blocksize, int blocksize_stripped);
	void nand_jak_bbh();
	void nand_jak_bbsf();

protected:
	void machine_reset() override ATTR_COLD;

	std::vector<uint8_t> m_strippedrom;

private:
	int m_firstvector = 0;
};

class spg29x_zonefamf_game_state : public spg29x_nand_game_state
{
public:
	spg29x_zonefamf_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		spg29x_nand_game_state(mconfig, type, tag)
	{ }

	void nand_zonefamf();

protected:
	void machine_reset() override ATTR_COLD;

private:
};





void spg29x_game_state::timers_clk_sel_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_timers_clk_sel);

	auto clock = 27_MHz_XTAL / ((data & 0xff) + 1);

	uint32_t mask = 0x100;
	for(int i=0; i<m_timers.size(); i++)
	{
		if (data & mask)
			m_timers[i]->set_clock(32.768_kHz_XTAL);
		else
			m_timers[i]->set_clock(clock);

		mask <<= 1;
	}
}

void spg29x_game_state::tve_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tve_control);
	rectangle visarea;
	switch(m_tve_control & 0xc)
	{
	case 0x0: // QVGA
		visarea.set(0, 320-1, 0, 240-1);
		break;
	case 0x4: // VGA
		visarea.set(0, 640-1, 0, 480-1);
		break;
	case 0x8: // HVGA
		visarea.set(0, 640-1, 0, 240-1);
		break;
	}

	int interlaced = m_tve_control & 1;
	if (m_tve_control & 2)
		m_screen->configure(864, 625, visarea, HZ_TO_ATTOSECONDS(27_MHz_XTAL) * 864 * 625 * (interlaced ? 2 : 1));      // PAL
	else
		m_screen->configure(858, 525, visarea, HZ_TO_ATTOSECONDS(27_MHz_XTAL) * 858 * 525 * (interlaced ? 2 : 1));      // NTSC
}

void spg29x_game_state::gpio_out_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_gpio_out);

	if (ACCESSING_BITS_0_7)
		m_hyperscan_card->write(BIT(m_gpio_out,1));

	for(int i=0; i<8; i++)
		m_leds[i] = BIT(m_gpio_out, 5 + i);
}

uint16_t spg29x_game_state::i2c_r(offs_t offset)
{
	int port = (offset >> 4) & 0x0f;

	if (port < 2)
		return m_hyperscan_ctrl[port]->read(offset);

	return 0xffff;
}

uint32_t spg29x_game_state::spg290_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_ppu->screen_update(screen, bitmap, cliprect);

	// TVE frame buffer
	if (m_tve_buffer_ctrl < 3)
	{
		auto &space = m_maincpu->space(AS_PROGRAM);
		const bool interlaced = m_tve_control & 1;
		for (int y = cliprect.min_y; y <= cliprect.max_y; y += (interlaced ? 1 : 2))
			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				const uint16_t rgb = space.read_word(m_tv_start_addr[m_tve_buffer_ctrl] + (y * cliprect.width() + x) * 2);
				const rgb_t pix = rgb_t(pal5bit(rgb >> 11), pal6bit(rgb >> 5), pal5bit(rgb >> 0));

				bitmap.pix(y, x) = pix;
				if (!interlaced && cliprect.contains(x, y + 1))
					bitmap.pix(y + 1, x) = pix;
			}
	}

	if (m_tve_fade_offset)
	{
		int fade_offset = 255 - m_tve_fade_offset;
		for (int y=0; y <= cliprect.max_y; y++)
			for (int x=0; x <= cliprect.max_x; x++)
			{
				rgb_t pix(bitmap.pix(y, x));
				bitmap.pix(y, x) = rgb_t(pix.r() * fade_offset / 255, pix.g() * fade_offset / 255, pix.b() * fade_offset / 255);
			}
	}

	return 0;
}

void spg29x_game_state::spg290_mem(address_map &map)
{
	map.global_mask(0x1fffffff);
	map(0x00000000, 0x00ffffff).ram().mirror(0x07000000);

	map(0x08030000, 0x08030003).w(FUNC(spg29x_game_state::tve_control_w)).lr32(NAME([this](uint32_t data) { return m_tve_control; }));
	map(0x0803000c, 0x0803000f).lw32(NAME([this](uint32_t data) { m_tve_fade_offset = data & 0xff; }));
	map(0x08070000, 0x0807000b).lr32(NAME([this](offs_t offset) { return m_tv_start_addr[offset]; }));
	map(0x08070000, 0x0807000b).lw32(NAME([this](offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_tv_start_addr[offset]); }));
	map(0x08090020, 0x08090023).lw32(NAME([this](uint32_t data) { m_tve_buffer_ctrl = data & 3; }));
	map(0x0807006c, 0x0807006f).lr32(NAME([]() { return 0x01;}));               // MUI Status: SDRAM is in the self-refresh mode
	//map(0x08150000, 0x08150000).lw8(NAME([this](uint8_t data) { printf("%c", data); })); // UART
	map(0x082100e4, 0x082100e7).w(FUNC(spg29x_game_state::timers_clk_sel_w)).lr32(NAME([this]() { return m_timers_clk_sel; }));       // Timer Source Clock Selection
	map(0x08240000, 0x0824000f).noprw();

	//map(0x08000000, 0x0800ffff);  // CSI
	map(0x08010000, 0x0801ffff).m("ppu", FUNC(spg290_ppu_device::map));
	//map(0x08020000, 0x0802ffff);  // JPG
	//map(0x08030000, 0x0803ffff);  // TV
	//map(0x08040000, 0x0804ffff);  // LCD
	//map(0x08050000, 0x0805ffff);  // SPU
	map(0x08060000, 0x0806ffff).rw("cdservo", FUNC(spg290_cdservo_device::read), FUNC(spg290_cdservo_device::write));
	//map(0x08070000, 0x0807ffff);  // MIU
	//map(0x08080000, 0x0808ffff);  // APBDMA
	//map(0x08090000, 0x0809ffff);  // BUFCTL
	//map(0x080a0000, 0x080affff);  // IRQCTL
	//map(0x080b0000, 0x080bffff);  // GPUBUF
	//map(0x080c0000, 0x080cffff);  // LDMDMA
	//map(0x080d0000, 0x080dffff);  // BLNDMA
	//map(0x080e0000, 0x080effff);  // TPGBUF
	//map(0x080f0000, 0x080fffff);  // AHBDEC
	//map(0x08100000, 0x0810ffff);  // GPIO
	//map(0x08110000, 0x0811ffff);  // SPI
	//map(0x08120000, 0x0812ffff);  // SIO
	map(0x08130000, 0x0813ffff).rw("i2c", FUNC(spg290_i2c_device::read), FUNC(spg290_i2c_device::write));
	//map(0x08140000, 0x0814ffff);  // I2S
	//map(0x08150000, 0x0815ffff);  // UART
	map(0x08160000, 0x08160fff).rw(m_timers[0], FUNC(spg290_timer_device::read), FUNC(spg290_timer_device::write));
	map(0x08161000, 0x08161fff).rw(m_timers[1], FUNC(spg290_timer_device::read), FUNC(spg290_timer_device::write));
	map(0x08162000, 0x08162fff).rw(m_timers[2], FUNC(spg290_timer_device::read), FUNC(spg290_timer_device::write));
	map(0x08163000, 0x08163fff).rw(m_timers[3], FUNC(spg290_timer_device::read), FUNC(spg290_timer_device::write));
	map(0x08164000, 0x08164fff).rw(m_timers[4], FUNC(spg290_timer_device::read), FUNC(spg290_timer_device::write));
	map(0x08165000, 0x08165fff).rw(m_timers[5], FUNC(spg290_timer_device::read), FUNC(spg290_timer_device::write));
	//map(0x08166000, 0x08166fff);  // RTC
	//map(0x08170000, 0x0817ffff);  // WDOG
	//map(0x08180000, 0x0818ffff);  // SD
	//map(0x08190000, 0x0819ffff);  // FLASH
	//map(0x081a0000, 0x081affff);  // ADC
	//map(0x081b0000, 0x081bffff);  // USB device
	//map(0x081c0000, 0x081cffff);  // USB host
	//map(0x081d0000, 0x081dffff);  // reserved
	//map(0x081e0000, 0x081effff);  // Reserved
	//map(0x081f0000, 0x081fffff);  // reserved
	//map(0x08200000, 0x0820ffff);  // SFTCFG
	//map(0x08210000, 0x0821ffff);  // CKG
	map(0x0821006c, 0x0821006f).w(m_timers[0], FUNC(spg290_timer_device::control_w));
	map(0x08210070, 0x08210073).w(m_timers[1], FUNC(spg290_timer_device::control_w));
	map(0x08210074, 0x08210077).w(m_timers[2], FUNC(spg290_timer_device::control_w));
	map(0x08210078, 0x0821007b).w(m_timers[3], FUNC(spg290_timer_device::control_w));
	map(0x0821007c, 0x0821007f).w(m_timers[4], FUNC(spg290_timer_device::control_w));
	map(0x08210080, 0x08210083).w(m_timers[5], FUNC(spg290_timer_device::control_w));
	//map(0x08220000, 0x0822ffff);  // MP4
	//map(0x08230000, 0x0823ffff);  // MIU2
	//map(0x08240000, 0x0824ffff);  // ECC

	map(0x0a000000, 0x0a003fff).ram();                         // internal SRAM
	map(0x0b000000, 0x0b007fff).rom().region("spg290", 0);  // internal ROM
}

void spg29x_game_state::spg290_bios_mem(address_map& map)
{
	spg290_mem(map);
	map(0x08200024, 0x08200027).w(FUNC(spg29x_game_state::gpio_out_w)).lr32(NAME([this]() { return m_gpio_out; }));
	map(0x08200068, 0x0820006b).lr32(NAME([this]() { return m_hyperscan_card->read(); }));
	map(0x10000000, 0x100fffff).rom().region("bios", 0).mirror(0x0ff00000);
}

/* Input ports */
static INPUT_PORTS_START( hyperscan )
INPUT_PORTS_END


void spg29x_game_state::machine_start()
{
	m_leds.resolve();

	save_item(NAME(m_tve_control));
	save_item(NAME(m_tve_fade_offset));
	save_item(NAME(m_timers_clk_sel));
	save_item(NAME(m_tve_buffer_ctrl));
	save_item(NAME(m_tv_start_addr));
	save_item(NAME(m_gpio_out));
}

void spg29x_game_state::machine_reset()
{
	m_tve_control = 0;
	m_tve_fade_offset = 0;
	m_timers_clk_sel = 0;
	m_tve_buffer_ctrl = 3;
	m_tv_start_addr[0] = m_tv_start_addr[1] = m_tv_start_addr[2] = 0;
	m_gpio_out = 0;

	// disable JTAG
	m_maincpu->set_state_int(SCORE_CR + 29, 0x20000000);

	// boot from Internal ROM - doesn't currently work as the internal ROM needs to correctly detect the external configuration before booting
	// m_maincpu->set_state_int(SCORE_PC, 0x8b000000);
}

void spg29x_nand_game_state::machine_reset()
{
	spg29x_game_state::machine_reset();

	uint32_t bootstrap_ram_start = get_u32le(&m_strippedrom[m_firstvector+0]);
	uint32_t bootstrap_ram_end   = get_u32le(&m_strippedrom[m_firstvector+4]);
	uint32_t bootstrap_ram_boot  = get_u32le(&m_strippedrom[m_firstvector+8]);

	// there is a 0x01 at 0x26, possibly related to source location / block in NAND to copy from?

	logerror("NAND Bootstrap RAM start: %08x RAM end: %08x RAM boot: %08x", bootstrap_ram_start, bootstrap_ram_end, bootstrap_ram_boot);

	uint32_t sourceaddr = 0x10000;
	for (uint32_t addr = bootstrap_ram_start; addr <= bootstrap_ram_end; addr++)
	{
		address_space& mem = m_maincpu->space(AS_PROGRAM);
		uint8_t byte = m_strippedrom[sourceaddr];
		mem.write_byte(addr, byte);
		sourceaddr++;
	}

	// probably jumped to from internal ROM?
	m_maincpu->set_state_int(SCORE_PC, bootstrap_ram_boot);
}

void spg29x_zonefamf_game_state::machine_reset()
{
	spg29x_game_state::machine_reset();

	uint32_t sourceaddr = 0x80000;
	for (uint32_t addr = 0; addr <= 0x80000; addr++)
	{
		address_space& mem = m_maincpu->space(AS_PROGRAM);
		uint8_t byte = m_strippedrom[sourceaddr];
		mem.write_byte(addr, byte);
		sourceaddr++;
	}

	m_maincpu->set_state_int(SCORE_PC, 0x4);
}



QUICKLOAD_LOAD_MEMBER(spg29x_game_state::quickload_hyper_exe)
{
	const uint32_t length = image.length();

	auto [err, ptr, actual] = read(image.image_core_file(), length);
	if (err || (actual != length))
		return std::make_pair(err ? err : std::errc::io_error, std::string());

	auto &space = m_maincpu->space(AS_PROGRAM);
	for (uint32_t i = 0; i < length; i++)
		space.write_byte(0xa00901fc + i, ptr[i]);

	m_maincpu->set_state_int(SCORE_PC, 0xa0091000); // Game entry point

	return std::make_pair(std::error_condition(), std::string());
}

void spg29x_game_state::spg29x(machine_config &config)
{
	/* basic machine hardware */
	SCORE7(config, m_maincpu, 27_MHz_XTAL * 4);   // 108MHz S+core 7
	m_maincpu->set_addrmap(AS_PROGRAM, &spg29x_game_state::spg290_mem);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(27_MHz_XTAL, 858, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(spg29x_game_state::spg290_screen_update));
	m_screen->screen_vblank().set(m_ppu, FUNC(spg290_ppu_device::screen_vblank));

	for (int i=0; i<6; i++)
	{
		SPG290_TIMER(config, m_timers[i], 27_MHz_XTAL);
		m_timers[i]->irq_cb().set_inputline(m_maincpu, 56);
	}

	SPG290_PPU(config, m_ppu, 27_MHz_XTAL, m_screen);
	m_ppu->vblank_irq_cb().set_inputline(m_maincpu, 53);
	m_ppu->space_read_cb().set(FUNC(spg29x_game_state::space_dword_r));

	spg290_cdservo_device &cdservo(SPG290_CDSERVO(config, "cdservo", 27_MHz_XTAL, "cdrom"));
	cdservo.irq_cb().set_inputline(m_maincpu, 60);
	cdservo.space_write_cb().set(FUNC(spg29x_game_state::space_byte_w));

	SPG290_I2C(config, m_i2c, 27_MHz_XTAL);
	m_i2c->irq_cb().set_inputline(m_maincpu, 39);
}

void spg29x_game_state::hyperscan(machine_config &config)
{
	spg29x(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &spg29x_game_state::spg290_bios_mem);

	m_i2c->i2c_read_cb().set(FUNC(spg29x_game_state::i2c_r));

	CDROM(config, "cdrom").set_interface("cdrom");

	HYPERSCAN_CTRL(config, m_hyperscan_ctrl[0], 0);
	HYPERSCAN_CTRL(config, m_hyperscan_ctrl[1], 0);

	HYPERSCAN_CARD(config, m_hyperscan_card, 0);

	SOFTWARE_LIST(config, "cd_list").set_original("hyperscan");
	SOFTWARE_LIST(config, "card_list").set_original("hyperscan_card");

	QUICKLOAD(config, "quickload", "exe").set_load_callback(FUNC(spg29x_game_state::quickload_hyper_exe));
}

void spg29x_nand_game_state::nand_init(int blocksize, int blocksize_stripped)
{
	uint8_t* rom = memregion("nand")->base();
	int size = memregion("nand")->bytes();

	int numblocks = size / blocksize;

	m_strippedrom.resize(numblocks * blocksize_stripped);

	for (int i = 0; i < numblocks; i++)
	{
		const int base = i * blocksize;
		const int basestripped = i * blocksize_stripped;

		for (int j = 0; j < blocksize_stripped; j++)
		{
			m_strippedrom[basestripped + j] = rom[base + j];
		}
	}

	// debug to allow for easy use of unidasm.exe
	if (0)
	{
		auto filename = "stripped_" + std::string(machine().system().name);
		auto fp = fopen(filename.c_str(), "w+b");
		if (fp)
		{
			fwrite(&m_strippedrom[0], blocksize_stripped * numblocks, 1, fp);
			fclose(fp);
		}
	}
}

void spg29x_nand_game_state::nand_jak_bbh()
{
	nand_init(0x210, 0x200);
	m_firstvector = 0xc;
}

void spg29x_nand_game_state::nand_jak_bbsf()
{
	nand_init(0x210, 0x200);
	m_firstvector = 0x8;
}

void spg29x_zonefamf_game_state::nand_zonefamf()
{
	nand_init(0x840, 0x800);
//  m_firstvector = 0x8;
}

/* ROM definition */
ROM_START( hyprscan )
	ROM_REGION( 0x100000, "bios", ROMREGION_32BIT | ROMREGION_LE )
	ROM_LOAD32_DWORD("hyperscan.bin", 0x000000, 0x100000, CRC(ce346a14) SHA1(560cb747e7193e6781d4b8b0bd4d7b45d3d28690))

	ROM_REGION( 0x008000, "spg290", ROMREGION_32BIT | ROMREGION_LE )
	ROM_LOAD32_DWORD("spg290.bin", 0x000000, 0x008000, CRC(41aad748) SHA1(3f65f8e88b1c5e9cbc8b39bb3228ebf616aced5a) ) // 256Kbit SPG290 internal ROM
ROM_END

// the sets below might be using the same SPG290 internal ROM as the above but configured to load from NAND
// however as the CPU dies were under epoxy globs the exact chip models are not confirmed

ROM_START( jak_bbh )
	ROM_REGION( 0x4200000, "nand", 0 ) // ID returned C25A, read as what appears to be a compatible type.
	ROM_LOAD("bigbuckhunterpro_as_hy27us0812a_c25a.bin", 0x000000, 0x4200000, CRC(e2627540) SHA1(c8c6e5fbc4084fa695390bbb4e1e52e671f050da) )

	ROM_REGION( 0x008000, "spg290", ROMREGION_32BIT | ROMREGION_LE )
	ROM_LOAD32_DWORD("internal.rom", 0x000000, 0x008000, NO_DUMP)
ROM_END


ROM_START( jak_bbsf )
	ROM_REGION( 0x4200000, "nand", 0 )
	ROM_LOAD("bigbucksafari.bin", 0x000000, 0x4200000, CRC(dc5f9bf1) SHA1(27893c396d62f353ced52ef88fd9ade5c051598f) )

	ROM_REGION( 0x008000, "spg290", ROMREGION_32BIT | ROMREGION_LE )
	ROM_LOAD32_DWORD("internal.rom", 0x000000, 0x008000, NO_DUMP)
ROM_END

ROM_START( zonefamf )
	ROM_REGION( 0x21000000, "nand", 0 )
	ROM_LOAD("hy27uf084g2m_withspare.u1", 0x000000, 0x21000000, CRC(ee12b689) SHA1(fd9c708b6bb2e7574173a140d8839869a8c9f51a) )

	ROM_REGION( 0x008000, "spg290", ROMREGION_32BIT | ROMREGION_LE )
	ROM_LOAD32_DWORD("internal.rom", 0x000000, 0x008000, NO_DUMP)

	//has 1x 48LC8M16A2 (128Mbit/16MByte SDRAM) for loading game into
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY   FULLNAME     FLAGS
COMP( 2006, hyprscan,   0,      0,      hyperscan, hyperscan, spg29x_game_state, empty_init, "Mattel", "HyperScan", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

// There were 1 player and 2 player versions for these JAKKS guns.  The 2nd gun appears to be simply a controller (no AV connectors) but as they were separate products with the 2 player verisons being released up to a year after the original, the code could differ.
// If they differ, it is currently uncertain which versions these ROMs are from
COMP( 2009, jak_bbh,    0,      0,      spg29x, hyperscan, spg29x_nand_game_state, nand_jak_bbh, "JAKKS Pacific Inc", "Big Buck Hunter Pro (JAKKS Pacific TV Game)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) //has ISSI 404A (24C04)
COMP( 2011, jak_bbsf,   0,      0,      spg29x, hyperscan, spg29x_nand_game_state, nand_jak_bbsf,"JAKKS Pacific Inc", "Big Buck Safari (JAKKS Pacific TV Game)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // has ISSI 416A (24C16)

COMP( 201?, zonefamf,  0,      0,      spg29x, hyperscan, spg29x_zonefamf_game_state, nand_zonefamf,"Zone", "Zone Family Fit", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

// the sets in spg29x_lexibook_jg7425.cpp probably also belong here, as they use an SPG293 which has the same peripheral mappings (but they make use of additional features)
// see emu293 https://github.com/gatecat/emu293
