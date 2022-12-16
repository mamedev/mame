// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Wilbert Pol
/***********************************************************************************************************

 Game Boy cart emulation


 Here we emulate carts without RAM banking.


 TODO:
 - YongYong:
   - During start there are 2 writes to 5000 and 5003, it is still unknown what these do.
 - Story of Lasama:
   - This should display the Gowin logo on boot on both DMG and CGB (not implemented yet)
 - Sachen MMC2:
   - Does it monitor the address to detect RAM writes for logo spoofing, or just /CS?
 - ATV Racing/Rocket Games:
   - Does it monitor the address to detect RAM writes for logo spoofing, or just /CS?
   - How does RAM banking work?  Are over-size RAM chips just inaccessible?

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"

#include "cartbase.ipp"

#include "bus/generic/slot.h"

#include "corestr.h"

#include <memory>
#include <optional>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

//**************************************************************************
//  Sachen MMC1/MMC2 ROM banking helper class (16 KiB * 256)
//**************************************************************************

class sachen_mmc_device_base : public mbc_dual_uniform_device_base
{
protected:
	sachen_mmc_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
		mbc_dual_uniform_device_base(mconfig, type, tag, owner, clock),
		m_bank_mux_rom(0U),
		m_bank_sel_rom{ 0U, 0U }
	{
	}

	virtual void device_start() override ATTR_COLD
	{
		mbc_dual_uniform_device_base::device_start();

		save_item(NAME(m_bank_mux_rom));
		save_item(NAME(m_bank_sel_rom));
	}

	virtual void device_reset() override ATTR_COLD
	{
		mbc_dual_uniform_device_base::device_reset();

		m_bank_mux_rom = 0U;
		m_bank_sel_rom[0] = 0U;
		m_bank_sel_rom[1] = 1U;
		update_banks();
	}

	bool check_rom(std::string &message) ATTR_COLD
	{
		set_bank_bits_rom(8);
		return mbc_dual_uniform_device_base::check_rom(message);
	}

	void install_bank_switch_handlers() ATTR_COLD
	{
		// A5, A3 and A2 are not connected to the controller (effective address mask 0xffd3)
		cart_space()->install_write_handler(0x0000, 0x1fff, write8smo_delegate(*this, FUNC(sachen_mmc_device_base::bank_rom_switch_outer)));
		cart_space()->install_write_handler(0x2000, 0x3fff, write8smo_delegate(*this, FUNC(sachen_mmc_device_base::bank_rom_switch_inner)));
		cart_space()->install_write_handler(0x4000, 0x5fff, write8smo_delegate(*this, FUNC(sachen_mmc_device_base::bank_rom_mux)));
	}

	u8 rom_read(offs_t offset, bool spoof)
	{
		// holds A7 high when spoofing the logo
		if (spoof)
			offset |= 0x0080;

		// cartridge header area has address lines scrambled - A6 swapped with A0 and A4 swapped with A1
		if (0x0100 == (offset & 0xff00))
			offset = (offset & 0xff80) | bitswap<7>(offset, 0, 5, 1, 3, 2, 4, 6);

		return (BIT(offset, 14) ? bank_rom_high_base() : bank_rom_low_base())[offset & 0x3fff];
	}

private:
	void bank_rom_switch_outer(u8 data)
	{
		if (0x30 == (m_bank_sel_rom[1] & 0x30))
		{
			m_bank_sel_rom[0] = data;
			update_banks();
		}
	}

	void bank_rom_switch_inner(u8 data)
	{
		m_bank_sel_rom[1] = data ? data : 1;
		u16 const hi(bank_rom_entry_high());
		LOG(
				"%s: Set ROM bank high = 0x%06X (0x%02X & 0x%02X | 0x%02X & 0x%02X)\n",
				machine().describe_context(),
				u32(hi) * 0x4000,
				m_bank_sel_rom[0],
				m_bank_mux_rom,
				m_bank_sel_rom[1],
				~m_bank_mux_rom);
		set_bank_rom_high(hi);
	}

	void bank_rom_mux(u8 data)
	{
		if (0x30 == (m_bank_sel_rom[1] & 0x30))
		{
			m_bank_mux_rom = data;
			update_banks();
		}
	}

	u8 bank_rom_entry_low() const
	{
		return m_bank_sel_rom[0] & m_bank_mux_rom;
	}

	u8 bank_rom_entry_high() const
	{
		return (m_bank_sel_rom[0] & m_bank_mux_rom) | (m_bank_sel_rom[1] & ~m_bank_mux_rom);
	}

	void update_banks()
	{
		u16 const lo(bank_rom_entry_low());
		u16 const hi(bank_rom_entry_high());
		LOG(
				"%s: Set ROM banks low = 0x%1$06X (0x%3$02X & 0x%5$02X), high = 0x%2$06X (0x%3$02X & 0x%5$02X | 0x%4$02X & 0x%6$02X)\n",
				machine().describe_context(),
				u32(lo) * 0x4000,
				u32(hi) * 0x4000,
				m_bank_sel_rom[0],
				m_bank_sel_rom[1],
				m_bank_mux_rom,
				~m_bank_mux_rom);
		set_bank_rom_low(lo);
		set_bank_rom_high(hi);
	}

	u8 m_bank_mux_rom;
	u8 m_bank_sel_rom[2];
};



//**************************************************************************
//  Game Boy Color logo spoof used by Sachen and Rocket Games
//**************************************************************************

template <typename Base>
class gbc_logo_spoof_device_base : public Base
{
public:
	using Base::logerror;

protected:
	gbc_logo_spoof_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
		Base(mconfig, type, tag, owner, clock),
		m_ram_tap(),
		m_notif_cart_space(),
		m_counter(0U),
		m_spoof_logo(0U),
		m_installing_tap(false)
	{
	}

	virtual void device_start() override ATTR_COLD
	{
		Base::device_start();

		this->save_item(NAME(m_counter));
		this->save_item(NAME(m_spoof_logo));
	}

	virtual void device_reset() override ATTR_COLD
	{
		Base::device_reset();

		m_counter = 0U;
		m_spoof_logo = 0U;

		install_ram_tap();
		m_notif_cart_space = this->cart_space()->add_change_notifier(
				[this] (read_or_write mode)
				{
					if (u32(mode) & u32(read_or_write::WRITE))
						install_ram_tap();
				});
	}

	bool spoof_logo() const
	{
		return 1U == m_spoof_logo;
	}

	void rom_access()
	{
		if (!this->machine().side_effects_disabled())
		{
			// These cartridges work by counting low-to-high transitions on A15.
			// CPU keeps A15 high while reading internal bootstrap ROM.
			// Doing this on ROM reads is a good enough approximation for it to work.
			if (0x30 != m_counter)
			{
				++m_counter;
			}
			else
			{
				switch (m_spoof_logo)
				{
				case 0U:
					m_counter = 1U;
					[[fallthrough]];
				case 1U:
					++m_spoof_logo;
					m_notif_cart_space.reset();
					m_ram_tap.remove();
					LOG(
							"%s: Spoof logo step %u\n",
							this->machine().describe_context(),
							m_spoof_logo);
				}
			}
		}
	}

private:
	void install_ram_tap()
	{
		if (!m_installing_tap)
		{
			m_installing_tap = true;
			m_ram_tap.remove();
			if (!m_spoof_logo)
			{
				m_ram_tap = this->cart_space()->install_write_tap(
						0xc000, 0xdfff,
						"ram_tap_w",
						[this] (offs_t offset, u8 &data, u8 mem_mask)
						{
							assert(!m_spoof_logo);

							if (!this->machine().side_effects_disabled())
							{
								m_counter = 0U;
								++m_spoof_logo;
								LOG(
										"%s: Spoof logo step %u\n",
										this->machine().describe_context(),
										m_spoof_logo);
								m_notif_cart_space.reset();
								m_ram_tap.remove();
							}
						},
						&m_ram_tap);
			}
			else
			{
				m_notif_cart_space.reset();
			}
			m_installing_tap = false;
		}
	}

	memory_passthrough_handler m_ram_tap;
	util::notifier_subscription m_notif_cart_space;

	u8 m_counter;
	u8 m_spoof_logo;
	bool m_installing_tap;
};



//**************************************************************************
//  Mega Duck flat ROM (max 32 KiB)
//**************************************************************************

class megaduck_flat_device : public device_t, public device_gb_cart_interface
{
public:
	megaduck_flat_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		megaduck_flat_device(mconfig, MEGADUCK_ROM_STD, tag, owner, clock)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// if there's no ROM region, there's nothing to do
		memory_region *const romregion(cart_rom_region());
		if (!romregion || !romregion->bytes())
			return image_init_result::PASS;

		// check for supported size
		auto const bytes(romregion->bytes());
		if (0x8000 < bytes)
		{
			message = "Unsupported cartridge ROM size (must be no larger than 32 KiB)";
			return image_init_result::FAIL;
		}

		// install ROM
		device_generic_cart_interface::install_non_power_of_two<0>(
				bytes,
				0x7fff,
				0,
				0,
				0x0000,
				[this, base = &romregion->as_u8()] (offs_t begin, offs_t end, offs_t mirror, offs_t src)
				{
					LOG(
							"Install ROM 0x%04X-0x%04X at 0x%04X-0x%04X mirror 0x%04X\n",
							src,
							src + (end - begin),
							begin,
							end,
							mirror);
					cart_space()->install_rom(begin, end, mirror, &base[src]);
				});

		// all good
		return image_init_result::PASS;
	}

protected:
	megaduck_flat_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_gb_cart_interface(mconfig, *this)
	{
	}

	virtual void device_start() override ATTR_COLD
	{
	}
};



//**************************************************************************
//  Mega Duck banked ROM (16 KiB fixed + 16 KiB * 256 or 32 KiB * 256)
//**************************************************************************

class megaduck_banked_device : public device_t, public device_gb_cart_interface
{
public:
	megaduck_banked_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		device_t(mconfig, MEGADUCK_ROM_BANKED, tag, owner, clock),
		device_gb_cart_interface(mconfig, *this),
		m_bank_rom_low(*this, "low"),
		m_bank_rom_high(*this, "high"),
		m_bank_mask_rom{ 0U, 0U }
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// if there's no ROM region, there's nothing to do
		memory_region *const romregion(cart_rom_region());
		if (!romregion || !romregion->bytes())
		{
			// setting default bank on reset causes a fatal error when no banks are configured
			message = "No ROM data found";
			return image_init_result::FAIL;
		}

		// work out whether a fixed bank is required
		auto const bytes(romregion->bytes());
		char const *const fixedbank_explicit(get_feature("fixedbank"));
		std::optional<bool> fixedbank;
		if (fixedbank_explicit)
		{
			// explicitly specified in software list
			if (util::streqlower(fixedbank_explicit, "yes") || util::streqlower(fixedbank_explicit, "true"))
			{
				fixedbank = true;
			}
			else if (util::streqlower(fixedbank_explicit, "no") || util::streqlower(fixedbank_explicit, "false"))
			{
				fixedbank = false;
			}
			else
			{
				message = "Invalid 'fixedbank' feature value (must be yes or no)";
				return image_init_result::FAIL;
			}
		}
		else if (bytes & 0x7fff)
		{
			// if it isn't a multiple of 32 KiB, it only makes sense with a fixed bank
			fixedbank = true;
		}
		else if ((0x4000 * 0x100) < bytes)
		{
			// if it's larger than 4 MiB, it doesn't make sense with a fixed bank
			fixedbank = false;
		}

		// reject unsupported sizes
		if ((bytes & 0x3fff) || ((0x8000 * 0x100) < bytes) || (fixedbank && (*fixedbank ? ((0x4000 * 0x100) < bytes) : (bytes & 0x7fff))))
		{
			message = "Unsupported cartridge ROM size (must be a multiple of bank size and no larger than 256 entries)";
			return image_init_result::FAIL;
		}

		// configure banks - low bank has 32 KiB entries, high bank has 16 KiB entries
		auto const base(&romregion->as_u8());
		m_bank_mask_rom[0] = device_generic_cart_interface::map_non_power_of_two(
				unsigned(bytes / 0x8000),
				[this, base] (unsigned entry, unsigned page)
				{
					LOG(
							"Install ROM 0x%04X-0x%04X in low bank entry %u\n",
							page * 0x8000,
							(page * 0x8000) + 0x7fff,
							entry);
					m_bank_rom_low->configure_entry(entry, &base[page * 0x8000]);
				});
		m_bank_mask_rom[1] = device_generic_cart_interface::map_non_power_of_two(
				unsigned(bytes / 0x4000),
				[this, base] (unsigned entry, unsigned page)
				{
					LOG(
							"Install ROM 0x%04X-0x%04X in high bank entry %u\n",
							page * 0x4000,
							(page * 0x4000) + 0x3fff,
							entry);
					m_bank_rom_high->configure_entry(entry, &base[page * 0x4000]);
				});

		// install handlers
		if (!fixedbank)
		{
			// can't tell whether this cartridge has a fixed low bank, take a bet each way
			logerror("Installing banked ROM at 0x0000-0x3FFF and 0x4000-0x7FFF\n");
			cart_space()->install_read_bank(0x0000, 0x3fff, m_bank_rom_low);
			cart_space()->install_read_bank(0x4000, 0x7fff, m_bank_rom_high);
			cart_space()->install_write_handler(0x0001, 0x0001, write8smo_delegate(*this, FUNC(megaduck_banked_device::bank_rom_switch_high)));
			cart_space()->install_write_handler(0xb000, 0xb000, write8smo_delegate(*this, FUNC(megaduck_banked_device::bank_rom_switch)));
		}
		else if (*fixedbank)
		{
			// fixed ROM at 0x0000-0x3fff, banked ROM at 0x4000-0x7fff
			logerror("Installing fixed ROM at 0x0000-0x3FFF and banked ROM at 0x4000-0x7FFF\n");
			cart_space()->install_rom(0x0000, 0x3fff, base);
			cart_space()->install_read_bank(0x4000, 0x7fff, m_bank_rom_high);
			cart_space()->install_write_handler(0x0001, 0x0001, write8smo_delegate(*this, FUNC(megaduck_banked_device::bank_rom_switch_high)));
		}
		else
		{
			// banked ROM at 0x0000-0x7fff
			logerror("Installing banked ROM at 0x0000-0x7FFF\n");
			cart_space()->install_read_bank(0x0000, 0x7fff, m_bank_rom_low);
			cart_space()->install_write_handler(0xb000, 0xb000, write8smo_delegate(*this, FUNC(megaduck_banked_device::bank_rom_switch_low)));
		}

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_start() override ATTR_COLD
	{
	}

	virtual void device_reset() override ATTR_COLD
	{
		m_bank_rom_low->set_entry(0 & m_bank_mask_rom[0]);
		m_bank_rom_high->set_entry(1 & m_bank_mask_rom[1]);
	}

private:
	void bank_rom_switch_low(u8 data)
	{
		u8 const lo = data & m_bank_mask_rom[0];
		LOG(
				"%s: Set ROM bank low = 0x%04X\n",
				machine().describe_context(),
				lo * 0x8000);
		m_bank_rom_low->set_entry(lo);
	}

	void bank_rom_switch_high(u8 data)
	{
		u8 const hi = data & m_bank_mask_rom[1];
		LOG(
				"%s: Set ROM bank high = 0x%04X\n",
				machine().describe_context(),
				hi * 0x4000);
		m_bank_rom_high->set_entry(hi);
	}

	void bank_rom_switch(u8 data)
	{
		u8 const lo = data & m_bank_mask_rom[0];
		u8 const hi = ((data * 2) + 1) & m_bank_mask_rom[1];
		LOG(
				"%s: Set ROM banks low = 0x%04X, high = 0x%04X\n",
				machine().describe_context(),
				lo * 0x8000,
				hi * 0x4000);
		m_bank_rom_low->set_entry(lo);
		m_bank_rom_high->set_entry(hi);
	}

	memory_bank_creator m_bank_rom_low;
	memory_bank_creator m_bank_rom_high;
	u8 m_bank_mask_rom[2];
};



//**************************************************************************
//  Game Boy flat ROM/RAM (max 32 KiB ROM, 8 KiB RAM)
//**************************************************************************

class flat_rom_ram_device : public flat_ram_device_base<megaduck_flat_device>
{
public:
	flat_rom_ram_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		flat_ram_device_base<megaduck_flat_device>(mconfig, GB_ROM_STD, tag, owner, clock)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// first set up cartridge RAM if any
		if (!check_ram(message))
			return image_init_result::FAIL;

		// ROM is installed the same way as a Mega Duck flat ROM cartridge
		image_init_result const romresult(megaduck_flat_device::load(message));
		if (image_init_result::PASS != romresult)
			return romresult;
		install_ram();

		// all good
		return image_init_result::PASS;
	}
};



//**************************************************************************
//  M161 (32 KiB * 8)
//**************************************************************************

class m161_device : public flat_ram_device_base<banked_32k_device_base>
{
public:
	m161_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		flat_ram_device_base<banked_32k_device_base>(mconfig, GB_ROM_M161, tag, owner, clock),
		m_bank_lock(0U)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// set up ROM
		set_bank_bits_rom(8); // 74HC161A - 3-bit bank, 1-bit lockout
		if (!check_rom(message))
			return image_init_result::FAIL;
		install_rom();
		install_ram();

		// install bank switch handler
		cart_space()->install_write_handler(0x0000, 0x7fff, write8sm_delegate(*this, FUNC(m161_device::bank_rom_switch)));

		// all good
		return image_init_result::PASS;
	}

private:
	virtual void device_start() override ATTR_COLD
	{
		flat_ram_device_base<banked_32k_device_base>::device_start();

		save_item(NAME(m_bank_lock));
	}

	virtual void device_reset() override ATTR_COLD
	{
		flat_ram_device_base<banked_32k_device_base>::device_reset();

		m_bank_lock = 0U;

		set_bank_rom(0);
	}

	void bank_rom_switch(offs_t offset, u8 data)
	{
		if (!m_bank_lock)
		{
			m_bank_lock = 1U; // D connected to Vcc
			set_bank_rom(data & 0x07); // A/B/C connected to D0/D1/D2
		}
	}

	u8 m_bank_lock;
};



//**************************************************************************
//  Wisdom Tree (theoretical 32 KiB * 16,386, limited to 32 KiB * 256)
//**************************************************************************

class wisdom_device : public flat_ram_device_base<banked_32k_device_base>
{
public:
	wisdom_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		flat_ram_device_base<banked_32k_device_base>(mconfig, GB_ROM_WISDOM, tag, owner, clock)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// set up ROM
		set_bank_bits_rom(8); // 8 MiB real-world limit with 74LS377 - change to 14 for 512 MiB theoretical limit
		if (!check_rom(message))
			return image_init_result::FAIL;
		install_rom();
		install_ram();
		set_bank_rom(0);

		// install bank switch handler
		cart_space()->install_write_handler(0x0000, 0x3fff, write8sm_delegate(*this, FUNC(wisdom_device::bank_rom_switch)));

		// all good
		return image_init_result::PASS;
	}

private:
	void bank_rom_switch(offs_t offset, u8 data)
	{
		set_bank_rom(offset);
	}
};



//**************************************************************************
//  Yong Yong Pirate
//**************************************************************************

class yong_device : public flat_ram_device_base<mbc_device_base>
{
public:
	yong_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		flat_ram_device_base<mbc_device_base>(mconfig, GB_ROM_YONG, tag, owner, clock)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// set up ROM/RAM
		set_bank_bits_rom(8);
		if (!check_rom(message))
			return image_init_result::FAIL;
		install_rom();
		install_ram();

		// install bank switch handler
		cart_space()->install_write_handler(0x2000, 0x2000, write8smo_delegate(*this, FUNC(yong_device::bank_rom_switch)));

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_reset() override ATTR_COLD
	{
		flat_ram_device_base<mbc_device_base>::device_reset();

		set_bank_rom(1);
	}

private:
	void bank_rom_switch(u8 data)
	{
		set_bank_rom(data);
	}
};



//**************************************************************************
//  MBC1 variant used by Yong Yong for Rockman 8
//**************************************************************************

class rockman8_device : public flat_ram_device_base<mbc_device_base>
{
public:
	rockman8_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		flat_ram_device_base<mbc_device_base>(mconfig, GB_ROM_ROCKMAN8, tag, owner, clock)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// set up ROM/RAM
		set_bank_bits_rom(5);
		if (!check_rom(message))
			return image_init_result::FAIL;
		install_rom();
		install_ram();

		// install bank switch handler
		cart_space()->install_write_handler(0x2000, 0x3fff, write8smo_delegate(*this, FUNC(rockman8_device::bank_rom_switch)));

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_reset() override ATTR_COLD
	{
		flat_ram_device_base<mbc_device_base>::device_reset();

		set_bank_rom(1);
	}

private:
	void bank_rom_switch(u8 data)
	{
		data &= 0x1f;
		set_bank_rom(!data ? 1 : (0x0f < data) ? (data - 8) : data);
	}
};



//**************************************************************************
//  MBC1 variant used by Yong Yong for Super Mario 3 Special
//**************************************************************************
/*
 Mario special seems to be a 512k image (mirrored up to 1M or 2M [redump needed to establish this])
 it consists of 13 unique 16k chunks laid out as follows
 unique chunk --> bank in bin
 1st to 7th   --> 0x00 to 0x06
 8th          --> 0x08
 9th          --> 0x0b
 10th         --> 0x0c
 11th         --> 0x0d
 12th         --> 0x0f
 13th         --> 0x13

 According to old doc from Brian Provinciano, writing bit 5 in 0x5000-0x5fff should
 change the bank layout, in the sense that writing to bank switch acts like if
 the original ROm has a different layout (as if unique chunks were under permutations
 (24), (365) and (8a9) with 0,1,7,b,c fixed) and the same table above is used.
 However, no such a write ever happen (only bit 4 is written, but changing mode with
 bit 4 breaks the graphics).
 */

class sm3sp_device : public flat_ram_device_base<mbc_device_base>
{
public:
	sm3sp_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		flat_ram_device_base<mbc_device_base>(mconfig, GB_ROM_SM3SP, tag, owner, clock),
		m_bank_rom_mode(0U)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// set up ROM/RAM
		set_bank_bits_rom(5);
		if (!check_rom(message))
			return image_init_result::FAIL;
		install_rom();
		install_ram();

		// install bank switch handler
		cart_space()->install_write_handler(0x2000, 0x2fff, write8smo_delegate(*this, FUNC(sm3sp_device::bank_rom_switch)));
		cart_space()->install_write_handler(0x5000, 0x5fff, write8smo_delegate(*this, FUNC(sm3sp_device::bank_rom_mode)));

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_start() override ATTR_COLD
	{
		flat_ram_device_base<mbc_device_base>::device_start();

		save_item(NAME(m_bank_rom_mode));
	}

	virtual void device_reset() override ATTR_COLD
	{
		flat_ram_device_base<mbc_device_base>::device_reset();

		m_bank_rom_mode = 0U;

		set_bank_rom(1);
	}

private:
	void bank_rom_switch(u8 data)
	{
		/*
		 writing data to 0x2000-0x2fff switches bank according to the table below
		 (the value values corresponding to table[0x0f] is not confirmed, choices
		 0,1,2,3,8,c,f freeze the game, while 4,5,6,7,b,d,0x13 work with glitches)

		 Table 1 confirmed...
		  0 -> 0
		  4 -> 2
		  6 -> 3
		 19 -> 5 (level 2 background graphics)
		 1b -> 8 (level 3 background graphics)
		 1c -> b (bonus house background graphics)
		 1d -> d (level 4 background graphics)
		 1e -> 6 (level 1 background graphics)
		 1 (9 maybe, or 3)? f (5 maybe)? 2->1?
		 16 -> 4-8? b?
		 */
		static constexpr u8 BANK_TABLE[0x20] = {
				0x00,0x04,0x01,0x05, 0x02,0x06,0x03,0x05,
				0x08,0x0c,0x03,0x0d, 0x03,0x0b,0x0b,0x08 /* original doc put 0x0f here (i.e. 11th unique bank) */,
				0x05,0x06,0x0b,0x0d, 0x08,0x06,0x13,0x0b,
				0x08,0x05,0x05,0x08, 0x0b,0x0d,0x06,0x05 };

		u8 const lookup(BANK_TABLE[data & 0x1f]);
		u8 remap(lookup);
		if (m_bank_rom_mode)
		{
			switch (lookup)
			{
			case 0x02: remap = 0x04; break;
			case 0x03: remap = 0x06; break;
			case 0x04: remap = 0x02; break;
			case 0x05: remap = 0x03; break;
			case 0x06: remap = 0x05; break;
			case 0x0b: remap = 0x0d; break;
			case 0x0c: remap = 0x0b; break;
			case 0x0d: remap = 0x0c; break;

			case 0x00:
			case 0x01:
			case 0x08:
			case 0x0f:
			case 0x13:
			default:
				break;
			}
		}
		LOG(
				"%s: Bank select 0x%02X -> 0x%02X -> 0x%02X\n",
				machine().describe_context(),
				data,
				lookup,
				remap);
		set_bank_rom(remap);
	}

	void bank_rom_mode(u8 data)
	{
		LOG("%s: Bank mode 0x%02X\n", machine().describe_context(), data);
		m_bank_rom_mode = BIT(data, 5);
	}

	u8 m_bank_rom_mode;
};



//**************************************************************************
//  Sachen MMC1 unlicensed
//**************************************************************************

class sachen_mmc1_device : public sachen_mmc_device_base
{
public:
	sachen_mmc1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		sachen_mmc_device_base(mconfig, GB_ROM_SACHEN1, tag, owner, clock),
		m_counter(0U)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// configure ROM banks but don't install directly
		if (!check_rom(message))
			return image_init_result::FAIL;
		configure_bank_rom();

		// actually install ROM and bank switch handlers
		logerror("Installing banked ROM at 0x0000-0x3FFF and 0x4000-0x7FFF\n");
		cart_space()->install_read_handler(0x0000, 0x7fff, read8sm_delegate(*this, FUNC(sachen_mmc1_device::read_rom)));
		install_bank_switch_handlers();

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_start() override ATTR_COLD
	{
		sachen_mmc_device_base::device_start();

		save_item(NAME(m_counter));
	}

	virtual void device_reset() override ATTR_COLD
	{
		sachen_mmc_device_base::device_reset();

		m_counter = 0U;
	}

private:
	u8 read_rom(offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			// These cartridges work by counting low-to-high transitions on A15.
			// CPU keeps A15 high while reading internal bootstrap ROM.
			// Doing this on ROM reads is a good enough approximation for it to work.
			if (0x31 != m_counter)
				++m_counter;
		}

		return rom_read(offset, 0x31 != m_counter);
	}

	u8 m_counter;
};



//**************************************************************************
//  Sachen MMC2 unlicensed
//**************************************************************************

class sachen_mmc2_device : public gbc_logo_spoof_device_base<sachen_mmc_device_base>
{
public:
	sachen_mmc2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		gbc_logo_spoof_device_base<sachen_mmc_device_base>(mconfig, GB_ROM_SACHEN2, tag, owner, clock)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// configure ROM banks but don't install directly
		if (!check_rom(message))
			return image_init_result::FAIL;
		configure_bank_rom();

		// actually install ROM and bank switch handlers
		logerror("Installing banked ROM at 0x0000-0x3FFF and 0x4000-0x7FFF\n");
		cart_space()->install_read_handler(0x0000, 0x7fff, read8sm_delegate(*this, FUNC(sachen_mmc2_device::read_rom)));
		install_bank_switch_handlers();

		// all good
		return image_init_result::PASS;
	}

private:
	u8 read_rom(offs_t offset)
	{
		rom_access();
		return rom_read(offset, spoof_logo());
	}
};



//**************************************************************************
//  Rocket Games unlicensed (incomplete)
//**************************************************************************

class rocket_device : public flat_ram_device_base<gbc_logo_spoof_device_base<mbc_dual_device_base> >
{
public:
	rocket_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		flat_ram_device_base<gbc_logo_spoof_device_base<mbc_dual_device_base> >(mconfig, GB_ROM_ROCKET, tag, owner, clock)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// configure ROM banks but don't install directly
		set_bank_bits_rom(8, 4);
		if (!check_rom(message))
			return image_init_result::FAIL;
		configure_bank_rom();
		install_ram();

		// actually install ROM and bank switch handlers
		logerror("Installing banked ROM at 0x0000-0x3FFF and 0x4000-0x7FFF\n");
		cart_space()->install_read_handler(0x0000, 0x7fff, read8sm_delegate(*this, FUNC(rocket_device::read_rom)));
		cart_space()->install_write_handler(0x3f00, 0x3f00, write8smo_delegate(*this, FUNC(rocket_device::bank_rom_switch_fine)));
		cart_space()->install_write_handler(0x3fc0, 0x3fc0, write8smo_delegate(*this, FUNC(rocket_device::bank_rom_switch_coarse)));

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_reset() override ATTR_COLD
	{
		flat_ram_device_base<gbc_logo_spoof_device_base<mbc_dual_device_base> >::device_reset();

		set_bank_rom_coarse(0);
		set_bank_rom_fine(1);
	}

private:
	u8 read_rom(offs_t offset)
	{
		// logo spoofing logic
		rom_access();

		// fetch the banked ROM data
		u8 const data((BIT(offset, 14) ? bank_rom_high_base() : bank_rom_low_base())[offset & 0x3fff]);
		if (spoof_logo() && (0x0104 <= offset) && (0x133 >= offset))
		{
			static constexpr u8 TRANSFORM[] = {
					0xdf, 0xce, 0x97, 0x78, 0xcd, 0x2f, 0xf0, 0x0b, 0x0b, 0xea, 0x78, 0x83, 0x08, 0x1d, 0x9a, 0x45,
					0x11, 0x2b, 0xe1, 0x11, 0xf8, 0x88, 0xf8, 0x8e, 0xfe, 0x88, 0x2a, 0xc4, 0xff, 0xfc, 0xd9, 0x87,
					0x22, 0xab, 0x67, 0x7d, 0x77, 0x2c, 0xa8, 0xee, 0xff, 0x9b, 0x99, 0x91, 0xaa, 0x9b, 0x33, 0x3e };
			return data ^ TRANSFORM[offset - 0x104];
		}
		else
		{
			return data;
		}
	}

	void bank_rom_switch_fine(u8 data)
	{
		set_bank_rom_fine(data ? data : 1);
	}

	void bank_rom_switch_coarse(u8 data)
	{
		set_bank_rom_coarse(data);
	}
};



//**************************************************************************
//  Story of Lasama pirate (incomplete)
//**************************************************************************

class lasama_device : public flat_ram_device_base<mbc_dual_device_base>
{
public:
	lasama_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock) :
		flat_ram_device_base<mbc_dual_device_base>(mconfig, GB_ROM_LASAMA, tag, owner, clock)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// set up ROM/RAM
		set_bank_bits_rom(1, 2);
		if (!check_rom(message))
			return image_init_result::FAIL;
		install_rom();
		install_ram();

		// install bank switch handlers
		cart_space()->install_write_handler(0x2080, 0x2080, write8smo_delegate(*this, FUNC(lasama_device::ctrl_2080_w)));
		cart_space()->install_write_handler(0x6000, 0x6000, write8smo_delegate(*this, FUNC(lasama_device::ctrl_6000_w)));

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_reset() override ATTR_COLD
	{
		flat_ram_device_base<mbc_dual_device_base>::device_reset();

		set_bank_rom_coarse(0);
		set_bank_rom_fine(1);
	}

private:
	void ctrl_2080_w(u8 data)
	{
		// high ROM fine bank?
		set_bank_rom_fine(data & 0x03);
	}

	void ctrl_6000_w(u8 data)
	{
		// On boot the following two get written right after each other:
		// 02
		// BE
		// disable logo switching?
		if (!BIT(data, 7))
			set_bank_rom_coarse(BIT(data, 1));
	}
};

} // anonymous namespace

} // namespace bus::gameboy


// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(MEGADUCK_ROM_STD,    device_gb_cart_interface, bus::gameboy::megaduck_flat_device,   "megaduck_rom",        "Mega Duck Flat ROM Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(MEGADUCK_ROM_BANKED, device_gb_cart_interface, bus::gameboy::megaduck_banked_device, "megaduck_rom_banked", "Mega Duck Banked ROM Cartridge")

DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_STD,          device_gb_cart_interface, bus::gameboy::flat_rom_ram_device,    "gb_rom",              "Game Boy Flat ROM Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_M161,         device_gb_cart_interface, bus::gameboy::m161_device,            "gb_rom_m161",         "Game Boy M161 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_WISDOM,       device_gb_cart_interface, bus::gameboy::wisdom_device,          "gb_rom_wisdom",       "Game Boy Wisdom Tree Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_YONG,         device_gb_cart_interface, bus::gameboy::yong_device,            "gb_rom_yong",         "Game Boy Young Yong Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_ROCKMAN8,     device_gb_cart_interface, bus::gameboy::rockman8_device,        "gb_rom_rockman8",     "Game Boy Rockman 8 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_SM3SP,        device_gb_cart_interface, bus::gameboy::sm3sp_device,           "gb_rom_sm3sp",        "Game Boy Super Mario 3 Special Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_SACHEN1,      device_gb_cart_interface, bus::gameboy::sachen_mmc1_device,     "gb_rom_sachen1",      "Game Boy Sachen MMC1 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_SACHEN2,      device_gb_cart_interface, bus::gameboy::sachen_mmc2_device,     "gb_rom_sachen2",      "Game Boy Sachen MMC2 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_ROCKET,       device_gb_cart_interface, bus::gameboy::rocket_device,          "gb_rom_rocket",       "Game Boy Rocket Games Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_LASAMA,       device_gb_cart_interface, bus::gameboy::lasama_device,          "gb_rom_lasama",       "Game Boy Story of Lasama Cartridge")
