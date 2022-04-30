// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cumana 68008 Upgrade Board

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Cumana_680082ndProcessor.html

    TODO:
    - Test Winchester, when suitable image is available.
    - RTC needs further work.
    - Interrupt handling needs more testing.

    Known Winchester:
    - NEC D3126 -chs 615,4,17 -ss 256 (not yet imaged)

**********************************************************************/


#include "emu.h"
#include "cumana68k.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/devices.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_CUMANA68K, bbc_cumana68k_device, "bbc_cumana68k", "Cumana 68008 Upgrade Board")


//-------------------------------------------------
//  ADDRESS_MAP( cumana68k_mem )
//-------------------------------------------------

void bbc_cumana68k_device::cumana68k_mem(address_map &map)
{
	map(0x00000, 0x7ffff).rw(FUNC(bbc_cumana68k_device::mem6502_r), FUNC(bbc_cumana68k_device::mem6502_w));
	map(0x80000, 0xfffff).ram();
}

//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void bbc_cumana68k_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_OS9_FORMAT);
}

//-------------------------------------------------
//  ROM( cumana68k )
//-------------------------------------------------

ROM_START(cumana68k)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_LOAD("os9boot.rom", 0x0000, 0x4000, CRC(9b7d6754) SHA1(1cb54119bf13b39d598573d8d6a7e9f70d88b50d))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_cumana68k_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	M68008(config, m_m68008, 16_MHz_XTAL / 2);
	m_m68008->set_addrmap(AS_PROGRAM, &bbc_cumana68k_device::cumana68k_mem);

	/* fdc */
	WD2797(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(m_pia_rtc, FUNC(pia6821_device::cb1_w));
	m_fdc->drq_wr_callback().set(m_pia_sasi, FUNC(pia6821_device::pb6_w));
	m_fdc->drq_wr_callback().append(DEVICE_SELF_OWNER, FUNC(bbc_internal_slot_device::nmi_w));
	m_fdc->set_force_ready(true);

	FLOPPY_CONNECTOR(config, m_floppy[0], "525qd", FLOPPY_525_QD, true, bbc_cumana68k_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], "525qd", FLOPPY_525_QD, true, bbc_cumana68k_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], "525qd", FLOPPY_525_QD, false, bbc_cumana68k_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[3], "525qd", FLOPPY_525_QD, false, bbc_cumana68k_device::floppy_formats).enable_sound(true);

	/* sasi */
	NSCSI_BUS(config, "sasi");
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, "s1410");
	NSCSI_CONNECTOR(config, "sasi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB)
		.machine_config([this](device_t* device) {
			downcast<nscsi_callback_device&>(*device).io_callback().set(m_pia_sasi, FUNC(pia6821_device::pb2_w));
			downcast<nscsi_callback_device&>(*device).bsy_callback().set(m_pia_sasi, FUNC(pia6821_device::pb3_w));
			downcast<nscsi_callback_device&>(*device).cd_callback().set(m_pia_sasi, FUNC(pia6821_device::pb4_w));
			downcast<nscsi_callback_device&>(*device).msg_callback().set(m_pia_sasi, FUNC(pia6821_device::pb5_w));
			downcast<nscsi_callback_device&>(*device).req_callback().set(m_pia_sasi, FUNC(pia6821_device::pb7_w));
			downcast<nscsi_callback_device&>(*device).req_callback().append(m_pia_sasi, FUNC(pia6821_device::ca1_w));
		});

	PIA6821(config, m_pia_sasi, 0);
	m_pia_sasi->readpa_handler().set(m_sasi, FUNC(nscsi_callback_device::read));
	m_pia_sasi->writepa_handler().set(m_sasi, FUNC(nscsi_callback_device::write));
	m_pia_sasi->writepb_handler().set(FUNC(bbc_cumana68k_device::pia_sasi_pb_w));
	m_pia_sasi->readca1_handler().set(m_sasi, FUNC(nscsi_callback_device::req_r));
	m_pia_sasi->readcb1_handler().set_constant(1); // tied to +5V
	m_pia_sasi->ca2_handler().set(m_sasi, FUNC(nscsi_callback_device::ack_w));
	m_pia_sasi->cb2_handler().set(m_sasi, FUNC(nscsi_callback_device::rst_w));
	m_pia_sasi->irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_pia_sasi->irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia_rtc, 0);
	m_pia_rtc->readpa_handler().set([this]() { return m_mc146818_data; });
	m_pia_rtc->writepa_handler().set([this](uint8_t data) { m_mc146818_data = data; });
	m_pia_rtc->writepb_handler().set(FUNC(bbc_cumana68k_device::pia_rtc_pb_w));
	m_pia_rtc->ca2_handler().set(FUNC(bbc_cumana68k_device::rtc_ce_w));
	m_pia_rtc->cb2_handler().set(FUNC(bbc_cumana68k_device::reset68008_w));
	m_pia_rtc->irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	m_pia_rtc->irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_internal_slot_device::irq_w));

	/* rtc */
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_pia_rtc, FUNC(pia6821_device::ca1_w));

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_68008").set_original("bbc_flop_68000").set_filter("Cumana");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_cumana68k_device::device_rom_region() const
{
	return ROM_NAME( cumana68k );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_cumana68k_device - constructor
//-------------------------------------------------

bbc_cumana68k_device::bbc_cumana68k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_CUMANA68K, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_m68008(*this, "m68008")
	, m_pia_sasi(*this, "pia_sasi")
	, m_pia_rtc(*this, "pia_rtc")
	, m_irqs(*this, "irqs")
	, m_rtc(*this, "rtc")
	, m_fdc(*this, "wd2797")
	, m_floppy(*this, "wd2797:%u", 0)
	, m_sasi(*this, "sasi:7:scsicb")
	, m_masknmi(0)
	, m_mc146818_data(0)
	, m_mc146818_as(0)
	, m_mc146818_ds(0)
	, m_mc146818_rw(0)
	, m_mc146818_ce(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_cumana68k_device::device_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_readwrite_handler(0xfc40, 0xfc43, read8sm_delegate(*m_pia_sasi, FUNC(pia6821_device::read)), write8sm_delegate(*m_pia_sasi, FUNC(pia6821_device::write)));
	program.install_readwrite_handler(0xfc44, 0xfc47, read8sm_delegate(*m_pia_rtc, FUNC(pia6821_device::read)), write8sm_delegate(*m_pia_rtc, FUNC(pia6821_device::write)));
	program.install_readwrite_handler(0xfc48, 0xfc4b, read8sm_delegate(*m_fdc, FUNC(wd2797_device::read)), write8sm_delegate(*m_fdc, FUNC(wd2797_device::write)));
	program.install_write_handler(0xfc4c, 0xfc4f, write8sm_delegate(*this, FUNC(bbc_cumana68k_device::fsel_w)));

	/* register for save states */
	save_item(NAME(m_mc146818_as));
	save_item(NAME(m_mc146818_ds));
	save_item(NAME(m_mc146818_rw));
	save_item(NAME(m_masknmi));
}

//-------------------------------------------------
//  device_reset_after_children - reset after child devices
//-------------------------------------------------

void bbc_cumana68k_device::device_reset_after_children()
{
	/* Pull up resistor R12 makes CB2 high when it's configured as an input, so set CB2 as output and high to compensate */
	m_pia_rtc->write(0x03, 0x3c);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER(bbc_cumana68k_device::reset68008_w)
{
	m_m68008->set_input_line(INPUT_LINE_HALT, state);
	m_m68008->set_input_line(INPUT_LINE_RESET, state);
}

WRITE_LINE_MEMBER(bbc_cumana68k_device::irq6502_w)
{
	m_m68008->set_input_line(M68K_IRQ_2, state);
}

void bbc_cumana68k_device::fsel_w(offs_t offset, uint8_t data)
{
	/* TODO: what does FSEL actually do? seems to affect the floppy FCLK */
	//logerror("fsel_w %02x\n", data);
}


uint8_t bbc_cumana68k_device::mem6502_r(offs_t offset)
{
	uint8_t data = 0xff;

	address_space &program = m_maincpu->space(AS_PROGRAM);

	switch (offset & 0xf000)
	{
	case 0x0000:
		data = program.read_byte(offset | 0x2000);
		break;
	case 0x2000:
		data = program.read_byte(offset & 0x0fff);
		break;
	default:
		data = program.read_byte(offset);
		break;
	}
	return data;
}

void bbc_cumana68k_device::mem6502_w(offs_t offset, uint8_t data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	switch (offset & 0xf000)
	{
	case 0x0000:
		program.write_byte(offset | 0x2000, data);
		break;
	case 0x2000:
		program.write_byte(offset & 0x0fff, data);
		break;
	default:
		program.write_byte(offset, data);
		break;
	}
}


WRITE_LINE_MEMBER(bbc_cumana68k_device::rtc_ce_w)
{
	m_mc146818_ce = !state;
}

void bbc_cumana68k_device::mc146818_set(int as, int ds, int rw)
{
	/* if chip enabled */
	if (m_mc146818_ce)
	{
		/* if address select is set then set the address in the 146818 */
		if (m_mc146818_as & !as)
		{
			m_rtc->write(0, m_mc146818_data);
			//logerror("addr_w: %02x\n", m_mc146818_data);
		}

		/* if data select is set then access the data in the 146818 */
		if (m_mc146818_ds & !ds)
		{
			if (m_mc146818_rw)
			{
				m_mc146818_data = m_rtc->read(1);
				//logerror("data_r: %02x\n", m_mc146818_data);
			}
			else
			{
				m_rtc->write(1, m_mc146818_data);
				//logerror("data_w: %02x\n", m_mc146818_data);
			}
		}
	}
	m_mc146818_as = as;
	m_mc146818_ds = ds;
	m_mc146818_rw = rw;
}


void bbc_cumana68k_device::pia_rtc_pb_w(uint8_t data)
{
	/* bit 0, 1: drive select */
	floppy_image_device *floppy = m_floppy[data & 0x03]->get_device();

	m_fdc->set_floppy(floppy);

	/* bit 2: motor enable */
	if (floppy)
		floppy->mon_w(BIT(data, 2));

	/* bit 3: density */
	m_fdc->dden_w(BIT(data, 3));

	/* bit 4: enable precomp */

	/* bit 5, 6, 7: rtc */
	//logerror("rtc: as=%d ds=%d rw=%d\n", BIT(data, 5), BIT(data, 6), BIT(data, 7));
	mc146818_set(BIT(data, 5), BIT(data, 6), BIT(data, 7));
}


void bbc_cumana68k_device::pia_sasi_pb_w(uint8_t data)
{
	/* bit 0: masknmi */
	m_masknmi = BIT(data, 0);
	logerror("mask nmi %d\n", m_masknmi);
}
