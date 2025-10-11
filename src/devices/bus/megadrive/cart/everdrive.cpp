// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Everdrive-MD (first gen)

TODO:
- avoid phantom cart slot loading (does nothing, we load BIOS from here);
- ST_M29W640FT core is incomplete (spurious unhandled writes);
- SPI comms dislikes receiving a SS signal when full speed is selected
\- goes 0 -> 1 -> 0, throwing a "SEL ERROR 120" if we don't guard against it;
- Add remaining cfg_w flags;
- Currently bases on HW spec 1.1 firmware v3, anything below that not yet supported;
- Overlay for ROM patching (Game Genie based);
- Reserved OS loading (A+B+C at power on, +UP for earlier carts) does nothing;
- Understand and implement module options;
- JTAG interface;

**************************************************************************************************/

#include "emu.h"
#include "everdrive.h"

#include "bus/generic/slot.h"

DEFINE_DEVICE_TYPE(MEGADRIVE_HB_EVERDRIVE, megadrive_hb_everdrive_device, "everdrive_md", "Megadrive Krikzz Everdrive-MD cart")

megadrive_hb_everdrive_device::megadrive_hb_everdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEGADRIVE_HB_EVERDRIVE, tag, owner, clock)
	, device_megadrive_cart_interface( mconfig, *this )
	, m_flash(*this, "flash")
	, m_sdcard(*this, "sdcard")
	, m_game_mode(*this, "game_mode")
{
}

void megadrive_hb_everdrive_device::device_add_mconfig(machine_config &config)
{
	// exact type is guessed by photo manipulation
	ST_M29W640FT(config, m_flash);

	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->set_prefer_sdhc();
	m_sdcard->spi_miso_callback().set([this](int state) { m_in_bit = state; });
}

// 0x00000 bootloader
// 0x10000 OS
// 0x20000 reserve OS copy
// 0x30000 settings
// 0x40000-0xfffff module ROM area
// NOTE: bootloader stores boot flags (?) at $d0000
ROM_START( everdrive_md )
	ROM_REGION16_BE(0x800000, "flash", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("v35.bin", 0x00000, 0x20000, CRC(161b4d2e) SHA1(fe71de7dd1f2117409b158ccd45c68b1d6781a9c) )

//	ROM_LOAD16_WORD_SWAP("game.bin", 0x400000, 0x3e0000, CRC(1) SHA1(1) )
//	ROM_LOAD16_WORD_SWAP("mdos_v2.bin", 0x10000, 0x20000, CRC(b777ef96) SHA1(8efe2ec873fa4fcaddf08a2c156e540ba4a05b57) )
//	ROM_LOAD16_WORD_SWAP("os-v36.bin", 0x10000, 0x10000, CRC(ff915066) SHA1(977a8cca44ce9fa0765001f535f19390a7d8ff16) )
//	ROM_LOAD16_WORD_SWAP("os-v22.bin", 0x10000, 0x10000, CRC(22bcd1c7) SHA1(a23916e6bc1e8d8681704b774a3cbc745065d21e) )
ROM_END



const tiny_rom_entry *megadrive_hb_everdrive_device::device_rom_region() const
{
	return ROM_NAME( everdrive_md );
}


void megadrive_hb_everdrive_device::device_start()
{
	m_spi_clock = timer_alloc(FUNC(megadrive_hb_everdrive_device::spi_clock_cb), this);

	save_item(NAME(m_spi_clock_state));
	save_item(NAME(m_spi_full_speed));
	save_item(NAME(m_spi_clock_cycles));
	save_item(NAME(m_in_bit));
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));

	save_item(NAME(m_vblv));
}

void megadrive_hb_everdrive_device::device_reset()
{
	m_vblv = 0;
	m_rom_map_port = 0;
	m_spi_clock->adjust(attotime::never);
	m_spi_clock_cycles = 0;
	m_spi_clock_state = false;
	m_sdcard->spi_ss_w(0);

	m_game_mode.select(0);
}

void megadrive_hb_everdrive_device::cart_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).view(m_game_mode);
	m_game_mode[0](0x00'0000, 0x3f'ffff).lrw16(
		NAME([this] (offs_t offset) {
			return m_flash->read(offset | m_rom_map_port);
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			m_flash->write(offset | m_rom_map_port, data);
		})
	);
	m_game_mode[1](0x00'0000, 0x3f'ffff).lr16(
		NAME([this] (offs_t offset) {
			return m_flash->read(offset | 0x20'0000);
		})
	);

	// vblank redirection (for cheating), in OS mode (so at copy time)
//	m_vbl_catch[0](0x00'0078, 0x00'0079).lr16(NAME([] () { return 0xff; }));
//	m_vbl_catch[0](0x00'007a, 0x00'007b).lr16(NAME([this] () { return m_vblv; }));
}

// TODO: OS range /TIME inaccessible when in game_mode
void megadrive_hb_everdrive_device::time_io_map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(megadrive_hb_everdrive_device::spi_data_r), FUNC(megadrive_hb_everdrive_device::spi_data_w));
	map(0x02, 0x03).rw(FUNC(megadrive_hb_everdrive_device::state_r), FUNC(megadrive_hb_everdrive_device::cfg_w));
	map(0x04, 0x05).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_vblv); }));
	// 0x06, 0x07 SRAMB, bank select for SRAM saving
	 // VER, unverified, makes an 'M' to appear on bottom right (for software controlled MEGAKEY?)
	map(0x08, 0x09).lr16(NAME([] () { return 13; }));
	map(0x0a, 0x0b).lw16(NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
		if (ACCESSING_BITS_0_7)
			m_rom_map_port = BIT(data, 0) ? (0x40'0000 >> 1) : 0;
	}));

	// TODO: assume cloning 315-5709 for RAM_MODE_1 and SSF2_MODE
}

u16 megadrive_hb_everdrive_device::spi_data_r(offs_t offset, u16 mem_mask)
{
	const u16 mask = m_spi_16 ? 0xffff : 0xff;
	return m_in_latch & mask;
}

void megadrive_hb_everdrive_device::spi_data_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_out_latch);
	const u16 mask = m_spi_16 ? 0xffff : 0xff;
	m_out_latch &= mask;

	m_spi_clock_cycles = m_spi_16 ? 16 : 8;
	m_spi_clock_state = false;

	// Timings reported are per single byte, estimated.
	// This will score a "time 1/10 sec: 96" / "speed kb/s: 105" in krikzz's benchmark
	const int ticks = (m_spi_full_speed ? 16 : 128) >> 3;
	m_spi_clock->adjust(attotime::from_ticks(ticks, this->clock()), 0, attotime::from_ticks(ticks, this->clock()));
}


u16 megadrive_hb_everdrive_device::state_r(offs_t offset, u16 mem_mask)
{
	const u8 spi_ready = m_spi_clock_cycles == 0;
	return (m_sdcard->get_card_present() << 3)
		// SMS key pressed if '1'
		| (0 << 2)
		| (m_flash->is_ready() << 1)
		| spi_ready;
}

void megadrive_hb_everdrive_device::cfg_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_spi_full_speed = BIT(data, 1);
		// assume negated by using #SS nomenclature
		// HACK: don't change SS line if full speed is selected
		if (!m_spi_full_speed)
			m_sdcard->spi_ss_w(BIT(~data, 0));
		// used on dir listing onward
		m_spi_16 = BIT(data, 2);
		//printf("SS %d full_speed %d\n", BIT(data, 0), m_spi_full_speed);

		// guess
		m_spi_clock->adjust(attotime::never);
		m_spi_clock_cycles = 0;

		m_game_mode.select(BIT(data, 3));

		// m_sms_mode = BIT(data, 4);
		// m_hard_reset = BIT(data, 5);
		// m_ram_mode_1 = BIT(data, 6);
		// m_ram_on = BIT(data, 7);
		// m_vbl_catch.select(BIT(data, 8));
		// m_megakey_on = BIT(data, 9);
		// m_megakey_region_1 = BIT(data, 10);
		// m_ssf2_mode = BIT(data, 11);
		// m_ram_fs = BIT(data, 12);
		// following is for Mega CD BIOS loading
		// m_cart = BIT(data, 13);
	}
}

TIMER_CALLBACK_MEMBER(megadrive_hb_everdrive_device::spi_clock_cb)
{
	if (m_spi_clock_cycles > 0)
	{
		//m_sdcard->spi_ss_w(1);

		if (m_spi_clock_state)
		{
			m_in_latch <<= 1;
			m_in_latch &= ~0x01;
			m_in_latch |= m_in_bit;

			m_sdcard->spi_clock_w(1);

			m_spi_clock_cycles--;
		}
		else
		{
			m_sdcard->spi_mosi_w(BIT(m_out_latch, m_spi_16 ? 15 : 7));
			m_sdcard->spi_clock_w(0);

			m_out_latch <<= 1;
		}

		m_spi_clock_state = !m_spi_clock_state;
	}
	else
	{
		//m_sdcard->spi_ss_w(0);

		m_spi_clock_state = false;
		m_spi_clock->adjust(attotime::never);
	}
}

