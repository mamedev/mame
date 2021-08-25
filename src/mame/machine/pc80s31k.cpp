// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Olivier Galibert
/**************************************************************************************************

	NEC PC-80S31K floppy interface board

	Z80 + μPD765 + dual PPI in cross configuration

	TODO:
	- PC=0x7dd reads from FDC bit 3 in ST3 (twosid_r fn), 
	  expecting a bit 3 high for all
      the PC8001 games otherwise keeps looping and eventually dies.
	  Are those incorrectly identified as 2HD? Hacked to work for now;
	- set_input_line_vector fn doesn't work properly when called from a device_reset, 
	  we currently just implement the irq_callback instead;
    - Implement derived HW for 2HD BIOS;
	- Bus option;

**************************************************************************************************/

#include "emu.h"
#include "pc80s31k.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


// device type definition
DEFINE_DEVICE_TYPE(PC80S31K, pc80s31k_device, "pc80s31k", "NEC PC-80S31K I/F")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************


//-------------------------------------------------
//  pc80s31k_device - constructor
//-------------------------------------------------


pc80s31k_device::pc80s31k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC80S31K, tag, owner, clock)
	, m_fdc_cpu(*this, "fdc_cpu")
	, m_fdc_rom(*this, "fdc_rom")
	, m_ppi_host(*this, "ppi_host")
	, m_ppi_fdc(*this, "ppi_fdc")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0U)
{
}

ROM_START( pc80s31k )
	ROM_REGION( 0x2000, "fdc_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "disk.rom", 0x0000, 0x0800, CRC(2158d307) SHA1(bb7103a0818850a039c67ff666a31ce49a8d516f) )
	ROM_FILL( 0x7df, 1, 0x00 )
	ROM_FILL( 0x7e0, 1, 0x00 )
ROM_END

const tiny_rom_entry *pc80s31k_device::device_rom_region() const
{
	return ROM_NAME( pc80s31k );
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void pc80s31k_device::host_map(address_map &map)
{
	map(0, 3).rw(m_ppi_host, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void pc80s31k_device::fdc_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("fdc_rom", 0);
	map(0x4000, 0x7fff).ram();
}

void pc80s31k_device::fdc_io(address_map &map)
{
	map.global_mask(0xff);
//	map(0xf0, 0xf0).w(FUNC(pc8801_state::fdc_irq_vector_w)); // Interrupt Opcode Port
//	map(0xf4, 0xf4).w(FUNC(pc8801_state::fdc_drive_mode_w)); // Drive mode, 2d, 2dd, 2hd
	map(0xf7, 0xf7).nopw(); // printer port output
	map(0xf8, 0xf8).rw(FUNC(pc80s31k_device::terminal_count_r), FUNC(pc80s31k_device::motor_control_w));
	map(0xfa, 0xfb).m(m_fdc, FUNC(upd765a_device::map));
	map(0xfc, 0xff).rw(m_ppi_fdc, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

static void pc88_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

IRQ_CALLBACK_MEMBER(pc80s31k_device::irq_cb)
{
	return 0;
}

void pc80s31k_device::device_add_mconfig(machine_config &config)
{
	constexpr XTAL fdc_xtal = XTAL(4'000'000);
	Z80(config, m_fdc_cpu, fdc_xtal);
	m_fdc_cpu->set_addrmap(AS_PROGRAM, &pc80s31k_device::fdc_map);
	m_fdc_cpu->set_addrmap(AS_IO, &pc80s31k_device::fdc_io);
	m_fdc_cpu->set_irq_acknowledge_callback(FUNC(pc80s31k_device::irq_cb));

	UPD765A(config, m_fdc, XTAL(4'000'000), true, true);
	m_fdc->intrq_wr_callback().set_inputline(m_fdc_cpu, INPUT_LINE_IRQ0);

	for (auto &floppy : m_floppy)
	{
		FLOPPY_CONNECTOR(config, floppy, pc88_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats);
		floppy->enable_sound(true);
	}

	// TODO: 8255 or 8255A?
	I8255A(config, m_ppi_host);
	m_ppi_host->in_pa_callback().set(m_ppi_fdc, FUNC(i8255_device::pb_r));
	m_ppi_host->in_pb_callback().set(m_ppi_fdc, FUNC(i8255_device::pa_r));
	m_ppi_host->in_pc_callback().set(FUNC(pc80s31k_device::host_portc_r));
	m_ppi_host->out_pc_callback().set(FUNC(pc80s31k_device::host_portc_w));

	I8255A(config, m_ppi_fdc);
	m_ppi_fdc->in_pa_callback().set(m_ppi_host, FUNC(i8255_device::pb_r));
	m_ppi_fdc->in_pb_callback().set(m_ppi_host, FUNC(i8255_device::pa_r));
	m_ppi_fdc->in_pc_callback().set(FUNC(pc80s31k_device::fdc_portc_r));
	m_ppi_fdc->out_pc_callback().set(FUNC(pc80s31k_device::fdc_portc_w));
}

//-------------------------------------------------
//  device_timer - device-specific timers
//-------------------------------------------------

void pc80s31k_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	assert(id == 0);

	m_fdc->tc_w(false);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc80s31k_device::device_start()
{
	m_tc_zero_timer = timer_alloc(0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------


void pc80s31k_device::device_reset()
{
	m_floppy[0]->get_device()->set_rpm(300);
	m_floppy[1]->get_device()->set_rpm(300);
	m_fdc->set_rate(250000);
	
	m_fdc_cpu->set_input_line_vector(0, 0); // Z80

	m_tc_zero_timer->adjust(attotime::never);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

u8 pc80s31k_device::host_portc_r()
{
	machine().scheduler().synchronize();
	return m_fdc_latch >> 4;
}

void pc80s31k_device::host_portc_w(u8 data)
{
	machine().scheduler().synchronize();
	m_host_latch = data;
}

u8 pc80s31k_device::fdc_portc_r()
{
	machine().scheduler().synchronize();
	return m_host_latch >> 4; 
}

void pc80s31k_device::fdc_portc_w(u8 data)
{
	machine().scheduler().synchronize();
	m_fdc_latch = data;
}

u8 pc80s31k_device::terminal_count_r()
{
	if (!machine().side_effects_disabled())
	{
		m_fdc->tc_w(true);
		m_tc_zero_timer->adjust(attotime::from_usec(50));
	}
	// value is meaningless
	return 0xff;
}

void pc80s31k_device::motor_control_w(uint8_t data)
{
	m_floppy[0]->get_device()->mon_w(!(data & 1));
	m_floppy[1]->get_device()->mon_w(!(data & 2));
	
	// TODO: according to docs a value of 0x07 enables precompensation to tracks 0-19, 0xf enables it on 20-39
}
