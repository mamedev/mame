// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Kevin Horton
/******************************************************************************
*
*  Bare bones Realvoice PC driver
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  Binary supplied by Kevin 'kevtris' Horton
*

******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/m6800/m6801.h"
#include "machine/mos6551.h"
//#include "dectalk.lh" //  hack to avoid screenless system crash
#include "machine/terminal.h"

/* Components */

struct hd63701y0_t
{
	uint8_t data[8];
	uint8_t P1DDR;
	uint8_t P2DDR;
	uint8_t PORT1;
	uint8_t PORT2;
	uint8_t P3DDR;
	uint8_t P4DDR;
	uint8_t PORT3;
	uint8_t PORT4;
	uint8_t TCSR1;
	uint8_t FRCH;
	uint8_t FRCL;
	uint8_t OCR1H;
	uint8_t OCR1L;
	uint8_t ICRH;
	uint8_t ICRL;
	uint8_t TCSR2;
	uint8_t RMCR;
	uint8_t TRCSR1;
	uint8_t RDR;
	uint8_t TDR;
	uint8_t RP5CR;
	uint8_t PORT5;
	uint8_t P6DDR;
	uint8_t PORT6;
	uint8_t PORT7;
	uint8_t OCR2H;
	uint8_t OCR2L;
	uint8_t TCSR3;
	uint8_t TCONR;
	uint8_t T2CNT;
	uint8_t TRCSR2;
	uint8_t TSTREG;
	uint8_t P5DDR;
	uint8_t P6CSR;
};

struct rvoicepc_t
{
	uint8_t data[8];
	uint8_t port1;
	uint8_t port2;
	uint8_t port3;
	uint8_t port4;
	uint8_t port5;
	uint8_t port6;
	uint8_t port7;
};

class rvoice_state : public driver_device
{
public:
	rvoice_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
			{ }

	void rvoicepc(machine_config &config);

	void init_rvoicepc();

private:
	hd63701y0_t m_hd63701y0;
	rvoicepc_t m_rvoicepc;
	DECLARE_READ8_MEMBER(main_hd63701_internal_registers_r);
	DECLARE_WRITE8_MEMBER(main_hd63701_internal_registers_w);
	virtual void machine_reset() override;
	void null_kbd_put(u8 data);
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	void hd63701_main_mem(address_map &map);
};


/* Devices */

void rvoice_state::init_rvoicepc()
{
}

void rvoice_state::machine_reset()
{
	/* following is from datasheet at http://datasheets.chipdb.org/Hitachi/63701/HD63701Y0.pdf */
	m_hd63701y0.P1DDR = 0xFE; // port 1 ddr, W
	m_hd63701y0.P2DDR = 0x00; // port 2 ddr, W
	m_hd63701y0.PORT1 = 0x00; // port 1, R/W
	m_hd63701y0.PORT2 = 0x00; // port 2, R/W
	m_hd63701y0.P3DDR = 0xFE; // port 3 ddr, W
	m_hd63701y0.P4DDR = 0x00; // port 4 ddr, W
	m_hd63701y0.PORT3 = 0x00; // port 3, R/W
	m_hd63701y0.PORT4 = 0x00; // port 4, R/W
	m_hd63701y0.TCSR1 = 0x00; // timer control/status, R/W
	m_hd63701y0.FRCH = 0x00;
	m_hd63701y0.FRCL = 0x00;
	m_hd63701y0.OCR1H = 0xFF;
	m_hd63701y0.OCR1L = 0xFF;
	m_hd63701y0.ICRH = 0x00;
	m_hd63701y0.ICRL = 0x00;
	m_hd63701y0.TCSR2 = 0x10;
	m_hd63701y0.RMCR = 0xC0;
	m_hd63701y0.TRCSR1 = 0x20;
	m_hd63701y0.RDR = 0x00; // Receive Data Reg, R
	m_hd63701y0.TDR = 0x00; // Transmit Data Reg, W
	m_hd63701y0.RP5CR = 0x78; // or 0xF8; Ram/Port5 Control Reg, R/W
	m_hd63701y0.PORT5 = 0x00; // port 5, R/W
	m_hd63701y0.P6DDR = 0x00; // port 6 ddr, W
	m_hd63701y0.PORT6 = 0x00; // port 6, R/W
	m_hd63701y0.PORT7 = 0x00; // port 7, R/W
	m_hd63701y0.OCR2H = 0xFF;
	m_hd63701y0.OCR2L = 0xFF;
	m_hd63701y0.TCSR3 = 0x20;
	m_hd63701y0.TCONR = 0xFF;
	m_hd63701y0.T2CNT = 0x00;
	m_hd63701y0.TRCSR2 = 0x28;
	m_hd63701y0.TSTREG = 0x00;
	m_hd63701y0.P5DDR = 0x00; // port 5 ddr, W
	m_hd63701y0.P6CSR = 0x00;
}

READ8_MEMBER(rvoice_state::main_hd63701_internal_registers_r)
{
	uint8_t data = 0;
	logerror("main hd637B01Y0: %04x: read from 0x%02X: ", m_maincpu->pc(), offset);
	switch(offset)
	{
		case 0x00: // Port 1 DDR
		case 0x01: // Port 2 DDR
		case 0x04: // Port 3 DDR
		case 0x05: // Port 4 DDR
		case 0x13: // TxD Register
		case 0x16: // Port 6 DDR
		case 0x1C: // Time Constant Register
		case 0x20: // Port 5 DDR
			logerror("a write only register! returning 0\n");
			data = 0;
			return data;
		case 0x02: // Port 1
			logerror("Port 1\n");
			data = m_hd63701y0.PORT1;
			break;
		case 0x03: // Port 2
			logerror("Port 2\n");
			data = m_hd63701y0.PORT1;
			break;
		case 0x06: // Port 3
			logerror("Port 3\n");
			data = m_hd63701y0.PORT3;
			break;
		case 0x07: // Port 4
			logerror("Port 4\n");
			data = m_hd63701y0.PORT4;
			break;
		case 0x08: // Timer Control/Status Register 1
			logerror("Timer Control/Status Register 1\n");
			data = m_hd63701y0.TCSR1;
			break;
		case 0x09: // Free Running Counter (MSB)
			logerror("Free Running Counter (MSB)\n");
			data = m_hd63701y0.FRCH;
			break;
		case 0x0A: // Free Running Counter (LSB)
			logerror("Free Running Counter (LSB)\n");
			data = m_hd63701y0.FRCL;
			break;
		// B C (D E)
		case 0x0F: // Timer Control/Status Register 2
			logerror("Timer Control/Status Register 2\n");
			data = m_hd63701y0.TCSR2;
			break;
		// 10 11 (12)

		case 0x14: // RAM/Port 5 Control Register
			logerror("RAM/Port 5 Control Register\n");
			data = m_hd63701y0.RP5CR;
			break;
		case 0x15: // Port 5
			logerror("Port 5\n");
			data = m_hd63701y0.PORT5;
			break;
		case 0x17: // Port 6
			logerror("Port 6\n");
			data = m_hd63701y0.PORT6;
			break;
		case 0x18: // Port 7
			logerror("Port 7\n");
			data = m_hd63701y0.PORT7;
			break;
		// 19 1a 1b 1c 1d 1e 1f
		case 0x21: // Port 6 Control/Status Register
			logerror("Port 6 Control/Status Register\n");
			data = m_hd63701y0.P6CSR;
			break;
		default:
			logerror("\n");
			data = 0;
			break;
	}
	logerror("returning %02X\n", data);
	return data;
}


WRITE8_MEMBER(rvoice_state::main_hd63701_internal_registers_w)
{
	logerror("main hd637B01Y0: %04x: write to 0x%02X: ", m_maincpu->pc(), offset);
	switch(offset)
	{
		case 0x00: // Port 1 DDR
			logerror("Port 1 DDR of %02X\n", data);
			m_hd63701y0.P1DDR = data;
			m_rvoicepc.port1 = (m_hd63701y0.PORT1 & m_hd63701y0.P1DDR);
			break;
		case 0x01: // Port 2 DDR
			logerror("Port 2 DDR of %02X\n", data);
			m_hd63701y0.P2DDR = data;
			m_rvoicepc.port2 = (m_hd63701y0.PORT2 & m_hd63701y0.P2DDR);
			break;
		case 0x02: // Port 1
			logerror("Port 1 of %02X\n", data);
			m_hd63701y0.PORT1 = data;
			m_rvoicepc.port1 = (m_hd63701y0.PORT1 & m_hd63701y0.P1DDR);
			break;
		case 0x03: // Port 2
			logerror("Port 2 of %02X\n", data);
			m_hd63701y0.PORT2 = data;
			m_rvoicepc.port2 = (m_hd63701y0.PORT2 & m_hd63701y0.P2DDR);
			break;
		case 0x04: // Port 3 DDR
			logerror("Port 3 DDR of %02X\n", data);
			m_hd63701y0.P3DDR = data;
			m_rvoicepc.port3 = (m_hd63701y0.PORT3 & m_hd63701y0.P3DDR);
			break;
		case 0x05: // Port 4 DDR
			logerror("Port 4 DDR of %02X\n", data);
			m_hd63701y0.P4DDR = data;
			m_rvoicepc.port4 = (m_hd63701y0.PORT4 & m_hd63701y0.P4DDR);
			break;
		case 0x06: // Port 3
			logerror("Port 3 of %02X\n", data);
			m_hd63701y0.PORT3 = data;
			m_rvoicepc.port3 = (m_hd63701y0.PORT3 & m_hd63701y0.P3DDR);
			break;
		case 0x07: // Port 4
			logerror("Port 4 of %02X\n", data);
			m_hd63701y0.PORT4 = data;
			m_rvoicepc.port4 = (m_hd63701y0.PORT4 & m_hd63701y0.P4DDR);
			break;
		case 0x08: // Timer Control/Status Register 1
			logerror("Timer Control/Status Register 1 of %02X\n", data);
			m_hd63701y0.TCSR1 = data;
			break;
		case 0x09: // Free Running Counter (MSB)
			logerror("Free Running Counter (MSB) of %02X\n", data);
			m_hd63701y0.FRCH = data;
			break;
		case 0x0A: // Free Running Counter (LSB)
			logerror("Free Running Counter (LSB) of %02X\n", data);
			m_hd63701y0.FRCL = data;
			break;
		// B C (D E)
		case 0x0F: // Timer Control/Status Register 2
			logerror("Timer Control/Status Register 2 of %02X\n", data);
			m_hd63701y0.TCSR2 = data;
			break;
		// 10 11 (12)
		case 0x13: // TxD Register
			logerror("TxD Register of %02X\n", data);
			m_hd63701y0.TDR = data;
			break;
		case 0x14: // RAM/Port 5 Control Register
			logerror("RAM/Port 5 Control Register of %02X\n", data);
			m_hd63701y0.RP5CR = data;
			break;
		case 0x15: // Port 5
			logerror("Port 5 of %02X\n", data);
			m_hd63701y0.PORT5 = data;
			m_rvoicepc.port5 = (m_hd63701y0.PORT5 & m_hd63701y0.P5DDR);
			break;
		case 0x16: // Port 6 DDR
			logerror("Port 6 DDR of %02X\n", data);
			m_hd63701y0.P6DDR = data;
			m_rvoicepc.port6 = (m_hd63701y0.PORT6 & m_hd63701y0.P6DDR);
			break;
		case 0x17: // Port 6
			logerror("Port 6 of %02X\n", data);
			m_hd63701y0.PORT6 = data;
			m_rvoicepc.port6 = (m_hd63701y0.PORT6 & m_hd63701y0.P6DDR);
			break;
		case 0x18: // Port 7
			logerror("Port 7 of %02X\n", data);
			m_hd63701y0.PORT7 = data;
			m_rvoicepc.port7 = data;
			break;
		// 19 1a 1b 1c 1d 1e 1f
		case 0x20: // Port 5 DDR
			logerror("Port 5 DDR of %02X\n", data);
			m_hd63701y0.P5DDR = data;
			m_rvoicepc.port5 = (m_hd63701y0.PORT5 & m_hd63701y0.P5DDR);
			break;
		case 0x21: // Port 6 Control/Status Register
			logerror("Port 6 Control/Status Register of %02X\n", data);
			m_hd63701y0.P6CSR = data;
			break;
		default:
			logerror("with data of %02X\n", data);
			break;
	}
}


/******************************************************************************
 Address Maps
******************************************************************************/

void rvoice_state::hd63701_main_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0027).rw(FUNC(rvoice_state::main_hd63701_internal_registers_r), FUNC(rvoice_state::main_hd63701_internal_registers_w)); // INTERNAL REGS
	map(0x0040, 0x005f).ram(); // INTERNAL RAM (overlaps acia)
	map(0x0060, 0x007f).rw("acia65c51", FUNC(mos6551_device::read), FUNC(mos6551_device::write)); // ACIA 65C51
	map(0x0080, 0x013f).ram(); // INTERNAL RAM (overlaps acia)
	map(0x2000, 0x7fff).ram(); // EXTERNAL SRAM
	map(0x8000, 0xffff).rom(); // 27512 EPROM
}


/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( rvoicepc )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/
void rvoice_state::null_kbd_put(u8 data)
{
}

void rvoice_state::rvoicepc(machine_config &config)
{
	/* basic machine hardware */
	HD63701(config, m_maincpu, XTAL(7'372'800));
	m_maincpu->set_addrmap(AS_PROGRAM, &rvoice_state::hd63701_main_mem);

	//hd63701_cpu_device &playercpu(HD63701(config "playercpu", XTAL(7'372'800))); // not dumped yet
	//playercpu.set_addrmap(AS_PROGRAM, &rvoice_state::hd63701_slave_mem);
	//playercpu.set_addrmap(AS_PROGRAM, &rvoice_state::hd63701_slave_io);
	config.m_minimum_quantum = attotime::from_hz(60);

	mos6551_device &acia(MOS6551(config, "acia65c51", 0));
	acia.set_xtal(1.8432_MHz_XTAL);

	/* video hardware */

	/* sound hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(rvoice_state::null_kbd_put));
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(rvoicepc)

	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rv_pc.bin", 0x08000, 0x08000, CRC(4001cd5f) SHA1(d973c6e19e493eedd4f7216bc530ddb0b6c4921e))
	ROM_CONTINUE(0x8000, 0x8000) // first half of 27c512 rom is blank due to stupid address decoder circuit

ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT     CLASS         INIT           COMPANY                           FULLNAME        FLAGS
COMP( 1988?, rvoicepc, 0,      0,      rvoicepc, rvoicepc, rvoice_state, init_rvoicepc, "Adaptive Communication Systems", "Realvoice PC", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
