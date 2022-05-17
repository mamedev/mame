// license:GPL-2.0+
// copyright-holders:Brandon Munger, Stephen Stair
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

/* Trace information about commands issued to SMIOC */
#define ENABLE_LOG_COMMAND 0
/* Trace accesses to parameter ram (used for DMA parameters from 68k) */
#define ENABLE_LOG_PARAMETER_RAM 0
/* Log DMA related memory movement and access to C01xx register space */
#define ENABLE_LOG_REGISTER_ACCESS 0
/* Trace some accesses to C01xx register space, related to command handling, command/status flow control */
#define ENABLE_LOG_REGISTER_DETAILS 0

#define LOG_COMMAND if(ENABLE_LOG_COMMAND) logerror
#define LOG_PARAMETER_RAM if(ENABLE_LOG_PARAMETER_RAM) logerror
#define LOG_REGISTER_ACCESS if(ENABLE_LOG_REGISTER_ACCESS) logerror
#define LOG_REGISTER_DETAILS if(ENABLE_LOG_REGISTER_DETAILS) logerror

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
	//map(0x40000, 0x4FFFF).rw(FUNC(smioc_device::ram2_mmio_r), FUNC(smioc_device::ram2_mmio_w));
	map(0x40000, 0x4FFFF).rw(FUNC(smioc_device::ram2_mmio_r), FUNC(smioc_device::ram2_mmio_w)); // 4kb of ram in the 0x4xxxx window, mainly used by the board's logic to proxy command parameters and data.
	map(0x50000, 0x5FFFF).rw(FUNC(smioc_device::dma68k_r), FUNC(smioc_device::dma68k_w));
	map(0xC0080, 0xC008F).rw("dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Probably RAM DMA
	map(0xC0090, 0xC009F).rw("dma8237_2", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC00A0, 0xC00AF).rw("dma8237_3", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC00B0, 0xC00BF).rw("dma8237_4", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC00C0, 0xC00CF).rw("dma8237_5", FUNC(am9517a_device::read), FUNC(am9517a_device::write)); // Serial DMA
	map(0xC0100, 0xC01FF).rw(FUNC(smioc_device::boardlogic_mmio_r), FUNC(smioc_device::boardlogic_mmio_w));
	map(0xC0200, 0xC023F).rw("scc2698b", FUNC(scc2698b_device::read), FUNC(scc2698b_device::write));
	map(0xF8000, 0xFFFFF).rom().region("rom", 0);
}

static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_7)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_EVEN)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


void smioc_device::device_add_mconfig(machine_config &config)
{
	/* CPU - Intel 80C188 */
	I80188(config, m_smioccpu, XTAL(20'000'000)); // Clock division unknown
	config.set_maximum_quantum(attotime::from_hz(1000));
	m_smioccpu->set_addrmap(AS_PROGRAM, &smioc_device::smioc_mem);

	/* DMA */
	for (required_device<am9517a_device> &dma : m_dma8237)
		AM9517A(config, dma, 20_MHz_XTAL / 4); // Clock division unknown

	/* RS232 */
	for (required_device<rs232_port_device> &rs232_port : m_rs232_p)
		RS232_PORT(config, rs232_port, default_rs232_devices, nullptr);

	m_rs232_p[0]->set_default_option("terminal");
	m_rs232_p[0]->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	/* SCC2698B */
	scc2698b_device &scc2698b(SCC2698B(config, "scc2698b", XTAL(3'686'400)));
	scc2698b.tx_callback<'a'>().set("rs232_p1", FUNC(rs232_port_device::write_txd));
	scc2698b.mpp1_callback<'a'>().set("dma8237_2", FUNC(am9517a_device::dreq1_w)).invert();
	scc2698b.mpp2_callback<'a'>().set("dma8237_2", FUNC(am9517a_device::dreq0_w)).invert();
	scc2698b.tx_callback<'b'>().set("rs232_p2", FUNC(rs232_port_device::write_txd));
	scc2698b.mpp1_callback<'b'>().set("dma8237_2", FUNC(am9517a_device::dreq3_w)).invert();
	scc2698b.mpp2_callback<'b'>().set("dma8237_2", FUNC(am9517a_device::dreq2_w)).invert();

	/* The first dma8237 is set up in cascade mode, and each of its four channels provides HREQ/HACK to the other 4 DMA controllers*/
	m_dma8237[0]->out_dack_callback<0>().set("dma8237_2", FUNC(am9517a_device::hack_w));
	m_dma8237[0]->out_dack_callback<1>().set("dma8237_3", FUNC(am9517a_device::hack_w));
	m_dma8237[0]->out_dack_callback<2>().set("dma8237_4", FUNC(am9517a_device::hack_w));
	m_dma8237[0]->out_dack_callback<3>().set("dma8237_5", FUNC(am9517a_device::hack_w));
	m_dma8237[0]->out_hreq_callback().set("dma8237_1", FUNC(am9517a_device::hack_w));

	/* Connect base DMA controller's Hold Request to its own Hold ACK
	    The CPU doesn't support hold request / hold ack pins, so we will pretend that the DMA controller immediately gets what it wants every time it asks.
	    This will keep things moving forward, but the CPU will continue going through DMA requests rather than halting for a few cycles each time.
	    It shouldn't cause a problem though.
	    */

	m_dma8237[1]->in_memr_callback().set(FUNC(smioc_device::dma8237_2_dmaread));
	m_dma8237[1]->out_memw_callback().set(FUNC(smioc_device::dma8237_2_dmawrite));
	m_dma8237[1]->out_hreq_callback().set("dma8237_1", FUNC(am9517a_device::dreq0_w));

	m_rs232_p[0]->rxd_handler().set("scc2698b", FUNC(scc2698b_device::port_a_rx_w));
	m_rs232_p[1]->rxd_handler().set("scc2698b", FUNC(scc2698b_device::port_b_rx_w));

}

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
	m_rs232_p(*this, "rs232_p%u", 1),
	m_scc2698b(*this, "scc2698b"),
	m_smioc_ram(*this, "smioc_ram"),
	m_dma_timer(nullptr),
	m_451_timer(nullptr),
	m_m68k_r_cb(*this),
	m_m68k_w_cb(*this)
{


}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void smioc_device::device_start()
{
	m_dma_timer = timer_alloc(FUNC(smioc_device::raise_drq), this);
	m_451_timer = timer_alloc(FUNC(smioc_device::raise_int1), this);

	/* Resolve callbacks */
	m_m68k_r_cb.resolve_safe(0);
	m_m68k_w_cb.resolve_safe();

	m_451_timer->adjust(attotime::from_msec(200), 0, attotime::from_msec(200));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void smioc_device::device_reset()
{
	/* Reset CPU */
	m_smioccpu->reset();
	m_smioccpu->drq0_w(1);
	m_deviceBusy = 1;


}

void smioc_device::SoftReset()
{
	m_smioccpu->reset();
	m_smioccpu->drq0_w(1);
	m_deviceBusy = 1;
}

TIMER_CALLBACK_MEMBER(smioc_device::raise_drq)
{
	// DMA Timer
	m_smioccpu->drq0_w(1);
}

TIMER_CALLBACK_MEMBER(smioc_device::raise_int1)
{
	// 451 emulation timer - Trigger the SMIOC to read from C0180 and store data
	m_smioccpu->int1_w(CLEAR_LINE);
	m_smioccpu->int1_w(HOLD_LINE);
}

void smioc_device::SendCommand(u16 command)
{
	LOG_COMMAND("%s SMIOC Command %04x (Address %04x, Length %04x)\n", machine().time().as_string(), command, ReadDmaParameter(smiocdma_sendaddress), ReadDmaParameter(smiocdma_sendlength));

	// Command 0xxx is a special value that tells the hardware what channel/port it's working with for the purpose of writing DMA base / length registers
	if (command < 0x1000)
	{
		m_activePortIndex = command & 7;
		return;
	}

	m_commandValue = command;
	m_requestFlags_11D |= 1;
	m_deviceBusy = 1;

	// Invalidate status if we hit a command.
	m_status = 0;
	m_statusvalid = false;
	m_enable_hacky_status = false;

	m_smioccpu->int2_w(CLEAR_LINE);
	m_smioccpu->int2_w(HOLD_LINE);

}

void smioc_device::SendCommand2(u16 command)
{
	LOG_COMMAND("%s SMIOC Command2 %04x (Address %04x, Length %04x)\n", machine().time().as_string(), command, ReadDmaParameter(smiocdma_sendaddress), ReadDmaParameter(smiocdma_sendlength));

	m_commandValue2 = command;
	m_requestFlags_11D |= 4;
	m_deviceBusy = 1;

	m_smioccpu->int2_w(CLEAR_LINE);
	m_smioccpu->int2_w(HOLD_LINE);

}
void smioc_device::SetCommandParameter(u16 parameter)
{
	WriteRamParameter("SetCommandParameter", "Parameter", 0xCC6, parameter);
	m_requestFlags_11D |= 0x40;
	m_smioccpu->int2_w(CLEAR_LINE);
	m_smioccpu->int2_w(HOLD_LINE);
}
void smioc_device::SetCommandParameter2(u16 parameter)
{
	WriteRamParameter("SetCommandParameter2", "Parameter", 0xB86, parameter);
	m_requestFlags_11D |= 0x80;
	m_smioccpu->int2_w(CLEAR_LINE);
	m_smioccpu->int2_w(HOLD_LINE);
}



u16 smioc_device::GetStatus()
{
	return m_status | 0x0008;
}
u16 smioc_device::GetStatus2()
{
	return m_status2 | 0x0008;
}


void smioc_device::ClearStatus()
{
	m_status = 0;
	m_statusvalid = false;
	if (m_statusrequest)
	{
		AdvanceStatus();
	}
}
void smioc_device::ClearStatus2()
{
	m_status2 = 0;
	m_statusvalid2 = false;
	if (m_statusrequest2)
	{
		AdvanceStatus2();
	}
}

void smioc_device::ClearParameter()
{
	// Indicate to SMIOC that parameter has been read.
	m_requestFlags_11D |= 0x02; // bit 1 - E2E 0x800
	m_smioccpu->int2_w(HOLD_LINE);

	m_wordcount = 0;
}
void smioc_device::ClearParameter2()
{
	// Indicate to SMIOC that alternate endpoint parameter has been read.
	m_requestFlags_11D |= 0x08; // bit 3 - E2E 0x400
	m_smioccpu->int2_w(HOLD_LINE);

	m_wordcount2 = 0;
}
void smioc_device::SetDmaParameter(smioc_dma_parameter_t param, u16 value)
{
	int address = DmaParameterAddress(param);

	static char const *const paramNames[] = { "smiocdma_sendaddress", "smiocdma_sendlength", "smiocdma_recvaddress", "smiocdma_recvlength" };
	const char* paramName = "?";
	if (param >= 0 && param < (sizeof(paramNames) / sizeof(*paramNames)))
	{
		paramName = paramNames[param];
	}

	WriteRamParameter("SetDmaParameter", paramName, address, value);
}

u16 smioc_device::ReadDmaParameter(smioc_dma_parameter_t param)
{
	int address = DmaParameterAddress(param);
	return m_logic_ram[address] | (m_logic_ram[address + 1] << 8);
}

int smioc_device::DmaParameterAddress(smioc_dma_parameter_t param)
{
	int baseAddress = -1;
	int p4offset = 0xC0; // The address offset from port 0 to port 4
	switch (param)
	{
	case smiocdma_sendaddress: // Send to SMIOC - For Serial TX data
		baseAddress = 0xCD8;
		break;

	case smiocdma_sendlength:
		baseAddress = 0xCE8;
		break;

	case smiocdma_recvaddress: // Recv from SMIOC - For Serial RX data
		baseAddress = 0xCF0;
		break;

	case smiocdma_recvlength:
		baseAddress = 0xCF8;
		break;
	}
	if (baseAddress != -1)
	{
		int portOffset = (m_activePortIndex & 1) * 4 + (m_activePortIndex & 2) + ((m_activePortIndex & 4) ? p4offset : 0);

		int address = baseAddress + portOffset;

		return address;
	}
	else
	{
		fatalerror("Invalid DMA Parameter 0x%x - unable to continue.", param);
	}
}

void smioc_device::WriteRamParameter(const char* function, const char* register_name, int address, int value)
{
	if (address < 0 || address > 0xFFE)
	{
		logerror("%s Invalid Parameter Ram write (%s %s %04x %04x)\n", machine().time().as_string(), function, register_name, address, value);
		return;
	}
	m_logic_ram[address] = value & 0xFF;
	m_logic_ram[address + 1] = (value >> 8) & 0xFF;

	LOG_PARAMETER_RAM("%s %s (%s) ram2[0x%04x] = %04X\n", machine().time().as_string(), function, register_name, address, value);
}




void smioc_device::update_and_log(u16& reg, u16 newValue, const char* register_name)
{
	LOG_PARAMETER_RAM("%s Update %s %04X -> %04X\n", machine().time().as_string(), register_name, reg, newValue);
	reg = newValue;
}

u8 smioc_device::ram2_mmio_r(offs_t offset)
{
	const char *description = "";
	u8 data = m_logic_ram[offset & 0xFFF];
	switch (offset)
	{

	case 0xB82: // Command 2 - C011D Bit 2
		data = m_commandValue2 & 0xFF;
		description = "(Command2)";
		break;
	case 0xB83:
		data = m_commandValue2 >> 8;
		description = "(Command2)";
		break;

	case 0xB86: // Command parameter 2 - C011D Bit 7
		description = "(Command Parameter 2)";
		break;
	case 0xB87:
		description = "(Command Parameter 2)";
		break;

	case 0xCC2: // Command - C011D Bit 0
		data = m_commandValue & 0xFF;
		description = "(Command)";
		break;
	case 0xCC3:
		data = m_commandValue >> 8;
		description = "(Command)";
		break;

	case 0xCC6: // Command parameter - C011D Bit 6
		description = "(Command Parameter)";
		break;
	case 0xCC7:
		description = "(Command Parameter)";
		break;

	case 0xCD8: // DMA source address (for writing characters) - Port 0
		description = "(DMA Source)";
		break;
	case 0xCD9:
		description = "(DMA Source)";
		break;

	case 0xCE8: // DMA Length (for writing characters) - Port 0
		description = "(DMA Length)";
		break;
	case 0xCE9:
		description = "(DMA Length)";
		break;

	}
	LOG_PARAMETER_RAM("ram2[%04X] => %02X %s\n", offset, data, description);
	return data;
}

void smioc_device::ram2_mmio_w(offs_t offset, u8 data)
{
	const char *description = "";

	m_logic_ram[offset & 0xFFF] = data;

	switch (offset) // Offset based on C0100 register base
	{
	case 0x11D:
		// This value is written after the CPU resets, it may be accessible to the 68k via some register but that is not clear.
		description = "(Indicate Reset)";
		break;

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

	case 0xB84:
		update_and_log(m_shadowstatus2, (m_shadowstatus2 & 0xFF00) | (data), "Status2[40B84]");
		return;
	case 0xB85:
		update_and_log(m_shadowstatus2, (m_shadowstatus2 & 0xFF) | (data << 8), "Status2[40B85]");
		m_status2 |= 0x40;
		return;

	case 0xCC4:
		update_and_log(m_shadowstatus, (m_shadowstatus & 0xFF00) | (data), "Status[40CC4]");
		return;
	case 0xCC5:
		update_and_log(m_shadowstatus, (m_shadowstatus & 0xFF) | (data << 8), "Status[40CC5]");
		return;

	case 0xCC8:
		update_and_log(m_wordcount, (m_wordcount & 0xFF00) | (data), "Wordcount[40CC8]");
		return;
	case 0xCC9:
		update_and_log(m_wordcount, (m_wordcount & 0xFF) | (data << 8), "Wordcount[40CC9]");
		return;

	case 0xB88:
		update_and_log(m_wordcount, (m_wordcount & 0xFF00) | (data), "Wordcount2[40B88]");
		return;
	case 0xB89:
		update_and_log(m_wordcount, (m_wordcount & 0xFF) | (data << 8), "Wordcount2[40CC9]");
		return;
	}

	LOG_PARAMETER_RAM("ram2[%04X] <= %02X %s\n", offset, data, description);
}

u8 smioc_device::dma68k_r(offs_t offset)
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
	LOG_REGISTER_ACCESS("%s dma68k[%04X] => %02X\n", machine().time().as_string(), offset, data);
	return data;
}

void smioc_device::dma68k_w(offs_t offset, u8 data)
{

	m_dma_timer->adjust(attotime::from_usec(10));

	m_m68k_w_cb(offset, data);

	LOG_REGISTER_ACCESS("%s dma68k[%04X] <= %02X\n", machine().time().as_string(), offset, data);
}

u8 smioc_device::boardlogic_mmio_r(offs_t offset)
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
		LOG_REGISTER_DETAILS("%s C011D Read => %02X\n", machine().time().as_string(), data);
		// Assume this is a clear-on-read register - It is read in one location and all set bits are acted on once it is read.
		m_requestFlags_11D = 0;

		break;

	case 0x1F: // C011F (HW Status flags?)
		// 0x80, 0x40 seem to be HW request to cancel ongoing DMA requests, maybe related to board reset?
		// 0x01 - When this is 0, advance some state perhaps related to talking to the 8051
		data = 0xFF;
		break;

	case 0x80: // C0180 - Related to talking to the 87C451 chip interfacing to the breakout box
		data = 0x05; // Hack hack
		// We need the 8051 to provide a pair of bytes with 0x05 followed by a byte with the bottom bit set.
		// For now 0x05 should work to provide this condition, and might make the required status work.
		break;
	}
	LOG_REGISTER_ACCESS("logic[%04X] => %02X\n", offset, data);
	return data;
}

void smioc_device::boardlogic_mmio_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0x10: // C0110 (Clear interrupt? This seems to happen a lot but without being related to actually completing anything.)
		LOG_REGISTER_DETAILS("%s C0110 Write, DeviceBusy = %02X, RequestFlags = %02X\n", machine().time().as_string(), m_deviceBusy, m_requestFlags_11D);
		m_deviceBusy = m_requestFlags_11D;
		m_smioccpu->int2_w(CLEAR_LINE);
		if (m_requestFlags_11D)
		{
			m_smioccpu->int2_w(HOLD_LINE);
		}
		break;

	case 0x11: // C0111 - Set to 1 after providing a status - Acknowledge by hardware by raising bit 4 in C011D (SMIOC E2E flag 0x200
		// Hypothesis: Status needs to be acknowledged before we trigger the interrupt - so moving the following code to clear status callback.

		m_statusrequest = true;
		if (!m_statusvalid)
		{
			// Immediately complete status write.
			AdvanceStatus();
		}
		else
		{
			// Wait until host clears status to proceed.
		}

		LOG_REGISTER_DETAILS("%s C0111 Write, RequestFlags = %02X\n", machine().time().as_string(), m_requestFlags_11D);
		break;

	case 0x12: // C0112 - Set to 1 after providing a status(2?) - Acknowledge by hardware by raising bit 5 in C011D (SMIOC E2E flag 0x100)
		m_statusrequest2 = true;
		if (!m_statusvalid2)
		{
			AdvanceStatus2();
		}
		LOG_REGISTER_DETAILS("%s C0112 Write, RequestFlags = %02X\n", machine().time().as_string(), m_requestFlags_11D);
		break;

	case 0x16: // C0116 - Set to 1 after processing 11D & 0x40
		break;
	case 0x17: // C0117 - Set to 1 after processing 11D & 0x80
		break;

	}
	LOG_REGISTER_ACCESS("logic[%04X] <= %02X\n", offset, data);
}

void smioc_device::AdvanceStatus()
{
	m_status = m_shadowstatus | 0x0040;
	m_statusvalid = true;
	m_enable_hacky_status = true;
	if (m_statusrequest)
	{
		m_requestFlags_11D |= 0x10; // bit 4
		m_smioccpu->int2_w(CLEAR_LINE);
		m_smioccpu->int2_w(HOLD_LINE);
	}
	m_statusrequest = false;
}
void smioc_device::AdvanceStatus2()
{
	m_status2 = m_shadowstatus2 | 0x0040;
	m_statusvalid2 = true;
	if (m_statusrequest2)
	{
		m_requestFlags_11D |= 0x20; // bit 5
		m_smioccpu->int2_w(CLEAR_LINE);
		m_smioccpu->int2_w(HOLD_LINE);
	}
	m_statusrequest2 = false;
}


u8 smioc_device::dma8237_2_dmaread(offs_t offset)
{
	int data = m_smioccpu->space(AS_PROGRAM).read_byte(offset);
	LOG_REGISTER_ACCESS("dma2read [0x%x] => 0x%x\n", offset, data);
	m_scc2698b->write(0x03, data);
	return data;
}
void smioc_device::dma8237_2_dmawrite(offs_t offset, u8 data)
{
	data = m_scc2698b->read(0x03);
	LOG_REGISTER_ACCESS("dma2write [0x%x] <= 0x%x\n", offset, data);
	m_smioccpu->space(AS_PROGRAM).write_byte(offset, data);
}

