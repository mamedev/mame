// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edstrom
/***************************************************************************

    Z80-SCC Serial Communications Controller emulation

    The z80scc is an updated version of the z80sio, with additional support for CRC
    checks and a number of data link layer protocols such as HDLC, SDLC and BiSync.
    (See https://en.wikipedia.org/wiki/Zilog_SCC). The variants in the SCC
    family are as follows:
           Zbus    Universal bus
    NMOS   Z8030   Z8530
    CMOS   Z80C30  Z85C30
    ESCC   Z80230  Z85230, Z8523L (L = low voltage)
    EMSCC          Z85233
    The difference between Zbus and Universal bus is mainly at hardware
    design level and suitable for Intel oriented (Zbus) or Motorola oriented
    chip designs.

TODO:
                            NMOS         CMOS       ESCC      EMSCC
    -------------------------------------------------------------------
    Channels                2 FD         2 FD       2 FD      2 FD         FD = Full Duplex
    Synch data rates        2Mbps        4Mbps      5Mbps     5Mbps
                            1Mbps (FM)
                           .5Mbps (NRZI)
    -- asynchrounous features -------------------------------------------
    5-8 bit per char         Y             Y          Y         Y
    1,1.5,2 stop bits        Y             Y          Y         Y
    odd/even parity          Y             Y          Y         Y
    x1,x16,x32,x64           Y             Y          Y         Y
    break det/gen            Y             Y          Y         Y
    parity, framing &        Y             Y          Y         Y
    overrun error det        Y             Y          Y         Y
    -- byte oriented synchrounous features -------------------------------
    Int/ext char sync        Y             Y          Y         Y
    1/2 synch chars          Y             Y          Y         Y
    Aut CRC gen/det          Y             Y          Y         Y
    -- SDLC/HDLC capabilities --------------------------------------------
    Abort seq gen/chk        Y             Y          Y         Y
    Aut zero ins/det         Y             Y          Y         Y
    Aut flag insert          Y             Y          Y         Y
    Addr field rec           Y             Y          Y         Y
    I-fld resid hand         Y             Y          Y         Y
    CRC gen/det              Y             Y          Y         Y
    SDLC loop w EOP          Y             Y          Y         Y
    --
    Receiver FIFO            3             3          8         8
    Transmitter FIFO         1             1          4         4
    NRZ, NRZI or             Y             Y          Y         Y
     FM enc/dec              Y             Y          Y         Y
    Manchester dec           Y             Y          Y         Y
    Baud gen per chan        Y             Y          Y         Y
    DPLL clock recov         Y             Y          Y         Y
    -- Additional features CMOS versions -----------------------------------
    Status FIFO              N             Y          Y         Y
    SWI ack feat             N             Y          Y         Y
    higher bps w ext DPLL    N           32Mbps     32Mbps    32Mbps
    -- Additional features 85C30 -------------------------------------------
    New WR7 feat             N           85C30        Y         Y
    Improved SDLC            N           85C30        Y         Y
    Improved reg handl       N           85C30        Y         Y
    Improved auto feat       N           85C30        Y         Y
    -- Additional features ESCC   -------------------------------------------
    Progrmbl FIFO int &
     DMA req lev             Y             Y          Y         Y
    Improved SDLC            Y             Y          Y         Y
    DPLL counter as TXc      Y             Y          Y         Y
    Improved reg handl       Y             Y          Y         Y
    -------------------------------------------------------------------------
    * = Features that has been implemented  n/a = features that will not
***************************************************************************/

#include "z80scc.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#if VERBOSE == 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define CHANA_TAG   "cha"
#define CHANB_TAG   "chb"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************
// device type definition
const device_type Z80SCC   = &device_creator<z80scc_device>;
const device_type Z80SCC_CHANNEL = &device_creator<z80scc_channel>;
const device_type SCC8030  = &device_creator<scc8030_device>;
const device_type SCC80C30 = &device_creator<scc80C30_device>;
const device_type SCC80230 = &device_creator<scc80230_device>;
const device_type SCC8530N = &device_creator<scc8530_device>;  // remove trailing N when 8530scc.c is fully replaced and removed
const device_type SCC85C30 = &device_creator<scc85C30_device>;
const device_type SCC85230 = &device_creator<scc85230_device>;
const device_type SCC85233 = &device_creator<scc85233_device>;
const device_type SCC8523L = &device_creator<scc8523L_device>;

//-------------------------------------------------
//  device_mconfig_additions -
//-------------------------------------------------
MACHINE_CONFIG_FRAGMENT( z80scc )
	MCFG_DEVICE_ADD(CHANA_TAG, Z80SCC_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANB_TAG, Z80SCC_CHANNEL, 0)
MACHINE_CONFIG_END

machine_config_constructor z80scc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( z80scc );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80scc_device - constructor
//-------------------------------------------------

z80scc_device::z80scc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_z80daisy_interface(mconfig, *this),
		m_chanA(*this, CHANA_TAG),
		m_chanB(*this, CHANB_TAG),
		m_rxca(0),
		m_txca(0),
		m_rxcb(0),
		m_txcb(0),
		m_out_txda_cb(*this),
		m_out_dtra_cb(*this),
		m_out_rtsa_cb(*this),
		m_out_wrdya_cb(*this),
		m_out_synca_cb(*this),
		m_out_txdb_cb(*this),
		m_out_dtrb_cb(*this),
		m_out_rtsb_cb(*this),
		m_out_wrdyb_cb(*this),
		m_out_syncb_cb(*this),
		m_out_int_cb(*this),
		m_out_rxdrqa_cb(*this),
		m_out_txdrqa_cb(*this),
		m_out_rxdrqb_cb(*this),
		m_out_txdrqb_cb(*this),
		m_variant(variant)
{
	for (int i = 0; i < 6; i++)
		m_int_state[i] = 0;
}

z80scc_device::z80scc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Z80SCC, "Z80 SCC", tag, owner, clock, "z80scc", __FILE__),
		device_z80daisy_interface(mconfig, *this),
		m_chanA(*this, CHANA_TAG),
		m_chanB(*this, CHANB_TAG),
		m_rxca(0),
		m_txca(0),
		m_rxcb(0),
		m_txcb(0),
		m_out_txda_cb(*this),
		m_out_dtra_cb(*this),
		m_out_rtsa_cb(*this),
		m_out_wrdya_cb(*this),
		m_out_synca_cb(*this),
		m_out_txdb_cb(*this),
		m_out_dtrb_cb(*this),
		m_out_rtsb_cb(*this),
		m_out_wrdyb_cb(*this),
		m_out_syncb_cb(*this),
		m_out_int_cb(*this),
		m_out_rxdrqa_cb(*this),
		m_out_txdrqa_cb(*this),
		m_out_rxdrqb_cb(*this),
		m_out_txdrqb_cb(*this),
		m_variant(TYPE_Z80SCC)
{
	for (int i = 0; i < 6; i++)
		m_int_state[i] = 0;
}

scc8030_device::scc8030_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80scc_device(mconfig, SCC8030, "SCC 8030", tag, owner, clock, TYPE_SCC8030, "scc8030", __FILE__){ }

scc80C30_device::scc80C30_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80scc_device(mconfig, SCC80C30, "SCC 80C30", tag, owner, clock, TYPE_SCC80C30, "scc80C30", __FILE__){ }

scc80230_device::scc80230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80scc_device(mconfig, SCC80230, "SCC 80230", tag, owner, clock, TYPE_SCC80230, "scc80230", __FILE__){ }

scc8530_device::scc8530_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80scc_device(mconfig, SCC8530N, "SCC 8530", tag, owner, clock, TYPE_SCC8530, "scc8530", __FILE__){ }

scc85C30_device::scc85C30_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80scc_device(mconfig, SCC85C30, "SCC 85C30", tag, owner, clock, TYPE_SCC85C30, "scc85C30", __FILE__){ }

scc85230_device::scc85230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80scc_device(mconfig, SCC85230, "SCC 85230", tag, owner, clock, TYPE_SCC85230, "scc85230", __FILE__){ }

scc85233_device::scc85233_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80scc_device(mconfig, SCC85233, "SCC 85233", tag, owner, clock, TYPE_SCC85233, "scc85233", __FILE__){ }

scc8523L_device::scc8523L_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80scc_device(mconfig, SCC8523L, "SCC 8523L", tag, owner, clock, TYPE_SCC8523L, "scc8523L", __FILE__){ }

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80scc_device::device_start()
{
		LOG(("%s\n", FUNCNAME));
	// resolve callbacks
	m_out_txda_cb.resolve_safe();
	m_out_dtra_cb.resolve_safe();
	m_out_rtsa_cb.resolve_safe();
	m_out_wrdya_cb.resolve_safe();
	m_out_synca_cb.resolve_safe();
	m_out_txdb_cb.resolve_safe();
	m_out_dtrb_cb.resolve_safe();
	m_out_rtsb_cb.resolve_safe();
	m_out_wrdyb_cb.resolve_safe();
	m_out_syncb_cb.resolve_safe();
	m_out_int_cb.resolve_safe();
	m_out_rxdrqa_cb.resolve_safe();
	m_out_txdrqa_cb.resolve_safe();
	m_out_rxdrqb_cb.resolve_safe();
	m_out_txdrqb_cb.resolve_safe();

	// configure channel A
	m_chanA->m_rxc = m_rxca;
	m_chanA->m_txc = m_txca;

	// configure channel B
	m_chanB->m_rxc = m_rxcb;
	m_chanB->m_txc = m_txcb;

	// state saving
	save_item(NAME(m_int_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z80scc_device::device_reset()
{
		LOG(("%s %s \n",FUNCNAME, tag()));

	m_chanA->reset();
	m_chanB->reset();
}

/*
 * Interrupts
Each of the SCC's two channels contain three sources of interrupts, making a total of six interrupt
sources. These three sources of interrupts are: 1) Receiver, 2) Transmitter, and 3) External/Status
conditions. In addition, there are several conditions that may cause these interrupts.*/
//-------------------------------------------------
//  z80daisy_irq_state - get interrupt status
//-------------------------------------------------

int z80scc_device::z80daisy_irq_state()
{
	int state = 0;
	int i;

	LOG(("Z80SCC \"%s\" : Interrupt State A:%d%d%d%d B:%d%d%d%d\n", tag(),
				m_int_state[0], m_int_state[1], m_int_state[2], m_int_state[3],
				m_int_state[4], m_int_state[5], m_int_state[6], m_int_state[7]));

	// loop over all interrupt sources
	for (i = 0; i < 6; i++)
	{
		// if we're servicing a request, don't indicate more interrupts
		if (m_int_state[i] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= m_int_state[i];
	}

	LOG(("Z80SCC \"%s\" : Interrupt State %u\n", tag(), state));

	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - interrupt acknowledge
//-------------------------------------------------

int z80scc_device::z80daisy_irq_ack()
{
	int i;

	LOG(("Z80SCC \"%s\" Interrupt Acknowledge\n", tag()));

	// loop over all interrupt sources
	for (i = 0; i < 6; i++)
	{
		// find the first channel with an interrupt requested
		if (m_int_state[i] & Z80_DAISY_INT)
		{
			// clear interrupt, switch to the IEO state, and update the IRQs
			m_int_state[i] = Z80_DAISY_IEO;
			m_chanA->m_rr0 &= ~z80scc_channel::RR0_INTERRUPT_PENDING;
			check_interrupts();

			LOG(("Z80SCC \"%s\" : Interrupt Acknowledge Vector %02x\n", tag(), m_chanB->m_rr2));

			return m_chanB->m_rr2;
		}
	}

	//logerror("z80scc_irq_ack: failed to find an interrupt to ack!\n");

	return m_chanB->m_rr2;
}


//-------------------------------------------------
//  z80daisy_irq_reti - return from interrupt
//-------------------------------------------------

void z80scc_device::z80daisy_irq_reti()
{
	int i;

	LOG(("Z80SCC \"%s\" Return from Interrupt\n", tag()));

	// loop over all interrupt sources
	for (i = 0; i < 6; i++)
	{
		// find the first channel with an IEO pending
		if (m_int_state[i] & Z80_DAISY_IEO)
		{
			// clear the IEO state and update the IRQs
			m_int_state[i] &= ~Z80_DAISY_IEO;
			check_interrupts();
			return;
		}
	}

	//logerror("z80scc_irq_reti: failed to find an interrupt to clear IEO on!\n");
}


//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

void z80scc_device::check_interrupts()
{
	int state = (z80daisy_irq_state() & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE;
		LOG(("Z80SCC \"%s\" : %s() state = %d\n", m_owner->tag(), __func__, state));
	m_out_int_cb(state);
}


//-------------------------------------------------
//  reset_interrupts -
//-------------------------------------------------

void z80scc_device::reset_interrupts()
{
		// reset internal interrupi sources
	for (int i = 0; i < 6; i++)
	{
		m_int_state[i] = 0;
	}

		// check external interrupt sources
	check_interrupts();
}

UINT8 z80scc_device::modify_vector(UINT8 vec, int i, UINT8 src)
{
		/*
		  Interrupt Vector Modification
		  V3 V2 V1 Status High/Status Low =0
		  V4 V5 V6 Status High/Status Low =1
		  0  0  0 Ch B Transmit Buffer Empty
		  0  0  1 Ch B External/Status Change
		  0  1  0 Ch B Receive Char. Available
		  0  1  1 Ch B Special Receive Condition
		  1  0  0 Ch A Transmit Buffer Empty
		  1  0  1 Ch A External/Status Change
		  1  1  0 Ch A Receive Char. Available
		  1  1  1 Ch A Special Receive Condition
		*/
		// Add channel offset according to table above
		src |= (i == CHANNEL_A  ? 0x04 : 0x00 );

		// Modify vector according to Hi/lo bit of WR9
		if (m_chanA->m_wr9 & z80scc_channel::WR9_BIT_SHSL) // Affect V4-V6
		{
				vec |= src << 4;
		}
		else                      // Affect V1-V3
		{
				vec |= src << 1;
		}
		return vec;
}


//-------------------------------------------------
//  trigger_interrupt -
//-------------------------------------------------
void z80scc_device::trigger_interrupt(int index, int state)
{
	UINT8 vector = m_chanB->m_wr2;
		UINT8 source = 0;
	int priority;

		int prio_level = 0;

		/* The Master Interrupt Enable (MIE) bit, WR9 D3, must be set to enable the SCC to generate interrupts.*/
		if (!(m_chanA->m_wr9 & z80scc_channel::WR9_BIT_MIE))
		{
				LOG(("Master Interrupt Enable is not set, blocking attempt to interrupt\n"));
				return;
		}

		switch(state)
	{
		case z80scc_channel::INT_RECEIVE:
				/*The sources of receive interrupts consist of Receive Character Available and Special Receive Condition.
				  The Special Receive Condition can be subdivided into Receive Overrun, Framing Error (Asynchronous) or
				  End of Frame (SDLC). In addition, a parity error can be a special receive condition by programming*/
				source = 2;
				prio_level = 2;
				break;
		case z80scc_channel::INT_TRANSMIT:
				/*The NMOS/CMOS version of the SCC only has a one byte deep transmit buffer. The status of the
				  transmit buffer can be determined through TBE bit in RR0, bit D2, which shows whether the
				  transmit buffer is empty or not. After a hardware reset (including a hardware reset by software), or
				  a channel reset, this bit is set to 1.
				  While transmit interrupts are enabled, the NMOS/CMOS version sets the Transmit Interrupt Pending
				  (TxIP) bit whenever the transmit buffer becomes empty. This means that the transmit buffer
				  must be full before the TxIP can be set. Thus, when transmit interrupts are first enabled, the TxIP
				  will not be set until after the first character is written to the NMOS/CMOS.*/
				source = 0;
				prio_level = 1;
				break;
		case z80scc_channel::INT_SPECIAL:
				/*This mode allows the receiver to interrupt only on
				  characters with a special receive condition. When an interrupt occurs, the data containing the error
				  is held in the Receive FIFO until an Error Reset command is issued. When using this mode in conjunction
				  with a DMA, the DMA is initialized and enabled before any characters have been
				  received by the ESCC. This eliminates the time-critical section of code required in the Receive
				  Interrupt on First Character or Special Condition mode. Hence, all data can be transferred via the
				  DMA so that the CPU need not handle the first received character as a special case. In SDLC
				  mode, if the SDLC Frame Status FIFO is enabled and an EOF is received, an interrupt with vector
				  for receive data available is generated and the Receive FIFO is not locked.*/
				source = 3;
				prio_level = 0;
				break;
		default:
				logerror("Attempt to trigger interrupt of unknown origin blocked: %02x on channel %c\n", state, 'A' + index);
				return;
		}

		// Vector modification requested?
		if (m_chanA->m_wr9 & z80scc_channel::WR9_BIT_VIS)
		{
				vector = modify_vector(vector, index, source);
		}

	LOG(("Z80SCC \"%s\" Channel %c : Interrupt Request %u\n", tag(), 'A' + index, state));

	// update vector register // TODO: What if interrupts are nested? May we loose the modified vector or even get the wrong one?
	m_chanB->m_wr2 = vector;

		/* Check the interrupt source and build the vector modification */
		/*Interrupt Source Priority order
		  Channel A Receive
		  Channel A Transmit
		  Channel A External/Status
		  Channel B Receive
		  Channel B Transmit
		  Channel B External/Status
		*/
		// Add channel offset to priority according to table above
		priority = prio_level + (index == CHANNEL_A ? 3 : 0 );

	// trigger interrupt
	m_int_state[priority] |= Z80_DAISY_INT;

		// Based on the fact that prio levels are aligned with the bitorder of rr3 we can do this...
	m_chanA->m_rr3 |= (prio_level << (index == CHANNEL_A ? 3 : 0 ));

	// check for interrupt
	check_interrupts();
}


//-------------------------------------------------
//  m1_r - interrupt acknowledge
//-------------------------------------------------

int z80scc_device::m1_r()
{
	return z80daisy_irq_ack();
}


//-------------------------------------------------
//  cd_ba_r - Universal Bus read
//-------------------------------------------------
READ8_MEMBER( z80scc_device::cd_ba_r )
{
	int ba = BIT(offset, 0);
	int cd = BIT(offset, 1);
	z80scc_channel *channel = ba ? m_chanB : m_chanA;

		/* Expell non-Universal Bus variants */
		if ( !(m_variant & SET_Z85X3X))
		{
				logerror("Z80SCC cd_ba_r not supported by this device variant, you should probably use combinations of c*_r/w and d*_r/w (see z80scc.h)\n");
				return 0;
		}

		//        LOG(("z80scc_device::cd_ba_r ba:%02x cd:%02x\n", ba, cd));
	return cd ? channel->control_read() : channel->data_read();
}

//-------------------------------------------------
//  cd_ba_w - Universal Bus write
//-------------------------------------------------
WRITE8_MEMBER( z80scc_device::cd_ba_w )
{
	int ba = BIT(offset, 0);
	int cd = BIT(offset, 1);
	z80scc_channel *channel = ba ? m_chanB : m_chanA;

		/* Expell non-Universal Bus variants */
		if ( !(m_variant & SET_Z85X3X) )
		{
				logerror("Z80SCC cd_ba_w not supported by this device variant, you should probably use combinations of c*_r/w and d*_r/w (see z80scc.h)\n");
				return;
		}

		//        LOG(("z80scc_device::cd_ba_w ba:%02x cd:%02x\n", ba, cd));
	if (cd)
		channel->control_write(data);
	else
		channel->data_write(data);
}


//-------------------------------------------------
//  ba_cd_r - Universal Bus read
//-------------------------------------------------

READ8_MEMBER( z80scc_device::ba_cd_r )
{
	int ba = BIT(offset, 1);
	int cd = BIT(offset, 0);
	z80scc_channel *channel = ba ? m_chanB : m_chanA;

		/* Expell non-Universal Bus variants */
		if ( !(m_variant & SET_Z85X3X) )
		{
				logerror("Z80SCC ba_cd_r not supported by this device variant, you should probably use combinations of c*_r/w and d*_r/w (see z80scc.h)\n");
				return 0;
		}

		//        LOG(("z80scc_device::ba_cd_r ba:%02x cd:%02x\n", ba, cd));
	return cd ? channel->control_read() : channel->data_read();
}


//-------------------------------------------------
//  ba_cd_w -
//-------------------------------------------------

WRITE8_MEMBER( z80scc_device::ba_cd_w )
{
	int ba = BIT(offset, 1);
	int cd = BIT(offset, 0);
	z80scc_channel *channel = ba ? m_chanB : m_chanA;

		/* Expell non-Universal Bus variants */
		if ( !(m_variant & SET_Z85X3X) )
		{
				logerror("Z80SCC ba_cd_w not supported by this device variant, you should probably use combinations of c*_r/w and d*_r/w (see z80scc.h)\n");
				return;
		}

		LOG(("z80scc_device::ba_cd_w ba:%02x cd:%02x\n", ba, cd));
	if (cd)
		channel->control_write(data);
	else
		channel->data_write(data);
}

//**************************************************************************
//  SCC CHANNEL
//**************************************************************************

//-------------------------------------------------
//  SCC_channel - constructor
//-------------------------------------------------

z80scc_channel::z80scc_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: z80sio_channel(                            mconfig,             tag,           owner,        clock)
{
		// Reset all SCC specific registers; z80sio_channel:: manages the base registers
		m_rr0 = m_rr1 = m_rr2 =
				m_rr3  = m_rr4  = m_rr5  = m_rr6  = m_rr7  = m_rr8  = m_rr9 =
				m_rr10 = m_rr11 = m_rr12 = m_rr13 = m_rr14 = m_rr15 = 0;
		m_wr0 = m_wr1 = m_wr2 = m_wr3  = m_wr4  = m_wr5  = m_wr6  = m_wr7 =
				m_wr8  = m_wr9  = m_wr10 = m_wr11 = m_wr12 = m_wr13 = m_wr14 = m_wr15 = 0;

	for (int i = 0; i < 3; i++) // TODO adapt to SCC fifos
	{
		m_rx_data_fifo[i] = 0;
		m_rx_error_fifo[i] = 0;
	}
}


//-------------------------------------------------
//  start - channel startup
//-------------------------------------------------

void z80scc_channel::device_start()
{
	m_uart = downcast<z80scc_device *>(owner());
		LOG(("%s\n", FUNCNAME));
	m_index = m_uart->get_channel_index(this);
		m_ph = 0;
		m_variant = ((z80scc_device *)m_owner)->m_variant;

		m_rx_fifo_sz = (m_variant & SET_ESCC) ? 8 : 3;
		m_rx_fifo_wp = m_rx_fifo_rp = 0;

	// state saving
		//  m_rr0-m_rr2 is handled by the z80sio_channel driver, our base class
	save_item(NAME(m_rr0));
	save_item(NAME(m_rr1));
	save_item(NAME(m_rr2));
	save_item(NAME(m_rr3));
	save_item(NAME(m_rr4));
	save_item(NAME(m_rr5));
	save_item(NAME(m_rr6));
	save_item(NAME(m_rr7));
	save_item(NAME(m_rr8));
	save_item(NAME(m_rr9));
	save_item(NAME(m_rr10));
	save_item(NAME(m_rr11));
	save_item(NAME(m_rr12));
	save_item(NAME(m_rr13));
	save_item(NAME(m_rr14));
	save_item(NAME(m_rr15));
		//  m_wr0-m_wr7 is handled by the z80sio_channel driver, our base class
	save_item(NAME(m_wr0));
	save_item(NAME(m_wr1));
	save_item(NAME(m_wr2));
	save_item(NAME(m_wr3));
	save_item(NAME(m_wr4));
	save_item(NAME(m_wr5));
	save_item(NAME(m_wr6));
	save_item(NAME(m_wr7));
	save_item(NAME(m_wr8));
	save_item(NAME(m_wr9));
	save_item(NAME(m_wr10));
	save_item(NAME(m_wr11));
	save_item(NAME(m_wr12));
	save_item(NAME(m_wr13));
	save_item(NAME(m_wr14));
	save_item(NAME(m_wr15));
	save_item(NAME(m_rx_data_fifo));
	save_item(NAME(m_rx_error_fifo));
	save_item(NAME(m_rx_fifo_rp));
	save_item(NAME(m_rx_fifo_wp));
	save_item(NAME(m_rx_fifo_sz));
	save_item(NAME(m_rx_clock));
	save_item(NAME(m_rx_first));
	save_item(NAME(m_rx_break));
	save_item(NAME(m_rx_rr0_latch));
	save_item(NAME(m_ri));
	save_item(NAME(m_cts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_tx_clock));
	save_item(NAME(m_dtr));
	save_item(NAME(m_rts));
	save_item(NAME(m_sync));
	save_item(NAME(m_ph));
	device_serial_interface::register_save_state(machine().save(), this);
}


//-------------------------------------------------
//  reset - reset channel status
//-------------------------------------------------

void z80scc_channel::device_reset()
{
		LOG(("Z80SCC \"%s\" Channel %c : %s\n", m_owner->tag(), 'A' + m_index, FUNCNAME));

		// Reset RS232 emulation
	receive_register_reset();
	transmit_register_reset();

		// Soft/Channel Reset values according to SCC users manual
		m_wr0   = 0x00;
		m_wr1  &= 0x24;
		m_wr3  &= 0x01;
		m_wr4  |= 0x04;
		m_wr5  &= 0x61;
		if (m_variant & (z80scc_device::TYPE_SCC85C30 | SET_ESCC))
				m_wr7 = 0x20;
		m_wr9  &= 0xdf; //  WR9 has a different hard reset value
		m_wr10 &= 0x60; // WR10 has a different hard reset value
		m_wr11 &= 0xff; // WR11 has a different hard reset value
		m_wr14 &= 0xc3; // WR14 has a different hard reset value
		m_wr14 |= 0x20;
		m_wr15  = 0xf8;
		m_rr0  &= 0xfc;
		m_rr0  |= 0x44;
		m_rr1  &= 0x07;
		m_rr1  |= 0x06;
		m_rr3   = 0x00;
		m_rr10 &= 0x40;

#if 0 // old reset code
	// disable transmitter
	m_wr5 &= ~WR5_TX_ENABLE;
	m_rr0 |= RR0_TX_BUFFER_EMPTY;
#endif
		// TODO: check dependencies on RR1_ALL_SENT and (re)move this setting
	m_rr1 |= RR1_ALL_SENT; // It is a don't care in the SCC user manual

	// reset external lines TODO: check relation to control bits and reset
	set_rts(1);
	set_dtr(1);

	// reset interrupts
	if (m_index == z80scc_device::CHANNEL_A)
	{
		m_uart->reset_interrupts();
	}
}

void z80scc_channel::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void z80scc_channel::tra_callback()
{
	if (!(m_wr5 & WR5_TX_ENABLE))
	{
				LOG(("%" I64FMT "d %s() \"%s \"Channel %c transmit mark 1 m_wr5:%02x\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, m_wr5));
		// transmit mark
		if (m_index == z80scc_device::CHANNEL_A)
			m_uart->m_out_txda_cb(1);
		else
			m_uart->m_out_txdb_cb(1);
	}
	else if (m_wr5 & WR5_SEND_BREAK)
	{
				LOG(("%" I64FMT "d %s() \"%s \"Channel %c send break 1 m_wr5:%02x\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, m_wr5));
		// transmit break
		if (m_index == z80scc_device::CHANNEL_A)
			m_uart->m_out_txda_cb(0);
		else
			m_uart->m_out_txdb_cb(0);
	}
	else if (!is_transmit_register_empty())
	{
				int db = transmit_register_get_data_bit();

				LOG(("%" I64FMT "d %s() \"%s \"Channel %c transmit data bit %d m_wr5:%02x\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, db, m_wr5));
		// transmit data
		if (m_index == z80scc_device::CHANNEL_A)
			m_uart->m_out_txda_cb(db);
		else
			m_uart->m_out_txdb_cb(db);
	}
		else
		{
				LOG(("%" I64FMT "d %s() \"%s \"Channel %c Failed to transmit m_wr5:%02x\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, m_wr5));
				logerror("%s \"%s \"Channel %c Failed to transmit\n", FUNCNAME, m_owner->tag(), 'A' + m_index);
		}
}


//-------------------------------------------------
//  tra_complete -
//-------------------------------------------------

void z80scc_channel::tra_complete()
{
	if ((m_wr5 & WR5_TX_ENABLE) && !(m_wr5 & WR5_SEND_BREAK) && !(m_rr0 & RR0_TX_BUFFER_EMPTY))
	{
				LOG(("%" I64FMT "d %s() \"%s \"Channel %c Transmit Data Byte '%02x' m_wr5:%02x\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, m_tx_data, m_wr5));

		transmit_register_setup(m_tx_data);

		// empty transmit buffer
		m_rr0 |= RR0_TX_BUFFER_EMPTY;

		if (m_wr1 & WR1_TX_INT_ENABLE)
			m_uart->trigger_interrupt(m_index, INT_TRANSMIT);
	}
	else if (m_wr5 & WR5_SEND_BREAK)
	{
				LOG(("%" I64FMT "d %s() \"%s \"Channel %c Transmit Break 0 m_wr5:%02x\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, m_wr5));
		// transmit break
		if (m_index == z80scc_device::CHANNEL_A)
			m_uart->m_out_txda_cb(0);
		else
			m_uart->m_out_txdb_cb(0);
	}
	else
	{
				LOG(("%" I64FMT "d %s() \"%s \"Channel %c Transmit Mark 1 m_wr5:%02x\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, m_wr5));
		// transmit mark
		if (m_index == z80scc_device::CHANNEL_A)
			m_uart->m_out_txda_cb(1);
		else
			m_uart->m_out_txdb_cb(1);
	}

	// if transmit buffer is empty
	if (m_rr0 & RR0_TX_BUFFER_EMPTY)
	{
				LOG(("%" I64FMT "d %s() \"%s \"Channel %c Transmit buffer empty m_wr5:%02x\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, m_wr5));
		// then all characters have been sent
		m_rr1 |= RR1_ALL_SENT;

		// when the RTS bit is reset, the _RTS output goes high after the transmitter empties
		if (!m_rts)
			set_rts(1);
	}
}


//-------------------------------------------------
//  rcv_callback -
//-------------------------------------------------

void z80scc_channel::rcv_callback()
{
	if (m_wr3 & WR3_RX_ENABLE)
	{
				LOG(("%" I64FMT "d %s() \"%s \"Channel %c Received Data Bit %d\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, m_rxd));
		receive_register_update_bit(m_rxd);
	}
		else
		{
				LOG(("%" I64FMT "d %s() \"%s \"Channel %c Received Data Bit but receiver is disabled\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index));
				logerror("Z80SCC %s() \"%s \"Channel %c Received data dit but receiver is disabled\n", __func__, m_owner->tag(), 'A' + m_index);
		}
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void z80scc_channel::rcv_complete()
{
		UINT8 data;

	receive_register_extract();
		data = get_received_char();
		LOG(("%" I64FMT "d %s() \"%s \"Channel %c Received Data %c\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, data));
	receive_data(data);
}


//-------------------------------------------------
//  get_clock_mode - get clock divisor
//-------------------------------------------------

int z80scc_channel::get_clock_mode()
{
	int clocks = 1;

	switch (m_wr4 & WR4_CLOCK_RATE_MASK)
	{
	case WR4_CLOCK_RATE_X1:     clocks = 1;     break;
	case WR4_CLOCK_RATE_X16:    clocks = 16;    break;
	case WR4_CLOCK_RATE_X32:    clocks = 32;    break;
	case WR4_CLOCK_RATE_X64:    clocks = 64;    break;
	}

	return clocks;
}

/* From Zilog SCC/ESCC USers manual, UM010902-0609:
"/RTSA, /RTSB. Request To Send (outputs, active Low). The /RTS pins can be used as generalpurpose
outputs or with the Auto Enable feature. When used with Auto Enable ON (WR3, D5=1)
in asynchronous mode, the /RTS pin goes High after the transmitter is empty. When Auto Enable
is OFF, the /RTS pins are used as general-purpose outputs, and, they strictly follow the inverse
state of WR5, bit D1.
ESCC and 85C30: In SDLC mode, the /RTS pins can be programmed to be deasserted when the closing
flag of the message clears the TxD pin, if WR7' D2 is set."
TODO:
- SDLC mode behaviour for ESCC/85C30
*/
void z80scc_channel::set_rts(int state)
{
		LOG(("Z80SCC \"%s\" Channel %c : %s(%d)\n", m_owner->tag(), 'A' + m_index, __func__, state));
	if (m_index == z80scc_device::CHANNEL_A)
		m_uart->m_out_rtsa_cb(state);
	else
		m_uart->m_out_rtsb_cb(state);
}

void z80scc_channel::update_rts()
{
		LOG(("Z80SCC \"%s\" Channel %c : %s\n", m_owner->tag(), 'A' + m_index, __func__));
		if (m_wr5 & WR5_RTS)
		{
				// when the RTS bit is set, the _RTS output goes low
				set_rts(0);
				m_rts = 1;
		}
		else
		{
				// when the RTS bit is reset, the _RTS output goes high after the transmitter empties
				m_rts = 0;
		}

		// data terminal ready output follows the state programmed into the DTR bit*/
		set_dtr((m_wr5 & WR5_DTR) ? 0 : 1);
}

//-------------------------------------------------
//  get_stop_bits - get number of stop bits
//-------------------------------------------------

device_serial_interface::stop_bits_t z80scc_channel::get_stop_bits()
{
	switch (m_wr4 & WR4_STOP_BITS_MASK)
	{
	case WR4_STOP_BITS_1: return STOP_BITS_1;
	case WR4_STOP_BITS_1_5: return STOP_BITS_1_5;
	case WR4_STOP_BITS_2: return STOP_BITS_2;
	}

	return STOP_BITS_0;
}


//-------------------------------------------------
//  get_rx_word_length - get receive word length
//-------------------------------------------------

int z80scc_channel::get_rx_word_length()
{
	int bits = 5;

	switch (m_wr3 & WR3_RX_WORD_LENGTH_MASK)
	{
	case WR3_RX_WORD_LENGTH_5:  bits = 5;       break;
	case WR3_RX_WORD_LENGTH_6:  bits = 6;       break;
	case WR3_RX_WORD_LENGTH_7:  bits = 7;       break;
	case WR3_RX_WORD_LENGTH_8:  bits = 8;       break;
	}

	return bits;
}


//-------------------------------------------------
//  get_tx_word_length - get transmit word length
//-------------------------------------------------

int z80scc_channel::get_tx_word_length()
{
	int bits = 5;

	switch (m_wr5 & WR5_TX_WORD_LENGTH_MASK)
	{
	case WR5_TX_WORD_LENGTH_5:  bits = 5;   break;
	case WR5_TX_WORD_LENGTH_6:  bits = 6;   break;
	case WR5_TX_WORD_LENGTH_7:  bits = 7;   break;
	case WR5_TX_WORD_LENGTH_8:  bits = 8;   break;
	}

	return bits;
}

/* From Zilog SCC/ESCC USers manual, UM010902-0609:
"RR2 contains the interrupt vector written into WR2. When the register is accessed in Channel A,
the vector returned is the vector actually stored in WR2. When this register is accessed in Channel
B, the vector returned includes status information in bits 1, 2 and 3 or in bits 6, 5 and 4, depending
on the state of the Status High/Status Low bit in WR9 and independent of the state of the VIS bit
in WR9."*/
UINT8 z80scc_channel::do_sccreg_rr2()
{
		LOG(("Z80SCC %s()\n", __func__));

		// Assume the unmodified in polled mode
		m_rr2 = m_uart->m_chanA->m_wr2;

		// If we are chan B we have to modify the vector regardless of the VIS bit
	if (m_index == z80scc_device::CHANNEL_B)
		{
				// loop over all interrupt sources
				for (int i = 0; i < 6; i++)
				{
						// find the first channel with an interrupt requested
						if (m_uart->m_int_state[i] & Z80_DAISY_INT)
						{
								m_rr2 = m_uart->modify_vector(m_rr2, i < 3 ? z80scc_device::CHANNEL_A : z80scc_device::CHANNEL_B, i & 3);
								break;
						}
				}
		}
		return m_rr2;
}

/* From Zilog SCC/ESCC USers manual, UM010902-0609:
RR3 is the interrupt Pending register. The status of each of the interrupt Pending bits in the SCC is
reported in this register. This register exists only in Channel A. If this register is accessed in Channel
B, all 0s are returned. The two unused bits are always returned as 0. Figure displays the bit positions for RR3."
*/
UINT8 z80scc_channel::do_sccreg_rr3()
{
		LOG(("Z80SCC %s()\n", __func__));
		return m_rr3; // TODO Update all bits of this status register
}


/* (ESCC and 85C30 Only) */
/*On the ESCC, Read Register 4 reflects the contents of Write Register 4 provided the Extended
  Read option is enabled. Otherwise, this register returns an image of RR0. On the NMOS/CMOS version,
  a read to this location returns an image of RR0.*/
UINT8 z80scc_channel::do_sccreg_rr4()
{
		LOG(("Z80SCC %s()\n", __func__));
		if (m_variant & (SET_ESCC | z80scc_device::TYPE_SCC85C30))
				return (BIT(m_wr7, 6) ? m_wr4 : m_rr0);
		else
				return m_rr0;
}

	/* (ESCC and 85C30 Only) */
/*On the ESCC, Read Register 5 reflects the contents of Write Register 5 provided the Extended
  Read option is enabled. Otherwise, this register returns an image of RR1. On the NMOS/CMOS version,
  a read to this register returns an image of RR1.*/
UINT8 z80scc_channel::do_sccreg_rr5()
{
		LOG(("Z80SCC %s()\n", __func__));
		if (m_variant & (SET_ESCC | z80scc_device::TYPE_SCC85C30))
				return BIT(m_wr7, 6) ? m_wr5 : m_rr1;
		else
				return m_rr1;
}

/* (not on NMOS)
 On the CMOS and ESCC, Read Register 6 contains the least significant byte of the frame byte
 count that is currently at the top of the Status FIFO. RR6 is displayed in Figure on page 183. This
 register is readable only if the FIFO is enabled (refer to the description Write Register 15, bit D2,
 and SDLC Frame Status FIFO on page 126). Otherwise, this register is an image of RR2.
 On the NMOS version, a read to this register location returns an image of RR2.*/
UINT8 z80scc_channel::do_sccreg_rr6()
{
		LOG(("Z80SCC %s()\n", __func__));
		if (!(m_variant & (SET_NMOS)))
		{
				logerror("Z80SCC %s() not implemented feature\n", __func__);
				return 0;
		}
		return m_rr2;
}

/* (not on NMOS)
 On the CMOS and ESCC, Read Register 7 contains the most significant six bits of the frame byte
 count that is currently at the top of the Status FIFO. Bit D7 is the FIFO Overflow Status and bit D6
 is the FIFO Data Available Status. The status indications are given in Table on page 184. RR7 is
 displayed in Figure on page 183. This register is readable only if the FIFO is enabled (refer to the
 description Write Register 15, bit D2). Otherwise this register is an image of RR3. Note, for proper
 operation of the FIFO and byte count logic, the registers should be read in the following order:
 RR7, RR6, RR1.*/
UINT8 z80scc_channel::do_sccreg_rr7()
{
		LOG(("Z80SCC %s()\n", __func__));
		if (!(m_variant & (SET_NMOS)))
		{
				logerror("Z80SCC %s() not implemented feature\n", __func__);
				return 0;
		}
		return m_rr3;
}

#if 0 // Short cutted in control_read()
/* RR8 is the Receive Data register. */
UINT8 z80scc_channel::do_sccreg_rr8()
{
		return data_read():
}
#endif

/* (ESCC and 85C30 Only)
 On the ESCC, Read Register 9 reflects the contents of Write Register 3 provided the Extended
 Read option has been enabled. On the NMOS/CMOS version, a read to this location returns an image
 of RR13. TODO: Check what is returned if Extended Read option is turned off */
UINT8 z80scc_channel::do_sccreg_rr9()
{
		LOG(("Z80SCC %s()\n", __func__));
		if (m_variant & (SET_ESCC | z80scc_device::TYPE_SCC85C30))
				return BIT(m_wr7, 6) ? m_wr3 : m_rr13;
		else
				return m_rr13;
}

/* RR10 contains some SDLC related miscellaneous status bits. Unused bits are always 0. */
UINT8 z80scc_channel::do_sccreg_rr10()
{
		LOG(("Z80SCC %s()\n", __func__));
		logerror("Z80SCC %s() not implemented feature\n", __func__);
		return m_rr10;
}

/* (ESCC and 85C30 Only)
 On the ESCC, Read Register 11 reflects the contents of Write Register 10 provided the Extended
 Read option has been enabled. Otherwise, this register returns an image of RR15.
 On the NMOS/CMOS version, a read to this location returns an image of RR15.*/
UINT8 z80scc_channel::do_sccreg_rr11()
{
		LOG(("Z80SCC %s()\n", __func__));
		if (m_variant & (SET_ESCC | z80scc_device::TYPE_SCC85C30))
				return BIT(m_wr7, 6) ? m_wr10 : m_rr15;
		else
				return m_rr15;
}

/*
 RR12 returns the value stored in WR12, the lower byte of the time constant, for the BRG.*/
UINT8 z80scc_channel::do_sccreg_rr12()
{
		return m_wr12;
}

/*
  RR13 returns the value stored in WR13, the upper byte of the time constant for the BRG. */
UINT8 z80scc_channel::do_sccreg_rr13()
{
		return m_wr13;
}

/* (ESCC and 85C30 Only)
On the ESCC, Read Register 14 reflects the contents of Write Register 7 Prime provided the
Extended Read option has been enabled. Otherwise, this register returns an image of RR10.
On the NMOS/CMOS version, a read to this location returns an image of RR10.*/
UINT8 z80scc_channel::do_sccreg_rr14()
{
		LOG(("Z80SCC %s()\n", __func__));
		if (m_variant & (SET_ESCC | z80scc_device::TYPE_SCC85C30))
				return BIT(m_wr7, 6) ? m_wr7 : m_rr10;
		else
				return m_rr10;
}

/*
 RR15 reflects the value stored in WR15, the External/Status IE bits. The two unused bits are
 always returned as Os. */
UINT8 z80scc_channel::do_sccreg_rr15()
{
		LOG(("Z80SCC %s()\n", __func__));
		logerror("Z80SCC %s() not implemented feature\n", __func__);
		return m_wr15 & 0xf5; // Mask out the used bits
}

//-------------------------------------------------
//  control_read - read control register
//-------------------------------------------------
UINT8 z80scc_channel::control_read()
{
	UINT8 data = 0;
		int reg = m_wr0;
		int regmask = (WR0_REGISTER_MASK | m_ph);

		//  LOG(("%s(%02x) reg %02x, regmask %02x, WR0 %02x\n", __func__, data, reg, regmask, m_wr0));

	m_ph = 0; // The "Point High" command is only valid for one access

		reg &= regmask;

	if (reg != 0)
	{
		// mask out register index
		m_wr0 &= ~regmask;
	}

		/* TODO. Sort out 80X30 limitations in register access */
	switch (reg)
	{
	case REG_RR0_STATUS:         data = do_sioreg_rr0(); break; // TODO: verify handling of SCC specific bits: D6 and D1
		case REG_RR1_SPEC_RCV_COND:  data = do_sioreg_rr1(); break;
	case REG_RR2_INTERRUPT_VECT: data = do_sccreg_rr2(); break; // Channel dependent and SCC specific handling compared to SIO
		/* registers 3-7 are specific to SCC. TODO: Check variant and log/stop misuse */
	case REG_RR3_INTERUPPT_PEND: data = do_sccreg_rr3(); break;
		case REG_RR4_WR4_OR_RR0:     data = do_sccreg_rr4(); break;
		case REG_RR5_WR5_OR_RR0:     data = do_sccreg_rr5(); break;
		case REG_RR6_LSB_OR_RR2:     data = do_sccreg_rr6(); break;
		case REG_RR7_MSB_OR_RR3:     data = do_sccreg_rr7(); break;
		/* registers 8-15 are specific to SCC */
		case REG_RR8_RECEIVE_DATA:   data = data_read(); break;
		case REG_RR9_WR3_OR_RR13:    data = do_sccreg_rr9(); break;
		case REG_RR10_MISC_STATUS:   data = do_sccreg_rr10(); break;
		case REG_RR11_WR10_OR_RR15:  data = do_sccreg_rr11(); break;
		case REG_RR12_LO_TIME_CONST: data = do_sccreg_rr12(); break;
		case REG_RR13_HI_TIME_CONST: data = do_sccreg_rr13(); break;
		case REG_RR14_WR7_OR_R10:    data = do_sccreg_rr14(); break;
		case REG_RR15_WR15_EXT_STAT: data = do_sccreg_rr15(); break;
	default:
				logerror("Z80SCC \"%s\" %s Channel %c : Unsupported RRx register:%02x\n", m_owner->tag(), __func__, 'A' + m_index, reg);
	}

	//LOG(("Z80SCC \"%s\" Channel %c : Register R%d read '%02x'\n", m_owner->tag(), 'A' + m_index, reg, data));

	return data;
}

/**/
void z80scc_channel::do_sccreg_wr0(UINT8 data)
{
		m_wr0 = data;

		/* Sort out SCC specific behaviours  from legacy SIO behaviour */
		/* WR0_Z_* are Z80X30 specific commands */
		switch (data & WR0_COMMAND_MASK)
	{
		case WR0_POINT_HIGH:
				/*This command effectively adds eight to the Register Pointer (D2-D0) by allowing
				  WR8 through WR15 to be accessed. The Point High command and the Register
				  Pointer bits are written simultaneously. This command is used in the Z85X30
				  version of the SCC. Note that WR0 changes form depending upon the SCC version.
				  Register access for the Z80X30 version of the SCC is accomplished through direct
				  addressing*/
				if (m_variant & SET_Z85X3X)
				{
						LOG(("Z80SCC \"%s\" %s Channel %c : %s - Point High command\n", m_owner->tag(), __func__, 'A' + m_index, __func__));
						m_ph = 8;
				}
				else
						LOG(("Z80SCC \"%s\" %s Channel %c : %s - NULL command 2\n", m_owner->tag(), __func__, 'A' + m_index, __func__));
				break;
		case WR0_RESET_EXT_STATUS: // TODO: Take care of the Zero Count flag and the 2 slot fifo
				/*After an External/Status interrupt (a change on a modem line or a break condition,
				  for example), the status bits in RR0 are latched. This command re-enables the bits
				  and allows interrupts to occur again as a result of a status change. Latching the
				  status bits captures short pulses until the CPU has time to read the change.
				  The SCC contains simple queueing logic associated with most of the external status
				  bits in RR0. If another External/Status condition changes while a previous condition
				  is still pending (Reset External/Status Interrupt has not yet been issued) and this
				  condition persists until after the command is issued, this second change causes another
				  External/Status interrupt. However, if this second status change does not persist
				  (there are two transitions), another interrupt is not generated. Exceptions to this
				  rule are detailed in the RR0 description.*/
				do_sioreg_wr0(data);
				if (!m_zc)
				{
						m_rr0 |= RR0_ZC;
				}
				LOG(("Z80SCC \"%s\" %s Channel %c : %s - Reset External/Status Interrupt\n", m_owner->tag(), __func__, 'A' + m_index, __func__));
				break;
		case WR0_RESET_HIGHEST_IUS:
				/* This command resets the highest priority Interrupt Under Service (IUS) bit, allowing lower
				   priority conditions to request interrupts. This command allows the use of the internal
				   daisy chain (even in systems without an external daisy chain) and is the last operation in
				   an interrupt service routine.TODO: Implement internal Daisychain */
				LOG(("Z80SCC \"%s\" %s Channel %c : Reset Highest IUS\n", m_owner->tag(), __func__, 'A' + m_index));
				break;
		case WR0_ERROR_RESET:
				/*Error Reset Command (110). This command resets the error bits in RR1. If interrupt on first Rx
				  Character or Interrupt on Special Condition modes is selected and a special condition exists, the
				  data with the special condition is held in the Receive FIFO until this command is issued. If either
				  of these modes is selected and this command is issued before the data has been read from the
				  Receive FIFO, the data is lost */
				LOG(("Z80SCC \"%s\" %s Channel %c : WR0_ERROR_RESET\n", m_owner->tag(), __func__, 'A' + m_index));
				do_sioreg_wr0(data); // reset status registers
				m_rx_fifo_rp_step(); // Reset error state in fifo and unlock it. unlock == step to next slot in fifo.
				break;
		case WR0_SEND_ABORT:
				data &= 0xef; // convert SCC SEND_ABORT command to a SIO SEND_ABORT command and fall through
		/* The following commands relies on the SIO default behviour */
		case WR0_NULL:
		case WR0_ENABLE_INT_NEXT_RX:
		case WR0_RESET_TX_INT:
		default:
				do_sioreg_wr0(data);
		}
		do_sioreg_wr0_resets(data);
		if ( m_variant & SET_Z80X30) // TODO: Implement adress decoding for Z80X30 using the shift logic described below
		{
				/*The registers in the Z80X30 are addressed via the address on AD7-AD0 and are latched by the rising
				  edge of /AS. The Shift Right/Shift Left bit in the Channel B WR0 controls which bits are
				  decoded to form the register address. It is placed in this register to simplify programming when the
				  current state of the Shift Right/Shift Left bit is not known.
				  A hardware reset forces Shift Left mode where the address is decoded from AD5-AD1. In Shift
				  Right mode, the address is decoded from AD4-AD0. The Shift Right/Shift Left bit is written via a
				  command to make the software writing to WR0 independent of the state of the Shift Right/Shift
				  Left bit.
				  While in the Shift Left mode, the register address is placed on AD4-AD1 and the Channel Select
				  bit, A/B, is decoded from AD5. The register map for this case is listed in Table on page 21. In
				  Shift Right mode, the register address is again placed on AD4-AD1 but the channel select A/B is
				  decoded from AD0. The register map for this case is listed in Table on page 23.
				  Because the Z80X30 does not contain 16 read registers, the decoding of the read registers is not
				  complete; this is listed in Table on page 21 and Table on page 23 by parentheses around the register
				  name. These addresses may also be used to access the read registers. Also, note that the
				  Z80X30 contains only one WR2 and WR9; these registers may be written from either channel.
				  Shift Left Mode is used when Channel A and B are to be programmed differently. This allows the
				  software to sequence through the registers of one channel at a time. The Shift Right Mode is used
				  when the channels are programmed the same. By incrementing the address, the user can program
				  the same data value into both the Channel A and Channel B register.*/
				switch(data & WR0_Z_SHIFT_MASK)
				{
				case WR0_Z_SEL_SHFT_LEFT:
						LOG(("Z80SCC \"%s\" Channel %c : %s - Shift Left Addressing Mode - not implemented\n", m_owner->tag(), 'A' + m_index, __func__));
						break;
				case WR0_Z_SEL_SHFT_RIGHT:
						LOG(("Z80SCC \"%s\" Channel %c : %s - Shift Right Addressing Mode - not implemented\n", m_owner->tag(), 'A' + m_index, __func__));
						break;
				default:
						break;
						// LOG(("Z80SCC \"%s\" Channel %c : %s - Null commands\n", m_owner->tag(), 'A' + m_index, __func__));
				}
		}
}

/* Write Register 1 is the control register for the various SCC interrupt and Wait/Request modes.*/
void z80scc_channel::do_sccreg_wr1(UINT8 data)
{
		LOG(("Z80SCC \"%s\" Channel %c : %s - %02x\n", m_owner->tag(), 'A' + m_index, __func__, data));
		/* TODO: Sort out SCC specific behaviours  from legacy SIO behaviours:
		 - Channel B only bits vs
		 - Parity Is Special Condition, bit2 */
		do_sioreg_wr1(data & ~0x40); // Lets SIO code handle it for now but mask out dangerous bits
		m_uart->check_interrupts();
}

/*WR2 is the interrupt vector register. Only one vector register exists in the SCC, and it can be
accessed through either channel. The interrupt vector can be modified by status information. This
is controlled by the Vector Includes Status (VIS) and the Status High/Status Low bits in WR9.*/
void z80scc_channel::do_sccreg_wr2(UINT8 data)
{
		LOG(("Z80SCC \"%s\" Channel %c : %s - Setting the interrupt vector to: %02x \n", m_owner->tag(), 'A' + m_index, __func__, data));
		m_wr2 = data;
		m_uart->m_chanA->m_rr2 = data;
		m_uart->m_chanB->m_rr2 = data; /* TODO: Sort out the setting of ChanB depending on bits in WR9 */

		m_uart->check_interrupts();
}

/*
 * NOTE: Register WR3-WR7 are identical to the Z80SIO so handled by z80sio.c
 */

/* WR8 is the transmit buffer register */
void z80scc_channel::do_sccreg_wr8(UINT8 data)
{
		LOG(("Z80SCC \"%s\" Channel %c : Transmit Buffer read %02x\n", m_owner->tag(), 'A' + m_index, data));
		data_write(data);
}

/*WR9 is the Master Interrupt Control register and contains the Reset command bits. Only one WR9
exists in the SCC and is accessed from either channel. The Interrupt control bits are programmed
at the same time as the Reset command, because these bits are only reset by a hardware reset
note that the Z80X30 contains only one WR2 and WR9; these registers may be written from either channel.*/
void z80scc_channel::do_sccreg_wr9(UINT8 data)
{
		if (m_variant & SET_Z80X30)
		{
				m_uart->m_chanA->m_wr9 = data;
				m_uart->m_chanB->m_wr9 = data;
		}
		else
				m_wr9 = data;

		switch (data & WR9_CMD_MASK)
		{
		case WR9_CMD_NORESET:
				LOG(("Z80SCC \"%s\" Channel %c : Master Interrupt Control - No reset  %02x\n", m_owner->tag(), 'A' + m_index, data));
				break;
		case WR9_CMD_CHNB_RESET:
				LOG(("Z80SCC \"%s\" Channel %c : Master Interrupt Control - Channel B reset  %02x\n", m_owner->tag(), 'A' + m_index, data));
				m_uart->m_chanB->reset();
				break;
		case WR9_CMD_CHNA_RESET:
				LOG(("Z80SCC \"%s\" Channel %c : Master Interrupt Control - Channel A reset  %02x\n", m_owner->tag(), 'A' + m_index, data));
				m_uart->m_chanA->reset();
				break;
		case WR9_CMD_HW_RESET:
				LOG(("Z80SCC \"%s\" Channel %c : Master Interrupt Control - Device reset  %02x\n", m_owner->tag(), 'A' + m_index, data));
				/*"The effects of this command are identical to those of a hardware reset, except that the Shift Right/Shift Left bit is
				  not changed and the MIE, Status High/Status Low and DLC bits take the programmed values that accompany this command."
				  The Shift Right/Shift Left bits of the WR0 is only valid on SCC8030 device hence not implemented yet, just the SCC8530 */
				if (data & (WR9_BIT_MIE | WR9_BIT_IACK | WR9_BIT_SHSL | WR9_BIT_DLC | WR9_BIT_NV))
						logerror("Z80SCC: SCC Interrupt system not yet implemented, please be patient!\n");
				m_uart->device_reset();
		default:
				logerror("Z80SCC Code is broken in WR9, please report!\n");
		}
}

/* WR10 contains miscellaneous control bits for both the receiver and the transmitter. Bit positions
for WR10 are displayed in Figure . On the ESCC and 85C30 with the Extended Read option
enabled, this register may be read as RR11.*/
void z80scc_channel::do_sccreg_wr10(UINT8 data)
{
		m_wr10 = data;
		LOG(("Z80SCC \"%s\" Channel %c : %s Misc Tx/Rx Control %02x - not implemented \n", m_owner->tag(), 'A' + m_index, __func__, data));
}

/* WR11 is the Clock Mode Control register. The bits in this register control the sources of both the
receive and transmit clocks, the type of signal on the /SYNC and /RTxC pins, and the direction of
the /TRxC pin.*/
void z80scc_channel::do_sccreg_wr11(UINT8 data)
{
		LOG(("Z80SCC \"%s\" Channel %c : %s Clock Mode Control %02x - not implemented \n", m_owner->tag(), 'A' + m_index, __func__, data));
		m_wr11 = data;
		/*Bit 7: This bit controls the type of input signal the SCC expects to see on the /RTxC pin. If this bit is set
		  to 0, the SCC expects a TTL-compatible signal as an input to this pin. If this bit is set to 1, the SCC
		  connects a high-gain amplifier between the /RTxC and /SYNC pins in expectation of a quartz
		  crystal being placed across the pins.
		  The output of this oscillator is available for use as a clocking source. In this mode of operation, the
		  /SYNC pin is unavailable for other use. The /SYNC signal is forced to zero internally. A hardware
		  reset forces /NO XTAL. (At least 20 ms should be allowed after this bit is set to allow the oscillator
		  to stabilize.)*/
		LOG(("  Clock type %s\n", data & WR11_RCVCLK_TYPE ? "Crystal oscillator between RTxC and /SYNC pins" : "TTL level on RTxC pin"));
		/*Bits 6 and 5: Receiver Clock select bits 1 and 0
		  These bits determine the source of the receive clock as listed below. They do not
		  interfere with any of the modes of operation in the SCC, but simply control a multiplexer just
		  before the internal receive clock input. A hardware reset forces the receive clock to come from the
		  /RTxC pin.*/
		LOG(("  Receive clock source is: "));
		switch (data & WR11_RCVCLK_SRC_MASK)
		{
		case WR11_RCVCLK_SRC_RTXC: LOG(("RTxC\n")); break;
		case WR11_RCVCLK_SRC_TRXC: LOG(("TRxC\n")); break;
		case WR11_RCVCLK_SRC_BR:   LOG(("Baudrate Generator\n")); break;
		case WR11_RCVCLK_SRC_DPLL: LOG(("DPLL\n")); break;
		default: logerror("Wrong!\n");/* Will not happen unless someone messes with the mask */
		}
		/*Bits 4 and 3: Transmit Clock select bits 1 and 0.
		  These bits determine the source of the transmit clock as listed in Table . They do not interfere with
		  any of the modes of operation of the SCC, but simply control a multiplexer just before the internal
		  transmit clock input. The DPLL output that is used to feed the transmitter in FM modes lags by 90
		  degrees the output of the DPLL used by the receiver. This makes the received and transmitted bit
		  cells occur simultaneously, neglecting delays. A hardware reset selects the /TRxC pin as the
		  source of the transmit clocks.*/
		LOG(("  Transmit clock source is: "));
		switch (data & WR11_TRACLK_SRC_MASK)
		{
		case WR11_TRACLK_SRC_RTXC: LOG(("RTxC\n")); break;
		case WR11_TRACLK_SRC_TRXC: LOG(("TRxC\n")); break;
		case WR11_TRACLK_SRC_BR:   LOG(("Baudrate Generator\n")); break;
		case WR11_TRACLK_SRC_DPLL: LOG(("DPLL\n")); break;
		default: logerror("Wrong!\n");/* Will not happen unless someone messes with the mask */
		}
		/* Bit 2: TRxC Pin I/O control bit
		   This bit determines the direction of the /TRxC pin. If this bit is set to 1, the /TRxC pin is an output
		   and carries the signal selected by D1 and D0 of this register. However, if either the receive or the
		   transmit clock is programmed to come from the /TRxC pin, /TRxC is an input, regardless of the
		   state of this bit. The /TRxC pin is also an input if this bit is set to 0. A hardware reset forces this bit
		   to 0.*/
		LOG(("  TRxC pin is %s\n", data & WR11_TRXC_DIRECTION ? "Output" : "Input"));
		/*Bits 1 and 0: /TRxC Output Source select bits 1 and 0
		  These bits determine the signal to be echoed out of the SCC via the /TRxC pin as listed in Table
		  on page 167. No signal is produced if /TRxC has been programmed as the source of either the
		  receive or the transmit clock. If /TRxC O/I (bit 2) is set to 0, these bits are ignored.
		  If the XTAL oscillator output is programmed to be echoed, and the XTAL oscillator is not enabled,
		  the /TRxC pin goes High. The DPLL signal that is echoed is the DPLL signal used by the receiver.
		  Hardware reset selects the XTAL oscillator as the output source*/
		LOG(("  TRxC clock source is: "));
		switch (data & WR11_TRXSRC_SRC_MASK)
		{
		case WR11_TRXSRC_SRC_XTAL: LOG(("the Oscillator\n")); break;
		case WR11_TRXSRC_SRC_TRA:  LOG(("Transmit clock\n")); break;
		case WR11_TRXSRC_SRC_BR:   LOG(("Baudrate Generator\n")); break;
		case WR11_TRXSRC_SRC_DPLL: LOG(("DPLL\n")); break;
		default: logerror("Wrong!\n");/* Will not happen unless someone messes with the mask */
		}
}

/*WR12 contains the lower byte of the time constant for the baud rate generator. The time constant
can be changed at any time, but the new value does not take effect until the next time the time constant
is loaded into the down counter. No attempt is made to synchronize the loading of the time
constant into WR12 and WR13 with the clock driving the down counter. For this reason, it is
advisable to disable the baud rate generator while the new time constant is loaded into WR12 and
WR13. Ordinarily, this is done anyway to prevent a load of the down counter between the writing
of the upper and lower bytes of the time constant.
The formula for determining the appropriate time constant for a given baud is shown below, with
the desired rate in bits per second and the BR clock period in seconds. This formula is derived
because the counter decrements from N down to zero-plus-one-cycle for reloading the time constant.
This is then fed to a toggle flip-flop to make the output a square wave.

   Time Constant = Clock Frequency / (2 * Desired Rate * Baud Rate Clock Period) - 2

*/
void z80scc_channel::do_sccreg_wr12(UINT8 data)
{
		m_wr12 = data;
		LOG(("Z80SCC \"%s\" Channel %c : %s  %02x Low byte of Time Constant for Baudrate generator - not implemented \n", m_owner->tag(), 'A' + m_index, __func__, data));
}

/* WR13 contains the upper byte of the time constant for the baud rate generator. */
void z80scc_channel::do_sccreg_wr13(UINT8 data)
{
		m_wr13 = data;
		LOG(("Z80SCC \"%s\" Channel %c : %s  %02x  High byte of Time Constant for Baudrate generator - not implemented \n", m_owner->tag(), 'A' + m_index, __func__, data));
}

/*
 WR14 contains some miscellaneous control bits */
void z80scc_channel::do_sccreg_wr14(UINT8 data)
{
		switch (data & WR14_DPLL_CMD_MASK)
		{
		case WR14_CMD_NULL:
				LOG(("Z80SCC \"%s\" Channel %c : %s  Misc Control Bits Null Command %02x\n", m_owner->tag(), 'A' + m_index, __func__, data));
				break;
		case WR14_CMD_ESM:
/* Issuing this command causes the DPLL to enter the Search mode, where the DPLL searches for a locking edge in the
   incoming data stream. The action taken by the DPLL upon receipt of this command depends on the operating mode of
   the DPLL. In NRZI mode, the output of the DPLL is High while the DPLL is waiting for an edge in the incoming data
   stream. After the Search mode is entered, the first edge the DPLL sees is assumed to be a valid data edge, and
   the DPLL begins the clock recovery operation from that point. The DPLL clock rate must be 32x the data rate in
   NRZI mode. Upon leaving the Search mode, the first sampling edge of the DPLL occurs 16 of these 32x clocks after
   the first data edge, and the second sampling occurs 48 of these 32x clocks after the first data edge. Beyond
   this point, the DPLL begins normal operation, adjusting the output to remain in sync with the incoming data.
   In FM mode, the output of the DPLL is Low while the DPLL is waiting for an edge in the incoming data stream.
   The first edge the DPLL detects is assumed to be a valid clock edge. For this to be the case, the line must
   contain only clock edges; i.e. with FM1 encoding, the line must be continuous 0s. With FM0 encoding the line must
   be continuous 1s, whereas Manchester encoding requires alternating 1s and 0s on the line. The DPLL clock rate must
   be 16 times the data rate in FM mode. The DPLL output causes the receiver to sample the data stream in the nominal
   center of the two halves of the bit to decide whether the data was a 1 or a 0. After this command is issued, as in
   NRZI mode, the DPLL starts sampling immediately after the first edge is detected. (In FM mode, the DPLL examines
   the clock edge of every other bit to decide what correction must be made to remain in sync.) If the DPLL does not
   see an edge during the expected window, the one clock missing bit in RR10 is set. If the DPLL does not see an edge
   after two successive attempts, the two clocks missing bits in RR10 are set and the DPLL automatically enters the
   Search mode. This command resets both clocks missing latches.*/
				LOG(("Z80SCC \"%s\" Channel %c : %s  Misc Control Bits Enter Search Mode Command - not implemented\n", m_owner->tag(), 'A' + m_index, __func__));
				break;
		case WR14_CMD_RMC:
/* Issuing this command disables the DPLL, resets the clock missing latches in RR10, and forces a continuous Search mode state */
				LOG(("Z80SCC \"%s\" Channel %c : %s  Misc Control Bits Reset Missing Clocks Command - not implemented\n", m_owner->tag(), 'A' + m_index, __func__));
				break;
		case WR14_CMD_DISABLE_DPLL:
/* Issuing this command disables the DPLL, resets the clock missing latches in RR10, and forces a continuous Search mode state.*/
				LOG(("Z80SCC \"%s\" Channel %c : %s  Misc Control Bits Disable DPLL Command - not implemented\n", m_owner->tag(), 'A' + m_index, __func__));
				break;
		case WR14_CMD_SS_BGR:
/* Issuing this command forces the clock for the DPLL to come from the output of the BRG. */
				LOG(("Z80SCC \"%s\" Channel %c : %s  Misc Control Bits Baudrate Generator Input DPLL Command - not implemented\n", m_owner->tag(), 'A' + m_index, __func__));
				break;
		case WR14_CMD_SS_RTXC:
/* Issuing the command forces the clock for the DPLL to come from the /RTxC pin or the crystal oscillator, depending on
   the state of the XTAL/no XTAL bit in WR11. This mode is selected by a channel or hardware reset*/
				LOG(("Z80SCC \"%s\" Channel %c : %s  Misc Control Bits RTxC Input DPLL Command - not implemented\n", m_owner->tag(), 'A' + m_index, __func__));
				break;
		case WR14_CMD_SET_FM:
/* This command forces the DPLL to operate in the FM mode and is used to recover the clock from FM or Manchester-Encoded
   data. (Manchester is decoded by placing the receiver in NRZ mode while the DPLL is in FM mode.)*/
				LOG(("Z80SCC \"%s\" Channel %c : %s  Misc Control Bits Set FM Mode Command - not implemented\n", m_owner->tag(), 'A' + m_index, __func__));
				break;
		case WR14_CMD_SET_NRZI:
/* Issuing this command forces the DPLL to operate in the NRZI mode. This mode is also selected by a hardware or channel reset.*/
				LOG(("Z80SCC \"%s\" Channel %c : %s  Mics Control Bits Set NRZI Mode Command - not implemented\n", m_owner->tag(), 'A' + m_index, __func__));
				break;
		default:
				logerror("Z80SCC \"%s\" Channel %c : %s Mics Control Bits command %02x - not implemented \n", m_owner->tag(), 'A' + m_index, __func__, data);
		}
		// TODO: Add info on the other bits of this register
		m_wr14 = data;
}

/* WR15 is the External/Status Source Control register. If the External/Status interrupts are enabled
as a group via WR1, bits in this register control which External/Status conditions cause an interrupt.
Only the External/Status conditions that occur after the controlling bit is set to 1 cause an
interrupt. This is true, even if an External/Status condition is pending at the time the bit is set*/
void z80scc_channel::do_sccreg_wr15(UINT8 data)
{
		LOG(("Z80SCC \"%s\" Channel %c : %s  External/Status Source Control Bits %02x - not implemented \n", m_owner->tag(), 'A' + m_index, __func__, data));
		m_wr15 = data;
}


//-------------------------------------------------
//  control_write - write control register
//-------------------------------------------------

void z80scc_channel::control_write(UINT8 data)
{
		UINT8 reg = m_wr0;
		UINT8 regmask = (WR0_REGISTER_MASK | m_ph);

	m_ph = 0; // The "Point High" command is only valid for one access

		reg &= regmask;

	if (reg != 0)
	{
		// mask out register index
		m_wr0 &= ~regmask;
	}

	LOG(("%s(%02x) reg %02x, regmask %02x, WR0 %02x\n", __func__, data, reg, regmask, m_wr0));

		/* TODO. Sort out 80X30 & other SCC variants limitations in register access */
	switch (reg)
	{
	case REG_WR0_COMMAND_REGPT:   do_sccreg_wr0(data); break;
	case REG_WR1_INT_DMA_ENABLE:  do_sccreg_wr1(data); m_uart->check_interrupts(); break;
	case REG_WR2_INT_VECTOR:      do_sccreg_wr2(data); break;
	case REG_WR3_RX_CONTROL:
				do_sioreg_wr3(data);
				update_serial();
				receive_register_reset();
				break;
	case REG_WR4_RX_TX_MODES:
				do_sioreg_wr4(data);
				update_serial();
				transmit_register_reset();
				receive_register_reset();
				break;
	case REG_WR5_TX_CONTROL:
				do_sioreg_wr5(data);
				update_serial();
				transmit_register_reset();
				update_rts();
				break;
	case REG_WR6_SYNC_OR_SDLC_A:  do_sioreg_wr6(data); break;
	case REG_WR7_SYNC_OR_SDLC_F:  do_sioreg_wr7(data); break;
	case REG_WR8_TRANSMIT_DATA:   do_sccreg_wr8(data); break;
	case REG_WR9_MASTER_INT_CTRL: do_sccreg_wr9(data); break;
		case REG_WR10_MSC_RX_TX_CTRL: do_sccreg_wr10(data); break;
		case REG_WR11_CLOCK_MODES:    do_sccreg_wr11(data); break;
		case REG_WR12_LO_BAUD_GEN:    do_sccreg_wr12(data); break;
		case REG_WR13_HI_BAUD_GEN:    do_sccreg_wr13(data); break;
		case REG_WR14_MISC_CTRL:      do_sccreg_wr14(data); break;
		case REG_WR15_EXT_ST_INT_CTRL:
		LOG(("Z80SCC \"%s\" Channel %c : unsupported command: External/Status Control Bits %02x\n", m_owner->tag(), 'A' + m_index, data));
				break;
	default:
				logerror("Z80SCC \"%s\" Channel %c : Unsupported WRx register:%02x\n", m_owner->tag(), 'A' + m_index, reg);
	}
}


//-------------------------------------------------
//  data_read - read data register from fifo
//-------------------------------------------------

UINT8 z80scc_channel::data_read()
{
	UINT8 data = 0;

		if (m_rx_fifo_wp != m_rx_fifo_rp)
	{
				/* Special Receive Condition interrupts are generated after the character is read from
				 the FIFO, not when the special condition is first detected. This is done so that when
				 using receive interrupt on first or Special Condition or Special Condition Only, data is
				 directly read out of the data FIFO without checking the status first. If a special condi-
				 tion interrupted the CPU when first detected, it would be necessary to read RR1
				 before each byte in the FIFO to determine which byte had the special condition.
				 Therefore, by not generating the interrupt until after the byte has been read and then
				 locking the FIFO, only one status read is necessary. A DMA can be used to do all data
				 transfers (otherwise, it would be necessary to disable the DMA to allow the CPU to
				 read the status on each byte). Consequently, since the special condition locks the
				 FIFO to preserve the status, it is necessary to issue the Error Reset command to
				 unlock it. Only the exit location of the FIFO is locked allowing more data to be
				 received into the other bytes of the Receive FIFO.*/

		// load data from the FIFO
		data = m_rx_fifo_rp_data();

				// load error status from the FIFO
		m_rr1 = (m_rr1 & ~(RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR)) | m_rx_error_fifo[m_rx_fifo_rp];

				// trigger interrup and lock the fifo if an error is present
				if (m_rr1 & (RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR))
				{
						switch (m_wr1 & WR1_RX_INT_MODE_MASK)
						{
						case WR1_RX_INT_FIRST:
								if (!m_rx_first)
								{
										m_uart->trigger_interrupt(m_index, INT_SPECIAL);
								}
								break;

						case WR1_RX_INT_ALL_PARITY:
						case WR1_RX_INT_ALL:
								m_uart->trigger_interrupt(m_index, INT_SPECIAL);
								break;
						}
				}
				else
				{
						// decrease FIFO pointer
						m_rx_fifo_rp_step();
				}
	}
		else
		{
				logerror("data_read: Attempt to read out character from empty FIFO\n");
		}

	LOG(("Z80SCC \"%s\" Channel %c : Data Register Read '%02x'\n", m_owner->tag(), 'A' + m_index, data));

	return data;
}

/* Get data from top of fifo data but restore read pointer in case of exit latch lock */
UINT8 z80scc_channel::m_rx_fifo_rp_data()
{
		UINT8 data;
		UINT8 old_rp = m_rx_fifo_rp;
		m_rx_fifo_rp_step();
		data = m_rx_data_fifo[m_rx_fifo_rp];
		m_rx_fifo_rp = old_rp;

		return data;
}

/* Step read pointer */
void z80scc_channel::m_rx_fifo_rp_step()
{
		m_rx_fifo_rp++;
		if (m_rx_fifo_rp >= m_rx_fifo_sz)
		{
				m_rx_fifo_rp = 0;
		}

		// check if FIFO is empty
		if (m_rx_fifo_rp == m_rx_fifo_wp)
		{
				// no more characters available in the FIFO
				m_rr0 &= ~ RR0_RX_CHAR_AVAILABLE;
		}
}

//-------------------------------------------------
//  data_write - write data register
//-------------------------------------------------

void z80scc_channel::data_write(UINT8 data)
{
	m_tx_data = data;

		if ((m_wr5 & WR5_TX_ENABLE) && is_transmit_register_empty())
	{
		LOG(("Z80SCC \"%s\" Channel %c : Transmit Data Byte '%02x'\n", m_owner->tag(), 'A' + m_index, m_tx_data));

		transmit_register_setup(m_tx_data);

		// empty transmit buffer
		m_rr0 |= RR0_TX_BUFFER_EMPTY;

		if (m_wr1 & WR1_TX_INT_ENABLE)
				{
			m_uart->trigger_interrupt(m_index, INT_TRANSMIT);
				}
	}
	else
	{
		m_rr0 &= ~RR0_TX_BUFFER_EMPTY;
	}

	m_rr1 &= ~RR1_ALL_SENT;

	LOG(("Z80SCC \"%s\" Channel %c : Data Register Write '%02x'\n", m_owner->tag(), 'A' + m_index, data));
}


//-------------------------------------------------
//  receive_data - receive data word into fifo
//-------------------------------------------------

void z80scc_channel::receive_data(UINT8 data)
{
	LOG(("Z80SCC \"%s\" Channel %c : Receive Data Byte '%02x'\n", m_owner->tag(), 'A' + m_index, data));

		if (m_rx_fifo_wp + 1 == m_rx_fifo_rp ||
			( (m_rx_fifo_wp + 1 == m_rx_fifo_sz) && (m_rx_fifo_rp == 0) ))
	{
		// receive overrun error detected
				m_rx_error_fifo[m_rx_fifo_wp] |= RR1_RX_OVERRUN_ERROR; // = m_rx_error;
		}
		else
		{
				m_rx_error_fifo[m_rx_fifo_wp] &= ~RR1_RX_OVERRUN_ERROR; // = m_rx_error;
				m_rx_fifo_wp++;
				if (m_rx_fifo_wp >= m_rx_fifo_sz)
				{
						m_rx_fifo_wp = 0;
				}
		}

	// store received character
	m_rx_data_fifo[m_rx_fifo_wp] = data;

	m_rr0 |= RR0_RX_CHAR_AVAILABLE;

#if 0 // interrupt on exit from fifo
	// receive interrupt
	switch (m_wr1 & WR1_RX_INT_MODE_MASK)
	{
	case WR1_RX_INT_FIRST:
		if (m_rx_first)
		{
			m_uart->trigger_interrupt(m_index, INT_RECEIVE);

			m_rx_first = 0;
		}
		break;

	case WR1_RX_INT_ALL_PARITY:
	case WR1_RX_INT_ALL:
		m_uart->trigger_interrupt(m_index, INT_RECEIVE);
		break;
	}
#endif
}


//-------------------------------------------------
//  cts_w - clear to send handler
//-------------------------------------------------

WRITE_LINE_MEMBER( z80scc_channel::cts_w )
{
	LOG(("Z80SCC \"%s\" %s Channel %c : CTS %u\n", m_owner->tag(), __func__, 'A' + m_index, state));

	if (m_cts != state)
	{
		// enable transmitter if in auto enables mode
		if (!state)
			if (m_wr3 & WR3_AUTO_ENABLES)
				m_wr5 |= WR5_TX_ENABLE;

		// set clear to send
		m_cts = state;

		if (!m_rx_rr0_latch)
		{
			if (!m_cts)
				m_rr0 |= RR0_CTS;
			else
				m_rr0 &= ~RR0_CTS;

			// trigger interrupt
			if (m_wr1 & WR1_EXT_INT_ENABLE)
			{
				// trigger interrupt
				m_uart->trigger_interrupt(m_index, INT_EXTERNAL);

				// latch read register 0
				m_rx_rr0_latch = 1;
			}
		}
	}
		//        m_rr0 &= ~RR0_CTS; // Remove, just to test
}


//-------------------------------------------------
//  dcd_w - data carrier detected handler
//-------------------------------------------------

WRITE_LINE_MEMBER( z80scc_channel::dcd_w )
{
	LOG(("Z80SCC \"%s\" Channel %c : DCD %u\n", m_owner->tag(), 'A' + m_index, state));

	if (m_dcd != state)
	{
		// enable receiver if in auto enables mode
		if (!state)
			if (m_wr3 & WR3_AUTO_ENABLES)
				m_wr3 |= WR3_RX_ENABLE;

		// set data carrier detect
		m_dcd = state;

		if (!m_rx_rr0_latch)
		{
			if (m_dcd)
				m_rr0 |= RR0_DCD;
			else
				m_rr0 &= ~RR0_DCD;

			if (m_wr1 & WR1_EXT_INT_ENABLE)
			{
				// trigger interrupt
				m_uart->trigger_interrupt(m_index, INT_EXTERNAL);

				// latch read register 0
				m_rx_rr0_latch = 1;
			}
		}
	}
}


//-------------------------------------------------
//  ri_w - ring indicator handler
//-------------------------------------------------

WRITE_LINE_MEMBER( z80scc_channel::ri_w )
{
	LOG(("Z80SCC \"%s\" Channel %c : RI %u\n", m_owner->tag(), 'A' + m_index, state));

	if (m_ri != state)
	{
		// set ring indicator state
		m_ri = state;

		if (!m_rx_rr0_latch)
		{
			if (m_ri)
				m_rr0 |= RR0_RI;
			else
				m_rr0 &= ~RR0_RI;

			if (m_wr1 & WR1_EXT_INT_ENABLE)
			{
				// trigger interrupt
				m_uart->trigger_interrupt(m_index, INT_EXTERNAL);

				// latch read register 0
				m_rx_rr0_latch = 1;
			}
		}
	}
}

//-------------------------------------------------
//  sync_w - sync handler
//-------------------------------------------------
WRITE_LINE_MEMBER( z80scc_channel::sync_w )
{
	LOG(("Z80SCC \"%s\" Channel %c : SYNC %u\n", m_owner->tag(), 'A' + m_index, state));
}

//-------------------------------------------------
//  rxc_w - receive clock
//-------------------------------------------------
WRITE_LINE_MEMBER( z80scc_channel::rxc_w )
{
	//LOG(("Z80SCC \"%s\" Channel %c : Receiver Clock Pulse\n", m_owner->tag(), m_index + 'A'));
	int clocks = get_clock_mode();
	if (clocks == 1)
		rx_clock_w(state);
	else if(state)
	{
		rx_clock_w(m_rx_clock < clocks/2);

		m_rx_clock++;
		if (m_rx_clock == clocks)
			m_rx_clock = 0;

	}
}

//-------------------------------------------------
//  txc_w - transmit clock
//-------------------------------------------------
WRITE_LINE_MEMBER( z80scc_channel::txc_w )
{
	//LOG(("Z80SCC \"%s\" Channel %c : Transmitter Clock Pulse\n", m_owner->tag(), m_index + 'A'));
	int clocks = get_clock_mode();
	if (clocks == 1)
		tx_clock_w(state);
	else if(state)
	{
		tx_clock_w(m_tx_clock < clocks/2);

		m_tx_clock++;
		if (m_tx_clock == clocks)
			m_tx_clock = 0;

	}
}

//-------------------------------------------------
//  update_serial -
//-------------------------------------------------
void z80scc_channel::update_serial()
{
	int data_bit_count = get_rx_word_length();
	stop_bits_t stop_bits = get_stop_bits();
	parity_t parity;

	if (m_wr4 & WR4_PARITY_ENABLE)
	{
		if (m_wr4 & WR4_PARITY_EVEN)
			parity = PARITY_EVEN;
		else
			parity = PARITY_ODD;
	}
	else
		parity = PARITY_NONE;

		LOG(("%" I64FMT "d %s() \"%s \"Channel %c setting data frame %d+%d%c%d\n", machine().firstcpu->total_cycles(), FUNCNAME, m_owner->tag(), 'A' + m_index, 1,
				data_bit_count, parity == PARITY_NONE ? 'N' : parity == PARITY_EVEN ? 'E' : 'O', (stop_bits + 1) / 2));
	set_data_frame(1, data_bit_count, parity, stop_bits);

	int clocks = get_clock_mode();

	if (m_rxc > 0)
	{
		set_rcv_rate(m_rxc / clocks);
				LOG(("       - Receiver clock: %d mode: %d rate: %d/%xh\n", m_rxc, clocks, m_rxc / clocks, m_rxc / clocks));
	}

	if (m_txc > 0)
	{
		set_tra_rate(m_txc / clocks);
				LOG(("       - Transmit clock: %d mode: %d rate: %d/%xh\n", m_rxc, clocks, m_rxc / clocks, m_rxc / clocks));
	}
}

//-------------------------------------------------
//  set_dtr -
//-------------------------------------------------
void z80scc_channel::set_dtr(int state)
{
	m_dtr = state;

		LOG(("Z80SCC \"%s\" Channel %c : %s(%d)\n", m_owner->tag(), 'A' + m_index, __func__, state));
	if (m_index == z80scc_device::CHANNEL_A)
		m_uart->m_out_dtra_cb(m_dtr);
	else
		m_uart->m_out_dtrb_cb(m_dtr);
}



//-------------------------------------------------
//  write_rx - called by terminal through rs232/diserial
//             when character is sent to board
//-------------------------------------------------
WRITE_LINE_MEMBER(z80scc_channel::write_rx)
{
		// printf("%c", state ? '+' : 'o');
	m_rxd = state;
	//only use rx_w when self-clocked
	if(m_rxc)
		device_serial_interface::rx_w(state);
}
