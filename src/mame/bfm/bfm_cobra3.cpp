// license:BSD-3-Clause
// copyright-holders:David Haywood, James Wallace, blueonesarefaster, MagikalUnicorn

/* Bellfruit SWP (Skill With Prizes) Video hardware
    aka Cobra 3

   Telly Addicts user notes:
   - Blank NVRAM produces a RAM ERROR on every startup.  Setting the stored
     volume does not clear this error, but it does not prevent the game from
     running.  The volume initially starts at its minimum setting.  From
     attract mode, toggle Refill/Volume Setup Mode (R), adjust it with Up/Down,
     press Start to exit, then toggle Refill/Volume Setup Mode off.
   - To use the test routines, open the Back and Front Doors (T), then press
     Test (F1).  Use Up/Down to choose a test and Start to enter or leave it.
     METER TEST requires Refill/Volume Setup Mode to be on before entering and
     off again before leaving.
   - With the doors open, pressing Test twice within one second supplies two
     demonstration credits.  This is free play rather than an automatic demo:
     the player must still answer the questions.  Counters and payouts remain
     disabled.
   - CASHFLOW TEST is a read-only audit display.  Cash and meter counters do
     not advance while the test routines are active.
   - Inserting £1 or 20p coins with Refill/Volume Setup Mode active replenishes
     the corresponding tube and records CASH REFILL rather than CASH IN.
   - The Initial Tube Fill adjusters are sampled when the machine starts.  At
     the default 100%, the £1 tube holds 40 coins (£40) and the 20p tube holds
     150 coins (£30), allowing immediate payouts.  Restart after changing an
     initial fill setting.

   Other title user notes:
   - Radio Times accepts £1 (5), 50p (6), 20p (7) and 10p (8).  Its left A-D
     controls use A/B/C/D, its right A-D controls use Z/X/V/N, and Start is 1.
   - Top of the Pops and The Phrase That Pays normally accept £1 (5), 50p (6),
     20p (7) and £2 (8).  Although their software recognises 10p and 5p codes,
     those denominations are inhibited and are not exposed as player inputs.
   - Top of the Pops identifies its payout unit as a £1 payslide.  The Phrase
     That Pays identifies its payout unit as a £1 hopper.
   - In The Phrase That Pays, after inserting credit, A (Right) (Left Shift)
     selects the 50p game and C (Right) (X) selects the £1 game.  Spin (1)
     starts the selected game.

   BTANB:
   - Pressing Test with the Back and Front Doors closed can skip or interrupt
     the current video and leave its overlay out of position.  This is not a
     valid operating sequence; the doors must be opened before using Test.
   - Enabling the Demo Sounds DIL does not make every attract sequence play
     sound.  It permits demo sounds, which the game uses for only a proportion
     of attract sequences.
   - RESET ERROR 1 is the game's anti-tamper response to fewer than five
     detected resets, not an emulation failure.  Close the Back and Front
     Doors and allow the alarm delay to expire to clear it.
   - RESET ERROR 2 has the same recovery procedure as RESET ERROR 1, but its
     alarm delay takes longer to expire.
   - Top of the Pops uses generic music tracks rather than the chart recordings
     represented by its questions.  This is assumed to be a licensing choice,
     not missing media or incorrect audio emulation.
*/

#include "emu.h"

#include "bus/nscsi/cd.h"
#include "bus/rs232/rs232.h"
#include "machine/68340.h"
#include "machine/bacta_datalogger.h"
#include "machine/meters.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/nvram.h"
#include "machine/rescap.h"
#include "machine/scc66470.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/flt_vol.h"
#include "sound/tms320av110.h"
#include "sound/ymz280b.h"
#include "video/ramdac.h"
#include "video/sti3400.h"

#include "screen.h"
#include "speaker.h"

#include "c3_ppays.lh"
#include "c3_rtime.lh"
#include "c3_telly.lh"
#include "c3_totp.lh"

#define LOG_UNKNOWN (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

namespace {

class bfm_cobra3_state : public driver_device
{
public:
	bfm_cobra3_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD;

	void bfm_cobra3(machine_config &config) ATTR_COLD;
	void c3_ppays(machine_config &config) ATTR_COLD;
	void c3_telly(machine_config &config) ATTR_COLD;

	int meter_sense_r();
	ioport_value coin_acceptor_r();
	int pound_tube_low_r();
	int twenty_p_tube_low_r();
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	void video_start() override ATTR_COLD;
	void video_reset() override ATTR_COLD;

private:
	static constexpr unsigned NVRAM_BYTES = 16 * 1024;

	void update_lamps();
	void update_diverters();
	void lamp_latch_w(u16 data, u16 mem_mask);
	void lamp_port_a_w(u8 data);
	void output_latch_w(u16 data, u16 mem_mask);
	void coin_lockouts_w(u8 enables);
	void diverter_latch_w(u16 data, u16 mem_mask);

	void volume_step(bool direction);
	void av110_reset_strobe_w(u8 data);

	u16 io_r(offs_t offset, u16 mem_mask);
	void io_w(offs_t offset, u16 data, u16 mem_mask);
	u16 mem_r(offs_t offset, u16 mem_mask = ~0);
	void mem_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scc_scanline);

	void bfm_cobra3_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
	void scc66470_map(address_map &map) ATTR_COLD;

	// Main board
	required_device<m68340_cpu_device> m_maincpu;
	required_region_ptr<u16> m_cpuregion;
	memory_share_creator<u16> m_mainram;
	required_device<ncr5380_device> m_scsic;
	required_device<watchdog_timer_device> m_watchdog;

	// Sound
	required_device<tms320av110_device> m_av110;
	required_device<ymz280b_device> m_ymz;
	required_device_array<filter_volume_device, 2> m_volume_filter;

	// Video
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ramdac_device> m_ramdac;
	required_device<scc66470_device> m_scc66470;
	required_device<sti3400_device> m_sti3400;
	std::unique_ptr<u8[]> m_scc_line_buffer;
	bitmap_rgb32 m_scc_bitmap;

	// Cabinet I/O
	required_ioport_array<5> m_strobein;
	required_ioport m_iostatus;
	optional_ioport m_coin_inputs;
	optional_ioport_array<2> m_initial_tube_fill;
	required_device<meters_device> m_meters;
	optional_device<hopper_device> m_hopper;
	output_finder<24> m_lamps;
	output_finder<5> m_coin_lockouts;
	output_finder<4> m_diverters;

	u8 m_active_strobe = 0;
	u8 m_volume = 0;
	u16 m_lamp_latch = 0;
	u8 m_lamp_port_a = 0;
	u16 m_output_latch = 0;
	u16 m_diverter_latch = 0;
	u8 m_pound_tube_level = 0;
	u8 m_twenty_p_tube_level = 0;
	bool m_tube_levels_initialized = false;
};

bfm_cobra3_state::bfm_cobra3_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_maincpu(*this, "maincpu")
	, m_cpuregion(*this, "maincpu")
	, m_mainram(*this, "nvram", NVRAM_BYTES, ENDIANNESS_BIG)
	, m_scsic(*this, "ncr5380")
	, m_watchdog(*this, "watchdog")
	, m_av110(*this, "av110")
	, m_ymz(*this, "ymz280b")
	, m_volume_filter(*this, { "volume_l", "volume_r" })
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
	, m_ramdac(*this, "ramdac")
	, m_scc66470(*this, "scc66470")
	, m_sti3400(*this, "sti3400")
	, m_strobein(*this, "STROBE%u", 0)
	, m_iostatus(*this, "IOSTATUS")
	, m_coin_inputs(*this, "COINS")
	, m_initial_tube_fill(*this, "TUBE%u", 0U)
	, m_meters(*this, "meters")
	, m_hopper(*this, "hopper")
	, m_lamps(*this, "lamp%u", 0U)
	, m_coin_lockouts(*this, "coin_lockout%u", 0U)
	, m_diverters(*this, "diverter%u", 0U)
{
}

void bfm_cobra3_state::update_lamps()
{
	for (unsigned i = 0; i < 16; i++)
		m_lamps[i] = BIT(m_lamp_latch, i);

	for (unsigned i = 0; i < 8; i++)
		m_lamps[16 + i] = BIT(m_lamp_port_a, i);
}

void bfm_cobra3_state::update_diverters()
{
	for (unsigned i = 0; i < 4; i++)
		m_diverters[i] = m_hopper && BIT(m_diverter_latch, i);
}

void bfm_cobra3_state::lamp_latch_w(u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_lamp_latch);
	update_lamps();
}

void bfm_cobra3_state::lamp_port_a_w(u8 data)
{
	m_lamp_port_a = data;
	update_lamps();
}

void bfm_cobra3_state::output_latch_w(u16 data, u16 mem_mask)
{
	// This address selects one shared 16-bit latch, including on byte writes.
	u16 const previous = m_output_latch;
	COMBINE_DATA(&m_output_latch);

	for (unsigned i = 0; i < 4; i++)
		m_meters->update(i, BIT(m_output_latch, i));

	if (m_initial_tube_fill[0].found() && m_initial_tube_fill[1].found() && BIT(m_strobein[2]->read(), 2))
	{
		u8 const rising = BIT(m_output_latch, 4, 3) & ~BIT(previous, 4, 3);

		if (BIT(rising, 0) && m_twenty_p_tube_level) // triac A: 20p payslide
			m_twenty_p_tube_level--;
		if (BIT(rising, 2) && m_pound_tube_level) // triac C: £1 payslide
			m_pound_tube_level--;
	}

	if (BIT(previous, 15) && !BIT(m_output_latch, 15))
		volume_step(BIT(m_output_latch, 7));

	m_watchdog->reset_line_w(BIT(m_output_latch, 8));

	for (unsigned i = 0; i < 5; i++)
	{
		if (BIT(m_output_latch, i + 10))
			m_active_strobe = i;
	}
}

void bfm_cobra3_state::diverter_latch_w(u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_diverter_latch);
	update_diverters();

	if (m_hopper)
	{
		// TPTP calls bits 0-3 Divert 0-3.  It asserts 1 and 3 together once the
		// recorded £1 hopper level reaches its £70 float, apparently routing
		// further £1 coins to the cash box.
		if (m_diverter_latch & ~0x000f)
			LOGMASKED(LOG_UNKNOWN, "%s: unimplemented diverter output data %04x mask %04x\n", machine().describe_context(), data, mem_mask);
	}
	else
	{
		LOGMASKED(LOG_UNKNOWN, "%s: unimplemented diverter output data %04x mask %04x\n", machine().describe_context(), data, mem_mask);
	}
}

void bfm_cobra3_state::coin_lockouts_w(u8 enables)
{
	bool const lockouts[] =
	{
		!BIT(enables, 3), // £1
		!BIT(enables, 2), // 50p
		!BIT(enables, 1), // 20p
		!BIT(enables, 0), // 10p
		!BIT(enables, 4)  // fifth validator channel
	};

	for (unsigned channel = 0; channel < std::size(lockouts); channel++)
		m_coin_lockouts[channel] = lockouts[channel];

	// The encoded acceptor uses the fourth logical coin input for £2, which has no inhibit output.
	unsigned const mapped_channels = m_coin_inputs.found() ? 3 : std::size(lockouts);
	for (unsigned channel = 0; channel < mapped_channels; channel++)
		machine().bookkeeping().coin_lockout_w(channel, lockouts[channel]);
	if (m_coin_inputs.found())
		machine().bookkeeping().coin_lockout_w(3, false);
}

void bfm_cobra3_state::volume_step(bool direction)
{
	if (!direction)
	{
		if (m_volume < 31)
			m_volume++;
	}
	else if (m_volume)
	{
		m_volume--;
	}

	float const fraction = (32 - m_volume) / 32.0f;
	m_volume_filter[0]->set_gain(fraction);
	m_volume_filter[1]->set_gain(fraction);
}

void bfm_cobra3_state::av110_reset_strobe_w(u8)
{
	// This decoded write pulses the AV110's active-low RESET input.
	m_av110->reset_w(0);
	m_av110->reset_w(1);
}

u16 bfm_cobra3_state::io_r(offs_t offset, u16 mem_mask)
{
	offset &= 0x7ff;

	switch ((offset * 2) & 0xf00)
	{
		case 0x300: // YMZ stereo sound accesses
			if (ACCESSING_BITS_0_7)
			{
				return m_ymz->read(offset & 1);
			}
			break;

		case 0x400:
			return (m_strobein[m_active_strobe]->read() << 8) | m_iostatus->read();

		case 0x500: // SCSI DMA
			if (ACCESSING_BITS_8_15)
			{
				return m_scsic->dma_r() << 8;
			}
			break;

		case 0x600: // RAMDAC palette read
			if (ACCESSING_BITS_0_7 && ((offset & 7) == 1))
				return m_ramdac->pal_r();
			break;

		case 0x800: // Phrase That Pays hopper opto inputs
			if (m_hopper)
				return m_hopper->line_r() ? 0 : 0x0002;
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_UNKNOWN, "%s: unknown 0x800 input read offset %08x mask %04x\n", machine().describe_context(), offset * 2, mem_mask);
			break;

		default:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_UNKNOWN, "%s: unknown I/O read offset %08x mask %04x\n", machine().describe_context(), offset * 2, mem_mask);
			break;
	}

	return 0;
}

void bfm_cobra3_state::io_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset &= 0x7ff;

	switch ((offset * 2) & 0xf00)
	{
		case 0x000:
			lamp_latch_w(data, mem_mask);
			break;

		case 0x100:
			if (ACCESSING_BITS_8_15)
				coin_lockouts_w(data >> 8);
			break;

		case 0x200:
			output_latch_w(data, mem_mask);
			break;

		case 0x300:
			if (ACCESSING_BITS_0_7)
				m_ymz->write(offset & 1, data);
			break;

		case 0x500: // SCSI DMA
			if (ACCESSING_BITS_8_15)
				m_scsic->dma_w(data >> 8);
			break;

		case 0x700: // RAMDAC palette write
			if (ACCESSING_BITS_0_7)
			{
				switch (offset & 7)
				{
					case 0:
						m_ramdac->index_w(data);
						break;

					case 1:
						m_ramdac->pal_w(data);
						break;

					case 2:
						m_ramdac->mask_w(data);
						break;

					case 3:
						m_ramdac->index_r_w(data);
						break;
				}
			}
			break;

		case 0x900: // coin diverter outputs
			diverter_latch_w(data, mem_mask);
			break;

		case 0xa00: // hopper drive outputs
		{
			u16 const handled = m_hopper ? 0x0020 : 0x0000;
			if (m_hopper && ACCESSING_BITS_0_7)
				m_hopper->motor_w(BIT(data, 5));
			if (data & mem_mask & ~handled)
				LOGMASKED(LOG_UNKNOWN, "%s: unimplemented hopper drive output write offset %08x data %04x mask %04x\n", machine().describe_context(), offset * 2, data, mem_mask);
			break;
		}

		default:
			LOGMASKED(LOG_UNKNOWN, "%s: unknown I/O write offset %08x data %04x mask %04x\n", machine().describe_context(), offset * 2, data, mem_mask);
			break;
	}
}

u16 bfm_cobra3_state::mem_r(offs_t offset, u16 mem_mask)
{
	u16 const cs = m_maincpu->get_cs(offset * 2);

	switch (cs)
	{
		case 1: // ROM
			return m_cpuregion[offset & (m_cpuregion.length() - 1)];

		case 2: // NVRAM
			return m_mainram[offset & (m_mainram.length() - 1)];

		case 3: // I/O
			return io_r(offset, mem_mask);

		case 4: // SCSI controller
			if (ACCESSING_BITS_8_15)
				return m_scsic->read(offset & 0x0f) << 8;
			break;

		default:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_UNKNOWN, "%s: unknown read offset %08x mask %04x CS%d\n", machine().describe_context(), offset * 2, mem_mask, cs);
			break;
	}

	return 0;
}

void bfm_cobra3_state::mem_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 const cs = m_maincpu->get_cs(offset * 2);

	switch (cs)
	{
		case 1: // ROM
			LOGMASKED(LOG_UNKNOWN, "%s: write to ROM offset %08x data %04x mask %04x\n", machine().describe_context(), offset * 2, data, mem_mask);
			break;

		case 2: // NVRAM
			COMBINE_DATA(&m_mainram[offset & (m_mainram.length() - 1)]);
			break;

		case 3: // I/O
			io_w(offset, data, mem_mask);
			break;

		case 4: // SCSI controller
			if (ACCESSING_BITS_8_15)
				m_scsic->write(offset & 0x0f, data >> 8);
			break;

		default:
			LOGMASKED(LOG_UNKNOWN, "%s: unknown write offset %08x data %04x mask %04x CS%d\n", machine().describe_context(), offset * 2, data, mem_mask, cs);
			break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(bfm_cobra3_state::scc_scanline)
{
	rectangle const &visible = m_screen->visible_area();
	if ((param < visible.top()) || (param > visible.bottom()))
		return;

	u32 *const destination = &m_scc_bitmap.pix(param, visible.left());

	if (m_scc66470->display_enabled())
	{
		m_scc66470->line(param, m_scc_line_buffer.get(), visible.width());
		pen_t const *const pens = m_palette->pens();
		for (int x = 0; x != visible.width(); x++)
		{
			u8 const pen = m_scc_line_buffer[x];
			// The Cobra mixer selects external MPEG video for palette index 0xfe.
			if (pen == 0xfe)
				destination[x] = rgb_t::transparent();
			else
				destination[x] = pens[pen];
		}
	}
	else
	{
		std::fill_n(destination, visible.width(), rgb_t::transparent());
	}
}

u32 bfm_cobra3_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	rectangle const &visible = screen.visible_area();

	if (m_sti3400->video_valid())
	{
		bitmap_rgb32 const &source = m_sti3400->bitmap();
		rectangle video(visible);
		video.set_size(std::min(source.width() * 2, visible.width()), std::min(source.height(), visible.height()));
		video.set_origin(visible.left() + (visible.width() - video.width()) / 2, visible.top() + (visible.height() - video.height()) / 2);

		// SCC66470 8-bit output repeats each stored pixel twice horizontally.
		s32 const startx = (((source.width() * 2 - video.width()) / 2) - video.left()) * 0x8000;
		s32 const starty = (((source.height() - video.height()) / 2) - video.top()) * 0x10000;
		copyrozbitmap(bitmap, video & cliprect, source, startx, starty, 0x8000, 0, 0, 0x10000, false);
	}

	copybitmap_transalpha(bitmap, m_scc_bitmap, 0, 0, 0, 0, visible & cliprect);

	return 0;
}

void bfm_cobra3_state::bfm_cobra3_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(bfm_cobra3_state::mem_r), FUNC(bfm_cobra3_state::mem_w));
	map(0x00800000, 0x009fffff).m(m_scc66470, FUNC(scc66470_device::map)).cswidth(16);
	map(0x00a40000, 0x00a4007f).m(m_sti3400, FUNC(sti3400_device::map));
	map(0x00a80000, 0x00a80001).w(FUNC(bfm_cobra3_state::av110_reset_strobe_w)).umask16(0x00ff);
	map(0x00a81000, 0x00a810ff).m(m_av110, FUNC(tms320av110_device::map)).umask16(0x00ff);
}

void bfm_cobra3_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw(m_ramdac, FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void bfm_cobra3_state::scc66470_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram();
}

void bfm_cobra3_state::machine_start()
{
	// Address-line mirroring below uses the allocation length as a mask.
	assert(std::has_single_bit(m_cpuregion.length()));
	assert(std::has_single_bit(m_mainram.length()));

	save_item(NAME(m_active_strobe));
	save_item(NAME(m_volume));
	save_item(NAME(m_lamp_latch));
	save_item(NAME(m_lamp_port_a));
	save_item(NAME(m_output_latch));
	save_item(NAME(m_diverter_latch));
	save_item(NAME(m_pound_tube_level));
	save_item(NAME(m_twenty_p_tube_level));
	save_item(NAME(m_tube_levels_initialized));
	machine().save().register_postload(save_prepost_delegate(FUNC(bfm_cobra3_state::update_lamps), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(bfm_cobra3_state::update_diverters), this));
}

void bfm_cobra3_state::machine_reset()
{
	if (!m_tube_levels_initialized && m_initial_tube_fill[0].found() && m_initial_tube_fill[1].found())
	{
		m_pound_tube_level = (m_initial_tube_fill[0]->read() * 40 + 50) / 100; // £40 capacity in £1 coins
		m_twenty_p_tube_level = (m_initial_tube_fill[1]->read() * 150 + 50) / 100; // £30 capacity in 20p coins
		m_tube_levels_initialized = true;
	}
}

void bfm_cobra3_state::video_start()
{
	m_scc_bitmap.allocate(m_screen->width(), m_screen->height());
	m_scc_line_buffer = std::make_unique<u8[]>(m_screen->visible_area().width());
	m_scc_bitmap.fill(rgb_t::transparent());

	// Earlier scanlines cannot be reconstructed from the restored SCC state.
	save_item(NAME(m_scc_bitmap));
}

void bfm_cobra3_state::video_reset()
{
	m_scc_bitmap.fill(rgb_t::transparent());
}

void bfm_cobra3_state::bfm_cobra3(machine_config &config)
{
	M68340(config, m_maincpu, 16'000'000); // TODO: verify clock source and frequency
	m_maincpu->set_addrmap(AS_PROGRAM, &bfm_cobra3_state::bfm_cobra3_map);
	m_maincpu->pa_out_callback().set(FUNC(bfm_cobra3_state::lamp_port_a_w));
	mc68340_serial_module_device &serial(*m_maincpu->subdevice<mc68340_serial_module_device>("serial"));
	serial.a_tx_cb().set("rs232_port1", FUNC(rs232_port_device::write_txd));
	serial.b_tx_cb().set("bacta", FUNC(bacta_datalogger_device::write_txd));

	rs232_port_device &rs232_port1(RS232_PORT(config, "rs232_port1", default_rs232_devices, nullptr));
	rs232_port1.rxd_handler().set(serial, FUNC(mc68340_serial_module_device::rx_a_w));
	rs232_port1.cts_handler().set(serial, FUNC(mc68340_serial_module_device::ip0_w));

	bacta_datalogger_device &bacta(BACTA_DATALOGGER(config, "bacta"));
	bacta.rxd_handler().set(serial, FUNC(mc68340_serial_module_device::rx_b_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen"));
	// The SCC66470 produces a pixel clock at half its oscillator frequency.
	// Cobra uses its 768-pixel, 280-visible-line mode in a 312-line field.
	screen.set_raw(30_MHz_XTAL / 2, 960, 0, 768, 312, 32, 312);
	screen.set_screen_update(FUNC(bfm_cobra3_state::screen_update));
	screen.screen_vblank().set(m_sti3400, FUNC(sti3400_device::vblank_w));

	PALETTE(config, m_palette).set_entries(256);

	RAMDAC(config, m_ramdac, m_palette); // MUSIC Semiconductor TR9C1710 RAMDAC
	m_ramdac->set_addrmap(0, &bfm_cobra3_state::ramdac_map);
	m_ramdac->set_split_read(1);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	FILTER_VOLUME(config, m_volume_filter[0]).add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	FILTER_VOLUME(config, m_volume_filter[1]).add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	YMZ280B(config, m_ymz, 16.9344_MHz_XTAL);
	m_ymz->add_route(0, m_volume_filter[0], 1.0);
	m_ymz->add_route(1, m_volume_filter[1], 1.0);

	TMS320AV110(config, m_av110, 24_MHz_XTAL);
	// Cobra enables decoder modes that require the optional external DRAM.
	m_av110->set_external_dram(true);
	// AV110 REQ and MC68340 DREQ2 are both active low.
	m_av110->req().set(m_maincpu, FUNC(m68340_cpu_device::dma_dreq2_w));
	m_av110->add_route(0, m_volume_filter[0], 1.0);
	m_av110->add_route(1, m_volume_filter[1], 1.0);

	SCC66470(config, m_scc66470, 30_MHz_XTAL);
	m_scc66470->set_addrmap(0, &bfm_cobra3_state::scc66470_map);
	m_scc66470->set_screen("screen");
	m_scc66470->irq().set_inputline(m_maincpu, 5);
	TIMER(config, "scc_scanline").configure_scanline(FUNC(bfm_cobra3_state::scc_scanline), m_screen, 0, 1);

	STI3400(config, m_sti3400, 0); // decoder clock and external-memory cycle timing are not modelled
	m_sti3400->set_dram_size(1024 * 1024); // Cobra's buffer pointers cover a 1 MiB address space
	m_sti3400->irq().set_inputline(m_maincpu, 6);

	nscsi_bus_device &scsi(NSCSI_BUS(config, "scsi"));
	// Phrase That Pays peaks near 418 KiB/s.
	auto &cdrom(NSCSI_CDROM(config, "cdrom"));
	scsi.set_external_device(2, cdrom);

	NCR5380(config, m_scsic);
	scsi.set_external_device(6, m_scsic);
	m_scsic->drq_handler().set(m_maincpu, FUNC(m68340_cpu_device::dma_dreq1_w)).invert();

	// Provisional values match the watchdog model used by earlier Bellfruit drivers.
	// TODO: Confirm the R/C values against Cobra 3 hardware.
	WATCHDOG_TIMER(config, m_watchdog).set_time(PERIOD_OF_555_MONOSTABLE(RES_K(120), CAP_N(100)));
	METERS(config, m_meters).set_number(4);
}

void bfm_cobra3_state::c3_ppays(machine_config &config)
{
	bfm_cobra3(config);

	// TODO: Verify the hopper pulse period against the cabinet hardware.
	HOPPER(config, m_hopper, attotime::from_msec(100));
}

void bfm_cobra3_state::c3_telly(machine_config &config)
{
	bfm_cobra3(config);

	m_meters->set_number(2);
}

int bfm_cobra3_state::meter_sense_r()
{
	for (unsigned i = 0; i < 4; i++)
	{
		if (m_meters->get_activity(i))
			return 1;
	}

	return 0;
}

ioport_value bfm_cobra3_state::coin_acceptor_r()
{
	// The later Cobra games decode six denominations from five acceptor lines.
	switch (m_coin_inputs->read())
	{
		case 0x01: return 0x15; // £1
		case 0x02: return 0x0b; // 50p
		case 0x04: return 0x0d; // 20p
		case 0x08: return 0x13; // 10p
		case 0x10: return 0x01; // 5p
		case 0x20: return 0x1f; // £2
		default:   return 0x00;
	}
}

int bfm_cobra3_state::pound_tube_low_r()
{
	return m_pound_tube_level <= 13; // the level switch opens above £13
}

int bfm_cobra3_state::twenty_p_tube_low_r()
{
	return m_twenty_p_tube_level <= 23; // the level switch opens above £4.60
}

INPUT_CHANGED_MEMBER(bfm_cobra3_state::coin_inserted)
{
	if (!newval || !BIT(m_strobein[2]->read(), 2))
		return;

	if ((param == 100) && (m_pound_tube_level < 40)) // £40 capacity
		m_pound_tube_level++;
	else if ((param == 20) && (m_twenty_p_tube_level < 150)) // £30 capacity
		m_twenty_p_tube_level++;
}

static INPUT_PORTS_START( bfm_cobra3 )
	PORT_START("IOSTATUS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	// Current through any active meter is returned on a shared sensing line.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(bfm_cobra3_state::meter_sense_r))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Test") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE3")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE4")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cobra3_direct_coins )
	PORT_MODIFY("IOSTATUS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("10p") PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("20p") PORT_IMPULSE(3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bfm_cobra3_state::coin_inserted), 20)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("50p") PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME(u8"£1") PORT_IMPULSE(3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bfm_cobra3_state::coin_inserted), 100)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( cobra3_encoded_coins )
	PORT_MODIFY("IOSTATUS")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(bfm_cobra3_state::coin_acceptor_r))

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME(u8"£1") PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("50p") PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("20p") PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("10p") PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_NAME("5p") PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_NAME(u8"£2") PORT_IMPULSE(3)
INPUT_PORTS_END

static INPUT_PORTS_START( cobra3_payslide )
	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(bfm_cobra3_state::pound_tube_low_r))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(bfm_cobra3_state::twenty_p_tube_low_r))
	PORT_CONFNAME( 0x04, 0x04, "Payout Unit" )
	PORT_CONFSETTING(    0x00, "Not Fitted" )
	PORT_CONFSETTING(    0x04, "Fitted" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_DOOR ) PORT_NAME("Cash Door Open") PORT_CODE(KEYCODE_Y) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_DOOR ) PORT_NAME("Back and Front Doors Open") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Refill/Volume Setup Mode") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("TUBE0")
	PORT_ADJUSTER(100, u8"Initial £1 Tube Fill")

	PORT_START("TUBE1")
	PORT_ADJUSTER(100, "Initial 20p Tube Fill")
INPUT_PORTS_END

static INPUT_PORTS_START( cobra3_abc_controls )
	PORT_MODIFY("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("A (Left)") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B (Left)") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("C (Left)") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("C (Right)")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("B (Right)")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("A (Right)")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Select") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cobra3_common_dils )
	PORT_MODIFY("STROBE4")
	PORT_DIPNAME( 0x01, 0x00, "Credit on Reset" ) PORT_DIPLOCATION("DIL:!01")
	PORT_DIPSETTING(    0x00, "Retained" )
	PORT_DIPSETTING(    0x01, "Lost" )
	PORT_BIT( 0x2e, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DIL:!05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Target Percentage" ) PORT_DIPLOCATION("DIL:!07,!08")
	PORT_DIPSETTING(    0x00, "30%" )
	PORT_DIPSETTING(    0x40, "35%" )
	PORT_DIPSETTING(    0x80, "40%" )
	PORT_DIPSETTING(    0xc0, "50%" )
INPUT_PORTS_END

static INPUT_PORTS_START( cobra3_standard_dils )
	PORT_INCLUDE(cobra3_common_dils)

	PORT_MODIFY("STROBE4")
	PORT_DIPNAME( 0x02, 0x00, "Reset Alarm" ) PORT_DIPLOCATION("DIL:!02")
	PORT_DIPSETTING(    0x00, "Enabled" )
	PORT_DIPSETTING(    0x02, "Disabled" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( c3_telly )
	PORT_INCLUDE(bfm_cobra3)
	PORT_INCLUDE(cobra3_direct_coins)
	PORT_INCLUDE(cobra3_payslide)
	PORT_INCLUDE(cobra3_abc_controls)
	PORT_INCLUDE(cobra3_common_dils)

	PORT_MODIFY("STROBE0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE ) PORT_NAME("Collect")
INPUT_PORTS_END

static INPUT_PORTS_START( c3_rtime )
	PORT_INCLUDE(bfm_cobra3)
	PORT_INCLUDE(cobra3_direct_coins)
	PORT_INCLUDE(cobra3_payslide)
	PORT_INCLUDE(cobra3_standard_dils)

	PORT_MODIFY("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("A (Left)") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B (Left)") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("C (Left)") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("D (Left)") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("D (Right)") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("C (Right)") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("B (Right)") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("A (Right)") PORT_CODE(KEYCODE_Z)

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( c3_totp )
	PORT_INCLUDE(bfm_cobra3)
	PORT_INCLUDE(cobra3_encoded_coins)
	PORT_INCLUDE(cobra3_payslide)
	PORT_INCLUDE(cobra3_abc_controls)
	PORT_INCLUDE(cobra3_standard_dils)

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME(u8"£1") PORT_IMPULSE(3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bfm_cobra3_state::coin_inserted), 100)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("20p") PORT_IMPULSE(3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bfm_cobra3_state::coin_inserted), 20)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME(u8"£2") PORT_IMPULSE(3)

	PORT_MODIFY("STROBE0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( c3_ppays )
	PORT_INCLUDE(bfm_cobra3)
	PORT_INCLUDE(cobra3_encoded_coins)
	PORT_INCLUDE(cobra3_standard_dils)

	PORT_MODIFY("COINS")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME(u8"£2") PORT_IMPULSE(3)

	PORT_MODIFY("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("A (Left)") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B (Left)") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("C (Left)") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x38, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Spin")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("C (Right)")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("B (Right)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("A (Right)")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_DOOR ) PORT_NAME("Cash Door Open") PORT_CODE(KEYCODE_Y) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_DOOR ) PORT_NAME("Back and Front Doors Open") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Refill/Volume Setup Mode") PORT_CODE(KEYCODE_R) PORT_TOGGLE

	PORT_MODIFY("STROBE4")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!03" )
	PORT_DIPNAME( 0x08, 0x00, "Refill Mode Statistics" ) PORT_DIPLOCATION("DIL:!04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "DIL:!06" )
INPUT_PORTS_END

ROM_START( c3_rtime )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400009.bin", 0x00001, 0x080000, CRC(a5e0a5ca) SHA1(e7063ddfb436152f15267fde2aa7695c8a262191) )
	ROM_LOAD16_BYTE( "95400010.bin", 0x00000, 0x080000, CRC(03fd5f72) SHA1(379cfc4ef5087f24989bc1f2246b6056e33fd472) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "95400063.lhs", 0x00001, 0x080000, CRC(eecb5f3b) SHA1(a1c6ad61a65c5361c38aaae2a064983a978c45ea) )
	ROM_LOAD16_BYTE( "95400064.rhs", 0x00000, 0x080000, CRC(251689f5) SHA1(4589a409c6b0f2869f99a08df8d76223e54d5b3c) )
	ROM_LOAD16_BYTE( "95401063.lhs", 0x00001, 0x080000, CRC(ea98c159) SHA1(6f665d80b71af57b31194fdc981707822e62053e) )
	ROM_LOAD16_BYTE( "95401064.rhs", 0x00000, 0x080000, CRC(bc125897) SHA1(a83fdb54349d3ea5d183754bf4b9fee1f0b73be3) )
	ROM_LOAD16_BYTE( "radtimes.lhs", 0x00001, 0x080000, CRC(c6574297) SHA1(bd9744c4b08f9ae35fe1523ebcd68c52a36a32e0) )
	ROM_LOAD16_BYTE( "radtimes.rhs", 0x00000, 0x080000, CRC(ed2c24f0) SHA1(5f06b2de7e2b2dccee7763ea0938849d67256ff2) )
	ROM_LOAD16_BYTE( "rt017.lhs", 0x00001, 0x080000, CRC(d2272c39) SHA1(f583fe39c153dca2e86e875ca39056a8756e0d2c) )
	ROM_LOAD16_BYTE( "rt018.rhs", 0x00000, 0x080000, CRC(52999d03) SHA1(21d1e9034a26f6f73109e9e83272dcff104993e5) )
	ROM_LOAD16_BYTE( "rtimesp1", 0x00001, 0x080000, CRC(f856d377) SHA1(a9fac7e2188bbd087f70c1c00cbf790bc52d573b) )
	ROM_LOAD16_BYTE( "rtimesp2", 0x00000, 0x080000, CRC(130d0864) SHA1(034d6c4fdec3acd4329d16315aeac43b1f1a5e91) )

	ROM_REGION( 0x1000000, "ymz280b", 0 )
	ROM_LOAD( "95004056.bin", 0x000000, 0x080000, CRC(24e8f9fb) SHA1(0d484a8f368b0f2140f148a1dc84db85a100af38) )
	ROM_LOAD( "95004057.bin", 0x080000, 0x080000, CRC(f73c92d6) SHA1(08c7db2baccb703f99efb81f618719a7789ca564) )

	DISK_REGION("cdrom")
	DISK_IMAGE_READONLY( "95100302", 0, SHA1(20accfe236a0c85108cd2a205399ed8959f1a638) )
ROM_END

ROM_START( c3_telly )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400021.p1",  0x00001, 0x080000, CRC(5c969746) SHA1(7458c613d7a3e7cf6a21e55f74dcdc052404f29c) )
	ROM_LOAD16_BYTE( "95400022.p2",  0x00000, 0x080000, CRC(fa1fdb7b) SHA1(eff87c197a62dba49d95810e8669026db2edb187) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "95401021.p1",  0x00001, 0x080000, CRC(24a334d3) SHA1(672f16cbd2ddf627213de71024b6fbaa28f526a5) )
	ROM_LOAD16_BYTE( "95401022.p2",  0x00000, 0x080000, CRC(90af3767) SHA1(e529ad7eef5e6d2a6951d46e77aaad2087890445) )
	ROM_LOAD16_BYTE( "tadd13lh",     0x00001, 0x080000, CRC(2d6ed08c) SHA1(efa39b9ff5605c2e29971fb5e874c9a0c178b1f0) )
	ROM_LOAD16_BYTE( "tadd14rh",     0x00000, 0x080000, CRC(26dd6ed6) SHA1(553f29017494b6f7ecc98940d527f498316ea55e) )
	ROM_LOAD16_BYTE( "telad.tl",     0x00001, 0x080000, CRC(e6906027) SHA1(20ca64417ea3795dc26adfea717cb3d724019c34) )
	ROM_LOAD16_BYTE( "telad.tr",     0x00000, 0x080000, CRC(38dbee05) SHA1(ee33cdaa7f817beb49a3cff49a5493a50d8d4504) )
	ROM_LOAD16_BYTE( "tasndl",       0x00001, 0x080000, CRC(3f0b9d2b) SHA1(6db3451c26a3e673204c316403e0bb7127191a1f) )
	ROM_LOAD16_BYTE( "tasndr",       0x00000, 0x080000, CRC(2dd9ebcf) SHA1(4d118d37e18266f82fb2acb37f5fd106e0f25a1f) )

	ROM_REGION( 0x1000000, "ymz280b", ROMREGION_ERASE00 )
	ROM_LOAD( "telsndl", 0x0000, 0x080000, CRC(74996fbd) SHA1(90e46130dccf47be1fcfaf549e548cdd4883e59d) )

	DISK_REGION("cdrom")
	DISK_IMAGE_READONLY( "95100300", 0, SHA1(98905cbff24c576c58210d1d003f710fa7064762) )
ROM_END


ROM_START( c3_tellyns )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400023.lhs", 0x00001, 0x080000, CRC(b79279b8) SHA1(010edf0c299b0b01ab43f52dce540ff0847fb4c5) )
	ROM_LOAD16_BYTE( "95400024.rhs", 0x00000, 0x080000, CRC(835d25fd) SHA1(6d780332f6016d6e1404922e0ac439a499211be3) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "95401023.lhs", 0x00001, 0x080000, CRC(85b95b56) SHA1(106e617fc92f95a6b3769db1fd4e5ab47c752c08) )
	ROM_LOAD16_BYTE( "95401024.rhs", 0x00000, 0x080000, CRC(835d25fd) SHA1(6d780332f6016d6e1404922e0ac439a499211be3) )

	ROM_REGION( 0x1000000, "ymz280b", ROMREGION_ERASE00 )
	ROM_LOAD( "telsndl", 0x0000, 0x080000, CRC(74996fbd) SHA1(90e46130dccf47be1fcfaf549e548cdd4883e59d) )

	DISK_REGION("cdrom")
	DISK_IMAGE_READONLY( "95100301", 0, SHA1(dbce040a6fb7916a240d24e2207cf6e1b3f572e7) )
ROM_END

ROM_START( c3_totp )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400101.lo", 0x00001, 0x080000, CRC(c95164c7) SHA1(7b2fada6a3208666219a53cba08f7acad015763d) )
	ROM_LOAD16_BYTE( "95400102.hi", 0x00000, 0x080000, CRC(5ebba159) SHA1(34bcf48140261cd87d81a32581e965d722f42f71) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "95401101.lo", 0x00001, 0x080000, CRC(97d2d90a) SHA1(d4a2afd3cc551986e76f107beb66e8c660a6ee1d) )
	ROM_LOAD16_BYTE( "95401102.hi", 0x00000, 0x080000, CRC(3599427f) SHA1(16d915553b2b490a047888c64ebcf952714b3168) )

	ROM_REGION( 0x1000000, "ymz280b", ROMREGION_ERASE00 )
	ROM_LOAD( "totpsnd.lhs", 0x000000, 0x080000, CRC(56a73136) SHA1(10656ede18de9432a8a728cc59d000b5b1bf0150) )
	ROM_LOAD( "totpsnd.rhs", 0x080000, 0x080000, CRC(28d156ab) SHA1(ebf5c4e008015b9b56b3aa5228c05b8e298daa80) )

	DISK_REGION("cdrom")
	DISK_IMAGE_READONLY( "95100307", 0, SHA1(27ad1565f9a153fe71b72d9c597a6e3c3f13ded0) )
ROM_END

ROM_START( c3_ppays )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400687.hi", 0x00000, 0x080000, CRC(56080e1c) SHA1(49391059b5a758690d4972abad04d7e7aef23423) )
	ROM_LOAD16_BYTE( "95400687.lo", 0x00001, 0x080000, CRC(8b2c9c3d) SHA1(921c900447870f6ae51a4f3baeb60ce94e732291) )

	ROM_REGION( 0x1000000, "ymz280b", ROMREGION_ERASE00 )
	ROM_LOAD( "phrasesn.l", 0x0000, 0x080000, CRC(a436ccf8) SHA1(18c39aa2e68c32242e0de1347b25d4af44b84548) )

	DISK_REGION("cdrom")
	DISK_IMAGE_READONLY( "95100315", 0, SHA1(fc76d3ab5ff38c2dc4f06399f5399a1ae3c136e9) )
ROM_END

} // anonymous namespace

GAMEL( 1995, c3_telly,  0, c3_telly, c3_telly, bfm_cobra3_state, empty_init, ROT0, "BFM", "Telly Addicts (Bellfruit) (Cobra 3)", MACHINE_SUPPORTS_SAVE, layout_c3_telly )
GAMEL( 1995, c3_tellyns, 0, c3_telly, c3_telly, bfm_cobra3_state, empty_init, ROT0, "BFM", "Telly Addicts (New Series) (Bellfruit) (Cobra 3)", MACHINE_SUPPORTS_SAVE, layout_c3_telly )
GAMEL( 1996, c3_rtime,  0, bfm_cobra3, c3_rtime, bfm_cobra3_state, empty_init, ROT0, "BFM", "Radio Times (Bellfruit) (Cobra 3)", MACHINE_SUPPORTS_SAVE, layout_c3_rtime )
GAMEL( 1997, c3_totp,   0, bfm_cobra3, c3_totp,  bfm_cobra3_state, empty_init, ROT0, "BFM", "Top of the Pops (Bellfruit) (Cobra 3?)", MACHINE_SUPPORTS_SAVE, layout_c3_totp )
GAMEL( 1998, c3_ppays,  0, c3_ppays, c3_ppays, bfm_cobra3_state, empty_init, ROT0, "BFM", "The Phrase That Pays (Bellfruit) (Cobra 3?)", MACHINE_SUPPORTS_SAVE, layout_c3_ppays )
