// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

/*
 * Besta CP31 board, possibly a customized version of Force CPU30.
 *
 * Supported by SysV R3 "Bestix" port and also by Linux port,
 * see https://github.com/shattered/linux-m68k
 *
 * Chips:
 *
 * 68030 @ 25 MHz - CPU
 * 68882 @ 25 MHz - FPU
 * 68561 - MPCC (serial port)
 * 68230 - PI/T (parallel interface and timer)
 * 68153 - BIM (interrupt router)
 * 62421 - RTC
 *
 * To do:
 *
 * - pass functional test
 * - boot to multiuser (SysV and Linux) (requires ISCSI-1)
 *
 */

#include "emu.h"

#include "vme_cp31.h"


//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)
#define LOG_INT     (1U <<  2)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP | LOG_INT)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)


#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


#define CLOCK50 XTAL(50'000'000) /* HCJ */
#define CLOCK40 XTAL(40'000'000) /* HCJ */
#define CLOCK32 XTAL(32'000'000) /* HCJ */

#define RS232P1_TAG "rs232p1"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VME_CP31, vme_cp31_card_device, "cp31", "Besta CP31 CPU board")

void vme_cp31_card_device::cp31_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x001fffff).ram().share("dram"); // local bus DRAM, 4 MB
//  map(0x08010000, 0x08011fff).ram();               // unknown -- accessed by cp31dssp
//  map(0xfca03500, 0xfca0350f).unmaprw();           // ISCSI-1 board on VME bus
	map(0xff000000, 0xff03ffff).rom().region("user1", 0);
	map(0xff040000, 0xff07ffff).ram();               // onboard SRAM
	map(0xff800000, 0xff80001f).rw(m_mpcc, FUNC(mpcc68561_device::read), FUNC(mpcc68561_device::write));
	map(0xff800200, 0xff8003ff).rw(m_pit2, FUNC(pit68230_device::read), FUNC(pit68230_device::write));
	map(0xff800800, 0xff80080f).rw(m_bim, FUNC(bim68153_device::read), FUNC(bim68153_device::write)).umask32(0xff00ff00);
	map(0xff800a00, 0xff800a1f).rw(m_rtc, FUNC(rtc62421_device::read), FUNC(rtc62421_device::write));
	map(0xff800c00, 0xff800dff).rw(m_pit1, FUNC(pit68230_device::read), FUNC(pit68230_device::write));
//  map(0xff800400, 0xff800xxx) // TIC? -- shows up in cp31dssp log
//  map(0xff800e00, 0xff800xxx) // PIT3?
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

// FIXME unverified
static INPUT_PORTS_START(cp31)
	PORT_START("SA1")
	PORT_DIPNAME(0x01, 0x00, "Clear macros on reset")
	PORT_DIPSETTING(0x00, "On")
	PORT_DIPSETTING(0x01, "Off")
	PORT_DIPNAME(0x02, 0x00, "VME bus width")
	PORT_DIPSETTING(0x00, "16 bits")
	PORT_DIPSETTING(0x02, "32 bits")
	PORT_DIPNAME(0x0c, 0x0c, "Boot into...")
	PORT_DIPSETTING(0x00, "UNIX")
	PORT_DIPSETTING(0x0c, "Monitor")
	PORT_DIPNAME(0x10, 0x10, "Console port setup")
	PORT_DIPSETTING(0x00, "Custom")
	PORT_DIPSETTING(0x10, "Standard")
	PORT_DIPNAME(0x20, 0x20, "Console port bits")
	PORT_DIPSETTING(0x00, "7N1")
	PORT_DIPSETTING(0x20, "8N2")
	PORT_DIPNAME(0xc0, 0xc0, "Console port speed")
	PORT_DIPSETTING(0xc0, "9600")
	PORT_DIPSETTING(0x80, "4800")
	PORT_DIPSETTING(0x40, "2400")
	PORT_DIPSETTING(0x00, "1200")
INPUT_PORTS_END

ROM_START(cp31)
	ROM_REGION32_BE(0x40000, "user1", ROMREGION_ERASEFF)

	ROM_DEFAULT_BIOS("cp31dssp")
	ROM_SYSTEM_BIOS(0, "cp31dbg", "CP31 Debug")
	ROMX_LOAD( "cp31dbgboot.27c512",  0x0000, 0x10000, CRC(9bf057de) SHA1(b13cb16042e4c6ca63ae26058a78259c0849d0b6), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "cp31dssp", "CP31 DSSP")
	ROMX_LOAD( "cp31dsspboot.27c512", 0x0000, 0x10000, CRC(607a0a55) SHA1(c257a88672ab39d2f3fad681d22e062182b0236d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "cp31os9", "CP31 OS9")
	ROMX_LOAD( "cp31os9.27c512",      0x0000, 0x10000, CRC(607a0a55) SHA1(c257a88672ab39d2f3fad681d22e062182b0236d), ROM_BIOS(2))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *vme_cp31_card_device::device_rom_region() const
{
	return ROM_NAME(cp31);
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vme_cp31_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cp31);
}


void vme_cp31_card_device::cpu_space_map(address_map &map)
{
	map(0xfffffff2, 0xffffffff).lr16(NAME([this](offs_t offset) -> u16 { return m_bim->iack(offset+1); }));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vme_cp31_card_device::device_add_mconfig(machine_config &config)
{
	M68030(config, m_maincpu, CLOCK50 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &vme_cp31_card_device::cp31_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_cp31_card_device::cpu_space_map);

	MC68153(config, m_bim, CLOCK32 / 8);
	m_bim->out_int_callback().set(FUNC(vme_cp31_card_device::bim_irq_callback));

	MPCC68561(config, m_mpcc, 1392000, 0, 0); // FIXME: XTAL unknown
	m_mpcc->out_txd_cb().set(RS232P1_TAG, FUNC(rs232_port_device::write_txd));
	m_mpcc->out_dtr_cb().set(RS232P1_TAG, FUNC(rs232_port_device::write_dtr));
	m_mpcc->out_rts_cb().set(RS232P1_TAG, FUNC(rs232_port_device::write_rts));
	m_mpcc->out_int_cb().set(m_bim, FUNC(bim68153_device::int1_w));

	rs232_port_device &rs232p1(RS232_PORT(config, RS232P1_TAG, default_rs232_devices, "terminal"));
	rs232p1.rxd_handler().set(m_mpcc, FUNC(mpcc68561_device::write_rx));
	rs232p1.cts_handler().set(m_mpcc, FUNC(mpcc68561_device::cts_w));
	rs232p1.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	RTC62421(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->out_int_handler().set(m_pit1, FUNC(pit68230_device::h2_w));

	// H1 is SYSFAIL and H3 is ACFAIL
	PIT68230(config, m_pit1, 8064000); // via c33_txt
	m_pit1->pa_in_callback().set_ioport("SA1");
	m_pit1->pb_out_callback().set(*this, FUNC(vme_cp31_card_device::pit1_pb_w));
	m_pit1->pc_in_callback().set(*this, FUNC(vme_cp31_card_device::pit1_pc_r));
//  m_pit1->pc_out_callback().set(*this, FUNC(vme_cp31_card_device::pit1_pc_w));
	m_pit1->timer_irq_callback().set(m_bim, FUNC(bim68153_device::int2_w));
	m_pit1->port_irq_callback().set(m_bim, FUNC(bim68153_device::int3_w));

	PIT68230(config, m_pit2, 8064000); // via c33_txt
	m_pit2->port_irq_callback().set(m_pit1, FUNC(pit68230_device::h4_w));
}

uint32_t vme_cp31_card_device::trap_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled()) set_bus_error((offset << 2), true, mem_mask);

	return 0xffffffff;
}

WRITE_LINE_MEMBER(vme_cp31_card_device::bim_irq_callback)
{
	LOGINT("%s(%02x)\n", FUNCNAME, state);

	bim_irq_state = state;
	bim_irq_level = m_bim->get_irq_level();
	LOGINT(" - BIM irq level %s\n", bim_irq_level == CLEAR_LINE ? "Cleared" : "Asserted");
	update_irq_to_maincpu();
}

// TODO: IRQ masking
void vme_cp31_card_device::update_irq_to_maincpu()
{
	LOGINT("%s()\n", FUNCNAME);
	LOGINT(" - bim_irq_level: %02x\n", bim_irq_level);
	LOGINT(" - bim_irq_state: %02x\n", bim_irq_state);
	switch (bim_irq_level & 0x07)
	{
	case 1: m_maincpu->set_input_line(M68K_IRQ_1, bim_irq_state); break;
	case 2: m_maincpu->set_input_line(M68K_IRQ_2, bim_irq_state); break;
	case 3: m_maincpu->set_input_line(M68K_IRQ_3, bim_irq_state); break;
	case 4: m_maincpu->set_input_line(M68K_IRQ_4, bim_irq_state); break;
	case 5: m_maincpu->set_input_line(M68K_IRQ_5, bim_irq_state); break;
	case 6: m_maincpu->set_input_line(M68K_IRQ_6, bim_irq_state); break;
	case 7: m_maincpu->set_input_line(M68K_IRQ_7, bim_irq_state); break;
	default: logerror("Programmatic error in %s, please report\n", FUNCNAME);
	}
}


void vme_cp31_card_device::trap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s(%08x,%08X)\n", FUNCNAME, offset << 2, data);
	if (!machine().side_effects_disabled()) set_bus_error((offset << 2), false, mem_mask);
}

// PC4 - IRQ7 state, PC6 - FPCP SENSE (0 = FPCP installed)
uint8_t vme_cp31_card_device::pit1_pc_r()
{
	uint8_t data = 0;

	LOG("%s(%02X)\n", FUNCNAME, data);

	return data;
}

// IRQ masking
void vme_cp31_card_device::pit1_pb_w(uint8_t data)
{
	LOG("%s(%02X)\n", FUNCNAME, data);
}

void vme_cp31_card_device::set_bus_error(uint32_t address, bool rw, uint32_t mem_mask)
{
	if (m_bus_error)
	{
		return;
	}
	LOG("bus error at %08x & %08x (%s)\n", address, mem_mask, rw ? "read" : "write");
	if (!ACCESSING_BITS_16_31)
	{
		address++;
	}
	m_bus_error = true;
	m_maincpu->set_buserror_details(address, rw, m_maincpu->get_fc());
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_bus_error_timer->adjust(m_maincpu->cycles_to_attotime(16)); // let rmw cycles complete
}

vme_cp31_card_device::vme_cp31_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_bim(*this, "bim")
	, m_mpcc(*this, "mpcc")
	, m_rtc(*this, "rtc")
	, m_pit1(*this, "pit1")
	, m_pit2(*this, "pit2")
	, m_p_ram(*this, "dram")
	, m_sysrom(*this, "user1")
{
	m_slot = 1;
}

//

vme_cp31_card_device::vme_cp31_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_cp31_card_device(mconfig, VME_CP31, tag, owner, clock)
{
}

void vme_cp31_card_device::device_start()
{
	m_bus_error_timer = timer_alloc(0);
}

void vme_cp31_card_device::device_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_rom(0x00000000, 0x00000007, m_sysrom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xff000000, 0xff007fff,
			"rom_shadow_r",
			[this] (offs_t offset, u32 &data, u32 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x00000000, 0x00000007, m_p_ram);
				}
			},
			&m_rom_shadow_tap);
}

void vme_cp31_card_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	m_bus_error = false;
}
