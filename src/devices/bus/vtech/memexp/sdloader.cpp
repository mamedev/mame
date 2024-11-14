// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    BennVenn SD Loader for VZ300

    Notes:
    - Also works for VZ200
    - The loader can't format the SD card, so you need to do that yourself
      Instructions for Linux:
      * dd if=/dev/zero of=vz300.img count=2048 bs=1M
      * mkfs.vfat vz300.img
      * mount image and copy .vz files to it
      * chdman createhd -o vz300.chd -i vz300.img -c none

    TODO:
    - SPI clock needed too fast

***************************************************************************/

#include "emu.h"
#include "sdloader.h"

#include "machine/spi_sdcard.h"

#define LOG_SPI     (1U << 1)
#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vtech_sdloader_device

class vtech_sdloader_device : public vtech_memexp_device
{
public:
	// construction/destruction
	vtech_sdloader_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void mem_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	required_device<spi_sdcard_device> m_sdcard;
	required_memory_bank m_dosbank;
	memory_view m_dosview;
	memory_bank_creator m_expbank;

	TIMER_CALLBACK_MEMBER(spi_clock);
	void spi_miso_w(int state);

	void mapper_w(uint8_t data);
	void sdcfg_w(uint8_t data);
	uint8_t sdio_r();
	void sdio_w(uint8_t data);
	void mode_w(uint8_t data);

	uint8_t exp_ram_r(offs_t offset);
	void exp_ram_w(offs_t offset, uint8_t data);

	emu_timer *m_spi_clock;
	bool m_spi_clock_state;
	bool m_spi_clock_sysclk;
	int m_spi_clock_cycles;
	int m_in_bit;
	uint8_t m_in_latch;
	uint8_t m_out_latch;

	std::unique_ptr<uint8_t[]> m_ram;
	bool m_vz300_mode;
};


//-------------------------------------------------
//  mem_map - memory space address map
//-------------------------------------------------

void vtech_sdloader_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x4000, 0x67ff).view(m_dosview);
	m_dosview[0](0x4000, 0x67ff).rom().region("software", 0).bankw(m_dosbank);
	m_dosview[1](0x4000, 0x67ff).bankrw(m_dosbank);
	map(0x9000, 0xffff).rw(FUNC(vtech_sdloader_device::exp_ram_r), FUNC(vtech_sdloader_device::exp_ram_w));
}

//-------------------------------------------------
//  io_map - io space address map
//-------------------------------------------------

void vtech_sdloader_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x37, 0x37).w(FUNC(vtech_sdloader_device::mapper_w));
	map(0x38, 0x38).w(FUNC(vtech_sdloader_device::sdcfg_w));
	map(0x39, 0x39).rw(FUNC(vtech_sdloader_device::sdio_r), FUNC(vtech_sdloader_device::sdio_w));
	map(0x3a, 0x3a).w(FUNC(vtech_sdloader_device::mode_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( sdloader )
	ROM_REGION(0x2800, "software", 0)
	ROM_DEFAULT_BIOS("18")
	ROM_SYSTEM_BIOS(0, "15", "Version 1.5")
	ROMX_LOAD("vzdos15.bin", 0x0000, 0x16c2, CRC(828f7703) SHA1(150c6e5a8f20416c0dab1fa96f68726f415a8b7e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "17", "Version 1.7")
	ROMX_LOAD("vzdos17.bin", 0x0000, 0x1783, CRC(7ef7fb1e) SHA1(6278ac675d6c08dca39a5a7f4c72988a178eff8a), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "18", "Version 1.8")
	ROMX_LOAD("vzdos18.bin", 0x0000, 0x1795, CRC(2b1cec28) SHA1(d4f8fa0c7a70984334be3e5c831017cfc53683b2), ROM_BIOS(2))
ROM_END

const tiny_rom_entry *vtech_sdloader_device::device_rom_region() const
{
	return ROM_NAME( sdloader );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vtech_sdloader_device::device_add_mconfig(machine_config &config)
{
	vtech_memexp_device::device_add_mconfig(config);

	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->set_prefer_sdhc();
	m_sdcard->spi_miso_callback().set(FUNC(vtech_sdloader_device::spi_miso_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_sdloader_device - constructor
//-------------------------------------------------

vtech_sdloader_device::vtech_sdloader_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_memexp_device(mconfig, VTECH_SDLOADER, tag, owner, clock),
	m_sdcard(*this, "sdcard"),
	m_dosbank(*this, "dosbank"),
	m_dosview(*this, "dosview"),
	m_expbank(*this, "expbank"),
	m_spi_clock_state(false),
	m_spi_clock_cycles(0),
	m_vz300_mode(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_sdloader_device::device_start()
{
	vtech_memexp_device::device_start();

	// init ram
	m_ram = std::make_unique<uint8_t[]>(0x20000);

	// configure banks
	m_dosbank->configure_entry(0, m_ram.get() + 0x00000);
	m_dosbank->configure_entry(1, m_ram.get() + 0x08000);
	m_expbank->configure_entry(0, m_ram.get() + 0x10000);
	m_expbank->configure_entry(1, m_ram.get() + 0x18000);

	// allocate timer for sdcard
	m_spi_clock = timer_alloc(FUNC(vtech_sdloader_device::spi_clock), this);

	// register for savestates
	save_item(NAME(m_spi_clock_state));
	save_item(NAME(m_spi_clock_sysclk));
	save_item(NAME(m_spi_clock_cycles));
	save_item(NAME(m_in_bit));
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));
	save_item(NAME(m_vz300_mode));
	save_pointer(NAME(m_ram), 0x20000);
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void vtech_sdloader_device::device_reset()
{
	// startup in vz200 mode
	m_vz300_mode = false;

	// rom enabled
	m_dosview.select(0);

	m_spi_clock->adjust(attotime::never);
	m_spi_clock_cycles = 0;
	m_spi_clock_state = false;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_CALLBACK_MEMBER(vtech_sdloader_device::spi_clock)
{
	if (m_spi_clock_cycles > 0)
	{
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
			if (m_spi_clock_cycles == 8)
				LOGMASKED(LOG_SPI, "SPI transfer start\n");

			m_sdcard->spi_mosi_w(BIT(m_out_latch, 7));
			m_sdcard->spi_clock_w(0);

			m_out_latch <<= 1;
		}

		m_spi_clock_state = !m_spi_clock_state;
	}
	else
	{
		LOGMASKED(LOG_SPI, "SPI transfer done\n");
		m_spi_clock_state = false;
		m_spi_clock->adjust(attotime::never);
	}
}

void vtech_sdloader_device::spi_miso_w(int state)
{
	LOGMASKED(LOG_SPI, "spi_miso_w: %d\n", state);

	m_in_bit = state;
}

void vtech_sdloader_device::mapper_w(uint8_t data)
{
	// 7654----  not used
	// ----3---  led
	// -----2--  expansion ram bank
	// ------1-  dos ram bank
	// -------0  dos rom/ram switch

	m_dosview.select(BIT(data, 0));
	m_dosbank->set_entry(BIT(data, 1));
	m_expbank->set_entry(BIT(data, 2));
}

void vtech_sdloader_device::sdcfg_w(uint8_t data)
{
	// 765432--  not used
	// ------1-  sd card cs
	// -------0  clock speed

	LOGMASKED(LOG_SPI, "sdcfg_w: %02x\n", data);

	m_sdcard->spi_ss_w(BIT(data, 1));
	m_spi_clock_sysclk = bool(BIT(data, 0));

	m_spi_clock_state = false;
	m_spi_clock->adjust(attotime::never);
}

uint8_t vtech_sdloader_device::sdio_r()
{
	LOGMASKED(LOG_SPI, "sdio_r: %02x\n", m_in_latch);

	return m_in_latch;
}

void vtech_sdloader_device::sdio_w(uint8_t data)
{
	LOGMASKED(LOG_SPI, "sdio_w: %02x\n", data);

	m_out_latch = data;
	m_spi_clock_cycles = 8;

	// TODO: too fast, also slightly different on vz200
	// code only waits for a single NOP before sending the next byte
	if (m_spi_clock_sysclk)
		m_spi_clock->adjust(attotime::from_hz(17.734470_MHz_XTAL / 1.5), 0, attotime::from_hz(17.734470_MHz_XTAL / 1.5));
	else
		m_spi_clock->adjust(attotime::from_hz(100_kHz_XTAL), 0, attotime::from_hz(100_kHz_XTAL));
}

void vtech_sdloader_device::mode_w(uint8_t data)
{
	LOG("Switching to %s mode\n", BIT(data, 0) ? "VZ-300" : "VZ-200");
	m_vz300_mode = bool(BIT(data, 0));
}

uint8_t vtech_sdloader_device::exp_ram_r(offs_t offset)
{
	offset += 0x9000;

	if (!m_vz300_mode || (m_vz300_mode && offset >= 0xb800))
		return reinterpret_cast<uint8_t *>(m_expbank->base())[offset & 0x7fff];

	return 0xff;
}

void vtech_sdloader_device::exp_ram_w(offs_t offset, uint8_t data)
{
	offset += 0x9000;

	if (!m_vz300_mode || (m_vz300_mode && offset >= 0xb800))
		reinterpret_cast<uint8_t *>(m_expbank->base())[offset & 0x7fff] = data;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(VTECH_SDLOADER, vtech_memexp_device, vtech_sdloader_device, "vtech_sdloader", "BennVenn SD Loader")
