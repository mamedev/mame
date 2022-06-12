// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Midway MCR system

***************************************************************************/

#include "emu.h"
#include "includes/mcr.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


/*************************************
 *
 *  Graphics declarations
 *
 *************************************/

const gfx_layout mcr_bg_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ STEP2(RGN_FRAC(1,2),1), STEP2(RGN_FRAC(0,2),1) },
	{ STEP8(0,2) },
	{ STEP8(0,16) },
	16*8
};


const gfx_layout mcr_sprite_layout =
{
	32,32,
	RGN_FRAC(1,4),
	4,
	{ STEP4(0,1) },
	{ STEP2(RGN_FRAC(0,4)+0,4), STEP2(RGN_FRAC(1,4)+0,4), STEP2(RGN_FRAC(2,4)+0,4), STEP2(RGN_FRAC(3,4)+0,4),
		STEP2(RGN_FRAC(0,4)+8,4), STEP2(RGN_FRAC(1,4)+8,4), STEP2(RGN_FRAC(2,4)+8,4), STEP2(RGN_FRAC(3,4)+8,4),
		STEP2(RGN_FRAC(0,4)+16,4), STEP2(RGN_FRAC(1,4)+16,4), STEP2(RGN_FRAC(2,4)+16,4), STEP2(RGN_FRAC(3,4)+16,4),
		STEP2(RGN_FRAC(0,4)+24,4), STEP2(RGN_FRAC(1,4)+24,4), STEP2(RGN_FRAC(2,4)+24,4), STEP2(RGN_FRAC(3,4)+24,4) },
	{ STEP32(0,32) },
	32*32
};



/*************************************
 *
 *  Generic MCR CTC interface
 *
 *************************************/

const z80_daisy_config mcr_daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


const z80_daisy_config mcr_ipu_daisy_chain[] =
{
	{ "ipu_ctc" },
	{ "ipu_pio1" },
	{ "ipu_sio" },
	{ "ipu_pio0" },
	{ nullptr }
};



/*************************************
 *
 *  Generic MCR machine initialization
 *
 *************************************/

void mcr_state::machine_start()
{
	save_item(NAME(m_mcr_cocktail_flip));
}


void mcr_nflfoot_state::machine_start()
{
	/* allocate a timer for the IPU watchdog */
	m_ipu_watchdog_timer = timer_alloc(FUNC(mcr_nflfoot_state::ipu_watchdog_reset), this);
}


void mcr_state::machine_reset()
{
	/* reset cocktail flip */
	m_mcr_cocktail_flip = 0;
}



/*************************************
 *
 *  Generic MCR interrupt handler
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(mcr_state::mcr_interrupt)
{
	int scanline = param;

	/* CTC line 2 is connected to VBLANK, which is once every 1/2 frame */
	/* for the 30Hz interlaced display */
	if(scanline == 0 || scanline == 240)
	{
		m_ctc->trg2(1);
		m_ctc->trg2(0);
	}

	/* CTC line 3 is connected to 493, which is signalled once every */
	/* frame at 30Hz */
	if (scanline == 0)
	{
		m_ctc->trg3(1);
		m_ctc->trg3(0);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(mcr_nflfoot_state::ipu_interrupt)
{
	int scanline = param;

	/* CTC line 3 is connected to 493, which is signalled once every */
	/* frame at 30Hz */
	if (scanline == 0)
	{
		m_ctc->trg3(1);
		m_ctc->trg3(0);
	}
}


/*************************************
 *
 *  NFL Football IPU board
 *
 *************************************/

WRITE_LINE_MEMBER(mcr_nflfoot_state::sio_txda_w)
{
	m_ipu_sio_txda = !state;
}

WRITE_LINE_MEMBER(mcr_nflfoot_state::sio_txdb_w)
{
	// disc player
	m_ipu_sio_txdb = !state;

	m_ipu_sio->rxb_w(state);
}

void mcr_nflfoot_state::ipu_laserdisk_w(offs_t offset, uint8_t data)
{
	/* bit 3 enables (1) LD video regardless of PIX SW */
	/* bit 2 enables (1) LD right channel audio */
	/* bit 1 enables (1) LD left channel audio */
	/* bit 0 enables (1) LD video if PIX SW == 1 */
	if (data != 0)
		logerror("%04X:mcr_ipu_laserdisk_w(%d) = %02X\n", m_ipu->pc(), offset, data);
}


TIMER_CALLBACK_MEMBER(mcr_nflfoot_state::ipu_watchdog_reset)
{
	logerror("ipu_watchdog_reset\n");
	m_ipu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	m_ipu_ctc->reset();
	m_ipu_pio0->reset();
	m_ipu_pio1->reset();
	m_ipu_sio->reset();
}

uint8_t mcr_nflfoot_state::ipu_watchdog_r()
{
	/* watchdog counter is clocked by 7.3728MHz crystal / 16 */
	/* watchdog is tripped when 14-bit counter overflows => / 32768 = 14.0625Hz*/
	if (!machine().side_effects_disabled())
		m_ipu_watchdog_timer->adjust(attotime::from_hz(7372800 / 16 / 32768));
	return 0xff;
}


void mcr_nflfoot_state::ipu_watchdog_w(uint8_t data)
{
	(void)ipu_watchdog_r();
}
