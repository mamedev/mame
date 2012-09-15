/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles

***************************************************************************

    Notes:
        * only loopback mode really works

**************************************************************************/

#include "emu.h"
#include "smc91c9x.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_ETHERNET		(0)
#define DISPLAY_STATS		(0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

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
static const char *const ethernet_regname[64] =
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



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct smc91c9x_state
{
	device_t *device;
	smc91c9x_irq_func irq_handler;

	/* raw register data and masks */
	UINT16			reg[64];
	UINT16			regmask[64];

	/* IRQ information */
	UINT8			irq_state;

	/* allocate information */
	UINT8			alloc_count;

	/* transmit/receive FIFOs */
	UINT8			fifo_count;
	UINT8			rx[ETHER_BUFFER_SIZE * ETHER_RX_BUFFERS];
	UINT8			tx[ETHER_BUFFER_SIZE];

	/* counters */
	UINT32			sent;
	UINT32			recd;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void update_ethernet_irq(smc91c9x_state *smc);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, an IDE controller
-------------------------------------------------*/

INLINE smc91c9x_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SMC91C94 || device->type() == SMC91C96);

	return (smc91c9x_state *)downcast<smc91c9x_device *>(device)->token();
}



/***************************************************************************
    INTERNAL HELPERS
***************************************************************************/

/*-------------------------------------------------
    update_ethernet_irq - update the IRQ state
-------------------------------------------------*/

static void update_ethernet_irq(smc91c9x_state *smc)
{
	UINT8 mask = smc->reg[EREG_INTERRUPT] >> 8;
	UINT8 state = smc->reg[EREG_INTERRUPT] & 0xff;

	/* update the IRQ state */
	smc->irq_state = ((mask & state) != 0);
	if (smc->irq_handler != NULL)
		(*smc->irq_handler)(smc->device, smc->irq_state ? ASSERT_LINE : CLEAR_LINE);
}


/*-------------------------------------------------
    update_stats - draw statistics
-------------------------------------------------*/

static void update_stats(smc91c9x_state *smc)
{
	if (DISPLAY_STATS)
		popmessage("Sent:%d  Rec'd:%d", smc->sent, smc->recd);
}


/*-------------------------------------------------
    finish_enqueue - complete an enqueued packet
-------------------------------------------------*/

static void finish_enqueue(smc91c9x_state *smc, int param)
{
	int is_broadcast = (smc->tx[4] == 0xff && smc->tx[5] == 0xff && smc->tx[6] == 0xff &&
						smc->tx[7] == 0xff && smc->tx[8] == 0xff && smc->tx[9] == 0xff);

	/* update the EPH register and stuff it in the first transmit word */
	smc->reg[EREG_EPH_STATUS] = 0x0001;
	if (is_broadcast)
		smc->reg[EREG_EPH_STATUS] |= 0x0040;
	smc->tx[0] = smc->reg[EREG_EPH_STATUS];
	smc->tx[1] = smc->reg[EREG_EPH_STATUS] >> 8;

	/* signal a transmit interrupt and mark the transmit buffer empty */
	smc->reg[EREG_INTERRUPT] |= EINT_TX;
	smc->reg[EREG_INTERRUPT] |= EINT_TX_EMPTY;
	smc->reg[EREG_FIFO_PORTS] |= 0x0080;
	smc->sent++;
	update_stats(smc);

	/* loopback? */
	if (smc->reg[EREG_TCR] & 0x2002)
		if (smc->fifo_count < ETHER_RX_BUFFERS)
		{
			int buffer_len = ((smc->tx[3] << 8) | smc->tx[2]) & 0x7ff;
			UINT8 *packet = &smc->rx[smc->fifo_count++ * ETHER_BUFFER_SIZE];
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
			memcpy(&packet[4], &smc->tx[4], 6);
			memcpy(&packet[10], &smc->tx[10], 6);
			memcpy(&packet[16], &smc->tx[16], buffer_len - 16);

			/* set the broadcast flag */
			if (is_broadcast)
				packet[1] |= 0x40;

			/* pad? */
			if (smc->reg[EREG_TCR & 0x0080])
				if (packet_len < 64)
				{
					memset(&packet[buffer_len], 0, 64+6 - buffer_len);
					packet[buffer_len - 1] = 0;
					buffer_len = 64+6;
					packet[2] = buffer_len;
					packet[3] = buffer_len >> 8;
				}

			/* signal a receive */
			smc->reg[EREG_INTERRUPT] |= EINT_RCV;
			smc->reg[EREG_FIFO_PORTS] &= ~0x8000;
		}
	update_ethernet_irq(smc);
}


/*-------------------------------------------------
    process_command - handle MMU commands
-------------------------------------------------*/

static void process_command(smc91c9x_state *smc, UINT16 data)
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
			smc->reg[EREG_PNR_ARR] &= ~0xff00;
			smc->reg[EREG_PNR_ARR] |= smc->alloc_count++ << 8;
			smc->reg[EREG_INTERRUPT] |= 0x0008;
			update_ethernet_irq(smc);
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
			smc->reg[EREG_INTERRUPT] &= ~EINT_RCV;
			if (smc->fifo_count > 0)
				smc->fifo_count--;
			if (smc->fifo_count > 0)
			{
				memmove(&smc->rx[0], &smc->rx[ETHER_BUFFER_SIZE], smc->fifo_count * ETHER_BUFFER_SIZE);
				smc->reg[EREG_INTERRUPT] |= EINT_RCV;
				smc->reg[EREG_FIFO_PORTS] &= ~0x8000;
			}
			else
				smc->reg[EREG_FIFO_PORTS] |= 0x8000;
			update_ethernet_irq(smc);
			smc->recd++;
			update_stats(smc);
			break;

		case ECMD_RELEASE_PACKET:
			if (LOG_ETHERNET)
				logerror("   RELEASE SPECIFIC PACKET\n");
			break;

		case ECMD_ENQUEUE_PACKET:
			if (LOG_ETHERNET)
				logerror("   ENQUEUE TX PACKET\n");
			finish_enqueue(smc, 0);
			break;

		case ECMD_RESET_FIFOS:
			if (LOG_ETHERNET)
				logerror("   RESET TX FIFOS\n");
			break;
	}
	smc->reg[EREG_MMU_COMMAND] &= ~0x0001;
}



/***************************************************************************
    CORE READ/WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    smc91c9x_r - handle a read from the device
-------------------------------------------------*/

READ16_DEVICE_HANDLER( smc91c9x_r )
{
	smc91c9x_state *smc = get_safe_token(device);
	UINT32 result = ~0;

	/* determine the effective register */
	offset %= 8;
	if (offset != EREG_BANK)
		offset += 8 * (smc->reg[EREG_BANK] & 7);
	result = smc->reg[offset];

	switch (offset)
	{
		case EREG_PNR_ARR:
			if (ACCESSING_BITS_8_15)
			{
				smc->reg[EREG_INTERRUPT] &= ~0x0008;
				update_ethernet_irq(smc);
			}
			break;

		case EREG_DATA_0:	/* data register */
		case EREG_DATA_1:	/* data register */
		{
			UINT8 *buffer = (smc->reg[EREG_POINTER] & 0x8000) ? smc->rx : smc->tx;
			int addr = smc->reg[EREG_POINTER] & 0x7ff;
			result = buffer[addr++];
			if (ACCESSING_BITS_8_15)
				result |= buffer[addr++] << 8;
			if (smc->reg[EREG_POINTER] & 0x4000)
				smc->reg[EREG_POINTER] = (smc->reg[EREG_POINTER] & ~0x7ff) | (addr & 0x7ff);
			break;
		}
	}

	if (LOG_ETHERNET && offset != EREG_BANK)
		logerror("%s:smc91c9x_r(%s) = %04X & %04X\n", device->machine().describe_context(), ethernet_regname[offset], result, mem_mask);
	return result;
}


/*-------------------------------------------------
    smc91c9x_w - handle a write to the device
-------------------------------------------------*/

WRITE16_DEVICE_HANDLER( smc91c9x_w )
{
	smc91c9x_state *smc = get_safe_token(device);
//  UINT16 olddata;

	/* determine the effective register */
	offset %= 8;
	if (offset != EREG_BANK)
		offset += 8 * (smc->reg[EREG_BANK] & 7);

	/* update the data generically */
//  olddata = smc->reg[offset];
	mem_mask &= smc->regmask[offset];
	COMBINE_DATA(&smc->reg[offset]);

	if (LOG_ETHERNET && offset != 7)
		logerror("%s:smc91c9x_w(%s) = %04X & %04X\n", device->machine().describe_context(), ethernet_regname[offset], data, mem_mask);

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
				if (data & 0x8000) device->reset();
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
			process_command(smc, data);
			break;

		case EREG_DATA_0:	/* data register */
		case EREG_DATA_1:	/* data register */
		{
			UINT8 *buffer = (smc->reg[EREG_POINTER] & 0x8000) ? smc->rx : smc->tx;
			int addr = smc->reg[EREG_POINTER] & 0x7ff;
			buffer[addr++] = data;
			if (ACCESSING_BITS_8_15)
				buffer[addr++] = data >> 8;
			if (smc->reg[EREG_POINTER] & 0x4000)
				smc->reg[EREG_POINTER] = (smc->reg[EREG_POINTER] & ~0x7ff) | (addr & 0x7ff);
			break;
		}

		case EREG_INTERRUPT:
			smc->reg[EREG_INTERRUPT] &= ~(data & 0x56);
			update_ethernet_irq(smc);
			break;
	}
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( smc91c9x )
{
	const smc91c9x_config *config = (const smc91c9x_config *)device->static_config();
	smc91c9x_state *smc = get_safe_token(device);

	/* validate some basic stuff */
	assert(device != NULL);

	/* store a pointer back to the device */
	smc->device = device;
	smc->irq_handler = config->interrupt;

	/* register ide states */
	device->save_item(NAME(smc->reg));
	device->save_item(NAME(smc->regmask));
	device->save_item(NAME(smc->irq_state));
	device->save_item(NAME(smc->alloc_count));
	device->save_item(NAME(smc->fifo_count));
	device->save_item(NAME(smc->rx));
	device->save_item(NAME(smc->tx));
	device->save_item(NAME(smc->sent));
	device->save_item(NAME(smc->recd));
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( smc91c9x )
{
	smc91c9x_state *smc = get_safe_token(device);

	memset(smc->reg, 0, sizeof(smc->reg));
	memset(smc->regmask, 0, sizeof(smc->regmask));
	smc->irq_state = 0;
	smc->alloc_count = 0;
	smc->fifo_count = 0;
	smc->sent = 0;
	smc->recd = 0;

	smc->reg[EREG_TCR]			= 0x0000;	smc->regmask[EREG_TCR]			= 0x3d87;
	smc->reg[EREG_EPH_STATUS]	= 0x0000;	smc->regmask[EREG_EPH_STATUS]	= 0x0000;
	smc->reg[EREG_RCR]			= 0x0000;	smc->regmask[EREG_RCR]			= 0xc307;
	smc->reg[EREG_COUNTER]		= 0x0000;	smc->regmask[EREG_COUNTER]		= 0x0000;
	smc->reg[EREG_MIR]			= 0x1212;	smc->regmask[EREG_MIR]			= 0x0000;
	smc->reg[EREG_MCR]			= 0x3300;	smc->regmask[EREG_MCR]			= 0x00ff;
	smc->reg[EREG_BANK]			= 0x3300;	smc->regmask[EREG_BANK]			= 0x0007;

	smc->reg[EREG_CONFIG]		= 0x0030;	smc->regmask[EREG_CONFIG]		= 0x17c6;
	smc->reg[EREG_BASE]			= 0x1866;	smc->regmask[EREG_BASE]			= 0xfffe;
	smc->reg[EREG_IA0_1]		= 0x0000;	smc->regmask[EREG_IA0_1]		= 0xffff;
	smc->reg[EREG_IA2_3]		= 0x0000;	smc->regmask[EREG_IA2_3]		= 0xffff;
	smc->reg[EREG_IA4_5]		= 0x0000;	smc->regmask[EREG_IA4_5]		= 0xffff;
	smc->reg[EREG_GENERAL_PURP]	= 0x0000;	smc->regmask[EREG_GENERAL_PURP]	= 0xffff;
	smc->reg[EREG_CONTROL]		= 0x0100;	smc->regmask[EREG_CONTROL]		= 0x68e7;

	smc->reg[EREG_MMU_COMMAND]	= 0x0000;	smc->regmask[EREG_MMU_COMMAND]	= 0x00e7;
	smc->reg[EREG_PNR_ARR]		= 0x8000;	smc->regmask[EREG_PNR_ARR]		= 0x00ff;
	smc->reg[EREG_FIFO_PORTS]	= 0x8080;	smc->regmask[EREG_FIFO_PORTS]	= 0x0000;
	smc->reg[EREG_POINTER]		= 0x0000;	smc->regmask[EREG_POINTER]		= 0xf7ff;
	smc->reg[EREG_DATA_0]		= 0x0000;	smc->regmask[EREG_DATA_0]		= 0xffff;
	smc->reg[EREG_DATA_1]		= 0x0000;	smc->regmask[EREG_DATA_1]		= 0xffff;
	smc->reg[EREG_INTERRUPT]	= 0x0004;	smc->regmask[EREG_INTERRUPT]	= 0x7f00;

	smc->reg[EREG_MT0_1]		= 0x0000;	smc->regmask[EREG_MT0_1]		= 0xffff;
	smc->reg[EREG_MT2_3]		= 0x0000;	smc->regmask[EREG_MT2_3]		= 0xffff;
	smc->reg[EREG_MT4_5]		= 0x0000;	smc->regmask[EREG_MT4_5]		= 0xffff;
	smc->reg[EREG_MT6_7]		= 0x0000;	smc->regmask[EREG_MT6_7]		= 0xffff;
	smc->reg[EREG_MGMT]			= 0x3030;	smc->regmask[EREG_MGMT]			= 0x0f0f;
	smc->reg[EREG_REVISION]		= 0x3340;	smc->regmask[EREG_REVISION]		= 0x0000;
	smc->reg[EREG_ERCV]			= 0x331f;	smc->regmask[EREG_ERCV]			= 0x009f;

	update_ethernet_irq(smc);
}


smc91c9x_device::smc91c9x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(smc91c9x_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void smc91c9x_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void smc91c9x_device::device_start()
{
	DEVICE_START_NAME( smc91c9x )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void smc91c9x_device::device_reset()
{
	DEVICE_RESET_NAME( smc91c9x )(this);
}


const device_type SMC91C94 = &device_creator<smc91c94_device>;

smc91c94_device::smc91c94_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: smc91c9x_device(mconfig, SMC91C94, "SMC91C94", tag, owner, clock)
{
}


const device_type SMC91C96 = &device_creator<smc91c96_device>;

smc91c96_device::smc91c96_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: smc91c9x_device(mconfig, SMC91C96, "SMC91C96", tag, owner, clock)
{
}



