// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
OP(cb,00) { _B = RLC(_B);                                         } /* RLC  B           */
OP(cb,01) { _C = RLC(_C);                                         } /* RLC  C           */
OP(cb,02) { _D = RLC(_D);                                         } /* RLC  D           */
OP(cb,03) { _E = RLC(_E);                                         } /* RLC  E           */
OP(cb,04) { _H = RLC(_H);                                         } /* RLC  H           */
OP(cb,05) { _L = RLC(_L);                                         } /* RLC  L           */
OP(cb,06) { WM( _HL, RLC(RM(_HL)) );                              } /* RLC  (HL)        */
OP(cb,07) { _A = RLC(_A);                                         } /* RLC  A           */

OP(cb,08) { _B = RRC(_B);                                         } /* RRC  B           */
OP(cb,09) { _C = RRC(_C);                                         } /* RRC  C           */
OP(cb,0a) { _D = RRC(_D);                                         } /* RRC  D           */
OP(cb,0b) { _E = RRC(_E);                                         } /* RRC  E           */
OP(cb,0c) { _H = RRC(_H);                                         } /* RRC  H           */
OP(cb,0d) { _L = RRC(_L);                                         } /* RRC  L           */
OP(cb,0e) { WM( _HL, RRC(RM(_HL)) );                              } /* RRC  (HL)        */
OP(cb,0f) { _A = RRC(_A);                                         } /* RRC  A           */

OP(cb,10) { _B = RL(_B);                                          } /* RL   B           */
OP(cb,11) { _C = RL(_C);                                          } /* RL   C           */
OP(cb,12) { _D = RL(_D);                                          } /* RL   D           */
OP(cb,13) { _E = RL(_E);                                          } /* RL   E           */
OP(cb,14) { _H = RL(_H);                                          } /* RL   H           */
OP(cb,15) { _L = RL(_L);                                          } /* RL   L           */
OP(cb,16) { WM( _HL, RL(RM(_HL)) );                               } /* RL   (HL)        */
OP(cb,17) { _A = RL(_A);                                          } /* RL   A           */

OP(cb,18) { _B = RR(_B);                                          } /* RR   B           */
OP(cb,19) { _C = RR(_C);                                          } /* RR   C           */
OP(cb,1a) { _D = RR(_D);                                          } /* RR   D           */
OP(cb,1b) { _E = RR(_E);                                          } /* RR   E           */
OP(cb,1c) { _H = RR(_H);                                          } /* RR   H           */
OP(cb,1d) { _L = RR(_L);                                          } /* RR   L           */
OP(cb,1e) { WM( _HL, RR(RM(_HL)) );                               } /* RR   (HL)        */
OP(cb,1f) { _A = RR(_A);                                          } /* RR   A           */

OP(cb,20) { _B = SLA(_B);                                         } /* SLA  B           */
OP(cb,21) { _C = SLA(_C);                                         } /* SLA  C           */
OP(cb,22) { _D = SLA(_D);                                         } /* SLA  D           */
OP(cb,23) { _E = SLA(_E);                                         } /* SLA  E           */
OP(cb,24) { _H = SLA(_H);                                         } /* SLA  H           */
OP(cb,25) { _L = SLA(_L);                                         } /* SLA  L           */
OP(cb,26) { WM( _HL, SLA(RM(_HL)) );                              } /* SLA  (HL)        */
OP(cb,27) { _A = SLA(_A);                                         } /* SLA  A           */

OP(cb,28) { _B = SRA(_B);                                         } /* SRA  B           */
OP(cb,29) { _C = SRA(_C);                                         } /* SRA  C           */
OP(cb,2a) { _D = SRA(_D);                                         } /* SRA  D           */
OP(cb,2b) { _E = SRA(_E);                                         } /* SRA  E           */
OP(cb,2c) { _H = SRA(_H);                                         } /* SRA  H           */
OP(cb,2d) { _L = SRA(_L);                                         } /* SRA  L           */
OP(cb,2e) { WM( _HL, SRA(RM(_HL)) );                              } /* SRA  (HL)        */
OP(cb,2f) { _A = SRA(_A);                                         } /* SRA  A           */

OP(cb,30) { _B = SLL(_B);                                         } /* SLL  B           */
OP(cb,31) { _C = SLL(_C);                                         } /* SLL  C           */
OP(cb,32) { _D = SLL(_D);                                         } /* SLL  D           */
OP(cb,33) { _E = SLL(_E);                                         } /* SLL  E           */
OP(cb,34) { _H = SLL(_H);                                         } /* SLL  H           */
OP(cb,35) { _L = SLL(_L);                                         } /* SLL  L           */
OP(cb,36) { WM( _HL, SLL(RM(_HL)) );                              } /* SLL  (HL)        */
OP(cb,37) { _A = SLL(_A);                                         } /* SLL  A           */

OP(cb,38) { _B = SRL(_B);                                         } /* SRL  B           */
OP(cb,39) { _C = SRL(_C);                                         } /* SRL  C           */
OP(cb,3a) { _D = SRL(_D);                                         } /* SRL  D           */
OP(cb,3b) { _E = SRL(_E);                                         } /* SRL  E           */
OP(cb,3c) { _H = SRL(_H);                                         } /* SRL  H           */
OP(cb,3d) { _L = SRL(_L);                                         } /* SRL  L           */
OP(cb,3e) { WM( _HL, SRL(RM(_HL)) );                              } /* SRL  (HL)        */
OP(cb,3f) { _A = SRL(_A);                                         } /* SRL  A           */

OP(cb,40) { BIT(0,_B);                                                } /* BIT  0,B         */
OP(cb,41) { BIT(0,_C);                                                } /* BIT  0,C         */
OP(cb,42) { BIT(0,_D);                                                } /* BIT  0,D         */
OP(cb,43) { BIT(0,_E);                                                } /* BIT  0,E         */
OP(cb,44) { BIT(0,_H);                                                } /* BIT  0,H         */
OP(cb,45) { BIT(0,_L);                                                } /* BIT  0,L         */
OP(cb,46) { BIT(0,RM(_HL));                                         } /* BIT  0,(HL)      */
OP(cb,47) { BIT(0,_A);                                                } /* BIT  0,A         */

OP(cb,48) { BIT(1,_B);                                                } /* BIT  1,B         */
OP(cb,49) { BIT(1,_C);                                                } /* BIT  1,C         */
OP(cb,4a) { BIT(1,_D);                                                } /* BIT  1,D         */
OP(cb,4b) { BIT(1,_E);                                                } /* BIT  1,E         */
OP(cb,4c) { BIT(1,_H);                                                } /* BIT  1,H         */
OP(cb,4d) { BIT(1,_L);                                                } /* BIT  1,L         */
OP(cb,4e) { BIT(1,RM(_HL));                                         } /* BIT  1,(HL)      */
OP(cb,4f) { BIT(1,_A);                                                } /* BIT  1,A         */

OP(cb,50) { BIT(2,_B);                                                } /* BIT  2,B         */
OP(cb,51) { BIT(2,_C);                                                } /* BIT  2,C         */
OP(cb,52) { BIT(2,_D);                                                } /* BIT  2,D         */
OP(cb,53) { BIT(2,_E);                                                } /* BIT  2,E         */
OP(cb,54) { BIT(2,_H);                                                } /* BIT  2,H         */
OP(cb,55) { BIT(2,_L);                                                } /* BIT  2,L         */
OP(cb,56) { BIT(2,RM(_HL));                                         } /* BIT  2,(HL)      */
OP(cb,57) { BIT(2,_A);                                                } /* BIT  2,A         */

OP(cb,58) { BIT(3,_B);                                                } /* BIT  3,B         */
OP(cb,59) { BIT(3,_C);                                                } /* BIT  3,C         */
OP(cb,5a) { BIT(3,_D);                                                } /* BIT  3,D         */
OP(cb,5b) { BIT(3,_E);                                                } /* BIT  3,E         */
OP(cb,5c) { BIT(3,_H);                                                } /* BIT  3,H         */
OP(cb,5d) { BIT(3,_L);                                                } /* BIT  3,L         */
OP(cb,5e) { BIT(3,RM(_HL));                                         } /* BIT  3,(HL)      */
OP(cb,5f) { BIT(3,_A);                                                } /* BIT  3,A         */

OP(cb,60) { BIT(4,_B);                                                } /* BIT  4,B         */
OP(cb,61) { BIT(4,_C);                                                } /* BIT  4,C         */
OP(cb,62) { BIT(4,_D);                                                } /* BIT  4,D         */
OP(cb,63) { BIT(4,_E);                                                } /* BIT  4,E         */
OP(cb,64) { BIT(4,_H);                                                } /* BIT  4,H         */
OP(cb,65) { BIT(4,_L);                                                } /* BIT  4,L         */
OP(cb,66) { BIT(4,RM(_HL));                                         } /* BIT  4,(HL)      */
OP(cb,67) { BIT(4,_A);                                                } /* BIT  4,A         */

OP(cb,68) { BIT(5,_B);                                                } /* BIT  5,B         */
OP(cb,69) { BIT(5,_C);                                                } /* BIT  5,C         */
OP(cb,6a) { BIT(5,_D);                                                } /* BIT  5,D         */
OP(cb,6b) { BIT(5,_E);                                                } /* BIT  5,E         */
OP(cb,6c) { BIT(5,_H);                                                } /* BIT  5,H         */
OP(cb,6d) { BIT(5,_L);                                                } /* BIT  5,L         */
OP(cb,6e) { BIT(5,RM(_HL));                                         } /* BIT  5,(HL)      */
OP(cb,6f) { BIT(5,_A);                                                } /* BIT  5,A         */

OP(cb,70) { BIT(6,_B);                                                } /* BIT  6,B         */
OP(cb,71) { BIT(6,_C);                                                } /* BIT  6,C         */
OP(cb,72) { BIT(6,_D);                                                } /* BIT  6,D         */
OP(cb,73) { BIT(6,_E);                                                } /* BIT  6,E         */
OP(cb,74) { BIT(6,_H);                                                } /* BIT  6,H         */
OP(cb,75) { BIT(6,_L);                                                } /* BIT  6,L         */
OP(cb,76) { BIT(6,RM(_HL));                                         } /* BIT  6,(HL)      */
OP(cb,77) { BIT(6,_A);                                                } /* BIT  6,A         */

OP(cb,78) { BIT(7,_B);                                                } /* BIT  7,B         */
OP(cb,79) { BIT(7,_C);                                                } /* BIT  7,C         */
OP(cb,7a) { BIT(7,_D);                                                } /* BIT  7,D         */
OP(cb,7b) { BIT(7,_E);                                                } /* BIT  7,E         */
OP(cb,7c) { BIT(7,_H);                                                } /* BIT  7,H         */
OP(cb,7d) { BIT(7,_L);                                                } /* BIT  7,L         */
OP(cb,7e) { BIT(7,RM(_HL));                                         } /* BIT  7,(HL)      */
OP(cb,7f) { BIT(7,_A);                                                } /* BIT  7,A         */

OP(cb,80) { _B = RES(0,_B);                                         } /* RES  0,B         */
OP(cb,81) { _C = RES(0,_C);                                         } /* RES  0,C         */
OP(cb,82) { _D = RES(0,_D);                                         } /* RES  0,D         */
OP(cb,83) { _E = RES(0,_E);                                         } /* RES  0,E         */
OP(cb,84) { _H = RES(0,_H);                                         } /* RES  0,H         */
OP(cb,85) { _L = RES(0,_L);                                         } /* RES  0,L         */
OP(cb,86) { WM( _HL, RES(0,RM(_HL)) );                              } /* RES  0,(HL)      */
OP(cb,87) { _A = RES(0,_A);                                         } /* RES  0,A         */

OP(cb,88) { _B = RES(1,_B);                                         } /* RES  1,B         */
OP(cb,89) { _C = RES(1,_C);                                         } /* RES  1,C         */
OP(cb,8a) { _D = RES(1,_D);                                         } /* RES  1,D         */
OP(cb,8b) { _E = RES(1,_E);                                         } /* RES  1,E         */
OP(cb,8c) { _H = RES(1,_H);                                         } /* RES  1,H         */
OP(cb,8d) { _L = RES(1,_L);                                         } /* RES  1,L         */
OP(cb,8e) { WM( _HL, RES(1,RM(_HL)) );                              } /* RES  1,(HL)      */
OP(cb,8f) { _A = RES(1,_A);                                         } /* RES  1,A         */

OP(cb,90) { _B = RES(2,_B);                                         } /* RES  2,B         */
OP(cb,91) { _C = RES(2,_C);                                         } /* RES  2,C         */
OP(cb,92) { _D = RES(2,_D);                                         } /* RES  2,D         */
OP(cb,93) { _E = RES(2,_E);                                         } /* RES  2,E         */
OP(cb,94) { _H = RES(2,_H);                                         } /* RES  2,H         */
OP(cb,95) { _L = RES(2,_L);                                         } /* RES  2,L         */
OP(cb,96) { WM( _HL, RES(2,RM(_HL)) );                              } /* RES  2,(HL)      */
OP(cb,97) { _A = RES(2,_A);                                         } /* RES  2,A         */

OP(cb,98) { _B = RES(3,_B);                                         } /* RES  3,B         */
OP(cb,99) { _C = RES(3,_C);                                         } /* RES  3,C         */
OP(cb,9a) { _D = RES(3,_D);                                         } /* RES  3,D         */
OP(cb,9b) { _E = RES(3,_E);                                         } /* RES  3,E         */
OP(cb,9c) { _H = RES(3,_H);                                         } /* RES  3,H         */
OP(cb,9d) { _L = RES(3,_L);                                         } /* RES  3,L         */
OP(cb,9e) { WM( _HL, RES(3,RM(_HL)) );                              } /* RES  3,(HL)      */
OP(cb,9f) { _A = RES(3,_A);                                         } /* RES  3,A         */

OP(cb,a0) { _B = RES(4,_B);                                         } /* RES  4,B         */
OP(cb,a1) { _C = RES(4,_C);                                         } /* RES  4,C         */
OP(cb,a2) { _D = RES(4,_D);                                         } /* RES  4,D         */
OP(cb,a3) { _E = RES(4,_E);                                         } /* RES  4,E         */
OP(cb,a4) { _H = RES(4,_H);                                         } /* RES  4,H         */
OP(cb,a5) { _L = RES(4,_L);                                         } /* RES  4,L         */
OP(cb,a6) { WM( _HL, RES(4,RM(_HL)) );                              } /* RES  4,(HL)      */
OP(cb,a7) { _A = RES(4,_A);                                         } /* RES  4,A         */

OP(cb,a8) { _B = RES(5,_B);                                         } /* RES  5,B         */
OP(cb,a9) { _C = RES(5,_C);                                         } /* RES  5,C         */
OP(cb,aa) { _D = RES(5,_D);                                         } /* RES  5,D         */
OP(cb,ab) { _E = RES(5,_E);                                         } /* RES  5,E         */
OP(cb,ac) { _H = RES(5,_H);                                         } /* RES  5,H         */
OP(cb,ad) { _L = RES(5,_L);                                         } /* RES  5,L         */
OP(cb,ae) { WM( _HL, RES(5,RM(_HL)) );                              } /* RES  5,(HL)      */
OP(cb,af) { _A = RES(5,_A);                                         } /* RES  5,A         */

OP(cb,b0) { _B = RES(6,_B);                                         } /* RES  6,B         */
OP(cb,b1) { _C = RES(6,_C);                                         } /* RES  6,C         */
OP(cb,b2) { _D = RES(6,_D);                                         } /* RES  6,D         */
OP(cb,b3) { _E = RES(6,_E);                                         } /* RES  6,E         */
OP(cb,b4) { _H = RES(6,_H);                                         } /* RES  6,H         */
OP(cb,b5) { _L = RES(6,_L);                                         } /* RES  6,L         */
OP(cb,b6) { WM( _HL, RES(6,RM(_HL)) );                              } /* RES  6,(HL)      */
OP(cb,b7) { _A = RES(6,_A);                                         } /* RES  6,A         */

OP(cb,b8) { _B = RES(7,_B);                                         } /* RES  7,B         */
OP(cb,b9) { _C = RES(7,_C);                                         } /* RES  7,C         */
OP(cb,ba) { _D = RES(7,_D);                                         } /* RES  7,D         */
OP(cb,bb) { _E = RES(7,_E);                                         } /* RES  7,E         */
OP(cb,bc) { _H = RES(7,_H);                                         } /* RES  7,H         */
OP(cb,bd) { _L = RES(7,_L);                                         } /* RES  7,L         */
OP(cb,be) { WM( _HL, RES(7,RM(_HL)) );                              } /* RES  7,(HL)      */
OP(cb,bf) { _A = RES(7,_A);                                         } /* RES  7,A         */

OP(cb,c0) { _B = SET(0,_B);                                         } /* SET  0,B         */
OP(cb,c1) { _C = SET(0,_C);                                         } /* SET  0,C         */
OP(cb,c2) { _D = SET(0,_D);                                         } /* SET  0,D         */
OP(cb,c3) { _E = SET(0,_E);                                         } /* SET  0,E         */
OP(cb,c4) { _H = SET(0,_H);                                         } /* SET  0,H         */
OP(cb,c5) { _L = SET(0,_L);                                         } /* SET  0,L         */
OP(cb,c6) { WM( _HL, SET(0,RM(_HL)) );                              } /* SET  0,(HL)      */
OP(cb,c7) { _A = SET(0,_A);                                         } /* SET  0,A         */

OP(cb,c8) { _B = SET(1,_B);                                         } /* SET  1,B         */
OP(cb,c9) { _C = SET(1,_C);                                         } /* SET  1,C         */
OP(cb,ca) { _D = SET(1,_D);                                         } /* SET  1,D         */
OP(cb,cb) { _E = SET(1,_E);                                         } /* SET  1,E         */
OP(cb,cc) { _H = SET(1,_H);                                         } /* SET  1,H         */
OP(cb,cd) { _L = SET(1,_L);                                         } /* SET  1,L         */
OP(cb,ce) { WM( _HL, SET(1,RM(_HL)) );                              } /* SET  1,(HL)      */
OP(cb,cf) { _A = SET(1,_A);                                         } /* SET  1,A         */

OP(cb,d0) { _B = SET(2,_B);                                         } /* SET  2,B         */
OP(cb,d1) { _C = SET(2,_C);                                         } /* SET  2,C         */
OP(cb,d2) { _D = SET(2,_D);                                         } /* SET  2,D         */
OP(cb,d3) { _E = SET(2,_E);                                         } /* SET  2,E         */
OP(cb,d4) { _H = SET(2,_H);                                         } /* SET  2,H         */
OP(cb,d5) { _L = SET(2,_L);                                         } /* SET  2,L         */
OP(cb,d6) { WM( _HL, SET(2,RM(_HL)) );                              }/* SET  2,(HL)      */
OP(cb,d7) { _A = SET(2,_A);                                         } /* SET  2,A         */

OP(cb,d8) { _B = SET(3,_B);                                         } /* SET  3,B         */
OP(cb,d9) { _C = SET(3,_C);                                         } /* SET  3,C         */
OP(cb,da) { _D = SET(3,_D);                                         } /* SET  3,D         */
OP(cb,db) { _E = SET(3,_E);                                         } /* SET  3,E         */
OP(cb,dc) { _H = SET(3,_H);                                         } /* SET  3,H         */
OP(cb,dd) { _L = SET(3,_L);                                         } /* SET  3,L         */
OP(cb,de) { WM( _HL, SET(3,RM(_HL)) );                              } /* SET  3,(HL)      */
OP(cb,df) { _A = SET(3,_A);                                         } /* SET  3,A         */

OP(cb,e0) { _B = SET(4,_B);                                         } /* SET  4,B         */
OP(cb,e1) { _C = SET(4,_C);                                         } /* SET  4,C         */
OP(cb,e2) { _D = SET(4,_D);                                         } /* SET  4,D         */
OP(cb,e3) { _E = SET(4,_E);                                         } /* SET  4,E         */
OP(cb,e4) { _H = SET(4,_H);                                         } /* SET  4,H         */
OP(cb,e5) { _L = SET(4,_L);                                         } /* SET  4,L         */
OP(cb,e6) { WM( _HL, SET(4,RM(_HL)) );                              } /* SET  4,(HL)      */
OP(cb,e7) { _A = SET(4,_A);                                         } /* SET  4,A         */

OP(cb,e8) { _B = SET(5,_B);                                         } /* SET  5,B         */
OP(cb,e9) { _C = SET(5,_C);                                         } /* SET  5,C         */
OP(cb,ea) { _D = SET(5,_D);                                         } /* SET  5,D         */
OP(cb,eb) { _E = SET(5,_E);                                         } /* SET  5,E         */
OP(cb,ec) { _H = SET(5,_H);                                         } /* SET  5,H         */
OP(cb,ed) { _L = SET(5,_L);                                         } /* SET  5,L         */
OP(cb,ee) { WM( _HL, SET(5,RM(_HL)) );                              } /* SET  5,(HL)      */
OP(cb,ef) { _A = SET(5,_A);                                         } /* SET  5,A         */

OP(cb,f0) { _B = SET(6,_B);                                         } /* SET  6,B         */
OP(cb,f1) { _C = SET(6,_C);                                         } /* SET  6,C         */
OP(cb,f2) { _D = SET(6,_D);                                         } /* SET  6,D         */
OP(cb,f3) { _E = SET(6,_E);                                         } /* SET  6,E         */
OP(cb,f4) { _H = SET(6,_H);                                         } /* SET  6,H         */
OP(cb,f5) { _L = SET(6,_L);                                         } /* SET  6,L         */
OP(cb,f6) { WM( _HL, SET(6,RM(_HL)) );                              } /* SET  6,(HL)      */
OP(cb,f7) { _A = SET(6,_A);                                         } /* SET  6,A         */

OP(cb,f8) { _B = SET(7,_B);                                         } /* SET  7,B         */
OP(cb,f9) { _C = SET(7,_C);                                         } /* SET  7,C         */
OP(cb,fa) { _D = SET(7,_D);                                         } /* SET  7,D         */
OP(cb,fb) { _E = SET(7,_E);                                         } /* SET  7,E         */
OP(cb,fc) { _H = SET(7,_H);                                         } /* SET  7,H         */
OP(cb,fd) { _L = SET(7,_L);                                         } /* SET  7,L         */
OP(cb,fe) { WM( _HL, SET(7,RM(_HL)) );                              } /* SET  7,(HL)      */
OP(cb,ff) { _A = SET(7,_A);                                         } /* SET  7,A         */
