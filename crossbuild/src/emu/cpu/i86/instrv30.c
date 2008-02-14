static void PREFIXV30(_0fpre) (void)	/* Opcode 0x0f */
{
	unsigned Opcode = FETCH;
	unsigned ModRM;
	unsigned tmp;
	unsigned tmp2;

	switch (Opcode)
	{
	case 0x10:							/* 0F 10 47 30 - TEST1 [bx+30h],cl */
		ModRM = FETCH;
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			nec_ICount -= 3;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			nec_ICount = old - 12;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = I.regs.b[CL] & 0x7;
		I.ZeroVal = tmp & bytes[tmp2] ? 1 : 0;
/*      SetZF(tmp & (1<<tmp2)); */
		break;

	case 0x11:							/* 0F 11 47 30 - TEST1 [bx+30h],cl */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.w[Mod_RM.RM.w[ModRM]];
			nec_ICount -= 3;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadWord(EA);
			nec_ICount = old - 12;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = I.regs.b[CL] & 0xF;
		I.ZeroVal = tmp & bytes[tmp2] ? 1 : 0;
/*      SetZF(tmp & (1<<tmp2)); */
		break;

	case 0x12:							/* 0F 12 [mod:000:r/m] - CLR1 reg/m8,cl */
		ModRM = FETCH;
		/* need the long if due to correct cycles OB[19.07.99] */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			nec_ICount -= 5;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			nec_ICount = old - 14;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = I.regs.b[CL] & 0x7;		/* hey its a Byte so &07 NOT &0f */
		tmp &= ~(bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x13:							/* 0F 13 [mod:000:r/m] - CLR1 reg/m16,cl */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.w[Mod_RM.RM.w[ModRM]];
			nec_ICount -= 5;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadWord(EA);
			nec_ICount = old - 14;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = I.regs.b[CL] & 0xF;		/* this time its a word */
		tmp &= ~(bytes[tmp2]);
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x14:							/* 0F 14 47 30 - SET1 [bx+30h],cl */
		ModRM = FETCH;
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			nec_ICount -= 4;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			nec_ICount = old - 13;
		}
		tmp2 = I.regs.b[CL] & 0x7;
		tmp |= (bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x15:							/* 0F 15 C6 - SET1 si,cl */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.w[Mod_RM.RM.w[ModRM]];
			nec_ICount -= 4;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadWord(EA);
			nec_ICount = old - 13;
		}
		tmp2 = I.regs.b[CL] & 0xF;
		tmp |= (bytes[tmp2]);
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x16:							/* 0F 16 C6 - NOT1 si,cl */
		ModRM = FETCH;
		/* need the long if due to correct cycles OB[19.07.99] */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			nec_ICount -= 4;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			nec_ICount = old - 18;		/* my source says 18 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = I.regs.b[CL] & 0x7;		/* hey its a Byte so &07 NOT &0f */
		if (tmp & bytes[tmp2])
			tmp &= ~(bytes[tmp2]);
		else
			tmp |= (bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x17:							/* 0F 17 C6 - NOT1 si,cl */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.w[Mod_RM.RM.w[ModRM]];
			nec_ICount -= 4;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadWord(EA);
			nec_ICount = old - 18;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = I.regs.b[CL] & 0xF;		/* this time its a word */
		if (tmp & bytes[tmp2])
			tmp &= ~(bytes[tmp2]);
		else
			tmp |= (bytes[tmp2]);
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x18:							/* 0F 18 XX - TEST1 [bx+30h],07 */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			nec_ICount -= 4;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			nec_ICount = old - 13;		/* my source says 15 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0xF;
		I.ZeroVal = tmp & (bytes[tmp2]) ? 1 : 0;
/*      SetZF(tmp & (1<<tmp2)); */
		break;

	case 0x19:							/* 0F 19 XX - TEST1 [bx+30h],07 */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.w[Mod_RM.RM.w[ModRM]];
			nec_ICount -= 4;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadWord(EA);
			nec_ICount = old - 13;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		I.ZeroVal = tmp & (bytes[tmp2]) ? 1 : 0;
/*      SetZF(tmp & (1<<tmp2)); */
		break;

	case 0x1a:							/* 0F 1A 06 - CLR1 si,cl */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			nec_ICount -= 6;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			nec_ICount = old - 15;		/* my source says 15 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0x7;
		tmp &= ~(bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x1B:							/* 0F 1B 06 - CLR1 si,cl */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.w[Mod_RM.RM.w[ModRM]];
			nec_ICount -= 6;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadWord(EA);
			nec_ICount = old - 15;		/* my source says 15 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0xF;
		tmp &= ~(bytes[tmp2]);
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x1C:							/* 0F 1C 47 30 - SET1 [bx+30h],cl */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			nec_ICount -= 5;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			nec_ICount = old - 14;		/* my source says 15 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0x7;
		tmp |= (bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x1D:							/* 0F 1D C6 - SET1 si,cl */
		/* logerror("PC=%06x : Set1 ",activecpu_get_pc()-2); */
		ModRM = FETCH;
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.w[Mod_RM.RM.w[ModRM]];
			nec_ICount -= 5;
			/* logerror("reg=%04x ->",tmp); */
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();			/* calculate EA */
			tmp = ReadWord(EA);			/* read from EA */
			nec_ICount = old - 14;
			/* logerror("[%04x]=%04x ->",EA,tmp); */
		}
		tmp2 = FETCH;
		tmp2 &= 0xF;
		tmp |= (bytes[tmp2]);
		/* logerror("%04x",tmp); */
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x1e:							/* 0F 1e C6 - NOT1 si,07 */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			nec_ICount -= 5;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			nec_ICount = old - 19;
		}
		tmp2 = FETCH;
		tmp2 &= 0x7;
		if (tmp & bytes[tmp2])
			tmp &= ~(bytes[tmp2]);
		else
			tmp |= (bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x1f:							/* 0F 1f C6 - NOT1 si,07 */
		ModRM = FETCH;
		//tmp = GetRMWord(ModRM);
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.w[Mod_RM.RM.w[ModRM]];
			nec_ICount -= 5;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadWord(EA);
			nec_ICount = old - 19;		/* my source says 15 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0xF;
		if (tmp & bytes[tmp2])
			tmp &= ~(bytes[tmp2]);
		else
			tmp |= (bytes[tmp2]);
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x20:							/* 0F 20 59 - add4s */
		{
			/* length in words ! */
			int count = (I.regs.b[CL] + 1) / 2;
			int i;
			unsigned di = I.regs.w[DI];
			unsigned si = I.regs.w[SI];

			I.ZeroVal = 1;
			I.CarryVal = 0;				/* NOT ADC */
			for (i = 0; i < count; i++)
			{
				int v1, v2;
				int result;

				tmp = GetMemB(DS, si);
				tmp2 = GetMemB(ES, di);

				v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf);
				result = v1 + v2 + I.CarryVal;
				I.CarryVal = result > 99 ? 1 : 0;
				result = result % 100;
				v1 = ((result / 10) << 4) | (result % 10);
				PutMemB(ES, di, v1);
				if (v1)
					I.ZeroVal = 0;
				si++;
				di++;
			}
			I.OverVal = I.CarryVal;
			nec_ICount -= 7 + 19 * count;	/* 7+19n, n #operand words */
		}
		break;

	case 0x22:							/* 0F 22 59 - sub4s */
		{
			int count = (I.regs.b[CL] + 1) / 2;
			int i;
			unsigned di = I.regs.w[DI];
			unsigned si = I.regs.w[SI];

			I.ZeroVal = 1;
			I.CarryVal = 0;				/* NOT ADC */
			for (i = 0; i < count; i++)
			{
				int v1, v2;
				int result;

				tmp = GetMemB(ES, di);
				tmp2 = GetMemB(DS, si);

				v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf);
				if (v1 < (v2 + I.CarryVal))
				{
					v1 += 100;
					result = v1 - (v2 + I.CarryVal);
					I.CarryVal = 1;
				}
				else
				{
					result = v1 - (v2 + I.CarryVal);
					I.CarryVal = 0;
				}
				v1 = ((result / 10) << 4) | (result % 10);
				PutMemB(ES, di, v1);
				if (v1)
					I.ZeroVal = 0;
				si++;
				di++;
			}
			I.OverVal = I.CarryVal;
			nec_ICount -= 7 + 19 * count;
		}
		break;

	case 0x25:
		/*
         * ----------O-MOVSPA---------------------------------
         * OPCODE MOVSPA     -  Move Stack Pointer After Bank Switched
         *
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Type of Instruction: System
         *
         * Instruction:  MOVSPA
         *
         * Description:  This instruction transfer   both SS and SP  of the old register
         * bank to new register bank after the bank has been switched by
         * interrupt or BRKCS instruction.
         *
         * Flags Affected:   None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   MOVSPA
         * COP (Code of Operation)   : 0Fh 25h
         *
         * Clocks:   16
         */
		logerror("PC=%06x : MOVSPA\n", activecpu_get_pc() - 2);
		nec_ICount -= 16;
		break;
	case 0x26:							/* 0F 22 59 - cmp4s */
		{
			int count = (I.regs.b[CL] + 1) / 2;
			int i;
			unsigned di = I.regs.w[DI];
			unsigned si = I.regs.w[SI];

			I.ZeroVal = 1;
			I.CarryVal = 0;				/* NOT ADC */
			for (i = 0; i < count; i++)
			{
				int v1, v2;
				int result;

				tmp = GetMemB(ES, di);
				tmp2 = GetMemB(DS, si);

				v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf);
				if (v1 < (v2 + I.CarryVal))
				{
					v1 += 100;
					result = v1 - (v2 + I.CarryVal);
					I.CarryVal = 1;
				}
				else
				{
					result = v1 - (v2 + I.CarryVal);
					I.CarryVal = 0;
				}
				v1 = ((result / 10) << 4) | (result % 10);
/*              PutMemB(ES, di,v1); */	/* no store, only compare */
				if (v1)
					I.ZeroVal = 0;
				si++;
				di++;
			}
			I.OverVal = I.CarryVal;
			nec_ICount -= 7 + 19 * (I.regs.b[CL] + 1);	// 7+19n, n #operand bytes
		}
		break;
	case 0x28:							/* 0F 28 C7 - ROL4 bh */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			nec_ICount -= 25;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			nec_ICount = old - 28;
		}
		tmp <<= 4;
		tmp |= I.regs.b[AL] & 0xF;
		I.regs.b[AL] = (I.regs.b[AL] & 0xF0) | ((tmp >> 8) & 0xF);
		tmp &= 0xff;
		PutbackRMByte(ModRM, tmp);
		break;

        /* Is this a REAL instruction?? */
	case 0x29:							/* 0F 29 C7 - ROL4 bx */

		ModRM = FETCH;
		/*
         * if (ModRM >= 0xc0)
         * {
         *     tmp=I.regs.w[Mod_RM.RM.w[ModRM]];
         *     nec_ICount-=29;
         * }
         * else
         * {
         *     int old=nec_ICount;
         *     (*GetEA[ModRM])();
         *     tmp=ReadWord(EA);
         *     nec_ICount=old-33;
         * }
         * tmp <<= 4;
         * tmp |= I.regs.b[AL] & 0xF;
         * I.regs.b[AL] = (I.regs.b[AL] & 0xF0) | ((tmp>>8)&0xF);
         * tmp &= 0xffff;
         * PutbackRMWord(ModRM,tmp);
         */
		logerror("PC=%06x : ROL4 %02x\n", activecpu_get_pc() - 3, ModRM);
		break;

	case 0x2A:							/* 0F 2a c2 - ROR4 bh */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			nec_ICount -= 29;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			nec_ICount = old - 33;
		}
		tmp2 = (I.regs.b[AL] & 0xF) << 4;
		I.regs.b[AL] = (I.regs.b[AL] & 0xF0) | (tmp & 0xF);
		tmp = tmp2 | (tmp >> 4);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x2B:							// 0F 2b c2 - ROR4 bx
		ModRM = FETCH;
		/*
         * /* tmp = GetRMWord(ModRM);
         * if (ModRM >= 0xc0)
         * {
         *     tmp=I.regs.w[Mod_RM.RM.w[ModRM]];
         *     nec_ICount-=29;
         * }
         * else {
         *     int old=nec_ICount;
         *     (*GetEA[ModRM])();
         *     tmp=ReadWord(EA);
         *     nec_ICount=old-33;
         * }
         * tmp2 = (I.regs.b[AL] & 0xF)<<4;
         * I.regs.b[AL] = (I.regs.b[AL] & 0xF0) | (tmp&0xF);
         * tmp = tmp2 | (tmp>>4);
         * PutbackRMWord(ModRM,tmp);
         */
		logerror("PC=%06x : ROR4 %02x\n", activecpu_get_pc() - 3, ModRM);
		break;

	case 0x2D:							/* 0Fh 2Dh <1111 1RRR> */
		/* OPCODE BRKCS  -   Break with Contex Switch
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Description:
         *
         * Perform a High-Speed Software Interrupt with contex-switch to
         * register bank indicated by the lower 3-bits of 'bank'.
         *
         * Info:    NEC V25/V35/V25 Plus/V35 Plus Bank System
         *
         * This Chips have   8 32bytes register banks, which placed in
         * Internal chip RAM by addresses:
         * xxE00h..xxE1Fh Bank 0
         * xxE20h..xxE3Fh Bank 1
         * .........
         * xxEC0h..xxEDFh Bank 6
         * xxEE0h..xxEFFh Bank 7
         * xxF00h..xxFFFh Special Functions Register
         * Where xx is Value of IDB register.
         * IBD is Byte Register contained Internal data area base
         * IBD addresses is FFFFFh and xxFFFh where xx is data in IBD.
         *
         * Format of Bank:
         * +0   Reserved
         * +2   Vector PC
         * +4   Save   PSW
         * +6   Save   PC
         * +8   DS0     ;DS
         * +A   SS      ;SS
         * +C   PS      ;CS
         * +E   DS1     ;ES
         * +10  IY      ;DI
         * +11  IX      ;SI
         * +14  BP      ;BP
         * +16  SP      ;SP
         * +18  BW      ;BX
         * +1A  DW      ;DX
         * +1C  CW      ;CX
         * +1E  AW      ;AX
         *
         * Format of V25 etc. PSW (FLAGS):
         * Bit  Description
         * 15   1
         * 14   RB2 \
         * 13   RB1  >  Current Bank Number
         * 12   RB0 /
         * 11   V   ;OF
         * 10   IYR ;DF
         * 9    IE  ;IF
         * 8    BRK ;TF
         * 7    S   ;SF
         * 6    Z   ;ZF
         * 5    F1  General Purpose user flag #1 (accessed by Flag Special Function Register)
         * 4    AC  ;AF
         * 3    F0  General purpose user flag #0 (accessed by Flag Special Function Register)
         * 2    P   ;PF
         * 1    BRKI    I/O Trap Enable Flag
         * 0    CY  ;CF
         *
         * Flags Affected:   None
         */
		ModRM = FETCH;
		logerror("PC=%06x : BRKCS %02x\n", activecpu_get_pc() - 3, ModRM);
		nec_ICount -= 15;				/* checked ! */
		break;

	case 0x31:							/* 0F 31 [mod:reg:r/m] - INS reg8,reg8 or INS reg8,imm4 */
		ModRM = FETCH;
		logerror("PC=%06x : INS ", activecpu_get_pc() - 2);
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			logerror("ModRM=%04x \n", ModRM);
			nec_ICount -= 29;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			logerror("ModRM=%04x  Byte=%04x\n", EA, tmp);
			nec_ICount = old - 33;
		}

		/* more to come
         * bfl=tmp2 & 0xf;      /* bit field length
         * bfs=tmp & 0xf;       /* bit field start (bit offset in DS:SI)
         * I.regs.b[AH] =0;     /* AH =0
         */

		/* 2do: the rest is silence....yet
         * ----------O-INS------------------------------------
         * OPCODE INS  -  Insert Bit String
         *
         * CPU: NEC/Sony  all V-series
         * Type of Instruction: User
         *
         * Instruction:  INS  start,len
         *
         * Description:
         *
         * BitField [        BASE =  ES:DI
         * START BIT OFFSET =  start
         * LENGTH =  len
         * ]   <-    AX [ bits= (len-1)..0]
         *
         * Note:    di and start automatically UPDATE
         * Note:    Alternative Name of this instruction is NECINS
         *
         * Flags Affected: None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form         : INS  reg8,reg8
         * COP (Code of Operation)   : 0FH 31H  PostByte
         */

		/* nec_ICount-=31; */			/* 31 -117 clocks ....*/
		break;

    case 0x33:                          /* 0F 33 [mod:reg:r/m] - EXT reg8,reg8 or EXT reg8,imm4 */
		ModRM = FETCH;
		logerror("PC=%06x : EXT ", activecpu_get_pc() - 2);
		if (ModRM >= 0xc0)
		{
			tmp = I.regs.b[Mod_RM.RM.b[ModRM]];
			logerror("ModRM=%04x \n", ModRM);
			nec_ICount -= 29;
		}
		else
		{
			int old = nec_ICount;

			(*GetEA[ModRM]) ();
			tmp = ReadByte(EA);
			logerror("ModRM=%04x  Byte=%04x\n", EA, tmp);
			nec_ICount = old - 33;
		}
		/* 2do: the rest is silence....yet
        /*
         * bfl=tmp2 & 0xf;      /* bit field length
         * bfs=tmp & 0xf;       /* bit field start (bit offset in DS:SI)
         * I.regs.b[AH] =0;     /* AH =0
         */

		/*
         *
         * ----------O-EXT------------------------------------
         * OPCODE EXT  -  Extract Bit Field
         *
         * CPU: NEC/Sony all  V-series
         * Type of Instruction: User
         *
         * Instruction:  EXT  start,len
         *
         * Description:
         *
         * AX <- BitField [
         *     BASE =  DS:SI
         *     START BIT OFFSET =  start
         *     LENGTH =  len
         * ];
         *
         * Note:    si and start automatically UPDATE
         *
         * Flags Affected: None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form         : EXT  reg8,reg8
         * COP (Code of Operation)   : 0FH 33H  PostByte
         *
         * Clocks:      EXT  reg8,reg8
         * NEC V20: 26-55
         */

		/* NEC_ICount-=26; */			/* 26 -55 clocks ....*/
		break;

    case 0x91:
		/*
         * ----------O-RETRBI---------------------------------
         * OPCODE RETRBI     -  Return from Register Bank Context
         * Switch  Interrupt.
         *
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Type of Instruction: System
         *
         * Instruction:  RETRBI
         *
         * Description:
         *
         * PC  <- Save PC;
         * PSW <- Save PSW;
         *
         * Flags Affected:   All
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   RETRBI
         * COP (Code of Operation)   : 0Fh 91h
         *
         * Clocks:   12
         */
		logerror("PC=%06x : RETRBI\n", activecpu_get_pc() - 2);
		nec_ICount -= 12;
		break;

	case 0x94:
		/*
         * ----------O-TSKSW----------------------------------
         * OPCODE TSKSW  -    Task Switch
         *
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Type of Instruction: System
         *
         * Instruction:  TSKSW   reg16
         *
         * Description:  Perform a High-Speed task switch to the register bank indicated
         * by lower 3 bits of reg16. The PC and PSW are saved in the old
         * banks. PC and PSW save Registers and the new PC and PSW values
         * are retrived from the new register bank's save area.
         *
         * Note:         See BRKCS instruction for more Info about banks.
         *
         * Flags Affected:   All
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   TSCSW reg16
         * COP (Code of Operation)   : 0Fh 94h <1111 1RRR>
         *
         * Clocks:   11
         */
		ModRM = FETCH;

		logerror("PC=%06x : TSCSW %02x\n", activecpu_get_pc() - 3, ModRM);
		nec_ICount -= 11;
		break;

    case 0x95:
		/*
         * ----------O-MOVSPB---------------------------------
         * OPCODE MOVSPB     -  Move Stack Pointer Before Bamk Switching
         *
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Type of Instruction: System
         *
         * Instruction:  MOVSPB  Number_of_bank
         *
         * Description:  The MOVSPB instruction transfers the current SP and SS before
         * the bank switching to new register bank.
         *
         * Note:          New Register Bank Number indicated by lower 3bit of Number_of_
         * _bank.
         *
         * Note:          See BRKCS instruction for more info about banks.
         *
         * Flags Affected:   None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   MOVSPB    reg16
         * COP (Code of Operation)   : 0Fh 95h <1111 1RRR>
         *
         * Clocks:   11
         */
		ModRM = FETCH;
		logerror("PC=%06x : MOVSPB %02x\n", activecpu_get_pc() - 3, ModRM);
		nec_ICount -= 11;
		break;

    case 0xbe:
		/*
         * ----------O-STOP-----------------------------------
         * OPCODE STOP    -  Stop CPU
         *
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Type of Instruction: System
         *
         * Instruction:  STOP
         *
         * Description:
         * PowerDown instruction, Stop Oscillator,
         * Halt CPU.
         *
         * Flags Affected:   None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   STOP
         * COP (Code of Operation)   : 0Fh BEh
         *
         * Clocks:   N/A
         */
		logerror("PC=%06x : STOP\n", activecpu_get_pc() - 2);
		nec_ICount -= 2;				/* of course this is crap */
		break;

    case 0xe0:
		/*
         * ----------O-BRKXA----------------------------------
         * OPCODE BRKXA   -  Break to Expansion Address
         *
         * CPU:  NEC V33/V53  only
         * Type of Instruction: System
         *
         * Instruction:  BRKXA int_vector
         *
         * Description:
         * [sp-1,sp-2] <- PSW       ; PSW EQU FLAGS
         * [sp-3,sp-4] <- PS        ; PS  EQU CS
         * [sp-5,sp-6] <- PC        ; PC  EQU IP
         * SP    <-  SP -6
         * IE    <-  0
         * BRK <-  0
         * MD    <-  0
         * PC    <- [int_vector*4 +0,+1]
         * PS    <- [int_vector*4 +2,+3]
         * Enter Expansion Address Mode.
         *
         * Note:    In NEC V53 Memory Space dividing into 1024 16K pages.
         * The programming model is Same as in Normal mode.
         *
         * Mechanism is:
         * 20 bit Logical Address:   19..14 Page Num  13..0 Offset
         *
         * page Num convertin by internal table to 23..14 Page Base
         * tHE pHYIXCAL ADDRESS is both Base and Offset.
         *
         * Address Expansion Registers:
         * logical Address A19..A14 I/O Address
         * 0                FF00h
         * 1                FF02h
         * ...              ...
         * 63               FF7Eh
         *
         * Register XAM aliased with port # FF80h indicated current mode
         * of operation.
         * Format of XAM register (READ ONLY):
         * 15..1    reserved
         * 0    XA Flag, if=1 then in XA mode.
         *
         * Format   of  V53 PSW:
         * 15..12   1
         * 11   V
         * 10   IYR
         * 9    IE
         * 8    BRK
         * 7    S
         * 6    Z
         * 5    0
         * 4    AC
         * 3    0
         * 2    P
         * 1    1
         * 0    CY
         *
         * Flags Affected:   None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   BRKXA  imm8
         * COP (Code of Operation)   : 0Fh E0h imm8
         */

		ModRM = FETCH;
		logerror("PC=%06x : BRKXA %02x\n", activecpu_get_pc() - 3, ModRM);
		nec_ICount -= 12;
		break;

    case 0xf0:
		/*
         * ----------O-RETXA----------------------------------
         * OPCODE RETXA   -  Return from  Expansion Address
         *
         * CPU:  NEC V33/V53 only
         * Type of Instruction: System
         *
         * Instruction:  RETXA int_vector
         *
         * Description:
         * [sp-1,sp-2] <- PSW       ; PSW EQU FLAGS
         * [sp-3,sp-4] <- PS        ; PS  EQU CS
         * [sp-5,sp-6] <- PC        ; PC  EQU IP
         * SP    <-  SP -6
         * IE    <-  0
         * BRK <-  0
         * MD    <-  0
         * PC    <- [int_vector*4 +0,+1]
         * PS    <- [int_vector*4 +2,+3]
         * Disable EA mode.
         *
         * Flags Affected:   None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   RETXA  imm8
         * COP (Code of Operation)   : 0Fh F0h imm8
         *
         * Clocks:   12
         */
		ModRM = FETCH;
		logerror("PC=%06x : RETXA %02x\n", activecpu_get_pc() - 3, ModRM);
		nec_ICount -= 12;
		break;

    case 0xff:                          /* 0F ff imm8 - BRKEM */
		/*
         * OPCODE BRKEM  -   Break for Emulation
         *
         * CPU: NEC/Sony V20/V30/V40/V50
         * Description:
         *
         * PUSH FLAGS
         * PUSH CS
         * PUSH IP
         * MOV  CS,0:[intnum*4+2]
         * MOV  IP,0:[intnum*4]
         * MD <- 0; // Enable 8080 emulation
         *
         * Note:
         * BRKEM instruction do software interrupt and then New CS,IP loaded
         * it switch to 8080 mode i.e. CPU will execute 8080 code.
         * Mapping Table of Registers in 8080 Mode
         * 8080 Md.   A  B   C  D  E  H  L  SP PC  F
         * native.     AL CH CL DH DL BH BL BP IP  FLAGS(low)
         * For Return of 8080 mode use CALLN instruction.
         * Note:    I.e. 8080 addressing only 64KB then "Real Address" is CS*16+PC
         *
         * Flags Affected: MD
         */
		ModRM = FETCH;
		nec_ICount -= 38;
		logerror("PC=%06x : BRKEM %02x\n", activecpu_get_pc() - 3, ModRM);
		PREFIXV30(_interrupt) (ModRM, 1);
		break;
	}
}

static void PREFIXV30(_brkn) (void)		/* Opcode 0x63 BRKN -  Break to Native Mode */
{
	/*
     * CPU:  NEC (V25/V35) Software Guard only
     * Instruction:  BRKN int_vector
     *
     * Description:
     * [sp-1,sp-2] <- PSW       ; PSW EQU FLAGS
     * [sp-3,sp-4] <- PS        ; PS  EQU CS
     * [sp-5,sp-6] <- PC        ; PC  EQU IP
     * SP    <-  SP -6
     * IE    <-  0
     * BRK <-  0
     * MD    <-  1
     * PC    <- [int_vector*4 +0,+1]
     * PS    <- [int_vector*4 +2,+3]
     *
     * Note:    The BRKN instruction switches operations in Native Mode
     * from Security Mode via Interrupt call. In Normal Mode
     * Instruction executed as   mPD70320/70322 (V25) operation mode.
     *
     * Flags Affected:   None
     *
     * CPU mode: RM
     *
     * +++++++++++++++++++++++
     * Physical Form:   BRKN  imm8
     * COP (Code of Operation)   : 63h imm8
     *
     * Clocks:   56+10T [44+10T]
     */
	/* nec_ICount-=56; */
	unsigned int_vector;

	int_vector = FETCH;
	logerror("PC=%06x : BRKN %02x\n", activecpu_get_pc() - 2, int_vector);
}

static void PREFIXV30(repc) (int flagval)
{
	/* Handles repc- and repnc- prefixes. flagval is the value of ZF
     * for the loop to continue for CMPS and SCAS instructions.
     */

	unsigned next = FETCHOP;
	unsigned count = I.regs.w[CX];

	switch (next)
	{
	case 0x26:							/* ES: */
		seg_prefix = TRUE;
		prefix_base = I.base[ES];
		nec_ICount -= 2;
		PREFIXV30(repc) (flagval);
		break;
	case 0x2e:							/* CS: */
		seg_prefix = TRUE;
		prefix_base = I.base[CS];
		nec_ICount -= 2;
		PREFIXV30(repc) (flagval);
		break;
	case 0x36:							/* SS: */
		seg_prefix = TRUE;
		prefix_base = I.base[SS];
		nec_ICount -= 2;
		PREFIXV30(repc) (flagval);
		break;
	case 0x3e:							/* DS: */
		seg_prefix = TRUE;
		prefix_base = I.base[DS];
		nec_ICount -= 2;
		PREFIXV30(repc) (flagval);
		break;
	case 0x6c:							/* REP INSB */
		nec_ICount -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX186(_insb) ();
		I.regs.w[CX] = count;
		break;
	case 0x6d:							/* REP INSW */
		nec_ICount -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX186(_insw) ();
		I.regs.w[CX] = count;
		break;
	case 0x6e:							/* REP OUTSB */
		nec_ICount -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX186(_outsb) ();
		I.regs.w[CX] = count;
		break;
	case 0x6f:							/* REP OUTSW */
		nec_ICount -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX186(_outsw) ();
		I.regs.w[CX] = count;
		break;
	case 0xa4:							/* REP MOVSB */
		nec_ICount -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_movsb) ();
		I.regs.w[CX] = count;
		break;
	case 0xa5:							/* REP MOVSW */
		nec_ICount -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_movsw) ();
		I.regs.w[CX] = count;
		break;
	case 0xa6:							/* REP(N)E CMPSB */
		nec_ICount -= 9;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--)
			PREFIX86(_cmpsb) ();
		I.regs.w[CX] = count;
		break;
	case 0xa7:							/* REP(N)E CMPSW */
		nec_ICount -= 9;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--)
			PREFIX86(_cmpsw) ();
		I.regs.w[CX] = count;
		break;
	case 0xaa:							/* REP STOSB */
		nec_ICount -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_stosb) ();
		I.regs.w[CX] = count;
		break;
	case 0xab:							/* REP STOSW */
		nec_ICount -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_stosw) ();
		I.regs.w[CX] = count;
		break;
	case 0xac:							/* REP LODSB */
		nec_ICount -= 9;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_lodsb) ();
		I.regs.w[CX] = count;
		break;
	case 0xad:							/* REP LODSW */
		nec_ICount -= 9;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_lodsw) ();
		I.regs.w[CX] = count;
		break;
	case 0xae:							/* REP(N)E SCASB */
		nec_ICount -= 9;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--)
			PREFIX86(_scasb) ();
		I.regs.w[CX] = count;
		break;
	case 0xaf:							/* REP(N)E SCASW */
		nec_ICount -= 9;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--)
			PREFIX86(_scasw) ();
		I.regs.w[CX] = count;
		break;
	default:
		PREFIXV30(_instruction)[next] ();
	}
}

static void PREFIXV30(_repnc) (void)	/* Opcode 0x64 */
{
	PREFIXV30(repc) (0);
}

static void PREFIXV30(_repc) (void)		/* Opcode 0x65 */
{
	PREFIXV30(repc) (1);
}

static void PREFIXV30(_setalc) (void)	/* Opcode 0xd6 */
{
	/*
     * ----------O-SETALC---------------------------------
     * OPCODE SETALC  - Set AL to Carry Flag
     *
     * CPU:  Intel 8086 and all its clones and upward
     * compatibility chips.
     * Type of Instruction: User
     *
     * Instruction: SETALC
     *
     * Description:
     *
     * IF (CF=0) THEN AL:=0 ELSE AL:=FFH;
     *
     * Flags Affected: None
     *
     * CPU mode: RM,PM,VM,SMM
     *
     * Physical Form:        SETALC
     * COP (Code of Operation): D6H
     * Clocks:        80286    : n/a   [3]
     * 80386    : n/a   [3]
     * Cx486SLC  : n/a   [2]
     * i486     : n/a   [3]
     * Pentium  : n/a   [3]
     * Note: n/a is Time that Intel etc not say.
     * [3] is real time it executed.
     *
     */
	I.regs.b[AL] = (CF) ? 0xff : 0x00;
	nec_ICount -= 3;					// V30
	logerror("PC=%06x : SETALC\n", activecpu_get_pc() - 1);
}

#if 0
static void PREFIXV30(_brks) (void)		/* Opcode 0xf1 - Break to Security Mode */
{
	/*
     * CPU:  NEC (V25/V35) Software Guard  only
     * Instruction:  BRKS int_vector
     *
     * Description:
     * [sp-1,sp-2] <- PSW       ; PSW EQU FLAGS
     * [sp-3,sp-4] <- PS        ; PS  EQU CS
     * [sp-5,sp-6] <- PC        ; PC  EQU IP
     * SP    <-  SP -6
     * IE    <-  0
     * BRK <-  0
     * MD    <-  0
     * PC    <- [int_vector*4 +0,+1]
     * PS    <- [int_vector*4 +2,+3]
     *
     * Note:    The BRKS instruction switches operations in Security Mode
     * via Interrupt call. In Security Mode the fetched operation
     * code is executed after conversion in accordance with build-in
     * translation table
     *
     * Flags Affected:   None
     *
     * CPU mode: RM
     *
     * +++++++++++++++++++++++
     * Physical Form:   BRKS  imm8
     * Clocks:   56+10T [44+10T]
     */
	unsigned int_vector;

	int_vector = FETCH;
	logerror("PC=%06x : BRKS %02x\n", activecpu_get_pc() - 2, int_vector);
}
#endif
