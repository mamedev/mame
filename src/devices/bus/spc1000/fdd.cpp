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

READ8_MEMBER(spc1000_fdd_exp_device::i8255_c_r)
{
	return m_i8255_0_pc >> 4;
}

WRITE8_MEMBER(spc1000_fdd_exp_device::i8255_b_w)
{
	m_i8255_portb = data;
}

WRITE8_MEMBER(spc1000_fdd_exp_device::i8255_c_w)
{
	m_i8255_1_pc = data;
}

//-------------------------------------------------
//  fdc interrupt
//-------------------------------------------------

READ8_MEMBER( spc1000_fdd_exp_device::tc_r )
{
	logerror("%s: tc_r\n", space.machine().describe_context());

	// toggle tc on read
	m_fdc->tc_w(true);
	m_timer_tc->adjust(attotime::zero);

	return 0xff;
}

WRITE8_MEMBER( spc1000_fdd_exp_device::control_w )
{
	logerror("%s: control_w(%02x)\n", space.machine().describe_context(), data);

	// bit 0, motor on signal
	if (m_fd0)
		m_fd0->mon_w(!BIT(data, 0));
	if (m_fd1)
		m_fd1->mon_w(!BIT(data, 0));
}

static ADDRESS_MAP_START( sd725_mem, AS_PROGRAM, 8, spc1000_fdd_exp_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sd725_io, AS_IO, 8, spc1000_fdd_exp_device )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf8, 0xf8) AM_READWRITE(tc_r, control_w) // (R) Terminal Count Port (W) Motor Control Port
	AM_RANGE(0xfa, 0xfb) AM_DEVICE("upd765", upd765a_device, map)
	AM_RANGE(0xfc, 0xff) AM_DEVREADWRITE("d8255_master", i8255_device, read, write)
ADDRESS_MAP_END

static SLOT_INTERFACE_START( sd725_floppies )
	SLOT_INTERFACE("sd320", EPSON_SD_320)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT(spc1000_fdd)

	/* sub CPU(5 inch floppy drive) */
	MCFG_CPU_ADD("fdccpu", Z80, XTAL_4MHz)       /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(sd725_mem)
	MCFG_CPU_IO_MAP(sd725_io)

	MCFG_DEVICE_ADD("d8255_master", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(DEVREAD8("d8255_master", i8255_device, pb_r))
	MCFG_I8255_IN_PORTB_CB(DEVREAD8("d8255_master", i8255_device, pa_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(spc1000_fdd_exp_device, i8255_b_w))
	MCFG_I8255_IN_PORTC_CB(READ8(spc1000_fdd_exp_device, i8255_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(spc1000_fdd_exp_device, i8255_c_w))

	// floppy disk controller
	MCFG_UPD765A_ADD("upd765", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE("fdccpu", INPUT_LINE_IRQ0))

	// floppy drives
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", sd725_floppies, "sd320", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", sd725_floppies, "sd320", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

//-------------------------------------------------
//  device_mconfig_additions
//-------------------------------------------------

machine_config_constructor spc1000_fdd_exp_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( spc1000_fdd );
}

ROM_START( spc1000_fdd )
	ROM_REGION(0x10000, "fdccpu", 0)
	ROM_LOAD("sd725a.bin", 0x0000, 0x1000, CRC(96ac2eb8) SHA1(8e9d8f63a7fb87af417e95603e71cf537a6e83f1))
ROM_END

//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const rom_entry *spc1000_fdd_exp_device::device_rom_region() const
{
	return ROM_NAME( spc1000_fdd );
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type SPC1000_FDD_EXP = &device_creator<spc1000_fdd_exp_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spc1000_fdd_exp_device - constructor
//-------------------------------------------------

spc1000_fdd_exp_device::spc1000_fdd_exp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, SPC1000_FDD_EXP, "SPC1000 FDD expansion", tag, owner, clock, "spc1000_fdd_exp", __FILE__),
		device_spc1000_card_interface(mconfig, *this),
		m_cpu(*this, "fdccpu"),
		m_fdc(*this, "upd765"),
		m_pio(*this, "d8255_master"), m_fd0(nullptr), m_fd1(nullptr), m_timer_tc(nullptr), m_i8255_0_pc(0), m_i8255_1_pc(0), m_i8255_portb(0)
	{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spc1000_fdd_exp_device::device_start()
{
	m_timer_tc = timer_alloc(TIMER_TC);
	m_timer_tc->adjust(attotime::never);

	m_fd0 = subdevice<floppy_connector>("upd765:0")->get_device();
	m_fd1 = subdevice<floppy_connector>("upd765:1")->get_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spc1000_fdd_exp_device::device_reset()
{
	m_cpu->set_input_line_vector(0, 0);

	// enable rom (is this really needed? it does not seem necessary for FDD to work)
	m_cpu->space(AS_PROGRAM).install_rom(0x0000, 0x0fff, 0, 0x2000, device().machine().root_device().memregion("fdccpu")->base());
}

void spc1000_fdd_exp_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_TC:
			m_fdc->tc_w(false);
			break;
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(spc1000_fdd_exp_device::read)
{
	// this should be m_pio->read on the whole 0x00-0x03 range?
	if (offset >= 3)
		return 0xff;
	else
	{
		UINT8 data = 0;
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

WRITE8_MEMBER(spc1000_fdd_exp_device::write)
{
	// this should be m_pio->write on the whole 0x00-0x03 range?
	if (offset < 3)
	{
		switch (offset)
		{
			case 0:
				m_pio->write(space, 1, data);
				break;
			case 2:
				m_i8255_0_pc = data;
				break;
		}
	}
}
