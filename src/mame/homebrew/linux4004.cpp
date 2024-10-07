// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 Linux/4004

 http://dmitry.gr/?r=05.Projects&proj=35.%20Linux4004

 A MIPS-I emulator with paravirtualised hardware running on an Intel
 4004 CPU.  It looks enough like a DECstation 3100 with an R3000 CPU to
 satisfy Linux.

 TODO:
 * Emulate the VFD (Futaba M402SD10FJ or Noritake CU40025-UW6J)
 */
#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/mcs40/mcs40.h"
#include "machine/sc16is741.h"
#include "machine/spi_psram.h"
#include "machine/spi_sdcard.h"

#include "softlist_dev.h"

#include <algorithm>

#include "lnux4004.lh"


namespace {

class linux4004_state : public driver_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

	linux4004_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "u1")
		, m_psram(*this, "u%u", 5U)
		, m_sdcard(*this, "sd")
		, m_uart(*this, "u9")
		, m_memory(*this, "memory")
		, m_status(*this, "status")
		, m_rom_bank(*this, "rom")
		, m_led_pc(*this, "pc%u", 0U)
		, m_led_sdcard(*this, "storage")
	{
	}

	void linux4004(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	template <unsigned N> void psram_sio_w(offs_t offset, u8 data, u8 mem_mask);
	template <unsigned N> void miso_w(int state);

	u8 u3_r();
	void u4002_1_3_w(u8 data);
	void u4002_1_4_w(u8 data);
	void u4002_2_3_w(u8 data);
	template <unsigned N> void led_pc_w(offs_t offset, u8 data);

	void umips_rom(address_map &map) ATTR_COLD;
	void umips_ram(address_map &map) ATTR_COLD;
	void umips_status(address_map &map) ATTR_COLD;
	void umips_rom_ports(address_map &map) ATTR_COLD;
	void umips_ram_ports(address_map &map) ATTR_COLD;

	required_device<i4004_cpu_device> m_cpu;
	required_device_array<spi_psram_device, 2> m_psram;
	required_device<spi_sdcard_device> m_sdcard;
	required_device<sc16is741a_device> m_uart;
	required_shared_ptr<u8> m_memory;
	required_shared_ptr<u8> m_status;
	required_memory_bank m_rom_bank;

	output_finder<32> m_led_pc;
	output_finder<> m_led_sdcard;

	u8 m_psram_so[2];
	u8 m_u3_in;
};


INPUT_PORTS_START(linux4004)
	PORT_START("CONF")
	PORT_CONFNAME(0x03, 0x00, "TLB Entries")
	PORT_CONFSETTING(   0x03, "4")
	PORT_CONFSETTING(   0x02, "8")
	PORT_CONFSETTING(   0x01, "12")
	PORT_CONFSETTING(   0x00, "16")
	PORT_CONFNAME(0x04, 0x00, "U4002-2-4 Installed")
	PORT_CONFSETTING(   0x00, DEF_STR(No))
	PORT_CONFSETTING(   0x04, DEF_STR(Yes))
INPUT_PORTS_END


void linux4004_state::linux4004(machine_config &config)
{
	config.set_default_layout(layout_lnux4004);

	I4004(config, m_cpu, 5.5296_MHz_XTAL / 7);
	m_cpu->set_rom_map(&linux4004_state::umips_rom);
	m_cpu->set_ram_memory_map(&linux4004_state::umips_ram);
	m_cpu->set_ram_status_map(&linux4004_state::umips_status);
	m_cpu->set_rom_ports_map(&linux4004_state::umips_rom_ports);
	m_cpu->set_ram_ports_map(&linux4004_state::umips_ram_ports);

	SPI_PSRAM(config, m_psram[0]);
	m_psram[0]->set_size(8 << 20); // supports 4 MiB to 16 MiB
	m_psram[0]->sio_cb().set(FUNC(linux4004_state::psram_sio_w<0>));

	SPI_PSRAM(config, m_psram[1]);
	m_psram[1]->set_size(4 << 20); // supports 128 KiB to 16 MiB
	m_psram[1]->sio_cb().set(FUNC(linux4004_state::psram_sio_w<1>));

	SPI_SDCARD(config, m_sdcard, 0U);
	m_sdcard->spi_miso_callback().set(FUNC(linux4004_state::miso_w<3>));

	SC16IS741A(config, m_uart, 3.072_MHz_XTAL);
	m_uart->so_cb().set(FUNC(linux4004_state::miso_w<2>));
	m_uart->irq_cb().set_inputline(m_cpu, I4004_TEST_LINE).invert();
	m_uart->tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart->rts_cb().set("rs232", FUNC(rs232_port_device::write_rts));

	auto &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_uart, FUNC(sc16is741a_device::rx_w));
	rs232.cts_handler().set(m_uart, FUNC(sc16is741a_device::cts_w));

	SOFTWARE_LIST(config, "sdcard_list").set_original("lnux4004");
}


void linux4004_state::machine_start()
{
	m_rom_bank->configure_entries(0, 2, memregion("4004firmware")->base(), 0x1000);
	m_led_pc.resolve();
	m_led_sdcard.resolve();

	std::fill(std::begin(m_psram_so), std::end(m_psram_so), 1);
	m_u3_in = 0;

	save_item(NAME(m_psram_so));
	save_item(NAME(m_u3_in));
}

void linux4004_state::machine_reset()
{
	ioport_value const conf(ioport("CONF")->read());
	auto const tlb_empty(BIT(conf, 0, 2));
	auto const u4002_2_4(BIT(conf, 2));

	m_cpu->space(i4004_cpu_device::AS_RAM_MEMORY).unmap_readwrite(0x0000, 0x02ff);
	m_cpu->space(i4004_cpu_device::AS_RAM_STATUS).unmap_readwrite(0x00, 0xbf);
	m_cpu->space(i4004_cpu_device::AS_RAM_PORTS).unmap_readwrite(0x08, 0x0b);
	m_cpu->space(i4004_cpu_device::AS_RAM_MEMORY).install_ram(0x0000, 0x02ff - (tlb_empty * 0x40), m_memory.target());
	m_cpu->space(i4004_cpu_device::AS_RAM_STATUS).install_ram(0x00, 0xbf - (tlb_empty * 0x10), m_status.target());
	m_cpu->space(i4004_cpu_device::AS_RAM_PORTS).install_write_handler(0x08, 0x0b - tlb_empty, emu::rw_delegate(*this, FUNC(linux4004_state::led_pc_w<4>)));
	if (!u4002_2_4)
	{
		m_cpu->space(i4004_cpu_device::AS_RAM_MEMORY).unmap_readwrite(0x01c0, 0x01ff);
		m_cpu->space(i4004_cpu_device::AS_RAM_STATUS).unmap_readwrite(0x70, 0x7f);
	}

	std::fill(m_memory.begin(), m_memory.end(), 0);
	std::fill(m_status.begin(), m_status.end(), 0);

	u4002_1_3_w(0);
	u4002_1_4_w(0);
	u4002_2_3_w(0);
	for (auto &led : m_led_pc)
		led = 0;
}


template <unsigned N>
void linux4004_state::psram_sio_w(offs_t offset, u8 data, u8 mem_mask)
{
	m_psram_so[N] = BIT(data | ~mem_mask, 1);
	miso_w<0>(m_psram_so[0] & m_psram_so[1]);
}

template <unsigned N>
void linux4004_state::miso_w(int state)
{
	m_u3_in = (m_u3_in & ~(u8(1) << N)) | (u8(state ? 1 : 0) << N);
}


u8 linux4004_state::u3_r()
{
	return m_u3_in;
}

void linux4004_state::u4002_1_3_w(u8 data)
{
	m_sdcard->spi_mosi_w(BIT(~data, 0));
	m_sdcard->spi_clock_w(BIT(~data, 1));
	m_sdcard->spi_ss_w(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
	m_led_sdcard = BIT(~data, 2);
	m_rom_bank->set_entry(BIT(data, 3) ^ 0x01);
}

void linux4004_state::u4002_1_4_w(u8 data)
{
	m_uart->si_w(BIT(~data, 0)); // TODO: also connected to VFD
	m_uart->sclk_w(BIT(~data, 1)); // TODO: also connected to VFD
	// VFD_NCS_HV
	m_uart->cs_w(BIT(~data, 3));
}

void linux4004_state::u4002_2_3_w(u8 data)
{
	m_psram[0]->si_w(BIT(~data, 0));
	m_psram[1]->si_w(BIT(~data, 0));
	m_psram[0]->sclk_w(BIT(~data, 1));
	m_psram[1]->sclk_w(BIT(~data, 1));
	m_psram[0]->ce_w(BIT(~data, 2));
	m_psram[1]->ce_w(BIT(~data, 3));
}

template <unsigned N>
void linux4004_state::led_pc_w(offs_t offset, u8 data)
{
	for (unsigned i = 0; 4 > i; ++i)
		m_led_pc[i | ((N + offset) << 2)] = BIT(data, i);
}


void linux4004_state::umips_rom(address_map &map)
{
	map.unmap_value_low();
	map.global_mask(0x0fff);
	map(0x0000, 0x0fff).bankr(m_rom_bank);
}

void linux4004_state::umips_ram(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0x02ff).ram().share(m_memory); // up to twelve 4002 chips
}

void linux4004_state::umips_status(address_map &map)
{
	map.unmap_value_low();
	map(0x00, 0xbf).ram().share(m_status); // up to twelve 4002 chips
}

void linux4004_state::umips_rom_ports(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x0ff);
	map(0x000, 0x0ff).r(FUNC(linux4004_state::u3_r));
}

void linux4004_state::umips_ram_ports(address_map &map)
{
	map(0x00, 0x03).w(FUNC(linux4004_state::led_pc_w<0>));
	map(0x04, 0x04).w(FUNC(linux4004_state::u4002_1_3_w));
	map(0x05, 0x05).w(FUNC(linux4004_state::u4002_1_4_w));
	map(0x06, 0x06).w(FUNC(linux4004_state::u4002_2_3_w));
	map(0x08, 0x0b).w(FUNC(linux4004_state::led_pc_w<4>));
}


ROM_START(lnux4004)
	ROM_REGION(0x2000, "4004firmware", 0)
	ROM_LOAD("umips.u4", 0x0000, 0x2000, CRC(27dd98c1) SHA1(a9d2b1990e7ae8ce4a53950430c5186d4cf55a01)) // AT28C64B
ROM_END

} // anonymous namespace


SYST( 2024, lnux4004, 0, 0, linux4004, linux4004, linux4004_state, empty_init, "Dmitry Grinberg", "Linux/4004", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
