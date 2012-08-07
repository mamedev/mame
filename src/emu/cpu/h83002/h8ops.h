/*
 
H8/3xx: Unknown opcode (PC=1c966) 10f - not a valid H8 or H8S opcode, either bad dump or banked ROM
maygayep.c  ep_cfallc

H8/3xx: Unknown opcode (PC=6bfffefe) 230 - STMAC
coinmvga.c  cmkenosp

H8/3xx: Unknown opcode (PC=67fffefe) 230 - STMAC
coinmvga.c  cmkenospa

H8/3xx: Unknown opcode (PC=8f91) aeb - ADD.L ERs, ERd
maygayep.c  ep_hogmnc

H8/3xx: Unknown opcode (PC=20000) 6b6e - MOV.B @ERs, Rd
maygayep.c  ep_wordf 
 
*/

static UINT32 udata32, address24;
static INT32 sdata32;
static UINT16 udata16, ext16;
static INT16 sdata16;
static UINT8 udata8;
static INT8 sdata8;
static UINT8 srcreg, dstreg;

static void h8_group0(h83xx_state *h8, UINT16 opcode);
static void h8_group1(h83xx_state *h8, UINT16 opcode);
static void h8_group5(h83xx_state *h8, UINT16 opcode);
static void h8_group6(h83xx_state *h8, UINT16 opcode);
static void h8_group7(h83xx_state *h8, UINT16 opcode);

static int h8_branch(h83xx_state *h8, UINT8 condition);

static UINT8 h8_mov8(h83xx_state *h8, UINT8 src);
static UINT16 h8_mov16(h83xx_state *h8, UINT16 src);
static UINT32 h8_mov32(h83xx_state *h8, UINT32 src);

static UINT8 h8_add8(h83xx_state *h8, UINT8 src, UINT8 dst);
static UINT16 h8_add16(h83xx_state *h8, UINT16 src, UINT16 dst);
static UINT32 h8_add32(h83xx_state *h8, UINT32 src, UINT32 dst);

static UINT8 h8_sub8(h83xx_state *h8, UINT8 src, UINT8 dst);
static UINT16 h8_sub16(h83xx_state *h8, UINT16 src, UINT16 dst);
static UINT32 h8_sub32(h83xx_state *h8, UINT32 src, UINT32 dst);

static UINT8 h8_addx8(h83xx_state *h8, UINT8 src, UINT8 dst);

static void h8_cmp8(h83xx_state *h8, UINT8 src, UINT8 dst);
static void h8_cmp16(h83xx_state *h8, UINT16 src, UINT16 dst);
static void h8_cmp32(h83xx_state *h8, UINT32 src, UINT32 dst);
static UINT8 h8_subx8(h83xx_state *h8, UINT8 src, UINT8 dst);

static UINT8 h8_or8(h83xx_state *h8, UINT8 src, UINT8 dst);
static UINT16 h8_or16(h83xx_state *h8, UINT16 src, UINT16 dst);
static UINT32 h8_or32(h83xx_state *h8, UINT32 src, UINT32 dst);

static UINT8 h8_xor8(h83xx_state *h8, UINT8 src, UINT8 dst);
static UINT16 h8_xor16(h83xx_state *h8, UINT16 src, UINT16 dst);
static UINT32 h8_xor32(h83xx_state *h8, UINT32 src, UINT32 dst);

static UINT8 h8_and8(h83xx_state *h8, UINT8 src, UINT8 dst);
static UINT16 h8_and16(h83xx_state *h8, UINT16 src, UINT16 dst);
static UINT32 h8_and32(h83xx_state *h8, UINT32 src, UINT32 dst);

static INT8 h8_neg8(h83xx_state *h8, INT8 src);
static INT16 h8_neg16(h83xx_state *h8, INT16 src);
static INT32 h8_neg32(h83xx_state *h8, INT32 src);

static UINT16 h8_divxu8 (h83xx_state *h8, UINT16 dst, UINT8  src);
static UINT32 h8_divxu16(h83xx_state *h8, UINT32 dst, UINT16 src);

static UINT8 h8_not8(h83xx_state *h8, UINT8 src);
static UINT16 h8_not16(h83xx_state *h8, UINT16 src);
static UINT32 h8_not32(h83xx_state *h8, UINT32 src);

static UINT8 h8_rotl8(h83xx_state *h8, UINT8 src);
static UINT16 h8_rotl16(h83xx_state *h8, UINT16 src);
static UINT32 h8_rotl32(h83xx_state *h8, UINT32 src);

static UINT8 h8_rotxl8(h83xx_state *h8, UINT8 src);
static UINT16 h8_rotxl16(h83xx_state *h8, UINT16 src);
static UINT32 h8_rotxl32(h83xx_state *h8, UINT32 src);

static UINT8 h8_rotr8(h83xx_state *h8, UINT8 src);
static UINT16 h8_rotr16(h83xx_state *h8, UINT16 src);
static UINT32 h8_rotr32(h83xx_state *h8, UINT32 src);

static UINT8 h8_rotxr8(h83xx_state *h8, UINT8 src);
static UINT16 h8_rotxr16(h83xx_state *h8, UINT16 src);
static UINT32 h8_rotxr32(h83xx_state *h8, UINT32 src);

static UINT8 h8_shll8(h83xx_state *h8, UINT8 src);
static UINT16 h8_shll16(h83xx_state *h8, UINT16 src);
static UINT32 h8_shll32(h83xx_state *h8, UINT32 src);

static UINT8 h8_shlr8(h83xx_state *h8, UINT8 src);
static UINT16 h8_shlr16(h83xx_state *h8, UINT16 src);
static UINT32 h8_shlr32(h83xx_state *h8, UINT32 src);

static INT8 h8_shal8(h83xx_state *h8, INT8 src);
static INT16 h8_shal16(h83xx_state *h8, INT16 src);
static INT32 h8_shal32(h83xx_state *h8, INT32 src);

static INT8 h8_shar8(h83xx_state *h8, INT8 src);
static INT16 h8_shar16(h83xx_state *h8, INT16 src);
static INT32 h8_shar32(h83xx_state *h8, INT32 src);

static UINT8 h8_dec8(h83xx_state *h8, UINT8 src);
static UINT16 h8_dec16(h83xx_state *h8, UINT16 src);
static UINT32 h8_dec32(h83xx_state *h8, UINT32 src);

static UINT8 h8_inc8(h83xx_state *h8, UINT8 src);
static UINT16 h8_inc16(h83xx_state *h8, UINT16 src);
static UINT32 h8_inc32(h83xx_state *h8, UINT32 src);

static UINT8 h8_bnot8(h83xx_state *h8, UINT8 src, UINT8 dst);
static UINT8 h8_bst8(h83xx_state *h8, UINT8 src, UINT8 dst);
static UINT8 h8_bist8(h83xx_state *h8, UINT8 src, UINT8 dst);
static UINT8 h8_bset8(h83xx_state *h8, UINT8 src, UINT8 dst);
static UINT8 h8_bclr8(h83xx_state *h8, UINT8 src, UINT8 dst);
static void h8_btst8(h83xx_state *h8, UINT8 src, UINT8 dst);
static void h8_bld8(h83xx_state *h8, UINT8 src, UINT8 dst); // loads to carry
static void h8_bild8(h83xx_state *h8, UINT8 src, UINT8 dst); // inverts and loads to carry
static void h8_bor8(h83xx_state *h8, UINT8 src, UINT8 dst); // result in carry
static void h8_bxor8(h83xx_state *h8, UINT8 src, UINT8 dst);
static void h8_band8(h83xx_state *h8, UINT8 src, UINT8 dst);

static INT16 h8_mulxs8(h83xx_state *h8, INT8 src, INT8 dst);
static INT32 h8_mulxs16(h83xx_state *h8, INT16 src, INT16 dst);
static UINT16 h8_divxs8(h83xx_state *h8, INT8 src, INT16 dst);
static UINT32 h8_divxs16(h83xx_state *h8, INT16 src, INT32 dst);

static CPU_EXECUTE(h8)
{
	h83xx_state *h8 = get_safe_token(device);
	UINT16 opcode=0;

	h8_check_irqs(h8);

	while ((h8->cyccnt > 0) && (!h8->h8err))
	{
		h8->ppc = h8->pc;

		debugger_instruction_hook(device, h8->pc);

		opcode = h8_readop16(h8, h8->pc);
		h8->pc += 2;

		switch((opcode>>12) & 0xf)
		{
		case 0x0:
			h8_group0(h8, opcode);
			break;
		case 0x1:
			h8_group1(h8, opcode);
			break;
		case 0x2:
			// mov.b @xx:8, Rd (abs)
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_mem_read8(0xffff00+(opcode & 0xff));
			h8_mov8(h8, udata8); // flags calculation, dont care about others
			h8_setreg8(h8, dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x3: // pass
			// mov.b Rs, @xx:8 (abs)
			srcreg = (opcode>>8) & 0xf;
			udata8 = h8_getreg8(h8, srcreg);
			h8_mov8(h8, udata8); // flags calculation, dont care about others
			h8_mem_write8(0xffff00+(opcode & 0xff), udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, 0xffff00+(opcode & 0xff));
			break;
		case 0x4:
			// bcc @xx:8
			sdata8 = (opcode & 0xff);
			if( h8_branch(h8, (opcode >> 8) & 0xf) == 1) h8->pc += sdata8;
			break;
		case 0x5:
			h8_group5(h8, opcode);
			break;
		case 0x6:
			h8_group6(h8, opcode);
			break;
		case 0x7:
			h8_group7(h8, opcode);
			break;
		case 0x8:
			// add.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_add8(h8, opcode & 0xff, h8_getreg8(h8, dstreg));
			h8_setreg8(h8, dstreg, udata8);
			H8_IFETCH_TIMING(1)
			break;
		case 0x9:
			// addx.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_addx8(h8, opcode & 0xff, h8_getreg8(h8, dstreg));
			h8_setreg8(h8, dstreg, udata8);
			H8_IFETCH_TIMING(1)
			break;
		case 0xa:
			// cmp.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			h8_cmp8(h8, opcode & 0xff, h8_getreg8(h8, dstreg));
			H8_IFETCH_TIMING(1);
			break;
		case 0xb:
			// subx.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_subx8(h8, opcode & 0xff, h8_getreg8(h8, dstreg));
			h8_setreg8(h8, dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0xc:
			// or.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_or8(h8, opcode & 0xff, h8_getreg8(h8, dstreg));
			h8_setreg8(h8, dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0xd:
			// xor.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_xor8(h8, opcode & 0xff, h8_getreg8(h8, dstreg));
			h8_setreg8(h8, dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0xe: // pass
			// and.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_and8(h8, opcode & 0xff, h8_getreg8(h8, dstreg));
			h8_setreg8(h8, dstreg, udata8);
			H8_IFETCH_TIMING(1)
			break;
		case 0xf: // pass
			// mov.b #xx:8, Rd
			dstreg = (opcode>>8) & 0xf;
			udata8 = h8_mov8(h8, opcode & 0xff);
			h8_setreg8(h8, dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		}
	}

	if (h8->h8err)
	{
		fatalerror("H8/3xx: Unknown opcode (PC=%x) %x", h8->ppc, opcode);
	}
}

static void h8_group0(h83xx_state *h8, UINT16 opcode)
{
	switch((opcode>>8)&0xf)
	{
	case 0:
		// nop
		H8_IFETCH_TIMING(1);
		break;
	case 0x1:
		// 0140 = LDC/STC.W
		if (opcode == 0x0140)
		{
			ext16 = h8_mem_read16(h8, h8->pc);
			h8->pc+=2;
			switch((ext16>>8) & 0xff)
			{
				case 0x69:
					// ERd
					if (ext16 & 0x80)
					{
						h8_setreg8(h8, (ext16>>4)&7, h8_get_ccr(h8));
					}
					else
					{
						udata8 = h8_getreg8(h8, (ext16>>4)&7);
						h8_set_ccr(h8, udata8);
					}
					H8_IFETCH_TIMING(1);
					break;

				case 0x6f:
					// @(disp, ERd)
					sdata16=h8_mem_read16(h8, h8->pc); // sign extend displacements !
					h8->pc += 2;
					address24 = (h8_getreg32(h8, (ext16 >> 4)&7)) & H8_ADDR_MASK;
					address24 += sdata16;
					H8_IFETCH_TIMING(3);
					H8_WORD_TIMING(2, address24);
					if (ext16 & 0x80)
					{
						h8_mem_write8(address24, h8_get_ccr(h8));
					}
					else
					{
						udata8 = h8_mem_read8(address24);
						h8_set_ccr(h8, udata8);
					}
					H8_BYTE_TIMING(1, sdata16);
					break;

				case 0x78:
					// @(disp24, ERd)
					srcreg = (ext16 >> 4) & 7;

					// 6b20
					udata16 = h8_mem_read16(h8, h8->pc);
					h8->pc += 2;
					dstreg = udata16 & 7;

					address24 = h8_mem_read32(h8, h8->pc);
					h8->pc += 4;
					address24 += h8_getreg32(h8, srcreg);
					address24 &= H8_ADDR_MASK;
					if (udata16 == 0x6b20)
					{
						udata8 = h8_mem_read8(address24);
						h8_set_ccr(h8, udata8);
					}
					else if (udata16 == 0x6ba0)
					{
						h8_mem_write8(address24, h8_get_ccr(h8));
					}
					else
					{
						h8->h8err = 1;
					}
					H8_IFETCH_TIMING(5);
					H8_WORD_TIMING(2, address24);
					break;

				case 0x6d:
					// stc.w ccr,-@ERd / ldc @ERd+, ccr
					srcreg = (ext16>>4)&7;
					if (ext16 & 0x80)
					{
						h8_setreg32(h8, srcreg, h8_getreg32(h8, srcreg)-2);
						address24 = h8_getreg32(h8, srcreg) & H8_ADDR_MASK;
						h8_mem_write8(address24, h8_get_ccr(h8));
					}
					else
					{
						address24 = h8_getreg32(h8, srcreg) & H8_ADDR_MASK;
						h8_setreg32(h8, srcreg, h8_getreg32(h8, srcreg)+2);
						udata8 = h8_mem_read8(address24);
						h8_set_ccr(h8, udata8);
					}
					H8_IFETCH_TIMING(2);
					H8_BYTE_TIMING(1, address24);
					H8_IOP_TIMING(2);
					break;

				case 0x6b:
					if ((ext16&0xff) == 0x00)	// 16 bit absolute
					{
						address24=h8_mem_read16(h8, h8->pc);
						h8->pc += 2;
						if (address24 & 0x8000)
							address24 |= 0xff0000;
						udata8 = h8_mem_read8(address24);
						h8_set_ccr(h8, udata8);
						H8_IFETCH_TIMING(4);
						H8_BYTE_TIMING(1, address24);
					}
					else if ((ext16&0xff) == 0x20)	// 24 bit absolute
					{
						address24=h8_mem_read32(h8, h8->pc);
						h8->pc += 4;
						udata8 = h8_mem_read8(address24);
						h8_set_ccr(h8, udata8);
						H8_IFETCH_TIMING(4);
						H8_BYTE_TIMING(1, address24);
					}
					else if ((ext16&0xff) == 0x80)	// 16 bit absolute
					{
						address24=h8_mem_read16(h8, h8->pc);
						h8->pc += 2;
						if (address24 & 0x8000)
							address24 |= 0xff0000;
						h8_mem_write8(address24, h8_get_ccr(h8));
						H8_IFETCH_TIMING(3);
						H8_BYTE_TIMING(1, address24);
					}
					else if ((ext16&0xff) == 0xa0)	// 24 bit absolute
					{
						address24=h8_mem_read32(h8, h8->pc);
						h8->pc += 4;
						h8_mem_write8(address24, h8_get_ccr(h8));
						H8_IFETCH_TIMING(4);
						H8_BYTE_TIMING(1, address24);
					}
					else
					{
						h8->h8err = 1;
					}
					break;
			}
			break;
		}

		// 01yx  where x should always be 0!
		if((opcode & 0xf) != 0)
		{
			h8->h8err = 1;
			break;
		}
		switch((opcode>>4) & 0xf)
		{
			// 0100 mov.l prefix
		case 0xf:     // and.l Rn, Rn
			ext16 = h8_mem_read16(h8, h8->pc);
			h8->pc += 2;
			if (ext16 & 0x88)
			{
				h8->h8err = 1;
			}
			else
			{
				dstreg = ext16 & 0x7;
				switch((ext16>>8)&0xff)
				{
				case 0x64:	// or.l ERs, ERd
					udata32 = h8_or32(h8, h8_getreg32(h8, (ext16>>4) & 0x7), h8_getreg32(h8, dstreg));
					break;
				case 0x65:	// xor.l ERs, ERd
					udata32 = h8_xor32(h8, h8_getreg32(h8, (ext16>>4) & 0x7), h8_getreg32(h8, dstreg));
					break;
				case 0x66:	// and.l ERs, ERd
					udata32 = h8_and32(h8, h8_getreg32(h8, (ext16>>4) & 0x7), h8_getreg32(h8, dstreg));
					break;
				default:
					h8->h8err = 1;
					return;
				}
				h8_setreg32(h8, dstreg, udata32);
				H8_IFETCH_TIMING(2);
			}
			break;

		case 0:
			ext16 = h8_mem_read16(h8, h8->pc);
			h8->pc+=2;
			switch((ext16 >> 8) & 0xff)
			{
			case 0x69:
				if((ext16 & 0x80) == 0x80)
				{
					// mov.l rx, @rx
					udata32 = h8_mov32(h8, h8_getreg32(h8, ext16 & 7));
					h8_mem_write32(h8, h8_getreg32(h8, (ext16 >> 4) & 7), udata32);
					H8_IFETCH_TIMING(2);
					H8_WORD_TIMING(2, h8_getreg32(h8, (ext16 >> 4) & 7));
				}
				else
				{
					// mov.l @rx, rx
					udata32 = h8_mem_read32(h8, h8_getreg32(h8, (ext16 >> 4) &7));
					h8_mov32(h8, udata32);
					h8_setreg32(h8, ext16 & 7, udata32);
					H8_IFETCH_TIMING(2);
					H8_WORD_TIMING(2, h8_getreg32(h8, (ext16 >> 4) &7));
				}
				break;
			case 0x6b:
				// mov.l rx, @xx / mov.l @xx, rx

				switch((ext16 >> 4)&0xf)
				{
				case 0x0:
					// mov.l @aa:16, ERx
					address24=h8_mem_read16(h8, h8->pc);
					h8->pc += 2;
					if (address24 & 0x8000)
						address24 |= 0xff0000;
					udata32=h8_mem_read32(h8, address24);
					h8_mov32(h8, udata32); // flags only
					h8_setreg32(h8, ext16 & 0x7, udata32);
					H8_IFETCH_TIMING(4);
					H8_WORD_TIMING(2, address24);
					break;
				case 0x2:
					// mov.l @aa:24, ERx
					address24=h8_mem_read32(h8, h8->pc);
					h8->pc += 4;
					udata32=h8_mem_read32(h8, address24);
					h8_mov32(h8, udata32); // flags only
					h8_setreg32(h8, ext16 & 0x7, udata32);
					H8_IFETCH_TIMING(4);
					H8_WORD_TIMING(2, address24);
					break;
				case 0xa:
					// mov.l ERx, @aa:24
					address24=h8_mem_read32(h8, h8->pc);
					h8->pc += 4;
					udata32=h8_getreg32(h8, ext16 & 0x7);
					h8_mov32(h8, udata32); // flags only
					h8_mem_write32(h8, address24, udata32);
					H8_IFETCH_TIMING(4);
					H8_WORD_TIMING(2, address24);
					break;
				case 0x8:
					// mov.l ERx, @aa:16
					address24=h8_mem_read16(h8, h8->pc);
					h8->pc += 2;
					if (address24 & 0x8000)
						address24 |= 0xff0000;
					udata32=h8_getreg32(h8, ext16 & 0x7);
					h8_mov32(h8, udata32); // flags only
					h8_mem_write32(h8, address24, udata32);
					H8_IFETCH_TIMING(3);
					H8_WORD_TIMING(2, address24);
					break;
				default:
					h8->h8err = 1;
					break;
				}
				break;
			case 0x6d:
				if(ext16 & 0x80)
				{
					// mov.l rs, @-erd
					srcreg = (ext16>>4)&7;
					h8_setreg32(h8, srcreg, h8_getreg32(h8, srcreg)-4);
					address24 = h8_getreg32(h8, srcreg) & H8_ADDR_MASK;
					udata32 = h8_getreg32(h8, ext16 & 0x7);
					h8_mem_write32(h8, address24, udata32);
					H8_IFETCH_TIMING(2);
					H8_WORD_TIMING(2, address24);
					H8_IOP_TIMING(2);
				}
				else
				{
					// mov.l @ers+, rd
					srcreg = (ext16 >>4)&7;
					address24 = h8_getreg32(h8, srcreg) & H8_ADDR_MASK;
					h8_setreg32(h8, srcreg, h8_getreg32(h8, srcreg)+4);
					udata32 = h8_mem_read32(h8, address24);
					h8_setreg32(h8, ext16 & 0x7, udata32);
					H8_IFETCH_TIMING(2);
					H8_WORD_TIMING(2, address24);
					H8_IOP_TIMING(2);
				}
				h8_mov32(h8, udata32);
				break;
			case 0x6f:
				// mov.l @(displ16 + Rs), rd
				sdata16=h8_mem_read16(h8, h8->pc); // sign extend displacements !
				h8->pc += 2;
				address24 = (h8_getreg32(h8, (ext16 >> 4)&7)) & H8_ADDR_MASK;
				address24 += sdata16;
				H8_IFETCH_TIMING(3);
				H8_WORD_TIMING(2, address24);
				if(ext16 & 0x80)
				{
					udata32 = h8_getreg32(h8, ext16 & 0x7);
					h8_mem_write32(h8, address24, udata32);
				}
				else
				{
					udata32 = h8_mem_read32(h8, address24);
					h8_setreg32(h8, ext16 & 0x7, udata32);
				}
				h8_mov32(h8, udata32);
				break;
			case 0x78:
				// prefix for
				// mov.l (@aa:x, rx), Rx
				//00000A10 010078606B2600201AC2 MOV.L   @($00201AC2,ER6),ER6
				// mov.l @(displ24 + Rs), rd
				srcreg = (ext16 >> 4) & 7;

				// 6b20
				udata16 = h8_mem_read16(h8, h8->pc);
				h8->pc += 2;
				dstreg = udata16 & 7;

				address24 = h8_mem_read32(h8, h8->pc);
				h8->pc += 4;
				address24 += h8_getreg32(h8, srcreg);
				address24 &= H8_ADDR_MASK;

				if ( (ext16 & 0x80) && ((udata16 & ~7) == 0x6ba0) )
				{
					udata32 = h8_getreg32(h8, dstreg);
					h8_mem_write32(h8, address24, udata32);
				}
				else if ( (!(ext16 & 0x80)) && ((udata16 & ~7) == 0x6b20) )
				{
					udata32 = h8_mem_read32(h8, address24);
					h8_setreg32(h8, dstreg, udata32);
				}
				else
				{
					h8->h8err = 1;
				}

				h8_mov32(h8, udata32);

				H8_IFETCH_TIMING(5);
				H8_WORD_TIMING(2, address24);

				break;
			default:
				h8->h8err = 1;
				break;
			}
			break;
			// ldm/stm
		case 0x1:
		case 0x2:
		case 0x3:
			ext16 = h8_mem_read16(h8, h8->pc);
			h8->pc+=2;

			// # of registers to save
			sdata8 = ((opcode>>4) & 0xf);

			// check LDM or STM
			if (ext16 & 0x80)	// STM
			{
				srcreg = (ext16 & 7);
				for (int i = 0; i <= sdata8; i++)
				{
					h8_setreg32(h8, 7, h8_getreg32(h8, 7)-4);
					address24 = h8_getreg32(h8, 7) & H8_ADDR_MASK;
					udata32 = h8_getreg32(h8, srcreg+i);
					h8_mem_write32(h8, address24, udata32);
					H8_IFETCH_TIMING(2);
					H8_WORD_TIMING(2, address24);
					H8_IOP_TIMING(2);
				}
			}
			else	// LDM
			{
				srcreg = (ext16 & 7);
				for (int i = 0; i <= sdata8; i++)
				{
					address24 = h8_getreg32(h8, 7) & H8_ADDR_MASK;
					h8_setreg32(h8, 7, h8_getreg32(h8, 7)+4);
					udata32 = h8_mem_read32(h8, address24);
					h8_setreg32(h8, srcreg-i, udata32);
					H8_IFETCH_TIMING(2);
					H8_WORD_TIMING(2, address24);
					H8_IOP_TIMING(2);
				}
			}
			break;
		case 0x8:	// sleep
			H8_IFETCH_TIMING(1);
			break;
		case 0xc:
			// mulxs
			ext16 = h8_mem_read16(h8, h8->pc);
			h8->pc+=2;
			if(((ext16>>8) & 0xf) == 0)	// .b (8x8 = 16)
			{
				sdata16 = h8_getreg8(h8, (ext16 & 0xf)+8);	// Rd - always the low 8 bits of Rd
				sdata8 = h8_getreg8(h8, (ext16>>4) & 0xf);	// Rs
				sdata16 = h8_mulxs8(h8, sdata8, sdata16);
				h8_setreg16(h8, ext16 & 0xf, sdata16);
				H8_IFETCH_TIMING(2);
				H8_IOP_TIMING(12);
			}
			else if(((ext16>>8) & 0xf) == 2)	// .w
			{
				sdata32 = h8_getreg32(h8, ext16 & 0x7);
				sdata16 = h8_getreg16(h8, (ext16>>4) & 0xf);
				sdata32 = h8_mulxs16(h8, sdata16, sdata32);
				h8_setreg32(h8, ext16 & 0x7, sdata32);
				H8_IFETCH_TIMING(2);
				H8_IOP_TIMING(20);
			}
			else
			{
				logerror("H8/3xx: Unk. group 0 mulxs %x\n", opcode);
				h8->h8err = 1;
			}
			break;

		case 0xd:
			//divxs - probably buggy (flags?)
			ext16 = h8_mem_read16(h8, h8->pc);
			h8->pc+=2;
			if(((ext16>>8) & 0xf) == 0)
			{
				sdata16 = h8_getreg16(h8, ext16 & 0xf);
				sdata8 = h8_getreg8(h8, (ext16>>4) & 0xf);
				sdata16 = h8_divxs8(h8, sdata8, sdata16);
				h8_setreg16(h8, ext16 & 0xf, sdata16);
				H8_IFETCH_TIMING(2);
				H8_IOP_TIMING(12);
			}
			else if(((ext16>>8) & 0xf) == 3)
			{
				sdata32 = h8_getreg32(h8, ext16 & 0x7);
				sdata16 = h8_getreg16(h8, (ext16>>4) & 0xf);
				sdata32 = h8_divxs16(h8, sdata16, sdata32);
				h8_setreg32(h8, ext16 & 0x7, sdata32);
				H8_IFETCH_TIMING(2);
				H8_IOP_TIMING(20);
			}
			else
			{
				h8->h8err = 1;
			}


		break;

		default:
			h8->h8err = 1;
			break;
		}
		break;
	case 0x2:
		// stc ccr, rd
		if(((opcode>>4) & 0xf) == 0)
		{
			h8_setreg8(h8, opcode & 0xf, h8_get_ccr(h8));
			H8_IFETCH_TIMING(1);
		}
		else if(((opcode>>4) & 0xf) == 1)
        {
			h8_setreg8(h8, opcode & 0xf, h8_get_exr(h8));
			H8_IFETCH_TIMING(1);
        }
		else
		{
			logerror("H8/3xx: Unk. group 0 2 %x\n", opcode);
			h8->h8err = 1;
		}
		break;
	case 0x3:
		// ldc rd, ccr
		if(((opcode>>4) & 0xf) == 0)
		{
			udata8 = h8_getreg8(h8, opcode & 0xf);
			h8_set_ccr(h8, udata8);
			H8_IFETCH_TIMING(1);
		}
		else if(((opcode>>4) & 0xf) == 1)
        {
			udata8 = h8_getreg8(h8, opcode & 0xf);
			h8_set_exr(h8, udata8);
			H8_IFETCH_TIMING(1);
        }
		else
		{
			logerror("H8/3xx: Unk. group 0 3 %x\n", opcode);
			h8->h8err = 1;
		}
		break;
	case 0x4: // pass
		// orc
		udata8 = h8_or8(h8, opcode & 0xff, h8_get_ccr(h8));
		h8_set_ccr(h8, udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0x6:
		// andc
		udata8 = h8_and8(h8, opcode & 0xff, h8_get_ccr(h8));
		h8_set_ccr(h8, udata8);
		H8_IFETCH_TIMING(1)
		break;
	case 0x7:
		// ldc
		h8_set_ccr(h8, opcode & 0xff);
		H8_IFETCH_TIMING(1)
		break;
	case 0x8:
		// add.b rx, ry
		dstreg = opcode & 0xf;
		udata8 = h8_add8(h8, h8_getreg8(h8, (opcode>>4) &0xf), h8_getreg8(h8, dstreg));
		h8_setreg8(h8, dstreg, udata8);
		H8_IFETCH_TIMING(1)
		break;
		// add.w rx, ry
	case 0x9:
		dstreg = opcode & 0xf;
		udata16 = h8_add16(h8, h8_getreg16(h8, (opcode>>4) &0xf), h8_getreg16(h8, dstreg));
		h8_setreg16(h8, dstreg, udata16);
		H8_IFETCH_TIMING(1)
		break;
		// inc.b rx
	case 0xA:
		if(opcode&0x80)
		{
			if(opcode & 0x8)
			{
				logerror("H8/3xx: Unk. group 0 a %x\n", opcode);
				h8->h8err = 1;
			}
			else
			{
				dstreg = opcode & 0x7;
				udata32 = h8_add32(h8, h8_getreg32(h8, (opcode>>4) &0x7), h8_getreg32(h8, dstreg));
				h8_setreg32(h8, dstreg, udata32);
				H8_IFETCH_TIMING(1)
			}
		}
		else
		{
			if(opcode & 0xf0)
			{
			logerror("H8/3xx: Unk. group 0 a2 %x\n", opcode);
				h8->h8err =1;
			}
			else
			{
				dstreg = opcode & 0xf;
				udata8 = h8_inc8(h8, h8_getreg8(h8, dstreg));
				h8_setreg8(h8, dstreg, udata8);
				H8_IFETCH_TIMING(1);
			}
		}
		break;
	case 0xb:
		switch((opcode>>4)& 0xf)
		{
		case 0:
			if(opcode & 8)
			{
				h8->h8err = 1;
			}
			else
			{
				dstreg = opcode & 7;
				udata32 = h8_getreg32(h8, dstreg) + 1;
				h8_setreg32(h8, dstreg, udata32);
				H8_IFETCH_TIMING(1)
			}
			break;
		case 5:
			dstreg = opcode & 0xf;
			udata16 = h8_inc16(h8, h8_getreg16(h8, dstreg));
			h8_setreg16(h8, dstreg, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 7:
			dstreg = opcode & 0x7;
			udata32 = h8_inc32(h8, h8_getreg32(h8, dstreg));
			h8_setreg32(h8, dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 8:
			if(opcode & 8)
			{
				h8->h8err = 1;
			}
			else
			{
				dstreg = opcode & 7;
				udata32 = h8_getreg32(h8, dstreg) + 2;
				h8_setreg32(h8, dstreg, udata32);
				H8_IFETCH_TIMING(1)
			}
			break;
		case 9:
			if(opcode & 8)
			{
				h8->h8err = 1;
			}
			else
			{
				dstreg = opcode & 7;
				udata32 = h8_getreg32(h8, dstreg) + 4;
				h8_setreg32(h8, dstreg, udata32);
				H8_IFETCH_TIMING(1)
			}
			break;
		case 0xd:
			dstreg = opcode & 0xf;
			udata16 = h8_inc16(h8, h8_getreg16(h8, dstreg));
			if(h8->h8vflag)
			{
				udata16 = h8_inc16(h8, udata16); // slow and easy
				h8->h8vflag = 1;
			}
			else
				udata16 = h8_inc16(h8, udata16); // slow and easy
			h8_setreg16(h8, dstreg, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xf:
			dstreg = opcode & 0x7;
			udata32 = h8_inc32(h8, h8_getreg32(h8, dstreg));
			if(h8->h8vflag)
			{
				udata32 = h8_inc32(h8, udata32); // slow and easy
				h8->h8vflag = 1;
			}
			else
				udata32 = h8_inc32(h8, udata32); // slow and easy
			h8_setreg32(h8, dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		default:
			logerror("H8/3xx: Unk. group 0 b %x\n", opcode);
			h8->h8err = 1;
			break;
		}
		break;
		// mov.b rx, ry
	case 0xc: // pass
		dstreg = opcode & 0xf;
		udata8 = h8_mov8(h8, h8_getreg8(h8, (opcode>>4) &0xf));
		h8_setreg8(h8, dstreg, udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0xd:
		// mov.w rx, ry
		dstreg = opcode & 0xf;
		udata16 = h8_mov16(h8, h8_getreg16(h8, (opcode>>4) &0xf));
		h8_setreg16(h8, dstreg, udata16);
		H8_IFETCH_TIMING(1);
		break;
	case 0xf:
		if(opcode & 0x80)
		{
			if(opcode & 8)
			{
			logerror("H8/3xx: Unk. group 0 f %x\n", opcode);
				h8->h8err = 1;
			}
			else
			{
				dstreg = opcode & 0x7;
				udata32 = h8_mov32(h8, h8_getreg32(h8, (opcode>>4) &0x7));
				h8_setreg32(h8, dstreg, udata32);
				H8_IFETCH_TIMING(1);
			}
		}
		else
		{
			h8->h8err = 1;
			logerror("H8/3xx: Unk. group 0 f2 %x\n", opcode);
			if((opcode & 0xf0) !=0)
			{
				h8->h8err = 1;
			}
			else
			{
				h8->h8err = 1;
			}
		}
		break;
	default:
		logerror("H8/3xx: Unk. group 0 tdef %x\n", opcode);
		h8->h8err = 1;
		break;
	}
}

static void h8_group1(h83xx_state *h8, UINT16 opcode)
{
	switch((opcode>>8)&0xf)
	{
	case 0x0:
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			// shll.b Rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_shll8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x1:
			// shll.w Rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_shll16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0x3:
			// shll.l Rx
			udata32 = h8_getreg32(h8, opcode & 0x7);
			udata32 = h8_shll32(h8, udata32);
			h8_setreg32(h8, opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0x4:
			// shll.b #2, Rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_shll8(h8, udata8);
			udata8 = h8_shll8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x5:
			// shll.w #2, Rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_shll16(h8, udata16);
			udata16 = h8_shll16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0x7:
			// shll.l #2, Rx
			udata32 = h8_getreg32(h8, opcode & 0x7);
			udata32 = h8_shll32(h8, udata32);
			udata32 = h8_shll32(h8, udata32);
			h8_setreg32(h8, opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;

		case 0x8:
			// shal.b Rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_shal8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x9:
			// shal.w Rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_shal16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xb:
			// shal.l ERx
			udata32 = h8_getreg32(h8, opcode & 0x7);
			udata32 = h8_shal32(h8, udata32);
			h8_setreg32(h8, opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0xc:
			// shal.b #2, Rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_shal8(h8, udata8);
			udata8 = h8_shal8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0xd:
			// shal.w #2, Rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_shal16(h8, udata16);
			udata16 = h8_shal16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xf:
			// shal.l #2, ERx
			udata32 = h8_getreg32(h8, opcode & 0x7);
			udata32 = h8_shal32(h8, udata32);
			udata32 = h8_shal32(h8, udata32);
			h8_setreg32(h8, opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;
		default:
			logerror("H8/3xx: Unk. group 1 0 %x\n", opcode);
			h8->h8err = 1;
			break;
		}
		break;
	case 0x1:
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			// shlr.b rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_shlr8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x1:
			// shlr.w rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_shlr16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
			// shlr.l rx
		case 0x3:
			udata32 = h8_getreg32(h8, opcode & 0x7);
			udata32 = h8_shlr32(h8, udata32);
			h8_setreg32(h8, opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0x4:
			// shlr.b #2, rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_shlr8(h8, udata8);
			udata8 = h8_shlr8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x5:
			// shlr.w #2, rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_shlr16(h8, udata16);
			udata16 = h8_shlr16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
			// shlr.l #2, rx
		case 0x7:
			udata32 = h8_getreg32(h8, opcode & 0x7);
			udata32 = h8_shlr32(h8, udata32);
			udata32 = h8_shlr32(h8, udata32);
			h8_setreg32(h8, opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0x8:
			// shar.b rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_shar8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x9:
			// shar.w rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_shar16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xb:
			// shar.l rx
			udata32 = h8_getreg32(h8, opcode & 0x7);
			udata32 = h8_shar32(h8, udata32);
			h8_setreg32(h8, opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0xc:
			// shar.b #2, rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_shar8(h8, udata8);
			udata8 = h8_shar8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0xd:
			// shar.w #2, rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_shar16(h8, udata16);
			udata16 = h8_shar16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xf:
			// shar.l #2, rx
			udata32 = h8_getreg32(h8, opcode & 0x7);
			udata32 = h8_shar32(h8, udata32);
			udata32 = h8_shar32(h8, udata32);
			h8_setreg32(h8, opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;
		default:
			logerror("H8/3xx: Unk. group 1 1 %x\n", opcode);
			h8->h8err = 1;
			break;
		}
		break;
	case 0x2:
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			// rotxl.b Rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_rotxl8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x1:
			// rotxl.w Rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_rotxl16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;

		case 0x3:
			// rotxl.l Rx
			udata32 = h8_getreg32(h8, opcode & 0x7);
			udata32 = h8_rotxl32(h8, udata32);
			h8_setreg32(h8, opcode & 0xf, udata32);
			H8_IFETCH_TIMING(1);
			break;

		case 0x8:
			// rotl.b Rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_rotl8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x9:
			// rotl.w Rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_rotl16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xb:
			// rotl.l ERx
			udata32 = h8_getreg32(h8, opcode & 0x7);
			udata32 = h8_rotl32(h8, udata32);
			h8_setreg32(h8, opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0xc:
			// rotl.b #2, Rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_rotl8(h8, udata8);
			udata8 = h8_rotl8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0xd:
			// rotl.w #2, Rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_rotl16(h8, udata16);
			udata16 = h8_rotl16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xf:
			// rotl.l #2, ERx
			udata32 = h8_getreg32(h8, opcode & 0x7);
			udata32 = h8_rotl32(h8, udata32);
			udata32 = h8_rotl32(h8, udata32);
			h8_setreg32(h8, opcode & 0x7, udata32);
			H8_IFETCH_TIMING(1);
			break;

		default:
			logerror("H8/3xx: Unk. group 1 2 %x\n", opcode);
			h8->h8err = 1;
			break;
		}
		break;
	case 0x3:
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			// rotxr.b Rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_rotxr8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x1:
			// rotxr.w Rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_rotxr16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0x3:
			// rotxr.l Rx
			udata32 = h8_getreg32(h8, opcode & 0xf);
			udata32 = h8_rotxr32(h8, udata32);
			h8_setreg32(h8, opcode & 0xf, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0x8:
			// rotr.b Rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_rotr8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x9:
			// rotr.w Rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_rotr16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xb:
			// rotr.l Rx
			udata32 = h8_getreg32(h8, opcode & 0xf);
			udata32 = h8_rotr32(h8, udata32);
			h8_setreg32(h8, opcode & 0xf, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0xc:
			// rotr.b #2, Rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_rotr8(h8, udata8);
			udata8 = h8_rotr8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0xd:
			// rotr.w #2, Rx
			udata16 = h8_getreg16(h8, opcode & 0xf);
			udata16 = h8_rotr16(h8, udata16);
			udata16 = h8_rotr16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xf:
			// rotr.l #2, Rx
			udata32 = h8_getreg32(h8, opcode & 0xf);
			udata32 = h8_rotr32(h8, udata32);
			udata32 = h8_rotr32(h8, udata32);
			h8_setreg32(h8, opcode & 0xf, udata32);
			H8_IFETCH_TIMING(1);
			break;
		default:
			logerror("H8/3xx: Unk. group 1 3 %x\n", opcode);
			h8->h8err = 1;
			break;
		}
		break;
	case 0x4:
		// or.b rs, rd
		dstreg = opcode & 0xf;
		udata8 = h8_or8(h8, h8_getreg8(h8, (opcode>>4) &0xf), h8_getreg8(h8, dstreg));
		h8_setreg8(h8, dstreg, udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0x5:
		// xor.b rs, rd
		dstreg = opcode & 0xf;
		udata8 = h8_xor8(h8, h8_getreg8(h8, (opcode>>4) &0xf), h8_getreg8(h8, dstreg));
		h8_setreg8(h8, dstreg, udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0x6:
		// and.b rs, rd
		dstreg = opcode & 0xf;
		udata8 = h8_and8(h8, h8_getreg8(h8, (opcode>>4) &0xf), h8_getreg8(h8, dstreg));
		h8_setreg8(h8, dstreg, udata8);
		H8_IFETCH_TIMING(1)
		break;
		// not
	case 0x7:
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			// not.b Rx
			dstreg = opcode & 0xf;
			udata8 = h8_not8(h8, h8_getreg8(h8, dstreg));
			h8_setreg8(h8, dstreg, udata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x1:
			// not.w Rx
			dstreg = opcode & 0xf;
			udata16 = h8_not16(h8, h8_getreg16(h8, dstreg));
			h8_setreg16(h8, dstreg, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0x3:
			// not.l ERx
			dstreg = opcode & 0x7;
			udata32 = h8_not32(h8, h8_getreg32(h8, dstreg));
			h8_setreg32(h8, dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;

		case 0x5:
			// extu.w Rx
			dstreg = opcode & 0xf;
			udata16 = h8_getreg16(h8, dstreg) & 0x00ff;
			h8_setreg16(h8, dstreg, udata16);
			h8->h8nflag = 0;
			h8->h8vflag = 0;
			h8->h8zflag = ((udata16 == 0) ? 1 : 0);
			H8_IFETCH_TIMING(1);
			break;
		case 0x7:
			// extu.l Rx
			dstreg = opcode & 0x7;
			udata32 = h8_getreg32(h8, dstreg) & 0x0000ffff;
			h8_setreg32(h8, dstreg, udata32);
			h8->h8nflag = 0;
			h8->h8vflag = 0;
			h8->h8zflag = ((udata32 == 0) ? 1 : 0);
			H8_IFETCH_TIMING(1);
			break;
		case 0x8:
			// neg.b Rx
			dstreg = opcode & 0xf;
			sdata8 = h8_neg8(h8, h8_getreg8(h8, dstreg));
			h8_setreg8(h8, dstreg, sdata8);
			H8_IFETCH_TIMING(1);
			break;
		case 0x9:
			// neg.w Rx
			dstreg = opcode & 0xf;
			sdata16 = h8_neg16(h8, h8_getreg16(h8, dstreg));
			h8_setreg16(h8, dstreg, sdata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xb:
			// neg.l ERx
			dstreg = opcode & 0x7;
			sdata32 = h8_neg32(h8, h8_getreg32(h8, dstreg));
			h8_setreg32(h8, dstreg, sdata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0xd:
			// exts.w Rx
			dstreg = opcode & 0xf;
			udata16=h8_getreg16(h8, dstreg)&0xff;
			if(udata16&0x80)
			{
				udata16|=0xff00;
			}
			h8_setreg16(h8, dstreg, udata16);

			h8->h8vflag = 0;
			h8->h8nflag = (udata16 & 0xff00) ? 1 : 0;
			h8->h8zflag = (udata16) ? 0 : 1;

			H8_IFETCH_TIMING(1);
			break;

		case 0xf:
			// exts.l Rx
			dstreg = opcode & 0x7;
			udata32=h8_getreg32(h8, dstreg)&0xffff;
			if(udata32&0x8000)
			{
				udata32|=0xffff0000;
			}
			h8_setreg32(h8, dstreg, udata32);

			h8->h8vflag = 0;
			h8->h8nflag = (udata32 & 0xffff0000) ? 1 : 0;
			h8->h8zflag = (udata32) ? 0 : 1;

			H8_IFETCH_TIMING(1);
			break;

		default:
			logerror("H8/3xx: Unk. group 1 7-9 %x\n", opcode);
			h8->h8err = 1;
			break;
		}
		break;
	case 0x8:
		// sub.b rs, rd
		dstreg = opcode & 0xf;
		udata8 = h8_sub8(h8, h8_getreg8(h8, (opcode>>4) &0xf), h8_getreg8(h8, dstreg));
		h8_setreg8(h8, dstreg, udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0x9:
		// sub.w rs, rd
		dstreg = opcode & 0xf;
		udata16 = h8_sub16(h8, h8_getreg16(h8, (opcode>>4) &0xf), h8_getreg16(h8, dstreg));
		h8_setreg16(h8, dstreg, udata16);
		H8_IFETCH_TIMING(1);
		break;
		// sub.b rx
	case 0xA:
		if(opcode&0x80)
		{
			//logerror("H8/3xx: Unk. group 1 A %x\n", opcode);

			// sub.l rs,rd
			dstreg = opcode & 0x7;
			udata32=h8_sub32(h8, h8_getreg32(h8, (opcode>>4) &0x7), h8_getreg32(h8, dstreg));
			h8_setreg32(h8, dstreg, udata32);
			H8_IFETCH_TIMING(2);
			break;

		}
		else
		{
			if(opcode & 0xf0)
			{
				logerror("H8/3xx: Unk. group A2 0 %x\n", opcode);
				h8->h8err = 1;
			}
			else
			{
				udata8 = h8_getreg8(h8, opcode & 0xf);
				udata8 = h8_dec8(h8, udata8);
				h8_setreg8(h8, opcode & 0xf, udata8);
				H8_IFETCH_TIMING(1);
			}
		}
		break;
		//
	case 0xb:
		switch((opcode>>4)& 0xf)
		{
		case 0:	// subs.l #1, rN (decrement without touching flags)
			dstreg = opcode & 0x7;
			udata32 = h8_getreg32(h8, dstreg);
			udata32--;
			h8_setreg32(h8, dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 5:	// dec.w #1, rN
			dstreg = opcode & 0xf;
			udata16 = h8_dec16(h8, h8_getreg16(h8, dstreg));
			h8_setreg16(h8, dstreg, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 7:	// dec.l #1, rN
			dstreg = opcode & 0x7;
			udata32 = h8_dec32(h8, h8_getreg32(h8, dstreg));
			h8_setreg32(h8, dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 8:	// subs.l #2, rN (decrement without touching flags)
			dstreg = opcode & 0x7;
			udata32 = h8_getreg32(h8, dstreg);
			udata32-=2;
			h8_setreg32(h8, dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 9:	// subs.l #4, rN (decrement without touching flags)
			dstreg = opcode & 0x7;
			udata32 = h8_getreg32(h8, dstreg);
			udata32-=4;
			h8_setreg32(h8, dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;
		case 0xd:	// dec.w #2, rN
			dstreg = opcode & 0xf;
			udata16 = h8_dec16(h8, h8_getreg16(h8, dstreg));
			if (h8->h8vflag)
			{
				udata16 = h8_dec16(h8, udata16);
				h8->h8vflag = 1;
			}
			else
				udata16 = h8_dec16(h8, udata16);
			h8_setreg16(h8, dstreg, udata16);
			H8_IFETCH_TIMING(1);
			break;
		case 0xf:	// dec.l #2, rN
			dstreg = opcode & 0x7;
			udata32 = h8_dec32(h8, h8_getreg32(h8, dstreg));
			if (h8->h8vflag)
			{
				udata32 = h8_dec32(h8, udata32);
				h8->h8vflag = 1;
			}
			else
				udata32 = h8_dec32(h8, udata32);
			h8_setreg32(h8, dstreg, udata32);
			H8_IFETCH_TIMING(1);
			break;

		default:
			logerror("H8/3xx: Unk. group 1 B %x\n", opcode);
			h8->h8err = 1;
			break;
		}
		break;
	case 0xc:
		// cmp.b rs, rd
		h8_cmp8(h8, h8_getreg8(h8, (opcode>>4) &0xf), h8_getreg8(h8, opcode & 0xf));
		H8_IFETCH_TIMING(1);
		break;
	case 0xd:
		// cmp.w rx, ry
		h8_cmp16(h8, h8_getreg16(h8, (opcode>>4) &0xf), h8_getreg16(h8, opcode & 0xf));
		H8_IFETCH_TIMING(1);
		break;
	case 0xe:
		// subx.b rx, ry
		dstreg = opcode & 0xf;
		udata8 = h8_subx8(h8, h8_getreg8(h8, (opcode>>4) &0xf), h8_getreg8(h8, dstreg));
		h8_setreg8(h8, dstreg, udata8);
		H8_IFETCH_TIMING(1);
		break;
	case 0xf:
		if(opcode & 0x80)
		{
			if(opcode & 8)
			{
				logerror("H8/3xx: Unk. group 1 f %x\n", opcode);
				h8->h8err = 1;
			}
			else
			{
				h8_cmp32(h8, h8_getreg32(h8, (opcode>>4) & 0x7), h8_getreg32(h8, opcode & 0x7));
				H8_IFETCH_TIMING(1);
			}
		}
		else
		{
			logerror("H8/3xx: Unk. group 1 f2 %x\n", opcode);
			h8->h8err = 1;
		}
		break;
	default:
		logerror("H8/3xx: Unk. group 1 def %x\n", opcode);
		h8->h8err = 1;
		break;
	}
}


static void h8_group5(h83xx_state *h8, UINT16 opcode)
{
	switch((opcode>>8)&0xf)
	{
	case 0x0:
		// mulxu.b
		udata8 = h8_getreg8(h8, (opcode>>4)&0xf);
		udata16 = h8_getreg16(h8, opcode & 0xf);
		udata16 &= 0xff;
		udata16 = udata16*udata8;
		// no flags modified!
		h8_setreg16(h8, opcode & 0xf, udata16);
		H8_IFETCH_TIMING(1);
		H8_IOP_TIMING(12);
		break;
	case 0x1:
		// divxu.b
		udata8 = h8_getreg8(h8, (opcode>>4)&0xf);
		udata16 = h8_getreg16(h8, opcode & 0x0f);
		udata16 = h8_divxu8(h8, udata16,udata8);
		h8_setreg16(h8, opcode & 0xf, udata16);
		H8_IFETCH_TIMING(1);
		H8_IOP_TIMING(12);
		break;
	case 0x2:
		// mulxu.w
		udata16 = h8_getreg16(h8, (opcode>>4)&0xf);
		udata32 = h8_getreg32(h8, opcode & 7);
		udata32 &= 0xffff;
		udata32 = udata32*udata16;
		// no flags modified!
		h8_setreg32(h8, opcode & 7, udata32);
		H8_IFETCH_TIMING(1);
		H8_IOP_TIMING(20);
		break;
	case 0x3:
		// divxu.w
		udata16 = h8_getreg16(h8, (opcode>>4)&0xf);
		udata32 = h8_getreg32(h8, opcode & 7);
		udata32 = h8_divxu16(h8, udata32,udata16);
		h8_setreg32(h8, opcode & 7, udata32);
		H8_IFETCH_TIMING(1);
		H8_IOP_TIMING(20);
		break;
	case 0x4:
		if(opcode == 0x5470)
		{
			// rts
			udata32 = h8_mem_read32(h8, h8_getreg32(h8, H8_SP));
			h8_setreg32(h8, H8_SP, h8_getreg32(h8, H8_SP)+4);
			// extended mode
			h8->pc = udata32 & 0xffffff;
			H8_IFETCH_TIMING(2);
			H8_STACK_TIMING(2);
			H8_IOP_TIMING(2);
		}
		else
		{
			logerror("H8/3xx: Unk. group 5 1 %x\n", opcode);
			h8->h8err = 1;
		}
		break;
	case 0x5:
		// bsr 8
		sdata8 = opcode & 0xff;
		// extended mode stack push!
		h8_setreg32(h8, H8_SP, h8_getreg32(h8, H8_SP)-4);
		h8_mem_write32(h8, h8_getreg32(h8, H8_SP), h8->pc);
		h8->pc = h8->pc + sdata8;
		H8_IFETCH_TIMING(2); H8_STACK_TIMING(2);
		break;
	case 0x6:
		// rte
		if(opcode == 0x5670)
		{
			// restore CCR
			udata8 = (UINT8)h8_mem_read16(h8, h8_getreg32(h8, H8_SP));
			h8_setreg32(h8, H8_SP, h8_getreg32(h8, H8_SP)+2);

			// check if PC is 16 or 24/32 bits wide
			if (h8->mode_8bit)
			{
				udata16 = h8_mem_read16(h8, h8_getreg16(h8, H8_SP));
				h8_setreg16(h8, H8_SP, h8_getreg16(h8, H8_SP)+2);

				h8->pc = udata16;
			}
			else
			{
				// extended mode restore PC
				udata32 = h8_mem_read32(h8, h8_getreg32(h8, H8_SP));
				h8_setreg32(h8, H8_SP, h8_getreg32(h8, H8_SP)+4);

				// extended mode
				h8->pc = udata32;
			}
			// must do this last, because set_ccr() does a check_irq()
			h8_set_ccr(h8, udata8);
			H8_IFETCH_TIMING(2);
			H8_STACK_TIMING(2);
			H8_IOP_TIMING(2);
		}
		else
		{
			logerror("H8/3xx: Unk. group 5 6 %x\n", opcode);
			h8->h8err = 1;
		}
		break;
		// trapa
	case 0x7:
		logerror("H8/3xx: Unk. group 5 7 %x\n", opcode);
		h8->h8err = 1;
		break;
	case 0x8:
		// bcc @xx:16
		if(opcode & 0xf)
		{
			logerror("H8/3xx: Unk. group 5 8 %x\n", opcode);
			h8->h8err = 1;
		}
		else
		{
			sdata16 = h8_mem_read16(h8, h8->pc);
			h8->pc += 2;
			if( h8_branch(h8, (opcode >> 4) & 0xf) == 1) h8->pc += sdata16;
			H8_IOP_TIMING(2)
		}
		break;
	case 0x9:
		// jmp @erd
		address24 = h8_getreg32(h8, (opcode>>4)&7);
		address24 &= H8_ADDR_MASK;
		h8->pc = address24;
		H8_IFETCH_TIMING(2);
		break;
		// jmp @aa:24
	case 0xa:
		address24 = h8_mem_read32(h8, h8->pc-2);
		address24 &= H8_ADDR_MASK;
		h8->pc = address24;
		H8_IFETCH_TIMING(2);
		H8_IOP_TIMING(2);
		break;
		// jmp @aa:8
	case 0xc:
		if(opcode & 0xff)
		{
			logerror("H8/3xx: Unk. group 5 c %x\n", opcode);
			h8->h8err = 1;
		}
		else
		{
			// bsr d:16
			sdata16=h8_mem_read16(h8, h8->pc);
			h8_setreg32(h8, H8_SP, h8_getreg32(h8, H8_SP)-4);
			h8_mem_write32(h8, h8_getreg32(h8, H8_SP), h8->pc+2);
			h8->pc += sdata16 + 2;
			H8_IFETCH_TIMING(2); H8_STACK_TIMING(2); H8_IOP_TIMING(2);
		}
		break;
	case 0xd:
		// jsr @reg
		address24=h8_getreg32(h8, (opcode>>4)&7);
		address24 &= H8_ADDR_MASK;
		// extended mode stack push!
		h8_setreg32(h8, H8_SP, h8_getreg32(h8, H8_SP)-4);
		h8_mem_write32(h8, h8_getreg32(h8, H8_SP), h8->pc);
		h8->pc = address24;
		H8_STACK_TIMING(2);
		H8_IOP_TIMING(2);
		break;
	case 0xe:
		// jsr @aa:24
		address24=h8_mem_read32(h8, h8->pc-2);
		address24 &= H8_ADDR_MASK;
		// extended mode stack push!
		h8_setreg32(h8, H8_SP, h8_getreg32(h8, H8_SP)-4);
		h8_mem_write32(h8, h8_getreg32(h8, H8_SP), h8->pc+2);
		h8->pc = address24;
		H8_IFETCH_TIMING(2);
		H8_STACK_TIMING(2);
		H8_IOP_TIMING(2);
		break;
		// jsr @aa:8
	default:
		logerror("H8: Unk. group 5 def %x\n", opcode);
		h8->h8err = 1;
		break;
	}
}

static void h8_group6(h83xx_state *h8, UINT16 opcode)
{
	switch((opcode>>8)&0xf)
	{
	case 0:case 1:case 2:case 3:
		{
			UINT8 bitnr;

			dstreg = opcode & 0xf;
			udata8 = h8_getreg8(h8, dstreg);
			bitnr = h8_getreg8(h8, (opcode>>4)& 0xf)&7;

			switch((opcode>>8)&0xf)
			{
			case 0:	udata8 = h8_bset8(h8, bitnr, udata8); h8_setreg8(h8, dstreg, udata8); H8_IFETCH_TIMING(1); break;
			case 2:	udata8 = h8_bclr8(h8, bitnr, udata8); h8_setreg8(h8, dstreg, udata8); H8_IFETCH_TIMING(1); break;
			case 3:	h8_btst8(h8, bitnr, udata8); H8_IFETCH_TIMING(1); break;
			default:
				logerror("H8/3xx: Unk. group 6 def 0-3-0 %x\n", opcode);
				h8->h8err = 1;
				break;
			}
		}
		break;
	case 0x4:
		// or.w rs, rd
		dstreg = opcode & 0xf;
		udata16 = h8_getreg16(h8, dstreg);
		udata16 = h8_or16(h8, h8_getreg16(h8, (opcode>>4) & 0xf), udata16);
		h8_setreg16(h8, dstreg, udata16);
		H8_IFETCH_TIMING(1);
		break;
	case 0x5:
		// xor.w rs, rd
		dstreg = opcode & 0xf;
		udata16 = h8_getreg16(h8, dstreg);
		udata16 = h8_xor16(h8, h8_getreg16(h8, (opcode>>4) & 0xf), udata16);
		h8_setreg16(h8, dstreg, udata16);
		H8_IFETCH_TIMING(1);
		break;
	case 0x6:
		// and.w rs, rd
		dstreg = opcode & 0xf;
		udata16 = h8_getreg16(h8, dstreg);
		udata16 = h8_and16(h8, h8_getreg16(h8, (opcode>>4) & 0xf), udata16);
		h8_setreg16(h8, dstreg, udata16);
		H8_IFETCH_TIMING(1)
		break;
	case 0x7:
		// bst/bist #imm, rd
		if(opcode & 0x80)
		{
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_bist8(h8, (opcode>>4) & 7, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
		}
		else
		{
			udata8 = h8_getreg8(h8, opcode & 0xf);
			udata8 = h8_bst8(h8, (opcode>>4) & 7, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
		}
		break;
	case 0x8:
		if(opcode & 0x80)
		{
			// mov.b rx, @rx
			udata8 = h8_getreg8(h8, opcode & 0xf);
			address24 = h8_getreg32(h8, (opcode>>4)&7) & H8_ADDR_MASK;
			h8_mov8(h8, udata8);
			h8_mem_write8(address24, udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
		}
		else
		{
			// mov.b @rx, rx
			address24 = h8_getreg32(h8, (opcode>>4)&7) & H8_ADDR_MASK;
			udata8 = h8_mem_read8(address24);
			h8_mov8(h8, udata8);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
		}
		break;
	case 0x9:
		if(opcode & 0x80)
		{
			// mov.w rx, @rx
			address24 = h8_getreg32(h8, (opcode>>4)&7) & H8_ADDR_MASK;
			udata16 = h8_getreg16(h8, opcode & 0xf);
			h8_mov16(h8, udata16);
			h8_mem_write16(h8, address24, udata16);
			H8_IFETCH_TIMING(1);
			H8_WORD_TIMING(1, address24);
		}
		else
		{
			// mov.w @rx, rx
			address24 = h8_getreg32(h8, (opcode>>4)&7) & H8_ADDR_MASK;
			udata16 = h8_mem_read16(h8, address24);
			h8_mov16(h8, udata16);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			H8_WORD_TIMING(1, address24);
		}
		break;
	case 0xa:
		// mov.b rx, @xx
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			sdata16=h8_mem_read16(h8, h8->pc);
			h8->pc += 2;
			address24 = sdata16 & H8_ADDR_MASK;
			udata8=h8_mem_read8(address24);
			h8_mov8(h8, udata8); // flags only
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
			break;
		case 0x2:
			address24=h8_mem_read32(h8, h8->pc);
			h8->pc += 4;
			udata8=h8_mem_read8(address24);
			h8_mov8(h8, udata8); // flags only
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(2);
			H8_BYTE_TIMING(1, address24);
			break;
		case 0x3:	// bclr #x, @imm:24
			address24=h8_mem_read32(h8, h8->pc);
			h8->pc += 4;
			udata16 = h8_mem_read16(h8, h8->pc);
			h8->pc += 2;
			switch ((udata16 >> 8) & 0xff)
			{
				case 0x60 :	// bset Rn, @abs:24
					udata8 = h8_mem_read8(address24);
					udata8 = h8_bset8(h8, h8_getreg8(h8, (udata16 >> 4) & 0xf), udata8);
					h8_mem_write8(address24, udata8);
					H8_BYTE_TIMING(2, address24);
					break;
				case 0x61 : // bnot Rn, @abs:24
					udata8 = h8_mem_read8(address24);
					udata8 = h8_bnot8(h8, h8_getreg8(h8, (udata16 >> 4) & 0xf), udata8);
					h8_mem_write8(address24, udata8);
					H8_BYTE_TIMING(2, address24);
					break;
				case 0x62 : // bclr Rn, @abs:24
					udata8 = h8_mem_read8(address24);
					udata8 = h8_bclr8(h8, h8_getreg8(h8, (udata16 >> 4) & 0xf), udata8);
					h8_mem_write8(address24, udata8);
					H8_IFETCH_TIMING(2);
					H8_BYTE_TIMING(2, address24);
					break;
				case 0x63 : // btst Rn, @abs:24
					udata8 = h8_mem_read8(address24);
					h8_btst8(h8, h8_getreg8(h8, (udata16 >> 4) & 0xf), udata8);
					h8_mem_write8(address24, udata8);
					H8_IFETCH_TIMING(2);
					H8_BYTE_TIMING(2, address24);
					break;
				case 0x70 : // bset #imm, @abs:24
					udata8 = h8_mem_read8(address24);
					udata8 = h8_bset8(h8, (udata16 >> 4) & 7, udata8);
					h8_mem_write8(address24, udata8);
					H8_IFETCH_TIMING(2);
					H8_BYTE_TIMING(2, address24);
					break;
				case 0x71 : // bnot #imm, @abs:24
					udata8 = h8_mem_read8(address24);
					udata8 = h8_bnot8(h8, (udata16 >> 4) & 7, udata8);
					h8_mem_write8(address24, udata8);
					H8_IFETCH_TIMING(2);
					H8_BYTE_TIMING(2, address24);
					break;
				case 0x72 : // bclr #imm, @abs:24
					udata8 = h8_mem_read8(address24);
					udata8 = h8_bclr8(h8, (udata16 >> 4) & 7, udata8);
					h8_mem_write8(address24, udata8);
					H8_IFETCH_TIMING(2);
					H8_BYTE_TIMING(2, address24);
					break;
				case 0x73 : // btst #imm, @abs:24
					udata8 = h8_mem_read8(address24);
					h8_btst8(h8, (udata16 >> 4) & 7, udata8);
					H8_IFETCH_TIMING(2);
					H8_BYTE_TIMING(1, address24);
					break;
			}
			break;
		case 0x8:
			sdata16=h8_mem_read16(h8, h8->pc);
			h8->pc += 2;
			address24 = sdata16 & H8_ADDR_MASK;
			udata8=h8_getreg8(h8, opcode & 0xf);
			h8_mov8(h8, udata8); // flags only
			h8_mem_write8(address24, udata8);
			H8_IFETCH_TIMING(3);
			H8_BYTE_TIMING(1, address24);
			break;
		case 0xa:
			address24=h8_mem_read32(h8, h8->pc);
			h8->pc += 4;
			udata8=h8_getreg8(h8, opcode & 0xf);
			h8_mov8(h8, udata8); // flags only
			h8_mem_write8(address24, udata8);
			H8_IFETCH_TIMING(3);
			H8_BYTE_TIMING(1, address24);
			break;
		default:
			logerror("H8/3xx: Unk. group 6 a %x\n", opcode);
			h8->h8err = 1;
			break;
		}
		break;
	case 0xb:
		// mov.w rx, @xx / mov.w @xx, rx
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			sdata16=h8_mem_read16(h8, h8->pc);
			address24 = sdata16;
			address24 &= H8_ADDR_MASK;
			h8->pc += 2;
			udata16 = h8_mem_read16(h8, address24);
			h8_mov16(h8, udata16); // flags only
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(2);
			H8_WORD_TIMING(1, address24);
			break;
		case 0x2:
			address24=h8_mem_read32(h8, h8->pc);
			h8->pc += 4;
			udata16=h8_mem_read16(h8, address24);
			h8_mov16(h8, udata16); // flags only
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(4);
			H8_WORD_TIMING(1, address24);
			break;
		case 0x8:
			sdata16=h8_mem_read16(h8, h8->pc);
			address24 = sdata16;
			address24 &= H8_ADDR_MASK;
			h8->pc += 2;
			udata16=h8_getreg16(h8, opcode & 0xf);
			h8_mov16(h8, udata16); // flags only
			h8_mem_write16(h8, address24, udata16);
			H8_IFETCH_TIMING(2);
			H8_WORD_TIMING(1, address24);
			break;
		case 0xa: // pass
			address24=h8_mem_read32(h8, h8->pc);
			h8->pc += 4;
			udata16=h8_getreg16(h8, opcode & 0xf);
			h8_mov16(h8, udata16); // flags only
			h8_mem_write16(h8, address24, udata16);
			H8_IFETCH_TIMING(4);
			H8_WORD_TIMING(1, address24);
			break;
		default:
			logerror("H8/3xx: Unk. group 6b %x\n", opcode);
			h8->h8err = 1;
			break;
		}
		break;
	case 0xc:
		if(opcode & 0x80)
		{
			// mov.b rx, @-erx
			srcreg = (opcode>>4)&7;
			h8_setreg32(h8, srcreg, h8_getreg32(h8, srcreg)-1);
			address24 = h8_getreg32(h8, srcreg) & H8_ADDR_MASK;
			udata8 = h8_getreg8(h8, opcode & 0xf);
			h8_mem_write8(address24, udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
			H8_IOP_TIMING(2);
		}
		else
		{
			// mov.b @erx+,rx
			srcreg = (opcode>>4)&7;
			address24 = h8_getreg32(h8, srcreg) & H8_ADDR_MASK;
			h8_setreg32(h8, srcreg, h8_getreg32(h8, srcreg)+1);
			udata8 = h8_mem_read8(address24);
			h8_setreg8(h8, opcode & 0xf, udata8);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
			H8_IOP_TIMING(2);
		}
		h8_mov8(h8, udata8);
		break;
	case 0xd:
		if(opcode & 0x80)
		{
			// mov.w rs, @-erd
			srcreg = (opcode>>4)&7;
			h8_setreg32(h8, srcreg, h8_getreg32(h8, srcreg)-2);
			address24 = h8_getreg32(h8, srcreg) & H8_ADDR_MASK;
			udata16 = h8_getreg16(h8, opcode & 0xf);
			h8_mem_write16(h8, address24, udata16);
			H8_IFETCH_TIMING(1);
			H8_WORD_TIMING(1, address24);
			H8_IOP_TIMING(2);
		}
		else
		{
			// mov.w @ers+, rd
			srcreg = (opcode>>4)&7;
			address24 = h8_getreg32(h8, srcreg) & H8_ADDR_MASK;
			h8_setreg32(h8, srcreg, h8_getreg32(h8, srcreg)+2);
			udata16 = h8_mem_read16(h8, address24);
			h8_setreg16(h8, opcode & 0xf, udata16);
			H8_IFETCH_TIMING(1);
			H8_BYTE_TIMING(1, address24);
			H8_IOP_TIMING(2);
		}
		h8_mov16(h8, udata16);
		break;
	case 0xe:
		// mov.b @(displ16 + Rs), rd
		sdata16=h8_mem_read16(h8, h8->pc); // sign extend displacements !
		h8->pc += 2;
		address24 = (h8_getreg32(h8, (opcode>>4)&7)) & H8_ADDR_MASK;
		address24 += sdata16;
		if(opcode & 0x80)
		{
			udata8 = h8_getreg8(h8, opcode & 0xf);
			h8_mem_write8(address24, udata8);
		}
		else
		{
			udata8 = h8_mem_read8(address24);
			h8_setreg8(h8, opcode & 0xf, udata8);
		}
		h8_mov8(h8, udata8);
		H8_IFETCH_TIMING(2);
		H8_BYTE_TIMING(1, address24);
		break;
	case 0xf:
		// mov.w @(displ16 + Rs), rd
		sdata16=h8_mem_read16(h8, h8->pc); // sign extend displacements !
		h8->pc += 2;
		address24 = (h8_getreg32(h8, (opcode>>4)&7)) & H8_ADDR_MASK;
		address24 += sdata16;
		if(opcode & 0x80)
		{
			udata16 = h8_getreg16(h8, opcode & 0xf);
			h8_mem_write16(h8, address24, udata16);
		}
		else
		{
			udata16 = h8_mem_read16(h8, address24);
			h8_setreg16(h8, opcode & 0xf, udata16);
		}
		h8_mov16(h8, udata16);
		H8_IFETCH_TIMING(2);
		H8_WORD_TIMING(1, address24);
		break;
	default:
		logerror("H8/3xx: Unk. group 6 def %x\n", opcode);
		h8->h8err = 1;
		break;
	}
}

static void h8_group7(h83xx_state *h8, UINT16 opcode)
{
	switch((opcode>>8)&0xf)
	{
	case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:
		{
			UINT8 bitnr;

			dstreg = opcode & 0xf;
			udata8 = h8_getreg8(h8, dstreg);
			bitnr = (opcode>>4)&7;

			if(((opcode>>4)&0x8) == 0)
			{
				switch((opcode>>8)&0x7)
				{
				case 0:	udata8 = h8_bset8(h8, bitnr, udata8); h8_setreg8(h8, dstreg, udata8); H8_IFETCH_TIMING(1); break;
				case 1:	udata8 = h8_bnot8(h8, bitnr, udata8); h8_setreg8(h8, dstreg, udata8); H8_IFETCH_TIMING(1); break;
				case 2:	udata8 = h8_bclr8(h8, bitnr, udata8); h8_setreg8(h8, dstreg, udata8);H8_IFETCH_TIMING(1);break;
				case 3:	h8_btst8(h8, bitnr, udata8); H8_IFETCH_TIMING(1); break;
				case 4:	h8_bor8(h8, bitnr, udata8); H8_IFETCH_TIMING(1); break;
				case 5:	h8_bxor8(h8, bitnr, udata8); H8_IFETCH_TIMING(1); break;
				case 6:	h8_band8(h8, bitnr, udata8); H8_IFETCH_TIMING(1); break;
				case 7:	h8_bld8(h8, bitnr, udata8); H8_IFETCH_TIMING(1); break;
				default:
					logerror("H8/3xx: Unk. group 7 0-7 def %x\n", opcode);
					h8->h8err = 1;
					break;
				}
			}
			else
			{
				switch((opcode>>8)&7)
				{
				case 7:	h8_bild8(h8, bitnr, udata8); H8_IFETCH_TIMING(1); break;
				default:
					logerror("H8/3xx: Unk. group 7 0-7-1 def %x\n", opcode);
					h8->h8err = 1;
					break;
				}
			}
		}
		break;
	case 0x8:
		ext16 = h8_mem_read16(h8, h8->pc);
		h8->pc += 2;
		udata32 = h8_mem_read32(h8, h8->pc);
		h8->pc += 4;

		if(((ext16>>8) & 0xf) == 0xa)
		{
			if(((ext16>>4) & 0xf) == 0xa)
			{
				udata8 = h8_getreg8(h8, ext16 & 0xf);
				h8_mov8(h8, udata8); // update flags !
				udata32 += h8_getreg32(h8, (opcode >> 4) & 7);
				h8_mem_write8(udata32, udata8);
			}
			else
			{
				udata32 += h8_getreg32(h8, (opcode >> 4) & 7);
				udata8 = h8_mem_read8(udata32);
				h8_mov8(h8, udata8); // update flags !
				h8_setreg8(h8, ext16 & 0xf, udata8);
			}
			H8_BYTE_TIMING(1, udata32);
			H8_IFETCH_TIMING(4);
		}
		else if ((ext16 & 0xfff0) == 0x6b20) // mov.w @(24-bit direct, rN), rM
		{
			udata32 += h8_getreg32(h8, (opcode >> 4) & 7);
			udata16 = h8_mem_read16(h8, udata32);
			h8_setreg16(h8, ext16 & 0xf, udata16);
			h8_mov16(h8, udata16); // update flags !
			H8_WORD_TIMING(1, udata32);
			H8_IFETCH_TIMING(4);
		}
		else if ((ext16 & 0xfff0) == 0x6ba0) // mov.w rM, @(24-bit direct, rN)
		{
			udata32 += h8_getreg32(h8, (opcode >> 4) & 7);
			udata16 = h8_getreg16(h8, ext16 & 0xf);
			h8_mem_write16(h8, udata32, udata16);
			h8_mov16(h8, udata16); // update flags !
			H8_WORD_TIMING(1, udata32);
			H8_IFETCH_TIMING(4);
		}
		else
		{
			logerror("H8/3xx: Unk. group 7 8 %x\n", opcode);
			h8->h8err = 1;
		}
		break;


		// xxx.w #aa:16, rd
	case 0x9:
		if( ((opcode>>4) & 0xf) > 0x6)
		{
			logerror("H8/3xx: Unk. group 7 9 %x\n", opcode);
			h8->h8err = 1;
		}
		else
		{
			UINT16 dst16;
			udata16 = h8_mem_read16(h8, h8->pc);
			h8->pc += 2;
			dstreg = opcode&0xf;
			dst16 = h8_getreg16(h8, dstreg);

			switch((opcode>>4)&7)
			{
			case 0:	dst16 = h8_mov16(h8, udata16); h8_setreg16(h8, dstreg, dst16); H8_IFETCH_TIMING(2);break;
			case 1: dst16 = h8_add16(h8, udata16, dst16); h8_setreg16(h8, dstreg, dst16); H8_IFETCH_TIMING(2); break;
			case 2: h8_cmp16(h8, udata16, dst16); H8_IFETCH_TIMING(2); break;
			case 3: dst16 = h8_sub16(h8, udata16, dst16); h8_setreg16(h8, dstreg, dst16); H8_IFETCH_TIMING(2); break;
			case 4: dst16 = h8_or16(h8, udata16, dst16); h8_setreg16(h8, dstreg, dst16); H8_IFETCH_TIMING(2); break;
			case 5: dst16 = h8_xor16(h8, udata16, dst16); h8_setreg16(h8, dstreg, dst16); H8_IFETCH_TIMING(2); break;
			case 6: dst16 = h8_and16(h8, udata16, dst16); h8_setreg16(h8, dstreg, dst16); H8_IFETCH_TIMING(2); break;
			default:
				logerror("H8/3xx: Unk. group 7 9 %x\n", opcode);
				h8->h8err = 1;
				break;
			}
		}
		break;
		// xxx.l #aa:32, erd
	case 0xa:
		if( (((opcode>>4) & 0xf) > 0x6) || (opcode & 0x8))
		{
			logerror("H8/3xx: Unk. group 7 a %x\n", opcode);
			h8->h8err = 1;
		}
		else
		{
			UINT32 dst32;
			udata32 = h8_mem_read32(h8, h8->pc);
			dstreg = opcode&0x7;
			h8->pc +=4;
			dst32 = h8_getreg32(h8, dstreg);

			switch((opcode>>4)&7)
			{
			case 0:	dst32 = h8_mov32(h8, udata32); h8_setreg32(h8, dstreg, dst32); H8_IFETCH_TIMING(3); break;
			case 1: dst32 = h8_add32(h8, udata32, dst32); h8_setreg32(h8, dstreg, dst32); H8_IFETCH_TIMING(3); break;
			case 2: h8_cmp32(h8, udata32, dst32); H8_IFETCH_TIMING(3); break;
			case 3: dst32 = h8_sub32(h8, udata32, dst32); h8_setreg32(h8, dstreg, dst32); H8_IFETCH_TIMING(3); break;
			case 4: dst32 = h8_or32(h8, udata32, dst32);  h8_setreg32(h8, dstreg, dst32); H8_IFETCH_TIMING(3); break;
			case 5: dst32 = h8_xor32(h8, udata32, dst32); h8_setreg32(h8, dstreg, dst32); H8_IFETCH_TIMING(3); break;
			case 6: dst32 = h8_and32(h8, udata32, dst32); h8_setreg32(h8, dstreg, dst32); H8_IFETCH_TIMING(3); break;
			default:
				logerror("H8/3xx: Unk. group 7 a2 %x\n", opcode);
				h8->h8err = 1;
				break;
			}
		}
		break;
		// eepmov
	case 0xb:
		switch (opcode & 0xff)
		{
			case 0xd4:	// eepmov.w
			{
				ext16 = h8_mem_read16(h8, h8->pc);
				h8->pc += 2;
				if (ext16 != 0x598f)
				{
					logerror("H8/3xx: Unk. eepmov form\n");
					h8->h8err = 1;
				}
				else
				{
					UINT16 cnt = h8_getreg16(h8, 4);

					H8_IFETCH_TIMING(2);
					H8_BYTE_TIMING((2*cnt)+2, h8->regs[5]);

					while (cnt > 0)
					{
						h8_mem_write8(h8->regs[6], h8_mem_read8(h8->regs[5]));
						h8->regs[5]++;
						h8->regs[6]++;
						cnt--;
					}
					h8_setreg16(h8, 4, 0);
				}
			}
			break;

			case 0x5c:	// eepmov.b
			{
				ext16 = h8_mem_read16(h8, h8->pc);
				h8->pc += 2;
				if (ext16 != 0x598f)
				{
					logerror("H8/3xx: Unk. eepmov form\n");
					h8->h8err = 1;
				}
				else
				{
					UINT8 cnt = h8_getreg8(h8, 8+4);

					H8_IFETCH_TIMING(2);
					H8_BYTE_TIMING((2*cnt)+2, h8->regs[5]);

					while (cnt > 0)
					{
						h8_mem_write8(h8->regs[6], h8_mem_read8(h8->regs[5]));
						h8->regs[5]++;
						h8->regs[6]++;
						cnt--;
					}
					h8_setreg8(h8, 8+4, 0);
				}
			}
			break;

			default:
				logerror("H8/3xx: Unk. eepmov form\n");
				h8->h8err = 1;
		}
		break;
		// bxx.b #xx:3, @rd
	case 0xc:
		{
			UINT8 bitnr;

			address24 = h8_getreg32(h8, (opcode>>4) & 0x7);
			udata8 = h8_mem_read8(address24);
			H8_BYTE_TIMING(1, address24);

			ext16 = h8_mem_read16(h8, h8->pc);
			h8->pc += 2;
			H8_IFETCH_TIMING(2);

			switch(ext16>>8)
			{
				// BTST Rn,@ERd
				case 0x63:
					srcreg = (ext16>>4)&0xf;
					bitnr = h8_getreg8(h8, srcreg)&7;
					h8_btst8(h8, bitnr, udata8);
					break;
				// btst.b #imm, @Rn
				case 0x73:
					bitnr = (ext16>>4)&7;
					h8_btst8(h8, bitnr, udata8);
					break;
				// bld.b #imm, @Rn
				case 0x77:
					udata16 = h8_mem_read16(h8, h8->pc);
					h8->pc += 2;
					dstreg = (opcode>>4) & 7;
					h8_bld8(h8, (udata16>>4) & 7, h8_mem_read8(h8_getreg16(h8, dstreg)));
					H8_IFETCH_TIMING(1);
					H8_WORD_TIMING(1, h8_getreg16(h8, dstreg));
					break;




				default:
					h8->h8err=1;
			}
		}
		break;
	case 0xd:
		ext16 = h8_mem_read16(h8, h8->pc);
		h8->pc += 2;
		address24 = h8_getreg32(h8, (opcode>>4) & 0x7);
		H8_IFETCH_TIMING(2);
		H8_BYTE_TIMING(1, address24);
		switch(ext16>>8)
		{
			// bset/bnot/bclr.b Rn, @ERd
			case 0x60:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x0f)!=0))	{	h8->h8err=1;	break;	}
				h8_mem_write8(address24, h8_bset8(h8, h8_getreg16(h8, (ext16>>4)&0xf)&7, h8_mem_read8(address24)));
				break;
			case 0x61:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x0f)!=0))	{	h8->h8err=1;	break;	}
				h8_mem_write8(address24, h8_bnot8(h8, h8_getreg16(h8, (ext16>>4)&0xf)&7, h8_mem_read8(address24)));
				break;
			case 0x62:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x0f)!=0))	{	h8->h8err=1;	break;	}
				h8_mem_write8(address24, h8_bclr8(h8, h8_getreg16(h8, (ext16>>4)&0xf)&7, h8_mem_read8(address24)));
				break;

			case 0x67:	// bst/bist.b #Imm:3, @ERd
				if (((opcode & 0x8f)!=0)||((ext16 & 0x0f)!=0))	{	h8->h8err=1;	break;	}
				if ((ext16 & 0x80)!=0)
				{
					h8_mem_write8(address24, h8_bist8(h8, (ext16>>4)&7, h8_mem_read8(address24)));
				}
				else
				{
					h8_mem_write8(address24, h8_bst8(h8, (ext16>>4)&7, h8_mem_read8(address24)));
				}
				break;

			// bset/bnot/bclr.b #Imm:3, @ERd
			case 0x70:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x8f)!=0))	{	h8->h8err=1;	break;	}
				h8_mem_write8(address24, h8_bset8(h8, (ext16>>4)&7, h8_mem_read8(address24)));
				break;
			case 0x71:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x8f)!=0))	{	h8->h8err=1;	break;	}
				h8_mem_write8(address24, h8_bnot8(h8, (ext16>>4)&7, h8_mem_read8(address24)));
				break;
			case 0x72:
				if (((opcode & 0x8f)!=0)||((ext16 & 0x8f)!=0))	{	h8->h8err=1;	break;	}
				h8_mem_write8(address24, h8_bclr8(h8, (ext16>>4)&7, h8_mem_read8(address24)));
				break;
		}
		break;

		// bxxx.b #imm, @aa:8
	case 0xe:
	case 0xf:
		{
			UINT8 bitnr=0;
			ext16 = h8_mem_read16(h8, h8->pc);
			h8->pc += 2;
			address24 = 0xffff00 + (opcode & 0xff);
			udata8 = h8_mem_read8(address24);

			switch((ext16>>8)&0xff)
			{
			case 0x30:
			case 0x60:
				bitnr = (ext16>>4)&7;
				udata8 = h8_bset8(h8, bitnr, udata8); h8_mem_write8(address24, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				break;
			case 0x70:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80) h8->h8err = 1;
				udata8 = h8_bset8(h8, bitnr, udata8); h8_mem_write8(address24, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				break;
			case 0x71:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80) h8->h8err = 1;
				udata8 = h8_bnot8(h8, bitnr, udata8); h8_mem_write8(address24, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				break;
			case 0x32:
			case 0x62:
				bitnr = h8_getreg8(h8, (ext16>>4)&0xf)&7;
				udata8 = h8_bclr8(h8, bitnr, udata8); h8_mem_write8(address24, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				break;
			case 0x72:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80) h8->h8err = 1;
				udata8 = h8_bclr8(h8, bitnr, udata8); h8_mem_write8(address24, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				break;
			case 0x63:
				bitnr = h8_getreg8(h8, (ext16>>4)&0xf)&7;
				h8_btst8(h8, bitnr, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(1, address24);
				break;
			case 0x73:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80) h8->h8err = 1;
				h8_btst8(h8, bitnr, udata8); H8_IFETCH_TIMING(2); H8_BYTE_TIMING(1, address24);
				break;
			case 0x74:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80)
				{
					// bior
					h8->h8err = 1;
				}
				else
				{
					h8_bor8(h8, bitnr, udata8);  H8_IFETCH_TIMING(2); H8_BYTE_TIMING(1, address24);
				}
				break;
			case 0x67:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80)
				{
					h8_bist8(h8, bitnr, udata8);  H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				}
				else
				{
					h8_bst8(h8, bitnr, udata8);  H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				}
				break;
			case 0x77:
				bitnr = (ext16>>4)&7;
				if(ext16&0x80)
				{
					h8_bild8(h8, bitnr, udata8);  H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				}
				else
				{
					h8_bld8(h8, bitnr, udata8);  H8_IFETCH_TIMING(2); H8_BYTE_TIMING(2, address24);
				}
				break;
			default:
				h8->h8err = 1;
				break;
			}
			if(h8->h8err)
				logerror("H8/3xx: Unk. group 7 e %x\n", opcode);
		}
		break;
	default:
		logerror("H8/3xx: Unk. group 7 def %x\n", opcode);
		h8->h8err = 1;
		break;
	}
}


static UINT8 h8_mov8(h83xx_state *h8, UINT8 src)
{
	// N and Z modified
	h8->h8nflag = (src>>7) & 1;
	h8->h8vflag = 0;

	// zflag
	if(src==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return src;
}

static UINT16 h8_mov16(h83xx_state *h8, UINT16 src)
{
	// N and Z modified
	h8->h8nflag = (src>>15) & 1;
	h8->h8vflag = 0;

	// zflag
	if(src==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}
	return src;
}

static UINT32 h8_mov32(h83xx_state *h8, UINT32 src)
{
	// N and Z modified
	h8->h8nflag = (src>>31) & 1;
	h8->h8vflag = 0;

	// zflag
	if(src==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return src;
}

static UINT8 h8_sub8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	UINT16 res;

	res = (UINT16)dst - src;
	// H,N,Z,V,C modified
	h8->h8nflag = (res>>7) & 1;
	h8->h8vflag = (((src^dst) & (res^dst))>>7) & 1;
	h8->h8cflag = (res >> 8) & 1;

	// zflag
	if((res&0xff)==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	h8->h8hflag = ((src^dst^res) & 0x10) ? 1 : 0;

	return res;
}

static UINT16 h8_sub16(h83xx_state *h8, UINT16 src, UINT16 dst)
{
	UINT32 res;

	res = (UINT32)dst - src;
	// H,N,Z,V,C modified
	h8->h8nflag = (res>>15) & 1;
	h8->h8vflag = (((src^dst) & (res^dst))>>15) & 1;
	h8->h8cflag = (res >> 16) & 1;
	//  h8->h8hflag = (res>>28) & 1;

	// zflag
	if((res&0xffff)==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	h8->h8hflag = ((src^dst^res) & 0x1000) ? 1 : 0;

	return res;
}

static UINT32 h8_sub32(h83xx_state *h8, UINT32 src, UINT32 dst)
{
	UINT64 res;

	res = (UINT64)dst - src;
	// H,N,Z,V,C modified
	h8->h8nflag = (res>>31) & 1;
	h8->h8vflag = (((src^dst) & (res^dst))>>31) & 1;
	h8->h8cflag = (res >> 32) & 1;
	//  h8->h8hflag = (res>>28) & 1;

	// zflag
	if((res&0xffffffff)==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	h8->h8hflag = ((src^dst^res) & 0x10000000) ? 1 : 0;

	return res;
}




static UINT8 h8_add8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	UINT16 res;

	res = (UINT16)src + dst;
	// H,N,Z,V,C modified
	h8->h8nflag = (res & 0x80) ? 1 : 0;
	h8->h8vflag = ((src^res) & (dst^res) & 0x80) ? 1 : 0;
	h8->h8cflag = (res & 0x100) ? 1 : 0;
	h8->h8zflag = (res & 0xff) ? 0 : 1;
	h8->h8hflag = ((src^dst^res) & 0x10) ? 1 : 0;

	return (UINT8)res;
}

static UINT16 h8_add16(h83xx_state *h8, UINT16 src, UINT16 dst)
{
	UINT32 res;

	res = (UINT32)src + dst;
	// H,N,Z,V,C modified
	h8->h8nflag = (res & 0x8000) ? 1 : 0;
	h8->h8vflag = ((src^res) & (dst^res) & 0x8000) ? 1 : 0;
	h8->h8cflag = (res & 0x10000) ? 1 : 0;
	h8->h8zflag = (res & 0xffff) ? 0 : 1;
	h8->h8hflag = ((src^dst^res) & 0x1000) ? 1 : 0;

	return res;
}

static UINT32 h8_add32(h83xx_state *h8, UINT32 src, UINT32 dst)
{
	UINT64 res;

	res = (UINT64)src + dst;
	// H,N,Z,V,C modified
	h8->h8nflag = (res & 0x80000000) ? 1 : 0;
	h8->h8vflag = (((src^res) & (dst^res)) & 0x80000000) ? 1 : 0;
	h8->h8cflag = ((res) & (((UINT64)1) << 32)) ? 1 : 0;
	h8->h8zflag = (res & 0xffffffff) ? 0 : 1;
	h8->h8hflag = ((src^dst^res) & 0x10000000) ? 1 : 0;

	return res;
}


static UINT8 h8_addx8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	UINT16 res;

	res = (UINT16)src + dst + h8->h8cflag;
	// H,N,Z,V,C modified
	h8->h8nflag = (res & 0x80) ? 1 : 0;
	h8->h8vflag = ((src^res) & (dst^res) & 0x80) ? 1 : 0;
	h8->h8cflag = (res >> 8) & 1;
	h8->h8hflag = ((src^dst^res) & 0x10) ? 1 : 0;
	h8->h8zflag = (res & 0xff) ? 0 : h8->h8zflag;

	return (UINT8)res;
}

static void h8_cmp8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	UINT16 res = (UINT16)dst - src;

	h8->h8cflag = (res & 0x100) ? 1 : 0;
	h8->h8vflag = (((dst) ^ (src)) & ((dst) ^ (res)) & 0x80) ? 1 : 0;
	h8->h8zflag = ((res & 0xff) == 0) ? 1 : 0;
	h8->h8nflag = (res & 0x80) ? 1 : 0;
	h8->h8hflag = ((src^dst^res) & 0x10) ? 1 : 0;
}

static void h8_cmp16(h83xx_state *h8, UINT16 src, UINT16 dst)
{
	UINT32 res = (UINT32)dst - src;

	h8->h8cflag = (res & 0x10000) ? 1 : 0;
	h8->h8vflag = (((dst) ^ (src)) & ((dst) ^ (res)) & 0x8000) ? 1 : 0;
	h8->h8zflag = ((res & 0xffff) == 0) ? 1 : 0;
	h8->h8nflag = (res & 0x8000) ? 1 : 0;
	h8->h8hflag = ((src^dst^res) & 0x1000) ? 1 : 0;
}

static void h8_cmp32(h83xx_state *h8, UINT32 src, UINT32 dst)
{
	UINT64 res = (UINT64)dst - src;

	h8->h8cflag = (res & (UINT64)U64(0x100000000)) ? 1 : 0;
	h8->h8vflag = (((dst) ^ (src)) & ((dst) ^ (res)) & 0x80000000) ? 1 : 0;
	h8->h8zflag = ((res & 0xffffffff) == 0) ? 1 : 0;
	h8->h8nflag = (res & 0x80000000) ? 1 : 0;
	h8->h8hflag = ((src^dst^res) & 0x10000000) ? 1 : 0;
}


static UINT8 h8_subx8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	UINT16 res;

	res = (UINT16)dst - src - ((h8->h8cflag) ? 1 : 0);
	// H,N,Z,V,C modified
	h8->h8nflag = (res>>7) & 1;
	h8->h8vflag = (((src^dst) & (res^dst))>>7) & 1;
	h8->h8cflag = (res >> 8) & 1;

	// zflag
	if((res&0xff)==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	h8->h8hflag = ((src^dst^res) & 0x10) ? 1 : 0;

	return res;
}

static UINT8 h8_or8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	UINT8 res;
	res = src | dst;

	h8->h8nflag = (res>>7) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_or16(h83xx_state *h8, UINT16 src, UINT16 dst)
{
	UINT16 res;
	res = src | dst;

	h8->h8nflag = (res>>15) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_or32(h83xx_state *h8, UINT32 src, UINT32 dst)
{
	UINT32 res;
	res = src | dst;

	h8->h8nflag = (res>>31) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT8 h8_xor8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	UINT8 res;
	res = src ^ dst;

	h8->h8nflag = (res>>7) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_xor16(h83xx_state *h8, UINT16 src, UINT16 dst)
{
	UINT16 res;
	res = src ^ dst;

	h8->h8nflag = (res>>15) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_xor32(h83xx_state *h8, UINT32 src, UINT32 dst)
{
	UINT32 res;
	res = src ^ dst;

	h8->h8nflag = (res>>31) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT8 h8_and8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	UINT8 res;

	res = src & dst;
	// N and Z modified
	h8->h8nflag = (res>>7) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_and16(h83xx_state *h8, UINT16 src, UINT16 dst)
{
	UINT16 res;

	res = src & dst;
	// N and Z modified
	h8->h8nflag = (res>>15) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_and32(h83xx_state *h8, UINT32 src, UINT32 dst)
{
	UINT32 res;

	res = src & dst;
	// N and Z modified
	h8->h8nflag = (res>>31) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static void h8_btst8(h83xx_state *h8, UINT8 bit, UINT8 dst)
{
	// test single bit and update Z flag
	if( (dst & (1<<bit)) == 0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}
}

static void h8_bld8(h83xx_state *h8, UINT8 bit, UINT8 dst)
{
	// load bit to carry
	h8->h8cflag = (dst >> bit) & 1;
}

static void h8_bild8(h83xx_state *h8, UINT8 bit, UINT8 dst)
{
	// load inverted bit to carry
	h8->h8cflag = ((~dst) >> bit) & 1;
}

static UINT8 h8_bnot8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	// invert single bit, no effect on C flag
	return dst ^ (1<<src);
}

static UINT8 h8_bst8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	UINT8 res;

	// store carry flag in bit position
	if(h8->h8cflag == 1)
	{
		res = dst | (1<<src);
	}
	else
	{
		res = dst & ~(1<<src); // mask off
	}
	return res;
}

static UINT8 h8_bist8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	UINT8 res;

	// store inverse of carry flag in bit position
	if(h8->h8cflag == 0)
	{
		res = dst | (1<<src);
	}
	else
	{
		res = dst & ~(1<<src); // mask off
	}
	return res;
}

static UINT8 h8_bset8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	// pass
	UINT8 res;
	res = dst | (1<<src);
	return res;
}

// does not affect result, res in C flag only
static void h8_bor8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	dst >>= src;
	dst &= 0x1;
	h8->h8cflag |= dst;
}

// does not affect result, res in C flag only
static void h8_band8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	dst >>= src;
	dst &= 0x1;
	h8->h8cflag &= dst;
}

// does not affect result, res in C flag only
static void h8_bxor8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	dst >>= src;
	dst &= 0x1;
	h8->h8cflag ^= dst;
}

static UINT8 h8_bclr8(h83xx_state *h8, UINT8 src, UINT8 dst)
{
	// pass
	UINT8 res;
	res = dst & ~(1<<src);
	return res;
}

static INT8 h8_neg8(h83xx_state *h8, INT8 src)
{
	INT8 res;

	if((UINT8)src == 0x80)
	{
		// overflow !
		h8->h8vflag = 1;
		res = (INT8)0x80;
	}
	else
	{
		h8->h8vflag = 0;
		res = 0-src;
	}

	// N and Z modified
	h8->h8nflag = (res>>7) & 1;
	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	h8->h8hflag = ((src|res)&0x08) ? 1 : 0;
	h8->h8cflag = ((src|res)&0x80) ? 1 : 0;

	return res;
}

static INT16 h8_neg16(h83xx_state *h8, INT16 src)
{
	INT16 res;

	if((UINT16)src == 0x8000)
	{
		// overflow !
		h8->h8vflag = 1;
		res = (INT16)0x8000;
	}
	else
	{
		h8->h8vflag = 0;
		res = 0-src;
	}

	// N and Z modified
	h8->h8nflag = (res>>15) & 1;
	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	h8->h8hflag = ((src|res)&0x0800) ? 1 : 0;
	h8->h8cflag = ((src|res)&0x8000) ? 1 : 0;

	return res;
}

static INT32 h8_neg32(h83xx_state *h8, INT32 src)
{
	INT32 res;

	if((UINT32)src == 0x80000000)
	{
		// overflow !
		h8->h8vflag = 1;
		res = 0x80000000;
	}
	else
	{
		h8->h8vflag = 0;
		res = 0-src;
	}

	// N and Z modified
	h8->h8nflag = (res>>31) & 1;
	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	h8->h8hflag = ((src|res)&0x08000000) ? 1 : 0;
	h8->h8cflag = ((src|res)&0x80000000) ? 1 : 0;

	return res;
}

static UINT8 h8_not8(h83xx_state *h8, UINT8 src)
{
	UINT8 res;

	res = ~src;

	// N and Z modified
	h8->h8nflag = (res>>7) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_not16(h83xx_state *h8, UINT16 src)
{
	UINT16 res;

	res = ~src;

	// N and Z modified
	h8->h8nflag = (res>>15) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_not32(h83xx_state *h8, UINT32 src)
{
	UINT32 res;

	res = ~src;

	// N and Z modified
	h8->h8nflag = (res>>31) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT8 h8_rotxr8(h83xx_state *h8, UINT8 src)
{
	UINT8 res;

	// rotate through carry right
	res = src>>1;
	if(h8->h8cflag)res |= 0x80; // put cflag in upper bit
	h8->h8cflag = src & 1;

	// N and Z modified
	h8->h8nflag = (res>>7) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT8 h8_rotr8(h83xx_state *h8, UINT8 src)
{
	UINT8 res;

	// rotate right, not through carry
	res = src>>1;
	if (src & 1) res |= 0x80; // put cflag in upper bit
	h8->h8cflag = src & 1;

	// N and Z modified
	h8->h8nflag = (res>>7) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_rotr16(h83xx_state *h8, UINT16 src)
{
	UINT16 res;

	// rotate right, not through carry
	res = src>>1;
	if (src & 1) res |= 0x8000; // put cflag in upper bit
	h8->h8cflag = src & 1;

	// N and Z modified
	h8->h8nflag = (res>>15) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_rotr32(h83xx_state *h8, UINT32 src)
{
	UINT32 res;

	// rotate right, not through carry
	res = src>>1;
	if (src & 1) res |= 0x80000000; // put cflag in upper bit
	h8->h8cflag = src & 1;

	// N and Z modified
	h8->h8nflag = (res>>31) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_rotxr16(h83xx_state *h8, UINT16 src)
{
	UINT16 res;

	// rotate through carry right
	res = src>>1;
	if(h8->h8cflag)res |= 0x8000; // put cflag in upper bit
	h8->h8cflag = src & 1;

	// N and Z modified
	h8->h8nflag = (res>>15) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_rotxr32(h83xx_state *h8, UINT32 src)
{
	UINT32 res;

	// rotate through carry right
	res = src>>1;
	if(h8->h8cflag)res |= 0x80000000; // put cflag in upper bit
	h8->h8cflag = src & 1;

	// N and Z modified
	h8->h8nflag = (res>>31) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT8 h8_rotxl8(h83xx_state *h8, UINT8 src)
{
	UINT8 res;

	// rotate through carry
	res = src<<1;
	res |= (h8->h8cflag & 1);
	h8->h8cflag = (src>>7) & 1;

	// N and Z modified
	h8->h8nflag = (res>>7) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_rotxl16(h83xx_state *h8, UINT16 src)
{
	UINT16 res;

	// rotate through carry
	res = src<<1;
	res |= (h8->h8cflag & 1);
	h8->h8cflag = (src>>15) & 1;

	// N and Z modified
	h8->h8nflag = (res>>15) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_rotxl32(h83xx_state *h8, UINT32 src)
{
	UINT32 res;

	// rotate through carry
	res = src<<1;
	res |= (h8->h8cflag & 1);
	h8->h8cflag = (src>>31) & 1;

	// N and Z modified
	h8->h8nflag = (res>>31) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}


static UINT8 h8_rotl8(h83xx_state *h8, UINT8 src)
{
	UINT8 res;

	// rotate
	res = src<<1;
	h8->h8cflag = (src>>7) & 1;
	res |= (h8->h8cflag & 1);

	// N and Z modified
	h8->h8nflag = (res>>7) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_rotl16(h83xx_state *h8, UINT16 src)
{
	UINT16 res;

	// rotate
	res = src<<1;
	h8->h8cflag = (src>>15) & 1;
	res |= (h8->h8cflag & 1);

	// N and Z modified
	h8->h8nflag = (res>>15) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_rotl32(h83xx_state *h8, UINT32 src)
{
	UINT32 res;

	// rotate
	res = src<<1;
	h8->h8cflag = (src>>31) & 1;
	res |= (h8->h8cflag & 1);

	// N and Z modified
	h8->h8nflag = (res>>31) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT8 h8_shll8(h83xx_state *h8, UINT8 src)
{
	UINT8 res;
	h8->h8cflag = (src>>7) & 1;
	res = src<<1;
	// N and Z modified
	h8->h8nflag = (res>>7) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_shll16(h83xx_state *h8, UINT16 src)
{
	UINT16 res;
	h8->h8cflag = (src>>15) & 1;
	res = src<<1;
	// N and Z modified
	h8->h8nflag = (res>>15) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_shll32(h83xx_state *h8, UINT32 src)
{
	UINT32 res;
	h8->h8cflag = (src>>31) & 1;
	res = src<<1;
	// N and Z modified
	h8->h8nflag = (res>>31) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT8 h8_shlr8(h83xx_state *h8, UINT8 src)
{
	UINT8 res;
	h8->h8cflag = src&1;
	res = src>>1;
	// N and Z modified
	h8->h8nflag = 0;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_shlr16(h83xx_state *h8, UINT16 src)
{
	UINT16 res;
	h8->h8cflag = src&1;
	res = src>>1;
	// N and Z modified
	h8->h8nflag = 0;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_shlr32(h83xx_state *h8, UINT32 src)
{
	UINT32 res;
	h8->h8cflag = src&1;
	res = src>>1;

	// N and Z modified, V always cleared
	h8->h8nflag = 0;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static INT8 h8_shar8(h83xx_state *h8, INT8 src)
{
	INT8 res;
	h8->h8cflag = src&1;
	res = (src>>1)|(src&0x80);
	// N and Z modified
	h8->h8nflag = (res>>7)&1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static INT16 h8_shar16(h83xx_state *h8, INT16 src)
{
	INT16 res;
	h8->h8cflag = src&1;
	res = (src>>1)|(src&0x8000);
	// N and Z modified
	h8->h8nflag = (res>>15)&1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static INT32 h8_shar32(h83xx_state *h8, INT32 src)
{
	INT32 res;

	h8->h8cflag = src&1;
	res = (src>>1)|(src&0x80000000);
	// N and Z modified
	h8->h8nflag = (res>>31) & 1;
	h8->h8vflag = 0;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static INT8 h8_shal8(h83xx_state *h8, INT8 src)
{
	INT8 res;

	h8->h8cflag = (src>>7)&1;
	res = src<<1;
	// N and Z modified
	h8->h8nflag = (res>>7)&1;
	h8->h8vflag = (src ^ res) >> 7;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static INT16 h8_shal16(h83xx_state *h8, INT16 src)
{
	INT16 res;

	h8->h8cflag = (src>>15)&1;
	res = src<<1;
	// N and Z modified
	h8->h8nflag = (res>>15)&1;
	h8->h8vflag = (src ^ res) >> 15;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static INT32 h8_shal32(h83xx_state *h8, INT32 src)
{
	INT32 res;

	h8->h8cflag = (src>>31)&1;
	res = src<<1;
	// N and Z modified
	h8->h8nflag = (res>>31)&1;
	h8->h8vflag = (src ^ res) >> 31;

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT8 h8_dec8(h83xx_state *h8, UINT8 src)
{
	UINT8 res;

	res = src - 1;
	// N and Z modified
	h8->h8nflag = (res>>7)&1;
	if(src == 0x80)
	{
		h8->h8vflag = 1;
	}
	else
	{
		h8->h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_dec16(h83xx_state *h8, UINT16 src)
{
	UINT16 res;

	res = src - 1;
	// N and Z modified
	h8->h8nflag = (res>>15)&1;
	if(src == 0x8000)
	{
		h8->h8vflag = 1;
	}
	else
	{
		h8->h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_dec32(h83xx_state *h8, UINT32 src)
{
	UINT32 res;

	res = src - 1;
	// N and Z modified
	h8->h8nflag = (res>>31)&1;
	if(src == 0x80000000)
	{
		h8->h8vflag = 1;
	}
	else
	{
		h8->h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT8 h8_inc8(h83xx_state *h8, UINT8 src)
{
	UINT8 res;

	res = src + 1;
	// N and Z modified
	h8->h8nflag = (res>>7)&1;
	if(src == 0x7f)
	{
		h8->h8vflag = 1;
	}
	else
	{
		h8->h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT16 h8_inc16(h83xx_state *h8, UINT16 src)
{
	UINT16 res;

	res = src + 1;
	// N and Z modified
	h8->h8nflag = (res>>15)&1;
	if(src == 0x7fff)
	{
		h8->h8vflag = 1;
	}
	else
	{
		h8->h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static UINT32 h8_inc32(h83xx_state *h8, UINT32 src)
{
	UINT32 res;

	res = src + 1;
	// N and Z modified
	h8->h8nflag = (res>>31)&1;
	if(src == 0x7fffffff)
	{
		h8->h8vflag = 1;
	}
	else
	{
		h8->h8vflag = 0;
	}

	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}

	return res;
}

static INT16 h8_mulxs8(h83xx_state *h8, INT8 src, INT8 dst)
{
	INT16 res;

	res = (INT16)src * dst;

	// N and Z modified
	h8->h8nflag = (res>>31)&1;
	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}
	return res;
}

static INT32 h8_mulxs16(h83xx_state *h8, INT16 src, INT16 dst)
{
	INT32 res;

	res = (INT32)src * dst;

	// N and Z modified
	h8->h8nflag = (res>>31)&1;
	// zflag
	if(res==0)
	{
		h8->h8zflag = 1;
	}
	else
	{
		h8->h8zflag = 0;
	}
	return res;
}

static UINT16 h8_divxs8(h83xx_state *h8, INT8 src, INT16 dst)
{
	// NOT tested !
	UINT16 res,r1,r2;
	INT8 remainder, quotient;

	if(src!=0)
	{
		quotient = dst/src;
		h8->h8zflag = 0;
	}
	else
	{
		quotient = 0;
		h8->h8zflag = 1;
	}
	remainder = dst%src;

	r1=*(&quotient);
	r2=*(&remainder);
	res=(r2<<8)|r1;

	h8->h8nflag = (quotient<0)?1:0;

	return res;
}

static UINT32 h8_divxs16(h83xx_state *h8, INT16 src, INT32 dst)
{
	// NOT tested !
	UINT32 res,r1,r2;
	INT16 remainder, quotient;

	if(src!=0)
	{
		quotient = dst/src;
		h8->h8zflag = 0;
	}
	else
	{
		quotient = 0;
		h8->h8zflag = 1;
	}
	remainder = dst%src;

	r1=*(&quotient);
	r2=*(&remainder);
	res=(r2<<16)|r1;

	h8->h8nflag = (quotient<0)?1:0;

	return res;
}

static UINT16 h8_divxu8(h83xx_state *h8, UINT16 dst, UINT8 src)
{
	UINT8 remainder, quotient;
	UINT16 res = 0;
	// N and Z modified
	h8->h8nflag = (src>>7)&1;
	// zflag
	if(src==0)
	{
		h8->h8zflag = 1;
		// dont do anything on division by zero !
	}
	else
	{
		h8->h8zflag = 0;
		quotient = dst / src;
		remainder = dst % src;
		res = (remainder << 8) | quotient;
	}
	return res;
}

static UINT32 h8_divxu16(h83xx_state *h8, UINT32 dst, UINT16 src)
{
	UINT16 remainder, quotient;
	UINT32 res = 0;
	// N and Z modified
	h8->h8nflag = (src>>15)&1;
	// zflag
	if(src==0)
	{
		h8->h8zflag = 1;
		// dont do anything on division by zero !
	}
	else
	{
		h8->h8zflag = 0;
		quotient = dst / src;
		remainder = dst % src;
		res = (remainder << 16) | quotient;
	}
	return res;
}

// input: branch condition
// output: 1 if condition met, 0 if not condition met
static int h8_branch(h83xx_state *h8, UINT8 condition)
{
	int taken = 0;

	// a branch always eats 2 ifetch states, regardless of if it's taken
	H8_IFETCH_TIMING(2)

	switch(condition)
	{
	case 0: // bt
		taken = 1;
		break;
	case 1: // bf
		break;
	case 2: // bhi (C | Z) == 0)
		if((h8->h8cflag | h8->h8zflag) == 0)taken = 1;
		break;
	case 3: // bls
		if((h8->h8cflag | h8->h8zflag) == 1)taken = 1;
		break;
	case 4: // bcc C = 0
		if(h8->h8cflag == 0)taken = 1;
		break;
	case 5: // bcs C = 1
		if(h8->h8cflag == 1)taken = 1;
		break;
	case 6: // bne Z = 0
		if(h8->h8zflag == 0)taken = 1;
		break;
	case 7: // beq Z = 1
		if(h8->h8zflag == 1)taken = 1;
		break;
	case 8: // bvc V = 0
		h8->h8err = 1;
		if(h8->h8vflag == 0)taken = 1;
		break;
	case 9: // bvs V = 1
		h8->h8err = 1;
		if(h8->h8vflag == 1)taken = 1;
		break;
	case 0xa: // bpl N = 0
		if(h8->h8nflag == 0)taken = 1;
		break;
	case 0xb: // bmi N = 1
		if(h8->h8nflag == 1)taken = 1;
		break;
	case 0xc: // bge (N ^ V) = 0
		if((h8->h8nflag ^ h8->h8vflag) == 0)taken = 1;
		break;
	case 0xd: // blt (N ^ V) = 1
		if((h8->h8nflag ^ h8->h8vflag) == 1)taken = 1;
		break;
	case 0xe: // bgt (Z | (N ^ V)) = 0
		if((h8->h8zflag | (h8->h8nflag ^ h8->h8vflag)) == 0)taken = 1;
		break;
	case 0xf: // ble (Z | (N ^ V)) = 1
		if((h8->h8zflag | (h8->h8nflag ^ h8->h8vflag)) == 1)taken = 1;
		break;
	}
	return taken;
}

