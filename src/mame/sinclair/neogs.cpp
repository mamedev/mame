// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/*******************************************************************************************

    Sound card NeoGS appropriate for playing trackers (MOD) and compressed (MP3) music on
Spectrum-compatible computers with ZXBUS slot.

Hardware:
- Compatible with General Sound (GS);
- Flexible scheme, based on fpga (Altera EP1K30);
- 4Mb RAM, 512Kb flash-ROM;
- Extended techniques of memory addressing: 2 memory windows in visible area, exclude ROM
  from visible area;
- Z80 frequency modes (10, 12, 20 and 24 Mhz);
- Support up to 8 sound channels (for MOD);
- SPI mp3-decoder (like VS1011);
- SD-card SPI interface;
- DMA to NeoGS memory (transfer data from ZX to NeoGS by instructions LDI, LDIR and similar);
- Parallel working (NeoGS play music independently from ZX).

Refs:
    http://nedopc.com/gs/ngs_eng.php
    https://github.com/psbhlw/gs-firmware
    https://8bit.yarek.pl/interface/zx.generalsound/index.html

TODO:
- ZXBUS
- SPI
- DMA
- MP3

*******************************************************************************************/

#include "emu.h"
#include "neogs.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(NEOGS, neogs_device, "neogs", "NeoGS / General Sound")

ROM_START( neogs )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_DEFAULT_BIOS("v1.08")

	ROM_SYSTEM_BIOS(0, "v1.08", "v1.08")
	ROMX_LOAD( "neogs108.rom", 0x0000, 0x80000, CRC(8e261323) SHA1(b500b4aa8dea7506d26ad9e50593d9d3bee11ae1), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "test_muchkin", "Test ROM (Muchkin)")
	ROMX_LOAD( "testrom_muchkin.rom", 0x0000, 0x80000, CRC(8ab10a88) SHA1(cff3ca96489e517568146a00412107259360d01e), ROM_BIOS(1))
ROM_END


void neogs_device::update_config()
{
	if (m_gscfg0 & 4) // EXPAG
		;

	if (m_gscfg0 & 1) // NOROM
	{
		m_bank_ram->set_entry(m_mpag % (m_ram->size() / 0x8000));
		m_view.select(BIT(m_gscfg0, 1)); // RAMRO
	}
	else
	{
		m_bank_rom->set_entry(m_mpag % (memregion("maincpu")->bytes() / 0x8000));
		m_view.disable();
	}

	const double cpu_scale = (BIT(m_gscfg0, 5) ? 1 : 1.2) * (1 << BIT(~m_gscfg0, 4));
	m_maincpu->set_clock_scale(cpu_scale);
}

void neogs_device::ctrl_w(u8 data)
{
	if (data & 0x80)
		reset();
	if (data & 0x40)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	if (data & 0x20)
		; // LED data:0 - 1-on, 0-off
}

template <u8 Bank> u8 neogs_device::ram_bank_r(offs_t offset)
{
	return m_ram->read((0xc000 * Bank) + offset);
}

template <u8 Bank> void neogs_device::ram_bank_w(offs_t offset, u8 data)
{
	m_ram->write((0xc000 * Bank) + offset, data);
}

INTERRUPT_GEN_MEMBER(neogs_device::irq0_line_assert)
{
	m_maincpu->pulse_input_line(INPUT_LINE_IRQ0, attotime::from_ticks(32,  m_maincpu->clock()));
}

void neogs_device::map_memory(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0x0000);
	map(0x4000, 0x7fff).rw(FUNC(neogs_device::ram_bank_r<1>), FUNC(neogs_device::ram_bank_w<1>));
	map(0x8000, 0xffff).bankr(m_bank_rom);

	map(0x0000, 0xffff).view(m_view);
	m_view[0](0x0000, 0x3fff).rw(FUNC(neogs_device::ram_bank_r<0>), FUNC(neogs_device::ram_bank_w<0>));
	m_view[0](0x8000, 0xffff).bankrw(m_bank_ram);
	m_view[1](0x0000, 0x3fff).r(FUNC(neogs_device::ram_bank_r<0>));
	m_view[1](0x8000, 0xffff).bankrw(m_bank_ram);
}

void neogs_device::map_io(address_map &map)
{
	map(0x0000, 0x0000).mirror(0xff00).lw8(
		NAME([this](offs_t offset, u8 data) { m_mpag = data; update_config(); }));
	map(0x0001, 0x0001).mirror(0xff00).lr8(
		NAME([this](offs_t offset)          { return m_command_in; }));
	map(0x0002, 0x0002).mirror(0xff00).lr8(
		NAME([this](offs_t offset)          { m_status &= ~0x80; return m_data_in; }));
	map(0x0003, 0x0003).mirror(0xff00).lw8(
		NAME([this](offs_t offset, u8 data) { m_status |= 0x80; m_data_out = data; }));
	map(0x0004, 0x0004).mirror(0xff00).lr8(
		NAME([this](offs_t offset)          { return m_status; }));
	map(0x0005, 0x0005).mirror(0xff00).lrw8(
		NAME([this](offs_t offset)          { m_status &= ~0x01; return ~0; }),
		NAME([this](offs_t offset, u8 data) { m_status &= ~0x01; }));
	map(0x0006, 0x0009).mirror(0xff00).lw8(
		NAME([this](offs_t offset, u8 data) { m_vol[offset] = data & 0x3f; }));
	map(0x0016, 0x0019).mirror(0xff00).lw8(
		NAME([this](offs_t offset, u8 data) { m_vol[4 + offset] = data & 0x3f; }));
	map(0x0010, 0x0010).mirror(0xff00).lw8(
		NAME([this](offs_t offset, u8 data) { m_mpagx = data; update_config(); }));
	//map(0x000a, 0x000a).mirror(0xff00).noprw();
	//map(0x000b, 0x000b).mirror(0xff00).noprw();

	map(0x000f, 0x000f).mirror(0xff00).lrw8(
		NAME([this](offs_t offset)          { return m_gscfg0; }),
		NAME([this](offs_t offset, u8 data) { m_gscfg0 = data; update_config(); }));

	map(0x0011, 0x0011).mirror(0xff00).nopw();
	map(0x0013, 0x0013).mirror(0xff00).noprw();
	map(0x0014, 0x0014).mirror(0xff00).nopr();
}

void neogs_device::device_add_mconfig(machine_config &config)
{
	RAM(config, m_ram).set_default_size("4M").set_default_value(0xff);

	Z80(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_memory_map(&neogs_device::map_memory);
	m_maincpu->set_io_map(&neogs_device::map_io);
	m_maincpu->set_periodic_int(FUNC(neogs_device::irq0_line_assert), attotime::from_hz(37.5_kHz_XTAL));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	for (auto i = 0; i < 8; i++)
		DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[i], 0).add_route(ALL_OUTPUTS, (i & 2) ? "rspeaker" : "lspeaker", 0.5); // TDA1543
}

const tiny_rom_entry *neogs_device::device_rom_region() const
{
	return ROM_NAME( neogs );
}

void neogs_device::device_start()
{
	if (!m_ram->started())
		throw device_missing_dependencies();

	memory_region *rom = memregion("maincpu");
	m_bank_rom->configure_entries(0, rom->bytes() / 0x8000,  rom->base(), 0x8000);
	m_bank_ram->configure_entries(0, m_ram->size() / 0x8000, m_ram->pointer(), 0x8000);

	m_maincpu->space(AS_PROGRAM).install_read_tap(0x6000, 0x7fff, "dac_w", [this](offs_t offset, u8 &data, u8 mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			const u8 chanel = BIT(offset, 8, BIT(m_gscfg0, 2) ? 3 : 2); // 8CHANS
			const u8 sample = data ^ (m_gscfg0 & 0x80); // INV7B
			const s16 out = (sample - 0x80) * (m_vol[chanel] << 2);
			m_dac[chanel]->data_w(out);
			if (BIT(m_gscfg0, 2) && BIT(m_gscfg0, 6)) // PAN4CH
				;
		}
	});
}

void neogs_device::device_reset()
{
	m_status = 0;
	m_gscfg0 = 0;
	m_mpag = 0;
	update_config();
}
