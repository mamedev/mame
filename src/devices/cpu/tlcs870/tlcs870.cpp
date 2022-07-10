// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************************************************

    Toshiba TLCS-870 Series MCUs

    The TLCS-870/X expands on this instruction set using the same base encoding.

    The TLCS-870/C appears to have a completely different encoding.

*************************************************************************************************************/



#include "emu.h"
#include "tlcs870.h"
#include "tlcs870d.h"

//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TMP87PH40AN, tmp87ph40an_device, "tmp87ph40an", "Toshiba TMP87PH40AN")

void tlcs870_device::tmp87ph40an_mem(address_map &map)
{
	map(0x0000, 0x0000).rw(FUNC(tlcs870_device::port0_r), FUNC(tlcs870_device::port0_w));
	map(0x0001, 0x0001).rw(FUNC(tlcs870_device::port1_r), FUNC(tlcs870_device::port1_w));
	map(0x0002, 0x0002).rw(FUNC(tlcs870_device::port2_r), FUNC(tlcs870_device::port2_w));
	map(0x0003, 0x0003).rw(FUNC(tlcs870_device::port3_r), FUNC(tlcs870_device::port3_w));
	map(0x0004, 0x0004).rw(FUNC(tlcs870_device::port4_r), FUNC(tlcs870_device::port4_w));
	map(0x0005, 0x0005).rw(FUNC(tlcs870_device::port5_r), FUNC(tlcs870_device::port5_w));
	map(0x0006, 0x0006).rw(FUNC(tlcs870_device::port6_r), FUNC(tlcs870_device::port6_w));
	map(0x0007, 0x0007).rw(FUNC(tlcs870_device::port7_r), FUNC(tlcs870_device::port7_w));
	// 0x8 reserved
	// 0x9 reserved
	map(0x000a, 0x000a).w(FUNC(tlcs870_device::p0cr_w)); // Port 0 I/O control
	map(0x000b, 0x000b).w(FUNC(tlcs870_device::p1cr_w)); // Port 1 I/O control
	map(0x000c, 0x000c).w(FUNC(tlcs870_device::p6cr_w)); // Port 6 I/O control
	map(0x000d, 0x000d).w(FUNC(tlcs870_device::p7cr_w)); // Port 7 I/O control

	map(0x000e, 0x000e).rw(FUNC(tlcs870_device::adccr_r), FUNC(tlcs870_device::adccr_w)); // A/D converter control
	map(0x000f, 0x000f).r(FUNC(tlcs870_device::adcdr_r)); // A/D converter result

	map(0x0010, 0x0010).w(FUNC(tlcs870_device::treg1a_l_w)); // Timer register 1A
	map(0x0011, 0x0011).w(FUNC(tlcs870_device::treg1a_h_w)); //
	map(0x0012, 0x0012).rw(FUNC(tlcs870_device::treg1b_l_r), FUNC(tlcs870_device::treg1b_l_w)); // Timer register 1B
	map(0x0013, 0x0013).rw(FUNC(tlcs870_device::treg1b_h_r), FUNC(tlcs870_device::treg1b_h_w)); //
	map(0x0014, 0x0014).w(FUNC(tlcs870_device::tc1cr_w)); // TC1 control
	map(0x0015, 0x0015).w(FUNC(tlcs870_device::tc2cr_w)); // TC2 control
	map(0x0016, 0x0016).w(FUNC(tlcs870_device::treg2_l_w)); // Timer register 2
	map(0x0017, 0x0017).w(FUNC(tlcs870_device::treg2_h_w)); //
	map(0x0018, 0x0018).rw(FUNC(tlcs870_device::treg3a_r), FUNC(tlcs870_device::treg3a_w)); // Timer register 3A
	map(0x0019, 0x0019).r(FUNC(tlcs870_device::treg3b_r)); // Timer register 3B
	map(0x001a, 0x001a).w(FUNC(tlcs870_device::tc3cr_w)); // TC3 control
	map(0x001b, 0x001b).w(FUNC(tlcs870_device::treg4_w)); // Timer register 4
	map(0x001c, 0x001c).w(FUNC(tlcs870_device::tc4cr_w)); // TC4 control
	// 0x1d reserved
	// 0x1e reserved
	// 0x1f reserved
	map(0x0020, 0x0020).rw(FUNC(tlcs870_device::sio1sr_r), FUNC(tlcs870_device::sio1cr1_w)); // SIO1 status / SIO1 control
	map(0x0021, 0x0021).w(FUNC(tlcs870_device::sio1cr2_w)); // SIO1 control
	map(0x0022, 0x0022).rw(FUNC(tlcs870_device::sio2sr_r), FUNC(tlcs870_device::sio2cr1_w)); // SIO2 status / SIO2 control
	map(0x0023, 0x0023).w(FUNC(tlcs870_device::sio2cr2_w)); // SIO2 control
	// 0x24 reserved
	// 0x25 reserved
	// 0x26 reserved
	// 0x27 reserved
	// 0x28 reserved
	// 0x29 reserved
	// 0x2a reserved
	// 0x2b reserved
	// 0x2c reserved
	// 0x2d reserved
	// 0x2e reserved
	// 0x2f reserved

	// 0x30 reserved
	// 0x31 reserved
	// 0x32 reserved
	// 0x33 reserved
	map(0x0034, 0x0034).w(FUNC(tlcs870_device::wdtcr1_w)); // WDT control
	map(0x0035, 0x0035).w(FUNC(tlcs870_device::wdtcr2_w)); //

	map(0x0036, 0x0036).rw(FUNC(tlcs870_device::tbtcr_r), FUNC(tlcs870_device::tbtcr_w)); // TBT / TG / DVO control

	map(0x0037, 0x0037).rw(FUNC(tlcs870_device::eintcr_r), FUNC(tlcs870_device::eintcr_w)); // External interrupt control

	map(0x0038, 0x0038).rw(FUNC(tlcs870_device::syscr1_r), FUNC(tlcs870_device::syscr1_w)); // System Control
	map(0x0039, 0x0039).rw(FUNC(tlcs870_device::syscr2_r), FUNC(tlcs870_device::syscr2_w)); //

	map(0x003a, 0x003a).rw(FUNC(tlcs870_device::eir_l_r), FUNC(tlcs870_device::eir_l_w)); // Interrupt enable register
	map(0x003b, 0x003b).rw(FUNC(tlcs870_device::eir_h_r), FUNC(tlcs870_device::eir_h_w)); //

	map(0x003c, 0x003c).rw(FUNC(tlcs870_device::il_l_r), FUNC(tlcs870_device::il_l_w)); // Interrupt latch
	map(0x003d, 0x003d).rw(FUNC(tlcs870_device::il_h_r), FUNC(tlcs870_device::il_h_w)); //

	// 0x3e reserved
	map(0x003f, 0x003f).rw(FUNC(tlcs870_device::psw_r), FUNC(tlcs870_device::rbs_w)); // Program status word / Register bank selector

	map(0x0040, 0x023f).ram().share("intram"); // register banks + internal RAM, not, code execution NOT allowed here (fetches FF and causes SWI)
	map(0x0f80, 0x0fef).ram();              // DBR (0f80 - 0fef = reserved)
	map(0x0ff0, 0x0fff).ram().share("dbr"); // DBR 0ff0-0ff7 = SIO1 buffer, 0ff8 - 0fff = SIO2 buffer)
	map(0xc000, 0xffff).rom();
}


tlcs870_device::tlcs870_device(const machine_config &mconfig, device_type optype, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program_map)
	: cpu_device(mconfig, optype, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, program_map)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_intram(*this, "intram")
	, m_dbr(*this, "dbr")
	, m_port_in_cb(*this)
	, m_port_out_cb(*this)
	, m_port_analog_in_cb(*this)
	, m_serial_out_cb(*this)
{
}


tmp87ph40an_device::tmp87ph40an_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tlcs870_device(mconfig, TMP87PH40AN, tag, owner, clock, address_map_constructor(FUNC(tmp87ph40an_device::tmp87ph40an_mem), this))
{
}

device_memory_interface::space_config_vector tlcs870_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

bool tlcs870_device::stream_arg(std::ostream &stream, uint32_t pc, const char *pre, const uint16_t mode, const uint16_t r, const uint16_t rb)
{
	return false;
}

// NOT using templates here because there are subtle differences in the port behavior (the ports are multi-purpose) that still need implementing
uint8_t tlcs870_device::port0_r()
{
	// need to use P0CR (0x000a) to control direction

	if (m_read_input_port)
		return m_port_in_cb[0]();
	else
		return m_port_out_latch[0];
}

uint8_t tlcs870_device::port1_r()
{
	// need to use P1CR (0x000b) to control direction

	if (m_read_input_port)
		return m_port_in_cb[1]();
	else
		return m_port_out_latch[1];
}

uint8_t tlcs870_device::port2_r() // 3-bit port
{
	if (m_read_input_port)
		return m_port_in_cb[2]() | 0xf8;
	else
		return m_port_out_latch[2];
}

uint8_t tlcs870_device::port3_r()
{
	if (m_read_input_port)
		return m_port_in_cb[3]();
	else
		return m_port_out_latch[3];
}

uint8_t tlcs870_device::port4_r()
{
	if (m_read_input_port)
		return m_port_in_cb[4]();
	else
		return m_port_out_latch[4];
}

uint8_t tlcs870_device::port5_r() // 5-bit port
{
	if (m_read_input_port)
		return m_port_in_cb[5]() | 0xe0;
	else
		return m_port_out_latch[5];
}

uint8_t tlcs870_device::port6_r() // doubles up as analog?
{
	// need to use P6CR (0x000c) to control direction

	if (m_read_input_port)
		return m_port_in_cb[6]();
	else
		return m_port_out_latch[6];
}

uint8_t tlcs870_device::port7_r()
{
	// need to use P7CR (0x000d) to control direction

	if (m_read_input_port)
		return m_port_in_cb[7]();
	else
		return m_port_out_latch[7];
}

void tlcs870_device::port0_w(uint8_t data)
{
	m_port_out_latch[0] = data;
	m_port_out_cb[0](data);
}

void tlcs870_device::port1_w(uint8_t data)
{
	m_port_out_latch[1] = data;
	m_port_out_cb[1](data);
}

void tlcs870_device::port2_w(uint8_t data)
{
	m_port_out_latch[2] = data;
	m_port_out_cb[2](data);
}

void tlcs870_device::port3_w(uint8_t data)
{
	m_port_out_latch[3] = data;
	m_port_out_cb[3](data);
}

void tlcs870_device::port4_w(uint8_t data)
{
	m_port_out_latch[4] = data;
	m_port_out_cb[4](data);
}

void tlcs870_device::port5_w(uint8_t data)
{
	m_port_out_latch[5] = data;
	m_port_out_cb[5](data);
}

void tlcs870_device::port6_w(uint8_t data)
{
	m_port_out_latch[6] = data;
	m_port_out_cb[6](data);
}

void tlcs870_device::port7_w(uint8_t data)
{
	m_port_out_latch[7] = data;
	m_port_out_cb[7](data);
}

void tlcs870_device::p0cr_w(uint8_t data)
{
	m_port0_cr = data;
}

void tlcs870_device::p1cr_w(uint8_t data)
{
	m_port1_cr = data;
}

void tlcs870_device::p6cr_w(uint8_t data)
{
	m_port6_cr = data;
}

void tlcs870_device::p7cr_w(uint8_t data)
{
	m_port7_cr = data;
}

// Timer emulation

// 16-Bit Timer / Counter 2 (TC1)

TIMER_CALLBACK_MEMBER(tlcs870_device::tc1_cb)
{

}

void tlcs870_device::tc1cr_w(uint8_t data)
{
	m_TC1CR = data;

	LOG("%s m_TC1CR (16-bit TC1 Timer Control Register) bits set to\n", machine().describe_context());
	LOG("%d: TFF1 (Timer F/F1 control for PPG mode)\n",   (m_TC1CR & 0x80) ? 1 : 0);
	LOG("%d: SCAP1/MCAP1/METT1/MPPG1\n",                  (m_TC1CR & 0x40) ? 1 : 0);
	LOG("%d: TC1S-1 (TC1 Start Control)\n",               (m_TC1CR & 0x20) ? 1 : 0);
	LOG("%d: TC1S-0 (TC1 Start Control)\n",               (m_TC1CR & 0x10) ? 1 : 0);
	LOG("%d: TC1CK-1 (TC1 Source Clock select)\n",        (m_TC1CR & 0x08) ? 1 : 0);
	LOG("%d: TC1CK-0 (TC1 Source Clock select)\n",        (m_TC1CR & 0x04) ? 1 : 0);
	LOG("%d: TC1M-1 (TC1 Mode Select)\n",                 (m_TC1CR & 0x02) ? 1 : 0);
	LOG("%d: TC1M-0 (TC1 Mode Select)\n",                 (m_TC1CR & 0x01) ? 1 : 0);
}

void tlcs870_device::treg1a_l_w(uint8_t data)
{
	m_TREG1A = (m_TREG1A & 0xff00) | data;
}

void tlcs870_device::treg1a_h_w(uint8_t data)
{
	m_TREG1A = (m_TREG1A & 0x00ff) | (data << 8);
}

void tlcs870_device::treg1b_l_w(uint8_t data)
{
	m_TREG1B = (m_TREG1B & 0xff00) | data;
}

void tlcs870_device::treg1b_h_w(uint8_t data)
{
	m_TREG1B = (m_TREG1B & 0x00ff) | (data << 8);
}

uint8_t tlcs870_device::treg1b_l_r()
{
	return m_TREG1B & 0xff;
}

uint8_t tlcs870_device::treg1b_h_r()
{
	return (m_TREG1B >>8) & 0xff;
}

// 16-Bit Timer / Counter 2 (TC2)

TIMER_CALLBACK_MEMBER(tlcs870_device::tc2_cb)
{
	m_IL |= 1 << (15-TLCS870_IRQ_INTTC2);
	tc2_reload();
}

void tlcs870_device::tc2_reload()
{
	m_tcx_timer[1]->adjust(cycles_to_attotime(1500)); // TODO: use real value
}

void tlcs870_device::tc2_cancel()
{
	m_tcx_timer[1]->adjust(attotime::never);
}

void tlcs870_device::tc2cr_w(uint8_t data)
{
	if (data & 0x20)
	{
		if (!(m_TC2CR & 0x20))
		{
			tc2_reload();
		}
	}
	else
	{
		tc2_cancel();
	}

	m_TC2CR = data;

	LOG("%s m_TC2CR (16-bit TC2 Timer Control Register) bits set to\n", machine().describe_context());
	LOG("%d: (invalid)\n",                         (m_TC2CR & 0x80) ? 1 : 0);
	LOG("%d: (invalid)\n",                         (m_TC2CR & 0x40) ? 1 : 0);
	LOG("%d: TC2S (TC2 Start Control)\n",          (m_TC2CR & 0x20) ? 1 : 0);
	LOG("%d: TC2CK-2 (TC2 Source Clock select)\n", (m_TC2CR & 0x10) ? 1 : 0);
	LOG("%d: TC2CK-1 (TC2 Source Clock select)\n", (m_TC2CR & 0x08) ? 1 : 0);
	LOG("%d: TC2CK-0 (TC2 Source Clock select)\n", (m_TC2CR & 0x04) ? 1 : 0);
	LOG("%d: (invalid)\n",                         (m_TC2CR & 0x02) ? 1 : 0);
	LOG("%d: TC2M (TC2 Mode Select)\n",            (m_TC2CR & 0x01) ? 1 : 0);
}

void tlcs870_device::treg2_l_w(uint8_t data)
{
	m_TREG2 = (m_TREG2 & 0xff00) | data;
}

void tlcs870_device::treg2_h_w(uint8_t data)
{
	m_TREG2 = (m_TREG2 & 0x00ff) | (data << 8);
}

// 8-Bit Timer / Counter 3 (TC3)

TIMER_CALLBACK_MEMBER(tlcs870_device::tc3_cb)
{

}

void tlcs870_device::tc3cr_w(uint8_t data)
{
	m_TC3CR = data;

	LOG("%s m_TC3CR (8-bit TC3 Timer Control Register) bits set to\n", machine().describe_context());
	LOG("%d: (invalid)\n",                         (m_TC3CR & 0x80) ? 1 : 0);
	LOG("%d: SCAP (Software Capture Control)\n",   (m_TC3CR & 0x40) ? 1 : 0);
	LOG("%d: (invalid)\n",                         (m_TC3CR & 0x20) ? 1 : 0);
	LOG("%d: TC3S (TC3 Start Control)\n",          (m_TC3CR & 0x10) ? 1 : 0);
	LOG("%d: TC3CK-1 (TC3 Source Clock select)\n", (m_TC3CR & 0x08) ? 1 : 0);
	LOG("%d: TC3CK-0 (TC3 Source Clock select)\n", (m_TC3CR & 0x04) ? 1 : 0);
	LOG("%d: (invalid)\n",                         (m_TC3CR & 0x02) ? 1 : 0);
	LOG("%d: TC3M (TC3 Mode Select)\n",            (m_TC3CR & 0x01) ? 1 : 0);
}

void tlcs870_device::treg3a_w(uint8_t data)
{
	m_TREG3A = data;
}

uint8_t tlcs870_device::treg3a_r()
{
	return m_TREG3A;
}

uint8_t tlcs870_device::treg3b_r()
{
	return m_TREG3B;
}

// 8-Bit Timer / Counter 3 (TC4)

TIMER_CALLBACK_MEMBER(tlcs870_device::tc4_cb)
{

}

void tlcs870_device::tc4cr_w(uint8_t data)
{
	m_TC4CR = data;

	LOG("%s m_TC4CR (8-bit TC4 Timer Control Register) bits set to\n", machine().describe_context());
	LOG("%d: TFF4-1 (Timer F/F 4 Control)\n",      (m_TC4CR & 0x80) ? 1 : 0);
	LOG("%d: TFF4-0 (Timer F/F 4 Control)\n",      (m_TC4CR & 0x40) ? 1 : 0);
	LOG("%d: (invalid)\n",                         (m_TC4CR & 0x20) ? 1 : 0);
	LOG("%d: TC4S (TC4 Start Control)\n",          (m_TC4CR & 0x10) ? 1 : 0);
	LOG("%d: TC4CK-1 (TC4 Source Clock select)\n", (m_TC4CR & 0x08) ? 1 : 0);
	LOG("%d: TC4CK-0 (TC4 Source Clock select)\n", (m_TC4CR & 0x04) ? 1 : 0);
	LOG("%d: TC4M-1 (TC4 Mode Select)\n",          (m_TC4CR & 0x02) ? 1 : 0);
	LOG("%d: TC4M-0 (TC4 Mode Select)\n",          (m_TC4CR & 0x01) ? 1 : 0);
}

void tlcs870_device::treg4_w(uint8_t data)
{
	m_TREG4 = data;
}

// Time Base Timer

// this is used with TLCS870_IRQ_INTTBT (FFF2 INTTBT) (not used by hng64)
// the divider output makes use of PORT1 bit 3, which must be properly configured
void tlcs870_device::tbtcr_w(uint8_t data)
{
	m_TBTCR = data;

	LOG("%s m_TBTCR (Time Base Timer) bits set to\n", machine().describe_context());
	LOG("%d: DV0EN (Divider Output Enable)\n",                  (m_TBTCR & 0x80) ? 1 : 0);
	LOG("%d: DVOCK-1 (Divide Output Frequency Selection)n",     (m_TBTCR & 0x40) ? 1 : 0);
	LOG("%d: DVOCK-0 (Divide Output Frequency Selection)\n",    (m_TBTCR & 0x20) ? 1 : 0);
	LOG("%d: DV7CK (?)\n",                                      (m_TBTCR & 0x10) ? 1 : 0);
	LOG("%d: TBTEN (Time Base Timer Enable)\n",                 (m_TBTCR & 0x08) ? 1 : 0);
	LOG("%d: TBTCK-2 (Time Base Timer Interrupt Frequency)\n",  (m_TBTCR & 0x04) ? 1 : 0);
	LOG("%d: TBTCK-1 (Time Base Timer Interrupt Frequency)\n",  (m_TBTCR & 0x02) ? 1 : 0);
	LOG("%d: TBTCK-0 (Time Base Timer Interrupt Frequency)\n",  (m_TBTCR & 0x01) ? 1 : 0);
}

uint8_t tlcs870_device::tbtcr_r()
{
	return m_TBTCR;
}

/* SIO emulation */

// TODO: use templates for SIO1/2 ports, as they're the same except for the DBR region they use?

// Serial Port 1
void tlcs870_device::sio1cr1_w(uint8_t data)
{
	m_SIOCR1[0] = data;

	LOG("%s m_SIOCR1[0] (Serial IO Port 1 Control Register 1) bits set to\n", machine().describe_context());
	LOG("%d: SIOS1 (Start/Stop transfer)\n",       (m_SIOCR1[0] & 0x80) ? 1 : 0);
	LOG("%d: SIOINH1 (Abort/Continue transfer)\n", (m_SIOCR1[0] & 0x40) ? 1 : 0);
	LOG("%d: SIOM1-2 (Serial Mode)\n",             (m_SIOCR1[0] & 0x20) ? 1 : 0);
	LOG("%d: SIOM1-1 (Serial Mode)\n",             (m_SIOCR1[0] & 0x10) ? 1 : 0);
	LOG("%d: SIOM1-0 (Serial Mode)\n",             (m_SIOCR1[0] & 0x08) ? 1 : 0);
	LOG("%d: SCK1-2 (Serial Clock)\n",             (m_SIOCR1[0] & 0x04) ? 1 : 0);
	LOG("%d: SCK1-1 (Serial Clock)\n",             (m_SIOCR1[0] & 0x02) ? 1 : 0);
	LOG("%d: SCK1-0 (Serial Clock)\n",             (m_SIOCR1[0] & 0x01) ? 1 : 0);

	m_transfer_mode[0] = (m_SIOCR1[0] & 0x38) >> 3;
	switch (m_transfer_mode[0])
	{
	case 0x0:
		LOG("(Serial set to 8-bit transmit mode)\n");
		m_transmit_bits[0] = 8;
		m_receive_bits[0] = 0;
		break;

	case 0x2:
		LOG("(Serial set to 4-bit transmit mode)\n");
		m_transmit_bits[0] = 4;
		m_receive_bits[0] = 0;
		break;

	case 0x4:
		LOG("(Serial set to 8-bit transmit/receive mode)\n");
		m_transmit_bits[0] = 8;
		m_receive_bits[0] = 8;
		break;

	case 0x5: LOG("(Serial set to 8-bit receive mode)\n");
		m_transmit_bits[0] = 0;
		m_receive_bits[0] = 8;
		break;

	case 0x6:
		LOG("(Serial set to 4-bit receive mode)\n");
		m_transmit_bits[0] = 0;
		m_receive_bits[0] = 4;
		break;

	default:
		LOG("(Serial set to invalid mode)\n");
		m_transmit_bits[0] = 0;
		m_receive_bits[0] = 0;
		break;
	}

	if ((m_SIOCR1[0] & 0xc0) == 0x80)
	{
		// start transfer
		m_transfer_shiftpos[0] = 0;
		m_transfer_shiftreg[0] = 0;
		m_transfer_pos[0] = 0;

		m_serial_transmit_timer[0]->adjust(attotime::zero);
	}
}


void tlcs870_device::sio1cr2_w(uint8_t data)
{
	m_SIOCR2[0] = data;

	LOG("%s m_SIOCR2[0] (Serial IO Port 1 Control Register 2) bits set to\n", machine().describe_context());
	LOG("%d: (invalid)\n",                          (m_SIOCR2[0] & 0x80) ? 1 : 0);
	LOG("%d: (invalid)\n",                          (m_SIOCR2[0] & 0x40) ? 1 : 0);
	LOG("%d: (invalid)\n",                          (m_SIOCR2[0] & 0x20) ? 1 : 0);
	LOG("%d: WAIT1-1 (Wait Control\n",              (m_SIOCR2[0] & 0x10) ? 1 : 0);
	LOG("%d: WAIT1-0 (Wait Control)\n",             (m_SIOCR2[0] & 0x08) ? 1 : 0);
	LOG("%d: BUF1-2 (Number of Transfer Bytes)\n",  (m_SIOCR2[0] & 0x04) ? 1 : 0);
	LOG("%d: BUF1-1 (Number of Transfer Bytes)\n",  (m_SIOCR2[0] & 0x02) ? 1 : 0);
	LOG("%d: BUF1-0 (Number of Transfer Bytes)\n",  (m_SIOCR2[0] & 0x01) ? 1 : 0);

	m_transfer_numbytes[0] = (m_SIOCR2[0] & 0x7);
	LOG("(serial set to transfer %01x bytes)\n", m_transfer_numbytes[0]+1);

}

uint8_t tlcs870_device::sio1sr_r()
{
	/* TS-- ----

	   T = Transfer in Progress
	   S = Shift in Progress

	*/
	return 0x00;
}

TIMER_CALLBACK_MEMBER(tlcs870_device::sio0_transmit_cb)
{
	if (m_transmit_bits[0]) // TODO: handle receive cases
	{
		int finish = 0;
		if (m_transfer_shiftpos[0] == 0)
		{
			m_transfer_shiftreg[0] = m_dbr[m_transfer_pos[0]];
			LOG("transmitting byte %02x\n", m_transfer_shiftreg[0]);
		}

		int dataout = m_transfer_shiftreg[0] & 0x01;

		m_serial_out_cb[0](dataout);

		m_transfer_shiftreg[0] >>= 1;
		m_transfer_shiftpos[0]++;

		if (m_transfer_shiftpos[0] == 8)
		{
			LOG("transmitted\n");

			m_transfer_shiftpos[0] = 0;
			m_transfer_pos[0]++;

			if (m_transfer_pos[0] > m_transfer_numbytes[0])
			{
				LOG("end of transmission\n");
				m_SIOCR1[0] &= ~0x80;
				// set interrupt latch
				m_IL |= 1 << (15 - TLCS870_IRQ_INTSIO1);
				finish = 1;
			}
		}

		if (!finish)
			m_serial_transmit_timer[0]->adjust(cycles_to_attotime(1000)); // TODO: use real speed
	}
}

// Serial Port 2
void tlcs870_device::sio2cr1_w(uint8_t data)
{
	m_SIOCR1[1] = data;

	LOG("%s m_SIOCR1[1] (Serial IO Port 2 Control Register 1) bits set to\n", machine().describe_context());
	LOG("%d: SIOS2 (Start/Stop transfer)\n",       (m_SIOCR1[1] & 0x80) ? 1 : 0);
	LOG("%d: SIOINH2 (Abort/Continue transfer)\n", (m_SIOCR1[1] & 0x40) ? 1 : 0);
	LOG("%d: SIOM2-2 (Serial Mode)\n",             (m_SIOCR1[1] & 0x20) ? 1 : 0);
	LOG("%d: SIOM2-1 (Serial Mode)\n",             (m_SIOCR1[1] & 0x10) ? 1 : 0);
	LOG("%d: SIOM2-0 (Serial Mode)\n",             (m_SIOCR1[1] & 0x08) ? 1 : 0);
	LOG("%d: SCK2-2 (Serial Clock)\n",             (m_SIOCR1[1] & 0x04) ? 1 : 0);
	LOG("%d: SCK2-1 (Serial Clock)\n",             (m_SIOCR1[1] & 0x02) ? 1 : 0);
	LOG("%d: SCK2-0 (Serial Clock)\n",             (m_SIOCR1[1] & 0x01) ? 1 : 0);
}

void tlcs870_device::sio2cr2_w(uint8_t data)
{
	m_SIOCR2[1] = data;

	LOG("%s m_SIOCR2[1] (Serial IO Port 2 Control Register 2) bits set to\n", machine().describe_context());
	LOG("%d: (invalid)\n",                          (m_SIOCR2[1] & 0x80) ? 1 : 0);
	LOG("%d: (invalid)\n",                          (m_SIOCR2[1] & 0x40) ? 1 : 0);
	LOG("%d: (invalid)\n",                          (m_SIOCR2[1] & 0x20) ? 1 : 0);
	LOG("%d: WAIT2-1 (Wait Control\n",              (m_SIOCR2[1] & 0x10) ? 1 : 0);
	LOG("%d: WAIT2-0 (Wait Control)\n",             (m_SIOCR2[1] & 0x08) ? 1 : 0);
	LOG("%d: BUF2-2 (Number of Transfer Bytes)\n",  (m_SIOCR2[1] & 0x04) ? 1 : 0);
	LOG("%d: BUF2-1 (Number of Transfer Bytes)\n",  (m_SIOCR2[1] & 0x02) ? 1 : 0);
	LOG("%d: BUF2-0 (Number of Transfer Bytes)\n",  (m_SIOCR2[1] & 0x01) ? 1 : 0);
}

TIMER_CALLBACK_MEMBER(tlcs870_device::sio1_transmit_cb)
{
}

uint8_t tlcs870_device::sio2sr_r()
{
	/* TS-- ----

	   T = Transfer in Progress
	   S = Shift in Progress

	*/
	return 0x00;
}

// WDT emulation (Watchdog Timer)

void tlcs870_device::wdtcr1_w(uint8_t data)
{
	m_WDTCR1 = data;

	LOG("%s m_WDTCR1 (Watchdog Timer Control 1) bits set to\n", machine().describe_context());
	LOG("%d: (invalid)\n",                                                          (m_WDTCR1 & 0x80) ? 1 : 0);
	LOG("%d: (invalid)\n",                                                          (m_WDTCR1 & 0x40) ? 1 : 0);
	LOG("%d: (invalid)\n",                                                          (m_WDTCR1 & 0x20) ? 1 : 0);
	LOG("%d: (invalid)\n",                                                          (m_WDTCR1 & 0x10) ? 1 : 0);
	LOG("%d: WDTEN (Watchdog Timer Enable, also req disable code to WDTCR2)\n",     (m_WDTCR1 & 0x08) ? 1 : 0);
	LOG("%d: WDTT-1 (Watchdog Timer Detection Time)\n",                             (m_WDTCR1 & 0x04) ? 1 : 0);
	LOG("%d: WDTT-0 (Watchdog Timer Detection Time)\n",                             (m_WDTCR1 & 0x02) ? 1 : 0);
	LOG("%d: WDTOUT (Watchdog Timer Output select, 0 = interrupt, 1 = reset out)\n",(m_WDTCR1 & 0x01) ? 1 : 0);

	// WDTOUT cannot be set to 1 by software
}

void tlcs870_device::wdtcr2_w(uint8_t data)
{
	if (data == 0x4e)
	{
		// clear watchdog counter
	}
	else if (data == 0xb1)
	{
		// disable code
		if (!(m_WDTCR1 & 0x08))
		{
			LOG("%s wdtcr2_w - Watchdog disabled\n", machine().describe_context());
		}
	}
}

// Misc

// not used by hng64
void tlcs870_device::syscr1_w(uint8_t data)
{
	m_SYSCR1 = data;

	LOG("%s m_SYSCR1 (System Control Register 1) bits set to\n", machine().describe_context());
	LOG("%d: STOP (STOP mode start)\n",                          (m_SYSCR1 & 0x80) ? 1 : 0);
	LOG("%d: RELM (release method for STOP, 0 edge, 1 level)\n", (m_SYSCR1 & 0x40) ? 1 : 0);
	LOG("%d: RETM (return mode after STOP, 0 normal, 1 slow)\n", (m_SYSCR1 & 0x20) ? 1 : 0);
	LOG("%d: OUTEN (port output control during STOP)\n",         (m_SYSCR1 & 0x10) ? 1 : 0);
	LOG("%d: WUT-1 (warm up time at STOP release)\n",            (m_SYSCR1 & 0x08) ? 1 : 0);
	LOG("%d: WUT-0 (warm up time at STOP release)\n",            (m_SYSCR1 & 0x04) ? 1 : 0);
	LOG("%d: (invalid)\n",                                       (m_SYSCR1 & 0x02) ? 1 : 0);
	LOG("%d: (invalid)\n",                                       (m_SYSCR1 & 0x01) ? 1 : 0);
}

void tlcs870_device::syscr2_w(uint8_t data)
{
	m_SYSCR2 = data;

	LOG("%s m_SYSCR2 (System Control Register 2) bits set to\n", machine().describe_context());
	LOG("%d: XEN (High Frequency Oscillator control)\n",         (m_SYSCR2 & 0x80) ? 1 : 0);
	LOG("%d: XTEN (Low Frequency Oscillator control)\n",         (m_SYSCR2 & 0x40) ? 1 : 0);
	LOG("%d: SYSCK (system clock select 0 high, 1 low)\n",       (m_SYSCR2 & 0x20) ? 1 : 0);
	LOG("%d: IDLE (IDLE mode start)\n",                          (m_SYSCR2 & 0x10) ? 1 : 0); // hng64 sets this in case of ram test failures
	LOG("%d: (invalid)\n",                                       (m_SYSCR2 & 0x08) ? 1 : 0);
	LOG("%d: (invalid)\n",                                       (m_SYSCR2 & 0x04) ? 1 : 0);
	LOG("%d: (invalid)\n",                                       (m_SYSCR2 & 0x02) ? 1 : 0);
	LOG("%d: (invalid)\n",                                       (m_SYSCR2 & 0x01) ? 1 : 0);
}

uint8_t tlcs870_device::syscr1_r()
{
	return m_SYSCR1; // low 2 bits are 'undefined'
}

uint8_t tlcs870_device::syscr2_r()
{
	return m_SYSCR2 | 0x0f; // low bits always read as 1
}

// RBS / PSW direct access

void tlcs870_device::rbs_w(uint8_t data)
{
	// upper bits of PSW (status flags) cannot be written, only the register bank
	m_RBS = data & 0x0f;
}

uint8_t tlcs870_device::psw_r()
{
	// outside of checking the results of opcodes that  use it directly (DAA / DAS) this is the only way to read / check the 'half' flag
	return get_PSW();
}

// ADC emulation

uint8_t tlcs870_device::adcdr_r()
{
	return m_ADCDR;
}

/*

 ADCCR register bits

 es-apppp

 e = end flag (1 = done, data available in ADCDR, 0 = not requested / not finished) (r/o)
 s = start flag (1 = request data be processed and put in ADCDR)

 a = analog input enable (won't function at all with this disabled?)

 p = analog port to use (upper bit is 'reserved', so 8 ports)

 current emulation assumes this is instant

 bits in P6CR (0x0c) should also be set to '1' to enable analog input on the port as the
 same pins are otherwise used as a normal input port, not this multiplexed ADC

 */

uint8_t tlcs870_device::adccr_r()
{
	return m_ADCCR | 0x80; // return with 'finished' bit set
}

void tlcs870_device::adccr_w(uint8_t data)
{
	m_ADCCR = data;

	if (data & 0x40)
	{
		m_ADCDR = m_port_analog_in_cb[data&0x07]();
	}
}


uint8_t tlcs870_device::eintcr_r()
{
	return 0x00;
}

void tlcs870_device::eintcr_w(uint8_t data)
{
	m_EINTCR = data;

	LOG("%s m_EINTCR (External Interrupt Control) bits set to\n", machine().describe_context());
	LOG("%d: INT1NC (Interrupt noise reject)\n", (m_EINTCR & 0x80) ? 1 : 0);
	LOG("%d: INT0EN (Interrupt 0 enable)\n",     (m_EINTCR & 0x40) ? 1 : 0);
	LOG("%d: (invalid)\n",                       (m_EINTCR & 0x20) ? 1 : 0);
	LOG("%d: INT4ES (edge select)\n",            (m_EINTCR & 0x10) ? 1 : 0);
	LOG("%d: INT3ES (edge select)\n",            (m_EINTCR & 0x08) ? 1 : 0);
	LOG("%d: INT2ES (edge select)\n",            (m_EINTCR & 0x04) ? 1 : 0);
	LOG("%d: INT1ES (edge select)\n",            (m_EINTCR & 0x02) ? 1 : 0);
	LOG("%d: (invalid)\n",                       (m_EINTCR & 0x01) ? 1 : 0);

	/* For edge select register 0 = rising edge, 1 = falling edge

	   For INT0EN if 1 then Port 1 bit 0 is used for IRQ0, otherwise it is used for a port bit
	   if it is used as an IRQ pin then it should also be configured as an input in P1CR
	*/
}

uint8_t tlcs870_device::eir_l_r()
{
	return m_EIR & 0xff;
}

uint8_t tlcs870_device::eir_h_r()
{
	return (m_EIR >> 8) & 0xff;
}

void tlcs870_device::eir_l_w(uint8_t data)
{
	m_EIR = (m_EIR & 0xff00) | data;

	LOG("%s m_EIR(LSB) (Interrupt Enable) bits set to\n", machine().describe_context());
	LOG("%d: EF7 (External Interrupt 2)\n",      (m_EIR & 0x0080) ? 1 : 0);
	LOG("%d: EF6 (Time Base Timer Interrupt)\n", (m_EIR & 0x0040) ? 1 : 0);
	LOG("%d: EF5 (External Interrupt 1)\n",      (m_EIR & 0x0020) ? 1 : 0);
	LOG("%d: EF4 (16-bit TC1 Interrupt)\n",      (m_EIR & 0x0010) ? 1 : 0);
	LOG("%d: (invalid)\n",                       (m_EIR & 0x0008) ? 1 : 0); // can't be External Int 0 (bit in different register is used)
	LOG("%d: (invalid)\n",                       (m_EIR & 0x0004) ? 1 : 0); // can't be Watchdog interrupt (non-maskable)
	LOG("%d: (invalid)\n",                       (m_EIR & 0x0002) ? 1 : 0); // can't be Software interrupt (non-maskable)
	LOG("%d: IMF\n",                             (m_EIR & 0x0001) ? 1 : 0); // can't be Reset interrupt (non-maskable)
}

void tlcs870_device::eir_h_w(uint8_t data)
{
	m_EIR = (m_EIR & 0x00ff) | (data << 8);

	LOG("%s m_EIR(MSB) (Interrupt Enable) bits set to\n", machine().describe_context());
	LOG("%d: EF15 (External Interrupt 5)\n",          (m_EIR & 0x8000) ? 1 : 0);
	LOG("%d: EF14 (16-bit TC2 Interrupt)\n",          (m_EIR & 0x4000) ? 1 : 0);
	LOG("%d: EF13 (Serial Interface 2 Interrupt)\n",  (m_EIR & 0x2000) ? 1 : 0);
	LOG("%d: EF12 (External Interrupt 4)\n",          (m_EIR & 0x1000) ? 1 : 0);
	LOG("%d: EF11 (External Interrupt 3)\n",          (m_EIR & 0x0800) ? 1 : 0);
	LOG("%d: EF10 (8-bit TC4 Interrupt)\n",           (m_EIR & 0x0400) ? 1 : 0);
	LOG("%d: EF9  (Serial Interface 1 Interrupt)\n",  (m_EIR & 0x0200) ? 1 : 0);
	LOG("%d: EF8  (8-bit TC3 Interrupt)\n",           (m_EIR & 0x0100) ? 1 : 0);
}

/*
    the READ/WRITE/MODIFY operations cannot be used to clear interrupt latches

    also you can't set a latch by writing '1' to it, only clear a latch
    by writing 0 to it

*/
uint8_t tlcs870_device::il_l_r()
{
	return m_IL & 0xff;
}

uint8_t tlcs870_device::il_h_r()
{
	return (m_IL >> 8) & 0xff;
}

void tlcs870_device::il_l_w(uint8_t data)
{
	// probably not this logic
	m_IL = (m_IL & 0xff00) | data;
}

void tlcs870_device::il_h_w(uint8_t data)
{
	// probably not this logic
	m_IL = (m_EIR & 0x00ff) | (data << 8);
}

void tlcs870_device::execute_set_input(int inputnum, int state)
{
	int32_t irqline = -1;

	switch (inputnum)
	{
	case INPUT_LINE_IRQ5:
		irqline = 15;
		break;

	case INPUT_LINE_IRQ4:
		irqline = 12;
		break;

	case INPUT_LINE_IRQ3:
		irqline = 11;
		break;

	case INPUT_LINE_IRQ2:
		irqline = 7;
		break;

	case INPUT_LINE_IRQ1:
		irqline = 5;
		break;

	case INPUT_LINE_IRQ0:
		irqline = 3;
		break;
	}

	if (irqline != -1)
	{
		set_irq_line(irqline, state);
	}
}

void tlcs870_device::set_irq_line(int irqline, int state)
{
	//LOG("set_irq_line %d %d\n", irqline, state);

	if (!(m_irqstate & (1 << irqline)))
	{
		// rising edge
		if (state)
		{
			m_irqstate |= 1<<irqline;

			// TODO: add checks to see if interrupt pin(s) are configured, and if they're in rising edge mode
			m_IL |= 1<<irqline;
		}
	}
	else
	{
		if (!state)
		{
			m_irqstate &= ~(1<<irqline);
		}
	}
}

void tlcs870_device::check_interrupts()
{
	// priority 0-2 are non-maskable, and should have already been processed before we get here
	for (int priority = 0; priority <= 15; priority++)
	{
		if (priority >= 3) // only priorities 0,1,2 are non-maskable
		{
			if (!(m_EIR & 1))
			{
				// maskable interrupts are disabled, bail
				continue;
			}

			int is_latched = m_IL & (1<<priority);

			if (is_latched)
			{
				take_interrupt(priority);
				return;
			}
		}
		else
		{
			// TODO: handle non-maskable here
		}
	}
}

void tlcs870_device::take_interrupt(int priority)
{
	m_IL &= ~(1<<priority);
	m_EIR &= ~1;
	LOG("taken interrupt %d\n", priority);

	uint16_t vector = RM16(0xffe0 + ((15-priority)*2));

	WM8(m_sp.d, get_PSW());
	WM16(m_sp.d - 2, m_addr);
	m_sp.d -= 3;

	m_pc.d = vector;
	LOG("setting PC to %04x\n", m_pc.d);

}

void tlcs870_device::execute_run()
{
	while (m_icount > 0)
	{
		check_interrupts();

		m_prvpc.d = m_pc.d;
		debugger_instruction_hook(m_pc.d);

		m_addr = m_pc.d;
		m_tmppc = m_addr; // used for jumps etc.
		m_cycles = 0;
		m_read_input_port = 1; // some operations force the output latches to read from the memory mapped ports, not input ports
		decode();
		m_pc.d = m_addr;

		if (m_cycles)
		{
			//m_icount -= m_cycles * 4; // 1 machine cycle = 4 clock cycles? (unclear, execution seems far too slow even for the ram test this way)
			m_icount -= m_cycles;
		}
		else
		{
			fatalerror("m_cycles == 0 after PC %04x\n", m_tmppc);
		}
	};
}

void tlcs870_device::device_reset()
{
	m_pc.d = RM16(0xfffe);

	m_RBS = 0x00;
	m_EIR = 0x0000;
	m_IL = 0x0000;
	m_EINTCR = 0x00;
	m_ADCCR = 0x00;
	m_ADCDR = 0x00;
	m_SYSCR1 = 0x00;
	m_SYSCR2 = 0x80; // | 0x40, can order parts with low frequency oscillator enabled by default too (although default state is always to use high one?)
	m_TBTCR = 0x00;

	m_TREG1A = 0x1234; // not initialized?
	m_TREG1B = 0x4321; // not initialized?
	m_TC1CR = 0x00;

	m_TREG2 = 0x2301; // not initialized?
	m_TC2CR = 0x00;

	m_TREG3A = 0x10; // not initialized?
	m_TREG3B = 0x32; // not initialized?
	m_TC3CR = 0x00;

	m_TREG4 = 0x30; // not initialized?
	m_TC3CR = 0x00;

	m_SIOCR1[0] = 0x00;
	m_SIOCR1[1] = 0x00;

	m_SIOCR2[0] = 0x00;
	m_SIOCR2[1] = 0x00;

	m_WDTCR1 = 0x09;

	m_irqstate = 0;
	m_transfer_numbytes[0] = 0;
	m_transfer_numbytes[1] = 0;
	m_transfer_mode[0] = 0;
	m_transfer_mode[1] = 0;
	m_transfer_pos[0] = 0;
	m_transfer_pos[1] = 0;
	m_transfer_shiftreg[0] = 0;
	m_transfer_shiftreg[1] = 0;
	m_transfer_shiftpos[0] = 0;
	m_transfer_shiftpos[1] = 0;


	m_port_out_latch[0] = 0x00;
	m_port_out_latch[1] = 0x00;
	m_port_out_latch[2] = 0xff;
	m_port_out_latch[3] = 0xff;
	m_port_out_latch[4] = 0xff;
	m_port_out_latch[5] = 0xff;
	m_port_out_latch[6] = 0x00;
	m_port_out_latch[7] = 0x00;

	m_port0_cr = 0xff;
	m_port1_cr = 0xff;
	m_port6_cr = 0xff;
	m_port7_cr = 0xff;
}

void tlcs870_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case DEBUGGER_REG_A:
		set_reg8(REG_A, m_debugger_temp);
		break;

	case DEBUGGER_REG_W:
		set_reg8(REG_W, m_debugger_temp);
		break;

	case DEBUGGER_REG_C:
		set_reg8(REG_C, m_debugger_temp);
		break;

	case DEBUGGER_REG_B:
		set_reg8(REG_B, m_debugger_temp);
		break;

	case DEBUGGER_REG_E:
		set_reg8(REG_E, m_debugger_temp);
		break;

	case DEBUGGER_REG_D:
		set_reg8(REG_D, m_debugger_temp);
		break;

	case DEBUGGER_REG_L:
		set_reg8(REG_L, m_debugger_temp);
		break;

	case DEBUGGER_REG_H:
		set_reg8(REG_H, m_debugger_temp);
		break;

	case DEBUGGER_REG_WA:
		set_reg16(REG_WA, m_debugger_temp);
		break;

	case DEBUGGER_REG_BC:
		set_reg16(REG_BC, m_debugger_temp);
		break;

	case DEBUGGER_REG_DE:
		set_reg16(REG_DE, m_debugger_temp);
		break;

	case DEBUGGER_REG_HL:
		set_reg16(REG_HL, m_debugger_temp);
		break;

	case DEBUGGER_REG_RB:
		m_RBS = m_debugger_temp;
		break;
	}
}


void tlcs870_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case DEBUGGER_REG_A:
		m_debugger_temp = get_reg8(REG_A);
		break;

	case DEBUGGER_REG_W:
		m_debugger_temp = get_reg8(REG_W);
		break;

	case DEBUGGER_REG_C:
		m_debugger_temp = get_reg8(REG_C);
		break;

	case DEBUGGER_REG_B:
		m_debugger_temp = get_reg8(REG_B);
		break;

	case DEBUGGER_REG_E:
		m_debugger_temp = get_reg8(REG_E);
		break;

	case DEBUGGER_REG_D:
		m_debugger_temp = get_reg8(REG_D);
		break;

	case DEBUGGER_REG_L:
		m_debugger_temp = get_reg8(REG_L);
		break;

	case DEBUGGER_REG_H:
		m_debugger_temp = get_reg8(REG_H);
		break;

	case DEBUGGER_REG_WA:
		m_debugger_temp = get_reg16(REG_WA);
		break;

	case DEBUGGER_REG_BC:
		m_debugger_temp = get_reg16(REG_BC);
		break;

	case DEBUGGER_REG_DE:
		m_debugger_temp = get_reg16(REG_DE);
		break;

	case DEBUGGER_REG_HL:
		m_debugger_temp = get_reg16(REG_HL);
		break;

	case DEBUGGER_REG_RB:
		m_debugger_temp = m_RBS;
		break;

	}
}


void tlcs870_device::device_start()
{
	//  int i, p;
	m_sp.d = 0x0000;
	m_F = 0;

	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);

	state_add(DEBUGGER_REG_A, "A", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_W, "W", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_C, "C", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_B, "B", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_E, "E", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_D, "D", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_L, "L", m_debugger_temp).callimport().callexport().formatstr("%02X");
	state_add(DEBUGGER_REG_H, "H", m_debugger_temp).callimport().callexport().formatstr("%02X");

	state_add(DEBUGGER_REG_WA, "WA", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add(DEBUGGER_REG_BC, "BC", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add(DEBUGGER_REG_DE, "DE", m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add(DEBUGGER_REG_HL, "HL", m_debugger_temp).callimport().callexport().formatstr("%04X");

	state_add(DEBUGGER_REG_RB, "RB", m_debugger_temp).callimport().callexport().formatstr("%01X");

	state_add(STATE_GENPC, "GENPC", m_pc.w.l).formatstr("%04X");
	state_add(STATE_GENPCBASE, "CURPC", m_prvpc.w.l).formatstr("%04X").noshow();
	state_add(DEBUGGER_REG_SP, "SP", m_sp.w.l).formatstr("%04X");
	state_add(STATE_GENFLAGS, "GENFLAGS", m_F).formatstr("%8s").noshow();

	set_icountptr(m_icount);

	m_port_in_cb.resolve_all_safe(0xff);
	m_port_out_cb.resolve_all_safe();
	m_port_analog_in_cb.resolve_all_safe(0xff);
	m_serial_out_cb.resolve_all_safe();

	m_serial_transmit_timer[0] = timer_alloc(FUNC(tlcs870_device::sio0_transmit_cb), this);
	m_serial_transmit_timer[1] = timer_alloc(FUNC(tlcs870_device::sio1_transmit_cb), this);

	m_tcx_timer[0] = timer_alloc(FUNC(tlcs870_device::tc1_cb), this);
	m_tcx_timer[1] = timer_alloc(FUNC(tlcs870_device::tc2_cb), this);
	m_tcx_timer[2] = timer_alloc(FUNC(tlcs870_device::tc3_cb), this);
	m_tcx_timer[3] = timer_alloc(FUNC(tlcs870_device::tc4_cb), this);
}



void tlcs870_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	int F = m_F;

	switch (entry.index())
	{

	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c",
			F & 0x80 ? 'J' : '.',
			F & 0x40 ? 'Z' : '.',
			F & 0x20 ? 'C' : '.',
			F & 0x10 ? 'H' : '.'
		);
		break;
	}

}

std::unique_ptr<util::disasm_interface> tlcs870_device::create_disassembler()
{
	return std::make_unique<tlcs870_disassembler>();
}
