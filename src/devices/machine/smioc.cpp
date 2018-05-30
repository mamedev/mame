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
	map(0x40000, 0x4FFFF).rw(this, FUNC(smioc_device::ram2_mmio_r), FUNC(smioc_device::ram2_mmio_w));
	map(0x50000, 0x5FFFF).rw(this, FUNC(smioc_device::dma68k_r), FUNC(smioc_device::dma68k_w));
	map(0xC0080, 0xC008F).rw("dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Probably RAM DMA
	map(0xC0090, 0xC009F).rw("dma8237_2", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC00A0, 0xC00AF).rw("dma8237_3", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC00B0, 0xC00BF).rw("dma8237_4", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC00C0, 0xC00CF).rw("dma8237_5", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC0100, 0xC011F).rw(this, FUNC(smioc_device::boardlogic_mmio_r), FUNC(smioc_device::boardlogic_mmio_w));
	map(0xC0200, 0xC023F).rw("scc2698b", FUNC(scc2698b_device::read), FUNC(scc2698b_device::write));
	map(0xF8000, 0xFFFFF).rom().region("rom", 0);
}

MACHINE_CONFIG_START(smioc_device::device_add_mconfig)
	/* CPU - Intel 80C188 */
	MCFG_DEVICE_ADD(I188_TAG, I80188, XTAL(20'000'000) / 2) // Clock division unknown
	MCFG_DEVICE_PROGRAM_MAP(smioc_mem)

	/* DMA */
	for (required_device<am9517a_device> &dma : m_dma8237)
		AM9517A(config, dma, 20_MHz_XTAL / 4); // Clock division unknown

	/* RS232 */	
	/* Port 1: Console */
	MCFG_DEVICE_ADD("rs232_p1", RS232_PORT, default_rs232_devices, "terminal")
	for (required_device<rs232_port_device> &rs232_port : m_rs232_p)
		RS232_PORT(config, rs232_port, default_rs232_devices, nullptr);

	

	/* SCC2698B */
	MCFG_DEVICE_ADD("scc2698b", SCC2698B, XTAL(3'686'400))
	MCFG_SCC2698B_TX_CALLBACK(a, WRITELINE("rs232_p1", rs232_port_device, write_txd))
	MCFG_SCC2698B_MPP1_CALLBACK(a, WRITELINE("dma8237_2", am9517a_device, dreq1_w)) // MPP1 output is TxRDY, DREQ1 is UART 0 TX request
	MCFG_SCC2698B_MPP2_CALLBACK(a, WRITELINE("dma8237_2", am9517a_device, dreq0_w)) // MPP2 output is RxRDY, DREQ0 is UART 0 RX request
	MCFG_SCC2698B_TX_CALLBACK(b, WRITELINE("rs232_p2", rs232_port_device, write_txd))
	MCFG_SCC2698B_MPP1_CALLBACK(b, WRITELINE("dma8237_2", am9517a_device, dreq3_w)) 
	MCFG_SCC2698B_MPP2_CALLBACK(b, WRITELINE("dma8237_2", am9517a_device, dreq2_w)) 


	MCFG_DEVICE_MODIFY("dma8237_2")
	MCFG_AM9517A_IN_MEMR_CB(READ8(*this, smioc_device, dma8237_2_dmaread))
	MCFG_AM9517A_OUT_MEMW_CB(WRITE8(*this, smioc_device, dma8237_2_dmawrite))
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(*this, smioc_device, dma8237_2_hreq_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(*this, smioc_device, dma8237_dack_2_0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(*this, smioc_device, dma8237_dack_2_1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(*this, smioc_device, dma8237_dack_2_2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(*this, smioc_device, dma8237_dack_2_3_w))

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
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void smioc_device::device_reset()
{
	/* Reset CPU */
	m_smioccpu->reset();
	m_smioccpu->drq0_w(1);

	/* Resolve callbacks */
	m_m68k_r_cb.resolve_safe(0);
	m_m68k_w_cb.resolve_safe();

	// Attempt to get DMA working by just holding DREQ high for the first dma chip
	//m_dma8237_2->dreq1_w(1);
	//m_dma8237_2->dreq3_w(1);
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
	if (reg != newValue)
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
	case 0xCC2: // Command from 68k?
		data = m_commandValue & 0xFF;
		break;
	case 0xCC3:
		data = m_commandValue >> 8;
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
		update_and_log(m_status2, (m_status2 & 0xFF00) | data, "Status2[40C84]");
		return;
	case 0xC85:
		update_and_log(m_status2, (m_status2 & 0xFF) | (data<<8), "Status2[40C85]");
		return;

	/*
	case 0xC88:
		update_and_log(m_status, (m_status & 0xFF00) | data, "Status[40C88]");
		return;
	case 0xC89:
		update_and_log(m_status, (m_status & 0xFF) | (data<<8), "Status[40C89]");
		return;
	*/
	case 0xCC4:
		update_and_log(m_status, (m_status & 0xFF) | (data<<8), "Status[40CC4]");
		break; // return;
	case 0xCC5:
		update_and_log(m_status, (m_status & 0xFF00) | (data), "Status[40CC5]");
		break; // return;

	}


	logerror("ram2[%04X] <= %02X\n", offset, data);
}

READ8_MEMBER(smioc_device::dma68k_r)
{
	u8 data = 0;

	m_dma_timer->adjust(attotime::from_usec(10));

	data = m_m68k_r_cb(offset);
	logerror("dma68k[%04X] => %02X\n", offset, data);
	return data;
}

WRITE8_MEMBER(smioc_device::dma68k_w)
{

	m_dma_timer->adjust(attotime::from_usec(10));

	m_m68k_w_cb(offset, data);

	logerror("dma68k[%04X] <= %02X\n", offset, data);
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
	logerror("logic[%04X] => %02X\n", offset, data);
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
	logerror("logic[%04X] <= %02X\n", offset, data);
}



// The logic on the SMIOC board somehow proxies the UART's information about what channels are ready into the DMA DREQ lines.
// It's not 100% clear how this works, but a rough guess is it's providing !(RDYN) & (!DACK).
// For now pretend the UART is always ready.
WRITE_LINE_MEMBER(smioc_device::dma8237_dack_2_0_w)
{
	//m_dma8237_2->dreq0_w(1); // Disable channel 0 (UART 0 RX)
}
WRITE_LINE_MEMBER(smioc_device::dma8237_dack_2_1_w)
{
	//m_dma8237_2->dreq1_w(!state); // Uart 0 TX
}
WRITE_LINE_MEMBER(smioc_device::dma8237_dack_2_2_w)
{
	//m_dma8237_2->dreq2_w(1); // Disable channel 2 (UART 1 RX)
}
WRITE_LINE_MEMBER(smioc_device::dma8237_dack_2_3_w)
{
	//m_dma8237_2->dreq3_w(!state);
}

WRITE_LINE_MEMBER(smioc_device::dma8237_2_hreq_w)
{
	//m_dma8237_2->hack_w(state);
}

READ8_MEMBER(smioc_device::dma8237_2_dmaread)
{
	logerror("dma2read\n");
	return 0;
}
WRITE8_MEMBER(smioc_device::dma8237_2_dmawrite)
{
	logerror("dma2write 0x%x\n", data);
}

