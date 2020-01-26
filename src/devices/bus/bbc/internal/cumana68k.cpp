// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cumana 68008 2nd processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Cumana_680082ndProcessor.html

**********************************************************************/


#include "emu.h"
#include "cumana68k.h"
#include "bus/scsi/scsihd.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_CUMANA68K, bbc_cumana68k_device, "bbc_cumana68k", "Cumana 68008 2nd Processor")


//-------------------------------------------------
//  ADDRESS_MAP( cumana68k_mem )
//-------------------------------------------------

void bbc_cumana68k_device::cumana68k_mem(address_map &map)
{
	map(0x000000, 0x00ffff).mirror(0x70000).rw(FUNC(bbc_cumana68k_device::mem6502_r), FUNC(bbc_cumana68k_device::mem6502_w));
	map(0x080000, 0x0fffff).ram();
}

//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(bbc_cumana68k_device::floppy_formats)
	FLOPPY_OS9_FORMAT
FLOPPY_FORMATS_END

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
	SCSI_PORT(config, m_sasibus);
	m_sasibus->set_data_input_buffer(m_sasi_data_in);
	m_sasibus->sel_handler().set(m_pia_sasi, FUNC(pia6821_device::pb1_w));
	m_sasibus->io_handler().set(m_pia_sasi, FUNC(pia6821_device::pb2_w));
	m_sasibus->bsy_handler().set(m_pia_sasi, FUNC(pia6821_device::pb3_w));
	m_sasibus->cd_handler().set(m_pia_sasi, FUNC(pia6821_device::pb4_w));
	m_sasibus->msg_handler().set(m_pia_sasi, FUNC(pia6821_device::pb5_w));
	m_sasibus->req_handler().set(m_pia_sasi, FUNC(pia6821_device::pb7_w));
	m_sasibus->req_handler().append(m_pia_sasi, FUNC(pia6821_device::ca1_w));
	m_sasibus->ack_handler().set(m_pia_sasi, FUNC(pia6821_device::ca2_w));
	m_sasibus->set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));

	OUTPUT_LATCH(config, m_sasi_data_out);
	m_sasibus->set_output_latch(*m_sasi_data_out);

	INPUT_BUFFER(config, m_sasi_data_in);

	PIA6821(config, m_pia_sasi, 0);
	m_pia_sasi->readpa_handler().set(m_sasi_data_in, FUNC(input_buffer_device::bus_r));
	m_pia_sasi->writepa_handler().set(m_sasi_data_out, FUNC(output_latch_device::bus_w));
	m_pia_sasi->writepb_handler().set(FUNC(bbc_cumana68k_device::pia_sasi_pb_w));
	m_pia_sasi->readcb1_handler().set_constant(1); // tied to +5V
	m_pia_sasi->cb2_handler().set(m_sasibus, FUNC(scsi_port_device::write_rst));
	m_pia_sasi->irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_pia_sasi->irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia_rtc, 0);
	m_pia_rtc->readpa_handler().set(m_rtc, FUNC(mc146818_device::read));
	m_pia_rtc->writepa_handler().set(m_rtc, FUNC(mc146818_device::write));
	m_pia_rtc->writepb_handler().set(FUNC(bbc_cumana68k_device::pia_rtc_pb_w));
	m_pia_rtc->ca2_handler().set(FUNC(bbc_cumana68k_device::rtc_ce_w));
	m_pia_rtc->cb2_handler().set(FUNC(bbc_cumana68k_device::reset68008_w));
	m_pia_rtc->irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	m_pia_rtc->irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_internal_slot_device::irq_w));
	m_irqs->output_handler().append_inputline(m_m68008, M68K_IRQ_2);

	/* rtc */
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_pia_rtc, FUNC(pia6821_device::ca1_w));

	/* software lists */
	//SOFTWARE_LIST(config, "flop_ls_68008").set_original("bbc_flop_68008");
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
	, m_sasibus(*this, "sasibus")
	, m_sasi_data_out(*this, "sasi_data_out")
	, m_sasi_data_in(*this, "sasi_data_in")
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


void bbc_cumana68k_device::fsel_w(offs_t offset, uint8_t data)
{
	/* TODO: what does FSEL actually do? seems to affect the floppy FCLK */
	//logerror("fsel_w %02x\n", data);
}


READ8_MEMBER(bbc_cumana68k_device::mem6502_r)
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

WRITE8_MEMBER(bbc_cumana68k_device::mem6502_w)
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
	m_mc146818_ce = state;
	mc146818_set();
}


void bbc_cumana68k_device::mc146818_set()
{
	/* if chip enabled */
	if (m_mc146818_ce)
	{
		/* if data select is set then access the data in the 146818 */
		if (m_mc146818_ds)
		{
			if (m_mc146818_rw)
			{
				m_pia_rtc->set_a_input(m_rtc->read(1));
			}
			else
			{
				m_rtc->write(1, m_pia_rtc->a_output());
			}
		}

		/* if address select is set then set the address in the 146818 */
		if (m_mc146818_as)
		{
			m_rtc->write(0, m_pia_rtc->a_output());
		}
	}
}


WRITE8_MEMBER(bbc_cumana68k_device::pia_rtc_pb_w)
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
	m_mc146818_as = BIT(data, 5);
	m_mc146818_ds = BIT(data, 6);
	m_mc146818_rw = BIT(data, 7);
	mc146818_set();
}


WRITE8_MEMBER(bbc_cumana68k_device::pia_sasi_pb_w)
{
	/* bit 0: masknmi */
	m_masknmi = BIT(data, 0);
}
