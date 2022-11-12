// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Wilbert Pol
/***********************************************************************************************************

 Game Boy carts with MBC (Memory Bank Controller)


 MBC1 Mapper
 ===========

 The MBC1 mapper has two modes: 2MB ROM/8KB RAM or 512KB ROM/32KB RAM.
 Initially, the mapper operates in 2MB ROM/8KB RAM mode.

 0000-1FFF - Writing to this area enables (value 0x0A) or disables (not 0x0A) the
             SRAM.
 2000-3FFF - Writing a value 0bXXXBBBBB into the 2000-3FFF memory area selects the
             lower 5 bits of the ROM bank to select for the 4000-7FFF memory area.
             If a value of 0bXXX00000 is written then this will automatically be
             changed to 0bXXX00001 by the mbc chip. Initial value 00.
 4000-5FFF - Writing a value 0bXXXXXXBB into the 4000-5FFF memory area either selects
             the RAM bank to use or bits 6 and 7 for the ROM bank to use for the 4000-7FFF
             memory area. This behaviour depends on the memory mode chosen.
             These address lines are fixed in mode 1 and switch depending on A14 in mode 0.
             In mode 0 these will drive 0 when RB 00 is accessed (A14 low) or the value set
             in 4000-5FFF when RB <> 00 is accessed (A14 high).
             Switching between modes does not clear this register. Initial value 00.
 6000-7FFF - Writing a value 0bXXXXXXXB into the 6000-7FFF memory area switches the mode.
             B=0 - 2MB ROM/8KB RAM mode
             B=1 - 512KB ROM/32KB RAM mode

 Regular ROM aliasing rules apply.


 MBC5 Mapper
 ===========

 0000-1FFF - Writing to this area enables (0x0A) or disables (not 0x0A) the SRAM area.
 2000-2FFF - Writing to this area updates bits 7-0 of the ROM bank number to
             appear at 4000-7FFF.
 3000-3FFF - Writing to this area updates bit 8 of the ROM bank number to appear
             at 4000-7FFF.
 4000-5FFF - Writing to this area select the RAM bank number to use. If the
             cartridge includes a Rumble Pack then bit 3 is used to control
             rumble motor (0 - disable motor, 1 - enable motor).


 TODO:
 * MBC5 logo spoofing class implements several strategies.  It's likely not all carts using it use all
   the strategies.  Strategies implemented by each cartridge should be identified.
 * HK0701 and HK0819 seem to differ in that HK0819 fully decodes ROM addresses while HK0701 mirrors - we
   should probably emulated the difference at some point.
 * BC-R1616T3P is a variant of DSHGGB-81 with the same ROM scrambling but without logo spoofing.

 ***********************************************************************************************************/


#include "emu.h"
#include "mbc.h"

#include "cartbase.ipp"
#include "cartheader.h"
#include "gbxfile.h"

#include "bus/generic/slot.h"

#include "corestr.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <locale>
#include <sstream>
#include <type_traits>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

//**************************************************************************
//  MBC1/MBC3/MBC5 banked RAM support helper class
//**************************************************************************

class rom_mbc_device_base : public mbc_ram_device_base<mbc_dual_device_base>
{
protected:
	rom_mbc_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
		mbc_ram_device_base<mbc_dual_device_base>(mconfig, type, tag, owner, clock),
		m_view_ram(*this, "ram"),
		m_bank_lines{ 0U, 0U }
	{
	}

	bool install_memory(std::string &message, unsigned highbits, unsigned lowbits) ATTR_COLD
	{
		// see if the cartridge shouldn't use all the low bank bits
		char const *const lowbitsfeature(get_feature("banklowbits"));
		if (lowbitsfeature)
		{
			std::istringstream s;
			s.imbue(std::locale::classic());
			s.str(lowbitsfeature);
			int b(-1);
			if (!(s >> b) || (0 > b) || (lowbits < b))
			{
				message = util::string_format(
						"Invalid 'banklowbits' value (must be a number from 0 to %u)\n",
						lowbits);
				return false;
			}
			lowbits = b;
		}

		// check for valid ROM/RAM regions
		set_bank_bits_rom(highbits, lowbits);
		set_bank_bits_ram(highbits);
		if (!check_rom(message) || !check_ram(message))
			return false;

		// everything checked out - install memory and return success
		m_bank_lines[0] = util::make_bitmask<u16>(lowbits);
		m_bank_lines[1] = util::make_bitmask<u16>(highbits);
		cart_space()->install_view(0xa000, 0xbfff, m_view_ram);
		install_rom();
		install_ram(m_view_ram[0]);
		return true;
	}

	void set_bank_rom_fine(u16 entry)
	{
		mbc_ram_device_base<mbc_dual_device_base>::set_bank_rom_fine(entry & m_bank_lines[0]);
	}

	void set_ram_enable(bool enable)
	{
		LOG(
				"%s: Cartridge RAM %s\n",
				machine().describe_context(),
				enable ? "enabled" : "disabled");
		if (enable)
			m_view_ram.select(0);
		else
			m_view_ram.disable();
	}

	void bank_switch_coarse(u8 data)
	{
		set_bank_rom_coarse(data & m_bank_lines[1]);
		set_bank_ram(data & m_bank_lines[1]);
	}

private:
	memory_view m_view_ram;

	u16 m_bank_lines[2];
};



//**************************************************************************
//  MBC5 and clones (max 128 MiB ROM, max 128 KiB SRAM)
//**************************************************************************

class mbc5_device_base : public rom_mbc_device_base
{
public:
	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// set up ROM and RAM
		if (!install_memory(message))
			return image_init_result::FAIL;

		// install handlers
		cart_space()->install_write_handler(
				0x0000, 0x1fff,
				write8smo_delegate(*this, FUNC(mbc5_device_base::enable_ram)));
		cart_space()->install_write_handler(
				0x2000, 0x2fff,
				write8smo_delegate(*this, FUNC(mbc5_device_base::bank_switch_fine_low)));
		cart_space()->install_write_handler(
				0x3000, 0x3fff,
				write8smo_delegate(*this, FUNC(mbc5_device_base::bank_switch_fine_high)));
		cart_space()->install_write_handler(
				0x4000, 0x5fff,
				write8smo_delegate(*this, FUNC(mbc5_device_base::bank_switch_coarse)));

		// all good
		return image_init_result::PASS;
	}

protected:
	mbc5_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
		rom_mbc_device_base(mconfig, type, tag, owner, clock)
	{
	}

	virtual void device_reset() override ATTR_COLD
	{
		rom_mbc_device_base::device_reset();

		set_bank_rom_coarse(0);
		set_bank_rom_fine(0);
		set_ram_enable(false);
		set_bank_ram(0);
	}

	bool install_memory(std::string &message) ATTR_COLD
	{
		return rom_mbc_device_base::install_memory(message, 4, 9);
	}

	void enable_ram(u8 data)
	{
		set_ram_enable(0x0a == data);
	}

	void bank_switch_fine_low(u8 data)
	{
		set_bank_rom_fine((bank_rom_fine() & 0x0100) | u16(data));
	}

	void bank_switch_fine_high(u8 data)
	{
		set_bank_rom_fine((bank_rom_fine() & 0x00ff) | (u16(data & 0x01) << 8));
	}
};



//**************************************************************************
//  MBC5-like pirate cartridges with logo spoofing
//**************************************************************************

class mbc5_logo_spoof_device_base : public mbc5_device_base
{
public:
	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// install regular MBC5 handlers
		image_init_result const result(mbc5_device_base::load(message));
		if (image_init_result::PASS != result)
			return result;

		// intercept ROM reads for logo spoofing
		cart_space()->install_read_handler(
				0x0000, 0x7fff,
				read8sm_delegate(*this, FUNC(mbc5_logo_spoof_device_base::read_rom)));

		// all good
		return image_init_result::PASS;
	}

protected:
	mbc5_logo_spoof_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
		mbc5_device_base(mconfig, type, tag, owner, clock),
		m_ram_tap(),
		m_notif_cart_space(),
		m_counter(0U),
		m_spoof_logo(0U),
		m_installing_tap(false)
	{
	}

	virtual void device_start() override ATTR_COLD
	{
		mbc5_device_base::device_start();

		save_item(NAME(m_counter));
		save_item(NAME(m_spoof_logo));
	}

	virtual void device_reset() override ATTR_COLD
	{
		mbc5_device_base::device_reset();

		m_counter = 0U;
		m_spoof_logo = 0U;

		install_ram_tap();
		add_ram_notifier();
	}

	virtual void device_post_load() override ATTR_COLD
	{
		mbc5_device_base::device_post_load();

		install_ram_tap();
		if (!m_spoof_logo)
			add_ram_notifier();
	}

	u8 read_rom(offs_t offset)
	{
		offset = rom_access(offset);
		return (BIT(offset, 14) ? bank_rom_high_base() : bank_rom_low_base())[offset & 0x3fff];
	}

	offs_t rom_access(offs_t offset)
	{
		if (!machine().side_effects_disabled() && (3U > m_spoof_logo))
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
							machine().describe_context(),
							m_spoof_logo);
				}
			}

			// accessing entry point disables logo spoofing logic
			if (0x0100 == offset)
				m_spoof_logo = 3U;
		}

		// A7 is held high to show pirate logo
		if (m_spoof_logo && ((2U != m_spoof_logo) || (cartheader::OFFSET_LOGO > offset) || (cartheader::OFFSET_TITLE <= offset)))
			return offset;
		else
			return offset | 0x0080;
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
				m_ram_tap = cart_space()->install_write_tap(
						0xc000, 0xdfff,
						"ram_tap_w",
						[this] (offs_t offset, u8 &data, u8 mem_mask)
						{
							assert(!m_spoof_logo);

							if (!machine().side_effects_disabled())
							{
								m_counter = 0U;
								++m_spoof_logo;
								LOG(
										"%s: Spoof logo step %u\n",
										machine().describe_context(),
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

	void add_ram_notifier()
	{
		m_notif_cart_space = cart_space()->add_change_notifier(
				[this] (read_or_write mode)
				{
					if (u32(mode) & u32(read_or_write::WRITE))
						install_ram_tap();
				});
	}

	memory_passthrough_handler m_ram_tap;
	util::notifier_subscription m_notif_cart_space;

	u8 m_counter;
	u8 m_spoof_logo;
	bool m_installing_tap;
};



//**************************************************************************
//  MBC5-like pirate cartridges with scrambled bank numbers and data
//**************************************************************************

class mbc5_scrambled_device_base : public mbc5_logo_spoof_device_base
{

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// install regular MBC5 handlers
		image_init_result const result(mbc5_logo_spoof_device_base::load(message));
		if (image_init_result::PASS != result)
			return result;

		// bank numbers and ROM contents are scrambled for protection
		cart_space()->install_read_handler(
				0x0000, 0x3fff,
				read8sm_delegate(*this, FUNC(mbc5_scrambled_device_base::read_rom_low)));
		cart_space()->install_read_handler(
				0x4000, 0x7fff,
				read8sm_delegate(*this, FUNC(mbc5_scrambled_device_base::read_rom_high)));
		cart_space()->install_write_handler(
				0x2000, 0x2000, 0x0000, 0x0f00, 0x0000,
				write8smo_delegate(*this, FUNC(mbc5_scrambled_device_base::bank_switch_fine_low_scrambled)));
		cart_space()->install_write_handler(
				0x2001, 0x2001, 0x0000, 0x0f00, 0x0000,
				write8smo_delegate(*this, FUNC(mbc5_scrambled_device_base::set_data_scramble)));
		cart_space()->install_write_handler(
				0x2080, 0x2080, 0x0000, 0x0f00, 0x0000,
				write8smo_delegate(*this, FUNC(mbc5_scrambled_device_base::set_bank_scramble)));

		// all good
		return image_init_result::PASS;
	}

protected:
	using unscramble_function = u8 (*)(u8);

	mbc5_scrambled_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
		mbc5_logo_spoof_device_base(mconfig, type, tag, owner, clock),
		m_unscramble_data(nullptr),
		m_unscramble_bank(nullptr),
		m_scramble_mode{ 0U, 0U }
	{
	}

	virtual void device_start() override ATTR_COLD
	{
		mbc5_logo_spoof_device_base::device_start();

		save_item(NAME(m_scramble_mode));
	}

	virtual void device_reset() override ATTR_COLD
	{
		mbc5_logo_spoof_device_base::device_reset();

		m_unscramble_data = unscramble_data(0U);
		m_unscramble_bank = unscramble_bank(0U);
		m_scramble_mode[0] = 0U;
		m_scramble_mode[1] = 0U;
	}

	virtual void device_post_load() override ATTR_COLD
	{
		mbc5_logo_spoof_device_base::device_post_load();

		m_unscramble_data = unscramble_data(m_scramble_mode[0]);
		m_unscramble_bank = unscramble_bank(m_scramble_mode[1]);
	}

	virtual unscramble_function unscramble_data(u8 data) = 0;
	virtual unscramble_function unscramble_bank(u8 data) = 0;

	template <unsigned... B>
	static u8 unscramble(u8 data)
	{
		return bitswap<8>(data, B...);
	}

private:
	u8 read_rom_low(offs_t offset)
	{
		offset = rom_access(offset);
		return bank_rom_low_base()[offset];
	}

	u8 read_rom_high(offs_t offset)
	{
		offset = rom_access(0x4000 | offset) & 0x3fff;
		return m_unscramble_data(bank_rom_high_base()[offset]);
	}

	void bank_switch_fine_low_scrambled(u8 data)
	{
		set_bank_rom_fine((bank_rom_fine() & 0x0100) | u16(m_unscramble_bank(data)));
	}

	void set_data_scramble(u8 data)
	{
		LOG(
				"%s: Set ROM data scrambling mode 0x%02X\n",
				machine().describe_context(),
				data);
		if (data != m_scramble_mode[0])
		{
			m_unscramble_data = unscramble_data(data);
			m_scramble_mode[0] = data;
		}

		// TODO: does this actually trigger a bank switch?
		set_bank_rom_fine((bank_rom_fine() & 0x0100) | u16(data));
	}

	void set_bank_scramble(u8 data)
	{
		LOG(
				"%s: Set ROM bank number scrambling mode 0x%02X\n",
				machine().describe_context(),
				data);
		if (data != m_scramble_mode[1])
		{
			m_unscramble_bank = unscramble_bank(data);
			m_scramble_mode[1] = data;
		}

		// TODO: does this actually trigger a bank switch?
		set_bank_rom_fine((bank_rom_fine() & 0x0100) | u16(data));
	}

	unscramble_function m_unscramble_data;
	unscramble_function m_unscramble_bank;

	u8 m_scramble_mode[2];
};



//**************************************************************************
//  MBC1 (max 2 MiB ROM, max 32 KiB SRAM)
//**************************************************************************

class mbc1_device : public rom_mbc_device_base
{
public:
	mbc1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		rom_mbc_device_base(mconfig, GB_ROM_MBC1, tag, owner, clock)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// probe for "collection" cartridges wired for 4-bit fine bank number
		unsigned banklowbits(5U);
		if (is_collection())
			banklowbits = 4U;

		// set up ROM and RAM
		if (!install_memory(message, 2, banklowbits))
			return image_init_result::FAIL;

		// install handlers
		cart_space()->install_write_handler(
				0x0000, 0x1fff,
				write8smo_delegate(*this, FUNC(mbc1_device::enable_ram)));
		cart_space()->install_write_handler(
				0x2000, 0x3fff,
				write8smo_delegate(*this, FUNC(mbc1_device::bank_switch_fine)));
		cart_space()->install_write_handler(
				0x4000, 0x5fff,
				write8smo_delegate(*this, FUNC(mbc1_device::bank_switch_coarse)));
		cart_space()->install_write_handler(
				0x6000, 0x7fff,
				write8smo_delegate(*this, FUNC(mbc1_device::bank_low_mask)));

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_reset() override ATTR_COLD
	{
		rom_mbc_device_base::device_reset();

		set_bank_rom_coarse(0);
		set_bank_rom_fine(1);
		set_bank_rom_low_coarse_mask(0x00);
		set_ram_enable(false);
		set_bank_ram(0);
		set_bank_ram_mask(0x00);
	}

private:
	void enable_ram(u8 data)
	{
		set_ram_enable(0x0a == (data & 0x0f));
	}

	void bank_switch_fine(u8 data)
	{
		data &= 0x1f;
		set_bank_rom_fine(data ? data : 1);
	}

	void bank_low_mask(u8 data)
	{
		u8 const mask(data ? 0x03 : 0x00);
		LOG(
				"%s: RAM/low ROM bank switching %s\n",
				machine().describe_context(),
				mask ? "enabled" : "disabled");
		set_bank_rom_low_coarse_mask(mask);
		set_bank_ram_mask(mask);
	}

	bool is_collection() ATTR_COLD
	{
		// addressing must be explicitly specified in software lists
		if (loaded_through_softlist())
			return false;

		// need ROM to probe
		memory_region *const romregion(cart_rom_region());
		if (!romregion)
			return false;

		// doesn't make sense without more than 256 KiB, and can address up to 1 MiB in 16 KiB pages
		auto const rombytes(romregion->bytes());
		if (((u32(0x4000) << 6) < rombytes) || ((u32(0x4000) << 4) >= rombytes) || (rombytes & 0x3fff))
			return false;

		// reject if the header checksum for the first page fails
		u8 const *const rombase(romregion->base());
		if (!cartheader::verify_header_checksum(rombase + 0x100))
			return false;

		// check title in first page header against list of known collections
		static constexpr u8 KNOWN_COLLECTIONS[][0x10] = {
				{ 'B', 'O', 'M', 'C', 'O', 'L', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
				{ 'B', 'O', 'M', 'S', 'E', 'L', 0,   0,   0,   0,   0,   'B', '2', 'C', 'K', 0xc0 },
				{ 'G', 'E', 'N', 'C', 'O', 'L', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
				{ 'M', 'O', 'M', 'O', 'C', 'O', 'L', 0,   0,   0,   0,   0,   0,   0,   0,   0 },
				{ 'M', 'O', 'R', 'T', 'A', 'L', 'K', 'O', 'M', 'B', 'A', 'T', ' ', 'D', 'U', 'O' },
				{ 'M', 'O', 'R', 'T', 'A', 'L', 'K', 'O', 'M', 'B', 'A', 'T', 'I', '&', 'I', 'I' },
				{ 'S', 'U', 'P', 'E', 'R', 'C', 'H', 'I', 'N', 'E', 'S', 'E', ' ', '1', '2', '3' } };
		auto const known(
				std::find_if(
					std::begin(KNOWN_COLLECTIONS),
					std::end(KNOWN_COLLECTIONS),
					[rombase] (auto const &name)
					{
						return std::equal(std::begin(name), std::end(name), &rombase[cartheader::OFFSET_TITLE]);
					}));
		if (std::end(KNOWN_COLLECTIONS) != known)
		{
			logerror("Detected known multi-game collection with 4-bit fine bank addressing\n");
			return true;
		}

		// reject if the header checksum for the second coarse page fails
		if (!cartheader::verify_header_checksum(rombase + (u32(0x4000) << 4) + 0x100))
			return false;

		// assume it's a collection if the type indicates an MBC1 cartridge
		switch (rombase[(u32(0x4000) << 4) + cartheader::OFFSET_TYPE])
		{
		case cartheader::TYPE_MBC1:
		case cartheader::TYPE_MBC1_RAM:
		case cartheader::TYPE_MBC1_RAM_BATT:
			logerror("Detected MBC1 header in page 0x10, using 4-bit fine bank addressing\n");
			return true;
		}

		// assume anything else uses a different (presumably more conventional) scheme
		return false;
	}
};



//**************************************************************************
//  MBC5 (max 128 MiB ROM, max 128 KiB SRAM)
//**************************************************************************

class mbc5_device : public mbc5_device_base
{
public:
	mbc5_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		mbc5_device_base(mconfig, GB_ROM_MBC5, tag, owner, clock),
		m_rumble(*this, "rumble"),
		m_has_rumble(false)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// check whether rumble motor is present
		char const *const rumblefeature(get_feature("rumble"));
		if (rumblefeature)
		{
			// explicitly specified in software list
			if (util::streqlower(rumblefeature, "yes") || util::streqlower(rumblefeature, "true"))
			{
				logerror("Rumble motor present\n");
				m_has_rumble = true;
			}
			else if (util::streqlower(rumblefeature, "no") || util::streqlower(rumblefeature, "false"))
			{
				logerror("No rumble motor present\n");
				m_has_rumble = false;
			}
			else
			{
				message = "Invalid 'rumble' feature value (must be yes or no)";
				return image_init_result::FAIL;
			}
		}
		else if (loaded_through_softlist())
		{
			logerror("No 'rumble' feature found, assuming no rumble motor present\n");
			m_has_rumble = false;
		}
		else
		{
			gbxfile::leader_1_0 leader;
			u8 const *extra;
			u32 extralen;
			if (gbxfile::get_data(gbx_footer_region(), leader, extra, extralen))
			{
				m_has_rumble = bool(leader.rumble);
				logerror("GBX format image specifies %srumble motor present\n", m_has_rumble ? "" : "no ");
			}
			else
			{
				// try probing the header
				memory_region *const romregion(cart_rom_region());
				if (romregion && (romregion->bytes() > cartheader::OFFSET_TYPE))
				{
					u8 const carttype((&romregion->as_u8())[cartheader::OFFSET_TYPE]);
					switch (carttype)
					{
					case cartheader::TYPE_MBC5_RUMBLE:
					case cartheader::TYPE_MBC5_RUMBLE_RAM:
					case cartheader::TYPE_MBC5_RUMBLE_RAM_BATT:
						m_has_rumble = true;
						break;
					case cartheader::TYPE_MBC5:
					case cartheader::TYPE_MBC5_RAM:
					case cartheader::TYPE_MBC5_RAM_BATT:
						m_has_rumble = false;
						break;
					default:
						osd_printf_warning(
								"[%s] Unrecognized cartridge type 0x%02X in header, assuming no rumble motor present\n",
								tag(),
								carttype);
						m_has_rumble = false;
					}
					logerror(
							"Cartridge type 0x%02X in header, %srumble motor present\n",
							carttype,
							m_has_rumble ? "" : "no ");
				}
			}
		}

		// install base MBC5 handlers
		image_init_result const result(mbc5_device_base::load(message));
		if (image_init_result::PASS != result)
			return result;

		// install rumble-aware handler if appropriate
		if (m_has_rumble)
		{
			cart_space()->install_write_handler(
					0x4000, 0x5fff,
					write8smo_delegate(*this, FUNC(mbc5_device::bank_switch_coarse)));
		}

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_start() override ATTR_COLD
	{
		mbc5_device_base::device_start();

		m_rumble.resolve();
	}

	virtual void device_reset() override ATTR_COLD
	{
		mbc5_device_base::device_reset();

		if (m_has_rumble)
			m_rumble = 0;
	}

private:
	void bank_switch_coarse(u8 data)
	{
		assert(m_has_rumble);
		mbc5_device_base::bank_switch_coarse(data);
		m_rumble = BIT(data, 3);
	}

	output_finder<> m_rumble;
	bool m_has_rumble;
};



//**************************************************************************
//  MBC5 variant used by BBD games
//**************************************************************************

class bbd_device : public mbc5_scrambled_device_base
{
public:
	bbd_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		mbc5_scrambled_device_base(mconfig, GB_ROM_BBD, tag, owner, clock)
	{
	}

protected:
	virtual unscramble_function unscramble_data(u8 data) override
	{
		switch (data & 0x07)
		{
		case 0x0: // no scrambling
			return &bbd_device::unscramble<7, 6, 5, 4, 3, 2, 1, 0>;
		case 0x4: // Garou
			return &bbd_device::unscramble<7, 6, 2, 4, 3, 1, 5, 0>;
		case 0x5: // Harry Potter
			return  &bbd_device::unscramble<7, 6, 5, 1, 3, 2, 4, 0>;
		case 0x7: // Digimon Adventure
			return  &bbd_device::unscramble<7, 6, 2, 4, 3, 5, 1, 0>;
		}

		logerror(
				"%s: Unsupported ROM data scrambling mode 0x%02X\n",
				machine().describe_context(),
				data);
		return &bbd_device::unscramble<7, 6, 5, 4, 3, 2, 1, 0>;
	}

	virtual unscramble_function unscramble_bank(u8 data) override
	{
		switch (data & 0x07)
		{
		case 0x0: // no scrambling
			return &bbd_device::unscramble<7, 6, 5, 4, 3, 2, 1, 0>;
		case 0x3: // Digimon Adventure, Garou - two most significant bits unconfirmed
			return &bbd_device::unscramble<7, 6, 5, 1, 0, 2, 4, 3>;
		case 0x5: // Harry Potter - two most significant bits unconfirmed
			return &bbd_device::unscramble<7, 6, 5, 0, 4, 3, 2, 1>;
		}

		logerror(
				"%s: Unsupported ROM bank number scrambling mode 0x%02X\n",
				machine().describe_context(),
				data);
		return &bbd_device::unscramble<7, 6, 5, 4, 3, 2, 1, 0>;
	}
};



//**************************************************************************
//  MBC5 variant with DSHGGB-81 PCB
//**************************************************************************

class dshggb81_device : public mbc5_scrambled_device_base
{
public:
	dshggb81_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		mbc5_scrambled_device_base(mconfig, GB_ROM_DSHGGB81, tag, owner, clock)
	{
	}

protected:
	virtual unscramble_function unscramble_data(u8 data) override
	{
		switch (data & 0x07)
		{
		default: // just to shut up dumb compilers
		case 0x0: return &dshggb81_device::unscramble<7, 6, 5, 4, 3, 2, 1, 0>;
		case 0x1: return &dshggb81_device::unscramble<7, 5, 6, 4, 3, 1, 2, 0>;
		case 0x2: return &dshggb81_device::unscramble<7, 1, 2, 4, 3, 5, 6, 0>;
		case 0x3: return &dshggb81_device::unscramble<7, 6, 2, 4, 3, 1, 5, 0>;
		case 0x4: return &dshggb81_device::unscramble<7, 6, 1, 4, 3, 2, 5, 0>;
		case 0x5: return &dshggb81_device::unscramble<7, 1, 5, 4, 3, 6, 2, 0>;
		case 0x6: return &dshggb81_device::unscramble<7, 5, 2, 4, 3, 6, 1, 0>;
		case 0x7: return &dshggb81_device::unscramble<7, 1, 6, 4, 3, 5, 2, 0>;
		}
	}

	virtual unscramble_function unscramble_bank(u8 data) override
	{
		switch (data & 0x07)
		{
		case 0x0: // no scrambling
			return &dshggb81_device::unscramble<7, 6, 5, 4, 3, 2, 1, 0>;
		}

		logerror(
				"%s: Unsupported ROM bank number scrambling mode 0x%02X\n",
				machine().describe_context(),
				data);
		return &dshggb81_device::unscramble<7, 6, 5, 4, 3, 2, 1, 0>;
	}
};



//**************************************************************************
//  MBC5 variant used by Sintax games
//**************************************************************************

class sintax_device : public mbc5_logo_spoof_device_base
{
public:
	sintax_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		mbc5_logo_spoof_device_base(mconfig, GB_ROM_SINTAX, tag, owner, clock),
		m_scramble_mode(0U),
		m_xor_index(0U)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// install regular MBC5 handlers
		image_init_result const result(mbc5_logo_spoof_device_base::load(message));
		if (image_init_result::PASS != result)
			return result;

		// bank numbers and ROM contents are scrambled for protection
		cart_space()->install_read_handler(
				0x0000, 0x3fff,
				read8sm_delegate(*this, FUNC(sintax_device::read_rom_low)));
		cart_space()->install_read_handler(
				0x4000, 0x7fff,
				read8sm_delegate(*this, FUNC(sintax_device::read_rom_high)));
		cart_space()->install_write_handler(
				0x2000, 0x2fff,
				write8smo_delegate(*this, FUNC(sintax_device::bank_switch_fine_low_scrambled)));
		cart_space()->install_write_handler(
				0x5000, 0x5fff,
				write8smo_delegate(*this, FUNC(sintax_device::set_scramble_mode)));
		cart_space()->install_write_handler(
				0x7000, 0x70ff, 0x0000, 0x0f00, 0x0000,
				write8sm_delegate(*this, FUNC(sintax_device::set_xor)));

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_start() override ATTR_COLD
	{
		mbc5_logo_spoof_device_base::device_start();

		save_item(NAME(m_scramble_mode));
		save_item(NAME(m_xor_index));
		save_item(NAME(m_xor_table));
	}

	virtual void device_reset() override ATTR_COLD
	{
		mbc5_logo_spoof_device_base::device_reset();

		m_scramble_mode = 0U;
		m_xor_index = 0U;
		std::fill(std::begin(m_xor_table), std::end(m_xor_table), 0U);
	}

private:
	u8 read_rom_low(offs_t offset)
	{
		offset = rom_access(offset);
		return bank_rom_low_base()[offset];
	}

	u8 read_rom_high(offs_t offset)
	{
		offset = rom_access(0x4000 | offset) & 0x3fff;
		return bank_rom_high_base()[offset] ^ m_xor_table[m_xor_index];
	}

	void bank_switch_fine_low_scrambled(u8 data)
	{
		m_xor_index = data & 0x03;

		switch (m_scramble_mode & 0x0f)
		{
		case 0x00: // Lion King, Golden Sun
			data = bitswap<8>(data, 7, 0, 5, 6, 3, 4, 1, 2);
			break;
		case 0x01: // Langrisser
			data = bitswap<8>(data, 0, 1, 6, 7, 4, 5, 2, 3);
			break;
		case 0x05: // Maple Story, Pokemon Platinum
			data = bitswap<8>(data, 7, 6, 1, 0, 3, 2, 5, 4); // needs verification
			break;
		case 0x07: // Bynasty Warriors 5
			data = bitswap<8>(data, 2, 0, 3, 1, 5, 4, 7, 6); // 5 and 7 unconfirmed
			break;
		case 0x09:
			data = bitswap<8>(data, 4, 5, 2, 3, 0, 1, 6, 7);
			break;
		case 0x0b: // Shaolin Legend
			data = bitswap<8>(data, 2, 3, 0, 1, 6, 7, 4, 5); // 5 and 6 unconfirmed
			break;
		case 0x0d: // older games
			data = bitswap<8>(data, 1, 0, 7, 6, 5, 4, 3, 2);
			break;
		case 0x0f: // no scrambling
			break;
		default:
			logerror(
					"%s: Unsupported ROM bank number scrambling mode 0x%X\n",
					machine().describe_context(),
					m_scramble_mode & 0x0f);
		}

		set_bank_rom_fine((bank_rom_fine() & 0x0100) | u16(data));
	}

	void set_scramble_mode(u8 data)
	{
		if (!m_scramble_mode)
		{
			LOG(
					"%s: Set scramble mode = 0x%02X\n",
					machine().describe_context(),
					data);
			m_scramble_mode = data;
			bank_switch_fine_low_scrambled(0x01);
		}
		else
		{
			LOG(
					"%s: Ignoring scramble mode = 0x%02X when already set to 0x%02X\n",
					machine().describe_context(),
					data,
					m_scramble_mode);
		}
	}

	void set_xor(offs_t offset, u8 data)
	{
		u8 const index(offset >> 4);
		if ((2 <= index) && (6 > index))
		{
			LOG(
					"%s: Set XOR entry %u = 0x%02X\n",
					machine().describe_context(),
					index - 2,
					data);
			m_xor_table[index - 2] = data;
		}
	}

	u8 m_scramble_mode;
	u8 m_xor_index;
	u8 m_xor_table[4];
};



//**************************************************************************
//  MBC5 variant used by Chǒngwù Xiǎo Jīnglíng - Jié Jīn Tǎ Zhī Wáng
//**************************************************************************

class chongwu_device : public mbc5_logo_spoof_device_base
{
public:
	chongwu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		mbc5_logo_spoof_device_base(mconfig, GB_ROM_CHONGWU, tag, owner, clock),
		m_protection_checked(0U),
		m_installing_tap(false)
	{
	}

protected:
	virtual void device_start() override ATTR_COLD
	{
		mbc5_logo_spoof_device_base::device_start();

		save_item(NAME(m_protection_checked));
	}

	virtual void device_reset() override ATTR_COLD
	{
		mbc5_logo_spoof_device_base::device_reset();

		m_protection_checked = 0U;

		install_protection_tap();
		m_notif_cart_space = cart_space()->add_change_notifier(
				[this] (read_or_write mode)
				{
					if (u32(mode) & u32(read_or_write::READ))
						install_protection_tap();
				});
	}

private:
	void install_protection_tap()
	{
		if (!m_installing_tap)
		{
			m_installing_tap = true;
			m_protection_tap.remove();
			if (!m_protection_checked)
			{
				m_protection_tap = cart_space()->install_read_tap(
						0x41c3, 0x41c3,
						"protection_tap_r",
						[this] (offs_t offset, u8 &data, u8 mem_mask)
						{
							assert(!m_protection_checked);

							m_protection_checked = 1U;
							m_notif_cart_space.reset();
							m_protection_tap.remove();

							data = 0x5d;
						},
						&m_protection_tap);
			}
			else
			{
				m_notif_cart_space.reset();
			}
			m_installing_tap = false;
		}
	}

	memory_passthrough_handler m_protection_tap;
	util::notifier_subscription m_notif_cart_space;

	u8 m_protection_checked;
	bool m_installing_tap;
};



//**************************************************************************
//  MBC5 variant used by Li Cheng/Niutoude games
//**************************************************************************

class licheng_device : public mbc5_logo_spoof_device_base
{
public:
	licheng_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		mbc5_logo_spoof_device_base(mconfig, GB_ROM_LICHENG, tag, owner, clock)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// install regular MBC5 handlers
		image_init_result const result(mbc5_logo_spoof_device_base::load(message));
		if (image_init_result::PASS != result)
			return result;

		// protection against using a standard MBC5 - actual ignored range uncertain
		// 0x2100 must not be ignored for Cannon Fodder sound
		// 0x2180 must be ignored for FF DX3
		cart_space()->unmap_write(0x2101, 0x2fff);

		// all good
		return image_init_result::PASS;
	}
};



//**************************************************************************
//  New Game Boy Color cartridges with HK0701 and HK0819 boards
//**************************************************************************

class ngbchk_device : public rom_mbc_device_base
{
public:
	ngbchk_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		rom_mbc_device_base(mconfig, GB_ROM_NEWGBCHK, tag, owner, clock),
		m_view_prot(*this, "protection")
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// TODO: ROM addresses seem to be fully decoded with HK0819 (rather than mirroring)

		// set up ROM and RAM
		// TODO: how many RAM bank outputs are actually present?
		if (!install_memory(message, 2, 7))
			return image_init_result::FAIL;

		// install handlers
		cart_space()->install_write_handler(
				0x0000, 0x1fff,
				write8smo_delegate(*this, FUNC(ngbchk_device::enable_ram)));
		cart_space()->install_write_handler(
				0x2000, 0x3fff,
				write8smo_delegate(*this, FUNC(ngbchk_device::bank_switch_fine)));
		cart_space()->install_write_handler(
				0x4000, 0x5fff,
				write8smo_delegate(*this, FUNC(ngbchk_device::bank_switch_coarse)));

		// install protection over the top of high ROM bank
		cart_space()->install_view(
				0x4000, 0x7fff,
				m_view_prot);
		m_view_prot[0].install_read_handler(
				0x4000, 0x4fff, 0x0ff0, 0x0000, 0x0000,
				read8sm_delegate(*this, FUNC(ngbchk_device::protection)));
		m_view_prot[0].unmap_read(0x5000, 0x7fff);

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_reset() override ATTR_COLD
	{
		rom_mbc_device_base::device_reset();

		// TODO: proper reset state
		set_bank_rom_coarse(0);
		set_bank_rom_fine(0);
		set_ram_enable(false);

		m_view_prot.disable();
	}

private:
	void enable_ram(u8 data)
	{
		// TODO: how many bits are checked?
		set_ram_enable(0x0a == data);
	}

	void bank_switch_fine(u8 data)
	{
		set_bank_rom_fine(data & 0x7f);
		LOG(
				"%s: Protection read %s\n",
				machine().describe_context(),
				BIT(data, 7) ? "enabled" : "disabled");
		if (BIT(data, 7))
			m_view_prot.select(0);
		else
			m_view_prot.disable();
	}

	u8 protection(offs_t offset)
	{
		offset >>= 4;
		switch (offset & 0x007)
		{
		default: // just to shut up dumb compilers
		case 0x000:
			return offset;
		case 0x001:
			return offset ^ 0xaa;
		case 0x002:
			return offset ^ 0x55;
		case 0x003:
			return (offset >> 1) | (offset << 7);
		case 0x004:
			return (offset << 1) | (offset >> 7);
		case 0x05:
			return bitswap<8>(offset, 0, 1, 2, 3, 4, 5, 6, 7);
		case 0x06:
			return
					(bitswap<4>(offset | (offset >> 1), 6, 4, 2, 0) << 4) |
					bitswap<4>(offset & (offset >> 1), 6, 4, 2, 0);
		case 0x07:
			return
					(bitswap<4>(offset ^ (~offset >> 1), 6, 4, 2, 0) << 4) |
					bitswap<4>(offset ^ (offset >> 1), 6, 4, 2, 0);
		}
	}

	memory_view m_view_prot;
};



//**************************************************************************
//  Fast Fame VF001 MBC5 variant with protection
//**************************************************************************

class vf001_device : public mbc5_logo_spoof_device_base
{
public:
	vf001_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		mbc5_logo_spoof_device_base(mconfig, GB_ROM_VF001, tag, owner, clock),
		m_bank_rom_upper(*this, "upper"),
		m_bank_mask_rom_upper(0U),
		m_command_preload(0U),
		m_prot_cmd_enable(0U),
		m_prot_cmd_data(0U),
		m_readback_address(0U),
		m_readback_bank(0U),
		m_readback_length(0U),
		m_readback_data{ 0U, 0U, 0U, 0U },
		m_readback_count(0U),
		m_split_address(0U),
		m_split_enable(0U)
	{
	}

	virtual image_init_result load(std::string &message) override ATTR_COLD
	{
		// get the command preload value
		// TODO: add a way to specify this in software lists
		gbxfile::leader_1_0 leader;
		u8 const *extra;
		u32 extralen;
		if (gbxfile::get_data(gbx_footer_region(), leader, extra, extralen))
		{
			if (1 <= extralen)
			{
				m_command_preload = extra[0];
				logerror(
						"GBX format image specifies command preload value 0x%02X\n",
						m_command_preload);
			}
		}

		// install regular MBC5 handlers
		image_init_result const result(mbc5_logo_spoof_device_base::load(message));
		if (image_init_result::PASS != result)
			return result;

		// set up an additional bank for the upper part of the low bank when split
		memory_region *const romregion(cart_rom_region());
		m_bank_mask_rom_upper = device_generic_cart_interface::map_non_power_of_two(
				unsigned(romregion->bytes() / 0x4000),
				[this, base = &romregion->as_u8()] (unsigned entry, unsigned page)
				{
					LOG(
							"Install ROM 0x%06X-0x%06X in upper bank entry %u\n",
							page * 0x4000,
							(page * 0x4000) + 0x3fff,
							entry);
					m_bank_rom_upper->configure_entry(entry, &base[page * 0x4000]);
				});
		m_bank_rom_upper->set_entry(0);

		// install handlers with protection emulation
		cart_space()->install_read_handler(
				0x0000, 0x7fff,
				read8sm_delegate(*this, FUNC(vf001_device::read_rom)));
		cart_space()->install_write_handler(
				0x6000, 0x700f, 0x100f, 0x0000, 0x0000,
				write8sm_delegate(*this, FUNC(vf001_device::prot_cmd)));

		// all good
		return image_init_result::PASS;
	}

protected:
	virtual void device_start() override ATTR_COLD
	{
		mbc5_logo_spoof_device_base::device_start();

		m_prot_cmd_data = 0U;
		m_readback_address = 0U;
		m_readback_bank = 0U;
		std::fill(std::begin(m_readback_data), std::end(m_readback_data), 0U);
		m_split_address = 0U;

		save_item(NAME(m_prot_cmd_enable));
		save_item(NAME(m_prot_cmd_data));
		save_item(NAME(m_readback_address));
		save_item(NAME(m_readback_bank));
		save_item(NAME(m_readback_length));
		save_item(NAME(m_readback_data));
		save_item(NAME(m_readback_count));
		save_item(NAME(m_split_address));
		save_item(NAME(m_split_enable));
	}

	virtual void device_reset() override ATTR_COLD
	{
		mbc5_logo_spoof_device_base::device_reset();

		m_prot_cmd_enable = 0U;
		m_readback_length = 0U;
		m_readback_count = 0U;
		m_split_enable = 0U;
	}

private:
	u8 read_rom(offs_t offset)
	{
		u8 const data(mbc5_logo_spoof_device_base::read_rom(offset));

		if (!m_readback_count && m_readback_length && !machine().side_effects_disabled())
		{
			if ((offset == m_readback_address) && ((BIT(offset, 14) ? bank_rom_fine() : 0) == m_readback_bank))
			{
				LOG(
						"%s: Readback triggered by read from ROM bank 0x%02X address 0x%04X, length = %u\n",
						machine().describe_context(),
						m_readback_bank,
						m_readback_address,
						m_readback_length);
				m_readback_count = m_readback_length;
			}
		}

		if (m_readback_count)
		{
			u8 const index(m_readback_length - m_readback_count);
			if (!machine().side_effects_disabled())
				--m_readback_count;
			LOG(
					"%s: Readback %u = 0x%02X\n",
					machine().describe_context(),
					index,
					m_readback_data[index]);
			return m_readback_data[index];
		}

		bool const split_upper(m_split_enable && (m_split_address <= offset) && (0x4000 > offset));
		return !split_upper ? data : reinterpret_cast<u8 const *>(m_bank_rom_upper->base())[offset];
	}

	void prot_cmd(offs_t offset, u8 data)
	{
		if ((0x1000 == offset) && (0x96 == data))
		{
			m_prot_cmd_enable = 1U;
			m_prot_cmd_data = m_command_preload;
			LOG(
					"%s: Enabled protection commands, initial data = 0x%02X\n",
					machine().describe_context(),
					m_prot_cmd_data);
		}
		else if ((0x100f == offset) && (0x96 == data))
		{
			LOG("%s: Disabled protection commands\n", machine().describe_context());
			m_prot_cmd_enable = 0U;
		}
		else if (m_prot_cmd_enable)
		{
			if (((0x0001 <= offset) && (0x1000 > offset)) || (0x100b <= offset))
			{
				logerror(
						"%s: Unknown protection command address 0x%04X = 0x%02X\n",
						machine().describe_context(),
						offset,
						data);
				return;
			}

			// previous data is rotated right and combined with new data
			m_prot_cmd_data = ((m_prot_cmd_data >> 1) | (m_prot_cmd_data << 7)) ^ data;

			switch (offset)
			{
			case 0x0000:
				m_bank_rom_upper->set_entry(m_prot_cmd_data & m_bank_mask_rom_upper);
				LOG(
						"%s: Bank split upper bank = 0x%02X\n",
						machine().describe_context(),
						m_prot_cmd_data);
				break;
			case 0x1000:
				if (BIT(m_prot_cmd_data, 2))
				{
					m_readback_length = (m_prot_cmd_data & 0x03) + 1;
					LOG(
							"%s: Readback length = %u\n",
							machine().describe_context(),
							m_readback_length);
				}
				break;
			case 0x1001:
			case 0x1002:
				m_readback_address &= BIT(offset, 0) ? 0xff00 : 0x00ff;
				m_readback_address |= u16(m_prot_cmd_data) << (BIT(offset, 0) ? 0 : 8);
				LOG(
						"%s: Readback address = 0x%04X\n",
						machine().describe_context(),
						m_readback_address);
				break;
			case 0x1003:
				m_readback_bank = m_prot_cmd_data;
				LOG(
						"%s: Readback bank = 0x%02X\n",
						machine().describe_context(),
						m_readback_bank);
				break;
			case 0x1004:
			case 0x1005:
			case 0x1006:
			case 0x1007:
				m_readback_data[offset & 0x0003] = m_prot_cmd_data;
				LOG(
						"%s: Readback data %u = 0x%02X\n",
						machine().describe_context(),
						offset & 0x0003,
						m_prot_cmd_data);
				break;
			case 0x1008:
				m_split_enable = (0x0f == (m_prot_cmd_data & 0x0f)) ? 1U : 0U;
				LOG(
						"%s: Bank split %s\n",
						machine().describe_context(),
						m_split_enable ? "enabled" : "disabled");
				break;
			case 0x1009:
			case 0x100a:
				m_split_address &= BIT(offset, 0) ? 0xff00 : 0x00ff;
				m_split_address |= u16(m_prot_cmd_data) << (BIT(offset, 0) ? 0 : 8);
				LOG(
						"%s: Bank split address = 0x%04X\n",
						machine().describe_context(),
						m_split_address);
				break;
			}
		}
	}

	memory_bank_creator m_bank_rom_upper;
	u8 m_bank_mask_rom_upper;
	u8 m_command_preload;

	u8 m_prot_cmd_enable;
	u8 m_prot_cmd_data;

	u16 m_readback_address;
	u8 m_readback_bank;
	u8 m_readback_length;
	u8 m_readback_data[4];
	u8 m_readback_count;

	u16 m_split_address;
	u8 m_split_enable;
};

} // anonymous namespace

} // namespace bus::gameboy


// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_MBC1,     device_gb_cart_interface, bus::gameboy::mbc1_device,        "gb_rom_mbc1",     "Game Boy MBC1 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_MBC5,     device_gb_cart_interface, bus::gameboy::mbc5_device,        "gb_rom_mbc5",     "Game Boy MBC5 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_BBD,      device_gb_cart_interface, bus::gameboy::bbd_device,         "gb_rom_bbd",      "Game Boy BBD Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_DSHGGB81, device_gb_cart_interface, bus::gameboy::dshggb81_device,    "gb_rom_dshggb81", "Game Boy DSHGGB-81 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_SINTAX,   device_gb_cart_interface, bus::gameboy::sintax_device,      "gb_rom_sintax",   "Game Boy Sintax MBC5 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_CHONGWU,  device_gb_cart_interface, bus::gameboy::chongwu_device,     "gb_rom_chongwu",  "Game Boy Chongwu Xiao Jingling Pokemon Pikecho Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_LICHENG,  device_gb_cart_interface, bus::gameboy::licheng_device,     "gb_rom_licheng",  "Game Boy Li Cheng MBC5 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_NEWGBCHK, device_gb_cart_interface, bus::gameboy::ngbchk_device,      "gb_rom_ngbchk",   "Game Boy HK0701/HK0819 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_VF001,    device_gb_cart_interface, bus::gameboy::vf001_device,       "gb_rom_vf001",    "Game Boy Vast Fame VF001 Cartridge")
