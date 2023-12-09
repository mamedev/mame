// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

/*
 * Silicon Graphics High Performance Peripheral Controller (HPC1).
 *
 * TODO:
 *  - hpc1.5 (second-generation, includes little-endian support)
 *  - clean up ethernet interface, interrupt delay
 *  - pbus and dsp
 */

#include "emu.h"
#include "hpc1.h"

#define LOG_SCSI        (1U << 1)
#define LOG_ENET        (1U << 2)

//#define VERBOSE         (LOG_SCSI | LOG_ENET)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_HPC1, hpc1_device, "hpc1", "SGI HPC1")

enum enet_rxs_mask : u32
{
	ENET_RXSTAT = 0x0000'bf00, // receive status
	ENET_SRXDMA = 0x0000'4000, // start receiver dma
};

enum enet_txs_mask : u32
{
	ENET_TXSTAT = 0x00bf'0000, // transmit status
	ENET_STXDMA = 0x0040'0000, // start transmitter dma
};

enum enet_ctrl_mask : u32
{
	ENET_RESET   = 0x01, // channel reset
	ENET_CLRINT  = 0x02, // interrupt pending
	ENET_MODNORM = 0x04, // 0=loopback
	ENET_RBO     = 0x08, // receive buffer overflow
};

enum scsi_ctrl_mask : u32
{
	SCSI_RESET    = 0x01,
	SCSI_FLUSH    = 0x02,
	SCSI_TO_MEM   = 0x10,
	SCSI_STARTDMA = 0x80,
};

enum scsi_cbp_mask : u32
{
	SCSI_CBP_BUFADDR = 0x0fff'ffff,
	SCSI_CBP_EOX     = 0x8000'0000,
};

hpc1_device::hpc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_HPC1, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_pbus("pbus", ENDIANNESS_BIG, 16, 20, -1, address_map_constructor(FUNC(hpc1_device::pbus_map), this))
	, m_gio(*this, finder_base::DUMMY_TAG, -1)
	, m_enet(*this, finder_base::DUMMY_TAG)
	, m_int_w(*this)
	, m_eeprom_dati(*this, 0)
	, m_dma_r(*this, 0)
	, m_dma_w(*this)
	, m_eeprom_out(*this)
{
}

void hpc1_device::device_start()
{
	save_item(NAME(m_drq));

	save_item(NAME(m_enet_cxbp));
	save_item(NAME(m_enet_nxbdp));
	save_item(NAME(m_enet_xbc));
	save_item(NAME(m_enet_cxbdp));
	save_item(NAME(m_enet_cpfxbdp));
	save_item(NAME(m_enet_ppfxbdp));
	save_item(NAME(m_enet_intdelay));
	save_item(NAME(m_enet_txs));
	save_item(NAME(m_enet_rxs));
	save_item(NAME(m_enet_ctrl));
	save_item(NAME(m_enet_rbc));
	save_item(NAME(m_enet_crbp));
	save_item(NAME(m_enet_nrbdp));
	save_item(NAME(m_enet_crbdp));

	save_item(NAME(m_scsi_bc));
	save_item(NAME(m_scsi_cbp));
	save_item(NAME(m_scsi_nbdp));
	save_item(NAME(m_scsi_ctrl));

	save_item(NAME(m_dsp_bc));

	save_item(NAME(m_aux));

	m_drq = 0;
}

void hpc1_device::device_reset()
{
	m_enet_rxs = 0;
	m_enet_txs = 0;
	m_enet_ctrl = 0;

	m_scsi_nbdp = 0;
	m_scsi_ctrl = 0;
}

device_memory_interface::space_config_vector hpc1_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_pbus) };
}

void hpc1_device::map(address_map &map)
{
	// xcount
	map(0x000c, 0x000f).rw(FUNC(hpc1_device::cxbp_r), FUNC(hpc1_device::cxbp_w));
	map(0x0010, 0x0013).rw(FUNC(hpc1_device::nxbdp_r), FUNC(hpc1_device::nxbdp_w));
	map(0x0014, 0x0017).rw(FUNC(hpc1_device::xbc_r), FUNC(hpc1_device::xbc_w));
	// xpntr
	// xfifo
	map(0x0020, 0x0023).rw(FUNC(hpc1_device::cxbdp_r), FUNC(hpc1_device::cxbdp_w));
	map(0x0024, 0x0027).rw(FUNC(hpc1_device::cpfxbdp_r), FUNC(hpc1_device::cpfxbdp_w));
	map(0x0028, 0x002b).rw(FUNC(hpc1_device::ppfxbdp_r), FUNC(hpc1_device::ppfxbdp_w));
	map(0x002c, 0x002f).rw(FUNC(hpc1_device::intdelay_r), FUNC(hpc1_device::intdelay_w));
	//
	map(0x0034, 0x0037).rw(FUNC(hpc1_device::trstat_r), FUNC(hpc1_device::trstat_w));
	map(0x0038, 0x003b).rw(FUNC(hpc1_device::rcvstat_r), FUNC(hpc1_device::rcvstat_w));
	map(0x003c, 0x003f).rw(FUNC(hpc1_device::ctl_r), FUNC(hpc1_device::ctl_w));
	//
	// rcount
	map(0x0048, 0x004b).rw(FUNC(hpc1_device::rbc_r), FUNC(hpc1_device::rbc_w));
	map(0x004c, 0x004f).rw(FUNC(hpc1_device::crbp_r), FUNC(hpc1_device::crbp_w));
	map(0x0050, 0x0053).rw(FUNC(hpc1_device::nrbdp_r), FUNC(hpc1_device::nrbdp_w));
	map(0x0054, 0x0057).rw(FUNC(hpc1_device::crbdp_r), FUNC(hpc1_device::crbdp_w));
	// rpntr
	// rfifo

	//map(0x0084, 0x0087).rw(FUNC(hpc1_device::scsi_count_r), FUNC(hpc1_device::scsi_count_w));
	map(0x0088, 0x008b).rw(FUNC(hpc1_device::scsi_bc_r), FUNC(hpc1_device::scsi_bc_w));
	map(0x008c, 0x008f).rw(FUNC(hpc1_device::scsi_cbp_r), FUNC(hpc1_device::scsi_cbp_w));
	map(0x0090, 0x0093).rw(FUNC(hpc1_device::scsi_nbdp_r), FUNC(hpc1_device::scsi_nbdp_w));
	map(0x0094, 0x0097).rw(FUNC(hpc1_device::scsi_ctrl_r), FUNC(hpc1_device::scsi_ctrl_w));
	//map(0x0098, 0x009b); // scsi pointer register
	//map(0x009c, 0x009f); // scsi fifo data register

	map(0x00a8, 0x00af).ram(); // parallel
	//map(0x00a8, 0x00ab); // parallel byte count and ie (bc)
	//map(0x00ac, 0x00af); // parallel current buf ptr (cbp)
	//map(0x00b0, 0x00b3); // parallel next buffer descriptor (nbdp)
	//map(0x00b4, 0x00b7); // parallel control (ctrl)

	map(0x0180, 0x01b7).ram(); // dsp interface
	map(0x0180, 0x0183).rw(FUNC(hpc1_device::dsp_bc_r), FUNC(hpc1_device::dsp_bc_w)).umask32(0xffff);

	map(0x01bc, 0x01bf).rw(FUNC(hpc1_device::aux_r), FUNC(hpc1_device::aux_w)).umask32(0xff);
}

void hpc1_device::pbus_map(address_map &map)
{
/*
 * Peripheral Bus (P-Bus): 20 bit address, 16-bit data
 *  - PROM
 *  - DUARTS
 *  - Real Time Clock
 *  - DSP (P-Bus master)
 *  - Audio SRAM
 * Ethernet
 * SCSI
 * Parallel
 * INT2
 *
 * hpc1 pbus has 20 bit address space (1MB)
 * 0x1fb8'0000-0x1fbf'ffff hpc i/o space  (512KB)
 * 0x1fc0'0000-0x1fc3'ffff hpc prom space (256KB)
 *
 * Indigo PROM is 256k, dsp ram is 128k

    DSP Y bus is 24 bit data, 16 bit address, word addressed
    PBUS is 16 bit data, 20 bit address, word addressed

    #define HPC1DMAWDCNT    0x1fb80180  // DMA transfer size (SRAM words)
    #define HPC1GIOADDL     0x1fb80184  // GIO-bus address, LSB (16 bit)
    #define HPC1GIOADDM     0x1fb80188  // GIO-bus address, MSB (16 bit)
    #define HPC1PBUSADD     0x1fb8018c  // PBUS address (16 bit)
    #define HPC1DMACTRL     0x1fb80190  // DMA Control (2 bit)
    #define HPC1COUNTER     0x1fb80194  // Counter (24 bits) (ro)
    #define HPC1HANDTX      0x1fb80198  // Handshake transmit (16 bit)
    #define HPC1HANDRX      0x1fb8019c  // Handshake receive (16 bit)
    #define HPC1CINTSTAT    0x1fb801a0  // CPU Interrupt status (3 bit)
    #define HPC1CINTMASK    0x1fb801a4  // CPU Interrupt masks (3 bit)
    #define HPC1MISCSR      0x1fb801b0  // Misc. control and status (8 bit)
    #define HPC1BURSTCTL    0x1fb801b4  // DMA Ballistics register (16 bit)

    ; DSP-HPC addresses
    DSP_HPC_BYTECNT     equ y:$ffe0     ; DMA transfer size
    DSP_HPC_GIOADDL     equ y:$ffe1     ; GIO-bus address, LSB
    DSP_HPC_GIOADDM     equ y:$ffe2     ; GIO-bus address, MSB
    DSP_HPC_PBUSADD     equ y:$ffe3     ; Pbus address
    DSP_HPC_DMACTRL     equ y:$ffe4     ; DMA control
    DSP_HPC_HANDTX      equ y:$ffe6     ; handshake transmit
    DSP_HPC_HANDRX      equ y:$ffe7     ; handshake receive
    DSP_HPC_INTSTAT     equ y:$ffe8     ; interrupt status
    DSP_HPC_INTMASK     equ y:$ffe9     ; interrupt mask
    DSP_HPC_INTCONF     equ y:$ffea     ; interrupt configuration
    DSP_HPC_INTPOL      equ y:$ffeb     ; interrupt polarity
    DSP_HPC_MISCSR      equ y:$ffec     ; miscellaneous control and status
    DSP_HPC_BURSTCTL    equ y:$ffed     ; dma burst control register

    # p: == x:
    p:8000-8fff == x:8000-8fff
    p:e000-efff == x:e000-efff

    # p: ^ 0x4000 == y:
    p:8000-8fff == y:c000-cfff
      9=d
      a=e
      b=f
      c=8
      d=9
    p:e000-efff == y:a000-afff
      f=b

    HEADPHONE_MDAC_L equ        y:$fffc
    HEADPHONE_MDAC_R equ        y:$fffd

    32k x 24bit dsp ram
    ram at c000
*/
	map(0xffe0, 0xffe0).rw(FUNC(hpc1_device::dsp_bc_r), FUNC(hpc1_device::dsp_bc_w));
}

void hpc1_device::scsi_nbdp_w(u32 data)
{
	LOGMASKED(LOG_SCSI, "nbdp_w 0x%08x\n", data);
	m_scsi_nbdp = data & 0x0fff'ffffU;

	scsi_chain();
}

void hpc1_device::scsi_ctrl_w(u32 data)
{
	m_scsi_ctrl = data & ~(SCSI_FLUSH | SCSI_RESET);

	if (BIT(m_drq, 0) && (m_scsi_ctrl & SCSI_STARTDMA))
		scsi_dma();
}

void hpc1_device::scsi_chain()
{
	u32 const bdp = m_scsi_nbdp;

	m_scsi_bc = m_gio->read_dword(bdp + 0) & 0x1fffU;
	m_scsi_cbp = m_gio->read_dword(bdp + 4) & (SCSI_CBP_EOX | SCSI_CBP_BUFADDR);
	m_scsi_nbdp = m_gio->read_dword(bdp + 8) & 0x0fff'ffffU;

	LOGMASKED(LOG_SCSI, "bdp 0x%08x bc 0x%04x cbp 0x%08x nbdp 0x%08x\n", bdp, m_scsi_bc, m_scsi_cbp, m_scsi_nbdp);
}

template <unsigned N> void hpc1_device::write_drq(int state)
{
	if (state)
	{
		m_drq |= 1U << N;

		if (N == 0 && (m_scsi_ctrl & SCSI_STARTDMA))
			scsi_dma();
		else if (N == 1)
			enet_dma();
	}
	else
		m_drq &= ~(1U << N);
}

template void hpc1_device::write_drq<0>(int state);
template void hpc1_device::write_drq<1>(int state);

void hpc1_device::write_int(int state)
{
	u8 const rxs = m_enet->read(6);
	u8 const txs = m_enet->read(7);

	// update status registers
	m_enet_rxs = (m_enet_rxs & ~ENET_RXSTAT) | ((u32(rxs) << 8) & ENET_RXSTAT);
	m_enet_txs = (m_enet_txs & ~ENET_TXSTAT) | ((u32(txs) << 16) & ENET_TXSTAT);

	LOGMASKED(LOG_ENET, "write_int %d rxs 0x%02x txs 0x%02x\n", state, rxs, txs);

	if (state)
	{
		bool interrupt = false;

		// receive interrupt
		if (BIT(m_enet_rxs, 8, 6))
		{
			// seeq receive fifo overflow or receive buffer overflow?
			if (BIT(m_enet_rxs, 8) || (m_enet_ctrl & ENET_RBO))
				m_enet_rxs &= ~ENET_SRXDMA;

			interrupt = true;
		}

		// transmit interrupt
		if (BIT(m_enet_txs, 16, 3) || (BIT(m_enet_txs, 19) && BIT(m_enet_xbc, 15)))
		{
			m_enet_txs &= ~ENET_STXDMA;

			interrupt = true;
		}

		// TODO: interrupt delay
		if (interrupt)
		{
			m_enet_ctrl |= ENET_CLRINT;
			m_int_w(1);
		}
	}
}

void hpc1_device::scsi_dma()
{
	if (m_scsi_bc)
	{
		if (m_scsi_ctrl & SCSI_TO_MEM)
			m_gio->write_byte(m_scsi_cbp & SCSI_CBP_BUFADDR, m_dma_r[0]());
		else
			m_dma_w[0](m_gio->read_byte(m_scsi_cbp &SCSI_CBP_BUFADDR));

		m_scsi_cbp++;
		m_scsi_bc--;

		if (m_scsi_bc == 0)
		{
			if (m_scsi_cbp & SCSI_CBP_EOX)
				m_scsi_ctrl &= ~SCSI_STARTDMA;
			else
				scsi_chain();
		}
	}
}

void hpc1_device::enet_dma()
{
	// transmit
	while (m_enet_txs & ENET_STXDMA)
	{
		m_enet_xbc = m_gio->read_dword(BIT(m_enet_cxbdp, 0, 30) + 0);
		m_enet_cxbp = m_gio->read_dword(BIT(m_enet_cxbdp, 0, 30) + 4);
		m_enet_nxbdp = m_gio->read_dword(BIT(m_enet_cxbdp, 0, 30) + 8);

		LOGMASKED(LOG_ENET, "transmit cxbdp 0x%08x cxbp 0x%08x xbc 0x%08x count %d nxbdp 0x%08x\n",
			m_enet_cxbdp, m_enet_cxbp, m_enet_xbc, BIT(m_enet_xbc, 0, 13), m_enet_nxbdp);
		for (unsigned i = 0; i < BIT(m_enet_xbc, 0, 13); i++)
			m_dma_w[1](m_gio->read_byte(BIT(m_enet_cxbp, 0, 28) + i));

		// eoxp: end of packet
		if (BIT(m_enet_xbc, 31))
		{
			LOGMASKED(LOG_ENET, "transmit end of packet\n");
			m_enet->txeof_w(1);
		}

		// eox: last descriptor
		if (BIT(m_enet_cxbp, 31))
		{
			LOGMASKED(LOG_ENET, "transmit stop\n");
			m_enet_txs &= ~ENET_STXDMA;
		}
		else
			m_enet_cxbdp = m_enet_nxbdp;
	}

	// receive
	while (m_enet_rxs & ENET_SRXDMA)
	{
		// check rxrdy
		if (!BIT(m_drq, 1))
			break;

		m_enet_crbdp = m_enet_nrbdp;

		m_enet_rbc = m_gio->read_dword(BIT(m_enet_crbdp, 0, 30) + 0);
		m_enet_crbp = m_gio->read_dword(BIT(m_enet_crbdp, 0, 30) + 4);
		m_enet_nrbdp = m_gio->read_dword(BIT(m_enet_crbdp, 0, 30) + 8);

		LOGMASKED(LOG_ENET, "receive crbdp 0x%08x crbp 0x%08x rbc 0x%08x count %d nrbdp 0x%08x\n",
			m_enet_crbdp, m_enet_crbp, m_enet_rbc, BIT(m_enet_rbc, 0, 13), m_enet_nrbdp);

		// rown?
		if (BIT(m_enet_rbc, 31))
		{
			unsigned count = 2;
			while (!m_enet->rxeof_r())
				m_gio->write_byte(BIT(m_enet_crbp, 0, 28) + count++, m_dma_r[1]());

			// FIXME: store rxstatus byte in receive buffer
			logerror("rx status 0x%02x\n", (m_enet_rxs & ENET_RXSTAT) >> 8);
			m_gio->write_byte(BIT(m_enet_crbp, 0, 28) + count++, 0x30);

			LOGMASKED(LOG_ENET, "receive data length %d\n", count - 3);
			m_gio->write_dword(BIT(m_enet_crbdp, 0, 28) + 0, BIT(m_enet_rbc, 0, 13) - count);
		}

		// eor: last descriptor
		if (BIT(m_enet_crbp, 31))
		{
			LOGMASKED(LOG_ENET, "receive stop\n");
			m_enet_rxs &= ~ENET_SRXDMA;
		}
	}
}

u32 hpc1_device::trstat_r()
{
	LOGMASKED(LOG_ENET, "trstat_r 0x%08x\n", m_enet_txs);

	return m_enet_txs;
}

u32 hpc1_device::rcvstat_r()
{
	LOGMASKED(LOG_ENET, "rcvstat_r 0x%08x\n", m_enet_rxs);

	return m_enet_rxs;
}

u8 hpc1_device::aux_r()
{
	if (m_eeprom_dati())
		return m_aux | 0x10;
	else
		return m_aux & ~0x10;
}

void hpc1_device::cxbp_w(u32 data)
{
	LOGMASKED(LOG_ENET, "cxbp_w 0x%08x\n", data);
	m_enet_cxbp = data;
}

void hpc1_device::nxbdp_w(u32 data)
{
	LOGMASKED(LOG_ENET, "nxbdp_w 0x%08x\n", data);
	m_enet_nxbdp = data;
}

void hpc1_device::xbc_w(u32 data)
{
	LOGMASKED(LOG_ENET, "xbc_w 0x%08x\n", data);
	m_enet_xbc = data;
}

void hpc1_device::cxbdp_w(u32 data)
{
	LOGMASKED(LOG_ENET, "cxbdp_w 0x%08x\n", data);
	m_enet_cxbdp = data;
}

void hpc1_device::cpfxbdp_w(u32 data)
{
	LOGMASKED(LOG_ENET, "cpfxbdp_w 0x%08x\n", data);
	m_enet_cpfxbdp = data;
}

void hpc1_device::ppfxbdp_w(u32 data)
{
	LOGMASKED(LOG_ENET, "ppfxbdp_w 0x%08x\n", data);
	m_enet_ppfxbdp = data;
}

void hpc1_device::intdelay_w(u32 data)
{
	LOGMASKED(LOG_ENET, "intdelay_w 0x%08x\n", data);
	m_enet_intdelay = data;
}

void hpc1_device::trstat_w(u32 data)
{
	LOGMASKED(LOG_ENET, "trstat_w 0x%08x (%s)\n", data, machine().describe_context());
	m_enet_txs = (m_enet_txs & ~ENET_STXDMA) | (data & ENET_STXDMA);

	if (data & ENET_STXDMA)
		enet_dma();
}

void hpc1_device::rcvstat_w(u32 data)
{
	LOGMASKED(LOG_ENET, "rcvstat_w 0x%08x\n", data);
	m_enet_rxs = (m_enet_rxs & ~ENET_SRXDMA) | (data & ENET_SRXDMA);

	if (data & ENET_SRXDMA)
		enet_dma();
}


void hpc1_device::ctl_w(u32 data)
{
	LOGMASKED(LOG_ENET, "ctl_w 0x%08x (%s)\n", data, machine().describe_context());

	// reset
	if (m_enet)
		m_enet->reset_w(!(data & ENET_RESET));

	// clear interrupt
	if (data & ENET_CLRINT)
	{
		m_enet_ctrl &= ~ENET_CLRINT;
		m_int_w(0);
	}

	// loopback
	if (m_enet)
		m_enet->set_loopback(!(data & ENET_MODNORM));

	// receive buffer overflow
	if (data & ENET_RBO)
		m_enet_ctrl &= ~ENET_RBO;

	m_enet_ctrl = (m_enet_ctrl & ~(ENET_MODNORM | ENET_RESET)) | (data & (ENET_MODNORM | ENET_RESET));
}

void hpc1_device::rbc_w(u32 data)
{
	LOGMASKED(LOG_ENET, "rbc_w 0x%08x\n", data);
	m_enet_rbc = data;
}

void hpc1_device::crbp_w(u32 data)
{
	LOGMASKED(LOG_ENET, "crbp_w 0x%08x\n", data);
	m_enet_crbp = data;
}

void hpc1_device::nrbdp_w(u32 data)
{
	LOGMASKED(LOG_ENET, "nrbdp_w 0x%08x\n", data);
	m_enet_nrbdp = data;
}

void hpc1_device::crbdp_w(u32 data)
{
	LOGMASKED(LOG_ENET, "crbdp_w 0x%08x\n", data);
	m_enet_crbdp = data;
}

void hpc1_device::aux_w(u8 data)
{
	m_eeprom_out(data);

	m_aux = data;
}
