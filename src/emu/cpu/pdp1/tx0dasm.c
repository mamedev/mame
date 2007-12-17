#include <stdio.h>
#include <string.h>
#include "cpu/pdp1/tx0.h"

unsigned tx0_dasm_64kw(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	int md;
	int x;

	md = *((UINT32 *) oprom);

	x = md & 0177777;
	switch (md >> 16)
	{
	case 0:
		sprintf (buffer, "sto 0%06o", x);
		break;
	case 1:
		sprintf (buffer, "add 0%06o", x);
		break;
	case 2:
		sprintf (buffer, "trn 0%06o", x);
		break;
	case 3:
		sprintf (buffer, "opr 0%06o", x);
		break;
	}
	return 1;
}

unsigned tx0_dasm_8kw(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	int md;
	int x;

	md = *((UINT32 *) oprom);

	x = md & 0017777;
	switch (md >> 13)
	{
	case 0:
		sprintf (buffer, "sto 0%05o", x);
		break;
	case 1:
		sprintf (buffer, "stx 0%05o", x);
		break;
	case 2:
		sprintf (buffer, "sxa 0%05o", x);
		break;
	case 3:
		sprintf (buffer, "ado 0%05o", x);
		break;
	case 4:
		sprintf (buffer, "slr 0%05o", x);
		break;
	case 5:
		sprintf (buffer, "slx 0%05o", x);
		break;
	case 6:
		sprintf (buffer, "stz 0%05o", x);
		break;
	case 8:
		sprintf (buffer, "add 0%05o", x);
		break;
	case 9:
		sprintf (buffer, "adx 0%05o", x);
		break;
	case 10:
		sprintf (buffer, "ldx 0%05o", x);
		break;
	case 11:
		sprintf (buffer, "aux 0%05o", x);
		break;
	case 12:
		sprintf (buffer, "llr 0%05o", x);
		break;
	case 13:
		sprintf (buffer, "llx 0%05o", x);
		break;
	case 14:
		sprintf (buffer, "lda 0%05o", x);
		break;
	case 15:
		sprintf (buffer, "lax 0%05o", x);
		break;
	case 16:
		sprintf (buffer, "trn 0%05o", x);
		break;
	case 17:
		sprintf (buffer, "tze 0%05o", x);
		break;
	case 18:
		sprintf (buffer, "tsx 0%05o", x);
		break;
	case 19:
		sprintf (buffer, "tix 0%05o", x);
		break;
	case 20:
		sprintf (buffer, "tra 0%05o", x);
		break;
	case 21:
		sprintf (buffer, "trx 0%05o", x);
		break;
	case 22:
		sprintf (buffer, "tlv 0%05o", x);
		break;
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
		sprintf (buffer, "opr 0%06o", md & 0177777);
		break;
	default:
		sprintf (buffer, "illegal");
		break;
	}
	return 1;
}
