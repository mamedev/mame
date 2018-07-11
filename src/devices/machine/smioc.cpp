// license:GPL-2.0+
// copyright-holders:Brandon Munger
/**********************************************************************

    ROLM 9751 9005 System Monitor Input/Output Card emulation

**********************************************************************/

/*
    Device SMIOC
    Board Copyright - IBM Corp 1989 Made in USA

    Labels:
        * 98R0083
          MN 90770AR

        * EC# A65048R

    Hardware:
        * CPU - Intel N80C188 L0450591 @ ??MHz - U23
        * MCU - Signetics SC87C451CCA68 220CP079109KA 97D8641 - U70
        * DMA - KS82C37A - U46, U47, U48, U49, U50
    * SCC - Signetics SCC2698BC1A84 - U67
        * Memory - NEC D43256AGU-10LL 8948A9038 SRAM 32KB - U51
        * Memory - Mitsubishi M5M187AJ 046101-35 SRAM 64K X 1?? - U37
    * Memory - AT&T M79018DX-15B 2K X 9 Dual Port SRAM - U53
    * Memory - AT&T M79018DX-15B 2K X 9 Dual Port SRAM - U54

    Logic:
        * U8 - 22V10-25JC
        * U33 - 22V10-25JC
        * U61 - 22V10-25JC
        * U63 - 22V10-25JC
        * U87 - 22V10-20JC
        * U88 - 22V10-20JC
        * U102 - 22V10-25JC
        * U111 - 22V10-25JC

    Switches:
        * S1 - Board reset

    Program Memory:
        * 0x00000 - 0x07FFF : SRAM D43256AGU-10LL 32KB
        * 0xF8000 - 0xFFFFF : ROM 27C256 PLCC32 32KB
    * 0xC0080 - 0xC008F : KS82C37A - Probably RAM DMA
    * 0xC0090 - 0xC009F : KS82C37A - Serial DMA (Port 1 and 2?)
    * 0xC00A0 - 0xC00AF : KS82C37A - Serial DMA (Port 3 and 4?)
    * 0xC00B0 - 0xC00BF : KS82C37A - Serial DMA (Port 5 and 6?)
    * 0xC00C0 - 0xC00CF : KS82C37A - Serial DMA (Port 7 and 8?)

    IO Memory:
        * Unknown

    TODO:
    * Emulate SCC and hook up RS232 ports
    * Hook up console to RS232 port 1
    * Hook up System Monitor II to RS232 port 2
    * Dump 87C451 rom data and emulate MCU
    * Dump 87C51 on SMIOC interconnect box
    * Dump all PAL chips
    * Hook up status LEDs
*/

#include "emu.h"
#include "smioc.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I188_TAG     "smioc_i188" // U23

#define ENABLE_LOG_REGISTER_ACCESS 0
#define ENABLE_LOG_COMMAND 1


#define LOG_REGISTER_ACCESS if(ENABLE_LOG_REGISTER_ACCESS) logerror
#define LOG_COMMAND if(ENABLE_LOG_COMMAND) logerror

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SMIOC, smioc_device, "rolm_smioc", "ROLM SMIOC")

//-------------------------------------------------
//  ROM( SMIOC )
//-------------------------------------------------

ROM_START( smioc )
	ROM_REGION( 0x8000, "rom", 0 )
	ROM_LOAD( "smioc.27256.u65", 0x0000, 0x8000, CRC(25b93192) SHA1(8ee9879033623490ce6703ba5429af2b16dbae84) )
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *smioc_device::device_rom_region() const
{
	return ROM_NAME( smioc );
}

//-------------------------------------------------
//  ADDRESS_MAP( smioc_mem )
//-------------------------------------------------
void smioc_device::smioc_mem(address_map &map)
{
	map(0x00000, 0x07FFF).ram().share("smioc_ram");
	map(0x40000, 0x4FFFF).rw(FUNC(smioc_device::ram2_mmio_r), FUNC(smioc_device::ram2_mmio_w));
	map(0x50000, 0x5FFFF).rw(FUNC(smioc_device::dma68k_r), FUNC(smioc_device::dma68k_w));
	map(0xC0080, 0xC008F).rw("dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Probably RAM DMA
	map(0xC0090, 0xC009F).rw("dma8237_2", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC00A0, 0xC00AF).rw("dma8237_3", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC00B0, 0xC00BF).rw("dma8237_4", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC00C0, 0xC00CF).rw("dma8237_5", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC0100, 0xC011F).rw(FUNC(smioc_device::boardlogic_mmio_r), FUNC(smioc_device::boardlogic_mmio_w));
	map(0xC0200, 0xC023F).rw("scc2698b", FUNC(scc2698b_device::read), FUNC(scc2698b_device::write));
	map(0xF8000, 0xFFFFF).rom().region("rom", 0);
}

static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_STARTBITS", 0xff, RS232_STARTBITS_1)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_7)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_EVEN)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


MACHINE_CONFIG_START(smioc_device::device_add_mconfig)
	/* CPU - Intel 80C188 */
	MCFG_DEVICE_ADD(I188_TAG, I80188, XTAL(20'000'000)) // Clock division unknown
	MCFG_QUANTUM_TIME(attotime::from_hz(1000))
	MCFG_DEVICE_PROGRAM_MAP(smioc_mem)

	/* DMA */
	for (required_device<am9517a_device> &dma : m_dma8237)
		AM9517A(config, dma, 20_MHz_XTAL / 4); // Clock division unknown

	/* RS232 */	
	/* Port 1: Console */
	MCFG_DEVICE_ADD("rs232_p1", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_SLOT_OPTION_DEVICE_INPUT_DEFAULTS("terminal", terminal)
	for (required_device<rs232_port_device> &rs232_port : m_rs232_p)
		RS232_PORT(config, rs232_port, default_rs232_devices, nullptr);

	

	/* SCC2698B */
	scc2698b_device &scc2698b(SCC2698B(config, "scc2698b", XTAL(3'686'400)));
	scc2698b.tx_callback('a').set("rs232_p1", FUNC(rs232_port_device::write_txd));
	scc2698b.mpp1_callback('a').set("dma8237_2", FUNC(am9517a_device::dreq1_w)).invert();
	scc2698b.mpp2_callback('a').set("dma8237_2", FUNC(am9517a_device::dreq0_w)).invert();
	scc2698b.tx_callback('b').set("rs232_p2", FUNC(rs232_port_device::write_txd));
	scc2698b.mpp1_callback('b').set("dma8237_2", FUNC(am9517a_device::dreq3_w)).invert();
	scc2698b.mpp2_callback('b').set("dma8237_2", FUNC(am9517a_device::dreq2_w)).invert();

	//MCFG_SCC2698B_TX_CALLBACK(a, WRITELINE("rs232_p1", rs232_port_device, write_txd))
	//MCFG_SCC2698B_MPP1_CALLBACK(a, WRITELINE("dma8237_2", am9517a_device, dreq1_w).invert()) // MPP1 output is TxRDY (Active High), DREQ1 is UART 0 TX request (Active Low)
	//MCFG_SCC2698B_MPP2_CALLBACK(a, WRITELINE("dma8237_2", am9517a_device, dreq0_w).invert()) // MPP2 output is RxRDY (Active High), DREQ0 is UART 0 RX request (Active Low)
	//MCFG_SCC2698B_TX_CALLBACK(b, WRITELINE("rs232_p2", rs232_port_device, write_txd))
	//MCFG_SCC2698B_MPP1_CALLBACK(b, WRITELINE("dma8237_2", am9517a_device, dreq3_w).invert())
	//MCFG_SCC2698B_MPP2_CALLBACK(b, WRITELINE("dma8237_2", am9517a_device, dreq2_w).invert())

	/* The first dma8237 is set up in cascade mode, and each of its four channels provides HREQ/HACK to the other 4 DMA controllers*/
	MCFG_DEVICE_MODIFY("dma8237_1")
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE("dma8237_2", am9517a_device, hack_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE("dma8237_3", am9517a_device, hack_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE("dma8237_4", am9517a_device, hack_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE("dma8237_5", am9517a_device, hack_w))
	MCFG_I8237_OUT_HREQ_CB(WRITELINE("dma8237_1", am9517a_device, hack_w))
	/* Connect base DMA controller's Hold Request to its own Hold ACK
		The CPU doesn't support hold request / hold ack pins, so we will pretend that the DMA controller immediately gets what it wants every time it asks.
		This will keep things moving forward, but the CPU will continue going through DMA requests rather than halting for a few cycles each time.
		It shouldn't cause a problem though.
		*/

	MCFG_DEVICE_MODIFY("dma8237_2")
	MCFG_AM9517A_IN_MEMR_CB(READ8(*this, smioc_device, dma8237_2_dmaread))
	MCFG_AM9517A_OUT_MEMW_CB(WRITE8(*this, smioc_device, dma8237_2_dmawrite))
	MCFG_I8237_OUT_HREQ_CB(WRITELINE("dma8237_1", am9517a_device, dreq0_w))


	MCFG_DEVICE_MODIFY("rs232_p1")
	MCFG_RS232_RXD_HANDLER(WRITELINE("scc2698b", scc2698b_device, port_a_rx_w))
	MCFG_DEVICE_MODIFY("rs232_p2")
	MCFG_RS232_RXD_HANDLER(WRITELINE("scc2698b", scc2698b_device, port_b_rx_w))


MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  smioc_device - constructor
//-------------------------------------------------

smioc_device::smioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMIOC, tag, owner, clock),
	m_smioccpu(*this, I188_TAG),
	m_dma8237(*this, "dma8237_%u", 1),
	m_rs232_p1(*this, "rs232_p1"),
	m_rs232_p(*this, "rs232_p%u", 2),
	m_scc2698b(*this, "scc2698b"),
	m_smioc_ram(*this, "smioc_ram"),
	m_dma_timer(nullptr),
	m_m68k_r_cb(*this),
	m_m68k_w_cb(*this)
{


}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void smioc_device::device_start()
{
	m_dma_timer = timer_alloc(0, nullptr);

	/* Resolve callbacks */
	m_m68k_r_cb.resolve_safe(0);
	m_m68k_w_cb.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void smioc_device::device_reset()
{
	/* Reset CPU */
	m_smioccpu->reset();
	m_smioccpu->drq0_w(1);



}

void smioc_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	switch (tid)
	{
	case 0: // DMA Timer
		m_smioccpu->drq0_w(1);
		break;
	}
}

void smioc_device::SendCommand(u16 command)
{
	LOG_COMMAND("SMIOC Command %04x (Address %04x, Length %04x)\n", command, m_dmaSendAddress, m_dmaSendLength);

	// Assume that command 0 is a hard reset command handled by the logic in the board rather than the cpu.
	// It's difficult to be sure that this is what actually should happen, but given the context and how it's used, this seems likely.
	if (command == 0)
	{
		m_smioccpu->reset();
		return;
	}

	m_commandValue = command;
	m_requestFlags_11D |= 1;
	m_deviceBusy = 1;
	
	//m_smioccpu->set_input_line(INPUT_LINE_IRQ2, HOLD_LINE);
	m_smioccpu->int2_w(HOLD_LINE);
	
}

void smioc_device::ClearStatus()
{
	m_status = 0;
}
void smioc_device::ClearStatus2()
{
	m_status2 = 0;
}


void smioc_device::update_and_log(u16& reg, u16 newValue, const char* register_name)
{
	//if (reg != newValue)
	{
		logerror("Update %s %04X -> %04X\n", register_name, reg, newValue);
		reg = newValue;
	}
}

READ8_MEMBER(smioc_device::ram2_mmio_r)
{
	u8 data = 0;
	switch (offset)
	{
	case 0xB83: // Unclear
		LOG_COMMAND("SMIOC Read B83 (Unknown)\n");
		break;

	case 0xB87: // Unclear
		LOG_COMMAND("SMIOC Read B87 (Unknown)\n");
		break;


	case 0xCC2: // Command from 68k?
		data = m_commandValue & 0xFF;
		break;
	case 0xCC3:
		data = m_commandValue >> 8;
		LOG_COMMAND("SMIOC Read CC3 (Command)\n");
		break;

	case 0xCC7:
		LOG_COMMAND("SMIOC Read CC7 (Unknown)\n");
		break;


	case 0xCD8: // DMA source address (for writing characters) - Port 0
		data = m_dmaSendAddress & 0xFF;
		break;
	case 0xCD9:
		data = m_dmaSendAddress >> 8;
		break;

	case 0xCE8: // DMA Length (for writing characters) - Port 0
		data = m_dmaSendLength & 0xFF;
		break;
	case 0xCE9:
		data = m_dmaSendLength >> 8;
		break;

	}


	logerror("ram2[%04X] => %02X\n", offset, data);
	return data;
}

WRITE8_MEMBER(smioc_device::ram2_mmio_w)
{
	switch (offset) // Offset based on C0100 register base
	{
		
	case 0xC84:
		//update_and_log(m_wordcount, (m_wordcount & 0xFF00) | data, "Wordcount[40C84]");
		return;
	case 0xC85:
		//update_and_log(m_wordcount, (m_wordcount & 0xFF) | (data<<8), "Wordcount[40C85]");
		return;

	case 0xC88:
		//update_and_log(m_wordcount2, (m_wordcount2 & 0xFF00) | data, "Wordcount2[40C88]");
		return;
	case 0xC89:
		//update_and_log(m_wordcount2, (m_wordcount2 & 0xFF) | (data<<8), "Wordcount2[40C89]");
		return;

	case 0xCC4:
		update_and_log(m_status, (m_status & 0xFF00) | (data), "Status[40CC4]");
		return;
	case 0xCC5:
		update_and_log(m_status, (m_status & 0xFF) | (data << 8), "Status[40CC5]");
		m_status |= 0x40;
		return;

	case 0xCC8:
		update_and_log(m_status2, (m_status2 & 0xFF) | (data << 8), "Status2[40CC8]");
		return;
	case 0xCC9:
		update_and_log(m_status2, (m_status2 & 0xFF00) | data, "Status2[40CC9]");
		m_status2 |= 0x40;
		return;

	}


	logerror("ram2[%04X] <= %02X\n", offset, data);
}

READ8_MEMBER(smioc_device::dma68k_r)
{
	u8 data = 0;

	m_dma_timer->adjust(attotime::from_usec(10));

	// This behavior is not currently well understood - but it seems like the address might be off by one.
	// When the SMIOC starts a DMA read from the 68k memory transfer space, the first word it reads (at the right address), it throws away
	// And it deliberately reads the DMA at address+1.
	// One possibility is that issuing the read is more like a doorbell for the hardware on the board to fetch a data stream starting at that address
	// but it's not currently clear how it works, so implementing read for the time being as reading address-1 (address - 2 bytes)
	// Another (and maybe more likely) possibility is that the hardware is doing a slow fetch of the value, and the N+1 read returns data from the Nth read's address

	data = m_m68k_r_cb(offset-1);
	LOG_REGISTER_ACCESS("dma68k[%04X] => %02X\n", offset, data);
	return data;
}

WRITE8_MEMBER(smioc_device::dma68k_w)
{

	m_dma_timer->adjust(attotime::from_usec(10));

	m_m68k_w_cb(offset, data);

	LOG_REGISTER_ACCESS("dma68k[%04X] <= %02X\n", offset, data);
}

READ8_MEMBER(smioc_device::boardlogic_mmio_r)
{
	u8 data = 0xFF;
	switch (offset)
	{

		case 0x19: // Hardware revision?
			// Top bit controls which set of register locations in RAM2 are polled
			data = 0x7F;
			break;

		case 0x1D: // C011D (HW Request flags)
			data = m_requestFlags_11D;
			// Assume this is a clear-on-read register - It is read in one location and all set bits are acted on once it is read.
			m_requestFlags_11D = 0;


			break;

		case 0x1F: // C011F (HW Status flags?)
			// 0x80, 0x40 seem to be HW request to cancel ongoing DMA requests, maybe related to board reset?
			// 0x01 - When this is 0, advance some state perhaps related to talking to the 8051
			data = 0xFF;
			break;

	}
	LOG_REGISTER_ACCESS("logic[%04X] => %02X\n", offset, data);
	return data;
}

WRITE8_MEMBER(smioc_device::boardlogic_mmio_w)
{
	switch (offset)
	{
	case 0x10: // C0110 (Clear interrupt? This seems to happen a lot but without being related to actually completing anything.)
		m_smioccpu->int2_w(CLEAR_LINE);
		m_deviceBusy = m_requestFlags_11D;
		break;

	case 0x11: // C0111 - Set to 1 after providing a status - Acknowledge by hardware by raising bit 4 in C011D (SMIOC E2E flag 0x200
		m_requestFlags_11D |= 0x10; // bit 4
		m_smioccpu->int2_w(HOLD_LINE);
		break;

	case 0x12: // C0112 - Set to 1 after providing a status(2?) - Acknowledge by hardware by raising bit 5 in C011D (SMIOC E2E flag 0x100)
		m_requestFlags_11D |= 0x20; // bit 5
		m_smioccpu->int2_w(HOLD_LINE);
		break;

	case 0x16: // C0116 - Set to 1 after processing 11D & 0x40
		break;
	case 0x17: // C0117 - Set to 1 after processing 11D & 0x80
		break;

	}
	LOG_REGISTER_ACCESS("logic[%04X] <= %02X\n", offset, data);
}




READ8_MEMBER(smioc_device::dma8237_2_dmaread)
{
	int data = m_smioccpu->space(AS_PROGRAM).read_byte(offset);
	LOG_REGISTER_ACCESS("dma2read [0x%x] => 0x%x\n", offset, data);
	m_scc2698b->write_reg(0x03, data);
	return data;
}
WRITE8_MEMBER(smioc_device::dma8237_2_dmawrite)
{
	LOG_REGISTER_ACCESS("dma2write 0x%x\n", data);
}

