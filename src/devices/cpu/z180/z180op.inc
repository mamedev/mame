// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************
 * main opcodes
 **********************************************************/
OP(op,00) {                                                         } /* NOP              */
OP(op,01) { _BC = ARG16();                                            } /* LD   BC,w        */
OP(op,02) { WM( _BC, _A );                                            } /* LD   (BC),A      */
OP(op,03) { _BC++;                                                    } /* INC  BC          */
OP(op,04) { _B = INC(_B);                                         } /* INC  B           */
OP(op,05) { _B = DEC(_B);                                         } /* DEC  B           */
OP(op,06) { _B = ARG();                                           } /* LD   B,n         */
OP(op,07) { RLCA;                                                   } /* RLCA             */

OP(op,08) { EX_AF;                                                  } /* EX   AF,AF'      */
OP(op,09) { ADD16(HL,BC);                                           } /* ADD  HL,BC       */
OP(op,0a) { _A = RM(_BC);                                         } /* LD   A,(BC)      */
OP(op,0b) { _BC--;                                                    } /* DEC  BC          */
OP(op,0c) { _C = INC(_C);                                         } /* INC  C           */
OP(op,0d) { _C = DEC(_C);                                         } /* DEC  C           */
OP(op,0e) { _C = ARG();                                           } /* LD   C,n         */
OP(op,0f) { RRCA;                                                   } /* RRCA             */

OP(op,10) { _B--; JR_COND( _B, 0x10 );                              } /* DJNZ o           */
OP(op,11) { _DE = ARG16();                                            } /* LD   DE,w        */
OP(op,12) { WM( _DE, _A );                                            } /* LD   (DE),A      */
OP(op,13) { _DE++;                                                    } /* INC  DE          */
OP(op,14) { _D = INC(_D);                                         } /* INC  D           */
OP(op,15) { _D = DEC(_D);                                         } /* DEC  D           */
OP(op,16) { _D = ARG();                                           } /* LD   D,n         */
OP(op,17) { RLA;                                                    } /* RLA              */

OP(op,18) { JR();                                                   } /* JR   o           */
OP(op,19) { ADD16(HL,DE);                                           } /* ADD  HL,DE       */
OP(op,1a) { _A = RM(_DE);                                         } /* LD   A,(DE)      */
OP(op,1b) { _DE--;                ;                                   } /* DEC  DE          */
OP(op,1c) { _E = INC(_E);                                         } /* INC  E           */
OP(op,1d) { _E = DEC(_E);                                         } /* DEC  E           */
OP(op,1e) { _E = ARG();                                           } /* LD   E,n         */
OP(op,1f) { RRA;                                                    } /* RRA              */

OP(op,20) { JR_COND( !(_F & ZF), 0x20 );                          } /* JR   NZ,o        */
OP(op,21) { _HL = ARG16();                                            } /* LD   HL,w        */
OP(op,22) { m_ea = ARG16(); WM16(m_ea, &m_HL );                   } /* LD   (w),HL      */
OP(op,23) { _HL++;                                                    } /* INC  HL          */
OP(op,24) { _H = INC(_H);                                         } /* INC  H           */
OP(op,25) { _H = DEC(_H);                                         } /* DEC  H           */
OP(op,26) { _H = ARG();                                           } /* LD   H,n         */
OP(op,27) { DAA;                                                    } /* DAA              */

OP(op,28) { JR_COND( _F & ZF, 0x28 );                             } /* JR   Z,o         */
OP(op,29) { ADD16(HL,HL);                                           } /* ADD  HL,HL       */
OP(op,2a) { m_ea = ARG16(); RM16(m_ea, &m_HL );                   } /* LD   HL,(w)      */
OP(op,2b) { _HL--;                                                    } /* DEC  HL          */
OP(op,2c) { _L = INC(_L);                                         } /* INC  L           */
OP(op,2d) { _L = DEC(_L);                                         } /* DEC  L           */
OP(op,2e) { _L = ARG();                                           } /* LD   L,n         */
OP(op,2f) { _A ^= 0xff; _F = (_F&(SF|ZF|PF|CF))|HF|NF|(_A&(YF|XF)); } /* CPL              */

OP(op,30) { JR_COND( !(_F & CF), 0x30 );                          } /* JR   NC,o        */
OP(op,31) { _SP = ARG16();                                            } /* LD   SP,w        */
OP(op,32) { m_ea = ARG16(); WM( m_ea, _A );                             } /* LD   (w),A       */
OP(op,33) { _SP++;                                                    } /* INC  SP          */
OP(op,34) { WM( _HL, INC(RM(_HL)) );                              } /* INC  (HL)        */
OP(op,35) { WM( _HL, DEC(RM(_HL)) );                              } /* DEC  (HL)        */
OP(op,36) { WM( _HL, ARG() );                                       } /* LD   (HL),n      */
OP(op,37) { _F = (_F & (SF|ZF|PF)) | CF | (_A & (YF|XF));         } /* SCF              */

OP(op,38) { JR_COND( _F & CF, 0x38 );                             } /* JR   C,o         */
OP(op,39) { ADD16(HL,SP);                                           } /* ADD  HL,SP       */
OP(op,3a) { m_ea = ARG16(); _A = RM( m_ea );                            } /* LD   A,(w)       */
OP(op,3b) { _SP--;                                                    } /* DEC  SP          */
OP(op,3c) { _A = INC(_A);                                         } /* INC  A           */
OP(op,3d) { _A = DEC(_A);                                         } /* DEC  A           */
OP(op,3e) { _A = ARG();                                           } /* LD   A,n         */
OP(op,3f) { _F = ((_F&(SF|ZF|PF|CF))|((_F&CF)<<4)|(_A&(YF|XF)))^CF; } /* CCF              */
//OP(op,3f) { _F = ((_F & ~(HF|NF)) | ((_F & CF)<<4)) ^ CF;           } /* CCF              */

OP(op,40) {                                                         } /* LD   B,B         */
OP(op,41) { _B = _C;                                                } /* LD   B,C         */
OP(op,42) { _B = _D;                                                } /* LD   B,D         */
OP(op,43) { _B = _E;                                                } /* LD   B,E         */
OP(op,44) { _B = _H;                                                } /* LD   B,H         */
OP(op,45) { _B = _L;                                                } /* LD   B,L         */
OP(op,46) { _B = RM(_HL);                                         } /* LD   B,(HL)      */
OP(op,47) { _B = _A;                                                } /* LD   B,A         */

OP(op,48) { _C = _B;                                                } /* LD   C,B         */
OP(op,49) {                                                         } /* LD   C,C         */
OP(op,4a) { _C = _D;                                                } /* LD   C,D         */
OP(op,4b) { _C = _E;                                                } /* LD   C,E         */
OP(op,4c) { _C = _H;                                                } /* LD   C,H         */
OP(op,4d) { _C = _L;                                                } /* LD   C,L         */
OP(op,4e) { _C = RM(_HL);                                         } /* LD   C,(HL)      */
OP(op,4f) { _C = _A;                                                } /* LD   C,A         */

OP(op,50) { _D = _B;                                                } /* LD   D,B         */
OP(op,51) { _D = _C;                                                } /* LD   D,C         */
OP(op,52) {                                                         } /* LD   D,D         */
OP(op,53) { _D = _E;                                                } /* LD   D,E         */
OP(op,54) { _D = _H;                                                } /* LD   D,H         */
OP(op,55) { _D = _L;                                                } /* LD   D,L         */
OP(op,56) { _D = RM(_HL);                                         } /* LD   D,(HL)      */
OP(op,57) { _D = _A;                                                } /* LD   D,A         */

OP(op,58) { _E = _B;                                                } /* LD   E,B         */
OP(op,59) { _E = _C;                                                } /* LD   E,C         */
OP(op,5a) { _E = _D;                                                } /* LD   E,D         */
OP(op,5b) {                                                         } /* LD   E,E         */
OP(op,5c) { _E = _H;                                                } /* LD   E,H         */
OP(op,5d) { _E = _L;                                                } /* LD   E,L         */
OP(op,5e) { _E = RM(_HL);                                         } /* LD   E,(HL)      */
OP(op,5f) { _E = _A;                                                } /* LD   E,A         */

OP(op,60) { _H = _B;                                                } /* LD   H,B         */
OP(op,61) { _H = _C;                                                } /* LD   H,C         */
OP(op,62) { _H = _D;                                                } /* LD   H,D         */
OP(op,63) { _H = _E;                                                } /* LD   H,E         */
OP(op,64) {                                                         } /* LD   H,H         */
OP(op,65) { _H = _L;                                                } /* LD   H,L         */
OP(op,66) { _H = RM(_HL);                                         } /* LD   H,(HL)      */
OP(op,67) { _H = _A;                                                } /* LD   H,A         */

OP(op,68) { _L = _B;                                                } /* LD   L,B         */
OP(op,69) { _L = _C;                                                } /* LD   L,C         */
OP(op,6a) { _L = _D;                                                } /* LD   L,D         */
OP(op,6b) { _L = _E;                                                } /* LD   L,E         */
OP(op,6c) { _L = _H;                                                } /* LD   L,H         */
OP(op,6d) {                                                         } /* LD   L,L         */
OP(op,6e) { _L = RM(_HL);                                         } /* LD   L,(HL)      */
OP(op,6f) { _L = _A;                                                } /* LD   L,A         */

OP(op,70) { WM( _HL, _B );                                            } /* LD   (HL),B      */
OP(op,71) { WM( _HL, _C );                                            } /* LD   (HL),C      */
OP(op,72) { WM( _HL, _D );                                            } /* LD   (HL),D      */
OP(op,73) { WM( _HL, _E );                                            } /* LD   (HL),E      */
OP(op,74) { WM( _HL, _H );                                            } /* LD   (HL),H      */
OP(op,75) { WM( _HL, _L );                                            } /* LD   (HL),L      */
OP(op,76) { ENTER_HALT();                                           } /* HALT             */
OP(op,77) { WM( _HL, _A );                                            } /* LD   (HL),A      */

OP(op,78) { _A = _B;                                                } /* LD   A,B         */
OP(op,79) { _A = _C;                                                } /* LD   A,C         */
OP(op,7a) { _A = _D;                                                } /* LD   A,D         */
OP(op,7b) { _A = _E;                                                } /* LD   A,E         */
OP(op,7c) { _A = _H;                                                } /* LD   A,H         */
OP(op,7d) { _A = _L;                                                } /* LD   A,L         */
OP(op,7e) { _A = RM(_HL);                                         } /* LD   A,(HL)      */
OP(op,7f) {                                                         } /* LD   A,A         */

OP(op,80) { ADD(_B);                                              } /* ADD  A,B         */
OP(op,81) { ADD(_C);                                              } /* ADD  A,C         */
OP(op,82) { ADD(_D);                                              } /* ADD  A,D         */
OP(op,83) { ADD(_E);                                              } /* ADD  A,E         */
OP(op,84) { ADD(_H);                                              } /* ADD  A,H         */
OP(op,85) { ADD(_L);                                              } /* ADD  A,L         */
OP(op,86) { ADD(RM(_HL));                                           } /* ADD  A,(HL)      */
OP(op,87) { ADD(_A);                                              } /* ADD  A,A         */

OP(op,88) { ADC(_B);                                              } /* ADC  A,B         */
OP(op,89) { ADC(_C);                                              } /* ADC  A,C         */
OP(op,8a) { ADC(_D);                                              } /* ADC  A,D         */
OP(op,8b) { ADC(_E);                                              } /* ADC  A,E         */
OP(op,8c) { ADC(_H);                                              } /* ADC  A,H         */
OP(op,8d) { ADC(_L);                                              } /* ADC  A,L         */
OP(op,8e) { ADC(RM(_HL));                                           } /* ADC  A,(HL)      */
OP(op,8f) { ADC(_A);                                              } /* ADC  A,A         */

OP(op,90) { SUB(_B);                                              } /* SUB  B           */
OP(op,91) { SUB(_C);                                              } /* SUB  C           */
OP(op,92) { SUB(_D);                                              } /* SUB  D           */
OP(op,93) { SUB(_E);                                              } /* SUB  E           */
OP(op,94) { SUB(_H);                                              } /* SUB  H           */
OP(op,95) { SUB(_L);                                              } /* SUB  L           */
OP(op,96) { SUB(RM(_HL));                                           } /* SUB  (HL)        */
OP(op,97) { SUB(_A);                                              } /* SUB  A           */

OP(op,98) { SBC(_B);                                              } /* SBC  A,B         */
OP(op,99) { SBC(_C);                                              } /* SBC  A,C         */
OP(op,9a) { SBC(_D);                                              } /* SBC  A,D         */
OP(op,9b) { SBC(_E);                                              } /* SBC  A,E         */
OP(op,9c) { SBC(_H);                                              } /* SBC  A,H         */
OP(op,9d) { SBC(_L);                                              } /* SBC  A,L         */
OP(op,9e) { SBC(RM(_HL));                                           } /* SBC  A,(HL)      */
OP(op,9f) { SBC(_A);                                              } /* SBC  A,A         */

OP(op,a0) { AND(_B);                                              } /* AND  B           */
OP(op,a1) { AND(_C);                                              } /* AND  C           */
OP(op,a2) { AND(_D);                                              } /* AND  D           */
OP(op,a3) { AND(_E);                                              } /* AND  E           */
OP(op,a4) { AND(_H);                                              } /* AND  H           */
OP(op,a5) { AND(_L);                                              } /* AND  L           */
OP(op,a6) { AND(RM(_HL));                                           } /* AND  (HL)        */
OP(op,a7) { AND(_A);                                              } /* AND  A           */

OP(op,a8) { XOR(_B);                                              } /* XOR  B           */
OP(op,a9) { XOR(_C);                                              } /* XOR  C           */
OP(op,aa) { XOR(_D);                                              } /* XOR  D           */
OP(op,ab) { XOR(_E);                                              } /* XOR  E           */
OP(op,ac) { XOR(_H);                                              } /* XOR  H           */
OP(op,ad) { XOR(_L);                                              } /* XOR  L           */
OP(op,ae) { XOR(RM(_HL));                                           } /* XOR  (HL)        */
OP(op,af) { XOR(_A);                                              } /* XOR  A           */

OP(op,b0) { OR(_B);                                               } /* OR   B           */
OP(op,b1) { OR(_C);                                               } /* OR   C           */
OP(op,b2) { OR(_D);                                               } /* OR   D           */
OP(op,b3) { OR(_E);                                               } /* OR   E           */
OP(op,b4) { OR(_H);                                               } /* OR   H           */
OP(op,b5) { OR(_L);                                               } /* OR   L           */
OP(op,b6) { OR(RM(_HL));                                            } /* OR   (HL)        */
OP(op,b7) { OR(_A);                                               } /* OR   A           */

OP(op,b8) { CP(_B);                                               } /* CP   B           */
OP(op,b9) { CP(_C);                                               } /* CP   C           */
OP(op,ba) { CP(_D);                                               } /* CP   D           */
OP(op,bb) { CP(_E);                                               } /* CP   E           */
OP(op,bc) { CP(_H);                                               } /* CP   H           */
OP(op,bd) { CP(_L);                                               } /* CP   L           */
OP(op,be) { CP(RM(_HL));                                            } /* CP   (HL)        */
OP(op,bf) { CP(_A);                                               } /* CP   A           */

OP(op,c0) { RET_COND( !(_F & ZF), 0xc0 );                         } /* RET  NZ          */
OP(op,c1) { POP(BC);                                              } /* POP  BC          */
OP(op,c2) { JP_COND( !(_F & ZF) );                                    } /* JP   NZ,a        */
OP(op,c3) { JP;                                                     } /* JP   a           */
OP(op,c4) { CALL_COND( !(_F & ZF), 0xc4 );                            } /* CALL NZ,a        */
OP(op,c5) { PUSH( BC );                                           } /* PUSH BC          */
OP(op,c6) { ADD(ARG());                                             } /* ADD  A,n         */
OP(op,c7) { RST(0x00);                                              } /* RST  0           */

OP(op,c8) { RET_COND( _F & ZF, 0xc8 );                                } /* RET  Z           */
OP(op,c9) { POP(PC);                                              } /* RET              */
OP(op,ca) { JP_COND( _F & ZF );                                   } /* JP   Z,a         */
OP(op,cb) { m_R++; m_extra_cycles += exec_cb(ROP());                                   } /* **** CB xx       */
OP(op,cc) { CALL_COND( _F & ZF, 0xcc );                           } /* CALL Z,a         */
OP(op,cd) { CALL();                                                 } /* CALL a           */
OP(op,ce) { ADC(ARG());                                             } /* ADC  A,n         */
OP(op,cf) { RST(0x08);                                              } /* RST  1           */

OP(op,d0) { RET_COND( !(_F & CF), 0xd0 );                         } /* RET  NC          */
OP(op,d1) { POP(DE);                                              } /* POP  DE          */
OP(op,d2) { JP_COND( !(_F & CF) );                                    } /* JP   NC,a        */
OP(op,d3) { unsigned n = ARG() | (_A << 8); OUT(n, _A );         } /* OUT  (n),A       */
OP(op,d4) { CALL_COND( !(_F & CF), 0xd4 );                            } /* CALL NC,a        */
OP(op,d5) { PUSH( DE );                                           } /* PUSH DE          */
OP(op,d6) { SUB(ARG());                                             } /* SUB  n           */
OP(op,d7) { RST(0x10);                                              } /* RST  2           */

OP(op,d8) { RET_COND( _F & CF, 0xd8 );                                } /* RET  C           */
OP(op,d9) { EXX;                                                    } /* EXX              */
OP(op,da) { JP_COND( _F & CF );                                   } /* JP   C,a         */
OP(op,db) { unsigned n = ARG() | (_A << 8); _A = IN(n );         } /* IN   A,(n)       */
OP(op,dc) { CALL_COND( _F & CF, 0xdc );                           } /* CALL C,a         */
OP(op,dd) { m_R++; m_extra_cycles += exec_dd(ROP());                                   } /* **** DD xx       */
OP(op,de) { SBC(ARG());                                             } /* SBC  A,n         */
OP(op,df) { RST(0x18);                                              } /* RST  3           */

OP(op,e0) { RET_COND( !(_F & PF), 0xe0 );                         } /* RET  PO          */
OP(op,e1) { POP(HL);                                              } /* POP  HL          */
OP(op,e2) { JP_COND( !(_F & PF) );                                    } /* JP   PO,a        */
OP(op,e3) { EXSP(HL);                                               } /* EX   HL,(SP)     */
OP(op,e4) { CALL_COND( !(_F & PF), 0xe4 );                            } /* CALL PO,a        */
OP(op,e5) { PUSH( HL );                                           } /* PUSH HL          */
OP(op,e6) { AND(ARG());                                             } /* AND  n           */
OP(op,e7) { RST(0x20);                                              } /* RST  4           */

OP(op,e8) { RET_COND( _F & PF, 0xe8 );                                } /* RET  PE          */
OP(op,e9) { _PC = _HL;                                              } /* JP   (HL)        */
OP(op,ea) { JP_COND( _F & PF );                                   } /* JP   PE,a        */
OP(op,eb) { EX_DE_HL;                                               } /* EX   DE,HL       */
OP(op,ec) { CALL_COND( _F & PF, 0xec );                           } /* CALL PE,a        */
OP(op,ed) { m_R++; m_extra_cycles += exec_ed(ROP());                                   } /* **** ED xx       */
OP(op,ee) { XOR(ARG());                                             } /* XOR  n           */
OP(op,ef) { RST(0x28);                                              } /* RST  5           */

OP(op,f0) { RET_COND( !(_F & SF), 0xf0 );                         } /* RET  P           */
OP(op,f1) { POP(AF);                                              } /* POP  AF          */
OP(op,f2) { JP_COND( !(_F & SF) );                                    } /* JP   P,a         */
OP(op,f3) { m_IFF1 = m_IFF2 = 0;                                        } /* DI               */
OP(op,f4) { CALL_COND( !(_F & SF), 0xf4 );                            } /* CALL P,a         */
OP(op,f5) { PUSH( AF );                                           } /* PUSH AF          */
OP(op,f6) { OR(ARG());                                              } /* OR   n           */
OP(op,f7) { RST(0x30);                                              } /* RST  6           */

OP(op,f8) { RET_COND( _F & SF, 0xf8 );                                } /* RET  M           */
OP(op,f9) { _SP = _HL;                                              } /* LD   SP,HL       */
OP(op,fa) { JP_COND(_F & SF);                                     } /* JP   M,a         */
OP(op,fb) { EI;                                                     } /* EI               */
OP(op,fc) { CALL_COND( _F & SF, 0xfc );                           } /* CALL M,a         */
OP(op,fd) { m_R++; m_extra_cycles += exec_fd(ROP());                                   } /* **** FD xx       */
OP(op,fe) { CP(ARG());                                              } /* CP   n           */
OP(op,ff) { RST(0x38);                                              } /* RST  7           */


int z180_device::take_interrupt(int irq)
{
	int irq_vector;
	int cycles = 0;

	/* there isn't a valid previous program counter */
	_PPC = -1;

	/* Check if processor was halted */
	LEAVE_HALT();

	/* Clear both interrupt flip flops */
	m_IFF1 = m_IFF2 = 0;

	if( irq == Z180_INT_IRQ0 )
	{
		/* Daisy chain mode? If so, call the requesting device */
		if (m_daisy.present())
			irq_vector = m_daisy.call_ack_device();

		/* else call back the cpu interface to retrieve the vector */
		else
			irq_vector = standard_irq_callback(0);

		LOG(("Z180 '%s' single int. irq_vector $%02x\n", tag().c_str(), irq_vector));

		/* Interrupt mode 2. Call [m_I:databyte] */
		if( m_IM == 2 )
		{
			irq_vector = (irq_vector & 0xff) + (m_I << 8);
			PUSH( PC );
			RM16(irq_vector, &m_PC );
			LOG(("Z180 '%s' IM2 [$%04x] = $%04x\n", tag().c_str() , irq_vector, _PCD));
			/* CALL opcode timing */
			cycles += m_cc[Z180_TABLE_op][0xcd];
		}
		else
		/* Interrupt mode 1. RST 38h */
		if( m_IM == 1 )
		{
			LOG(("Z180 '%s' IM1 $0038\n", tag().c_str() ));
			PUSH( PC );
			_PCD = 0x0038;
			/* RST $38 + 'interrupt latency' cycles */
			cycles += m_cc[Z180_TABLE_op][0xff] - m_cc[Z180_TABLE_ex][0xff];
		}
		else
		{
			/* Interrupt mode 0. We check for CALL and JP instructions, */
			/* if neither of these were found we assume a 1 byte opcode */
			/* was placed on the databus                                */
			LOG(("Z180 '%s' IM0 $%04x\n", tag().c_str() , irq_vector));
			switch (irq_vector & 0xff0000)
			{
				case 0xcd0000:  /* call */
					PUSH( PC );
					_PCD = irq_vector & 0xffff;
						/* CALL $xxxx + 'interrupt latency' cycles */
					cycles += m_cc[Z180_TABLE_op][0xcd] - m_cc[Z180_TABLE_ex][0xff];
					break;
				case 0xc30000:  /* jump */
					_PCD = irq_vector & 0xffff;
					/* JP $xxxx + 2 cycles */
					cycles += m_cc[Z180_TABLE_op][0xc3] - m_cc[Z180_TABLE_ex][0xff];
					break;
				default:        /* rst (or other opcodes?) */
					PUSH( PC );
					_PCD = irq_vector & 0x0038;
					/* RST $xx + 2 cycles */
					cycles += m_cc[Z180_TABLE_op][_PCD] - m_cc[Z180_TABLE_ex][_PCD];
					break;
			}
		}
	}
	else
	{
		irq_vector = (IO(Z180_IL) & Z180_IL_IL) + (irq - Z180_INT_IRQ1) * 2;
		irq_vector = (m_I << 8) + (irq_vector & 0xff);
		PUSH( PC );
		RM16(irq_vector, &m_PC );
		LOG(("Z180 '%s' INT%d [$%04x] = $%04x\n", tag().c_str(), irq, irq_vector, _PCD));
		/* CALL opcode timing */
		cycles += m_cc[Z180_TABLE_op][0xcd];
	}

	return cycles;
}
