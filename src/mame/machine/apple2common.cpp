// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    apple2common.cpp

    Apple II stuff shared between apple2/apple2e/apple2gs.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"

#include "apple2common.h"

// device type definition
DEFINE_DEVICE_TYPE(APPLE2_COMMON, apple2_common_device, "apple2com", "Apple II Common Components")

//-------------------------------------------------
//  apple2_common_device - constructor
//-------------------------------------------------

apple2_common_device::apple2_common_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, APPLE2_COMMON, tag, owner, clock),
	m_GScpu(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apple2_common_device::device_start()
{
	// precalculate joystick time constants
	m_x_calibration = attotime::from_nsec(10800).as_double();
	m_y_calibration = attotime::from_nsec(10800).as_double();
	m_joystick_x1_time = m_joystick_x2_time = m_joystick_y1_time = m_joystick_y2_time = 0.0;
}

//-------------------------------------------------
//  device_validity_check - device-specific validation
//-------------------------------------------------
void apple2_common_device::device_validity_check(validity_checker &valid) const
{
	if ((m_GScpu.finder_tag() != finder_base::DUMMY_TAG) && !m_GScpu)
	{
		osd_printf_error("IIgs CPU configured but not found.\n");
	}
}

struct dasm_data
{
	u16 addr;
	const char *name;
};

struct dasm_data32
{
	u32 addr;
	const char *name;
};

static const struct dasm_data p8_calls[] =
{
	{ 0x40, "ALLOC_INTERRUPT" }, { 0x41, "DEALLOC_INTERRUPT" },
	{ 0x65, "QUIT" }, { 0x80, "READ_BLOCK" }, { 0x81, "WRITE_BLOCK" }, { 0x82, "GET_TIME" },
	{ 0x99, "ATINIT" }, { 0xc0, "CREATE" }, { 0xc1, "DESTROY" }, { 0xc2, "RENAME" }, { 0xc3, "SET_FILE_INFO" },
	{ 0xc4, "GET_FILE_INFO" }, { 0xc5, "ONLINE" }, { 0xc6, "SET_PREFIX" }, { 0xc7, "GET_PREFIX" }, { 0xc8, "OPEN" },
	{ 0xc9, "NEWLINE" }, { 0xca, "READ" }, { 0xcb, "WRITE" }, { 0xcc, "CLOSE" }, { 0xcd, "FLUSH" }, { 0xce, "SET_MARK" },
	{ 0xcf, "GET_MARK" }, { 0xd0, "SET_EOF" }, { 0xd1, "GET_EOF" }, { 0xd2, "SET_BUF" }, { 0xd3, "GET_BUF" },

	{ 0xffff, "" }
};

static const struct dasm_data a2_stuff[] =
{
	{ 0x0020, "WNDLFT" }, { 0x0021, "WNDWDTH" }, { 0x0022, "WNDTOP" }, { 0x0023, "WNDBTM" },
	{ 0x0024, "CH" }, { 0x0025, "CV" }, { 0x0026, "GBASL" }, { 0x0027, "GBASH" },
	{ 0x0028, "BASL" }, { 0x0029, "BASH" }, { 0x002b, "BOOTSLOT" }, { 0x002c, "H2" },
	{ 0x002d, "V2" }, { 0x002e, "MASK" }, { 0x0030, "COLOR" }, { 0x0031, "MODE" },
	{ 0x0032, "INVFLG" }, { 0x0033, "PROMPT" }, { 0x0036, "CSWL" }, { 0x0037, "CSWH" },
	{ 0x0038, "KSWL" }, { 0x0039, "KSWH" }, { 0x0045, "ACC" }, { 0x0046, "XREG" },
	{ 0x0047, "YREG" }, { 0x0048, "STATUS" }, { 0x004E, "RNDL" }, { 0x004F, "RNDH" },
	{ 0x0067, "TXTTAB" }, { 0x0069, "VARTAB" }, { 0x006b, "ARYTAB" }, { 0x6d, "STREND" },
	{ 0x006f, "FRETOP" }, { 0x0071, "FRESPC" }, { 0x0073, "MEMSIZ" }, { 0x0075, "CURLIN" },
	{ 0x0077, "OLDLIN" }, { 0x0079, "OLDTEXT" }, { 0x007b, "DATLIN" }, { 0x007d, "DATPTR" },
	{ 0x007f, "INPTR" }, { 0x0081, "VARNAM" }, { 0x0083, "VARPNT" }, { 0x0085, "FORPNT" },
	{ 0x009A, "EXPON" }, { 0x009C, "EXPSGN" }, { 0x009d, "FAC" }, { 0x00A2, "FAC.SIGN" },
	{ 0x00a5, "ARG" }, { 0x00AA, "ARG.SIGN" }, { 0x00af, "PRGEND" }, { 0x00B8, "TXTPTR" },
	{ 0x00C9, "RNDSEED" }, { 0x00D6, "LOCK" }, { 0x00D8, "ERRFLG" }, { 0x00DA, "ERRLIN" },
	{ 0x00DE, "ERRNUM" }, { 0x00E4, "HGR.COLOR" }, { 0x00E6, "HGR.PAGE" }, { 0x00F1, "SPEEDZ" },

	{ 0xc000, "KBD / 80STOREOFF" }, { 0xc001, "80STOREON" }, { 0xc002, "RDMAINRAM" }, {0xc003, "RDCARDRAM" }, {0xc004, "WRMAINRAM" },
	{ 0xc005, "WRCARDRAM" }, { 0xc006, "SETSLOTCXROM" }, { 0xc007, "SETINTCXROM" }, { 0xc008, "SETSTDZP" },
	{ 0xc009, "SETALTZP "}, { 0xc00a, "SETINTC3ROM" }, { 0xc00b, "SETSLOTC3ROM" }, { 0xc00c, "CLR80VID" },
	{ 0xc00d, "SET80VID" }, { 0xc00e, "CLRALTCHAR" }, { 0xc00f, "SETALTCHAR" }, { 0xc010, "KBDSTRB" },
	{ 0xc011, "RDLCBNK2" }, { 0xc012, "RDLCRAM" }, { 0xc013, "RDRAMRD" }, { 0xc014, "RDRAMWRT" },
	{ 0xc015, "RDCXROM" }, { 0xc016, "RDALTZP" }, { 0xc017, "RDC3ROM" }, { 0xc018, "RD80STORE" },
	{ 0xc019, "RDVBL" }, { 0xc01a, "RDTEXT" }, { 0xc01b, "RDMIXED" }, { 0xc01c, "RDPAGE2" },
	{ 0xc01d, "RDHIRES" }, { 0xc01e, "RDALTCHAR" }, { 0xc01f, "RD80VID" }, { 0xc020, "TAPEOUT" },
	{ 0xc021, "MONOCOLOR" }, { 0xc022, "TBCOLOR" }, { 0xc023, "VGCINT" }, { 0xc024, "MOUSEDATA" },
	{ 0xc025, "KEYMODREG" }, { 0xc026, "DATAREG" }, { 0xc027, "KMSTATUS" }, { 0xc028, "ROMBANK" },
	{ 0xc029, "NEWVIDEO"}, { 0xc02b, "LANGSEL" }, { 0xc02c, "CHARROM" }, { 0xc02d, "SLOTROMSEL" },
	{ 0xc02e, "VERTCNT" }, { 0xc02f, "HORIZCNT" }, { 0xc030, "SPKR" }, { 0xc031, "DISKREG" },
	{ 0xc032, "SCANINT" }, { 0xc033, "CLOCKDATA" }, { 0xc034, "CLOCKCTL" }, { 0xc035, "SHADOW" },
	{ 0xc036, "FPIREG/CYAREG" }, { 0xc037, "BMAREG" }, { 0xc038, "SCCBREG" }, { 0xc039, "SCCAREG" },
	{ 0xc03a, "SCCBDATA" }, { 0xc03b, "SCCADATA" }, { 0xc03c, "SOUNDCTL" }, { 0xc03d, "SOUNDDATA" },
	{ 0xc03e, "SOUNDADRL" }, { 0xc03f, "SOUNDADRH" }, { 0xc040, "STROBE/RDXYMSK" }, { 0xc041, "RDVBLMSK" },
	{ 0xc042, "RDX0EDGE" }, { 0xc043, "RDY0EDGE" }, { 0xc044, "MMDELTAX" }, { 0xc045, "MMDELTAY" },
	{ 0xc046, "DIAGTYPE" }, { 0xc047, "CLRVBLINT" }, { 0xc048, "CLRXYINT" }, { 0xc04f, "EMUBYTE" },
	{ 0xc050, "TXTCLR" }, { 0xc051, "TXTSET" },
	{ 0xc052, "MIXCLR" }, { 0xc053, "MIXSET" }, { 0xc054, "TXTPAGE1" }, { 0xc055, "TXTPAGE2" },
	{ 0xc056, "LORES" }, { 0xc057, "HIRES" }, { 0xc058, "CLRAN0" }, { 0xc059, "SETAN0" },
	{ 0xc05a, "CLRAN1" }, { 0xc05b, "SETAN1" }, { 0xc05c, "CLRAN2" }, { 0xc05d, "SETAN2" },
	{ 0xc05e, "DHIRESON" }, { 0xc05f, "DHIRESOFF" }, { 0xc060, "TAPEIN" }, { 0xc061, "RDBTN0" },
	{ 0xc062, "BUTN1" }, { 0xc063, "RD63" }, { 0xc064, "PADDL0" }, { 0xc065, "PADDL1" },
	{ 0xc066, "PADDL2" }, { 0xc067, "PADDL3" }, { 0xc068, "STATEREG" }, { 0xc070, "PTRIG" }, { 0xc073, "BANKSEL" },
	{ 0xc07e, "IOUDISON" }, { 0xc07f, "IOUDISOFF" }, { 0xc081, "ROMIN" }, { 0xc083, "LCBANK2" },
	{ 0xc085, "ROMIN" }, { 0xc087, "LCBANK2" }, { 0xcfff, "DISCC8ROM" },

	{ 0xF800, "F8ROM:PLOT" }, { 0xF80E, "F8ROM:PLOT1" } , { 0xF819, "F8ROM:HLINE" }, { 0xF828, "F8ROM:VLINE" },
	{ 0xF832, "F8ROM:CLRSCR" }, { 0xF836, "F8ROM:CLRTOP" }, { 0xF838, "F8ROM:CLRSC2" }, { 0xF847, "F8ROM:GBASCALC" },
	{ 0xF856, "F8ROM:GBCALC" }, { 0xF85F, "F8ROM:NXTCOL" }, { 0xF864, "F8ROM:SETCOL" }, { 0xF871, "F8ROM:SCRN" },
	{ 0xF882, "F8ROM:INSDS1" }, { 0xF88E, "F8ROM:INSDS2" }, { 0xF8A5, "F8ROM:ERR" }, { 0xF8A9, "F8ROM:GETFMT" },
	{ 0xF8D0, "F8ROM:INSTDSP" }, { 0xF940, "F8ROM:PRNTYX" }, { 0xF941, "F8ROM:PRNTAX" }, { 0xF944, "F8ROM:PRNTX" },
	{ 0xF948, "F8ROM:PRBLNK" }, { 0xF94A, "F8ROM:PRBL2" },  { 0xF84C, "F8ROM:PRBL3" }, { 0xF953, "F8ROM:PCADJ" },
	{ 0xF854, "F8ROM:PCADJ2" }, { 0xF856, "F8ROM:PCADJ3" }, { 0xF85C, "F8ROM:PCADJ4" }, { 0xF962, "F8ROM:FMT1" },
	{ 0xF9A6, "F8ROM:FMT2" }, { 0xF9B4, "F8ROM:CHAR1" }, { 0xF9BA, "F8ROM:CHAR2" }, { 0xF9C0, "F8ROM:MNEML" },
	{ 0xFA00, "F8ROM:MNEMR" }, { 0xFA40, "F8ROM:OLDIRQ" }, { 0xFA4C, "F8ROM:BREAK" }, { 0xFA59, "F8ROM:OLDBRK" },
	{ 0xFA62, "F8ROM:RESET" }, { 0xFAA6, "F8ROM:PWRUP" }, { 0xFABA, "F8ROM:SLOOP" }, { 0xFAD7, "F8ROM:REGDSP" },
	{ 0xFADA, "F8ROM:RGDSP1" }, { 0xFAE4, "F8ROM:RDSP1" }, { 0xFB19, "F8ROM:RTBL" }, { 0xFB1E, "F8ROM:PREAD" },
	{ 0xFB21, "F8ROM:PREAD4" }, { 0xFB25, "F8ROM:PREAD2" }, { 0xFB2F, "F8ROM:INIT" }, { 0xFB39, "F8ROM:SETTXT" },
	{ 0xFB40, "F8ROM:SETGR" }, { 0xFB4B, "F8ROM:SETWND" }, { 0xFB51, "F8ROM:SETWND2" }, { 0xFB5B, "F8ROM:TABV" },
	{ 0xFB60, "F8ROM:APPLEII" }, { 0xFB6F, "F8ROM:SETPWRC" }, { 0xFB78, "F8ROM:VIDWAIT" }, { 0xFB88, "F8ROM:KBDWAIT" },
	{ 0xFBB3, "F8ROM:VERSION" }, { 0xFBBF, "F8ROM:ZIDBYTE2" }, { 0xFBC0, "F8ROM:ZIDBYTE" }, { 0xFBC1, "F8ROM:BASCALC" },
	{ 0xFBD0, "F8ROM:BSCLC2" }, { 0xFBDD, "F8ROM:BELL1" }, { 0xFBE2, "F8ROM:BELL1.2" }, { 0xFBE4, "F8ROM:BELL2" },
	{ 0xFBF0, "F8ROM:STORADV" }, { 0xFBF4, "F8ROM:ADVANCE" }, { 0xFBFD, "F8ROM:VIDOUT" }, { 0xFC10, "F8ROM:BS" },
	{ 0xFC1A, "F8ROM:UP" }, { 0xFC22, "F8ROM:VTAB" }, { 0xFC24, "F8ROM:VTABZ" }, { 0xFC42, "F8ROM:CLREOP" },
	{ 0xFC46, "F8ROM:CLEOP1" }, { 0xFC58, "F8ROM:HOME" }, { 0xFC62, "F8ROM:CR" }, { 0xFC66, "F8ROM:LF" },
	{ 0xFC70, "F8ROM:SCROLL" }, { 0xFC95, "F8ROM:SCRL3" }, { 0xFC9C, "F8ROM:CLREOL" }, { 0xFC9E, "F8ROM:CLREOLZ" },
	{ 0xFCA8, "F8ROM:WAIT" }, { 0xFCB4, "F8ROM:NXTA4" }, { 0xFCBA, "F8ROM:NXTA1" }, { 0xFCC9, "F8ROM:HEADR" },
	{ 0xFCEC, "F8ROM:RDBYTE" }, { 0xFCEE, "F8ROM:RDBYT2" }, { 0xFCFA, "F8ROM:RD2BIT" }, { 0xFD0C, "F8ROM:RDKEY" },
	{ 0xFD18, "F8ROM:RDKEY1" }, { 0xFD1B, "F8ROM:KEYIN" }, { 0xFD2F, "F8ROM:ESC" }, { 0xFD35, "F8ROM:RDCHAR" },
	{ 0xFD3D, "F8ROM:NOTCR" }, { 0xFD62, "F8ROM:CANCEL" }, { 0xFD67, "F8ROM:GETLNZ" }, { 0xFD6A, "F8ROM:GETLN" },
	{ 0xFD6C, "F8ROM:GETLN0" }, { 0xFD6F, "F8ROM:GETLN1" }, { 0xFD8B, "F8ROM:CROUT1" }, { 0xFD8E, "F8ROM:CROUT" },
	{ 0xFD92, "F8ROM:PRA1" }, { 0xFDA3, "F8ROM:XAM8" }, { 0xFDDA, "F8ROM:PRBYTE" }, { 0xFDE3, "F8ROM:PRHEX" },
	{ 0xFDE5, "F8ROM:PRHEXZ" }, { 0xFDED, "F8ROM:COUT" }, { 0xFDF0, "F8ROM:COUT1" }, { 0xFDF6, "F8ROM:COUTZ" },
	{ 0xFE18, "F8ROM:SETMODE" }, { 0xFE1F, "F8ROM:IDROUTINE" }, { 0xFE20, "F8ROM:LT" }, { 0xFE22, "F8ROM:LT2" },
	{ 0xFE2C, "F8ROM:MOVE" }, { 0xFE36, "F8ROM:VFY" }, { 0xFE5E, "F8ROM:LIST" }, { 0xFE63, "F8ROM:LIST2" },
	{ 0xFE75, "F8ROM:A1PC" }, { 0xFE80, "F8ROM:SETINV" }, { 0xFE84, "F8ROM:SETNORM" }, { 0xFE89, "F8ROM:SETKBD" },
	{ 0xFE8B, "F8ROM:INPORT" }, { 0xFE8D, "F8ROM:INPRT" }, { 0xFE93, "F8ROM:SETVID" }, { 0xFE95, "F8ROM:OUTPORT" },
	{ 0xFE97, "F8ROM:OUTPRT" }, { 0xFEB0, "F8ROM:XBASIC" }, { 0xFEB3, "F8ROM:BASCONT" }, { 0xFEB6, "F8ROM:GO" },
	{ 0xFECA, "F8ROM:USR" }, { 0xFECD, "F8ROM:WRITE" }, { 0xFEFD, "F8ROM:READ" }, { 0xFF2D, "F8ROM:PRERR" },
	{ 0xFF3A, "F8ROM:BELL" }, { 0xFF3F, "F8ROM:RESTORE" }, { 0xFF4A, "F8ROM:SAVE" }, { 0xFF58, "F8ROM:IORTS" },
	{ 0xFF59, "F8ROM:OLDRST" }, { 0xFF65, "F8ROM:MON" }, { 0xFF69, "F8ROM:MONZ" }, { 0xFF6C, "F8ROM:MONZ2" },
	{ 0xFF70, "F8ROM:MONZ4" }, { 0xFF8A, "F8ROM:DIG" }, { 0xFFA7, "F8ROM:GETNUM" }, { 0xFFAD, "F8ROM:NXTCHR" },
	{ 0xFFBE, "F8ROM:TOSUB" }, { 0xFFC7, "F8ROM:ZMODE" }, { 0xFFCC, "F8ROM:CHRTBL" }, { 0xFFE3, "F8ROM:SUBTBL" },

	{ 0xffff, "" }
};

static const struct dasm_data gs_tools[] =
{
	{ 0x0101, "TLBootInit()" }, { 0x0201, "TLStartUp()" }, { 0x0301, "TLShutDown()" }, { 0x0401, "TLVersion():Vers" }, { 0x0501, "TLReset()" },
	{ 0x0601, "TLStatus():ActFlg" }, { 0x0901, "GetTSPtr(SysFlg,TS#):@FPT" }, { 0x0A01, "SetTSPtr(SysFlg,TS#,@FPT)" }, { 0x0B01, "GetFuncPtr(SysFlg,Func):@Func" },
	{ 0x0C01, "GetWAP(SysFlg,TS#):@WAP" }, { 0x0D01, "SetWAP(SysFlg,TS#,@WAP)" }, { 0x0E01, "LoadTools(@ToolTable)" }, { 0x0F01, "LoadOneTool(TS#,MinVers)" },
	{ 0x1001, "UnloadOneTool(TS#)" }, { 0x1101, "TLMountVolume(X,Y,@L1,@L2,@B1,@B2):Btn#" }, { 0x1201, "TLTextMountVolume(@L1,@L2,@B1,@B2):Btn#" },
	{ 0x1301, "SaveTextState():StateH" }, { 0x1401, "RestoreTextState(StateH)" }, { 0x1501, "MessageCenter(Action,Type,MsgH)" }, { 0x1601, "SetDefaultTPT()" },
	{ 0x1701, "MessageByName(CreateF,@inpRec):Created,Type" }, { 0x1801, "StartUpTools(MemID,ssDesc,ssRef/4):ssRef/4" }, { 0x1901, "ShutDownTools(ssDesc,ssRef/4)" },
	{ 0x1A01, "GetMsgHandle(Flags,MsgRef/4):H" }, { 0x1B01, "AcceptRequests(@NameStr,UserID,@ReqProc)" }, { 0x1C01, "SendRequest(ReqCode,How,Target/4,@In,@Out)" },
	{ 0x0102, "MMBootInit()" }, { 0x0202, "MMStartUp():MemID" }, { 0x0302, "MMShutDown(MemID)" }, { 0x0402, "MMVersion():Vers" }, { 0x0502, "MMReset()" },
	{ 0x0602, "MMStatus():ActFlg" }, { 0x0902, "NewHandle(Size/4,MemID,Attr,@loc):H" }, { 0x0A02, "ReAllocHandle(Size/4,MemID,Attr,@loc,H)" }, { 0x0B02, "RestoreHandle(H)" },
	{ 0x0C02, "AddToOOMQueue(@header)" }, { 0x0D02, "RemoveFromOOMQueue(@header)" }, { 0x1002, "DisposeHandle(H)" }, { 0x1102, "DisposeAll(MemID)" },
	{ 0x1202, "PurgeHandle(H)" }, { 0x1302, "PurgeAll(MemID)" }, { 0x1802, "GetHandleSize(H):Size/4" }, { 0x1902, "SetHandleSize(Size/4,H)" }, { 0x1A02, "FindHandle(@byte):H" },
	{ 0x1B02, "FreeMem():FreeBytes/4" }, { 0x1C02, "MaxBlock():Size/4" }, { 0x1D02, "TotalMem():Size/4" }, { 0x1E02, "CheckHandle(H)" }, { 0x1F02, "CompactMem()" },
	{ 0x2002, "HLock(H)" }, { 0x2102, "HLockAll(MemID)" }, { 0x2202, "HUnlock(H)" }, { 0x2302, "HUnlockAll(MemID)" }, { 0x2402, "SetPurge(PrgLvl,H)" },
	{ 0x2502, "SetPurgeAll(PrgLvl,MemID)" }, { 0x2802, "PtrToHand(@Src,DestH,Count/4)" }, { 0x2902, "HandToPtr(SrcH,@Dest,Count/4)" }, { 0x2A02, "HandToHand(SrcH,DestH,Count/4)" },
	{ 0x2B02, "BlockMove(@Source,@Dest,Count/4)" }, { 0x2F02, "RealFreeMem():Size/4" }, { 0x3002, "SetHandleID(newMemID,theH):oldMemID" }, { 0x0103, "MTBootInit()" },
	{ 0x0203, "MTStartUp()" }, { 0x0303, "MTShutDown()" }, { 0x0403, "MTVersion():Vers" }, { 0x0503, "MTReset()" }, { 0x0603, "MTStatus():ActFlg" },
	{ 0x0903, "WriteBRam(@Buff)" }, { 0x0A03, "ReadBRam(@Buff)" }, { 0x0B03, "WriteBParam(Data,Parm#)" }, { 0x0C03, "ReadBParam(Parm#):Data" }, { 0x0D03, "ReadTimeHex():WkDay,Mn&Dy,Yr&Hr,Mn&Sec" },
	{ 0x0E03, "WriteTimeHex(Mn&Dy,Yr&Hr,Mn&Sec)" }, { 0x0F03, "ReadAsciiTime(@Buff)" }, { 0x1003, "SetVector(Vec#,@x)" }, { 0x1103, "GetVector(Vec#):@x" },
	{ 0x1203, "SetHeartBeat(@Task)" }, { 0x1303, "DelHeartBeat(@Task)" }, { 0x1403, "ClrHeartBeat()" }, { 0x1503, "SysFailMgr(Code,@Msg)" }, { 0x1603, "GetAddr(Ref#):@Parm" },
	{ 0x1703, "ReadMouse():X,Y,Stat&Mode" }, { 0x1803, "InitMouse(Slot)" }, { 0x1903, "SetMouse(Mode)" }, { 0x1A03, "HomeMouse()" }, { 0x1B03, "ClearMouse()" },
	{ 0x1C03, "ClampMouse(Xmn,Xmx,Ymn,Ymx)" }, { 0x1D03, "GetMouseClamp():Xmn,Xmx,Ymn,Ymx" }, { 0x1E03, "PosMouse(X,Y)" }, { 0x1F03, "ServeMouse():IntStat" },
	{ 0x2003, "GetNewID(Kind):MemID" }, { 0x2103, "DeleteID(MemID)" }, { 0x2203, "StatusID(MemID)" }, { 0x2303, "IntSource(Ref#)" }, { 0x2403, "FWEntry(A,X,Y,Address):P,A,X,Y" },
	{ 0x2503, "GetTick():Ticks/4" }, { 0x2603, "PackBytes(@StartPtr,@Sz,@OutBf,OutSz):Size" }, { 0x2703, "UnPackBytes(@Buff,BfSz,@StartPtr,@Sz):Size" },
	{ 0x2803, "Munger(@Dst,@DstL,@t,tL,@Rpl,RplL,@Pad):N" }, { 0x2903, "GetIRQEnable():IntStat" }, { 0x2A03, "SetAbsClamp(Xmn,Xmx,Ymn,Ymx)" },
	{ 0x2B03, "GetAbsClamp():Xmn,Xmx,Ymn,Ymx" }, { 0x2C03, "SysBeep()" }, { 0x2E03, "AddToQueue(@newTask,@queueHeader)" }, { 0x2F03, "DeleteFromQueue(@task,@queueHeader)" },
	{ 0x3003, "SetInterruptState(@stateRec,NumBytes)" }, { 0x3103, "GetInterruptState(@stateRec,NumBytes)" }, { 0x3203, "GetIntStateRecSize():Size" },
	{ 0x3303, "ReadMouse2():xPos,yPos,StatMode" }, { 0x3403, "GetCodeResConverter():@proc" }, { 0x3503, "GetROMResource(?,?/4):???H" },
	{ 0x3603, "ReleaseROMResource(?,?/4)" }, { 0x3703, "ConvSeconds(convVerb,Secs/4,@Date):SecondsOut/4" }, { 0x3803, "SysBeep2(beepKind)" },
	{ 0x3903, "VersionString(flags,Version/4,@Buffer)" }, { 0x3A03, "WaitUntil(WaitFromTime,DelayTime):NewTime" }, { 0x3B03, "StringToText(flags,@String,StrLen,@Buffer):ResFlags,PrntLen" },
	{ 0x3C03, "ShowBootInfo(@String,@Icon)" }, { 0x3D03, "ScanDevices():DevNum" }, { 0x3E03, "AlertMessage(@Table,MsgNum,@Subs):Button" }, { 0x3F03, "DoSysPrefs(bitsToClear,bitsToSet):SysPrefs" },
	{ 0x0104, "QDBootInit()" }, { 0x0204, "QDStartUp(DirPg,MastSCB,MaxWid,MemID)" }, { 0x0304, "QDShutDown()" }, { 0x0404, "QDVersion():Vers" }, { 0x0504, "QDReset()" },
	{ 0x0604, "QDStatus():ActFlg" }, { 0x0904, "GetAddress(what):@Table" }, { 0x0A04, "GrafOn()" }, { 0x0B04, "GrafOff()" }, { 0x0C04, "GetStandardSCB():SCB" },
	{ 0x0D04, "InitColorTable(@Table)" }, { 0x0E04, "SetColorTable(Tab#,@SrcTab)" }, { 0x0F04, "GetColorTable(Tab#,@DestTbl)" }, { 0x1004, "SetColorEntry(Tab#,Ent#,NewCol)" },
	{ 0x1104, "GetColorEntry(Tab#,Ent#):Color" }, { 0x1204, "SetSCB(Line#,SCB)" }, { 0x1304, "GetSCB(Line#):SCB" }, { 0x1404, "SetAllSCBs(SCB)" }, { 0x1504, "ClearScreen(Color)" },
	{ 0x1604, "SetMasterSCB(SCB)" }, { 0x1704, "GetMasterSCB():SCB" }, { 0x1804, "OpenPort(@Port)" }, { 0x1904, "InitPort(@Port)" }, { 0x1A04, "ClosePort(@Port)" },
	{ 0x1B04, "SetPort(@Port)" }, { 0x1C04, "GetPort():@Port" }, { 0x1D04, "SetPortLoc(@LocInfo)" }, { 0x1E04, "GetPortLoc(@LocInfo)" }, { 0x1F04, "SetPortRect(@Rect)" },
	{ 0x2004, "GetPortRect(@Rect)" }, { 0x2104, "SetPortSize(w,h)" }, { 0x2204, "MovePortTo(h,v)" }, { 0x2304, "SetOrigin(h,v)" }, { 0x2404, "SetClip(RgnH)" },
	{ 0x2504, "GetClip(RgnH)" }, { 0x2604, "ClipRect(@Rect)" }, { 0x2704, "HidePen()" }, { 0x2804, "ShowPen()" }, { 0x2904, "GetPen(@Pt)" }, { 0x2A04, "SetPenState(@PenSt)" },
	{ 0x2B04, "GetPenState(@PenSt)" }, { 0x2C04, "SetPenSize(w,h)" }, { 0x2D04, "GetPenSize(@Pt)" }, { 0x2E04, "SetPenMode(Mode)" }, { 0x2F04, "GetPenMode():Mode" }, { 0x3004, "SetPenPat(@Patt)" },
	{ 0x3104, "GetPenPat(@Patt)" }, { 0x3204, "SetPenMask(@Mask)" }, { 0x3304, "GetPenMask(@Mask)" }, { 0x3404, "SetBackPat(@Patt)" }, { 0x3504, "GetBackPat(@Patt)" },
	{ 0x3604, "PenNormal()" }, { 0x3704, "SetSolidPenPat(Color)" }, { 0x3804, "SetSolidBackPat(Color)" }, { 0x3904, "SolidPattern(Color,@Patt)" }, { 0x3A04, "MoveTo(h,v)" },
	{ 0x3B04, "Move(dh,dv)" },  { 0x3C04, "LineTo(h,v)" }, { 0x3D04, "Line(dh,dv)" }, { 0x3E04, "SetPicSave(Val/4)" }, { 0x3F04, "GetPicSave():Val/4" },{ 0x4004, "SetRgnSave(Val/4)" },
	{ 0x4104, "GetRgnSave():Val/4" },  { 0x4204, "SetPolySave(Val/4)" }, { 0x4304, "GetPolySave():Val/4" }, { 0x4404, "SetGrafProcs(@GrafProcs)" },{ 0x4504, "GetGrafProcs():@GrafProcs" },
	{ 0x4604, "SetUserField(Val/4)" }, { 0x4704, "GetUserField():Val/4" }, { 0x4804, "SetSysField(Val/4)" }, { 0x4904, "GetSysField():Val/4" }, { 0x4A04, "SetRect(@Rect,left,top,right,bot)" },
	{ 0x4B04, "OffsetRect(@Rect,dh,dv)" }, { 0x4C04, "InsetRect(@Rect,dh,dv)" }, { 0x4D04, "SectRect(@R1,@R2,@DstR):nonEmptyF" }, { 0x4E04, "UnionRect(@Rect1,@Rect2,@UnionRect)" },
	{ 0x4F04, "PtInRect(@Pt,@Rect):Flag" }, { 0x5004, "Pt2Rect(@Pt1,@Pt2,@Rect)" }, { 0x5104, "EqualRect(@Rect1,@Rect2):Flag" }, { 0x5204, "NotEmptyRect(@Rect):Flag" },
	{ 0x5304, "FrameRect(@Rect)" }, { 0x5404, "PaintRect(@Rect)" }, { 0x5504, "EraseRect(@Rect)" }, { 0x5604, "InvertRect(@Rect)" }, { 0x5704, "FillRect(@Rect,@Patt)" },
	{ 0x5804, "FrameOval(@Rect)" }, { 0x5904, "PaintOval(@Rect)" }, { 0x5A04, "EraseOval(@Rect)" }, { 0x5B04, "InvertOval(@Rect)" }, { 0x5C04, "FillOval(@Rect,@Patt)" },
	{ 0x5D04, "FrameRRect(@Rect,OvalW,OvalHt)" }, { 0x5E04, "PaintRRect(@Rect,OvalW,OvalHt)" }, { 0x5F04, "EraseRRect(@Rect,OvalW,OvalHt)" }, { 0x6004, "InvertRRect(@Rect,OvalW,OvalHt)" },
	{ 0x6104, "FillRRect(@Rect,OvalW,OvalHt,@Patt)" }, { 0x6204, "FrameArc(@Rect,Ang1,ArcAng)" }, { 0x6304, "PaintArc(@Rect,Ang1,ArcAng)" }, { 0x6404, "EraseArc(@Rect,Ang1,ArcAng)" },
	{ 0x6504, "InvertArc(@Rect,Ang1,ArcAng)" }, { 0x6604, "FillArc(@Rect,Ang1,ArcAng,@Patt)" }, { 0x6704, "NewRgn():RgnH" }, { 0x6804, "DisposeRgn(RgnH)" },
	{ 0x6904, "CopyRgn(SrcRgnH,DestRgnH)" }, { 0x6A04, "SetEmptyRgn(RgnH)" }, { 0x6B04, "SetRectRgn(RgnH,left,top,right,bot)" }, { 0x6C04, "RectRgn(RgnH,@Rect)" },
	{ 0x6D04, "OpenRgn()" }, { 0x6E04, "CloseRgn(RgnH)" }, { 0x6F04, "OffsetRgn(RgnH,dh,dv)" }, { 0x7004, "InsetRgn(RgnH,dh,dv)" }, { 0x7104, "SectRgn(Rgn1H,Rgn2H,DstRgnH)" },
	{ 0x7204, "UnionRgn(Rgn1H,Rgn2H,UnionRgnH)" }, { 0x7304, "DiffRgn(Rgn1H,Rgn2H,DstRgnH)" }, { 0x7404, "XorRgn(Rgn1H,Rgn2H,DstRgnH)" }, { 0x7504, "PtInRgn(@Pt,RgnH):Flag" },
	{ 0x7604, "RectInRgn(@Rect,RgnH):Flag" }, { 0x7704, "EqualRgn(Rgn1H,Rgn2H):Flag" }, { 0x7804, "EmptyRgn(RgnH):Flag" }, { 0x7904, "FrameRgn(RgnH)" },
	{ 0x7A04, "PaintRgn(RgnH)" }, { 0x7B04, "EraseRgn(RgnH)" }, { 0x7C04, "InvertRgn(RgnH)" }, { 0x7D04, "FillRgn(RgnH,@Patt)" }, { 0x7E04, "ScrollRect(@Rect,dh,dv,UpdtRgnH)" },
	{ 0x7F04, "PaintPixels(@ppParms)" }, { 0x8004, "AddPt(@SrcPt,@DestPt)" }, { 0x8104, "SubPt(@SrcPt,@DstPt)" }, { 0x8204, "SetPt(@Pt,h,v)" }, { 0x8304, "EqualPt(@Pt1,@Pt2):Flag" },
	{ 0x8404, "LocalToGlobal(@Pt)" }, { 0x8504, "GlobalToLocal(@Pt)" }, { 0x8604, "Random():N" }, { 0x8704, "SetRandSeed(Seed/4)" }, { 0x8804, "GetPixel(Hor,Vert):Pixel" },
	{ 0x8904, "ScalePt(@Pt,@SrcRect,@DstRect)" }, { 0x8A04, "MapPt(@Pt,@SrcRect,@DstRect)" }, { 0x8B04, "MapRect(@Rect,@SrcRect,@DstRect)" }, { 0x8C04, "MapRgn(MapRgnH,@SrcRect,@DstRect)" },
	{ 0x8D04, "SetStdProcs(@StdProcRec)" }, { 0x8E04, "SetCursor(@Curs)" }, { 0x8F04, "GetCursorAdr():@Curs" }, { 0x9004, "HideCursor()" }, { 0x9104, "ShowCursor()" },
	{ 0x9204, "ObscureCursor()" }, { 0x9304, "SetMouseLoc ???" }, { 0x9404, "SetFont(FontH)" }, { 0x9504, "GetFont():FontH" }, { 0x9604, "GetFontInfo(@InfoRec)" },
	{ 0x9704, "GetFontGlobals(@FGRec)" }, { 0x9804, "SetFontFlags(Flags)" }, { 0x9904, "GetFontFlags():Flags" }, { 0x9A04, "SetTextFace(TextF)" }, { 0x9B04, "GetTextFace():TextF" },
	{ 0x9C04, "SetTextMode(TextM)" }, { 0x9D04, "GetTextMode():TextM" }, { 0x9E04, "SetSpaceExtra(SpEx/4f)" }, { 0x9F04, "GetSpaceExtra():SpEx/4f" }, { 0xA004, "SetForeColor(Color)" },
	{ 0xA104, "GetForeColor():Color" }, { 0xA204, "SetBackColor(BackCol)" }, { 0xA304, "GetBackColor():BackCol" }, { 0xA404, "DrawChar(Char)" }, { 0xA504, "DrawString(@Str)" },
	{ 0xA604, "DrawCString(@cStr)" }, { 0xA704, "DrawText(@Text,Len)" }, { 0xA804, "CharWidth(Char):Width" }, { 0xA904, "StringWidth(@Str):Width" }, { 0xAA04, "CStringWidth(@cStr):Width" },
	{ 0xAB04, "TextWidth(@Text,Len):Width" }, { 0xAC04, "CharBounds(Char,@Rect)" }, { 0xAD04, "StringBounds(@Str,@Rect)" }, { 0xAE04, "CStringBounds(@cStr,@Rect)" },
	{ 0xAF04, "TextBounds(@Text,Len,@Rect)" }, { 0xB004, "SetArcRot(ArcRot)" }, { 0xB104, "GetArcRot():ArcRot" }, { 0xB204, "SetSysFont(FontH)" }, { 0xB304, "GetSysFont():FontH" },
	{ 0xB404, "SetVisRgn(RgnH)" }, { 0xB504, "GetVisRgn(RgnH)" }, { 0xB604, "SetIntUse(Flag)" }, { 0xB704, "OpenPicture(@FrameRect):PicH" }, { 0xB804, "PicComment(Kind,DataSz,DataH)" },
	{ 0xB904, "ClosePicture()" }, { 0xBA04, "DrawPicture(PicH,@DstRect)" }, { 0xBB04, "KillPicture(PicH)" }, { 0xBC04, "FramePoly(PolyH)" }, { 0xBD04, "PaintPoly(PolyH)" },
	{ 0xBE04, "ErasePoly(PolyH)" }, { 0xBF04, "InvertPoly(PolyH)" }, { 0xC004, "FillPoly(PolyH,@Patt)" }, { 0xC104, "OpenPoly():PolyH" }, { 0xC204, "ClosePoly()" },
	{ 0xC304, "KillPoly(PolyH)" }, { 0xC404, "OffsetPoly(PolyH,dh,dv)" }, { 0xC504, "MapPoly(PolyH,@SrcRect,@DstRect)" }, { 0xC604, "SetClipHandle(RgnH)" },
	{ 0xC704, "GetClipHandle():RgnH" }, { 0xC804, "SetVisHandle(RgnH)" }, { 0xC904, "GetVisHandle():RgnH" }, { 0xCA04, "InitCursor()" }, { 0xCB04, "SetBufDims(MaxW,MaxFontHt,MaxFBRext)" },
	{ 0xCC04, "ForceBufDims(MaxW,MaxFontHt,MaxFBRext)" }, { 0xCD04, "SaveBufDims(@SizeInfo)" }, { 0xCE04, "RestoreBufDims(@SizeInfo)" }, { 0xCF04, "GetFGSize():FGSize" },
	{ 0xD004, "SetFontID(FontID/4)" }, { 0xD104, "GetFontID():FontID/4" }, { 0xD204, "SetTextSize(TextSz)" }, { 0xD304, "GetTextSize():TextSz" }, { 0xD404, "SetCharExtra(ChEx/4f)" },
	{ 0xD504, "GetCharExtra():ChEx/4f" }, { 0xD604, "PPToPort(@SrcLoc,@SrcRect,X,Y,Mode)" }, { 0xD704, "InflateTextBuffer(NewW,NewHt)" }, { 0xD804, "GetRomFont(@Rec)" },
	{ 0xD904, "GetFontLore(@Rec,RecSize):Size" }, { 0xDA04, "Get640Colors():@PattTable" }, { 0xDB04, "Set640Color(color)" }, { 0x0105, "DeskBootInit()" },
	{ 0x0205, "DeskStartUp()" }, { 0x0305, "DeskShutDown()" }, { 0x0405, "DeskVersion():Vers" }, { 0x0505, "DeskReset()" }, { 0x0605, "DeskStatus():ActFlg" },
	{ 0x0905, "SaveScrn()" }, { 0x0A05, "RestScrn()" }, { 0x0B05, "SaveAll()" }, { 0x0C05, "RestAll()" }, { 0x0E05, "InstallNDA(ndaH)" }, { 0x0F05, "InstallCDA(cdaH)" },
	{ 0x1105, "ChooseCDA()" }, { 0x1305, "SetDAStrPtr(AltDispH,@StrTbl)" }, { 0x1405, "GetDAStrPtr():@StrTbl" }, { 0x1505, "OpenNDA(ItemID):Ref#" }, { 0x1605, "CloseNDA(Ref#)" },
	{ 0x1705, "SystemClick(@EvRec,@Wind,fwRes)" }, { 0x1805, "SystemEdit(eType):Flag" }, { 0x1905, "SystemTask()" }, { 0x1A05, "SystemEvent(Mods,Where/4,When/4,Msg/4,What):F" },
	{ 0x1B05, "GetNumNDAs():N" }, { 0x1C05, "CloseNDAbyWinPtr(@Wind)" }, { 0x1D05, "CloseAllNDAs()" }, { 0x1E05, "FixAppleMenu(MenuID)" }, { 0x1F05, "AddToRunQ(@taskHeader)" },
	{ 0x2005, "RemoveFromRunQ(@taskHeader)" }, { 0x2105, "RemoveCDA(cdaH)" }, { 0x2205, "RemoveNDA(ndaH)" }, { 0x2305, "GetDeskAccInfo(flags,daRef/4,BufSize,@Buffer)" },
	{ 0x2405, "CallDeskAcc(flags,daRef/4,Action,Data/4):Result" }, { 0x2505, "GetDeskGlobal(selector):Value/4" }, { 0x0106, "EMBootInit()" }, { 0x0206, "EMStartUp(DirPg,qSz,Xmn,Xmx,Ymn,Ymx,MemID)" },
	{ 0x0306, "EMShutDown()" }, { 0x0406, "EMVersion():Vers" }, { 0x0506, "EMReset()" }, { 0x0606, "EMStatus():ActFlg" }, { 0x0906, "DoWindows():DirPg" },{ 0x0A06, "GetNextEvent(evMask,@EvRec):Flag" },
	{ 0x0B06, "EventAvail(evMask,@EvRec):Flag" }, { 0x0C06, "GetMouse(@Pt)" }, { 0x0D06, "Button(Btn#):DownFlg" }, { 0x0E06, "StillDown(Btn#):Flag" },
	{ 0x0F06, "WaitMouseUp(Btn#):Flag" }, { 0x1006, "TickCount():Ticks/4" }, { 0x1106, "GetDblTime():Ticks/4" }, { 0x1206, "GetCaretTime():Ticks/4" },
	{ 0x1306, "SetSwitch()" }, { 0x1406, "PostEvent(code,Msg/4):Flag" }, { 0x1506, "FlushEvents(evMask,StopMask):F" }, { 0x1606, "GetOSEvent(evMask,@EvRec):Flag" },
	{ 0x1706, "OSEventAvail(evMask,@EvRec):Flag" }, { 0x1806, "SetEventMask(evMask)" }, { 0x1906, "FakeMouse(ChFlg,Mods,X,Y,BtnStat)" }, { 0x1A06, "SetAutoKeyLimit(NewLimit)" },
	{ 0x1B06, "GetKeyTranslation():kTransID" }, { 0x1C06, "SetKeyTranslation(kTransID)" }, { 0x0107, "SchBootInit()" }, { 0x0207, "SchStartUp()" }, { 0x0307, "SchShutDown()" },
	{ 0x0407, "SchVersion():Vers" }, { 0x0507, "SchReset()" }, { 0x0607, "SchStatus():ActFlg" }, { 0x0907, "SchAddTask(@Task):Flag" }, { 0x0A07, "SchFlush()" },
	{ 0x0108, "SoundBootInit()" }, { 0x0208, "SoundStartUp(DirPg)" }, { 0x0308, "SoundShutDown()" }, { 0x0408, "SoundVersion():Vers" }, { 0x0508, "SoundReset()" },
	{ 0x0608, "SoundToolStatus():ActFlg" }, { 0x0908, "WriteRamBlock(@Src,DOCStart,Count)" }, { 0x0A08, "ReadRamBlock(@Dest,DOCStart,Count)" }, { 0x0B08, "GetTableAddress():@JumpTbl" },
	{ 0x0C08, "GetSoundVolume(Gen#):Vol" }, { 0x0D08, "SetSoundVolume(Vol,Gen#)" }, { 0x0E08, "FFStartSound(GenN&mode,@Parms)" }, { 0x0F08, "FFStopSound(GenMask)" },
	{ 0x1008, "FFSoundStatus():ActFlg" }, { 0x1108, "FFGeneratorStatus(Gen#):Stat" }, { 0x1208, "SetSoundMIRQV(@IntHandler)" }, { 0x1308, "SetUserSoundIRQV(@NewIRQ):@OldIRQ" },
	{ 0x1408, "FFSoundDoneStatus(Gen#):Stat" }, { 0x1508, "FFSetUpSound(ChannelGen,@Parms)" }, { 0x1608, "FFStartPlaying(GenWord)" }, { 0x1708, "SetDocReg(@DocRegParms)" },
	{ 0x1808, "ReadDocReg(@DocRegParms)" }, { 0x0109, "ADBBootInit()" }, { 0x0209, "ADBStartUp()" }, { 0x0309, "ADBShutDown()" }, { 0x0409, "ADBVersion():Vers" },
	{ 0x0509, "ADBReset()" }, { 0x0609, "ADBStatus():ActFlg" }, { 0x0909, "SendInfo(NumB,@Data,Cmd)" }, { 0x0A09, "ReadKeyMicroData(NumB,@Data,Cmd)" }, { 0x0B09, "ReadKeyMicroMemory(@DataOut,@DataIn,Cmd)" },
	{ 0x0C09, "[resynch--don't call]" }, { 0x0D09, "AsyncADBReceive(@CompVec,Cmd)" }, { 0x0E09, "SyncADBReceive(InputWrd,@CompVec,Cmd)" }, { 0x0F09, "AbsOn()" },
	{ 0x1009, "AbsOff()" }, { 0x1109, "RdAbs():Flag" }, { 0x1209, "SetAbsScale(@DataOut)" }, { 0x1309, "GetAbsScale(@DataIn)" }, { 0x1409, "SRQPoll(@CompVec,ADBreg)" },
	{ 0x1509, "SRQRemove(ADBreg)" }, { 0x1609, "ClearSRQTable()" }, { 0xFF09, "[OBSOLETE: Use 09FF]" }, { 0x010A, "SANEBootInit()" }, { 0x020A, "SANEStartUp(DirPg)" },
	{ 0x030A, "SANEShutDown()" }, { 0x040A, "SANEVersion():Vers" }, { 0x050A, "SANEReset()" }, { 0x060A, "SANEStatus():ActFlg" }, { 0x090A, "FPNum (...)" },
	{ 0x0A0A, "DecStrNum (...)" }, { 0x0B0A, "ElemNum (...)" }, { 0xFF0A, "[OBSOLETE: USE $0AFF]" }, { 0x010B, "IMBootInit()" }, { 0x020B, "IMStartUp()" },
	{ 0x030B, "IMShutDown()" }, { 0x040B, "IMVersion():Vers" }, { 0x050B, "IMReset()" }, { 0x060B, "IMStatus():ActFlg" }, { 0x090B, "Multiply(A,B):Prod/4" },
	{ 0x0A0B, "SDivide(Num,Den):Rem,Quot" }, { 0x0B0B, "UDivide(Num,Den):Rem,Quot" }, { 0x0C0B, "LongMul(A/4,B/4):Prod/8" }, { 0x0D0B, "LongDivide(Num/4,Denom/4):Rem/4,Quot/4" },
	{ 0x0E0B, "FixRatio(Numer,Denom):fxRatio/4" }, { 0x0F0B, "FixMul(fx1/4,fx2/4):fxProd/4" }, { 0x100B, "FracMul(fr1/4,fr2/4):frRes/4" }, { 0x110B, "FixDiv(Quot/4,Divisor/4):fxRes/4" },
	{ 0x120B, "FracDiv(Quot/4,Divisor/4):frRes/4" }, { 0x130B, "FixRound(fxVal/4):Int" }, { 0x140B, "FracSqrt(frVal/4):frRes/4" }, { 0x150B, "FracCos(fxAngle/4):frRes/4" },
	{ 0x160B, "FracSin(fxAngle/4):frRes/4" }, { 0x170B, "FixATan2(In1/4,In2/4):fxArcTan/4" }, { 0x180B, "HiWord(Long/4):Int" }, { 0x190B, "LoWord(Long/4):Int" },
	{ 0x1A0B, "Long2Fix(Long/4):fxRes/4" }, { 0x1B0B, "Fix2Long(Fix/4):Long/4" }, { 0x1C0B, "Fix2Frac(fxVal/4):Frac/4" }, { 0x1D0B, "Frac2Fix(frVal/4):fxRes/4" },
	{ 0x1E0B, "Fix2X(Fix/4,@Extended)" }, { 0x1F0B, "Frac2X(frVal/4,@Extended)" }, { 0x200B, "X2Fix(@Extended):fxRes/4" }, { 0x210B, "X2Frac(@Extended):frRes/4" },
	{ 0x220B, "Int2Hex(Int,@Str,Len)" }, { 0x230B, "Long2Hex(Long/4,@Str,Len)" }, { 0x240B, "Hex2Int(@Str,Len):Int" }, { 0x250B, "Hex2Long(@Str,Len):Long/4" },
	{ 0x260B, "Int2Dec(Int,@Str,Len,SgnFlg)" }, { 0x270B, "Long2Dec(Long/4,@Str,Len,SgnFlg)" }, { 0x280B, "Dec2Int(@Str,Len,SgnFlg):Int" }, { 0x290B, "Dec2Long(@Str,Len,SgnFlg):Long/4" },
	{ 0x2A0B, "HexIt(Int):Hex/4" }, { 0x010C, "TextBootInit()" }, { 0x020C, "TextStartUp()" }, { 0x030C, "TextShutDown()" }, { 0x040C, "TextVersion():Vers" },
	{ 0x050C, "TextReset()" }, { 0x060C, "TextStatus():ActFlg" }, { 0x090C, "SetInGlobals(ANDmsk,ORmsk)" }, { 0x0A0C, "SetOutGlobals(ANDmsk,ORmsk)" },
	{ 0x0B0C, "SetErrGlobals(ANDmsk,ORmsk)" }, { 0x0C0C, "GetInGlobals():ANDmsk,ORmsk" }, { 0x0D0C, "GetOutGlobals():ANDmsk,ORmsk" }, { 0x0E0C, "GetErrGlobals():ANDmsk,ORmsk" },
	{ 0x0F0C, "SetInputDevice(Type,@drvr|Slot/4)" }, { 0x100C, "SetOutputDevice(Type,@drvr|Slot/4)" }, { 0x110C, "SetErrorDevice(Type,@drvr|Slot/4)" }, { 0x120C, "GetInputDevice():Type,@drvr|Slot/4" },
	{ 0x130C, "GetOutputDevice():Type,@drvr|Slot/4" }, { 0x140C, "GetErrorDevice():Type,@drvr|Slot/4" },{ 0x150C, "InitTextDev(dev)" }, { 0x160C, "CtlTextDev(dev,code)" },
	{ 0x170C, "StatusTextDev(dev,request)" }, { 0x180C, "WriteChar(Char)" }, { 0x190C, "ErrWriteChar(Char)" }, { 0x1A0C, "WriteLine(@Str)" }, { 0x1B0C, "ErrWriteLine(@Str)" },
	{ 0x1C0C, "WriteString(@Str)" }, { 0x1D0C, "ErrWriteString(@Str)" }, { 0x1E0C, "TextWriteBlock(@Text,Offset,Len)" }, { 0x1F0C, "ErrWriteBlock(@Text,Offset,Len)" },{ 0x200C, "WriteCString(@cStr)" },
	{ 0x210C, "ErrWriteCString(@cStr)" }, { 0x220C, "ReadChar(EchoFlg):Char" }, { 0x230C, "TextReadBlock(@Buff,Offset,Size,EchoFlg)" }, { 0x240C, "ReadLine(@Buff,Max,EOLch,EchoFlg):Count" },
	{ 0x010E, "WindBootInit()" }, { 0x020E, "WindStartUp(MemID)" }, { 0x030E, "WindShutDown()" }, { 0x040E, "WindVersion():Vers" }, { 0x050E, "WindReset()" },{ 0x060E, "WindStatus():ActFlg" },
	{ 0x090E, "NewWindow(@Parms):@Wind" }, { 0x0A0E, "CheckUpdate(@EvRec):Flag" }, { 0x0B0E, "CloseWindow(@Wind)" }, { 0x0C0E, "Desktop(Oper,param/4):result/4" },
	{ 0x0D0E, "SetWTitle(@Title,@Wind)" }, { 0x0E0E, "GetWTitle(@Wind):@Title" }, { 0x0F0E, "SetFrameColor(@NewColTbl,@Wind)" }, { 0x100E, "GetFrameColor(@Table,@Wind)" },
	{ 0x110E, "SelectWindow(@Wind)" }, { 0x120E, "HideWindow(@Wind)" }, { 0x130E, "ShowWindow(@Wind)" }, { 0x140E, "SendBehind(@BehindWho,@Wind)" },
	{ 0x150E, "FrontWindow():@Wind" }, { 0x160E, "SetInfoDraw(@Proc,@Wind)" }, { 0x170E, "FindWindow(@WindVar,X,Y):Where" }, { 0x180E, "TrackGoAway(X,Y,@Wind):Flag" },
	{ 0x190E, "MoveWindow(X,Y,@Wind)" }, { 0x1A0E, "DragWindow(Grid,X,Y,Grace,@bRect,@Wind)" }, { 0x1B0E, "GrowWindow(mnW,mnH,X,Y,@Wind):nSize/4" },
	{ 0x1C0E, "SizeWindow(w,h,@Wind)" }, { 0x1D0E, "TaskMaster(evMask,@TaskRec):Code" }, { 0x1E0E, "BeginUpdate(@Wind)" }, { 0x1F0E, "EndUpdate(@Wind)" },
	{ 0x200E, "GetWMgrPort():@Port" }, { 0x210E, "PinRect(X,Y,@Rect):Point/4" }, { 0x220E, "HiliteWindow(Flag,@Wind)" }, { 0x230E, "ShowHide(Flag,@Wind)" },
	{ 0x240E, "BringToFront(@Wind)" }, { 0x250E, "WindNewRes()" }, { 0x260E, "TrackZoom(X,Y,@Wind):Flag" }, { 0x270E, "ZoomWindow(@Wind)" }, { 0x280E, "SetWRefCon(Refcon/4,@Wind)" },
	{ 0x290E, "GetWRefCon(@Wind):Refcon/4" }, { 0x2A0E, "GetNextWindow(@Wind):@Wind" }, { 0x2B0E, "GetWKind(@Wind):Flag" }, { 0x2C0E, "GetWFrame(@Wind):Frame" },
	{ 0x2D0E, "SetWFrame(Frame,@Wind)" }, { 0x2E0E, "GetStructRgn(@Wind):StructRgnH" }, { 0x2F0E, "GetContentRgn(@Wind):ContRgnH" }, { 0x300E, "GetUpdateRgn(@Wind):UpdateRgnH" },
	{ 0x310E, "GetDefProc(@Wind):@Proc" }, { 0x320E, "SetDefProc(@Proc,@Wind)" }, { 0x330E, "GetWControls(@Wind):CtrlH" }, { 0x340E, "SetOriginMask(Mask,@Wind)" },
	{ 0x350E, "GetInfoRefCon(@Wind):Refcon/4" }, { 0x360E, "SetInfoRefCon(Val/4,@Wind)" }, { 0x370E, "GetZoomRect(@Wind):@zRect" }, { 0x380E, "SetZoomRect(@zRect,@Wind)" },
	{ 0x390E, "RefreshDesktop(@Rect)" }, { 0x3A0E, "InvalRect(@Rect)" }, { 0x3B0E, "InvalRgn(RgnH)" }, { 0x3C0E, "ValidRect(@Rect)" }, { 0x3D0E, "ValidRgn(RgnH)" },
	{ 0x3E0E, "GetContentOrigin(@Wind):Origin/4" }, { 0x3F0E, "SetContentOrigin(X,Y,@Wind)" }, { 0x400E, "GetDataSize(@Wind):DataSize/4" }, { 0x410E, "SetDataSize(w,h,@Wind)" },
	{ 0x420E, "GetMaxGrow(@Wind):MaxGrow/4" }, { 0x430E, "SetMaxGrow(maxWidth,maxHeight,@Wind)" }, { 0x440E, "GetScroll(@Wind):Scroll/4" }, { 0x450E, "SetScroll(h,v,@Wind)" },
	{ 0x460E, "GetPage(@Wind):Page/4" }, { 0x470E, "SetPage(h,v,@Wind)" }, { 0x480E, "GetContentDraw(@Wind):@Proc" }, { 0x490E, "SetContentDraw(@Proc,@Wind)" },
	{ 0x4A0E, "GetInfoDraw(@Wind):@Proc" }, { 0x4B0E, "SetSysWindow(@Wind)" }, { 0x4C0E, "GetSysWFlag(@Wind):Flag" }, { 0x4D0E, "StartDrawing(@Wind)" },
	{ 0x4E0E, "SetWindowIcons(NewFontH):OldFontH" }, { 0x4F0E, "GetRectInfo(@InfoRect,@Wind)" }, { 0x500E, "StartInfoDrawing(@iRect,@Wind)" }, { 0x510E, "EndInfoDrawing()" },
	{ 0x520E, "GetFirstWindow():@Wind" }, { 0x530E, "WindDragRect(@a,@P,X,Y,@R,@lR,@sR,F):M/4" }, { 0x540E, "Private01():@func [GetDragRectPtr]" },
	{ 0x550E, "DrawInfoBar(@Wind)" }, { 0x560E, "WindowGlobal(Flags):Flags" }, { 0x570E, "SetContentOrigin2(ScrollFlag,X,Y,@Wind)" }, { 0x580E, "GetWindowMgrGlobals():@Globals" },
	{ 0x590E, "AlertWindow(AlertDesc,@SubArray,AlertRef/4):Btn" }, { 0x5A0E, "StartFrameDrawing(@Wind)" }, { 0x5B0E, "EndFrameDrawing()" }, { 0x5C0E, "ResizeWindow(hidden,@ContRect,@Wind)" },
	{ 0x5D0E, "TaskMasterContent" }, { 0x5E0E, "TaskMasterKey" }, { 0x5F0E, "TaskMasterDA(evMask,@bigTaskRec):taskCode" }, { 0x600E, "CompileText(subType,@subs,@text,size):H" },
	{ 0x610E, "NewWindow2(@T,RC/4,@draw,@def,pDesc,pRef/4,rType):@W" }, { 0x620E, "ErrorWindow(subType,@subs,ErrNum):Button" },
	{ 0x630E, "GetAuxWindInfo(@Wind):@Info" }, { 0x640E, "DoModalWindow(@Event,@Update,@EvHook,@Beep,Flags):Result/4" }, { 0x650E, "MWGetCtlPart():Part" },
	{ 0x660E, "MWSetMenuProc(@NewMenuProc):@OldMenuProc" }, { 0x670E, "MWStdDrawProc()" }, { 0x680E, "MWSetUpEditMenu()" }, { 0x690E, "FindCursorCtl(@CtrlH,x,y,@Wind):PartCode" },
	{ 0x6A0E, "ResizeInfoBar(flags,newHeight,@Wind)" }, { 0x6B0E, "HandleDiskInsert(flags,devNum):resFlags,resDevNum" }, { 0x6C0E, "UpdateWindow(flags,@Wind)" },
	{ 0x010F, "MenuBootInit()" }, { 0x020F, "MenuStartUp(MemID,DirPg)" }, { 0x030F, "MenuShutDown()" }, { 0x040F, "MenuVersion():Vers" }, { 0x050F, "MenuReset()" },
	{ 0x060F, "MenuStatus():ActFlg" }, { 0x090F, "MenuKey(@TaskRec,BarH)" }, { 0x0A0F, "GetMenuBar():BarH" }, { 0x0B0F, "MenuRefresh(@RedrawProc)" }, { 0x0C0F, "FlashMenuBar()" },
	{ 0x0D0F, "InsertMenu(MenuH,AfterWhat)" }, { 0x0E0F, "DeleteMenu(MenuID)" }, { 0x0F0F, "InsertMItem(@Item,AfterItem,MenuID)" }, { 0x100F, "DeleteMItem(ItemID)" },
	{ 0x110F, "GetSysBar():BarH" }, { 0x120F, "SetSysBar(BarH)" }, { 0x130F, "FixMenuBar():Height" }, { 0x140F, "CountMItems(MenuID):N" }, { 0x150F, "NewMenuBar(@Wind):BarH" },
	{ 0x160F, "GetMHandle(MenuID):MenuH" }, { 0x170F, "SetBarColors(BarCol,InvCol,OutCol)" }, { 0x180F, "GetBarColors():Colors/4" }, { 0x190F, "SetMTitleStart(hStart)" },
	{ 0x1A0F, "GetMTitleStart():hStart" }, { 0x1B0F, "GetMenuMgrPort():@Port" }, { 0x1C0F, "CalcMenuSize(w,h,MenuID)" }, { 0x1D0F, "SetMTitleWidth(w,MenuID)" },
	{ 0x1E0F, "GetMTitleWidth(MenuID):TitleWidth" }, { 0x1F0F, "SetMenuFlag(Flags,MenuID)" }, { 0x200F, "GetMenuFlag(MenuID):Flags" },{ 0x210F, "SetMenuTitle(@Title,MenuID)" },
	{ 0x220F, "GetMenuTitle(MenuID):@Title" }, { 0x230F, "MenuGlobal(Flags):Flags" }, { 0x240F, "SetMItem(@Str,ItemID)" }, { 0x250F, "GetMItem(ItemID):@ItemName" },
	{ 0x260F, "SetMItemFlag(Flags,ItemID)" }, { 0x270F, "GetMItemFlag(ItemID):Flag" }, { 0x280F, "SetMItemBlink(Count)" }, { 0x290F, "MenuNewRes()" },
	{ 0x2A0F, "DrawMenuBar()" }, { 0x2B0F, "MenuSelect(@TaskRec,BarH)" }, { 0x2C0F, "HiliteMenu(Flag,MenuID)" }, { 0x2D0F, "NewMenu(@MenuStr):MenuH" }, { 0x2E0F, "DisposeMenu(MenuH)" },
	{ 0x2F0F, "InitPalette()" }, { 0x300F, "EnableMItem(ItemID)" }, { 0x310F, "DisableMItem(ItemID)" }, { 0x320F, "CheckMItem(Flag,ItemID)" }, { 0x330F, "SetMItemMark(MarkCh,ItemID)" },
	{ 0x340F, "GetMItemMark(ItemID):MarkChar" }, { 0x350F, "SetMItemStyle(TextStyle,ItemID)" }, { 0x360F, "GetMItemStyle(ItemID):TextStyle" }, { 0x370F, "SetMenuID(New,Old)" },
	{ 0x380F, "SetMItemID(New,Old)" }, { 0x390F, "SetMenuBar(BarH)" }, { 0x3A0F, "SetMItemName(@Str,ItemID)" }, { 0x3B0F, "GetPopUpDefProc():@proc" },
	{ 0x3C0F, "PopUpMenuSelect(SelID,left,top,flag,MenuH):id" }, { 0x3D0F, "[DrawPopUp(SelID,Flag,right,bottom,left,top,MenuH)]" }, { 0x3E0F, "NewMenu2(RefDesc,Ref/4):MenuH" },
	{ 0x3F0F, "InsertMItem2(RefDesc,Ref/4,After,MenuID)" }, { 0x400F, "SetMenuTitle2(RefDesc,TitleRef/4,MenuID)" }, { 0x410F, "SetMItem2(RefDesc,Ref/4,Item)" },
	{ 0x420F, "SetMItemName2(RefDesc,Ref/4,Item)" }, { 0x430F, "NewMenuBar2(RefDesc,Ref/4,@Wind):BarH" }, { 0x450F, "HideMenuBar()" }, { 0x460F, "ShowMenuBar()" },
	{ 0x470F, "SetMItemIcon(IconDesc,IconRef/4,ItemID)" }, { 0x480F, "GetMItemIcon(ItemID):IconRef/4" }, { 0x490F, "SetMItemStruct(Desc,StructRef/4,ItemID)" },
	{ 0x4A0F, "GetMItemStruct(ItemID):ItemStruct/4" }, { 0x4B0F, "RemoveMItemStruct(ItemID)" }, { 0x4C0F, "GetMItemFlag2(ItemID):ItemFlag2" }, { 0x4D0F, "SetMItemFlag2(newValue,ItemID)" },
	{ 0x4F0F, "GetMItemBlink():Count" }, { 0x500F, "InsertPathMItems(flags,@Path,devnum,MenuID,AfterID,StartID,@Results)" }, { 0x0110, "CtlBootInit()" },
	{ 0x0210, "CtlStartUp(MemID,DirPg)" }, { 0x0310, "CtlShutDown()" }, { 0x0410, "CtlVersion():Vers" }, { 0x0510, "CtlReset()" }, { 0x0610, "CtlStatus():ActFlg" },
	{ 0x0910, "NewControl(@W,@R,@T,F,V,P1,P2,@p,r/4,@C):cH" }, { 0x0A10, "DisposeControl(CtrlH)" }, { 0x0B10, "KillControls(@Wind)" }, { 0x0C10, "SetCtlTitle(@Title,CtrlH)" },
	{ 0x0D10, "GetCtlTitle(CtrlH):@Title" }, { 0x0E10, "HideControl(CtrlH)" }, { 0x0F10, "ShowControl(CtrlH)" }, { 0x1010, "DrawControls(@Wind)" },{ 0x1110, "HiliteControl(Flag,CtrlH)" },
	{ 0x1210, "CtlNewRes()" }, { 0x1310, "FindControl(@CtrlHVar,X,Y,@Wind):Part" }, { 0x1410, "TestControl(X,Y,CtrlH):Part" }, { 0x1510, "TrackControl(X,Y,@ActProc,CtrlH):Part" },
	{ 0x1610, "MoveControl(X,Y,CtrlH)" }, { 0x1710, "DragControl(X,Y,@LimR,@slR,Axis,CtrlH)" }, { 0x1810, "SetCtlIcons(FontH):OldFontH" }, { 0x1910, "SetCtlValue(Val,CtrlH)" },
	{ 0x1A10, "GetCtlValue(CtrlH):Val" }, { 0x1B10, "SetCtlParams(P2,P1,CtrlH)" }, { 0x1C10, "GetCtlParams(CtrlH):P1,P2" }, { 0x1D10, "DragRect(@acPr,@P,X,Y,@drR,@l,@slR,F):M/4" },
	{ 0x1E10, "GrowSize():Size/4" }, { 0x1F10, "GetCtlDpage():DirPg" }, { 0x2010, "SetCtlAction(@ActProc,CtrlH)" }, { 0x2110, "GetCtlAction(CtrlH):Action/4" }, { 0x2210, "SetCtlRefCon(Refcon/4,CtrlH)" },
	{ 0x2310, "GetCtlRefCon(CtrlH):Refcon/4" }, { 0x2410, "EraseControl(CtrlH)" }, { 0x2510, "DrawOneCtl(CtrlH)" }, { 0x2610, "FindTargetCtl():CtrlH" },
	{ 0x2710, "MakeNextCtlTarget():CtrlH" }, { 0x2810, "MakeThisCtlTarget(CtrlH)" }, { 0x2910, "SendEventToCtl(TgtOnly,@Wind,@eTask):Accepted" }, { 0x2A10, "GetCtlID(CtrlH):CtlID/4" },
	{ 0x2B10, "SetCtlID(CtlID/4,CtrlH)" }, { 0x2C10, "CallCtlDefProc(CtrlH,Msg,Param/4):Result/4" }, { 0x2D10, "NotifyCtls(Mask,Msg,Param/4,@Wind)" },
	{ 0x2E10, "GetCtlMoreFlags(CtrlH):Flags" }, { 0x2F10, "SetCtlMoreFlags(Flags,CtrlH)" }, { 0x3010, "GetCtlHandleFromID(@Wind,CtlID/4):CtrlH" }, { 0x3110, "NewControl2(@Wind,InKind,InRef/4):CtrlH" },
	{ 0x3210, "CMLoadResource(rType,rID/4):resH" }, { 0x3310, "CMReleaseResource(rType,rID/4)" }, { 0x3410, "SetCtlParamPtr(@SubArray)" }, { 0x3510, "GetCtlParamPtr():@SubArray" },
	{ 0x3710, "InvalCtls(@Wind)" }, { 0x3810, "[reserved]" }, { 0x3910, "FindRadioButton(@Wind,FamilyNum):WhichRadio" }, { 0x3A10, "SetLETextByID(@Wind,leID/4,@PString)" },
	{ 0x3B10, "GetLETextByID(@Wind,leID/4,@PString)" }, { 0x3C10, "SetCtlValueByID(Value,@Wind,CtlID/4)" }, { 0x3D10, "GetCtlValueByID(@Wind,CtlID/4):Value" },{ 0x3E10, "InvalOneCtlByID(@Wind,CtlID/4)" },
	{ 0x3F10, "HiliteCtlByID(Hilite,@Wind,CtlID/4)" }, { 0x0111, "LoaderBootInit()" }, { 0x0211, "LoaderStartUp()" }, { 0x0311, "LoaderShutDown()" },
	{ 0x0411, "LoaderVersion():Vers" }, { 0x0511, "LoaderReset()" }, { 0x0611, "LoaderStatus():ActFlg" }, { 0x0911, "InitialLoad(MemID,@path,F):dpsSz,dps,@l,MemID" },
	{ 0x0A11, "Restart(MemID):dpsSz,dps,@loc,MemID" }, { 0x0B11, "LoadSegNum(MemID,file#,seg#):@loc" }, { 0x0C11, "UnloadSegNum(MemID,file#,seg#)" }, { 0x0D11, "LoadSegName(MemID,@path,@segn):@loc,MemID,file#,sg#" },
	{ 0x0E11, "UnloadSeg(@loc):seg#,file#,MemID" }, { 0x0F11, "GetLoadSegInfo(MemID,file#,seg#,@buff)" }, { 0x1011, "GetUserID(@Pathname):MemID" }, { 0x1111, "LGetPathname(MemID,file#):@path" },
	{ 0x1211, "UserShutDown(MemID,qFlag):MemID" }, { 0x1311, "RenamePathname(@path1,@path2)" }, { 0x2011, "InitialLoad2(MemID,@in,F,Type):dpsSz,dps,@l,MemID" },
	{ 0x2111, "GetUserID2(@path):MemID" }, { 0x2211, "LGetPathname2(MemID,file#):@path" }, { 0x0112, "QDAuxBootInit()" }, { 0x0212, "QDAuxStartUp()" },
	{ 0x0312, "QDAuxShutDown()" }, { 0x0412, "QDAuxVersion():Vers" }, { 0x0512, "QDAuxReset()" }, { 0x0612, "QDAuxStatus():ActFlg" }, { 0x0912, "CopyPixels(@sLoc,@dLoc,@sRect,@dRct,M,MskH)" },
	{ 0x0A12, "WaitCursor()" }, { 0x0B12, "DrawIcon(@Icon,Mode,X,Y)" }, { 0x0C12, "SpecialRect(@Rect,FrameColor,FillColor)" }, { 0x0D12, "SeedFill(@sLoc,@sR,@dLoc,@dR,X,Y,Mode,@Patt,@Leak)" },
	{ 0x0E12, "CalcMask(@sLoc,@sR,@dLoc,@dR,Mode,@Patt,@Leak)" }, { 0x0F12, "GetSysIcon(flags,value,aux/4):@Icon" }, { 0x1012, "PixelMap2Rgn(@srcLoc,bitsPerPix,colorMask):RgnH" },
	{ 0x1312, "IBeamCursor()" }, { 0x1412, "WhooshRect(flags/4,@smallRect,@bigRect)" }, { 0x1512, "DrawStringWidth(Flags,Ref/4,Width)" }, { 0x1612, "UseColorTable(tableNum,@Table,Flags):ColorInfoH" },
	{ 0x1712, "RestoreColorTable(ColorInfoH,Flags)" }, { 0x0113, "PMBootInit()" }, { 0x0213, "PMStartUp(MemID,DirPg)" }, { 0x0313, "PMShutDown()" },
	{ 0x0413, "PMVersion():Vers" }, { 0x0513, "PMReset()" }, { 0x0613, "PMStatus():ActFlg" }, { 0x0913, "PrDefault(PrRecH)" }, { 0x0A13, "PrValidate(PrRecH):Flag" },
	{ 0x0B13, "PrStlDialog(PrRecH):Flag" }, { 0x0C13, "PrJobDialog(PrRecH):Flag" }, { 0x0D13, "PrPixelMap(@LocInfo,@SrcRect,colorFlag)" }, { 0x0E13, "PrOpenDoc(PrRecH,@Port):@Port" },
	{ 0x0F13, "PrCloseDoc(@Port)" }, { 0x1013, "PrOpenPage(@Port,@Frame)" }, { 0x1113, "PrClosePage(@Port)" }, { 0x1213, "PrPicFile(PrRecH,@Port,@StatRec)" },{ 0x1313, "PrControl [obsolete]" },
	{ 0x1413, "PrError():Error" }, { 0x1513, "PrSetError(Error)" }, { 0x1613, "PrChoosePrinter():DrvFlag" }, { 0x1813, "PrGetPrinterSpecs():Type,Characteristics" },
	{ 0x1913, "PrDevPrChanged(@PrinterName)" }, { 0x1A13, "PrDevStartup(@PrinterName,@ZoneName)" }, { 0x1B13, "PrDevShutDown()" }, { 0x1C13, "PrDevOpen(@compProc,Reserved/4)" },
	{ 0x1D13, "PrDevRead(@buffer,reqCount):xferCount" }, { 0x1E13, "PrDevWrite(@compProc,@buff,bufLen)" }, { 0x1F13, "PrDevClose()" }, { 0x2013, "PrDevStatus(@statBuff)" },
	{ 0x2113, "PrDevAsyncRead(@compPr,bufLen,@buff):xferCount" }, { 0x2213, "PrDevWriteBackground(@compProc,bufLen,@buff)" }, { 0x2313, "PrDriverVer():Vers" },
	{ 0x2413, "PrPortVer():Vers" }, { 0x2513, "PrGetZoneName():@ZoneName" }, { 0x2813, "PrGetPrinterDvrName():@Name" }, { 0x2913, "PrGetPortDvrName():@Name" },
	{ 0x2A13, "PrGetUserName():@Name" }, { 0x2B13, "PrGetNetworkName():@Name" }, { 0x3013, "PrDevIsItSafe():safeFlag" }, { 0x3113, "GetZoneList     [obsolete?]" },
	{ 0x3213, "GetMyZone       [obsolete?]" }, { 0x3313, "GetPrinterList  [obsolete?]" }, { 0x3413, "PMUnloadDriver(whichDriver)" }, { 0x3513, "PMLoadDriver(whichDriver)" },
	{ 0x3613, "PrGetDocName():@pStr" }, { 0x3713, "PrSetDocName(@pStr)" }, { 0x3813, "PrGetPgOrientation(PrRecH):Orientation" }, { 0x0114, "LEBootInit()" },
	{ 0x0214, "LEStartUp(MemID,DirPg)" }, { 0x0314, "LEShutDown()" }, { 0x0414, "LEVersion():Vers" }, { 0x0514, "LEReset()" }, { 0x0614, "LEStatus():ActFlg" },
	{ 0x0914, "LENew(@DstRect,@ViewRect,MaxL):leH" }, { 0x0A14, "LEDispose(leH)" }, { 0x0B14, "LESetText(@Text,Len,leH)" }, { 0x0C14, "LEIdle(leH)" },
	{ 0x0D14, "LEClick(@EvRec,leH)" }, { 0x0E14, "LESetSelect(Start,End,leH)" }, { 0x0F14, "LEActivate(leH)" }, { 0x1014, "LEDeactivate(leH)" }, { 0x1114, "LEKey(Key,Mods,leH)" },
	{ 0x1214, "LECut(leH)" }, { 0x1314, "LECopy(leH)" }, { 0x1414, "LEPaste(leH)" }, { 0x1514, "LEDelete(leH)" },{ 0x1614, "LEInsert(@Text,Len,leH)" },
	{ 0x1714, "LEUpdate(leH)" }, { 0x1814, "LETextBox(@Text,Len,@Rect,Just)" }, { 0x1914, "LEFromScrap()" }, { 0x1A14, "LEToScrap()" }, { 0x1B14, "LEScrapHandle():ScrapH" },
	{ 0x1C14, "LEGetScrapLen():Len" }, { 0x1D14, "LESetScrapLen(NewL)" }, { 0x1E14, "LESetHilite(@HiliteProc,leH)" }, { 0x1F14, "LESetCaret(@CaretProc,leH)" },
	{ 0x2014, "LETextBox2(@Text,Len,@Rect,Just)" }, { 0x2114, "LESetJust(Just,leH)" }, { 0x2214, "LEGetTextHand(leH):TextH" }, { 0x2314, "LEGetTextLen(leH):TxtLen" },
	{ 0x2414, "GetLEDefProc():@proc" }, { 0x2514, "LEClassifyKey(@Event):Flag" }, { 0x0115, "DialogBootInit()" }, { 0x0215, "DialogStartUp(MemID)" }, { 0x0315, "DialogShutDown()" },
	{ 0x0415, "DialogVersion():Vers" }, { 0x0515, "DialogReset()" }, { 0x0615, "DialogStatus():ActFlg" }, { 0x0915, "ErrorSound(@SoundProc)" }, { 0x0A15, "NewModalDialog(@bR,vis,refcon/4):@Dlog" },
	{ 0x0B15, "NewModelessDialog(@R,@T,@b,fr,rf/4,@zR):@D" }, { 0x0C15, "CloseDialog(@Dlog)" }, { 0x0D15, "NewDItem(@Dlog,dItem,@R,ty,Des/4,V,F,@Col)" },
	{ 0x0E15, "RemoveDItem(@Dlog,dItem)" }, { 0x0F15, "ModalDialog(@FilterProc):Hit" }, { 0x1015, "IsDialogEvent(@EvRec):Flag" },{ 0x1115, "DialogSelect(@EvRec,@Dlog,@Hit):Flag" },
	{ 0x1215, "DlgCut(@Dlog)" }, { 0x1315, "DlgCopy(@Dlog)" }, { 0x1415, "DlgPaste(@Dlog)" }, { 0x1515, "DlgDelete(@Dlog)" }, { 0x1615, "DrawDialog(@Dlog)" },
	{ 0x1715, "Alert(@AlertTmpl,@FiltProc):Hit" }, { 0x1815, "StopAlert(@AlertTmpl,@FiltProc):Hit" }, { 0x1915, "NoteAlert(@AlertTmpl,@FiltProc):Hit" },
	{ 0x1A15, "CautionAlert(@AlertTmpl,@FiltProc):Hit" }, { 0x1B15, "ParamText(@P0,@P1,@P2,@P3)" }, { 0x1C15, "SetDAFont(FontH)" }, { 0x1E15, "GetControlDItem(@Dlog,dItem):CtrlH" },
	{ 0x1F15, "GetIText(@Dlog,dItem,@Str)" }, { 0x2015, "SetIText(@Dlog,dItem,@Str)" }, { 0x2115, "SelectIText(@Dlog,dItem,start,end)" }, { 0x2215, "HideDItem(@Dlog,dItem)" },
	{ 0x2315, "ShowDItem(@Dlog,dItem)" }, { 0x2415, "FindDItem(@Dlog,Point/4):Hit" }, { 0x2515, "UpdateDialog(@Dlog,UpdtRgnH)" }, { 0x2615, "GetDItemType(@Dlog,dItem):type" },
	{ 0x2715, "SetDItemType(type,@Dlog,dItem)" }, { 0x2815, "GetDItemBox(@Dlog,dItem,@Rect)" }, { 0x2915, "SetDItemBox(@Dlog,dItem,@Rect)" },
	{ 0x2A15, "GetFirstDItem(@Dlog):dItem" }, { 0x2B15, "GetNextDItem(@Dlog,dItem):dItem" }, { 0x2C15, "ModalDialog2(@FilterProc):HitInfo/4" },{ 0x2E15, "GetDItemValue(@Dlog,dItem):Val" },
	{ 0x2F15, "SetDItemValue(val,@Dlog,dItem)" }, { 0x3215, "GetNewModalDialog(@DlogTmpl):@Dlog" }, { 0x3315, "GetNewDItem(@Dlog,@ItemTmpl)" },{ 0x3415, "GetAlertStage():Stage" },
	{ 0x3515, "ResetAlertStage()" }, { 0x3615, "DefaultFilter(@Dlog,@EvRec,@Hit):Flag" }, { 0x3715, "GetDefButton(@Dlog):dItem" }, { 0x3815, "SetDefButton(BtnID,@Dlog)" },
	{ 0x3915, "DisableDItem(@Dlog,dItem)" }, { 0x3A15, "EnableDItem(@Dlog,dItem)" }, { 0x0116, "ScrapBootInit()" }, { 0x0216, "ScrapStartUp()" }, { 0x0316, "ScrapShutDown()" },
	{ 0x0416, "ScrapVersion():Vers" }, { 0x0516, "ScrapReset()" }, { 0x0616, "ScrapStatus():ActFlg" }, { 0x0916, "UnloadScrap()" }, { 0x0A16, "LoadScrap()" },
	{ 0x0B16, "ZeroScrap()" }, { 0x0C16, "PutScrap(Count/4,Type,@Src)" }, { 0x0D16, "GetScrap(DestH,Type)" }, { 0x0E16, "GetScrapHandle(Type):ScrapH" },
	{ 0x0F16, "GetScrapSize(Type):Size/4" },{ 0x1016, "GetScrapPath():@Pathname" }, { 0x1116, "SetScrapPath(@Pathname)" }, { 0x1216, "GetScrapCount():Count" },
	{ 0x1316, "GetScrapState():State" }, { 0x1416, "GetIndScrap(Index,@buffer)" }, { 0x1516, "ShowClipboard(flags,@rect):@Wind" }, { 0x0117, "SFBootInit()" },
	{ 0x0217, "SFStartUp(MemID,DirPg)" }, { 0x0317, "SFShutDown()" }, { 0x0417, "SFVersion():Vers" }, { 0x0517, "SFReset()" },{ 0x0617, "SFStatus():ActFlg" },
	{ 0x0917, "SFGetFile(X,Y,@Prmpt,@FPrc,@tL,@Reply)" }, { 0x0A17, "SFPutFile(X,Y,@Prompt,@DfltName,mxL,@Reply)" },{ 0x0B17, "SFPGetFile(X,Y,@P,@FPrc,@tL,@dTmp,@dHk,@Rp)" },
	{ 0x0C17, "SFPPutFile(X,Y,@P,@Df,mxL,@dTmpl,@dHk,@Rply)" }, { 0x0D17, "SFAllCaps(Flag)" },{ 0x0E17, "SFGetFile2(X,Y,prDesc,prRef/4,@fProc,@tList,@rep)" },
	{ 0x0F17, "SFPutFile2(X,Y,prDesc,prRef/4,nmDesc,nmRef/4,@rep)" }, { 0x1017, "SFPGetFile2(X,Y,@draw,prD,prRf/4,@fP,@tL,@d,@hk,@rep)" }, { 0x1117, "SFPPutFile2(X,Y,@draw,prD,prRf/4,nmD,nmRf/4,@d,@hk,@rep)" },
	{ 0x1217, "SFShowInvisible(InvisState):OldState" }, { 0x1317, "SFReScan(@filterProc,@typeList)" }, { 0x1417, "SFMultiGet2(X,Y,prDesc,prRef/4,@fP,@tL,@rep)" },
	{ 0x1517, "SFPMultiGet2(X,Y,@draw,prD,prRf/4,@fP,@tL,@d,@hk,@rep)" }, { 0x0119, "NSBootInit()" }, { 0x0219, "NSStartUp(Rate,@UpdProc)" }, { 0x0319, "NSShutDown()" },
	{ 0x0419, "NSVersion():Vers" }, { 0x0519, "NSReset()" }, { 0x0619, "NSStatus():ActFlg" }, { 0x0919, "AllocGen(Priority):Gen#" }, { 0x0A19, "DeallocGen(Gen#)" },
	{ 0x0B19, "NoteOn(Gen#,Semitone,Vol,@Instr)" }, { 0x0C19, "NoteOff(Gen#,Semitone)" }, { 0x0D19, "AllNotesOff()" }, { 0x0E19, "NSSetUpdateRate(NewRate):OldRate" },
	{ 0x0F19, "NSSetUserUpdateRtn(@New):@Old" }, { 0x011A, "SeqBootInit()" }, { 0x021A, "SeqStartUp(DirPg,Mode,Rate,Incr)" }, { 0x031A, "SeqShutDown()" },
	{ 0x041A, "SeqVersion():Vers" }, { 0x051A, "SeqReset()" }, { 0x061A, "SeqStatus():ActFlg" }, { 0x091A, "SetIncr(Increment)" }, { 0x0A1A, "ClearIncr():OldIncr" },
	{ 0x0B1A, "GetTimer():Tick" }, { 0x0C1A, "GetLoc():Phrase,Patt,Level" }, { 0x0D1A, "SeqAllNotesOff()" }, { 0x0E1A, "SetTrkInfo(Priority,InstIndex,TrkNum)" },
	{ 0x0F1A, "StartSeq(@ErrRtn,@CompRtn,SeqH)" }, { 0x101A, "StepSeq()" }, { 0x111A, "StopSeq(NextFlag)" }, { 0x121A, "SetInstTable(TableH)" }, { 0x131A, "StartInts()" },
	{ 0x141A, "StopInts()" }, { 0x151A, "StartSeqRel(@errHndlr,@CompRtn,SeqH)" }, { 0x011B, "FMBootInit()" }, { 0x021B, "FMStartUp(MemID,DirPg)" }, { 0x031B, "FMShutDown()" },
	{ 0x041B, "FMVersion():Vers" }, { 0x051B, "FMReset()" }, { 0x061B, "FMStatus():ActFlg" }, { 0x091B, "CountFamilies(FamSpecs):Count" }, { 0x0A1B, "FindFamily(Specs,Pos,@Name):FamNum" },
	{ 0x0B1B, "GetFamInfo(FamNum,@Name):FamStats" }, { 0x0C1B, "GetFamNum(@Name):FamNum" }, { 0x0D1B, "AddFamily(FamNum,@Name)" }, { 0x0E1B, "InstallFont(ID/4,Scale)" },
	{ 0x0F1B, "SetPurgeStat(FontID/4,PrgStat)" }, { 0x101B, "CountFonts(ID/4,Specs):N" }, { 0x111B, "FindFontStats(ID/4,Specs,Pos,@FStatRec)" }, { 0x121B, "LoadFont(ID/4,Specs,Pos,@FStatRec)" },
	{ 0x131B, "LoadSysFont()" }, { 0x141B, "AddFontVar(FontH,NewSpecs)" }, { 0x151B, "FixFontMenu(MenuID,StartID,FamSpecs)" }, { 0x161B, "ChooseFont(CurrID/4,Famspecs):NewID/4" },
	{ 0x171B, "ItemID2FamNum(ItemID):FamNum" }, { 0x181B, "FMSetSysFont(FontID/4)" }, { 0x191B, "FMGetSysFID():SysID/4" }, { 0x1A1B, "FMGetCurFID():CurID/4" },
	{ 0x1B1B, "FamNum2ItemID(FamNum):ItemID" }, { 0x1C1B, "InstallWithStats(ID/4,Scale,@ResultRec)" }, { 0x011C, "ListBootInit()" }, { 0x021C, "ListStartUp()" }, { 0x031C, "ListShutDown()" },
	{ 0x041C, "ListVersion():Vers" }, { 0x051C, "ListReset()" }, { 0x061C, "ListStatus():ActFlg" }, { 0x091C, "CreateList(@Wind,@ListRec):CtrlH" }, { 0x0A1C, "SortList(@CompareProc,@ListRec)" },
	{ 0x0B1C, "NextMember(@Member,@ListRec):@NxtMemVal" }, { 0x0C1C, "DrawMember(@Member,@ListRec)" }, { 0x0D1C, "SelectMember(@Member,@ListRec)" }, { 0x0E1C, "GetListDefProc():@Proc" },
	{ 0x0F1C, "ResetMember(@ListRec):NxtMemVal/4" }, { 0x101C, "NewList(@Member,@ListRec)" }, { 0x111C, "DrawMember2(itemNum,CtrlH)" }, { 0x121C, "NextMember2(itemNum,CtrlH):itemNum" },
	{ 0x131C, "ResetMember2(CtrlH):itemNum" }, { 0x141C, "SelectMember2(itemNum,CtrlH)" }, { 0x151C, "SortList2(@CompareProc,CtrlH)" }, { 0x161C, "NewList2(@draw,start,ref/4,refKind,size,CtrlH)" },
	{ 0x171C, "ListKey(flags,@EventRec,CtrlH)" }, { 0x181C, "CompareStrings(flags,@String1,@String2):Order" }, { 0x011D, "ACEBootInit()" }, { 0x021D, "ACEStartUp(DirPg)" },
	{ 0x031D, "ACEShutDown()" }, { 0x041D, "ACEVersion():Vers" }, { 0x051D, "ACEReset()" }, { 0x061D, "ACEStatus():ActFlg" }, { 0x071D, "ACEInfo(Code):Value/4" },
	{ 0x091D, "ACECompress(SrcH,SrcOff/4,DestH,DestOff/4,Blks,Method)" }, { 0x0A1D, "ACEExpand(SrcH,SrcOff/4,DestH,DestOff/4,Blks,Method)" }, { 0x0B1D, "ACECompBegin()" },
	{ 0x0C1D, "ACEExpBegin()" }, { 0x0D1D, "GetACEExpState(@Buffer)" }, { 0x0E1D, "SetACEExpState(@Buffer)" }, { 0x011E, "ResourceBootInit()" }, { 0x021E, "ResourceStartUp(MemID)" },
	{ 0x031E, "ResourceShutDown()" }, { 0x041E, "ResourceVersion():Vers" }, { 0x051E, "ResourceReset()" }, { 0x061E, "ResourceStatus():ActFlag" }, { 0x091E, "CreateResourceFile(aux/4,fType,Access,@n)" },
	{ 0x0A1E, "OpenResourceFile(reqAcc,@mapAddr,@n):fileID" }, { 0x0B1E, "CloseResourceFile(fileID)" }, { 0x0C1E, "AddResource(H,Attr,rType,rID/4)" }, { 0x0D1E, "UpdateResourcefile(fileID)" },
	{ 0x0E1E, "LoadResource(rType,rID/4):H" }, { 0x0F1E, "RemoveResource(rType,rID/4)" }, { 0x101E, "MarkResourceChange(changeFlag,rType,rID/4)" }, { 0x111E, "SetCurResourceFile(fileID)" },
	{ 0x121E, "GetCurResourceFile():fileID" }, { 0x131E, "SetCurResourceApp(MemID)" }, { 0x141E, "GetCurResourceApp():MemID" }, { 0x151E, "HomeResourceFile(rType,rID/4):fileID" },
	{ 0x161E, "WriteResource(rType,rID/4)" }, { 0x171E, "ReleaseResource(PurgeLevel,rType,rID/4)" }, { 0x181E, "DetachResource(rType,rID/4)" }, { 0x191E, "UniqueResourceID(IDrange,rType):rID/4" },
	{ 0x1A1E, "SetResourceID(newID/4,rType,oldID/4)" }, { 0x1B1E, "GetResourceAttr(rType,rID/4):Attr" }, { 0x1C1E, "SetResourceAttr(rAttr,rType,rID/4)" }, { 0x1D1E, "GetResourceSize(rType,rID/4):Size/4" },
	{ 0x1E1E, "MatchResourceHandle(@buffer,H)" }, { 0x1F1E, "GetOpenFileRefNum(fileID):RefNum" }, { 0x201E, "CountTypes():Num" }, { 0x211E, "GetIndType(tIndex):rType" },
	{ 0x221E, "CountResources(rType):Num/4" }, { 0x231E, "GetIndResource(rType,rIndex/4):rID/4" }, { 0x241E, "SetResourceLoad(Flag):oldFlag" }, { 0x251E, "SetResourceFileDepth(Depth):oldDepth" },
	{ 0x261E, "GetMapHandle(fileID):MapH" }, { 0x271E, "LoadAbsResource(@loc,MaxSize/4,rType,rID/4):Size/4" }, { 0x281E, "ResourceConverter(@proc,rType,logFlags)" },
	{ 0x291E, "LoadResource2(flag,@AttrBuff,rType,rID/4):H" }, { 0x2A1E, "RMFindNamedResource(rType,@name,@fileID):rID/4" },{ 0x2B1E, "RMGetResourceName(rType,rID/4,@nameBuffer)" },
	{ 0x2C1E, "RMLoadNamedResource(rType,@name):H" }, { 0x2D1E, "RMSetResourceName(rType,rID/4,@name)" }, { 0x2E1E, "OpenResourceFileByID(reqAcc,userID):oldResApp" },
	{ 0x2F1E, "CompactResourceFile(flags,fileID)" }, { 0x0120, "MidiBootInit()" }, { 0x0220, "MidiStartUp(MemID,DirPg)" }, { 0x0320, "MidiShutDown()" },
	{ 0x0420, "MidiVersion():Vers" }, { 0x0520, "MidiReset()" }, { 0x0620, "MidiStatus():ActFlg" }, { 0x0920, "MidiControl(Function,Argument/4)" }, { 0x0A20, "MidiDevice(Function,@DriverInfo)" },
	{ 0x0B20, "MidiClock(Function,Argument/4)" }, { 0x0C20, "MidiInfo(Function):Info/4" }, { 0x0D20, "MidiReadPacket(@buff,size):Count" }, { 0x0E20, "MidiWritePacket(@buff):Count" },
	{ 0x0121, "VDBootInit()" }, { 0x0221, "VDStartUp()" }, { 0x0321, "VDShutDown()" }, { 0x0421, "VDVersion():Vers" }, { 0x0521, "VDReset()" }, { 0x0621, "VDStatus():ActFlg" },
	{ 0x0921, "VDInStatus(Selector):Status" }, { 0x0A21, "VDInSetStd(InStandard)" }, { 0x0B21, "VDInGetStd():InStandard" }, { 0x0C21, "VDInConvAdj(Selector,AdjFunction)" },
	{ 0x0D21, "VDKeyControl(Selector,KeyerCtrlVal)" }, { 0x0E21, "VDKeyStatus(Selector):KeyerStatus" }, { 0x0F21, "VDKeySetKCol(Red,Green,Blue)" }, { 0x1021, "VDKeyGetKRCol():RedValue" },
	{ 0x1121, "VDKeyGetKGCol():GreenValue" }, { 0x1221, "VDKeyGetKBCol():BlueValue" }, { 0x1321, "VDKeySetKDiss(KDissolve)" }, { 0x1421, "VDKeyGetKDiss():KDissolve" },
	{ 0x1521, "VDKeySetNKDiss(NKDissolve)" }, { 0x1621, "VDKeyGetNKDiss():NKDissolve" }, { 0x1721, "VDOutSetStd(OutStandard)" }, { 0x1821, "VDOutGetStd():OutStandard" },
	{ 0x1921, "VDOutControl(Selector,Value)" }, { 0x1A21, "VDOutStatus(Selector):OutStatus" }, { 0x1B21, "VDGetFeatures(Feature):Info" }, { 0x1C21, "VDInControl(Selector,Value)" },
	{ 0x1D21, "VDGGControl(Selector,Value)" }, { 0x1E21, "VDGGStatus(Selector):Value" }, { 0x0122, "TEBootInit()" }, { 0x0222, "TEStartUp(MemID,DirPg)" },
	{ 0x0322, "TEShutDown()" }, { 0x0422, "TEVersion():Vers" }, { 0x0522, "TEReset()" }, { 0x0622, "TEStatus():ActFlg" }, { 0x0922, "TENew(@parms):teH" },
	{ 0x0A22, "TEKill(teH)" }, { 0x0B22, "TESetText(tDesc,tRef/4,Len/4,stDesc,stRef/4,teH)" }, { 0x0C22, "TEGetText(bDesc,bRef/4,bLen/4,stDesc,stRef/4,teH):L/4" },
	{ 0x0D22, "TEGetTextInfo(@infoRec,parmCount,teH)" }, { 0x0E22, "TEIdle(teH)" }, { 0x0F22, "TEActivate(teH)" }, { 0x1022, "TEDeactivate(teH)" }, { 0x1122, "TEClick(@eventRec,teH)" },
	{ 0x1222, "TEUpdate(teH)" }, { 0x1322, "TEPaintText(@Port,startLn/4,@R,Flags,teH):NextLn/4" }, { 0x1422, "TEKey(@eventRec,teH)" }, { 0x1522, "[not supported]" },
	{ 0x1622, "TECut(teH)" }, { 0x1722, "TECopy(teH)" }, { 0x1822, "TEPaste(teH)" }, { 0x1922, "TEClear(teH)" }, { 0x1A22, "TEInsert(tDesc,tRef/4,tLen/4,stDesc,stRef/4,teH)" },{ 0x1B22, "TEReplace(tDesc,tRef/4,tLen/4,stDesc,stRef/4,teH)" },
	{ 0x1C22, "TEGetSelection(@selStart,@selEnd,teH)" }, { 0x1D22, "TESetSelection(selStart/4,selEnd/4,teH)" }, { 0x1E22, "TEGetSelectionStyle(@stRec,stH,teH):comFlag" }, { 0x1F22, "TEStyleChange(flags,@stRec,teH)" },
	{ 0x2022, "TEOffsetToPoint(offset/4,@vertPos,@horPos,teH)" }, { 0x2122, "TEPointToOffset(vertPos/4,horPos/4,teH):textOffset/4" }, { 0x2222, "TEGetDefProc():@defProc" },
	{ 0x2322, "TEGetRuler(rulerDesc,rulerRef/4,teH)" }, { 0x2422, "TESetRuler(rulerDesc,rulerRef/4,teH)" }, { 0x2522, "TEScroll(scrDesc,vertAmt/4,horAmt/4,teH):Offset/4" },
	{ 0x2622, "TEGetInternalProc():@proc" }, { 0x2722, "TEGetLastError(clearFlag,teH):lastError" }, { 0x2822, "TECompactRecord(teH)" }, { 0x0123, "MSBootInit()" },
	{ 0x0223, "MSStartUp()" }, { 0x0323, "MSShutDown()" }, { 0x0423, "MSVersion():Vers" }, { 0x0523, "MSReset()" }, { 0x0623, "MSStatus():ActFlg" },{ 0x0923, "SetBasicChannel(Channel)" },
	{ 0x0A23, "SetMIDIMode(Mode)" }, { 0x0B23, "PlayNote(Channel,NoteNum,KeyVel)" }, { 0x0C23, "StopNote(Channel,NoteNum)" }, { 0x0D23, "KillAllNotes()" },
	{ 0x0E23, "SetRecTrack(TrackNum)" }, { 0x0F23, "SetPlayTrack(TrackNum,State)" }, { 0x1023, "TrackToChannel(TrackNum,ChannelNum)" }, { 0x1123, "Locate(TimeStamp/4,@SeqBuff):@SeqItem" },
	{ 0x1223, "SetVelComp(VelocityOffset)" }, { 0x1323, "SetMIDIPort(EnabInput,EnabOutput)" }, { 0x1423, "SetInstrument(@InstRec,InstNum)" }, { 0x1523, "SeqPlayer(@SeqPlayerRec)" },
	{ 0x1623, "SetTempo(Tempo)" }, { 0x1723, "SetCallBack(@CallBackRec)" }, { 0x1823, "SysExOut(@Msg,Delay,@MonRoutine)" }, { 0x1923, "SetBeat(BeatDuration)" },
	{ 0x1A23, "MIDIMessage(Dest,nBytes,Message,Byte1,Byte2)" }, { 0x1B23, "LocateEnd(@seqBuffer):@End" }, { 0x1C23, "Merge(@Buffer1,@Buffer2)" }, { 0x1D23, "DeleteTrack(TrackNum,@Seq)" },
	{ 0x1E23, "SetMetro(Volume,Freq,@Wave)" }, { 0x1F23, "GetMSData():Reserved/4,@DirPage" }, { 0x2023, "ConvertToTime(TkPerBt,BtPerMsr,BeatNum,MsrNum):Ticks/4" }, { 0x2123, "ConvertToMeasure(TkPerBt,BtPerMsr,Ticks/4):Ticks,Beat,Msr" },
	{ 0x2223, "MSSuspend()" }, { 0x2323, "MSResume()" }, { 0x2423, "SetTuningTable(@Table)" }, { 0x2523, "GetTuningTable(@Buffer)" }, { 0x2623, "SetTrackOut(TrackNum,PathVal)" },
	{ 0x2723, "InitMIDIDriver(Slot,Internal,UserID,@Driver)" }, { 0x2823, "RemoveMIDIDriver()" }, { 0x0126, "MCBootInit()" }, { 0x0226, "MCStartUp(MemID)" }, { 0x0326, "MCShutDown()" },
	{ 0x0426, "MCVersion():Vers" }, { 0x0526, "MCReset()" }, { 0x0626, "MCStatus():ActFlg" }, { 0x0926, "MCGetErrorMsg(mcErrorNo,@PStringBuff)" }, { 0x0A26, "MCLoadDriver(mcChannelNo)" },
	{ 0x0B26, "MCUnLoadDriver(mcChannelNo)" }, { 0x0C26, "MCTimeToBin(mcTimeValue/4):result/4" }, { 0x0D26, "MCBinToTime(mcBinVal/4):result/4" }, { 0x0E26, "MCGetTrackTitle(mcDiskID/4,mcTrackNo,@PStringBuff)" },
	{ 0x0F26, "MCSetTrackTitle(mcDiskID/4,TrackNum,@title)" }, { 0x1026, "MCGetProgram(mcDiskID/4,@resultBuff)" }, { 0x1126, "MCSetProgram(mcDiskID/4,@mcProg)" },
	{ 0x1226, "MCGetDiscTitle(mcDiskID/4,@PStringBuff)" }, { 0x1326, "MCSetDiscTitle(mcDiskID/4,@title)" }, { 0x1426, "MCDStartUp(mcChannelNo,@portName,userID)" },
	{ 0x1526, "MCDShutDown(mcChannelNo)" }, { 0x1626, "MCGetFeatures(mcChannelNo,mcFeatSel):result/4" }, { 0x1726, "MCPlay(mcChannelNo)" }, { 0x1826, "MCPause(mcChannelNo)" },
	{ 0x1926, "MCSendRawData(mcChannelNo,@mcNative)" }, { 0x1A26, "MCGetStatus(mcChannelNo,mcStatusSel):result" }, { 0x1B26, "MCControl(mcChannelNo,ctlCommand)" },
	{ 0x1C26, "MCScan(mcChannelNo,mcDirection)" }, { 0x1D26, "MCGetSpeeds(mcChannelNo,@PStringBuff)" }, { 0x1E26, "MCSpeed(mcChannelNo,mcFPS)" },
	{ 0x1F26, "MCStopAt(mcChannelNo,mcUnitType,mcStopLoc/4)" }, { 0x2026, "MCJog(mcChannelNo,mcUnitType,mcNJog/4,mcJogRepeat)" }, { 0x2126, "MCSearchTo(mcChannelNo,mcUnitType,searchLoc/4)" },
	{ 0x2226, "MCSearchDone(mcChannelNo):result" }, { 0x2326, "MCSearchWait(mcChannelNo)" }, { 0x2426, "MCGetPosition(mcChannelNo,mcUnitType):result/4" },{ 0x2526, "MCSetAudio(mcChannelNo,mcAudioCtl)" },
	{ 0x2626, "MCGetTimes(mcChannelNo,mctimesSel):result/4" }, { 0x2726, "MCGetDiscTOC(mcChannelNo,mcTrackNo):result/4" }, { 0x2826, "MCGetDiscID(mcChannelNo):result/4" },
	{ 0x2926, "MCGetNoTracks(mcChannelNo):result" }, { 0x2A26, "MCRecord(mcChannelNo)" }, { 0x2B26, "MCStop(mcChannelNo)" }, { 0x2C26, "MCWaitRawData(mcChannelNo,@result,tickWait,termMask)" },
	{ 0x2D26, "MCGetName(mcChannelNo,@PStringBuff)" }, { 0x2E26, "MCSetVolume(mcChannelNo,mcLeftVol,mcRightVol)" }, { 0x0136, "TCPIPBootInit()" }, { 0x0236, "TCPIPStartUp()" },
	{ 0x0336, "TCPIPShutDown()" }, { 0x0436, "TCPIPVersion():Vers" }, { 0x0536, "TCPIPReset()" }, { 0x0636, "TCPIPStatus():ActFlg" }, { 0x0836, "TCPIPLongVersion():rVersion/4" },
	{ 0x0936, "TCPIPGetConnectStatus():connectedFlag" }, { 0x0A36, "TCPIPGetErrorTable():@errTablePtr" },{ 0x0B36, "TCPIPGetReconnectStatus():reconnectFlag" },{ 0x0C36, "TCPIPReconnect(@displayPtr)" },
	{ 0x0D36, "TCPIPConvertIPToHex(@cvtRecPtr,@ddippstring)" }, { 0x0E36, "TCPIPConvertIPToASCII(ipaddress/4,@ddpstring,flags):strlen" }, { 0x0F36, "TCPIPGetMyIPAddress():ipaddress/4" },
	{ 0x1036, "TCPIPGetConnectMethod():method" }, { 0x1136, "TCPIPSetConnectMethod(method)" }, { 0x1236, "TCPIPConnect(@displayPtr)" }, { 0x1336, "TCPIPDisconnect(forceflag,@displayPtr)" },
	{ 0x1436, "TCPIPGetMTU():mtu" }, { 0x1536, "TCPIPValidateIPCString(@cstringPtr):validFlag" }, { 0x1636, "TCPIPGetConnectData(userid,method):H" }, { 0x1736, "TCPIPSetConnectData(method,H)" },
	{ 0x1836, "TCPIPGetDisconnectData(userid,method):H" }, { 0x1936, "TCPIPSetDisconnectData(method,H)" }, { 0x1A36, "TCPIPLoadPreferences()" }, { 0x1B36, "TCPIPSavePreferences()" },
	{ 0x1C36, "TCPIPGetDNS(@DNSRecPtr)" }, { 0x1D36, "TCPIPSetDNS(@DNSRecPtr)" }, { 0x1E36, "TCPIPGetTuningTable(@tunePtr)" }, { 0x1F36, "TCPIPSetTuningTable(@tunePtr)" },
	{ 0x2036, "TCPIPCancelDNR(@dnrBufferPtr)" }, { 0x2136, "TCPIPDNRNameToIP(@nameptr,@dnrBufferPtr)" }, { 0x2236, "TCPIPPoll()" }, { 0x2336, "TCPIPLogin(userid,destip/4,destport,defaultTOS,defaultTTL):ipid" },
	{ 0x2436, "TCPIPLogout(ipid)" }, { 0x2536, "TCPIPSendICMP(ipid,@messagePtr,messageLen)" }, { 0x2636, "TCPIPSendUDP(ipid,@udpPtr,udpLen)" }, { 0x2736, "TCPIPGetDatagramCount(ipid,protocol):dgmCount" },
	{ 0x2836, "TCPIPGetNextDatagram(ipid,protocol,flags):H" }, { 0x2936, "TCPIPGetLoginCount():loginCount" }, { 0x2A36, "TCPIPSendICMPEcho(ipid,seqNum)" },
	{ 0x2B36, "TCPIPReceiveICMPEcho(ipid):seqNum" }, { 0x2C36, "TCPIPOpenTCP(ipid):tcpError" }, { 0x2D36, "TCPIPWriteTCP(ipid,@dataPtr,dataLength/4,pushFlag,urgentFlag):tcpError" },
	{ 0x2E36, "TCPIPReadTCP(ipid,buffType,buffData/4,buffLen/4,@rrBuffPtr):tcpError" }, { 0x2F36, "TCPIPCloseTCP(ipid):tcpError" }, { 0x3036, "TCPIPAbortTCP(ipid):tcpError" },
	{ 0x3136, "TCPIPStatusTCP(ipid,@srBuffPtr):tcpError" }, { 0x3236, "TCPIPGetSourcePort(ipid):sourcePort" }, { 0x3336, "TCPIPGetTOS(ipid):TOS" }, { 0x3436, "TCPIPSetTOS(ipid,TOS)" },
	{ 0x3536, "TCPIPGetTTL(ipid):TTL" }, { 0x3636, "TCPIPSetTTL(ipid,TTL)" }, { 0x3736, "TCPIPSetSourcePort(ipid,sourcePort)" }, { 0x3836, "TCPIPSetMyIPAddress(ipaddress/4)" },
	{ 0x3936, "TCPIPGetDP():dp" }, { 0x3A36, "TCPIPGetDebugHex():debugFlag" }, { 0x3B36, "TCPIPDebugHex(debugFlag)" }, { 0x3C36, "TCPIPGetDebugTCP():debugFlag" },
	{ 0x3D36, "TCPIPDebugTCP(debugFlag)" }, { 0x3E36, "TCPIPGetUserRecord(ipid):userRecEntry/4" }, { 0x3F36, "TCPIPConvertIPCToHex(@cvtRecPtr,@ddipcstring)" },
	{ 0x4036, "TCPIPSendIPDatagram(@datagramPtr)" }, { 0x4136, "TCPIPConvertIPToClass(ipaddress/4):class" }, { 0x4236, "TCPIPGetConnectMsgFlag():conMsgFlag" },
	{ 0x4336, "TCPIPSetConnectMsgFlag(conMsgFlag)" }, { 0x4436, "TCPIPGetUsername(@unBuffPtr)" }, { 0x4536, "TCPIPSetUsername(@usernamePtr)" }, { 0x4636, "TCPIPGetPassword(@pwBuffPtr)" },
	{ 0x4736, "TCPIPSetPassword(@passwordPtr)" }, { 0x4836, "TCPIPValidateIPString(@pstringPtr):validFlag" }, { 0x4936, "TCPIPGetUserStatistic(ipid,statisticNum):statistic/4" },{ 0x4A36, "TCPIPGetLinkVariables():@variablesPtr" },
	{ 0x4B36, "TCPIPEditLinkConfig(connectHandle/4,disconnectHandle/4)" }, { 0x4C36, "TCPIPGetModuleNames():@moduleListPtr" }, { 0x4D36, "TCPIPRebuildModuleList()" },
	{ 0x4E36, "TCPIPListenTCP(ipid):tcpError" }, { 0x4F36, "TCPIPAcceptTCP(ipid,reserved):newipid" }, { 0x5036, "TCPIPSetNewDestination(ipid,destip/4,destport)" },{ 0x5136, "TCPIPGetHostName(@hnBuffPtr)" },
	{ 0x5236, "TCPIPSetHostName(@hostNamePtr)" }, { 0x5336, "TCPIPStatusUDP(ipid,@udpVarsPtr)" }, { 0x5436, "TCPIPGetLinkLayer(@linkInfoBlkPtr)" }, { 0x5536, "TCPIPPtrToPtr(@from,@to,len/4)" },
	{ 0x5636, "TCPIPPtrToPtrNeg(@fromend,@toend,len/4)" },{ 0x5736, "TCPIPGetAuthMessage(userid):authMsgHandle/4" },
	{ 0x5836, "TCPIPConvertIPToCASCII(ipaddress/4,@ddcstring,flags):strlen" },{ 0x5936, "TCPIPMangleDomainName(flags,@dnPstringPtr):port" },
	{ 0x5A36, "TCPIPGetAliveFlag():aliveFlag" },{ 0x5B36, "TCPIPSetAliveFlag(aliveFlag)" },{ 0x5C36, "TCPIPGetAliveMinutes():aliveMinutes" },
	{ 0x5D36, "TCPIPSetAliveMinutes(aliveMinutes)" },{ 0x5E36, "TCPIPReadLineTCP(ipid,@delimitStrPtr,buffType,buffData/4,buffLen/4,@rrBuffPtr):tcpError" },
	{ 0x5F36, "TCPIPGetBootConnectFlag():bootConnectFlag" }, { 0x6036, "TCPIPSetBootConnectFlag(bootConnectFlag)" }, { 0x6136, "TCPIPSetUDPDispatch(ipid,dispatchFlag)" },
	{ 0x6236, "TCPIPGetDestination(ipid,@destRecPtr)" }, { 0x6336, "TCPIPGetUserEventTrigger(triggerNumber,ipid):triggerProcPtr/4" }, { 0x6436, "TCPIPSetUserEventTrigger(triggerNumber,ipid,@triggerProcPtr)" },
	{ 0x6536, "TCPIPGetSysEventTrigger(triggerNumber):triggerProcPtr/4" }, { 0x6636, "TCPIPSetSysEventTrigger(triggerNumber,@triggerProcPtr)" }, { 0x6736, "TCPIPGetDNRTimeouts(@dnrTimeoutsBuffPtr)" },
	{ 0x6836, "TCPIPSetDNRTimeouts(@dnrTimeoutsBuffPtr)" },

	{ 0xffff, "" }
};

static const struct dasm_data32 gs_vectors[] =
{
	{ 0xE10000, "System Tool dispatcher" }, { 0xE10004, "System Tool dispatcher, glue entry" }, { 0xE10008, "User Tool dispatcher" }, { 0xE1000C, "User Tool dispatcher, glue entry" },
	{ 0xE10010, "Interrupt mgr" }, { 0xE10014, "COP mgr" }, { 0xE10018, "Abort mgr" }, { 0xE1001C, "System Death mgr" }, { 0xE10020, "AppleTalk interrupt" },
	{ 0xE10024, "Serial interrupt" }, { 0xE10028, "Scanline interrupt" }, { 0xE1002C, "Sound interrupt" }, { 0xE10030, "VertBlank interrupt" }, { 0xE10034, "Mouse interrupt" },
	{ 0xE10038, "1/4 sec interrupt" }, { 0xE1003C, "Keyboard interrupt" }, { 0xE10040, "ADB Response byte int" }, { 0xE10044, "ADB SRQ int" }, { 0xE10048, "Desk Acc mgr" },
	{ 0xE1004C, "FlushBuffer handler" }, { 0xE10050, "KbdMicro interrupt" }, { 0xE10054, "1 sec interrupt" }, { 0xE10058, "External VGC int" }, { 0xE1005C, "other interrupt" },
	{ 0xE10060, "Cursor update" }, { 0xE10064, "IncBusy" }, { 0xE10068, "DecBusy" }, { 0xE1006C, "Bell vector" }, { 0xE10070, "Break vector" }, { 0xE10074, "Trace vector" },
	{ 0xE10078, "Step vector" }, { 0xE1007C, "[install ROMdisk]" }, { 0xE10080, "ToWriteBram" }, { 0xE10084, "ToReadBram" }, { 0xE10088, "ToWriteTime" },
	{ 0xE1008C, "ToReadTime" }, { 0xE10090, "ToCtrlPanel" }, { 0xE10094, "ToBramSetup" }, { 0xE10098, "ToPrintMsg8" }, { 0xE1009C, "ToPrintMsg16" }, { 0xE100A0, "Native Ctrl-Y" },
	{ 0xE100A4, "ToAltDispCDA" }, { 0xE100A8, "ProDOS 16 [inline parms]" }, { 0xE100AC, "OS vector" }, { 0xE100B0, "GS/OS(@parms,call) [stack parms]" },
	{ 0xE100B4, "OS_P8_Switch" }, { 0xE100B8, "OS_Public_Flags" }, { 0xE100BC, "OS_KIND (byte: 0=P8,1=P16)" }, { 0xE100BD, "OS_BOOT (byte)" }, { 0xE100BE, "OS_BUSY (bit 15=busy)" },
	{ 0xE100C0, "MsgPtr" }, { 0xe10135, "CURSOR" }, { 0xe10136, "NXTCUR" },
	{ 0xE10180, "ToBusyStrip" }, { 0xE10184, "ToStrip" }, { 0xe10198, "MDISPATCH" }, { 0xe1019c, "MAINSIDEPATCH" },
	{ 0xE101B2, "MidiInputPoll" }, { 0xE10200, "Memory Mover" }, { 0xE10204, "Set System Speed" },
	{ 0xE10208, "Slot Arbiter" }, { 0xE10220, "HyperCard IIgs callback" }, { 0xE10224, "WordForRTL" }, { 0xE11004, "ATLK: BASIC" }, { 0xE11008, "ATLK: Pascal" },
	{ 0xE1100C, "ATLK: RamGoComp" }, { 0xE11010, "ATLK: SoftReset" }, { 0xE11014, "ATLK: RamDispatch" }, { 0xE11018, "ATLK: RamForbid" }, { 0xE1101C, "ATLK: RamPermit" },
	{ 0xE11020, "ATLK: ProEntry" }, { 0xE11022, "ATLK: ProDOS" }, { 0xE11026, "ATLK: SerStatus" }, { 0xE1102A, "ATLK: SerWrite" }, { 0xE1102E, "ATLK: SerRead" },
	{ 0xE1103A, "ATLK: InitFileHook" }, { 0xE1103E, "ATLK: PFI Vector" }, { 0xE1D600, "ATLK: CmdTable" }, { 0xE1DA00, "ATLK: TickCount" },
	{ 0xE01D00, "BRegSave" }, { 0xE01D02, "IntStatus" }, { 0xE01D03, "SVStateReg" }, { 0xE01D04, "80ColSave" }, { 0xE01D05, "LoXClampSave" },
	{ 0xE01D07, "LoYClampSave" }, { 0xE01D09, "HiXClampSave" }, { 0xE01D0B, "HiYClampSave" }, { 0xE01D0D, "OutGlobals" }, { 0xE01D14, "Want40" },
	{ 0xE01D16, "CursorSave" }, { 0xE01D18, "NEWVIDSave" }, { 0xE01D1A, "TXTSave" }, { 0xE01D1B, "MIXSave" }, { 0xE01D1C, "PAGE2Save" },
	{ 0xE01D1D, "HIRESSave" }, { 0xE01D1E, "ALTCHARSave" }, { 0xE01D1F, "VID80Save" }, { 0xE01D20, "Int1AY" }, { 0xE01D2D, "Int1BY" },
	{ 0xE01D39, "Int2AY" }, { 0xE01D4C, "Int2BY" }, { 0xE01D61, "MOUSVBLSave" }, { 0xE01D63, "DirPgSave" }, { 0xE01D65, "C3ROMSave" },
	{ 0xE01D66, "Save4080" }, { 0xE01D67, "NumInts" }, { 0xE01D68, "MMode" }, { 0xE01D6A, "MyMSLOT" }, { 0xE01D6C, "Slot" },
	{ 0xE01D6E, "EntryCount" }, { 0xE01D70, "BottomLine" }, { 0xE01D72, "HPos" }, { 0xE01D74, "VPos" }, { 0xE01D76, "CurScreenLoc" },
	{ 0xE01D7C, "NumDAs" }, { 0xE01D7E, "LeftBorder" }, { 0xE01D80, "FirstMenuItem" }, { 0xE01D82, "IDNum" }, { 0xE01D84, "CDATabHndl" },
	{ 0xE01D88, "RoomLeft" }, { 0xE01D8A, "KeyInput" }, { 0xE01D8C, "EvntRec" }, { 0xE01D8E, "Message" }, { 0xE01D92, "When" },
	{ 0xE01D96, "Where" }, { 0xE01D9A, "Mods" }, { 0xE01D9C, "StackSave" }, { 0xE01D9E, "OldOutGlobals" }, { 0xE01DA2, "OldOutDevice" },
	{ 0xE01DA8, "CDataBPtr" }, { 0xE01DAC, "DAStrPtr" }, { 0xE01DB0, "CurCDA" }, { 0xE01DB2, "OldOutHook" }, { 0xE01DB4, "OldInDev" },
	{ 0xE01DBA, "OldInGlob" }, { 0xE01DBE, "RealDeskStat" }, { 0xE01DC0, "Next" }, { 0xE01DDE, "SchActive" }, { 0xE01DDF, "TaskQueue" },
	{ 0xE01DDF, "FirstTask" }, { 0xE01DE3, "SecondTask" }, { 0xE01DED, "Scheduler" }, { 0xE01DEF, "Offset" }, { 0xE01DFF, "Lastbyte" },
	{ 0xE01E04, "QD:StdText" }, { 0xE01E08, "QD:StdLine" }, { 0xE01E0C, "QD:StdRect" }, { 0xE01E10, "QD:StdRRect" }, { 0xE01E14, "QD:StdOval" }, { 0xE01E18, "QD:StdArc" }, { 0xE01E1C, "QD:StdPoly" },
	{ 0xE01E20, "QD:StdRgn" }, { 0xE01E24, "QD:StdPixels" }, { 0xE01E28, "QD:StdComment" }, { 0xE01E2C, "QD:StdTxMeas" }, { 0xE01E30, "QD:StdTxBnds" }, { 0xE01E34, "QD:StdGetPic" },
	{ 0xE01E38, "QD:StdPutPic" }, { 0xE01E98, "QD:ShieldCursor" }, { 0xE01E9C, "QD:UnShieldCursor" },
	{ 0x010100, "MNEMSTKPTR" }, { 0x010101, "ALEMSTKPTR" }, { 0x01FC00, "SysSrv:DEV_DISPATCHER" }, { 0x01FC04, "SysSrv:CACHE_FIND_BLK" }, { 0x01FC08, "SysSrv:CACHE_ADD_BLK" },
	{ 0x01FC0C, "SysSrv:CACHE_INIT" }, { 0x01FC10, "SysSrv:CACHE_SHUTDN" }, { 0x01FC14, "SysSrv:CACHE_DEL_BLK" }, { 0x01FC18, "SysSrv:CACHE_DEL_VOL" },
	{ 0x01FC1C, "SysSrv:ALLOC_SEG" }, { 0x01FC20, "SysSrv:RELEASE_SEG" }, { 0x01FC24, "SysSrv:ALLOC_VCR" }, { 0x01FC28, "SysSrv:RELEASE_VCR" },
	{ 0x01FC2C, "SysSrv:ALLOC_FCR" }, { 0x01FC30, "SysSrv:RELEASE_FCR" }, { 0x01FC34, "SysSrv:SWAP_OUT" }, { 0x01FC38, "SysSrv:DEREF" },
	{ 0x01FC3C, "SysSrv:GET_SYS_GBUF" }, { 0x01FC40, "SysSrv:SYS_EXIT" }, { 0x01FC44, "SysSrv:SYS_DEATH" }, { 0x01FC48, "SysSrv:FIND_VCR" },
	{ 0x01FC4C, "SysSrv:FIND_FCR" }, { 0x01FC50, "SysSrv:SET_SYS_SPEED" }, { 0x01FC54, "SysSrv:CACHE_FLSH_DEF" }, { 0x01FC58, "SysSrv:RENAME_VCR" },
	{ 0x01FC5C, "SysSrv:RENAME_FCR" }, { 0x01FC60, "SysSrv:GET_VCR" }, { 0x01FC64, "SysSrv:GET_FCR" }, { 0x01FC68, "SysSrv:LOCK_MEM" },
	{ 0x01FC6C, "SysSrv:UNLOCK_MEM" }, { 0x01FC70, "SysSrv:MOVE_INFO" }, { 0x01FC74, "SysSrv:CVT_0TO1" }, { 0x01FC78, "SysSrv:CVT_1TO0" },
	{ 0x01FC7C, "SysSrv:REPLACE80" }, { 0x01FC80, "SysSrv:TO_B0_CORE" }, { 0x01FC84, "SysSrv:G_DISPATCH" }, { 0x01FC88, "SysSrv:SIGNAL" },
	{ 0x01FC8C, "SysSrv:GET_SYS_BUFF" }, { 0x01FC90, "SysSrv:SET_DISK_SW" }, { 0x01FC94, "SysSrv:REPORT_ERROR" }, { 0x01FC98, "SysSrv:MOUNT_MESSAGE" },
	{ 0x01FC9C, "SysSrv:FULL_ERROR" }, { 0x01FCA0, "SysSrv:RESERVED_07" }, { 0x01FCA4, "SysSrv:SUP_DRVR_DISP" }, { 0x01FCA8, "SysSrv:INSTALL_DRIVER" },
	{ 0x01FCAC, "SysSrv:S_GET_BOOT_PFX" },  { 0x01FCB0, "SysSrv:S_SET_BOOT_PFX" }, { 0x01FCB4, "SysSrv:LOW_ALLOCATE" },
	{ 0x01FCB8, "SysSrv:GET_STACKED_ID" }, { 0x01FCBC, "SysSrv:DYN_SLOT_ARBITER" }, { 0x01FCC0, "SysSrv:PARSE_PATH" },
	{ 0x01FCC4, "SysSrv:OS_EVENT" }, { 0x01FCC8, "SysSrv:INSERT_DRIVER" }, { 0x01FCCC, "SysSrv:(device manager?)" },
	{ 0x01FCD0, "SysSrv:Old Device Dispatcher" }, { 0x01FCD4, "SysSrv:INIT_PARSE_PATH" }, { 0x01FCD8, "SysSrv:UNBIND_INT_VEC" },
	{ 0x01FCDC, "SysSrv:DO_INSERT_SCAN" }, { 0x01FCE0, "SysSrv:TOOLBOX_MSG" },

	{ 0xffff, "" }
};

offs_t apple2_common_device::com_2byte_op(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname)
{
	int item = 0;

	if (m_GScpu)
	{
		if (m_GScpu->g65816_get_reg(g65816_device::G65816_D) != 0)
		{
			return 0;
		}
	}

	u16 operand = (u16)opcodes.r8(pc+1);
	while ((a2_stuff[item].addr & 0xff00) == 0)
	{
		if (a2_stuff[item].addr == operand)
		{
			stream << opname << " <" << a2_stuff[item].name;
			return 2 | util::disasm_interface::SUPPORTED;
		}
		item++;
	}

	return 0;
}

offs_t apple2_common_device::com_3byte_op(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname)
{
	int item = 0;
	u16 operand = opcodes.r16(pc+1);

	// check for GS-specific stores with the databank set
	if (m_GScpu)
	{
		u32 bank = m_GScpu->g65816_get_reg(g65816_device::G65816_DB) << 16;
		bank |= operand;

		int item = 0;
		while (gs_vectors[item].addr != 0xffff)
		{
			if (gs_vectors[item].addr == bank)
			{
				stream << opname << " " << gs_vectors[item].name;
				return 3 | util::disasm_interface::SUPPORTED;
			}
			item++;
		}

		// if we failed the GS stuff and the DB isn't a special bank, don't match the classic A2 switches.
		bank >>= 16;
		bank &= 0xff;
		if ((bank != 0) && (bank != 1) && (bank != 0xe0) && (bank != 0xe1))
		{
			return 0;
		}
	}

	while (a2_stuff[item].addr != 0xffff)
	{
		if (a2_stuff[item].addr == operand)
		{
			stream << opname << " " << a2_stuff[item].name;
			return 3 | util::disasm_interface::SUPPORTED;
		}
		item++;
	}

	return 0;
}

offs_t apple2_common_device::com_long_op(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname)
{
	int item = 0;
	u32 operand = opcodes.r32(pc) >> 8;
	while (gs_vectors[item].addr != 0xffff)
	{
		if (gs_vectors[item].addr == operand)
		{
			stream << opname << " >" << gs_vectors[item].name;
			return 4 | util::disasm_interface::SUPPORTED;
		}
		item++;
	}

	return 0;
}

offs_t apple2_common_device::dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	u8 opcode;
	unsigned result = 0;

	opcode = opcodes.r8(pc);

	switch (opcode)
	{
		case 0x0d: // ORA
			return com_3byte_op(stream, pc, opcodes, "ora");

		case 0xad: // LDA
			return com_3byte_op(stream, pc, opcodes, "lda");

		case 0x8d:   // STA
			return com_3byte_op(stream, pc, opcodes, "sta");

		case 0x2c:   // BIT
			return com_3byte_op(stream, pc, opcodes, "bit");

		case 0x9c:  // STZ
			return com_3byte_op(stream, pc, opcodes, "stz");

		case 0x20:   // JSR
			{
				u16 operand = opcodes.r16(pc + 1);
				if (operand == 0xbf00)
				{
					u8 p8call = opcodes.r8(pc + 3);
					u16 p8params = opcodes.r16(pc + 4);
					int item = 0;

					while (p8_calls[item].addr != 0xffff)
					{
						if (p8_calls[item].addr == p8call)
						{
							util::stream_format(stream, "jsr ProDOS 8: %s ($%04x)", p8_calls[item].name, p8params);
							return 6 | util::disasm_interface::SUPPORTED | util::disasm_interface::STEP_OVER;
						}
						item++;
					}
				}
				else
				{
					offs_t rv = com_3byte_op(stream, pc, opcodes, "jsr");

					if (rv > 0)
					{
						rv |= util::disasm_interface::STEP_OVER;
					}
					return rv;
				}
			}
			break;

		case 0x4c:   // JMP
			return com_3byte_op(stream, pc, opcodes, "jmp");

		case 0x84:  // STY ZP
			return com_2byte_op(stream, pc, opcodes, "sty");

		case 0x85:  // STA ZP
			return com_2byte_op(stream, pc, opcodes, "sta");

		case 0xa4:  // LDY ZP
			return com_2byte_op(stream, pc, opcodes, "ldy");

		case 0xa5:  // LDA ZP
			return com_2byte_op(stream, pc, opcodes, "lda");

		case 0x65:  // ADC ZP
			return com_2byte_op(stream, pc, opcodes, "adc");

		case 0xc5:  // CMP ZP
			return com_2byte_op(stream, pc, opcodes, "cmp");

		case 0xc6:  // DEC ZP
			return com_2byte_op(stream, pc, opcodes, "dec");

		case 0xe6:  // INC ZP
			return com_2byte_op(stream, pc, opcodes, "inc");

		default:
			break;
	}

	return result;
}

offs_t apple2_common_device::dasm_override_GS(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	u8 opcode;
	unsigned result = 0;

	assert(m_GScpu);    // ensure the IIgs CPU was configured if we're using this with a IIgs

	opcode = opcodes.r8(pc);

	switch (opcode)
	{
		case 0x0c:   // TSB
			return com_3byte_op(stream, pc, opcodes, "tsb");

		case 0xa2:  // LDX
			if (opcodes.r32(pc + 3) == 0xe1000022)  // JSL E10000
			{
				u16 toolcall = opcodes.r16(pc + 1);
				int item = 0;
				while (gs_tools[item].addr != 0xffff)
				{
					if (gs_tools[item].addr == toolcall)
					{
						util::stream_format(stream, "_%s (%04x)", gs_tools[item].name, toolcall);
						return 7 | util::disasm_interface::SUPPORTED | util::disasm_interface::STEP_OVER;
					}
					item++;
				}
			}
			else
			{
				u32 vec = opcodes.r32(pc) >> 8;
				int item = 0;
				while (gs_vectors[item].addr != 0xffff)
				{
					if (gs_tools[item].addr == vec)
					{
						util::stream_format(stream, "jsl %s (%06x)", gs_tools[item].name, vec);
						return 4 | util::disasm_interface::SUPPORTED | util::disasm_interface::STEP_OVER;
					}
					item++;
				}
			}
			break;

		case 0x8f:  // STA long
			return com_long_op(stream, pc, opcodes, "sta");

		// on IIgs, this is more likely to refer to some non-Monitor direct page, so don't do these that way
		// (we need a m_maincpu so we can check the D register for a smarter version of this)
		case 0x84:  // STY ZP
		case 0x85:  // STA ZP
		case 0xa4:  // LDY ZP
		case 0xa5:  // LDA ZP
		case 0x65:  // ADC ZP
		case 0xc5:  // CMP ZP
		case 0xc6:  // DEC ZP
		case 0xe6:  // INC ZP
			return dasm_override(stream, pc, opcodes, params);


		default:
			break;
	}

	if (!result)
	{
		return dasm_override(stream, pc, opcodes, params);
	}

	return result;
}
