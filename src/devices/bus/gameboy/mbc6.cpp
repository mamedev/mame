// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Nintendo Game Boy Memory Bank Controller 6

 Only used for one game, the cartridges contain a Macronix MX29F008TC-14
 Flash ROM (8-bit, 1 MiB), as well as the MBC6 chip, program ROM,
 W24257S-70LL static RAM, MM1134, and CR1616 backup cell.  Supports up to
 1 MiB ROM (128 8 KiB pages) and 32 KiB static RAM (8 4 KiB pages).

 The MBC6 chip seems to have support for an additional ROM chip, as setting
 bit 7 of the page seems to deselect the program ROM.  RAM banking lines
 could theoretically be used for outer ROM banking, allowing up to 8 MiB
 program ROM (1024 8 KiB pages), or 16 MiB program ROM (2048 8 KiB pages)
 if dual ROM chips are supported.

 The MX29F008TC doesn't use either of the standard "boot sector" layouts
 used by MX29LV008CT and MX29F008CB chips.  It seems to use 4 KiB sectors.
 The reported silicon ID is 0xC2 0x81.  It's not clear how write protection
 works.  The game doesn't seem to set up conventional sector protection
 (command 0x60).  It's possible the Flash chips had all sectors protected
 before being installed in cartridges and the Flash write enable output just
 drives RESET# to Vhv to temporarily disable sector protection.

 0x0000-3FFF R  - Fixed ROM bank, always first 16 KiB of ROM.
 0x4000-5FFF RW - Lower selectable ROM/Flash bank.
 0x6000-7FFF RW - Upper selectable ROM/Flash bank.

 0xA000-AFFF RW - Lower selectable static RAM bank if enabled.
 0xB000-BFFF RW - Upper selectable static RAM bank if enabled.

 0x0000-03FF W  - Static RAM enable - write 0x0A on low nybble to enable,
                  any other value to disable.
 0x0400-07FF W  - Select static RAM page mapped at 0xA000.
 0x0800-0BFF W  - Select static RAM page mapped at 0xB000.
 0x0C00-0FFF W  - Least significant bit enables Flash access.
 0x1000-???? W  - Least significant bit enables Flash write.  Seems to be
                  required to enable or disable Flash access, too.  Doesn't
                  prevent writing commands to Flash, only erasing or writing
                  data (can read the silicon ID without setting this).
 0x2000-27FF W  - Select ROM/Flash page mapped at 0x4000.
 0x2800-2FFF W  - Select ROM (0x00) or Flash (0x08) at 0x4000.
 0x3000-37FF W  - Select ROM/Flash page mapped at 0x6000.
 0x3800-3FFF W  - Select ROM (0x00) or Flash (0x08) at 0x6000.

 TODO:
 * Hook up Flash write enable.
 * What's the correct reset state?
 * What happens if bit 7 of the page number is set when Flash is selected?
 * What do the unknown bits in the Flash enable register do?

 ***************************************************************************/

#include "emu.h"
#include "mbc6.h"

#include "cartbase.h"

#include "bus/generic/slot.h"
#include "machine/intelfsh.h"

#include <string>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class mbc6_device : public mbc_8k_device_base
{
public:
	static constexpr feature_type imperfect_features() { return feature::ROM; }

	mbc6_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual std::error_condition load(std::string &message) override ATTR_COLD;
	virtual void unload() override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static inline constexpr unsigned PAGE_RAM_SIZE = 0x1000;

	template <unsigned Bank> u8 read_flash(offs_t offset);
	template <unsigned Bank> void write_flash(offs_t offset, u8 data);
	void bank_switch_rom(offs_t offset, u8 data);
	void select_flash(offs_t offset, u8 data);
	template <unsigned Bank> void bank_switch_ram(u8 data);
	void enable_flash(u8 data);
	void enable_flash_write(u8 data);
	void enable_ram(u8 data);

	bool check_ram(std::string &message) ATTR_COLD;
	void install_ram() ATTR_COLD;

	required_device<intelfsh8_device> m_flash;
	memory_view m_view_rom[2];
	memory_view m_view_ram;
	memory_bank_array_creator<2> m_bank_ram;
	u8 m_bank_mask_ram;

	u8 m_bank_sel_rom[2];
	u8 m_bank_sel_ram[2];
	u8 m_flash_select[2];
	u8 m_flash_enable;
	u8 m_flash_writable;
};


mbc6_device::mbc6_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_8k_device_base(mconfig, GB_ROM_MBC6, tag, owner, clock),
	m_flash(*this, "flash"),
	m_view_rom{ { *this, "romlow"}, { *this, "romhigh" } },
	m_view_ram(*this, "ram"),
	m_bank_ram(*this, { "ramlow", "ramhigh" }),
	m_bank_mask_ram(0U),
	m_bank_sel_rom{ 0U, 0U },
	m_bank_sel_ram{ 0U, 0U },
	m_flash_select{ 0U, 0U },
	m_flash_enable(0U),
	m_flash_writable(0U)
{
}


std::error_condition mbc6_device::load(std::string &message)
{
	// first check that ROM/RAM regions are supportable
	set_bank_bits_rom(7);
	if (!check_rom(message) || !check_ram(message))
		return image_error::BADSOFTWARE;

	// install views for ROM/flash and RAM
	cart_space()->install_view(0x4000, 0x5fff, m_view_rom[0]);
	cart_space()->install_view(0x6000, 0x7fff, m_view_rom[1]);
	cart_space()->install_view(0xa000, 0xbfff, m_view_ram);

	// set up ROM and RAM as appropriate
	install_rom(*cart_space(), m_view_rom[0][0], m_view_rom[1][0]);
	install_ram();

	// install memory controller handlers
	cart_space()->install_write_handler(
			0x0000, 0x03ff,
			emu::rw_delegate(*this, FUNC(mbc6_device::enable_ram)));
	cart_space()->install_write_handler(
			0x0400, 0x07ff,
			emu::rw_delegate(*this, FUNC(mbc6_device::bank_switch_ram<0>)));
	cart_space()->install_write_handler(
			0x0800, 0x0bff,
			emu::rw_delegate(*this, FUNC(mbc6_device::bank_switch_ram<1>)));
	cart_space()->install_write_handler(
			0x0c00, 0x0fff,
			emu::rw_delegate(*this, FUNC(mbc6_device::enable_flash)));
	cart_space()->install_write_handler(
			0x1000, 0x1000, // TODO: what range does this actually respond to?
			emu::rw_delegate(*this, FUNC(mbc6_device::enable_flash_write)));
	cart_space()->install_write_handler(
			0x2000, 0x27ff, 0x0000, 0x0000, 0x1000,
			emu::rw_delegate(*this, FUNC(mbc6_device::bank_switch_rom)));
	cart_space()->install_write_handler(
			0x2800, 0x2fff, 0x0000, 0x0000, 0x1000,
			emu::rw_delegate(*this, FUNC(mbc6_device::select_flash)));

	// install Flash handlers
	m_view_rom[0][1].install_readwrite_handler(
			0x4000, 0x5fff,
			emu::rw_delegate(*this, FUNC(mbc6_device::read_flash<0>)),
			emu::rw_delegate(*this, FUNC(mbc6_device::write_flash<0>)));
	m_view_rom[1][1].install_readwrite_handler(
			0x6000, 0x7fff,
			emu::rw_delegate(*this, FUNC(mbc6_device::read_flash<1>)),
			emu::rw_delegate(*this, FUNC(mbc6_device::write_flash<1>)));

	// all good
	return std::error_condition();
}


void mbc6_device::unload()
{
	memory_region *const nvramregion(cart_nvram_region());
	if (nvramregion && nvramregion->bytes())
		battery_save(nvramregion->base(), nvramregion->bytes());
}


void mbc6_device::device_add_mconfig(machine_config &config)
{
	MACRONIX_29F008TC(config, m_flash);
}


void mbc6_device::device_start()
{
	mbc_8k_device_base::device_start();

	save_item(NAME(m_bank_sel_rom));
	save_item(NAME(m_bank_sel_ram));
	save_item(NAME(m_flash_select));
	save_item(NAME(m_flash_enable));
	save_item(NAME(m_flash_writable));
}


void mbc6_device::device_reset()
{
	mbc_8k_device_base::device_reset();

	m_bank_sel_rom[0] = 0U;
	m_bank_sel_rom[1] = 0U;
	m_bank_sel_ram[0] = 0U;
	m_bank_sel_ram[1] = 0U;
	m_flash_select[0] = 0U;
	m_flash_select[1] = 0U;
	m_flash_enable = 0U;
	m_flash_writable = 0U;

	m_view_rom[0].select(0);
	m_view_rom[1].select(0);
	m_view_ram.disable();

	if (m_bank_mask_ram)
	{
		m_bank_ram[0]->set_entry(0);
		m_bank_ram[1]->set_entry(0);
	}

	set_bank_rom_low(0);
	set_bank_rom_high(0);
}


template <unsigned Bank>
u8 mbc6_device::read_flash(offs_t offset)
{
	offset |= offs_t(m_bank_sel_rom[Bank] & 0x7f) << 13;
	return m_flash->read(offset);
}


template <unsigned Bank>
void mbc6_device::write_flash(offs_t offset, u8 data)
{
	offset |= offs_t(m_bank_sel_rom[Bank] & 0x7f) << 13;
	LOG(
			"%s: Write Flash %s 0x%05X = 0x%02X\n",
			machine().describe_context(),
			Bank ? "high" : "low",
			offset,
			data);
	m_flash->write(offset, data);
}


void mbc6_device::bank_switch_rom(offs_t offset, u8 data)
{
	auto const bank(BIT(offset, 12));
	m_bank_sel_rom[bank] = data;

	if (!m_flash_select[bank])
	{
		if (BIT(data, 7))
		{
			LOG(
					"%s: ROM bank %s unmapped\n",
					machine().describe_context(),
					bank ? "high" : "low");
			m_view_rom[bank].disable(); // is there a chip select for a second program ROM?
		}
		else
		{
			m_view_rom[bank].select(0);
		}
	}

	if (bank)
		set_bank_rom_high(data & 0x7f);
	else
		set_bank_rom_low(data & 0x7f);
}


void mbc6_device::select_flash(offs_t offset, u8 data)
{
	auto const bank(BIT(offset, 12));
	m_flash_select[bank] = BIT(data, 3);
	if (m_flash_select[bank])
	{
		LOG(
				"%s: Flash bank %s selected (%s)\n",
				machine().describe_context(),
				bank ? "high" : "low",
				m_flash_enable ? "enabled" : "disabled");
		if (m_flash_enable)
			m_view_rom[bank].select(1);
		else
			m_view_rom[bank].disable();
	}
	else if (BIT(m_bank_sel_rom[bank], 7))
	{
		LOG(
				"%s: ROM bank %s unmapped\n",
				machine().describe_context(),
				bank ? "high" : "low");
		m_view_rom[bank].disable(); // is there a chip select for a second program ROM?
	}
	else
	{
		LOG(
				"%s: ROM bank %s selected\n",
				machine().describe_context(),
				bank ? "high" : "low");
		m_view_rom[bank].select(0);
	}

	// game writes 0xc6 when selecting ROM during boot - what do the other bits do?
	if (data & ~0x08)
	{
		LOG(
				"%s: ROM/Flash select %s with unknown bits set 0x%02X\n",
				machine().describe_context(),
				bank ? "high" : "low",
				data);
	}
}


template <unsigned Bank>
void mbc6_device::bank_switch_ram(u8 data)
{
	if (m_bank_mask_ram)
	{
		data &= m_bank_mask_ram;
		LOG(
				"%s: Set RAM bank %s = 0x%04X\n",
				machine().describe_context(),
				Bank ? "high" : "low",
				data * PAGE_RAM_SIZE);
		m_bank_ram[Bank]->set_entry(data);
	}
}


void mbc6_device::enable_flash(u8 data)
{
	// game seems to always set Flash write enable before changing this
	if (m_flash_writable)
	{
		m_flash_enable = BIT(data, 0);
		if (m_flash_enable)
		{
			LOG("%s: Flash enabled\n", machine().describe_context());
			if (m_flash_select[0])
				m_view_rom[0].select(1);
			if (m_flash_select[1])
				m_view_rom[1].select(1);
		}
		else
		{
			LOG("%s: Flash disabled\n", machine().describe_context());
			if (m_flash_select[0])
				m_view_rom[0].disable();
			if (m_flash_select[1])
				m_view_rom[1].disable();
		}

		if (data & ~0x01)
		{
			logerror(
					"%s: Flash enable with unknown bits set 0x%02X\n",
					machine().describe_context(),
					data);
		}
	}
	else
	{
		logerror(
				"%s: Flash enable 0x%02X when Flash write disabled\n",
				machine().describe_context(),
				data);
	}
}


void mbc6_device::enable_flash_write(u8 data)
{
	// FIXME: this should also control whether Flash can be erased/written
	m_flash_writable = BIT(data, 0);
	LOG(
			"%s: Flash write %s\n",
			machine().describe_context(),
			m_flash_writable ? "enabled" : "disabled");

	if (data & ~0x01)
	{
		logerror(
				"%s: Flash write enable with unknown bits set 0x%02X\n",
				machine().describe_context(),
				data);
	}
}


void mbc6_device::enable_ram(u8 data)
{
	if (0x0a == (data & 0x0f))
	{
		LOG("%s: Cartridge RAM enabled\n", machine().describe_context());
		m_view_ram.select(0);
	}
	else
	{
		LOG("%s: Cartridge RAM disabled\n", machine().describe_context());
		m_view_ram.disable();
	}
}


bool mbc6_device::check_ram(std::string &message)
{
	memory_region *const ramregion(cart_ram_region());
	memory_region *const nvramregion(cart_nvram_region());
	auto const rambytes(ramregion ? ramregion->bytes() : 0);
	auto const nvrambytes(nvramregion ? nvramregion->bytes() : 0);
	if (rambytes && nvrambytes && ((rambytes & (PAGE_RAM_SIZE - 1)) || (nvrambytes & (PAGE_RAM_SIZE - 1)) || ((nvrambytes - 1) & nvrambytes)))
	{
		message = "Unsupported cartridge RAM size (if not all RAM is battery-backed, battery backed RAM size must be a power of two no smaller than 4 KiB, and total RAM size must be a multiple of 4 KiB)";
		return false;
	}

	auto const ramtotal(rambytes + nvrambytes);
	if (((PAGE_RAM_SIZE < ramtotal) && (ramtotal & (PAGE_RAM_SIZE - 1))) && ((u32(PAGE_RAM_SIZE) << 3) < ramtotal))
	{
		message = "Unsupported cartridge RAM size (must be no larger than 4 KiB, or a multiple of 4 KiB no larger than 32 KiB)";
		return false;
	}

	return true;
}


void mbc6_device::install_ram()
{
	memory_region *const ramregion(this->cart_ram_region());
	memory_region *const nvramregion(this->cart_nvram_region());
	auto const rambytes(ramregion ? ramregion->bytes() : 0);
	auto const nvrambytes(nvramregion ? nvramregion->bytes() : 0);
	auto const ramtotal(rambytes + nvrambytes);
	u8 *const rambase(rambytes ? &ramregion->as_u8() : nullptr);
	u8 *const nvrambase(nvrambytes ? &nvramregion->as_u8() : nullptr);

	if (!ramtotal)
	{
		// need to avoid fatal errors
		m_bank_mask_ram = 0U;
		m_view_ram[0];
	}
	else if (PAGE_RAM_SIZE >= ramtotal)
	{
		// install small amounts of RAM directly - mixed volatile and non-volatile RAM is not supported
		device_generic_cart_interface::install_non_power_of_two<0>(
				nvrambytes ? nvrambytes : rambytes,
				PAGE_RAM_SIZE - 1,
				0,
				0xa000,
				[this, base = nvrambase ? nvrambase : rambase] (offs_t begin, offs_t end, offs_t mirror, offs_t src)
				{
					LOG(
							"Install RAM 0x%05X-0x%05X at 0x%04X-0x%04X mirror 0x%04X\n",
							src,
							src + (end - begin),
							begin,
							end,
							0x1000 | mirror);
					m_view_ram[0].install_ram(begin, end, 0x1000 | mirror, &base[src]);
				});
		m_bank_mask_ram = 0U;
	}
	else
	{
		// install banked RAM, assuming lower pages are battery-backed
		m_bank_mask_ram = device_generic_cart_interface::map_non_power_of_two(
				unsigned(ramtotal / PAGE_RAM_SIZE),
				[this, nvrampages = nvrambytes / PAGE_RAM_SIZE, rambase, nvrambase] (unsigned entry, unsigned page)
				{
					bool const nonvolatile(page < nvrampages);
					u8 *const base(nonvolatile ? nvrambase : rambase);
					auto const offset((page - (nonvolatile ? 0 : nvrampages)) * PAGE_RAM_SIZE);
					LOG(
							"Install %s 0x%05X-0x%05X in bank entry %u\n",
							nonvolatile ? "NVRAM" : "RAM",
							offset,
							offset + (PAGE_RAM_SIZE - 1),
							entry);
					m_bank_ram[0]->configure_entry(entry, &base[offset]);
					m_bank_ram[1]->configure_entry(entry, &base[offset]);
				});
		m_view_ram[0].install_readwrite_bank(0xa000, 0xafff, m_bank_ram[0]);
		m_view_ram[0].install_readwrite_bank(0xb000, 0xbfff, m_bank_ram[1]);
	}

	if (nvrambytes)
	{
		save_pointer(NAME(nvrambase), nvrambytes);
		battery_load(nvrambase, nvrambytes, nullptr);
	}
	if (rambytes)
		save_pointer(NAME(rambase), rambytes);
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_MBC6, device_gb_cart_interface, bus::gameboy::mbc6_device, "gb_rom_mbc6", "Game Boy MBC6 Cartridge")
