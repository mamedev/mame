/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
OP(cb,00) { cpustate->_B = RLC(cpustate, cpustate->_B);                                         } /* RLC  B           */
OP(cb,01) { cpustate->_C = RLC(cpustate, cpustate->_C);                                         } /* RLC  C           */
OP(cb,02) { cpustate->_D = RLC(cpustate, cpustate->_D);                                         } /* RLC  D           */
OP(cb,03) { cpustate->_E = RLC(cpustate, cpustate->_E);                                         } /* RLC  E           */
OP(cb,04) { cpustate->_H = RLC(cpustate, cpustate->_H);                                         } /* RLC  H           */
OP(cb,05) { cpustate->_L = RLC(cpustate, cpustate->_L);                                         } /* RLC  L           */
OP(cb,06) { WM(cpustate,  cpustate->_HL, RLC(cpustate, RM(cpustate, cpustate->_HL)) );                              } /* RLC  (HL)        */
OP(cb,07) { cpustate->_A = RLC(cpustate, cpustate->_A);                                         } /* RLC  A           */

OP(cb,08) { cpustate->_B = RRC(cpustate, cpustate->_B);                                         } /* RRC  B           */
OP(cb,09) { cpustate->_C = RRC(cpustate, cpustate->_C);                                         } /* RRC  C           */
OP(cb,0a) { cpustate->_D = RRC(cpustate, cpustate->_D);                                         } /* RRC  D           */
OP(cb,0b) { cpustate->_E = RRC(cpustate, cpustate->_E);                                         } /* RRC  E           */
OP(cb,0c) { cpustate->_H = RRC(cpustate, cpustate->_H);                                         } /* RRC  H           */
OP(cb,0d) { cpustate->_L = RRC(cpustate, cpustate->_L);                                         } /* RRC  L           */
OP(cb,0e) { WM(cpustate,  cpustate->_HL, RRC(cpustate, RM(cpustate, cpustate->_HL)) );                              } /* RRC  (HL)        */
OP(cb,0f) { cpustate->_A = RRC(cpustate, cpustate->_A);                                         } /* RRC  A           */

OP(cb,10) { cpustate->_B = RL(cpustate, cpustate->_B);                                          } /* RL   B           */
OP(cb,11) { cpustate->_C = RL(cpustate, cpustate->_C);                                          } /* RL   C           */
OP(cb,12) { cpustate->_D = RL(cpustate, cpustate->_D);                                          } /* RL   D           */
OP(cb,13) { cpustate->_E = RL(cpustate, cpustate->_E);                                          } /* RL   E           */
OP(cb,14) { cpustate->_H = RL(cpustate, cpustate->_H);                                          } /* RL   H           */
OP(cb,15) { cpustate->_L = RL(cpustate, cpustate->_L);                                          } /* RL   L           */
OP(cb,16) { WM(cpustate,  cpustate->_HL, RL(cpustate, RM(cpustate, cpustate->_HL)) );                               } /* RL   (HL)        */
OP(cb,17) { cpustate->_A = RL(cpustate, cpustate->_A);                                          } /* RL   A           */

OP(cb,18) { cpustate->_B = RR(cpustate, cpustate->_B);                                          } /* RR   B           */
OP(cb,19) { cpustate->_C = RR(cpustate, cpustate->_C);                                          } /* RR   C           */
OP(cb,1a) { cpustate->_D = RR(cpustate, cpustate->_D);                                          } /* RR   D           */
OP(cb,1b) { cpustate->_E = RR(cpustate, cpustate->_E);                                          } /* RR   E           */
OP(cb,1c) { cpustate->_H = RR(cpustate, cpustate->_H);                                          } /* RR   H           */
OP(cb,1d) { cpustate->_L = RR(cpustate, cpustate->_L);                                          } /* RR   L           */
OP(cb,1e) { WM(cpustate,  cpustate->_HL, RR(cpustate, RM(cpustate, cpustate->_HL)) );                               } /* RR   (HL)        */
OP(cb,1f) { cpustate->_A = RR(cpustate, cpustate->_A);                                          } /* RR   A           */

OP(cb,20) { cpustate->_B = SLA(cpustate, cpustate->_B);                                         } /* SLA  B           */
OP(cb,21) { cpustate->_C = SLA(cpustate, cpustate->_C);                                         } /* SLA  C           */
OP(cb,22) { cpustate->_D = SLA(cpustate, cpustate->_D);                                         } /* SLA  D           */
OP(cb,23) { cpustate->_E = SLA(cpustate, cpustate->_E);                                         } /* SLA  E           */
OP(cb,24) { cpustate->_H = SLA(cpustate, cpustate->_H);                                         } /* SLA  H           */
OP(cb,25) { cpustate->_L = SLA(cpustate, cpustate->_L);                                         } /* SLA  L           */
OP(cb,26) { WM(cpustate,  cpustate->_HL, SLA(cpustate, RM(cpustate, cpustate->_HL)) );                              } /* SLA  (HL)        */
OP(cb,27) { cpustate->_A = SLA(cpustate, cpustate->_A);                                         } /* SLA  A           */

OP(cb,28) { cpustate->_B = SRA(cpustate, cpustate->_B);                                         } /* SRA  B           */
OP(cb,29) { cpustate->_C = SRA(cpustate, cpustate->_C);                                         } /* SRA  C           */
OP(cb,2a) { cpustate->_D = SRA(cpustate, cpustate->_D);                                         } /* SRA  D           */
OP(cb,2b) { cpustate->_E = SRA(cpustate, cpustate->_E);                                         } /* SRA  E           */
OP(cb,2c) { cpustate->_H = SRA(cpustate, cpustate->_H);                                         } /* SRA  H           */
OP(cb,2d) { cpustate->_L = SRA(cpustate, cpustate->_L);                                         } /* SRA  L           */
OP(cb,2e) { WM(cpustate,  cpustate->_HL, SRA(cpustate, RM(cpustate, cpustate->_HL)) );                              } /* SRA  (HL)        */
OP(cb,2f) { cpustate->_A = SRA(cpustate, cpustate->_A);                                         } /* SRA  A           */

OP(cb,30) { cpustate->_B = SLL(cpustate, cpustate->_B);                                         } /* SLL  B           */
OP(cb,31) { cpustate->_C = SLL(cpustate, cpustate->_C);                                         } /* SLL  C           */
OP(cb,32) { cpustate->_D = SLL(cpustate, cpustate->_D);                                         } /* SLL  D           */
OP(cb,33) { cpustate->_E = SLL(cpustate, cpustate->_E);                                         } /* SLL  E           */
OP(cb,34) { cpustate->_H = SLL(cpustate, cpustate->_H);                                         } /* SLL  H           */
OP(cb,35) { cpustate->_L = SLL(cpustate, cpustate->_L);                                         } /* SLL  L           */
OP(cb,36) { WM(cpustate,  cpustate->_HL, SLL(cpustate, RM(cpustate, cpustate->_HL)) );                              } /* SLL  (HL)        */
OP(cb,37) { cpustate->_A = SLL(cpustate, cpustate->_A);                                         } /* SLL  A           */

OP(cb,38) { cpustate->_B = SRL(cpustate, cpustate->_B);                                         } /* SRL  B           */
OP(cb,39) { cpustate->_C = SRL(cpustate, cpustate->_C);                                         } /* SRL  C           */
OP(cb,3a) { cpustate->_D = SRL(cpustate, cpustate->_D);                                         } /* SRL  D           */
OP(cb,3b) { cpustate->_E = SRL(cpustate, cpustate->_E);                                         } /* SRL  E           */
OP(cb,3c) { cpustate->_H = SRL(cpustate, cpustate->_H);                                         } /* SRL  H           */
OP(cb,3d) { cpustate->_L = SRL(cpustate, cpustate->_L);                                         } /* SRL  L           */
OP(cb,3e) { WM(cpustate,  cpustate->_HL, SRL(cpustate, RM(cpustate, cpustate->_HL)) );                              } /* SRL  (HL)        */
OP(cb,3f) { cpustate->_A = SRL(cpustate, cpustate->_A);                                         } /* SRL  A           */

OP(cb,40) { BIT(0,cpustate->_B);                                                } /* BIT  0,B         */
OP(cb,41) { BIT(0,cpustate->_C);                                                } /* BIT  0,C         */
OP(cb,42) { BIT(0,cpustate->_D);                                                } /* BIT  0,D         */
OP(cb,43) { BIT(0,cpustate->_E);                                                } /* BIT  0,E         */
OP(cb,44) { BIT(0,cpustate->_H);                                                } /* BIT  0,H         */
OP(cb,45) { BIT(0,cpustate->_L);                                                } /* BIT  0,L         */
OP(cb,46) { BIT(0,RM(cpustate, cpustate->_HL));                                         } /* BIT  0,(HL)      */
OP(cb,47) { BIT(0,cpustate->_A);                                                } /* BIT  0,A         */

OP(cb,48) { BIT(1,cpustate->_B);                                                } /* BIT  1,B         */
OP(cb,49) { BIT(1,cpustate->_C);                                                } /* BIT  1,C         */
OP(cb,4a) { BIT(1,cpustate->_D);                                                } /* BIT  1,D         */
OP(cb,4b) { BIT(1,cpustate->_E);                                                } /* BIT  1,E         */
OP(cb,4c) { BIT(1,cpustate->_H);                                                } /* BIT  1,H         */
OP(cb,4d) { BIT(1,cpustate->_L);                                                } /* BIT  1,L         */
OP(cb,4e) { BIT(1,RM(cpustate, cpustate->_HL));                                         } /* BIT  1,(HL)      */
OP(cb,4f) { BIT(1,cpustate->_A);                                                } /* BIT  1,A         */

OP(cb,50) { BIT(2,cpustate->_B);                                                } /* BIT  2,B         */
OP(cb,51) { BIT(2,cpustate->_C);                                                } /* BIT  2,C         */
OP(cb,52) { BIT(2,cpustate->_D);                                                } /* BIT  2,D         */
OP(cb,53) { BIT(2,cpustate->_E);                                                } /* BIT  2,E         */
OP(cb,54) { BIT(2,cpustate->_H);                                                } /* BIT  2,H         */
OP(cb,55) { BIT(2,cpustate->_L);                                                } /* BIT  2,L         */
OP(cb,56) { BIT(2,RM(cpustate, cpustate->_HL));                                         } /* BIT  2,(HL)      */
OP(cb,57) { BIT(2,cpustate->_A);                                                } /* BIT  2,A         */

OP(cb,58) { BIT(3,cpustate->_B);                                                } /* BIT  3,B         */
OP(cb,59) { BIT(3,cpustate->_C);                                                } /* BIT  3,C         */
OP(cb,5a) { BIT(3,cpustate->_D);                                                } /* BIT  3,D         */
OP(cb,5b) { BIT(3,cpustate->_E);                                                } /* BIT  3,E         */
OP(cb,5c) { BIT(3,cpustate->_H);                                                } /* BIT  3,H         */
OP(cb,5d) { BIT(3,cpustate->_L);                                                } /* BIT  3,L         */
OP(cb,5e) { BIT(3,RM(cpustate, cpustate->_HL));                                         } /* BIT  3,(HL)      */
OP(cb,5f) { BIT(3,cpustate->_A);                                                } /* BIT  3,A         */

OP(cb,60) { BIT(4,cpustate->_B);                                                } /* BIT  4,B         */
OP(cb,61) { BIT(4,cpustate->_C);                                                } /* BIT  4,C         */
OP(cb,62) { BIT(4,cpustate->_D);                                                } /* BIT  4,D         */
OP(cb,63) { BIT(4,cpustate->_E);                                                } /* BIT  4,E         */
OP(cb,64) { BIT(4,cpustate->_H);                                                } /* BIT  4,H         */
OP(cb,65) { BIT(4,cpustate->_L);                                                } /* BIT  4,L         */
OP(cb,66) { BIT(4,RM(cpustate, cpustate->_HL));                                         } /* BIT  4,(HL)      */
OP(cb,67) { BIT(4,cpustate->_A);                                                } /* BIT  4,A         */

OP(cb,68) { BIT(5,cpustate->_B);                                                } /* BIT  5,B         */
OP(cb,69) { BIT(5,cpustate->_C);                                                } /* BIT  5,C         */
OP(cb,6a) { BIT(5,cpustate->_D);                                                } /* BIT  5,D         */
OP(cb,6b) { BIT(5,cpustate->_E);                                                } /* BIT  5,E         */
OP(cb,6c) { BIT(5,cpustate->_H);                                                } /* BIT  5,H         */
OP(cb,6d) { BIT(5,cpustate->_L);                                                } /* BIT  5,L         */
OP(cb,6e) { BIT(5,RM(cpustate, cpustate->_HL));                                         } /* BIT  5,(HL)      */
OP(cb,6f) { BIT(5,cpustate->_A);                                                } /* BIT  5,A         */

OP(cb,70) { BIT(6,cpustate->_B);                                                } /* BIT  6,B         */
OP(cb,71) { BIT(6,cpustate->_C);                                                } /* BIT  6,C         */
OP(cb,72) { BIT(6,cpustate->_D);                                                } /* BIT  6,D         */
OP(cb,73) { BIT(6,cpustate->_E);                                                } /* BIT  6,E         */
OP(cb,74) { BIT(6,cpustate->_H);                                                } /* BIT  6,H         */
OP(cb,75) { BIT(6,cpustate->_L);                                                } /* BIT  6,L         */
OP(cb,76) { BIT(6,RM(cpustate, cpustate->_HL));                                         } /* BIT  6,(HL)      */
OP(cb,77) { BIT(6,cpustate->_A);                                                } /* BIT  6,A         */

OP(cb,78) { BIT(7,cpustate->_B);                                                } /* BIT  7,B         */
OP(cb,79) { BIT(7,cpustate->_C);                                                } /* BIT  7,C         */
OP(cb,7a) { BIT(7,cpustate->_D);                                                } /* BIT  7,D         */
OP(cb,7b) { BIT(7,cpustate->_E);                                                } /* BIT  7,E         */
OP(cb,7c) { BIT(7,cpustate->_H);                                                } /* BIT  7,H         */
OP(cb,7d) { BIT(7,cpustate->_L);                                                } /* BIT  7,L         */
OP(cb,7e) { BIT(7,RM(cpustate, cpustate->_HL));                                         } /* BIT  7,(HL)      */
OP(cb,7f) { BIT(7,cpustate->_A);                                                } /* BIT  7,A         */

OP(cb,80) { cpustate->_B = RES(0,cpustate->_B);                                         } /* RES  0,B         */
OP(cb,81) { cpustate->_C = RES(0,cpustate->_C);                                         } /* RES  0,C         */
OP(cb,82) { cpustate->_D = RES(0,cpustate->_D);                                         } /* RES  0,D         */
OP(cb,83) { cpustate->_E = RES(0,cpustate->_E);                                         } /* RES  0,E         */
OP(cb,84) { cpustate->_H = RES(0,cpustate->_H);                                         } /* RES  0,H         */
OP(cb,85) { cpustate->_L = RES(0,cpustate->_L);                                         } /* RES  0,L         */
OP(cb,86) { WM(cpustate,  cpustate->_HL, RES(0,RM(cpustate, cpustate->_HL)) );                              } /* RES  0,(HL)      */
OP(cb,87) { cpustate->_A = RES(0,cpustate->_A);                                         } /* RES  0,A         */

OP(cb,88) { cpustate->_B = RES(1,cpustate->_B);                                         } /* RES  1,B         */
OP(cb,89) { cpustate->_C = RES(1,cpustate->_C);                                         } /* RES  1,C         */
OP(cb,8a) { cpustate->_D = RES(1,cpustate->_D);                                         } /* RES  1,D         */
OP(cb,8b) { cpustate->_E = RES(1,cpustate->_E);                                         } /* RES  1,E         */
OP(cb,8c) { cpustate->_H = RES(1,cpustate->_H);                                         } /* RES  1,H         */
OP(cb,8d) { cpustate->_L = RES(1,cpustate->_L);                                         } /* RES  1,L         */
OP(cb,8e) { WM(cpustate,  cpustate->_HL, RES(1,RM(cpustate, cpustate->_HL)) );                              } /* RES  1,(HL)      */
OP(cb,8f) { cpustate->_A = RES(1,cpustate->_A);                                         } /* RES  1,A         */

OP(cb,90) { cpustate->_B = RES(2,cpustate->_B);                                         } /* RES  2,B         */
OP(cb,91) { cpustate->_C = RES(2,cpustate->_C);                                         } /* RES  2,C         */
OP(cb,92) { cpustate->_D = RES(2,cpustate->_D);                                         } /* RES  2,D         */
OP(cb,93) { cpustate->_E = RES(2,cpustate->_E);                                         } /* RES  2,E         */
OP(cb,94) { cpustate->_H = RES(2,cpustate->_H);                                         } /* RES  2,H         */
OP(cb,95) { cpustate->_L = RES(2,cpustate->_L);                                         } /* RES  2,L         */
OP(cb,96) { WM(cpustate,  cpustate->_HL, RES(2,RM(cpustate, cpustate->_HL)) );                              } /* RES  2,(HL)      */
OP(cb,97) { cpustate->_A = RES(2,cpustate->_A);                                         } /* RES  2,A         */

OP(cb,98) { cpustate->_B = RES(3,cpustate->_B);                                         } /* RES  3,B         */
OP(cb,99) { cpustate->_C = RES(3,cpustate->_C);                                         } /* RES  3,C         */
OP(cb,9a) { cpustate->_D = RES(3,cpustate->_D);                                         } /* RES  3,D         */
OP(cb,9b) { cpustate->_E = RES(3,cpustate->_E);                                         } /* RES  3,E         */
OP(cb,9c) { cpustate->_H = RES(3,cpustate->_H);                                         } /* RES  3,H         */
OP(cb,9d) { cpustate->_L = RES(3,cpustate->_L);                                         } /* RES  3,L         */
OP(cb,9e) { WM(cpustate,  cpustate->_HL, RES(3,RM(cpustate, cpustate->_HL)) );                              } /* RES  3,(HL)      */
OP(cb,9f) { cpustate->_A = RES(3,cpustate->_A);                                         } /* RES  3,A         */

OP(cb,a0) { cpustate->_B = RES(4,cpustate->_B);                                         } /* RES  4,B         */
OP(cb,a1) { cpustate->_C = RES(4,cpustate->_C);                                         } /* RES  4,C         */
OP(cb,a2) { cpustate->_D = RES(4,cpustate->_D);                                         } /* RES  4,D         */
OP(cb,a3) { cpustate->_E = RES(4,cpustate->_E);                                         } /* RES  4,E         */
OP(cb,a4) { cpustate->_H = RES(4,cpustate->_H);                                         } /* RES  4,H         */
OP(cb,a5) { cpustate->_L = RES(4,cpustate->_L);                                         } /* RES  4,L         */
OP(cb,a6) { WM(cpustate,  cpustate->_HL, RES(4,RM(cpustate, cpustate->_HL)) );                              } /* RES  4,(HL)      */
OP(cb,a7) { cpustate->_A = RES(4,cpustate->_A);                                         } /* RES  4,A         */

OP(cb,a8) { cpustate->_B = RES(5,cpustate->_B);                                         } /* RES  5,B         */
OP(cb,a9) { cpustate->_C = RES(5,cpustate->_C);                                         } /* RES  5,C         */
OP(cb,aa) { cpustate->_D = RES(5,cpustate->_D);                                         } /* RES  5,D         */
OP(cb,ab) { cpustate->_E = RES(5,cpustate->_E);                                         } /* RES  5,E         */
OP(cb,ac) { cpustate->_H = RES(5,cpustate->_H);                                         } /* RES  5,H         */
OP(cb,ad) { cpustate->_L = RES(5,cpustate->_L);                                         } /* RES  5,L         */
OP(cb,ae) { WM(cpustate,  cpustate->_HL, RES(5,RM(cpustate, cpustate->_HL)) );                              } /* RES  5,(HL)      */
OP(cb,af) { cpustate->_A = RES(5,cpustate->_A);                                         } /* RES  5,A         */

OP(cb,b0) { cpustate->_B = RES(6,cpustate->_B);                                         } /* RES  6,B         */
OP(cb,b1) { cpustate->_C = RES(6,cpustate->_C);                                         } /* RES  6,C         */
OP(cb,b2) { cpustate->_D = RES(6,cpustate->_D);                                         } /* RES  6,D         */
OP(cb,b3) { cpustate->_E = RES(6,cpustate->_E);                                         } /* RES  6,E         */
OP(cb,b4) { cpustate->_H = RES(6,cpustate->_H);                                         } /* RES  6,H         */
OP(cb,b5) { cpustate->_L = RES(6,cpustate->_L);                                         } /* RES  6,L         */
OP(cb,b6) { WM(cpustate,  cpustate->_HL, RES(6,RM(cpustate, cpustate->_HL)) );                              } /* RES  6,(HL)      */
OP(cb,b7) { cpustate->_A = RES(6,cpustate->_A);                                         } /* RES  6,A         */

OP(cb,b8) { cpustate->_B = RES(7,cpustate->_B);                                         } /* RES  7,B         */
OP(cb,b9) { cpustate->_C = RES(7,cpustate->_C);                                         } /* RES  7,C         */
OP(cb,ba) { cpustate->_D = RES(7,cpustate->_D);                                         } /* RES  7,D         */
OP(cb,bb) { cpustate->_E = RES(7,cpustate->_E);                                         } /* RES  7,E         */
OP(cb,bc) { cpustate->_H = RES(7,cpustate->_H);                                         } /* RES  7,H         */
OP(cb,bd) { cpustate->_L = RES(7,cpustate->_L);                                         } /* RES  7,L         */
OP(cb,be) { WM(cpustate,  cpustate->_HL, RES(7,RM(cpustate, cpustate->_HL)) );                              } /* RES  7,(HL)      */
OP(cb,bf) { cpustate->_A = RES(7,cpustate->_A);                                         } /* RES  7,A         */

OP(cb,c0) { cpustate->_B = SET(0,cpustate->_B);                                         } /* SET  0,B         */
OP(cb,c1) { cpustate->_C = SET(0,cpustate->_C);                                         } /* SET  0,C         */
OP(cb,c2) { cpustate->_D = SET(0,cpustate->_D);                                         } /* SET  0,D         */
OP(cb,c3) { cpustate->_E = SET(0,cpustate->_E);                                         } /* SET  0,E         */
OP(cb,c4) { cpustate->_H = SET(0,cpustate->_H);                                         } /* SET  0,H         */
OP(cb,c5) { cpustate->_L = SET(0,cpustate->_L);                                         } /* SET  0,L         */
OP(cb,c6) { WM(cpustate,  cpustate->_HL, SET(0,RM(cpustate, cpustate->_HL)) );                              } /* SET  0,(HL)      */
OP(cb,c7) { cpustate->_A = SET(0,cpustate->_A);                                         } /* SET  0,A         */

OP(cb,c8) { cpustate->_B = SET(1,cpustate->_B);                                         } /* SET  1,B         */
OP(cb,c9) { cpustate->_C = SET(1,cpustate->_C);                                         } /* SET  1,C         */
OP(cb,ca) { cpustate->_D = SET(1,cpustate->_D);                                         } /* SET  1,D         */
OP(cb,cb) { cpustate->_E = SET(1,cpustate->_E);                                         } /* SET  1,E         */
OP(cb,cc) { cpustate->_H = SET(1,cpustate->_H);                                         } /* SET  1,H         */
OP(cb,cd) { cpustate->_L = SET(1,cpustate->_L);                                         } /* SET  1,L         */
OP(cb,ce) { WM(cpustate,  cpustate->_HL, SET(1,RM(cpustate, cpustate->_HL)) );                              } /* SET  1,(HL)      */
OP(cb,cf) { cpustate->_A = SET(1,cpustate->_A);                                         } /* SET  1,A         */

OP(cb,d0) { cpustate->_B = SET(2,cpustate->_B);                                         } /* SET  2,B         */
OP(cb,d1) { cpustate->_C = SET(2,cpustate->_C);                                         } /* SET  2,C         */
OP(cb,d2) { cpustate->_D = SET(2,cpustate->_D);                                         } /* SET  2,D         */
OP(cb,d3) { cpustate->_E = SET(2,cpustate->_E);                                         } /* SET  2,E         */
OP(cb,d4) { cpustate->_H = SET(2,cpustate->_H);                                         } /* SET  2,H         */
OP(cb,d5) { cpustate->_L = SET(2,cpustate->_L);                                         } /* SET  2,L         */
OP(cb,d6) { WM(cpustate,  cpustate->_HL, SET(2,RM(cpustate, cpustate->_HL)) );                              }/* SET  2,(HL)      */
OP(cb,d7) { cpustate->_A = SET(2,cpustate->_A);                                         } /* SET  2,A         */

OP(cb,d8) { cpustate->_B = SET(3,cpustate->_B);                                         } /* SET  3,B         */
OP(cb,d9) { cpustate->_C = SET(3,cpustate->_C);                                         } /* SET  3,C         */
OP(cb,da) { cpustate->_D = SET(3,cpustate->_D);                                         } /* SET  3,D         */
OP(cb,db) { cpustate->_E = SET(3,cpustate->_E);                                         } /* SET  3,E         */
OP(cb,dc) { cpustate->_H = SET(3,cpustate->_H);                                         } /* SET  3,H         */
OP(cb,dd) { cpustate->_L = SET(3,cpustate->_L);                                         } /* SET  3,L         */
OP(cb,de) { WM(cpustate,  cpustate->_HL, SET(3,RM(cpustate, cpustate->_HL)) );                              } /* SET  3,(HL)      */
OP(cb,df) { cpustate->_A = SET(3,cpustate->_A);                                         } /* SET  3,A         */

OP(cb,e0) { cpustate->_B = SET(4,cpustate->_B);                                         } /* SET  4,B         */
OP(cb,e1) { cpustate->_C = SET(4,cpustate->_C);                                         } /* SET  4,C         */
OP(cb,e2) { cpustate->_D = SET(4,cpustate->_D);                                         } /* SET  4,D         */
OP(cb,e3) { cpustate->_E = SET(4,cpustate->_E);                                         } /* SET  4,E         */
OP(cb,e4) { cpustate->_H = SET(4,cpustate->_H);                                         } /* SET  4,H         */
OP(cb,e5) { cpustate->_L = SET(4,cpustate->_L);                                         } /* SET  4,L         */
OP(cb,e6) { WM(cpustate,  cpustate->_HL, SET(4,RM(cpustate, cpustate->_HL)) );                              } /* SET  4,(HL)      */
OP(cb,e7) { cpustate->_A = SET(4,cpustate->_A);                                         } /* SET  4,A         */

OP(cb,e8) { cpustate->_B = SET(5,cpustate->_B);                                         } /* SET  5,B         */
OP(cb,e9) { cpustate->_C = SET(5,cpustate->_C);                                         } /* SET  5,C         */
OP(cb,ea) { cpustate->_D = SET(5,cpustate->_D);                                         } /* SET  5,D         */
OP(cb,eb) { cpustate->_E = SET(5,cpustate->_E);                                         } /* SET  5,E         */
OP(cb,ec) { cpustate->_H = SET(5,cpustate->_H);                                         } /* SET  5,H         */
OP(cb,ed) { cpustate->_L = SET(5,cpustate->_L);                                         } /* SET  5,L         */
OP(cb,ee) { WM(cpustate,  cpustate->_HL, SET(5,RM(cpustate, cpustate->_HL)) );                              } /* SET  5,(HL)      */
OP(cb,ef) { cpustate->_A = SET(5,cpustate->_A);                                         } /* SET  5,A         */

OP(cb,f0) { cpustate->_B = SET(6,cpustate->_B);                                         } /* SET  6,B         */
OP(cb,f1) { cpustate->_C = SET(6,cpustate->_C);                                         } /* SET  6,C         */
OP(cb,f2) { cpustate->_D = SET(6,cpustate->_D);                                         } /* SET  6,D         */
OP(cb,f3) { cpustate->_E = SET(6,cpustate->_E);                                         } /* SET  6,E         */
OP(cb,f4) { cpustate->_H = SET(6,cpustate->_H);                                         } /* SET  6,H         */
OP(cb,f5) { cpustate->_L = SET(6,cpustate->_L);                                         } /* SET  6,L         */
OP(cb,f6) { WM(cpustate,  cpustate->_HL, SET(6,RM(cpustate, cpustate->_HL)) );                              } /* SET  6,(HL)      */
OP(cb,f7) { cpustate->_A = SET(6,cpustate->_A);                                         } /* SET  6,A         */

OP(cb,f8) { cpustate->_B = SET(7,cpustate->_B);                                         } /* SET  7,B         */
OP(cb,f9) { cpustate->_C = SET(7,cpustate->_C);                                         } /* SET  7,C         */
OP(cb,fa) { cpustate->_D = SET(7,cpustate->_D);                                         } /* SET  7,D         */
OP(cb,fb) { cpustate->_E = SET(7,cpustate->_E);                                         } /* SET  7,E         */
OP(cb,fc) { cpustate->_H = SET(7,cpustate->_H);                                         } /* SET  7,H         */
OP(cb,fd) { cpustate->_L = SET(7,cpustate->_L);                                         } /* SET  7,L         */
OP(cb,fe) { WM(cpustate,  cpustate->_HL, SET(7,RM(cpustate, cpustate->_HL)) );                              } /* SET  7,(HL)      */
OP(cb,ff) { cpustate->_A = SET(7,cpustate->_A);                                         } /* SET  7,A         */
