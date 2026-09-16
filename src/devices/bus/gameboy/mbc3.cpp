// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Nintendo Game Boy Memory Bank Controller 3

 Includes a real-time clock in addition to the memory controller features.
 The real-time clock can be used with or without a backup battery.  Without
 a backup battery, it just keeps time while the game is running.  Several
 games also used this memory controller without a real-time clock crystal.

 The MBC3 supports up to 2 MiB ROM (128 16 KiB pages) and 32 KiB static RAM
 (4 8 KiB pages).  The MBC30 supports up to 4 MiB ROM (256 16 KiB pages) and
 64 KiB static RAM (8 8 KiB pages).  If RAM bank lines are used for coarse
 ROM banking, up to 8 MiB ROM (512 16 KiB pages) can be supported by the
 MBC3, and up to 32 MiB ROM (2048 16 KiB pages) can be supported by the
 MBC30.

 The MBC30 was only used for a single game.  In practice, 128K*8 static RAM
 chips were used, but only half the space is accessible as the MBC30 chip
 only has three RAM bank output lines.

 0x0000-3FFF R  - Fixed ROM bank, always first page of ROM.
 0x4000-7FFF R  - Selectable ROM bank, page 0-127 (MBC) or 0-255 (MBC30) of
                  ROM.
 0xA000-BFFF RW - Static RAM or real-time clock registers.

 0x0000-1FFF W  - I/O enable - write 0x0A on low nybble to enable static RAM
                  or real-time clock registers, any other value to disable.
 0x2000-3FFF W  - Select ROM page mapped at 0x4000.  Bit 7 is ignored by
                  MBC3; all bits are significant for MBC30 Writing 0 selects
                  page 1.
 0x4000-5FFF W  - Select RAM page or real-time clock register mapped at
                  0xA000 (if enabled).  If bit 3 is clear, static RAM is
                  selected; if bit 3 is set, real-time clock registers are
                  selected.  Bits 2-0 select the RAM page or register.
 0x6000-7FFF W  - Write 0x00 followed by 0x01 to latch real-time clock
                  registers.

 Real-time clock registers:
 0x0 - Seconds.
 0x1 - Minutes.
 0x2 - Hours.
 0x3 - Bits 7-0 of days.
 0x4 - X------- Day counter overflow (set on carry out of bit 8, cleared
                manually).
       -X------ Set to halt real-time clock.
       -------X Bit 8 of days.

 Telefang bootlegs have three additional registers:
 0x5 - Low value.
 0x6 - High value.
 0x7 - Command:
       0x11 Decrement low value.
       0x12 Decrement high value.
       0x41 Add high value to low value.
       0x42 Add low value to high value.
       0x51 Increment low value.
       0x52 Increment high value.

 TODO:
 * How are the static RAM bank outputs set when real-time clock registers
   are selected?
 * What happens if invalid real-time clock registers 5-7 are selected?
 * How do invalid seconds, minutes and hours values roll over?
 * Does MBC30 really have eight ROM bank outputs?  The one game using it
   only uses seven.
 * Does the bootleg Keitai Denjuu Telefang controller actually include a
   real-time clock?  No oscillator crystal is present.

 ***************************************************************************/

#include "emu.h"
#include "mbc3.h"

#include "cartbase.ipp"
#include "cartheader.h"
#include "gbxfile.h"

#include "dirtc.h"

#include "corestr.h"

#include <string>
#include <tuple>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

//**************************************************************************
//  Class declarations
//**************************************************************************

class mbc3_device_base : public mbc_ram_device_base<mbc_dual_device_base>, public device_rtc_interface, public device_nvram_interface
{
protected:
	mbc3_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override ATTR_COLD;

	virtual void nvram_default() override ATTR_COLD;
	virtual bool nvram_read(util::read_stream &file) override ATTR_COLD;
	virtual bool nvram_write(util::write_stream &file) override ATTR_COLD;
	virtual bool nvram_can_write() const override ATTR_COLD;

	std::error_condition install_memory(std::string &message, unsigned highbits, unsigned lowbits) ATTR_COLD;

protected:
	u8 const rtc_select() const { return BIT(m_rtc_select, 3); }
	u8 const rtc_register() const { return m_rtc_select & 0x07; }

	virtual u8 read_rtc(address_space &space);
	virtual void write_rtc(u8 data);

private:
	static inline constexpr u8 RTC_MASK[]{ 0x3f, 0x3f, 0x1f, 0xff, 0xc1 };
	static inline constexpr u8 RTC_ROLLOVER[]{ 0x3c, 0x3c, 0x18, 0x00, 0x00 };

	TIMER_CALLBACK_MEMBER(rtc_advance_seconds);

	void enable_ram_rtc(u8 data);
	void bank_switch_fine(u8 data);
	void select_ram_rtc(u8 data);
	void latch_rtc(u8 data);

	u8 rtc_increment(unsigned index)
	{
		m_rtc_regs[0][index] = (m_rtc_regs[0][index] + 1) & RTC_MASK[index];
		if (RTC_ROLLOVER[index] == (m_rtc_regs[0][index] & RTC_ROLLOVER[index]))
			m_rtc_regs[0][index] = 0U;
		return m_rtc_regs[0][index];
	}

	memory_view m_view_ram;

	emu_timer *m_timer_rtc;
	s64 m_machine_seconds;
	bool m_has_rtc_xtal;
	bool m_has_battery;

	u8 m_rtc_regs[2][5];
	u8 m_rtc_enable;
	u8 m_rtc_select;
	u8 m_rtc_latch;
};


class mbc3_device : public mbc3_device_base
{
public:
	mbc3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual std::error_condition load(std::string &message) override ATTR_COLD;
};


class mbc30_device : public mbc3_device_base
{
public:
	mbc30_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual std::error_condition load(std::string &message) override ATTR_COLD;
};


class tfboot_device : public mbc3_device_base
{
public:
	tfboot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual std::error_condition load(std::string &message) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u8 read_rtc(address_space &space) override;
	virtual void write_rtc(u8 data) override;

private:
	u8 m_protection[2];
};



//**************************************************************************
//  mbc3_device_base
//**************************************************************************

mbc3_device_base::mbc3_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_ram_device_base<mbc_dual_device_base>(mconfig, type, tag, owner, clock),
	device_rtc_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_view_ram(*this, "ram"),
	m_timer_rtc(nullptr),
	m_machine_seconds(0),
	m_has_rtc_xtal(false),
	m_has_battery(false),
	m_rtc_enable(0U),
	m_rtc_select(0U),
	m_rtc_latch(0U)
{
}


std::error_condition mbc3_device_base::install_memory(
		std::string &message,
		unsigned highbits,
		unsigned lowbits)
{
	// check for RTC oscillator and backup battery
	if (loaded_through_softlist())
	{
		// there's a feature tag indicating presence or absence of RTC crystal
		char const *const rtcfeature(get_feature("rtc"));
		if (rtcfeature)
		{
			// explicitly specified in software list
			if (util::streqlower(rtcfeature, "yes") || util::streqlower(rtcfeature, "true"))
			{
				logerror("Real-time clock crystal present\n");
				m_has_rtc_xtal = true;
			}
			else if (util::streqlower(rtcfeature, "no") || util::streqlower(rtcfeature, "false"))
			{
				logerror("No real-time clock crystal present\n");
				m_has_rtc_xtal = false;
			}
			else
			{
				message = "Invalid 'rtc' feature value (must be yes or no)";
				return image_error::BADSOFTWARE;
			}
		}
		else
		{
			logerror("No 'rtc' feature found, assuming no real-time clock crystal present\n");
			m_has_rtc_xtal = false;
		}

		// if there's an NVRAM region, there must be a backup battery
		if (cart_nvram_region())
		{
			logerror("Found 'nvram' region, backup battery must be present\n");
			m_has_battery = true;
		}
		else
		{
			logerror("No 'nvram' region found, assuming no backup battery present\n");
			m_has_battery = true;
		}
	}
	else
	{
		gbxfile::leader_1_0 leader;
		u8 const *extra;
		u32 extralen;
		if (gbxfile::get_data(gbx_footer_region(), leader, extra, extralen))
		{
			m_has_rtc_xtal = bool(leader.rtc);
			m_has_battery = bool(leader.batt);
			logerror(
					"GBX format image specifies %sreal-time clock crystal present, %sbackup battery present\n",
					m_has_rtc_xtal ? "" : "no ",
					m_has_battery ? "" : "no ");
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
				case cartheader::TYPE_MBC3_RTC_BATT:
				case cartheader::TYPE_MBC3_RTC_RAM_BATT:
					m_has_rtc_xtal = true;
					m_has_battery = true;
					break;
				case cartheader::TYPE_MBC3:
					m_has_rtc_xtal = false;
					m_has_battery = false;
					break;
				case cartheader::TYPE_MBC3_RAM:
				case cartheader::TYPE_MBC3_RAM_BATT:
					m_has_rtc_xtal = false;
					m_has_battery = true;
					break;
				default:
					osd_printf_warning(
							"[%s] Unrecognized cartridge type 0x%02X in header, assuming no real-time clock crystal or backup battery present\n",
							tag(),
							carttype);
					m_has_rtc_xtal = false;
					m_has_battery = false;
				}
				logerror(
						"Cartridge type 0x%02X in header, %sreal-time clock crystal present, %sbackup battery present\n",
						carttype,
						m_has_rtc_xtal ? "" : "no ",
						m_has_battery ? "" : "no ");
			}
		}
	}

	// check for valid ROM/RAM regions
	set_bank_bits_rom(highbits, lowbits);
	set_bank_bits_ram(highbits);
	if (!check_rom(message) || !check_ram(message))
		return image_error::BADSOFTWARE;

	// set up ROM and RAM
	cart_space()->install_view(0xa000, 0xbfff, m_view_ram);
	install_rom();
	install_ram(m_view_ram[0]);

	// install bank switching handlers
	cart_space()->install_write_handler(
			0x0000, 0x1fff,
			emu::rw_delegate(*this, FUNC(mbc3_device_base::enable_ram_rtc)));
	cart_space()->install_write_handler(
			0x2000, 0x3fff,
			emu::rw_delegate(*this, FUNC(mbc3_device_base::bank_switch_fine)));
	cart_space()->install_write_handler(
			0x4000, 0x5fff,
			emu::rw_delegate(*this, FUNC(mbc3_device_base::select_ram_rtc)));
	cart_space()->install_write_handler(
			0x6000, 0x7fff,
			emu::rw_delegate(*this, FUNC(mbc3_device_base::latch_rtc)));

	// install real-time clock handlers
	m_view_ram[1].install_read_handler(
			0xa000, 0xbfff,
			emu::rw_delegate(*this, FUNC(mbc3_device_base::read_rtc)));
	m_view_ram[1].install_write_handler(
			0xa000, 0xbfff,
			emu::rw_delegate(*this, FUNC(mbc3_device_base::write_rtc)));

	// if real-time clock crystal is present, start it ticking
	if (m_has_rtc_xtal)
	{
		logerror("Real-time clock crystal present, starting timer\n");
		m_timer_rtc->adjust(attotime(1, 0), 0, attotime(1, 0));
	}

	// all good
	return std::error_condition();
}


void mbc3_device_base::device_start()
{
	mbc_ram_device_base<mbc_dual_device_base>::device_start();

	m_timer_rtc = timer_alloc(FUNC(mbc3_device_base::rtc_advance_seconds), this);

	save_item(NAME(m_rtc_regs));
	save_item(NAME(m_rtc_enable));
	save_item(NAME(m_rtc_select));
	save_item(NAME(m_rtc_latch));
}


void mbc3_device_base::device_reset()
{
	mbc_ram_device_base<mbc_dual_device_base>::device_reset();

	m_rtc_enable = 0U;
	m_rtc_select = 0U;
	m_rtc_latch = 0U;

	set_bank_rom_coarse(0);
	set_bank_rom_fine(1);
	set_bank_ram(0);
	m_view_ram.disable();
}


void mbc3_device_base::rtc_clock_updated(
		int year,
		int month,
		int day,
		int day_of_week,
		int hour,
		int minute,
		int second)
{
	if (!m_has_rtc_xtal && !m_has_battery)
	{
		logerror("No real-time clock crystal or no battery present, not updating for elapsed time\n");
	}
	else if (std::numeric_limits<s64>::min() == m_machine_seconds)
	{
		logerror("Failed to load machine time from previous session, not updating for elapsed time\n");
	}
	else if (BIT(m_rtc_regs[0][4], 6))
	{
		logerror("Real-time clock halted, not updating for elapsed time\n");
	}
	else
	{
		// do a simple seconds elapsed since last run calculation
		system_time current;
		machine().current_datetime(current);
		s64 delta(std::make_signed_t<decltype(current.time)>(current.time) - m_machine_seconds);
		logerror("Previous session time, %d current time %d, delta %d\n", current.time, m_machine_seconds, delta);
		if (0 > delta)
		{
			// This happens if the user runs the emulation faster
			// than real time, exits, and then starts again without
			// waiting for the difference between emulated and real
			// time to elapse.
			logerror("Previous session ended in the future, not updating for elapsed time\n");
		}
		else
		{
			logerror(
					"Time before applying delta %u %02u:%02u:%02u%s\n",
					(u16(BIT(m_rtc_regs[0][4], 0)) << 8) | m_rtc_regs[0][3],
					m_rtc_regs[0][2],
					m_rtc_regs[0][1],
					m_rtc_regs[0][0],
					BIT(m_rtc_regs[0][4], 7) ? " (overflow)" : "");

			// annoyingly, we can get two rollovers if we started with an invalid value
			unsigned seconds(delta % 60);
			delta /= 60;
			if (60 <= m_rtc_regs[0][0])
			{
				m_rtc_regs[0][0] = 0U;
				--seconds;
				++delta;
			}
			if (60 <= (m_rtc_regs[0][0] + seconds))
				++delta;
			m_rtc_regs[0][0] = (m_rtc_regs[0][0] + seconds) % 60;

			// minutes is the same
			unsigned minutes(delta % 60);
			delta /= 60;
			if (60 <= m_rtc_regs[0][1])
			{
				m_rtc_regs[0][1] = 0U;
				--minutes;
				++delta;
			}
			if (60 <= (m_rtc_regs[0][1] + minutes))
				++delta;
			m_rtc_regs[0][1] = (m_rtc_regs[0][1] + minutes) % 60;

			// hours just has a different rollover point
			unsigned hours(delta % 24);
			delta /= 24;
			if (24 <= m_rtc_regs[0][2])
			{
				m_rtc_regs[0][2] = 0U;
				--hours;
				++delta;
			}
			if (24 <= (m_rtc_regs[0][2] + hours))
				++delta;
			m_rtc_regs[0][2] = (m_rtc_regs[0][2] + hours) % 24;

			// days has simple binary rollover
			unsigned days(delta % 256);
			if (256 <= (m_rtc_regs[0][3] + days))
				++delta;
			m_rtc_regs[0][3] += days;

			// set overflow flag if appropriate
			if ((1 < delta) || (BIT(m_rtc_regs[0][4], 0) && delta))
				m_rtc_regs[0][4] |= 0x80;
			m_rtc_regs[0][4] ^= BIT(delta, 0);

			logerror(
					"Time after applying delta %u %02u:%02u:%02u%s\n",
					(u16(BIT(m_rtc_regs[0][4], 0)) << 8) | m_rtc_regs[0][3],
					m_rtc_regs[0][2],
					m_rtc_regs[0][1],
					m_rtc_regs[0][0],
					BIT(m_rtc_regs[0][4], 7) ? " (overflow)" : "");
		}
	}
}


void mbc3_device_base::nvram_default()
{
	// TODO: proper cold RTC state
	m_machine_seconds = std::numeric_limits<s64>::min();
	for (unsigned i = 0U; std::size(m_rtc_regs[0]) > i; ++i)
		m_rtc_regs[0][i] = RTC_MASK[i];
}


bool mbc3_device_base::nvram_read(util::read_stream &file)
{
	if (m_has_battery)
	{
		// read previous machine time (seconds since epoch) and RTC registers
		std::error_condition err;
		std::size_t actual;

		u64 seconds;
		std::tie(err, actual) = read(file, &seconds, sizeof(seconds));
		if (err || (sizeof(seconds) != actual))
			return false;
		m_machine_seconds = big_endianize_int64(seconds);

		std::tie(err, actual) = read(file, &m_rtc_regs[0][0], sizeof(m_rtc_regs[0]));
		if (err || (sizeof(m_rtc_regs[0]) != actual))
			return false;
	}
	else
	{
		logerror("No battery present, not loading real-time clock register contents\n");
	}
	return true;
}


bool mbc3_device_base::nvram_write(util::write_stream &file)
{
	// save current machine time as seconds since epoch and RTC registers
	system_time current;
	machine().current_datetime(current);
	u64 const seconds(big_endianize_int64(s64(std::make_signed_t<decltype(current.time)>(current.time))));
	std::error_condition err;
	std::size_t written;
	std::tie(err, written) = write(file, &seconds, sizeof(seconds));
	if (err)
		return false;
	std::tie(err, written) = write(file, &m_rtc_regs[0][0], sizeof(m_rtc_regs[0]));
	if (err)
		return false;
	return true;
}


bool mbc3_device_base::nvram_can_write() const
{
	return m_has_battery;
}


TIMER_CALLBACK_MEMBER(mbc3_device_base::rtc_advance_seconds)
{
	if (BIT(m_rtc_regs[0][4], 6))
		return;

	if (rtc_increment(0))
		return;
	if (rtc_increment(1))
		return;
	if (rtc_increment(2))
		return;
	if (++m_rtc_regs[0][3])
		return;

	if (BIT(m_rtc_regs[0][4], 0))
	{
		LOG("Day counter overflow");
		m_rtc_regs[0][4] |= 0x80;
	}
	m_rtc_regs[0][4] ^= 0x01;
}


void mbc3_device_base::enable_ram_rtc(u8 data)
{
	m_rtc_enable = (0x0a == (data & 0x0f)) ? 1U : 0U;
	if (!m_rtc_enable)
	{
		LOG(
				"%s: Cartridge RAM and RTC registers disabled\n",
				machine().describe_context());
		m_view_ram.disable();
	}
	else if (rtc_select())
	{
		LOG(
				"%s: RTC register %u enabled\n",
				machine().describe_context(),
				rtc_register());
		m_view_ram.select(1);
	}
	else
	{
		LOG("%s: Cartridge RAM enabled\n", machine().describe_context());
		m_view_ram.select(0);
	}
}


void mbc3_device_base::bank_switch_fine(u8 data)
{
	data &= 0x7f;
	set_bank_rom_fine(data ? data : 1);
}


void mbc3_device_base::select_ram_rtc(u8 data)
{
	// TODO: what happens with the RAM bank outputs when the RTC is selected?
	// TODO: what happens if RTC register 5-7 is selected?
	// TODO: is the high nybble ignored altogether?
	set_bank_rom_coarse(data & 0x07);
	set_bank_ram(data & 0x07);
	m_rtc_select = data;
	if (m_rtc_enable)
	{
		if (BIT(data, 3))
		{
			LOG(
					"%s: RTC register %u enabled\n",
					machine().describe_context(),
					data & 0x07);
			m_view_ram.select(1);
		}
		else
		{
			LOG("%s: Cartridge RAM enabled\n", machine().describe_context());
			m_view_ram.select(0);
		}
	}
}


void mbc3_device_base::latch_rtc(u8 data)
{
	// FIXME: does it just check the least significant bit, or does it look for 0x00 and 0x01?
	LOG("Latch RTC 0x%02X -> 0x%02X\n", m_rtc_latch, data);
	if (!BIT(m_rtc_latch, 0) && BIT(data, 0))
	{
		LOG("%s: Latching RTC registers\n", machine().describe_context());
		std::copy(std::begin(m_rtc_regs[0]), std::end(m_rtc_regs[0]), std::begin(m_rtc_regs[1]));
	}
	m_rtc_latch = data;
}


u8 mbc3_device_base::read_rtc(address_space &space)
{
	u8 const reg(rtc_register());
	if (std::size(m_rtc_regs[1]) > reg)
	{
		LOG(
				"%s: Read RTC register %u = 0x%02X\n",
				machine().describe_context(),
				reg,
				m_rtc_regs[1][reg]);
		return m_rtc_regs[1][reg];
	}
	else
	{
		LOG(
				"%s: Read invalid RTC register %u\n",
				machine().describe_context(),
				reg);
		return space.unmap();
	}
}


void mbc3_device_base::write_rtc(u8 data)
{
	u8 const reg(rtc_register());
	if (std::size(m_rtc_regs[0]) > reg)
	{
		LOG(
				"%s: Write RTC register %u = 0x%02X\n",
				machine().describe_context(),
				reg,
				data);
		if (4U == reg)
		{
			// TODO: are bits 5-1 physically present, and if not, what do they read as?
			// TODO: how does halting the RTC interact with the prescaler?
			data &= 0xc1;
			m_rtc_regs[0][reg] = data;
		}
		else
		{
			m_rtc_regs[0][reg] = data;
		}
	}
	else
	{
		LOG(
				"%s: Write invalid RTC register %u = 0x%02X\n",
				machine().describe_context(),
				reg,
				data);
	}
}



//**************************************************************************
//  mbc3_device
//**************************************************************************

mbc3_device::mbc3_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc3_device_base(mconfig, GB_ROM_MBC3, tag, owner, clock)
{
}


std::error_condition mbc3_device::load(std::string &message)
{
	return install_memory(message, 2, 7);
}



//**************************************************************************
//  mbc30_device
//**************************************************************************

mbc30_device::mbc30_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc3_device_base(mconfig, GB_ROM_MBC30, tag, owner, clock)
{
}


std::error_condition mbc30_device::load(std::string &message)
{
	return install_memory(message, 3, 8);
}



//**************************************************************************
//  tfboot_device
//**************************************************************************

tfboot_device::tfboot_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc3_device_base(mconfig, GB_ROM_TFANGBOOT, tag, owner, clock),
	m_protection{ 0U, 0U }
{
}


std::error_condition tfboot_device::load(std::string &message)
{
	return install_memory(message, 2, 7);
}


void tfboot_device::device_start()
{
	mbc3_device_base::device_start();

	save_item(NAME(m_protection));
}


void tfboot_device::device_reset()
{
	mbc3_device_base::device_reset();

	m_protection[0] = 0U;
	m_protection[1] = 0U;
}


u8 tfboot_device::read_rtc(address_space &space)
{
	u8 const reg(rtc_register());
	switch (reg)
	{
	case 0x05:
	case 0x06:
		return m_protection[BIT(reg, 0)];
	case 0x07:
		return 0U;
	default:
		return mbc3_device_base::read_rtc(space);
	}
}


void tfboot_device::write_rtc(u8 data)
{
	u8 const reg(rtc_register());
	switch (reg)
	{
	case 0x05:
	case 0x06:
		m_protection[BIT(reg, 0)] = data;
		break;
	case 0x07:
		LOG("%s: Protection command 0x%02X\n", machine().describe_context(), data);
		switch (data)
		{
		case 0x11:
		case 0x12:
			--m_protection[BIT(data, 0)];
			break;
		case 0x41:
		case 0x42:
			m_protection[BIT(data, 0)] += m_protection[BIT(~data, 0)];
			break;
		case 0x51:
		case 0x52:
			++m_protection[BIT(data, 0)];
			break;
		default:
			logerror(
					"%s: Unknown protection command 0x%02X\n",
					machine().describe_context(),
					data);
		}
		break;
	default:
		mbc3_device_base::write_rtc(data);
	}
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_MBC3,      device_gb_cart_interface, bus::gameboy::mbc3_device,   "gb_rom_mbc3",   "Game Boy MBC3 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_MBC30,     device_gb_cart_interface, bus::gameboy::mbc30_device,  "gb_rom_mbc30",  "Game Boy MBC30 Cartridge")
DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_TFANGBOOT, device_gb_cart_interface, bus::gameboy::tfboot_device, "gb_rom_tfboot", "Game Boy Telefang bootleg Cartridge")
