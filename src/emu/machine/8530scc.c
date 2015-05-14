// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    8530scc.c

    Zilog 8530 SCC (Serial Control Chip) code

*********************************************************************/


#include "emu.h"
#include "8530scc.h"

const device_type SCC8530 = &device_creator<scc8530_t>;


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_SCC (0)

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

scc8530_t::scc8530_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SCC8530, "Zilog 8530 SCC", tag, owner, clock, "scc8530", __FILE__),
	intrq_cb(*this)
{
}


/*-------------------------------------------------
    scc_updateirqs
-------------------------------------------------*/

void scc8530_t::updateirqs()
{
	int irqstat;

	irqstat = 0;
	if (MasterIRQEnable)
	{
		if ((channel[0].txIRQEnable) && (channel[0].txIRQPending))
		{
			IRQType = IRQ_B_TX;
			irqstat = 1;
		}
		else if ((channel[1].txIRQEnable) && (channel[1].txIRQPending))
		{
			IRQType = IRQ_A_TX;
			irqstat = 1;
		}
		else if ((channel[0].extIRQEnable) && (channel[0].extIRQPending))
		{
			IRQType = IRQ_B_EXT;
			irqstat = 1;
		}
		else if ((channel[1].extIRQEnable) && (channel[1].extIRQPending))
		{
			IRQType = IRQ_A_EXT;
			irqstat = 1;
		}
	}
	else
	{
		IRQType = IRQ_NONE;
	}

//  printf("SCC: irqstat %d, last %d\n", irqstat, lastIRQStat);
//  printf("ch0: en %d pd %d  ch1: en %d pd %d\n", channel[0].txIRQEnable, channel[0].txIRQPending, channel[1].txIRQEnable, channel[1].txIRQPending);

	// don't spam the driver with unnecessary transitions
	if (irqstat != lastIRQStat)
	{
		lastIRQStat = irqstat;

		// tell the driver the new IRQ line status if possible
#if LOG_SCC
		printf("SCC8530 IRQ status => %d\n", irqstat);
#endif
		if(!intrq_cb.isnull())
			intrq_cb(irqstat);
	}
}

/*-------------------------------------------------
    scc_initchannel
-------------------------------------------------*/
void scc8530_t::initchannel(int ch)
{
	channel[ch].syncHunt = 1;
}

/*-------------------------------------------------
    scc_resetchannel
-------------------------------------------------*/
void scc8530_t::resetchannel(int ch)
{
	emu_timer *timersave = channel[ch].baudtimer;

	memset(&channel[ch], 0, sizeof(Chan));

	channel[ch].txUnderrun = 1;
	channel[ch].baudtimer = timersave;

	channel[ch].baudtimer->adjust(attotime::never, ch);
}

/*-------------------------------------------------
    scc8530_baud_expire - baud rate timer expiry
-------------------------------------------------*/

void scc8530_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	Chan *pChan = &channel[id];
	int brconst = pChan->reg_val[13]<<8 | pChan->reg_val[14];
	int rate;

	if (brconst)
	{
		rate = clock() / brconst;
	}
	else
	{
		rate = 0;
	}

	// is baud counter IRQ enabled on this channel?
	// always flag pending in case it's enabled after this
	pChan->baudIRQPending = 1;
	if (pChan->baudIRQEnable)
	{
		if (pChan->extIRQEnable)
		{
			pChan->extIRQPending = 1;
			pChan->baudIRQPending = 0;
			updateirqs();
		}
	}

	// reset timer according to current register values
	if (rate)
	{
		attotime attorate = attotime::from_hz(rate);
		timer.adjust(attorate, 0, attorate);
	}
	else
	{
		timer.adjust(attotime::never, 0, attotime::never);
	}
}

/*-------------------------------------------------
    device_start - device-specific startup
-------------------------------------------------*/

void scc8530_t::device_start()
{
	intrq_cb.resolve();

	memset(channel, 0, sizeof(channel));

	mode = 0;
	reg = 0;
	status = 0;
	IRQV = 0;
	MasterIRQEnable = 0;
	lastIRQStat = 0;
	IRQType = IRQ_NONE;

	channel[0].baudtimer = timer_alloc(0);
	channel[1].baudtimer = timer_alloc(1);
}


/*-------------------------------------------------
    device_reset - device-specific reset
-------------------------------------------------*/
void scc8530_t::device_reset()
{
	IRQType = IRQ_NONE;
	MasterIRQEnable = 0;
	IRQV = 0;

	initchannel(0);
	initchannel(1);
	resetchannel(0);
	resetchannel(1);
}

/*-------------------------------------------------
    scc_set_status
-------------------------------------------------*/

void scc8530_t::set_status(int _status)
{
	status = _status;
}

/*-------------------------------------------------
    scc_acknowledge
-------------------------------------------------*/

void scc8530_t::acknowledge()
{
	if(!intrq_cb.isnull())
		intrq_cb(0);
}

/*-------------------------------------------------
    scc_getareg
-------------------------------------------------*/

UINT8 scc8530_t::getareg()
{
	/* Not yet implemented */
	#if LOG_SCC
	printf("SCC: port A reg %d read 0x%02x\n", reg, channel[0].reg_val[reg]);
	#endif

	if (reg == 0)
	{
		UINT8 rv = 0;

		Chan *ourCh = &channel[0];

		rv |= (ourCh->txUnderrun) ? 0x40 : 0;
		rv |= (ourCh->syncHunt) ? 0x10 : 0;
		rv |= channel[0].reg_val[0] & 0x0D; // pick up TXBE, RXBF, DCD bits

		return rv;
	}
	else if (reg == 10)
	{
		return 0;
	}
	return channel[0].reg_val[reg];
}



/*-------------------------------------------------
    scc_getareg
-------------------------------------------------*/

UINT8 scc8530_t::getbreg()
{
	#if LOG_SCC
	printf("SCC: port B reg %i read 0x%02x\n", reg, channel[1].reg_val[reg]);
	#endif

	if (reg == 0)
	{
		UINT8 rv = 0;

		Chan *ourCh = &channel[1];

		rv |= (ourCh->txUnderrun) ? 0x40 : 0;
		rv |= (ourCh->syncHunt) ? 0x10 : 0;
		rv |= channel[1].reg_val[0] & 0x0D; // pick up TXBE, RXBF, DCD bits

		return rv;
	}
	else if (reg == 2)
	{
		/* HACK! but lets the Mac Plus mouse move again.  Needs further investigation. */
		acknowledge();

		return status;
	}
	else if (reg == 10)
	{
		return 0;
	}

	return channel[1].reg_val[reg];
}



/*-------------------------------------------------
    scc_putreg
-------------------------------------------------*/

void scc8530_t::putreg(int ch, UINT8 data)
{
	Chan *pChan = &channel[ch];

	channel[ch].reg_val[reg] = data;
	#if LOG_SCC
	printf("SCC: port %c reg %d write 0x%02x\n", 'A'+ch, reg, data);
	#endif

	switch (reg)
	{
		case 0: // command register
			switch ((data >> 3) & 7)
			{
				case 1: // select high registers (handled elsewhere)
					break;

				case 2: // reset external and status IRQs
					pChan->syncHunt = 0;
					break;

				case 5: // ack Tx IRQ
					pChan->txIRQPending = 0;
					updateirqs();
					break;

				case 0: // nothing
				case 3: // send SDLC abort
				case 4: // enable IRQ on next Rx byte
				case 6: // reset errors
				case 7: // reset highest IUS
					// we don't handle these yet
					break;

			}
			break;

		case 1: // Tx/Rx IRQ and data transfer mode defintion
			pChan->extIRQEnable = (data & 1);
			pChan->txIRQEnable = (data & 2) ? 1 : 0;
			pChan->rxIRQEnable = (data >> 3) & 3;
			updateirqs();
			break;

		case 2: // IRQ vector
			IRQV = data;
			break;

		case 3: // Rx parameters and controls
			pChan->rxEnable = (data & 1);
			pChan->syncHunt = (data & 0x10) ? 1 : 0;
			break;

		case 5: // Tx parameters and controls
//          printf("ch %d TxEnable = %d [%02x]\n", ch, data & 8, data);
			pChan->txEnable = data & 8;

			if (pChan->txEnable)
			{
				pChan->reg_val[0] |= 0x04;  // Tx empty
			}
			break;

		case 4: // Tx/Rx misc parameters and modes
		case 6: // sync chars/SDLC address field
		case 7: // sync char/SDLC flag
			break;

		case 9: // master IRQ control
			MasterIRQEnable = (data & 8) ? 1 : 0;
			updateirqs();

			// channel reset command
			switch ((data>>6) & 3)
			{
				case 0: // do nothing
					break;

				case 1: // reset channel B
					resetchannel(0);
					break;

				case 2: // reset channel A
					resetchannel(1);
					break;

				case 3: // force h/w reset (entire chip)
					IRQType = IRQ_NONE;
					MasterIRQEnable = 0;
					IRQV = 0;

					initchannel(0);
					initchannel(1);
					resetchannel(0);
					resetchannel(1);

					// make sure we stop yanking the IRQ line if we were
					updateirqs();
					break;

			}
			break;

		case 10:    // misc transmitter/receiver control bits
		case 11:    // clock mode control
		case 12:    // lower byte of baud rate gen
		case 13:    // upper byte of baud rate gen
			break;

		case 14:    // misc control bits
			if (data & 0x01)    // baud rate generator enable?
			{
				int brconst = pChan->reg_val[13]<<8 | pChan->reg_val[14];
				int rate = clock() / brconst;

				pChan->baudtimer->adjust(attotime::from_hz(rate), 0, attotime::from_hz(rate));
			}
			break;

		case 15:    // external/status interrupt control
			pChan->baudIRQEnable = (data & 2) ? 1 : 0;
			pChan->DCDEnable = (data & 8) ? 1 : 0;
			pChan->CTSEnable = (data & 0x20) ? 1 : 0;
			pChan->txUnderrunEnable = (data & 0x40) ? 1 : 0;
			break;
	}
}

/*-------------------------------------------------
    scc8530_get_reg_a
-------------------------------------------------*/

UINT8 scc8530_t::get_reg_a(int reg)
{
	return channel[0].reg_val[reg];
}



/*-------------------------------------------------
    scc8530_get_reg_b
-------------------------------------------------*/

UINT8 scc8530_t::get_reg_b(int reg)
{
	return channel[1].reg_val[reg];
}



/*-------------------------------------------------
    scc8530_set_reg_a
-------------------------------------------------*/

void scc8530_t::set_reg_a(int reg, UINT8 data)
{
	channel[0].reg_val[reg] = data;
}



/*-------------------------------------------------
    scc8530_set_reg_a
-------------------------------------------------*/

void scc8530_t::set_reg_b(int reg, UINT8 data)
{
	channel[1].reg_val[reg] = data;
}



/*-------------------------------------------------
    scc8530_r
-------------------------------------------------*/

READ8_MEMBER( scc8530_t::reg_r)
{
	UINT8 result = 0;

	offset %= 4;

	switch(offset)
	{
		case 0:
			/* Channel B (Printer Port) Control */
			if (mode == 1)
				mode = 0;
			else
				reg = 0;

			result = getbreg();
			break;

		case 1:
			/* Channel A (Modem Port) Control */
			if (mode == 1)
				mode = 0;
			else
				reg = 0;

			result = getareg();
			break;

		case 2:
			/* Channel B (Printer Port) Data */
			result = channel[1].rxData;
			break;

		case 3:
			/* Channel A (Modem Port) Data */
			result = channel[0].rxData;
			break;
	}
	return result;
}



/*-------------------------------------------------
    scc8530_w
-------------------------------------------------*/

WRITE8_MEMBER( scc8530_t::reg_w )
{
	Chan *pChan;

	offset &= 3;

//  printf(" mode %d data %x offset %d  \n", mode, data, offset);

	switch(offset)
	{
		case 0:
			/* Channel B (Printer Port) Control */
			if (mode == 0)
			{
				if((data & 0xf0) == 0)  // not a reset command
				{
					mode = 1;
					reg = data & 0x0f;
//                  putbreg(data & 0xf0);
				}
				else if (data == 0x10)
				{
					pChan = &channel[1];
					// clear ext. interrupts
					pChan->extIRQPending = 0;
					pChan->baudIRQPending = 0;
					updateirqs();
				}
			}
			else
			{
				mode = 0;
				putreg(1, data);
			}
			break;

		case 1:
			/* Channel A (Modem Port) Control */
			if (mode == 0)
			{
				if((data & 0xf0) == 0)  // not a reset command
				{
					mode = 1;
					reg = data & 0x0f;
//                  putareg(data & 0xf0);
				}
				else if (data == 0x10)
				{
					pChan = &channel[0];
					// clear ext. interrupts
					pChan->extIRQPending = 0;
					pChan->baudIRQPending = 0;
					updateirqs();
				}
			}
			else
			{
				mode = 0;
				putreg(0, data);
			}
			break;

		case 2:
			/* Channel B (Printer Port) Data */
			pChan = &channel[1];

			if (pChan->txEnable)
			{
				pChan->txData = data;
				// local loopback?
				if (pChan->reg_val[14] & 0x10)
				{
					pChan->rxData = data;
					pChan->reg_val[0] |= 0x01;  // Rx character available
				}
				pChan->reg_val[1] |= 0x01;  // All sent
				pChan->reg_val[0] |= 0x04;  // Tx empty
				pChan->txUnderrun = 1;
				pChan->txIRQPending = 1;
				updateirqs();
			}
			break;

		case 3:
			/* Channel A (Modem Port) Data */
			pChan = &channel[0];

			if (pChan->txEnable)
			{
				pChan->txData = data;
				// local loopback?
				if (pChan->reg_val[14] & 0x10)
				{
					pChan->rxData = data;
					pChan->reg_val[0] |= 0x01;  // Rx character available
				}
				pChan->reg_val[1] |= 0x01;  // All sent
				pChan->reg_val[0] |= 0x04;  // Tx empty
				pChan->txUnderrun = 1;
				pChan->txIRQPending = 1;
				updateirqs();
			}
			break;
	}
}

/*

AppleTalk check:

SCC: port B reg 9 write 0x40      Channel Reset B
SCC: port B reg 4 write 0x20      SDLC mode
SCC: port B reg 10 write 0xe0     CRC preset + FM0
SCC: port B reg 6 write 0x00      SDLC address
SCC: port B reg 7 write 0x7e      SDLC flag
SCC: port B reg 12 write 0x06     baud rate low
SCC: port B reg 13 write 0x00     baud rate high
SCC: port B reg 14 write 0xc0     Set FM mode
SCC: port B reg 3 write 0xdd      Rx 8 bits, enter hunt mode, CRC enable, address search mode, Rx enable
SCC: port B reg 2 write 0x00      interrupt vector 0
SCC: port B reg 15 write 0x08     DCD interrupt enable
SCC: port B reg 1 write 0x09      Rx IRQ on first char or special, ext int enable
SCC: port B reg 9 write 0x0a      Master IRQ enable, no-vector mode
SCC: port B reg 11 write 0x70     Rx clock = DPLL output, Tx clock = BR generator
SCC: port B reg 14 write 0x21     Enter search mode, BR generator enable
SCC: port B reg 5 write 0x60      Tx 8 bits/char
SCC: port B reg 6 write 0x2a      SDLC address
SCC: port B reg 0 read 0x00
SCC: port B reg 15 write 0x88     DCD interrupt enable, break/abort interrupt enable

(repeats)
SCC: port B reg 1 read 0x09
SCC: port B reg 3 write 0xd0
SCC: port B reg 3 write 0xdd      Rx 8 bits, enter hunt mode, CRC enable, address search mode, Rx enable
SCC: port B reg 15 write 0x08     DCD interrupt enable
SCC: port B reg 0 read 0x00
SCC: port B reg 15 write 0x88

System 7:

SCC: port B reg 9 write 0x40      Channel Reset B
SCC: port B reg 4 write 0x20      SDLC mode
SCC: port B reg 10 write 0xe0     CRC preset + FM0
SCC: port B reg 6 write 0x00      SDLC address
SCC: port B reg 7 write 0x7e      SDLC flag
SCC: port B reg 12 write 0x06     baud rate low
SCC: port B reg 13 write 0x00     baud rate high
SCC: port B reg 14 write 0xc0     Set FM mode
SCC: port B reg 3 write 0xdd      Rx 8 bits, enter hunt mode, CRC enable, address search mode, Rx enable
SCC: port B reg 2 write 0x00      interrupt vector 0
SCC: port B reg 15 write 0x08     DCD interrupt enable
SCC: port B reg 1 write 0x09      Rx IRQ on first char or special, ext int enable
SCC: port B reg 9 write 0x0a      Master IRQ enable, no-vector mode
SCC: port B reg 11 write 0x70     Rx clock = DPLL output, Tx clock = BR generator
SCC: port B reg 14 write 0x21     Enter search mode, BR generator enable
SCC: port B reg 5 write 0x60      Tx 8 bits/char
SCC: port B reg 6 write 0x01      SDLC address
SCC: port B reg 3 write 0xdd      Rx 8 bits, enter hunt mode, CRC enable, address search mode, Rx enable

(repeats)

SCC: port B reg 0 read 0x00
SCC: port B reg 15 write 0x88     DCD interrupt enable, break/abort interrupt enable
SCC: port B reg 15 write 0x08     DCD interrupt enable
SCC: port B reg 1 read 0x09   Rx IRQ on first char or special, ext int enable
SCC: port B reg 3 write 0xdd      Rx 8 bits, enter hunt mode, CRC enable, address search mode, Rx enable

*/
