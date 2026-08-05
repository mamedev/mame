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

template <typename T>
const T* apple2_common_device::find_symbol(const T* symbols, const T* end, decltype(T{}.addr) address)
{
	const T* symbol = std::lower_bound(
		symbols,
		end,
		address,
		[](const T& sym, const auto& addr) { return sym.addr < addr; });

	if (symbol != end && symbol->addr == address)
		return symbol;
	return nullptr;
}

static const struct dasm_data p8_calls[] =
{
	{ 0x40, "ALLOC_INTERRUPT" }, { 0x41, "DEALLOC_INTERRUPT" },
	{ 0x65, "QUIT" }, { 0x80, "READ_BLOCK" }, { 0x81, "WRITE_BLOCK" }, { 0x82, "GET_TIME" },
	{ 0x99, "ATINIT" }, { 0xc0, "CREATE" }, { 0xc1, "DESTROY" }, { 0xc2, "RENAME" }, { 0xc3, "SET_FILE_INFO" },
	{ 0xc4, "GET_FILE_INFO" }, { 0xc5, "ONLINE" }, { 0xc6, "SET_PREFIX" }, { 0xc7, "GET_PREFIX" }, { 0xc8, "OPEN" },
	{ 0xc9, "NEWLINE" }, { 0xca, "READ" }, { 0xcb, "WRITE" }, { 0xcc, "CLOSE" }, { 0xcd, "FLUSH" }, { 0xce, "SET_MARK" },
	{ 0xcf, "GET_MARK" }, { 0xd0, "SET_EOF" }, { 0xd1, "GET_EOF" }, { 0xd2, "SET_BUF" }, { 0xd3, "GET_BUF" },
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
	{ 0x0067, "TXTTAB" }, { 0x0069, "VARTAB" }, { 0x006b, "ARYTAB" }, { 0x006d, "STREND" },
	{ 0x006f, "FRETOP" }, { 0x0071, "FRESPC" }, { 0x0073, "MEMSIZ" }, { 0x0075, "CURLIN" },
	{ 0x0077, "OLDLIN" }, { 0x0079, "OLDTEXT" }, { 0x007b, "DATLIN" }, { 0x007d, "DATPTR" },
	{ 0x007f, "INPTR" }, { 0x0081, "VARNAM" }, { 0x0083, "VARPNT" }, { 0x0085, "FORPNT" },
	{ 0x009A, "EXPON" }, { 0x009C, "EXPSGN" }, { 0x009d, "FAC" }, { 0x00A2, "FAC.SIGN" },
	{ 0x00a5, "ARG" }, { 0x00AA, "ARG.SIGN" }, { 0x00af, "PRGEND" }, { 0x00B8, "TXTPTR" },
	{ 0x00C9, "RNDSEED" }, { 0x00D6, "LOCK" }, { 0x00D8, "ERRFLG" }, { 0x00DA, "ERRLIN" },
	{ 0x00DE, "ERRNUM" }, { 0x00E4, "HGR.COLOR" }, { 0x00E6, "HGR.PAGE" }, { 0x00F1, "SPEEDZ" },

	{ 0xc000, "KBD/80STOREOFF" }, { 0xc001, "80STOREON" }, { 0xc002, "RDMAINRAM" }, { 0xc003, "RDCARDRAM" }, { 0xc004, "WRMAINRAM" },
	{ 0xc005, "WRCARDRAM" }, { 0xc006, "SETSLOTCXROM" }, { 0xc007, "SETINTCXROM" }, { 0xc008, "SETSTDZP" },
	{ 0xc009, "SETALTZP" }, { 0xc00a, "SETINTC3ROM" }, { 0xc00b, "SETSLOTC3ROM" }, { 0xc00c, "CLR80VID" },
	{ 0xc00d, "SET80VID" }, { 0xc00e, "CLRALTCHAR" }, { 0xc00f, "SETALTCHAR" }, { 0xc010, "KBDSTRB" },
	{ 0xc011, "RDLCBNK2" }, { 0xc012, "RDLCRAM" }, { 0xc013, "RDRAMRD" }, { 0xc014, "RDRAMWRT" },
	{ 0xc015, "RDCXROM/RSTXINT" }, { 0xc016, "RDALTZP" }, { 0xc017, "RDC3ROM/RSTYINT" }, { 0xc018, "RD80STORE" },
	{ 0xc019, "RDVBL/RSTVBL" }, { 0xc01a, "RDTEXT" }, { 0xc01b, "RDMIXED" }, { 0xc01c, "RDPAGE2" },
	{ 0xc01d, "RDHIRES" }, { 0xc01e, "RDALTCHAR" }, { 0xc01f, "RD80VID" }, { 0xc020, "TAPEOUT" },
	{ 0xc021, "MONOCOLOR" }, { 0xc022, "TBCOLOR" }, { 0xc023, "VGCINT" }, { 0xc024, "MOUSEDATA" },
	{ 0xc025, "KEYMODREG" }, { 0xc026, "DATAREG" }, { 0xc027, "KMSTATUS" }, { 0xc028, "ROMBANK" },
	{ 0xc029, "NEWVIDEO" }, { 0xc02b, "LANGSEL" }, { 0xc02c, "CHARROM" }, { 0xc02d, "SLTROMSEL" },
	{ 0xc02e, "VERTCNT" }, { 0xc02f, "HORIZCNT" }, { 0xc030, "SPKR" }, { 0xc031, "DISKREG" },
	{ 0xc032, "SCANINT" }, { 0xc033, "CLOCKDATA" }, { 0xc034, "CLOCKCTL" }, { 0xc035, "SHADOW" },
	{ 0xc036, "FPIREG/CYAREG" }, { 0xc037, "DMAREG" }, { 0xc038, "SCCBREG" }, { 0xc039, "SCCAREG" },
	{ 0xc03a, "SCCBDATA" }, { 0xc03b, "SCCADATA" }, { 0xc03c, "SOUNDCTL" }, { 0xc03d, "SOUNDDATA" },
	{ 0xc03e, "SOUNDADRL" }, { 0xc03f, "SOUNDADRH" }, { 0xc040, "STROBE/RDXYMSK" }, { 0xc041, "RDVBLMSK/INTEN" },
	{ 0xc042, "RDX0EDGE" }, { 0xc043, "RDY0EDGE" }, { 0xc044, "MMDELTAX" }, { 0xc045, "MMDELTAY" },
	{ 0xc046, "DIAGTYPE/INTFLAG" }, { 0xc047, "CLRVBLINT" }, { 0xc048, "CLRXYINT/RSTXY" }, { 0xc04f, "EMUBYTE" },
	{ 0xc050, "TXTCLR" }, { 0xc051, "TXTSET" }, { 0xc052, "MIXCLR" }, { 0xc053, "MIXSET" }, { 0xc054, "TXTPAGE1" },
	{ 0xc055, "TXTPAGE2" }, { 0xc056, "LORES" }, { 0xc057, "HIRES" }, { 0xc058, "CLRAN0/DISXY" }, { 0xc059, "SETAN0/ENBXY" },
	{ 0xc05a, "CLRAN1/DISVBL" }, { 0xc05b, "SETAN1/ENVBL" }, { 0xc05c, "CLRAN2/X0EDGE" }, { 0xc05d, "SETAN2/X0EDGE" },
	{ 0xc05e, "DHIRESON/Y0EDGE" }, { 0xc05f, "DHIRESOFF/Y0EDGE" }, { 0xc060, "TAPEIN/BUTN3" }, { 0xc061, "BUTN0" },
	{ 0xc062, "BUTN1" }, { 0xc063, "BUTN2" }, { 0xc064, "PADDL0" }, { 0xc065, "PADDL1" }, { 0xc066, "PADDL2/RDMOUX1" },
	{ 0xc067, "PADDL3/RDMOUY1" }, { 0xc068, "STATEREG" }, { 0xc06d, "TESTREG" }, { 0xc070, "PTRIG" }, { 0xc073, "BANKSEL" },
	{ 0xc07e, "IOUDISON/RDIOUDIS" }, { 0xc07f, "IOUDISOFF/RDDHIRES" }, { 0xc081, "ROMIN" }, { 0xc083, "LCBANK2" },
	{ 0xc085, "ROMIN" }, { 0xc087, "LCBANK2" }, { 0xc08b, "LCBANK1" }, { 0xcfff, "CLRROM" },

	{ 0xF800, "F8ROM:PLOT" }, { 0xF80E, "F8ROM:PLOT1" }, { 0xF819, "F8ROM:HLINE" }, { 0xF828, "F8ROM:VLINE" },
	{ 0xF832, "F8ROM:CLRSCR" }, { 0xF836, "F8ROM:CLRTOP" }, { 0xF838, "F8ROM:CLRSC2" }, { 0xF847, "F8ROM:GBASCALC" },
	{ 0xF856, "F8ROM:GBCALC" }, { 0xF85F, "F8ROM:NXTCOL" }, { 0xF864, "F8ROM:SETCOL" }, { 0xF871, "F8ROM:SCRN" },
	{ 0xF882, "F8ROM:INSDS1" }, { 0xF88E, "F8ROM:INSDS2" }, { 0xF8A5, "F8ROM:ERR" }, { 0xF8A9, "F8ROM:GETFMT" },
	{ 0xF8D0, "F8ROM:INSTDSP" }, { 0xF940, "F8ROM:PRNTYX" }, { 0xF941, "F8ROM:PRNTAX" }, { 0xF944, "F8ROM:PRNTX" },
	{ 0xF948, "F8ROM:PRBLNK" }, { 0xF94A, "F8ROM:PRBL2" }, { 0xF94C, "F8ROM:PRBL3" }, { 0xF953, "F8ROM:PCADJ" },
	{ 0xF954, "F8ROM:PCADJ2" }, { 0xF956, "F8ROM:PCADJ3" }, { 0xF95C, "F8ROM:PCADJ4" }, { 0xF962, "F8ROM:FMT1" },
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
};

static const struct dasm_data gs_tools[] =
{
	// the routine,tool numbers have been byte-swapped here for binary search
	{ 0x0101, "TLBootInit()" }, { 0x0102, "TLStartUp()" }, { 0x0103, "TLShutDown()" }, { 0x0104, "TLVersion():Vers" }, { 0x0105, "TLReset()" },
	{ 0x0106, "TLStatus():ActFlg" }, { 0x0109, "GetTSPtr(SysFlg,TS#):@FPT" }, { 0x010A, "SetTSPtr(SysFlg,TS#,@FPT)" }, { 0x010B, "GetFuncPtr(SysFlg,Func):@Func" },
	{ 0x010C, "GetWAP(SysFlg,TS#):@WAP" }, { 0x010D, "SetWAP(SysFlg,TS#,@WAP)" }, { 0x010E, "LoadTools(@ToolTable)" }, { 0x010F, "LoadOneTool(TS#,MinVers)" },
	{ 0x0110, "UnloadOneTool(TS#)" }, { 0x0111, "TLMountVolume(X,Y,@L1,@L2,@B1,@B2):Btn#" }, { 0x0112, "TLTextMountVolume(@L1,@L2,@B1,@B2):Btn#" },
	{ 0x0113, "SaveTextState():StateH" }, { 0x0114, "RestoreTextState(StateH)" }, { 0x0115, "MessageCenter(Action,Type,MsgH)" }, { 0x0116, "SetDefaultTPT()" },
	{ 0x0117, "MessageByName(CreateF,@inpRec):Created,Type" }, { 0x0118, "StartUpTools(MemID,ssDesc,ssRef/4):ssRef/4" }, { 0x0119, "ShutDownTools(ssDesc,ssRef/4)" },
	{ 0x011A, "GetMsgHandle(Flags,MsgRef/4):H" }, { 0x011B, "AcceptRequests(@NameStr,UserID,@ReqProc)" }, { 0x011C, "SendRequest(ReqCode,How,Target/4,@In,@Out)" },
	{ 0x0201, "MMBootInit()" }, { 0x0202, "MMStartUp():MemID" }, { 0x0203, "MMShutDown(MemID)" }, { 0x0204, "MMVersion():Vers" }, { 0x0205, "MMReset()" },
	{ 0x0206, "MMStatus():ActFlg" }, { 0x0209, "NewHandle(Size/4,MemID,Attr,@loc):H" }, { 0x020A, "ReAllocHandle(Size/4,MemID,Attr,@loc,H)" }, { 0x020B, "RestoreHandle(H)" },
	{ 0x020C, "AddToOOMQueue(@header)" }, { 0x020D, "RemoveFromOOMQueue(@header)" }, { 0x0210, "DisposeHandle(H)" }, { 0x0211, "DisposeAll(MemID)" },
	{ 0x0212, "PurgeHandle(H)" }, { 0x0213, "PurgeAll(MemID)" }, { 0x0218, "GetHandleSize(H):Size/4" }, { 0x0219, "SetHandleSize(Size/4,H)" }, { 0x021A, "FindHandle(@byte):H" },
	{ 0x021B, "FreeMem():FreeBytes/4" }, { 0x021C, "MaxBlock():Size/4" }, { 0x021D, "TotalMem():Size/4" }, { 0x021E, "CheckHandle(H)" }, { 0x021F, "CompactMem()" },
	{ 0x0220, "HLock(H)" }, { 0x0221, "HLockAll(MemID)" }, { 0x0222, "HUnlock(H)" }, { 0x0223, "HUnlockAll(MemID)" }, { 0x0224, "SetPurge(PrgLvl,H)" },
	{ 0x0225, "SetPurgeAll(PrgLvl,MemID)" }, { 0x0228, "PtrToHand(@Src,DestH,Count/4)" }, { 0x0229, "HandToPtr(SrcH,@Dest,Count/4)" }, { 0x022A, "HandToHand(SrcH,DestH,Count/4)" },
	{ 0x022B, "BlockMove(@Source,@Dest,Count/4)" }, { 0x022F, "RealFreeMem():Size/4" }, { 0x0230, "SetHandleID(newMemID,theH):oldMemID" }, { 0x0301, "MTBootInit()" },
	{ 0x0302, "MTStartUp()" }, { 0x0303, "MTShutDown()" }, { 0x0304, "MTVersion():Vers" }, { 0x0305, "MTReset()" }, { 0x0306, "MTStatus():ActFlg" },
	{ 0x0309, "WriteBRam(@Buff)" }, { 0x030A, "ReadBRam(@Buff)" }, { 0x030B, "WriteBParam(Data,Parm#)" }, { 0x030C, "ReadBParam(Parm#):Data" }, { 0x030D, "ReadTimeHex():WkDay,Mn&Dy,Yr&Hr,Mn&Sec" },
	{ 0x030E, "WriteTimeHex(Mn&Dy,Yr&Hr,Mn&Sec)" }, { 0x030F, "ReadAsciiTime(@Buff)" }, { 0x0310, "SetVector(Vec#,@x)" }, { 0x0311, "GetVector(Vec#):@x" },
	{ 0x0312, "SetHeartBeat(@Task)" }, { 0x0313, "DelHeartBeat(@Task)" }, { 0x0314, "ClrHeartBeat()" }, { 0x0315, "SysFailMgr(Code,@Msg)" }, { 0x0316, "GetAddr(Ref#):@Parm" },
	{ 0x0317, "ReadMouse():X,Y,Stat&Mode" }, { 0x0318, "InitMouse(Slot)" }, { 0x0319, "SetMouse(Mode)" }, { 0x031A, "HomeMouse()" }, { 0x031B, "ClearMouse()" },
	{ 0x031C, "ClampMouse(Xmn,Xmx,Ymn,Ymx)" }, { 0x031D, "GetMouseClamp():Xmn,Xmx,Ymn,Ymx" }, { 0x031E, "PosMouse(X,Y)" }, { 0x031F, "ServeMouse():IntStat" },
	{ 0x0320, "GetNewID(Kind):MemID" }, { 0x0321, "DeleteID(MemID)" }, { 0x0322, "StatusID(MemID)" }, { 0x0323, "IntSource(Ref#)" }, { 0x0324, "FWEntry(A,X,Y,Address):P,A,X,Y" },
	{ 0x0325, "GetTick():Ticks/4" }, { 0x0326, "PackBytes(@StartPtr,@Sz,@OutBf,OutSz):Size" }, { 0x0327, "UnPackBytes(@Buff,BfSz,@StartPtr,@Sz):Size" },
	{ 0x0328, "Munger(@Dst,@DstL,@t,tL,@Rpl,RplL,@Pad):N" }, { 0x0329, "GetIRQEnable():IntStat" }, { 0x032A, "SetAbsClamp(Xmn,Xmx,Ymn,Ymx)" },
	{ 0x032B, "GetAbsClamp():Xmn,Xmx,Ymn,Ymx" }, { 0x032C, "SysBeep()" }, { 0x032E, "AddToQueue(@newTask,@queueHeader)" }, { 0x032F, "DeleteFromQueue(@task,@queueHeader)" },
	{ 0x0330, "SetInterruptState(@stateRec,NumBytes)" }, { 0x0331, "GetInterruptState(@stateRec,NumBytes)" }, { 0x0332, "GetIntStateRecSize():Size" },
	{ 0x0333, "ReadMouse2():xPos,yPos,StatMode" }, { 0x0334, "GetCodeResConverter():@proc" }, { 0x0335, "GetROMResource(?,?/4):???H" },
	{ 0x0336, "ReleaseROMResource(?,?/4)" }, { 0x0337, "ConvSeconds(convVerb,Secs/4,@Date):SecondsOut/4" }, { 0x0338, "SysBeep2(beepKind)" },
	{ 0x0339, "VersionString(flags,Version/4,@Buffer)" }, { 0x033A, "WaitUntil(WaitFromTime,DelayTime):NewTime" }, { 0x033B, "StringToText(flags,@String,StrLen,@Buffer):ResFlags,PrntLen" },
	{ 0x033C, "ShowBootInfo(@String,@Icon)" }, { 0x033D, "ScanDevices():DevNum" }, { 0x033E, "AlertMessage(@Table,MsgNum,@Subs):Button" }, { 0x033F, "DoSysPrefs(bitsToClear,bitsToSet):SysPrefs" },
	{ 0x0401, "QDBootInit()" }, { 0x0402, "QDStartUp(DirPg,MastSCB,MaxWid,MemID)" }, { 0x0403, "QDShutDown()" }, { 0x0404, "QDVersion():Vers" }, { 0x0405, "QDReset()" },
	{ 0x0406, "QDStatus():ActFlg" }, { 0x0409, "GetAddress(what):@Table" }, { 0x040A, "GrafOn()" }, { 0x040B, "GrafOff()" }, { 0x040C, "GetStandardSCB():SCB" },
	{ 0x040D, "InitColorTable(@Table)" }, { 0x040E, "SetColorTable(Tab#,@SrcTab)" }, { 0x040F, "GetColorTable(Tab#,@DestTbl)" }, { 0x0410, "SetColorEntry(Tab#,Ent#,NewCol)" },
	{ 0x0411, "GetColorEntry(Tab#,Ent#):Color" }, { 0x0412, "SetSCB(Line#,SCB)" }, { 0x0413, "GetSCB(Line#):SCB" }, { 0x0414, "SetAllSCBs(SCB)" }, { 0x0415, "ClearScreen(Color)" },
	{ 0x0416, "SetMasterSCB(SCB)" }, { 0x0417, "GetMasterSCB():SCB" }, { 0x0418, "OpenPort(@Port)" }, { 0x0419, "InitPort(@Port)" }, { 0x041A, "ClosePort(@Port)" },
	{ 0x041B, "SetPort(@Port)" }, { 0x041C, "GetPort():@Port" }, { 0x041D, "SetPortLoc(@LocInfo)" }, { 0x041E, "GetPortLoc(@LocInfo)" }, { 0x041F, "SetPortRect(@Rect)" },
	{ 0x0420, "GetPortRect(@Rect)" }, { 0x0421, "SetPortSize(w,h)" }, { 0x0422, "MovePortTo(h,v)" }, { 0x0423, "SetOrigin(h,v)" }, { 0x0424, "SetClip(RgnH)" },
	{ 0x0425, "GetClip(RgnH)" }, { 0x0426, "ClipRect(@Rect)" }, { 0x0427, "HidePen()" }, { 0x0428, "ShowPen()" }, { 0x0429, "GetPen(@Pt)" }, { 0x042A, "SetPenState(@PenSt)" },
	{ 0x042B, "GetPenState(@PenSt)" }, { 0x042C, "SetPenSize(w,h)" }, { 0x042D, "GetPenSize(@Pt)" }, { 0x042E, "SetPenMode(Mode)" }, { 0x042F, "GetPenMode():Mode" }, { 0x0430, "SetPenPat(@Patt)" },
	{ 0x0431, "GetPenPat(@Patt)" }, { 0x0432, "SetPenMask(@Mask)" }, { 0x0433, "GetPenMask(@Mask)" }, { 0x0434, "SetBackPat(@Patt)" }, { 0x0435, "GetBackPat(@Patt)" },
	{ 0x0436, "PenNormal()" }, { 0x0437, "SetSolidPenPat(Color)" }, { 0x0438, "SetSolidBackPat(Color)" }, { 0x0439, "SolidPattern(Color,@Patt)" }, { 0x043A, "MoveTo(h,v)" },
	{ 0x043B, "Move(dh,dv)" }, { 0x043C, "LineTo(h,v)" }, { 0x043D, "Line(dh,dv)" }, { 0x043E, "SetPicSave(Val/4)" }, { 0x043F, "GetPicSave():Val/4" }, { 0x0440, "SetRgnSave(Val/4)" },
	{ 0x0441, "GetRgnSave():Val/4" }, { 0x0442, "SetPolySave(Val/4)" }, { 0x0443, "GetPolySave():Val/4" }, { 0x0444, "SetGrafProcs(@GrafProcs)" }, { 0x0445, "GetGrafProcs():@GrafProcs" },
	{ 0x0446, "SetUserField(Val/4)" }, { 0x0447, "GetUserField():Val/4" }, { 0x0448, "SetSysField(Val/4)" }, { 0x0449, "GetSysField():Val/4" }, { 0x044A, "SetRect(@Rect,left,top,right,bot)" },
	{ 0x044B, "OffsetRect(@Rect,dh,dv)" }, { 0x044C, "InsetRect(@Rect,dh,dv)" }, { 0x044D, "SectRect(@R1,@R2,@DstR):nonEmptyF" }, { 0x044E, "UnionRect(@Rect1,@Rect2,@UnionRect)" },
	{ 0x044F, "PtInRect(@Pt,@Rect):Flag" }, { 0x0450, "Pt2Rect(@Pt1,@Pt2,@Rect)" }, { 0x0451, "EqualRect(@Rect1,@Rect2):Flag" }, { 0x0452, "NotEmptyRect(@Rect):Flag" },
	{ 0x0453, "FrameRect(@Rect)" }, { 0x0454, "PaintRect(@Rect)" }, { 0x0455, "EraseRect(@Rect)" }, { 0x0456, "InvertRect(@Rect)" }, { 0x0457, "FillRect(@Rect,@Patt)" },
	{ 0x0458, "FrameOval(@Rect)" }, { 0x0459, "PaintOval(@Rect)" }, { 0x045A, "EraseOval(@Rect)" }, { 0x045B, "InvertOval(@Rect)" }, { 0x045C, "FillOval(@Rect,@Patt)" },
	{ 0x045D, "FrameRRect(@Rect,OvalW,OvalHt)" }, { 0x045E, "PaintRRect(@Rect,OvalW,OvalHt)" }, { 0x045F, "EraseRRect(@Rect,OvalW,OvalHt)" }, { 0x0460, "InvertRRect(@Rect,OvalW,OvalHt)" },
	{ 0x0461, "FillRRect(@Rect,OvalW,OvalHt,@Patt)" }, { 0x0462, "FrameArc(@Rect,Ang1,ArcAng)" }, { 0x0463, "PaintArc(@Rect,Ang1,ArcAng)" }, { 0x0464, "EraseArc(@Rect,Ang1,ArcAng)" },
	{ 0x0465, "InvertArc(@Rect,Ang1,ArcAng)" }, { 0x0466, "FillArc(@Rect,Ang1,ArcAng,@Patt)" }, { 0x0467, "NewRgn():RgnH" }, { 0x0468, "DisposeRgn(RgnH)" },
	{ 0x0469, "CopyRgn(SrcRgnH,DestRgnH)" }, { 0x046A, "SetEmptyRgn(RgnH)" }, { 0x046B, "SetRectRgn(RgnH,left,top,right,bot)" }, { 0x046C, "RectRgn(RgnH,@Rect)" },
	{ 0x046D, "OpenRgn()" }, { 0x046E, "CloseRgn(RgnH)" }, { 0x046F, "OffsetRgn(RgnH,dh,dv)" }, { 0x0470, "InsetRgn(RgnH,dh,dv)" }, { 0x0471, "SectRgn(Rgn1H,Rgn2H,DstRgnH)" },
	{ 0x0472, "UnionRgn(Rgn1H,Rgn2H,UnionRgnH)" }, { 0x0473, "DiffRgn(Rgn1H,Rgn2H,DstRgnH)" }, { 0x0474, "XorRgn(Rgn1H,Rgn2H,DstRgnH)" }, { 0x0475, "PtInRgn(@Pt,RgnH):Flag" },
	{ 0x0476, "RectInRgn(@Rect,RgnH):Flag" }, { 0x0477, "EqualRgn(Rgn1H,Rgn2H):Flag" }, { 0x0478, "EmptyRgn(RgnH):Flag" }, { 0x0479, "FrameRgn(RgnH)" },
	{ 0x047A, "PaintRgn(RgnH)" }, { 0x047B, "EraseRgn(RgnH)" }, { 0x047C, "InvertRgn(RgnH)" }, { 0x047D, "FillRgn(RgnH,@Patt)" }, { 0x047E, "ScrollRect(@Rect,dh,dv,UpdtRgnH)" },
	{ 0x047F, "PaintPixels(@ppParms)" }, { 0x0480, "AddPt(@SrcPt,@DestPt)" }, { 0x0481, "SubPt(@SrcPt,@DstPt)" }, { 0x0482, "SetPt(@Pt,h,v)" }, { 0x0483, "EqualPt(@Pt1,@Pt2):Flag" },
	{ 0x0484, "LocalToGlobal(@Pt)" }, { 0x0485, "GlobalToLocal(@Pt)" }, { 0x0486, "Random():N" }, { 0x0487, "SetRandSeed(Seed/4)" }, { 0x0488, "GetPixel(Hor,Vert):Pixel" },
	{ 0x0489, "ScalePt(@Pt,@SrcRect,@DstRect)" }, { 0x048A, "MapPt(@Pt,@SrcRect,@DstRect)" }, { 0x048B, "MapRect(@Rect,@SrcRect,@DstRect)" }, { 0x048C, "MapRgn(MapRgnH,@SrcRect,@DstRect)" },
	{ 0x048D, "SetStdProcs(@StdProcRec)" }, { 0x048E, "SetCursor(@Curs)" }, { 0x048F, "GetCursorAdr():@Curs" }, { 0x0490, "HideCursor()" }, { 0x0491, "ShowCursor()" },
	{ 0x0492, "ObscureCursor()" }, { 0x0493, "SetMouseLoc ???" }, { 0x0494, "SetFont(FontH)" }, { 0x0495, "GetFont():FontH" }, { 0x0496, "GetFontInfo(@InfoRec)" },
	{ 0x0497, "GetFontGlobals(@FGRec)" }, { 0x0498, "SetFontFlags(Flags)" }, { 0x0499, "GetFontFlags():Flags" }, { 0x049A, "SetTextFace(TextF)" }, { 0x049B, "GetTextFace():TextF" },
	{ 0x049C, "SetTextMode(TextM)" }, { 0x049D, "GetTextMode():TextM" }, { 0x049E, "SetSpaceExtra(SpEx/4f)" }, { 0x049F, "GetSpaceExtra():SpEx/4f" }, { 0x04A0, "SetForeColor(Color)" },
	{ 0x04A1, "GetForeColor():Color" }, { 0x04A2, "SetBackColor(BackCol)" }, { 0x04A3, "GetBackColor():BackCol" }, { 0x04A4, "DrawChar(Char)" }, { 0x04A5, "DrawString(@Str)" },
	{ 0x04A6, "DrawCString(@cStr)" }, { 0x04A7, "DrawText(@Text,Len)" }, { 0x04A8, "CharWidth(Char):Width" }, { 0x04A9, "StringWidth(@Str):Width" }, { 0x04AA, "CStringWidth(@cStr):Width" },
	{ 0x04AB, "TextWidth(@Text,Len):Width" }, { 0x04AC, "CharBounds(Char,@Rect)" }, { 0x04AD, "StringBounds(@Str,@Rect)" }, { 0x04AE, "CStringBounds(@cStr,@Rect)" },
	{ 0x04AF, "TextBounds(@Text,Len,@Rect)" }, { 0x04B0, "SetArcRot(ArcRot)" }, { 0x04B1, "GetArcRot():ArcRot" }, { 0x04B2, "SetSysFont(FontH)" }, { 0x04B3, "GetSysFont():FontH" },
	{ 0x04B4, "SetVisRgn(RgnH)" }, { 0x04B5, "GetVisRgn(RgnH)" }, { 0x04B6, "SetIntUse(Flag)" }, { 0x04B7, "OpenPicture(@FrameRect):PicH" }, { 0x04B8, "PicComment(Kind,DataSz,DataH)" },
	{ 0x04B9, "ClosePicture()" }, { 0x04BA, "DrawPicture(PicH,@DstRect)" }, { 0x04BB, "KillPicture(PicH)" }, { 0x04BC, "FramePoly(PolyH)" }, { 0x04BD, "PaintPoly(PolyH)" },
	{ 0x04BE, "ErasePoly(PolyH)" }, { 0x04BF, "InvertPoly(PolyH)" }, { 0x04C0, "FillPoly(PolyH,@Patt)" }, { 0x04C1, "OpenPoly():PolyH" }, { 0x04C2, "ClosePoly()" },
	{ 0x04C3, "KillPoly(PolyH)" }, { 0x04C4, "OffsetPoly(PolyH,dh,dv)" }, { 0x04C5, "MapPoly(PolyH,@SrcRect,@DstRect)" }, { 0x04C6, "SetClipHandle(RgnH)" },
	{ 0x04C7, "GetClipHandle():RgnH" }, { 0x04C8, "SetVisHandle(RgnH)" }, { 0x04C9, "GetVisHandle():RgnH" }, { 0x04CA, "InitCursor()" }, { 0x04CB, "SetBufDims(MaxW,MaxFontHt,MaxFBRext)" },
	{ 0x04CC, "ForceBufDims(MaxW,MaxFontHt,MaxFBRext)" }, { 0x04CD, "SaveBufDims(@SizeInfo)" }, { 0x04CE, "RestoreBufDims(@SizeInfo)" }, { 0x04CF, "GetFGSize():FGSize" },
	{ 0x04D0, "SetFontID(FontID/4)" }, { 0x04D1, "GetFontID():FontID/4" }, { 0x04D2, "SetTextSize(TextSz)" }, { 0x04D3, "GetTextSize():TextSz" }, { 0x04D4, "SetCharExtra(ChEx/4f)" },
	{ 0x04D5, "GetCharExtra():ChEx/4f" }, { 0x04D6, "PPToPort(@SrcLoc,@SrcRect,X,Y,Mode)" }, { 0x04D7, "InflateTextBuffer(NewW,NewHt)" }, { 0x04D8, "GetRomFont(@Rec)" },
	{ 0x04D9, "GetFontLore(@Rec,RecSize):Size" }, { 0x04DA, "Get640Colors():@PattTable" }, { 0x04DB, "Set640Color(color)" }, { 0x0501, "DeskBootInit()" },
	{ 0x0502, "DeskStartUp()" }, { 0x0503, "DeskShutDown()" }, { 0x0504, "DeskVersion():Vers" }, { 0x0505, "DeskReset()" }, { 0x0506, "DeskStatus():ActFlg" },
	{ 0x0509, "SaveScrn()" }, { 0x050A, "RestScrn()" }, { 0x050B, "SaveAll()" }, { 0x050C, "RestAll()" }, { 0x050E, "InstallNDA(ndaH)" }, { 0x050F, "InstallCDA(cdaH)" },
	{ 0x0511, "ChooseCDA()" }, { 0x0513, "SetDAStrPtr(AltDispH,@StrTbl)" }, { 0x0514, "GetDAStrPtr():@StrTbl" }, { 0x0515, "OpenNDA(ItemID):Ref#" }, { 0x0516, "CloseNDA(Ref#)" },
	{ 0x0517, "SystemClick(@EvRec,@Wind,fwRes)" }, { 0x0518, "SystemEdit(eType):Flag" }, { 0x0519, "SystemTask()" }, { 0x051A, "SystemEvent(Mods,Where/4,When/4,Msg/4,What):F" },
	{ 0x051B, "GetNumNDAs():N" }, { 0x051C, "CloseNDAbyWinPtr(@Wind)" }, { 0x051D, "CloseAllNDAs()" }, { 0x051E, "FixAppleMenu(MenuID)" }, { 0x051F, "AddToRunQ(@taskHeader)" },
	{ 0x0520, "RemoveFromRunQ(@taskHeader)" }, { 0x0521, "RemoveCDA(cdaH)" }, { 0x0522, "RemoveNDA(ndaH)" }, { 0x0523, "GetDeskAccInfo(flags,daRef/4,BufSize,@Buffer)" },
	{ 0x0524, "CallDeskAcc(flags,daRef/4,Action,Data/4):Result" }, { 0x0525, "GetDeskGlobal(selector):Value/4" }, { 0x0601, "EMBootInit()" }, { 0x0602, "EMStartUp(DirPg,qSz,Xmn,Xmx,Ymn,Ymx,MemID)" },
	{ 0x0603, "EMShutDown()" }, { 0x0604, "EMVersion():Vers" }, { 0x0605, "EMReset()" }, { 0x0606, "EMStatus():ActFlg" }, { 0x0609, "DoWindows():DirPg" }, { 0x060A, "GetNextEvent(evMask,@EvRec):Flag" },
	{ 0x060B, "EventAvail(evMask,@EvRec):Flag" }, { 0x060C, "GetMouse(@Pt)" }, { 0x060D, "Button(Btn#):DownFlg" }, { 0x060E, "StillDown(Btn#):Flag" },
	{ 0x060F, "WaitMouseUp(Btn#):Flag" }, { 0x0610, "TickCount():Ticks/4" }, { 0x0611, "GetDblTime():Ticks/4" }, { 0x0612, "GetCaretTime():Ticks/4" },
	{ 0x0613, "SetSwitch()" }, { 0x0614, "PostEvent(code,Msg/4):Flag" }, { 0x0615, "FlushEvents(evMask,StopMask):F" }, { 0x0616, "GetOSEvent(evMask,@EvRec):Flag" },
	{ 0x0617, "OSEventAvail(evMask,@EvRec):Flag" }, { 0x0618, "SetEventMask(evMask)" }, { 0x0619, "FakeMouse(ChFlg,Mods,X,Y,BtnStat)" }, { 0x061A, "SetAutoKeyLimit(NewLimit)" },
	{ 0x061B, "GetKeyTranslation():kTransID" }, { 0x061C, "SetKeyTranslation(kTransID)" }, { 0x0701, "SchBootInit()" }, { 0x0702, "SchStartUp()" }, { 0x0703, "SchShutDown()" },
	{ 0x0704, "SchVersion():Vers" }, { 0x0705, "SchReset()" }, { 0x0706, "SchStatus():ActFlg" }, { 0x0709, "SchAddTask(@Task):Flag" }, { 0x070A, "SchFlush()" },
	{ 0x0801, "SoundBootInit()" }, { 0x0802, "SoundStartUp(DirPg)" }, { 0x0803, "SoundShutDown()" }, { 0x0804, "SoundVersion():Vers" }, { 0x0805, "SoundReset()" },
	{ 0x0806, "SoundToolStatus():ActFlg" }, { 0x0809, "WriteRamBlock(@Src,DOCStart,Count)" }, { 0x080A, "ReadRamBlock(@Dest,DOCStart,Count)" }, { 0x080B, "GetTableAddress():@JumpTbl" },
	{ 0x080C, "GetSoundVolume(Gen#):Vol" }, { 0x080D, "SetSoundVolume(Vol,Gen#)" }, { 0x080E, "FFStartSound(GenN&mode,@Parms)" }, { 0x080F, "FFStopSound(GenMask)" },
	{ 0x0810, "FFSoundStatus():ActFlg" }, { 0x0811, "FFGeneratorStatus(Gen#):Stat" }, { 0x0812, "SetSoundMIRQV(@IntHandler)" }, { 0x0813, "SetUserSoundIRQV(@NewIRQ):@OldIRQ" },
	{ 0x0814, "FFSoundDoneStatus(Gen#):Stat" }, { 0x0815, "FFSetUpSound(ChannelGen,@Parms)" }, { 0x0816, "FFStartPlaying(GenWord)" }, { 0x0817, "SetDocReg(@DocRegParms)" },
	{ 0x0818, "ReadDocReg(@DocRegParms)" }, { 0x0901, "ADBBootInit()" }, { 0x0902, "ADBStartUp()" }, { 0x0903, "ADBShutDown()" }, { 0x0904, "ADBVersion():Vers" },
	{ 0x0905, "ADBReset()" }, { 0x0906, "ADBStatus():ActFlg" }, { 0x0909, "SendInfo(NumB,@Data,Cmd)" }, { 0x090A, "ReadKeyMicroData(NumB,@Data,Cmd)" }, { 0x090B, "ReadKeyMicroMemory(@DataOut,@DataIn,Cmd)" },
	{ 0x090C, "[resynch--don't call]" }, { 0x090D, "AsyncADBReceive(@CompVec,Cmd)" }, { 0x090E, "SyncADBReceive(InputWrd,@CompVec,Cmd)" }, { 0x090F, "AbsOn()" },
	{ 0x0910, "AbsOff()" }, { 0x0911, "RdAbs():Flag" }, { 0x0912, "SetAbsScale(@DataOut)" }, { 0x0913, "GetAbsScale(@DataIn)" }, { 0x0914, "SRQPoll(@CompVec,ADBreg)" },
	{ 0x0915, "SRQRemove(ADBreg)" }, { 0x0916, "ClearSRQTable()" }, { 0x09FF, "[OBSOLETE: Use 09FF]" }, { 0x0A01, "SANEBootInit()" }, { 0x0A02, "SANEStartUp(DirPg)" },
	{ 0x0A03, "SANEShutDown()" }, { 0x0A04, "SANEVersion():Vers" }, { 0x0A05, "SANEReset()" }, { 0x0A06, "SANEStatus():ActFlg" }, { 0x0A09, "FPNum (...)" },
	{ 0x0A0A, "DecStrNum (...)" }, { 0x0A0B, "ElemNum (...)" }, { 0x0AFF, "[OBSOLETE: USE $0AFF]" }, { 0x0B01, "IMBootInit()" }, { 0x0B02, "IMStartUp()" },
	{ 0x0B03, "IMShutDown()" }, { 0x0B04, "IMVersion():Vers" }, { 0x0B05, "IMReset()" }, { 0x0B06, "IMStatus():ActFlg" }, { 0x0B09, "Multiply(A,B):Prod/4" },
	{ 0x0B0A, "SDivide(Num,Den):Rem,Quot" }, { 0x0B0B, "UDivide(Num,Den):Rem,Quot" }, { 0x0B0C, "LongMul(A/4,B/4):Prod/8" }, { 0x0B0D, "LongDivide(Num/4,Denom/4):Rem/4,Quot/4" },
	{ 0x0B0E, "FixRatio(Numer,Denom):fxRatio/4" }, { 0x0B0F, "FixMul(fx1/4,fx2/4):fxProd/4" }, { 0x0B10, "FracMul(fr1/4,fr2/4):frRes/4" }, { 0x0B11, "FixDiv(Quot/4,Divisor/4):fxRes/4" },
	{ 0x0B12, "FracDiv(Quot/4,Divisor/4):frRes/4" }, { 0x0B13, "FixRound(fxVal/4):Int" }, { 0x0B14, "FracSqrt(frVal/4):frRes/4" }, { 0x0B15, "FracCos(fxAngle/4):frRes/4" },
	{ 0x0B16, "FracSin(fxAngle/4):frRes/4" }, { 0x0B17, "FixATan2(In1/4,In2/4):fxArcTan/4" }, { 0x0B18, "HiWord(Long/4):Int" }, { 0x0B19, "LoWord(Long/4):Int" },
	{ 0x0B1A, "Long2Fix(Long/4):fxRes/4" }, { 0x0B1B, "Fix2Long(Fix/4):Long/4" }, { 0x0B1C, "Fix2Frac(fxVal/4):Frac/4" }, { 0x0B1D, "Frac2Fix(frVal/4):fxRes/4" },
	{ 0x0B1E, "Fix2X(Fix/4,@Extended)" }, { 0x0B1F, "Frac2X(frVal/4,@Extended)" }, { 0x0B20, "X2Fix(@Extended):fxRes/4" }, { 0x0B21, "X2Frac(@Extended):frRes/4" },
	{ 0x0B22, "Int2Hex(Int,@Str,Len)" }, { 0x0B23, "Long2Hex(Long/4,@Str,Len)" }, { 0x0B24, "Hex2Int(@Str,Len):Int" }, { 0x0B25, "Hex2Long(@Str,Len):Long/4" },
	{ 0x0B26, "Int2Dec(Int,@Str,Len,SgnFlg)" }, { 0x0B27, "Long2Dec(Long/4,@Str,Len,SgnFlg)" }, { 0x0B28, "Dec2Int(@Str,Len,SgnFlg):Int" }, { 0x0B29, "Dec2Long(@Str,Len,SgnFlg):Long/4" },
	{ 0x0B2A, "HexIt(Int):Hex/4" }, { 0x0C01, "TextBootInit()" }, { 0x0C02, "TextStartUp()" }, { 0x0C03, "TextShutDown()" }, { 0x0C04, "TextVersion():Vers" },
	{ 0x0C05, "TextReset()" }, { 0x0C06, "TextStatus():ActFlg" }, { 0x0C09, "SetInGlobals(ANDmsk,ORmsk)" }, { 0x0C0A, "SetOutGlobals(ANDmsk,ORmsk)" },
	{ 0x0C0B, "SetErrGlobals(ANDmsk,ORmsk)" }, { 0x0C0C, "GetInGlobals():ANDmsk,ORmsk" }, { 0x0C0D, "GetOutGlobals():ANDmsk,ORmsk" }, { 0x0C0E, "GetErrGlobals():ANDmsk,ORmsk" },
	{ 0x0C0F, "SetInputDevice(Type,@drvr|Slot/4)" }, { 0x0C10, "SetOutputDevice(Type,@drvr|Slot/4)" }, { 0x0C11, "SetErrorDevice(Type,@drvr|Slot/4)" }, { 0x0C12, "GetInputDevice():Type,@drvr|Slot/4" },
	{ 0x0C13, "GetOutputDevice():Type,@drvr|Slot/4" }, { 0x0C14, "GetErrorDevice():Type,@drvr|Slot/4" }, { 0x0C15, "InitTextDev(dev)" }, { 0x0C16, "CtlTextDev(dev,code)" },
	{ 0x0C17, "StatusTextDev(dev,request)" }, { 0x0C18, "WriteChar(Char)" }, { 0x0C19, "ErrWriteChar(Char)" }, { 0x0C1A, "WriteLine(@Str)" }, { 0x0C1B, "ErrWriteLine(@Str)" },
	{ 0x0C1C, "WriteString(@Str)" }, { 0x0C1D, "ErrWriteString(@Str)" }, { 0x0C1E, "TextWriteBlock(@Text,Offset,Len)" }, { 0x0C1F, "ErrWriteBlock(@Text,Offset,Len)" }, { 0x0C20, "WriteCString(@cStr)" },
	{ 0x0C21, "ErrWriteCString(@cStr)" }, { 0x0C22, "ReadChar(EchoFlg):Char" }, { 0x0C23, "TextReadBlock(@Buff,Offset,Size,EchoFlg)" }, { 0x0C24, "ReadLine(@Buff,Max,EOLch,EchoFlg):Count" },
	{ 0x0E01, "WindBootInit()" }, { 0x0E02, "WindStartUp(MemID)" }, { 0x0E03, "WindShutDown()" }, { 0x0E04, "WindVersion():Vers" }, { 0x0E05, "WindReset()" }, { 0x0E06, "WindStatus():ActFlg" },
	{ 0x0E09, "NewWindow(@Parms):@Wind" }, { 0x0E0A, "CheckUpdate(@EvRec):Flag" }, { 0x0E0B, "CloseWindow(@Wind)" }, { 0x0E0C, "Desktop(Oper,param/4):result/4" },
	{ 0x0E0D, "SetWTitle(@Title,@Wind)" }, { 0x0E0E, "GetWTitle(@Wind):@Title" }, { 0x0E0F, "SetFrameColor(@NewColTbl,@Wind)" }, { 0x0E10, "GetFrameColor(@Table,@Wind)" },
	{ 0x0E11, "SelectWindow(@Wind)" }, { 0x0E12, "HideWindow(@Wind)" }, { 0x0E13, "ShowWindow(@Wind)" }, { 0x0E14, "SendBehind(@BehindWho,@Wind)" },
	{ 0x0E15, "FrontWindow():@Wind" }, { 0x0E16, "SetInfoDraw(@Proc,@Wind)" }, { 0x0E17, "FindWindow(@WindVar,X,Y):Where" }, { 0x0E18, "TrackGoAway(X,Y,@Wind):Flag" },
	{ 0x0E19, "MoveWindow(X,Y,@Wind)" }, { 0x0E1A, "DragWindow(Grid,X,Y,Grace,@bRect,@Wind)" }, { 0x0E1B, "GrowWindow(mnW,mnH,X,Y,@Wind):nSize/4" },
	{ 0x0E1C, "SizeWindow(w,h,@Wind)" }, { 0x0E1D, "TaskMaster(evMask,@TaskRec):Code" }, { 0x0E1E, "BeginUpdate(@Wind)" }, { 0x0E1F, "EndUpdate(@Wind)" },
	{ 0x0E20, "GetWMgrPort():@Port" }, { 0x0E21, "PinRect(X,Y,@Rect):Point/4" }, { 0x0E22, "HiliteWindow(Flag,@Wind)" }, { 0x0E23, "ShowHide(Flag,@Wind)" },
	{ 0x0E24, "BringToFront(@Wind)" }, { 0x0E25, "WindNewRes()" }, { 0x0E26, "TrackZoom(X,Y,@Wind):Flag" }, { 0x0E27, "ZoomWindow(@Wind)" }, { 0x0E28, "SetWRefCon(Refcon/4,@Wind)" },
	{ 0x0E29, "GetWRefCon(@Wind):Refcon/4" }, { 0x0E2A, "GetNextWindow(@Wind):@Wind" }, { 0x0E2B, "GetWKind(@Wind):Flag" }, { 0x0E2C, "GetWFrame(@Wind):Frame" },
	{ 0x0E2D, "SetWFrame(Frame,@Wind)" }, { 0x0E2E, "GetStructRgn(@Wind):StructRgnH" }, { 0x0E2F, "GetContentRgn(@Wind):ContRgnH" }, { 0x0E30, "GetUpdateRgn(@Wind):UpdateRgnH" },
	{ 0x0E31, "GetDefProc(@Wind):@Proc" }, { 0x0E32, "SetDefProc(@Proc,@Wind)" }, { 0x0E33, "GetWControls(@Wind):CtrlH" }, { 0x0E34, "SetOriginMask(Mask,@Wind)" },
	{ 0x0E35, "GetInfoRefCon(@Wind):Refcon/4" }, { 0x0E36, "SetInfoRefCon(Val/4,@Wind)" }, { 0x0E37, "GetZoomRect(@Wind):@zRect" }, { 0x0E38, "SetZoomRect(@zRect,@Wind)" },
	{ 0x0E39, "RefreshDesktop(@Rect)" }, { 0x0E3A, "InvalRect(@Rect)" }, { 0x0E3B, "InvalRgn(RgnH)" }, { 0x0E3C, "ValidRect(@Rect)" }, { 0x0E3D, "ValidRgn(RgnH)" },
	{ 0x0E3E, "GetContentOrigin(@Wind):Origin/4" }, { 0x0E3F, "SetContentOrigin(X,Y,@Wind)" }, { 0x0E40, "GetDataSize(@Wind):DataSize/4" }, { 0x0E41, "SetDataSize(w,h,@Wind)" },
	{ 0x0E42, "GetMaxGrow(@Wind):MaxGrow/4" }, { 0x0E43, "SetMaxGrow(maxWidth,maxHeight,@Wind)" }, { 0x0E44, "GetScroll(@Wind):Scroll/4" }, { 0x0E45, "SetScroll(h,v,@Wind)" },
	{ 0x0E46, "GetPage(@Wind):Page/4" }, { 0x0E47, "SetPage(h,v,@Wind)" }, { 0x0E48, "GetContentDraw(@Wind):@Proc" }, { 0x0E49, "SetContentDraw(@Proc,@Wind)" },
	{ 0x0E4A, "GetInfoDraw(@Wind):@Proc" }, { 0x0E4B, "SetSysWindow(@Wind)" }, { 0x0E4C, "GetSysWFlag(@Wind):Flag" }, { 0x0E4D, "StartDrawing(@Wind)" },
	{ 0x0E4E, "SetWindowIcons(NewFontH):OldFontH" }, { 0x0E4F, "GetRectInfo(@InfoRect,@Wind)" }, { 0x0E50, "StartInfoDrawing(@iRect,@Wind)" }, { 0x0E51, "EndInfoDrawing()" },
	{ 0x0E52, "GetFirstWindow():@Wind" }, { 0x0E53, "WindDragRect(@a,@P,X,Y,@R,@lR,@sR,F):M/4" }, { 0x0E54, "Private01():@func [GetDragRectPtr]" },
	{ 0x0E55, "DrawInfoBar(@Wind)" }, { 0x0E56, "WindowGlobal(Flags):Flags" }, { 0x0E57, "SetContentOrigin2(ScrollFlag,X,Y,@Wind)" }, { 0x0E58, "GetWindowMgrGlobals():@Globals" },
	{ 0x0E59, "AlertWindow(AlertDesc,@SubArray,AlertRef/4):Btn" }, { 0x0E5A, "StartFrameDrawing(@Wind)" }, { 0x0E5B, "EndFrameDrawing()" }, { 0x0E5C, "ResizeWindow(hidden,@ContRect,@Wind)" },
	{ 0x0E5D, "TaskMasterContent" }, { 0x0E5E, "TaskMasterKey" }, { 0x0E5F, "TaskMasterDA(evMask,@bigTaskRec):taskCode" }, { 0x0E60, "CompileText(subType,@subs,@text,size):H" },
	{ 0x0E61, "NewWindow2(@T,RC/4,@draw,@def,pDesc,pRef/4,rType):@W" }, { 0x0E62, "ErrorWindow(subType,@subs,ErrNum):Button" },
	{ 0x0E63, "GetAuxWindInfo(@Wind):@Info" }, { 0x0E64, "DoModalWindow(@Event,@Update,@EvHook,@Beep,Flags):Result/4" }, { 0x0E65, "MWGetCtlPart():Part" },
	{ 0x0E66, "MWSetMenuProc(@NewMenuProc):@OldMenuProc" }, { 0x0E67, "MWStdDrawProc()" }, { 0x0E68, "MWSetUpEditMenu()" }, { 0x0E69, "FindCursorCtl(@CtrlH,x,y,@Wind):PartCode" },
	{ 0x0E6A, "ResizeInfoBar(flags,newHeight,@Wind)" }, { 0x0E6B, "HandleDiskInsert(flags,devNum):resFlags,resDevNum" }, { 0x0E6C, "UpdateWindow(flags,@Wind)" },
	{ 0x0F01, "MenuBootInit()" }, { 0x0F02, "MenuStartUp(MemID,DirPg)" }, { 0x0F03, "MenuShutDown()" }, { 0x0F04, "MenuVersion():Vers" }, { 0x0F05, "MenuReset()" },
	{ 0x0F06, "MenuStatus():ActFlg" }, { 0x0F09, "MenuKey(@TaskRec,BarH)" }, { 0x0F0A, "GetMenuBar():BarH" }, { 0x0F0B, "MenuRefresh(@RedrawProc)" }, { 0x0F0C, "FlashMenuBar()" },
	{ 0x0F0D, "InsertMenu(MenuH,AfterWhat)" }, { 0x0F0E, "DeleteMenu(MenuID)" }, { 0x0F0F, "InsertMItem(@Item,AfterItem,MenuID)" }, { 0x0F10, "DeleteMItem(ItemID)" },
	{ 0x0F11, "GetSysBar():BarH" }, { 0x0F12, "SetSysBar(BarH)" }, { 0x0F13, "FixMenuBar():Height" }, { 0x0F14, "CountMItems(MenuID):N" }, { 0x0F15, "NewMenuBar(@Wind):BarH" },
	{ 0x0F16, "GetMHandle(MenuID):MenuH" }, { 0x0F17, "SetBarColors(BarCol,InvCol,OutCol)" }, { 0x0F18, "GetBarColors():Colors/4" }, { 0x0F19, "SetMTitleStart(hStart)" },
	{ 0x0F1A, "GetMTitleStart():hStart" }, { 0x0F1B, "GetMenuMgrPort():@Port" }, { 0x0F1C, "CalcMenuSize(w,h,MenuID)" }, { 0x0F1D, "SetMTitleWidth(w,MenuID)" },
	{ 0x0F1E, "GetMTitleWidth(MenuID):TitleWidth" }, { 0x0F1F, "SetMenuFlag(Flags,MenuID)" }, { 0x0F20, "GetMenuFlag(MenuID):Flags" }, { 0x0F21, "SetMenuTitle(@Title,MenuID)" },
	{ 0x0F22, "GetMenuTitle(MenuID):@Title" }, { 0x0F23, "MenuGlobal(Flags):Flags" }, { 0x0F24, "SetMItem(@Str,ItemID)" }, { 0x0F25, "GetMItem(ItemID):@ItemName" },
	{ 0x0F26, "SetMItemFlag(Flags,ItemID)" }, { 0x0F27, "GetMItemFlag(ItemID):Flag" }, { 0x0F28, "SetMItemBlink(Count)" }, { 0x0F29, "MenuNewRes()" },
	{ 0x0F2A, "DrawMenuBar()" }, { 0x0F2B, "MenuSelect(@TaskRec,BarH)" }, { 0x0F2C, "HiliteMenu(Flag,MenuID)" }, { 0x0F2D, "NewMenu(@MenuStr):MenuH" }, { 0x0F2E, "DisposeMenu(MenuH)" },
	{ 0x0F2F, "InitPalette()" }, { 0x0F30, "EnableMItem(ItemID)" }, { 0x0F31, "DisableMItem(ItemID)" }, { 0x0F32, "CheckMItem(Flag,ItemID)" }, { 0x0F33, "SetMItemMark(MarkCh,ItemID)" },
	{ 0x0F34, "GetMItemMark(ItemID):MarkChar" }, { 0x0F35, "SetMItemStyle(TextStyle,ItemID)" }, { 0x0F36, "GetMItemStyle(ItemID):TextStyle" }, { 0x0F37, "SetMenuID(New,Old)" },
	{ 0x0F38, "SetMItemID(New,Old)" }, { 0x0F39, "SetMenuBar(BarH)" }, { 0x0F3A, "SetMItemName(@Str,ItemID)" }, { 0x0F3B, "GetPopUpDefProc():@proc" },
	{ 0x0F3C, "PopUpMenuSelect(SelID,left,top,flag,MenuH):id" }, { 0x0F3D, "[DrawPopUp(SelID,Flag,right,bottom,left,top,MenuH)]" }, { 0x0F3E, "NewMenu2(RefDesc,Ref/4):MenuH" },
	{ 0x0F3F, "InsertMItem2(RefDesc,Ref/4,After,MenuID)" }, { 0x0F40, "SetMenuTitle2(RefDesc,TitleRef/4,MenuID)" }, { 0x0F41, "SetMItem2(RefDesc,Ref/4,Item)" },
	{ 0x0F42, "SetMItemName2(RefDesc,Ref/4,Item)" }, { 0x0F43, "NewMenuBar2(RefDesc,Ref/4,@Wind):BarH" }, { 0x0F45, "HideMenuBar()" }, { 0x0F46, "ShowMenuBar()" },
	{ 0x0F47, "SetMItemIcon(IconDesc,IconRef/4,ItemID)" }, { 0x0F48, "GetMItemIcon(ItemID):IconRef/4" }, { 0x0F49, "SetMItemStruct(Desc,StructRef/4,ItemID)" },
	{ 0x0F4A, "GetMItemStruct(ItemID):ItemStruct/4" }, { 0x0F4B, "RemoveMItemStruct(ItemID)" }, { 0x0F4C, "GetMItemFlag2(ItemID):ItemFlag2" }, { 0x0F4D, "SetMItemFlag2(newValue,ItemID)" },
	{ 0x0F4F, "GetMItemBlink():Count" }, { 0x0F50, "InsertPathMItems(flags,@Path,devnum,MenuID,AfterID,StartID,@Results)" }, { 0x1001, "CtlBootInit()" },
	{ 0x1002, "CtlStartUp(MemID,DirPg)" }, { 0x1003, "CtlShutDown()" }, { 0x1004, "CtlVersion():Vers" }, { 0x1005, "CtlReset()" }, { 0x1006, "CtlStatus():ActFlg" },
	{ 0x1009, "NewControl(@W,@R,@T,F,V,P1,P2,@p,r/4,@C):cH" }, { 0x100A, "DisposeControl(CtrlH)" }, { 0x100B, "KillControls(@Wind)" }, { 0x100C, "SetCtlTitle(@Title,CtrlH)" },
	{ 0x100D, "GetCtlTitle(CtrlH):@Title" }, { 0x100E, "HideControl(CtrlH)" }, { 0x100F, "ShowControl(CtrlH)" }, { 0x1010, "DrawControls(@Wind)" }, { 0x1011, "HiliteControl(Flag,CtrlH)" },
	{ 0x1012, "CtlNewRes()" }, { 0x1013, "FindControl(@CtrlHVar,X,Y,@Wind):Part" }, { 0x1014, "TestControl(X,Y,CtrlH):Part" }, { 0x1015, "TrackControl(X,Y,@ActProc,CtrlH):Part" },
	{ 0x1016, "MoveControl(X,Y,CtrlH)" }, { 0x1017, "DragControl(X,Y,@LimR,@slR,Axis,CtrlH)" }, { 0x1018, "SetCtlIcons(FontH):OldFontH" }, { 0x1019, "SetCtlValue(Val,CtrlH)" },
	{ 0x101A, "GetCtlValue(CtrlH):Val" }, { 0x101B, "SetCtlParams(P2,P1,CtrlH)" }, { 0x101C, "GetCtlParams(CtrlH):P1,P2" }, { 0x101D, "DragRect(@acPr,@P,X,Y,@drR,@l,@slR,F):M/4" },
	{ 0x101E, "GrowSize():Size/4" }, { 0x101F, "GetCtlDpage():DirPg" }, { 0x1020, "SetCtlAction(@ActProc,CtrlH)" }, { 0x1021, "GetCtlAction(CtrlH):Action/4" }, { 0x1022, "SetCtlRefCon(Refcon/4,CtrlH)" },
	{ 0x1023, "GetCtlRefCon(CtrlH):Refcon/4" }, { 0x1024, "EraseControl(CtrlH)" }, { 0x1025, "DrawOneCtl(CtrlH)" }, { 0x1026, "FindTargetCtl():CtrlH" },
	{ 0x1027, "MakeNextCtlTarget():CtrlH" }, { 0x1028, "MakeThisCtlTarget(CtrlH)" }, { 0x1029, "SendEventToCtl(TgtOnly,@Wind,@eTask):Accepted" }, { 0x102A, "GetCtlID(CtrlH):CtlID/4" },
	{ 0x102B, "SetCtlID(CtlID/4,CtrlH)" }, { 0x102C, "CallCtlDefProc(CtrlH,Msg,Param/4):Result/4" }, { 0x102D, "NotifyCtls(Mask,Msg,Param/4,@Wind)" },
	{ 0x102E, "GetCtlMoreFlags(CtrlH):Flags" }, { 0x102F, "SetCtlMoreFlags(Flags,CtrlH)" }, { 0x1030, "GetCtlHandleFromID(@Wind,CtlID/4):CtrlH" }, { 0x1031, "NewControl2(@Wind,InKind,InRef/4):CtrlH" },
	{ 0x1032, "CMLoadResource(rType,rID/4):resH" }, { 0x1033, "CMReleaseResource(rType,rID/4)" }, { 0x1034, "SetCtlParamPtr(@SubArray)" }, { 0x1035, "GetCtlParamPtr():@SubArray" },
	{ 0x1037, "InvalCtls(@Wind)" }, { 0x1038, "[reserved]" }, { 0x1039, "FindRadioButton(@Wind,FamilyNum):WhichRadio" }, { 0x103A, "SetLETextByID(@Wind,leID/4,@PString)" },
	{ 0x103B, "GetLETextByID(@Wind,leID/4,@PString)" }, { 0x103C, "SetCtlValueByID(Value,@Wind,CtlID/4)" }, { 0x103D, "GetCtlValueByID(@Wind,CtlID/4):Value" }, { 0x103E, "InvalOneCtlByID(@Wind,CtlID/4)" },
	{ 0x103F, "HiliteCtlByID(Hilite,@Wind,CtlID/4)" }, { 0x1101, "LoaderBootInit()" }, { 0x1102, "LoaderStartUp()" }, { 0x1103, "LoaderShutDown()" },
	{ 0x1104, "LoaderVersion():Vers" }, { 0x1105, "LoaderReset()" }, { 0x1106, "LoaderStatus():ActFlg" }, { 0x1109, "InitialLoad(MemID,@path,F):dpsSz,dps,@l,MemID" },
	{ 0x110A, "Restart(MemID):dpsSz,dps,@loc,MemID" }, { 0x110B, "LoadSegNum(MemID,file#,seg#):@loc" }, { 0x110C, "UnloadSegNum(MemID,file#,seg#)" }, { 0x110D, "LoadSegName(MemID,@path,@segn):@loc,MemID,file#,sg#" },
	{ 0x110E, "UnloadSeg(@loc):seg#,file#,MemID" }, { 0x110F, "GetLoadSegInfo(MemID,file#,seg#,@buff)" }, { 0x1110, "GetUserID(@Pathname):MemID" }, { 0x1111, "LGetPathname(MemID,file#):@path" },
	{ 0x1112, "UserShutDown(MemID,qFlag):MemID" }, { 0x1113, "RenamePathname(@path1,@path2)" }, { 0x1120, "InitialLoad2(MemID,@in,F,Type):dpsSz,dps,@l,MemID" },
	{ 0x1121, "GetUserID2(@path):MemID" }, { 0x1122, "LGetPathname2(MemID,file#):@path" }, { 0x1201, "QDAuxBootInit()" }, { 0x1202, "QDAuxStartUp()" },
	{ 0x1203, "QDAuxShutDown()" }, { 0x1204, "QDAuxVersion():Vers" }, { 0x1205, "QDAuxReset()" }, { 0x1206, "QDAuxStatus():ActFlg" }, { 0x1209, "CopyPixels(@sLoc,@dLoc,@sRect,@dRct,M,MskH)" },
	{ 0x120A, "WaitCursor()" }, { 0x120B, "DrawIcon(@Icon,Mode,X,Y)" }, { 0x120C, "SpecialRect(@Rect,FrameColor,FillColor)" }, { 0x120D, "SeedFill(@sLoc,@sR,@dLoc,@dR,X,Y,Mode,@Patt,@Leak)" },
	{ 0x120E, "CalcMask(@sLoc,@sR,@dLoc,@dR,Mode,@Patt,@Leak)" }, { 0x120F, "GetSysIcon(flags,value,aux/4):@Icon" }, { 0x1210, "PixelMap2Rgn(@srcLoc,bitsPerPix,colorMask):RgnH" },
	{ 0x1213, "IBeamCursor()" }, { 0x1214, "WhooshRect(flags/4,@smallRect,@bigRect)" }, { 0x1215, "DrawStringWidth(Flags,Ref/4,Width)" }, { 0x1216, "UseColorTable(tableNum,@Table,Flags):ColorInfoH" },
	{ 0x1217, "RestoreColorTable(ColorInfoH,Flags)" }, { 0x1301, "PMBootInit()" }, { 0x1302, "PMStartUp(MemID,DirPg)" }, { 0x1303, "PMShutDown()" },
	{ 0x1304, "PMVersion():Vers" }, { 0x1305, "PMReset()" }, { 0x1306, "PMStatus():ActFlg" }, { 0x1309, "PrDefault(PrRecH)" }, { 0x130A, "PrValidate(PrRecH):Flag" },
	{ 0x130B, "PrStlDialog(PrRecH):Flag" }, { 0x130C, "PrJobDialog(PrRecH):Flag" }, { 0x130D, "PrPixelMap(@LocInfo,@SrcRect,colorFlag)" }, { 0x130E, "PrOpenDoc(PrRecH,@Port):@Port" },
	{ 0x130F, "PrCloseDoc(@Port)" }, { 0x1310, "PrOpenPage(@Port,@Frame)" }, { 0x1311, "PrClosePage(@Port)" }, { 0x1312, "PrPicFile(PrRecH,@Port,@StatRec)" }, { 0x1313, "PrControl [obsolete]" },
	{ 0x1314, "PrError():Error" }, { 0x1315, "PrSetError(Error)" }, { 0x1316, "PrChoosePrinter():DrvFlag" }, { 0x1318, "PrGetPrinterSpecs():Type,Characteristics" },
	{ 0x1319, "PrDevPrChanged(@PrinterName)" }, { 0x131A, "PrDevStartup(@PrinterName,@ZoneName)" }, { 0x131B, "PrDevShutDown()" }, { 0x131C, "PrDevOpen(@compProc,Reserved/4)" },
	{ 0x131D, "PrDevRead(@buffer,reqCount):xferCount" }, { 0x131E, "PrDevWrite(@compProc,@buff,bufLen)" }, { 0x131F, "PrDevClose()" }, { 0x1320, "PrDevStatus(@statBuff)" },
	{ 0x1321, "PrDevAsyncRead(@compPr,bufLen,@buff):xferCount" }, { 0x1322, "PrDevWriteBackground(@compProc,bufLen,@buff)" }, { 0x1323, "PrDriverVer():Vers" },
	{ 0x1324, "PrPortVer():Vers" }, { 0x1325, "PrGetZoneName():@ZoneName" }, { 0x1328, "PrGetPrinterDvrName():@Name" }, { 0x1329, "PrGetPortDvrName():@Name" },
	{ 0x132A, "PrGetUserName():@Name" }, { 0x132B, "PrGetNetworkName():@Name" }, { 0x1330, "PrDevIsItSafe():safeFlag" }, { 0x1331, "GetZoneList     [obsolete?]" },
	{ 0x1332, "GetMyZone       [obsolete?]" }, { 0x1333, "GetPrinterList  [obsolete?]" }, { 0x1334, "PMUnloadDriver(whichDriver)" }, { 0x1335, "PMLoadDriver(whichDriver)" },
	{ 0x1336, "PrGetDocName():@pStr" }, { 0x1337, "PrSetDocName(@pStr)" }, { 0x1338, "PrGetPgOrientation(PrRecH):Orientation" }, { 0x1401, "LEBootInit()" },
	{ 0x1402, "LEStartUp(MemID,DirPg)" }, { 0x1403, "LEShutDown()" }, { 0x1404, "LEVersion():Vers" }, { 0x1405, "LEReset()" }, { 0x1406, "LEStatus():ActFlg" },
	{ 0x1409, "LENew(@DstRect,@ViewRect,MaxL):leH" }, { 0x140A, "LEDispose(leH)" }, { 0x140B, "LESetText(@Text,Len,leH)" }, { 0x140C, "LEIdle(leH)" },
	{ 0x140D, "LEClick(@EvRec,leH)" }, { 0x140E, "LESetSelect(Start,End,leH)" }, { 0x140F, "LEActivate(leH)" }, { 0x1410, "LEDeactivate(leH)" }, { 0x1411, "LEKey(Key,Mods,leH)" },
	{ 0x1412, "LECut(leH)" }, { 0x1413, "LECopy(leH)" }, { 0x1414, "LEPaste(leH)" }, { 0x1415, "LEDelete(leH)" }, { 0x1416, "LEInsert(@Text,Len,leH)" },
	{ 0x1417, "LEUpdate(leH)" }, { 0x1418, "LETextBox(@Text,Len,@Rect,Just)" }, { 0x1419, "LEFromScrap()" }, { 0x141A, "LEToScrap()" }, { 0x141B, "LEScrapHandle():ScrapH" },
	{ 0x141C, "LEGetScrapLen():Len" }, { 0x141D, "LESetScrapLen(NewL)" }, { 0x141E, "LESetHilite(@HiliteProc,leH)" }, { 0x141F, "LESetCaret(@CaretProc,leH)" },
	{ 0x1420, "LETextBox2(@Text,Len,@Rect,Just)" }, { 0x1421, "LESetJust(Just,leH)" }, { 0x1422, "LEGetTextHand(leH):TextH" }, { 0x1423, "LEGetTextLen(leH):TxtLen" },
	{ 0x1424, "GetLEDefProc():@proc" }, { 0x1425, "LEClassifyKey(@Event):Flag" }, { 0x1501, "DialogBootInit()" }, { 0x1502, "DialogStartUp(MemID)" }, { 0x1503, "DialogShutDown()" },
	{ 0x1504, "DialogVersion():Vers" }, { 0x1505, "DialogReset()" }, { 0x1506, "DialogStatus():ActFlg" }, { 0x1509, "ErrorSound(@SoundProc)" }, { 0x150A, "NewModalDialog(@bR,vis,refcon/4):@Dlog" },
	{ 0x150B, "NewModelessDialog(@R,@T,@b,fr,rf/4,@zR):@D" }, { 0x150C, "CloseDialog(@Dlog)" }, { 0x150D, "NewDItem(@Dlog,dItem,@R,ty,Des/4,V,F,@Col)" },
	{ 0x150E, "RemoveDItem(@Dlog,dItem)" }, { 0x150F, "ModalDialog(@FilterProc):Hit" }, { 0x1510, "IsDialogEvent(@EvRec):Flag" }, { 0x1511, "DialogSelect(@EvRec,@Dlog,@Hit):Flag" },
	{ 0x1512, "DlgCut(@Dlog)" }, { 0x1513, "DlgCopy(@Dlog)" }, { 0x1514, "DlgPaste(@Dlog)" }, { 0x1515, "DlgDelete(@Dlog)" }, { 0x1516, "DrawDialog(@Dlog)" },
	{ 0x1517, "Alert(@AlertTmpl,@FiltProc):Hit" }, { 0x1518, "StopAlert(@AlertTmpl,@FiltProc):Hit" }, { 0x1519, "NoteAlert(@AlertTmpl,@FiltProc):Hit" },
	{ 0x151A, "CautionAlert(@AlertTmpl,@FiltProc):Hit" }, { 0x151B, "ParamText(@P0,@P1,@P2,@P3)" }, { 0x151C, "SetDAFont(FontH)" }, { 0x151E, "GetControlDItem(@Dlog,dItem):CtrlH" },
	{ 0x151F, "GetIText(@Dlog,dItem,@Str)" }, { 0x1520, "SetIText(@Dlog,dItem,@Str)" }, { 0x1521, "SelectIText(@Dlog,dItem,start,end)" }, { 0x1522, "HideDItem(@Dlog,dItem)" },
	{ 0x1523, "ShowDItem(@Dlog,dItem)" }, { 0x1524, "FindDItem(@Dlog,Point/4):Hit" }, { 0x1525, "UpdateDialog(@Dlog,UpdtRgnH)" }, { 0x1526, "GetDItemType(@Dlog,dItem):type" },
	{ 0x1527, "SetDItemType(type,@Dlog,dItem)" }, { 0x1528, "GetDItemBox(@Dlog,dItem,@Rect)" }, { 0x1529, "SetDItemBox(@Dlog,dItem,@Rect)" },
	{ 0x152A, "GetFirstDItem(@Dlog):dItem" }, { 0x152B, "GetNextDItem(@Dlog,dItem):dItem" }, { 0x152C, "ModalDialog2(@FilterProc):HitInfo/4" }, { 0x152E, "GetDItemValue(@Dlog,dItem):Val" },
	{ 0x152F, "SetDItemValue(val,@Dlog,dItem)" }, { 0x1532, "GetNewModalDialog(@DlogTmpl):@Dlog" }, { 0x1533, "GetNewDItem(@Dlog,@ItemTmpl)" }, { 0x1534, "GetAlertStage():Stage" },
	{ 0x1535, "ResetAlertStage()" }, { 0x1536, "DefaultFilter(@Dlog,@EvRec,@Hit):Flag" }, { 0x1537, "GetDefButton(@Dlog):dItem" }, { 0x1538, "SetDefButton(BtnID,@Dlog)" },
	{ 0x1539, "DisableDItem(@Dlog,dItem)" }, { 0x153A, "EnableDItem(@Dlog,dItem)" }, { 0x1601, "ScrapBootInit()" }, { 0x1602, "ScrapStartUp()" }, { 0x1603, "ScrapShutDown()" },
	{ 0x1604, "ScrapVersion():Vers" }, { 0x1605, "ScrapReset()" }, { 0x1606, "ScrapStatus():ActFlg" }, { 0x1609, "UnloadScrap()" }, { 0x160A, "LoadScrap()" },
	{ 0x160B, "ZeroScrap()" }, { 0x160C, "PutScrap(Count/4,Type,@Src)" }, { 0x160D, "GetScrap(DestH,Type)" }, { 0x160E, "GetScrapHandle(Type):ScrapH" },
	{ 0x160F, "GetScrapSize(Type):Size/4" }, { 0x1610, "GetScrapPath():@Pathname" }, { 0x1611, "SetScrapPath(@Pathname)" }, { 0x1612, "GetScrapCount():Count" },
	{ 0x1613, "GetScrapState():State" }, { 0x1614, "GetIndScrap(Index,@buffer)" }, { 0x1615, "ShowClipboard(flags,@rect):@Wind" }, { 0x1701, "SFBootInit()" },
	{ 0x1702, "SFStartUp(MemID,DirPg)" }, { 0x1703, "SFShutDown()" }, { 0x1704, "SFVersion():Vers" }, { 0x1705, "SFReset()" }, { 0x1706, "SFStatus():ActFlg" },
	{ 0x1709, "SFGetFile(X,Y,@Prmpt,@FPrc,@tL,@Reply)" }, { 0x170A, "SFPutFile(X,Y,@Prompt,@DfltName,mxL,@Reply)" }, { 0x170B, "SFPGetFile(X,Y,@P,@FPrc,@tL,@dTmp,@dHk,@Rp)" },
	{ 0x170C, "SFPPutFile(X,Y,@P,@Df,mxL,@dTmpl,@dHk,@Rply)" }, { 0x170D, "SFAllCaps(Flag)" }, { 0x170E, "SFGetFile2(X,Y,prDesc,prRef/4,@fProc,@tList,@rep)" },
	{ 0x170F, "SFPutFile2(X,Y,prDesc,prRef/4,nmDesc,nmRef/4,@rep)" }, { 0x1710, "SFPGetFile2(X,Y,@draw,prD,prRf/4,@fP,@tL,@d,@hk,@rep)" }, { 0x1711, "SFPPutFile2(X,Y,@draw,prD,prRf/4,nmD,nmRf/4,@d,@hk,@rep)" },
	{ 0x1712, "SFShowInvisible(InvisState):OldState" }, { 0x1713, "SFReScan(@filterProc,@typeList)" }, { 0x1714, "SFMultiGet2(X,Y,prDesc,prRef/4,@fP,@tL,@rep)" },
	{ 0x1715, "SFPMultiGet2(X,Y,@draw,prD,prRf/4,@fP,@tL,@d,@hk,@rep)" }, { 0x1901, "NSBootInit()" }, { 0x1902, "NSStartUp(Rate,@UpdProc)" }, { 0x1903, "NSShutDown()" },
	{ 0x1904, "NSVersion():Vers" }, { 0x1905, "NSReset()" }, { 0x1906, "NSStatus():ActFlg" }, { 0x1909, "AllocGen(Priority):Gen#" }, { 0x190A, "DeallocGen(Gen#)" },
	{ 0x190B, "NoteOn(Gen#,Semitone,Vol,@Instr)" }, { 0x190C, "NoteOff(Gen#,Semitone)" }, { 0x190D, "AllNotesOff()" }, { 0x190E, "NSSetUpdateRate(NewRate):OldRate" },
	{ 0x190F, "NSSetUserUpdateRtn(@New):@Old" }, { 0x1A01, "SeqBootInit()" }, { 0x1A02, "SeqStartUp(DirPg,Mode,Rate,Incr)" }, { 0x1A03, "SeqShutDown()" },
	{ 0x1A04, "SeqVersion():Vers" }, { 0x1A05, "SeqReset()" }, { 0x1A06, "SeqStatus():ActFlg" }, { 0x1A09, "SetIncr(Increment)" }, { 0x1A0A, "ClearIncr():OldIncr" },
	{ 0x1A0B, "GetTimer():Tick" }, { 0x1A0C, "GetLoc():Phrase,Patt,Level" }, { 0x1A0D, "SeqAllNotesOff()" }, { 0x1A0E, "SetTrkInfo(Priority,InstIndex,TrkNum)" },
	{ 0x1A0F, "StartSeq(@ErrRtn,@CompRtn,SeqH)" }, { 0x1A10, "StepSeq()" }, { 0x1A11, "StopSeq(NextFlag)" }, { 0x1A12, "SetInstTable(TableH)" }, { 0x1A13, "StartInts()" },
	{ 0x1A14, "StopInts()" }, { 0x1A15, "StartSeqRel(@errHndlr,@CompRtn,SeqH)" }, { 0x1B01, "FMBootInit()" }, { 0x1B02, "FMStartUp(MemID,DirPg)" }, { 0x1B03, "FMShutDown()" },
	{ 0x1B04, "FMVersion():Vers" }, { 0x1B05, "FMReset()" }, { 0x1B06, "FMStatus():ActFlg" }, { 0x1B09, "CountFamilies(FamSpecs):Count" }, { 0x1B0A, "FindFamily(Specs,Pos,@Name):FamNum" },
	{ 0x1B0B, "GetFamInfo(FamNum,@Name):FamStats" }, { 0x1B0C, "GetFamNum(@Name):FamNum" }, { 0x1B0D, "AddFamily(FamNum,@Name)" }, { 0x1B0E, "InstallFont(ID/4,Scale)" },
	{ 0x1B0F, "SetPurgeStat(FontID/4,PrgStat)" }, { 0x1B10, "CountFonts(ID/4,Specs):N" }, { 0x1B11, "FindFontStats(ID/4,Specs,Pos,@FStatRec)" }, { 0x1B12, "LoadFont(ID/4,Specs,Pos,@FStatRec)" },
	{ 0x1B13, "LoadSysFont()" }, { 0x1B14, "AddFontVar(FontH,NewSpecs)" }, { 0x1B15, "FixFontMenu(MenuID,StartID,FamSpecs)" }, { 0x1B16, "ChooseFont(CurrID/4,Famspecs):NewID/4" },
	{ 0x1B17, "ItemID2FamNum(ItemID):FamNum" }, { 0x1B18, "FMSetSysFont(FontID/4)" }, { 0x1B19, "FMGetSysFID():SysID/4" }, { 0x1B1A, "FMGetCurFID():CurID/4" },
	{ 0x1B1B, "FamNum2ItemID(FamNum):ItemID" }, { 0x1B1C, "InstallWithStats(ID/4,Scale,@ResultRec)" }, { 0x1C01, "ListBootInit()" }, { 0x1C02, "ListStartUp()" }, { 0x1C03, "ListShutDown()" },
	{ 0x1C04, "ListVersion():Vers" }, { 0x1C05, "ListReset()" }, { 0x1C06, "ListStatus():ActFlg" }, { 0x1C09, "CreateList(@Wind,@ListRec):CtrlH" }, { 0x1C0A, "SortList(@CompareProc,@ListRec)" },
	{ 0x1C0B, "NextMember(@Member,@ListRec):@NxtMemVal" }, { 0x1C0C, "DrawMember(@Member,@ListRec)" }, { 0x1C0D, "SelectMember(@Member,@ListRec)" }, { 0x1C0E, "GetListDefProc():@Proc" },
	{ 0x1C0F, "ResetMember(@ListRec):NxtMemVal/4" }, { 0x1C10, "NewList(@Member,@ListRec)" }, { 0x1C11, "DrawMember2(itemNum,CtrlH)" }, { 0x1C12, "NextMember2(itemNum,CtrlH):itemNum" },
	{ 0x1C13, "ResetMember2(CtrlH):itemNum" }, { 0x1C14, "SelectMember2(itemNum,CtrlH)" }, { 0x1C15, "SortList2(@CompareProc,CtrlH)" }, { 0x1C16, "NewList2(@draw,start,ref/4,refKind,size,CtrlH)" },
	{ 0x1C17, "ListKey(flags,@EventRec,CtrlH)" }, { 0x1C18, "CompareStrings(flags,@String1,@String2):Order" }, { 0x1D01, "ACEBootInit()" }, { 0x1D02, "ACEStartUp(DirPg)" },
	{ 0x1D03, "ACEShutDown()" }, { 0x1D04, "ACEVersion():Vers" }, { 0x1D05, "ACEReset()" }, { 0x1D06, "ACEStatus():ActFlg" }, { 0x1D07, "ACEInfo(Code):Value/4" },
	{ 0x1D09, "ACECompress(SrcH,SrcOff/4,DestH,DestOff/4,Blks,Method)" }, { 0x1D0A, "ACEExpand(SrcH,SrcOff/4,DestH,DestOff/4,Blks,Method)" }, { 0x1D0B, "ACECompBegin()" },
	{ 0x1D0C, "ACEExpBegin()" }, { 0x1D0D, "GetACEExpState(@Buffer)" }, { 0x1D0E, "SetACEExpState(@Buffer)" }, { 0x1E01, "ResourceBootInit()" }, { 0x1E02, "ResourceStartUp(MemID)" },
	{ 0x1E03, "ResourceShutDown()" }, { 0x1E04, "ResourceVersion():Vers" }, { 0x1E05, "ResourceReset()" }, { 0x1E06, "ResourceStatus():ActFlag" }, { 0x1E09, "CreateResourceFile(aux/4,fType,Access,@n)" },
	{ 0x1E0A, "OpenResourceFile(reqAcc,@mapAddr,@n):fileID" }, { 0x1E0B, "CloseResourceFile(fileID)" }, { 0x1E0C, "AddResource(H,Attr,rType,rID/4)" }, { 0x1E0D, "UpdateResourcefile(fileID)" },
	{ 0x1E0E, "LoadResource(rType,rID/4):H" }, { 0x1E0F, "RemoveResource(rType,rID/4)" }, { 0x1E10, "MarkResourceChange(changeFlag,rType,rID/4)" }, { 0x1E11, "SetCurResourceFile(fileID)" },
	{ 0x1E12, "GetCurResourceFile():fileID" }, { 0x1E13, "SetCurResourceApp(MemID)" }, { 0x1E14, "GetCurResourceApp():MemID" }, { 0x1E15, "HomeResourceFile(rType,rID/4):fileID" },
	{ 0x1E16, "WriteResource(rType,rID/4)" }, { 0x1E17, "ReleaseResource(PurgeLevel,rType,rID/4)" }, { 0x1E18, "DetachResource(rType,rID/4)" }, { 0x1E19, "UniqueResourceID(IDrange,rType):rID/4" },
	{ 0x1E1A, "SetResourceID(newID/4,rType,oldID/4)" }, { 0x1E1B, "GetResourceAttr(rType,rID/4):Attr" }, { 0x1E1C, "SetResourceAttr(rAttr,rType,rID/4)" }, { 0x1E1D, "GetResourceSize(rType,rID/4):Size/4" },
	{ 0x1E1E, "MatchResourceHandle(@buffer,H)" }, { 0x1E1F, "GetOpenFileRefNum(fileID):RefNum" }, { 0x1E20, "CountTypes():Num" }, { 0x1E21, "GetIndType(tIndex):rType" },
	{ 0x1E22, "CountResources(rType):Num/4" }, { 0x1E23, "GetIndResource(rType,rIndex/4):rID/4" }, { 0x1E24, "SetResourceLoad(Flag):oldFlag" }, { 0x1E25, "SetResourceFileDepth(Depth):oldDepth" },
	{ 0x1E26, "GetMapHandle(fileID):MapH" }, { 0x1E27, "LoadAbsResource(@loc,MaxSize/4,rType,rID/4):Size/4" }, { 0x1E28, "ResourceConverter(@proc,rType,logFlags)" },
	{ 0x1E29, "LoadResource2(flag,@AttrBuff,rType,rID/4):H" }, { 0x1E2A, "RMFindNamedResource(rType,@name,@fileID):rID/4" }, { 0x1E2B, "RMGetResourceName(rType,rID/4,@nameBuffer)" },
	{ 0x1E2C, "RMLoadNamedResource(rType,@name):H" }, { 0x1E2D, "RMSetResourceName(rType,rID/4,@name)" }, { 0x1E2E, "OpenResourceFileByID(reqAcc,userID):oldResApp" },
	{ 0x1E2F, "CompactResourceFile(flags,fileID)" }, { 0x2001, "MidiBootInit()" }, { 0x2002, "MidiStartUp(MemID,DirPg)" }, { 0x2003, "MidiShutDown()" },
	{ 0x2004, "MidiVersion():Vers" }, { 0x2005, "MidiReset()" }, { 0x2006, "MidiStatus():ActFlg" }, { 0x2009, "MidiControl(Function,Argument/4)" }, { 0x200A, "MidiDevice(Function,@DriverInfo)" },
	{ 0x200B, "MidiClock(Function,Argument/4)" }, { 0x200C, "MidiInfo(Function):Info/4" }, { 0x200D, "MidiReadPacket(@buff,size):Count" }, { 0x200E, "MidiWritePacket(@buff):Count" },
	{ 0x2101, "VDBootInit()" }, { 0x2102, "VDStartUp()" }, { 0x2103, "VDShutDown()" }, { 0x2104, "VDVersion():Vers" }, { 0x2105, "VDReset()" }, { 0x2106, "VDStatus():ActFlg" },
	{ 0x2109, "VDInStatus(Selector):Status" }, { 0x210A, "VDInSetStd(InStandard)" }, { 0x210B, "VDInGetStd():InStandard" }, { 0x210C, "VDInConvAdj(Selector,AdjFunction)" },
	{ 0x210D, "VDKeyControl(Selector,KeyerCtrlVal)" }, { 0x210E, "VDKeyStatus(Selector):KeyerStatus" }, { 0x210F, "VDKeySetKCol(Red,Green,Blue)" }, { 0x2110, "VDKeyGetKRCol():RedValue" },
	{ 0x2111, "VDKeyGetKGCol():GreenValue" }, { 0x2112, "VDKeyGetKBCol():BlueValue" }, { 0x2113, "VDKeySetKDiss(KDissolve)" }, { 0x2114, "VDKeyGetKDiss():KDissolve" },
	{ 0x2115, "VDKeySetNKDiss(NKDissolve)" }, { 0x2116, "VDKeyGetNKDiss():NKDissolve" }, { 0x2117, "VDOutSetStd(OutStandard)" }, { 0x2118, "VDOutGetStd():OutStandard" },
	{ 0x2119, "VDOutControl(Selector,Value)" }, { 0x211A, "VDOutStatus(Selector):OutStatus" }, { 0x211B, "VDGetFeatures(Feature):Info" }, { 0x211C, "VDInControl(Selector,Value)" },
	{ 0x211D, "VDGGControl(Selector,Value)" }, { 0x211E, "VDGGStatus(Selector):Value" }, { 0x2201, "TEBootInit()" }, { 0x2202, "TEStartUp(MemID,DirPg)" },
	{ 0x2203, "TEShutDown()" }, { 0x2204, "TEVersion():Vers" }, { 0x2205, "TEReset()" }, { 0x2206, "TEStatus():ActFlg" }, { 0x2209, "TENew(@parms):teH" },
	{ 0x220A, "TEKill(teH)" }, { 0x220B, "TESetText(tDesc,tRef/4,Len/4,stDesc,stRef/4,teH)" }, { 0x220C, "TEGetText(bDesc,bRef/4,bLen/4,stDesc,stRef/4,teH):L/4" },
	{ 0x220D, "TEGetTextInfo(@infoRec,parmCount,teH)" }, { 0x220E, "TEIdle(teH)" }, { 0x220F, "TEActivate(teH)" }, { 0x2210, "TEDeactivate(teH)" }, { 0x2211, "TEClick(@eventRec,teH)" },
	{ 0x2212, "TEUpdate(teH)" }, { 0x2213, "TEPaintText(@Port,startLn/4,@R,Flags,teH):NextLn/4" }, { 0x2214, "TEKey(@eventRec,teH)" }, { 0x2215, "[not supported]" },
	{ 0x2216, "TECut(teH)" }, { 0x2217, "TECopy(teH)" }, { 0x2218, "TEPaste(teH)" }, { 0x2219, "TEClear(teH)" }, { 0x221A, "TEInsert(tDesc,tRef/4,tLen/4,stDesc,stRef/4,teH)" }, { 0x221B, "TEReplace(tDesc,tRef/4,tLen/4,stDesc,stRef/4,teH)" },
	{ 0x221C, "TEGetSelection(@selStart,@selEnd,teH)" }, { 0x221D, "TESetSelection(selStart/4,selEnd/4,teH)" }, { 0x221E, "TEGetSelectionStyle(@stRec,stH,teH):comFlag" }, { 0x221F, "TEStyleChange(flags,@stRec,teH)" },
	{ 0x2220, "TEOffsetToPoint(offset/4,@vertPos,@horPos,teH)" }, { 0x2221, "TEPointToOffset(vertPos/4,horPos/4,teH):textOffset/4" }, { 0x2222, "TEGetDefProc():@defProc" },
	{ 0x2223, "TEGetRuler(rulerDesc,rulerRef/4,teH)" }, { 0x2224, "TESetRuler(rulerDesc,rulerRef/4,teH)" }, { 0x2225, "TEScroll(scrDesc,vertAmt/4,horAmt/4,teH):Offset/4" },
	{ 0x2226, "TEGetInternalProc():@proc" }, { 0x2227, "TEGetLastError(clearFlag,teH):lastError" }, { 0x2228, "TECompactRecord(teH)" }, { 0x2301, "MSBootInit()" },
	{ 0x2302, "MSStartUp()" }, { 0x2303, "MSShutDown()" }, { 0x2304, "MSVersion():Vers" }, { 0x2305, "MSReset()" }, { 0x2306, "MSStatus():ActFlg" }, { 0x2309, "SetBasicChannel(Channel)" },
	{ 0x230A, "SetMIDIMode(Mode)" }, { 0x230B, "PlayNote(Channel,NoteNum,KeyVel)" }, { 0x230C, "StopNote(Channel,NoteNum)" }, { 0x230D, "KillAllNotes()" },
	{ 0x230E, "SetRecTrack(TrackNum)" }, { 0x230F, "SetPlayTrack(TrackNum,State)" }, { 0x2310, "TrackToChannel(TrackNum,ChannelNum)" }, { 0x2311, "Locate(TimeStamp/4,@SeqBuff):@SeqItem" },
	{ 0x2312, "SetVelComp(VelocityOffset)" }, { 0x2313, "SetMIDIPort(EnabInput,EnabOutput)" }, { 0x2314, "SetInstrument(@InstRec,InstNum)" }, { 0x2315, "SeqPlayer(@SeqPlayerRec)" },
	{ 0x2316, "SetTempo(Tempo)" }, { 0x2317, "SetCallBack(@CallBackRec)" }, { 0x2318, "SysExOut(@Msg,Delay,@MonRoutine)" }, { 0x2319, "SetBeat(BeatDuration)" },
	{ 0x231A, "MIDIMessage(Dest,nBytes,Message,Byte1,Byte2)" }, { 0x231B, "LocateEnd(@seqBuffer):@End" }, { 0x231C, "Merge(@Buffer1,@Buffer2)" }, { 0x231D, "DeleteTrack(TrackNum,@Seq)" },
	{ 0x231E, "SetMetro(Volume,Freq,@Wave)" }, { 0x231F, "GetMSData():Reserved/4,@DirPage" }, { 0x2320, "ConvertToTime(TkPerBt,BtPerMsr,BeatNum,MsrNum):Ticks/4" }, { 0x2321, "ConvertToMeasure(TkPerBt,BtPerMsr,Ticks/4):Ticks,Beat,Msr" },
	{ 0x2322, "MSSuspend()" }, { 0x2323, "MSResume()" }, { 0x2324, "SetTuningTable(@Table)" }, { 0x2325, "GetTuningTable(@Buffer)" }, { 0x2326, "SetTrackOut(TrackNum,PathVal)" },
	{ 0x2327, "InitMIDIDriver(Slot,Internal,UserID,@Driver)" }, { 0x2328, "RemoveMIDIDriver()" }, { 0x2501, "AnimBootInit()" }, { 0x2502, "AnimStartUp(MemID,DPage,ModeFlag,@PatchTbl)" },
	{ 0x2503, "AnimShutDown()" }, { 0x2504, "AnimVersion():Vers" }, { 0x2505, "AnimReset()" }, { 0x2506, "AnimStatus():Status" },
	{ 0x2507, "CleanExit()" }, { 0x2508, "AnimIdleDebug()" }, { 0x2509, "StartScene(Mode, @Rect)" }, { 0x250A, "StopScene()" }, { 0x250B, "StartFrameTimer(Interval)" },
	{ 0x250C, "StopFrameTimer()" }, { 0x250D, "SetBackGndPort()" }, { 0x250E, "RefreshBackCall()" }, { 0x250F, "StartCharCall(PassX, PassY, PassMaxP/PassFlag, PassMinP/PassDir, LPassChar)" },
	{ 0x2510, "MoveCharCall(PassIter, PassDY/PassIter, PassDX/PassFlag, LPassChar)" }, { 0x2511, "GetCharRecPtr(CharNum):CharRecPtr" },
	{ 0x2512, "KillCharCall(CharNum)" }, { 0x2513, "LoadActor(@ActorPtr, CharNum)" }, { 0x2514, "SetCharScript(@theScript, CharNum)" }, { 0x2515, "RunAnimScripts()" },
	{ 0x2516, "FillAddressTbl(@TablePtr)" }, { 0x2517, "CreateReblitCode(@RectPtr, GenFlags)" }, { 0x2518, "StartTockTask(thePtr, theData, theID, theCount)" }, { 0x2519, "FireTockTask()" },
	{ 0x2601, "MCBootInit()" }, { 0x2602, "MCStartUp(MemID)" }, { 0x2603, "MCShutDown()" }, { 0x2604, "MCVersion():Vers" }, { 0x2605, "MCReset()" }, { 0x2606, "MCStatus():ActFlg" },
	{ 0x2609, "MCGetErrorMsg(mcErrorNo,@PStringBuff)" }, { 0x260A, "MCLoadDriver(mcChannelNo)" }, { 0x260B, "MCUnLoadDriver(mcChannelNo)" }, { 0x260C, "MCTimeToBin(mcTimeValue/4):result/4" },
	{ 0x260D, "MCBinToTime(mcBinVal/4):result/4" }, { 0x260E, "MCGetTrackTitle(mcDiskID/4,mcTrackNo,@PStringBuff)" },
	{ 0x260F, "MCSetTrackTitle(mcDiskID/4,TrackNum,@title)" }, { 0x2610, "MCGetProgram(mcDiskID/4,@resultBuff)" }, { 0x2611, "MCSetProgram(mcDiskID/4,@mcProg)" },
	{ 0x2612, "MCGetDiscTitle(mcDiskID/4,@PStringBuff)" }, { 0x2613, "MCSetDiscTitle(mcDiskID/4,@title)" }, { 0x2614, "MCDStartUp(mcChannelNo,@portName,userID)" },
	{ 0x2615, "MCDShutDown(mcChannelNo)" }, { 0x2616, "MCGetFeatures(mcChannelNo,mcFeatSel):result/4" }, { 0x2617, "MCPlay(mcChannelNo)" }, { 0x2618, "MCPause(mcChannelNo)" },
	{ 0x2619, "MCSendRawData(mcChannelNo,@mcNative)" }, { 0x261A, "MCGetStatus(mcChannelNo,mcStatusSel):result" }, { 0x261B, "MCControl(mcChannelNo,ctlCommand)" },
	{ 0x261C, "MCScan(mcChannelNo,mcDirection)" }, { 0x261D, "MCGetSpeeds(mcChannelNo,@PStringBuff)" }, { 0x261E, "MCSpeed(mcChannelNo,mcFPS)" },
	{ 0x261F, "MCStopAt(mcChannelNo,mcUnitType,mcStopLoc/4)" }, { 0x2620, "MCJog(mcChannelNo,mcUnitType,mcNJog/4,mcJogRepeat)" }, { 0x2621, "MCSearchTo(mcChannelNo,mcUnitType,searchLoc/4)" },
	{ 0x2622, "MCSearchDone(mcChannelNo):result" }, { 0x2623, "MCSearchWait(mcChannelNo)" }, { 0x2624, "MCGetPosition(mcChannelNo,mcUnitType):result/4" }, { 0x2625, "MCSetAudio(mcChannelNo,mcAudioCtl)" },
	{ 0x2626, "MCGetTimes(mcChannelNo,mctimesSel):result/4" }, { 0x2627, "MCGetDiscTOC(mcChannelNo,mcTrackNo):result/4" }, { 0x2628, "MCGetDiscID(mcChannelNo):result/4" },
	{ 0x2629, "MCGetNoTracks(mcChannelNo):result" }, { 0x262A, "MCRecord(mcChannelNo)" }, { 0x262B, "MCStop(mcChannelNo)" }, { 0x262C, "MCWaitRawData(mcChannelNo,@result,tickWait,termMask)" },
	{ 0x262D, "MCGetName(mcChannelNo,@PStringBuff)" }, { 0x262E, "MCSetVolume(mcChannelNo,mcLeftVol,mcRightVol)" }, { 0x3601, "TCPIPBootInit()" }, { 0x3602, "TCPIPStartUp()" },
	{ 0x3603, "TCPIPShutDown()" }, { 0x3604, "TCPIPVersion():Vers" }, { 0x3605, "TCPIPReset()" }, { 0x3606, "TCPIPStatus():ActFlg" }, { 0x3608, "TCPIPLongVersion():rVersion/4" },
	{ 0x3609, "TCPIPGetConnectStatus():connectedFlag" }, { 0x360A, "TCPIPGetErrorTable():@errTablePtr" }, { 0x360B, "TCPIPGetReconnectStatus():reconnectFlag" }, { 0x360C, "TCPIPReconnect(@displayPtr)" },
	{ 0x360D, "TCPIPConvertIPToHex(@cvtRecPtr,@ddippstring)" }, { 0x360E, "TCPIPConvertIPToASCII(ipaddress/4,@ddpstring,flags):strlen" }, { 0x360F, "TCPIPGetMyIPAddress():ipaddress/4" },
	{ 0x3610, "TCPIPGetConnectMethod():method" }, { 0x3611, "TCPIPSetConnectMethod(method)" }, { 0x3612, "TCPIPConnect(@displayPtr)" }, { 0x3613, "TCPIPDisconnect(forceflag,@displayPtr)" },
	{ 0x3614, "TCPIPGetMTU():mtu" }, { 0x3615, "TCPIPValidateIPCString(@cstringPtr):validFlag" }, { 0x3616, "TCPIPGetConnectData(userid,method):H" }, { 0x3617, "TCPIPSetConnectData(method,H)" },
	{ 0x3618, "TCPIPGetDisconnectData(userid,method):H" }, { 0x3619, "TCPIPSetDisconnectData(method,H)" }, { 0x361A, "TCPIPLoadPreferences()" }, { 0x361B, "TCPIPSavePreferences()" },
	{ 0x361C, "TCPIPGetDNS(@DNSRecPtr)" }, { 0x361D, "TCPIPSetDNS(@DNSRecPtr)" }, { 0x361E, "TCPIPGetTuningTable(@tunePtr)" }, { 0x361F, "TCPIPSetTuningTable(@tunePtr)" },
	{ 0x3620, "TCPIPCancelDNR(@dnrBufferPtr)" }, { 0x3621, "TCPIPDNRNameToIP(@nameptr,@dnrBufferPtr)" }, { 0x3622, "TCPIPPoll()" }, { 0x3623, "TCPIPLogin(userid,destip/4,destport,defaultTOS,defaultTTL):ipid" },
	{ 0x3624, "TCPIPLogout(ipid)" }, { 0x3625, "TCPIPSendICMP(ipid,@messagePtr,messageLen)" }, { 0x3626, "TCPIPSendUDP(ipid,@udpPtr,udpLen)" }, { 0x3627, "TCPIPGetDatagramCount(ipid,protocol):dgmCount" },
	{ 0x3628, "TCPIPGetNextDatagram(ipid,protocol,flags):H" }, { 0x3629, "TCPIPGetLoginCount():loginCount" }, { 0x362A, "TCPIPSendICMPEcho(ipid,seqNum)" },
	{ 0x362B, "TCPIPReceiveICMPEcho(ipid):seqNum" }, { 0x362C, "TCPIPOpenTCP(ipid):tcpError" }, { 0x362D, "TCPIPWriteTCP(ipid,@dataPtr,dataLength/4,pushFlag,urgentFlag):tcpError" },
	{ 0x362E, "TCPIPReadTCP(ipid,buffType,buffData/4,buffLen/4,@rrBuffPtr):tcpError" }, { 0x362F, "TCPIPCloseTCP(ipid):tcpError" }, { 0x3630, "TCPIPAbortTCP(ipid):tcpError" },
	{ 0x3631, "TCPIPStatusTCP(ipid,@srBuffPtr):tcpError" }, { 0x3632, "TCPIPGetSourcePort(ipid):sourcePort" }, { 0x3633, "TCPIPGetTOS(ipid):TOS" }, { 0x3634, "TCPIPSetTOS(ipid,TOS)" },
	{ 0x3635, "TCPIPGetTTL(ipid):TTL" }, { 0x3636, "TCPIPSetTTL(ipid,TTL)" }, { 0x3637, "TCPIPSetSourcePort(ipid,sourcePort)" }, { 0x3638, "TCPIPSetMyIPAddress(ipaddress/4)" },
	{ 0x3639, "TCPIPGetDP():dp" }, { 0x363A, "TCPIPGetDebugHex():debugFlag" }, { 0x363B, "TCPIPDebugHex(debugFlag)" }, { 0x363C, "TCPIPGetDebugTCP():debugFlag" },
	{ 0x363D, "TCPIPDebugTCP(debugFlag)" }, { 0x363E, "TCPIPGetUserRecord(ipid):userRecEntry/4" }, { 0x363F, "TCPIPConvertIPCToHex(@cvtRecPtr,@ddipcstring)" },
	{ 0x3640, "TCPIPSendIPDatagram(@datagramPtr)" }, { 0x3641, "TCPIPConvertIPToClass(ipaddress/4):class" }, { 0x3642, "TCPIPGetConnectMsgFlag():conMsgFlag" },
	{ 0x3643, "TCPIPSetConnectMsgFlag(conMsgFlag)" }, { 0x3644, "TCPIPGetUsername(@unBuffPtr)" }, { 0x3645, "TCPIPSetUsername(@usernamePtr)" }, { 0x3646, "TCPIPGetPassword(@pwBuffPtr)" },
	{ 0x3647, "TCPIPSetPassword(@passwordPtr)" }, { 0x3648, "TCPIPValidateIPString(@pstringPtr):validFlag" }, { 0x3649, "TCPIPGetUserStatistic(ipid,statisticNum):statistic/4" }, { 0x364A, "TCPIPGetLinkVariables():@variablesPtr" },
	{ 0x364B, "TCPIPEditLinkConfig(connectHandle/4,disconnectHandle/4)" }, { 0x364C, "TCPIPGetModuleNames():@moduleListPtr" }, { 0x364D, "TCPIPRebuildModuleList()" },
	{ 0x364E, "TCPIPListenTCP(ipid):tcpError" }, { 0x364F, "TCPIPAcceptTCP(ipid,reserved):newipid" }, { 0x3650, "TCPIPSetNewDestination(ipid,destip/4,destport)" }, { 0x3651, "TCPIPGetHostName(@hnBuffPtr)" },
	{ 0x3652, "TCPIPSetHostName(@hostNamePtr)" }, { 0x3653, "TCPIPStatusUDP(ipid,@udpVarsPtr)" }, { 0x3654, "TCPIPGetLinkLayer(@linkInfoBlkPtr)" }, { 0x3655, "TCPIPPtrToPtr(@from,@to,len/4)" },
	{ 0x3656, "TCPIPPtrToPtrNeg(@fromend,@toend,len/4)" }, { 0x3657, "TCPIPGetAuthMessage(userid):authMsgHandle/4" },
	{ 0x3658, "TCPIPConvertIPToCASCII(ipaddress/4,@ddcstring,flags):strlen" }, { 0x3659, "TCPIPMangleDomainName(flags,@dnPstringPtr):port" },
	{ 0x365A, "TCPIPGetAliveFlag():aliveFlag" }, { 0x365B, "TCPIPSetAliveFlag(aliveFlag)" }, { 0x365C, "TCPIPGetAliveMinutes():aliveMinutes" },
	{ 0x365D, "TCPIPSetAliveMinutes(aliveMinutes)" }, { 0x365E, "TCPIPReadLineTCP(ipid,@delimitStrPtr,buffType,buffData/4,buffLen/4,@rrBuffPtr):tcpError" },
	{ 0x365F, "TCPIPGetBootConnectFlag():bootConnectFlag" }, { 0x3660, "TCPIPSetBootConnectFlag(bootConnectFlag)" }, { 0x3661, "TCPIPSetUDPDispatch(ipid,dispatchFlag)" },
	{ 0x3662, "TCPIPGetDestination(ipid,@destRecPtr)" }, { 0x3663, "TCPIPGetUserEventTrigger(triggerNumber,ipid):triggerProcPtr/4" }, { 0x3664, "TCPIPSetUserEventTrigger(triggerNumber,ipid,@triggerProcPtr)" },
	{ 0x3665, "TCPIPGetSysEventTrigger(triggerNumber):triggerProcPtr/4" }, { 0x3666, "TCPIPSetSysEventTrigger(triggerNumber,@triggerProcPtr)" }, { 0x3667, "TCPIPGetDNRTimeouts(@dnrTimeoutsBuffPtr)" },
	{ 0x3668, "TCPIPSetDNRTimeouts(@dnrTimeoutsBuffPtr)" },
};

static const struct dasm_data gsos_calls[] =
{
	{ 0x0001, "CREATE" }, { 0x0002, "DESTROY" }, { 0x0004, "CHANGE_PATH" }, { 0x0005, "SET_FILE_INFO" },
	{ 0x0006, "GET_FILE_INFO" }, { 0x0008, "VOLUME" }, { 0x0009, "SET_PREFIX" }, { 0x000A, "GET_PREFIX" },
	{ 0x000B, "CLEAR_BACKUP_BIT" }, { 0x0010, "OPEN" }, { 0x0011, "NEWLINE" }, { 0x0012, "READ" },
	{ 0x0013, "WRITE" }, { 0x0014, "CLOSE" }, { 0x0015, "FLUSH" }, { 0x0016, "SET_MARK" },
	{ 0x0017, "GET_MARK" }, { 0x0018, "SET_EOF" }, { 0x0019, "GET_EOF" }, { 0x001A, "SET_LEVEL" },
	{ 0x001B, "GET_LEVEL" }, { 0x001C, "GET_DIR_ENTRY" }, { 0x0020, "GET_DEV_NUM" }, { 0x0021, "GET_LAST_DEV" },
	{ 0x0022, "READ_BLOCK" }, { 0x0023, "WRITE_BLOCK" }, { 0x0024, "FORMAT" }, { 0x0025, "ERASE_DISK" },
	{ 0x0027, "GET_NAME" }, { 0x0028, "GET_BOOT_VOL" }, { 0x0029, "QUIT" }, { 0x002A, "GET_VERSION" },
	{ 0x002C, "D_INFO" }, { 0x0031, "ALLOC_INTERRUPT" }, { 0x0032, "DEALLOC_INTERRUPT" }, { 0x2001, "CreateGS" },
	{ 0x2002, "DestroyGS" }, { 0x2003, "OSShutdownGS" }, { 0x2004, "ChangePathGS" }, { 0x2005, "SetFileInfoGS" },
	{ 0x2006, "GetFileInfoGS" }, { 0x2007, "JudgeNameGS" }, { 0x2008, "VolumeGS" }, { 0x2009, "SetPrefixGS" },
	{ 0x200A, "GetPrefixGS" }, { 0x200B, "ClearBackupGS" }, { 0x200C, "SetSysPrefsGS" }, { 0x200D, "NullGS" },
	{ 0x200E, "ExpandPathGS" }, { 0x200F, "GetSysPrefsGS" }, { 0x2010, "OpenGS" }, { 0x2011, "NewLineGS" },
	{ 0x2012, "ReadGS" }, { 0x2013, "WriteGS" }, { 0x2014, "CloseGS" }, { 0x2015, "FlushGS" },
	{ 0x2016, "SetMarkGS" }, { 0x2017, "GetMarkGS" }, { 0x2018, "SetEOFGS" }, { 0x2019, "GetEOFGS" },
	{ 0x201A, "SetLevelGS" }, { 0x201B, "GetLevelGS" }, { 0x201C, "GetDirEntryGS" }, { 0x201D, "BeginSessionGS" },
	{ 0x201E, "EndSessionGS" }, { 0x201F, "SessionStatusGS" }, { 0x2020, "GetDevNumberGS" }, { 0x2024, "FormatGS" },
	{ 0x2025, "EraseDiskGS" }, { 0x2026, "ResetCacheGS" }, { 0x2027, "GetNameGS" }, { 0x2028, "GetBootvolGS" },
	{ 0x2029, "QuitGS" }, { 0x202A, "GetVersionGS" }, { 0x202B, "GetFSTInfoGS" }, { 0x202C, "DInfoGS" },
	{ 0x202D, "DStatusGS" }, { 0x202E, "DControlGS" }, { 0x202F, "DReadGS" }, { 0x2030, "DWriteGS" },
	{ 0x2031, "BindIntGS" }, { 0x2032, "UnbindIntGS" }, { 0x2033, "FSTSpecificGS" }, { 0x2034, "AddNotifyProcGS" },
	{ 0x2035, "DelNotifyProcGS" }, { 0x2036, "DRenameGS" }, { 0x2037, "GetStdRefNumGS" }, { 0x2038, "GetRefNumGS" },
	{ 0x2039, "GetRefInfoGS" }, { 0x203A, "SetStdRefNumGS" },
};

static const struct dasm_data32 gs_vectors[] =
{
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
	{ 0x01FCAC, "SysSrv:S_GET_BOOT_PFX" }, { 0x01FCB0, "SysSrv:S_SET_BOOT_PFX" }, { 0x01FCB4, "SysSrv:LOW_ALLOCATE" },
	{ 0x01FCB8, "SysSrv:GET_STACKED_ID" }, { 0x01FCBC, "SysSrv:DYN_SLOT_ARBITER" }, { 0x01FCC0, "SysSrv:PARSE_PATH" },
	{ 0x01FCC4, "SysSrv:OS_EVENT" }, { 0x01FCC8, "SysSrv:INSERT_DRIVER" }, { 0x01FCCC, "SysSrv:(device manager?)" },
	{ 0x01FCD0, "SysSrv:Old Device Dispatcher" }, { 0x01FCD4, "SysSrv:INIT_PARSE_PATH" }, { 0x01FCD8, "SysSrv:UNBIND_INT_VEC" },
	{ 0x01FCDC, "SysSrv:DO_INSERT_SCAN" }, { 0x01FCE0, "SysSrv:TOOLBOX_MSG" },
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
	{ 0xE01DBA, "OldInGlob" }, { 0xE01DBE, "RealDeskStat" }, { 0xE01DC0, "Next" }, { 0xE01DDE, "SchActive" }, { 0xE01DDF, "TaskQueue/FirstTask" },
	{ 0xE01DE3, "SecondTask" }, { 0xE01DED, "Scheduler" }, { 0xE01DEF, "Offset" }, { 0xE01DFF, "Lastbyte" },
	{ 0xE01E04, "QD:StdText" }, { 0xE01E08, "QD:StdLine" }, { 0xE01E0C, "QD:StdRect" }, { 0xE01E10, "QD:StdRRect" }, { 0xE01E14, "QD:StdOval" }, { 0xE01E18, "QD:StdArc" }, { 0xE01E1C, "QD:StdPoly" },
	{ 0xE01E20, "QD:StdRgn" }, { 0xE01E24, "QD:StdPixels" }, { 0xE01E28, "QD:StdComment" }, { 0xE01E2C, "QD:StdTxMeas" }, { 0xE01E30, "QD:StdTxBnds" }, { 0xE01E34, "QD:StdGetPic" },
	{ 0xE01E38, "QD:StdPutPic" }, { 0xE01E98, "QD:ShieldCursor" }, { 0xE01E9C, "QD:UnShieldCursor" },
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
	u16 operand = opcodes.r16(pc+1);

	// check for GS-specific stores with the databank set
	if (m_GScpu)
	{
		u32 bank = m_GScpu->g65816_get_reg(g65816_device::G65816_DB) << 16;
		bank |= operand;

		if (const auto symbol = find_symbol(gs_vectors, std::end(gs_vectors), bank))
		{
			stream << opname << " " << symbol->name;
			return 3 | util::disasm_interface::SUPPORTED;
		}

		// if we failed the GS stuff and the DB isn't a special bank, don't match the classic A2 switches.
		bank >>= 16;
		bank &= 0xff;
		if ((bank != 0) && (bank != 1) && (bank != 0xe0) && (bank != 0xe1))
		{
			return 0;
		}
	}

	if (const auto symbol = find_symbol(a2_stuff, std::end(a2_stuff), operand))
	{
		stream << opname << " " << symbol->name;
		return 3 | util::disasm_interface::SUPPORTED;
	}
	return 0;
}

offs_t apple2_common_device::com_long_op(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const char *opname)
{
	u32 operand = opcodes.r32(pc) >> 8;
	if (const auto symbol = find_symbol(gs_vectors, std::end(gs_vectors), operand))
	{
		stream << opname << " >" << symbol->name;
		return 4 | util::disasm_interface::SUPPORTED;
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
		// 6502/65c02 direct page
		case 0x04: return com_2byte_op(stream, pc, opcodes, "tsb");
		case 0x05: return com_2byte_op(stream, pc, opcodes, "ora");
		case 0x06: return com_2byte_op(stream, pc, opcodes, "asl");
		case 0x14: return com_2byte_op(stream, pc, opcodes, "trb");
		case 0x24: return com_2byte_op(stream, pc, opcodes, "bit");
		case 0x25: return com_2byte_op(stream, pc, opcodes, "and");
		case 0x26: return com_2byte_op(stream, pc, opcodes, "rol");
		case 0x45: return com_2byte_op(stream, pc, opcodes, "eor");
		case 0x46: return com_2byte_op(stream, pc, opcodes, "lsr");
		case 0x64: return com_2byte_op(stream, pc, opcodes, "stz");
		case 0x65: return com_2byte_op(stream, pc, opcodes, "adc");
		case 0x66: return com_2byte_op(stream, pc, opcodes, "ror");
		case 0x84: return com_2byte_op(stream, pc, opcodes, "sty");
		case 0x85: return com_2byte_op(stream, pc, opcodes, "sta");
		case 0x86: return com_2byte_op(stream, pc, opcodes, "stx");
		case 0xa4: return com_2byte_op(stream, pc, opcodes, "ldy");
		case 0xa5: return com_2byte_op(stream, pc, opcodes, "lda");
		case 0xa6: return com_2byte_op(stream, pc, opcodes, "ldx");
		case 0xc4: return com_2byte_op(stream, pc, opcodes, "cpy");
		case 0xc5: return com_2byte_op(stream, pc, opcodes, "cmp");
		case 0xc6: return com_2byte_op(stream, pc, opcodes, "dec");
		case 0xe4: return com_2byte_op(stream, pc, opcodes, "cpx");
		case 0xe5: return com_2byte_op(stream, pc, opcodes, "sbc");
		case 0xe6: return com_2byte_op(stream, pc, opcodes, "inc");

		// 6502/65c02 absolute
		case 0x0c: return com_3byte_op(stream, pc, opcodes, "tsb");
		case 0x0d: return com_3byte_op(stream, pc, opcodes, "ora");
		case 0x0e: return com_3byte_op(stream, pc, opcodes, "asl");
		case 0x1c: return com_3byte_op(stream, pc, opcodes, "trb");
		case 0x2c: return com_3byte_op(stream, pc, opcodes, "bit");
		case 0x2d: return com_3byte_op(stream, pc, opcodes, "and");
		case 0x2e: return com_3byte_op(stream, pc, opcodes, "rol");
		case 0x4c: return com_3byte_op(stream, pc, opcodes, "jmp");
		case 0x4d: return com_3byte_op(stream, pc, opcodes, "eor");
		case 0x4e: return com_3byte_op(stream, pc, opcodes, "lsr");
		case 0x6d: return com_3byte_op(stream, pc, opcodes, "adc");
		case 0x6e: return com_3byte_op(stream, pc, opcodes, "ror");
		case 0x8c: return com_3byte_op(stream, pc, opcodes, "sty");
		case 0x8d: return com_3byte_op(stream, pc, opcodes, "sta");
		case 0x8e: return com_3byte_op(stream, pc, opcodes, "stx");
		case 0x9c: return com_3byte_op(stream, pc, opcodes, "stz");
		case 0xac: return com_3byte_op(stream, pc, opcodes, "ldy");
		case 0xad: return com_3byte_op(stream, pc, opcodes, "lda");
		case 0xae: return com_3byte_op(stream, pc, opcodes, "ldx");
		case 0xcc: return com_3byte_op(stream, pc, opcodes, "cpy");
		case 0xcd: return com_3byte_op(stream, pc, opcodes, "cmp");
		case 0xce: return com_3byte_op(stream, pc, opcodes, "dec");
		case 0xec: return com_3byte_op(stream, pc, opcodes, "cpx");
		case 0xed: return com_3byte_op(stream, pc, opcodes, "sbc");
		case 0xee: return com_3byte_op(stream, pc, opcodes, "inc");

		case 0x20:   // JSR
			{
				u16 operand = opcodes.r16(pc + 1);
				if (operand == 0xbf00)
				{
					u8 p8call = opcodes.r8(pc + 3);
					u16 p8params = opcodes.r16(pc + 4);

					if (const auto symbol = find_symbol(p8_calls, std::end(p8_calls), p8call))
					{
						util::stream_format(stream, "jsr ProDOS 8: %s ($%04x)", symbol->name, p8params);
						return 6 | util::disasm_interface::SUPPORTED | util::disasm_interface::STEP_OVER;
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
		// 65816 absolute long
		case 0x0f: return com_long_op(stream, pc, opcodes, "ora");
		case 0x2f: return com_long_op(stream, pc, opcodes, "and");
		case 0x4f: return com_long_op(stream, pc, opcodes, "eor");
		case 0x5c: return com_long_op(stream, pc, opcodes, "jml");
		case 0x6f: return com_long_op(stream, pc, opcodes, "adc");
		case 0x8f: return com_long_op(stream, pc, opcodes, "sta");
		case 0xaf: return com_long_op(stream, pc, opcodes, "lda");
		case 0xcf: return com_long_op(stream, pc, opcodes, "cmp");
		case 0xef: return com_long_op(stream, pc, opcodes, "sbc");

		case 0xa2:  // LDX
			if (opcodes.r32(pc + 3) == 0xe1000022)  // JSL E10000
			{
				u16 toolcall = opcodes.r16(pc + 1);
				u16 toolswap = swapendian_int16(toolcall);

				if (const auto symbol = find_symbol(gs_tools, std::end(gs_tools), toolswap))
				{
					util::stream_format(stream, "_%s (%04x)", symbol->name, toolcall);
					return 7 | util::disasm_interface::SUPPORTED | util::disasm_interface::STEP_OVER;
				}
			}
			break;

		case 0x22:  // JSL
			{
				u32 operand = opcodes.r32(pc) >> 8;
				if (operand == 0xe100a8) // ProDOS 16
				{
					u16 call = opcodes.r8(pc + 4);
					u32 params = opcodes.r32(pc + 6) & 0xffffff;

					if (const auto symbol = find_symbol(gsos_calls, std::end(gsos_calls), call))
					{
						util::stream_format(stream, "_%s %06x", symbol->name, params);
						return 10 | util::disasm_interface::SUPPORTED | util::disasm_interface::STEP_OVER;
					}
				}
				offs_t rv = com_long_op(stream, pc, opcodes, "jsl");

				if (rv > 0)
					return rv | util::disasm_interface::STEP_OVER;

#if 0 // very slow in debug_data_buffer::fill()
				// jsl to inline debug name?
				if (opcodes.r8(operand) == 0x82 && opcodes.r16(operand + 3) == 0x7771)
				{
					int n = opcodes.r8(operand + 5);
					std::string name;
					for (int i = 0; i < n; ++i)
						name.push_back(opcodes.r8(operand + 6 + i));

					util::stream_format(stream, "jsl %s (%06x)", name.c_str(), operand);
					return 4 | util::disasm_interface::SUPPORTED | util::disasm_interface::STEP_OVER;
				}
#endif
			}
			break;

		case 0x82:  // BRL
			if (opcodes.r16(pc + 3) == 0x7771)
			{
				// inline debug name format (Apple IIGS Technical Note #103):
				// 82 xx xx      brl past name
				// 71 77         signature
				// nn xx xx xxx  pascal string
				//               pastname
				s16 operand = opcodes.r16(pc + 1);
				int n = opcodes.r8(pc + 5);
				if (operand >= n)
				{
					std::string name;
					for (int i = 0; i < n; ++i)
						name.push_back(opcodes.r8(pc + 6 + i));

					stream << name;
					return (operand + 3) | util::disasm_interface::SUPPORTED;
				}
			}
			break;

		default:
			break;
	}

	if (!result)
	{
		return dasm_override(stream, pc, opcodes, params);
	}

	return result;
}
