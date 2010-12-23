/****************************************************************************

    NEC V25/V35 special function registers and internal ram access

****************************************************************************/

#include "emu.h"
#include "nec.h"
#include "v25priv.h"

#define RAMEN	(nec_state->PRC & 0x40)

int read_sfr(v25_state_t *nec_state, unsigned o)
{
	switch(o)
	{
		case 0xEA:	/* FLAG */
			return ((nec_state->F0 << 3) | (nec_state->F1 << 5));
		case 0xEB:	/* PRC */
			return nec_state->PRC;
			break;
		case 0xFF: /* IDB */
			return nec_state->IDB;
			break;
		default:
			logerror("%06x: Read from special function register %02x\n",PC(nec_state),o);
			return 0;
	}
}

void write_sfr(v25_state_t *nec_state, unsigned o, unsigned d)
{
	switch(o)
	{
		case 0xEA: /* FLAG */
			nec_state->F0 = ((d & 0x08) == 0x08); \
			nec_state->F1 = ((d & 0x20) == 0x20); \
			break;
		case 0xEB: /* PRC */
			nec_state->PRC = d & 0x4F;
			break;
		case 0xFF: /* IDB */
			nec_state->IDB = d;
			break;
		default:
			logerror("%06x: Wrote %02x to special function register %02x\n",PC(nec_state),d,o);
	}
}

int v25_read_byte(v25_state_t *nec_state, unsigned a)
{
	unsigned page = a >> 8;
	unsigned offs = a & 0xff;

	if(RAMEN && page == (nec_state->IDB << 4 | 0xE))
		return nec_state->ram.b[BYTE_XOR_LE(offs)];

	if(a == 0xFFFFF || page == (nec_state->IDB << 4 | 0xF))
		return read_sfr(nec_state, offs);

	return nec_state->program->read_byte(a);
}

int v25_read_word(v25_state_t *nec_state, unsigned a)
{
	if( a & 1 )
		return (v25_read_byte(nec_state, a) | (v25_read_byte(nec_state, a + 1) << 8));

	unsigned page = a >> 8;
	unsigned offs = a & 0xff;

	if(RAMEN && page == (nec_state->IDB << 4 | 0xE))
		return nec_state->ram.w[offs/2];

	if(page == (nec_state->IDB << 4 | 0xF))
		return (read_sfr(nec_state, offs) | (read_sfr(nec_state, offs+1) << 8));

	if(a == 0xFFFFE)	/* not sure about this */
		return (nec_state->program->read_byte(a) | (read_sfr(nec_state, 0xFF) << 8));

	return nec_state->program->read_word(a);
}

void v25_write_byte(v25_state_t *nec_state, unsigned a, unsigned d)
{
	unsigned page = a >> 8;
	unsigned offs = a & 0xff;

	if(RAMEN && page == (nec_state->IDB << 4 | 0xE))
	{
		nec_state->ram.b[BYTE_XOR_LE(offs)] = d;
		return;
	}

	if(a == 0xFFFFF || page == (nec_state->IDB << 4 | 0xF))
	{
		write_sfr(nec_state, offs, d);
		return;
	}

	nec_state->program->write_byte(a, d);
}

void v25_write_word(v25_state_t *nec_state, unsigned a, unsigned d)
{
	if( a & 1 )
	{
		v25_write_byte(nec_state, a, d & 0xff);
		v25_write_byte(nec_state, a + 1, d >> 8);
		return;
	}

	unsigned page = a >> 8;
	unsigned offs = a & 0xff;

	if(RAMEN && page == (nec_state->IDB << 4 | 0xE))
	{
		nec_state->ram.w[offs/2] = d;
		return;
	}

	if(page == (nec_state->IDB << 4 | 0xF))
	{
		write_sfr(nec_state, offs, d & 0xff);
		write_sfr(nec_state, offs + 1, d >> 8);
		return;
	}

	if(a == 0xFFFFE)	/* not sure about this */
	{
		nec_state->program->write_byte(a, d & 0xff);
		write_sfr(nec_state, 0xFF, d >> 8);
		return;
	}

	nec_state->program->write_word(a, d);
}
