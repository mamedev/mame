#define I 0
#define J 1
#define A 2
#define B 3
#define XL 4
#define XH 5
#define YL 6
#define YH 7
#define K 8
#define L 9
#define V 10 // some docus m
#define W 11 // some docus n
#define IA 92
#define IB 93
#define F0 94
#define C 95

enum {
	SC61860_PC=1, SC61860_DP,
	SC61860_P, SC61860_Q, SC61860_R,
	SC61860_CARRY,
	SC61860_ZERO,
	// the following are in the internal ram!
	SC61860_BA,
	SC61860_X, SC61860_Y,
	SC61860_I, SC61860_J, SC61860_K, SC61860_L, SC61860_V, SC61860_W,
	SC61860_H

//  SC61860_NMI_STATE,
//  SC61860_IRQ_STATE
};

#define PEEK_OP(pc) cpu_readop(pc)
#define PEEK_NIBBLE(adr) cpu_readmem_16(adr)
