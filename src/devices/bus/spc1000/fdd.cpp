// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    SPC-1000 FDD unit

***************************************************************************/

#include "emu.h"
#include "fdd.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

uint8_t spc1000_fdd_exp_device::i8255_c_r()
{
	return m_i8255_0_pc >> 4;
}

void spc1000_fdd_exp_device::i8255_b_w(uint8_t data)
{
	m_i8255_portb = data;
}

void spc1000_fdd_exp_device::i8255_c_w(uint8_t data)
{
	m_i8255_1_pc = data;
}

//-------------------------------------------------
//  fdc interrupt
//-------------------------------------------------

uint8_t spc1000_fdd_exp_device::tc_r()
{
	if (!machine().side_effects_disabled())
	{
		logerror("%s: tc_r\n", machine().describe_context());

		// toggle tc on read
		m_fdc->tc_w(true);
		m_timer_tc->adjust(attotime::zero);
	}

	return 0xff;
}

void spc1000_fdd_exp_device::control_w(uint8_t data)
{
	logerror("%s: control_w(%02x)\n", machine().describe_context(), data);

	// bit 0, motor on signal
	for (auto &fd : m_fd)
	{
		floppy_image_device *img = fd->get_device();
		if (img)
			img->mon_w(!BIT(data, 0));
	}
}

void spc1000_fdd_exp_device::sd725_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom().region("fdccpu", 0);
	map(0x4000, 0x7fff).ram(); // 16K dynamic RAM (2x TMS4416-15NL)
}

void spc1000_fdd_exp_device::sd725_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf8, 0xf8).rw(FUNC(spc1000_fdd_exp_device::tc_r), FUNC(spc1000_fdd_exp_device::control_w)); // (R) Terminal Count Port (W) Motor Control Port
	map(0xfa, 0xfb).m("upd765", FUNC(upd765a_device::map));
	map(0xfc, 0xff).rw("d8255_master", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

static void sd725_floppies(device_slot_interface &device)
{
	device.option_add("sd320", EPSON_SD_320);
}

//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void spc1000_fdd_exp_device::device_add_mconfig(machine_config &config)
{
	// Z80A sub CPU (5 inch floppy drive)
	Z80(config, m_cpu, 8_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &spc1000_fdd_exp_device::sd725_mem);
	m_cpu->set_addrmap(AS_IO, &spc1000_fdd_exp_device::sd725_io);
	m_cpu->set_irq_acknowledge_callback(NAME([](device_t &, int) { return 0xcf; })); // vector to 0008 in IM 0

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(m_ppi, FUNC(i8255_device::pb_r));
	m_ppi->in_pb_callback().set(m_ppi, FUNC(i8255_device::pa_r));
	m_ppi->out_pb_callback().set(FUNC(spc1000_fdd_exp_device::i8255_b_w));
	m_ppi->in_pc_callback().set(FUNC(spc1000_fdd_exp_device::i8255_c_r));
	m_ppi->out_pc_callback().set(FUNC(spc1000_fdd_exp_device::i8255_c_w));

	// floppy disk controller
	UPD765A(config, m_fdc, 8_MHz_XTAL / 2, true, true);
	m_fdc->intrq_wr_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);

	// floppy drives
	FLOPPY_CONNECTOR(config, "upd765:0", sd725_floppies, "sd320", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", sd725_floppies, "sd320", floppy_image_device::default_mfm_floppy_formats);
}

ROM_START( spc1000_fdd )
	ROM_REGION(0x1000, "fdccpu", 0)
	ROM_LOAD("sd725a.bin", 0x0000, 0x1000, CRC(96ac2eb8) SHA1(8e9d8f63a7fb87af417e95603e71cf537a6e83f1))
ROM_END

//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const tiny_rom_entry *spc1000_fdd_exp_device::device_rom_region() const
{
	return ROM_NAME( spc1000_fdd );
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SPC1000_FDD_EXP, spc1000_fdd_exp_device, "spc1000_fdd_exp", "SPC1000 FDD expansion")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spc1000_fdd_exp_device - constructor
//-------------------------------------------------

spc1000_fdd_exp_device::spc1000_fdd_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SPC1000_FDD_EXP, tag, owner, clock),
	device_spc1000_card_interface(mconfig, *this),
	m_cpu(*this, "fdccpu"),
	m_fdc(*this, "upd765"),
	m_ppi(*this, "d8255_master"),
	m_fd(*this, "upd765:%u", 0U),
	m_timer_tc(nullptr), m_i8255_0_pc(0), m_i8255_1_pc(0), m_i8255_portb(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spc1000_fdd_exp_device::device_start()
{
	m_timer_tc = timer_alloc(FUNC(spc1000_fdd_exp_device::tc_off), this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spc1000_fdd_exp_device::device_reset()
{
}

TIMER_CALLBACK_MEMBER(spc1000_fdd_exp_device::tc_off)
{
	m_fdc->tc_w(false);
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

uint8_t spc1000_fdd_exp_device::read(offs_t offset)
{
	// this should be m_ppi->read on the whole 0x00-0x03 range?
	if (offset >= 3)
		return 0xff;
	else
	{
		uint8_t data = 0;
		switch (offset)
		{
			case 1:
				data = m_i8255_portb;
				break;
			case 2:
				data = m_i8255_1_pc >> 4;
				break;
		}
		return data;
	}
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void spc1000_fdd_exp_device::write(offs_t offset, uint8_t data)
{
	// this should be m_ppi->write on the whole 0x00-0x03 range?
	if (offset < 3)
	{
		switch (offset)
		{
			case 0:
				m_ppi->write(1, data);
				break;
			case 2:
				m_i8255_0_pc = data;
				break;
		}
	}
}
