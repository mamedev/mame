// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                             -= Galaxy Games =-

                    driver by Luca Elia (l.elia@tin.it)


CPU:    68000
Video:  Blitter with two layers and double buffering (Xilinx FPGA)
Sound:  OKI M6295
Input:  Trackballs and buttons
Other:  EEPROM
Carts:  EEPROM + Flash + PIC

To Do:

- Coin optics

Notes:

- 4 known game carts where produced, these are:

    Star Pak 1: Seek the Peaks, 21 Thunder, Solar Solitaire, Prism Poker, Pharaoh's Tomb, Magic Black Jack,
                Twenty One Thunder Plus, Power Pairs, Prism Poker Plus & Have A Cow
    Star Pak 2: Pac-Man, Ms.Pac-Man, Pharaoh's Tomb, Solar Solitaire, Power Pairs, Seek The peeks & Have A Cow
    Star Pak 3: Centipede, Great Wall, Ker-Chunk, Diamond Derby, Word Sleuth, Pull!, Astro Blast & Sweeper
    Star Pak 4: Berzerk, Neon Nightmare, Battle Checkers, Orbit, Deep Sea Shadow, Star Tiger & Orbit Freefall

- There is a hard lock that SP1 and the PAC-MAN games (on SP2) cannot play together. Was a licensing issue with Namco.
  The system checks for cartridges on power up by querying the PIC parts. If the system sees SP1 & SP2 it disables SP2.

- Early flyers show "Star Pak 1" titled as Cardmania!
- Early flyers show "Star Pak 2" titled as Galaxy Games Volume 2
- There is an early flyer showing a Cardmania! cartridge in front of a partialy blocked cartridge labeled Casino

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/eepromser.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "video/cesblit.h"

#include "dirom.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


/***************************************************************************

                              Galgames Cart Device

 Each cartridge contains a PIC, that provides, among other things, a 32-byte
 response starting with "CES1997", followed by the contents of the cart rom
 at some fixed addresses.

***************************************************************************/

DECLARE_DEVICE_TYPE(GALGAMES_CART,          galgames_cart_device)
DECLARE_DEVICE_TYPE(GALGAMES_BIOS_CART,     galgames_bios_cart_device)
DECLARE_DEVICE_TYPE(GALGAMES_STARPAK2_CART, galgames_starpak2_cart_device)
DECLARE_DEVICE_TYPE(GALGAMES_STARPAK3_CART, galgames_starpak3_cart_device)
DECLARE_DEVICE_TYPE(GALGAMES_SLOT,          galgames_slot_device)

// CART declaration

class galgames_cart_device : public device_t, public device_rom_interface<21, 1, 0, ENDIANNESS_BIG>
{
public:
	// construction/destruction

	galgames_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, u8 cart)
		: galgames_cart_device(mconfig, GALGAMES_CART, tag, owner, (u32)0)
	{
		set_cart(cart);
	}

	// static configuration
	void set_cart(u8 cart) { m_cart = cart; }
	void set_pic_bits(int clk, int in, int out, int dis);

	// ROM
	u16 rom_r(offs_t offset)    { return read_word(offset*2); }

	// EEPROM
	u8 eeprom_r();
	void eeprom_w(u8 data);
	void eeprom_cs_write(int state);

	// PIC
	u8 pic_status_r();
	void pic_data_w(u8 data);
	u8 pic_data_r();
	void set_pic_reset_line(int state);

	u8 int_pic_data_r();
	void int_pic_data_w(u8 data);
	void int_pic_bank_w(u8 data);

protected:
	galgames_cart_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	bool is_selected();

	u8 m_cart;

	// SLOT
	required_device<galgames_slot_device> m_slot;

	// EEPROM
	optional_device<eeprom_serial_93cxx_device> m_eeprom;

	// PIC
	optional_device<pic16c5x_device> m_pic;

	u8 m_pic_iobits = 0, m_pic_data = 0, m_pic_data_rdy = 0, m_pic_data_bit = 0, m_pic_data_clk = 0;
	u8 m_pic_clk_mask = 0, m_pic_in_mask = 0, m_pic_out_mask = 0, m_pic_dis_mask = 0;

	void log_cart_comm(const char *text, u8 data);
	void pic_comm_reset();
};

// device type definition
DEFINE_DEVICE_TYPE(GALGAMES_CART, galgames_cart_device, "starpak_cart", "Galaxy Games StarPak Cartridge")


// BIOS "cart"

class galgames_bios_cart_device : public galgames_cart_device
{
public:
	// construction/destruction
	galgames_bios_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, u8 cart)
		: galgames_cart_device(mconfig, GALGAMES_BIOS_CART, tag, owner, (u32)0)
	{
		set_cart(cart);
	}

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DEFINE_DEVICE_TYPE(GALGAMES_BIOS_CART, galgames_bios_cart_device, "galgames_bios_cart", "Galaxy Games BIOS Cartridge")

void galgames_bios_cart_device::device_add_mconfig(machine_config &config)
{
	EEPROM_93C76_8BIT(config, "eeprom");
}


// STARPAK2 cart

class galgames_starpak2_cart_device : public galgames_cart_device
{
public:
	// construction/destruction
	galgames_starpak2_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, u8 cart)
		: galgames_cart_device(mconfig, GALGAMES_STARPAK2_CART, tag, owner, (u32)0)
	{
		set_cart(cart);
		set_pic_bits(5, 4, 2, 1);
	}

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DEFINE_DEVICE_TYPE(GALGAMES_STARPAK2_CART, galgames_starpak2_cart_device, "starpak2_cart", "Galaxy Games StarPak 2 Cartridge")

void galgames_starpak2_cart_device::device_add_mconfig(machine_config &config)
{
	pic16c56_device &pic(PIC16C56(config, "pic", XTAL(4'000'000)));  // !! PIC12C508 !! 4MHz internal RC oscillator (selected by the configuration word)
	pic.read_b().set(FUNC(galgames_cart_device::int_pic_data_r));
	pic.write_b().set(FUNC(galgames_cart_device::int_pic_data_w));

	EEPROM_93C76_8BIT(config, "eeprom");
}


// STARPAK3 cart

class galgames_starpak3_cart_device : public galgames_cart_device
{
public:
	// construction/destruction
	galgames_starpak3_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, u8 cart)
		: galgames_cart_device(mconfig, GALGAMES_STARPAK3_CART, tag, owner, (u32)0)
	{
		set_cart(cart);
		set_pic_bits(0, 2, 3, 4);
	}

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DEFINE_DEVICE_TYPE(GALGAMES_STARPAK3_CART, galgames_starpak3_cart_device, "starpak3_cart", "Galaxy Games StarPak 3 Cartridge")

void galgames_starpak3_cart_device::device_add_mconfig(machine_config &config)
{
	pic16c56_device &pic(PIC16C56(config, "pic", XTAL(4'000'000)));
	pic.write_a().set(FUNC(galgames_cart_device::int_pic_bank_w));
	pic.read_b().set(FUNC(galgames_cart_device::int_pic_data_r));
	pic.write_b().set(FUNC(galgames_cart_device::int_pic_data_w));

	EEPROM_93C76_8BIT(config, "eeprom");
}


/***************************************************************************

                              Galgames Slot Device

***************************************************************************/

// SLOT declaration

class galgames_slot_device : public device_t, public device_memory_interface
{
public:
	// construction/destruction
	galgames_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void slot_map(address_map &map) ATTR_COLD;

	u16 read(offs_t offset, u16 mem_mask = ~0)               { return m_space->read_word(offset * 2, mem_mask); }
	void write(offs_t offset, u16 data, u16 mem_mask = ~0)   { m_space->write_word(offset * 2, data, mem_mask); }

	// SLOT
	void cart_sel_w(u8 data);
	void ram_sel_w(u8 data);

	// ROM
	u16 rom0_r(offs_t offset);
	u16 rom_r(offs_t offset);
	u16 rom0_or_ram_r(offs_t offset);
	u16 rom_or_ram_r(offs_t offset);
	void ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	// PIC
	u8 pic_status_r();
	void pic_data_w(u8 data);
	u8 pic_data_r();
	void set_pic_reset_line(int state);

	// EEPROM
	u8 eeprom_r();
	void eeprom_w(u8 data);
	void eeprom_cs_write(int state);

	u8 get_cart() const { return m_cart; }

protected:

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_space_config;
	address_space *m_space = nullptr;

	required_shared_ptr<u16> m_ram;

	required_device<galgames_cart_device> m_cart0;

	required_device<galgames_cart_device> m_cart1;
	required_device<galgames_cart_device> m_cart2;
	required_device<galgames_cart_device> m_cart3;
	required_device<galgames_cart_device> m_cart4;

	void set_cart(int cart);
	void reset_eeproms_except(int cart);

	galgames_cart_device *m_carts[1+4]{};

	u8 m_cart = 0;
	bool m_is_ram_active = false;
};

device_memory_interface::space_config_vector galgames_slot_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_space_config)
	};
}

// device type definition
DEFINE_DEVICE_TYPE(GALGAMES_SLOT, galgames_slot_device, "starpak_slot", "Galaxy Games Slot")

// CART implementation

galgames_cart_device::galgames_cart_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		u32 clock):
	device_t(mconfig, type, tag, owner, clock),
	device_rom_interface(mconfig, *this),
	m_cart(0),
	m_slot(*this, "^slot"),
	m_eeprom(*this, "eeprom"),
	m_pic(*this, "pic")
{
}

void galgames_cart_device::device_start()
{
	save_item(NAME(m_cart));
	save_item(NAME(m_pic_iobits));
	save_item(NAME(m_pic_data));
	save_item(NAME(m_pic_data_rdy));
	save_item(NAME(m_pic_data_clk));
	save_item(NAME(m_pic_data_bit));
}

void galgames_cart_device::device_reset()
{
	pic_comm_reset();
}

bool galgames_cart_device::is_selected()
{
	return m_slot->get_cart() == m_cart;
}

void galgames_cart_device::set_pic_reset_line(int state)
{
	if (!m_pic)
		return;

//  logerror("reset line = %x\n", state);

	if (!m_pic->input_state(INPUT_LINE_RESET) && state)
		pic_comm_reset();

	m_pic->set_input_line(INPUT_LINE_RESET, state);
}

void galgames_cart_device::log_cart_comm(const char *text, u8 data)
{
//  logerror("%s: comm %-10s %02x - data:%02x bit:%02x rdy:%x clk:%02x\n", machine().describe_context(),
//      text, data, m_pic_data, m_pic_data_bit, m_pic_data_rdy, m_pic_data_clk );

//  logerror("%s: comm %-10s %02x\n", machine().describe_context(), text, data);
}

void galgames_cart_device::pic_comm_reset()
{
	m_pic_iobits = m_pic_data = m_pic_data_rdy = m_pic_data_clk = 0;
	m_pic_data_bit = 0xff;
//  logerror("%s: comm reset\n", machine().describe_context());
}

// External PIC status and data interface

u8 galgames_cart_device::pic_status_r()
{
	// bit 7 = data from the cart PIC can be read
	return (is_selected() && (m_pic_data_rdy == 2)) ? 0x80 : 0;
}

u8 galgames_cart_device::pic_data_r()
{
	if (is_selected())
	{
		if (!machine().side_effects_disabled())
			m_pic_data_rdy = 0;
		return m_pic_data;
	}
	return 0xff;
}

void galgames_cart_device::pic_data_w(u8 data)
{
	if (is_selected())
	{
		m_pic_data      =   data;
		m_pic_data_rdy  =   1;
		m_pic_data_bit  =   0xff;
		m_pic_data_clk  =   0;
		log_cart_comm("EXT WRITE", data);
	}
}

/*
galgame2:
    bit 0 = cleared at boot (never touched again)
    bit 1 = PIC waits for it to become 0 before reading (or to become 1 when another byte is expected)
    bit 2 = data out
    bit 3   unused
    bit 4 = data in
    bit 5 = clock
    bit 6   n.c.
    bit 7   n.c.

galgame3:
    bit 0 = clock
    bit 1   unused
    bit 2 = data in
    bit 3 = data out
    bit 4 = PIC waits for it to become 0 before reading (or to become 1 when another byte is expected)
    bit 5 = 0
    bit 6 = 1
    bit 7   unused
*/
void galgames_cart_device::set_pic_bits(int clk, int in, int out, int dis)
{
	m_pic_clk_mask  = 1 << clk;
	m_pic_in_mask   = 1 << in;
	m_pic_out_mask  = 1 << out;
	m_pic_dis_mask  = 1 << dis;
}

u8 galgames_cart_device::int_pic_data_r()
{
	if (!machine().side_effects_disabled())
	{
		// clock
		u8 clk0 = m_pic_data_clk & 0x80;
		m_pic_data_clk += 0x10;
		u8 clk1 = m_pic_data_clk & 0x80;

		m_pic_iobits = (m_pic_iobits & (~m_pic_clk_mask)) | (clk1 ? m_pic_clk_mask : 0);

		// disabled
		bool disabled = !is_selected();
		m_pic_iobits = (m_pic_iobits & (~m_pic_dis_mask)) | (disabled ? m_pic_dis_mask : 0);

		// The PIC waits for a falling edge before reading the new input bit.
		// It waits for a rising edge before setting the new output bit.
		// Hence we shift the data on the falling edge of the clock.
		if (clk0 && !clk1)
		{
			u8 bit_in = 0;

			if (m_pic_data_rdy == 1)
			{
				if (m_pic_data_bit == 0xff)
				{
					// first read bit must be 1 (sync)
					bit_in = 1;
					m_pic_data_bit = 7;
				}
				else
				{
					// read current bit and move to the next
					bit_in = BIT(m_pic_data, m_pic_data_bit);
					--m_pic_data_bit;

					if (m_pic_data_bit == 0xff)
					{
						m_pic_data_rdy = 0;
						log_cart_comm("PIC should have READ", m_pic_data);
					}
				}
			}
			else if (m_pic_data_rdy == 0)
			{
				u8 bit_out = m_pic_iobits & m_pic_out_mask;

				if (m_pic_data_bit == 0xff)
				{
					// first written bit must be 1 (sync)
					if (bit_out)
						m_pic_data_bit = 7;
				}
				else
				{
					// write current bit and move to the next
					u8 mask = 1 << m_pic_data_bit;
					m_pic_data = (m_pic_data & (~mask)) | (bit_out ? mask : 0);
					--m_pic_data_bit;

					if (m_pic_data_bit == 0xff)
					{
						m_pic_data_rdy = 2;
						log_cart_comm("PIC should have WRITTEN", m_pic_data);
					}
				}
			}

			m_pic_iobits = (m_pic_iobits & (~m_pic_in_mask)) | (bit_in ? m_pic_in_mask : 0);
		}

	//  log_cart_comm("PIC READ", m_pic_iobits);
	}
	return m_pic_iobits;
}

void galgames_cart_device::int_pic_data_w(u8 data)
{
	m_pic_iobits = (m_pic_iobits & (~m_pic_out_mask)) | (data & m_pic_out_mask);

//  log_cart_comm("PIC WRITE", data);
}

/*
galgame3, port A:
    bit 2 = bank lsb
    bit 3 = bank msb
*/
void galgames_cart_device::int_pic_bank_w(u8 data)
{
	set_rom_bank((data >> 2) & 3);
}


// External EEPROM interface

u8 galgames_cart_device::eeprom_r()
{
	return (m_eeprom && m_eeprom->do_read()) ? 0x80 : 0x00;
}

void galgames_cart_device::eeprom_w(u8 data)
{
	if (!m_eeprom)
		return;

	if (data & ~0x03)
		logerror("Unknown EEPROM bit written %04X\n", data);

	// latch the bit
	m_eeprom->di_write(data & 0x01);

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
}

void galgames_cart_device::eeprom_cs_write(int state)
{
	if (!m_eeprom)
		return;

	m_eeprom->cs_write(state);
}


// SLOT implementation

void galgames_slot_device::slot_map(address_map &map)
{
	map(0x000000, 0x1fffff).r(FUNC(galgames_slot_device::rom0_r));
	map(0x000000, 0x03ffff).rw(FUNC(galgames_slot_device::rom0_or_ram_r), FUNC(galgames_slot_device::ram_w)).share("ram");
	map(0x200000, 0x3fffff).r(FUNC(galgames_slot_device::rom_r));
	map(0x200000, 0x23ffff).rw(FUNC(galgames_slot_device::rom_or_ram_r), FUNC(galgames_slot_device::ram_w));
}

galgames_slot_device::galgames_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, GALGAMES_SLOT, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_space_config("slot_space", ENDIANNESS_BIG, 16,22, 0, address_map_constructor(FUNC(galgames_slot_device::slot_map), this)),
	m_ram(*this, "ram"),
	m_cart0(*this, "^cart0"),
	m_cart1(*this, "^cart1"),
	m_cart2(*this, "^cart2"),
	m_cart3(*this, "^cart3"),
	m_cart4(*this, "^cart4")
{
}

void galgames_slot_device::device_start()
{
	m_space = &space(AS_PROGRAM);

	m_carts[0] = m_cart0;
	m_carts[1] = m_cart1;
	m_carts[2] = m_cart2;
	m_carts[3] = m_cart3;
	m_carts[4] = m_cart4;

	save_item(NAME(m_cart));
	save_item(NAME(m_is_ram_active));
}

void galgames_slot_device::device_reset()
{
	m_is_ram_active = false;

	set_cart(0);
	reset_eeproms_except(-1);
}

void galgames_slot_device::set_cart(int cart)
{
//  if (m_cart != cart)
//      logerror("%s: cart sel = %02x\n", machine().describe_context(), cart);

	m_cart = cart;
}

void galgames_slot_device::reset_eeproms_except(int cart)
{
	// Reset all eeproms except the selected one
	for (int i = 0; i < 5; i++)
		m_carts[i]->eeprom_cs_write((cart == i) ? ASSERT_LINE : CLEAR_LINE);
}

void galgames_slot_device::cart_sel_w(u8 data)
{
	// cart selection (0 1 2 3 4 7)

	switch (data)
	{
		case 0x07:  // 7 resets all
			reset_eeproms_except(-1);
			break;

		case 0x00:  // cart 0 (motherboard)
		case 0x01:  // cart 1
		case 0x02:  // cart 2
		case 0x03:  // cart 3
		case 0x04:  // cart 4
			set_cart(data);
			reset_eeproms_except(data);
			break;

		default:
			set_cart(0);
			reset_eeproms_except(0);
			logerror("%s: unknown cart sel = %02x\n", machine().describe_context(), data);
			break;
	}
}

// Select RAM, reset PIC

void galgames_slot_device::ram_sel_w(u8 data)
{
	// bit 3 = PIC reset (active low)
	set_pic_reset_line(BIT(data, 3) ? CLEAR_LINE : ASSERT_LINE);

	// ROM/RAM banking
	if ((data & 0xf7) == 0x05)
	{
		m_is_ram_active = true;
//      logerror("%s: romram bank = %02x\n", machine().describe_context(), data);
	}
}

// External ROM read

u16 galgames_slot_device::rom0_r(offs_t offset)
{
	return m_carts[0]->rom_r(offset);
}
u16 galgames_slot_device::rom_r(offs_t offset)
{
	return m_carts[m_cart]->rom_r(offset);
}

u16 galgames_slot_device::rom0_or_ram_r(offs_t offset)
{
	return m_is_ram_active ? m_ram[offset] : m_carts[0]->rom_r(offset);
}
u16 galgames_slot_device::rom_or_ram_r(offs_t offset)
{
	return !m_is_ram_active ? m_ram[offset] : m_carts[m_cart]->rom_r(offset);
}

void galgames_slot_device::ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(m_ram + offset);
}

// External PIC status and data interface

u8 galgames_slot_device::pic_status_r()
{
	return m_carts[m_cart]->pic_status_r();
}
u8 galgames_slot_device::pic_data_r()
{
	return m_carts[m_cart]->pic_data_r();
}
void galgames_slot_device::pic_data_w(u8 data)
{
	m_carts[m_cart]->pic_data_w(data);
}

void galgames_slot_device::set_pic_reset_line(int state)
{
	m_carts[m_cart]->set_pic_reset_line(state);
}

// External EEPROM interface

u8 galgames_slot_device::eeprom_r()
{
	return m_carts[m_cart]->eeprom_r();
}
void galgames_slot_device::eeprom_w(u8 data)
{
	m_carts[m_cart]->eeprom_w(data);
}
void galgames_slot_device::eeprom_cs_write(int state)
{
	m_carts[m_cart]->eeprom_cs_write(state);
}


namespace {

/***************************************************************************

                                 General

***************************************************************************/

class galgames_state : public driver_device
{
public:
	galgames_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_blitter(*this, "blitter"),
		m_oki(*this, "oki"),
		m_slot(*this, "slot"),
		m_okiram(*this, "okiram")
	{ }

	void blitter_irq_callback(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_interrupt);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void palette_offset_w(u8 data);
	void palette_data_w(u8 data);

	u8 okiram_r(offs_t offset);
	void okiram_w(offs_t offset, u8 data);

	u16 fpga_status_r();
	void outputs_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void galgames_base(machine_config &config);
	void galgbios(machine_config &config);
	void galgame2(machine_config &config);
	void galgame3(machine_config &config);
	void blitter_map(address_map &map) ATTR_COLD;
	void galgames_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<cesblit_device> m_blitter;
	required_device<okim6295_device> m_oki;
	required_device<galgames_slot_device> m_slot;
	required_shared_ptr<u8> m_okiram;

	u8 m_palette_offset = 0;
	u8 m_palette_index = 0;
	u8 m_palette_data[3]{};
};

void galgames_state::blitter_irq_callback(int state)
{
//  logerror("%s: Blitter IRQ callback state = %x\n", machine().describe_context(), state);
	m_maincpu->set_input_line(2, state);
}

/***************************************************************************

                                   Video

***************************************************************************/

u32 galgames_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	return m_blitter->screen_update(screen, bitmap, cliprect);
}

// BT481A Palette RAMDAC

void galgames_state::palette_offset_w(u8 data)
{
	m_palette_offset = data & 0xff;
	m_palette_index = 0;
}

void galgames_state::palette_data_w(u8 data)
{
	m_palette_data[m_palette_index] = data & 0xff;
	if (++m_palette_index == 3)
	{
		for (int palette_base = 0; palette_base < 0x1000; palette_base += 0x100)
			m_palette->set_pen_color(m_palette_offset + palette_base, rgb_t(m_palette_data[0], m_palette_data[1], m_palette_data[2]));
		m_palette_index = 0;
		m_palette_offset++;
	}
}

void galgames_state::video_start()
{
	save_item(NAME(m_palette_offset));
	save_item(NAME(m_palette_index));
	save_item(NAME(m_palette_data));
}

/***************************************************************************

                                   Sound

***************************************************************************/

u8 galgames_state::okiram_r(offs_t offset)
{
	return m_okiram[offset];
}

void galgames_state::okiram_w(offs_t offset, u8 data)
{
	m_okiram[offset] = data;
}

/***************************************************************************

                                Memory Maps

***************************************************************************/

// Outputs

void galgames_state::outputs_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (data & mem_mask & ~0x000f)
		logerror("%s: Unknown output bit written %04X\n", machine().describe_context(), data);

	if (ACCESSING_BITS_0_7)
	{
		// bit 0 & 1 : ? always on (may be trackball lights or coin lockouts)
		// bit 2     : coin counter
		// bit 3     : ? set after selecting a game (mostly off in-game)
		machine().bookkeeping().coin_counter_w(0, data & 0x0004);
	}

//  popmessage("OUT %02X", data & mem_mask);
}

// FPGA

u16 galgames_state::fpga_status_r()
{
	return 0x3; // Pass the check at PC = 0xfae & a later one
}

void galgames_state::galgames_map(address_map &map)
{
	map(0x000000, 0x3fffff).rw(m_slot, FUNC(galgames_slot_device::read), FUNC(galgames_slot_device::write));

	map(0x400000, 0x400011).w(m_blitter, FUNC(cesblit_device::regs_w));
	map(0x400012, 0x400013).w(m_blitter, FUNC(cesblit_device::addr_hi_w));
	map(0x400014, 0x400015).w(m_blitter, FUNC(cesblit_device::color_w));
	map(0x400020, 0x400021).r(m_blitter, FUNC(cesblit_device::status_r));

	map(0x600000, 0x600001).r(FUNC(galgames_state::fpga_status_r));
	map(0x700000, 0x700001).r(FUNC(galgames_state::fpga_status_r)).nopw();
	map(0x800020, 0x80003f).noprw();   // ?
	map(0x900000, 0x900001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));

	map(0xa00001, 0xa00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xb00000, 0xb7ffff).rw(FUNC(galgames_state::okiram_r), FUNC(galgames_state::okiram_w)).umask16(0x00ff); // (only low bytes tested) 4x N341024SJ-15

	map(0xc00001, 0xc00001).w(FUNC(galgames_state::palette_offset_w));
	map(0xc00003, 0xc00003).w(FUNC(galgames_state::palette_data_w));

	map(0xd00000, 0xd00001).portr("TRACKBALL_1_X");
	map(0xd00000, 0xd00001).nopw();  // bit 0: FPGA programming serial in (lsb first)
	map(0xd00002, 0xd00003).portr("TRACKBALL_1_Y");
	map(0xd00004, 0xd00005).portr("TRACKBALL_2_X");
	map(0xd00006, 0xd00007).portr("TRACKBALL_2_Y");
	map(0xd00008, 0xd00009).portr("P1");
	map(0xd0000a, 0xd0000b).portr("P2");
	map(0xd0000c, 0xd0000d).portr("SYSTEM").w(FUNC(galgames_state::outputs_w));

	map(0xd0000e, 0xd0000f).nopr();
	map(0xd0000f, 0xd0000f).w(m_slot, FUNC(galgames_slot_device::cart_sel_w));
	map(0xd00011, 0xd00011).rw(m_slot, FUNC(galgames_slot_device::eeprom_r), FUNC(galgames_slot_device::eeprom_w));
	map(0xd00013, 0xd00013).rw(m_slot, FUNC(galgames_slot_device::pic_data_r), FUNC(galgames_slot_device::pic_data_w));
	map(0xd00015, 0xd00015).rw(m_slot, FUNC(galgames_slot_device::pic_status_r), FUNC(galgames_slot_device::ram_sel_w));
}

void galgames_state::blitter_map(address_map &map)
{
	map(0x000000, 0x1fffff).r(":slot", FUNC(galgames_slot_device::rom_r));
}

void galgames_state::oki_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram().share("okiram");
}

/***************************************************************************

                                Input Ports

***************************************************************************/

static INPUT_PORTS_START(galgames)
	PORT_START("P1")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)   // Button A (right)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)   // Button B (left)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("P2")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)   // Button A (right)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)   // Button B (left)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_SERVICE1)                  // DBA (coin)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SYSTEM")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_COIN1)    // CS 1 (coin)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_COIN2)    // CS 2 (coin)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH,IPT_OTHER) PORT_CODE(KEYCODE_F2) PORT_NAME("Service Mode (Coin Door)") PORT_TOGGLE
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("TRACKBALL_1_X")
	PORT_BIT(0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(1) PORT_RESET

	PORT_START("TRACKBALL_1_Y")
	PORT_BIT(0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(1) PORT_RESET

	PORT_START("TRACKBALL_2_X")
	PORT_BIT(0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(2) PORT_RESET

	PORT_START("TRACKBALL_2_Y")
	PORT_BIT(0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(2) PORT_RESET
INPUT_PORTS_END

/***************************************************************************

                               Machine Drivers

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(galgames_state::scanline_interrupt)
{
	int scanline = param;

	if (scanline == 0) // vblank, FIXME
		m_maincpu->set_input_line(3, HOLD_LINE);
	else if ((scanline % 16) == 0)
		m_maincpu->set_input_line(1, HOLD_LINE);

	// lev 2 triggered at the end of the blit
}

int galgames_compute_addr(u16 reg_low, u16 reg_mid, u16 reg_high)
{
	return reg_low | (reg_mid << 16);
}

void galgames_state::galgames_base(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &galgames_state::galgames_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(galgames_state::scanline_interrupt), "screen", 0, 1);
	WATCHDOG_TIMER(config, "watchdog");

	GALGAMES_SLOT(config, m_slot, 0);
	GALGAMES_BIOS_CART(config, "cart0", 0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(400, 256);
	m_screen->set_visarea(0, 400-1, 0, 256-1);
	m_screen->set_screen_update(FUNC(galgames_state::screen_update));
	m_screen->set_palette(m_palette);

	CESBLIT(config, m_blitter, XTAL(24'000'000), m_screen);
	m_blitter->set_addrmap(AS_PROGRAM, &galgames_state::blitter_map);
	m_blitter->set_compute_addr(galgames_compute_addr);
	m_blitter->irq_callback().set(FUNC(galgames_state::blitter_irq_callback));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x1000); // only 0x100 used

	// sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki, XTAL(24'000'000) / 16, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified (voices in galgame4 seem ok)
	m_oki->set_addrmap(0, &galgames_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void galgames_state::galgbios(machine_config &config)
{
	galgames_base(config);
	GALGAMES_CART(config, "cart1", 1);
	GALGAMES_CART(config, "cart2", 2);
	GALGAMES_CART(config, "cart3", 3);
	GALGAMES_CART(config, "cart4", 4);
}

void galgames_state::galgame2(machine_config &config)
{
	galgames_base(config);
	GALGAMES_STARPAK2_CART(config, "cart1", 1);
	GALGAMES_CART(config, "cart2", 2);
	GALGAMES_CART(config, "cart3", 3);
	GALGAMES_CART(config, "cart4", 4);
}

void galgames_state::galgame3(machine_config &config)
{
	galgames_base(config);
	GALGAMES_STARPAK3_CART(config, "cart1", 1);
	GALGAMES_CART(config, "cart2", 2);
	GALGAMES_CART(config, "cart3", 3);
	GALGAMES_CART(config, "cart4", 4);
}

/***************************************************************************

                               ROMs Loading

***************************************************************************/


/***************************************************************************

Galaxy Games BIOS

This is a multi-game cocktail cabinet released in 1998. Namco and Atari
licensed their IP for some cartridges for it.

Trackball-based. 'BIOS' has 7 built-in games. There are two LEDs on the PCB.

More information here : http://www.cesgames.com/museum/galaxy/index.html

----

Board silkscreened  237-0211-00
                    REV.-D

Cartridge based mother board
Holds up to 4 cartridges
Chips labeled
    GALAXY U1 V1.90 12/1/98
    GALAXY U2 V1.90 12/1/98

Motorola MC68HC000FN12
24 MHz oscillator
Xilinx XC5206
Xilinx XC5202
BT481AKPJ110 (Palette RAMDAC)
NKK N341024SJ-15    x8  (128kB RAM)
OKI M6295 8092352-2

PAL16V8H-15 @ U24   Blue dot on it
PAL16V8H-15 @ U25   Yellow dot on it
PAL16V8H-15 @ U26   Red dot on it
PAL16V8H-15 @ U27   Green dot on it
PAL16V8H-15 @ U45   Magenta dot on it

Copyright notice in rom states: Creative Electronics & Software Written by Keith M. Kolmos May 29,1998

***************************************************************************/

#define ROM_LOAD16_BYTE_BIOS( bios, name, offset, length, hash ) \
	ROMX_LOAD( name, offset, length, hash, ROM_SKIP(1) | ROM_BIOS(bios) )

#define GALGAMES_BIOS_ROMS \
	ROM_SYSTEM_BIOS( 0, "1.90",   "v1.90 12/01/98" ) \
	ROM_LOAD16_BYTE_BIOS( 0, "galaxy_u2__v1.90_12-01-98.u2", 0x000000, 0x100000, CRC(e51ff184) SHA1(aaa795f2c15ec29b3ceeb5c917b643db0dbb7083) ) \
	ROM_LOAD16_BYTE_BIOS( 0, "galaxy_u1__v1.90_12-01-98.u1", 0x000001, 0x100000, CRC(c6d7bc6d) SHA1(93c032f9aa38cbbdda59a8a25ff9f38f7ad9c760) ) \
	\
	ROM_SYSTEM_BIOS( 1, "1.80",   "v1.80 10/05/98" ) \
	ROM_LOAD16_BYTE_BIOS( 1, "galaxy_u2__v1.80_10-15-98.u2", 0x000000, 0x100000, CRC(73cff284) SHA1(e6f7d92999cdb478c21c3b65a04eade84299ac12) ) \
	ROM_LOAD16_BYTE_BIOS( 1, "galaxy_u1__v1.80_10-15-98.u1", 0x000001, 0x100000, CRC(e3ae423c) SHA1(66d1964845a99a5ed4b19b4135b55cde6b5fe295) )

#define GALGAMES_MB_PALS \
	ROM_REGION( 0xa00, "pals", 0 ) \
	ROM_LOAD( "16v8h-blue.u24",    0x000, 0x117, NO_DUMP) \
	ROM_LOAD( "16v8h-yellow.u25",  0x200, 0x117, NO_DUMP) \
	ROM_LOAD( "16v8h-magenta.u26", 0x400, 0x117, NO_DUMP) \
	ROM_LOAD( "16v8h-green.u27",   0x600, 0x117, NO_DUMP) \
	ROM_LOAD( "16v8h-red.u45",     0x800, 0x117, NO_DUMP)

ROM_START( galgbios )
	ROM_REGION16_BE( 0x200000, "cart0", 0 )
	GALGAMES_BIOS_ROMS

	ROM_REGION(0x200000, "cart1", ROMREGION_ERASEFF)
	ROM_REGION(0x200000, "cart2", ROMREGION_ERASEFF)
	ROM_REGION(0x200000, "cart3", ROMREGION_ERASEFF)
	ROM_REGION(0x200000, "cart4", ROMREGION_ERASEFF)

	GALGAMES_MB_PALS
ROM_END

/***************************************************************************

Galaxy Games StarPak 2

Cartridge with 7 games, including Namco licensed Pac-Man & Ms. Pac-Man.

AKA NAMCO 307 Cartridge

.U1 AM29F800BB
.U2 AM29F800BB
.U3 93AA76/SN
.U4 PIC 12C508
.L1 Led

Board silkscreened  237-0209-00
                    REV.-C

***************************************************************************/

ROM_START( galgame2 )
	ROM_REGION16_BE( 0x200000, "cart0", 0 )
	GALGAMES_BIOS_ROMS

	ROM_REGION( 0x200000, "cart1", 0 )
	ROM_LOAD16_BYTE( "am29f800bb.u2", 0x000000, 0x100000, CRC(f43c0c54) SHA1(4a13946c3d173b0e4ab25b01849574fa3302b417) ) // MAY 29, 1998
	ROM_LOAD16_BYTE( "am29f800bb.u1", 0x000001, 0x100000, CRC(b8c34a8b) SHA1(40d3b35f573d2bd2ae1c7d876c55fc436864fa3f) ) // ""

	ROM_REGION( 0x2000, "cart1:pic", 0 )
	ROM_LOAD( "pic12c508.u4", 0x0000, 0x2000, CRC(bb253913) SHA1(eace069344da6dda7c05673e422876d130ed5d48) )  // includes config word at fff, hence size is 2*1000

	ROM_REGION( 0x200000, "cart2", ROMREGION_ERASEFF )
	ROM_REGION( 0x200000, "cart3", ROMREGION_ERASEFF )
	ROM_REGION( 0x200000, "cart4", ROMREGION_ERASEFF )

	GALGAMES_MB_PALS
ROM_END

/***************************************************************************

Galaxy Games StarPak 3

Cartridge with 8 games, including Atari licensed Centipede.

.U1 AM29F032B
.U2 AM29F032B
.U5 93AA76/SN
.U6 PIC 16C56-XT/P
.L1 Led

Board silkscreened  237-0228-00
                    REV.-B

***************************************************************************/

ROM_START( galgame3 )
	ROM_REGION16_BE( 0x200000, "cart0", 0 )
	GALGAMES_BIOS_ROMS

	ROM_REGION( 0x800000, "cart1", 0 )
	ROM_LOAD16_BYTE( "am29f032b.u2", 0x000000, 0x400000, CRC(a4ffc70a) SHA1(328c6ceef025af7ff5b7998df59a10d90c654d53) )
	ROM_LOAD16_BYTE( "am29f032b.u1", 0x000001, 0x400000, CRC(b0876751) SHA1(487f052989e4b2df2df2293b283e8e03ffc3ddf4) )

	ROM_REGION( 0x800, "cart1:pic", 0 )
	ROM_LOAD( "pic16c56.u6", 0x000, 0x800, CRC(cf901ed8) SHA1(ebb2da0f50ba82a038f315aab7e6b20b9a1af3a1) )

	ROM_REGION( 0x200000, "cart2", ROMREGION_ERASEFF )
	ROM_REGION( 0x200000, "cart3", ROMREGION_ERASEFF )
	ROM_REGION( 0x200000, "cart4", ROMREGION_ERASEFF )

	GALGAMES_MB_PALS
ROM_END

/***************************************************************************

Galaxy Games StarPak 4

Cartridge with 7 games, including Berzerk.

.U1 AM29F032B
.U2 AM29F032B
.U5 93AA76/SN
.U6 PIC 16C56-XT/P
.L1 Led

Board silkscreened  237-0228-00
                    REV.-B

NOTE: Unlike previous cartridges, there is no licensing information shown for
      Berzerk as was done for Pac-Man / Ms. Pac-Man in StarPak 2 and Centipede
      in StarPak 3

***************************************************************************/

ROM_START( galgame4 )
	ROM_REGION16_BE( 0x200000, "cart0", 0 )
	GALGAMES_BIOS_ROMS

	ROM_REGION( 0x800000, "cart1", 0 )
	ROM_LOAD16_BYTE( "am29f032b.u2", 0x000000, 0x400000, CRC(60f14d02) SHA1(581511898338246476ac8359a7427ffed26e233e) ) // JANUARY 12, 1998
	ROM_LOAD16_BYTE( "am29f032b.u1", 0x000001, 0x400000, CRC(9dc6c588) SHA1(a242de749a563cb26fce6901f202d5fc4ae1beb0) ) // ""

	ROM_REGION( 0x2000, "cart1:pic", 0 )
	ROM_LOAD( "pic16c56.u6", 0x000, 0x2000, CRC(ea994ab7) SHA1(4d59355f11e86f43e6a553140fb89aebbd8981a6) ) // unprotected

	ROM_REGION( 0x200000, "cart2", ROMREGION_ERASEFF )
	ROM_REGION( 0x200000, "cart3", ROMREGION_ERASEFF )
	ROM_REGION( 0x200000, "cart4", ROMREGION_ERASEFF )

	GALGAMES_MB_PALS
ROM_END

/***************************************************************************

Galaxy Games StarPak 4 (prototype)

Files from a dev board provided by the developer.

NOTE: The graphics tiles are misaligned for the games Star Tiger and Battle
      Checkers for this prototype set. This isn't an issue with the release
      version above.

NOTE: PIC images are NOT interchangable between release and prototype sets.

***************************************************************************/

ROM_START( galgame4p )
	ROM_REGION16_BE( 0x200000, "cart0", 0 )
	GALGAMES_BIOS_ROMS

	ROM_REGION( 0x800000, "cart1", 0 )
	ROM_LOAD16_BYTE( "sp4.u2",  0x000000, 0x100000, CRC(e51bc5e1) SHA1(dacf6cefd792713b34382b827952b66e2cb5c2b4) ) // JANUARY 12, 1998
	ROM_LOAD16_BYTE( "sp4.u1",  0x000001, 0x100000, CRC(695ab775) SHA1(e88d5f982df19e70be6124e6fdf20830475641e0) ) // ""
	ROM_LOAD16_BYTE( "sp4.u6",  0x200000, 0x100000, CRC(7716895d) SHA1(8f86ffe2d94d3e756a3b7661d480e3a8c53cf178) )
	ROM_LOAD16_BYTE( "sp4.u5",  0x200001, 0x100000, CRC(6c699ba3) SHA1(f675997e1b808758f79a21b883161526242990b4) )
	ROM_LOAD16_BYTE( "sp4.u8",  0x400000, 0x100000, CRC(cdf45446) SHA1(da4e1667c7c47239e770018a7d3b8c1e4e2f4a63) )
	ROM_LOAD16_BYTE( "sp4.u7",  0x400001, 0x100000, CRC(813c46c8) SHA1(3fd4192ec7e8d5e6bfbc2a37d9b4bbebe6132b99) )
	ROM_LOAD16_BYTE( "sp4.u10", 0x600000, 0x100000, CRC(52dbf088) SHA1(da7c37366e884f40f1dea243d4aea0b2d2b314db) )
	ROM_LOAD16_BYTE( "sp4.u9",  0x600001, 0x100000, CRC(9ded1dc2) SHA1(5319edfccf47d02dfd3664cb3782cc2281c769c4) )

	ROM_REGION( 0x2000, "cart1:pic", 0 )
	ROM_LOAD( "sp4.pic", 0x000, 0x2000, CRC(008ef1ba) SHA1(4065fcf00922de3e629084f4f4815355f271c954) )

	ROM_REGION( 0x200000, "cart2", ROMREGION_ERASEFF )
	ROM_REGION( 0x200000, "cart3", ROMREGION_ERASEFF )
	ROM_REGION( 0x200000, "cart4", ROMREGION_ERASEFF )

	GALGAMES_MB_PALS
ROM_END

} // anonymous namespace


GAME(1998, galgbios,  0,        galgbios, galgames, galgames_state, empty_init, ROT0, "Creative Electronics & Software",         "Galaxy Games BIOS",                  MACHINE_IS_BIOS_ROOT)
GAME(1998, galgame2,  galgbios, galgame2, galgames, galgames_state, empty_init, ROT0, "Creative Electronics & Software / Namco", "Galaxy Games StarPak 2",             0)
GAME(1998, galgame3,  galgbios, galgame3, galgames, galgames_state, empty_init, ROT0, "Creative Electronics & Software / Atari", "Galaxy Games StarPak 3",             0)
GAME(1998, galgame4,  galgbios, galgame3, galgames, galgames_state, empty_init, ROT0, "Creative Electronics & Software",         "Galaxy Games StarPak 4",             0)
GAME(1998, galgame4p, galgame4, galgame3, galgames, galgames_state, empty_init, ROT0, "Creative Electronics & Software",         "Galaxy Games StarPak 4 (prototype)", MACHINE_IMPERFECT_GRAPHICS)
