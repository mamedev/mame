// license:BSD-3-Clause
// copyright-holders:Curt Coder, Fausto Pracek
/**********************************************************************

    Wang PC-PM001 Winchester Disk Controller emulation

    The programming interface is documented in chapter 10 of the Wang
    Professional Computer Technical Reference Manual (2nd edition,
    August 1985, 715-0080A): table 10-1 the host ports, table 10-2 the
    main status register, tables 10-3/10-4 the command set and the
    status bytes each command returns, table 10-5 the recording format
    - which matches the firmware byte for byte, down to the 4-byte
    header ID and the 256-byte data field. What the manual does not
    describe is anything behind that interface, so the board itself is
    modelled from the 378-9040 R9 Z80 firmware and the system BIOS; no
    schematics are known to survive.

    Host interface (I/O window selected by the slot):
    - +0x00 read: status latch (Z80 port 0x03); bit 0 is a busy
      flip-flop, set in hardware by a command latch write or a
      response latch read, cleared by the next Z80 status write. The
      firmware supplies the other bits documented in table 10-2:
      controller fault, read status, drive size, and a 4-bit count of
      the command or status bytes transferred so far.
    - +0x02 write: command latch (Z80 port 0x01); commands are 8-byte
      blocks, each byte handshaked through the busy bit. The write
      also strobes a CTC trigger to wake the Z80 from HALT.
    - +0x02 read: response latch (Z80 port 0x20); result blocks are
      8 bytes, and the read strobes the same trigger for the next one.
    - +0x04 read: clears the host interrupt.
    - +0x06 write: DMA/IRQ channel select, 1 << channel. This one is
      absent from table 10-1, which assigns the channel through bits
      1-3 of the option register instead; the diagnostic uses +0x06
      and the operational path uses +0xFE, so the board decodes both.
    - +0xFC write: board reset, running the power-on diagnostic.
    - +0xFE: option register (ID 0x01 in the low bits, interrupt
      status in bit 7); a write assigns the interrupt level and DMA
      channel together, bit 1 giving channel 1 / level 5, bit 2
      channel 2 / level 6, bit 3 channel 3 / level 7.

    Table 10-1 also gives a write to +0x00 as "abort the operation in
    progress and initialize for a new command". No software seen so
    far uses it - the BIOS and the Wang utilities reset through +0xFC
    - so it is left unimplemented rather than guessed at.

    DMA data path: a Z80 read anywhere in the 0x2000-0x27FF sector
    buffer loads the DMA address counter (the firmware deliberately
    touches the first byte to transfer right before requesting, at
    0x0E6D in the diagnostic and 0x0419 in operation); handshake
    port bit 5 sets the bus request latch, bit 7 clears it and
    raises the host interrupt. Each DACK serves the counter and
    increments it; after every 0x100 bytes served the hardware
    releases the request and strobes CTC TRG1, pacing the firmware
    one physical sector at a time while the host programs its DMA
    controller only once for the whole operation.

    Z80 side:
    - port 0x00 read: drive type ID on bits 7-5 (geometry table at
      ROM 0x0B42, 0xE0 = 305+1 cylinders, 4 heads, 32 sectors of
      256 bytes = 10MB), bit 4 ready, bit 1 seek ready.
    - port 0x00 write: head select (bits 2-0), drive enable (bit 4),
      read gate (bit 6), write gate (bit 7).
    - port 0x02 write: handshake/stepper: bit 0 step pulse, bit 1
      direction, bit 5 DMA request, bit 7 done + host interrupt.
    - memory 0x3000: MFM serializer status (sectors are built raw in
      the 0x2000 buffer: A1 FE C H S sum A1 FB + 256 data + ECC);
      the sequencing PROM 378-9041 defines the record layout.

    The serializer/disk path is high-level emulated against a CHD
    (chdman createhd -chs 306,4,32 -ss 256). The board passes the
    BIOS POST diagnostic, boots MS-DOS from the Winchester, and runs
    the original SPFORMAT and INITW utilities.

    TODO:
    - dump of the DL2212-105 PROM (376-8002.l66) is still missing
    - low-level serializer emulation using the 378-9041 PROM tables

**********************************************************************/

#include "emu.h"
#include "wdc.h"

#define VERBOSE 0
#include "logmacro.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define OPTION_ID       0x01

#define Z80_TAG         "z80"
#define MK3882_TAG      "z80ctc"

// drive type ID on port 0 bits 7-5: 0xE0 = 305 cylinders, 4 heads
// (ST-412 class, the standard Wang 10MB Winchester)
#define DRIVE_ID        0xe0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WANGPC_WDC, wangpc_wdc_device, "wangpc_wdc", "Wang PC-PM001 Winchester Disk Controller")


//-------------------------------------------------
//  ROM( wangpc_wdc )
//-------------------------------------------------

ROM_START( wangpc_wdc )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "378-9040 r9.l19", 0x0000, 0x1000, CRC(282770d2) SHA1(a0e3bad5041e0dfd6087907015b07a093b576bc0) )

	ROM_REGION( 0x1000, "address", 0 )
	ROM_LOAD( "378-9041.l54", 0x0000, 0x1000, CRC(94e9a17d) SHA1(060c576d70069ece2d0dbce86ffc448df2b169e7) )

	ROM_REGION( 0x100, "prom", 0 )
	ROM_LOAD( "376-8002.l66", 0x000, 0x100, NO_DUMP ) // DL2212-105
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *wangpc_wdc_device::device_rom_region() const
{
	return ROM_NAME( wangpc_wdc );
}


//-------------------------------------------------
//  ADDRESS_MAP( wangpc_wdc_mem )
//-------------------------------------------------

void wangpc_wdc_device::wangpc_wdc_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region(Z80_TAG, 0);
	map(0x1000, 0x17ff).ram();
	map(0x2000, 0x27ff).rw(FUNC(wangpc_wdc_device::buffer_r), FUNC(wangpc_wdc_device::buffer_w));
	map(0x3000, 0x3000).r(FUNC(wangpc_wdc_device::serializer_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( wangpc_wdc_io )
//-------------------------------------------------

void wangpc_wdc_device::wangpc_wdc_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(wangpc_wdc_device::drive_r), FUNC(wangpc_wdc_device::drive_w));
	map(0x01, 0x01).r(FUNC(wangpc_wdc_device::cmd_r));
	map(0x02, 0x02).rw(FUNC(wangpc_wdc_device::handshake_r), FUNC(wangpc_wdc_device::handshake_w));
	map(0x03, 0x03).w(FUNC(wangpc_wdc_device::status_w));
	map(0x10, 0x10).rw(FUNC(wangpc_wdc_device::ctc_ch0_r), FUNC(wangpc_wdc_device::ctc_ch0_w));
	map(0x14, 0x14).rw(FUNC(wangpc_wdc_device::ctc_ch1_r), FUNC(wangpc_wdc_device::ctc_ch1_w));
	map(0x18, 0x18).rw(FUNC(wangpc_wdc_device::ctc_ch2_r), FUNC(wangpc_wdc_device::ctc_ch2_w));
	map(0x1c, 0x1c).rw(FUNC(wangpc_wdc_device::ctc_ch3_r), FUNC(wangpc_wdc_device::ctc_ch3_w));
	map(0x20, 0x20).w(FUNC(wangpc_wdc_device::response_w));
}


//-------------------------------------------------
//  z80_daisy_config wangpc_wdc_daisy_chain
//-------------------------------------------------

static const z80_daisy_config wangpc_wdc_daisy_chain[] =
{
	{ MK3882_TAG },
	{ nullptr }
};


//-------------------------------------------------
//  machine_config( wangpc_wdc )
//-------------------------------------------------

void wangpc_wdc_device::device_add_mconfig(machine_config &config)
{
	// XTAL(10'000'000) / 2: the 10 MHz crystal feeds the MFM serializer,
	// the CPU runs at half of it. The BIOS DMA test proves the firmware
	// cannot be slower: its buffer verify (49 T-states per byte, 512
	// bytes) has to finish inside the host's settle delay of ~10.7 ms,
	// which needs at least a 4 MHz clock.
	Z80(config, m_maincpu, 10_MHz_XTAL / 2);
	m_maincpu->set_daisy_config(wangpc_wdc_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &wangpc_wdc_device::wangpc_wdc_mem);
	m_maincpu->set_addrmap(AS_IO, &wangpc_wdc_device::wangpc_wdc_io);

	Z80CTC(config, m_ctc, 10_MHz_XTAL / 2);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	HARDDISK(config, m_harddisk, 0);

}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_irq -
//-------------------------------------------------

inline void wangpc_wdc_device::set_irq(int state)
{
	m_irq = state;

	// routed by the same channel select as the DMA request: the host
	// writes it either at +0x06 (BIOS diagnostic, SPFORMAT) or through
	// the DREQ bits of the option register (BIOS boot)
	if (BIT(m_dma_select, 1)) m_bus->irq5_w(m_irq);
	if (BIT(m_dma_select, 2)) m_bus->irq6_w(m_irq);
	if (BIT(m_dma_select, 3)) m_bus->irq7_w(m_irq);
}


//-------------------------------------------------
//  set_drq - DMA request on the configured channel
//-------------------------------------------------

void wangpc_wdc_device::set_drq(int state)
{
	// the host picks the channel by writing 1 << channel to +0x06
	// (0x02 = channel 1, 0x04 = channel 2, 0x08 = channel 3)
	//
	// the Wang PC programs its 8237 for active low DREQ, so a request
	// is a low level on the line
	int const level = (state == ASSERT_LINE) ? 0 : 1;


	if (BIT(m_dma_select, 1)) m_bus->drq1_w(level);
	if (BIT(m_dma_select, 2)) m_bus->drq2_w(level);
	if (BIT(m_dma_select, 3)) m_bus->drq3_w(level);
}


//-------------------------------------------------
//  wangpcbus_tc_w - DMA terminal count
//
//  End of burst: two falling edges on CTC TRG1, which is what
//  the firmware waits for at 0x0E72 (counter must reach 2).
//-------------------------------------------------

void wangpc_wdc_device::wangpcbus_tc_w(int state)
{
	// Nothing to do: the host interrupt comes only from the firmware's
	// handshake bit 7 write (XFER_DONE / result block), a few dozen us
	// after terminal count - well within the host's settle delays.
	// Raising it here too would make the host ISR run twice per
	// operation, and the second entrance, finding no result pending,
	// flags an error that breaks SPFORMAT's state machine. The firmware
	// side needs nothing either: sector_throttle() strobes TRG1 at
	// every 0x100-byte boundary, including the one at terminal count.
}


//-------------------------------------------------
//  strobe_cmd_ctc - two falling edges on CTC TRG2
//
//  The command latch handshake needs two edges per byte: the
//  firmware re-arms channel 2 with a time constant of 2 after
//  the first transfer.
//-------------------------------------------------

void wangpc_wdc_device::strobe_cmd_ctc()
{
	machine().scheduler().perfect_quantum(attotime::from_usec(50));

	m_ctc->trg2(1);
	m_ctc->trg2(0);
	m_ctc->trg2(1);
	m_ctc->trg2(0);

	machine().scheduler().synchronize();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_wdc_device - constructor
//-------------------------------------------------

wangpc_wdc_device::wangpc_wdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WANGPC_WDC, tag, owner, clock),
	device_wangpcbus_card_interface(mconfig, *this),
	m_maincpu(*this, Z80_TAG),
	m_ctc(*this, MK3882_TAG),
	m_status(0), m_response(0), m_cmd(0), m_drive_ctrl(0), m_handshake(0),
	m_option(0), m_cmd_pending(false), m_irq(CLEAR_LINE),
	m_index_timer(nullptr), m_xfer_timer(nullptr),
	m_harddisk(*this, "harddisk"), m_serializer(0), m_dma_select(0),
	m_dma_active(false), m_dma_enabled(false), m_dma_count(0),
	m_dma_addr(0), m_buffer{}
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpc_wdc_device::device_start()
{
	// index pulses from the rotating platter (3600 rpm)
	m_index_timer = timer_alloc(FUNC(wangpc_wdc_device::index_tick), this);
	m_xfer_timer = timer_alloc(FUNC(wangpc_wdc_device::transfer_done), this);

	// state saving
	save_item(NAME(m_status));
	save_item(NAME(m_response));
	save_item(NAME(m_cmd));
	save_item(NAME(m_drive_ctrl));
	save_item(NAME(m_handshake));
	save_item(NAME(m_option));
	save_item(NAME(m_cmd_pending));
	save_item(NAME(m_irq));
	save_item(NAME(m_dma_active));
	save_item(NAME(m_dma_enabled));
	save_item(NAME(m_dma_count));
	save_item(NAME(m_dma_addr));
	save_item(NAME(m_serializer));
	save_item(NAME(m_dma_select));
	save_item(NAME(m_buffer));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wangpc_wdc_device::device_reset()
{
	m_status = 0;
	m_response = 0;
	// m_cmd survives reset: the firmware samples the latch right after
	// restart to select diagnostic modes (0x77/0x72/0x71)
	m_drive_ctrl = 0;
	m_handshake = 0;
	m_option = 0;
	m_cmd_pending = false;
	m_dma_active = false;
	m_dma_enabled = false;
	m_dma_count = 0;
	m_dma_addr = 0;

	set_irq(CLEAR_LINE);
	set_drq(CLEAR_LINE);

	m_serializer = 0;
	m_dma_select = 0;

	m_index_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}


//-------------------------------------------------
//  index_tick - one index pulse per revolution
//
//  The firmware arms CTC channel 3 and expects an interrupt
//  within a timeout, otherwise it reports drive error 0xA8.
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(wangpc_wdc_device::index_tick)
{
	m_ctc->trg3(1);
	m_ctc->trg3(0);
}


//-------------------------------------------------
//  wangpcbus_mrdc_r - memory read
//-------------------------------------------------

uint16_t wangpc_wdc_device::wangpcbus_mrdc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	return data;
}


//-------------------------------------------------
//  wangpcbus_amwc_w - memory write
//-------------------------------------------------

void wangpc_wdc_device::wangpcbus_amwc_w(offs_t offset, uint16_t mem_mask, uint16_t data)
{
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

uint16_t wangpc_wdc_device::wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0x00/2:
			// bit 0 = busy: set while a command byte awaits the Z80
			// NB: this read must NOT strobe CTC TRG1 (an early workaround
			// did): the firmware counts TRG1 pulses in the A register at
			// 0x0E72 and a stray count from a host poll landing inside a
			// transfer window skews the counter, or worse hijacks the
			// exit path at 0x0E76 where A holds other values
			data = 0xff00 | m_status | (m_cmd_pending ? 0x01 : 0x00);
			LOG("WDC host status read %02x\n", data & 0xff);
			break;

		case 0x02/2:
			data = 0xff00 | m_response;
			LOG("WDC host response read %02x\n", m_response);

			// reading the response latch wakes the Z80 for the next
			// result byte (the loop at 0x03E3 halts after each one),
			// through the same strobe as the command latch write.
			// It also sets the busy bit in hardware, symmetrically to
			// the command latch write: without it the host polls
			// status ready again before the firmware has woken up and
			// reads the same byte twice (SPFORMAT's tight reader)
			if (!machine().side_effects_disabled())
			{
				m_cmd_pending = true;
				strobe_cmd_ctc();
			}
			break;

		case 0x04/2:
			LOG("WDC host irq clear read\n");
			if (!machine().side_effects_disabled())
				set_irq(CLEAR_LINE);
			break;

		case 0xfe/2:
			data = 0xff00 | (m_irq << 7) | OPTION_ID;
			LOG("WDC host option read %02x (irq %d)\n", data & 0xff, m_irq);
			break;

		default:
			logerror("WDC host read unknown offset %02x\n", (offset & 0x7f) * 2);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_aiowc_w - I/O write
//-------------------------------------------------

void wangpc_wdc_device::wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data)
{
	if (sad(offset) && ACCESSING_BITS_0_7)
	{
		switch (offset & 0x7f)
		{
		case 0x02/2:
			// host command byte: latch it and strobe the CTC triggers to
			// wake the Z80 (ch0 armed in command mode, ch2 in diag mode)
			m_cmd = data & 0xff;
			m_cmd_pending = true;
			LOG("%s WDC host command write %02x\n", machine().describe_context(), m_cmd);
			strobe_cmd_ctc();
			break;

		case 0x06/2:
			// DMA channel select: the host writes 1 << (channel - 1)
			// (host ROM at FCC77: mov al,1 / shl al,cl / out dx,al).
			// Re-route a request in progress: release the old channel
			// lines first, then drive the new one to the current level,
			// otherwise the request stays pending on the old channel
			// and the new channel only starts on a later handshake write
			LOG("WDC host DMA channel select %02x\n", data & 0xff);
			set_drq(CLEAR_LINE);
			m_dma_select = data & 0xff;
			set_drq(m_dma_active ? ASSERT_LINE : CLEAR_LINE);
			break;

		case 0xfc/2:
			// board reset: restart the Z80 too, so the firmware
			// re-samples the command latch for 0x77/0x72/0x71 modes
			LOG("WDC host board reset write\n");
			device_reset();
			m_maincpu->reset();
			break;

		case 0xfe/2:
			{
				LOG("WDC host option write %02x\n", data & 0xff);

				bool irq = (m_irq == ASSERT_LINE);
				bool changed = ((m_option & 0x0e) != (data & 0x0e));

				if (irq && changed) set_irq(CLEAR_LINE);

				m_option = data & 0xff;

				if (irq && changed) set_irq(ASSERT_LINE);

				// the DREQ bits of the option register route the DMA
				// request too: in operation the host selects the channel
				// here (the diagnostic writes +0x06 instead)
				if (changed)
				{
					set_drq(CLEAR_LINE);
					m_dma_select = data & 0x0e;
					set_drq(m_dma_active ? ASSERT_LINE : CLEAR_LINE);
				}
			}
			break;

		default:
			logerror("WDC host write unknown offset %02x = %02x\n", (offset & 0x7f) * 2, data & 0xff);
			break;
		}
	}
}


//-------------------------------------------------
//  buffer_r / buffer_w - sector buffer window
//
//  A CPU read in the buffer window also loads the DMA address
//  counter: both the diagnostic (LD D,(HL) with HL=0x2000 at
//  0x0E6D) and the operational data phase (LD A,(IX+7) at 0x0419)
//  deliberately touch the first byte to transfer right before
//  raising the DMA request; each DACK then serves the counter
//  and increments it.
//-------------------------------------------------

uint8_t wangpc_wdc_device::buffer_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		m_dma_addr = 0x2000 | offset;
		m_dma_count = 0;
	}

	return m_buffer[offset];
}

void wangpc_wdc_device::buffer_w(offs_t offset, uint8_t data)
{
	m_buffer[offset] = data;
}


//-------------------------------------------------
//  wangpcbus_dack_r - DMA acknowledge read
//-------------------------------------------------

uint8_t wangpc_wdc_device::wangpcbus_dack_r(int line)
{
	if (!m_dma_active)
		return 0;

	uint8_t const data = m_buffer[m_dma_addr & 0x7ff];

	if (m_dma_count == 0)
		LOG("WDC burst card->host ch%d addr=%04x\n", line, m_dma_addr);

	if (!machine().side_effects_disabled())
	{
		m_dma_count++;
		m_dma_addr = 0x2000 | ((m_dma_addr + 1) & 0x7ff);
		sector_throttle();
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_dack_w - DMA acknowledge write
//-------------------------------------------------

void wangpc_wdc_device::wangpcbus_dack_w(int line, uint8_t data)
{
	if (!m_dma_active)
		return;

	if (m_dma_count == 0)
		LOG("WDC burst host->card ch%d addr=%04x\n", line, m_dma_addr);

	m_buffer[m_dma_addr & 0x7ff] = data;

	m_dma_count++;
	m_dma_addr = 0x2000 | ((m_dma_addr + 1) & 0x7ff);
	sector_throttle();
}


//-------------------------------------------------
//  sector_throttle - release the request every 0x100 bytes
//
//  The host programs its DMA controller once with the byte count
//  of the whole operation (e.g. FDBFC: sectors << 9 for the boot
//  read) and never touches it again: the card paces the stream.
//  A hardware counter releases the request after every 0x100 bytes
//  served (one physical sector of data) and strobes TRG1.
//  - In operation CTC channel 1 (armed as a counter by the code at
//    0x0091, vector at 0x0052 -> plain RETI) wakes the firmware,
//    which prepares the next sector and requests again (0x0419).
//  - In the diagnostic the pulse ISR at 0x0EA0 counts it and
//    re-requests: its redirect walks HL forward (INC H at 0x0E6D),
//    touching 0x2000 then 0x2100 - exactly the resume points of the
//    two halves of the 0x200-byte transfer. The wait at 0x0E72
//    completes at the second pulse, i.e. at the terminal count.
//-------------------------------------------------

void wangpc_wdc_device::sector_throttle()
{
	if (m_dma_active && m_dma_count != 0 && (m_dma_count & 0xff) == 0)
	{
		m_dma_enabled = false;
		update_drq();

		m_ctc->trg1(1);
		m_ctc->trg1(0);
	}
}


//-------------------------------------------------
//  wangpcbus_have_dack -
//-------------------------------------------------

bool wangpc_wdc_device::wangpcbus_have_dack(int line)
{
	return (BIT(m_dma_select, 1) && (line == 1))
		|| (BIT(m_dma_select, 2) && (line == 2))
		|| (BIT(m_dma_select, 3) && (line == 3));
}


//-------------------------------------------------
//  start_transfer - HLE of the MFM serializer
//
//  Opening the read or write gate hands the whole sector to
//  the hardware: it streams the raw record to/from the Z80
//  buffer pointed to by IX while the firmware sits in HALT.
//  The record layout is built by the firmware at 0x0AE1:
//
//    00     A1   sync
//    01     FE   ID address mark
//    02-04  cylinder / head / sector
//    05-06  A1 FB  data address mark
//    07..   256 data bytes
//-------------------------------------------------

void wangpc_wdc_device::start_transfer(bool writing)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	// target position, as maintained by the firmware
	uint16_t const cylinder = space.read_byte(0x17a2) | (space.read_byte(0x17a3) << 8);
	uint8_t const head = space.read_byte(0x17a4);
	uint8_t const sector = space.read_byte(0x17a5);
	offs_t const buffer = m_maincpu->state_int(Z80_IX);

	LOG("WDC %s C%u H%u S%u -> buffer %04x\n", writing ? "write" : "read", cylinder, head, sector, buffer);

	if (m_harddisk->exists())
	{
		hard_disk_file::info const &info = m_harddisk->get_info();
		uint32_t const lba = ((cylinder * info.heads) + head) * info.sectors + sector;
		uint8_t data[512] = { 0 };
		offs_t const base = buffer & 0x7ff;

		if (writing)
		{
			for (int i = 0; i < 256; i++)
				data[i] = m_buffer[(base + 7 + i) & 0x7ff];

			if (!m_harddisk->write(lba, data))
				logerror("WDC write failed, LBA %u\n", lba);
		}
		else if (m_harddisk->read(lba, data))
		{
			// the ID field is already in the buffer: the firmware lays it
			// out itself at 0x0AE1 before opening the gate, and compares
			// it against what comes off the platter
			for (int i = 0; i < 256; i++)
				m_buffer[(base + 7 + i) & 0x7ff] = data[i];

			// ECC syndrome: three zero bytes mean the sector read clean
			// (checked by the CPI loop at 0x0709)
			m_buffer[(base + 0x107) & 0x7ff] = 0;
			m_buffer[(base + 0x108) & 0x7ff] = 0;
			m_buffer[(base + 0x109) & 0x7ff] = 0;
		}
		else
		{
			logerror("WDC read failed, LBA %u\n", lba);
		}
	}

	// sync found and data window open
	m_serializer = 0x06;

	// one sector time at 3600 rpm with 17 sectors per track
	m_xfer_timer->adjust(attotime::from_usec(980));
}


//-------------------------------------------------
//  transfer_done - release the firmware from HALT
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(wangpc_wdc_device::transfer_done)
{
	m_serializer = 0;

	// rising edge on CTC TRG0 (armed with a time constant of 1);
	// its vector points at the EI/RETI stub at 0x08B0
	m_ctc->trg0(0);
	m_ctc->trg0(1);
}


//-------------------------------------------------
//  drive_r - drive type ID and status
//-------------------------------------------------

uint8_t wangpc_wdc_device::drive_r()
{
	/*

	    bit     description

	    7-5     drive type ID (0xE0 = 305 cyl, 4 heads)
	    4       drive ready (tested right after the select write, 0x0EC8)
	    3-2     must both read 0 for the drive to come up (0x0ED4)
	    1-0     unknown

	*/

	// bit 1 must read 1 or the seek routine bails out at 0x0A5C
	// before ever opening the read gate (error path 0x0A87)
	uint8_t data = DRIVE_ID | 0x10 | 0x02;

	LOG("%s WDC drive_r %02x\n", machine().describe_context(), data);

	return data;
}


//-------------------------------------------------
//  drive_w - drive control
//-------------------------------------------------

void wangpc_wdc_device::drive_w(uint8_t data)
{
	/*

	    bit     description

	    2-0     head select
	    4       drive enable
	    6       read gate
	    7       write gate

	*/

	LOG("%s WDC drive_w %02x\n", machine().describe_context(), data);

	// opening a gate hands the sector over to the serializer
	if (BIT(data, 6) && !BIT(m_drive_ctrl, 6))
		start_transfer(false);
	else if (BIT(data, 7) && !BIT(m_drive_ctrl, 7))
		start_transfer(true);

	m_drive_ctrl = data;
}


//-------------------------------------------------
//  cmd_r - host command byte latch
//-------------------------------------------------

uint8_t wangpc_wdc_device::cmd_r()
{
	LOG("%s WDC cmd_r %02x\n", machine().describe_context(), m_cmd);

	return m_cmd;
}


//-------------------------------------------------
//  handshake_r / handshake_w
//-------------------------------------------------

uint8_t wangpc_wdc_device::handshake_r()
{
	// bit 4 = write protect sense
	return 0x00;
}

void wangpc_wdc_device::handshake_w(uint8_t data)
{
	LOG("%s WDC handshake_w %02x\n", machine().describe_context(), data);

	// bit 0 = step pulse and bit 1 = step direction (this port also
	// drives the stepper, see the seek routine at 0x0A46-0x0A6C);
	// bit 5 sets the DMA request latch; writing bit 7, or a write
	// that drops a previously set bit 5, clears it
	bool const was_requesting = BIT(m_handshake, 5);

	if (BIT(data, 5))
		m_dma_enabled = true;
	else if (BIT(data, 7) || was_requesting)
		m_dma_enabled = false;

	// bit 7 also raises the host interrupt: it ends a diagnostic
	// window (XFER_DONE at 0x0E7C) and announces the result block in
	// operation (0x03DC, where bit 1 is just the leftover direction).
	// It is a set flip-flop, not an edge: every write with the bit
	// high asserts, the host clears by reading +0x04
	if (BIT(data, 7))
		set_irq(ASSERT_LINE);

	m_handshake = data;

	update_drq();
}


//-------------------------------------------------
//  update_drq - drive the DREQ line
//
//  The DREQ level is the request latch: handshake bit 5 sets it,
//  bit 7 (or dropping bit 5) clears it. The firmware raises it
//  only once the buffer is ready (0x0E6F in the diagnostic,
//  0x041E in the operational data phase), right after touching
//  the buffer to load the DMA address counter.
//-------------------------------------------------

void wangpc_wdc_device::update_drq()
{
	bool const level = m_dma_enabled;

	if (level && !m_dma_active)
		LOG("WDC drq rise select=%02x addr=%04x\n", m_dma_select, m_dma_addr);

	m_dma_active = level;

	set_drq(level ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  status_w - status latch to host
//-------------------------------------------------

void wangpc_wdc_device::status_w(uint8_t data)
{
	LOG("%s WDC status_w %02x\n", machine().describe_context(), data);

	// writing the status latch acknowledges the pending command byte:
	// the firmware does this once the response latch has been loaded
	m_cmd_pending = false;

	m_status = data;
}


//-------------------------------------------------
//  response_w - response/data latch to host
//-------------------------------------------------

void wangpc_wdc_device::response_w(uint8_t data)
{
	LOG("%s WDC response_w %02x\n", machine().describe_context(), data);

	m_response = data;
}


//-------------------------------------------------
//  serializer_r - MFM serializer status
//-------------------------------------------------

uint8_t wangpc_wdc_device::serializer_r()
{
	// bit 1 = sync detected, bit 2 = data window open
	return m_serializer;
}


uint8_t wangpc_wdc_device::ctc_ch0_r() { return m_ctc->read(0); }
void wangpc_wdc_device::ctc_ch0_w(uint8_t data) { m_ctc->write(0, data); }
uint8_t wangpc_wdc_device::ctc_ch1_r() { return m_ctc->read(1); }
void wangpc_wdc_device::ctc_ch1_w(uint8_t data) { m_ctc->write(1, data); }
uint8_t wangpc_wdc_device::ctc_ch2_r() { return m_ctc->read(2); }
void wangpc_wdc_device::ctc_ch2_w(uint8_t data) { m_ctc->write(2, data); }
uint8_t wangpc_wdc_device::ctc_ch3_r() { return m_ctc->read(3); }
void wangpc_wdc_device::ctc_ch3_w(uint8_t data) { m_ctc->write(3, data); }
