// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    MacroSystem Toccata

    16bit/48KHz Audio Digitizer

    Hardware:
    - AD1848KP SoundPort
    - 2x 7202LA 1024x9 FIFO
    - XTAL 24.576 MHz (also seen with 24.582) and 16.9344 MHz
    - MC33078
    - GALs for autoconfig and other logic
    - 3 stereo inputs, 1 stereo output

    Notes:
    - Needs more testing

    TODO:
    - Verify data lanes
    - Audio input

***************************************************************************/

#include "emu.h"
#include "toccata.h"

#define LOG_IRQ    (1U << 1)
#define LOG_FIFO   (1U << 2)
#define LOG_REG    (1U << 3)
#define LOG_AD1848 (1U << 4)

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_TOCCATA, bus::amiga::zorro::toccata_device, "amiga_toccata", "Toccata SoundCard")

namespace bus::amiga::zorro {

toccata_device::toccata_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_TOCCATA, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_ad1848(*this, "ad1848"),
	m_fifo(*this, "fifo%u", 0U)
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void toccata_device::mmio_map(address_map &map)
{
	map(0x0000, 0x0001).mirror(0x97fe).rw(FUNC(toccata_device::status_r), FUNC(toccata_device::control_w)).umask16(0xff00);
	map(0x2000, 0x2001).mirror(0x97fe).rw(FUNC(toccata_device::fifo_r), FUNC(toccata_device::fifo_w));
	map(0x6000, 0x6001).mirror(0x97fe).rw(FUNC(toccata_device::ad1848_idx_r), FUNC(toccata_device::ad1848_idx_w)).umask16(0x00ff);
	map(0x6800, 0x6801).mirror(0x97fe).rw(FUNC(toccata_device::ad1848_data_r), FUNC(toccata_device::ad1848_data_w)).umask16(0x00ff);
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void toccata_device::device_add_mconfig(machine_config &config)
{
	AD1848(config, m_ad1848, 0);
	m_ad1848->drq().set(FUNC(toccata_device::drq_w));

	IDT7202(config, m_fifo[0]);
	m_fifo[0]->hf_handler().set(FUNC(toccata_device::playback_hf_w));

	IDT7202(config, m_fifo[1]);
	m_fifo[1]->hf_handler().set(FUNC(toccata_device::record_hf_w));
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void toccata_device::device_start()
{
	// register for save states
	save_item(NAME(m_status));
	save_item(NAME(m_control));
}

void toccata_device::update_interrupts()
{
	if ((BIT(m_control, 7) && BIT(m_status, 3)) || (BIT(m_control, 6) && BIT(m_status, 2)))
	{
		LOGMASKED(LOG_IRQ, "generating interrupt, control = %02x, status = %02x\n", m_control, m_status);

		m_status &= ~(1 << 7);
		m_zorro->int6_w(1);
	}
}

void toccata_device::playback_hf_w(int state)
{
	LOGMASKED(LOG_IRQ, "playback_hf_w: %d, status %02x\n", state, m_status);

	if (BIT(m_control, 7) && BIT(m_control, 4))
	{
		m_status &= ~(1 << 3);
		m_status |= state << 3;

		LOGMASKED(LOG_IRQ, "playback interrupts enabled, status now %02x\n", m_status);

		update_interrupts();
	}
}

void toccata_device::record_hf_w(int state)
{
	LOGMASKED(LOG_IRQ, "record_hf_w: %d, status %02x\n", state, m_status);

	if (BIT(m_control, 7) && BIT(m_control, 3))
	{
		m_status &= ~(1 << 2);
		m_status |= state << 2;

		LOGMASKED(LOG_IRQ, "record interrupts enabled, status now %02x\n", m_status);

		update_interrupts();
	}
}

void toccata_device::drq_w(int state)
{
	if (BIT(m_control, 2) && BIT(m_control, 4) && state)
	{
		uint8_t tmp = 0;

		// 16-bit stereo, bytes swapped
		tmp = m_fifo[0]->data_byte_r();
		m_ad1848->dack_w(m_fifo[0]->data_byte_r());
		m_ad1848->dack_w(tmp);

		tmp = m_fifo[0]->data_byte_r();
		m_ad1848->dack_w(m_fifo[0]->data_byte_r());
		m_ad1848->dack_w(tmp);
	}
}

uint8_t toccata_device::status_r(offs_t offset)
{
	// 7-------  interrupt pending (active low)
	// -6------  unknown
	// --5-----  unknown
	// ---4----  unknown
	// ----3---  playback fifo half empty
	// -----2--  record fifo half full
	// ------1-  unknown
	// -------0  unknown

	uint8_t data = m_status;

	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_REG, "status_r: %02x\n", m_status);

		// reading the status clears the interrupt
		m_status = 0x80;
		m_zorro->int6_w(0);
	}

	return data;
}

void toccata_device::control_w(offs_t offset, uint8_t data)
{
	// 7-------  playback interrupt enable
	// -6------  record interrupt enable
	// --5-----  unknown
	// ---4----  fifo playback
	// ----3---  fifo record
	// -----2--  enable fifo
	// ------1-  card reset
	// -------0  card active

	LOGMASKED(LOG_REG, "control_w: %02x\n", data);

	if (BIT(data, 1))
	{
		// reset card
		LOG("card reset\n");

		m_status = 0x80;
		m_control = 0x00;

		m_fifo[0]->reset();
		m_fifo[1]->reset();
		m_ad1848->reset();
	}
	else
	{
		// not sure
		if (data == 0x01)
		{
			LOG("fifo reset\n");

			m_status = 0x80;

			m_fifo[0]->reset();
			m_fifo[1]->reset();
		}

		m_control = data;

		update_interrupts();
	}
}

uint16_t toccata_device::fifo_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	if (ACCESSING_BITS_0_7)
		data |= m_fifo[1]->data_byte_r() << 0;

	if (ACCESSING_BITS_8_15)
		data |= m_fifo[1]->data_byte_r() << 8;

	LOGMASKED(LOG_FIFO, "fifo_r: %04x & %04x\n", data, mem_mask);

	return data;
}

void toccata_device::fifo_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_FIFO, "fifo_w: %04x & %04x\n", data, mem_mask);

	if (ACCESSING_BITS_0_7)
		m_fifo[0]->data_byte_w(data >> 0);

	if (ACCESSING_BITS_8_15)
		m_fifo[0]->data_byte_w(data >> 8);
}

uint8_t toccata_device::ad1848_idx_r(offs_t offset)
{
	return m_ad1848->read(0);
}

void toccata_device::ad1848_idx_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_AD1848, "ad1848_idx_w: %02x\n", data);
	m_ad1848->write(0, data);
}

uint8_t toccata_device::ad1848_data_r(offs_t offset)
{
	return m_ad1848->read(1);
}

void toccata_device::ad1848_data_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_AD1848, "ad1848_data_w: %02x\n", data);
	m_ad1848->write(1, data);
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void toccata_device::autoconfig_base_address(offs_t address)
{
	LOG("autoconfig_base_address received: 0x%06x\n", address);
	LOG("-> installing toccata\n");

	// stop responding to default autoconfig
	m_zorro->space().unmap_readwrite(0xe80000, 0xe8007f);

	// toccata registers
	m_zorro->space().install_device(address, address + 0x0ffff, *this, &toccata_device::mmio_map);

	// we're done
	m_zorro->cfgout_w(0);
}

void toccata_device::cfgin_w(int state)
{
	LOG("cfgin_w (%d)\n", state);

	if (state == 0)
	{
		// setup autoconfig
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_board_size(BOARD_SIZE_64K);
		autoconfig_link_into_memory(false);
		autoconfig_rom_vector_valid(false);
		autoconfig_multi_device(false);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true);
		autoconfig_product(12);
		autoconfig_manufacturer(18260);
		autoconfig_serial(0x00000000);
		autoconfig_rom_vector(0x0000);

		// install autoconfig handler
		m_zorro->space().install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
			write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
	}
}

} // namespace bus::amiga::zorro
