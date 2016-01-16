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
#include "cpu/m6502/m6502.h"

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
					: device_slot_card_interface(mconfig, device),
						m_i2cmem(*this, "i2cmem"),
						m_rom(nullptr), m_bank(0)
				{
}

datach_cart_interface::~datach_cart_interface()
{
}

READ8_MEMBER(datach_cart_interface::read)
{
	if (offset < 0x4000)
		return m_rom[(m_bank * 0x4000) + (offset & 0x3fff)];
	else
		return m_rom[(0x0f * 0x4000) + (offset & 0x3fff)];
}

//-------------------------------------------------
//  sub-cart slot device
//-------------------------------------------------

const device_type NES_DATACH_SLOT = &device_creator<nes_datach_slot_device>;

nes_datach_slot_device::nes_datach_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, NES_DATACH_SLOT, "NES Datach Cartridge Slot", tag, owner, clock, "nes_datach_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this), m_cart(nullptr)
{
}

nes_datach_slot_device::~nes_datach_slot_device()
{
}


void nes_datach_slot_device::device_start()
{
	m_cart = dynamic_cast<datach_cart_interface *>(get_card_device());
}

READ8_MEMBER(nes_datach_slot_device::read)
{
	if (m_cart)
		return m_cart->read(space, offset, mem_mask);

	return 0xff;
}

bool nes_datach_slot_device::call_load()
{
	if (m_cart)
	{
		UINT8 *ROM = m_cart->get_cart_base();

		if (!ROM)
			return IMAGE_INIT_FAIL;

		// Existing Datach carts are all 256K, so we only load files of this size
		if (software_entry() == nullptr)
		{
			if (length() != 0x40000 && length() != 0x40010)
				return IMAGE_INIT_FAIL;

			int shift = length() - 0x40000;
			UINT8 temp[0x40010];
			fread(&temp, length());
			memcpy(ROM, temp + shift, 0x40000);

			// double check that iNES files are really mapper 157
			// (or 16, since some older .nes files marked Datach as mapper 16)
			if (length() == 0x40010)
			{
				UINT8 mapper = (temp[6] & 0xf0) >> 4;
				mapper |= temp[7] & 0xf0;
				if (mapper != 157 && mapper != 16)
				{
					return IMAGE_INIT_FAIL;
				}
			}
		}
		else
		{
			if (get_software_region_length("rom") != 0x40000)
				return IMAGE_INIT_FAIL;

			memcpy(ROM, get_software_region("rom"), 0x40000);
		}
	}

	return IMAGE_INIT_PASS;
}


bool nes_datach_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	machine().rom_load().load_software_part_region(*this, swlist, swname, start_entry );
	return TRUE;
}

std::string nes_datach_slot_device::get_default_card_software()
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

const device_type NES_DATACH_ROM = &device_creator<nes_datach_rom_device>;
const device_type NES_DATACH_24C01 = &device_creator<nes_datach_24c01_device>;

nes_datach_rom_device::nes_datach_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						datach_cart_interface( mconfig, *this )
{
}

nes_datach_rom_device::nes_datach_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, NES_DATACH_ROM, "NES Datach ROM", tag, owner, clock, "nes_datach_rom", __FILE__),
						datach_cart_interface( mconfig, *this )
{
}

nes_datach_24c01_device::nes_datach_24c01_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_datach_rom_device(mconfig, NES_DATACH_24C01, "NES Datach + 24C01 PCB", tag, owner, clock, "nes_datach_ep1", __FILE__)
{
}


void nes_datach_rom_device::device_start()
{
	m_rom = (UINT8*)memregion("datachrom")->base();
	save_item(NAME(m_bank));
}

void nes_datach_rom_device::device_reset()
{
	m_bank = 0;
}

const rom_entry *nes_datach_rom_device::device_rom_region() const
{
	return ROM_NAME( datach_rom );
}

UINT8 *nes_datach_rom_device::get_cart_base()
{
	return m_rom;
}


MACHINE_CONFIG_FRAGMENT( subcart_i2c_24c01 )
	MCFG_24C01_ADD("i2cmem")
MACHINE_CONFIG_END

machine_config_constructor nes_datach_24c01_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( subcart_i2c_24c01 );
}


//---------------------------------
//
//  Datach Base Unit implementation
//
//---------------------------------

const device_type NES_DATACH = &device_creator<nes_datach_device>;


nes_datach_device::nes_datach_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_lz93d50_device(mconfig, NES_DATACH, "NES Cart Bandai Datach PCB", tag, owner, clock, "nes_datach", __FILE__), m_datach_latch(0),
						m_i2cmem(*this, "i2cmem"),
						m_reader(*this, "datach"),
						m_subslot(*this, "datach_slot"), m_i2c_dir(0), m_i2c_in_use(0), serial_timer(nullptr)
				{
}


void nes_datach_device::device_start()
{
	common_start();
	irq_timer = timer_alloc(TIMER_IRQ);
	serial_timer = timer_alloc(TIMER_SERIAL);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));
	serial_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1000));

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_datach_latch));
}

void nes_datach_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
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


READ8_MEMBER(nes_datach_device::read_m)
{
	LOG_MMC(("Datach read_m, offset: %04x\n", offset));
	UINT8 i2c_val = 0;
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


READ8_MEMBER(nes_datach_device::read_h)
{
	LOG_MMC(("Datach read_h, offset: %04x\n", offset));
	// this shall be the proper code, but it's a bit slower, so we access directly the subcart below
	//return m_subslot->read(space, offset, mem_mask);

	if (m_subslot->m_cart)
		return m_subslot->m_cart->read(space, offset, mem_mask);
	else    // this is "fake" in the sense that we fill CPU space with 0xff if no Datach cart is loaded
		return hi_access_rom(offset);
}

WRITE8_MEMBER(nes_datach_device::write_h)
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
			fcg_write(space, offset & 0x0f, data, mem_mask);
			break;
	}
}

//-------------------------------------------------
//  BARCODE READER + CART SLOT + X24C02
//-------------------------------------------------

static SLOT_INTERFACE_START(datach_cart)
	SLOT_INTERFACE_INTERNAL("datach_rom", NES_DATACH_ROM)
	SLOT_INTERFACE_INTERNAL("datach_ep1", NES_DATACH_24C01)
SLOT_INTERFACE_END


MACHINE_CONFIG_FRAGMENT( bandai_datach )
	MCFG_BARCODE_READER_ADD("datach")
	MCFG_DATACH_MINICART_ADD("datach_slot", datach_cart)
	MCFG_24C02_ADD("i2cmem")
MACHINE_CONFIG_END

machine_config_constructor nes_datach_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( bandai_datach );
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void nes_datach_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
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
				m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
				m_irq_enable = 0;
			}
		}
	}
	if (id == TIMER_SERIAL)
	{
		m_datach_latch = (m_reader->read_pixel() << 3);
	}
}
