// license:BSD-3-Clause
// copyright-holders:Curt Coder, Joakim Larsson Edstrom
/***************************************************************************

    Z80-SIO Serial Input/Output emulation
    Z80-DART Dual Asynchronous Receiver/Transmitter emulation
    Intel 8274 Multi-Protocol Serial Controller emulation
    NEC µPD7201 Multiprotocol Serial Communications Controller emulation

    The variants in the SIO family are only different in the packaging
    but has the same register features. However, since some signals are
    not connected to the pins on the package or share a pin with another
    signal the functionality is limited. However, this driver does not
    check that an operation is invalid because of package type but relies
    on the software to be adapated for the particular version.

    Package:                DIP40  SIO/0 (RxCB and TxCB share one pin)
                            DIP40  SIO/1 (no DTRB pin)
                            DIP40  SIO/2 (no SYNCB pin)
                            DIP40  SIO/9 (channel B is nonfunctional)
                            QFP44  SIO/3
                            PLCC44 SIO/4
    -------------------------------------------------------------------
    Channels / Full Duplex  2 / Y
    Synch data rates  2Mhz  500Kbps
                      4MHz  800Kbps
                      6MHz 1200Kbps
                     10MHz 2500Kbps
   -- Asynchrounous features -------------------------------------------
  * 5-8 bit per char         Y
  * 1,1.5,2 stop bits        Y
  * odd/even parity          Y
  * x1,x16,x32,x64           Y
    break det/gen            Y
  * parity, framing &        Y
      overrun error det      Y
    -- Byte oriented synchrounous features -------------------------------
    Int/ext char sync        Y
    1/2 synch chars          Y
    Aut synch char insertion Y
    Aut CRC gen/det          Y
    -- SDLC/HDLC capabilities --------------------------------------------
    Abort seq gen/chk        Y
    Aut zero ins/det         Y
    Aut flag insert          Y
    Addr field rec           Y
    1-fld resid hand         Y
    Valid rec msg protection Y
    --
  * Receiver FIFO            3
  * Transmitter FIFO         1
    -------------------------------------------------------------------------
    * = Features that has been implemented  n/a = features that will not

    Mostek not only second-sourced the Z80 SIO but redesigned it for
    68000 compatibility as the MK68564 SIO. This 48-pin device has a
    revamped register interface with five address inputs to make every
    register separately selectable, and most control registers may be read
    back as written. The RxRDY and TxRDY pins are separate here, and many
    control bits have been shifted around. The MK68564 also features a
    built-in baud rate generator (not compatible with the Z8530 SCC's).

***************************************************************************/

#include "emu.h"
#include "z80sio.h"

#include "machine/sdlc.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG_SETUP   (1U << 1)
#define LOG_READ    (1U << 2)
#define LOG_INT     (1U << 3)
#define LOG_CMD     (1U << 4)
#define LOG_TX      (1U << 5)
#define LOG_RCV     (1U << 6)
#define LOG_CTS     (1U << 7)
#define LOG_DCD     (1U << 8)
#define LOG_SYNC    (1U << 9)
#define LOG_BIT     (1U << 10)
#define LOG_RTS     (1U << 11)
#define LOG_BRG     (1U << 12)

//#define VERBOSE  (LOG_CMD | LOG_SETUP | LOG_SYNC | LOG_BIT | LOG_TX )
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,     __VA_ARGS__)
#define LOGCMD(...)   LOGMASKED(LOG_CMD,     __VA_ARGS__)
#define LOGTX(...)    LOGMASKED(LOG_TX,      __VA_ARGS__)
#define LOGRCV(...)   LOGMASKED(LOG_RCV,     __VA_ARGS__)
#define LOGCTS(...)   LOGMASKED(LOG_CTS,     __VA_ARGS__)
#define LOGRTS(...)   LOGMASKED(LOG_RTS,     __VA_ARGS__)
#define LOGDCD(...)   LOGMASKED(LOG_DCD,     __VA_ARGS__)
#define LOGSYNC(...)  LOGMASKED(LOG_SYNC,    __VA_ARGS__)
#define LOGBIT(...)   LOGMASKED(LOG_BIT,     __VA_ARGS__)
#define LOGBRG(...)   LOGMASKED(LOG_BRG,     __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define CHANA_TAG   "cha"
#define CHANB_TAG   "chb"


enum : uint8_t
{
	RR0_RX_CHAR_AVAILABLE     = 0x01,
	RR0_INTERRUPT_PENDING     = 0x02,
	RR0_TX_BUFFER_EMPTY       = 0x04,
	RR0_DCD                   = 0x08,
	RR0_SYNC_HUNT             = 0x10, // RI on DART
	RR0_CTS                   = 0x20,
	RR0_TX_UNDERRUN           = 0x40,
	RR0_BREAK_ABORT           = 0x80
};

enum : uint8_t
{
	RR1_ALL_SENT              = 0x01,
	// This bit is not actually present in RR1 register
	// It's enqueued in the rx error FIFO to mark the spot
	// where "interrupt on 1st rx character" should occur.
	// It's stripped off before head of error FIFO is dequeued
	// into RR1
	RR1_HIDDEN_1ST_MARKER     = 0x01,
	RR1_RESIDUE_CODE_MASK     = 0x0e,
	RR1_PARITY_ERROR          = 0x10,
	RR1_RX_OVERRUN_ERROR      = 0x20,
	RR1_CRC_FRAMING_ERROR     = 0x40,
	RR1_END_OF_FRAME          = 0x80
};

enum : uint8_t
{
	RR2_INT_VECTOR_MASK       = 0xff,
	RR2_INT_VECTOR_V1         = 0x02,
	RR2_INT_VECTOR_V2         = 0x04,
	RR2_INT_VECTOR_V3         = 0x08
};

enum : uint8_t
{
	WR0_REGISTER_MASK         = 0x07,
	WR0_COMMAND_MASK          = 0x38,
	WR0_NULL                  = 0x00,
	WR0_SEND_ABORT            = 0x08,
	WR0_RESET_EXT_STATUS      = 0x10,
	WR0_CHANNEL_RESET         = 0x18,
	WR0_ENABLE_INT_NEXT_RX    = 0x20,
	WR0_RESET_TX_INT          = 0x28,
	WR0_ERROR_RESET           = 0x30,
	WR0_RETURN_FROM_INT       = 0x38,
	WR0_CRC_RESET_CODE_MASK   = 0xc0,
	WR0_CRC_RESET_NULL        = 0x00,
	WR0_CRC_RESET_RX          = 0x40,
	WR0_CRC_RESET_TX          = 0x80,
	WR0_CRC_RESET_TX_UNDERRUN = 0xc0
};

enum : uint8_t
{
	WR1_EXT_INT_ENABLE        = 0x01,
	WR1_TX_INT_ENABLE         = 0x02,
	WR1_STATUS_VECTOR         = 0x04,
	WR1_RX_INT_MODE_MASK      = 0x18,
	WR1_RX_INT_DISABLE        = 0x00,
	WR1_RX_INT_FIRST          = 0x08,
	WR1_RX_INT_ALL_PARITY     = 0x10,
	WR1_RX_INT_ALL            = 0x18,
	WR1_WRDY_ON_RX_TX         = 0x20,
	WR1_WRDY_FUNCTION         = 0x40,
	WR1_WRDY_ENABLE           = 0x80
};

enum : uint8_t
{
	WR2_DATA_XFER_INT         = 0x00, // not supported
	WR2_DATA_XFER_DMA_INT     = 0x01, // not supported
	WR2_DATA_XFER_DMA         = 0x02, // not supported
	WR2_DATA_XFER_ILLEGAL     = 0x03, // not supported
	WR2_DATA_XFER_MASK        = 0x03, // not supported
	WR2_PRIORITY              = 0x04,
	WR2_MODE_8085_1           = 0x00, // not supported
	WR2_MODE_8085_2           = 0x08, // not supported
	WR2_MODE_8086_8088        = 0x10, // not supported
	WR2_MODE_ILLEGAL          = 0x18, // not supported
	WR2_MODE_MASK             = 0x18, // not supported
	WR2_VECTORED_INT          = 0x20, // partially supported
	WR2_PIN10_SYNDETB_RTSB    = 0x80  // not supported
};

enum : uint8_t
{
	WR3_RX_ENABLE             = 0x01,
	WR3_SYNC_CHAR_LOAD_INHIBIT= 0x02,
	WR3_ADDRESS_SEARCH_MODE   = 0x04,
	WR3_RX_CRC_ENABLE         = 0x08,
	WR3_ENTER_HUNT_PHASE      = 0x10,
	WR3_AUTO_ENABLES          = 0x20,
	WR3_RX_WORD_LENGTH_MASK   = 0xc0,
	WR3_RX_WORD_LENGTH_5      = 0x00,
	WR3_RX_WORD_LENGTH_7      = 0x40,
	WR3_RX_WORD_LENGTH_6      = 0x80,
	WR3_RX_WORD_LENGTH_8      = 0xc0
};

enum : uint8_t
{
	WR4_PARITY_ENABLE         = 0x01,
	WR4_PARITY_EVEN           = 0x02,
	WR4_STOP_BITS_MASK        = 0x0c,
	WR4_STOP_BITS_SYNC        = 0x00,
	WR4_STOP_BITS_1           = 0x04,
	WR4_STOP_BITS_1_5         = 0x08,
	WR4_STOP_BITS_2           = 0x0c,
	WR4_SYNC_MODE_MASK        = 0x30,
	WR4_SYNC_MODE_8_BIT       = 0x00,
	WR4_SYNC_MODE_16_BIT      = 0x10,
	WR4_SYNC_MODE_SDLC        = 0x20,
	WR4_SYNC_MODE_EXT         = 0x30, // partially supported
	WR4_CLOCK_RATE_MASK       = 0xc0,
	WR4_CLOCK_RATE_X1         = 0x00,
	WR4_CLOCK_RATE_X16        = 0x40,
	WR4_CLOCK_RATE_X32        = 0x80,
	WR4_CLOCK_RATE_X64        = 0xc0
};

enum : uint8_t
{
	WR5_TX_CRC_ENABLE         = 0x01,
	WR5_RTS                   = 0x02,
	WR5_CRC16                 = 0x04,
	WR5_TX_ENABLE             = 0x08,
	WR5_SEND_BREAK            = 0x10,
	WR5_TX_WORD_LENGTH_MASK   = 0x60,
	WR5_TX_WORD_LENGTH_5      = 0x00,
	WR5_TX_WORD_LENGTH_6      = 0x40,
	WR5_TX_WORD_LENGTH_7      = 0x20,
	WR5_TX_WORD_LENGTH_8      = 0x60,
	WR5_DTR                   = 0x80
};

constexpr uint32_t TX_SR_MASK   = 0xfffffU;
constexpr uint16_t SDLC_RESIDUAL    = 0x1d0f;

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(Z80SIO_CHANNEL,  z80sio_channel,     "z80sio_channel",  "Z80 SIO channel")
DEFINE_DEVICE_TYPE(Z80DART_CHANNEL, z80dart_channel,    "z80dart_channel", "Z80 DART channel")
DEFINE_DEVICE_TYPE(I8274_CHANNEL,   i8274_channel,      "i8274_channel",   "Intel 8274 MPSC channel")
DEFINE_DEVICE_TYPE(MK68564_CHANNEL, mk68564_channel,    "mk68564_channel", "Mostek MK68564 SIO channel")
DEFINE_DEVICE_TYPE(Z80SIO,          z80sio_device,      "z80sio",          "Z80 SIO")
DEFINE_DEVICE_TYPE(Z80DART,         z80dart_device,     "z80dart",         "Z80 DART")
DEFINE_DEVICE_TYPE(I8274,           i8274_device,       "i8274",           "Intel 8274 MPSC")
DEFINE_DEVICE_TYPE(UPD7201,         upd7201_device,     "upd7201",         "NEC uPD7201 MPSC")
DEFINE_DEVICE_TYPE(MK68564,         mk68564_device,     "mk68564",         "Mostek MK68564 SIO")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void z80sio_device::device_add_mconfig(machine_config &config)
{
	Z80SIO_CHANNEL(config, CHANA_TAG, 0);
	Z80SIO_CHANNEL(config, CHANB_TAG, 0);
}

void z80dart_device::device_add_mconfig(machine_config &config)
{
	Z80DART_CHANNEL(config, CHANA_TAG, 0);
	Z80DART_CHANNEL(config, CHANB_TAG, 0);
}

void i8274_device::device_add_mconfig(machine_config &config)
{
	I8274_CHANNEL(config, CHANA_TAG, 0);
	I8274_CHANNEL(config, CHANB_TAG, 0);
}

void mk68564_device::device_add_mconfig(machine_config &config)
{
	MK68564_CHANNEL(config, CHANA_TAG, 0);
	MK68564_CHANNEL(config, CHANB_TAG, 0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

inline void z80sio_channel::out_txd_cb(int state)
{
	m_uart->m_out_txd_cb[m_index](state);
}

inline void z80sio_channel::out_rts_cb(int state)
{
	m_uart->m_out_rts_cb[m_index](state);
}

inline void z80sio_channel::out_dtr_cb(int state)
{
	m_uart->m_out_dtr_cb[m_index](state);
}

inline void z80sio_channel::update_wait_ready()
{
	bool ready = false;

	if (m_wr1 & WR1_WRDY_ENABLE)
	{
		if (m_wr1 & WR1_WRDY_ON_RX_TX)
		{
			// Monitor rx
			ready = bool(m_rr0 & RR0_RX_CHAR_AVAILABLE);
		}
		else
		{
			// Monitor tx
			ready = get_tx_empty();
		}
		if (!(m_wr1 & WR1_WRDY_FUNCTION))
		{
			// Ready function has opposite polarity of wait function
			ready = !ready;
		}
	}

	// ready/wait is active low
	m_uart->m_out_wrdy_cb[m_index](ready ? 0 : 1);
}

inline bool z80sio_channel::receive_allowed() const
{
	return (m_wr3 & WR3_RX_ENABLE) && (!(m_wr3 & WR3_AUTO_ENABLES) || !m_dcd);
}

bool z80sio_channel::transmit_allowed() const
{
	return (m_wr5 & WR5_TX_ENABLE) && (!(m_wr3 & WR3_AUTO_ENABLES) || !m_cts);
}

bool mk68564_channel::transmit_allowed() const
{
	return (m_wr5 & WR5_TX_ENABLE) && (!m_tx_auto_enable || !m_cts);
}

inline void z80sio_channel::set_rts(int state)
{
	if (bool(m_rts) != bool(state))
	{
		LOGRTS("%s(%d) \"%s\" Channel %c \n", FUNCNAME, state, owner()->tag(), 'A' + m_index);
		out_rts_cb(m_rts = state);
	}
}

inline void z80sio_channel::set_dtr(int state)
{
	if (bool(m_dtr) != bool(state))
	{
		LOG("%s(%d) \"%s\" Channel %c \n", FUNCNAME, state, owner()->tag(), 'A' + m_index);
		out_dtr_cb(m_dtr = state);
	}
}

inline void z80sio_channel::tx_setup(uint16_t data, int bits, bool framing, bool crc_tx, bool abort_tx)
{
	m_rr1 |= RR1_ALL_SENT;
	m_tx_parity = false;
	m_tx_sr = data;
	m_tx_sr &= ~(~uint32_t(0) << bits);
	m_tx_sr |= ~uint32_t(0) << (bits + 3);
	m_tx_flags =
			((!framing && (m_wr5 & WR5_TX_CRC_ENABLE)) ? TX_FLAG_CRC : 0U) |
			(framing ? TX_FLAG_FRAMING : 0U) |
			(abort_tx ? TX_FLAG_ABORT_TX : 0U) |
			(crc_tx ? TX_FLAG_CRC_TX : 0U) |
			(!framing && !crc_tx && !abort_tx ? TX_FLAG_DATA_TX : 0U);
	LOGBIT("%.6f TX_SR %05x data %04x flags %x\n" , machine().time().as_double() , m_tx_sr & TX_SR_MASK , data , m_tx_flags);
}

void z80sio_channel::tx_setup_idle()
{
	switch (m_wr4 & WR4_SYNC_MODE_MASK)
	{
	case WR4_SYNC_MODE_8_BIT:
	case WR4_SYNC_MODE_EXT:
		// External sync mode sends a single sync byte
		tx_setup(m_wr6, 8, true, false, false);
		break;
	case WR4_SYNC_MODE_16_BIT:
		tx_setup(uint16_t(m_wr6) | (uint16_t(m_wr7) << 8), 16, true, false, false);
		break;
	case WR4_SYNC_MODE_SDLC:
		// SDLC transmit examples don't show flag being loaded, implying it's hard-coded on the transmit side
		//tx_setup(0x7e, 8, true, false, false);
		// Verified on a 8274, the 0x7e SYNC byte is required in CR7 to start transmitting, other values fails
		tx_setup(m_wr7, 8, true, false, false);
		break;
	}
	m_tx_in_pkt = false;
}

void z80dart_channel::tx_setup_idle()
{
	logerror("%s (sync mode not supported by DART)\n", FUNCNAME);
}

//-------------------------------------------------
//  z80sio_device - constructor
//-------------------------------------------------
z80sio_device::z80sio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_z80daisy_interface(mconfig, *this),
	m_chanA(*this, CHANA_TAG),
	m_chanB(*this, CHANB_TAG),
	m_hostcpu(*this, finder_base::DUMMY_TAG),
	m_out_txd_cb(*this),
	m_out_dtr_cb(*this),
	m_out_rts_cb(*this),
	m_out_wrdy_cb(*this),
	m_out_sync_cb(*this),
	m_out_int_cb(*this),
	m_out_rxdrq_cb(*this),
	m_out_txdrq_cb(*this)
{
	for (auto & elem : m_int_state)
		elem = 0;
}

z80sio_device::z80sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	z80sio_device(mconfig, Z80SIO, tag, owner, clock)
{
}

z80dart_device::z80dart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	z80sio_device(mconfig, Z80DART, tag, owner, clock)
{
}

i8274_device::i8274_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	z80sio_device(mconfig, type, tag, owner, clock)
{
}

i8274_device::i8274_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i8274_device(mconfig, I8274, tag, owner, clock)
{
}

upd7201_device::upd7201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i8274_device(mconfig, UPD7201, tag, owner, clock)
{
}

mk68564_device::mk68564_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i8274_device(mconfig, MK68564, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_validity_check - device-specific validation
//-------------------------------------------------
void z80sio_device::device_validity_check(validity_checker &valid) const
{
	if ((m_hostcpu.finder_tag() != finder_base::DUMMY_TAG) && !m_hostcpu)
		osd_printf_error("Host CPU configured but not found.\n");
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void z80sio_device::device_start()
{
	LOG("%s\n", FUNCNAME);

	// state saving
	save_item(NAME(m_int_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void z80sio_device::device_reset()
{
	LOG("%s \"%s\" \n", FUNCNAME, tag());
}

//-------------------------------------------------
//  z80daisy_irq_state - get interrupt status
//-------------------------------------------------
int z80sio_device::z80daisy_irq_state()
{

	int const *const prio = interrupt_priorities();
	LOGINT("%s %s Hi->Lo:%d%d%d%d%d%d ", tag(), FUNCNAME,
			m_int_state[prio[0]], m_int_state[prio[1]], m_int_state[prio[2]],
			m_int_state[prio[3]], m_int_state[prio[4]], m_int_state[prio[5]]);

	// loop over all interrupt sources
	int state = 0;
	for (int i = 0; std::size(m_int_state) > i; ++i)
	{
		// if we're servicing a request, don't indicate more interrupts
		if (m_int_state[prio[i]] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= m_int_state[prio[i]];
	}

	LOGINT("Interrupt State %u\n", state);
	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - interrupt acknowledge
//-------------------------------------------------
int z80sio_device::z80daisy_irq_ack()
{
	LOGINT("%s \n", FUNCNAME);

	// loop over all interrupt sources
	int const *const prio = interrupt_priorities();
	for (int i = 0; std::size(m_int_state) > i; ++i)
	{
		// find the first channel with an interrupt requested
		if (m_int_state[prio[i]] & Z80_DAISY_INT)
		{
			m_int_state[prio[i]] |= Z80_DAISY_IEO; // Set IUS bit (called IEO in z80 daisy lingo)
			unsigned const vector = read_vector();
			LOGINT(" - Found an INT request, returning RR2: %02x\n", vector);
			check_interrupts();
			return vector;
		}
	}

	// Did we not find a vector? Get the notion of a default vector from the CPU implementation
	logerror(" - failed to find an interrupt to ack!\n");
	if (m_hostcpu)
	{
		// default irq vector is -1 for 68000 but 0 for z80 for example...
		int const ret = m_hostcpu->default_irq_vector(INPUT_LINE_IRQ0);
		LOGINT(" - failed to find an interrupt to ack [%s], returning default IRQ vector: %02x\n", m_hostcpu->tag(), ret);
		return ret;
	}

	// indicate default vector
	return -1;
}

int i8274_device::z80daisy_irq_ack()
{
	// FIXME: we're not modelling the full behaviour of this chip
	// The 8274 is designed to work with Intel processors with multiple interrupt acknowledge cycles
	// Values placed on the bus depend on WR2 A mode bits and /IPI input
	// +----+----+----+------+--------------+-------+--------+
	// | D5 | D4 | D3 | /IPI | Mode         | Cycle | Data   |
	// +----+----+----+------+--------------+-------+--------+
	// | 0  | -  | -  | -    | non-vectored | -     | hi-Z   |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 0  | 0  | 0    | 8085 (1)     | 1     | 0xcd   |
	// |    |    |    |      |              | 2     | vector |
	// |    |    |    |      |              | 3     | 0x00   |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 0  | 0  | 1    | 8085 (1)     | 1     | 0xcd   |
	// |    |    |    |      |              | 2     | hi-Z   |
	// |    |    |    |      |              | 3     | hi-Z   |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 0  | 1  | 0    | 8085 (2)     | 1     | hi-Z   |
	// |    |    |    |      |              | 2     | vector |
	// |    |    |    |      |              | 3     | 0x00   |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 0  | 1  | 1    | 8085 (2)     | 1     | hi-Z   |
	// |    |    |    |      |              | 2     | hi-Z   |
	// |    |    |    |      |              | 3     | hi-Z   |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 1  | 0  | 0    | 8086         | 1     | hi-Z   |
	// |    |    |    |      |              | 2     | vector |
	// +----+----+----+------+--------------+-------+--------+
	// | 1  | 1  | 0  | 1    | 8086         | 1     | hi-Z   |
	// |    |    |    |      |              | 2     | hi-Z   |
	// +----+----+----+------+--------------+-------+--------+
	LOGINT("%s \n", FUNCNAME);

	// don't do this in non-vectored mode
	if (m_chanB->m_wr2 & WR2_VECTORED_INT)
	{
		// loop over all interrupt sources
		int const *const prio = interrupt_priorities();
		for (int i = 0; std::size(m_int_state) > i; ++i)
		{
			// find the first channel with an interrupt requested
			if (m_int_state[prio[i]] & Z80_DAISY_INT)
			{
				m_int_state[prio[i]] |= Z80_DAISY_IEO; // Set IUS bit (called IEO in z80 daisy lingo)
				unsigned const vector = read_vector();
				LOGINT(" - Found an INT request, returning RR2: %02x\n", vector);
				check_interrupts();
				return vector;
			}
		}

		// Did we not find a vector? Get the notion of a default vector from the CPU implementation
		logerror(" - failed to find an interrupt to ack!\n");
	}

	if (m_hostcpu)
	{
		// default irq vector is -1 for 68000 but 0 for z80 for example...
		int const ret = m_hostcpu->default_irq_vector(INPUT_LINE_IRQ0);
		LOGINT(" - failed to find an interrupt to ack [%s], returning default IRQ vector: %02x\n", m_hostcpu->tag(), ret);
		return ret;
	}

	// indicate default vector
	return -1;
}


//-------------------------------------------------
//  z80daisy_irq_reti - return from interrupt
//-------------------------------------------------
void z80sio_device::z80daisy_irq_reti()
{
	LOGINT("%s\n", FUNCNAME);
	return_from_interrupt();
}

void i8274_device::z80daisy_irq_reti()
{
	LOGINT("%s - i8274/uPD7201 lacks RETI detection, no action taken\n", FUNCNAME);
}


//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------
void z80sio_device::check_interrupts()
{
	LOGINT("%s %s \n", FUNCNAME, tag());
	int state = (z80daisy_irq_state() & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE;
	m_out_int_cb(state);
}


//-------------------------------------------------
//  reset_interrupts -
//-------------------------------------------------
void z80sio_device::reset_interrupts()
{
	LOGINT("%s %s \n",FUNCNAME, tag());
	// reset internal interrupt sources
	for (auto & elem : m_int_state)
	{
		elem = 0;
	}

	check_interrupts();
}

//-------------------------------------------------
//  trigger_interrupt - interrupt has fired
//-------------------------------------------------
void z80sio_device::trigger_interrupt(int index, int type)
{
	LOGINT("%s Chan:%c Type:%s\n", FUNCNAME, 'A' + index, std::array<char const *, 3>
		   {{"INT_TRANSMIT", "INT_EXTERNAL", "INT_RECEIVE"}}[type]);

	// trigger interrupt
	if (m_int_state[(index * 3) + type] & Z80_DAISY_INT)
		return;
	m_int_state[(index * 3) + type] |= Z80_DAISY_INT;
	m_chanA->m_rr0 |= RR0_INTERRUPT_PENDING;

	// check for interrupt
	check_interrupts();
}


//-------------------------------------------------
//  clear_interrupt - interrupt has been cleared
//-------------------------------------------------
void z80sio_device::clear_interrupt(int index, int type)
{
	LOGINT("%s Chan:%c Type:%s\n", FUNCNAME, 'A' + index, std::array<char const *, 3>
		   {{"INT_TRANSMIT", "INT_EXTERNAL", "INT_RECEIVE"}}[type]);

	// clear interrupt
	if (!(m_int_state[(index * 3) + type] & Z80_DAISY_INT))
		return;
	m_int_state[(index * 3) + type] &= ~Z80_DAISY_INT;
	if (std::find_if(std::begin(m_int_state), std::end(m_int_state), [] (int state) { return bool(state & Z80_DAISY_INT); }) == std::end(m_int_state))
		m_chanA->m_rr0 &= ~RR0_INTERRUPT_PENDING;

	// update interrupt output
	check_interrupts();
}


//-------------------------------------------------
//  return_from_interrupt - reset interrupt under
//  service latch
//-------------------------------------------------
void z80sio_device::return_from_interrupt()
{
	// loop over all interrupt sources
	int const *const prio = interrupt_priorities();
	for (int i = 0; std::size(m_int_state) > i; ++i)
	{
		// find the first channel with an interrupt requested
		if (m_int_state[prio[i]] & (Z80_DAISY_IEO))
		{
			// clear the IEO state and update the IRQs
			m_int_state[prio[i]] &= ~Z80_DAISY_IEO;
			check_interrupts();
			LOGINT("%s - cleared IEO\n", FUNCNAME);
			return;
		}
	}
	LOGINT("%s - failed to find an interrupt to clear IEO on!", FUNCNAME);
}


//-------------------------------------------------
//  read_vector - read modified interrupt vector
//-------------------------------------------------
uint8_t z80sio_device::read_vector()
{
	uint8_t vec = m_chanB->m_wr2;

	// if status doesn't affect vector, return unmodified value
	if (!(m_chanB->m_wr1 & WR1_STATUS_VECTOR))
		return vec;

	// modify vector for highest-priority pending interrupt
	int const *const prio = interrupt_priorities();
	vec &= 0xf1U;
	for (int i = 0; std::size(m_int_state) > i; ++i)
	{
		if (m_int_state[prio[i]] & Z80_DAISY_INT)
		{
			switch (prio[i])
			{
			case 0 + z80sio_channel::INT_TRANSMIT:
				return vec | 0x08U;
			case 0 + z80sio_channel::INT_EXTERNAL:
				return vec | 0x0aU;
			case 0 + z80sio_channel::INT_RECEIVE:
				if (m_chanA->m_rr1 & m_chanA->get_special_rx_mask())
					return vec | 0x0eU;
				else
					return vec | 0x0cU;
			case 3 + z80sio_channel::INT_TRANSMIT:
				return vec | 0x00U;
			case 3 + z80sio_channel::INT_EXTERNAL:
				return vec | 0x02U;
			case 3 + z80sio_channel::INT_RECEIVE:
				if (m_chanB->m_rr1 & m_chanB->get_special_rx_mask())
					return vec | 0x06U;
				else
					return vec | 0x04U;
			}
		}
	}

	// no interrupt pending - stuff 011 in the variable bits
	return vec | 0x06U;
}

/*
   8274: "RR2 contains the vector which gets modified to indicate the source of interrupt. However, the state of
   the vector does not change if no new interrupts are generated. The contents of RR2 are only changed when
   a new interrupt is generated. In order to get the correct information, RR2 must be read only after an
   interrrupt is generated, otherwise it will indicate the previous state."
   8274: "If RR2 is specified but not read, no internal interrupts, regardless of priority, are accepted."
*/
uint8_t i8274_device::read_vector()
{
	// 8086 and 8085 modes have different variable bits
	bool const aff(m_chanB->m_wr1 & WR1_STATUS_VECTOR);
	int const shift(((m_chanA->m_wr2 & WR2_MODE_MASK) == WR2_MODE_8086_8088) ? 0 : 2);
	uint8_t vec(m_chanB->m_wr2);

	// if status doesn't affect vector, return unmodified value
	if (aff)
		vec &= ~(0x07U << shift);

	// modify vector for highest-priority pending interrupt
	int const *const prio = interrupt_priorities();
	for (int i = 0; std::size(m_int_state) > i; ++i)
	{
		if (m_int_state[prio[i]] & Z80_DAISY_INT)
		{
			constexpr uint8_t RR1_SPECIAL(RR1_RX_OVERRUN_ERROR | RR1_CRC_FRAMING_ERROR | RR1_END_OF_FRAME);

			// in non-vectored mode this serves the same function as the end of the second acknowldege cycle
			if (!(m_chanB->m_wr2 & WR2_VECTORED_INT) && !machine().side_effects_disabled())
			{
				m_int_state[prio[i]] |= Z80_DAISY_IEO;
				check_interrupts();
			}

			// if status doesn't affect vector return unmodified value
			if (!aff)
				return vec;

			switch (prio[i])
			{
			case 0 + z80sio_channel::INT_TRANSMIT:
				return vec | (0x04U << shift);
			case 0 + z80sio_channel::INT_EXTERNAL:
				return vec | (0x05U << shift);
			case 0 + z80sio_channel::INT_RECEIVE:
				if (((m_chanA->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL_PARITY) && (m_chanA->m_rr1 & (RR1_SPECIAL | RR1_PARITY_ERROR)))
					return vec | (0x07U << shift);
				else if (((m_chanA->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL) && (m_chanA->m_rr1 & RR1_SPECIAL))
					return vec | (0x07U << shift);
				else
					return vec | (0x06U << shift);
			case 3 + z80sio_channel::INT_TRANSMIT:
				return vec | (0x00U << shift);
			case 3 + z80sio_channel::INT_EXTERNAL:
				return vec | (0x01U << shift);
			case 3 + z80sio_channel::INT_RECEIVE:
				if (((m_chanB->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL_PARITY) && (m_chanB->m_rr1 & (RR1_SPECIAL | RR1_PARITY_ERROR)))
					return vec | (0x03U << shift);
				else if (((m_chanB->m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL) && (m_chanB->m_rr1 & RR1_SPECIAL))
					return vec | (0x03U << shift);
				else
					return vec | (0x02U << shift);
			}
		}
	}

	// no interrupt pending - stuff 111 in the variable bits
	return aff ? (vec | (0x07 << shift)) : vec;
}


//-------------------------------------------------
//  interrupt_priorities - get interrupt indexes
//  in priority order
//-------------------------------------------------
int const *z80sio_device::interrupt_priorities() const
{
	static constexpr EQUIVALENT_ARRAY(m_int_state, int) prio{
			0 + z80sio_channel::INT_RECEIVE, 0 + z80sio_channel::INT_TRANSMIT, 0 + z80sio_channel::INT_EXTERNAL,
			3 + z80sio_channel::INT_RECEIVE, 3 + z80sio_channel::INT_TRANSMIT, 3 + z80sio_channel::INT_EXTERNAL };
	return prio;
}

int const *i8274_device::interrupt_priorities() const
{
	static constexpr EQUIVALENT_ARRAY(m_int_state, int) prio_a{
			0 + z80sio_channel::INT_RECEIVE, 3 + z80sio_channel::INT_RECEIVE,
			0 + z80sio_channel::INT_TRANSMIT, 3 + z80sio_channel::INT_TRANSMIT,
			0 + z80sio_channel::INT_EXTERNAL, 3 + z80sio_channel::INT_EXTERNAL };
	static constexpr EQUIVALENT_ARRAY(m_int_state, int) prio_b{
			0 + z80sio_channel::INT_RECEIVE, 0 + z80sio_channel::INT_TRANSMIT,
			3 + z80sio_channel::INT_RECEIVE, 3 + z80sio_channel::INT_TRANSMIT,
			0 + z80sio_channel::INT_EXTERNAL, 3 + z80sio_channel::INT_EXTERNAL };
	return (m_chanA->m_wr2 & WR2_PRIORITY) ? prio_a : prio_b;
}


//-------------------------------------------------
//  m1_r - interrupt acknowledge
//-------------------------------------------------
int z80sio_device::m1_r()
{
	LOGINT("%s %s \n",FUNCNAME, tag());
	return z80daisy_irq_ack();
}


//-------------------------------------------------
//  cd_ba_r -
//-------------------------------------------------
uint8_t z80sio_device::cd_ba_r(offs_t offset)
{
	int ba = BIT(offset, 0);
	int cd = BIT(offset, 1);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

	return cd ? channel->control_read() : channel->data_read();
}


//-------------------------------------------------
//  cd_ba_w -
//-------------------------------------------------
void z80sio_device::cd_ba_w(offs_t offset, uint8_t data)
{
	int ba = BIT(offset, 0);
	int cd = BIT(offset, 1);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

	if (cd)
		channel->control_write(data);
	else
		channel->data_write(data);
}


//-------------------------------------------------
//  ba_cd_r -
//-------------------------------------------------
uint8_t z80sio_device::ba_cd_r(offs_t offset)
{
	int ba = BIT(offset, 1);
	int cd = BIT(offset, 0);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

	return cd ? channel->control_read() : channel->data_read();
}


//-------------------------------------------------
//  ba_cd_w -
//-------------------------------------------------
void z80sio_device::ba_cd_w(offs_t offset, uint8_t data)
{
	int ba = BIT(offset, 1);
	int cd = BIT(offset, 0);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

	if (cd)
		channel->control_write(data);
	else
		channel->data_write(data);
}

//**************************************************************************
//  SIO CHANNEL
//**************************************************************************

//-------------------------------------------------
//  z80sio_channel - constructor
//-------------------------------------------------
z80sio_channel::z80sio_channel(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		uint8_t rr1_auto_reset)
	: device_t(mconfig, type, tag, owner, clock)
	, m_rx_fifo_depth(0)
	, m_rx_data_fifo(0)
	, m_rx_error_fifo(0)
	, m_rx_clock(0)
	, m_rx_count(0)
	, m_rx_bit(0)
	, m_rx_sr(0)
	, m_rx_first(false)
	, m_rxd(1)
	, m_tx_data(0)
	, m_tx_clock(0), m_tx_count(0), m_tx_parity(0), m_tx_sr(0), m_tx_crc(0), m_tx_hist(0), m_tx_flags(0)
	, m_txd(1), m_dtr(0), m_rts(0)
	, m_ext_latched(false), m_brk_latched(false)
	, m_cts(0), m_dcd(0), m_sync(0)
	, m_rr1_auto_reset(rr1_auto_reset)
{
	LOG("%s\n",FUNCNAME);

	// Reset all registers
	m_rr0 = m_rr1 =  0;
	m_wr0 = m_wr1 = m_wr2 = m_wr3 = m_wr4 = m_wr5 = m_wr6 = m_wr7 = 0;
}

z80sio_channel::z80sio_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z80sio_channel(mconfig, Z80SIO_CHANNEL, tag, owner, clock, RR1_CRC_FRAMING_ERROR | RR1_RESIDUE_CODE_MASK)
{
}

z80dart_channel::z80dart_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z80sio_channel(mconfig, Z80DART_CHANNEL, tag, owner, clock, RR1_END_OF_FRAME | RR1_CRC_FRAMING_ERROR | RR1_RESIDUE_CODE_MASK)
{
}

i8274_channel::i8274_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z80sio_channel(mconfig, I8274_CHANNEL, tag, owner, clock, RR1_RX_OVERRUN_ERROR)
{
}

mk68564_channel::mk68564_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z80sio_channel(mconfig, MK68564_CHANNEL, tag, owner, clock, RR1_END_OF_FRAME | RR1_CRC_FRAMING_ERROR | RR1_RESIDUE_CODE_MASK)
	, m_tx_auto_enable(false)
	, m_brg_tc(0)
	, m_brg_control(0)
	, m_brg_state(false)
	, m_brg_timer(nullptr)
{
}


//-------------------------------------------------
//  resove_objects - channel setup
//-------------------------------------------------
void z80sio_channel::device_resolve_objects()
{
	LOG("%s\n",FUNCNAME);
	m_uart = downcast<z80sio_device *>(owner());
	m_index = m_uart->get_channel_index(this);
}

//-------------------------------------------------
//  start - channel startup
//-------------------------------------------------
void z80sio_channel::device_start()
{
	LOG("%s\n",FUNCNAME);

	// state saving
	save_item(NAME(m_rr0));
	save_item(NAME(m_rr1));
	save_item(NAME(m_wr0));
	save_item(NAME(m_wr1));
	save_item(NAME(m_wr2));
	save_item(NAME(m_wr3));
	save_item(NAME(m_wr4));
	save_item(NAME(m_wr5));
	save_item(NAME(m_rx_fifo_depth));
	save_item(NAME(m_rx_data_fifo));
	save_item(NAME(m_rx_error_fifo));
	save_item(NAME(m_rx_clock));
	save_item(NAME(m_rx_count));
	save_item(NAME(m_rx_bit));
	save_item(NAME(m_rx_sr));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_rx_first));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_tx_clock));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_tx_phase));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_tx_in_pkt)); // TODO: does this actually function in async mode?
	save_item(NAME(m_tx_sr));
	save_item(NAME(m_tx_hist));
	save_item(NAME(m_tx_flags));
	save_item(NAME(m_tx_delay));
	save_item(NAME(m_all_sent_delay));
	save_item(NAME(m_txd));
	save_item(NAME(m_dtr));
	save_item(NAME(m_rts));
	save_item(NAME(m_ext_latched));
	save_item(NAME(m_brk_latched));
	save_item(NAME(m_dcd));
	save_item(NAME(m_sync));
	save_item(NAME(m_cts));
	sync_save_state();
}

void z80sio_channel::sync_save_state()
{
	save_item(NAME(m_wr6));
	save_item(NAME(m_wr7));
	save_item(NAME(m_dlyd_rxd));
	save_item(NAME(m_rx_bit_limit));
	save_item(NAME(m_rx_sync_fsm));
	save_item(NAME(m_rx_one_cnt));
	save_item(NAME(m_rx_sync_sr));
	save_item(NAME(m_rx_crc_delay));
	save_item(NAME(m_rx_crc));
	save_item(NAME(m_rx_crc_en));
	//save_item(NAME(m_tx_in_pkt));
	save_item(NAME(m_tx_forced_sync));
	save_item(NAME(m_tx_crc));
}

void z80dart_channel::sync_save_state()
{
	// no need to save the above members
}

void mk68564_channel::device_start()
{
	z80sio_channel::device_start();

	m_brg_timer = timer_alloc(FUNC(mk68564_channel::brg_timeout), this);

	save_item(NAME(m_tx_auto_enable));
	save_item(NAME(m_brg_tc));
	save_item(NAME(m_brg_control));
	save_item(NAME(m_brg_state));
}


//-------------------------------------------------
//  reset - reset channel status
//-------------------------------------------------
void z80sio_channel::device_reset()
{
	LOG("%s\n", FUNCNAME);

	// Reset RS232 emulation
	m_rx_fifo_depth = 0;
	m_rx_data_fifo = m_rx_error_fifo = 0U;
	m_rx_bit = 0;
	m_rx_one_cnt = 0;
	m_rx_sync_fsm = SYNC_FSM_HUNT;
	m_tx_count = 0;
	m_rr0 &= ~RR0_RX_CHAR_AVAILABLE;
	m_rr0 |= RR0_SYNC_HUNT;
	m_rr1 &= ~(RR1_PARITY_ERROR | RR1_RX_OVERRUN_ERROR | RR1_CRC_FRAMING_ERROR | RR1_END_OF_FRAME);

	// disable receiver
	m_wr3 &= ~WR3_RX_ENABLE;

	// disable transmitter
	m_wr5 &= ~WR5_TX_ENABLE;
	m_rr0 |= RR0_TX_BUFFER_EMPTY | RR0_TX_UNDERRUN;
	m_rr1 |= RR1_ALL_SENT;
	m_tx_flags = 0U;
	m_tx_delay = ~0;
	m_all_sent_delay = 0;
	m_tx_in_pkt = false;
	m_tx_forced_sync = true;
	m_txd = 1;
	out_txd_cb(1);
	m_tx_sr = ~0;

	// Disable wait & ready
	m_wr1 &= ~WR1_WRDY_ENABLE;
	update_wait_ready();

	// reset external lines
	out_rts_cb(m_rts = 1);
	out_dtr_cb(m_dtr = 1);

	// reset interrupts
	m_uart->clear_interrupt(m_index, INT_TRANSMIT);
	m_uart->clear_interrupt(m_index, INT_RECEIVE);
	reset_ext_status();
	// FIXME: should this actually reset all the interrupts, or just the prioritisation (daisy chain) logic?
	if (m_index == z80sio_device::CHANNEL_A)
		m_uart->reset_interrupts();
}

void mk68564_channel::device_reset()
{
	z80sio_channel::device_reset();

	m_tx_auto_enable = false;
	m_brg_tc = 0;
	m_brg_control = 0;
	m_brg_state = false;
	m_brg_timer->adjust(attotime::never);
}

bool z80sio_channel::is_tx_idle() const
{
	return (m_tx_sr & TX_SR_MASK) == TX_SR_MASK;
}

//-------------------------------------------------
//  transmit_enable - start transmission if
//  conditions met
//-------------------------------------------------
void z80sio_channel::transmit_enable()
{
	LOGTX("%s\n", FUNCNAME);

	if (transmit_allowed())
	{
		if (is_tx_idle())
		{
			if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC)
			{
				LOGTX("Channel %c synchronous transmit enabled - load sync pattern\n", 'A' + m_index);
				tx_setup_idle();
				m_tx_forced_sync = false;
				update_wait_ready();
			}
			else if (!(m_rr0 & RR0_TX_BUFFER_EMPTY))
				async_tx_setup();
		}
	}
	else
	{
		// Send at least 1 sync once tx is re-enabled
		m_tx_forced_sync = true;
		LOGBIT("tx forced set 1\n");

		// If tx is disabled during CRC transmission, flag/sync is sent for the remaining bits
		if (m_tx_flags & TX_FLAG_CRC_TX)
		{
			m_tx_flags = TX_FLAG_FRAMING;
			set_tx_empty(false , (m_rr0 & RR0_TX_BUFFER_EMPTY) != 0);
		}
		m_tx_in_pkt = false;
		// Not sure if RR0_TX_UNDERRUN is set when tx is disabled. It certainly makes sense to be that way.
		m_rr0 |= RR0_TX_UNDERRUN;
	}
}

//-------------------------------------------------
//  transmit_complete - transmit shift register
//  empty
//-------------------------------------------------
void z80sio_channel::transmit_complete()
{
	if (!m_rts) LOGTX("%s %s\n",FUNCNAME, tag());

	if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC)
		sync_tx_sr_empty();
	else if (transmit_allowed() && !(m_rr0 & RR0_TX_BUFFER_EMPTY))
		async_tx_setup(); // async mode, with data available
	else
		LOGTX("%s() \"%s \"Channel %c Transmit buffer empty m_wr5:%02x\n", FUNCNAME, owner()->tag(), 'A' + m_index, m_wr5);
}

//-------------------------------------------------
//  sync_tx_sr_empty - set up next chunk of bits
//  to send
//-------------------------------------------------
void z80sio_channel::sync_tx_sr_empty()
{
	if (!transmit_allowed())
	{
		if (!m_rts) LOGTX("%s() Channel %c Transmitter Disabled m_wr5:%02x\n", FUNCNAME, 'A' + m_index, m_wr5);
		m_tx_flags = 0;
	}
	else if (m_tx_forced_sync ||
			 ((m_rr0 & RR0_TX_BUFFER_EMPTY) && ((m_rr0 & RR0_TX_UNDERRUN) || !(m_wr5 & WR5_TX_CRC_ENABLE))))
	{
		LOGBIT("tx forced = %d\n" , m_tx_forced_sync);
		m_tx_forced_sync = false;

		if (!(m_rr0 & RR0_TX_UNDERRUN))
		{
			m_rr0 |= RR0_TX_UNDERRUN;
			trigger_ext_int();
		}
		// TODO: Check
		// if ((m_tx_flags & (TX_FLAG_CRC_TX | TX_FLAG_DATA_TX)) && (m_wr1 & WR1_TX_INT_ENABLE))
		//  // At the beginning of the sync/flag sequence that closes a frame, send tx interrupt
		//  m_uart->trigger_interrupt(m_index, INT_TRANSMIT);
		if (m_tx_flags & TX_FLAG_CRC_TX)
		{
			// At the end of CRC transmission, set tx empty
			m_tx_flags = 0;
			set_tx_empty (false , (m_rr0 & RR0_TX_BUFFER_EMPTY) != 0);
		}
		tx_setup_idle();
	}
	else if (!(m_rr0 & RR0_TX_BUFFER_EMPTY))
	{
		LOGTX("%s() Channel %c Transmit Data Byte '%02x' m_wr5:%02x\n", FUNCNAME, 'A' + m_index, m_tx_data, m_wr5);
		tx_setup(m_tx_data, get_tx_word_length(), false, false, false);
		// empty transmit buffer
		set_tx_empty(false , true);
	}
	else
	{
		LOGTX("%s() Channel %c Transmit FCS '%04x' m_wr5:%02x\n", FUNCNAME, 'A' + m_index, m_tx_crc, m_wr5);

		// Send CRC. 16 bits are counted by loading 2 flag/sync bytes into tx SR (these bits
		// are actually sent out when tx is disabled during CRC transmission)
		uint16_t flags = 0;
		switch (m_wr4 & WR4_SYNC_MODE_MASK)
		{
		case WR4_SYNC_MODE_8_BIT:
		case WR4_SYNC_MODE_EXT:
			flags = (uint16_t(m_wr6) << 8) | m_wr6;
			break;
		case WR4_SYNC_MODE_16_BIT:
			flags = uint16_t(m_wr6) | (uint16_t(m_wr7) << 8);
			break;
		case WR4_SYNC_MODE_SDLC:
			flags = 0x7e7e;
			// In SDLC mode, invert CRC before sending it out
			m_tx_crc = ~m_tx_crc;
			// In addition, ensure at least 1 flag is sent out before next frame
			m_tx_forced_sync = true;
			break;
		}
		tx_setup(flags, 16, false, true, false);
		set_tx_empty(true , true);
		LOGBIT("Send CRC=%04x\n" , m_tx_crc);

		// set the underrun flag so it will send sync next time
		m_rr0 |= RR0_TX_UNDERRUN;
		trigger_ext_int();
	}
}

void z80dart_channel::sync_tx_sr_empty()
{
	LOG("%s (sync mode not supported by DART)\n", FUNCNAME);
}

bool z80sio_channel::get_tx_empty() const
{
	// During CRC transmission, tx buffer is shown as full
	return (m_rr0 & RR0_TX_BUFFER_EMPTY) &&
		(m_tx_flags & TX_FLAG_CRC_TX) == 0;
}

void z80sio_channel::set_tx_empty(bool prev_state, bool new_state)
{
	if (new_state)
		m_rr0 |= RR0_TX_BUFFER_EMPTY;
	else
		m_rr0 &= ~RR0_TX_BUFFER_EMPTY;

	update_wait_ready();

	bool curr_tx_empty = get_tx_empty();

	if (!prev_state && curr_tx_empty && (m_wr1 & WR1_TX_INT_ENABLE))
	{
		m_uart->trigger_interrupt(m_index, INT_TRANSMIT);
	}
}

void z80sio_channel::update_crc(uint16_t& crc , bool bit)
{
	if (BIT(crc , 15) ^ bit)
		crc = (crc << 1) ^ ((m_wr5 & WR5_CRC16) ? 0x8005U : 0x1021U);
	else
		crc <<= 1;
}

//-------------------------------------------------
//  async_tx_setup - set up for asynchronous
//  transmission
//-------------------------------------------------
void z80sio_channel::async_tx_setup()
{
	LOGTX("%s() Channel %c Transmit Data Byte '%02x' m_wr5:%02x\n", FUNCNAME, 'A' + m_index, m_tx_data, m_wr5);

	// 5 bits: | 11x 1 | tx_data (8 bits) | 0 |
	// 6 bits: | 10x 1 | 000 | tx_data (6 bits) | 0 |
	// 7 bits: |  9x 1 | 000 | tx_data (7 bits) | 0 |
	// 8 bits: |  8x 1 | 000 | tx_data (8 bits) | 0 |
	// Add start bit on the right
	m_tx_sr = uint32_t(m_tx_data) << 1;
	auto wl = get_tx_word_length();
	if (wl != 5)
		// Filter out bits to be ignored in m_tx_data
		m_tx_sr &= ~(~uint32_t(0) << (wl + 1));
	// Add 1s on the left
	m_tx_sr |= ~uint32_t(0) << (wl + 4);
	LOGBIT("TX_SR %05x TX_D %02x\n" , m_tx_sr & TX_SR_MASK , m_tx_data);
	m_tx_parity = false;

	m_tx_flags = TX_FLAG_DATA_TX;

	// empty transmit buffer
	set_tx_empty(false , true);
	m_rr1 &= ~RR1_ALL_SENT;
	m_all_sent_delay = 0;
}


//-------------------------------------------------
//  reset_ext_status - reset external/status
//  condiotions
//-------------------------------------------------
void z80sio_channel::reset_ext_status()
{
	// this will clear latched external pin state
	m_ext_latched = false;
	m_brk_latched = false;
	read_ext();

	// Clear any pending External interrupt
	m_uart->clear_interrupt(m_index, INT_EXTERNAL);
}


//-------------------------------------------------
//  read_ext - copy external status to register
//-------------------------------------------------
void z80sio_channel::read_ext()
{
	// clear to send
	if (m_cts)
		m_rr0 &= ~RR0_CTS;
	else
		m_rr0 |= RR0_CTS;

	// data carrier detect
	if (m_dcd)
		m_rr0 &= ~RR0_DCD;
	else
		m_rr0 |= RR0_DCD;

	// sync is a general-purpose input in asynchronous mode
	if ((m_wr4 & WR4_STOP_BITS_MASK) != WR4_STOP_BITS_SYNC)
	{
		if (m_sync)
			m_rr0 &= ~RR0_SYNC_HUNT;
		else
			m_rr0 |= RR0_SYNC_HUNT;
	}
}

//-------------------------------------------------
//  trigger_ext_int - trigger external signal
//  interrupt
//-------------------------------------------------
void z80sio_channel::trigger_ext_int()
{
	// update line
	if (!m_ext_latched)
		read_ext();
	m_ext_latched = true;

	// trigger interrupt if enabled
	if (m_wr1 & WR1_EXT_INT_ENABLE)
		m_uart->trigger_interrupt(m_index, INT_EXTERNAL);
}


//-------------------------------------------------
//  get_clock_mode - get clock divisor
//-------------------------------------------------
int z80sio_channel::get_clock_mode() const
{
	//LOG("%s %s\n",FUNCNAME, tag());
	int clocks = 1;

	switch (m_wr4 & WR4_CLOCK_RATE_MASK)
	{
	case WR4_CLOCK_RATE_X1: clocks = 1; break;
	case WR4_CLOCK_RATE_X16:    clocks = 16;    break;
	case WR4_CLOCK_RATE_X32:    clocks = 32;    break;
	case WR4_CLOCK_RATE_X64:    clocks = 64;    break;
	}

	return clocks;
}

/*
   From "uPD7201/7201A MULTI PROTOCOL SERIAL COMMUNICATION CONTROLLER" by NEC:
   "RTSA (Request to Send A): The state of the RTS bit (01 of the CR5 register) controls this pin. If
   the RTS bit is reset in the asynchronous mode, a high level will not be output on the RTS pin until
   all transmit characters are written and the all sent bit (D0 of the SR1 register) is set. In the
   synchronous mode, the state of the RTS bit is used as is. That is, when the RTS bit is 0, the RTS
   pin is 1. When the RTS bit is 1, the RTS pin is O."

   CR5 = m_wr5 and SR1 = m_rr1
*/
void z80sio_channel::update_dtr_rts_break()
{
	//    LOG("%s(%d) \"%s\" Channel %c \n", FUNCNAME, state, owner()->tag(), 'A' + m_index);
	LOG("%s() \"%s\" Channel %c \n", FUNCNAME, owner()->tag(), 'A' + m_index);

	// RTS is affected by transmit queue state in asynchronous mode
	if (m_wr5 & WR5_RTS)
		set_rts(0); // when the RTS bit is set, the _RTS output goes low
	else if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC || (m_rr1 & RR1_ALL_SENT))
		set_rts(1); // in synchronous mode, there's no automatic RTS

	// break immediately forces spacing condition on TxD output
	out_txd_cb((m_wr5 & WR5_SEND_BREAK) ? 0 : m_txd);

	// data terminal ready output follows the state programmed into the DTR bit
	set_dtr((m_wr5 & WR5_DTR) ? 0 : 1);
}


//-------------------------------------------------
//  get_rx_word_length - get receive word length
//-------------------------------------------------
int z80sio_channel::get_rx_word_length() const
{
	LOG("%s %s\n",FUNCNAME, tag());
	int bits = 5;

	switch (m_wr3 & WR3_RX_WORD_LENGTH_MASK)
	{
	case WR3_RX_WORD_LENGTH_5:  bits = 5;   break;
	case WR3_RX_WORD_LENGTH_6:  bits = 6;   break;
	case WR3_RX_WORD_LENGTH_7:  bits = 7;   break;
	case WR3_RX_WORD_LENGTH_8:  bits = 8;   break;
	}

	return bits;
}


//-------------------------------------------------
//  get_tx_word_length - get transmit word length
//-------------------------------------------------
int z80sio_channel::get_tx_word_length() const
{
	LOG("%s\n", FUNCNAME);

	switch (m_wr5 & WR5_TX_WORD_LENGTH_MASK)
	{
	case WR5_TX_WORD_LENGTH_5: return 5;
	case WR5_TX_WORD_LENGTH_6: return 6;
	case WR5_TX_WORD_LENGTH_7: return 7;
	case WR5_TX_WORD_LENGTH_8: return 8;
	}

	return 5;
}

/*
 * This register contains the status of the receive and transmit buffers; the
 * DCD, CTS, and SYNC inputs; the Transmit Underrun/EOM latch; and the
 * Break/Abort latch. */
uint8_t z80sio_channel::do_sioreg_rr0()
{
	uint8_t tmp = m_rr0 & ~RR0_TX_BUFFER_EMPTY;
	if (get_tx_empty())
		tmp |= RR0_TX_BUFFER_EMPTY;
	LOGR("%s: %02x\n", FUNCNAME, tmp);
	return tmp;
}

/*
 * This register contains the Special Receive condition status bits and Residue
 * codes for the I-Field in the SDLC Receive Mode. */
uint8_t z80sio_channel::do_sioreg_rr1()
{
	LOGR("%s\n", FUNCNAME);
	return m_rr1;
}

/* Z80-SIO Technical Manual: "This register contains the interrupt vector
   written into WR2 if the Status Affects Vector control bit is not set.
   If the control bit is set, it contains the modified vector listed in
   the Status Affects Vector paragraph of the Write Register 1 section.
   When this register is read, the vector returned is modified by the
   highest priority interrupting condition at the time of the read. If
   no interrupts are pending, the vector is modified with V3 = 0, V2 = 1, and
   V1 = 1. This register is read only through Channel B."

   Intel 8274 datasheet: "RR2 - Channel B: Interrupt Vector - Contains the interrupt
   vector programmed in into WR2. If the status affects vector mode is selected (WR1:D2),
   it containes the modified vector for the highest priority interrupt pending.
   If no interrupts are pending the variable bits in the vector are set to one."

   NEC upd7201 MPSC2 Technical Manual: "When the MPSC2 is used in vectored mode, the
   contents of this register are placed on the bus during the appropriate portion of
   interrupt acknowledge sequence. You can read the value of CR2B at any time.
   This is particularly useful in determining the cause of an interrupt when using the
   MPSC2 in Non-vectored mode."
*/
uint8_t z80sio_channel::do_sioreg_rr2()
{
	LOGINT("%s %s Chan:%c\n", tag(), FUNCNAME, 'A' + m_index);

	// channel B only, channel A returns 0
	if (m_index == z80sio_device::CHANNEL_A)
		return 0U;
	else
		return m_uart->read_vector();
}


//-------------------------------------------------
//  control_read - read control register
//-------------------------------------------------
uint8_t z80sio_channel::control_read()
{
	uint8_t data = 0;
	uint8_t const reg  = m_wr0 & WR0_REGISTER_MASK;

	//LOG("%s %s\n",FUNCNAME, tag());
	// mask out register index
	if (!machine().side_effects_disabled())
		m_wr0 &= ~WR0_REGISTER_MASK;

	switch (reg)
	{
	case REG_RR0_STATUS:         data = do_sioreg_rr0(); break;
	case REG_RR1_SPEC_RCV_COND:  data = do_sioreg_rr1(); break;
	case REG_RR2_INTERRUPT_VECT: data = do_sioreg_rr2(); break;
	default:
		logerror("Z80SIO \"%s\" Channel %c : Unsupported RRx register:%02x\n", owner()->tag(), 'A' + m_index, reg);
		LOG("%s %s unsupported register:%02x\n",FUNCNAME, tag(), reg);
	}

	LOGR(" * %s %c Reg %02x -> %02x - %s\n", tag(), 'A' + m_index, reg, data, std::array<char const *, 3>
		 {{"RR0 status register", "RR1 - Special Receive Conditions", "RR2 - Interrupt Vector"}}[reg]);
	return data;
}

/* SIO CRC Initialization Code handling - candidate for breaking out in a z80sio_base class
 Handle the WR0 CRC Reset/Init bits separatelly, needed by derived devices separatelly from the commands */
void z80sio_channel::do_sioreg_wr0_resets(uint8_t data)
{
	LOG("%s\n", FUNCNAME);
	switch (data & WR0_CRC_RESET_CODE_MASK)
	{
	case WR0_CRC_RESET_NULL:
		LOGCMD("Z80SIO Channel %c : CRC_RESET_NULL\n", 'A' + m_index);
		break;
	case WR0_CRC_RESET_RX: /* In Synchronous mode: all Os (zeros) (CCITT-O CRC-16) */
		LOGCMD("Z80SIO Channel %c : CRC_RESET_RX\n", 'A' + m_index);
		m_rx_crc = ((m_wr4 & WR4_SYNC_MODE_MASK) == WR4_SYNC_MODE_SDLC) ? ~uint16_t(0U) : uint16_t(0U);
		m_rx_crc_en = false;
		break;
	case WR0_CRC_RESET_TX: /* In HDLC mode: all 1s (ones) (CCITT-1) */
		LOGCMD("Z80SIO Channel %c : CRC_RESET_TX\n", 'A' + m_index);
		m_tx_crc = ((m_wr4 & WR4_SYNC_MODE_MASK) == WR4_SYNC_MODE_SDLC) ? ~uint16_t(0U) : uint16_t(0U);
		break;
	case WR0_CRC_RESET_TX_UNDERRUN: /* Resets Tx underrun/EOM bit (D6 of the SRO register) */
		LOGCMD("Z80SIO Channel %c : CRC_RESET_TX_UNDERRUN\n", 'A' + m_index);
		// Command is accepted in active part of packet only
		if (m_tx_in_pkt)
			m_rr0 &= ~RR0_TX_UNDERRUN;
		else
			LOGCMD(" - not accepted as not in active part of packet\n");
		break;
	default: /* Will not happen unless someone messes with the mask */
		logerror("Z80SIO Channel %c : %s Wrong CRC reset/init command:%02x\n", 'A' + m_index, FUNCNAME, data & WR0_CRC_RESET_CODE_MASK);
	}
}

void z80sio_channel::do_sioreg_wr0(uint8_t data)
{
	m_wr0 = data;

	if ((data & WR0_COMMAND_MASK) != WR0_NULL)
		LOGSETUP("\n * %s %c Reg %02x <- %02x \n", owner()->tag(), 'A' + m_index, 0, data);
	switch (data & WR0_COMMAND_MASK)
	{
	case WR0_NULL:
		LOGCMD("%s Ch:%c : Null command\n", FUNCNAME, 'A' + m_index);
		break;
	case WR0_SEND_ABORT:
		// TODO: what actually happens if you try this in a mode other than SDLC?
		if ((m_wr4 & (WR4_STOP_BITS_MASK | WR4_SYNC_MODE_MASK)) != (WR4_STOP_BITS_SYNC | WR4_SYNC_MODE_SDLC))
		{
			LOGCMD("%s Ch:%c : Send abort command (not in SDLC mode, ignored)\n", FUNCNAME, 'A' + m_index);
		}
		else
		{
			LOGCMD("%s Ch:%c : Send abort command\n", FUNCNAME, 'A' + m_index);
			bool prev_tx_empty = get_tx_empty();
			tx_setup(0xff, 8, true, false, true);
			set_tx_empty(prev_tx_empty , true);
		}
		break;
	case WR0_RESET_EXT_STATUS:
		reset_ext_status();
		LOGINT("%s Ch:%c : Reset External/Status Interrupt\n", FUNCNAME, 'A' + m_index);
		break;
	case WR0_CHANNEL_RESET:
		// channel reset
		LOGCMD("%s Ch:%c : Channel Reset\n", FUNCNAME, 'A' + m_index);
		device_reset();
		break;
	case WR0_ENABLE_INT_NEXT_RX:
		// enable interrupt on next receive character
		LOGINT("%s Ch:%c : Enable Interrupt on Next Received Character\n", FUNCNAME, 'A' + m_index);
		m_rx_first = true;
		break;
	case WR0_RESET_TX_INT:
		LOGCMD("%s Ch:%c : Reset Transmitter Interrupt Pending\n", FUNCNAME, 'A' + m_index);
		// reset transmitter interrupt pending
		m_uart->clear_interrupt(m_index, INT_TRANSMIT);
		break;
	case WR0_ERROR_RESET:
		// error reset
		LOGCMD("%s Ch:%c : Error Reset\n", FUNCNAME, 'A' + m_index);
		if ((WR1_RX_INT_FIRST == (m_wr1 & WR1_RX_INT_MODE_MASK)) && (m_rr1 & (RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_END_OF_FRAME)))
		{
			// clearing framing and overrun errors advances the FIFO
			// TODO: Intel 8274 manual doesn't mention this behaviour - is it specific to Z80 SIO?
			m_rr1 &= ~(RR1_END_OF_FRAME | RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR);
			advance_rx_fifo();
		}
		else
		{
			m_rr1 &= ~(RR1_END_OF_FRAME | RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR);
			update_rx_int();
		}
		break;
	case WR0_RETURN_FROM_INT:
		LOGINT("%s Ch:%c : Return from interrupt\n", FUNCNAME, 'A' + m_index);
		if (m_index == z80sio_device::CHANNEL_A)
			m_uart->return_from_interrupt();
		break;
	default:
		LOG("Z80SIO Channel %c : Unsupported WR0 command %02x mask %02x\n", 'A' + m_index, data, WR0_REGISTER_MASK);

	}
	do_sioreg_wr0_resets(data);
}

void z80sio_channel::do_sioreg_wr1(uint8_t data)
{
/* TODO: implement vector modifications when WR1 bit D2 is changed */
	m_wr1 = data;
	LOGSETUP("Z80SIO \"%s\" Channel %c :\n", owner()->tag(), 'A' + m_index);
	LOGSETUP(" - External Interrupt Enable %u\n", (data & WR1_EXT_INT_ENABLE) ? 1 : 0);
	LOGSETUP(" - Transmit Interrupt Enable %u\n", (data & WR1_TX_INT_ENABLE) ? 1 : 0);
	LOGSETUP(" - Status Affects Vector %u\n", (data & WR1_STATUS_VECTOR) ? 1 : 0);
	LOGSETUP(" - Wait/Ready Enable %u\n",     (data & WR1_WRDY_ENABLE) ? 1 : 0);
	LOGSETUP(" - Wait/Ready Function %s\n",   (data & WR1_WRDY_FUNCTION) ? "Ready" : "Wait");
	LOGSETUP(" - Wait/Ready on %s\n",         (data & WR1_WRDY_ON_RX_TX) ? "Rx" : "Tx");
	LOGSETUP(" - Receiver Interrupt %s\n",  std::array<char const *, 4>
		 {{"Disabled", "on First Character", "on All Characters, Parity Affects Vector", "on All Characters"}}[(m_wr2 >> 3) & 0x03]);

	update_wait_ready();
}

void z80sio_channel::do_sioreg_wr2(uint8_t data)
{
	m_wr2 = data;
	LOGSETUP("Z80SIO \"%s\" Channel %c : ", owner()->tag(), 'A' + m_index);
	if (m_index == 0)
	{
		LOGSETUP(" - INT/DMA priority and mode: %02x\n", m_wr2 & 0x07);
		LOGSETUP(" - Interrupt mode: %s\n", std::array<char const *, 4> {{"85-1", "85-2", "85-3", "86"}}[(m_wr2 >> 3) & 0x03]);
		LOGSETUP(" - Vector mode: %s\n", (m_wr2 & 0x20) ? "Vectored" : "Non-vectored");
		LOGSETUP(" - Rx INT mask: %d\n", (m_wr2 >> 6) & 0x01 );
		LOGSETUP(" - Pin 10: %s\n",  (m_wr2 & 0x80) ? "SYNCB" : "RTSB");
	}
	else
	{
		LOGSETUP("Interrupt Vector %02x\n", m_wr2);
	}
}

void z80sio_channel::do_sioreg_wr3(uint8_t data)
{
	LOGSETUP("Z80SIO Channel %c : Receiver Enable %u\n", 'A' + m_index, (data & WR3_RX_ENABLE) ? 1 : 0);
	LOGSETUP("Z80SIO Channel %c : Sync Character Load Inhibit %u\n", 'A' + m_index, (data & WR3_SYNC_CHAR_LOAD_INHIBIT) ? 1 : 0);
	LOGSETUP("Z80SIO Channel %c : Receive CRC Enable %u\n", 'A' + m_index, (data & WR3_RX_CRC_ENABLE) ? 1 : 0);
	LOGSETUP("Z80SIO Channel %c : Auto Enables %u\n", 'A' + m_index, (data & WR3_AUTO_ENABLES) ? 1 : 0);
	LOGSETUP("Z80SIO Channel %c : Enter Hunt Phase %u\n", 'A' + m_index, (data & WR3_ENTER_HUNT_PHASE) ? 1 : 0);
		 //if (data & WR3_ENTER_HUNT_PHASE)
		 //LOGCMD("Z80SIO Channel %c : Enter Hunt Phase\n", 'A' + m_index);

	bool const was_allowed(receive_allowed());
	m_wr3 = data;
	LOG("Z80SIO Channel %c : Receiver Bits/Character %u\n", 'A' + m_index, get_rx_word_length()); // depends on m_wr3 being updated

	if (!was_allowed && receive_allowed())
	{
		receive_enabled();
	}
	else if ((data & WR3_ENTER_HUNT_PHASE) && ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC))
	{
		// TODO: should this re-initialise hunt logic if already in hunt phase for 8-bit/16-bit/SDLC sync?
		enter_hunt_mode();
	}
}

void z80sio_channel::do_sioreg_wr4(uint8_t data)
{
	m_wr4 = data;
	LOGSETUP("Z80SIO \"%s\" Channel %c : Parity Enable %u\n", owner()->tag(), 'A' + m_index, (data & WR4_PARITY_ENABLE) ? 1 : 0);
	LOGSETUP("Z80SIO \"%s\" Channel %c : Parity %s\n", owner()->tag(), 'A' + m_index, (data & WR4_PARITY_EVEN) ? "Even" : "Odd");
	if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC)
		LOGSETUP("Z80SIO \"%s\" Channel %c : Synchronous Mode %s\n", owner()->tag(), 'A' + m_index,
			std::array<char const *, 4> {{"Monosync", "Bisync", "HDLC/SDLC", "External"}}[(m_wr4 >> 4) & 0x03]);
	else
		LOGSETUP("Z80SIO \"%s\" Channel %c : Stop Bits %g\n", owner()->tag(), 'A' + m_index, (((m_wr4 & WR4_STOP_BITS_MASK) >> 2) + 1) / 2.);
	LOGSETUP("Z80SIO \"%s\" Channel %c : Clock Mode %uX\n", owner()->tag(), 'A' + m_index, get_clock_mode());
}

void z80sio_channel::do_sioreg_wr5(uint8_t data)
{
	m_wr5 = data;
	LOGSETUP("Z80SIO Channel %c\n", 'A' + m_index);
	LOGSETUP(" - Transmitter Enable %u\n",         (data & WR5_TX_ENABLE) ? 1 : 0);
	LOGSETUP(" - Transmitter Bits/Character %u\n", get_tx_word_length());
	LOGSETUP(" - Transmit CRC Enable %u\n",        (data & WR5_TX_CRC_ENABLE) ? 1 : 0);
	LOGSETUP(" - %s Frame Check Polynomial\n",     (data & WR5_CRC16) ? "CRC-16" : "SDLC");
	LOGSETUP(" - Send Break %u\n",                 (data & WR5_SEND_BREAK) ? 1 : 0);
	LOGSETUP(" - Request to Send %u\n",            (data & WR5_RTS) ? 1 : 0);
	LOGSETUP(" - Data Terminal Ready %u\n",        (data & WR5_DTR) ? 1 : 0);

	if (~data & WR5_TX_ENABLE)
		m_uart->clear_interrupt(m_index, INT_TRANSMIT);
}

void z80sio_channel::do_sioreg_wr6(uint8_t data)
{
	LOGSETUP("Z80SIO \"%s\" Channel %c : Transmit Sync/Sync 1/SDLC Address %02x\n", owner()->tag(), 'A' + m_index, data);
	m_wr6 = data;
}

void z80sio_channel::do_sioreg_wr7(uint8_t data)
{
	LOGSETUP("Z80SIO \"%s\" Channel %c : Receive Sync/Sync 2/SDLC Flag %02x\n", owner()->tag(), 'A' + m_index, data);
	m_wr7 = data;
}

//-------------------------------------------------
//  control_write - write control register
//-------------------------------------------------
void z80sio_channel::control_write(uint8_t data)
{
	uint8_t   reg = m_wr0 & WR0_REGISTER_MASK;

	if (reg != 0)
	{
		LOGSETUP(" * %s %c Reg %02x <- %02x - %s\n", tag(), 'A' + m_index, reg, data, std::array<char const *, 8>
			 {{"WR0", "WR1", "WR2", "WR3", "WR4", "WR5", "WR6", "WR7"}}[reg]);
		// mask out register index
		m_wr0 &= ~WR0_REGISTER_MASK;
	}

	LOG("%s(%02x) reg %02x\n", FUNCNAME, data, reg);

	switch (reg)
	{
	case REG_WR0_COMMAND_REGPT:     do_sioreg_wr0(data); break;
	case REG_WR1_INT_DMA_ENABLE:    do_sioreg_wr1(data); m_uart->check_interrupts(); break;
	case REG_WR2_INT_VECTOR:        do_sioreg_wr2(data); break;
	case REG_WR3_RX_CONTROL:        do_sioreg_wr3(data); break;
	case REG_WR4_RX_TX_MODES:       do_sioreg_wr4(data); update_dtr_rts_break(); break;
	case REG_WR5_TX_CONTROL:        do_sioreg_wr5(data); update_dtr_rts_break(); transmit_enable(); break;
	case REG_WR6_SYNC_OR_SDLC_A:    do_sioreg_wr6(data); break;
	case REG_WR7_SYNC_OR_SDLC_F:    do_sioreg_wr7(data); break;
	default:
		logerror("Z80SIO \"%s\" Channel %c : Unsupported WRx register:%02x\n", owner()->tag(), 'A' + m_index, reg);
	}
}


//-------------------------------------------------
//  data_read - read data register
//-------------------------------------------------
uint8_t z80sio_channel::data_read()
{
	uint8_t const data = uint8_t(m_rx_data_fifo & 0x000000ffU);

	if (!machine().side_effects_disabled())
	{
		// framing and overrun errors need to be cleared to advance the FIFO in interrupt-on-first mode
		// TODO: Intel 8274 manual doesn't mention this behaviour - is it specific to Z80 SIO?
		if ((WR1_RX_INT_FIRST != (m_wr1 & WR1_RX_INT_MODE_MASK)) || !(m_rr1 & (RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR)))
			advance_rx_fifo();

		LOG("Z80SIO \"%s\" Channel %c : Data Register Read '%02x'\n", owner()->tag(), 'A' + m_index, data);
	}

	return data;
}


//-------------------------------------------------
//  data_write - write data register
//-------------------------------------------------
void z80sio_channel::data_write(uint8_t data)
{
	if (!(m_rr0 & RR0_TX_BUFFER_EMPTY))
		LOGTX("Z80SIO \"%s\" Channel %c : Dropped Data Byte '%02x'\n", owner()->tag(), 'A' + m_index, m_tx_data);
	LOGTX("Z80SIO Channel %c : Queue Data Byte '%02x'\n", 'A' + m_index, data);

	// fill transmit buffer
	m_tx_data = data;
	set_tx_empty(get_tx_empty() , false);
	if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC)
	{
		LOGTX("Z80SIO: WR4_STOP_BITS_SYNC detected\n");
		m_tx_in_pkt = true;
	}
	else
	{
		// ALL_SENT is only meaningful in async mode, in sync mode it's always 1
		LOGTX("Z80SIO: WR4_STOP_BITS_SYNC *not* detected\n");
		m_rr1 &= ~RR1_ALL_SENT;
		m_all_sent_delay = 0;
	}

	bool const async((m_wr4 & WR4_STOP_BITS_MASK) != WR4_STOP_BITS_SYNC);

	// clear transmit interrupt
	m_uart->clear_interrupt(m_index, INT_TRANSMIT);

	// may be possible to transmit immediately (synchronous mode will load when sync pattern completes)
	if (async && is_tx_idle() && transmit_allowed())
		async_tx_setup();
}


//-------------------------------------------------
//  advance_rx_fifo - move to next received byte
//-------------------------------------------------
void z80sio_channel::advance_rx_fifo()
{
	if (m_rx_fifo_depth)
	{
		if (--m_rx_fifo_depth)
		{
			// shift the FIFO
			m_rx_data_fifo >>= 8;
			m_rx_error_fifo >>= 8;

			// load error status from the FIFO
			m_rr1 = (m_rr1 & ~m_rr1_auto_reset) | uint8_t(m_rx_error_fifo & 0x000000ffU & ~RR1_HIDDEN_1ST_MARKER);
		}
		else
		{
			// no more characters available in the FIFO
			m_rr0 &= ~RR0_RX_CHAR_AVAILABLE;
			update_wait_ready();
		}
	}
	update_rx_int();
}

uint8_t z80sio_channel::get_special_rx_mask() const
{
	if ((m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_DISABLE)
	{
		return 0;
	}
	else
	{
		uint8_t mask = ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC) ?
			(RR1_RX_OVERRUN_ERROR | RR1_END_OF_FRAME) :
			(RR1_RX_OVERRUN_ERROR | RR1_CRC_FRAMING_ERROR);
		if ((m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_ALL_PARITY)
			mask |= RR1_PARITY_ERROR;
		return mask;
	}
}

void z80sio_channel::update_rx_int()
{
	bool state = false;

	auto rx_int_mode = m_wr1 & WR1_RX_INT_MODE_MASK;
	if (rx_int_mode != WR1_RX_INT_DISABLE)
	{
		if (m_rr1 & get_special_rx_mask())
			state = true;
		else if (m_rx_fifo_depth)
		{
			// FIFO not empty
			if (rx_int_mode != WR1_RX_INT_FIRST)
				state = true;
			else if (m_rx_error_fifo & RR1_HIDDEN_1ST_MARKER)
				state = true;
		}
	}
	LOGINT("rx %d wr1 %02x rr1 %02x fd %u ref %06x\n", state, m_wr1, m_rr1, m_rx_fifo_depth, m_rx_error_fifo);
	if (state)
		m_uart->trigger_interrupt(m_index, INT_RECEIVE);
	else
		m_uart->clear_interrupt(m_index, INT_RECEIVE);
}

//-------------------------------------------------
//  receive_enabled - conditions have changed
//  allowing reception to begin
//-------------------------------------------------

void z80sio_channel::receive_enabled()
{
	bool const sync_mode((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC);
	m_rx_count = sync_mode ? 0 : ((get_clock_mode() - 1) / 2);
	m_rx_bit = 0;
	if (sync_mode)
		enter_hunt_mode();
}

void z80sio_channel::enter_hunt_mode()
{
	if (!(m_rr0 & RR0_SYNC_HUNT))
	{
		m_rx_sync_fsm = SYNC_FSM_HUNT;
		m_rr0 |= RR0_SYNC_HUNT;
		trigger_ext_int();
	}
}

void z80dart_channel::enter_hunt_mode()
{
	LOG("%s (sync mode not supported by DART)\n", FUNCNAME);
}

//-------------------------------------------------
//  sync_receive - synchronous reception handler
//-------------------------------------------------
void z80sio_channel::sync_receive()
{
	LOGBIT("%.6f Channel %c Sync Received Bit %d, sync=%02x, sr=%03x, crc_dly=%02x, crc=%04x, FSM=%d, bit=%d, limit=%d\n" , machine().time().as_double(), 'A' + m_index, m_dlyd_rxd, m_rx_sync_sr, m_rx_sr, m_rx_crc_delay, m_rx_crc, m_rx_sync_fsm, m_rx_bit, m_rx_bit_limit);

	bool sync_sr_out = BIT(m_rx_sync_sr , 0);
	m_rx_sync_sr = (m_rx_sync_sr >> 1) & 0x7f;
	if (m_dlyd_rxd)
		m_rx_sync_sr |= 0x80;

	bool wr7_matched = m_rx_sync_sr == m_wr7;

	switch (m_rx_sync_fsm)
	{
	case SYNC_FSM_HUNT:
	{
		bool got_sync = false;
		switch (m_wr4 & WR4_SYNC_MODE_MASK)
		{
		case WR4_SYNC_MODE_8_BIT:
			if (wr7_matched)
			{
				LOGRCV("Channel %c 8-bit Sync Acquired\n", 'A' + m_index);
				got_sync = true;
			}
			break;

		case WR4_SYNC_MODE_16_BIT:
		{
			m_rx_sr = (m_rx_sr >> 1) & 0x7f;
			if (sync_sr_out)
				m_rx_sr |= 0x80;
			if ((m_rx_sr & 0xff) == m_wr6 && wr7_matched)
			{
				LOGRCV("Channel %c 16-bit Sync Acquired\n", 'A' + m_index);
				got_sync = true;
			}
			break;
		}

		case WR4_SYNC_MODE_EXT:
			// Not entirely correct: sync input is synchronized 2 bits off in the real hw
			got_sync = m_sync;
			break;

		default:
			break;
		}
		if (got_sync)
		{
			if (m_rr0 & RR0_SYNC_HUNT)
			{
				m_rr0 &= ~RR0_SYNC_HUNT;
				trigger_ext_int();
			}
			m_rx_sync_fsm = SYNC_FSM_1ST_CHAR;
			m_rx_crc_en = false;
			m_rx_bit = 0;
			m_rx_bit_limit = get_rx_word_length() + ((m_wr4 & WR4_PARITY_ENABLE) ? 1 : 0);
			m_rx_parity = false;
		}
	}
	break;

	case SYNC_FSM_1ST_CHAR:
	case SYNC_FSM_IN_FRAME:
	{
		bool rx_sr_out = BIT(m_rx_sr , 0);
		bool rx_crc_delay_out = BIT(m_rx_crc_delay , 0);
		m_rx_crc_delay = (m_rx_crc_delay >> 1);
		if (rx_sr_out)
			m_rx_crc_delay |= 0x80;
		if (m_rx_crc_en)
			update_crc(m_rx_crc , rx_crc_delay_out);
		m_rx_sr = (m_rx_sr >> 1) & ((1U << (m_rx_bit_limit - 1)) - 1);
		if (m_dlyd_rxd)
		{
			m_rx_sr |= (1U << (m_rx_bit_limit - 1));
			m_rx_parity = !m_rx_parity;
		}
		if (++m_rx_bit == m_rx_bit_limit)
		{
			if (!(m_wr3 & WR3_SYNC_CHAR_LOAD_INHIBIT) ||
				((m_rx_sr & 0xff) != m_wr6 && !wr7_matched))
			{
				uint8_t status_byte = 0;
				if (m_rx_crc != 0)
					status_byte |= RR1_CRC_FRAMING_ERROR;
				if (m_wr4 & WR4_PARITY_EVEN)
					m_rx_parity = !m_rx_parity;
				if (!m_rx_parity && (m_wr4 & WR4_PARITY_ENABLE))
					status_byte |= RR1_PARITY_ERROR;
				uint8_t data = m_rx_sr & 0xff;
				if (m_rx_bit_limit < 8)
					// Fill the unused part of character with ones
					data |= ~((1U << m_rx_bit_limit) - 1);
				queue_received(data , status_byte);
			}
			m_rx_bit = 0;
			m_rx_bit_limit = get_rx_word_length() + ((m_wr4 & WR4_PARITY_ENABLE) ? 1 : 0);
			m_rx_parity = false;
			m_rx_crc_en = (m_rx_sync_fsm == SYNC_FSM_IN_FRAME) && (m_wr3 & WR3_RX_CRC_ENABLE);
			m_rx_sync_fsm = SYNC_FSM_IN_FRAME;
		}
		break;
	}

	default:
		LOG("Invalid Sync FSM state (%d)\n" , m_rx_sync_fsm);
		m_rx_sync_fsm = SYNC_FSM_HUNT;
	}

	m_dlyd_rxd = m_rxd;
}

void z80dart_channel::sync_receive()
{
	LOG("%s (sync mode not supported by DART)\n", FUNCNAME);
}

//-------------------------------------------------
//  sdlc_receive - SDLC reception handler
//-------------------------------------------------
void z80sio_channel::sdlc_receive()
{
	LOGBIT("Channel %c SDLC Received Bit %d, sync=%02x, sr=%03x, FSM=%d, bit=%d, limit=%d\n", 'A' + m_index, m_rxd, m_rx_sync_sr, m_rx_sr, m_rx_sync_fsm, m_rx_bit, m_rx_bit_limit);

	// Check for flag
	bool flag_matched = m_rx_sync_sr == m_wr7;

	// Shift RxD into sync SR
	bool sync_sr_out = BIT(m_rx_sync_sr , 0);
	m_rx_sync_sr >>= 1;
	if (m_rxd)
		m_rx_sync_sr |= 0x80;

	// Zero deletion & abort detection
	bool zero_deleted = false;
	if (sync_sr_out)
	{
		m_rx_sr = (m_rx_sr >> 1) | (1U << 10);
		if (m_rx_one_cnt < 7 && ++m_rx_one_cnt == 7)
		{
			LOGRCV("SDLC Abort detected\n");
			m_rr0 |= RR0_BREAK_ABORT;
			if (!m_brk_latched) {
				m_brk_latched = true;
				trigger_ext_int();
			}
			enter_hunt_mode();
		}
	}
	else if (m_rx_one_cnt == 5)
	{
		m_rx_one_cnt = 0;
		// Ignore the zero
		zero_deleted = true;
	}
	else
	{
		m_rx_sr >>= 1;
		m_rx_one_cnt = 0;
		if (m_rr0 & RR0_BREAK_ABORT)
		{
			m_rr0 &= ~RR0_BREAK_ABORT;
			if (!m_brk_latched) {
				m_brk_latched = true;
				trigger_ext_int();
			}
		}
	}

	switch (m_rx_sync_fsm)
	{
	case SYNC_FSM_HUNT:
	case SYNC_FSM_EVICT:
		if (flag_matched)
		{
			// Got sync
			m_rx_sync_fsm = SYNC_FSM_EVICT;
			m_rx_bit = 0;
			m_rx_bit_limit = 7;
			LOGRCV("Channel %c SDLC Sync Acquired\n", 'A' + m_index);
			if (m_rr0 & RR0_SYNC_HUNT)
			{
				m_rr0 &= ~RR0_SYNC_HUNT;
				trigger_ext_int();
			}
		}
		else if (m_rx_sync_fsm == SYNC_FSM_EVICT && ++m_rx_bit == m_rx_bit_limit)
		{
			m_rx_sync_fsm = SYNC_FSM_1ST_CHAR;
			m_rx_crc = ~0;
			m_rx_bit = 0;
			m_rx_bit_limit = 11;
		}
		break;

	case SYNC_FSM_1ST_CHAR:
	case SYNC_FSM_IN_FRAME:
		if (zero_deleted)
			break;
		if (++m_rx_bit == m_rx_bit_limit)
			m_rx_bit = 0;
		if (flag_matched)
		{
			// Got closing flag
			if (m_rx_sync_fsm != SYNC_FSM_1ST_CHAR)
			{
				// Frame ended
				LOGRCV("SDLC frame ended, CRC=%04x, residual=%d\n" , m_rx_crc , m_rx_bit);
				uint8_t status_byte = RR1_END_OF_FRAME;
				if (m_rx_crc != SDLC_RESIDUAL)
					status_byte |= RR1_CRC_FRAMING_ERROR;
				// The residue code is nothing but the bit-reversed accumulated bit count
				if (BIT(m_rx_bit , 0))
					status_byte |= 0x08;
				if (BIT(m_rx_bit , 1))
					status_byte |= 0x04;
				if (BIT(m_rx_bit , 2))
					status_byte |= 0x02;
				// Is the last character masked according to rx word length?
				// We don't mask it here, after all it just holds a (useless) part of CRC
				queue_received(m_rx_sr & 0xff , status_byte);
			}
			// else: frame ended before 11 bits are received, discard it
			m_rx_sync_fsm = SYNC_FSM_EVICT;
			m_rx_bit = 0;
			m_rx_bit_limit = 7;
		}
		else
		{
			// Update rx CRC
			update_crc(m_rx_crc , sync_sr_out);
			LOGBIT("SDLC CRC=%04x/%d\n" , m_rx_crc , sync_sr_out);

			if (m_rx_bit == 0)
			{
				// Check for address byte
				if (m_rx_sync_fsm == SYNC_FSM_1ST_CHAR && (m_wr3 & WR3_ADDRESS_SEARCH_MODE) &&
					(m_rx_sr & 0xff) != 0xff && (m_rx_sr & 0xff) != m_wr6)
				{
					LOGRCV("Channel %c SDLC Address %02x not matching %02x\n" , 'A' + m_index , m_rx_sr & 0xff , m_wr6);
					// Address not matching, ignore this frame
					m_rx_sync_fsm = SYNC_FSM_HUNT;
				}
				else
				{
					m_rx_bit_limit = get_rx_word_length();
					uint8_t data = m_rx_sr & 0xff;
					if (m_rx_bit_limit != 8)
						// Fill the unused part of character with ones
						data |= ~((1U << m_rx_bit_limit) - 1);
					LOGRCV("SDLC rx data=%02x (%d bits)\n" , data , m_rx_bit_limit);
					queue_received(data , 0);
					if (m_rx_sync_fsm == SYNC_FSM_1ST_CHAR)
					{
						// reception of 1st char clears END-OF-FRAME
						m_rr1 &= ~RR1_END_OF_FRAME;
						update_rx_int();
						m_rx_sync_fsm = SYNC_FSM_IN_FRAME;
					}
				}
			}
		}
		break;

	default:
		LOG("Invalid SDLC FSM state (%d)\n" , m_rx_sync_fsm);
		m_rx_sync_fsm = SYNC_FSM_HUNT;
	}
}

void z80dart_channel::sdlc_receive()
{
	logerror("%s (sync mode not supported by DART)\n", FUNCNAME);
}

//-------------------------------------------------
//  receive_data - receive data word
//-------------------------------------------------

void z80sio_channel::receive_data()
{
}

//-------------------------------------------------
//  queue_recevied - queue recevied character
//-------------------------------------------------

void z80sio_channel::queue_received(uint16_t data, uint32_t error)
{
	if ((m_wr1 & WR1_RX_INT_MODE_MASK) == WR1_RX_INT_FIRST && m_rx_first)
	{
		// insert a hidden marker for 1st received character
		error |= RR1_HIDDEN_1ST_MARKER;
		m_rx_first = false;
	}

	if (3 == m_rx_fifo_depth)
	{
		LOG("  Receive FIFO overrun detected\n");
		// receive overrun error detected
		error |= RR1_RX_OVERRUN_ERROR;

		m_rx_data_fifo = (m_rx_data_fifo & 0x0000ffffU) | (uint32_t(data & 0x00ffU) << 16);
		m_rx_error_fifo = (m_rx_error_fifo & 0x0000ffffU) | (error << 16);
	}
	else
	{
		// store received character and error status into FIFO
		if (!m_rx_fifo_depth)
			m_rx_data_fifo = m_rx_error_fifo = 0U;
		m_rx_data_fifo |= uint32_t(data & 0x00ffU) << (8 * m_rx_fifo_depth);
		m_rx_error_fifo |= error << (8 * m_rx_fifo_depth);
		if (!m_rx_fifo_depth)
			m_rr1 = (m_rr1 & ~m_rr1_auto_reset) | uint8_t(error & ~RR1_HIDDEN_1ST_MARKER);
		++m_rx_fifo_depth;
	}

	m_rr0 |= RR0_RX_CHAR_AVAILABLE;
	update_wait_ready();

	// receive interrupt
	update_rx_int();
}


//-------------------------------------------------
//  cts_w - clear to send handler
//-------------------------------------------------
void z80sio_channel::cts_w(int state)
{
	if (bool(m_cts) != bool(state))
	{
		LOGCTS("Z80SIO Channel %c : CTS %u\n", 'A' + m_index, state);

		m_cts = state;
		trigger_ext_int();

		// this may enable/disable transmission
		transmit_enable();
	}
}


//-------------------------------------------------
//  dcd_w - data carrier detected handler
//-------------------------------------------------
void z80sio_channel::dcd_w(int state)
{
	if (bool(m_dcd) != bool(state))
	{
		LOGDCD("Z80SIO Channel %c : DCD %u\n", 'A' + m_index, state);

		bool const was_allowed(receive_allowed());
		m_dcd = state;
		trigger_ext_int();

		// in auto-enable mode, this can start the receiver
		if (!was_allowed && receive_allowed())
			receive_enabled();
	}
}


//-------------------------------------------------
//  sh_w - Sync Hunt handler
//-------------------------------------------------
void z80sio_channel::sync_w(int state)
{
	if (bool(m_sync) != bool(state))
	{
		LOGSYNC("Z80SIO Channel %c : Sync %u\n", 'A' + m_index, state);

		m_sync = state;

		// sync is a general-purpose input in asynchronous mode
		if ((m_wr4 & WR4_STOP_BITS_MASK) != WR4_STOP_BITS_SYNC)
			trigger_ext_int();
	}
}


//-------------------------------------------------
//  rxc_w - receive clock
//-------------------------------------------------
void z80sio_channel::rxc_w(int state)
{
	//LOG("Z80SIO \"%s\" Channel %c : Receiver Clock Pulse\n", owner()->tag(), m_index + 'A');
	//if ((receive_allowed() || m_rx_bit != 0) && state && !m_rx_clock)
	if (receive_allowed() && state && !m_rx_clock)
	{
		// RxD sampled on rising edge
		int const clocks = get_clock_mode() - 1;

		if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC)
		{
			// synchronous receive is a different beast
			if (!m_rx_count)
			{
				if ((m_wr4 & WR4_SYNC_MODE_MASK) == WR4_SYNC_MODE_SDLC)
					sdlc_receive();
				else
					sync_receive();
				m_rx_count = clocks;
			}
			else
			{
				--m_rx_count;
			}
		}
		else if (!(m_rr0 & RR0_BREAK_ABORT) || m_rxd)
		{
			// break termination detection
			if ((m_rr0 & RR0_BREAK_ABORT) && m_rxd)
			{
				LOGRCV("Break termination detected\n");
				m_rr0 &= ~RR0_BREAK_ABORT;
				if (!m_brk_latched) {
					m_brk_latched = true;
					trigger_ext_int();
				}
			}
			if (!m_rx_bit)
			{
				// look for start bit
				if (m_rxd)
				{
					// line idle
					m_rx_count = (std::max)(m_rx_count, (clocks / 2) + 1) - 1;
				}
				else if (!m_rx_count)
				{
					// half a bit period expired, start shifting bits
					m_rx_count = clocks;
					++m_rx_bit;
					m_rx_sr = ~uint16_t(0U);
				}
				else
				{
					// ensure start bit lasts long enough
					--m_rx_count;
				}
			}
			else if (!m_rx_count)
			{
				// sample a data/parity/stop bit
				if (!m_rxd)
					m_rx_sr &= ~uint16_t(1U << (m_rx_bit - 1));
				int const word_length(get_rx_word_length() + ((m_wr4 & WR4_PARITY_ENABLE) ? 1 : 0));
				bool const stop_reached((word_length + 1) == m_rx_bit);
				LOGBIT("%s() Channel %c Received %s Bit %d\n", FUNCNAME, 'A' + m_index, stop_reached ? "Stop" : "Data", m_rxd);

				if (stop_reached)
				{
					// this is the stop bit - framing error adds a half bit period
					m_rx_count = m_rxd ? (clocks / 2) : clocks;
					m_rx_bit = 0;

					LOGRCV("%s() Channel %c Received Data %02x\n", FUNCNAME, 'A' + m_index, m_rx_sr & 0xff);

					// check framing errors and break condition
					uint16_t const stop_bit = uint16_t(1U) << word_length;
					bool const brk(!(m_rx_sr & ((stop_bit << 1) - 1)));
					uint8_t error = brk || (m_rx_sr & stop_bit) ? 0U : RR1_CRC_FRAMING_ERROR;
					if (m_wr4 & WR4_PARITY_ENABLE)
					{
						int const word_length = get_rx_word_length();
						uint16_t par(m_rx_sr);
						for (int i = 1; word_length >= i; ++i)
							par ^= BIT(par, i);

						if (bool(BIT(par, 0)) == bool(m_wr4 & WR4_PARITY_EVEN))
						{
							LOGRCV("  Parity error detected\n");
							error |= RR1_PARITY_ERROR;
						}
					}

					queue_received(m_rx_sr | stop_bit, error);

					// break interrupt
					if (brk && !m_brk_latched && !(m_rr0 & RR0_BREAK_ABORT))
					{
						LOGRCV("Break detected\n");
						m_rr0 |= RR0_BREAK_ABORT;
						m_brk_latched = true;
						trigger_ext_int();
					}
				}
				else
				{
					// wait a whole bit period for the next bit
					m_rx_count = clocks;
					++m_rx_bit;
				}
			}
			else
			{
				// bit period hasn't expired
				--m_rx_count;
			}
		}
	}
	m_rx_clock = state;
}


//-------------------------------------------------
//  txc_w - transmit clock
//-------------------------------------------------
void z80sio_channel::txc_w(int state)
{
	//LOG("Z80SIO \"%s\" Channel %c : Transmitter Clock Pulse\n", owner()->tag(), m_index + 'A');
	if (!state && m_tx_clock)
	{
		// falling edge active
		if (m_tx_count == 0)
		{
			// x1 clock
			m_tx_phase = true;
			// Shift delay by a half bit and duplicate last input bit
			m_tx_delay = (m_tx_delay << 1) | (m_tx_delay & 1);
		}
		else
			m_tx_count--;
		if (m_tx_count == 0)
		{
			m_tx_phase = !m_tx_phase;
			// Load delay for half bit
			m_tx_count = get_clock_mode() / 2;
			// Send out a delayed half bit
			bool new_txd = BIT(m_tx_delay , 3);
			if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_SYNC || !(m_rr1 & RR1_ALL_SENT))
				LOGBIT("%.6f TX %d DLY %x\n" , machine().time().as_double() , new_txd , m_tx_delay & 0xf);
			if (new_txd != m_txd && !(m_wr5 & WR5_SEND_BREAK))
			{
				out_txd_cb(new_txd);
			}
			m_txd = new_txd;
			// Check for ALL SENT condition
			if (!(m_rr1 & RR1_ALL_SENT) && BIT(m_all_sent_delay , 3))
			{
				LOGBIT("%.6f ALL_SENT\n" , machine().time().as_double());
				m_rr1 |= RR1_ALL_SENT;
				if (!(m_wr5 & WR5_RTS))
					set_rts(1);
			}
			// Shift delay by a half bit and duplicate last input bit
			// When m_tx_phase is false, LSB is replaced by new bit (see below)
			m_tx_delay = (m_tx_delay << 1) | (m_tx_delay & 1);
			m_all_sent_delay <<= 1;
			if (!m_tx_phase)
			{
				// Generate a new bit
				bool new_bit = false;
				if ((m_wr4 & (WR4_SYNC_MODE_MASK | WR4_STOP_BITS_MASK)) == (WR4_SYNC_MODE_SDLC | WR4_STOP_BITS_SYNC) &&
						(m_tx_flags & (TX_FLAG_DATA_TX | TX_FLAG_CRC_TX)) && (m_tx_hist & 0x1f) == 0x1f)
				{
					// SDLC, sending data/CRC & 5 ones in a row: do zero insertion
					new_bit = false;
				}
				else
				{
					bool get_out = false;
					while (!get_out)
					{
						// Pattern for parity bit in SR?
						// 17x 1 || 000
						if ((m_tx_sr & TX_SR_MASK) == 0xffff8)
						{
							if ((m_wr4 & WR4_PARITY_ENABLE) != 0 &&
								(m_tx_flags & TX_FLAG_DATA_TX))
							{
								new_bit = m_tx_parity;
								if (!(m_wr4 & WR4_PARITY_EVEN))
									new_bit = !new_bit;
								get_out = true;
							}
						}
						// Pattern for 1st stop bit?
						// 18x 1 || 00
						else if ((m_tx_sr & TX_SR_MASK) == 0xffffc)
						{
							if ((m_wr4 & WR4_STOP_BITS_MASK) != WR4_STOP_BITS_SYNC)
							{
								new_bit = true;
								get_out = true;
							}
						}
						// Pattern for 2nd stop bit?
						// 19x 1 || 0
						else if ((m_tx_sr & TX_SR_MASK) == 0xffffe)
						{
							if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_1_5 ||
								(m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_2)
							{
								new_bit = true;
								if ((m_wr4 & WR4_STOP_BITS_MASK) == WR4_STOP_BITS_1_5)
									// Force current stop bit to last for 1/2 bit time
									m_tx_phase = true;
								get_out = true;
							}
						}
						// Pattern for idle tx?
						// 20x 1
						else if (is_tx_idle())
						{
							transmit_complete();
							if (is_tx_idle())
							{
								new_bit = true;
								get_out = true;
							}
							else
								continue;
						}
						else if (m_tx_flags & TX_FLAG_CRC_TX)
						{
							// CRC bits (from MSB to LSB)
							new_bit = BIT(m_tx_crc , 15);
							m_tx_crc <<= 1;
							if ((m_wr4 & WR4_SYNC_MODE_MASK) == WR4_SYNC_MODE_SDLC)
								m_tx_crc |= 1;
							get_out = true;
						}
						else
						{
							// Start bit or data bits
							new_bit = BIT(m_tx_sr , 0);
							// Update parity
							if (new_bit)
								m_tx_parity = !m_tx_parity;
							// Update CRC
							if (m_tx_flags & TX_FLAG_CRC)
							{
								LOGBIT("CRC %04x/%d\n" , m_tx_crc , new_bit);
								update_crc(m_tx_crc , new_bit);
							}
							get_out = true;
						}
						// Shift right 1 bit && insert 1 at MSB
						m_tx_sr = (m_tx_sr >> 1) | 0x80000;
					}
					if ((m_wr4 & WR4_STOP_BITS_MASK) != WR4_STOP_BITS_SYNC && is_tx_idle() && (m_rr0 & RR0_TX_BUFFER_EMPTY))
						m_all_sent_delay |= 1U;
					else
						m_all_sent_delay = 0;
				}
				if (m_tx_flags & (TX_FLAG_DATA_TX | TX_FLAG_CRC_TX))
					m_tx_hist = (m_tx_hist << 1) | new_bit;
				else
					m_tx_hist = 0;
				// Insert new bit in delay register
				m_tx_delay = (m_tx_delay & ~1U) | new_bit;
			}
		}
	}
	m_tx_clock = state;
}

//**************************************************************************
//  MK68564 REGISTER INTERFACE
//**************************************************************************

//-------------------------------------------------
//  cmdreg_r - read from command register
//-------------------------------------------------
uint8_t mk68564_channel::cmdreg_r()
{
	return m_wr0;
}


//-------------------------------------------------
//  cmdreg_w - write to command register
//-------------------------------------------------
void mk68564_channel::cmdreg_w(uint8_t data)
{
	// TODO: bit 0 sets loop mode (no register select)
	// FIXME: no return from interrupt command
	do_sioreg_wr0(data);
}


//-------------------------------------------------
//  modectl_r - read from mode control register
//-------------------------------------------------
uint8_t mk68564_channel::modectl_r()
{
	return m_wr4;
}


//-------------------------------------------------
//  modectl_w - write to mode control register
//-------------------------------------------------
void mk68564_channel::modectl_w(uint8_t data)
{
	do_sioreg_wr4(data);
}


//-------------------------------------------------
//  intctl_r - read from interrupt control register
//-------------------------------------------------
uint8_t mk68564_channel::intctl_r()
{
	return m_wr1 | (m_wr5 & WR5_CRC16 ? 0x80 : 0);
}


//-------------------------------------------------
//  intctl_w - write to interrupt control register
//-------------------------------------------------
void mk68564_channel::intctl_w(uint8_t data)
{
	if (BIT(data, 7))
		m_wr5 |= WR5_CRC16;
	else
		m_wr5 &= ~WR5_CRC16;

	// TODO: bits 5 and 6 are independent RxRDY and WxRDY enables
	do_sioreg_wr1(data & 0x7f);
}


//-------------------------------------------------
//  sync1_r - read from sync word register 1
//-------------------------------------------------
uint8_t mk68564_channel::sync1_r()
{
	return m_wr6;
}


//-------------------------------------------------
//  sync1_w - write to sync word register 1
//-------------------------------------------------
void mk68564_channel::sync1_w(uint8_t data)
{
	do_sioreg_wr6(data);
}


//-------------------------------------------------
//  sync2_r - read from sync word register 2
//-------------------------------------------------
uint8_t mk68564_channel::sync2_r()
{
	return m_wr7;
}


//-------------------------------------------------
//  sync2_w - write to sync word register 2
//-------------------------------------------------
void mk68564_channel::sync2_w(uint8_t data)
{
	do_sioreg_wr7(data);
}


//-------------------------------------------------
//  rcvctl_r - read from receiver control register
//-------------------------------------------------
uint8_t mk68564_channel::rcvctl_r()
{
	return bitswap<8>(m_wr3, 6, 7, 5, 4, 3, 2, 1, 0) & ~WR3_ENTER_HUNT_PHASE;
}


//-------------------------------------------------
//  rcvctl_w - write to receiver control register
//-------------------------------------------------
void mk68564_channel::rcvctl_w(uint8_t data)
{
	do_sioreg_wr3(bitswap<8>(data, 6, 7, 5, 4, 3, 2, 1, 0));
}


//-------------------------------------------------
//  xmtctl_r - read from transmitter control
//  register
//-------------------------------------------------
uint8_t mk68564_channel::xmtctl_r()
{
	uint8_t xmtctl = 0;
	if (m_wr5 & WR5_TX_ENABLE)
		xmtctl |= 0x01;
	if (m_wr5 & WR5_RTS)
		xmtctl |= 0x02;
	if (m_wr5 & WR5_DTR)
		xmtctl |= 0x04;
	if (m_wr5 & WR5_TX_CRC_ENABLE)
		xmtctl |= 0x08;
	if (m_wr5 & WR5_SEND_BREAK)
		xmtctl |= 0x10;
	if (m_tx_auto_enable)
		xmtctl |= 0x20;
	xmtctl |= (m_wr5 & 0x40) << 1;
	xmtctl |= (m_wr5 & 0x80) >> 1;
	return xmtctl;
}


//-------------------------------------------------
//  xmtctl_w - write to transmitter control
//  register
//-------------------------------------------------
void mk68564_channel::xmtctl_w(uint8_t data)
{
	uint8_t control =
		(BIT(data, 0) ? WR5_TX_ENABLE : 0) |
		(BIT(data, 1) ? WR5_RTS : 0) |
		(BIT(data, 2) ? WR5_DTR : 0) |
		(BIT(data, 3) ? WR5_TX_CRC_ENABLE : 0) |
		(BIT(data, 4) ? WR5_SEND_BREAK : 0) |
		(data & 0x40) << 1 |
		(data & 0x80) >> 1 |
		(m_wr5 & WR5_CRC16);
	do_sioreg_wr5(control);

	m_tx_auto_enable = BIT(data, 5);
}


//-------------------------------------------------
//  tcreg_r - read from time constant register
//-------------------------------------------------
uint8_t mk68564_channel::tcreg_r()
{
	return m_brg_tc;
}


//-------------------------------------------------
//  tcreg_w - write to time constant register
//-------------------------------------------------
void mk68564_channel::tcreg_w(uint8_t data)
{
	m_brg_tc = data;
}


//-------------------------------------------------
//  brgctl_r - read from baud rate generator
//  control register
//-------------------------------------------------
uint8_t mk68564_channel::brgctl_r()
{
	// unused bits are all zero
	return m_brg_control & 0x0f;
}


//-------------------------------------------------
//  brgctl_w - write to baud rate generator
//  control register
//-------------------------------------------------
void mk68564_channel::brgctl_w(uint8_t data)
{
	if (BIT(data, 0))
		LOGBRG("%s: BRG enabled, divide by %d, RxC %sternal, TxC %sternal (TC = %d, %.1f Hz)\n",
			machine().describe_context(),
			BIT(data, 1) ? 64 : 4,
			BIT(data, 2) ? "in" : "ex",
			BIT(data, 3) ? "in" : "ex",
			m_brg_tc,
			clocks_to_attotime((m_brg_tc ? m_brg_tc : 256) * (BIT(data, 1) ? 64 : 4)).as_hz());
	else
		LOGBRG("%s: BRG disabled\n", machine().describe_context());

	m_brg_control = data & 0x0f;
	m_brg_state = false;
	brg_update();
}


//-------------------------------------------------
//  vectrg_w - write to the interrupt vector
//  register (only one exists)
//-------------------------------------------------
void mk68564_device::vectrg_w(uint8_t data)
{
	m_chanB->do_sioreg_wr2(data);
}


//-------------------------------------------------
//  read - 68000 compatible bus read
//-------------------------------------------------
uint8_t mk68564_device::read(offs_t offset)
{
	mk68564_channel &channel = downcast<mk68564_channel &>(BIT(offset, 4) ? *m_chanB : *m_chanA);

	switch (offset & 0x0f)
	{
	case 0x00:
		return channel.cmdreg_r();

	case 0x01:
		return channel.modectl_r();

	case 0x02:
		return channel.intctl_r();

	case 0x03:
		return channel.sync1_r();

	case 0x04:
		return channel.sync2_r();

	case 0x05:
		return channel.rcvctl_r();

	case 0x06:
		return channel.xmtctl_r();

	case 0x07:
		return channel.do_sioreg_rr0();

	case 0x08:
		return channel.do_sioreg_rr1();

	case 0x09:
		return channel.data_read();

	case 0x0a:
		return channel.tcreg_r();

	case 0x0b:
		return channel.brgctl_r();

	case 0x0c: // vector register is addressable through either channel
		return read_vector();

	default: // unused registers read as FF
		return 0xff;
	}
}


//-------------------------------------------------
//  write - 68000 compatible bus write
//-------------------------------------------------
void mk68564_device::write(offs_t offset, uint8_t data)
{
	mk68564_channel &channel = downcast<mk68564_channel &>(BIT(offset, 4) ? *m_chanB : *m_chanA);

	switch (offset & 0x0f)
	{
	case 0x00:
		channel.cmdreg_w(data);
		break;

	case 0x01:
		channel.modectl_w(data);
		break;

	case 0x02:
		channel.intctl_w(data);
		break;

	case 0x03:
		channel.sync1_w(data);
		break;

	case 0x04:
		channel.sync2_w(data);
		break;

	case 0x05:
		channel.rcvctl_w(data);
		break;

	case 0x06:
		channel.xmtctl_w(data);
		break;

	case 0x09:
		channel.data_write(data);
		break;

	case 0x0a:
		channel.tcreg_w(data);
		break;

	case 0x0b:
		channel.brgctl_w(data);
		break;

	case 0x0c: // vector register is addressable through either channel
		vectrg_w(data);
		break;

	default:
		logerror("Write %02X to unused/read-only register %02X\n", data, offset & 0x1f);
		break;
	}
}

//**************************************************************************
//  MK68564 BAUD RATE GENERATOR
//**************************************************************************

void mk68564_device::set_xtal(uint32_t clock)
{
	assert(!configured());
	subdevice<mk68564_channel>(CHANA_TAG)->set_clock(clock);
	subdevice<mk68564_channel>(CHANB_TAG)->set_clock(clock);
}

void mk68564_channel::brg_update()
{
	if (BIT(m_brg_control, 2))
		rxc_w(m_brg_state);
	if (BIT(m_brg_control, 3))
		txc_w(m_brg_state);

	if (BIT(m_brg_control, 0))
		m_brg_timer->adjust(clocks_to_attotime((m_brg_tc ? m_brg_tc : 256) * (BIT(m_brg_control, 1) ? 32 : 2)));
	else
		m_brg_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(mk68564_channel::brg_timeout)
{
	m_brg_state = !m_brg_state;
	brg_update();
}
