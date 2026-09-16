// license:BSD-3-Clause
// copyright-holders:Angelo Salese

// SWI lookup table for Portfolio OS
// bp 8,1,{history 0,2;g}

#include "emu.h"
#include "3do_portfolio.h"

static const char *lookup_swi(uint32_t opcode)
{
	static const struct
	{
		uint32_t swi_number;
		const char *name;
	} SWIs[] =
	{
//      { 0x00010, "<UNKNOWN>" },
		{ 0x00101, "void Debug(void);" },
		{ 0x10000, "Item CreateSizedItem(int32 ctype, TagArg *p, int32 size);" },
		{ 0x10001, "int32 WaitSignal(uint32 sigMask);" },
		{ 0x10002, "Err SendSignal(Item task, uint32 sigMask);" },
		{ 0x10003, "Err DeleteItem(Item i);" },
		{ 0x10004, "Item FindItem(int32 ctype, TagArg *tp);" },
		{ 0x10005, "Item OpenItem(Item foundItem, void *args);" },
//      { 0x10006, "Err UnlockSemaphore(Item s);" },
		{ 0x10006, "Err UnlockItem(Item s);" },
//      { 0x10007, "int32 LockSemaphore(Item s, uint32 flags);" },
		{ 0x10007, "int32 LockItem(Item s, uint32 flags);" },
		{ 0x10008, "Err CloseItem(Item i);" },
		{ 0x10009, "void Yield(void);" },
		{ 0x1000A, "int32 SetItemPri(Item i, uint8 newpri);" },
		{ 0x1000D, "void *AllocMemBlocks(int32 size, uint32 typebits);" },
		{ 0x1000E, "void kprintf(const char*fmt, ...);" },
		{ 0x1000F, "Item GetThisMsg(Item msg);" },
		{ 0x10010, "Err SendMsg(Item mp, Item msg, const void *dataptr, int32 datasize);" },
//      { 0x10010, "Err SendSmallMsg(Item mp, Item msg, uint32 val1, uint32 val2);" },
		{ 0x10011, "uint32 ReadHardwareRandomNumber(void);" },
		{ 0x10012, "Err ReplyMsg(Item msg, int32 result, const void *dataptr, int32 datasize);" },
//      { 0x10012, "Err ReplySmallMsg(Item msg, int32 result, uint32 val1, int32 val2);" },
		{ 0x10013, "Item GetMsg(Item mp);" },
		{ 0x10014, "Err ControlMem(void *p, int32 size, int32 cmd, Item task);" },
		{ 0x10015, "int32 AllocSignal(uint32 sigMask);" },
		{ 0x10016, "Err FreeSignal(uint32 sigMask);" },
		{ 0x10018, "Err SendIO(Item ior, const IOInfo *ioiP);" },
		{ 0x10019, "Err AbortIO(Item ior);" },
		{ 0x1001A, "int32 SetItemPri(Item i, uint8 newpri);" },
		{ 0x1001C, "Err SetItemOwner(Item i, Item newOwner);" },
//      { 0x1001D, "<UNKNOWN>" },
		{ 0x1001E, "int MayGetChar(void);" },
		{ 0x10021, "int32 SystemScavengeMem(void);" },
		{ 0x10024, "Item FindAndOpenItem(int32 ctype, TagArg *tp);" },
		{ 0x10025, "Err DoIO(Item ior, const IOInfo *ioiP);" },
		{ 0x10026, "uint32 SampleSystemTime(void);" },
		{ 0x10027, "Err SetExitStatus(int32 status);" },
		{ 0x10028, "Item WaitPort(Item mp, Item msg);" },
		{ 0x10029, "Err WaitIO(Item ior);" },
//      { 0x1002A, "<UNKNOWN>" },

		{ 0x20000, "DEFUNCT - grafinit" },
		{ 0x20001, "Err SetReadAddress(Item bitmapItem, ubyte *buffer, int32 width);" },
		{ 0x20002, "Err ResetReadAddress(Item bitmapItem);" },
		{ 0x20003, "Err SetClipOrigin(Item bitmapItem, int32 x, int32 y);" },
		{ 0x20004, "DEFUNCT - WaitForLine" },
		{ 0x20005, "Err EnableVAVG(Item screenItem);" },
		{ 0x20006, "Err DisableVAVG(Item screenItem);" },
		{ 0x20007, "Err EnableHAVG(Item screenItem);" },
		{ 0x20008, "Err DisableHAVG(Item screenItem);" },
		{ 0x20009, "Err SetScreenColor(Item screenItem, uint32 colorEntry);" },
		{ 0x2000A, "Err ResetScreenColors(Item screenItem);" },
		{ 0x2000B, "DEFUNCT - ResetSystemGraphics" },
		{ 0x2000C, "None" },
		{ 0x2000D, "Err SetScreenColors(Item screenItem, uint32 *entries, int32 count);" },
		{ 0x2000E, "swiSuperResetCurrentFont" },
		{ 0x2000F, "None" },
		{ 0x20010, "None" },
		{ 0x20011, "Err AddScreenGroup(Item screenGroup, TagArg *targs);" },
		{ 0x20012, "Err RemoveScreenGroup(Item screenGroup);" },
		{ 0x20013, "Err SetClipWidth(Item bitmapItem, int32 clipWidth);" },
		{ 0x20014, "Err SetClipHeight(Item bitmapItem, int32 clipHeight);" },
		{ 0x20015, "None" },
		{ 0x20016, "None" },
		{ 0x20017, "Err DrawScreenCels(Item screenItem, CCB *ccb);" },
		{ 0x20018, "DEFUNCT - FillEllipse" },
		{ 0x20019, "DEFUNCT - swiSuperOpenFileFont" },
		{ 0x2001A, "DEFUNCT - SetFileFontCacheSize" },
		{ 0x2001B, "swiSuperOpenRAMFont" },
		{ 0x2001C, "DEFUNCT - OpenFileFont" },
		{ 0x2001D, "Err DrawText16(GrafCon *gcon, Item bitmapItem, uint16 *text);" },
		{ 0x2001E, "DEFUNCT - swiSuperCloseFont" },
		{ 0x2001F, "Err DrawChar(GrafCon *gcon, Item bitmapItem, uint32 character);" },
		{ 0x20020, "None" },
		{ 0x20021, "Err DrawTo(Item bitmapItem, GrafCon *grafcon, Coord x, Coord y);" },
		{ 0x20022, "None" },
		{ 0x20023, "Err FillRect(Item bitmapItem, GrafCon *gc, Rect *r);" },
		{ 0x20024, "Err SetCurrentFontCCB(CCB *ccb);" },
		{ 0x20025, "Font* GetCurrentFont(void);" },
		{ 0x20026, "Err DrawText8(GrafCon *gcon, Item bitmapItem, uint8 *text);" },
		{ 0x20027, "Err DrawCels(Item bitmapItem, CCB *ccb);" },
		{ 0x20028, "None" },
		{ 0x20029, "Err SetCEControl(Item bitmapItem, int32 controlWord, int32 controlMask);" },
		{ 0x2002A, "Err SetCEWatchDog(Item bitmapItem, int32 db_ctr);" },
		{ 0x2002B, "DEFUNCT - CopyRect" },
		{ 0x2002C, "DEFUNCT - DeleteScreenGroup" },
		{ 0x2002D, "Err DisplayScreen(Item screenItem0, Item screenItem1);" },
		{ 0x2002E, "DEFUNCT - DeleteVDL" },
		{ 0x2002F, "Err SetVDL(Item screenItem, Item vdlItem);" },
		{ 0x20030, "Item SubmitVDL(VDLEntry *VDLDataPtr, int32 length, int32 type);" },
		{ 0x20031, "None" },
		{ 0x20032, "realCreateScreenGroup" },
		{ 0x20033, "Err ModifyVDL(Item vdlItem, TagArg* vdlTags);" },

		{ 0x30000, "Item OpenDiskFile(char *path);" },
		{ 0x30001, "int32 CloseDiskFile(Item fileItem);" },
//      { 0x30002, "<UNKNOWN>" },
		{ 0x30004, "Item MountFileSystem(Item deviceItem, int32 unit, uint32 blockOffset);" },
		{ 0x30005, "Item OpenDiskFileInDir(Item dirItem, char *path);" },
		{ 0x30006, "Item MountMacFileSystem(char *path);" },
		{ 0x30007, "Item ChangeDirectory(char *path);" },
		{ 0x30008, "Item GetDirectory(char *pathBuf, int pathBufLen);" },
		{ 0x30009, "Item CreateFile(char *path);" },
		{ 0x3000A, "Err DeleteFile(char *path);" },
		{ 0x3000B, "Item CreateAlias(char *aliasPath, char *realPath);" },
		{ 0x3000D, "Err DismountFileSystem(char *name);" },

		{ 0x40000, "Err TweakKnob(Item KnobItem, int32 Value);" },
		{ 0x40001, "Err StartInstrument(Item Instrument, TagArg *TagList);" },
		{ 0x40002, "Err ReleaseInstrument(Item Instrument, TagArg *TagList);" },
		{ 0x40003, "Err StopInstrument(Item Instrument, TagArg *TagList);" },
		{ 0x40004, "Err TuneInsTemplate(Item Instrument, Item Tuning);" },
		{ 0x40005, "Err TunInstrument(Item Instrument, Item Tuning);" },
		{ 0x40008, "Err ConnectInstruments(Item SrcIns, char *SrcName, Item DstIns, char *DstName);" },
		{ 0x40009, "uint32 TraceAudio(int32 Mask);" },
		{ 0x4000A, "int32 AllocAmplitude(int32 Amplitude);" },
		{ 0x4000B, "Err FreeAmplitude(int32 Amplitude);" },
		{ 0x4000C, "Err DisconnectInstruments(Item SrcIns, char *SrcName, Item DstIns, char *DstName);" },
		{ 0x4000D, "Err SignalAtTime(Item Cue, AudioTime Time);" },
//      { 0x4000E, "<UNKNOWN>" },
		{ 0x4000F, "Err SetAudioRate(Item Owner, frac16 Rate);" },
		{ 0x40010, "Err SetAudioDuration(Item Owner, uint32 Frames);" },
		{ 0x40011, "Err TweakRawKnob(Item KnobItem, int32 Value);" },
		{ 0x40012, "Err StartAttachment(Item Attachment, TagArg *tp);" },
		{ 0x40013, "Err ReleaseAttachment(Item Attachment, TagArg *tp);" },
		{ 0x40014, "Err StopAttachment(Item Attachment, TagArg *tp);" },
		{ 0x40015, "Err LinkAttachments(Item At1, Item At2);" },
		{ 0x40016, "Err MonitorAttachment(Item Attachment, Item Cue, int32 CueAt);" },
		{ 0x40018, "Err AbandonInstrument(Item Instrument);" },
		{ 0x40019, "Item AdoptInstrument(Item InsTemplate);" },
		{ 0x4001A, "Item ScavengeInstrument(Item InsTemplate, uint8 Priority, int32 MaxActivity, int32 IfSystemWide);" },
		{ 0x4001B, "Err SetAudioItemInfo(Item AnyItem, TagArg *tp);" },
		{ 0x4001C, "Err PauseInstrument(Item Instrument);" },
		{ 0x4001D, "Err ResumeInstrument(Item Instrument);" },
		{ 0x4001E, "int32 WhereAttachment(Item Attachment);" },
		{ 0x40020, "Err BendInstrumentPitch(Item Instrument, frac16 BendFrac);" },
		{ 0x40021, "Err AbortTimerCue(Item Cue);" },
		{ 0x40022, "Err EnableAudioInput(int32 OnOrOff, TagArg *Tags);" },
		{ 0x40024, "Err ReadProbe(Item Probe, int32 *ValuePtr);" },
		{ 0x40026, "uint16 GetAudioFrameCount(void);" },
		{ 0x40027, "int32 GetAudioCyclesUsed(void);" },

		{ 0x50000, "void MulVec3Mat33_F16(vec3f16 dest, vec3f16 vec, mat33f16 mat);" },
		{ 0x50001, "void MulMat33Mat33_F16(mat33f16 dest, mat33f16 src1, mat33f16 src2);" },
		{ 0x50002, "void MulManyVec3Mat33_F16(vec3f16 *dest, vec3f16 *src, mat33f16 mat, int32 count);" },
		{ 0x50003, "void MulObjectVec3Mat33_F16(void *objectlist[], ObjOffset1 *offsetstruct, int32 count);" },
		{ 0x50004, "void MulObjectMat33_F16(void *objectlist[], ObjOffset2 *offsetstruct, mat33f16 mat, int32 count);" },
		{ 0x50005, "void MulManyF16(frac16 *dest, frac16 *src1, frac16 *src2, int32 count);" },
		{ 0x50006, "void MulScalerF16(frac16 *dest, frac16 *src, frac16 scaler, int32 count);" },
		{ 0x50007, "void MulVec4Mat44_F16(vec4f16 dest, vec4f16 vec, mat44f16 mat);" },
		{ 0x50008, "void MulMat44Mat44_F16(mat44f16 dest, mat44f16 src1, mat44f16 src2);" },
		{ 0x50009, "void MulManyVec4Mat44_F16(vec4f16 *dest, vec4f16 *src, mat44f16 mat, int32 count);" },
		{ 0x5000A, "void MulObjectVec4Mat44_F16(void *objectlist[], ObjeOffset1 *offsetstruct, int32 count);" },
		{ 0x5000B, "void MulObjectMat44_F16(void *objectlist[], ObjOffset2 *offsetstruct, mat44f16 mat, int32 count);" },
		{ 0x5000C, "frac16 Dot3_F16(vec3f16 v1, vec3f16 v2);" },
		{ 0x5000D, "frac16 Dot4_F16(vec4f16 v1, vec4f16 v2);" },
		{ 0x5000E, "void Cross3_F16(vec3f16 dest, vec3f16 v1, vec3f16 v2);" },
		{ 0x5000F, "frac16 AbsVec3_F16(vec3f16 vec);" },
		{ 0x50010, "frac16 AbsVec4_F16(vec4f16 vec);" },
		{ 0x50011, "void MulVec3Mat33DivZ_F16(vec3f16 dest, vec3f16 vec, mat33f16 mat, frac16 n);" },
		{ 0x50012, "void MulManyVec3Mat33DivZ_F16(mmv3m33d *s);" },
	};

	int i;

	for (i = 0; i < std::size(SWIs); i++)
	{
		if (SWIs[i].swi_number == opcode)
			return SWIs[i].name;
	}
	return nullptr;
}



offs_t portfolio_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	uint32_t opcode;
	unsigned result = 0;
	const char *swi_name;

	opcode = opcodes.r32(pc);
	if ((opcode & 0x0f00'0000) == 0x0f00'0000)
	{
		const u32 swi_number = opcode & 0xff'ffff;
		swi_name = lookup_swi(swi_number);
		if (swi_name != nullptr)
		{
			static const char *const pConditionCodeTable[16] =
			{
				"EQ","NE","CS","CC",
				"MI","PL","VS","VC",
				"HI","LS","GE","LT",
				"GT","LE","","NV"
			};
			//stream << swi_name;
			util::stream_format(stream, "SWI%s %s", pConditionCodeTable[opcode >> 28], swi_name);
			result = 4;
		}
	}

	return result;
}
