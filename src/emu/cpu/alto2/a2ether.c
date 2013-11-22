/*****************************************************************************
 *
 *   Xerox AltoII ethernet task
 *
 *   Copyright © Jürgen Buchmüller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "alto2cpu.h"
#include "a2roms.h"

#define DEBUG_PACKETS   1


/**
 * @brief BPROMs P3601-1; 256x4; enet.a41 "PE1" and enet.a42 "PE2"
 *
 * Phase encoder
 *
 * a41: P3601-1; 256x4; "PE1"
 * a42: P3601-1; 256x4; "PE2"
 *
 * PE1/PE2 inputs
 * ----------------
 * A0  (5) OUTGO
 * A1  (6) XDATA
 * A2  (7) OSDATAG
 * A3  (4) XCLOCK
 * A4  (3) OCNTR0
 * A5  (2) OCNTR1
 * A6  (1) OCNTR2
 * A7 (15) OCNTR3
 *
 * PE1 outputs
 * ----------------
 * D0 (12) OCNTR0
 * D1 (11) OCNTR1
 * D2 (10) OCNTR2
 * D3  (9) OCNTR3
 *
 * PE2 outputs
 * ----------------
 * D0 (12) n.c.
 * D1 (11) to OSLOAD flip flop J and K'
 * D2 (10) XDATA
 * D3  (9) XCLOCK
 */
static const prom_load_t pl_enet_a41 =
{	/* P3601 256x4 BPROM; Ethernet phase encoder 1 "PE1" */
	"enet.a41",
	0,
	"d5de8d86",
	"c134a4c898c73863124361a9b0218f7a7f00082a",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	0,
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

static const prom_load_t pl_enet_a42 =
{	/* P3601 256x4 BPROM; Ethernet phase encoder 2 "PE2" */
	"enet.a42",
	0,
	"9d5c81bd",
	"ac7e63332a3dad0bef7cd0349b24e156a96a4bf0",
	/* size */	0400,
	/* amap */	AMAP_DEFAULT,
	/* axor */	0,
	/* dxor */	0,
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

/**
 * @brief BPROM; P3601-1; 265x4 enet.a49 "AFIFO"
 *
 * FIFO control
 *
 * a49: P3601-1; 256x4; "AFIFO"
 *
 * inputs
 * ----------------
 * A0  (5) fifo_wr[0]
 * A1  (6) fifo_wr[1]
 * A2  (7) fifo_wr[2]
 * A3  (4) fifo_wr[3]
 * A4  (3) fifo_rd[0]
 * A5  (2) fifo_rd[1]
 * A6  (1) fifo_rd[2]
 * A7 (15) fifo_rd[3]
 *
 * outputs active low
 * ----------------------------
 * D0 (12) BE'    (buffer empty)
 * D1 (11) BNE'   (buffer nearly empty)
 * D2 (10) BNNE'  (buffer next nearly empty)
 * D3  (9) BF'    (buffer full)
 */
static const prom_load_t pl_enet_a49 =
{	/* P3601 256x4 BPROM; Ethernet FIFO control "AFIFO" */
	"enet.a49",
	0,
	"4d2dcdb2",
	"583327a7d70cd02702c941c0e43c1e9408ff7fd0",
	/* size */	0400,
	/* amap */	AMAP_REVERSE_0_7,				// reverse address lines A0-A7
	/* axor */	0,
	/* dxor */	0,
	/* width */	4,
	/* shift */	0,
	/* dmap */	DMAP_DEFAULT,
	/* dand */	ZERO,
	/* type */	sizeof(UINT8)
};

#define	GET_ETH_WLF(st)			X_BIT(st,16,4)              //!< hardware status: write latch full/filled (? set by EODFCT)
#define	PUT_ETH_WLF(st,val)		X_WRBITS(st,16,4,4,val)

#define	GET_ETH_OEOT(st)		X_BIT(st,16,5)              //!< hardware status: output end of transmission (set by EEFCT)
#define	PUT_ETH_OEOT(st,val)	X_WRBITS(st,16,5,5,val)

#define	GET_ETH_IGONE(st)		X_BIT(st,16,6)              //!< hardware status: input gone
#define	PUT_ETH_IGONE(st,val)	X_WRBITS(st,16,6,6,val)

#define	GET_ETH_IBUSY(st)		X_BIT(st,16,7)              //!< hardware status: input busy (set by EISFCT, bit isn't visible to microcode)
#define	PUT_ETH_IBUSY(st,val)	X_WRBITS(st,16,7,7,val)

#define	GET_ETH_OGONE(st)		X_BIT(st,16,8)              //!< hardware status: output gone
#define	PUT_ETH_OGONE(st,val)	X_WRBITS(st,16,8,8,val)

#define	GET_ETH_OBUSY(st)		X_BIT(st,16,9)              //!< hardware status: output busy (set by EOSFCT, bit isn't visible to microcode)
#define	PUT_ETH_OBUSY(st,val)	X_WRBITS(st,16,9,9,val)

#define	GET_ETH_IDL(st)			X_BIT(st,16,10)             //!< hardware status: input data late
#define	PUT_ETH_IDL(st,val)		X_WRBITS(st,16,10,10,val)

#define	GET_ETH_COLL(st)		X_BIT(st,16,11)             //!< hardware status: collision
#define	PUT_ETH_COLL(st,val)	X_WRBITS(st,16,11,11,val)

#define	GET_ETH_CRC(st)			X_BIT(st,16,12)             //!< hardware status: CRC error
#define	PUT_ETH_CRC(st,val)		X_WRBITS(st,16,12,12,val)

#define	GET_ETH_ICMD(st)		X_BIT(st,16,13)				//!< hardware status: input command (set from BUS[14] on SIO, reset by EPFCT)
#define	PUT_ETH_ICMD(st,val)	X_WRBITS(st,16,13,13,val)

#define	GET_ETH_OCMD(st)		X_BIT(st,16,14)				//!< hardware status: output command (set from BUS[15] on SIO, reset by EPFCT)
#define	PUT_ETH_OCMD(st,val)	X_WRBITS(st,16,14,14,val)

#define	GET_ETH_IT(st)			X_BIT(st,16,15)				//!< hardware status: IT flip flop & ISRFULL'
#define	PUT_ETH_IT(st,val)		X_WRBITS(st,16,15,15,val)

#define	BE(a49)   ((a49 & ether_a49_BE) ? 1 : 0)		//! buffer empty
#define	BNE(a49)  ((a49 & ether_a49_BNE) ? 1 : 0)		//! buffer next empty
#define	BNNE(a49) ((a49 & ether_a49_BNNE) ? 1 : 0)		//! buffer next next empty
#define	BF(a49)   ((a49 & ether_a49_BF) ? 1 : 0)		//! buffer full

#define	BREATHLEN	ALTO2_ETHER_PACKET_SIZE	//!< ethernet packet length
#define	BREATHADDR	0177400					//!< destination and source
#define	BREATHTYPE	0000602					//!< ethernet packet type
static const UINT16 duckbreath_data[BREATHLEN] =
{
	BREATHADDR,		/* 3MB destination and source */
	BREATHTYPE,		/* ether packet type  */
	/* the rest is the contents of a breath of life packet.
	 * see <altosource>etherboot.dm (etherboot.asm) for the
	 * Alto assembly code.
	 */
	0022574, 0100000, 0040437, 0102000, 0034431, 0164000,
	0061005, 0102460, 0024567, 0034572, 0061006, 0024565, 0034570, 0061006,
	0024564, 0034566, 0061006, 0020565, 0034565, 0061005, 0125220, 0046573,
	0020576, 0061004, 0123400, 0030551, 0041211, 0004416, 0000000, 0001000,
	0000026, 0000244, 0000000, 0000000, 0000000, 0000000, 0000004, 0000000,
	0000000, 0000020, 0177777, 0055210, 0025400, 0107000, 0045400, 0041411,
	0020547, 0041207, 0020544, 0061004, 0006531, 0034517, 0030544, 0051606,
	0020510, 0041605, 0042526, 0102460, 0041601, 0020530, 0061004, 0021601,
	0101014, 0000414, 0061020, 0014737, 0000773, 0014517, 0000754, 0020517,
	0061004, 0030402, 0002402, 0000000, 0000732, 0034514, 0162414, 0000746,
	0021001, 0024511, 0106414, 0000742, 0021003, 0163400, 0035005, 0024501,
	0106415, 0175014, 0000733, 0021000, 0042465, 0034457, 0056445, 0055775,
	0055776, 0101300, 0041400, 0020467, 0041401, 0020432, 0041402, 0121400,
	0041403, 0021006, 0041411, 0021007, 0041412, 0021010, 0041413, 0021011,
	0041406, 0021012, 0041407, 0021013, 0041410, 0015414, 0006427, 0012434,
	0006426, 0020421, 0024437, 0134000, 0030417, 0002422, 0177035, 0000026,
	0000415, 0000427, 0000567, 0000607, 0000777, 0177751, 0177641, 0177600,
	0000225, 0177624, 0001013, 0000764, 0000431, 0000712, 0000634, 0000735,
	0000611, 0000567, 0000564, 0000566, 0000036, 0000002, 0000003, 0000015,
	0000030, 0000377, 0001000, 0177764, 0000436, 0054731, 0050750, 0020753,
	0040745, 0102460, 0040737, 0020762, 0061004, 0020734, 0105304, 0000406,
	0020743, 0101014, 0014741, 0000772, 0002712, 0034754, 0167700, 0116415,
	0024752, 0021001, 0106414, 0000754, 0021000, 0024703, 0106414, 0000750,
	0021003, 0163400, 0024736, 0106405, 0000404, 0121400, 0101404, 0000740,
	0044714, 0021005, 0042732, 0024664, 0122405, 0000404, 0101405, 0004404,
	0000727, 0010656, 0034654, 0024403, 0120500, 0101404, 0000777, 0040662,
	0040664, 0040664, 0102520, 0061004, 0020655, 0101015, 0000776, 0106415,
	0001400, 0014634, 0000761, 0020673, 0061004, 0000400, 0061005, 0102000,
	0143000, 0034672, 0024667, 0166400, 0061005, 0004670, 0020663, 0034664,
	0164000, 0147000, 0061005, 0024762, 0132414, 0133000, 0020636, 0034416,
	0101015, 0156415, 0131001, 0000754, 0024643, 0044625, 0101015, 0000750,
	0014623, 0004644, 0020634, 0061004, 0002000, 0176764, 0001401, 0041002
};

#if	DEBUG_PACKETS
static void dump_ascii(const UINT16 *src, size_t size)
{
	printf(" [");
	for (size_t offs = 0; offs < size; offs++) {
		char ch1 = src[offs] / 256;
		char ch2 = src[offs] % 256;
		printf("%c", ch1 < 32 || ch1 > 126 ? '.' : ch1);
		printf("%c", ch2 < 32 || ch2 > 126 ? '.' : ch2);
	}
	printf("]\n");
}

size_t dump_packet(const UINT16 *src, size_t addr, size_t size)
{
	size_t offs;
	for (offs = 0; offs < size; offs++) {
		UINT16 word = src[offs];
		if (offs % 8) {
			printf(" %06o", word);
		} else {
			if (offs > 0)
				dump_ascii(&src[offs-8], 8);
			printf("\t%05o: %06o", static_cast<unsigned>(addr + offs), word);
		}
	}
	if (offs % 8) {
		dump_ascii(&src[offs - (offs % 8)], offs % 8);
	} else if (offs > 0) {
		dump_ascii(&src[offs - 8], 8);
	}
	return size;
}
#endif

/**
 * @brief check for the various reasons to wakeup the ethernet task
 */
void alto2_cpu_device::eth_wakeup()
{
	register int st = m_eth.status;
	LOG((LOG_ETH,0,"IBUSY=%d OBUSY=%d ", GET_ETH_IBUSY(st), GET_ETH_OBUSY(st)));
	UINT8 busy = GET_ETH_IBUSY(st) | GET_ETH_OBUSY(st);
	if (0 == busy) {
		// if not busy, reset the FIFO read and write counters
		m_eth.fifo_rd = 0;
		m_eth.fifo_wr = 0;
	}

	/*
	 * POST conditions to wakeup the Ether task:
	 *	input data late
	 *	output command
	 *	input command
	 *	output gone
	 *	input gone
	 */
	if (GET_ETH_IDL(st)) {
		LOG((LOG_ETH,0,"POST (input data late)\n"));
		m_task_wakeup |= 1 << task_ether;
		return;
	}
	if (GET_ETH_OCMD(st)) {
		LOG((LOG_ETH,0,"POST (output command)\n"));
		m_task_wakeup |= 1 << task_ether;
		return;
	}
	if (GET_ETH_ICMD(st)) {
		LOG((LOG_ETH,0,"POST (input command)\n"));
		m_task_wakeup |= 1 << task_ether;
		return;
	}
	if (GET_ETH_OGONE(st)) {
		LOG((LOG_ETH,0,"POST (output gone)\n"));
		m_task_wakeup |= 1 << task_ether;
		return;
	}
	if (GET_ETH_IGONE(st)) {
		LOG((LOG_ETH,0,"POST (input gone)\n"));
		m_task_wakeup |= 1 << task_ether;
		return;
	}

	/*
	 * IDR (input data ready) conditions to wakeup the Ether task (AND):
	 *	IBUSY   input busy
	 *	BNNE    buffer next nearly empty
	 *	BNE     buffer nearly empty
	 *	ETAC    ether task active
	 *
	 * IDR' = (IBUSY & (BNNE & (BNE' & ETAC')')')'
	 */
	UINT8 a49 = m_ether_a49[16 * m_eth.fifo_rd + m_eth.fifo_wr];
	UINT8 etac = m_task == task_ether ? 0 : 1;
	UINT8 idr = GET_ETH_IBUSY(st) & ~(~BNNE(a49) & ~(BNE(a49) & etac));
	if (idr) {
		m_task_wakeup |= 1 << task_ether;
		LOG((LOG_ETH,0,"input data ready\n"));
		return;
	}

	/*
	 * ODR (output data ready) conditions to wakeup the Ether task:
	 *	WLF    write latch filled(?)
	 *	BF     buffer (FIFO) full
	 *	OEOT   output end of transmission
	 *	OBUSY  output busy
	 *
	 * ODR'	= (OBUSY & OEOT' & (BF' & WLF')')'
	 */
	UINT8 odr = GET_ETH_OBUSY(st) & ~(~GET_ETH_OEOT(st) & ~(BF(a49) & ~GET_ETH_WLF(st)));
	if (odr) {
		m_task_wakeup |= 1 << task_ether;
		LOG((LOG_ETH,0,"output data ready\n"));
		return;
	}

	/*
	 * EWFCT (ether wake function) conditions to wakeup the Ether task:
	 *	EWFCT	flip flop set by the F1 EWFCT
	 * The task is activated by the display code together with the
	 * next wakeup of the memory refresh task (MRT).
	 */
	if (m_ewfct) {
		m_task_wakeup |= 1 << task_ether;
		LOG((LOG_ETH,0,"EWFCT (ether wake function)\n"));
		return;
	}

	// otherwise no more wakeups for the ether task
	LOG((LOG_ETH,0,"stop wake\n"));
	m_task_wakeup &= ~(1 << task_ether);
}

/**
 * @brief F9401 CRC checker
 * <PRE>
 *
 * The F9401 looks similiar to the SN74F401. However, in the schematics
 * there is a connection from pin 9 (labeled D9) to pin 2 (labeled Q8).
 * See below for the difference:
 *
 *           SN74F401                       F9401
 *         +---+-+---+                   +---+-+---+
 *         |   +-+   |                   |   +-+   |
 *    CP' -|1      14|-  Vcc       CLK' -|1      14|-  Vcc
 *         |         |                   |         |
 *     P' -|2      13|-  ER          P' -|2      13|-  CRCZ'
 *         |         |                   |         |
 *    S0  -|3      12|-  Q           Z  -|3      12|-  CRCDATA
 *         |         |                   |         |
 *    MR  -|4      11|-  D          MR  -|4      11|-  SDI
 *         |         |                   |         |
 *    S1  -|5      10|-  CWE         Y  -|5      10|-  SR
 *         |         |                   |         |
 *    NC  -|6       9|-  NC         D1  -|6       9|-  D9
 *         |         |                   |         |
 *   GND  -|7       8|-  S2        GND  -|7       8|-  X
 *         |         |                   |         |
 *         +---------+	                 +---------+
 *
 * Functional description (SN74F401)
 *
 * The 'F401 is a 16-bit programmable device which operates on serial data
 * streams and provides a means of detecting transmission errors. Cyclic
 * encoding and decoding schemes for error detection are based on polynomial
 * manipulation in modulo arithmetic. For encoding, the data stream (message
 * polynomial) is divided by a selected polynomial. This division results
 * in a remainder which is appended to the message as check bits. For error
 * checking, the bit stream containing both data and check bits is divided
 * by the same selected polynomial. If there are no detectable errors, this
 * division results in a zero remainder. Although it is possible to choose
 * many generating polynomials of a given degree, standards exist that
 * specify a small number of useful polynomials. The 'F401 implements the
 * polynomials listed in Tabel I by applying the appropriate logic levels
 * to the select pins S0, S1 and S2.
 *
 * Teh 'F401 consists of a 16-bit register, a Read Only Memory (ROM) and
 * associated control circuitry as shown in the block diagram. The
 * polynomial control code presented at inputs S0, S1 and S2 is decoded
 * by the ROM, selecting the desired polynomial by establishing shift
 * mode operation on the register with Exclusive OR gates at appropriate
 * inputs. To generate check bits, the data stream is entered via the
 * Data inputs (D), using the HIGH-to-LOW transition of the Clock input
 * (CP'). This data is gated with the most significant output (Q) of
 * the register, and controls the Exclusive OR gates (Figure 1). The
 * Check Word Enable (CWE) must be held HIGH while the data is being
 * entered. After the last data bit is entered, the CWE is brought LOW
 * and the check bits are shifted out of the register and appended to
 * the data bits using external gating (Figure 2).
 *
 * To check an incoming message for errors, both the data and check bits
 * are entered through the D input with the CWE input held HIGH. The
 * 'F401 is not in the data path, but only monitors the message. The
 * Error output becomes valid after the last check bit has been entered
 * into the 'F401 by a HIGH-to-LOW transition of CP'. If no detectable
 * errors have occured during the transmission, the resultant internal
 * register bits are all LOW and the Error Output (ER) is LOW.
 * If a detectable error has occured, ER is HIGH.
 *
 * A HIGH on the Master Reset input (MR) asynchronously clears the
 * register. A LOW on the Preset input (P') asynchronously sets the
 * entire register if the control code inputs specify a 16-bit
 * polynomial; in the case of 12- or 8-bit check polynomials only the
 * most significant 12 or 8 register bits are set and the remaining
 * bits are cleared.
 *
 * [Table I]
 *
 * S2 S1 S0	polynomial                      remarks
 * ----------------------------------------------------------------
 * L  L  L	x^16+x^15+x^2+1                 CRC16
 * L  L  H	x^16+x^14+x+1                   CRC16 reverse
 * L  H  L	x^16+x^15+x^13+x^7+x^4+x^2+x+1  -/-
 * L  H  H	x^12+x^11+x^3+x^2+x+1           CRC-12
 * H  L  L	x^8+x^7+x^5+x^4+x+1             -/-
 * H  L  H	x^8+1                           LRC-8
 * H  H  L	X^16+x^12+x^5+1                 CRC-CCITT
 * H  H  H	X^16+x^11+x^4+1                 CRC-CCITT reverse
 *
 * </PRE>
 * The Alto ethernet interface seems to be using the last one of the polynomials,
 * or perhaps something entirely different?
 *
 * TODO: verify polynomial generator; build a lookup table to make it faster.
 *
 * @param crc previous CRC value
 * @param data 16 bit data
 * @return new CRC value after 16 bits
 */
UINT32 f9401_7(UINT32 crc, UINT32 data)
{
	static const UINT32 XOR = (1 << 10) | (1 << 3) | (1 << 0);
	crc ^= data;
	for (int i = 0; i < 16; i++)
		crc = (crc >> 1) ^ ((crc & 1) ? XOR : 0);
	return crc & 0177777;
}

/**
 * @brief HACK: pull the next word from the duckbreath_data in the fifo
 *
 * This is probably lacking the updates to one or more of
 * the status flip flops.
 */
void alto2_cpu_device::rx_duckbreath(void* ptr, INT32 arg)
{
	UINT32 data;

	if (arg == 0) {
		// on the first word set the IBUSY flip flop
		PUT_ETH_IBUSY(m_eth.status, 1);
		m_eth.rx_count = 0;
	}

	if (arg >= BREATHLEN) {
		// CRC after the data
		data = m_eth.rx_crc;
		arg++;
	} else {
		// next data word
		data = duckbreath_data[arg++];
	}
	m_eth.rx_crc = f9401_7(m_eth.rx_crc, data);
	m_eth.fifo[m_eth.fifo_wr] = data;
	m_eth.fifo_wr = (m_eth.fifo_wr + 1) % ALTO2_ETHER_FIFO_SIZE;

//	PUT_ETH_WLF(m_eth.status, 1);	// set WLF (write latch full)?

	UINT8 a49 = m_ether_a49[16 * m_eth.fifo_rd + m_eth.fifo_wr];
	if (0 == BF(a49))
		PUT_ETH_IDL(m_eth.status, 1);	// fifo is overrun: set input data late flip flop

	if (arg > BREATHLEN) {
		/*
		 * TODO: if data comes from some other source,
		 * compare our CRC with the next word received
		 * and set the CRC error flag if they differ.
		 */
		m_eth.rx_crc = 0;
		PUT_ETH_IGONE(m_eth.status, 1);		// set the IGONE flip flop
		m_eth.rx_timer->adjust(attotime::from_seconds(m_duckbreath_sec), 0);
	} else {
		// receive at a rate of 5.44us per word
		m_eth.rx_timer->adjust(attotime::from_usec(5.44), arg);
	}
	eth_wakeup();
}

/**
 * @brief transmit data from the FIFO to <nirvana for now>
 *
 * @param ptr unused pointer
 * @param arg word count if >= 0, -1 if CRC is to be transmitted (last word)
 */
void alto2_cpu_device::tx_packet(void* ptr, INT32 arg)
{
	UINT32 data;

	// the last word sent is the CRC
	if (-1 == arg) {
		m_eth.tx_timer->reset();
		LOG((LOG_ETH,0," CRC:%06o\n", m_eth.tx_crc));
		// TODO: send the CRC as final word of the packet
		m_eth.tx_crc = 0;
		PUT_ETH_OGONE(m_eth.status, 1);		// set the OGONE flip flop
		eth_wakeup();
		return;
	}

	data = m_eth.fifo[m_eth.fifo_rd];
	m_eth.tx_crc = f9401_7(m_eth.tx_crc, data);
	m_eth.fifo_rd = (m_eth.fifo_rd + 1) % ALTO2_ETHER_FIFO_SIZE;

	UINT8 a49 = m_ether_a49[16 * m_eth.fifo_rd + m_eth.fifo_wr];
	if (0 == BE(a49)) {
		// the FIFO is empty now: clear the OBUSY and WLF flip flops
		PUT_ETH_OBUSY(m_eth.status, 0);
		PUT_ETH_WLF(m_eth.status, 0);
		m_eth.tx_timer->adjust(attotime::from_usec(5.44), -1);
	} else {
		// transmit teh next word after 5.44µs
		m_eth.tx_timer->adjust(attotime::from_usec(5.44), arg + 1);
	}
	eth_wakeup();
}

/**
 * @brief ethernet start function - called from the emulator task
 */
void alto2_cpu_device::eth_startf()
{
	PUT_ETH_ICMD(m_eth.status, X_BIT(m_bus,16,14));
	PUT_ETH_OCMD(m_eth.status, X_BIT(m_bus,16,15));
	LOG((LOG_ETH,3, "	STARTF; ICMD=%u OCMD=%u\n", GET_ETH_ICMD(m_eth.status), GET_ETH_ICMD(m_eth.status)));
	eth_wakeup();
}

/**
 * @brief ethernet input data function
 *
 * Gates the contents of the FIFO to BUS[0-15], and increments
 * the read pointer at the end of the cycle.
 */
void alto2_cpu_device::bs_early_eidfct()
{
	UINT16 r = m_eth.fifo[m_eth.fifo_rd];
	LOG((LOG_ETH,3, "	←EIDFCT; pull %06o from FIFO[%02o]\n", r, m_eth.fifo_rd));
	m_eth.fifo_rd = (m_eth.fifo_rd + 1) % ALTO2_ETHER_FIFO_SIZE;
	m_bus &= r;

#if	DEBUG_PACKETS
	if (m_eth.rx_count < ALTO2_ETHER_PACKET_SIZE)
		m_eth.rx_packet[m_eth.rx_count] = r;
	m_eth.rx_count++;
	if (ALTO2_ETHER_PACKET_SIZE == m_eth.rx_count) {
		dump_packet(m_eth.rx_packet, 0, m_eth.rx_count);
		m_eth.rx_count = 0;
	}
#endif
	eth_wakeup();
}

/**
 * @brief block the ethernet task
 */
void alto2_cpu_device::f1_early_eth_block()
{
	LOG((LOG_ETH,2,"	BLOCK %s\n", task_name(m_task)));
	m_task_wakeup &= ~(1 << task_ether);
}

/**
 * @brief ethernet input look function
 *
 * Gates the contents of the FIFO to BUS[0-15], but does not
 * increment the read pointer;
 */
void alto2_cpu_device::f1_early_eilfct()
{
	UINT16 r = m_eth.fifo[m_eth.fifo_rd];
	LOG((LOG_ETH,3, "	←EILFCT; %06o at FIFO[%02o]\n", r, m_eth.fifo_rd));
	m_bus &= r;
}

/**
 * @brief ethernet post function
 *
 * Gates the interface status to BUS[8-15]. Resets the interface
 * at the end of the function.
 *
 * The schematics suggest that just BUS[10-15] is modified.
 *
 * Also a comment from the microcode suggests this:
 *<PRE>
 * ;Ether Post Function - EPFCT.  Gate the hardware status
 * ;(LOW TRUE) to Bus [10:15], reset interface.
 *</PRE>
 */
void alto2_cpu_device::f1_early_epfct()
{
	UINT16 r = ~X_RDBITS(m_eth.status,16,10,15) & 0177777;
	m_eth.status = 0;
	m_eth.tx_count = 0;
	eth_wakeup();

	LOG((LOG_ETH,3, "	←EPFCT; BUS[8-15] = STATUS (%#o)\n", r));
	LOG((LOG_ETH,5, "		IDL'    : %u\n", GET_ETH_IDL(r)));
	LOG((LOG_ETH,5, "		COLL'   : %u\n", GET_ETH_COLL(r)));
	LOG((LOG_ETH,5, "		CRC'    : %u\n", GET_ETH_CRC(r)));
	LOG((LOG_ETH,5, "		ICMD'   : %u\n", GET_ETH_ICMD(r)));
	LOG((LOG_ETH,5, "		OCMD'   : %u\n", GET_ETH_OCMD(r)));
	LOG((LOG_ETH,5, "		IT'     : %u\n", GET_ETH_IT(r)));

	m_bus &= r;
}

/**
 * @brief ethernet countdown wakeup function
 *
 * Sets a flip flop in the interface that will cause a wakeup to the
 * Ether task on the next tick of SWAKMRT (memory refresh task).
 * This function must be issued in the instruction after a TASK.
 * The resulting wakeup is cleared when the Ether task next runs.
 */
void alto2_cpu_device::f1_late_ewfct()
{
	/*
	 * Set a flag in the CPU to handle the next task switch
	 * to the task_mrt by also waking up the task_ether.
	 */
	m_ewfct = m_ether_enable;
}

/**
 * @brief ethernet output data function
 *
 * Loads the FIFO from BUS[0-15], then increments the write
 * pointer at the end of the cycle.
 *
 * Comment from the micro code:
 *<PRE>
 * Ether Output Data Function - EODFCT.  Copy the bus into the
 * interface data buffer, increment the write pointer, clears wakeup
 * request if the buffer is now nearly full (one slot available).
 *</PRE>
 */
void alto2_cpu_device::f2_late_eodfct()
{
	LOG((LOG_ETH,3, "	EODFCT←; push %06o into FIFO[%02o]\n", m_bus, m_eth.fifo_wr));
	m_eth.fifo[m_eth.fifo_wr] = m_bus;
	m_eth.fifo_wr = (m_eth.fifo_wr + 1) % ALTO2_ETHER_FIFO_SIZE;

#if	DEBUG_PACKETS
	if (m_eth.tx_count < ALTO2_ETHER_PACKET_SIZE)
		m_eth.tx_packet[m_eth.tx_count] = m_bus;
	m_eth.tx_count++;
	if (ALTO2_ETHER_PACKET_SIZE == m_eth.tx_count) {
		dump_packet(m_eth.tx_packet, 0, m_eth.tx_count);
		m_eth.tx_count = 0;
	}
#endif

	PUT_ETH_WLF(m_eth.status, 1);			// set WLF (write latch full)
	PUT_ETH_OBUSY(m_eth.status, 1);			// set OBUSY (output busy)
	eth_wakeup();
	// if the FIFO is full, stop wakeup and kick off the timer
	UINT8 a49 = m_ether_a49[16 * m_eth.fifo_rd + m_eth.fifo_wr];
	if (0 == BF(a49)) {
		m_task_wakeup &= ~(1 << task_ether);
		m_eth.tx_timer->adjust(attotime::from_usec(5.44), 0);
	}
}

/**
 * @brief f2_eosfct late: ethernet output start function
 *
 * Sets the OBUSY flip flop in the interface, starting data
 * wakeups to fill the FIFO for output. When the FIFO is full,
 * or EEFCT has been issued, the interface will wait for silence
 * on the Ether and begin transmitting.
 */
void alto2_cpu_device::f2_late_eosfct()
{
	LOG((LOG_ETH,3, "	EOSFCT\n"));
	PUT_ETH_WLF(m_eth.status, 1);
	PUT_ETH_OBUSY(m_eth.status, 1);
	eth_wakeup();
}

/**
 * @brief f2_erbfct late: ethernet reset branch function
 *
 * This command dispatch function merges the ICMD and OCMD flip flops
 * into NEXT[6-7]. These flip flops are the means of communication
 * between the emulator task and the ethernet task. The emulator
 * task sets them up from BUS[14-15] with the STARTF function,
 * causing the ethernet task to wakeup, dispatch on them and then
 * reset them with EPFCT.
 */
void alto2_cpu_device::f2_late_erbfct()
{
	UINT16 r = 0;
	X_WRBITS(r,10,6,6,GET_ETH_ICMD(m_eth.status));
	X_WRBITS(r,10,7,7,GET_ETH_OCMD(m_eth.status));
	LOG((LOG_ETH,3, "	ERBFCT; NEXT[6-7] = ICMD,OCMD (%#o | %#o)\n", m_next2, r));
	m_next2 |= r;
	eth_wakeup();
}

/**
 * @brief ethernet end of transmission function
 *
 * This function is issued when all of the main memory output buffer
 * has been transferred to the FIFO. EEFCT disables further data
 * wakeups.
 */
void alto2_cpu_device::f2_late_eefct()
{
	PUT_ETH_OBUSY(m_eth.status, 1);
	PUT_ETH_OEOT(m_eth.status, 1);
	// end transmitting the packet
	m_eth.tx_timer->adjust(attotime::from_usec(5.44), -1);
	eth_wakeup();
}

/**
 * @brief ethernet branch function
 *
 * ORs a 1 into NEXT[6] if a collision is detected.
 * ORs a 1 into NEXT[7] if an input data late is detected,
 *     or a SIO with AC0[14-15] non-zero is issued,
 *     or if the transmitter is gone
 *     or if the receiver is gone.
 */
void alto2_cpu_device::f2_late_ebfct()
{
	UINT16 r = 0;
	X_WRBITS(r,10,6,6, GET_ETH_COLL(m_eth.status));
	X_WRBITS(r,10,7,7,
			 GET_ETH_IDL(m_eth.status) |
			 GET_ETH_ICMD(m_eth.status) |
			 GET_ETH_OCMD(m_eth.status) |
			 GET_ETH_IGONE(m_eth.status) |
			 GET_ETH_OGONE(m_eth.status));
	LOG((LOG_ETH,3, "	EBFCT; NEXT ... (%#o | %#o)\n", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_ecbfct late: ethernet countdown branch function
 *
 * The BE' (buffer empty) signal is output D0 of PROM a49
 * ORs a one into NEXT[7] if the FIFO is not empty.
 */
void alto2_cpu_device::f2_late_ecbfct()
{
	UINT16 r = 0;
	UINT8 a49 = m_ether_a49[16 * m_eth.fifo_rd + m_eth.fifo_wr];
	X_WRBITS(r,10,7,7,~BE(a49));
	LOG((LOG_ETH,3, "	ECBFCT; NEXT[7] = FIFO %sempty (%#o | %#o)\n", r ? "not " : "is ", m_next2, r));
	m_next2 |= r;
}

/**
 * @brief f2_eisfct late: ethernet input start function
 *
 * Sets the IBUSY flip flop in the interface, causing it to hunt
 * for the beginning of a packet: silence on the Ether followed
 * by a transition. When the interface has collected two words,
 * it will begin generating data wakeups to the microcode.
 */
void alto2_cpu_device::f2_late_eisfct()
{
	LOG((LOG_ETH,3, "	EISFCT\n"));
	PUT_ETH_IBUSY(m_eth.status, 1);
	eth_wakeup();
}

/** @brief called by the CPU when the ethernet task becomes active
 *
 * Reset the Ether wake flip flop
 */
void alto2_cpu_device::activate_eth()
{
	m_ewfct = 0;
}

/**
 * @brief ethernet task slot initialization
 */
void alto2_cpu_device::init_ether(int task)
{
	// intialize all ethernet variables
	memset(&m_eth, 0, sizeof(m_eth));

	m_ether_a41 = prom_load(machine(), &pl_enet_a41, memregion("ether_a41")->base());
	m_ether_a42 = prom_load(machine(), &pl_enet_a42, memregion("ether_a42")->base());
	m_ether_a49 = prom_load(machine(), &pl_enet_a49, memregion("ether_a49")->base());

	set_bs(task, bs_ether_eidfct,	&alto2_cpu_device::bs_early_eidfct,	0);

	set_f1(task, f1_block,			&alto2_cpu_device::f1_early_eth_block, 0);
	set_f1(task, f1_ether_eilfct,	&alto2_cpu_device::f1_early_eilfct, 0);
	set_f1(task, f1_ether_epfct,	&alto2_cpu_device::f1_early_epfct, 0);
	set_f1(task, f1_ether_ewfct,	0, &alto2_cpu_device::f1_late_ewfct);

	set_f2(task, f2_ether_eodfct,	0, &alto2_cpu_device::f2_late_eodfct);
	set_f2(task, f2_ether_eosfct,	0, &alto2_cpu_device::f2_late_eosfct);
	set_f2(task, f2_ether_erbfct,	0, &alto2_cpu_device::f2_late_erbfct);
	set_f2(task, f2_ether_eefct,	0, &alto2_cpu_device::f2_late_eefct);
	set_f2(task, f2_ether_ebfct,	0, &alto2_cpu_device::f2_late_ebfct);
	set_f2(task, f2_ether_ecbfct,	0, &alto2_cpu_device::f2_late_ecbfct);
	set_f2(task, f2_ether_eisfct,	0, &alto2_cpu_device::f2_late_eisfct);

	m_active_callback[task] = &alto2_cpu_device::activate_eth;

	m_eth.rx_packet = auto_alloc_array(machine(), UINT16, sizeof(UINT16)*ALTO2_ETHER_PACKET_SIZE);
	m_eth.tx_packet = auto_alloc_array(machine(), UINT16, sizeof(UINT16)*ALTO2_ETHER_PACKET_SIZE);

	m_eth.tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(alto2_cpu_device::tx_packet),this));
	m_eth.tx_timer->reset();

	m_eth.rx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(alto2_cpu_device::rx_duckbreath),this));
	if (m_eth.duckbreath)
		m_eth.rx_timer->adjust(attotime::from_seconds(m_eth.duckbreath), 0);
	else
		m_eth.rx_timer->reset();
}

void alto2_cpu_device::exit_ether()
{
	// nothing to do yet
}

//! delay between two duckbreaths in seconds
static const int duckbreath_sec[8] = {
	0, 5, 10, 15, 30, 60, 90, 120
};
void alto2_cpu_device::reset_ether()
{
	memset(m_eth.fifo, 0, sizeof(m_eth.fifo));
	m_eth.fifo_rd = 0;
	m_eth.fifo_wr = 0;
	m_eth.status = 0;
	m_eth.rx_crc = 0;
	m_eth.tx_crc = 0;
	m_eth.rx_count = 0;
	m_eth.tx_count = 0;
	m_eth.rx_timer->reset();
	m_eth.tx_timer->reset();
	ioport_port* config = ioport(":CONFIG");
	// config should be valid, unless the driver doesn't define it
	if (config)
		m_eth.duckbreath = duckbreath_sec[(config->read() >> 4) & 7];
	else
		m_eth.duckbreath = 0;
	logerror("Ethernet duckbreath %d sec\n", m_eth.duckbreath);
}
