// license:BSD-3-Clause
// copyright-holders:R. Belmont, Karl Stenerud
/*

Mitsubishi 7700 Series CPU disassembler v1.1

By R. Belmont
Based on G65C816 CPU Emulator by Karl Stenerud

*/

#include "emu.h"
#include "m7700ds.h"

#define ADDRESS_24BIT(A) ((A)&0xffffff)

const char *const m7700_disassembler::s_opnames[] =
{
	"ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRA",
	"BRK", "BRL", "BVC", "BVS", "CLC", "CLD", "CLI", "CLV", "CMP", "COP", "CPX",
	"CPY", "DEA", "DEC", "DEX", "DEY", "EOR", "INA", "INC", "INX", "INY", "JML",
	"JMP", "JSL", "JSR", "LDA", "LDX", "LDY", "LSR", "MVN", "MVP", "NOP", "ORA",
	"PEA", "PEI", "PER", "PHA", "PHT", "PHD", "PHK", "PHP", "PHX", "PHY", "PLA",
	"PLT", "PLD", "PLP", "PLX", "PLY", "CLP", "ROL", "ROR", "RTI", "RTL", "RTS",
	"SBC", "SEC", "SED", "SEI", "SEP", "STA", "STP", "STX", "STY", "STZ", "TAX",
	"TAY", "TCS", "TCD", "TDC", "TRB", "TSB", "TSC", "TSX", "TXA", "TXS", "TXY",
	"TYA", "TYX", "WAI", "WDM", "XBA", "XCE", "MPY", "DIV", "MPYS","DIVS","RLA",
	"EXTS","EXTZ","LDT", "LDM", "UNK", "SEB", "SEM", "CLM", "STB", "LDB", "ADCB",
	"SBCB","EORB","TBX", "CMPB","INB", "DEB", "TXB", "TYB", "LSRB","ORB", "CLB",
	"BBC", "BBS", "TBY", "ANDB","PUL", "PSH", "PLB", "XAB", "PHB",
};

const m7700_disassembler::m7700_opcode_struct m7700_disassembler::s_opcodes[256] =
{
	{op::BRK, I, SIG  }, {op::ORA, M, DXI }, {op::UNK, I, SIG }, {op::ORA, M, S    },
	{op::SEB, M, LDM4 }, {op::ORA, M, D   }, {op::ASL, M, D   }, {op::ORA, M, DLI  },
	{op::PHP, I, IMP  }, {op::ORA, M, IMM }, {op::ASL, M, ACC }, {op::PHD, I, IMP  },
	{op::SEB, M, LDM5 }, {op::ORA, M, A   }, {op::ASL, M, A   }, {op::ORA, M, AL   },
// 0x10
	{op::BPL, I, RELB }, {op::ORA, M, DIY }, {op::ORA, M, DI  }, {op::ORA, M, SIY  },
	{op::CLB, M, LDM4 }, {op::ORA, M, DX  }, {op::ASL, M, DX  }, {op::ORA, M, DLIY },
	{op::CLC, I, IMP  }, {op::ORA, M, AY  }, {op::DEA, I, IMP }, {op::TCS, I, IMP  },
	{op::CLB, M, LDM5 }, {op::ORA, M, AX  }, {op::ASL, M, AX  }, {op::ORA, M, ALX  },
// 0x20
	{op::JSR, I, A    }, {op::AND, M, DXI }, {op::JSL, I, AL  }, {op::AND, M, S    },
	{op::BBS, M, BBCD }, {op::AND, M, D   }, {op::ROL, M, D   }, {op::AND, M, DLI  },
	{op::PLP, I, IMP  }, {op::AND, M, IMM }, {op::ROL, M, ACC }, {op::PLD, I, IMP  },
	{op::BBS, M, BBCA }, {op::AND, M, A   }, {op::ROL, M, A   }, {op::AND, M, AL   },
// 0x30
	{op::BMI, I, RELB }, {op::AND, M, DIY }, {op::AND, M, DI  }, {op::AND, M, SIY  },
	{op::BBC, M, BBCD }, {op::AND, M, DX  }, {op::ROL, M, DX  }, {op::AND, M, DLIY },
	{op::SEC, I, IMP  }, {op::AND, M, AY  }, {op::INA, I, IMP }, {op::TSC, I, IMP  },
	{op::BBC, M, BBCA }, {op::AND, M, AX  }, {op::ROL, M, AX  }, {op::AND, M, ALX  },
// 0x40
	{op::RTI, I, IMP  }, {op::EOR, M, DXI }, {op::WDM, I, IMP }, {op::EOR, M, S    },
	{op::MVP, I, MVP  }, {op::EOR, M, D   }, {op::LSR, M, D   }, {op::EOR, M, DLI  },
	{op::PHA, I, IMP  }, {op::EOR, M, IMM }, {op::LSR, M, ACC }, {op::PHK, I, IMP  },
	{op::JMP, I, A    }, {op::EOR, M, A   }, {op::LSR, M, A   }, {op::EOR, M, AL   },
// 0x50
	{op::BVC, I, RELB }, {op::EOR, M, DIY }, {op::EOR, M, DI  }, {op::EOR, M, SIY  },
	{op::MVN, I, MVN  }, {op::EOR, M, DX  }, {op::LSR, M, DX  }, {op::EOR, M, DLIY },
	{op::CLI, I, IMP  }, {op::EOR, M, AY  }, {op::PHY, I, IMP }, {op::TCD, I, IMP  },
	{op::JMP, I, AL   }, {op::EOR, M, AX  }, {op::LSR, M, AX  }, {op::EOR, M, ALX  },
// 0x60
	{op::RTS, I, IMP  }, {op::ADC, M, DXI }, {op::PER, I, PER }, {op::ADC, M, S    },
	{op::LDM, M, LDM4 }, {op::ADC, M, D   }, {op::ROR, M, D   }, {op::ADC, M, DLI  },
	{op::PLA, I, IMP  }, {op::ADC, M, IMM }, {op::ROR, M, ACC }, {op::RTL, I, IMP  },
	{op::JMP, I, AI   }, {op::ADC, M, A   }, {op::ROR, M, A   }, {op::ADC, M, AL   },
// 0x70
	{op::BVS, I, RELB }, {op::ADC, M, DIY }, {op::ADC, M, DI  }, {op::ADC, M, SIY  },
	{op::LDM, M, LDM4X}, {op::ADC, M, DX  }, {op::ROR, M, DX  }, {op::ADC, M, DLIY },
	{op::SEI, I, IMP  }, {op::ADC, M, AY  }, {op::PLY, I, IMP }, {op::TDC, I, IMP  },
	{op::JMP, I, AXI  }, {op::ADC, M, AX  }, {op::ROR, M, AX  }, {op::ADC, M, ALX  },
// 0x80
	{op::BRA, I, RELB }, {op::STA, M, DXI }, {op::BRL, I, RELW}, {op::STA, M, S    },
	{op::STY, X, D    }, {op::STA, M, D   }, {op::STX, X, D   }, {op::STA, M, DLI  },
	{op::DEY, I, IMP  }, {op::BIT, M, IMM }, {op::TXA, I, IMP }, {op::PHT, I, IMP  },
	{op::STY, X, A    }, {op::STA, M, A   }, {op::STX, X, A   }, {op::STA, M, AL   },
// 0x90
	{op::BCC, I, RELB }, {op::STA, M, DIY }, {op::STA, M, DI  }, {op::STA, M, SIY  },
	{op::STY, X, DX   }, {op::STA, M, DX  }, {op::STX, X, DY  }, {op::STA, M, DLIY },
	{op::TYA, I, IMP  }, {op::STA, M, AY  }, {op::TXS, I, IMP }, {op::TXY, I, IMP  },
	{op::LDM, M, LDM5 }, {op::STA, M, AX  }, {op::LDM, M, LDM5X},{op::STA, M, ALX  },
// 0xA0
	{op::LDY, X, IMM  }, {op::LDA, M, DXI }, {op::LDX, X, IMM }, {op::LDA, M, S    },
	{op::LDY, X, D    }, {op::LDA, M, D   }, {op::LDX, X, D   }, {op::LDA, M, DLI  },
	{op::TAY, I, IMP  }, {op::LDA, M, IMM }, {op::TAX, I, IMP }, {op::PLB, I, IMP  },
	{op::LDY, X, A    }, {op::LDA, M, A   }, {op::LDX, X, A   }, {op::LDA, M, AL   },
// 0xB0
	{op::BCS, I, RELB }, {op::LDA, M, DIY }, {op::LDA, M, DI  }, {op::LDA, M, SIY  },
	{op::LDY, X, DX   }, {op::LDA, M, DX  }, {op::LDX, X, DY  }, {op::LDA, M, DLIY },
	{op::CLV, I, IMP  }, {op::LDA, M, AY  }, {op::TSX, I, IMP }, {op::TYX, I, IMP  },
	{op::LDY, X, AX   }, {op::LDA, M, AX  }, {op::LDX, X, AY  }, {op::LDA, M, ALX  },
// 0xC0
	{op::CPY, X, IMM  }, {op::CMP, M, DXI }, {op::CLP, I, IMM }, {op::CMP, M, S    },
	{op::CPY, X, D    }, {op::CMP, M, D   }, {op::DEC, M, D   }, {op::CMP, M, DLI  },
	{op::INY, I, IMP  }, {op::CMP, M, IMM }, {op::DEX, I, IMP }, {op::WAI, I, IMP  },
	{op::CPY, X, A    }, {op::CMP, M, A   }, {op::DEC, M, A   }, {op::CMP, M, AL   },
// 0xD0
	{op::BNE, I, RELB }, {op::CMP, M, DIY }, {op::CMP, M, DI  }, {op::CMP, M, SIY  },
	{op::PEI, I, PEI  }, {op::CMP, M, DX  }, {op::DEC, M, DX  }, {op::CMP, M, DLIY },
	{op::CLM, I, IMP  }, {op::CMP, M, AY  }, {op::PHX, I, IMP }, {op::STP, I, IMP  },
	{op::JML, I, AI   }, {op::CMP, M, AX  }, {op::DEC, M, AX  }, {op::CMP, M, ALX  },
// 0xE0
	{op::CPX, X, IMM  }, {op::SBC, M, DXI }, {op::SEP, I, IMM }, {op::SBC, M, S    },
	{op::CPX, X, D    }, {op::SBC, M, D   }, {op::INC, M, D   }, {op::SBC, M, DLI  },
	{op::INX, M, IMP  }, {op::SBC, M, IMM }, {op::NOP, I, IMP }, {op::PSH, I, IMM  },
	{op::CPX, X, A    }, {op::SBC, M, A   }, {op::INC, M, A   }, {op::SBC, M, AL   },
// 0xF0
	{op::BEQ, I, RELB }, {op::SBC, M, DIY }, {op::SBC, M, DI  }, {op::SBC, M, SIY  },
	{op::PEA, I, PEA  }, {op::SBC, M, DX  }, {op::INC, M, DX  }, {op::SBC, M, DLIY },
	{op::SEM, I, IMP  }, {op::SBC, M, AY  }, {op::PLX, I, IMP }, {op::PUL, I, IMM  },
	{op::JSR, I, AXI  }, {op::SBC, M, AX  }, {op::INC, M, AX  }, {op::SBC, M, ALX  }
};

const m7700_disassembler::m7700_opcode_struct m7700_disassembler::s_opcodes_prefix42[256] =
{
	{op::BRK, I, SIG  }, {op::ORB, M, DXI }, {op::COP, I, SIG }, {op::ORB,  M, S   },
	{op::TSB, M, D    }, {op::ORB, M, D   }, {op::ASL, M, D   }, {op::ORB,  M, DLI },
	{op::PHP, I, IMP  }, {op::ORB, M, IMM }, {op::ASL, M, ACCB}, {op::PHD,  I, IMP },
	{op::TSB, M, A    }, {op::ORB, M, A   }, {op::ASL, M, A   }, {op::ORB,  M, AL  },
// 0x10
	{op::BPL, I, RELB }, {op::ORB, M, DIY }, {op::ORB, M, DI  }, {op::ORB,  M, SIY },
	{op::TRB, M, D    }, {op::ORB, M, DX  }, {op::ASL, M, DX  }, {op::ORB,  M, DLIY},
	{op::CLC, I, IMP  }, {op::ORB, M, AY  }, {op::DEB, I, IMP }, {op::TCS,  I, IMP },
	{op::TRB, M, A    }, {op::ORB, M, AX  }, {op::ASL, M, AX  }, {op::ORB,  M, ALX },
// 0x20
	{op::JSR, I, A    }, {op::ANDB,M, DXI }, {op::JSL, I, AL  }, {op::ANDB, M, S   },
	{op::BIT, M, D    }, {op::ANDB,M, D   }, {op::ROL, M, D   }, {op::ANDB, M, DLI },
	{op::PLP, I, IMP  }, {op::ANDB,M, IMM }, {op::ROL, M, ACCB}, {op::PLD,  I, IMP },
	{op::BIT, M, A    }, {op::ANDB,M, A   }, {op::ROL, M, A   }, {op::ANDB, M, AL  },
// 0x30
	{op::BMI, I, RELB }, {op::AND, M, DIY }, {op::AND, M, DI  }, {op::AND,  M, SIY },
	{op::BIT, M, DX   }, {op::AND, M, DX  }, {op::ROL, M, DX  }, {op::AND,  M, DLIY},
	{op::SEC, I, IMP  }, {op::AND, M, AY  }, {op::INB, I, IMP }, {op::TSC,  I, IMP },
	{op::BIT, M, AX   }, {op::AND, M, AX  }, {op::ROL, M, AX  }, {op::AND,  M, ALX },
// 0x40
	{op::RTI, I, IMP  }, {op::EORB,M, DXI }, {op::WDM, I, IMP }, {op::EORB, M, S   },
	{op::MVP, I, MVP  }, {op::EORB,M, D   }, {op::LSRB,M, D   }, {op::EORB, M, DLI },
	{op::PHB, I, IMP  }, {op::EORB,M, IMM }, {op::LSRB,M, ACC }, {op::PHK,  I, IMP },
	{op::JMP, I, A    }, {op::EORB,M, A   }, {op::LSRB,M, A   }, {op::EORB, M, AL  },
// 0x50
	{op::BVC, I, RELB }, {op::EORB,M, DIY }, {op::EORB,M, DI  }, {op::EORB, M, SIY },
	{op::MVN, I, MVN  }, {op::EORB,M, DX  }, {op::LSRB,M, DX  }, {op::EORB, M, DLIY},
	{op::CLI, I, IMP  }, {op::EORB,M, AY  }, {op::PHY, I, IMP }, {op::TCD,  I, IMP },
	{op::JMP, I, AL   }, {op::EORB,M, AX  }, {op::LSRB,M, AX  }, {op::EORB, M, ALX },
// 0x60
	{op::RTS, I, IMP  }, {op::ADCB,M, DXI }, {op::PER, I, PER }, {op::ADCB, M, S   },
	{op::STZ, M, D    }, {op::ADCB,M, D   }, {op::ROR, M, D   }, {op::ADCB, M, DLI },
	{op::PLAB,I, IMP  }, {op::ADCB,M, IMM }, {op::ROR, M, ACC }, {op::RTL,  I, IMP },
	{op::JMP, I, AI   }, {op::ADCB,M, A   }, {op::ROR, M, A   }, {op::ADCB, M, AL  },
// 0x70
	{op::BVS, I, RELB }, {op::ADCB,M, DIY }, {op::ADCB,M, DI  }, {op::ADCB, M, SIY },
	{op::STZ, M, DX   }, {op::ADCB,M, DX  }, {op::ROR, M, DX  }, {op::ADCB, M, DLIY},
	{op::SEI, I, IMP  }, {op::ADCB,M, AY  }, {op::PLY, I, IMP }, {op::TDC,  I, IMP },
	{op::JMP, I, AXI  }, {op::ADCB,M, AX  }, {op::ROR, M, AX  }, {op::ADCB, M, ALX },
// 0x80
	{op::BRA, I, RELB }, {op::STB, M, DXI }, {op::BRL, I, RELW}, {op::STB,  M, S   },
	{op::STY, X, D    }, {op::STB, M, D   }, {op::STX, X, D   }, {op::STB,  M, DLI },
	{op::DEY, I, IMP  }, {op::BIT, M, IMM }, {op::TXB, I, IMP }, {op::PHB,  I, IMP },
	{op::STY, X, A    }, {op::STB, M, A   }, {op::STX, X, A   }, {op::STB,  M, AL  },
// 0x90
	{op::BCC, I, RELB }, {op::STB, M, DIY }, {op::STB, M, DI  }, {op::STB,  M, SIY },
	{op::STY, X, DX   }, {op::STB, M, DX  }, {op::STX, X, DY  }, {op::STB,  M, DLIY},
	{op::TYB, I, IMP  }, {op::STB, M, AY  }, {op::TXS, I, IMP }, {op::TXY,  I, IMP },
	{op::STZ, M, A    }, {op::STB, M, AX  }, {op::STZ, M, AX  }, {op::STB,  M, ALX },
// 0xA0
	{op::LDY, X, IMM  }, {op::LDB, M, DXI }, {op::LDX, X, IMM }, {op::LDB,  M, S   },
	{op::LDY, X, D    }, {op::LDB, M, D   }, {op::LDX, X, D   }, {op::LDB,  M, DLI },
	{op::TBY, I, IMP  }, {op::LDB, M, IMM }, {op::TBX, I, IMP }, {op::PLB,  I, IMP },
	{op::LDY, X, A    }, {op::LDB, M, A   }, {op::LDX, X, A   }, {op::LDB,  M, AL  },
// 0xB0
	{op::BCS, I, RELB }, {op::LDB, M, DIY }, {op::LDB, M, DI  }, {op::LDB,  M, SIY },
	{op::LDY, X, DX   }, {op::LDB, M, DX  }, {op::LDX, X, DY  }, {op::LDB,  M, DLIY},
	{op::CLV, I, IMP  }, {op::LDB, M, AY  }, {op::TSX, I, IMP }, {op::TYX,  I, IMP },
	{op::LDY, X, AX   }, {op::LDB, M, AX  }, {op::LDX, X, AY  }, {op::LDB,  M, ALX },
// 0xC0
	{op::CPY, X, IMM  }, {op::CMPB,M, DXI }, {op::CLP, I, IMM }, {op::CMPB, M, S   },
	{op::CPY, X, D    }, {op::CMPB,M, D   }, {op::DEC, M, D   }, {op::CMPB, M, DLI },
	{op::INY, I, IMP  }, {op::CMPB,M, IMM }, {op::DEX, I, IMP }, {op::WAI,  I, IMP },
	{op::CPY, X, A    }, {op::CMPB,M, A   }, {op::DEC, M, A   }, {op::CMPB, M, AL  },
// 0xD0
	{op::BNE, I, RELB }, {op::CMPB,M, DIY }, {op::CMPB,M, DI  }, {op::CMPB, M, SIY },
	{op::PEI, I, PEI  }, {op::CMPB,M, DX  }, {op::DEC, M, DX  }, {op::CMPB, M, DLIY},
	{op::CLD, I, IMP  }, {op::CMPB,M, AY  }, {op::PHX, I, IMP }, {op::STP,  I, IMP },
	{op::JML, I, AI   }, {op::CMPB,M, AX  }, {op::DEC, M, AX  }, {op::CMPB, M, ALX },
// 0xE0
	{op::CPX, X, IMM  }, {op::SBCB,M, DXI }, {op::SEP, I, IMM }, {op::SBCB, M, S   },
	{op::CPX, X, D    }, {op::SBCB,M, D   }, {op::INC, M, D   }, {op::SBCB, M, DLI },
	{op::INX, M, IMP  }, {op::SBCB,M, IMM }, {op::NOP, I, IMP }, {op::XBA,  I, IMP },
	{op::CPX, X, A    }, {op::SBCB,M, A   }, {op::INC, M, A   }, {op::SBCB, M, AL  },
// 0xF0
	{op::BEQ, I, RELB }, {op::SBCB,M, DIY }, {op::SBCB,M, DI  }, {op::SBCB, M, SIY },
	{op::PEA, I, PEA  }, {op::SBCB,M, DX  }, {op::INC, M, DX  }, {op::SBCB, M, DLIY},
	{op::SED, I, IMP  }, {op::SBCB,M, AY  }, {op::PLX, I, IMP }, {op::XCE,  I, IMP },
	{op::JSR, I, AXI  }, {op::SBCB,M, AX  }, {op::INC, M, AX  }, {op::SBCB, M, ALX }
};

const m7700_disassembler::m7700_opcode_struct m7700_disassembler::s_opcodes_prefix89[256] =
{
	{op::BRK, I, SIG  }, {op::MPY, M, DXI }, {op::COP, I, SIG }, {op::MPY, M, S    },
	{op::TSB, M, D    }, {op::MPY, M, D   }, {op::ASL, M, D   }, {op::MPY, M, DLI  },
	{op::PHP, I, IMP  }, {op::MPY, M, IMM }, {op::ASL, M, ACC }, {op::PHD, I, IMP  },
	{op::TSB, M, A    }, {op::MPY, M, A   }, {op::ASL, M, A   }, {op::MPY, M, AL   },
// 0x10
	{op::BPL, I, RELB }, {op::ORA, M, DIY }, {op::ORA, M, DI  }, {op::ORA, M, SIY  },
	{op::TRB, M, D    }, {op::MPY, M, DX  }, {op::ASL, M, DX  }, {op::ORA, M, DLIY },
	{op::CLC, I, IMP  }, {op::MPY, M, AY  }, {op::INA, I, IMP }, {op::TCS, I, IMP  },
	{op::TRB, M, A    }, {op::ORA, M, AX  }, {op::ASL, M, AX  }, {op::ORA, M, ALX  },
// 0x20
	{op::JSR, I, A    }, {op::AND, M, DXI }, {op::JSL, I, AL  }, {op::AND, M, S    },
	{op::BIT, M, D    }, {op::AND, M, D   }, {op::ROL, M, D   }, {op::AND, M, DLI  },
	{op::XAB, I, IMP  }, {op::AND, M, IMM }, {op::ROL, M, ACC }, {op::PLD, I, IMP  },
	{op::BIT, M, A    }, {op::AND, M, A   }, {op::ROL, M, A   }, {op::AND, M, AL   },
// 0x30
	{op::BMI, I, RELB }, {op::AND, M, DIY }, {op::AND, M, DI  }, {op::AND, M, SIY  },
	{op::BIT, M, DX   }, {op::AND, M, DX  }, {op::ROL, M, DX  }, {op::AND, M, DLIY },
	{op::SEC, I, IMP  }, {op::AND, M, AY  }, {op::DEA, I, IMP }, {op::TSC, I, IMP  },
	{op::BIT, M, AX   }, {op::AND, M, AX  }, {op::ROL, M, AX  }, {op::AND, M, ALX  },
// 0x40
	{op::RTI, I, IMP  }, {op::EOR, M, DXI }, {op::WDM, I, IMP }, {op::EOR, M, S    },
	{op::MVP, I, MVP  }, {op::EOR, M, D   }, {op::LSR, M, D   }, {op::EOR, M, DLI  },
	{op::PHA, I, IMP  }, {op::RLA, M, IMM }, {op::LSR, M, ACC }, {op::PHK, I, IMP  },
	{op::JMP, I, A    }, {op::EOR, M, A   }, {op::LSR, M, A   }, {op::EOR, M, AL   },
// 0x50
	{op::BVC, I, RELB }, {op::EOR, M, DIY }, {op::EOR, M, DI  }, {op::EOR, M, SIY  },
	{op::MVN, I, MVN  }, {op::EOR, M, DX  }, {op::LSR, M, DX  }, {op::EOR, M, DLIY },
	{op::CLI, I, IMP  }, {op::EOR, M, AY  }, {op::PHY, I, IMP }, {op::TCD, I, IMP  },
	{op::JMP, I, AL   }, {op::EOR, M, AX  }, {op::LSR, M, AX  }, {op::EOR, M, ALX  },
// 0x60
	{op::RTS, I, IMP  }, {op::ADC, M, DXI }, {op::PER, I, PER }, {op::ADC, M, S    },
	{op::STZ, M, D    }, {op::ADC, M, D   }, {op::ROR, M, D   }, {op::ADC, M, DLI  },
	{op::PLA, I, IMP  }, {op::ADC, M, IMM }, {op::ROR, M, ACC }, {op::RTL, I, IMP  },
	{op::JMP, I, AI   }, {op::ADC, M, A   }, {op::ROR, M, A   }, {op::ADC, M, AL   },
// 0x70
	{op::BVS, I, RELB }, {op::ADC, M, DIY }, {op::ADC, M, DI  }, {op::ADC, M, SIY  },
	{op::STZ, M, DX   }, {op::ADC, M, DX  }, {op::ROR, M, DX  }, {op::ADC, M, DLIY },
	{op::SEI, I, IMP  }, {op::ADC, M, AY  }, {op::PLY, I, IMP }, {op::TDC, I, IMP  },
	{op::JMP, I, AXI  }, {op::ADC, M, AX  }, {op::ROR, M, AX  }, {op::ADC, M, ALX  },
// 0x80
	{op::BRA, I, RELB }, {op::STA, M, DXI }, {op::BRL, I, RELW}, {op::STA, M, S    },
	{op::STY, X, D    }, {op::STA, M, D   }, {op::STX, X, D   }, {op::STA, M, DLI  },
	{op::DEY, I, IMP  }, {op::BIT, M, IMM }, {op::TXA, I, IMP }, {op::PHB, I, IMP  },
	{op::STY, X, A    }, {op::STA, M, A   }, {op::STX, X, A   }, {op::STA, M, AL   },
// 0x90
	{op::BCC, I, RELB }, {op::STA, M, DIY }, {op::STA, M, DI  }, {op::STA, M, SIY  },
	{op::STY, X, DX   }, {op::STA, M, DX  }, {op::STX, X, DY  }, {op::STA, M, DLIY },
	{op::TYA, I, IMP  }, {op::STA, M, AY  }, {op::TXS, I, IMP }, {op::TXY, I, IMP  },
	{op::STZ, M, A    }, {op::STA, M, AX  }, {op::STZ, M, AX  }, {op::STA, M, ALX  },
// 0xA0
	{op::LDY, X, IMM  }, {op::LDA, M, DXI }, {op::LDX, X, IMM }, {op::LDA, M, S    },
	{op::LDY, X, D    }, {op::LDA, M, D   }, {op::LDX, X, D   }, {op::LDA, M, DLI  },
	{op::TAY, I, IMP  }, {op::LDA, M, IMM }, {op::TAX, I, IMP }, {op::PLB, I, IMP  },
	{op::LDY, X, A    }, {op::LDA, M, A   }, {op::LDX, X, A   }, {op::LDA, M, AL   },
// 0xB0
	{op::BCS, I, RELB }, {op::LDA, M, DIY }, {op::LDA, M, DI  }, {op::LDA, M, SIY  },
	{op::LDY, X, DX   }, {op::LDA, M, DX  }, {op::LDX, X, DY  }, {op::LDA, M, DLIY },
	{op::CLV, I, IMP  }, {op::LDA, M, AY  }, {op::TSX, I, IMP }, {op::TYX, I, IMP  },
	{op::LDY, X, AX   }, {op::LDA, M, AX  }, {op::LDX, X, AY  }, {op::LDA, M, ALX  },
// 0xC0
	{op::CPY, X, IMM  }, {op::CMP, M, DXI }, {op::LDT, I, IMM }, {op::CMP, M, S    },
	{op::CPY, X, D    }, {op::CMP, M, D   }, {op::DEC, M, D   }, {op::CMP, M, DLI  },
	{op::INY, I, IMP  }, {op::CMP, M, IMM }, {op::DEX, I, IMP }, {op::WAI, I, IMP  },
	{op::CPY, X, A    }, {op::CMP, M, A   }, {op::DEC, M, A   }, {op::CMP, M, AL   },
// 0xD0
	{op::BNE, I, RELB }, {op::CMP, M, DIY }, {op::CMP, M, DI  }, {op::CMP, M, SIY  },
	{op::PEI, I, PEI  }, {op::CMP, M, DX  }, {op::DEC, M, DX  }, {op::CMP, M, DLIY },
	{op::CLD, I, IMP  }, {op::CMP, M, AY  }, {op::PHX, I, IMP }, {op::STP, I, IMP  },
	{op::JML, I, AI   }, {op::CMP, M, AX  }, {op::DEC, M, AX  }, {op::CMP, M, ALX  },
// 0xE0
	{op::CPX, X, IMM  }, {op::SBC, M, DXI }, {op::SEP, I, IMM }, {op::SBC, M, S    },
	{op::CPX, X, D    }, {op::SBC, M, D   }, {op::INC, M, D   }, {op::SBC, M, DLI  },
	{op::INX, M, IMP  }, {op::SBC, M, IMM }, {op::NOP, I, IMP }, {op::XBA, I, IMP  },
	{op::CPX, X, A    }, {op::SBC, M, A   }, {op::INC, M, A   }, {op::SBC, M, AL   },
// 0xF0
	{op::BEQ, I, RELB }, {op::SBC, M, DIY }, {op::SBC, M, DI  }, {op::SBC, M, SIY  },
	{op::PEA, I, PEA  }, {op::SBC, M, DX  }, {op::INC, M, DX  }, {op::SBC, M, DLIY },
	{op::SEM, I, IMP  }, {op::SBC, M, AY  }, {op::PLX, I, IMP }, {op::XCE, I, IMP  },
	{op::JSR, I, AXI  }, {op::SBC, M, AX  }, {op::INC, M, AX  }, {op::SBC, M, ALX  }
};

inline std::string m7700_disassembler::int_8_str(unsigned int val)
{
	val &= 0xff;

	if(val & 0x80)
		return util::string_format("-$%x", (0-val) & 0xff);
	else
		return util::string_format("$%x", val & 0xff);
}

inline std::string m7700_disassembler::int_16_str(unsigned int val)
{
	val &= 0xffff;

	if(val & 0x8000)
		return util::string_format("-$%x", (0-val) & 0xffff);
	else
		return util::string_format("$%x", val & 0xffff);
}

m7700_disassembler::m7700_disassembler(config *conf) : m_config(conf)
{
}

u32 m7700_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t m7700_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	unsigned int instruction;
	const m7700_opcode_struct *opcode;
	int var;
	signed char varS;
	int length = 1;
	//unsigned int start;
	uint32_t flags = 0;

	offs_t address = pc;
	u32 pb = pc & 0xffff0000;
	pc &= 0xffff;
	address = pc | pb;

	instruction = opcodes.r8(address);

	int m_flag = m_config->get_m_flag();
	int x_flag = m_config->get_x_flag();

	// check for prefixes
	switch (instruction)
	{
	case 0x42:
		address++;
		length++;
		instruction = opcodes.r8(address);
		opcode = &m7700_opcode_struct::get_prefix42(instruction);
		break;

	case 0x89:
		address++;
		length++;
		instruction = opcodes.r8(address);
		opcode = &m7700_opcode_struct::get_prefix89(instruction);
		break;

	default:
		opcode = &m7700_opcode_struct::get(instruction);
		break;
	}

	if (opcode->is_call())
		flags = STEP_OVER;
	else if (opcode->is_return())
		flags = STEP_OUT;

	stream << opcode->name();

	switch(opcode->ea)
	{
		case IMP :
			break;
		case ACC :
			util::stream_format(stream, " A");
			break;
		case ACCB :
			util::stream_format(stream, " B");
			break;
		case RELB:
			varS = opcodes.r8(address + 1);
			length++;
			util::stream_format(stream, " %06x (%s)", pb | ((pc + length + varS)&0xffff), int_8_str(varS));
			break;
		case RELW:
		case PER :
			var = opcodes.r16(address + 1);
			length += 2;
			util::stream_format(stream, " %06x (%s)", pb | ((pc + length + var)&0xffff), int_16_str(var));
			break;
		case IMM :
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				util::stream_format(stream, " #$%04x", opcodes.r16(address + 1));
				length += 2;
			}
			else
			{
				util::stream_format(stream, " #$%02x", opcodes.r8(address + 1));
				length++;
			}
			break;
		case BBCD:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				varS = opcodes.r8(address + 4);
				length += 4;
				util::stream_format(stream, " #$%04x, $%02x, %06x (%s)", opcodes.r16(address + 2), opcodes.r8(address + 1), pb | ((pc + length + varS)&0xffff), int_8_str(varS));
			}
			else
			{
				varS = opcodes.r8(address + 3);
				length += 3;
				util::stream_format(stream, " #$%02x, $%02x, %06x (%s)", opcodes.r8(address + 2), opcodes.r8(address + 1), pb | ((pc + length + varS)&0xffff), int_8_str(varS));
			}
			break;
		case BBCA:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				length += 5;
				varS = opcodes.r8(address + 5);
				util::stream_format(stream, " #$%04x, $%04x, %06x (%s)", opcodes.r16(address + 3), opcodes.r16(address + 1), pb | ((pc + length + varS)&0xffff), int_8_str(varS));
			}
			else
			{
				length += 4;
				varS = opcodes.r8(address + 4);
				util::stream_format(stream, " #$%02x, $%04x, %06x (%s)", opcodes.r8(address + 3), opcodes.r16(address + 1), pb | ((pc + length + varS)&0xffff), int_8_str(varS));
			}
			break;
		case LDM4:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				util::stream_format(stream, " #$%04x, $%02x", opcodes.r16(address + 2), opcodes.r8(address + 1));
				length += 3;
			}
			else
			{
				util::stream_format(stream, " #$%02x, $%02x", opcodes.r8(address + 2), opcodes.r8(address + 1));
				length += 2;
			}
			break;
		case LDM5:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				util::stream_format(stream, " #$%04x, $%04x", opcodes.r16(address + 3), opcodes.r16(address + 1));
				length += 4;
			}
			else
			{
				util::stream_format(stream, " #$%02x, $%04x", opcodes.r8(address + 3), opcodes.r16(address + 1));
				length += 3;
			}
			break;
		case LDM4X:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				util::stream_format(stream, " #$%04x, $%02x, X", opcodes.r16(address + 2), opcodes.r8(address + 1));
				length += 3;
			}
			else
			{
				util::stream_format(stream, " #$%02x, $%02x, X", opcodes.r8(address + 2), opcodes.r8(address + 1));
				length += 2;
			}
			break;
		case LDM5X:
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				util::stream_format(stream, " #$%04x, $%04x, X", opcodes.r16(address + 3), opcodes.r16(address + 1));
				length += 4;
			}
			else
			{
				util::stream_format(stream, " #$%02x, $%04x, X", opcodes.r8(address + 3), opcodes.r16(address + 1));
				length += 3;
			}
			break;
		case A   :
		case PEA :
			util::stream_format(stream, " $%04x", opcodes.r16(address + 1));
			length += 2;
			break;
		case AI  :
			util::stream_format(stream, " ($%04x)", opcodes.r16(address + 1));
			length += 2;
			break;
		case AL  :
			util::stream_format(stream, " $%06x", opcodes.r32(address + 1) & 0xffffff);
			length += 3;
			break;
		case ALX :
			util::stream_format(stream, " $%06x,X", opcodes.r32(address + 1) & 0xffffff);
			length += 3;
			break;
		case AX  :
			util::stream_format(stream, " $%04x,X", opcodes.r16(address + 1));
			length += 2;
			break;
		case AXI :
			util::stream_format(stream, " ($%04x,X)", opcodes.r16(address + 1));
			length += 2;
			break;
		case AY  :
			util::stream_format(stream, " $%04x,Y", opcodes.r16(address + 1));
			length += 2;
			break;
		case D   :
			util::stream_format(stream, " $%02x", opcodes.r8(address + 1));
			length++;
			break;
		case DI  :
		case PEI :
			util::stream_format(stream, " ($%02x)", opcodes.r8(address + 1));
			length++;
			break;
		case DIY :
			util::stream_format(stream, " ($%02x),Y", opcodes.r8(address + 1));
			length++;
			break;
		case DLI :
			util::stream_format(stream, " [$%02x]", opcodes.r8(address + 1));
			length++;
			break;
		case DLIY:
			util::stream_format(stream, " [$%02x],Y", opcodes.r8(address + 1));
			length++;
			break;
		case DX  :
			util::stream_format(stream, " $%02x,X", opcodes.r8(address + 1));
			length++;
			break;
		case DXI :
			util::stream_format(stream, " ($%02x,X)", opcodes.r8(address + 1));
			length++;
			break;
		case DY  :
			util::stream_format(stream, " $%02x,Y", opcodes.r8(address + 1));
			length++;
			break;
		case S   :
			util::stream_format(stream, " %s,S", int_8_str(opcodes.r8(address + 1)));
			length++;
			break;
		case SIY :
			util::stream_format(stream, " (%s,S),Y", int_8_str(opcodes.r8(address + 1)));
			length++;
			break;
		case SIG :
			util::stream_format(stream, " #$%02x", opcodes.r8(address + 1));
			length++;
			break;
		case MVN :
		case MVP :
			util::stream_format(stream, " $%02x, $%02x", opcodes.r8(address + 2), opcodes.r8(address + 1));
			length += 2;
			break;
	}

	return length | flags | SUPPORTED;
}
