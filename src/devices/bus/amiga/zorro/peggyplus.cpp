// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Ingenieurbuero Helfrich Peggy Plus

    Zorro II MPEG-1 audio/video decoder with Amiga video slot connection
    Also sold as the Scala MD100

    Hardware (PCB version 1.1):
    - C-Cube CL450-P160 MPEG video decoder
    - LSI Logic L64111QC-30 MPEG audio decoder
    - STV8438 video DAC
    - 512 KiB RAM, expandable to 1 MiB
    - MACH120 and GALs for bus interface and control logic
    - 30 MHz and 24.576 MHz XTALs
    - RGB video output and stereo line output
    - Optional composite/Y/C video module

    Notes:
    - Register locations and RAM banking inferred from peggympeg.device
      7.16 (10.11.95) on the installation disk

    TODO:
    - CL450 and L64111 devices
    - Board control register (interrupt enables, FIFO status, A/V switching)
    - Video timing and synchronization to the Amiga (needs video slot support)
    - Optional RAM expansion

***************************************************************************/

#include "emu.h"
#include "peggyplus.h"

#include "machine/autoconfig.h"
#include "screen.h"
#include "speaker.h"

#define LOG_VIDEO (1U << 1)
#define LOG_AUDIO (1U << 2)
#define LOG_REG   (1U << 3)

#define VERBOSE (LOG_GENERAL | LOG_VIDEO | LOG_AUDIO | LOG_REG)
#include "logmacro.h"


namespace bus::amiga::zorro {


class peggyplus_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	peggyplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::GRAPHICS | feature::SOUND; }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void busrst_w(int state) override;
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	static constexpr uint32_t BOARD_SIZE = 0x10000;
	static constexpr uint32_t RAM_SIZE = 0x80000;

	void mmio_map(address_map &map) ATTR_COLD;

	uint16_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void ram_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t cl450_r(offs_t offset);
	void cl450_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void video_data_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t video_status_r();

	void audio_data_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t l64111_r(offs_t offset);
	void l64111_w(offs_t offset, uint8_t data);

	uint16_t status_r();
	void control_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	std::unique_ptr<uint16_t[]> m_ram;
	offs_t m_base_address = 0;
	uint16_t m_ram_bank = 0;
	uint16_t m_control = 0;
};

peggyplus_device::peggyplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_PEGGYPLUS, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_screen(*this, "screen")
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void peggyplus_device::mmio_map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(peggyplus_device::ram_r), FUNC(peggyplus_device::ram_w));
	map(0x4000, 0x40ff).rw(FUNC(peggyplus_device::cl450_r), FUNC(peggyplus_device::cl450_w));
	map(0x6000, 0x6001).w(FUNC(peggyplus_device::video_data_w));
	map(0x8000, 0x8001).w(FUNC(peggyplus_device::audio_data_w));
	map(0x8002, 0x803f).rw(FUNC(peggyplus_device::l64111_r), FUNC(peggyplus_device::l64111_w)).umask16(0x00ff);
	map(0x8040, 0x8041).rw(FUNC(peggyplus_device::status_r), FUNC(peggyplus_device::control_w));
	map(0x8044, 0x8045).w(FUNC(peggyplus_device::ram_bank_w));
	map(0xc040, 0xc041).r(FUNC(peggyplus_device::video_status_r)); // might be a mirror of 0x8040
	map(0xc046, 0xc047).unmaprw(); // unknown
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void peggyplus_device::device_add_mconfig(machine_config &config)
{
	// placeholder PAL MPEG format
	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(25);
	m_screen->set_size(352, 288);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(peggyplus_device::screen_update));

	SPEAKER(config, "speaker", 2).front();
}


//**************************************************************************
//  ZORRO / AUTOCONFIG
//**************************************************************************

void peggyplus_device::busrst_w(int state)
{
	if (state)
		return;

	m_ram_bank = 0;
	m_control = 0;
	int6_w(0);

	if (m_base_address)
		zorro_space().unmap_readwrite(m_base_address, m_base_address + BOARD_SIZE - 1);

	m_base_address = 0;
}

void peggyplus_device::cfgin_w(int state)
{
	LOG("cfgin_w (%d)\n", state);

	if (state)
		return;

	// setup autoconfig
	autoconfig_board_type(BOARD_TYPE_ZORRO2);
	autoconfig_board_size(BOARD_SIZE_64K);
	autoconfig_product(7);
	autoconfig_manufacturer(2195);
	autoconfig_serial(0x00000002);
	autoconfig_rom_vector(0x0000);
	autoconfig_link_into_memory(false);
	autoconfig_rom_vector_valid(false);
	autoconfig_multi_device(false);
	autoconfig_8meg_preferred(false);
	autoconfig_can_shutup(true); // unverified

	// install autoconfig handler
	zorro_space().install_readwrite_handler(0xe80000, 0xe8007f,
		read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
		write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
}

void peggyplus_device::autoconfig_base_address(offs_t address)
{
	LOG("Autoconfig address received: 0x%06x\n", uint32_t(address));

	// stop responding to default autoconfig
	zorro_space().unmap_readwrite(0xe80000, 0xe8007f);

	m_base_address = address;

	// a zero address means the board was shut up
	if (m_base_address)
		zorro_space().install_device(address, address + BOARD_SIZE - 1, *this, &peggyplus_device::mmio_map);

	cfgout_w(0);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void peggyplus_device::device_start()
{
	m_ram = make_unique_clear<uint16_t[]>(RAM_SIZE / 2);

	save_pointer(NAME(m_ram), RAM_SIZE / 2);
	save_item(NAME(m_base_address));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_control));
}

uint16_t peggyplus_device::ram_r(offs_t offset)
{
	return m_ram[((m_ram_bank & 0xfc00) << 2) | offset];
}

void peggyplus_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ram[((m_ram_bank & 0xfc00) << 2) | offset]);
}

void peggyplus_device::ram_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// provides A13 to A18 to access the RAM
	LOGMASKED(LOG_REG, "ram_bank_w: %04x & %04x\n", data, mem_mask);
	COMBINE_DATA(&m_ram_bank);
}

uint16_t peggyplus_device::cl450_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_VIDEO, "CL450 read: %02x\n", offset << 1);

	return 0;
}

void peggyplus_device::cl450_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_VIDEO, "CL450 write: %02x = %04x & %04x\n", offset << 1, data, mem_mask);
}

void peggyplus_device::video_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_VIDEO, "MPEG video data: %04x & %04x\n", data, mem_mask);
}

uint16_t peggyplus_device::video_status_r()
{
	// bit 12: ready for video data
	return 0x1000;
}

void peggyplus_device::audio_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_AUDIO, "MPEG audio data: %04x & %04x\n", data, mem_mask);
}

uint8_t peggyplus_device::l64111_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_AUDIO, "L64111 read: %02x\n", (offset + 1) << 1);

	return 0;
}

void peggyplus_device::l64111_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_AUDIO, "L64111 write: %02x = %02x\n", (offset + 1) << 1, data);
}

uint16_t peggyplus_device::status_r()
{
	// no interrupts, fifo not full
	return 0xc000;
}

void peggyplus_device::control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_REG, "control_w: %04x & %04x\n", data, mem_mask);
	COMBINE_DATA(&m_control);
}

uint32_t peggyplus_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);
	return 0;
}


} // namespace bus::amiga::zorro


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(AMIGA_PEGGYPLUS, device_zorro2_card_interface, bus::amiga::zorro::peggyplus_device, "amiga_peggyplus", "Helfrich Peggy Plus MPEG Decoder")
