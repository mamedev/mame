#include "i386ex.h"
#include "i386priv.h"

DEFINE_DEVICE_TYPE(I386EX, i386ex_device, "i386ex", "Intel I386EX")

i386ex_device::i386ex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i386_device(mconfig, I386EX, tag, owner, clock, 16, 26, 16)
	, m_386ex_io_config("io", ENDIANNESS_LITTLE, 16, 16, 0)
	, m_uart0(*this, "uart0")
	, m_uart1(*this, "uart1")
	, m_pit(*this, "pit")
	, m_pic_master(*this, "pic_master")
	, m_pic_slave(*this, "pic_slave")
{
}

void i386ex_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0x03ff0000;
	m_sreg[CS].limit    = 0x03ff;
	m_sreg[CS].flags    = 0x9b;
	m_sreg[CS].valid    = true;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0092;
	m_sreg[DS].valid = m_sreg[ES].valid = m_sreg[FS].valid = m_sreg[GS].valid = m_sreg[SS].valid =true;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;
	m_smm = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;

	m_a20_mask = ~0;

	m_cr[0] = 0x7fffffe0; // reserved bits set to 1
	m_eflags = 0;
	m_eflags_mask = 0x00037fd7;
	m_eip = 0xfff0;

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 3 (386), Model 0 (DX), Stepping 8 (D1)
	REG32(EAX) = 0;
	REG32(EDX) = (3 << 8) | (0 << 4) | (8);
	m_cpu_version = REG32(EDX);

	m_CPL = 0;

	CHANGE_PC(m_eip);

	for (int i=0; i<8; i++){
		m_CS_address[i] = 0;
		m_CS_mask[i] = 0;
	}
}

uint8_t i386ex_device::get_slave_ack(offs_t offset)
{
	if (offset==2) // IRQ = 2
		return m_pic_slave->acknowledge();

	return 0x00;
}

//':maincpu' (000F7254) =>  P3CFG = 00
//':maincpu' (000F7647) =>  TMRCFG = 00
//':maincpu' (000F75D8) =>  SIOCFG = C7

// SIOCFG: 1100 0111
// 7: 1 = SIO1-Clear-to-send = external pin RTS1# (PINCFG.0 = 1)
// 6: 1 = SIO0-Clear-to-send = external pin RTS0# (P1CFG.1 == 1 ?)
// 5: 0 = RESERVED
// 4: 0 = RESERVED
// 3: 0 = RESERVED 
// 2: 1 = SSIO-Baud-CLK-IN = SERCLK
// 1: 1 = SIO1-Baud-CLK-IN = SERCLK
// 0: 1 = SIO0-Baud-CLK-IN = SERCLK

void i386ex_device::set_clock_prescaler(offs_t offset, uint16_t data)
{
#define CLK2 48000000
//TODO: use CLK2 value from the CPU configured master clock value.
	uint32_t PSCLK = CLK2/((data & 0x1ff) + 2);
	logerror("Setting clock prescaler: PSCLK = %f MHz\n", float(PSCLK)/1000000);
}

void i386ex_device::set_pin_configuration(offs_t offset, uint16_t data)
{
	//This register selects which signals are multiplexed to the
	// multi-funcion pins of the CPU package.

	//This driver is currently hardcoded to the settings used by VMP3700, as indicated below:
	// PINCFG = 3F: 0011 1111
	// PINCFG.7: 0 = RESERVED 
	// PINCFG.6: 0 = PinMode 6: CS6#
	// PINCFG.5: 1 = PinMode 5: TMROUT2, TMRCLK2, and TMRGATE2
	// PINCFG.4: 1 = PinMode 4: CS5#
	// PINCFG.3: 1 = PinMode 3: CTS1#
	// PINCFG.2: 1 = PinMode 2: TXD1
	// PINCFG.1: 1 = PinMode 1: DTR1#
	// PINCFG.0: 1 = PinMode 0: RTS1#

	//TODO: Implement handling the signals identified below:
	logerror(
		"Setting PINCFG: %02X\n"
		"\tPM0: %s\n"
		"\tPM1: %s\n"
		"\tPM2: %s\n"
		"\tPM3: %s\n"
		"\tPM4: %s\n"
		"\tPM5: %s\n"
		"\tPM6: %s\n",
		data,
		(BIT(data, 0) ? "RTS1#" : "SSIOTX"),
		(BIT(data, 1) ? "DTR1#" : "SRXCLK"),
		(BIT(data, 2) ? "TXD1#" : "DACK1#"),
		(BIT(data, 3) ? "CTS1#" : "EOP#"),
		(BIT(data, 4) ? "CS5#" : "DACK0#"),
		(BIT(data, 5) ? "TMROUT2, TMRCLK2 and TMRGATE2" : "PEREQ, BUSY# and ERROR#"),
		(BIT(data, 6) ? "REFRESH#" : "CS6#")
	);
}

uint16_t i386ex_device::get_interrupt_configuration(offs_t offset)
{
	return m_INTCFG;
}

void i386ex_device::set_interrupt_configuration(offs_t offset, uint16_t data)
{
	//This register selects the configuration of the Interrupt Controller Unit
	//This driver is currently hardcoded to the settings used by VMP3700, as indicated below:
	//':maincpu' (000F729A) =>  INTCFG = 00

	//TODO: Implement handling the signals identified below:
	m_INTCFG = data;
	logerror(
		"Setting INTCFG: %02X\n"
		"\tCE - Cascade Enable: %s\n"
		"\tIR3 - Internal Master IR3 Connection: %s\n"
		"\tIR4 - Internal Master IR4 Connection: %s\n"
		"\tSWAP - INT6/DMAINT Connection: %s\n"
		"\tIR6 - Internal Slave IR6 Connection: %s\n"
		"\tIR4/IR5 - Internal Slave IR4 or IR5 Connection: %s\n"
		"\tIR1 - Internal Slave IR1 Connection: %s\n"
		"\tIR0 - Internal Slave IR0 Connection: %s\n",
		data,
		(BIT(data, 7) ? "1" : "0"),
		(BIT(data, 6) ? "1" : "0"),
		(BIT(data, 5) ? "1" : "0"),
		(BIT(data, 4) ? "1" : "0"),
		(BIT(data, 3) ? "1" : "0"),
		(BIT(data, 2) ? "1" : "0"),
		(BIT(data, 1) ? "1" : "0"),
		(BIT(data, 0) ? "1" : "0")
	);
}


void i386ex_device::set_dma_configuration(offs_t offset, uint16_t data)
{
	//This register selects the configuration of the DMA Controller Unit

	//TODO: Implement handling the signals identified below:
	logerror("Setting DMACFG: %02X\n", data);
}

void i386ex_device::set_timer_configuration(offs_t offset, uint16_t data)
{
	//This register selects the configuration of the Timer Controller Unit

	//TODO: Implement handling the signals identified below:
	logerror("Setting TMRCFG: %02X\n", data);
}

void i386ex_device::set_serial_io_configuration(offs_t offset, uint16_t data)
{
	//This register selects the configuration of the Serial I/O Controller Unit

	//TODO: Implement handling the signals identified below:
	logerror("Setting SIOCFG: %02X\n", data);
}


void i386ex_device::chip_select_unit_w(offs_t offset, uint16_t data)
{
	// Chip Select Unit
	// TODO: The memory map is currently hardcoded in the VMP3700 driver

	uint8 which = (offset>>2) & 0x7;
	uint8 reg = (offset) & 0x03;
	switch (reg){
		case 0: //Low Address: CSnADL (n = 0-6), UCSADL
			m_CS_address[which] = (m_CS_address[which] & 0x7fe0) | ((data >> 11) & 0x1f);
			break;
		case 1: //High Address: CSnADH (n = 0-6), UCSADH
			m_CS_address[which] = ((data & 0x3ff) << 5) | (m_CS_address[which] & 0x1f);
			break;
//		default:
//			logerror("Not implemented (CSU registers).\n");
	}
	if ((offset & 0x3) == 0x3){
		logerror(
			"CS%d# address: %08X mask: %08X\n",
			which,
			m_CS_address[which] << 11,
			(m_CS_mask[which] << 11) | 0x7ff
		);
	}
}

void i386ex_device::io_map(address_map &map)
{
//	map(0x0000, 0x001f).rw("dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0xffff);
	map(0x0020, 0x003f).rw("pic_master", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0xffff);
	map(0x0022, 0x0023).noprw(); // TODO: Implement-me: Setup Extended I/O
	map(0x0040, 0x005f).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0xffff);
	map(0x00a0, 0x00bf).rw("pic_slave", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0xffff);
//	map(0x00c0, 0x00df).rw("dma8237_2", FUNC(am9517a_device::read), FUNC(am9517a_device::write).umask16(0x00ff);
	map(0x02f8, 0x02ff).rw("uart1", FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w)).umask16(0x00ff); //Asynchronous Serial I/O Channel 0 (COM2)
	map(0x03f8, 0x03ff).rw("uart0", FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w)).umask16(0x00ff); //Asynchronous Serial I/O Channel 0 (COM1)
	map(0xf020, 0xf03f).rw("pic_master", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0xffff);
	map(0xf040, 0xf05f).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0xffff);
	map(0xf0a0, 0xf0bf).rw("pic_slave", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0xffff);
	map(0xf400, 0xf43f).w(FUNC(i386ex_device::chip_select_unit_w)); //TODO: read ?
//	map(0xf480, 0xf48b).noprw(); // TODO: Implement-me: Synchrounous Serial Unit
//	map(0xf4a0, 0xf4a7).noprw(); // TODO: Implement-me: Refresh Control Unit
//	map(0xf4c0, 0xf4cb).noprw(); // TODO: Implement-me: Watchdot Timer Unit
	map(0xf4f8, 0xf4ff).rw("uart0", FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w)).umask16(0x00ff); //Asynchronous Serial I/O Channel 0 (COM1)
//	map(0xf800, 0xf803).noprw(); // TODO: Implement-me: Power Management
	map(0xf804, 0xf805).w(FUNC(i386ex_device::set_clock_prescaler));
//	map(0xf820, 0xf825).noprw(); // TODO: Device Configuration Registers (move here code from vmp3700 driver.)
	map(0xf826, 0xf827).w(FUNC(i386ex_device::set_pin_configuration)); //(TODO: READ)
	map(0xf830, 0xf831).w(FUNC(i386ex_device::set_dma_configuration)); // DMA Configuration Register (TODO: READ)
	map(0xf832, 0xf833).rw(FUNC(i386ex_device::get_interrupt_configuration), FUNC(i386ex_device::set_interrupt_configuration)); // Interrupt Configuration Register
	map(0xf834, 0xf835).w(FUNC(i386ex_device::set_timer_configuration)); // Timer Configuration Register (TODO: READ)
	map(0xf836, 0xf837).w(FUNC(i386ex_device::set_serial_io_configuration)); // SIO and SSIO Configuration Register(TODO: READ)
//	map(0xf860, 0xf875).noprw(); // TODO: Parallel I/O Ports (move here code from vmp3700 driver.)
	map(0xf8f8, 0xf8ff).rw("uart1", FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w)).umask16(0x00ff); //Asynchronous Serial I/O Channel 1 (COM2)
//	map(0x00c0, 0x00df).rw("dma8237_2", FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0x00ff);
}


// Serial port setup:
//
// [:maincpu] ':maincpu' (000F7703): unmapped io memory write to F8FA = 8000 & FF00
// [:maincpu] ':maincpu' (000F7710): unmapped io memory write to F8F8 = 0000 & FF00
// [:maincpu] ':maincpu' (000F7717): unmapped io memory write to F8F8 = 0018 & 00FF
// [:maincpu] ':maincpu' (000F771D): unmapped io memory write to F8FA = 0300 & FF00
// [:maincpu] ':maincpu' (000F7723): unmapped io memory write to F8FC = 0003 & 00FF
// [:maincpu] ':maincpu' (000F7729): unmapped io memory write to F8F8 = 0100 & FF00
//
// Meaning:
// F8FB (LCR1 = Serial Line #1 Control Register) = 80
//      
// F8F9 (DLH1) = 00 (0x18 >> 8 == 00)
// F8F8 (DLL1) = 18 (ss:[bp - 02h] with SS:BP = 0C9E:03DA)
//
// F8FB (LCR1) = 03
//
// F8FC (MCR1 = Modem Control #1 Register) = 03
//     Normal mode.
//     
// F8F9 () = 01
// 
//  SERCLK = 48MHz/4 = 12MHz
//  baud-rate output freq = BCLKIN/(0x18) = SERCLK/24 = 500kHz
//  500kHz / 16 = 31250 baud
//  1 stop-bit, 8bit, no parity.


// machine configs
void i386ex_device::device_add_mconfig(machine_config &config)
{	// Asynch Serial I/O ports:
	NS16450(config, m_uart0, DERIVED_CLOCK(1, 2));
	m_uart0->out_int_callback().set(m_pic_master, FUNC(pic8259_device::ir4_w)); // TODO: (mux:SIOINT0/INT9)

	NS16450(config, m_uart1, DERIVED_CLOCK(1, 2) / 0x18); // FIXME: hardcoded value based on vmp3700 boot code
	m_uart1->out_int_callback().set(m_pic_master, FUNC(pic8259_device::ir3_w)); // TODO: (mux:SIOINT1/INT8)

	// Programmable Interval Timer:
	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(DERIVED_CLOCK(1, 2));
	m_pit->set_clk<1>(DERIVED_CLOCK(1, 2));
	m_pit->set_clk<2>(DERIVED_CLOCK(1, 2));
	m_pit->out_handler<0>().set(m_pic_master, FUNC(pic8259_device::ir0_w));
	m_pit->out_handler<1>().set(m_pic_slave, FUNC(pic8259_device::ir2_w));
	m_pit->out_handler<2>().set(m_pic_slave, FUNC(pic8259_device::ir3_w));

	// Programmable Interrupt Controllers:
	PIC8259(config, m_pic_master, 0);
	// FIXME: INPUTLINE(DEVICE_SELF, 0), VCC, 
	m_pic_master->read_slave_ack_callback().set(FUNC(i386ex_device::get_slave_ack));

	PIC8259(config, m_pic_slave, 0);
	m_pic_slave->out_int_callback().set(m_pic_master, FUNC(pic8259_device::ir2_w));

	//TODO: (mux:Vss/INT0) DEVWRITELINE("pic8259_master", pic8259_device, ir1_w)
	//TODO: (mux:Vss/INT1) DEVWRITELINE("pic8259_master", pic8259_device, ir5_w)
	//TODO: (mux:Vss/INT2) DEVWRITELINE("pic8259_master", pic8259_device, ir6_w)
	//TODO: (mux:Vss/INT3) DEVWRITELINE("pic8259_master", pic8259_device, ir7_w)

	//TODO: (mux:Vss/INT4) DEVWRITELINE("pic8259_slave", pic8259_device, ir0_w)
	//(*vmp3700) TODO: (mux:SSIOINT/INT5) DEVWRITELINE("pic8259_slave", pic8259_device, ir1_w)
	//(*vmp3700) TODO: (mux:DMAINT/INT6) DEVWRITELINE("pic8259_slave", pic8259_device, ir4_w)
	//TODO: (mux:Vss/INT6/DMAINT) DEVWRITELINE("pic8259_slave", pic8259_device, ir5_w)
	//TODO: (mux:Vss/INT7) DEVWRITELINE("pic8259_slave", pic8259_device, ir6_w)
	//TODO: WDTOUT DEVWRITELINE("pic8259_slave", pic8259_device, ir7_w)
}
