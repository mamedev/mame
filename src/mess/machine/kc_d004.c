/***************************************************************************

    kc_d004.c

    KC85 D004 Floppy Disk Interface

***************************************************************************/

#include "emu.h"
#include "kc_d004.h"

#define Z80_TAG			"disk"
#define Z80CTC_TAG		"z80ctc"
#define UPD765_TAG		"upd765"
#define IDE_TAG			"ide"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static ADDRESS_MAP_START(kc_d004_mem, AS_PROGRAM, 8, kc_d004_device)
	AM_RANGE(0x0000, 0xfbff) AM_RAM
	AM_RANGE(0xfc00, 0xffff) AM_RAM		AM_SHARE("koppelram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(kc_d004_io, AS_IO, 8, kc_d004_device)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf0) AM_DEVREAD_LEGACY(UPD765_TAG, upd765_status_r)
	AM_RANGE(0xf1, 0xf1) AM_DEVREADWRITE_LEGACY(UPD765_TAG, upd765_data_r, upd765_data_w)
	AM_RANGE(0xf2, 0xf3) AM_DEVREADWRITE_LEGACY(UPD765_TAG, upd765_dack_r, upd765_dack_w)
	AM_RANGE(0xf4, 0xf4) AM_READ(hw_input_gate_r)
	AM_RANGE(0xf6, 0xf7) AM_WRITE(fdd_select_w)
	AM_RANGE(0xf8, 0xf9) AM_WRITE(hw_terminal_count_w)
	AM_RANGE(0xfc, 0xff) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START(kc_d004_gide_io, AS_IO, 8, kc_d004_gide_device)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00f0, 0x00f0) AM_MIRROR(0xff00)	AM_DEVREAD_LEGACY(UPD765_TAG, upd765_status_r)
	AM_RANGE(0x00f1, 0x00f1) AM_MIRROR(0xff00)	AM_DEVREADWRITE_LEGACY(UPD765_TAG, upd765_data_r, upd765_data_w)
	AM_RANGE(0x00f2, 0x00f3) AM_MIRROR(0xff00)	AM_DEVREADWRITE_LEGACY(UPD765_TAG, upd765_dack_r, upd765_dack_w)
	AM_RANGE(0x00f4, 0x00f4) AM_MIRROR(0xff00)	AM_READ(hw_input_gate_r)
	AM_RANGE(0x00f6, 0x00f7) AM_MIRROR(0xff00)	AM_WRITE(fdd_select_w)
	AM_RANGE(0x00f8, 0x00f9) AM_MIRROR(0xff00)	AM_WRITE(hw_terminal_count_w)
	AM_RANGE(0x00fc, 0x00ff) AM_MIRROR(0xff00)	AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(gide_r, gide_w)
ADDRESS_MAP_END

static LEGACY_FLOPPY_OPTIONS_START(kc_d004)
	LEGACY_FLOPPY_OPTION(kc85, "img", "KC85 disk image 800KB", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([5])
		SECTOR_LENGTH([1024])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION(kc85, "img", "KC85 disk image 720KB", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION(kc85, "img", "KC85 disk image 640KB", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface kc_d004_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(kc_d004),
	"floppy_5_25",
	NULL
};

static device_t *kc_d004_get_fdd(device_t *device, int floppy_index)
{
	kc_d004_device* owner = dynamic_cast<kc_d004_device *>(device->owner());
	return owner->get_active_fdd();
}

static const upd765_interface kc_d004_interface =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, kc_d004_device, fdc_interrupt),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, kc_d004_device, fdc_dma_request),
	kc_d004_get_fdd,
	UPD765_RDY_PIN_NOT_CONNECTED,
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

static const z80_daisy_config kc_d004_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ NULL }
};

static Z80CTC_INTERFACE( kc_d004_ctc_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, 0),				/* interrupt callback */
	DEVCB_DEVICE_LINE_MEMBER(Z80CTC_TAG, z80ctc_device, trg1),	/* ZC/TO0 callback */
	DEVCB_DEVICE_LINE_MEMBER(Z80CTC_TAG, z80ctc_device, trg2),	/* ZC/TO1 callback */
	DEVCB_DEVICE_LINE_MEMBER(Z80CTC_TAG, z80ctc_device, trg3)	/* ZC/TO2 callback */
};

static MACHINE_CONFIG_FRAGMENT(kc_d004)
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_PROGRAM_MAP(kc_d004_mem)
	MCFG_CPU_IO_MAP(kc_d004_io)
	MCFG_CPU_CONFIG(kc_d004_daisy_chain)

	MCFG_Z80CTC_ADD( Z80CTC_TAG, XTAL_8MHz/2, kc_d004_ctc_intf )

	MCFG_UPD765A_ADD(UPD765_TAG, kc_d004_interface)
	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(kc_d004_floppy_interface)
MACHINE_CONFIG_END


static MACHINE_CONFIG_FRAGMENT(kc_d004_gide)
	MCFG_FRAGMENT_ADD(kc_d004)

	MCFG_CPU_MODIFY(Z80_TAG)
	MCFG_CPU_IO_MAP(kc_d004_gide_io)

	MCFG_IDE_CONTROLLER_ADD(IDE_TAG, NULL, ide_image_devices, "hdd", "hdd", false)
MACHINE_CONFIG_END


ROM_START( kc_d004 )
	ROM_REGION(0x2000, Z80_TAG, 0)
	ROM_LOAD_OPTIONAL("d004v20.bin",	0x0000, 0x2000, CRC(4f3494f1) SHA1(66f476de78fb474d9ac61c6eaffce3354fd66776))
ROM_END

ROM_START( kc_d004_gide )
	ROM_REGION(0x2000, Z80_TAG, 0)
	ROM_LOAD_OPTIONAL("d004v33_3.bin",	0x0000, 0x2000, CRC(945f3e4b) SHA1(cce5d9eea82582270660c8275336b15bf9906253))	// KC85/3
	ROM_LOAD_OPTIONAL("d004v33_4.bin",	0x0000, 0x2000, CRC(1451efd7) SHA1(9db201af408adb02254094dc7aa7185bf5a7b9b1))	// KC85/4-5
	ROM_LOAD_OPTIONAL("d004v30.bin",	0x0000, 0x2000, CRC(6fe0a283) SHA1(5582b2541a34a90c7a9516a6a222d4961fc54fcf))	// KC85/4-5
	ROM_LOAD_OPTIONAL("d004v31.bin",	0x0000, 0x2000, CRC(712547de) SHA1(38b3164dce23573375fc0237f348d9a699fc6f9f))	// KC85/4-5
	ROM_LOAD_OPTIONAL("d004v32.bin",	0x0000, 0x2000, CRC(9a3d3511) SHA1(8232adb5e5f0b25b52f9873cff14831da3a0398a))	// KC85/4-5
ROM_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type KC_D004 = &device_creator<kc_d004_device>;
const device_type KC_D004_GIDE = &device_creator<kc_d004_gide_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kc_d004_device - constructor
//-------------------------------------------------

kc_d004_device::kc_d004_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
      : device_t(mconfig, KC_D004, "D004 Floppy Disk Interface", tag, owner, clock),
		device_kcexp_interface( mconfig, *this ),
		m_cpu(*this, Z80_TAG),
		m_fdc(*this, UPD765_TAG),
		m_koppel_ram(*this, "koppelram")
{
}

kc_d004_device::kc_d004_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
      : device_t(mconfig, type, name, tag, owner, clock),
		device_kcexp_interface( mconfig, *this ),
		m_cpu(*this, Z80_TAG),
		m_fdc(*this, UPD765_TAG),
		m_koppel_ram(*this, "koppelram")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kc_d004_device::device_start()
{
	m_rom  = memregion(Z80_TAG)->base();

	m_reset_timer = timer_alloc(TIMER_RESET);
	m_tc_clear_timer = timer_alloc(TIMER_TC_CLEAR);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_d004_device::device_reset()
{
	m_rom_base = 0xc000;
	m_enabled = m_connected = 0;
	m_floppy = downcast<device_t *>(subdevice(FLOPPY_0));

	// hold cpu at reset
	m_reset_timer->adjust(attotime::zero);
}

//-------------------------------------------------
//  device_mconfig_additions
//-------------------------------------------------

machine_config_constructor kc_d004_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kc_d004 );
}

//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const rom_entry *kc_d004_device::device_rom_region() const
{
	return ROM_NAME( kc_d004 );
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void kc_d004_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_RESET:
			device_set_input_line(m_cpu, INPUT_LINE_RESET, ASSERT_LINE);
			break;
		case TIMER_TC_CLEAR:
			upd765_tc_w(m_fdc, 0x00);
			break;
	}
}

/*-------------------------------------------------
    set module status
-------------------------------------------------*/

void kc_d004_device::control_w(UINT8 data)
{
	m_enabled = BIT(data, 0);
	m_connected = BIT(data, 2);
	m_rom_base = (data & 0x20) ? 0xe000 : 0xc000;
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void kc_d004_device::read(offs_t offset, UINT8 &data)
{
	if (offset >= m_rom_base && offset < (m_rom_base + 0x2000) && m_enabled)
		data = m_rom[offset & 0x1fff];
}

//-------------------------------------------------
//  IO read
//-------------------------------------------------

void kc_d004_device::io_read(offs_t offset, UINT8 &data)
{
	if ((offset & 0xff) == 0x80)
	{
		UINT8 slot_id = (offset>>8) & 0xff;

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

void kc_d004_device::io_write(offs_t offset, UINT8 data)
{
	if ((offset & 0xff) == 0x80)
	{
		UINT8 slot_id = (offset>>8) & 0xff;

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
					device_set_input_line(m_cpu, INPUT_LINE_RESET, CLEAR_LINE);

				if (data & 0x02)
				{
					for (int i=0; i<0xfc00; i++)
						m_cpu->memory().space(AS_PROGRAM)->write_byte(i, 0);

					device_set_input_line(m_cpu, INPUT_LINE_RESET, ASSERT_LINE);
				}

				if (data & 0x04)
					device_set_input_line(m_cpu, INPUT_LINE_RESET, HOLD_LINE);

				if (data & 0x08)
					device_set_input_line(m_cpu, INPUT_LINE_NMI, HOLD_LINE);

				//printf("D004 CPU state: %x\n", data & 0x0f);
				break;
			}
		}
	}
}

//**************************************************************************
//  FDC emulation
//**************************************************************************

READ8_MEMBER(kc_d004_device::hw_input_gate_r)
{
	/*

        bit 7: DMA Request (DRQ from FDC)
        bit 6: Interrupt (INT from FDC)
        bit 5: Drive Ready
        bit 4: Index pulse from disc

    */

	if (floppy_ready_r(m_floppy))
		m_hw_input_gate |= 0x20;
	else
		m_hw_input_gate &= ~0x20;

	if (floppy_index_r(m_floppy))
		m_hw_input_gate &= ~0x10;
	else
		m_hw_input_gate |= 0x10;

	return m_hw_input_gate | 0x0f;
}

WRITE8_MEMBER(kc_d004_device::fdd_select_w)
{
	if (data & 0x01)
		m_floppy = downcast<device_t *>(subdevice(FLOPPY_0));
	else if (data & 0x02)
		m_floppy = downcast<device_t *>(subdevice(FLOPPY_1));
	else if (data & 0x04)
		m_floppy = downcast<device_t *>(subdevice(FLOPPY_2));
	else if (data & 0x08)
		m_floppy = downcast<device_t *>(subdevice(FLOPPY_3));
}

WRITE8_MEMBER(kc_d004_device::hw_terminal_count_w)
{
	upd765_tc_w(m_fdc, 1);

	m_tc_clear_timer->adjust(attotime::from_nsec(200));
}

WRITE_LINE_MEMBER( kc_d004_device::fdc_interrupt )
{
	if (state)
		m_hw_input_gate &= ~0x40;
	else
		m_hw_input_gate |= 0x40;
}

WRITE_LINE_MEMBER( kc_d004_device::fdc_dma_request )
{
	if (state)
		m_hw_input_gate &= ~0x80;
	else
		m_hw_input_gate |= 0x80;
}



//**************************************************************************
//  D004 Floppy Disk + GIDE Interface
//**************************************************************************

//-------------------------------------------------
//  kc_d004_gide_device - constructor
//-------------------------------------------------

kc_d004_gide_device::kc_d004_gide_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
      : kc_d004_device(mconfig, KC_D004, "D004 Floppy Disk + GIDE Interface", tag, owner, clock),
	    m_ide(*this, IDE_TAG)
{
}

//-------------------------------------------------
//  device_mconfig_additions
//-------------------------------------------------

machine_config_constructor kc_d004_gide_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kc_d004_gide );
}


//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const rom_entry *kc_d004_gide_device::device_rom_region() const
{
	return ROM_NAME( kc_d004_gide );
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_d004_gide_device::device_reset()
{
	kc_d004_device::device_reset();

	m_ide_data	= 0;
	m_lh		= 0;
}

//-------------------------------------------------
//  GIDE read
//-------------------------------------------------

READ8_MEMBER(kc_d004_gide_device::gide_r)
{
	UINT8 data = 0xff;
	UINT8 io_addr = offset & 0x0f;

	if (io_addr == 0x05)
	{
		UINT8 rtc_addr = (offset >> 8) & 0x0f;

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
				m_ide_data = ide_bus_r(m_ide, ide_cs, io_addr & 0x07);

			data = (m_ide_data >> data_shift) & 0xff;
		}

		m_lh = (io_addr == 0x08) ? !m_lh : ((io_addr > 0x08) ? 0 : m_lh);
	}

	return data;
}

//-------------------------------------------------
//  GIDE write
//-------------------------------------------------

WRITE8_MEMBER(kc_d004_gide_device::gide_w)
{
	UINT8 io_addr = offset & 0x0f;

	if (io_addr == 0x05)
	{
		UINT8 rtc_addr = (offset >> 8) & 0x0f;

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

			m_ide_data = (data << data_shift) | (m_ide_data & (0xff00 >> data_shift));

			if (io_addr == 0x06 || io_addr == 0x07 || io_addr > 0x08 || (io_addr == 0x08 && m_lh))
				ide_bus_w(m_ide, ide_cs, io_addr & 0x07, m_ide_data);
		}

		m_lh = (io_addr == 0x08) ? !m_lh : ((io_addr > 0x08) ? 0 : m_lh);
	}
}
