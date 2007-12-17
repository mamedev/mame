/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles

***************************************************************************

    Notes:
        * only loopback mode really works

**************************************************************************/

#include "driver.h"
#include "smc91c9x.h"



/*************************************
 *
 *  Debugging constants
 *
 *************************************/

#define LOG_ETHERNET		(0)
#define DISPLAY_STATS		(0)



/*************************************
 *
 *  Ethernet constants
 *
 *************************************/

#define ETHER_BUFFER_SIZE	(2048)
#define ETHER_RX_BUFFERS	(4)

/* Ethernet registers - bank 0 */
#define EREG_TCR			(0*8 + 0)
#define EREG_EPH_STATUS		(0*8 + 1)
#define EREG_RCR			(0*8 + 2)
#define EREG_COUNTER		(0*8 + 3)
#define EREG_MIR			(0*8 + 4)
#define EREG_MCR			(0*8 + 5)
#define EREG_BANK			(0*8 + 7)

/* Ethernet registers - bank 1 */
#define EREG_CONFIG			(1*8 + 0)
#define EREG_BASE			(1*8 + 1)
#define EREG_IA0_1			(1*8 + 2)
#define EREG_IA2_3			(1*8 + 3)
#define EREG_IA4_5			(1*8 + 4)
#define EREG_GENERAL_PURP	(1*8 + 5)
#define EREG_CONTROL		(1*8 + 6)

/* Ethernet registers - bank 2 */
#define EREG_MMU_COMMAND	(2*8 + 0)
#define EREG_PNR_ARR		(2*8 + 1)
#define EREG_FIFO_PORTS		(2*8 + 2)
#define EREG_POINTER		(2*8 + 3)
#define EREG_DATA_0			(2*8 + 4)
#define EREG_DATA_1			(2*8 + 5)
#define EREG_INTERRUPT		(2*8 + 6)

/* Ethernet registers - bank 3 */
#define EREG_MT0_1			(3*8 + 0)
#define EREG_MT2_3			(3*8 + 1)
#define EREG_MT4_5			(3*8 + 2)
#define EREG_MT6_7			(3*8 + 3)
#define EREG_MGMT			(3*8 + 4)
#define EREG_REVISION		(3*8 + 5)
#define EREG_ERCV			(3*8 + 6)

/* Ethernet MMU commands */
#define ECMD_NOP			0
#define ECMD_ALLOCATE		1
#define ECMD_RESET_MMU		2
#define ECMD_REMOVE			3
#define ECMD_REMOVE_RELEASE	4
#define ECMD_RELEASE_PACKET	5
#define ECMD_ENQUEUE_PACKET	6
#define ECMD_RESET_FIFOS	7

/* Ethernet interrupt bits */
#define EINT_RCV			0x01
#define EINT_TX				0x02
#define EINT_TX_EMPTY		0x04
#define EINT_ALLOC			0x08
#define EINT_RX_OVRN		0x10
#define EINT_EPH			0x20
#define EINT_ERCV			0x40

/* Ethernet register names */
static const char *ethernet_regname[64] =
{
	"TCR", "EPH STATUS", "RCR", "COUNTER", "MIR", "MCR", "(0.6)", "BANK",
	"CONFIG", "BASE", "IA0-1", "IA2-3", "IA4-5", "GENERAL PURPOSE", "CONTROL", "BANK",
	"MMU COMMAND", "PNR ARR", "FIFO PORTS", "POINTER", "DATA", "DATA", "INTERRUPT", "BANK",
	"MT0-1", "MT2-3", "MT4-5", "MT6-7", "MGMT", "REVISION", "ERCV", "BANK",
	"(4.0)", "(4.1)", "(4.2)", "(4.3)", "(4.4)", "(4.5)", "(4.6)", "BANK",
	"(5.0)", "(5.1)", "(5.2)", "(5.3)", "(5.4)", "(5.5)", "(5.6)", "BANK",
	"(6.0)", "(6.1)", "(6.2)", "(6.3)", "(6.4)", "(6.5)", "(6.6)", "BANK",
	"(7.0)", "(7.1)", "(7.2)", "(7.3)", "(7.4)", "(7.5)", "(7.6)", "BANK"
};



/*************************************
 *
 *  Structures
 *
 *************************************/

struct smc91c94_data
{
	void (*irq_handler)(int state);

	/* raw register data and masks */
	UINT16			reg[64];
	UINT16			regmask[64];

	/* IRQ information */
	UINT8			irq_state;

	/* allocate information */
	UINT8			alloc_count;

	/* transmit/receive FIFOs */
	UINT8 			fifo_count;
	UINT8			rx[ETHER_BUFFER_SIZE * ETHER_RX_BUFFERS];
	UINT8			tx[ETHER_BUFFER_SIZE];

	/* counters */
	UINT32			sent;
	UINT32			recd;
};



/*************************************
 *
 *  Local variables
 *
 *************************************/

static struct smc91c94_data ethernet;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void update_ethernet_irq(void);



/*************************************
 *
 *  Initialization
 *
 *************************************/

void smc91c94_init(struct smc91c9x_interface *config)
{
	ethernet.irq_handler = config->irq_handler;
}



/*************************************
 *
 *  Reset
 *
 *************************************/

void smc91c94_reset(void)
{
	void (*saved_handler)(int) = ethernet.irq_handler;
	memset(&ethernet, 0, sizeof(ethernet));
	ethernet.irq_handler = saved_handler;

	ethernet.reg[EREG_TCR]			= 0x0000;	ethernet.regmask[EREG_TCR]			= 0x3d87;
	ethernet.reg[EREG_EPH_STATUS]	= 0x0000;	ethernet.regmask[EREG_EPH_STATUS]	= 0x0000;
	ethernet.reg[EREG_RCR]			= 0x0000;	ethernet.regmask[EREG_RCR]			= 0xc307;
	ethernet.reg[EREG_COUNTER]		= 0x0000;	ethernet.regmask[EREG_COUNTER]		= 0x0000;
	ethernet.reg[EREG_MIR]			= 0x1212;	ethernet.regmask[EREG_MIR]			= 0x0000;
	ethernet.reg[EREG_MCR]			= 0x3300;	ethernet.regmask[EREG_MCR]			= 0x00ff;
	ethernet.reg[EREG_BANK]			= 0x3300;	ethernet.regmask[EREG_BANK]			= 0x0007;

	ethernet.reg[EREG_CONFIG]		= 0x0030;	ethernet.regmask[EREG_CONFIG]		= 0x17c6;
	ethernet.reg[EREG_BASE]			= 0x1866;	ethernet.regmask[EREG_BASE]			= 0xfffe;
	ethernet.reg[EREG_IA0_1]		= 0x0000;	ethernet.regmask[EREG_IA0_1]		= 0xffff;
	ethernet.reg[EREG_IA2_3]		= 0x0000;	ethernet.regmask[EREG_IA2_3]		= 0xffff;
	ethernet.reg[EREG_IA4_5]		= 0x0000;	ethernet.regmask[EREG_IA4_5]		= 0xffff;
	ethernet.reg[EREG_GENERAL_PURP]	= 0x0000;	ethernet.regmask[EREG_GENERAL_PURP]	= 0xffff;
	ethernet.reg[EREG_CONTROL]		= 0x0100;	ethernet.regmask[EREG_CONTROL]		= 0x68e7;

	ethernet.reg[EREG_MMU_COMMAND]	= 0x0000;	ethernet.regmask[EREG_MMU_COMMAND]	= 0x00e7;
	ethernet.reg[EREG_PNR_ARR]		= 0x8000;	ethernet.regmask[EREG_PNR_ARR]		= 0x00ff;
	ethernet.reg[EREG_FIFO_PORTS]	= 0x8080;	ethernet.regmask[EREG_FIFO_PORTS]	= 0x0000;
	ethernet.reg[EREG_POINTER]		= 0x0000;	ethernet.regmask[EREG_POINTER]		= 0xf7ff;
	ethernet.reg[EREG_DATA_0]		= 0x0000;	ethernet.regmask[EREG_DATA_0]		= 0xffff;
	ethernet.reg[EREG_DATA_1]		= 0x0000;	ethernet.regmask[EREG_DATA_1]		= 0xffff;
	ethernet.reg[EREG_INTERRUPT]	= 0x0004;	ethernet.regmask[EREG_INTERRUPT]	= 0x7f00;

	ethernet.reg[EREG_MT0_1]		= 0x0000;	ethernet.regmask[EREG_MT0_1]		= 0xffff;
	ethernet.reg[EREG_MT2_3]		= 0x0000;	ethernet.regmask[EREG_MT2_3]		= 0xffff;
	ethernet.reg[EREG_MT4_5]		= 0x0000;	ethernet.regmask[EREG_MT4_5]		= 0xffff;
	ethernet.reg[EREG_MT6_7]		= 0x0000;	ethernet.regmask[EREG_MT6_7]		= 0xffff;
	ethernet.reg[EREG_MGMT]			= 0x3030;	ethernet.regmask[EREG_MGMT]			= 0x0f0f;
	ethernet.reg[EREG_REVISION]		= 0x3340;	ethernet.regmask[EREG_REVISION]		= 0x0000;
	ethernet.reg[EREG_ERCV]			= 0x331f;	ethernet.regmask[EREG_ERCV]			= 0x009f;

	update_ethernet_irq();
}



/*************************************
 *
 *  Internal IRQ handling
 *
 *************************************/

static void update_ethernet_irq(void)
{
	UINT8 mask = ethernet.reg[EREG_INTERRUPT] >> 8;
	UINT8 state = ethernet.reg[EREG_INTERRUPT] & 0xff;

	/* update the IRQ state */
	ethernet.irq_state = ((mask & state) != 0);
	if (ethernet.irq_handler)
		(*ethernet.irq_handler)(ethernet.irq_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Draw the stats
 *
 *************************************/

static void update_stats(void)
{
	if (DISPLAY_STATS)
		popmessage("Sent:%d  Rec'd:%d", ethernet.sent, ethernet.recd);
}




/*************************************
 *
 *  Complete an enqueued packet
 *
 *************************************/

static void finish_enqueue(int param)
{
	int is_broadcast = (ethernet.tx[4] == 0xff && ethernet.tx[5] == 0xff && ethernet.tx[6] == 0xff &&
						ethernet.tx[7] == 0xff && ethernet.tx[8] == 0xff && ethernet.tx[9] == 0xff);

	/* update the EPH register and stuff it in the first transmit word */
	ethernet.reg[EREG_EPH_STATUS] = 0x0001;
	if (is_broadcast)
		ethernet.reg[EREG_EPH_STATUS] |= 0x0040;
	ethernet.tx[0] = ethernet.reg[EREG_EPH_STATUS];
	ethernet.tx[1] = ethernet.reg[EREG_EPH_STATUS] >> 8;

	/* signal a transmit interrupt and mark the transmit buffer empty */
	ethernet.reg[EREG_INTERRUPT] |= EINT_TX;
	ethernet.reg[EREG_INTERRUPT] |= EINT_TX_EMPTY;
	ethernet.reg[EREG_FIFO_PORTS] |= 0x0080;
	ethernet.sent++;
	update_stats();

	/* loopback? */
	if (ethernet.reg[EREG_TCR] & 0x2002)
		if (ethernet.fifo_count < ETHER_RX_BUFFERS)
		{
			int buffer_len = ((ethernet.tx[3] << 8) | ethernet.tx[2]) & 0x7ff;
			UINT8 *packet = &ethernet.rx[ethernet.fifo_count++ * ETHER_BUFFER_SIZE];
			int packet_len;

			/* compute the packet length */
			packet_len = buffer_len - 6;
			if (packet[buffer_len - 1] & 0x20)
				packet_len++;

			/* build up the packet */
			packet[0] = 0x0000;
			packet[1] = 0x0000;
			packet[2] = buffer_len;
			packet[3] = buffer_len >> 8;
			memcpy(&packet[4], &ethernet.tx[4], 6);
			memcpy(&packet[10], &ethernet.tx[10], 6);
			memcpy(&packet[16], &ethernet.tx[16], buffer_len - 16);

			/* set the broadcast flag */
			if (is_broadcast)
				packet[1] |= 0x40;

			/* pad? */
			if (ethernet.reg[EREG_TCR & 0x0080])
				if (packet_len < 64)
				{
					memset(&packet[buffer_len], 0, 64+6 - buffer_len);
					packet[buffer_len - 1] = 0;
					buffer_len = 64+6;
					packet[2] = buffer_len;
					packet[3] = buffer_len >> 8;
				}

			/* signal a receive */
			ethernet.reg[EREG_INTERRUPT] |= EINT_RCV;
			ethernet.reg[EREG_FIFO_PORTS] &= ~0x8000;
		}
	update_ethernet_irq();
}



/*************************************
 *
 *  MMU command processing
 *
 *************************************/

static void process_command(UINT16 data)
{
	switch ((data >> 5) & 7)
	{
		case ECMD_NOP:
			if (LOG_ETHERNET)
				logerror("   NOP\n");
			break;

		case ECMD_ALLOCATE:
			if (LOG_ETHERNET)
				logerror("   ALLOCATE MEMORY FOR TX (%d)\n", (data & 7));
			ethernet.reg[EREG_PNR_ARR] &= ~0xff00;
			ethernet.reg[EREG_PNR_ARR] |= ethernet.alloc_count++ << 8;
			ethernet.reg[EREG_INTERRUPT] |= 0x0008;
			update_ethernet_irq();
			break;

		case ECMD_RESET_MMU:
			if (LOG_ETHERNET)
				logerror("   RESET MMU\n");
			break;

		case ECMD_REMOVE:
			if (LOG_ETHERNET)
				logerror("   REMOVE FRAME FROM RX FIFO\n");
			break;

		case ECMD_REMOVE_RELEASE:
			if (LOG_ETHERNET)
				logerror("   REMOVE AND RELEASE FRAME FROM RX FIFO\n");
			ethernet.reg[EREG_INTERRUPT] &= ~EINT_RCV;
			if (ethernet.fifo_count > 0)
				ethernet.fifo_count--;
			if (ethernet.fifo_count > 0)
			{
				memmove(&ethernet.rx[0], &ethernet.rx[ETHER_BUFFER_SIZE], ethernet.fifo_count * ETHER_BUFFER_SIZE);
				ethernet.reg[EREG_INTERRUPT] |= EINT_RCV;
				ethernet.reg[EREG_FIFO_PORTS] &= ~0x8000;
			}
			else
				ethernet.reg[EREG_FIFO_PORTS] |= 0x8000;
			update_ethernet_irq();
			ethernet.recd++;
			update_stats();
			break;

		case ECMD_RELEASE_PACKET:
			if (LOG_ETHERNET)
				logerror("   RELEASE SPECIFIC PACKET\n");
			break;

		case ECMD_ENQUEUE_PACKET:
			if (LOG_ETHERNET)
				logerror("   ENQUEUE TX PACKET\n");
			finish_enqueue(0);
			break;

		case ECMD_RESET_FIFOS:
			if (LOG_ETHERNET)
				logerror("   RESET TX FIFOS\n");
			break;
	}
	ethernet.reg[EREG_MMU_COMMAND] &= ~0x0001;
}



/*************************************
 *
 *  Core read handler
 *
 *************************************/

READ16_HANDLER( smc91c94_r )
{
	UINT32 result = ~0;

	/* determine the effective register */
	offset %= 8;
	if (offset != EREG_BANK)
		offset += 8 * (ethernet.reg[EREG_BANK] & 7);
	result = ethernet.reg[offset];

	switch (offset)
	{
		case EREG_PNR_ARR:
			if (!(mem_mask & 0xff00))
			{
				ethernet.reg[EREG_INTERRUPT] &= ~0x0008;
				update_ethernet_irq();
			}
			break;

		case EREG_DATA_0:	/* data register */
		case EREG_DATA_1:	/* data register */
		{
			UINT8 *buffer = (ethernet.reg[EREG_POINTER] & 0x8000) ? ethernet.rx : ethernet.tx;
			int addr = ethernet.reg[EREG_POINTER] & 0x7ff;
			result = buffer[addr++];
			if (!(mem_mask & 0xff00))
				result |= buffer[addr++] << 8;
			if (ethernet.reg[EREG_POINTER] & 0x4000)
				ethernet.reg[EREG_POINTER] = (ethernet.reg[EREG_POINTER] & ~0x7ff) | (addr & 0x7ff);
			break;
		}
	}

	if (LOG_ETHERNET && offset != EREG_BANK)
		logerror("%08X:ethernet_r(%s) = %04X & %04X\n", activecpu_get_pc(), ethernet_regname[offset], result, mem_mask ^ 0xffff);
	return result;
}



/*************************************
 *
 *  Core write handler
 *
 *************************************/

WRITE16_HANDLER( smc91c94_w )
{
	UINT16 olddata;

	/* determine the effective register */
	offset %= 8;
	if (offset != EREG_BANK)
		offset += 8 * (ethernet.reg[EREG_BANK] & 7);

	/* update the data generically */
	olddata = ethernet.reg[offset];
	mem_mask |= ~ethernet.regmask[offset];
	COMBINE_DATA(&ethernet.reg[offset]);

	if (LOG_ETHERNET && offset != 7)
		logerror("%08X:ethernet_w(%s) = %04X & %04X\n", activecpu_get_pc(), ethernet_regname[offset], data, mem_mask ^ 0xffff);

	/* handle it */
	switch (offset)
	{
		case EREG_TCR:		/* transmit control register */
			if (LOG_ETHERNET)
			{
				if (data & 0x2000) logerror("   EPH LOOP\n");
				if (data & 0x1000) logerror("   STP SQET\n");
				if (data & 0x0800) logerror("   FDUPLX\n");
				if (data & 0x0400) logerror("   MON_CSN\n");
				if (data & 0x0100) logerror("   NOCRC\n");
				if (data & 0x0080) logerror("   PAD_EN\n");
				if (data & 0x0004) logerror("   FORCOL\n");
				if (data & 0x0002) logerror("   LOOP\n");
				if (data & 0x0001) logerror("   TXENA\n");
			}
			break;

		case EREG_RCR:		/* receive control register */
			if (LOG_ETHERNET)
			{
				if (data & 0x8000) smc91c94_reset();
				if (data & 0x8000) logerror("   SOFT RST\n");
				if (data & 0x4000) logerror("   FILT_CAR\n");
				if (data & 0x0200) logerror("   STRIP CRC\n");
				if (data & 0x0100) logerror("   RXEN\n");
				if (data & 0x0004) logerror("   ALMUL\n");
				if (data & 0x0002) logerror("   PRMS\n");
				if (data & 0x0001) logerror("   RX_ABORT\n");
			}
			break;

		case EREG_CONFIG:		/* configuration register */
			if (LOG_ETHERNET)
			{
				if (data & 0x1000) logerror("   NO WAIT\n");
				if (data & 0x0400) logerror("   FULL STEP\n");
				if (data & 0x0200) logerror("   SET SQLCH\n");
				if (data & 0x0100) logerror("   AUI SELECT\n");
				if (data & 0x0080) logerror("   16 BIT\n");
				if (data & 0x0040) logerror("   DIS LINK\n");
				if (data & 0x0004) logerror("   INT SEL1\n");
				if (data & 0x0002) logerror("   INT SEL0\n");
			}
			break;

		case EREG_BASE:		/* base address register */
			if (LOG_ETHERNET)
			{
				logerror("   base = $%04X\n", (data & 0xe000) | ((data & 0x1f00) >> 3));
				logerror("   romsize = %d\n", ((data & 0xc0) >> 6));
				logerror("   romaddr = $%05X\n", ((data & 0x3e) << 13));
			}
			break;

		case EREG_CONTROL:		/* control register */
			if (LOG_ETHERNET)
			{
				if (data & 0x4000) logerror("   RCV_BAD\n");
				if (data & 0x2000) logerror("   PWRDN\n");
				if (data & 0x0800) logerror("   AUTO RELEASE\n");
				if (data & 0x0080) logerror("   LE ENABLE\n");
				if (data & 0x0040) logerror("   CR ENABLE\n");
				if (data & 0x0020) logerror("   TE ENABLE\n");
				if (data & 0x0004) logerror("   EEPROM SELECT\n");
				if (data & 0x0002) logerror("   RELOAD\n");
				if (data & 0x0001) logerror("   STORE\n");
			}
			break;

		case EREG_MMU_COMMAND:	/* command register */
			process_command(data);
			break;

		case EREG_DATA_0:	/* data register */
		case EREG_DATA_1:	/* data register */
		{
			UINT8 *buffer = (ethernet.reg[EREG_POINTER] & 0x8000) ? ethernet.rx : ethernet.tx;
			int addr = ethernet.reg[EREG_POINTER] & 0x7ff;
			buffer[addr++] = data;
			if (!(mem_mask & 0xff00))
				buffer[addr++] = data >> 8;
			if (ethernet.reg[EREG_POINTER] & 0x4000)
				ethernet.reg[EREG_POINTER] = (ethernet.reg[EREG_POINTER] & ~0x7ff) | (addr & 0x7ff);
			break;
		}

		case EREG_INTERRUPT:
			ethernet.reg[EREG_INTERRUPT] &= ~(data & 0x56);
			update_ethernet_irq();
			break;
	}
}
