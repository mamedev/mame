/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
OP(xycb,00) { _B = RLC( RM(EA) ); WM( EA,_B );						} /* RLC  B=(XY+o)    */
OP(xycb,01) { _C = RLC( RM(EA) ); WM( EA,_C );						} /* RLC  C=(XY+o)    */
OP(xycb,02) { _D = RLC( RM(EA) ); WM( EA,_D );						} /* RLC  D=(XY+o)    */
OP(xycb,03) { _E = RLC( RM(EA) ); WM( EA,_E );						} /* RLC  E=(XY+o)    */
OP(xycb,04) { _H = RLC( RM(EA) ); WM( EA,_H );						} /* RLC  H=(XY+o)    */
OP(xycb,05) { _L = RLC( RM(EA) ); WM( EA,_L );						} /* RLC  L=(XY+o)    */
OP(xycb,06) { WM( EA, RLC( RM(EA) ) );								} /* RLC  (XY+o)      */
OP(xycb,07) { _A = RLC( RM(EA) ); WM( EA,_A );						} /* RLC  A=(XY+o)    */

OP(xycb,08) { _B = RRC( RM(EA) ); WM( EA,_B );						} /* RRC  B=(XY+o)    */
OP(xycb,09) { _C = RRC( RM(EA) ); WM( EA,_C );						} /* RRC  C=(XY+o)    */
OP(xycb,0a) { _D = RRC( RM(EA) ); WM( EA,_D );						} /* RRC  D=(XY+o)    */
OP(xycb,0b) { _E = RRC( RM(EA) ); WM( EA,_E );						} /* RRC  E=(XY+o)    */
OP(xycb,0c) { _H = RRC( RM(EA) ); WM( EA,_H );						} /* RRC  H=(XY+o)    */
OP(xycb,0d) { _L = RRC( RM(EA) ); WM( EA,_L );						} /* RRC  L=(XY+o)    */
OP(xycb,0e) { WM( EA,RRC( RM(EA) ) );								} /* RRC  (XY+o)      */
OP(xycb,0f) { _A = RRC( RM(EA) ); WM( EA,_A );						} /* RRC  A=(XY+o)    */

OP(xycb,10) { _B = RL( RM(EA) ); WM( EA,_B );						} /* RL   B=(XY+o)    */
OP(xycb,11) { _C = RL( RM(EA) ); WM( EA,_C );						} /* RL   C=(XY+o)    */
OP(xycb,12) { _D = RL( RM(EA) ); WM( EA,_D );						} /* RL   D=(XY+o)    */
OP(xycb,13) { _E = RL( RM(EA) ); WM( EA,_E );						} /* RL   E=(XY+o)    */
OP(xycb,14) { _H = RL( RM(EA) ); WM( EA,_H );						} /* RL   H=(XY+o)    */
OP(xycb,15) { _L = RL( RM(EA) ); WM( EA,_L );						} /* RL   L=(XY+o)    */
OP(xycb,16) { WM( EA,RL( RM(EA) ) );								} /* RL   (XY+o)      */
OP(xycb,17) { _A = RL( RM(EA) ); WM( EA,_A );						} /* RL   A=(XY+o)    */

OP(xycb,18) { _B = RR( RM(EA) ); WM( EA,_B );						} /* RR   B=(XY+o)    */
OP(xycb,19) { _C = RR( RM(EA) ); WM( EA,_C );						} /* RR   C=(XY+o)    */
OP(xycb,1a) { _D = RR( RM(EA) ); WM( EA,_D );						} /* RR   D=(XY+o)    */
OP(xycb,1b) { _E = RR( RM(EA) ); WM( EA,_E );						} /* RR   E=(XY+o)    */
OP(xycb,1c) { _H = RR( RM(EA) ); WM( EA,_H );						} /* RR   H=(XY+o)    */
OP(xycb,1d) { _L = RR( RM(EA) ); WM( EA,_L );						} /* RR   L=(XY+o)    */
OP(xycb,1e) { WM( EA,RR( RM(EA) ) );								} /* RR   (XY+o)      */
OP(xycb,1f) { _A = RR( RM(EA) ); WM( EA,_A );						} /* RR   A=(XY+o)    */

OP(xycb,20) { _B = SLA( RM(EA) ); WM( EA,_B );						} /* SLA  B=(XY+o)    */
OP(xycb,21) { _C = SLA( RM(EA) ); WM( EA,_C );						} /* SLA  C=(XY+o)    */
OP(xycb,22) { _D = SLA( RM(EA) ); WM( EA,_D );						} /* SLA  D=(XY+o)    */
OP(xycb,23) { _E = SLA( RM(EA) ); WM( EA,_E );						} /* SLA  E=(XY+o)    */
OP(xycb,24) { _H = SLA( RM(EA) ); WM( EA,_H );						} /* SLA  H=(XY+o)    */
OP(xycb,25) { _L = SLA( RM(EA) ); WM( EA,_L );						} /* SLA  L=(XY+o)    */
OP(xycb,26) { WM( EA,SLA( RM(EA) ) );								} /* SLA  (XY+o)      */
OP(xycb,27) { _A = SLA( RM(EA) ); WM( EA,_A );						} /* SLA  A=(XY+o)    */

OP(xycb,28) { _B = SRA( RM(EA) ); WM( EA,_B );						} /* SRA  B=(XY+o)    */
OP(xycb,29) { _C = SRA( RM(EA) ); WM( EA,_C );						} /* SRA  C=(XY+o)    */
OP(xycb,2a) { _D = SRA( RM(EA) ); WM( EA,_D );						} /* SRA  D=(XY+o)    */
OP(xycb,2b) { _E = SRA( RM(EA) ); WM( EA,_E );						} /* SRA  E=(XY+o)    */
OP(xycb,2c) { _H = SRA( RM(EA) ); WM( EA,_H );						} /* SRA  H=(XY+o)    */
OP(xycb,2d) { _L = SRA( RM(EA) ); WM( EA,_L );						} /* SRA  L=(XY+o)    */
OP(xycb,2e) { WM( EA,SRA( RM(EA) ) );								} /* SRA  (XY+o)      */
OP(xycb,2f) { _A = SRA( RM(EA) ); WM( EA,_A );						} /* SRA  A=(XY+o)    */

OP(xycb,30) { _B = SLL( RM(EA) ); WM( EA,_B );						} /* SLL  B=(XY+o)    */
OP(xycb,31) { _C = SLL( RM(EA) ); WM( EA,_C );						} /* SLL  C=(XY+o)    */
OP(xycb,32) { _D = SLL( RM(EA) ); WM( EA,_D );						} /* SLL  D=(XY+o)    */
OP(xycb,33) { _E = SLL( RM(EA) ); WM( EA,_E );						} /* SLL  E=(XY+o)    */
OP(xycb,34) { _H = SLL( RM(EA) ); WM( EA,_H );						} /* SLL  H=(XY+o)    */
OP(xycb,35) { _L = SLL( RM(EA) ); WM( EA,_L );						} /* SLL  L=(XY+o)    */
OP(xycb,36) { WM( EA,SLL( RM(EA) ) );								} /* SLL  (XY+o)      */
OP(xycb,37) { _A = SLL( RM(EA) ); WM( EA,_A );						} /* SLL  A=(XY+o)    */

OP(xycb,38) { _B = SRL( RM(EA) ); WM( EA,_B );						} /* SRL  B=(XY+o)    */
OP(xycb,39) { _C = SRL( RM(EA) ); WM( EA,_C );						} /* SRL  C=(XY+o)    */
OP(xycb,3a) { _D = SRL( RM(EA) ); WM( EA,_D );						} /* SRL  D=(XY+o)    */
OP(xycb,3b) { _E = SRL( RM(EA) ); WM( EA,_E );						} /* SRL  E=(XY+o)    */
OP(xycb,3c) { _H = SRL( RM(EA) ); WM( EA,_H );						} /* SRL  H=(XY+o)    */
OP(xycb,3d) { _L = SRL( RM(EA) ); WM( EA,_L );						} /* SRL  L=(XY+o)    */
OP(xycb,3e) { WM( EA,SRL( RM(EA) ) );								} /* SRL  (XY+o)      */
OP(xycb,3f) { _A = SRL( RM(EA) ); WM( EA,_A );						} /* SRL  A=(XY+o)    */

OP(xycb,40) { xycb_46();											} /* BIT  0,B=(XY+o)  */
OP(xycb,41) { xycb_46();													  } /* BIT  0,C=(XY+o)  */
OP(xycb,42) { xycb_46();											} /* BIT  0,D=(XY+o)  */
OP(xycb,43) { xycb_46();											} /* BIT  0,E=(XY+o)  */
OP(xycb,44) { xycb_46();											} /* BIT  0,H=(XY+o)  */
OP(xycb,45) { xycb_46();											} /* BIT  0,L=(XY+o)  */
OP(xycb,46) { BIT_XY(0,RM(EA)); 									} /* BIT  0,(XY+o)    */
OP(xycb,47) { xycb_46();											} /* BIT  0,A=(XY+o)  */

OP(xycb,48) { xycb_4e();											} /* BIT  1,B=(XY+o)  */
OP(xycb,49) { xycb_4e();													  } /* BIT  1,C=(XY+o)  */
OP(xycb,4a) { xycb_4e();											} /* BIT  1,D=(XY+o)  */
OP(xycb,4b) { xycb_4e();											} /* BIT  1,E=(XY+o)  */
OP(xycb,4c) { xycb_4e();											} /* BIT  1,H=(XY+o)  */
OP(xycb,4d) { xycb_4e();											} /* BIT  1,L=(XY+o)  */
OP(xycb,4e) { BIT_XY(1,RM(EA)); 									} /* BIT  1,(XY+o)    */
OP(xycb,4f) { xycb_4e();											} /* BIT  1,A=(XY+o)  */

OP(xycb,50) { xycb_56();											} /* BIT  2,B=(XY+o)  */
OP(xycb,51) { xycb_56();													  } /* BIT  2,C=(XY+o)  */
OP(xycb,52) { xycb_56();											} /* BIT  2,D=(XY+o)  */
OP(xycb,53) { xycb_56();											} /* BIT  2,E=(XY+o)  */
OP(xycb,54) { xycb_56();											} /* BIT  2,H=(XY+o)  */
OP(xycb,55) { xycb_56();											} /* BIT  2,L=(XY+o)  */
OP(xycb,56) { BIT_XY(2,RM(EA)); 									} /* BIT  2,(XY+o)    */
OP(xycb,57) { xycb_56();											} /* BIT  2,A=(XY+o)  */

OP(xycb,58) { xycb_5e();											} /* BIT  3,B=(XY+o)  */
OP(xycb,59) { xycb_5e();													  } /* BIT  3,C=(XY+o)  */
OP(xycb,5a) { xycb_5e();											} /* BIT  3,D=(XY+o)  */
OP(xycb,5b) { xycb_5e();											} /* BIT  3,E=(XY+o)  */
OP(xycb,5c) { xycb_5e();											} /* BIT  3,H=(XY+o)  */
OP(xycb,5d) { xycb_5e();											} /* BIT  3,L=(XY+o)  */
OP(xycb,5e) { BIT_XY(3,RM(EA)); 									} /* BIT  3,(XY+o)    */
OP(xycb,5f) { xycb_5e();											} /* BIT  3,A=(XY+o)  */

OP(xycb,60) { xycb_66();											} /* BIT  4,B=(XY+o)  */
OP(xycb,61) { xycb_66();													  } /* BIT  4,C=(XY+o)  */
OP(xycb,62) { xycb_66();											} /* BIT  4,D=(XY+o)  */
OP(xycb,63) { xycb_66();											} /* BIT  4,E=(XY+o)  */
OP(xycb,64) { xycb_66();											} /* BIT  4,H=(XY+o)  */
OP(xycb,65) { xycb_66();											} /* BIT  4,L=(XY+o)  */
OP(xycb,66) { BIT_XY(4,RM(EA)); 									} /* BIT  4,(XY+o)    */
OP(xycb,67) { xycb_66();											} /* BIT  4,A=(XY+o)  */

OP(xycb,68) { xycb_6e();											} /* BIT  5,B=(XY+o)  */
OP(xycb,69) { xycb_6e();													  } /* BIT  5,C=(XY+o)  */
OP(xycb,6a) { xycb_6e();											} /* BIT  5,D=(XY+o)  */
OP(xycb,6b) { xycb_6e();											} /* BIT  5,E=(XY+o)  */
OP(xycb,6c) { xycb_6e();											} /* BIT  5,H=(XY+o)  */
OP(xycb,6d) { xycb_6e();											} /* BIT  5,L=(XY+o)  */
OP(xycb,6e) { BIT_XY(5,RM(EA)); 									} /* BIT  5,(XY+o)    */
OP(xycb,6f) { xycb_6e();											} /* BIT  5,A=(XY+o)  */

OP(xycb,70) { xycb_76();											} /* BIT  6,B=(XY+o)  */
OP(xycb,71) { xycb_76();													  } /* BIT  6,C=(XY+o)  */
OP(xycb,72) { xycb_76();											} /* BIT  6,D=(XY+o)  */
OP(xycb,73) { xycb_76();											} /* BIT  6,E=(XY+o)  */
OP(xycb,74) { xycb_76();											} /* BIT  6,H=(XY+o)  */
OP(xycb,75) { xycb_76();											} /* BIT  6,L=(XY+o)  */
OP(xycb,76) { BIT_XY(6,RM(EA)); 									} /* BIT  6,(XY+o)    */
OP(xycb,77) { xycb_76();											} /* BIT  6,A=(XY+o)  */

OP(xycb,78) { xycb_7e();											} /* BIT  7,B=(XY+o)  */
OP(xycb,79) { xycb_7e();													  } /* BIT  7,C=(XY+o)  */
OP(xycb,7a) { xycb_7e();											} /* BIT  7,D=(XY+o)  */
OP(xycb,7b) { xycb_7e();											} /* BIT  7,E=(XY+o)  */
OP(xycb,7c) { xycb_7e();											} /* BIT  7,H=(XY+o)  */
OP(xycb,7d) { xycb_7e();											} /* BIT  7,L=(XY+o)  */
OP(xycb,7e) { BIT_XY(7,RM(EA)); 									} /* BIT  7,(XY+o)    */
OP(xycb,7f) { xycb_7e();											} /* BIT  7,A=(XY+o)  */

OP(xycb,80) { _B = RES(0, RM(EA) ); WM( EA,_B );					} /* RES  0,B=(XY+o)  */
OP(xycb,81) { _C = RES(0, RM(EA) ); WM( EA,_C );					} /* RES  0,C=(XY+o)  */
OP(xycb,82) { _D = RES(0, RM(EA) ); WM( EA,_D );					} /* RES  0,D=(XY+o)  */
OP(xycb,83) { _E = RES(0, RM(EA) ); WM( EA,_E );					} /* RES  0,E=(XY+o)  */
OP(xycb,84) { _H = RES(0, RM(EA) ); WM( EA,_H );					} /* RES  0,H=(XY+o)  */
OP(xycb,85) { _L = RES(0, RM(EA) ); WM( EA,_L );					} /* RES  0,L=(XY+o)  */
OP(xycb,86) { WM( EA, RES(0,RM(EA)) );								} /* RES  0,(XY+o)    */
OP(xycb,87) { _A = RES(0, RM(EA) ); WM( EA,_A );					} /* RES  0,A=(XY+o)  */

OP(xycb,88) { _B = RES(1, RM(EA) ); WM( EA,_B );					} /* RES  1,B=(XY+o)  */
OP(xycb,89) { _C = RES(1, RM(EA) ); WM( EA,_C );					} /* RES  1,C=(XY+o)  */
OP(xycb,8a) { _D = RES(1, RM(EA) ); WM( EA,_D );					} /* RES  1,D=(XY+o)  */
OP(xycb,8b) { _E = RES(1, RM(EA) ); WM( EA,_E );					} /* RES  1,E=(XY+o)  */
OP(xycb,8c) { _H = RES(1, RM(EA) ); WM( EA,_H );					} /* RES  1,H=(XY+o)  */
OP(xycb,8d) { _L = RES(1, RM(EA) ); WM( EA,_L );					} /* RES  1,L=(XY+o)  */
OP(xycb,8e) { WM( EA, RES(1,RM(EA)) );								} /* RES  1,(XY+o)    */
OP(xycb,8f) { _A = RES(1, RM(EA) ); WM( EA,_A );					} /* RES  1,A=(XY+o)  */

OP(xycb,90) { _B = RES(2, RM(EA) ); WM( EA,_B );					} /* RES  2,B=(XY+o)  */
OP(xycb,91) { _C = RES(2, RM(EA) ); WM( EA,_C );					} /* RES  2,C=(XY+o)  */
OP(xycb,92) { _D = RES(2, RM(EA) ); WM( EA,_D );					} /* RES  2,D=(XY+o)  */
OP(xycb,93) { _E = RES(2, RM(EA) ); WM( EA,_E );					} /* RES  2,E=(XY+o)  */
OP(xycb,94) { _H = RES(2, RM(EA) ); WM( EA,_H );					} /* RES  2,H=(XY+o)  */
OP(xycb,95) { _L = RES(2, RM(EA) ); WM( EA,_L );					} /* RES  2,L=(XY+o)  */
OP(xycb,96) { WM( EA, RES(2,RM(EA)) );								} /* RES  2,(XY+o)    */
OP(xycb,97) { _A = RES(2, RM(EA) ); WM( EA,_A );					} /* RES  2,A=(XY+o)  */

OP(xycb,98) { _B = RES(3, RM(EA) ); WM( EA,_B );					} /* RES  3,B=(XY+o)  */
OP(xycb,99) { _C = RES(3, RM(EA) ); WM( EA,_C );					} /* RES  3,C=(XY+o)  */
OP(xycb,9a) { _D = RES(3, RM(EA) ); WM( EA,_D );					} /* RES  3,D=(XY+o)  */
OP(xycb,9b) { _E = RES(3, RM(EA) ); WM( EA,_E );					} /* RES  3,E=(XY+o)  */
OP(xycb,9c) { _H = RES(3, RM(EA) ); WM( EA,_H );					} /* RES  3,H=(XY+o)  */
OP(xycb,9d) { _L = RES(3, RM(EA) ); WM( EA,_L );					} /* RES  3,L=(XY+o)  */
OP(xycb,9e) { WM( EA, RES(3,RM(EA)) );								} /* RES  3,(XY+o)    */
OP(xycb,9f) { _A = RES(3, RM(EA) ); WM( EA,_A );					} /* RES  3,A=(XY+o)  */

OP(xycb,a0) { _B = RES(4, RM(EA) ); WM( EA,_B );					} /* RES  4,B=(XY+o)  */
OP(xycb,a1) { _C = RES(4, RM(EA) ); WM( EA,_C );					} /* RES  4,C=(XY+o)  */
OP(xycb,a2) { _D = RES(4, RM(EA) ); WM( EA,_D );					} /* RES  4,D=(XY+o)  */
OP(xycb,a3) { _E = RES(4, RM(EA) ); WM( EA,_E );					} /* RES  4,E=(XY+o)  */
OP(xycb,a4) { _H = RES(4, RM(EA) ); WM( EA,_H );					} /* RES  4,H=(XY+o)  */
OP(xycb,a5) { _L = RES(4, RM(EA) ); WM( EA,_L );					} /* RES  4,L=(XY+o)  */
OP(xycb,a6) { WM( EA, RES(4,RM(EA)) );								} /* RES  4,(XY+o)    */
OP(xycb,a7) { _A = RES(4, RM(EA) ); WM( EA,_A );					} /* RES  4,A=(XY+o)  */

OP(xycb,a8) { _B = RES(5, RM(EA) ); WM( EA,_B );					} /* RES  5,B=(XY+o)  */
OP(xycb,a9) { _C = RES(5, RM(EA) ); WM( EA,_C );					} /* RES  5,C=(XY+o)  */
OP(xycb,aa) { _D = RES(5, RM(EA) ); WM( EA,_D );					} /* RES  5,D=(XY+o)  */
OP(xycb,ab) { _E = RES(5, RM(EA) ); WM( EA,_E );					} /* RES  5,E=(XY+o)  */
OP(xycb,ac) { _H = RES(5, RM(EA) ); WM( EA,_H );					} /* RES  5,H=(XY+o)  */
OP(xycb,ad) { _L = RES(5, RM(EA) ); WM( EA,_L );					} /* RES  5,L=(XY+o)  */
OP(xycb,ae) { WM( EA, RES(5,RM(EA)) );								} /* RES  5,(XY+o)    */
OP(xycb,af) { _A = RES(5, RM(EA) ); WM( EA,_A );					} /* RES  5,A=(XY+o)  */

OP(xycb,b0) { _B = RES(6, RM(EA) ); WM( EA,_B );					} /* RES  6,B=(XY+o)  */
OP(xycb,b1) { _C = RES(6, RM(EA) ); WM( EA,_C );					} /* RES  6,C=(XY+o)  */
OP(xycb,b2) { _D = RES(6, RM(EA) ); WM( EA,_D );					} /* RES  6,D=(XY+o)  */
OP(xycb,b3) { _E = RES(6, RM(EA) ); WM( EA,_E );					} /* RES  6,E=(XY+o)  */
OP(xycb,b4) { _H = RES(6, RM(EA) ); WM( EA,_H );					} /* RES  6,H=(XY+o)  */
OP(xycb,b5) { _L = RES(6, RM(EA) ); WM( EA,_L );					} /* RES  6,L=(XY+o)  */
OP(xycb,b6) { WM( EA, RES(6,RM(EA)) );								} /* RES  6,(XY+o)    */
OP(xycb,b7) { _A = RES(6, RM(EA) ); WM( EA,_A );					} /* RES  6,A=(XY+o)  */

OP(xycb,b8) { _B = RES(7, RM(EA) ); WM( EA,_B );					} /* RES  7,B=(XY+o)  */
OP(xycb,b9) { _C = RES(7, RM(EA) ); WM( EA,_C );					} /* RES  7,C=(XY+o)  */
OP(xycb,ba) { _D = RES(7, RM(EA) ); WM( EA,_D );					} /* RES  7,D=(XY+o)  */
OP(xycb,bb) { _E = RES(7, RM(EA) ); WM( EA,_E );					} /* RES  7,E=(XY+o)  */
OP(xycb,bc) { _H = RES(7, RM(EA) ); WM( EA,_H );					} /* RES  7,H=(XY+o)  */
OP(xycb,bd) { _L = RES(7, RM(EA) ); WM( EA,_L );					} /* RES  7,L=(XY+o)  */
OP(xycb,be) { WM( EA, RES(7,RM(EA)) );								} /* RES  7,(XY+o)    */
OP(xycb,bf) { _A = RES(7, RM(EA) ); WM( EA,_A );					} /* RES  7,A=(XY+o)  */

OP(xycb,c0) { _B = SET(0, RM(EA) ); WM( EA,_B );					} /* SET  0,B=(XY+o)  */
OP(xycb,c1) { _C = SET(0, RM(EA) ); WM( EA,_C );					} /* SET  0,C=(XY+o)  */
OP(xycb,c2) { _D = SET(0, RM(EA) ); WM( EA,_D );					} /* SET  0,D=(XY+o)  */
OP(xycb,c3) { _E = SET(0, RM(EA) ); WM( EA,_E );					} /* SET  0,E=(XY+o)  */
OP(xycb,c4) { _H = SET(0, RM(EA) ); WM( EA,_H );					} /* SET  0,H=(XY+o)  */
OP(xycb,c5) { _L = SET(0, RM(EA) ); WM( EA,_L );					} /* SET  0,L=(XY+o)  */
OP(xycb,c6) { WM( EA, SET(0,RM(EA)) );								} /* SET  0,(XY+o)    */
OP(xycb,c7) { _A = SET(0, RM(EA) ); WM( EA,_A );					} /* SET  0,A=(XY+o)  */

OP(xycb,c8) { _B = SET(1, RM(EA) ); WM( EA,_B );					} /* SET  1,B=(XY+o)  */
OP(xycb,c9) { _C = SET(1, RM(EA) ); WM( EA,_C );					} /* SET  1,C=(XY+o)  */
OP(xycb,ca) { _D = SET(1, RM(EA) ); WM( EA,_D );					} /* SET  1,D=(XY+o)  */
OP(xycb,cb) { _E = SET(1, RM(EA) ); WM( EA,_E );					} /* SET  1,E=(XY+o)  */
OP(xycb,cc) { _H = SET(1, RM(EA) ); WM( EA,_H );					} /* SET  1,H=(XY+o)  */
OP(xycb,cd) { _L = SET(1, RM(EA) ); WM( EA,_L );					} /* SET  1,L=(XY+o)  */
OP(xycb,ce) { WM( EA, SET(1,RM(EA)) );								} /* SET  1,(XY+o)    */
OP(xycb,cf) { _A = SET(1, RM(EA) ); WM( EA,_A );					} /* SET  1,A=(XY+o)  */

OP(xycb,d0) { _B = SET(2, RM(EA) ); WM( EA,_B );					} /* SET  2,B=(XY+o)  */
OP(xycb,d1) { _C = SET(2, RM(EA) ); WM( EA,_C );					} /* SET  2,C=(XY+o)  */
OP(xycb,d2) { _D = SET(2, RM(EA) ); WM( EA,_D );					} /* SET  2,D=(XY+o)  */
OP(xycb,d3) { _E = SET(2, RM(EA) ); WM( EA,_E );					} /* SET  2,E=(XY+o)  */
OP(xycb,d4) { _H = SET(2, RM(EA) ); WM( EA,_H );					} /* SET  2,H=(XY+o)  */
OP(xycb,d5) { _L = SET(2, RM(EA) ); WM( EA,_L );					} /* SET  2,L=(XY+o)  */
OP(xycb,d6) { WM( EA, SET(2,RM(EA)) );								} /* SET  2,(XY+o)    */
OP(xycb,d7) { _A = SET(2, RM(EA) ); WM( EA,_A );					} /* SET  2,A=(XY+o)  */

OP(xycb,d8) { _B = SET(3, RM(EA) ); WM( EA,_B );					} /* SET  3,B=(XY+o)  */
OP(xycb,d9) { _C = SET(3, RM(EA) ); WM( EA,_C );					} /* SET  3,C=(XY+o)  */
OP(xycb,da) { _D = SET(3, RM(EA) ); WM( EA,_D );					} /* SET  3,D=(XY+o)  */
OP(xycb,db) { _E = SET(3, RM(EA) ); WM( EA,_E );					} /* SET  3,E=(XY+o)  */
OP(xycb,dc) { _H = SET(3, RM(EA) ); WM( EA,_H );					} /* SET  3,H=(XY+o)  */
OP(xycb,dd) { _L = SET(3, RM(EA) ); WM( EA,_L );					} /* SET  3,L=(XY+o)  */
OP(xycb,de) { WM( EA, SET(3,RM(EA)) );								} /* SET  3,(XY+o)    */
OP(xycb,df) { _A = SET(3, RM(EA) ); WM( EA,_A );					} /* SET  3,A=(XY+o)  */

OP(xycb,e0) { _B = SET(4, RM(EA) ); WM( EA,_B );					} /* SET  4,B=(XY+o)  */
OP(xycb,e1) { _C = SET(4, RM(EA) ); WM( EA,_C );					} /* SET  4,C=(XY+o)  */
OP(xycb,e2) { _D = SET(4, RM(EA) ); WM( EA,_D );					} /* SET  4,D=(XY+o)  */
OP(xycb,e3) { _E = SET(4, RM(EA) ); WM( EA,_E );					} /* SET  4,E=(XY+o)  */
OP(xycb,e4) { _H = SET(4, RM(EA) ); WM( EA,_H );					} /* SET  4,H=(XY+o)  */
OP(xycb,e5) { _L = SET(4, RM(EA) ); WM( EA,_L );					} /* SET  4,L=(XY+o)  */
OP(xycb,e6) { WM( EA, SET(4,RM(EA)) );								} /* SET  4,(XY+o)    */
OP(xycb,e7) { _A = SET(4, RM(EA) ); WM( EA,_A );					} /* SET  4,A=(XY+o)  */

OP(xycb,e8) { _B = SET(5, RM(EA) ); WM( EA,_B );					} /* SET  5,B=(XY+o)  */
OP(xycb,e9) { _C = SET(5, RM(EA) ); WM( EA,_C );					} /* SET  5,C=(XY+o)  */
OP(xycb,ea) { _D = SET(5, RM(EA) ); WM( EA,_D );					} /* SET  5,D=(XY+o)  */
OP(xycb,eb) { _E = SET(5, RM(EA) ); WM( EA,_E );					} /* SET  5,E=(XY+o)  */
OP(xycb,ec) { _H = SET(5, RM(EA) ); WM( EA,_H );					} /* SET  5,H=(XY+o)  */
OP(xycb,ed) { _L = SET(5, RM(EA) ); WM( EA,_L );					} /* SET  5,L=(XY+o)  */
OP(xycb,ee) { WM( EA, SET(5,RM(EA)) );								} /* SET  5,(XY+o)    */
OP(xycb,ef) { _A = SET(5, RM(EA) ); WM( EA,_A );					} /* SET  5,A=(XY+o)  */

OP(xycb,f0) { _B = SET(6, RM(EA) ); WM( EA,_B );					} /* SET  6,B=(XY+o)  */
OP(xycb,f1) { _C = SET(6, RM(EA) ); WM( EA,_C );					} /* SET  6,C=(XY+o)  */
OP(xycb,f2) { _D = SET(6, RM(EA) ); WM( EA,_D );					} /* SET  6,D=(XY+o)  */
OP(xycb,f3) { _E = SET(6, RM(EA) ); WM( EA,_E );					} /* SET  6,E=(XY+o)  */
OP(xycb,f4) { _H = SET(6, RM(EA) ); WM( EA,_H );					} /* SET  6,H=(XY+o)  */
OP(xycb,f5) { _L = SET(6, RM(EA) ); WM( EA,_L );					} /* SET  6,L=(XY+o)  */
OP(xycb,f6) { WM( EA, SET(6,RM(EA)) );								} /* SET  6,(XY+o)    */
OP(xycb,f7) { _A = SET(6, RM(EA) ); WM( EA,_A );					} /* SET  6,A=(XY+o)  */

OP(xycb,f8) { _B = SET(7, RM(EA) ); WM( EA,_B );					} /* SET  7,B=(XY+o)  */
OP(xycb,f9) { _C = SET(7, RM(EA) ); WM( EA,_C );					} /* SET  7,C=(XY+o)  */
OP(xycb,fa) { _D = SET(7, RM(EA) ); WM( EA,_D );					} /* SET  7,D=(XY+o)  */
OP(xycb,fb) { _E = SET(7, RM(EA) ); WM( EA,_E );					} /* SET  7,E=(XY+o)  */
OP(xycb,fc) { _H = SET(7, RM(EA) ); WM( EA,_H );					} /* SET  7,H=(XY+o)  */
OP(xycb,fd) { _L = SET(7, RM(EA) ); WM( EA,_L );					} /* SET  7,L=(XY+o)  */
OP(xycb,fe) { WM( EA, SET(7,RM(EA)) );								} /* SET  7,(XY+o)    */
OP(xycb,ff) { _A = SET(7, RM(EA) ); WM( EA,_A );					} /* SET  7,A=(XY+o)  */

