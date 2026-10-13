// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    Compound devices for the XaviX, XaviX2000 and SuperXaviX SoCs. Connect the
    CPU, internal RAM and on-chip peripherals, install their shared bus maps,
    and provide the board-facing memory, video, audio and input interfaces.

    Implement system-control registers, watchdog and power requests. Board
    ROM banking and external peripherals remain in the system drivers;
    on-chip operations and state belong to the respective peripheral devices.

    TODO:
    - Apply internal bus-slot priorities and the external-bus release
      request.
    - Determine sleep clock domains, readback, wake sources and retained
      state.
    - Implement the individual power-control disable effects and GPIO
      clock output.
    - Confirm physical RAM power-on contents and backup extent, and
      generation/revision status values.

***************************************************************************/

#include "emu.h"
#include "xavix_soc.h"

#include <algorithm>
#include <random>

#define LOG_IO  (1U << 2)
#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_SOC, xavix_soc_device, "xavix_soc", "XaviX SoC")
DEFINE_DEVICE_TYPE(XAVIX2000_SOC, xavix2000_soc_device, "xavix2000_soc", "XaviX2000 SoC")
DEFINE_DEVICE_TYPE(SUPERXAVIX_SOC, superxavix_soc_device, "superxavix_soc", "SuperXaviX SoC")

xavix_soc_device::xavix_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_soc_device(mconfig, XAVIX_SOC, tag, owner, clock, generation::XAVIX)
{
}

xavix_soc_device::xavix_soc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, generation model)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_intc(*this, "intc")
	, m_io(*this, "io")
	, m_adc(*this, "adc")
	, m_anport(*this, "anport")
	, m_math(*this, "math")
	, m_sound(*this, "xavix_sound")
	, m_timer(*this, "timer")
	, m_dma(*this, "dma")
	, m_video(*this, "video")
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_mainram(*this, "mainram")
	, m_lightgun_read_cb(*this, 0)
	, m_power_read_cb(*this, 0)
	, m_power_off_cb(*this)
	, m_external_bus_ctrl_cb(*this)
	, m_vblank_cb(*this)
	, m_program_map_device(nullptr)
	, m_external_map_device(nullptr)
	, m_generation(model)
{
}

xavix2000_soc_device::xavix2000_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix2000_soc_device(mconfig, XAVIX2000_SOC, tag, owner, clock, generation::XAVIX2000)
{
}

xavix2000_soc_device::xavix2000_soc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, generation model)
	: xavix_soc_device(mconfig, type, tag, owner, clock, model)
{
}

superxavix_soc_device::superxavix_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix2000_soc_device(mconfig, SUPERXAVIX_SOC, tag, owner, clock, generation::SUPERXAVIX)
	, m_pio(*this, "pio")
	, m_plotter(*this, "plotter")
{
}

void xavix_soc_device::xavix_map(address_map &map)
{
	if (!m_program_map.isnull())
		map(0x000000, 0xffffff).m(m_program_map_device, m_program_map);
}

void xavix_soc_device::xavix_extbus_map(address_map &map)
{
	if (!m_external_map.isnull())
		map(0x000000, 0xffffff).m(m_external_map_device, m_external_map);
}

void xavix_soc_device::xavix_lowbus_map(address_map &map)
{
	// Video registers
	map(0x0000, 0x7fff).m(m_video, FUNC(xavix_video_device::map));

	// Main RAM is shared by CPU accesses, sound descriptors and sprite DMA.
	map(0x0000, 0x3fff).ram().share(m_mainram);

	// Light guns / pens 1 and 2
	map(0x6ffc, 0x6fff).r(FUNC(xavix_soc_device::lightgun_r));
	map(0x7b18, 0x7b1b).r(FUNC(xavix_soc_device::lightgun2_r));

	// Sound registers
	map(0x7400, 0x75ff).m(m_sound, FUNC(xavix_sound_device::map));

	// Internal- and external-bus arbitration
	// Indexed stores perform discarded reads of these write-only selectors.
	map(0x7800, 0x780f).nopr().w(FUNC(xavix_soc_device::internal_arbiter_select_w));
	map(0x7810, 0x7810).rw(FUNC(xavix_soc_device::internal_arbiter_ctrl_r), FUNC(xavix_soc_device::internal_arbiter_ctrl_w));
	map(0x7888, 0x7888).rw(FUNC(xavix_soc_device::external_arbiter_ctrl_r), FUNC(xavix_soc_device::external_arbiter_ctrl_w));

	// External-bus control
	map(0x7900, 0x7900).rw(FUNC(xavix_soc_device::rom_access_ctrl_r), FUNC(xavix_soc_device::rom_access_ctrl_w));
	map(0x7901, 0x7901).rw(FUNC(xavix_soc_device::ram_access_ctrl_r), FUNC(xavix_soc_device::ram_access_ctrl_w));
	map(0x7902, 0x7902).rw(FUNC(xavix_soc_device::external_map_ctrl_r), FUNC(xavix_soc_device::external_map_ctrl_w));

	// General DMA
	map(0x7980, 0x7988).m(m_dma, FUNC(xavix_dma_device::map));

	// General-purpose I/O and event interrupts
	map(0x7a00, 0x7a03).m(m_io, FUNC(xavix_io_device::map));
	map(0x7a80, 0x7a81).m(m_io, FUNC(xavix_io_device::event_map));

	// Analogue input ports 0-3 (mouse/trackball in some systems)
	map(0x7b00, 0x7b11).m(m_anport, FUNC(xavix_anport_device::map));

	// Analogue-to-digital converter
	map(0x7b80, 0x7b82).m(m_adc, FUNC(xavix_adc_device::map));

	// Sleep control
	map(0x7b83, 0x7b83).w(FUNC(xavix_soc_device::sleep_ctrl_w));

	// General timer
	map(0x7c00, 0x7c04).m(m_timer, FUNC(xavix_timer_device::map));

	// Arithmetic registers
	map(0x7ff0, 0x7ff6).m(m_math, FUNC(xavix_math_device::map));

	// Power control
	map(0x7ff8, 0x7ff8).rw(FUNC(xavix_soc_device::power_status_r), FUNC(xavix_soc_device::power_ctrl_w));

	// Interrupt sources and vectors
	map(0x7ff9, 0x7fff).m(m_intc, FUNC(xavix_intc_device::map));

	// Version and watchdog service
	map(0x7ffd, 0x7ffd).rw(FUNC(xavix_soc_device::version_r), FUNC(xavix_soc_device::watchdog_reset_w));
}

void superxavix_soc_device::superxavix_lowbus_map(address_map &map)
{
	xavix_lowbus_map(map);

	// Bitmap plotter
	map(0x6f60, 0x6f67).m(m_plotter, FUNC(superxavix_plotter_device::map));

	// External memory region access control
	map(0x7889, 0x7889).rw(FUNC(superxavix_soc_device::instruction_burst_ctrl_r), FUNC(superxavix_soc_device::instruction_burst_ctrl_w));
	map(0x7900, 0x7900).rw(FUNC(superxavix_soc_device::external_legacy_access_ctrl_r<0>), FUNC(superxavix_soc_device::external_legacy_access_ctrl_w<0>));
	map(0x7901, 0x7901).rw(FUNC(superxavix_soc_device::external_legacy_access_ctrl_r<1>), FUNC(superxavix_soc_device::external_legacy_access_ctrl_w<1>));
	map(0x7902, 0x7902).lr8(NAME([] () -> u8 { return 0x01; })).nopw();
	map(0x7909, 0x7909).rw(FUNC(superxavix_soc_device::external_region_access_ctrl_r<0>), FUNC(superxavix_soc_device::external_region_access_ctrl_w<0>));
	map(0x790b, 0x790b).rw(FUNC(superxavix_soc_device::external_region_access_ctrl_r<1>), FUNC(superxavix_soc_device::external_region_access_ctrl_w<1>));
	map(0x790d, 0x790d).rw(FUNC(superxavix_soc_device::external_region_access_ctrl_r<2>), FUNC(superxavix_soc_device::external_region_access_ctrl_w<2>));

	// Output clocks
	map(0x7a04, 0x7a05).w(FUNC(superxavix_soc_device::output_clock_w));

	// Programmable I/O
	map(0x7a10, 0x7a42).m(m_pio, FUNC(superxavix_pio_device::map));

	// Reserved byte following the 16-bit arithmetic result.
	map(0x7ff7, 0x7ff7).lr8(NAME([] () -> u8 { return 0; }));
}

void xavix_soc_device::device_add_mconfig(machine_config &config)
{
	switch (m_generation)
	{
	case generation::XAVIX:
		XAVIX(config, m_maincpu, DERIVED_CLOCK(1, 2));
		XAVIX_VIDEO(config, m_video, DERIVED_CLOCK(1, 1));
		break;
	case generation::XAVIX2000:
		XAVIX2000(config, m_maincpu, DERIVED_CLOCK(1, 2));
		XAVIX2000_VIDEO(config, m_video, DERIVED_CLOCK(1, 1));
		break;
	case generation::SUPERXAVIX:
		SUPERXAVIX(config, m_maincpu, DERIVED_CLOCK(1, 1));
		SUPERXAVIX_VIDEO(config, m_video, DERIVED_CLOCK(1, 1));
		break;
	}

	m_maincpu->set_addrmap(AS_PROGRAM, *this, &xavix_soc_device::xavix_map);
	m_maincpu->set_addrmap(xavix_device::AS_EXTERNAL_BUS, *this, &xavix_soc_device::xavix_extbus_map);
	if (m_generation == generation::SUPERXAVIX)
		m_maincpu->set_addrmap(xavix_device::AS_LOW_BUS, downcast<superxavix_soc_device &>(*this), &superxavix_soc_device::superxavix_lowbus_map);
	else
		m_maincpu->set_addrmap(xavix_device::AS_LOW_BUS, *this, &xavix_soc_device::xavix_lowbus_map);

	XAVIX_INTC(config, m_intc);
	m_intc->irq_callback().set_inputline(m_maincpu, xavix_device::IRQ_LINE);
	m_maincpu->set_interrupt_vector_callback(m_intc, FUNC(xavix_intc_device::interrupt_vector_r));

	m_video->irq_callback().set(m_intc, FUNC(xavix_intc_device::irq_w<xavix_intc_device::IRQ_VIDEO>));
	m_video->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_video->sprite_dma_source_callback().set([this](offs_t offset) { return m_maincpu->space(xavix_device::AS_LOW_BUS).read_byte(offset); });

	if (m_generation == generation::SUPERXAVIX)
		SUPERXAVIX_IO(config, m_io);
	else
		XAVIX_IO(config, m_io);
	m_io->irq_callback().set(m_intc, FUNC(xavix_intc_device::irq_w<xavix_intc_device::IRQ_IO>));

	XAVIX_ADC(config, m_adc, DERIVED_CLOCK(1, 2));
	// Earlier firmware retains Start readback and acknowledges completion on Start.
	m_adc->set_legacy_control(m_generation == generation::XAVIX);
	m_adc->irq_callback().set(m_intc, FUNC(xavix_intc_device::irq_w<xavix_intc_device::IRQ_ADC>));

	XAVIX_ANPORT(config, m_anport);

	XAVIX_MATH(config, m_math);

	XAVIX_TIMER(config, m_timer, DERIVED_CLOCK(1, 2));
	m_timer->irq_callback().set(m_intc, FUNC(xavix_intc_device::irq_w<xavix_intc_device::IRQ_TIMER>));

	XAVIX_DMA(config, m_dma, DERIVED_CLOCK(1, 2));
	m_dma->destination_write_callback().set([this](offs_t address, u8 data) { m_maincpu->write_banked_data(address, data); });
	m_dma->irq_callback().set(m_intc, FUNC(xavix_intc_device::irq_w<xavix_intc_device::IRQ_DMA>));

	XAVIX_SOUND(config, m_sound, DERIVED_CLOCK(1, 2));
	m_sound->set_addrmap(AS_PROGRAM, address_map_constructor(
		[this](address_map &map) { map(0x0000, 0x3fff).ram().share(m_mainram); },
		"sound_descriptors"));
	m_sound->set_host_space(m_maincpu, xavix_device::AS_LOW_BUS);
	m_sound->irq_callback().set(m_intc, FUNC(xavix_intc_device::irq_w<xavix_intc_device::IRQ_SOUND>));
	m_sound->add_route(0, *this, 1.0, 0);
	m_sound->add_route(1, *this, 1.0, 1);
}

void superxavix_soc_device::device_add_mconfig(machine_config &config)
{
	xavix2000_soc_device::device_add_mconfig(config);

	SUPERXAVIX_PIO(config, m_pio);
	SUPERXAVIX_PLOTTER(config, m_plotter);

	downcast<superxavix_io_device &>(*m_io).set_pio(m_pio);
	m_pio->legacy_input_callback().set(m_io, FUNC(xavix_io_device::external_input_r));
	m_pio->legacy_output_callback().set(m_io, FUNC(xavix_io_device::external_output_w));

	downcast<superxavix_device &>(*m_maincpu).memory_control_callback().set(FUNC(superxavix_soc_device::cpu_memory_control_r));
}

void xavix_soc_device::configure_screen()
{
	auto const [base, tag] = m_screen.finder_target();
	m_screen_tag = base.subtag(tag);
	m_maincpu.lookup()->set_vblank_int(m_screen_tag.c_str(), *this, FUNC(xavix_soc_device::vblank));
	m_video.lookup()->set_screen(m_screen);
}

void xavix_soc_device::device_start()
{
	// Zero-filled RAM gives rad_crdn/rad_crdnp a false Continue option.
	// A uniform 0xff fill gives tak_hamr a fixed seed for its title background.
	std::ranges::generate(m_mainram, [rng = std::mt19937(std::random_device{}())] () mutable { return u8(rng()); });

	m_vblank_cb.resolve_safe();
	m_watchdog_timer = timer_alloc(FUNC(xavix_soc_device::watchdog_expired), this);
	auto const service = [this](offs_t offset, u8 &data, u8 mem_mask)
	{
		if (m_watchdog_port_mode && !machine().side_effects_disabled())
			service_watchdog();
	};
	m_watchdog_tap = m_maincpu->space(xavix_device::AS_LOW_BUS).install_readwrite_tap(0x7a00, 0x7a01, "watchdog_ports", service, service);

	save_item(NAME(m_rom_access_ctrl));
	save_item(NAME(m_ram_access_ctrl));
	save_item(NAME(m_external_map_ctrl));
	save_item(NAME(m_internal_arbiter_select));
	save_item(NAME(m_internal_arbiter_ctrl));
	save_item(NAME(m_external_arbiter_ctrl));
	save_item(NAME(m_power_ctrl));
	save_item(NAME(m_watchdog_port_mode));
	save_item(NAME(m_watchdog_count));
	save_item(NAME(m_watchdog_time));
	save_item(NAME(m_watchdog_last_expiry));
}

void superxavix_soc_device::device_start()
{
	xavix_soc_device::device_start();
	save_item(NAME(m_external_region_access_ctrl));
	save_item(NAME(m_instruction_burst_ctrl));
}

void xavix_soc_device::device_reset()
{
	m_rom_access_ctrl = 0;
	m_ram_access_ctrl = 0;
	m_external_map_ctrl = 0;
	std::ranges::fill(m_internal_arbiter_select, 0);
	m_internal_arbiter_ctrl = 0;
	m_external_arbiter_ctrl = 0;
	m_power_ctrl = 0;
	m_watchdog_port_mode = true;
	m_watchdog_count = 0;
	m_watchdog_time = machine().time();
	m_watchdog_last_expiry = attotime::never;
	schedule_watchdog();
	refresh_external_bus_ctrl();
}

void superxavix_soc_device::device_reset()
{
	xavix_soc_device::device_reset();
	std::ranges::fill(m_external_region_access_ctrl, 0);
	m_instruction_burst_ctrl = 0;
}

//-------------------------------------------------
//  Board Interfaces
//-------------------------------------------------

void xavix_soc_device::reset_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(xavix_soc_device::reset_changed), this), state);
}

TIMER_CALLBACK_MEMBER(xavix_soc_device::reset_changed)
{
	// Restore reset values on release after clocks have advanced with the CPU held.
	reset();

	m_maincpu->set_input_line(INPUT_LINE_RESET, param ? ASSERT_LINE : CLEAR_LINE);
}

u8 xavix_soc_device::read_low_bus_bypass(u32 address)
{
	address &= 0x7fff;
	if (m_watchdog_port_mode && (address == 0x7a00 || address == 0x7a01) && !machine().side_effects_disabled())
		service_watchdog();
	return address < m_mainram.length() ? m_mainram[address] : m_video->read_low_bus_bypass(address);
}

u8 xavix_soc_device::lightgun_r(offs_t offset)
{
	return m_lightgun_read_cb(offset);
}

u8 xavix_soc_device::lightgun2_r(offs_t offset)
{
	return m_lightgun_read_cb(offset + 4);
}

INTERRUPT_GEN_MEMBER(xavix_soc_device::vblank)
{
	m_video->vblank();
	m_vblank_cb();
}

u32 xavix_soc_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_video->screen_update(screen, bitmap, cliprect);
}

//-------------------------------------------------
//  Memory Access Control
//-------------------------------------------------

u8 xavix_soc_device::rom_access_ctrl_r()
{
	return m_rom_access_ctrl;
}

void xavix_soc_device::rom_access_ctrl_w(u8 data)
{
	LOG("%s: rom_access_ctrl_w %02x\n", machine().describe_context(), data);
	m_rom_access_ctrl = data;
	m_external_bus_ctrl_cb(0, data);
}

u8 xavix_soc_device::ram_access_ctrl_r()
{
	return m_ram_access_ctrl;
}

void xavix_soc_device::ram_access_ctrl_w(u8 data)
{
	LOG("%s: ram_access_ctrl_w %02x\n", machine().describe_context(), data);
	m_ram_access_ctrl = data;
	m_external_bus_ctrl_cb(1, data);
}

u8 xavix_soc_device::external_map_ctrl_r()
{
	return m_external_map_ctrl;
}

void xavix_soc_device::external_map_ctrl_w(u8 data)
{
	// Popira and Taiko de Popira cartridges use this bank to select between ROM and other peripherals for data-bus accesses.
	LOG("%s: external_map_ctrl_w %02x\n", machine().describe_context(), data);
	m_external_map_ctrl = data;
	m_external_bus_ctrl_cb(2, data);
}

void xavix_soc_device::refresh_external_bus_ctrl()
{
	m_external_bus_ctrl_cb(0, m_rom_access_ctrl);
	m_external_bus_ctrl_cb(1, m_ram_access_ctrl);
	m_external_bus_ctrl_cb(2, m_external_map_ctrl);
}

//-------------------------------------------------
//  Bus Arbitration
//-------------------------------------------------

void xavix_soc_device::internal_arbiter_select_w(offs_t offset, u8 data)
{
	// TODO: Apply slot priorities when internal bus contention is modeled.
	m_internal_arbiter_select[offset] = data & 0x03;
}

u8 xavix_soc_device::internal_arbiter_ctrl_r()
{
	return m_internal_arbiter_ctrl;
}

void xavix_soc_device::internal_arbiter_ctrl_w(u8 data)
{
	// On Super XaviX, bit 0 is stored without changing bus priority.
	LOG("%s: internal_arbiter_ctrl_w %02x\n", machine().describe_context(), data);
	m_internal_arbiter_ctrl = data & 0x01;
}

u8 xavix_soc_device::external_arbiter_ctrl_r()
{
	return m_external_arbiter_ctrl;
}

void xavix_soc_device::external_arbiter_ctrl_w(u8 data)
{
	// On Super XaviX, bit 0 is stored without changing bus priority.
	// TODO: Honor bit 1's external-bus release request when native use is identified.
	m_external_arbiter_ctrl = data & 0x03;
}

//-------------------------------------------------
//  Power and System Status
//-------------------------------------------------

void xavix_soc_device::sleep_ctrl_w(u8 data)
{
	// Bit 0 enables sleep.  The selector at 0x7b82 chooses master-clock divisors
	// 96, 768, 6144 or 49152 for codes 0-3; codes 4-7 are unspecified.
	// No dumped software has been observed accessing this register.
	// TODO: Determine the affected clock domains, readback, wake sources and state retention.
	LOGMASKED(LOG_IO, "%s: sleep_ctrl_w %02x\n", machine().describe_context(), data);
}

u8 xavix_soc_device::power_status_r()
{
	// Software debounces bit 7 and acts on its active-high to inactive transition.
	return (m_power_read_cb() & 0x80) | m_power_ctrl | (m_watchdog_port_mode ? 0x20 : 0);
}

void xavix_soc_device::power_ctrl_w(u8 data)
{
	// Bits 0-3 disable the clock, picture, sound and ADC; bit 6 requests power-off.
	// TODO: Model the individual disable effects once they can be observed.
	synchronize_watchdog();
	// Terminal count has priority over a control write on the same edge.
	m_power_ctrl = (data & 0x4f) | (m_watchdog_last_expiry == m_watchdog_time ? 0x40 : 0);
	LOG("%s: power_ctrl_w %02x\n", machine().describe_context(), data);

	if (BIT(data, 6) && !machine().side_effects_disabled())
		m_power_off_cb(1);
}

u8 xavix_soc_device::version_r()
{
	// Software uses this to select generation-dependent peripheral paths.
	// Currently returns 0x10 for XaviX/XaviX2000 and 0x20 for SuperXaviX.
	// maxheart also checks 0x21/0x22; anpanmdx accepts the 0x20-0x23 range.
	// Hi-kara checks 0x7f. The corresponding physical chip revisions are unconfirmed.
	return m_generation == generation::SUPERXAVIX ? 0x20 : 0x10;
}

//-------------------------------------------------
//  Watchdog
//-------------------------------------------------

void xavix_soc_device::watchdog_reset_w(u8 data)
{
	service_watchdog();
	m_watchdog_port_mode = false;
}

void xavix_soc_device::synchronize_watchdog()
{
	if (!clock())
		return;
	attotime const delta = machine().time() - m_watchdog_time;
	if (delta <= attotime::zero)
		return;
	u64 const elapsed = u64(delta.seconds()) * clock() + delta.attoseconds() / HZ_TO_ATTOSECONDS(clock());
	m_watchdog_time += attotime::from_ticks(elapsed, clock());
	u64 const count = m_watchdog_count + elapsed;
	m_watchdog_count = count & 0xffffff;
	if (count >= 0x1000000)
	{
		m_watchdog_last_expiry = m_watchdog_time - attotime::from_ticks(m_watchdog_count, clock());
		// Supply removal depends on the board; retain the request for readback.
		m_power_ctrl |= 0x40;
	}
}

void xavix_soc_device::schedule_watchdog()
{
	if (clock())
		m_watchdog_timer->adjust(m_watchdog_time + attotime::from_ticks(0x1000000 - m_watchdog_count, clock()) - machine().time());
	else
		m_watchdog_timer->enable(false);
}

void xavix_soc_device::service_watchdog()
{
	synchronize_watchdog();
	m_watchdog_count = 0;
	schedule_watchdog();
}

TIMER_CALLBACK_MEMBER(xavix_soc_device::watchdog_expired)
{
	synchronize_watchdog();
	schedule_watchdog();
}

//-------------------------------------------------
//  SuperXaviX Clock and Memory Control
//-------------------------------------------------

void superxavix_soc_device::output_clock_w(offs_t offset, u8 data)
{
	// 0x7a04 selects the divider; 0x7a05 bit 0 enables clock output on GPIO23.
	// domstepc only clears the rate during startup.
	// TODO: Emulate the clock output when native software enables it.
	LOGMASKED(LOG_IO, "%s: output_clock_w %u %02x\n", machine().describe_context(), unsigned(offset), data);
}

u8 superxavix_soc_device::cpu_memory_control_r(offs_t offset)
{
	return offset < m_external_region_access_ctrl.size() ? m_external_region_access_ctrl[offset] : m_instruction_burst_ctrl;
}

u8 superxavix_soc_device::instruction_burst_ctrl_r()
{
	return m_instruction_burst_ctrl;
}

void superxavix_soc_device::instruction_burst_ctrl_w(u8 data)
{
	m_instruction_burst_ctrl = data & 0x01;
}

template <unsigned Region>
u8 superxavix_soc_device::external_legacy_access_ctrl_r()
{
	return BIT(m_external_region_access_ctrl[Region], 1, 3);
}

template <unsigned Region>
void superxavix_soc_device::external_legacy_access_ctrl_w(u8 data)
{
	// The byte-access aliases clear the bus-width and page-access settings.
	m_external_region_access_ctrl[Region] = (data & 0x07) << 1;
	if constexpr (Region == 1)
		m_external_region_access_ctrl[2] = m_external_region_access_ctrl[1];
}

template <unsigned Region>
u8 superxavix_soc_device::external_region_access_ctrl_r()
{
	return m_external_region_access_ctrl[Region];
}

template <unsigned Region>
void superxavix_soc_device::external_region_access_ctrl_w(u8 data)
{
	LOG("%s: external_region_access_ctrl_w (region %u) %02x\n", machine().describe_context(), Region, data);
	m_external_region_access_ctrl[Region] = data;
}
