// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    d004.c

    KC85 D004 Floppy Disk Interface

***************************************************************************/

#include "emu.h"
#include "d004.h"
#include "formats/mfi_dsk.h"
#include "formats/kc85_dsk.h"

#define Z80_TAG         "disk"
#define Z80CTC_TAG      "z80ctc"
#define UPD765_TAG      "upd765"
#define ATA_TAG         "ata"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

void kc_d004_device::kc_d004_mem(address_map &map)
{
	map(0x0000, 0xfbff).ram();
	map(0xfc00, 0xffff).ram().share("koppelram");
}

void kc_d004_device::kc_d004_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf0, 0xf1).m(UPD765_TAG, FUNC(upd765a_device::map));
	map(0xf2, 0xf3).rw(UPD765_TAG, FUNC(upd765a_device::dma_r), FUNC(upd765a_device::dma_w));
	map(0xf4, 0xf4).r(FUNC(kc_d004_device::hw_input_gate_r));
	map(0xf6, 0xf7).w(FUNC(kc_d004_device::fdd_select_w));
	map(0xf8, 0xf9).w(FUNC(kc_d004_device::hw_terminal_count_w));
	map(0xfc, 0xff).rw(Z80CTC_TAG, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

void kc_d004_gide_device::kc_d004_gide_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(kc_d004_gide_device::gide_r), FUNC(kc_d004_gide_device::gide_w));
	map(0x00f0, 0x00f1).mirror(0xff00).m(UPD765_TAG, FUNC(upd765a_device::map));
	map(0x00f2, 0x00f3).mirror(0xff00).rw(UPD765_TAG, FUNC(upd765a_device::dma_r), FUNC(upd765a_device::dma_w));
	map(0x00f4, 0x00f4).mirror(0xff00).r(FUNC(kc_d004_gide_device::hw_input_gate_r));
	map(0x00f6, 0x00f7).mirror(0xff00).w(FUNC(kc_d004_gide_device::fdd_select_w));
	map(0x00f8, 0x00f9).mirror(0xff00).w(FUNC(kc_d004_gide_device::hw_terminal_count_w));
	map(0x00fc, 0x00ff).mirror(0xff00).rw(Z80CTC_TAG, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

void kc_d004_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_KC85_FORMAT);
}

static void kc_d004_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

static const z80_daisy_config kc_d004_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ nullptr }
};

ROM_START( kc_d004 )
	ROM_REGION(0x2000, Z80_TAG, 0)
	ROM_LOAD_OPTIONAL("d004v20.bin",    0x0000, 0x2000, CRC(4f3494f1) SHA1(66f476de78fb474d9ac61c6eaffce3354fd66776))
ROM_END

ROM_START( kc_d004_gide )
	ROM_REGION(0x2000, Z80_TAG, 0)
	ROM_SYSTEM_BIOS(0, "v33_4", "ver 3.3 (KC 85/4)")
	ROMX_LOAD("d004v33_4.bin",  0x0000, 0x2000, CRC(1451efd7) SHA1(9db201af408adb02254094dc7aa7185bf5a7b9b1), ROM_BIOS(0) ) // KC85/4-5
	ROM_SYSTEM_BIOS(1, "v33_3", "ver 3.3 (KC 85/3)")
	ROMX_LOAD( "d004v33_3.bin", 0x0000, 0x2000, CRC(945f3e4b) SHA1(cce5d9eea82582270660c8275336b15bf9906253), ROM_BIOS(1) ) // KC85/3
	ROM_SYSTEM_BIOS(2, "v30", "ver 3.0")
	ROMX_LOAD("d004v30.bin",    0x0000, 0x2000, CRC(6fe0a283) SHA1(5582b2541a34a90c7a9516a6a222d4961fc54fcf), ROM_BIOS(2) ) // KC85/4-5
	ROM_SYSTEM_BIOS(3, "v31", "ver 3.1")
	ROMX_LOAD("d004v31.bin",    0x0000, 0x2000, CRC(712547de) SHA1(38b3164dce23573375fc0237f348d9a699fc6f9f), ROM_BIOS(3) ) // KC85/4-5
	ROM_SYSTEM_BIOS(4, "v32", "ver 3.2")
	ROMX_LOAD("d004v32.bin",    0x0000, 0x2000, CRC(9a3d3511) SHA1(8232adb5e5f0b25b52f9873cff14831da3a0398a), ROM_BIOS(4) ) // KC85/4-5
ROM_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(KC_D004,      kc_d004_device,      "kc_d004",      "D004 Floppy Disk Interface")
DEFINE_DEVICE_TYPE(KC_D004_GIDE, kc_d004_gide_device, "kc_d004_gide", "D004 Floppy Disk + GIDE Interface")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kc_d004_device - constructor
//-------------------------------------------------

kc_d004_device::kc_d004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kc_d004_device(mconfig, KC_D004, tag, owner, clock)
{
}

kc_d004_device::kc_d004_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_kcexp_interface( mconfig, *this ),
	m_cpu(*this, Z80_TAG),
	m_fdc(*this, UPD765_TAG),
	m_floppy0(*this, UPD765_TAG ":0"),
	m_floppy1(*this, UPD765_TAG ":1"),
	m_floppy2(*this, UPD765_TAG ":2"),
	m_floppy3(*this, UPD765_TAG ":3"),
	m_koppel_ram(*this, "koppelram"),
	m_reset_timer(nullptr), m_rom(nullptr), m_rom_base(0), m_enabled(0), m_connected(0), m_floppy(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kc_d004_device::device_start()
{
	m_rom  = memregion(Z80_TAG)->base();

	m_reset_timer = timer_alloc(FUNC(kc_d004_device::reset_tick), this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_d004_device::device_reset()
{
	m_rom_base = 0xc000;
	m_enabled = m_connected = 0;
	m_floppy = m_floppy0->get_device();

	// hold cpu at reset
	m_reset_timer->adjust(attotime::zero);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void kc_d004_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, 8_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &kc_d004_device::kc_d004_mem);
	m_cpu->set_addrmap(AS_IO, &kc_d004_device::kc_d004_io);
	m_cpu->set_daisy_config(kc_d004_daisy_chain);

	z80ctc_device &ctc(Z80CTC(config, Z80CTC_TAG, 8_MHz_XTAL / 2));
	ctc.intr_callback().set_inputline(Z80_TAG, 0);
	ctc.zc_callback<0>().set(Z80CTC_TAG, FUNC(z80ctc_device::trg1));
	ctc.zc_callback<1>().set(Z80CTC_TAG, FUNC(z80ctc_device::trg2));
	ctc.zc_callback<2>().set(Z80CTC_TAG, FUNC(z80ctc_device::trg3));

	UPD765A(config, m_fdc, 8_MHz_XTAL);
	m_fdc->set_ready_line_connected(false);
	m_fdc->set_select_lines_connected(false);
	m_fdc->intrq_wr_callback().set(FUNC(kc_d004_device::fdc_irq));
	FLOPPY_CONNECTOR(config, m_floppy0, kc_d004_floppies, "525qd", kc_d004_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, kc_d004_floppies, "525qd", kc_d004_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy2, kc_d004_floppies, "525qd", kc_d004_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy3, kc_d004_floppies, "525qd", kc_d004_device::floppy_formats);
}

//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const tiny_rom_entry *kc_d004_device::device_rom_region() const
{
	return ROM_NAME( kc_d004 );
}

//-------------------------------------------------
//  reset_tick - reset the main CPU when needed
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(kc_d004_device::reset_tick)
{
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

/*-------------------------------------------------
    set module status
-------------------------------------------------*/

void kc_d004_device::control_w(uint8_t data)
{
	m_enabled = BIT(data, 0);
	m_connected = BIT(data, 2);
	m_rom_base = (data & 0x20) ? 0xe000 : 0xc000;
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kc_d004_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= m_rom_base && offset < (m_rom_base + 0x2000) && m_enabled)
		data = m_rom[offset & 0x1fff];
}

//-------------------------------------------------
//  IO read
//-------------------------------------------------

void kc_d004_device::io_read(offs_t offset, uint8_t &data)
{
	if ((offset & 0xff) == 0x80)
	{
		uint8_t slot_id = (offset>>8) & 0xff;

		if (slot_id == 0xfc)
			data = module_id_r();
	}
	else
	{
		if (m_connected)
		{
			switch(offset & 0xff)
			{
			case 0xf0:
			case 0xf1:
			case 0xf2:
			case 0xf3:
				data = m_koppel_ram[((offset & 0x03)<<8) | ((offset>>8) & 0xff)];
				break;
			}
		}
	}
}

//-------------------------------------------------
//  IO write
//-------------------------------------------------

void kc_d004_device::io_write(offs_t offset, uint8_t data)
{
	if ((offset & 0xff) == 0x80)
	{
		uint8_t slot_id = (offset>>8) & 0xff;

		if (slot_id == 0xfc)
			control_w(data);
	}
	else
	{
		if (m_connected)
		{
			switch(offset & 0xff)
			{
			case 0xf0:
			case 0xf1:
			case 0xf2:
			case 0xf3:
				m_koppel_ram[((offset & 0x03)<<8) | ((offset>>8) & 0xff)] = data;
				break;
			case 0xf4:
				if (data & 0x01)
					m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

				if (data & 0x02)
				{
					for (int i=0; i<0xfc00; i++)
						m_cpu->space(AS_PROGRAM).write_byte(i, 0);

					m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
				}

				if (data & 0x04)
					m_cpu->set_input_line(INPUT_LINE_RESET, HOLD_LINE);

				if (data & 0x08)
					m_cpu->set_input_line(INPUT_LINE_NMI, HOLD_LINE);

				//printf("D004 CPU state: %x\n", data & 0x0f);
				break;
			}
		}
	}
}

//**************************************************************************
//  FDC emulation
//**************************************************************************

uint8_t kc_d004_device::hw_input_gate_r()
{
	/*
	    bit 7: DMA Request (DRQ from FDC)
	    bit 6: Interrupt (INT from FDC)
	    bit 5: Drive Ready
	    bit 4: Index pulse from disc
	*/

	uint8_t hw_input_gate = 0x0f;

	if (m_floppy && !m_floppy->idx_r())
		hw_input_gate |= 0x10;

	if (m_floppy && !m_floppy->ready_r())
		hw_input_gate |= 0x20;

	if (!m_fdc->get_irq())
		hw_input_gate |= 0x40;

	if (!m_fdc->get_drq())
		hw_input_gate |= 0x80;

	return hw_input_gate;
}

void kc_d004_device::fdd_select_w(uint8_t data)
{
	if (data & 0x01)
		m_floppy = m_floppy0->get_device();
	else if (data & 0x02)
		m_floppy = m_floppy1->get_device();
	else if (data & 0x04)
		m_floppy = m_floppy2->get_device();
	else if (data & 0x08)
		m_floppy = m_floppy3->get_device();
	else
		m_floppy = nullptr;

	if (m_floppy)
		m_floppy->mon_w(0);

	m_fdc->set_floppy(m_floppy);
}

void kc_d004_device::hw_terminal_count_w(uint8_t data)
{
	m_fdc->tc_w(true);
}

WRITE_LINE_MEMBER(kc_d004_device::fdc_irq)
{
	if (state)
		m_fdc->tc_w(false);
}



//**************************************************************************
//  D004 Floppy Disk + GIDE Interface
//**************************************************************************

//-------------------------------------------------
//  kc_d004_gide_device - constructor
//-------------------------------------------------

kc_d004_gide_device::kc_d004_gide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kc_d004_device(mconfig, KC_D004_GIDE, tag, owner, clock),
	m_ata(*this, ATA_TAG), m_ata_data(0), m_lh(0)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void kc_d004_gide_device::device_add_mconfig(machine_config &config)
{
	kc_d004_device::device_add_mconfig(config);

	m_cpu->set_addrmap(AS_IO, &kc_d004_gide_device::kc_d004_gide_io);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
}


//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const tiny_rom_entry *kc_d004_gide_device::device_rom_region() const
{
	return ROM_NAME( kc_d004_gide );
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_d004_gide_device::device_reset()
{
	kc_d004_device::device_reset();

	m_ata_data  = 0;
	m_lh        = 0;
}

//-------------------------------------------------
//  GIDE read
//-------------------------------------------------

uint8_t kc_d004_gide_device::gide_r(offs_t offset)
{
	uint8_t data = 0xff;
	uint8_t io_addr = offset & 0x0f;

	if (io_addr == 0x05)
	{
		uint8_t rtc_addr = (offset >> 8) & 0x0f;

		// TODO RTC-72421
		logerror("GIDE %s read RTC 0x%02x\n", tag(), rtc_addr);
		data = 0;
	}
	else
	{
		int ide_cs = (io_addr & 0x08) ? 0 : ((io_addr == 0x06 || io_addr == 0x07) ? 1 : -1);

		if (ide_cs != -1)
		{
			int data_shift = 0;

			if (io_addr == 0x08 && m_lh)
				data_shift = 8;

			if (io_addr == 0x06 || io_addr == 0x07 || io_addr > 0x08 || (io_addr == 0x08 && !m_lh))
			{
				if (ide_cs == 0 )
				{
					m_ata_data = m_ata->cs0_r(io_addr & 0x07);
				}
				else
				{
					m_ata_data = m_ata->cs1_r(io_addr & 0x07);
				}
			}

			data = (m_ata_data >> data_shift) & 0xff;
		}

		m_lh = (io_addr == 0x08) ? !m_lh : ((io_addr > 0x08) ? 0 : m_lh);
	}

	return data;
}

//-------------------------------------------------
//  GIDE write
//-------------------------------------------------

void kc_d004_gide_device::gide_w(offs_t offset, uint8_t data)
{
	uint8_t io_addr = offset & 0x0f;

	if (io_addr == 0x05)
	{
		uint8_t rtc_addr = (offset >> 8) & 0x0f;

		// TODO RTC-72421
		logerror("GIDE %s wrire RTC 0x%02x 0x%02x\n", tag(), rtc_addr, data);
	}
	else
	{
		int ide_cs = (io_addr & 0x08) ? 0 : ((io_addr == 0x06 || io_addr == 0x07) ? 1 : -1);

		if (ide_cs != -1)
		{
			int data_shift = 0;

			if (io_addr == 0x08 && m_lh)
				data_shift = 8;

			m_ata_data = (data << data_shift) | (m_ata_data & (0xff00 >> data_shift));

			if (io_addr == 0x06 || io_addr == 0x07 || io_addr > 0x08 || (io_addr == 0x08 && m_lh))
			{
				if (ide_cs == 0)
				{
					m_ata->cs0_w(io_addr & 0x07, m_ata_data);
				}
				else
				{
					m_ata->cs1_w(io_addr & 0x07, m_ata_data);
				}
			}
		}

		m_lh = (io_addr == 0x08) ? !m_lh : ((io_addr > 0x08) ? 0 : m_lh);
	}
}
