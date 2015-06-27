// license:BSD-3-Clause
// copyright-holders:David Haywood

// CPS2 keys

#define CRYPT_PARAMS( _key1, _key2, _lower, _upper ) \
	ROM_PARAMETER( "cryptkey1",   _key1 ) \
	ROM_PARAMETER( "cryptkey2",   _key2 ) \
	ROM_PARAMETER( "cryptlower", _lower ) \
	ROM_PARAMETER( "cryptupper", _upper )
// the watchdog opcode sequence should also be a parameter if we want to emulate that too?



// On a dead board, the only encrypted range is actually FF0000-FFFFFF.
// It doesn't start from 0, and it's the upper half of a 128kB bank.

//                                                   key1        key2        lowwer    upper          watchdog
#define CPS2_DEAD_KEY                  CRYPT_PARAMS( "ffffffff","ffffffff", "ff0000", "ffffff" )    // ffff ffff ffff

#define SSF2_WORLD_KEY                 CRYPT_PARAMS( "23456789","abcdef01", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000
#define SSF2_USA_KEY                   CRYPT_PARAMS( "12345678","9abcdef0", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000
#define SSF2_ASIA_KEY                  CRYPT_PARAMS( "3456789a","bcdef012", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000
#define SSF2_JAPAN_KEY                 CRYPT_PARAMS( "01234567","89abcdef", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000
#define SSF2_HISPANIC_KEY              CRYPT_PARAMS( "56789abc","def01234", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000

#define SSF2TB_WORLD_KEY               CRYPT_PARAMS( "89abcdef","01234567", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000
#define SSF2TB_JAPAN_KEY               CRYPT_PARAMS( "6789abcd","ef012345", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000
#define SSF2TB_HISPANIC_KEY            CRYPT_PARAMS( "bcdef012","3456789a", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000

#define DDTOD_WORLD_KEY                CRYPT_PARAMS( "4767fe08","14ca35d9", "000000", "180000" )    // 0C78 1019 4000  cmpi.w  #$1019,$4000
#define DDTOD_USA_KEY                  CRYPT_PARAMS( "eca19c3d","24736bf0", "000000", "180000" )    // 0C78 1019 4000  cmpi.w  #$1019,$4000
#define DDTOD_JAPAN_KEY                CRYPT_PARAMS( "4510e79c","f36b8a2d", "000000", "180000" )    // 0C78 1019 4000  cmpi.w  #$1019,$4000
#define DDTOD_ASIA_KEY                 CRYPT_PARAMS( "decac105","19710411", "000000", "180000" )    // 0C78 1019 4000  cmpi.w  #$1019,$4000
#define DDTOD_HISPANIC_KEY             CRYPT_PARAMS( "19691019","e825dde0", "000000", "180000" )    // 0C78 1019 4000  cmpi.w  #$1019,$4000

#define ECOFGHTR_WORLD_KEY             CRYPT_PARAMS( "931031dc","ba987654", "000000", "200000" )    // 0838 0003 7345  btst    #3,$7345
#define ECOFGHTR_USA_KEY               CRYPT_PARAMS( "931031ed","cba98765", "000000", "200000" )    // 0838 0003 7345  btst    #3,$7345
#define ECOFGHTR_JAPAN_KEY             CRYPT_PARAMS( "931031fe","dcba9876", "000000", "200000" )    // 0838 0003 7345  btst    #3,$7345
#define ECOFGHTR_ASIA_KEY              CRYPT_PARAMS( "931031ba","98765432", "000000", "200000" )    // 0838 0003 7345  btst    #3,$7345
#define ECOFGHTR_HISPANIC_KEY          CRYPT_PARAMS( "931031cb","a9876543", "000000", "200000" )    // 0838 0003 7345  btst    #3,$7345

#define SSF2T_WORLD_KEY                CRYPT_PARAMS( "944e8302","56d3143c", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000
#define SSF2T_ASIA_KEY                 CRYPT_PARAMS( "94c4d002","664a1471", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000
#define SSF2T_USA_KEY                  CRYPT_PARAMS( "94fa8902","4c77143f", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000
#define SSF2T_JAPAN_KEY                CRYPT_PARAMS( "942a5702","05ac140e", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000
#define SSF2T_JAPAN_RENTAL_KEY         CRYPT_PARAMS( "943c2b02","7acd1422", "000000", "400000" )    // 0838 0007 2000  btst    #7,$2000 // curious, not the usual Japan key on the rent version

#define XMCOTA_WORLD_KEY               CRYPT_PARAMS( "3bc6eda4","97f80251", "000000", "100000" )    // 0C80 1972 0301  cmpi.l  #$19720301,D0
#define XMCOTA_USA_KEY                 CRYPT_PARAMS( "32a57ecd","98016f4b", "000000", "100000" )    // 0C80 1972 0301  cmpi.l  #$19720301,D0
#define XMCOTA_HISPANIC_KEY            CRYPT_PARAMS( "f5e8dc34","a096b217", "000000", "100000" )    // 0C80 1972 0301  cmpi.l  #$19720301,D0
#define XMCOTA_JAPAN_KEY               CRYPT_PARAMS( "46027315","af8bcd9e", "000000", "100000" )    // 0C80 1972 0301  cmpi.l  #$19720301,D0
#define XMCOTA_ASIA_KEY                CRYPT_PARAMS( "0795a4e2","db3f861c", "000000", "100000" )    // 0C80 1972 0301  cmpi.l  #$19720301,D0

#define ARMWAR_WORLD_KEY               CRYPT_PARAMS( "9e9d4c0b","8a39081f", "000000", "100000" )    // 3039 0080 4020  move.w  $00804020,D0
#define ARMWAR_USA_KEY                 CRYPT_PARAMS( "d4c0b8a3","9081f9e9", "000000", "100000" )    // 3039 0080 4020  move.w  $00804020,D0
#define ARMWAR_JAPAN_KEY               CRYPT_PARAMS( "9d4c0b8a","39081f9e", "000000", "100000" )    // 3039 0080 4020  move.w  $00804020,D0
#define ARMWAR_ASIA_KEY                CRYPT_PARAMS( "1f9e9d4c","0b8a3908", "000000", "100000" )    // 3039 0080 4020  move.w  $00804020,D0

#define AVSP_WORLD_KEY                 CRYPT_PARAMS( "15208f79","4ade6cb3", "000000", "100000" )    // 0C80 1234 5678  cmpi.l  #$12345678,D0
#define AVSP_USA_KEY                   CRYPT_PARAMS( "b4f61089","ccf75a23", "000000", "100000" )    // 0C80 1234 5678  cmpi.l  #$12345678,D0
#define AVSP_JAPAN_KEY                 CRYPT_PARAMS( "e9dcb8fa","51372064", "000000", "100000" )    // 0C80 1234 5678  cmpi.l  #$12345678,D0
#define AVSP_ASIA_KEY                  CRYPT_PARAMS( "c168f3bd","2e4a5970", "000000", "100000" )    // 0C80 1234 5678  cmpi.l  #$12345678,D0
#define AVSP_HISPANIC_KEY              CRYPT_PARAMS( "712b690a","43cd8e5f", "000000", "100000" )    // 0C80 1234 5678  cmpi.l  #$12345678,D0

#define DSTLK_WORLD_KEY                CRYPT_PARAMS( "13d8a7a8","0008b090", "000000", "100000" )    // 0838 0000 6160  btst    #0,$6160
#define DSTLK_USA_KEY                  CRYPT_PARAMS( "1e80ebf0","10227119", "000000", "100000" )    // 0838 0000 6160  btst    #0,$6160
#define DSTLK_ASIA_KEY                 CRYPT_PARAMS( "205d8398","06221971", "000000", "100000" )    // 0838 0000 6160  btst    #0,$6160
#define DSTLK_HISPANIC_KEY             CRYPT_PARAMS( "22463efe","011169aa", "000000", "100000" )    // 0838 0000 6160  btst    #0,$6160
#define DSTLK_JAPAN_KEY                CRYPT_PARAMS( "efcb0804","026819ae", "000000", "100000" )    // 0838 0000 6160  btst    #0,$6160

#define RINGDEST_WORLD_KEY             CRYPT_PARAMS( "19940727","17444903", "000000", "180000" )    // 3039 0080 4020  move.w  $00804020,D0
#define RINGDEST_JAPAN_KEY             CRYPT_PARAMS( "19940209","17031403", "000000", "180000" )    // 3039 0080 4020  move.w  $00804020,D0
#define RINGDEST_ASIA_KEY              CRYPT_PARAMS( "19940727","17452103", "000000", "180000" )    // 3039 0080 4020  move.w  $00804020,D0

#define CYBOTS_WORLD_KEY               CRYPT_PARAMS( "45425943","05090901", "000000", "100000" )    // 0C38 00FF 0C38  cmpi.b  #$FF,$0C38
#define CYBOTS_USA_KEY                 CRYPT_PARAMS( "43050909","01554259", "000000", "100000" )    // 0C38 00FF 0C38  cmpi.b  #$FF,$0C38
#define CYBOTS_JAPAN_KEY               CRYPT_PARAMS( "05090901","4a425943", "000000", "100000" )    // 0C38 00FF 0C38  cmpi.b  #$FF,$0C38

#define MSH_WORLD_KEY                  CRYPT_PARAMS( "1a11ee26","e7955d17", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1
#define MSH_USA_KEY                    CRYPT_PARAMS( "8705a24e","4a17319b", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1
#define MSH_JAPAN_KEY                  CRYPT_PARAMS( "05e88219","31ad2142", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1
#define MSH_ASIA_KEY                   CRYPT_PARAMS( "457aeb01","3897c53d", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1
#define MSH_HISPANIC_KEY               CRYPT_PARAMS( "fc4c5a50","b59cc190", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1
#define MSH_BRAZIL_KEY                 CRYPT_PARAMS( "7a152416","ad27f8e6", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1

#define NWARR_WORLD_KEY                CRYPT_PARAMS( "1019d145","03f05a05", "000000", "180000" )    // 0838 0000 6160  btst    #0,$6160
#define NWARR_USA_KEY                  CRYPT_PARAMS( "104a7d0c","3f1b7a1e", "000000", "180000" )    // 0838 0000 6160  btst    #0,$6160
#define NWARR_HISPANIC_KEY             CRYPT_PARAMS( "c4961b01","2a946020", "000000", "180000" )    // 0838 0000 6160  btst    #0,$6160
#define NWARR_BRAZIL_KEY               CRYPT_PARAMS( "17c67109","b7362a20", "000000", "180000" )    // 0838 0000 6160  btst    #0,$6160
#define NWARR_ASIA_KEY                 CRYPT_PARAMS( "4e940d0c","39b861a4", "000000", "180000" )    // 0838 0000 6160  btst    #0,$6160
#define NWARR_JAPAN_KEY                CRYPT_PARAMS( "1135b2c3","a4e9d7f2", "000000", "180000" )    // 0838 0000 6160  btst    #0,$6160

#define SFA_WORLD_KEY                  CRYPT_PARAMS( "0f895d6e","c4273a1b", "000000", "080000" )    // 0C80 0564 2194  cmpi.l  #$05642194,D0
#define SFA_USA_KEY                    CRYPT_PARAMS( "25bead36","97cf4018", "000000", "080000" )    // 0C80 0564 2194  cmpi.l  #$05642194,D0
#define SFA_ASIA_KEY                   CRYPT_PARAMS( "e43dc508","621b9a7f", "000000", "080000" )    // 0C80 0564 2194  cmpi.l  #$05642194,D0
#define SFA_JAPAN_KEY                  CRYPT_PARAMS( "8db3167a","c29e0f45", "000000", "080000" )    // 0C80 0564 2194  cmpi.l  #$05642194,D0
#define SFA_HISPANIC_KEY               CRYPT_PARAMS( "876b0e39","5ca24fd1", "000000", "080000" )    // 0C80 0564 2194  cmpi.l  #$05642194,D0
#define SFA_BRAZIL_KEY                 CRYPT_PARAMS( "ef415bd3","7a92c680", "000000", "080000" )    // 0C80 0564 2194  cmpi.l  #$05642194,D0

#define MMANCP2_USA_KEY                CRYPT_PARAMS( "054893fa","94642525", "000000", "100000" )    // 0C80 0564 2194  cmpi.l  #$05642194,D0
#define MMANCP2_JAPAN_KEY              CRYPT_PARAMS( "07215501","37fa32d0", "000000", "100000" )    // 0C80 0564 2194  cmpi.l  #$05642194,D0

#define _19XX_USA_KEY                  CRYPT_PARAMS( "0e07181f","5fd0f080", "000000", "200000" )    // 0C81 0095 1101  cmpi.l  #$00951101,D1
#define _19XX_ASIA_KEY                 CRYPT_PARAMS( "cce74cf5","b7da3711", "000000", "200000" )    // 0C81 0095 1101  cmpi.l  #$00951101,D1
#define _19XX_JAPAN_KEY                CRYPT_PARAMS( "00115df8","000ff87e", "000000", "200000" )    // 0C81 0095 1101  cmpi.l  #$00951101,D1
#define _19XX_HISPANIC_KEY             CRYPT_PARAMS( "5d49bafa","f7216c9f", "000000", "200000" )    // 0C81 0095 1101  cmpi.l  #$00951101,D1
#define _19XX_BRAZIL_KEY               CRYPT_PARAMS( "e5f9476a","2dfb623f", "000000", "200000" )    // 0C81 0095 1101  cmpi.l  #$00951101,D1

#define DDSOM_WORLD_KEY                CRYPT_PARAMS( "87889abc","d81f5f63", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1
#define DDSOM_USA_KEY                  CRYPT_PARAMS( "489f0526","1bcd3e7a", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1
#define DDSOM_JAPAN_KEY                CRYPT_PARAMS( "ae92fa94","315a9045", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1
#define DDSOM_ASIA_KEY                 CRYPT_PARAMS( "8719abcd","ef028345", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1
#define DDSOM_HISPANIC_KEY             CRYPT_PARAMS( "42134245","120de607", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1
#define DDSOM_BRAZIL_KEY               CRYPT_PARAMS( "7149a782","f3a5bfce", "000000", "100000" )    // 0C81 1966 0419  cmpi.l  #$19660419,D1

#define MEGAMAN2_USA_KEY               CRYPT_PARAMS( "50501cac","ed346550", "000000", "100000" )    // 0C80 0164 7101  cmpi.l  #$01647101,D0
#define MEGAMAN2_ASIA_KEY              CRYPT_PARAMS( "3f148a2b","d6790a15", "000000", "100000" )    // 0C80 0164 7101  cmpi.l  #$01647101,D0
#define MEGAMAN2_JAPAN_KEY             CRYPT_PARAMS( "319eca73","10551270", "000000", "100000" )    // 0C80 0164 7101  cmpi.l  #$01647101,D0
#define MEGAMAN2_HISPANIC_KEY          CRYPT_PARAMS( "765573ca","250210d0", "000000", "100000" )    // 0C80 0164 7101  cmpi.l  #$01647101,D0

#define QNDREAM_JAPAN_KEY              CRYPT_PARAMS( "5804ea73","f66b0798", "000000", "080000" )    // 0C81 1973 0827  cmpi.l  #$19730827,D1

#define SFA2_WORLD_KEY                 CRYPT_PARAMS( "fc4acf9c","3bfbe1f9", "000000", "100000" )    // 0C80 3039 9783  cmpi.l  #$30399783,D0
#define SFA2_USA_KEY                   CRYPT_PARAMS( "1bbf3d96","8af4614a", "000000", "100000" )    // 0C80 3039 9783  cmpi.l  #$30399783,D0
#define SFA2_JAPAN_KEY                 CRYPT_PARAMS( "83f47e99","da772111", "000000", "100000" )    // 0C80 3039 9783  cmpi.l  #$30399783,D0
#define SFA2_ASIA_KEY                  CRYPT_PARAMS( "afc2e8f4","43789487", "000000", "100000" )    // 0C80 3039 9783  cmpi.l  #$30399783,D0
#define SFA2_BRAZIL_KEY                CRYPT_PARAMS( "ac134599","61f8bb2e", "000000", "100000" )    // 0C80 3039 9783  cmpi.l  #$30399783,D0
#define SFA2_HISPANIC_KEY              CRYPT_PARAMS( "f98a2d42","597b089f", "000000", "100000" )    // 0C80 3039 9783  cmpi.l  #$30399783,D0
#define SFA2_OCEANIA_KEY               CRYPT_PARAMS( "e32bf89c","a57b46dc", "000000", "100000" )    // 0C80 3039 9783  cmpi.l  #$30399783,D0

#define SFZ2AL_ASIA_KEY                CRYPT_PARAMS( "f172c0d0","040621a6", "000000", "100000" )    // 0C80 8E73 9110  cmpi.l  #$8E739110,D0
#define SFZ2AL_JAPAN_KEY               CRYPT_PARAMS( "99450c88","a00a2c4d", "000000", "100000" )    // 0C80 8E73 9110  cmpi.l  #$8E739110,D0
#define SFZ2AL_HISPANIC_KEY            CRYPT_PARAMS( "95f15b7c","200c08c6", "000000", "100000" )    // 0C80 8E73 9110  cmpi.l  #$8E739110,D0
#define SFZ2AL_BRAZIL_KEY              CRYPT_PARAMS( "73cd4a28","ff83af1c", "000000", "100000" )    // 0C80 8E73 9110  cmpi.l  #$8E739110,D0

#define SPF2T_WORLD_KEY                CRYPT_PARAMS( "dde26f09","55821ee7", "000000", "040000" )    // 0C80 3039 9819  cmpi.l  #$30399819,D0
#define SPF2T_USA_KEY                  CRYPT_PARAMS( "706a8750","7d0fc185", "000000", "040000" )    // 0C80 3039 9819  cmpi.l  #$30399819,D0
#define SPF2T_JAPAN_KEY                CRYPT_PARAMS( "b12c835a","e90976ff", "000000", "040000" )    // 0C80 3039 9819  cmpi.l  #$30399819,D0
#define SPF2T_ASIA_KEY                 CRYPT_PARAMS( "9c48e1ab","d60f34fb", "000000", "040000" )    // 0C80 3039 9819  cmpi.l  #$30399819,D0
#define SPF2T_HISPANIC_KEY             CRYPT_PARAMS( "51ed8cab","228f85b6", "000000", "040000" )    // 0C80 3039 9819  cmpi.l  #$30399819,D0

#define XMVSSF_WORLD_KEY               CRYPT_PARAMS( "bdcf8519","3fb2acea", "000000", "100000" )    // 0C81 1972 0327  cmpi.l  #$19720327,D1
#define XMVSSF_USA_KEY                 CRYPT_PARAMS( "4fcb03d2","f8653bc1", "000000", "100000" )    // 0C81 1972 0327  cmpi.l  #$19720327,D1
#define XMVSSF_JAPAN_KEY               CRYPT_PARAMS( "38df93bc","210373ac", "000000", "100000" )    // 0C81 1972 0327  cmpi.l  #$19720327,D1
#define XMVSSF_ASIA_KEY                CRYPT_PARAMS( "7438fc3e","19abed90", "000000", "100000" )    // 0C81 1972 0327  cmpi.l  #$19720327,D1
#define XMVSSF_HISPANIC_KEY            CRYPT_PARAMS( "835fb2d0","42fa9137", "000000", "100000" )    // 0C81 1972 0327  cmpi.l  #$19720327,D1
#define XMVSSF_BRAZIL_KEY              CRYPT_PARAMS( "8ead9e4a","b02184f0", "000000", "100000" )    // 0C81 1972 0327  cmpi.l  #$19720327,D1

#define BATCIR_WORLD_KEY               CRYPT_PARAMS( "d195e597","3cbce2b5", "000000", "200000" )    // 0C81 0097 0131  cmpi.l  #$00970131,D1
#define BATCIR_ASIA_KEY                CRYPT_PARAMS( "1e5d80cb","98882ec7", "000000", "200000" )    // 0C81 0097 0131  cmpi.l  #$00970131,D1
#define BATCIR_JAPAN_KEY               CRYPT_PARAMS( "00ff4dd8","000008e8", "000000", "200000" )    // 0C81 0097 0131  cmpi.l  #$00970131,D1

#define CSCLUB_WORLD_KEY               CRYPT_PARAMS( "662e9fa0","4210e7c1", "000000", "200000" )    // 0C81 0097 0310  cmpi.l  #$00970310,D1
#define CSCLUB_ASIA_KEY                CRYPT_PARAMS( "1366de2a","9ab42937", "000000", "200000" )    // 0C81 0097 0310  cmpi.l  #$00970310,D1
#define CSCLUB_JAPAN_KEY               CRYPT_PARAMS( "4a2d0be5","56c013c0", "000000", "200000" )    // 0C81 0097 0310  cmpi.l  #$00970310,D1
#define CSCLUB_HISPANIC_KEY            CRYPT_PARAMS( "f014a8a7","2e7794d0", "000000", "200000" )    // 0C81 0097 0310  cmpi.l  #$00970310,D1

#define MSHVSSF_WORLD_KEY              CRYPT_PARAMS( "1384ae60","9cd725bf", "000000", "100000" )    // 0C81 1972 1027  cmpi.l  #$19721027,D1
#define MSHVSSF_USA_KEY                CRYPT_PARAMS( "a36d4971","cef51b28", "000000", "100000" )    // 0C81 1972 1027  cmpi.l  #$19721027,D1
#define MSHVSSF_JAPAN_KEY              CRYPT_PARAMS( "5dc391f8","a627e0b4", "000000", "100000" )    // 0C81 1972 1027  cmpi.l  #$19721027,D1
#define MSHVSSF_HISPANIC_KEY           CRYPT_PARAMS( "7e916fc4","03ab852d", "000000", "100000" )    // 0C81 1972 1027  cmpi.l  #$19721027,D1
#define MSHVSSF_ASIA_KEY               CRYPT_PARAMS( "52e3fa61","0c497bd8", "000000", "100000" )    // 0C81 1972 1027  cmpi.l  #$19721027,D1
#define MSHVSSF_BRAZIL_KEY             CRYPT_PARAMS( "da68c749","5bf3e201", "000000", "100000" )    // 0C81 1972 1027  cmpi.l  #$19721027,D1

#define SGEMF_USA_KEY                  CRYPT_PARAMS( "84234976","5e0fbb7e", "000000", "080000" )    // 0C80 1F74 0D12  cmpi.l  #$1F740D12,D0
#define SGEMF_JAPAN_KEY                CRYPT_PARAMS( "97d2ebc0","308f94d7", "000000", "080000" )    // 0C80 1F74 0D12  cmpi.l  #$1F740D12,D0
#define SGEMF_ASIA_KEY                 CRYPT_PARAMS( "090b412a","c47ee993", "000000", "080000" )    // 0C80 1F74 0D12  cmpi.l  #$1F740D12,D0
#define SGEMF_HISPANIC_KEY             CRYPT_PARAMS( "8163a71b","7c8fd224", "000000", "080000" )    // 0C80 1F74 0D12  cmpi.l  #$1F740D12,D0

#define VHUNT2_JAPAN_KEY               CRYPT_PARAMS( "36c1eba3","26b10f18", "000000", "100000" )    // 0C80 0692 0760  cmpi.l  #$06920760,D0

#define VSAV_WORLD_KEY                 CRYPT_PARAMS( "e0cd5881","71babb70", "000000", "100000" )    // 0C80 726A 4BAF  cmpi.l  #$726A4BAF,D0
#define VSAV_USA_KEY                   CRYPT_PARAMS( "a62ea0ee","573e03e6", "000000", "100000" )    // 0C80 726A 4BAF  cmpi.l  #$726A4BAF,D0
#define VSAV_JAPAN_KEY                 CRYPT_PARAMS( "fa8f4e33","a4b881b9", "000000", "100000" )    // 0C80 726A 4BAF  cmpi.l  #$726A4BAF,D0
#define VSAV_ASIA_KEY                  CRYPT_PARAMS( "47ee9930","90b412ac", "000000", "100000" )    // 0C80 726A 4BAF  cmpi.l  #$726A4BAF,D0
#define VSAV_HISPANIC_KEY              CRYPT_PARAMS( "b2d37c8d","d3b7aadd", "000000", "100000" )    // 0C80 726A 4BAF  cmpi.l  #$726A4BAF,D0

#define VSAV2_JAPAN_KEY                CRYPT_PARAMS( "d681e4f4","60371edf", "000000", "100000" )    // 0C80 0692 0760  cmpi.l  #$06920760,D0

#define MVSC_WORLD_KEY                 CRYPT_PARAMS( "48025ade","1c697b27", "000000", "100000" )    // 0C81 1972 0121  cmpi.l  #$19720121,D1
#define MVSC_USA_KEY                   CRYPT_PARAMS( "692dc41b","7ef1c805", "000000", "100000" )    // 0C81 1972 0121  cmpi.l  #$19720121,D1
#define MVSC_JAPAN_KEY                 CRYPT_PARAMS( "afc16138","97123eb0", "000000", "100000" )    // 0C81 1972 0121  cmpi.l  #$19720121,D1
#define MVSC_ASIA_KEY                  CRYPT_PARAMS( "f248aec6","7905cd17", "000000", "100000" )    // 0C81 1972 0121  cmpi.l  #$19720121,D1
#define MVSC_HISPANIC_KEY              CRYPT_PARAMS( "9d5c7a23","e56b18ef", "000000", "100000" )    // 0C81 1972 0121  cmpi.l  #$19720121,D1
#define MVSC_BRAZIL_KEY                CRYPT_PARAMS( "0874d6eb","51c2b798", "000000", "100000" )    // 0C81 1972 0121  cmpi.l  #$19720121,D1

#define SFA3_WORLD_KEY                 CRYPT_PARAMS( "6abfc8e0","2780ddc1", "000000", "100000" )    // 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
#define SFA3_USA_KEY                   CRYPT_PARAMS( "e7bbf0e5","67943248", "000000", "100000" )    // 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
#define SFA3_HISPANIC_KEY              CRYPT_PARAMS( "8422df8c","7b17a361", "000000", "100000" )    // 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
#define SFA3_BRAZIL_KEY                CRYPT_PARAMS( "d421c0b2","8116d296", "000000", "100000" )    // 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
#define SFA3_JAPAN_KEY                 CRYPT_PARAMS( "7d49f803","0cbe2d79", "000000", "100000" )    // 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0
#define SFA3_ASIA_KEY                  CRYPT_PARAMS( "990b9301","a4e42c7e", "000000", "100000" )    // 0C80 1C62 F5A8  cmpi.l  #$1C62F5A8,D0

#define JYANGOKU_JAPAN_KEY             CRYPT_PARAMS( "6ca42ae6","92f63f59", "000000", "400000" )    // 0C80 3652 1573  cmpi.l  #$36521573,D0 // range unknown

#define HSF2_USA_KEY                   CRYPT_PARAMS( "5a369ddd","fea3189c", "000000", "100000" )    // 0838 0007 2000  btst    #7,$2000
#define HSF2_ASIA_KEY                  CRYPT_PARAMS( "b8ed3630","aae30a3d", "000000", "100000" )    // 0838 0007 2000  btst    #7,$2000
#define HSF2_JAPAN_KEY                 CRYPT_PARAMS( "65d82fe0","dbb83e47", "000000", "100000" )    // 0838 0007 2000  btst    #7,$2000

#define GIGAWING_USA_KEY               CRYPT_PARAMS( "e52de290","1b60d780", "000000", "100000" )    // 0C81 1972 1027  cmpi.l  #$19721027,D1
#define GIGAWING_JAPAN_KEY             CRYPT_PARAMS( "1126196a","bef50895", "000000", "100000" )    // 0C81 1972 1027  cmpi.l  #$19721027,D1
#define GIGAWING_ASIA_KEY              CRYPT_PARAMS( "3506a85a","66b1b768", "000000", "100000" )    // 0C81 1972 1027  cmpi.l  #$19721027,D1
#define GIGAWING_HISPANIC_KEY          CRYPT_PARAMS( "cea74211","400da385", "000000", "100000" )    // 0C81 1972 1027  cmpi.l  #$19721027,D1
#define GIGAWING_BRAZIL_KEY            CRYPT_PARAMS( "0fe745b0","96ef7f9d", "000000", "100000" )    // 0C81 1972 1027  cmpi.l  #$19721027,D1

#define MMATRIX_USA_KEY                CRYPT_PARAMS( "ac9ebd79","410467df", "000000", "180000" )    // B6C0 B447 BACF  cmpa.w  D0,A3   cmp.w   D7,D2   cmpa.w  A7,A5
#define MMATRIX_JAPAN_KEY              CRYPT_PARAMS( "4df81e95","72ed9823", "000000", "180000" )    // B6C0 B447 BACF  cmpa.w  D0,A3   cmp.w   D7,D2   cmpa.w  A7,A5

#define MPANG_ALLREGIONS_KEY           CRYPT_PARAMS( "95f741c6","e547a21b", "000000", "100000" )    // 0C84 347D 89A3  cmpi.l  #$347D89A3,D4

#define PZLOOP2_ALLREGIONS_KEY         CRYPT_PARAMS( "a054f812","c40d36b4", "000000", "400000" )    // 0C82 9A73 15F1  cmpi.l  #$9A7315F1,D2

#define CHOKO_JAPAN_KEY                CRYPT_PARAMS( "d3fb12c6","7f8e17b5", "000000", "400000" )    // 0C86 4D17 5B3C  cmpi.l  #$4D175B3C,D6

#define DIMAHOO_WORLD_KEY              CRYPT_PARAMS( "0ddb8e40","2817fd2b", "000000", "080000" )    // BE4C B244 B6C5  cmp.w   A4,D7   cmp.w   D4,D1   cmpa.w  D5,A3
#define DIMAHOO_USA_KEY                CRYPT_PARAMS( "6575af59","b0fea691", "000000", "080000" )    // BE4C B244 B6C5  cmp.w   A4,D7   cmp.w   D4,D1   cmpa.w  D5,A3
#define DIMAHOO_JAPAN_KEY              CRYPT_PARAMS( "97f7be58","6121eb62", "000000", "080000" )    // BE4C B244 B6C5  cmp.w   A4,D7   cmp.w   D4,D1   cmpa.w  D5,A3

#define _1944_USA_KEY                  CRYPT_PARAMS( "1d3e724c","8b59fc7a", "000000", "080000" )    // 0C86 7B5D 94F1  cmpi.l  #$7B5D94F1,D6
#define _1944_JAPAN_KEY                CRYPT_PARAMS( "23d79c3a","e18b2746", "000000", "080000" )    // 0C86 7B5D 94F1  cmpi.l  #$7B5D94F1,D6

#define PROGEAR_USA_KEY                CRYPT_PARAMS( "639ad8c6","ef130df3", "000000", "400000" )    // 0C81 63A1 B8D3  cmpi.l  #$63A1B8D3,D1
#define PROGEAR_JAPAN_KEY              CRYPT_PARAMS( "9f7edc56","39fb47be", "000000", "400000" )    // 0C81 63A1 B8D3  cmpi.l  #$63A1B8D3,D1
#define PROGEAR_ASIA_KEY               CRYPT_PARAMS( "658ab128","fddc9b5e", "000000", "400000" )    // 0C81 63A1 B8D3  cmpi.l  #$63A1B8D3,D1
