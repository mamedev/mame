// license:BSD-3-Clause
// copyright-holders:R. Belmont, Karl Stenerud
/*

Mitsubishi 7700 Series CPU disassembler v1.1

By R. Belmont
Based on G65C816 CPU Emulator by Karl Stenerud

*/

#include "emu.h"
#include "m7700ds.h"

#ifdef SEC
#undef SEC
#endif

#define ADDRESS_24BIT(A) ((A)&0xffffff)

struct m7700_opcode_struct
{
	unsigned char name;
	unsigned char flag;
	unsigned char ea;
};

enum
{
	IMP , ACC , RELB, RELW, IMM , A   , AI  , AL  , ALX , AX  , AXI ,
	AY  , D   , DI  , DIY , DLI , DLIY, DX  , DXI , DY  , S   , SIY ,
	SIG /*, MVN , MVP , PEA , PEI , PER */, LDM4, LDM5, LDM4X, LDM5X,
	BBCD, BBCA, ACCB
};

enum
{
	I, /* ignore */
	M, /* check m bit */
	X  /* check x bit */
};

enum
{
	ADC ,  AND ,  ASL ,  BCC ,  BCS ,  BEQ ,  BIT ,  BMI ,  BNE ,  BPL ,  BRA ,
	BRK ,  BRL ,  BVC ,  BVS ,  CLC ,  CLD ,  CLI ,  CLV ,  CMP ,  COP ,  CPX ,
	CPY ,  DEA ,  DEC ,  DEX ,  DEY ,  EOR ,  INA ,  INC ,  INX ,  INY ,  JML ,
	JMP ,  JSL ,  JSR ,  LDA ,  LDX ,  LDY ,  LSR ,  MVN ,  MVP ,  NOP ,  ORA ,
	PEA ,  PEI ,  PER ,  PHA ,  PHT ,  PHD ,  PHK ,  PHP ,  PHX ,  PHY ,  PLA ,
	PLB ,  PLD ,  PLP ,  PLX ,  PLY ,  CLP ,  ROL ,  ROR ,  RTI ,  RTL ,  RTS ,
	SBC ,  SEC ,  SED ,  SEI ,  SEP ,  STA ,  STP ,  STX ,  STY ,  STZ ,  TAX ,
	TAY ,  TCS ,  TCD ,  TDC ,  TRB ,  TSB ,  TSC ,  TSX ,  TXA ,  TXS ,  TXY ,
	TYA ,  TYX ,  WAI ,  WDM ,  XBA ,  XCE ,  MPY ,  DIV ,  MPYS,  DIVS,  RLA ,
	EXTS, EXTZ ,  LDT ,  LDM ,  UNK ,  SEB ,  SEM ,  CLM ,  STB ,  LDB ,  ADCB ,
	SBCB, EORB ,  TBX ,  CMPB,  INB ,  DEB ,  TXB ,  TYB ,  LSRB,  ORB ,  CLB ,
	BBC,   BBS,   TBY,   ANDB,  PUL ,  PSH ,  PLAB,  XAB ,  PHB
};

static const char *const g_opnames[] =
{
	"ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRA",
	"BRK", "BRL", "BVC", "BVS", "CLC", "CLD", "CLI", "CLV", "CMP", "COP", "CPX",
	"CPY", "DEA", "DEC", "DEX", "DEY", "EOR", "INA", "INC", "INX", "INY", "JML",
	"JMP", "JSL", "JSR", "LDA", "LDX", "LDY", "LSR", "MVN", "MVP", "NOP", "ORA",
	"PEA", "PEI", "PER", "PHA", "PHT", "PHD", "PHK", "PHP", "PHX", "PHY", "PLA",
	"PLT", "PLD", "PLP", "PLX", "PLY", "CLP", "ROL", "ROR", "RTI", "RTL", "RTS",
	"SBC", "SEC", "SED", "SEI", "SEP", "STA", "STP", "STX", "STY", "STZ", "TAX",
	"TAY", "TCS", "TCD", "TDC", "TRB", "TSB", "TSC", "TSX", "TXA", "TXS", "TXY",
	"TYA", "TYX", "WAI", "WDM", "XBA", "XCE", "MPY", "DIV", "MPYS", "DIVS", "RLA",
	"EXTS","EXTZ","LDT", "LDM", "UNK", "SEB", "SEM", "CLM", "STB", "LDB", "ADCB",
	"SBCB","EORB","TBX", "CMPB","INB", "DEB", "TXB", "TYB", "LSRB", "ORB", "CLB",
	"BBC", "BBS", "TBY", "ANDB","PUL", "PSH", "PLB", "XAB", "PHB",
};

static const m7700_opcode_struct g_opcodes[256] =
{
	{BRK, I, SIG }, {ORA, M, DXI }, {UNK, I, SIG }, {ORA, M, S   },
	{SEB, M, LDM4 }, {ORA, M, D   }, {ASL, M, D   }, {ORA, M, DLI },
	{PHP, I, IMP }, {ORA, M, IMM }, {ASL, M, ACC }, {PHD, I, IMP },
	{SEB, M, LDM5 }, {ORA, M, A   }, {ASL, M, A   }, {ORA, M, AL  },
// 0x10
	{BPL, I, RELB}, {ORA, M, DIY }, {ORA, M, DI  }, {ORA, M, SIY },
	{CLB, M, LDM4}, {ORA, M, DX  }, {ASL, M, DX  }, {ORA, M, DLIY},
	{CLC, I, IMP }, {ORA, M, AY  }, {DEA, I, IMP }, {TCS, I, IMP },
	{CLB, M, LDM5}, {ORA, M, AX  }, {ASL, M, AX  }, {ORA, M, ALX },
// 0x20
	{JSR, I, A   }, {AND, M, DXI }, {JSL, I, AL  }, {AND, M, S   },
	{BBS, M, BBCD}, {AND, M, D   }, {ROL, M, D   }, {AND, M, DLI },
	{PLP, I, IMP }, {AND, M, IMM }, {ROL, M, ACC }, {PLD, I, IMP },
	{BBS, M, BBCA}, {AND, M, A   }, {ROL, M, A   }, {AND, M, AL  },
// 0x30
	{BMI, I, RELB}, {AND, M, DIY }, {AND, M, DI  }, {AND, M, SIY },
	{BBC, M, BBCD}, {AND, M, DX  }, {ROL, M, DX  }, {AND, M, DLIY},
	{SEC, I, IMP }, {AND, M, AY  }, {INA, I, IMP }, {TSC, I, IMP },
	{BBC, M, BBCA}, {AND, M, AX  }, {ROL, M, AX  }, {AND, M, ALX },
// 0x40
	{RTI, I, IMP }, {EOR, M, DXI }, {WDM, I, IMP }, {EOR, M, S   },
	{MVP, I, MVP }, {EOR, M, D   }, {LSR, M, D   }, {EOR, M, DLI },
	{PHA, I, IMP }, {EOR, M, IMM }, {LSR, M, ACC }, {PHK, I, IMP },
	{JMP, I, A   }, {EOR, M, A   }, {LSR, M, A   }, {EOR, M, AL  },
// 0x50
	{BVC, I, RELB}, {EOR, M, DIY }, {EOR, M, DI  }, {EOR, M, SIY },
	{MVN, I, MVN }, {EOR, M, DX  }, {LSR, M, DX  }, {EOR, M, DLIY},
	{CLI, I, IMP }, {EOR, M, AY  }, {PHY, I, IMP }, {TCD, I, IMP },
	{JMP, I, AL  }, {EOR, M, AX  }, {LSR, M, AX  }, {EOR, M, ALX },
// 0x60
	{RTS, I, IMP }, {ADC, M, DXI }, {PER, I, PER }, {ADC, M, S   },
	{LDM, M, LDM4 }, {ADC, M, D   }, {ROR, M, D   }, {ADC, M, DLI },
	{PLA, I, IMP }, {ADC, M, IMM }, {ROR, M, ACC }, {RTL, I, IMP },
	{JMP, I, AI  }, {ADC, M, A   }, {ROR, M, A   }, {ADC, M, AL  },
// 0x70
	{BVS, I, RELB}, {ADC, M, DIY }, {ADC, M, DI  }, {ADC, M, SIY },
	{LDM, M, LDM4X }, {ADC, M, DX  }, {ROR, M, DX  }, {ADC, M, DLIY},
	{SEI, I, IMP }, {ADC, M, AY  }, {PLY, I, IMP }, {TDC, I, IMP },
	{JMP, I, AXI }, {ADC, M, AX  }, {ROR, M, AX  }, {ADC, M, ALX },
// 0x80
	{BRA, I, RELB}, {STA, M, DXI }, {BRL, I, RELW}, {STA, M, S   },
	{STY, X, D   }, {STA, M, D   }, {STX, X, D   }, {STA, M, DLI },
	{DEY, I, IMP }, {BIT, M, IMM }, {TXA, I, IMP }, {PHT, I, IMP },
	{STY, X, A   }, {STA, M, A   }, {STX, X, A   }, {STA, M, AL  },
// 0x90
	{BCC, I, RELB}, {STA, M, DIY }, {STA, M, DI  }, {STA, M, SIY },
	{STY, X, DX  }, {STA, M, DX  }, {STX, X, DY  }, {STA, M, DLIY},
	{TYA, I, IMP }, {STA, M, AY  }, {TXS, I, IMP }, {TXY, I, IMP },
	{LDM, M, LDM5 }, {STA, M, AX  }, {LDM, M, LDM5X }, {STA, M, ALX },
// 0xA0
	{LDY, X, IMM }, {LDA, M, DXI }, {LDX, X, IMM }, {LDA, M, S   },
	{LDY, X, D   }, {LDA, M, D   }, {LDX, X, D   }, {LDA, M, DLI },
	{TAY, I, IMP }, {LDA, M, IMM }, {TAX, I, IMP }, {PLB, I, IMP },
	{LDY, X, A   }, {LDA, M, A   }, {LDX, X, A   }, {LDA, M, AL  },
// 0xB0
	{BCS, I, RELB}, {LDA, M, DIY }, {LDA, M, DI  }, {LDA, M, SIY },
	{LDY, X, DX  }, {LDA, M, DX  }, {LDX, X, DY  }, {LDA, M, DLIY},
	{CLV, I, IMP }, {LDA, M, AY  }, {TSX, I, IMP }, {TYX, I, IMP },
	{LDY, X, AX  }, {LDA, M, AX  }, {LDX, X, AY  }, {LDA, M, ALX },
// 0xC0
	{CPY, X, IMM }, {CMP, M, DXI }, {CLP, I, IMM }, {CMP, M, S   },
	{CPY, X, D   }, {CMP, M, D   }, {DEC, M, D   }, {CMP, M, DLI },
	{INY, I, IMP }, {CMP, M, IMM }, {DEX, I, IMP }, {WAI, I, IMP },
	{CPY, X, A   }, {CMP, M, A   }, {DEC, M, A   }, {CMP, M, AL  },
// 0xD0
	{BNE, I, RELB}, {CMP, M, DIY }, {CMP, M, DI  }, {CMP, M, SIY },
	{PEI, I, PEI }, {CMP, M, DX  }, {DEC, M, DX  }, {CMP, M, DLIY},
	{CLM, I, IMP }, {CMP, M, AY  }, {PHX, I, IMP }, {STP, I, IMP },
	{JML, I, AI  }, {CMP, M, AX  }, {DEC, M, AX  }, {CMP, M, ALX },
// 0xE0
	{CPX, X, IMM }, {SBC, M, DXI }, {SEP, I, IMM }, {SBC, M, S   },
	{CPX, X, D   }, {SBC, M, D   }, {INC, M, D   }, {SBC, M, DLI },
	{INX, M, IMP }, {SBC, M, IMM }, {NOP, I, IMP }, {PSH, I, IMM },
	{CPX, X, A   }, {SBC, M, A   }, {INC, M, A   }, {SBC, M, AL  },
// 0xF0
	{BEQ, I, RELB}, {SBC, M, DIY }, {SBC, M, DI  }, {SBC, M, SIY },
	{PEA, I, PEA }, {SBC, M, DX  }, {INC, M, DX  }, {SBC, M, DLIY},
	{SEM, I, IMP }, {SBC, M, AY  }, {PLX, I, IMP }, {PUL, I, IMM },
	{JSR, I, AXI }, {SBC, M, AX  }, {INC, M, AX  }, {SBC, M, ALX }
};

static const m7700_opcode_struct g_opcodes_prefix42[256] =
{
	{BRK, I, SIG }, {ORB, M, DXI }, {COP, I, SIG }, {ORB, M, S   },
	{TSB, M, D   }, {ORB, M, D   }, {ASL, M, D   }, {ORB, M, DLI },
	{PHP, I, IMP }, {ORB, M, IMM }, {ASL, M, ACCB }, {PHD, I, IMP },
	{TSB, M, A   }, {ORB, M, A   }, {ASL, M, A   }, {ORB, M, AL  },
// 0x10
	{BPL, I, RELB}, {ORB, M, DIY }, {ORB, M, DI  }, {ORB, M, SIY },
	{TRB, M, D   }, {ORB, M, DX  }, {ASL, M, DX  }, {ORB, M, DLIY},
	{CLC, I, IMP }, {ORB, M, AY  }, {DEB, I, IMP }, {TCS, I, IMP },
	{TRB, M, A   }, {ORB, M, AX  }, {ASL, M, AX  }, {ORB, M, ALX },
// 0x20
	{JSR, I, A   }, {ANDB, M, DXI }, {JSL, I, AL  }, {ANDB, M, S   },
	{BIT, M, D   }, {ANDB, M, D   }, {ROL, M, D   }, {ANDB, M, DLI },
	{PLP, I, IMP }, {ANDB, M, IMM }, {ROL, M, ACCB }, {PLD, I, IMP },
	{BIT, M, A   }, {ANDB, M, A   }, {ROL, M, A   }, {ANDB, M, AL  },
// 0x30
	{BMI, I, RELB}, {AND, M, DIY }, {AND, M, DI  }, {AND, M, SIY },
	{BIT, M, DX  }, {AND, M, DX  }, {ROL, M, DX  }, {AND, M, DLIY},
	{SEC, I, IMP }, {AND, M, AY  }, {INB, I, IMP }, {TSC, I, IMP },
	{BIT, M, AX  }, {AND, M, AX  }, {ROL, M, AX  }, {AND, M, ALX },
// 0x40
	{RTI, I, IMP }, {EORB, M, DXI }, {WDM, I, IMP }, {EORB, M, S   },
	{MVP, I, MVP }, {EORB, M, D   }, {LSRB, M, D   }, {EORB, M, DLI },
	{PHB, I, IMP }, {EORB, M, IMM }, {LSRB, M, ACC }, {PHK, I, IMP },
	{JMP, I, A   }, {EORB, M, A   }, {LSRB, M, A   }, {EORB, M, AL  },
// 0x50
	{BVC, I, RELB}, {EORB, M, DIY }, {EORB, M, DI  }, {EORB, M, SIY },
	{MVN, I, MVN }, {EORB, M, DX  }, {LSRB, M, DX  }, {EORB, M, DLIY},
	{CLI, I, IMP }, {EORB, M, AY  }, {PHY, I, IMP }, {TCD, I, IMP },
	{JMP, I, AL  }, {EORB, M, AX  }, {LSRB, M, AX  }, {EORB, M, ALX },
// 0x60
	{RTS, I, IMP }, {ADCB, M, DXI }, {PER, I, PER }, {ADCB, M, S   },
	{STZ, M, D   }, {ADCB, M, D   }, {ROR, M, D   }, {ADCB, M, DLI },
	{PLAB,I, IMP }, {ADCB, M, IMM }, {ROR, M, ACC }, {RTL, I, IMP },
	{JMP, I, AI  }, {ADCB, M, A   }, {ROR, M, A   }, {ADCB, M, AL  },
// 0x70
	{BVS, I, RELB}, {ADCB, M, DIY }, {ADCB, M, DI  }, {ADCB, M, SIY },
	{STZ, M, DX  }, {ADCB, M, DX  }, {ROR, M, DX  }, {ADCB, M, DLIY},
	{SEI, I, IMP }, {ADCB, M, AY  }, {PLY, I, IMP }, {TDC, I, IMP },
	{JMP, I, AXI }, {ADCB, M, AX  }, {ROR, M, AX  }, {ADCB, M, ALX },
// 0x80
	{BRA, I, RELB}, {STB, M, DXI }, {BRL, I, RELW}, {STB, M, S   },
	{STY, X, D   }, {STB, M, D   }, {STX, X, D   }, {STB, M, DLI },
	{DEY, I, IMP }, {BIT, M, IMM }, {TXB, I, IMP }, {PHB, I, IMP },
	{STY, X, A   }, {STB, M, A   }, {STX, X, A   }, {STB, M, AL  },
// 0x90
	{BCC, I, RELB}, {STB, M, DIY }, {STB, M, DI  }, {STB, M, SIY },
	{STY, X, DX  }, {STB, M, DX  }, {STX, X, DY  }, {STB, M, DLIY},
	{TYB, I, IMP }, {STB, M, AY  }, {TXS, I, IMP }, {TXY, I, IMP },
	{STZ, M, A   }, {STB, M, AX  }, {STZ, M, AX  }, {STB, M, ALX },
// 0xA0
	{LDY, X, IMM }, {LDB, M, DXI }, {LDX, X, IMM }, {LDB, M, S   },
	{LDY, X, D   }, {LDB, M, D   }, {LDX, X, D   }, {LDB, M, DLI },
	{TBY, I, IMP }, {LDB, M, IMM }, {TBX, I, IMP }, {PLB, I, IMP },
	{LDY, X, A   }, {LDB, M, A   }, {LDX, X, A   }, {LDB, M, AL  },
// 0xB0
	{BCS, I, RELB}, {LDB, M, DIY }, {LDB, M, DI  }, {LDB, M, SIY },
	{LDY, X, DX  }, {LDB, M, DX  }, {LDX, X, DY  }, {LDB, M, DLIY},
	{CLV, I, IMP }, {LDB, M, AY  }, {TSX, I, IMP }, {TYX, I, IMP },
	{LDY, X, AX  }, {LDB, M, AX  }, {LDX, X, AY  }, {LDB, M, ALX },
// 0xC0
	{CPY, X, IMM }, {CMPB, M, DXI }, {CLP, I, IMM }, {CMPB, M, S   },
	{CPY, X, D   }, {CMPB, M, D   }, {DEC, M, D   }, {CMPB, M, DLI },
	{INY, I, IMP }, {CMPB, M, IMM }, {DEX, I, IMP }, {WAI, I, IMP },
	{CPY, X, A   }, {CMPB, M, A   }, {DEC, M, A   }, {CMPB, M, AL  },
// 0xD0
	{BNE, I, RELB}, {CMPB, M, DIY }, {CMPB, M, DI  }, {CMPB, M, SIY },
	{PEI, I, PEI }, {CMPB, M, DX  }, {DEC, M, DX  }, {CMPB, M, DLIY},
	{CLD, I, IMP }, {CMPB, M, AY  }, {PHX, I, IMP }, {STP, I, IMP },
	{JML, I, AI  }, {CMPB, M, AX  }, {DEC, M, AX  }, {CMPB, M, ALX },
// 0xE0
	{CPX, X, IMM }, {SBCB, M, DXI }, {SEP, I, IMM }, {SBCB, M, S   },
	{CPX, X, D   }, {SBCB, M, D   }, {INC, M, D   }, {SBCB, M, DLI },
	{INX, M, IMP }, {SBCB, M, IMM }, {NOP, I, IMP }, {XBA, I, IMP },
	{CPX, X, A   }, {SBCB, M, A   }, {INC, M, A   }, {SBCB, M, AL  },
// 0xF0
	{BEQ, I, RELB}, {SBCB, M, DIY }, {SBCB, M, DI  }, {SBCB, M, SIY },
	{PEA, I, PEA }, {SBCB, M, DX  }, {INC, M, DX  }, {SBCB, M, DLIY},
	{SED, I, IMP }, {SBCB, M, AY  }, {PLX, I, IMP }, {XCE, I, IMP },
	{JSR, I, AXI }, {SBCB, M, AX  }, {INC, M, AX  }, {SBCB, M, ALX }
};

static const m7700_opcode_struct g_opcodes_prefix89[256] =
{
	{BRK, I, SIG }, {MPY, M, DXI }, {COP, I, SIG }, {MPY, M, S   },
	{TSB, M, D   }, {MPY, M, D   }, {ASL, M, D   }, {MPY, M, DLI },
	{PHP, I, IMP }, {MPY, M, IMM }, {ASL, M, ACC }, {PHD, I, IMP },
	{TSB, M, A   }, {MPY, M, A   }, {ASL, M, A   }, {MPY, M, AL  },
// 0x10
	{BPL, I, RELB}, {ORA, M, DIY }, {ORA, M, DI  }, {ORA, M, SIY },
	{TRB, M, D   }, {MPY, M, DX  }, {ASL, M, DX  }, {ORA, M, DLIY},
	{CLC, I, IMP }, {MPY, M, AY  }, {INA, I, IMP }, {TCS, I, IMP },
	{TRB, M, A   }, {ORA, M, AX  }, {ASL, M, AX  }, {ORA, M, ALX },
// 0x20
	{JSR, I, A   }, {AND, M, DXI }, {JSL, I, AL  }, {AND, M, S   },
	{BIT, M, D   }, {AND, M, D   }, {ROL, M, D   }, {AND, M, DLI },
	{XAB, I, IMP }, {AND, M, IMM }, {ROL, M, ACC }, {PLD, I, IMP },
	{BIT, M, A   }, {AND, M, A   }, {ROL, M, A   }, {AND, M, AL  },
// 0x30
	{BMI, I, RELB}, {AND, M, DIY }, {AND, M, DI  }, {AND, M, SIY },
	{BIT, M, DX  }, {AND, M, DX  }, {ROL, M, DX  }, {AND, M, DLIY},
	{SEC, I, IMP }, {AND, M, AY  }, {DEA, I, IMP }, {TSC, I, IMP },
	{BIT, M, AX  }, {AND, M, AX  }, {ROL, M, AX  }, {AND, M, ALX },
// 0x40
	{RTI, I, IMP }, {EOR, M, DXI }, {WDM, I, IMP }, {EOR, M, S   },
	{MVP, I, MVP }, {EOR, M, D   }, {LSR, M, D   }, {EOR, M, DLI },
	{PHA, I, IMP }, {RLA, M, IMM }, {LSR, M, ACC }, {PHK, I, IMP },
	{JMP, I, A   }, {EOR, M, A   }, {LSR, M, A   }, {EOR, M, AL  },
// 0x50
	{BVC, I, RELB}, {EOR, M, DIY }, {EOR, M, DI  }, {EOR, M, SIY },
	{MVN, I, MVN }, {EOR, M, DX  }, {LSR, M, DX  }, {EOR, M, DLIY},
	{CLI, I, IMP }, {EOR, M, AY  }, {PHY, I, IMP }, {TCD, I, IMP },
	{JMP, I, AL  }, {EOR, M, AX  }, {LSR, M, AX  }, {EOR, M, ALX },
// 0x60
	{RTS, I, IMP }, {ADC, M, DXI }, {PER, I, PER }, {ADC, M, S   },
	{STZ, M, D   }, {ADC, M, D   }, {ROR, M, D   }, {ADC, M, DLI },
	{PLA, I, IMP }, {ADC, M, IMM }, {ROR, M, ACC }, {RTL, I, IMP },
	{JMP, I, AI  }, {ADC, M, A   }, {ROR, M, A   }, {ADC, M, AL  },
// 0x70
	{BVS, I, RELB}, {ADC, M, DIY }, {ADC, M, DI  }, {ADC, M, SIY },
	{STZ, M, DX  }, {ADC, M, DX  }, {ROR, M, DX  }, {ADC, M, DLIY},
	{SEI, I, IMP }, {ADC, M, AY  }, {PLY, I, IMP }, {TDC, I, IMP },
	{JMP, I, AXI }, {ADC, M, AX  }, {ROR, M, AX  }, {ADC, M, ALX },
// 0x80
	{BRA, I, RELB}, {STA, M, DXI }, {BRL, I, RELW}, {STA, M, S   },
	{STY, X, D   }, {STA, M, D   }, {STX, X, D   }, {STA, M, DLI },
	{DEY, I, IMP }, {BIT, M, IMM }, {TXA, I, IMP }, {PHB, I, IMP },
	{STY, X, A   }, {STA, M, A   }, {STX, X, A   }, {STA, M, AL  },
// 0x90
	{BCC, I, RELB}, {STA, M, DIY }, {STA, M, DI  }, {STA, M, SIY },
	{STY, X, DX  }, {STA, M, DX  }, {STX, X, DY  }, {STA, M, DLIY},
	{TYA, I, IMP }, {STA, M, AY  }, {TXS, I, IMP }, {TXY, I, IMP },
	{STZ, M, A   }, {STA, M, AX  }, {STZ, M, AX  }, {STA, M, ALX },
// 0xA0
	{LDY, X, IMM }, {LDA, M, DXI }, {LDX, X, IMM }, {LDA, M, S   },
	{LDY, X, D   }, {LDA, M, D   }, {LDX, X, D   }, {LDA, M, DLI },
	{TAY, I, IMP }, {LDA, M, IMM }, {TAX, I, IMP }, {PLB, I, IMP },
	{LDY, X, A   }, {LDA, M, A   }, {LDX, X, A   }, {LDA, M, AL  },
// 0xB0
	{BCS, I, RELB}, {LDA, M, DIY }, {LDA, M, DI  }, {LDA, M, SIY },
	{LDY, X, DX  }, {LDA, M, DX  }, {LDX, X, DY  }, {LDA, M, DLIY},
	{CLV, I, IMP }, {LDA, M, AY  }, {TSX, I, IMP }, {TYX, I, IMP },
	{LDY, X, AX  }, {LDA, M, AX  }, {LDX, X, AY  }, {LDA, M, ALX },
// 0xC0
	{CPY, X, IMM }, {CMP, M, DXI }, {LDT, I, IMM }, {CMP, M, S   },
	{CPY, X, D   }, {CMP, M, D   }, {DEC, M, D   }, {CMP, M, DLI },
	{INY, I, IMP }, {CMP, M, IMM }, {DEX, I, IMP }, {WAI, I, IMP },
	{CPY, X, A   }, {CMP, M, A   }, {DEC, M, A   }, {CMP, M, AL  },
// 0xD0
	{BNE, I, RELB}, {CMP, M, DIY }, {CMP, M, DI  }, {CMP, M, SIY },
	{PEI, I, PEI }, {CMP, M, DX  }, {DEC, M, DX  }, {CMP, M, DLIY},
	{CLD, I, IMP }, {CMP, M, AY  }, {PHX, I, IMP }, {STP, I, IMP },
	{JML, I, AI  }, {CMP, M, AX  }, {DEC, M, AX  }, {CMP, M, ALX },
// 0xE0
	{CPX, X, IMM }, {SBC, M, DXI }, {SEP, I, IMM }, {SBC, M, S   },
	{CPX, X, D   }, {SBC, M, D   }, {INC, M, D   }, {SBC, M, DLI },
	{INX, M, IMP }, {SBC, M, IMM }, {NOP, I, IMP }, {XBA, I, IMP },
	{CPX, X, A   }, {SBC, M, A   }, {INC, M, A   }, {SBC, M, AL  },
// 0xF0
	{BEQ, I, RELB}, {SBC, M, DIY }, {SBC, M, DI  }, {SBC, M, SIY },
	{PEA, I, PEA }, {SBC, M, DX  }, {INC, M, DX  }, {SBC, M, DLIY},
	{SEM, I, IMP }, {SBC, M, AY  }, {PLX, I, IMP }, {XCE, I, IMP },
	{JSR, I, AXI }, {SBC, M, AX  }, {INC, M, AX  }, {SBC, M, ALX }
};

INLINE unsigned int read_8(const UINT8 *oprom, unsigned int offset)
{
	return oprom[offset];
}

INLINE unsigned int read_16(const UINT8 *oprom, unsigned int offset)
{
	unsigned int val = read_8(oprom, offset);
	return val | (read_8(oprom, offset+1)<<8);
}

INLINE unsigned int read_24(const UINT8 *oprom, unsigned int offset)
{
	unsigned int val = read_8(oprom, offset);
	val |= (read_8(oprom, offset+1)<<8);
	return val | (read_8(oprom, offset+2)<<16);
}

INLINE char* int_8_str(unsigned int val)
{
	static char str[20];

	val &= 0xff;

	if(val & 0x80)
		sprintf(str, "-$%x", (0-val) & 0x7f);
	else
		sprintf(str, "$%x", val & 0x7f);

	return str;
}

INLINE char* int_16_str(unsigned int val)
{
	static char str[20];

	val &= 0xffff;

	if(val & 0x8000)
		sprintf(str, "-$%x", (0-val) & 0x7fff);
	else
		sprintf(str, "$%x", val & 0x7fff);

	return str;
}


int m7700_disassemble(char* buff, unsigned int pc, unsigned int pb, const UINT8 *oprom, int m_flag, int x_flag)
{
	unsigned int instruction;
	const m7700_opcode_struct *opcode;
	char* ptr;
	int var;
	signed char varS;
	int length = 1;
	unsigned int address;
	//unsigned int start;
	UINT32 flags = 0;

	pb <<= 16;
	address = pc | pb;
	//start = address;

	instruction = read_8(oprom,0);

	// check for prefixes
	switch (instruction)
	{
		case 0x42:
			address++;
			length++;
			oprom++;
			instruction = read_8(oprom,0);
			opcode = g_opcodes_prefix42 + instruction;
			break;

		case 0x89:
			address++;
			length++;
			oprom++;
			instruction = read_8(oprom,0);
			opcode = g_opcodes_prefix89 + instruction;
			break;

		default:
			opcode = g_opcodes + instruction;
			break;
	}

	if (opcode->name == JSR)
		flags = DASMFLAG_STEP_OVER;
	else if (opcode->name == RTS || opcode->name == RTI)
		flags = DASMFLAG_STEP_OUT;

	sprintf(buff, "%s", g_opnames[opcode->name]);
	ptr = buff + strlen(buff);

	switch(opcode->ea)
	{
		case IMP :
			break;
		case ACC :
			sprintf(ptr, " A");
			break;
		case ACCB :
			sprintf(ptr, " B");
			break;
		case RELB:
			varS = read_8(oprom,1);
			length++;
			sprintf(ptr, " %06x (%s)", pb | ((pc + length + varS)&0xffff), int_8_str(varS));
			break;
		case RELW:
		case PER :
			var = read_16(oprom,1);
			length += 2;
			sprintf(ptr, " %06x (%s)", pb | ((pc + length + var)&0xffff), int_16_str(var));
			break;
		case IMM :
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				sprintf(ptr, " #$%04x", read_16(oprom,1));
				length += 2;
			}
			else
			{
				sprintf(ptr, " #$%02x", read_8(oprom,1));
				length++;
			}
			break;
		case BBCD:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				varS = read_8(oprom,4);
				length += 4;
				sprintf(ptr, " #$%04x, $%02x, %06x (%s)", read_16(oprom,2), read_8(oprom,1), pb | ((pc + length + varS)&0xffff), int_8_str(varS));
			}
			else
			{
				varS = read_8(oprom,3);
				length += 3;
				sprintf(ptr, " #$%02x, $%02x, %06x (%s)", read_8(oprom,2), read_8(oprom,1), pb | ((pc + length + varS)&0xffff), int_8_str(varS));
			}
			break;
		case BBCA:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				length += 5;
				varS = read_8(oprom,5);
				sprintf(ptr, " #$%04x, $%04x, %06x (%s)", read_16(oprom,3), read_16(oprom,1), pb | ((pc + length + varS)&0xffff), int_8_str(varS));
			}
			else
			{
				length += 4;
				varS = read_8(oprom,4);
				sprintf(ptr, " #$%02x, $%04x, %06x (%s)", read_8(oprom,3), read_16(oprom,1), pb | ((pc + length + varS)&0xffff), int_8_str(varS));
			}
			break;
		case LDM4:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				sprintf(ptr, " #$%04x, $%02x", read_16(oprom,2), read_8(oprom,1));
				length += 3;
			}
			else
			{
				sprintf(ptr, " #$%02x, $%02x", read_8(oprom,2), read_8(oprom,1));
				length += 2;
			}
			break;
		case LDM5:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				sprintf(ptr, " #$%04x, $%04x", read_16(oprom,3), read_16(oprom,1));
				length += 4;
			}
			else
			{
				sprintf(ptr, " #$%02x, $%04x", read_8(oprom,3), read_16(oprom,1));
				length += 3;
			}
			break;
		case LDM4X:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				sprintf(ptr, " #$%04x, $%02x, X", read_16(oprom,2), read_8(oprom,1));
				length += 3;
			}
			else
			{
				sprintf(ptr, " #$%02x, $%02x, X", read_8(oprom,2), read_8(oprom,1));
				length += 2;
			}
			break;
		case LDM5X:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				sprintf(ptr, " #$%04x, $%04x, X", read_16(oprom,3), read_16(oprom,1));
				length += 4;
			}
			else
			{
				sprintf(ptr, " #$%02x, $%04x, X", read_8(oprom,3), read_16(oprom,1));
				length += 3;
			}
			break;
		case A   :
		case PEA :
			sprintf(ptr, " $%04x", read_16(oprom,1));
			length += 2;
			break;
		case AI  :
			sprintf(ptr, " ($%04x)", read_16(oprom,1));
			length += 2;
			break;
		case AL  :
			sprintf(ptr, " $%06x", read_24(oprom,1));
			length += 3;
			break;
		case ALX :
			sprintf(ptr, " $%06x,X", read_24(oprom,1));
			length += 3;
			break;
		case AX  :
			sprintf(ptr, " $%04x,X", read_16(oprom,1));
			length += 2;
			break;
		case AXI :
			sprintf(ptr, " ($%04x,X)", read_16(oprom,1));
			length += 2;
			break;
		case AY  :
			sprintf(ptr, " $%04x,Y", read_16(oprom,1));
			length += 2;
			break;
		case D   :
			sprintf(ptr, " $%02x", read_8(oprom,1));
			length++;
			break;
		case DI  :
		case PEI :
			sprintf(ptr, " ($%02x)", read_8(oprom,1));
			length++;
			break;
		case DIY :
			sprintf(ptr, " ($%02x),Y", read_8(oprom,1));
			length++;
			break;
		case DLI :
			sprintf(ptr, " [$%02x]", read_8(oprom,1));
			length++;
			break;
		case DLIY:
			sprintf(ptr, " [$%02x],Y", read_8(oprom,1));
			length++;
			break;
		case DX  :
			sprintf(ptr, " $%02x,X", read_8(oprom,1));
			length++;
			break;
		case DXI :
			sprintf(ptr, " ($%02x,X)", read_8(oprom,1));
			length++;
			break;
		case DY  :
			sprintf(ptr, " $%02x,Y", read_8(oprom,1));
			length++;
			break;
		case S   :
			sprintf(ptr, " %s,S", int_8_str(read_8(oprom,1)));
			length++;
			break;
		case SIY :
			sprintf(ptr, " (%s,S),Y", int_8_str(read_8(oprom,1)));
			length++;
			break;
		case SIG :
			sprintf(ptr, " #$%02x", read_8(oprom,1));
			length++;
			break;
		case MVN :
		case MVP :
			sprintf(ptr, " $%02x, $%02x", read_8(oprom,2), read_8(oprom,1));
			length += 2;
			break;
	}

	return length | flags | DASMFLAG_SUPPORTED;
}

CPU_DISASSEMBLE( m37710_generic )
{
	return m7700_disassemble(buffer, (pc&0xffff), pc>>16, oprom, 0, 0);
}
