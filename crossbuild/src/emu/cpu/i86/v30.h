#define SetMD(x)		(I.MF = (x))	/* OB [19.07.99] Mode Flag V30 */

#define MD		(I.MF!=0)

#undef CompressFlags
#define CompressFlags() (WORD)(CF | (PF << 2) | (AF << 4) | (ZF << 6) \
				| (SF << 7) | (I.TF << 8) | (I.IF << 9) \
				| (I.DF << 10) | (OF << 11)| (MD << 15))

#undef ExpandFlags
#define ExpandFlags(f) \
{ \
	  I.CarryVal = (f) & 1; \
	  I.ParityVal = !((f) & 4); \
	  I.AuxVal = (f) & 16; \
	  I.ZeroVal = !((f) & 64); \
	  I.SignVal = (f) & 128 ? -1 : 0; \
	  I.TF = ((f) & 256) == 256; \
	  I.IF = ((f) & 512) == 512; \
	  I.DF = ((f) & 1024) == 1024; \
	  I.OverVal = (f) & 2048; \
	  I.MF = ((f) & 0x8000) == 0x8000; \
}
