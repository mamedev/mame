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
	map.unmap_value_high();
	map(0x000000, 0x00ffff).rw(FUNC(bbc_cumana68k_device::dma_r), FUNC(bbc_cumana68k_device::dma_w));
	map(0x010000, 0x0fffff).ram();
}

//-------------------------------------------------
//  SLOT_INTERFACE( os9_floppies )
//-------------------------------------------------

static void os9_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
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
	//m_m68008->set_irq_acknowledge_callback(device_irq_acknowledge_delegate(FUNC(bbc_cumana68k_device::irq_callback), this));

	/* fdc */
	WD2797(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(m_pia_rtc, FUNC(pia6821_device::cb1_w));
	m_fdc->drq_wr_callback().set(FUNC(bbc_cumana68k_device::drq_w));
	m_fdc->set_force_ready(true);

	FLOPPY_CONNECTOR(config, m_floppy0, os9_floppies, "525qd", floppy_image_device::default_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, os9_floppies, "525qd", floppy_image_device::default_floppy_formats).enable_sound(true);

	/* scsi */
	//MCFG_DEVICE_ADD("sasi", SCSI_PORT, 0)
	//MCFG_SCSI_DATA_INPUT_BUFFER("scsi_data_in")
	//MCFG_SCSI_SEL_HANDLER(WRITELINE(*this, bbc_cumana68k_device, write_sasi_sel))
	//MCFG_SCSI_IO_HANDLER(WRITELINE(*this, bbc_cumana68k_device, write_sasi_io))
	//MCFG_SCSI_BSY_HANDLER(WRITELINE(*this, bbc_cumana68k_device, write_sasi_bsy))
	//MCFG_SCSI_CD_HANDLER(WRITELINE(*this, bbc_cumana68k_device, write_sasi_cd))
	//MCFG_SCSI_MSG_HANDLER(WRITELINE(*this, bbc_cumana68k_device, write_sasi_msg))
	//MCFG_SCSI_REQ_HANDLER(WRITELINE(*this, bbc_cumana68k_device, write_sasi_req))
	//MCFG_SCSI_ACK_HANDLER(WRITELINE("pia_sasi", pia6821_device, ca2_w))
	//MCFG_SCSIDEV_ADD("sasi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_0)

	//MCFG_SCSI_OUTPUT_LATCH_ADD("scsi_data_out", "sasi")
	//MCFG_DEVICE_ADD("scsi_data_in", INPUT_BUFFER, 0)

	PIA6821(config, m_pia_sasi, 0);
	m_pia_sasi->readpa_handler().set(FUNC(bbc_cumana68k_device::pia_sasi_pa_r));
	m_pia_sasi->readpb_handler().set(FUNC(bbc_cumana68k_device::pia_sasi_pb_r));
	m_pia_sasi->readcb1_handler().set(FUNC(bbc_cumana68k_device::pia_sasi_cb1_r));
	//m_pia_sasi->cb2_handler().set(m_scsibus, FUNC(scsi_port_device::write_rst));
	m_pia_sasi->irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_pia_sasi->irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia_rtc, 0);
	m_pia_rtc->readpa_handler().set(m_rtc, FUNC(mc146818_device::read));
	m_pia_rtc->writepa_handler().set(m_rtc, FUNC(mc146818_device::write));
	m_pia_rtc->writepb_handler().set(FUNC(bbc_cumana68k_device::pia_rtc_pb_w));
	//m_pia_rtc->cb2_handler().set(m_scsibus, FUNC(scsi_port_device::write_rst));
	m_pia_rtc->irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	m_pia_rtc->irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(FUNC(bbc_cumana68k_device::intrq_w));

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
	, m_maincpu(*this, ":maincpu")
	, m_m68008(*this, "m68008")
	, m_pia_sasi(*this, "pia_sasi")
	, m_pia_rtc(*this, "pia_rtc")
	, m_irqs(*this, "irqs")
	, m_rtc(*this, "rtc")
	, m_fdc(*this, "wd2797")
	, m_floppy0(*this, "wd2797:0")
	, m_floppy1(*this, "wd2797:1")
	//, m_scsibus(*this, "sasi")
	//, m_scsi_data_out(*this, "scsi_data_out")
	//, m_scsi_data_in(*this, "scsi_data_in")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_cumana68k_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_cumana68k_device::device_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_readwrite_handler(0xfc40, 0xfc43, read8sm_delegate(*m_pia_sasi, FUNC(pia6821_device::read)), write8sm_delegate(*m_pia_sasi, FUNC(pia6821_device::write)));
	program.install_readwrite_handler(0xfc44, 0xfc47, read8sm_delegate(*m_pia_rtc, FUNC(pia6821_device::read)), write8sm_delegate(*m_pia_rtc, FUNC(pia6821_device::write)));
	program.install_readwrite_handler(0xfc48, 0xfc4b, read8sm_delegate(*m_fdc, FUNC(wd2797_device::read)), write8sm_delegate(*m_fdc, FUNC(wd2797_device::write)));
}

void bbc_cumana68k_device::device_reset_after_children()
{
	m_bEnabled = false;

	// Pull up resistor R12 makes CB2 high when it's configured as an input, so set CB2 as output and high to compensate
	m_pia_rtc->write(0x03, 0x3c);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER(bbc_cumana68k_device::reset68008_w)
{
	if (!state) m_bEnabled = true;

	m_m68008->set_input_line(INPUT_LINE_HALT, state);
	m_m68008->set_input_line(INPUT_LINE_RESET, state);
}


READ8_MEMBER(bbc_cumana68k_device::dma_r)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	if (m_bEnabled)
	{
		if (offset < 0x1000)
			return program.read_byte(offset + 0x2000);
		else if (offset > 0x1fff && offset < 0x3000)
			return program.read_byte(offset - 0x2000);
		else
			return program.read_byte(offset);
	}
	return 0xff;
}

WRITE8_MEMBER(bbc_cumana68k_device::dma_w)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	if (m_bEnabled)
	{
		if (offset < 0x1000)
			program.write_byte(offset + 0x2000, data);
		else if (offset > 0x1fff && offset < 0x3000)
			program.write_byte(offset - 0x2000, data);
		else
			program.write_byte(offset, data);
	}
}


WRITE8_MEMBER(bbc_cumana68k_device::pia_rtc_pb_w)
{
	floppy_image_device *floppy = nullptr;
	logerror("pia1_pb_w %02x\n", data);
	// bit 0, 1: drive select
	switch (data & 0x03)
	{
	case 0: floppy = m_floppy0->get_device(); break;
	case 1: floppy = m_floppy1->get_device(); break;
	}
	m_fdc->set_floppy(floppy);

	// bit 2: motor enable
	if (floppy)
		floppy->mon_w(BIT(data, 2));

	// bit 3: density
	m_fdc->dden_w(BIT(data, 3));

	// bit 4: enable precomp
}

WRITE_LINE_MEMBER(bbc_cumana68k_device::intrq_w)
{
	//m_m68008->set_input_line_and_vector(M68K_IRQ_2, state ? ASSERT_LINE : CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR);
	m_m68008->set_input_line(M68K_IRQ_2, state);
	m_slot->irq_w(state);
}

WRITE_LINE_MEMBER(bbc_cumana68k_device::drq_w)
{
	m_pia_sasi_pb &= ~0x40;
	m_pia_sasi_pb |= state << 6;

	//m_m68008->set_input_line(M68K_IRQ_7, state);
	m_slot->nmi_w(!state);
}


IRQ_CALLBACK_MEMBER(bbc_cumana68k_device::irq_callback)
{
	//m_m68008->set_input_line(M68K_IRQ_2, CLEAR_LINE);
	// see https://github.com/mamedev/mame/commit/e39802db90860d6628c35265f7c78ce3534f5a53
	return 0;  //M68K_INT_ACK_AUTOVECTOR;
}


WRITE_LINE_MEMBER(bbc_cumana68k_device::write_sasi_sel)
{
	m_pia_sasi_pb &= ~0x02;
	m_pia_sasi_pb |= state << 1;
}

WRITE_LINE_MEMBER(bbc_cumana68k_device::write_sasi_io)
{
	m_pia_sasi_pb &= ~0x04;
	m_pia_sasi_pb |= state << 2;
}

WRITE_LINE_MEMBER(bbc_cumana68k_device::write_sasi_bsy)
{
	m_pia_sasi_pb &= ~0x08;
	m_pia_sasi_pb |= state << 3;
}

WRITE_LINE_MEMBER(bbc_cumana68k_device::write_sasi_cd)
{
	m_pia_sasi_pb &= ~0x10;
	m_pia_sasi_pb |= state << 4;
}

WRITE_LINE_MEMBER(bbc_cumana68k_device::write_sasi_msg)
{
	m_pia_sasi_pb &= ~0x20;
	m_pia_sasi_pb |= state << 5;
}

WRITE_LINE_MEMBER(bbc_cumana68k_device::write_sasi_req)
{
	m_pia_sasi_pb &= ~0x40;
	m_pia_sasi_pb |= state << 6;

	m_pia_sasi->ca1_w(state);
}


READ8_MEMBER(bbc_cumana68k_device::pia_sasi_pa_r)
{
	return 0; // m_scsi_data_in->read();
}

WRITE8_MEMBER(bbc_cumana68k_device::pia_sasi_pa_w)
{
	//m_scsi_data_out->write(data);
}


READ8_MEMBER(bbc_cumana68k_device::pia_sasi_pb_r)
{
	return m_pia_sasi_pb;
}

READ_LINE_MEMBER(bbc_cumana68k_device::pia_sasi_cb1_r)
{
	// CB1 is tied to +5V
	return 1;
}
