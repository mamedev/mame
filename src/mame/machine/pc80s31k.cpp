// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Olivier Galibert
/**************************************************************************************************

	NEC PC-80S31(K) Mini Disk Unit

	Z80 + μPD765 + PPI that connects in cross with an host PPI
	
	"Mini" as compared to the PC-8881 8-inch floppy I/F

	|--------------------------------------|
	|      P1 P2 P3         X1             |
	|                                      |
	| CN2                                  |
    |                 B6102C022            |
    |      FD1 FD2              (**)       |
    |       |   |                          |
    | CN1 -------       D765AC  D2364EC (*)|
	|                                   (*)|
	|      D8255AC-2  B6101C017 D780C-1    |
	|                                      |
	|--------------------------------------|
	
	PCB is marked PC-80S31KFDC NEC-14T
	P1, P2, P3 are power supplies (+5V, +12V, -5V for P1, lacks -5V for the other two)
	μPD2364EC is at position IC13
	(*) are μPD41416C
	(**) marked as JP, unpopulated x 3;
	μPB6101/2 are bipolar TTL gate arrays, presumably
	Cannot read marking of X1 (8 MHz?)

	TODO:
	- Support for the "2HD/2DD/2D variant" (the "K" board?)
	  The μPD2364 is a 8192 size, matching the later PC88 FDC BIOS we have.
	  There's also PC-80S32, which I don't know how much related to this is;
	- PC=0x7dd reads from FDC bit 3 in ST3 (twosid_r fn), 
	  expecting a bit 3 high for all the PC8001 games otherwise keeps looping and eventually dies.
	  Are those incorrectly identified as 2DD? Hacked to work for now;
	- set_input_line_vector fn doesn't work properly when issued from a device_reset, 
	  we currently just implement the irq_callback instead;
	- Bus option;
	- Cascade mode, i.e. the CN2 connector used to accept a second disk unit for drive 2 & 3;

**************************************************************************************************/

#include "emu.h"
#include "pc80s31k.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


// device type definition
DEFINE_DEVICE_TYPE(PC80S31K, pc80s31k_device, "pc80s31k", "NEC PC-80S31(K) Mini Disk Unit I/F")


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
//	map(0xf6, 0xf6).nopw(); // 
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

	I8255A(config, m_ppi_host);
	m_ppi_host->in_pa_callback().set(m_ppi_fdc, FUNC(i8255_device::pb_r));
	m_ppi_host->in_pb_callback().set(m_ppi_fdc, FUNC(i8255_device::pa_r));
	m_ppi_host->in_pc_callback().set(FUNC(pc80s31k_device::host_portc_r));
	m_ppi_host->out_pc_callback().set(FUNC(pc80s31k_device::host_portc_w));

	// 8255AC-2
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
