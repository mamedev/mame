/**********************************************************
 * IY register related opcodes (FD prefix)
 **********************************************************/
OP(fd,00) { illegal_1(); op_00();									} /* DB   FD          */
OP(fd,01) { illegal_1(); op_01();									} /* DB   FD          */
OP(fd,02) { illegal_1(); op_02();									} /* DB   FD          */
OP(fd,03) { illegal_1(); op_03();									} /* DB   FD          */
OP(fd,04) { illegal_1(); op_04();									} /* DB   FD          */
OP(fd,05) { illegal_1(); op_05();									} /* DB   FD          */
OP(fd,06) { illegal_1(); op_06();									} /* DB   FD          */
OP(fd,07) { illegal_1(); op_07();									} /* DB   FD          */

OP(fd,08) { illegal_1(); op_08();									} /* DB   FD          */
OP(fd,09) { _R++; ADD16(IY,BC); 									} /* ADD  IY,BC       */
OP(fd,0a) { illegal_1(); op_0a();									} /* DB   FD          */
OP(fd,0b) { illegal_1(); op_0b();									} /* DB   FD          */
OP(fd,0c) { illegal_1(); op_0c();									} /* DB   FD          */
OP(fd,0d) { illegal_1(); op_0d();									} /* DB   FD          */
OP(fd,0e) { illegal_1(); op_0e();									} /* DB   FD          */
OP(fd,0f) { illegal_1(); op_0f();									} /* DB   FD          */

OP(fd,10) { illegal_1(); op_10();									} /* DB   FD          */
OP(fd,11) { illegal_1(); op_11();									} /* DB   FD          */
OP(fd,12) { illegal_1(); op_12();									} /* DB   FD          */
OP(fd,13) { illegal_1(); op_13();									} /* DB   FD          */
OP(fd,14) { illegal_1(); op_14();									} /* DB   FD          */
OP(fd,15) { illegal_1(); op_15();									} /* DB   FD          */
OP(fd,16) { illegal_1(); op_16();									} /* DB   FD          */
OP(fd,17) { illegal_1(); op_17();									} /* DB   FD          */

OP(fd,18) { illegal_1(); op_18();									} /* DB   FD          */
OP(fd,19) { _R++; ADD16(IY,DE); 									} /* ADD  IY,DE       */
OP(fd,1a) { illegal_1(); op_1a();									} /* DB   FD          */
OP(fd,1b) { illegal_1(); op_1b();									} /* DB   FD          */
OP(fd,1c) { illegal_1(); op_1c();									} /* DB   FD          */
OP(fd,1d) { illegal_1(); op_1d();									} /* DB   FD          */
OP(fd,1e) { illegal_1(); op_1e();									} /* DB   FD          */
OP(fd,1f) { illegal_1(); op_1f();									} /* DB   FD          */

OP(fd,20) { illegal_1(); op_20();									} /* DB   FD          */
OP(fd,21) { _R++; _IY = ARG16();									} /* LD   IY,w        */
OP(fd,22) { _R++; EA = ARG16(); WM16( EA, &Z180.IY );				} /* LD   (w),IY      */
OP(fd,23) { _R++; _IY++;											} /* INC  IY          */
OP(fd,24) { _R++; _HY = INC(_HY);									} /* INC  HY          */
OP(fd,25) { _R++; _HY = DEC(_HY);									} /* DEC  HY          */
OP(fd,26) { _R++; _HY = ARG();										} /* LD   HY,n        */
OP(fd,27) { illegal_1(); op_27();									} /* DB   FD          */

OP(fd,28) { illegal_1(); op_28();									} /* DB   FD          */
OP(fd,29) { _R++; ADD16(IY,IY); 									} /* ADD  IY,IY       */
OP(fd,2a) { _R++; EA = ARG16(); RM16( EA, &Z180.IY );				} /* LD   IY,(w)      */
OP(fd,2b) { _R++; _IY--;											} /* DEC  IY          */
OP(fd,2c) { _R++; _LY = INC(_LY);									} /* INC  LY          */
OP(fd,2d) { _R++; _LY = DEC(_LY);									} /* DEC  LY          */
OP(fd,2e) { _R++; _LY = ARG();										} /* LD   LY,n        */
OP(fd,2f) { illegal_1(); op_2f();									} /* DB   FD          */

OP(fd,30) { illegal_1(); op_30();									} /* DB   FD          */
OP(fd,31) { illegal_1(); op_31();									} /* DB   FD          */
OP(fd,32) { illegal_1(); op_32();									} /* DB   FD          */
OP(fd,33) { illegal_1(); op_33();									} /* DB   FD          */
OP(fd,34) { _R++; EAY; WM( EA, INC(RM(EA)) );						} /* INC  (IY+o)      */
OP(fd,35) { _R++; EAY; WM( EA, DEC(RM(EA)) );						} /* DEC  (IY+o)      */
OP(fd,36) { _R++; EAY; WM( EA, ARG() ); 							} /* LD   (IY+o),n    */
OP(fd,37) { illegal_1(); op_37();									} /* DB   FD          */

OP(fd,38) { illegal_1(); op_38();									} /* DB   FD          */
OP(fd,39) { _R++; ADD16(IY,SP); 									} /* ADD  IY,SP       */
OP(fd,3a) { illegal_1(); op_3a();									} /* DB   FD          */
OP(fd,3b) { illegal_1(); op_3b();									} /* DB   FD          */
OP(fd,3c) { illegal_1(); op_3c();									} /* DB   FD          */
OP(fd,3d) { illegal_1(); op_3d();									} /* DB   FD          */
OP(fd,3e) { illegal_1(); op_3e();									} /* DB   FD          */
OP(fd,3f) { illegal_1(); op_3f();									} /* DB   FD          */

OP(fd,40) { illegal_1(); op_40();									} /* DB   FD          */
OP(fd,41) { illegal_1(); op_41();									} /* DB   FD          */
OP(fd,42) { illegal_1(); op_42();									} /* DB   FD          */
OP(fd,43) { illegal_1(); op_43();									} /* DB   FD          */
OP(fd,44) { _R++; _B = _HY; 										} /* LD   B,HY        */
OP(fd,45) { _R++; _B = _LY; 										} /* LD   B,LY        */
OP(fd,46) { _R++; EAY; _B = RM(EA); 								} /* LD   B,(IY+o)    */
OP(fd,47) { illegal_1(); op_47();									} /* DB   FD          */

OP(fd,48) { illegal_1(); op_48();									} /* DB   FD          */
OP(fd,49) { illegal_1(); op_49();									} /* DB   FD          */
OP(fd,4a) { illegal_1(); op_4a();									} /* DB   FD          */
OP(fd,4b) { illegal_1(); op_4b();									} /* DB   FD          */
OP(fd,4c) { _R++; _C = _HY; 										} /* LD   C,HY        */
OP(fd,4d) { _R++; _C = _LY; 										} /* LD   C,LY        */
OP(fd,4e) { _R++; EAY; _C = RM(EA); 								} /* LD   C,(IY+o)    */
OP(fd,4f) { illegal_1(); op_4f();									} /* DB   FD          */

OP(fd,50) { illegal_1(); op_50();									} /* DB   FD          */
OP(fd,51) { illegal_1(); op_51();									} /* DB   FD          */
OP(fd,52) { illegal_1(); op_52();									} /* DB   FD          */
OP(fd,53) { illegal_1(); op_53();									} /* DB   FD          */
OP(fd,54) { _R++; _D = _HY; 										} /* LD   D,HY        */
OP(fd,55) { _R++; _D = _LY; 										} /* LD   D,LY        */
OP(fd,56) { _R++; EAY; _D = RM(EA); 								} /* LD   D,(IY+o)    */
OP(fd,57) { illegal_1(); op_57();									} /* DB   FD          */

OP(fd,58) { illegal_1(); op_58();									} /* DB   FD          */
OP(fd,59) { illegal_1(); op_59();									} /* DB   FD          */
OP(fd,5a) { illegal_1(); op_5a();									} /* DB   FD          */
OP(fd,5b) { illegal_1(); op_5b();									} /* DB   FD          */
OP(fd,5c) { _R++; _E = _HY; 										} /* LD   E,HY        */
OP(fd,5d) { _R++; _E = _LY; 										} /* LD   E,LY        */
OP(fd,5e) { _R++; EAY; _E = RM(EA); 								} /* LD   E,(IY+o)    */
OP(fd,5f) { illegal_1(); op_5f();									} /* DB   FD          */

OP(fd,60) { _R++; _HY = _B; 										} /* LD   HY,B        */
OP(fd,61) { _R++; _HY = _C; 										} /* LD   HY,C        */
OP(fd,62) { _R++; _HY = _D; 										} /* LD   HY,D        */
OP(fd,63) { _R++; _HY = _E; 										} /* LD   HY,E        */
OP(fd,64) { _R++;													} /* LD   HY,HY       */
OP(fd,65) { _R++; _HY = _LY;										} /* LD   HY,LY       */
OP(fd,66) { _R++; EAY; _H = RM(EA); 								} /* LD   H,(IY+o)    */
OP(fd,67) { _R++; _HY = _A; 										} /* LD   HY,A        */

OP(fd,68) { _R++; _LY = _B; 										} /* LD   LY,B        */
OP(fd,69) { _R++; _LY = _C; 										} /* LD   LY,C        */
OP(fd,6a) { _R++; _LY = _D; 										} /* LD   LY,D        */
OP(fd,6b) { _R++; _LY = _E; 										} /* LD   LY,E        */
OP(fd,6c) { _R++; _LY = _HY;										} /* LD   LY,HY       */
OP(fd,6d) { _R++;													} /* LD   LY,LY       */
OP(fd,6e) { _R++; EAY; _L = RM(EA); 								} /* LD   L,(IY+o)    */
OP(fd,6f) { _R++; _LY = _A; 										} /* LD   LY,A        */

OP(fd,70) { _R++; EAY; WM( EA, _B );								} /* LD   (IY+o),B    */
OP(fd,71) { _R++; EAY; WM( EA, _C );								} /* LD   (IY+o),C    */
OP(fd,72) { _R++; EAY; WM( EA, _D );								} /* LD   (IY+o),D    */
OP(fd,73) { _R++; EAY; WM( EA, _E );								} /* LD   (IY+o),E    */
OP(fd,74) { _R++; EAY; WM( EA, _H );								} /* LD   (IY+o),H    */
OP(fd,75) { _R++; EAY; WM( EA, _L );								} /* LD   (IY+o),L    */
OP(fd,76) { illegal_1(); op_76();									}		  /* DB   FD          */
OP(fd,77) { _R++; EAY; WM( EA, _A );								} /* LD   (IY+o),A    */

OP(fd,78) { illegal_1(); op_78();									} /* DB   FD          */
OP(fd,79) { illegal_1(); op_79();									} /* DB   FD          */
OP(fd,7a) { illegal_1(); op_7a();									} /* DB   FD          */
OP(fd,7b) { illegal_1(); op_7b();									} /* DB   FD          */
OP(fd,7c) { _R++; _A = _HY; 										} /* LD   A,HY        */
OP(fd,7d) { _R++; _A = _LY; 										} /* LD   A,LY        */
OP(fd,7e) { _R++; EAY; _A = RM(EA); 								} /* LD   A,(IY+o)    */
OP(fd,7f) { illegal_1(); op_7f();									} /* DB   FD          */

OP(fd,80) { illegal_1(); op_80();									} /* DB   FD          */
OP(fd,81) { illegal_1(); op_81();									} /* DB   FD          */
OP(fd,82) { illegal_1(); op_82();									} /* DB   FD          */
OP(fd,83) { illegal_1(); op_83();									} /* DB   FD          */
OP(fd,84) { _R++; ADD(_HY); 										} /* ADD  A,HY        */
OP(fd,85) { _R++; ADD(_LY); 										} /* ADD  A,LY        */
OP(fd,86) { _R++; EAY; ADD(RM(EA)); 								} /* ADD  A,(IY+o)    */
OP(fd,87) { illegal_1(); op_87();									} /* DB   FD          */

OP(fd,88) { illegal_1(); op_88();									} /* DB   FD          */
OP(fd,89) { illegal_1(); op_89();									} /* DB   FD          */
OP(fd,8a) { illegal_1(); op_8a();									} /* DB   FD          */
OP(fd,8b) { illegal_1(); op_8b();									} /* DB   FD          */
OP(fd,8c) { _R++; ADC(_HY); 										} /* ADC  A,HY        */
OP(fd,8d) { _R++; ADC(_LY); 										} /* ADC  A,LY        */
OP(fd,8e) { _R++; EAY; ADC(RM(EA)); 								} /* ADC  A,(IY+o)    */
OP(fd,8f) { illegal_1(); op_8f();									} /* DB   FD          */

OP(fd,90) { illegal_1(); op_90();									} /* DB   FD          */
OP(fd,91) { illegal_1(); op_91();									} /* DB   FD          */
OP(fd,92) { illegal_1(); op_92();									} /* DB   FD          */
OP(fd,93) { illegal_1(); op_93();									} /* DB   FD          */
OP(fd,94) { _R++; SUB(_HY); 										} /* SUB  HY          */
OP(fd,95) { _R++; SUB(_LY); 										} /* SUB  LY          */
OP(fd,96) { _R++; EAY; SUB(RM(EA)); 								} /* SUB  (IY+o)      */
OP(fd,97) { illegal_1(); op_97();									} /* DB   FD          */

OP(fd,98) { illegal_1(); op_98();									} /* DB   FD          */
OP(fd,99) { illegal_1(); op_99();									} /* DB   FD          */
OP(fd,9a) { illegal_1(); op_9a();									} /* DB   FD          */
OP(fd,9b) { illegal_1(); op_9b();									} /* DB   FD          */
OP(fd,9c) { _R++; SBC(_HY); 										} /* SBC  A,HY        */
OP(fd,9d) { _R++; SBC(_LY); 										} /* SBC  A,LY        */
OP(fd,9e) { _R++; EAY; SBC(RM(EA)); 								} /* SBC  A,(IY+o)    */
OP(fd,9f) { illegal_1(); op_9f();									} /* DB   FD          */

OP(fd,a0) { illegal_1(); op_a0();									} /* DB   FD          */
OP(fd,a1) { illegal_1(); op_a1();									} /* DB   FD          */
OP(fd,a2) { illegal_1(); op_a2();									} /* DB   FD          */
OP(fd,a3) { illegal_1(); op_a3();									} /* DB   FD          */
OP(fd,a4) { _R++; AND(_HY); 										} /* AND  HY          */
OP(fd,a5) { _R++; AND(_LY); 										} /* AND  LY          */
OP(fd,a6) { _R++; EAY; AND(RM(EA)); 								} /* AND  (IY+o)      */
OP(fd,a7) { illegal_1(); op_a7();									} /* DB   FD          */

OP(fd,a8) { illegal_1(); op_a8();									} /* DB   FD          */
OP(fd,a9) { illegal_1(); op_a9();									} /* DB   FD          */
OP(fd,aa) { illegal_1(); op_aa();									} /* DB   FD          */
OP(fd,ab) { illegal_1(); op_ab();									} /* DB   FD          */
OP(fd,ac) { _R++; XOR(_HY); 										} /* XOR  HY          */
OP(fd,ad) { _R++; XOR(_LY); 										} /* XOR  LY          */
OP(fd,ae) { _R++; EAY; XOR(RM(EA)); 								} /* XOR  (IY+o)      */
OP(fd,af) { illegal_1(); op_af();									} /* DB   FD          */

OP(fd,b0) { illegal_1(); op_b0();									} /* DB   FD          */
OP(fd,b1) { illegal_1(); op_b1();									} /* DB   FD          */
OP(fd,b2) { illegal_1(); op_b2();									} /* DB   FD          */
OP(fd,b3) { illegal_1(); op_b3();									} /* DB   FD          */
OP(fd,b4) { _R++; OR(_HY);											} /* OR   HY          */
OP(fd,b5) { _R++; OR(_LY);											} /* OR   LY          */
OP(fd,b6) { _R++; EAY; OR(RM(EA));									} /* OR   (IY+o)      */
OP(fd,b7) { illegal_1(); op_b7();									} /* DB   FD          */

OP(fd,b8) { illegal_1(); op_b8();									} /* DB   FD          */
OP(fd,b9) { illegal_1(); op_b9();									} /* DB   FD          */
OP(fd,ba) { illegal_1(); op_ba();									} /* DB   FD          */
OP(fd,bb) { illegal_1(); op_bb();									} /* DB   FD          */
OP(fd,bc) { _R++; CP(_HY);											} /* CP   HY          */
OP(fd,bd) { _R++; CP(_LY);											} /* CP   LY          */
OP(fd,be) { _R++; EAY; CP(RM(EA));									} /* CP   (IY+o)      */
OP(fd,bf) { illegal_1(); op_bf();									} /* DB   FD          */

OP(fd,c0) { illegal_1(); op_c0();									} /* DB   FD          */
OP(fd,c1) { illegal_1(); op_c1();									} /* DB   FD          */
OP(fd,c2) { illegal_1(); op_c2();									} /* DB   FD          */
OP(fd,c3) { illegal_1(); op_c3();									} /* DB   FD          */
OP(fd,c4) { illegal_1(); op_c4();									} /* DB   FD          */
OP(fd,c5) { illegal_1(); op_c5();									} /* DB   FD          */
OP(fd,c6) { illegal_1(); op_c6();									} /* DB   FD          */
OP(fd,c7) { illegal_1(); op_c7();									} /* DB   FD          */

OP(fd,c8) { illegal_1(); op_c8();									} /* DB   FD          */
OP(fd,c9) { illegal_1(); op_c9();									} /* DB   FD          */
OP(fd,ca) { illegal_1(); op_ca();									} /* DB   FD          */
OP(fd,cb) { _R++; EAY; EXEC(xycb,ARG());							} /* **   FD CB xx    */
OP(fd,cc) { illegal_1(); op_cc();									} /* DB   FD          */
OP(fd,cd) { illegal_1(); op_cd();									} /* DB   FD          */
OP(fd,ce) { illegal_1(); op_ce();									} /* DB   FD          */
OP(fd,cf) { illegal_1(); op_cf();									} /* DB   FD          */

OP(fd,d0) { illegal_1(); op_d0();									} /* DB   FD          */
OP(fd,d1) { illegal_1(); op_d1();									} /* DB   FD          */
OP(fd,d2) { illegal_1(); op_d2();									} /* DB   FD          */
OP(fd,d3) { illegal_1(); op_d3();									} /* DB   FD          */
OP(fd,d4) { illegal_1(); op_d4();									} /* DB   FD          */
OP(fd,d5) { illegal_1(); op_d5();									} /* DB   FD          */
OP(fd,d6) { illegal_1(); op_d6();									} /* DB   FD          */
OP(fd,d7) { illegal_1(); op_d7();									} /* DB   FD          */

OP(fd,d8) { illegal_1(); op_d8();									} /* DB   FD          */
OP(fd,d9) { illegal_1(); op_d9();									} /* DB   FD          */
OP(fd,da) { illegal_1(); op_da();									} /* DB   FD          */
OP(fd,db) { illegal_1(); op_db();									} /* DB   FD          */
OP(fd,dc) { illegal_1(); op_dc();									} /* DB   FD          */
OP(fd,dd) { illegal_1(); op_dd();									} /* DB   FD          */
OP(fd,de) { illegal_1(); op_de();									} /* DB   FD          */
OP(fd,df) { illegal_1(); op_df();									} /* DB   FD          */

OP(fd,e0) { illegal_1(); op_e0();									} /* DB   FD          */
OP(fd,e1) { _R++; POP(IY);											} /* POP  IY          */
OP(fd,e2) { illegal_1(); op_e2();									} /* DB   FD          */
OP(fd,e3) { _R++; EXSP(IY); 										} /* EX   (SP),IY     */
OP(fd,e4) { illegal_1(); op_e4();									} /* DB   FD          */
OP(fd,e5) { _R++; PUSH( IY );										} /* PUSH IY          */
OP(fd,e6) { illegal_1(); op_e6();									} /* DB   FD          */
OP(fd,e7) { illegal_1(); op_e7();									} /* DB   FD          */

OP(fd,e8) { illegal_1(); op_e8();									} /* DB   FD          */
OP(fd,e9) { _R++; _PC = _IY; z180_change_pc(_PCD);					} /* JP   (IY)        */
OP(fd,ea) { illegal_1(); op_ea();									} /* DB   FD          */
OP(fd,eb) { illegal_1(); op_eb();									} /* DB   FD          */
OP(fd,ec) { illegal_1(); op_ec();									} /* DB   FD          */
OP(fd,ed) { illegal_1(); op_ed();									} /* DB   FD          */
OP(fd,ee) { illegal_1(); op_ee();									} /* DB   FD          */
OP(fd,ef) { illegal_1(); op_ef();									} /* DB   FD          */

OP(fd,f0) { illegal_1(); op_f0();									} /* DB   FD          */
OP(fd,f1) { illegal_1(); op_f1();									} /* DB   FD          */
OP(fd,f2) { illegal_1(); op_f2();									} /* DB   FD          */
OP(fd,f3) { illegal_1(); op_f3();									} /* DB   FD          */
OP(fd,f4) { illegal_1(); op_f4();									} /* DB   FD          */
OP(fd,f5) { illegal_1(); op_f5();									} /* DB   FD          */
OP(fd,f6) { illegal_1(); op_f6();									} /* DB   FD          */
OP(fd,f7) { illegal_1(); op_f7();									} /* DB   FD          */

OP(fd,f8) { illegal_1(); op_f8();									} /* DB   FD          */
OP(fd,f9) { _R++; _SP = _IY;										} /* LD   SP,IY       */
OP(fd,fa) { illegal_1(); op_fa();									} /* DB   FD          */
OP(fd,fb) { illegal_1(); op_fb();									} /* DB   FD          */
OP(fd,fc) { illegal_1(); op_fc();									} /* DB   FD          */
OP(fd,fd) { illegal_1(); op_fd();									} /* DB   FD          */
OP(fd,fe) { illegal_1(); op_fe();									} /* DB   FD          */
OP(fd,ff) { illegal_1(); op_ff();									} /* DB   FD          */

