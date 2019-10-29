// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***********************************************************************************************************
 *
 *   Ericsson Information Systems/Nokia Data/ICL, SAD 8852 IBM 3270/5250 terminal emulation adapter
 *
 * This board is a terminal adapter for XT class PC machines to be connected as a terminal to
 * IBM mainframes. There are two on board connectors, a BNC connector to act as a 3270 terminal
 * and a twinax connector for the older 5250 terminal.
 *
 * TODO:
 *    - Hook up 8274 fully
 *    - Add bitbanger device and hook it up to an (emulated?!) IBM mainframe
 *
 ************************************************************************************************************/
/*
 Links:
 ------
 https://github.com/MattisLind/alfaskop_emu

 Ericsson manufactured board -  marked 83016053-30, assembled in 1984 indicated by chip dates
 +--------------------------------------------------------------------------------------+ ___
 |O 83016053-30                                          IC11     IC20      X1      |o|_||
 |                     IC14         IC15                74LS74   74S37    19.170MHz |____|
 |                                                                         NDK 4Y       ||
 | RS232   IC13                          IC18                                           ||
 |                   IC12               ULA 2C143E Ferranti                             ||---
 | X.27              i8274 MPSC                                                       P1|| |= Twinax (5250)
 |                   serial controller        IC19    IC17                              ||---
 |  P3     IC16                             754528P  LM339             1234567890       ||
 |                              IC10                                      SW2 DIP       ||
 |                             74LS08        IC21                      RP2 Resistor SIL ||
 |        W2                                74LS30                  IC2       IC8       ||
 |    IC5         IC1                        IC7                   74LS125   74LS240    ||
 |   74LS04      74LS244   RP1              74LS86         W1     CSA1   CSA2           ||---
 |     IC4        IC3       Resistor DIL     IC6          ooooo      o5     o5 IC9    P2|| |- BNC(3270)
 |    74LS00     74LS00   1234567890        74LS86        ooooo   1o o4  1o o4 74LS245  ||---
 |                          SW1 DIP                               2o o3  2o o3       ____|
 +----------------------------------------------------------------------------------|o___|   
                                                           |||||||||||||||||||||||||     |
 Notes                                                                                   |
 ------------------------------------------------------------------------------          |
 IC13-IC16 unpopulated IC:s
 CSA1,CSA2 unpopulated jumper areas
 W2        unpopulated jumper area
 P3        unpopulated connector RS232/X.27(RS422)

 General description
 -------------------
 This is a passive ISA8 board that should be fitted into an Ericsson PC (epc) and
 driven by suitable software. It was replaced by the ISA16 SAD8852 intelligent TWIB
 board for WS286 and higher a few years later. 

 */

#include "emu.h"
#include "eis_twib.h"
#include "machine/z80sio.h"

#define LOG_READ   (1U << 1)
#define LOG_IRQ    (1U << 2)

//#define VERBOSE (LOG_IRQ)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGR(...)   LOGMASKED(LOG_READ, __VA_ARGS__)
#define LOGIRQ(...) LOGMASKED(LOG_IRQ, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

DEFINE_DEVICE_TYPE(ISA8_EIS_TWIB, isa8_eistwib_device, "eistwib", "EIS TWIB IBM mainframe terminal adapter")

//-------------------------------------------------
//  Access methods from ISA bus
//-------------------------------------------------
READ8_MEMBER( isa8_eistwib_device::twib_r )
{
	LOGR("%s : offset=%d\n", FUNCNAME, offset);
	return m_uart8274->cd_ba_r(offset);
}

WRITE8_MEMBER( isa8_eistwib_device::twib_w )
{
	LOG("%s : offset=%d data=0x%02x\n", FUNCNAME, offset, data);
	m_uart8274->cd_ba_w(offset, data);
}

TIMER_DEVICE_CALLBACK_MEMBER(isa8_eistwib_device::tick_bitclock)
{
	m_uart8274->txca_w(m_bitclock);
	m_uart8274->rxca_w(m_bitclock);
	m_sdlclogger->clock_w(m_bitclock);
	m_bitclock = !m_bitclock;
}


//----------------------------------------------------------
//  UI I/O
//----------------------------------------------------------
static INPUT_PORTS_START( eistwib_ports )
	PORT_START("SW1")
	PORT_DIPNAME( 0x3f0, 0x380, "I/O Base address" )
	PORT_DIPSETTING( 0x000, "0x000" )
	PORT_DIPSETTING( 0x010, "0x010" )
	PORT_DIPSETTING( 0x020, "0x020" )
	PORT_DIPSETTING( 0x030, "0x030" )
	PORT_DIPSETTING( 0x040, "0x040" )
	PORT_DIPSETTING( 0x050, "0x050" )
	PORT_DIPSETTING( 0x060, "0x060" )
	PORT_DIPSETTING( 0x070, "0x070" )
	PORT_DIPSETTING( 0x080, "0x080" )
	PORT_DIPSETTING( 0x090, "0x090" )
	PORT_DIPSETTING( 0x0a0, "0x0a0" )
	PORT_DIPSETTING( 0x0b0, "0x0b0" )
	PORT_DIPSETTING( 0x0c0, "0x0c0" )
	PORT_DIPSETTING( 0x0d0, "0x0d0" )
	PORT_DIPSETTING( 0x0e0, "0x0e0" )
	PORT_DIPSETTING( 0x0f0, "0x0f0" )
	PORT_DIPSETTING( 0x100, "0x100" )
	PORT_DIPSETTING( 0x110, "0x110" )
	PORT_DIPSETTING( 0x120, "0x120" )
	PORT_DIPSETTING( 0x130, "0x130" )
	PORT_DIPSETTING( 0x140, "0x140" )
	PORT_DIPSETTING( 0x150, "0x150" )
	PORT_DIPSETTING( 0x160, "0x160" )
	PORT_DIPSETTING( 0x170, "0x170" )
	PORT_DIPSETTING( 0x180, "0x180" )
	PORT_DIPSETTING( 0x190, "0x190" )
	PORT_DIPSETTING( 0x1a0, "0x1a0" )
	PORT_DIPSETTING( 0x1b0, "0x1b0" )
	PORT_DIPSETTING( 0x1c0, "0x1c0" )
	PORT_DIPSETTING( 0x1d0, "0x1d0" )
	PORT_DIPSETTING( 0x1e0, "0x1e0" )
	PORT_DIPSETTING( 0x1f0, "0x1f0" )
	PORT_DIPSETTING( 0x200, "0x200" )
	PORT_DIPSETTING( 0x210, "0x210" )
	PORT_DIPSETTING( 0x220, "0x220" )
	PORT_DIPSETTING( 0x230, "0x230" )
	PORT_DIPSETTING( 0x240, "0x240" )
	PORT_DIPSETTING( 0x250, "0x250" )
	PORT_DIPSETTING( 0x260, "0x260" )
	PORT_DIPSETTING( 0x270, "0x270" )
	PORT_DIPSETTING( 0x280, "0x280" )
	PORT_DIPSETTING( 0x290, "0x290" )
	PORT_DIPSETTING( 0x2a0, "0x2a0" )
	PORT_DIPSETTING( 0x2b0, "0x2b0" )
	PORT_DIPSETTING( 0x2c0, "0x2c0" )
	PORT_DIPSETTING( 0x2d0, "0x2d0" )
	PORT_DIPSETTING( 0x2e0, "0x2e0" )
	PORT_DIPSETTING( 0x2f0, "0x2f0" )
	PORT_DIPSETTING( 0x300, "0x300" )
	PORT_DIPSETTING( 0x310, "0x310" )
	PORT_DIPSETTING( 0x320, "0x320" )
	PORT_DIPSETTING( 0x330, "0x330" )
	PORT_DIPSETTING( 0x340, "0x340" )
	PORT_DIPSETTING( 0x350, "0x350" )
	PORT_DIPSETTING( 0x360, "0x360" )
	PORT_DIPSETTING( 0x370, "0x370" )
	PORT_DIPSETTING( 0x380, "0x380" )
	PORT_DIPSETTING( 0x390, "0x390" )
	PORT_DIPSETTING( 0x3a0, "0x3a0" )
	PORT_DIPSETTING( 0x3b0, "0x3b0" )
	PORT_DIPSETTING( 0x3c0, "0x3c0" )
	PORT_DIPSETTING( 0x3d0, "0x3d0" )
	PORT_DIPSETTING( 0x3e0, "0x3e0" )
	PORT_DIPSETTING( 0x3f0, "0x3f0" )

	PORT_START("W1") // Jumper area, field 0=no jumper 1=LPT 2=COM 3=n/a
	PORT_DIPNAME(0x01, 0x00, "ISA IRQ2")
	PORT_DIPSETTING(0x00, "no jumper")
	PORT_DIPSETTING(0x01, "8274 INT")
	PORT_DIPNAME(0x02, 0x00, "ISA IRQ3")
	PORT_DIPSETTING(0x00, "no jumper")
	PORT_DIPSETTING(0x02, "8274 INT")
	PORT_DIPNAME(0x04, 0x04, "ISA IRQ4")
	PORT_DIPSETTING(0x00, "no jumper")
	PORT_DIPSETTING(0x04, "8274 INT")
	PORT_DIPNAME(0x08, 0x00, "ISA IRQ5")
	PORT_DIPSETTING(0x00, "no jumper")
	PORT_DIPSETTING(0x08, "8274 INT")
	PORT_DIPNAME(0x10, 0x00, "ISA IRQ6")
	PORT_DIPSETTING(0x00, "no jumper")
	PORT_DIPSETTING(0x10, "8274 INT")
INPUT_PORTS_END

ioport_constructor isa8_eistwib_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( eistwib_ports );
}

//-------------------------------------------------
//  Board configuration
//-------------------------------------------------
void isa8_eistwib_device::device_add_mconfig(machine_config &config)
{
	SDLC_LOGGER(config, m_sdlclogger, 0); // To decode the frames
	I8274_NEW(config, m_uart8274, (XTAL(14'318'181)/ 3) / 2); // Half the 4,77 MHz ISA bus CLK signal
	//m_uart8274->out_rtsa_callback().set([this] (int state) { m_rts = state; });
	m_uart8274->out_txda_callback().set([this] (int state) { m_txd = state; m_sdlclogger->data_w(state); });
	m_uart8274->out_int_callback().set([this] (int state)
	{   // Jumper field W1 decides what IRQs to pull
		if (m_isairq->read() & 0x01) { LOGIRQ("TWIB IRQ2: %d\n", state); m_isa->irq2_w(state); }
		if (m_isairq->read() & 0x02) { LOGIRQ("TWIB IRQ3: %d\n", state); m_isa->irq3_w(state); }
		if (m_isairq->read() & 0x04) { LOGIRQ("TWIB IRQ4: %d\n", state); m_isa->irq4_w(state); }
		if (m_isairq->read() & 0x08) { LOGIRQ("TWIB IRQ5: %d\n", state); m_isa->irq5_w(state); }
		if (m_isairq->read() & 0x10) { LOGIRQ("TWIB IRQ6: %d\n", state); m_isa->irq6_w(state); }
	});

	TIMER(config, "bitclock").configure_periodic(FUNC(isa8_eistwib_device::tick_bitclock), attotime::from_hz(300000));
}

isa8_eistwib_device::isa8_eistwib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_EIS_TWIB, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_uart8274(*this, "terminal")
	, m_sdlclogger(*this, "logger")
	, m_bitclock(false)
	, m_rts(false)
	, m_txd(false)
	, m_sw1(*this, "SW1")
	, m_isairq(*this, "W1")
	, m_installed(false)
{
}

//-------------------------------------------------
//  Overloading methods
//-------------------------------------------------
void isa8_eistwib_device::device_start()
{
	set_isa_device();
	m_installed = false;
	m_bitclock = false;
	save_item(NAME(m_installed));
	save_item(NAME(m_bitclock));
}

void isa8_eistwib_device::device_reset()
{
	int base = m_sw1->read();
	if (!m_installed)
	{
		LOG("Installing twib device at %04x\n", base); 
		m_isa->install_device(
				base, base + 0x0f,
				read8_delegate(*this, FUNC( isa8_eistwib_device::twib_r )),
				write8_delegate(*this, FUNC( isa8_eistwib_device::twib_w )));
		m_installed = true;
	}
	// CD and CTS input are tied to ground
	m_uart8274->ctsa_w(0);
	m_uart8274->dcda_w(0);
}
