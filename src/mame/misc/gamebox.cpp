// license:BSD-3-Clause
// copyright-holders:R. Belmont

/*
Gamebox, 2010
Hardware info by Guru
---------------------

This is a Chinese multi-game emulation box from around 2010 probably running MAME.
At bootup it shows a menu where games can be selected.
This is the first 'Gamebox' version released.
It contains a bunch of CPS1 games (IIRC), one bad quality video and some other random games (maybe Gameboy?)
Main components are....

- PCB: GAMEBOX 20100720_V1.03
- Ingenic JZ4755 SOC (many documents available)
- 1Gbit (128MB x8-bit) SDRAM (4x Samsung 256Mbit K4S560832E-TC75)
- 2GByte NAND ROM (Hynix H27UAG8T2ATR)
- 4x USB connector for controllers, PS1 style (these are said to be custom, apparently normal USB controllers won't work)
- MicroSD slot for expansion (file type / game type unknown)
- 32 pin single-sided cart slot for expansion, labelled 'GAME CARD'. Actual cart type might be Gameboy which also has 32 pins.
- Mini USB connector, connected to SOC pin 67 (USB DATA-) and pin 66 (USB DATA+)
- RCA Jacks for Composite Video and Left/Right Audio
- 3 Crystals (to be measured later when unit re-assembled)
- Small SOIC8 chip with surface scratched. Pin 5 connected to SOC pin 39 (I2C_SDA) then 2.2k pullup resistor, pin 6 connected
  to SOC pin 38 (I2C_SCK) then 2.2k pulldown resistor. It's probably an I2C EEPROM.

**********************************************************

Firmware notes
--------------
The NAND dump holds 4096-byte pages followed by 448 spare bytes (the part itself has 218).
There is no Linux here: the box runs Ingenic's bare-metal MINIOS with loadable modules.

Boot sequence:
- The JZ4755 boot ROM reads the first 12 bytes of the NAND for the geometry, then copies
  8 KB (the first 2 KB of pages 0-3, since the header claims 2 KB pages) to 0x80000000 and
  jumps to offset 12.  The boot ROM is not dumped; machine_reset() does the copy and a
  four-instruction stub at 0xbfc00000 performs the jump.
- The SPL ("Ingenic Semiconductor Co., Ltd. Loader Version V1.0") sets up the PLL for
  24 MHz * 36 / 2 = 432 MHz, initialises the SDRAM, reads the NAND parameters from page 61
  (4096-byte pages, 218 spare, 128 pages per block) and copies NAND block 1 (the MINIOS
  kernel) to 0x80800000 with 8-bit BCH checking before jumping to it.  Progress is
  printed on UART1.
- MINIOS loads modules listed in a table at NAND offset 0x4400000: mobile_tv.bin,
  desktop.bin, gameplay.bin (the launcher), game_cps1.bin, game_mvs.bin, game_pgm.bin
  (MAME-derived emulator cores), gbaemu.bin, video and audio codecs.
- Output is composite video through the LCD controller's TV encoder.
- The four controller ports carry an I2C bus each over the USB-shaped connectors (power, ground,
  clock and data).  The firmware bit-bangs them in parallel on GPIO pins and reads the pad as an
  I2C slave at address 0x50.
- The 32-pin GAME CARD slot is a Game Boy Advance cartridge slot.  There is no memory
  controller behind it: the main application bit-bangs the cartridge bus on GPIO pins
  (PC23 = /CS, PD26 = /RD, PD27 = /WR, AD0-15 on PE0-11/18/19/22/23, A16-21 on PB26-31,
  A22/A23 on PD16/PD17) and the GBA emulator module reads the ROM through it as the
  pseudo-file "slot:\rom.gba".  At boot the game code at header offset 0xAC is probed; a
  readable code makes the box launch the cartridge instead of the menu.
- The MicroSD slot sits on MSC1 (volume "mmc:").  It can hold a firmware update and extra
  games, and is exposed as a second USB mass-storage unit.  Not emulated yet.

**********************************************************
*/


#include "emu.h"

#include "bus/gba/rom.h"
#include "cpu/mips/mips3.h"
#include "machine/i2chle.h"
#include "machine/jz4755.h"
#include "machine/nandflash.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "multibyte.h"

#include <algorithm>
#include <iterator>


namespace {

// The controller is an I2C slave at address 0x50.  A poll reads twelve active-high buttons
// (high nibble first), then 0x00/0xff for a digital pad or two analogue nibbles.  The host
// never acknowledges, so the pad keeps shifting.  Writing 0x55, 0xaa, x, y arms an unused
// challenge that returns the first two bytes scrambled.
class gamebox_pad_device : public device_t, public i2c_hle_interface
{
public:
	gamebox_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// i2c_hle_interface implementation
	virtual uint8_t read_data(uint16_t offset) override;
	virtual void write_data(uint16_t offset, uint8_t data) override;
	virtual void i2c_start() override { m_data_offset = 0; }
	virtual bool has_subaddress() const override { return false; }
	virtual bool master_nack_stops() const override { return false; }
	virtual const char *get_tag() override { return tag(); }

private:
	required_ioport m_buttons;
	uint8_t m_challenge[4];
	uint8_t m_challenge_bytes;

	static uint8_t scramble(uint8_t value);
};

DEFINE_DEVICE_TYPE_PRIVATE(GAMEBOX_PAD, gamebox_pad_device, gamebox_pad_device, "gamebox_pad", "Gamebox controller")

gamebox_pad_device::gamebox_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GAMEBOX_PAD, tag, owner, clock),
	i2c_hle_interface(mconfig, *this, 0x50),
	m_buttons(*this, "BUTTONS"),
	m_challenge{ 0, 0, 0, 0 },
	m_challenge_bytes(0)
{
}

static INPUT_PORTS_START( gamebox_pad )
	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("%p Triangle")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("%p R")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("%p L")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("%p Circle")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("%p Cross")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("%p Square")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
INPUT_PORTS_END

ioport_constructor gamebox_pad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gamebox_pad);
}

void gamebox_pad_device::device_start()
{
	save_item(NAME(m_challenge));
	save_item(NAME(m_challenge_bytes));
}

// rotate by the low two bits, then swap the nibbles
uint8_t gamebox_pad_device::scramble(uint8_t value)
{
	const uint8_t shift = value & 3;
	const uint8_t rotated = (value >> shift) | (value << (8 - shift));
	return (rotated << 4) | (rotated >> 4);
}

uint8_t gamebox_pad_device::read_data(uint16_t offset)
{
	if (m_challenge_bytes == 4)
	{
		// answer the challenge once
		if (offset == 3)
		{
			m_challenge_bytes = 0;
		}
		return (offset < 2) ? scramble(m_challenge[offset]) : m_challenge[offset];
	}

	const uint16_t buttons = m_buttons->read();
	switch (offset)
	{
		case 0: return (buttons >> 8) & 0x0f;
		case 1: return buttons & 0xff;
		case 2: return 0x00;
	}
	return 0xff;
}

void gamebox_pad_device::write_data(uint16_t offset, uint8_t data)
{
	if (offset < 4)
	{
		m_challenge[offset] = data;
		m_challenge_bytes = offset + 1;
	}
}


class gamebox_state : public driver_device
{
public:
	gamebox_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soc(*this, "soc"),
		m_nand(*this, "nand"),
		m_cart(*this, "cartslot"),
		m_pad(*this, "pad%u", 1U),
		m_sdram(*this, "sdram"),
		m_bootrom(*this, "bootrom"),
		m_cart_addr(0)
	{ }

	void gamebox(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// the GPIO pins each controller port's clock and data lines sit on
	struct pad_pins { uint8_t clk_port, clk_bit, dat_port, dat_bit; };
	static constexpr pad_pins PAD_PINS[4] =
	{
		{ jz4755_device::GPIO_F, 12, jz4755_device::GPIO_C, 24 },
		{ jz4755_device::GPIO_F, 10, jz4755_device::GPIO_F, 11 },
		{ jz4755_device::GPIO_D, 24, jz4755_device::GPIO_D, 25 },
		{ jz4755_device::GPIO_E, 13, jz4755_device::GPIO_E, 12 }
	};

	// GBA cartridge bus pins: /CS on port C, /RD and /WR on port D, AD0-15 on port E, A16-21 on port B
	static constexpr uint32_t CART_CS = 1U << 23;
	static constexpr uint32_t CART_RD = 1U << 26;
	static constexpr uint32_t CART_WR = 1U << 27;
	static constexpr uint32_t CART_AD = 0x00cc0fff;
	static constexpr uint32_t CART_A16 = 0xfc000000;

	required_device<xburst_device> m_maincpu;
	required_device<jz4755_device> m_soc;
	required_device<nand_device> m_nand;
	required_device<gba_cart_slot_device> m_cart;
	required_device_array<gamebox_pad_device, 4> m_pad;
	required_shared_ptr<uint32_t> m_sdram;
	required_region_ptr<uint32_t> m_bootrom;

	uint32_t m_gpio_levels[jz4755_device::GPIO_PORTS];
	uint32_t m_cart_addr;

	void program_map(address_map &map) ATTR_COLD;

	template <uint32_t Port> void gpio_out_w(uint32_t data);
	template <uint32_t N> void pad_sda_w(int state);
	void cart_bus_w(uint32_t port, uint32_t previous);

	static constexpr uint16_t cart_ad_from_pins(uint32_t pe) { return (pe & 0xfff) | ((pe >> 6) & 0x3000) | ((pe >> 8) & 0xc000); }
	static constexpr uint32_t cart_pins_from_ad(uint16_t ad) { return (ad & 0xfff) | ((ad & 0x3000) << 6) | ((ad & 0xc000) << 8); }
};


void gamebox_state::machine_start()
{
	// stand-in for the undumped boot ROM: jump to the SPL copied to the start of SDRAM
	static constexpr uint32_t BOOT_STUB[] =
	{
		0x3c088000,     // lui   t0,0x8000
		0x3508000c,     // ori   t0,t0,0x000c
		0x01000008,     // jr    t0
		0x00000000      // nop
	};
	std::copy(std::begin(BOOT_STUB), std::end(BOOT_STUB), &m_bootrom[0]);

	std::fill(std::begin(m_gpio_levels), std::end(m_gpio_levels), 0xffffffff);

	save_item(NAME(m_gpio_levels));
	save_item(NAME(m_cart_addr));
}

void gamebox_state::machine_reset()
{
	// what the boot ROM does on a NAND boot: the first 2 KB of pages 0-3 land at 0x80000000
	const uint8_t *const nand = m_nand->nand_data();
	const uint32_t page_total = m_nand->page_total_size();
	for (uint32_t page = 0; page < 4; page++)
	{
		for (uint32_t i = 0; i < 2048 / 4; i++)
		{
			m_sdram[page * (2048 / 4) + i] = get_u32le(&nand[page * page_total + i * 4]);
		}
	}
}


// GPIO output levels feed the controllers' clock and data lines and the cartridge bus
template <uint32_t Port> void gamebox_state::gpio_out_w(uint32_t data)
{
	const uint32_t previous = m_gpio_levels[Port];
	m_gpio_levels[Port] = data;
	for (uint32_t n = 0; n < 4; n++)
	{
		const pad_pins &pins = PAD_PINS[n];
		if (pins.clk_port == Port || pins.dat_port == Port)
		{
			// data first so a change while the clock is high reads as a start or stop condition
			m_pad[n]->sda_write(BIT(m_gpio_levels[pins.dat_port], pins.dat_bit));
			m_pad[n]->scl_write(BIT(m_gpio_levels[pins.clk_port], pins.clk_bit));
		}
	}
	if constexpr (Port == jz4755_device::GPIO_C || Port == jz4755_device::GPIO_D)
	{
		cart_bus_w(Port, previous);
	}
}

// GBA cartridge bus: the falling edge of /CS latches a 24-bit halfword address, then every
// /RD strobe returns a halfword and advances the cartridge's 16-bit address counter
void gamebox_state::cart_bus_w(uint32_t port, uint32_t previous)
{
	const uint32_t levels = m_gpio_levels[port];
	const uint32_t changed = previous ^ levels;
	if (port == jz4755_device::GPIO_C)
	{
		if (!(changed & CART_CS))
		{
			return;
		}
		if (levels & CART_CS)
		{
			// deselected: the data lines float high again
			m_soc->gpio_input_mask(jz4755_device::GPIO_E, CART_AD, CART_AD);
		}
		else
		{
			m_cart_addr = cart_ad_from_pins(m_gpio_levels[jz4755_device::GPIO_E]);
			m_cart_addr |= (m_gpio_levels[jz4755_device::GPIO_B] & CART_A16) >> 10;
			m_cart_addr |= BIT(m_gpio_levels[jz4755_device::GPIO_D], 16, 2) << 22;
		}
	}
	else if (!(m_gpio_levels[jz4755_device::GPIO_C] & CART_CS))
	{
		if ((changed & CART_RD) && !(levels & CART_RD) && m_cart->exists())
		{
			const uint32_t word = m_cart->read_rom(m_cart_addr >> 1);
			const uint16_t data = BIT(m_cart_addr, 0) ? (word >> 16) : (word & 0xffff);
			m_soc->gpio_input_mask(jz4755_device::GPIO_E, CART_AD, cart_pins_from_ad(data));
		}
		// writes are ignored but still advance the counter
		if (levels & changed & (CART_RD | CART_WR))
		{
			m_cart_addr = (m_cart_addr & 0xff0000) | ((m_cart_addr + 1) & 0xffff);
		}
	}
}

// the controller pulls its data line low for acknowledges and zero bits
template <uint32_t N> void gamebox_state::pad_sda_w(int state)
{
	m_soc->gpio_input(PAD_PINS[N].dat_port, PAD_PINS[N].dat_bit, state);
}


void gamebox_state::program_map(address_map &map)
{
	// 128 MB SDRAM, also reachable through the EMC's remapped window
	map(0x00000000, 0x07ffffff).mirror(0x20000000).ram().share(m_sdram);
	map(0x10000000, 0x13ffffff).m(m_soc, FUNC(jz4755_device::map));
	map(0x18000000, 0x1bffffff).m(m_soc, FUNC(jz4755_device::nand_map));
	map(0x1fc00000, 0x1fc01fff).rom().region(m_bootrom, 0);
	// tightly coupled shared memory (untranslated kseg3 window)
	map(0xf4000000, 0xf4003fff).ram();
}


static INPUT_PORTS_START( gamebox )
INPUT_PORTS_END


static void gba_cart(device_slot_interface &device)
{
	device.option_add_internal("gba_rom",          GBA_ROM_STD);
	device.option_add_internal("gba_sram",         GBA_ROM_SRAM);
	device.option_add_internal("gba_drilldoz",     GBA_ROM_DRILLDOZ);
	device.option_add_internal("gba_wariotws",     GBA_ROM_WARIOTWS);
	device.option_add_internal("gba_eeprom",       GBA_ROM_EEPROM);
	device.option_add_internal("gba_eeprom_4k",    GBA_ROM_EEPROM);
	device.option_add_internal("gba_yoshiug",      GBA_ROM_YOSHIUG);
	device.option_add_internal("gba_eeprom_64k",   GBA_ROM_EEPROM64);
	device.option_add_internal("gba_boktai",       GBA_ROM_BOKTAI);
	device.option_add_internal("gba_flash",        GBA_ROM_FLASH);
	device.option_add_internal("gba_flash_rtc",    GBA_ROM_FLASH_RTC);
	device.option_add_internal("gba_flash_512",    GBA_ROM_FLASH);
	device.option_add_internal("gba_flash_1m",     GBA_ROM_FLASH1M);
	device.option_add_internal("gba_flash_1m_rtc", GBA_ROM_FLASH1M_RTC);
	device.option_add_internal("gba_3dmatrix",     GBA_ROM_3DMATRIX);
}


void gamebox_state::gamebox(machine_config &config)
{
	// the SPL programs the PLL for 432 MHz from the 24 MHz crystal; the CPM follows later changes
	XBURST(config, m_maincpu, 24_MHz_XTAL * 18);
	m_maincpu->set_addrmap(AS_PROGRAM, &gamebox_state::program_map);

	JZ4755(config, m_soc, 24_MHz_XTAL);
	m_soc->set_cpu_tag(m_maincpu);
	m_soc->set_nand_tag(m_nand);
	m_soc->set_nand_rb(jz4755_device::GPIO_C, 27);
	m_soc->gpio_out<jz4755_device::GPIO_B>().set(FUNC(gamebox_state::gpio_out_w<jz4755_device::GPIO_B>));
	m_soc->gpio_out<jz4755_device::GPIO_C>().set(FUNC(gamebox_state::gpio_out_w<jz4755_device::GPIO_C>));
	m_soc->gpio_out<jz4755_device::GPIO_D>().set(FUNC(gamebox_state::gpio_out_w<jz4755_device::GPIO_D>));
	m_soc->gpio_out<jz4755_device::GPIO_E>().set(FUNC(gamebox_state::gpio_out_w<jz4755_device::GPIO_E>));
	m_soc->gpio_out<jz4755_device::GPIO_F>().set(FUNC(gamebox_state::gpio_out_w<jz4755_device::GPIO_F>));

	GAMEBOX_PAD(config, m_pad[0]);
	m_pad[0]->sda_callback().set(FUNC(gamebox_state::pad_sda_w<0>));
	GAMEBOX_PAD(config, m_pad[1]);
	m_pad[1]->sda_callback().set(FUNC(gamebox_state::pad_sda_w<1>));
	GAMEBOX_PAD(config, m_pad[2]);
	m_pad[2]->sda_callback().set(FUNC(gamebox_state::pad_sda_w<2>));
	GAMEBOX_PAD(config, m_pad[3]);
	m_pad[3]->sda_callback().set(FUNC(gamebox_state::pad_sda_w<3>));

	HYNIX_H27UAG8T2A(config, m_nand);
	m_nand->nvram_enable_backup(false);     // 2.3 GB; don't write it back on exit

	// the GAME CARD slot takes Game Boy Advance cartridges, read over GPIO
	GBA_CART_SLOT(config, m_cart, gba_cart, nullptr);
	SOFTWARE_LIST(config, "cart_list").set_original("gba");

	// composite video from the TV encoder
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(13'500'000, 858, 0, 720, 525, 0, 480);
	screen.set_screen_update(m_soc, FUNC(jz4755_device::screen_update));
	screen.screen_vblank().set(m_soc, FUNC(jz4755_device::vblank_w));

	// the internal codec's headphone/line output
	SPEAKER(config, "speaker", 2).front();
	m_soc->add_route(0, "speaker", 1.0, 0);
	m_soc->add_route(1, "speaker", 1.0, 1);
}


ROM_START( gamebox )
	ROM_REGION( 0x8e000000, "nand", 0 )
	ROM_LOAD( "h27uag8t2atr.u1", 0x00000000, 0x8e000000, BAD_DUMP CRC(bcab3fe4) SHA1(6a80099a64822e2495463e2efd70a60f8878a918) )

	ROM_REGION32_LE( 0x2000, "bootrom", ROMREGION_ERASEFF )
	ROM_LOAD( "jz4755_bootrom", 0x0000, 0x2000, NO_DUMP )     // on-chip boot ROM
ROM_END

} // anonymous namespace


CONS( 2010, gamebox, 0, 0, gamebox, gamebox, gamebox_state, empty_init, "<unknown>", "Gamebox", MACHINE_NOT_WORKING )
