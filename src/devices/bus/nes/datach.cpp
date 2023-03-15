// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Bandai Datach PCBs


 Here we emulate the Bandai Datach Joint ROM System [mapper 157].
 The base unit features: a Bandai LZ93D50 + 24C02 EEPROM PCB
 + barcode reader + subslot for PRG cart


 ***********************************************************************************************************/


#include "emu.h"
#include "datach.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)

#define EEPROM_INTERNAL 0
#define EEPROM_EXTERNAL 1


#define TEST_EEPROM 0

//--------------------------------
//
//  Datach Cartslot implementation
//
//--------------------------------

//-------------------------------------------------
//  sub-cart interface
//-------------------------------------------------

datach_cart_interface::datach_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "datachcart")
	, m_i2cmem(*this, "i2cmem")
	, m_rom(nullptr), m_bank(0)
{
}

datach_cart_interface::~datach_cart_interface()
{
}

uint8_t datach_cart_interface::read(offs_t offset)
{
	if (offset < 0x4000)
		return m_rom[(m_bank * 0x4000) + (offset & 0x3fff)];
	else
		return m_rom[(0x0f * 0x4000) + (offset & 0x3fff)];
}

//-------------------------------------------------
//  sub-cart slot device
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_DATACH_SLOT, nes_datach_slot_device, "nes_datach_slot", "NES Datach Cartridge Slot")

nes_datach_slot_device::nes_datach_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NES_DATACH_SLOT, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<datach_cart_interface>(mconfig, *this)
	, m_cart(nullptr)
{
}

nes_datach_slot_device::~nes_datach_slot_device()
{
}


void nes_datach_slot_device::device_start()
{
	m_cart = get_card_device();
}

uint8_t nes_datach_slot_device::read(offs_t offset)
{
	if (m_cart)
		return m_cart->read(offset);
	else
		return 0xff;
}

image_init_result nes_datach_slot_device::call_load()
{
	if (m_cart)
	{
		uint8_t *ROM = m_cart->get_cart_base();

		if (!ROM)
			return image_init_result::FAIL;

		// Existing Datach carts are all 256K, so we only load files of this size
		if (!loaded_through_softlist())
		{
			if (length() != 0x40000 && length() != 0x40010)
				return image_init_result::FAIL;

			int shift = length() - 0x40000;
			uint8_t temp[0x40010];
			fread(&temp, length());
			memcpy(ROM, temp + shift, 0x40000);

			// double check that iNES files are really mapper 157
			// (or 16, since some older .nes files marked Datach as mapper 16)
			if (length() == 0x40010)
			{
				uint8_t mapper = (temp[6] & 0xf0) >> 4;
				mapper |= temp[7] & 0xf0;
				if (mapper != 157 && mapper != 16)
				{
					return image_init_result::FAIL;
				}
			}
		}
		else
		{
			if (get_software_region_length("rom") != 0x40000)
				return image_init_result::FAIL;

			memcpy(ROM, get_software_region("rom"), 0x40000);
		}
	}

	return image_init_result::PASS;
}


std::string nes_datach_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	// any way to detect the game with X24C01?
	return software_get_default_slot("datach_rom");
}


//--------------------------------
//
//  Datach Minicart implementation
//
//  Two kinds of PCB exist
//  * ROM only, used by most games
//  * ROM + X24C01 EEPROM, used by
//    Battle Rush
//
//--------------------------------

ROM_START( datach_rom )
	ROM_REGION(0x40000, "datachrom", ROMREGION_ERASEFF)
ROM_END

DEFINE_DEVICE_TYPE(NES_DATACH_ROM,   nes_datach_rom_device,   "nes_datach_rom", "NES Datach ROM")
DEFINE_DEVICE_TYPE(NES_DATACH_24C01, nes_datach_24c01_device, "nes_datach_ep1", "NES Datach + 24C01 PCB")

nes_datach_rom_device::nes_datach_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, datach_cart_interface(mconfig, *this)
{
}

nes_datach_rom_device::nes_datach_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_datach_rom_device(mconfig, NES_DATACH_ROM, tag, owner, clock)
{
}

nes_datach_24c01_device::nes_datach_24c01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_datach_rom_device(mconfig, NES_DATACH_24C01, tag, owner, clock)
{
}


void nes_datach_rom_device::device_start()
{
	m_rom = (uint8_t*)memregion("datachrom")->base();
	save_item(NAME(m_bank));
}

void nes_datach_rom_device::device_reset()
{
	m_bank = 0;
}

const tiny_rom_entry *nes_datach_rom_device::device_rom_region() const
{
	return ROM_NAME( datach_rom );
}

uint8_t *nes_datach_rom_device::get_cart_base()
{
	return m_rom;
}


void nes_datach_24c01_device::device_add_mconfig(machine_config &config)
{
	I2C_24C01(config, m_i2cmem);
}


//---------------------------------
//
//  Datach Base Unit implementation
//
//---------------------------------

DEFINE_DEVICE_TYPE(NES_DATACH, nes_datach_device, "nes_datach", "NES Cart Bandai Datach PCB")


nes_datach_device::nes_datach_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_lz93d50_device(mconfig, NES_DATACH, tag, owner, clock)
	, m_datach_latch(0)
	, m_i2cmem(*this, "i2cmem")
	, m_reader(*this, "datach")
	, m_subslot(*this, "datach_slot")
	, m_i2c_dir(0), m_i2c_in_use(0)
	, serial_timer(nullptr)
{
}


void nes_datach_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(FUNC(nes_datach_device::irq_timer_tick), this);
	serial_timer = timer_alloc(FUNC(nes_datach_device::serial_tick), this);
	irq_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));
	serial_timer->adjust(attotime::zero, 0, clocks_to_attotime(1000));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_datach_latch));
}

void nes_datach_device::pcb_reset()
{
	prg16_89ab(0);
	prg16_cdef(m_prg_chunks - 1);
	chr8(0, m_chr_source);

	m_irq_enable = 0;
	m_irq_count = 0;
	m_datach_latch = 0;
	m_i2c_in_use = EEPROM_INTERNAL;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bandai LZ93D50 + Datach barcode reader emulation

 Games: Datach Games

 iNES: mappers 157

 In MESS: Supported

 TODO: Datach carts should actually be handled
 separately! Original carts were minicarts to be
 inserted in a smaller slot of the Barcode reader
 FC cart. The Barcode reader acts as a passthrough
 but it has no internal ROM (it does not work if
 you don't have any minicart inserted)

 TODO2: This class should be derived from the
 LZ93D50 + X24C02 class, since the main board
 has this EEPROM. Moreover, Datach - Battle Rush
 has a second X24C01 EEPROM that we don't emulate yet...

 -------------------------------------------------*/


uint8_t nes_datach_device::read_m(offs_t offset)
{
	LOG_MMC(("Datach read_m, offset: %04x\n", offset));
	uint8_t i2c_val = 0;
#if TEST_EEPROM
	if (m_i2c_dir)
	{
		if (m_i2c_in_use == EEPROM_INTERNAL)
			i2c_val = (m_i2cmem->read_sda() & 1) << 4;
		if (m_i2c_in_use == EEPROM_EXTERNAL && m_subslot->m_cart && m_subslot->m_cart->m_i2cmem)
			i2c_val = (m_subslot->m_cart->m_i2cmem->read_sda() & 1) << 4;
	}
#endif
	return m_datach_latch | i2c_val;
}


uint8_t nes_datach_device::read_h(offs_t offset)
{
	LOG_MMC(("Datach read_h, offset: %04x\n", offset));
	// this shall be the proper code, but it's a bit slower, so we access directly the subcart below
	//return m_subslot->read(offset);

	if (m_subslot->m_cart)
		return m_subslot->m_cart->read(offset);
	else    // this is "fake" in the sense that we fill CPU space with 0xff if no Datach cart is loaded
		return hi_access_rom(offset);
}

void nes_datach_device::write_h(offs_t offset, uint8_t data)
{
	LOG_MMC(("Datach write_h, offset: %04x, data: %02x\n", offset, data));

	switch (offset & 0x0f)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
			// these don't switch CHR bank (if you try this, both Battle Rush and SD Gundam Wars will have glitches!)
			// bit3 goes to SCL of the external EEPROM (and we use write=1 to enable reading from this EEPROM)
			// docs from naruko don't specify the bit, our choice comes from observation of writes performed by Battle Rush
#if TEST_EEPROM
			if (m_subslot->m_cart && m_subslot->m_cart->m_i2cmem)
			{
				if (BIT(data, 3))
					m_i2c_in_use = EEPROM_EXTERNAL;
				m_subslot->m_cart->m_i2cmem->write_scl(BIT(data, 3));
			}
#endif
			break;
		case 0x08:
			m_subslot->write_prg_bank(data & 0x0f);
			break;
		case 0x0d:
#if TEST_EEPROM
			// bit7, select SDA direction LZ93D50P -> EEPROM or EEPROM -> LZ93D50P
			m_i2c_dir = BIT(data, 7);

			// bit6 goes to SDA line, which is in common with the 2nd EEPROM, if present
			m_i2cmem->write_sda(BIT(data, 6));
			if (m_subslot->m_cart && m_subslot->m_cart->m_i2cmem)
				m_subslot->m_cart->m_i2cmem->write_sda(BIT(data, 6));

			// bit5 goes to SCL of the internal EEPROM (and we use write=1 to enable reading from this EEPROM)
			if (BIT(data, 5))
				m_i2c_in_use = EEPROM_INTERNAL;
			m_i2cmem->write_scl(BIT(data, 5));
#endif
			break;
		default:
			fcg_write(offset & 0x0f, data);
			break;
	}
}

//-------------------------------------------------
//  BARCODE READER + CART SLOT + X24C02
//-------------------------------------------------

static void datach_cart(device_slot_interface &device)
{
	device.option_add_internal("datach_rom", NES_DATACH_ROM);
	device.option_add_internal("datach_ep1", NES_DATACH_24C01);
}


void nes_datach_device::device_add_mconfig(machine_config &config)
{
	BARCODE_READER(config, m_reader, 0);
	NES_DATACH_SLOT(config, m_subslot, 0, datach_cart);
	I2C_24C02(config, m_i2cmem);
}


//-------------------------------------------------
//  irq_timer_tick - handle IRQ timer
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(nes_datach_device::irq_timer_tick)
{
	if (m_irq_enable)
	{
		// 16bit counter, IRQ fired when the counter goes from 1 to 0
		// after firing, the counter is *not* reloaded, but next clock
		// counter wraps around from 0 to 0xffff
		if (!m_irq_count)
			m_irq_count = 0xffff;
		else
			m_irq_count--;

		if (!m_irq_count)
		{
			set_irq_line(ASSERT_LINE);
			m_irq_enable = 0;
		}
	}
}

//-------------------------------------------------
//  serial_tick - tick in a serial bit
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(nes_datach_device::serial_tick)
{
	m_datach_latch = (m_reader->read_pixel() << 3);
}
