// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer
/*
 * debug/apollo.c - APOLLO DN3500/DN3000 debug functions
 *
 *  Created on: Nov. 21, 2010
 *      Author: Hans Ostermeyer
 *
 */

#define VERBOSE 1

#include "includes/apollo.h"
#include "cpu/m68000/m68kcpu.h"

//------------------------------------------------------
//         TRAP 0
//------------------------------------------------------

static const char* trap0[] = {
/* 0x00 */NULL, NULL,
/* 0x01 */"FIM_$GET_FIM_ADDR", "",
/* 0x02 */NULL, NULL,
/* 0x03 */"DTTY_$RELOAD_FONT", "",
/* 0x04 */"FILE_$UNLOCK_ALL", "",
/* 0x05 */NULL, NULL,
/* 0x06 */NULL, NULL,
/* 0x07 */"PROC2_$MY_PID", "",
/* 0x08 */"SMD_$OP_WAIT_U", "",
/* 0x09 */"TPAD_$RE_RANGE", "",
/* 0x0a */NULL, NULL,
/* 0x0b */NULL, NULL,
/* 0x0c */NULL, NULL,
/* 0x0d */"ACL_$UP", "",
/* 0x0e */"ACL_$DOWN", "",
/* 0x0f */NULL, NULL,
/* 0x10 */"TPAD_$INQ_DTYPE", "",
/* 0x11 */NULL, NULL,
/* 0x12 */"CACHE_$CLEAR", "",
/* 0x13 */"RIP_$ANNOUNCE_NS", "",
/* 0x14 */NULL, NULL,
/* 0x15 */NULL, NULL,
/* 0x16 */NULL, NULL,
/* 0x17 */NULL, NULL,
/* 0x18 */"PROC2_$DELIVER_PENDING", "",
/* 0x19 */"PROC2_$COMPLETE_FORK", "",
/* 0x1a */"PACCT_$STOP", "",
/* 0x1b */"PACCT_$ON", "",
/* 0x1c */"ACL_$GET_LOCAL_LOCKSMITH", "",
/* 0x1d */"ACL_$IS_SUSER", "",
/* 0x1e */NULL, NULL,
/* 0x1f */"SMD_$N_DEVICES", "",
/* 0x20 */"PROC1_$GET_MAXID", "",
/* 0x21 */"THREAD_$DELETE", "",
/* 0x22 */"THREAD_$SELF_OS", "",
/* 0x23 */"THREAD_$NUMBER_THREADS", "",
/* 0x24 */"ACL_$GET_PROTECT_OPTION", "",
/* 0x25 */"ACL_$PROTECT_CHECK", "",
/* 0x26 */"THREAD_$YIELD", "" };

//------------------------------------------------------
//         TRAP1
//------------------------------------------------------

static const char* trap1[] = {
/* 0x00 */NULL, NULL,
/* 0x01 */NULL, NULL,
/* 0x02 */"FIM_$INSTALL", "x",
/* 0x03 */"NETWORK_$READ_SERVICE", "x",
/* 0x04 */"PROC1_$GET_CPUT", "x",
/* 0x05 */"SET_LITES_LOC", "x",
/* 0x06 */"TIME_$CLOCK", "x",
/* 0x07 */"ASKNODE_$READ_FAILURE_REC", "x",
/* 0x08 */"CAL_$APPLY_LOCAL_OFFSET", "x",
/* 0x09 */"CAL_$GET_INFO", "x",
/* 0x0a */"CAL_$GET_LOCAL_TIME", "x",
/* 0x0b */"CAL_$REMOVE_LOCAL_OFFSET", "x",
/* 0x0c */"CAL_$SET_DRIFT", "x",
/* 0x0d */"DISK_$GET_ERROR_INFO", "x",
/* 0x0e */NULL, NULL,
/* 0x0f */"MSG_$CLOSE", "x",
/* 0x10 */"PROC2_$WHO_AM_I", "u",
/* 0x11 */"SMD_$CLEAR_KBD_CURSOR", "x",
/* 0x12 */NULL, NULL,
/* 0x13 */NULL, NULL,
/* 0x14 */"SMD_$SEND_RESPONSE", "x",
/* 0x15 */"SMD_$STOP_TP_CURSOR", "x",
/* 0x16 */NULL, NULL,
/* 0x17 */NULL, NULL,
/* 0x18 */"SMD_$UNMAP_DISPLAY_U", "x",
/* 0x19 */"UID_$GEN", "x",
/* 0x1a */"TONE_$TIME", "x",
/* 0x1b */"SMD_$INQ_DISP_TYPE", "x",
/* 0x1c */NULL, NULL,
/* 0x1d */NULL, NULL,
/* 0x1e */NULL, NULL,
/* 0x1f */"TPAD_$SET_CURSOR", "x",
/* 0x20 */NULL, NULL,
/* 0x21 */NULL, NULL,
/* 0x22 */"NAME_$GET_WDIR_UID", "u",
/* 0x23 */"NAME_$GET_NDIR_UID", "u",
/* 0x24 */"NAME_$GET_ROOT_UID", "u",
/* 0x25 */"NAME_$GET_NODE_UID", "u",
/* 0x26 */"NAME_$GET_NODE_DATA_UID", "u",
/* 0x27 */"NAME_$GET_CANNED_ROOT_UID", "u",
/* 0x28 */"MSG_$GET_MY_NET", "x",
/* 0x29 */"MSG_$GET_MY_NODE", "x",
/* 0x2a */NULL, NULL,
/* 0x2b */NULL, NULL,
/* 0x2c */NULL, NULL,
/* 0x2d */NULL, NULL,
/* 0x2e */NULL, NULL,
/* 0x2f */NULL, NULL,
/* 0x30 */"GPU_$INIT", "x",
/* 0x31 */NULL, NULL,
/* 0x32 */"SMD_$INIT_STATE", "x",
/* 0x33 */"SMD_$CLR_TRK_RECT", "x",
/* 0x34 */"PROC2_$GET_SIG_MASK", "x",
/* 0x35 */"FIM_$FRESTORE", "x",
/* 0x36 */"TIME_$GET_TIME_OF_DAY", "x",
/* 0x37 */"PROC1_$GET_LOADAV", "x",
/* 0x38 */"PROC2_$GET_BOOT_FLAGS", "x",
/* 0x39 */NULL, NULL,
/* 0x3a */"PROC2_$SET_TTY", "x",
/* 0x3b */"OS_$SHUTDOWN", "x",
/* 0x3c */"PBU_$FAULTED_UNITS", "x",
/* 0x3d */"PROC2_$GET_CPU_USAGE", "x",
/* 0x3e */NULL, NULL,
/* 0x3f */NULL, NULL,
/* 0x40 */NULL, NULL,
/* 0x41 */"TIME_$GET_ADJUST", "x",
/* 0x42 */"FILE_$SYNC", "x",
/* 0x43 */"XPAGE_$IS_MAPPED", "x",
/* 0x44 */"THREAD_$SUSPEND_FOR_FORK", "x",
/* 0x45 */"THREAD_$RESUME_FOR_FORK", "x" };

//------------------------------------------------------
//         TRAP2
//------------------------------------------------------

static const char* trap2[] = {
/* 0x00 */NULL, NULL,
/* 0x01 */NULL, NULL,
/* 0x02 */"FILE_$DELETE", "x,x",
/* 0x03 */"EC2_$WAKEUP", "x,x",
/* 0x04 */NULL, NULL,
/* 0x05 */"ACL_$GET_SID", "x,x",
/* 0x06 */"FILE_$MK_PERMANENT", "x,x",
/* 0x07 */"FILE_$UNLOCK_VOL", "x,x",
/* 0x08 */"CAL_$READ_TIMEZONE", "x,x",
/* 0x09 */"CAL_$SEC_TO_CLOCK", "x,x",
/* 0x0a */"CAL_$WRITE_TIMEZONE", "x,x",
/* 0x0b */"DISK_$UNASSIGN", "x,x",
/* 0x0c */"FILE_$FORCE_UNLOCK", "x,x",
/* 0x0d */"FILE_$FW_FILE", "x,x",
/* 0x0e */"FILE_$MK_IMMUTABLE", "x,x",
/* 0x0f */"FILE_$PURIFY", "x,x",
/* 0x10 */"GET_BUILD_TIME", "x,x",
/* 0x11 */NULL, NULL,
/* 0x12 */NULL, NULL,
/* 0x13 */"MSG_$ALLOCATE", "x,x",
/* 0x14 */"MSG_$OPEN", "x,x",
/* 0x15 */"MSG_$WAIT", "x,x",
/* 0x16 */NULL, NULL,
/* 0x17 */NULL, NULL,
/* 0x18 */"PBU_$DEVICE_INTERRUPTING", "x,x",
/* 0x19 */"PBU_$DISABLE_DEVICE", "x,x",
/* 0x1a */"PBU_$ENABLE_DEVICE", "x,x",
/* 0x1b */"PBU_$FREE_MAP", "x,x",
/* 0x1c */"PROC2_$QUIT", "x,x",
/* 0x1d */"PROC2_$RESUME", "x,x",
/* 0x1e */"PROC2_$SUSPEND", "x,x",
/* 0x1f */NULL, NULL,
/* 0x20 */NULL, NULL,
/* 0x21 */NULL, NULL,
/* 0x22 */NULL, NULL,
/* 0x23 */NULL, NULL,
/* 0x24 */NULL, NULL,
/* 0x25 */NULL, NULL,
/* 0x26 */"SMD_$MAP_DISPLAY_U", "x,x",
/* 0x27 */"SMD_$MOVE_KBD_CURSOR", "x,x",
/* 0x28 */NULL, NULL,
/* 0x29 */"SMD_$RETURN_DISPLAY", "x,x",
/* 0x2a */NULL, NULL,
/* 0x2b */NULL, NULL,
/* 0x2c */NULL, NULL,
/* 0x2d */NULL, NULL,
/* 0x2e */"PEB_$GET_INFO", "x,x",
/* 0x2f */NULL, NULL,
/* 0x30 */"EC2_$GET_VAL", "x,x",
/* 0x31 */"AST_$ADD_ASTES", "x,x",
/* 0x32 */"PBU_$DISSOC", "x,x",
/* 0x33 */"PROC2_$MAKE_ORPHAN", "x,x",
/* 0x34 */"FILE_$DELETE_FORCE", "x,x",
/* 0x35 */"FILE_$DELETE_WHEN_UNLOCKED", "x,x",
/* 0x36 */"FILE_$MK_TEMPORARY", "x,x",
/* 0x37 */NULL, NULL,
/* 0x38 */NULL, NULL,
/* 0x39 */NULL, NULL,
/* 0x3a */NULL, NULL,
/* 0x3b */"SMD_$INQ_KBD_CURSOR", "x,x",
/* 0x3c */"ACL_$ENTER_SUBS", "u,x",
/* 0x3d */NULL, NULL,
/* 0x3e */NULL, NULL,
/* 0x3f */NULL, NULL,
/* 0x40 */NULL, NULL,
/* 0x41 */NULL, NULL,
/* 0x42 */"FILE_$DELETE_FORCE_WHEN_UNLOCKED", "x,x",
/* 0x43 */NULL, NULL,
/* 0x44 */"CT_$ACQUIRE", "x,x",
/* 0x45 */"HINT_$ADD", "x,x",
/* 0x46 */"DIR_$FIX_DIR", "x,x",
/* 0x47 */"NAME_$SET_WDIRUS", "u,x",
/* 0x48 */"NAME_$SET_NDIRUS", "u,x",
/* 0x49 */NULL, NULL,
/* 0x4a */NULL, NULL,
/* 0x4b */"MSG_$CLOSEI", "x,x",
/* 0x4c */"NETWORK_$ADD_PAGE_SERVERS", "x,x",
/* 0x4d */"NETWORK_$ADD_REQUEST_SERVERS", "x,x",
/* 0x4e */"ACL_$ADD_PROJ", "x,x",
/* 0x4f */"ACL_$DELETE_PROJ", "x,x",
/* 0x50 */NULL, NULL,
/* 0x51 */NULL, NULL,
/* 0x52 */"SMD_$SET_TP_REPORTING", "x,x",
/* 0x53 */"SMD_$DISABLE_TRACKING", "x,x",
/* 0x54 */NULL, NULL,
/* 0x55 */NULL, NULL,
/* 0x56 */NULL, NULL,
/* 0x57 */NULL, NULL,
/* 0x58 */"HINT_$ADDI", "x,x",
/* 0x59 */NULL, NULL,
/* 0x5a */NULL, NULL,
/* 0x5b */"PBU_$ALLOCATE_EC", "x,x",
/* 0x5c */"SMD_$SET_DISP_UNIT", "x,x",
/* 0x5d */"SMD_$VIDEO_CTL", "x,x",
/* 0x5e */"SMD_$SET_CURSOR_POS", "x,x",
/* 0x5f */"TERM_$SEND_KBD_STRING", "x,x",
/* 0x60 */"AUDIT_$CONTROL", "x,x",
/* 0x61 */"PROC2_$SIGBLOCK", "x,x",
/* 0x62 */"PROC2_$SIGSETMASK", "x,x",
/* 0x63 */"PROC2_$SIGPAUSE", "x,x",
/* 0x64 */"ACL_$FIPS_CTL", "x,x",
/* 0x65 */"AS_$GET_ADDR", "x,x",
/* 0x66 */"PROC2_$GET_ASID", "x,x",
/* 0x67 */"TTY_$K_FLUSH_INPUT", "x,x",
/* 0x68 */"TTY_$K_FLUSH_OUTPUT", "x,x",
/* 0x69 */"TTY_$K_DRAIN_OUTPUT", "x,x",
/* 0x6a */"PROC2_$DEBUG", "x,x",
/* 0x6b */"PROC2_$UNDEBUG", "x,x",
/* 0x6c */"ACL_$DEF_ACLDATA", "x,x",
/* 0x6d */"PROC2_$OVERRIDE_DEBUG", "x,x",
/* 0x6e */"TIME_$SET_TIME_OF_DAY", "x,x",
/* 0x6f */"CAL_$DECODE_TIME", "x,x",
/* 0x70 */"ACL_$INHERIT_SUBSYS", "x,x",
/* 0x71 */"ACL_$SET_LOCAL_LOCKSMITH", "x,x",
/* 0x72 */NULL, NULL,
/* 0x73 */"SMD_$DISSOC", "x,x",
/* 0x74 */"SMD_$BUSY_WAIT", "x,x",
/* 0x75 */"TTY_$K_OLD_RESET", "x,x",
/* 0x76 */NULL, NULL,
/* 0x77 */NULL, NULL,
/* 0x78 */NULL, NULL,
/* 0x79 */"SCSI_$ABORT_OPERATION", "x,x",
/* 0x7a */"SCSI_$RELEASE", "x,x",
/* 0x7b */"SCSI_$RESET_DEVICE", "x,x",
/* 0x7c */NULL, NULL,
/* 0x7d */"TPAD_$RE_RANGE_UNIT", "x,x",
/* 0x7e */"DISK_$FORMAT_WHOLE", "x,x",
/* 0x7f */NULL, NULL,
/* 0x80 */NULL, NULL,
/* 0x81 */NULL, NULL,
/* 0x82 */NULL, NULL,
/* 0x83 */NULL, NULL,
/* 0x84 */"SMD_$SEND_UNIT_RESPONSE", "x,x",
/* 0x85 */NULL, NULL,
/* 0x86 */"MST_$FW_XTAL", "x,x",
/* 0x87 */"THREAD_$TERMINATE", "x,x",
/* 0x88 */"THREAD_$TERMINATE_ALL", "u,x",
/* 0x89 */"THREAD_$SUSPEND", "x,x",
/* 0x8a */"THREAD_$RESUME", "x,x",
/* 0x8b */"THREAD_$HANDLE_SIGNALS", "x,x",
/* 0x8c */"THREAD_$ABORT", "x,x",
/* 0x8d */"ACL_$SET_PROTECT_OPTION", "x,x",
/* 0x8e */"TTY_$K_SUSPEND_INPUT", "x,x",
/* 0x8f */"TTY_$K_RESUME_INPUT", "x,x",
/* 0x90 */"TTY_$K_SUSPEND_OUTPUT", "x,x",
/* 0x91 */"TTY_$K_RESUME_OUTPUT", "x,x" };

//------------------------------------------------------
//         TRAP3
//------------------------------------------------------

static const char* trap3[] = {
/* 0x00 */"PBU2_$DMA_STOP", "x,x,x",
/* 0x01 */NULL, NULL,
/* 0x02 */"FILE_$CREATE", "x,x,x",
/* 0x03 */"FILE_$UNLOCK", "u,x,x",
/* 0x04 */"FILE_$TRUNCATE", "u,x,x",
/* 0x05 */"MST_$UNMAPS", "x,x,x",
/* 0x06 */"VFMT_$WRITE", "s,x,x",
/* 0x07 */"FILE_$ATTRIBUTES", "x,x,x",
/* 0x08 */"FILE_$SET_LEN", "x,x,x",
/* 0x09 */"FILE_$SET_TYPE", "x,x,x",
/* 0x0a */NULL, NULL,
/* 0x0b */NULL, NULL,
/* 0x0c */NULL, NULL,
/* 0x0d */NULL, NULL,
/* 0x0e */"NETWORK_$SET_SERVICE", "x,x,x",
/* 0x0f */"ASKNODE_$WHO", "s,x,x",
/* 0x10 */"FILE_$ACT_ATTRIBUTES", "x,x,x",
/* 0x11 */"FILE_$LOCATE", "x,x,x",
/* 0x12 */"FILE_$NEIGHBORS", "x,x,x",
/* 0x13 */"FILE_$READ_LOCK_ENTRYU", "x,x,x",
/* 0x14 */"FILE_$SET_ACL", "x,x,x",
/* 0x15 */"FILE_$SET_DIRPTR", "x,x,x",
/* 0x16 */"FILE_$SET_TROUBLE", "x,x,x",
/* 0x17 */NULL, NULL,
/* 0x18 */NULL, NULL,
/* 0x19 */NULL, NULL,
/* 0x1a */NULL, NULL,
/* 0x1b */NULL, NULL,
/* 0x1c */"PBU_$DMA_STOP", "x,x,x",
/* 0x1d */"PBU_$SETDOWN", "x,x,x",
/* 0x1e */"PBU_$SETUP", "x,x,x",
/* 0x1f */"PROC2_$LIST", "x,x,x",
/* 0x20 */"FIM_$SINGLE_STEP", "x,x,x",
/* 0x21 */"SMD_$ASSOC", "x,x,x",
/* 0x22 */"SMD_$BORROW_DISPLAY", "x,x,x",
/* 0x23 */NULL, NULL,
/* 0x24 */NULL, NULL,
/* 0x25 */"SMD_$SET_TP_CURSOR", "x,x,x",
/* 0x26 */"TIME_$WAIT", "x,x,x",
/* 0x27 */"RINGLOG_$CNTL", "x,x,x",
/* 0x28 */"SMD_$ALLOC_HDM", "x,x,x",
/* 0x29 */"SMD_$FREE_HDM", "x,x,x",
/* 0x2a */NULL, NULL,
/* 0x2b */"OS_$GET_EC", "x,x,x",
/* 0x2c */"TIME_$GET_EC", "x,x,x",
/* 0x2d */NULL, NULL,
/* 0x2e */"PROC2_$UPID_TO_UID", "x,u,x",
/* 0x2f */"MSG_$GET_EC", "x,x,x",
/* 0x30 */"DISK_$AS_OPTIONS", "x,x,x",
/* 0x31 */"SMD_$GET_EC", "x,x,x",
/* 0x32 */"NAME_$SET_ACL", "x,x,x",
/* 0x33 */"FILE_$SET_REFCNT", "x,x,x",
/* 0x34 */NULL, NULL,
/* 0x35 */"PROC1_$GET_INFO", "x,x,x",
/* 0x36 */NULL, NULL,
/* 0x37 */NULL, NULL,
/* 0x38 */NULL, NULL,
/* 0x39 */NULL, NULL,
/* 0x3a */NULL, NULL,
/* 0x3b */NULL, NULL,
/* 0x3c */"AS_$GET_INFO", "x,x,x",
/* 0x3d */"FILE_$SET_DTM", "x,x,x",
/* 0x3e */"FILE_$SET_DTU", "x,x,x",
/* 0x3f */NULL, NULL,
/* 0x40 */"LOG_$READ", "x,x,x",
/* 0x41 */"PROC2_$SET_PGROUP", "x,x,x",
/* 0x42 */"SMD_$SET_BLANK_TIMEOUT", "x,x,x",
/* 0x43 */"SMD_$INQ_BLANK_TIMEOUT", "x,x,x",
/* 0x44 */"FILE_$REMOVE_WHEN_UNLOCKED", "x,x,x",
/* 0x45 */"CT_$RELEASE", "x,x,x",
/* 0x46 */"PROC2_$UPGID_TO_UID", "x,x,x",
/* 0x47 */"TIME_$GET_ITIMER", "x,x,x",
/* 0x48 */"DIR_$SET_DAD", "x,x,x",
/* 0x49 */NULL, NULL,
/* 0x4a */NULL, NULL,
/* 0x4b */NULL, NULL,
/* 0x4c */NULL, NULL,
/* 0x4d */NULL, NULL,
/* 0x4e */NULL, NULL,
/* 0x4f */"RIP_$TABLE", "x,x,x",
/* 0x50 */"FILE_$LOCATEI", "x,x,x",
/* 0x51 */"MSG_$OPENI", "x,x,x",
/* 0x52 */"MSG_$ALLOCATEI", "x,x,x",
/* 0x53 */"MSG_$WAITI", "x,x,x",
/* 0x54 */"ACL_$SET_PROJ_LIST", "x,x,x",
/* 0x55 */"ACL_$GET_RE_SIDS", "x,x,x",
/* 0x56 */NULL, NULL,
/* 0x57 */"MSG_$SET_HPIPC", "x,x,x",
/* 0x58 */"DIR_$VALIDATE_ROOT_ENTRY", "x,x,x",
/* 0x59 */"SMD_$ENABLE_TRACKING", "x,x,x",
/* 0x5a */"FILE_$READ_LOCK_ENTRYUI", "x,x,x",
/* 0x5b */NULL, NULL,
/* 0x5c */"ROUTE_$SERVICE", "x,x,x",
/* 0x5d */NULL, NULL,
/* 0x5e */NULL, NULL,
/* 0x5f */NULL, NULL,
/* 0x60 */"PBU_$ADVANCE_EC", "x,x,x",
/* 0x61 */"PBU_$RELEASE_EC", "x,x,x",
/* 0x62 */NULL, NULL,
/* 0x63 */"SMD_$GET_IDM_EVENT", "x,x,x",
/* 0x64 */"MSG_$TEST_FOR_MESSAGE", "x,x,x",
/* 0x65 */"SMD_$ADD_TRK_RECT", "x,x,x",
/* 0x66 */"SMD_$DEL_TRK_RECT", "x,x,x",
/* 0x67 */"SMD_$SET_KBD_TYPE", "x,x,x",
/* 0x68 */NULL, NULL,
/* 0x69 */"FILE_$SET_AUDITED", "x,x,x",
/* 0x6a */"PROC2_$ACKNOWLEDGE", "x,x,x",
/* 0x6b */"PROC2_$GET_MY_UPIDS", "x,x,x",
/* 0x6c */"TTY_$K_INQ_INPUT_FLAGS", "x,x,x",
/* 0x6d */"TTY_$K_INQ_OUTPUT_FLAGS", "x,x,x",
/* 0x6e */"TTY_$K_INQ_ECHO_FLAGS", "x,x,x",
/* 0x6f */"TTY_$K_SET_INPUT_BREAK_MODE", "x,x,x",
/* 0x70 */"TTY_$K_INQ_INPUT_BREAK_MODE", "x,x,x",
/* 0x71 */"TTY_$K_SET_PGROUP", "x,x,x",
/* 0x72 */"TTY_$K_INQ_PGROUP", "x,x,x",
/* 0x73 */"TTY_$K_SIMULATE_TERMINAL_INPUT", "x,x,x",
/* 0x74 */"TTY_$K_INQ_FUNC_ENABLED", "x,x,x",
/* 0x75 */"SIO_$K_TIMED_BREAK", "x,x,x",
/* 0x76 */"FILE_$SET_DEVNO", "x,x,x",
/* 0x77 */"XPD_$SET_PTRACE_OPTS", "x,x,x",
/* 0x78 */"XPD_$INQ_PTRACE_OPTS", "x,x,x",
/* 0x79 */"FILE_$SET_MAND_LOCK", "x,x,x",
/* 0x7a */"TIME_$SET_CPU_LIMIT", "x,x,x",
/* 0x7b */"CAL_$WEEKDAY", "x,x,x",
/* 0x7c */"SIO_$K_SIGNAL_WAIT", "x,x,x",
/* 0x7d */"TERM_$SET_DISCIPLINE", "x,x,x",
/* 0x7e */"PROC2_$SET_SERVER", "x,x,x",
/* 0x7f */"PACCT_$START", "x,x,x",
/* 0x80 */"FILE_$SET_DTU_F", "x,x,x",
/* 0x81 */"PROC2_$PGUID_TO_UPGID", "x,x,x",
/* 0x82 */"TERM_$INQ_DISCIPLINE", "x,x,x",
/* 0x83 */"PBU_$DMA_STATUS", "x,x,x",
/* 0x84 */NULL, NULL,
/* 0x85 */"MST_$UNMAPS_AND_FREE_AREA", "x,x,x",
/* 0x86 */"SMD_$ASSOC_CSRS", "x,x,x",
/* 0x87 */"SMD_$INQ_DISP_INFO", "x,x,x",
/* 0x88 */"SMD_$INQ_DISP_UID", "x,x,x",
/* 0x89 */"SMD_$DISPLAY_LOGO", "x,x,x",
/* 0x8a */"TERM_$SET_REAL_LINE_DISCIPLINE", "x,x,x",
/* 0x8b */"TIME_$ADJUST_TIME_OF_DAY", "x,x,x",
/* 0x8c */"PROC2_$UID_TO_UPID", "x,x,x",
/* 0x8d */"PROC2_$SET_SESSION_ID", "x,x,x",
/* 0x8e */"SMD_$GET_UNIT_EVENT", "x,x,x",
/* 0x8f */"TPAD_$SET_UNIT_CURSOR", "x,x,x",
/* 0x90 */"TPAD_$SET_PUNCH_IMPACT", "x,x,x",
/* 0x91 */"TPAD_$INQ_PUNCH_IMPACT", "x,x,x",
/* 0x92 */"TTY_$K_INQ_SESSION_ID", "x,x,x",
/* 0x93 */"TTY_$K_SET_SESSION_ID", "x,x,x",
/* 0x94 */NULL, NULL,
/* 0x95 */NULL, NULL,
/* 0x96 */NULL, NULL,
/* 0x97 */NULL, NULL,
/* 0x98 */"SMD_$SHARE_DEVICE", "x,x,x",
/* 0x99 */"SMD_$SET_UNIT_CURSOR_POS", "x,x,x",
/* 0x9a */"SMD_$CLR_AND_LOAD_TRK_RECT", "x,x,x",
/* 0x9b */"PROC2_$GET_PARM", "x,x,x",
/* 0x9c */NULL, NULL,
/* 0x9d */NULL, NULL,
/* 0x9e */NULL, NULL,
/* 0x9f */"FILE_$CW_READ", "x,x,x",
/* 0xa0 */"FILE_$CW_WRITE", "x,x,x",
/* 0xa1 */"MST_$TRUNC_XTAL", "x,x,x",
/* 0xa2 */"MST_$SET_LEN_XTAL", "x,x,x",
/* 0xa3 */"THREAD_$CREATE", "x,x,x",
/* 0xa4 */"THREAD_$CLEANUP", "x,x,x",
/* 0xa5 */"THREAD_$SET_INH_PTR", "x,x,x",
/* 0xa6 */"SMD_$BORROW_DISPLAY_AND_KBD", "x,x,x",
/* 0xa7 */"THREAD_$CLEANUP_ALL", "x,x,x",
/* 0xa8 */"DIR_$ADD_MOUNT", "x,x,x",
/* 0xa9 */"DIR_$REMOVE_MOUNT", "x,x,x",
/* 0xaa */NULL, NULL,
/* 0xab */"TTY_$K_RESET", "x,x,x",
/* 0xac */"MST_$SHADOW_MAP", "x,x,x",
/* 0xad */NULL, NULL };

//------------------------------------------------------
//         TRAP4
//------------------------------------------------------

static const char* trap4[] = {
/* 0x00 */NULL, NULL,
/* 0x01 */"MST_$SET_GUARD", "u,x,x,x",
/* 0x02 */"MST_$UNMAP_GLOBAL", "u,x,x,x",
/* 0x03 */"MST_$GET_UID", "x,u,x,x",
/* 0x04 */"EC2_$WAIT", "x,x,x,x",
/* 0x05 */"FILE_$READ_LOCK_ENTRY", "x,x,x,x",
/* 0x06 */"MST_$UNMAP", "u,x,x,x",
/* 0x07 */"MST_$GROW_AREA", "x,x,x,x",
/* 0x08 */NULL, NULL,
/* 0x09 */NULL, NULL,
/* 0x0a */NULL, NULL,
/* 0x0b */NULL, NULL,
/* 0x0c */"TERM_$CONTROL", "x,x,x,x",
/* 0x0d */"TERM_$READ", "x,s,x,x",
/* 0x0e */"TERM_$WRITE", "x,x,x,x",
/* 0x0f */"DISK_$FORMAT", "x,x,x,x",
/* 0x10 */"DISK_$LV_ASSIGN", "x,x,x,x",
/* 0x11 */"FILE_$FW_PARTIAL", "x,x,x,x",
/* 0x12 */NULL, NULL,
/* 0x13 */NULL, NULL,
/* 0x14 */NULL, NULL,
/* 0x15 */NULL, NULL,
/* 0x16 */NULL, NULL,
/* 0x17 */NULL, NULL,
/* 0x18 */NULL, NULL,
/* 0x19 */NULL, NULL,
/* 0x1a */NULL, NULL,
/* 0x1b */NULL, NULL,
/* 0x1c */NULL, NULL,
/* 0x1d */NULL, NULL,
/* 0x1e */NULL, NULL,
/* 0x1f */NULL, NULL,
/* 0x20 */NULL, NULL,
/* 0x21 */NULL, NULL,
/* 0x22 */NULL, NULL,
/* 0x23 */"PBU_$WAIT", "x,x,x,x",
/* 0x24 */"PBU_$GET_PTR", "x,x,x,x",
/* 0x25 */"PCHIST_$CNTL", "x,x,x,x",
/* 0x26 */"PROC2_$GET_INFO", "u,x,x,x",
/* 0x27 */NULL, NULL,
/* 0x28 */NULL, NULL,
/* 0x29 */NULL, NULL,
/* 0x2a */"SMD_$SIGNAL", "x,x,x,x",
/* 0x2b */NULL, NULL,
/* 0x2c */NULL, NULL,
/* 0x2d */"TERM_$INQUIRE", "x,x,x,x",
/* 0x2e */NULL, NULL,
/* 0x2f */NULL, NULL,
/* 0x30 */NULL, NULL,
/* 0x31 */"TERM_$GET_EC", "x,x,x,x",
/* 0x32 */NULL, NULL,
/* 0x33 */"PBU_$GET_EC", "x,x,x,x",
/* 0x34 */NULL, NULL,
/* 0x35 */NULL, NULL,
/* 0x36 */NULL, NULL,
/* 0x37 */NULL, NULL,
/* 0x38 */"TERM_$READ_COND", "x,x,x,x",
/* 0x39 */NULL, NULL,
/* 0x3a */NULL, NULL,
/* 0x3b */"PBU2_$WIRE", "x,x,x,x",
/* 0x3c */"PROC2_$SET_NAME", "x,x,x,x",
/* 0x3d */"PROC2_$SET_PRIORITY", "x,x,x,x",
/* 0x3e */"PROC2_$GET_EC", "u,x,x,x",
/* 0x3f */"PROC2_$LIST_PGROUP", "x,x,x,x",
/* 0x40 */NULL, NULL,
/* 0x41 */NULL, NULL,
/* 0x42 */NULL, NULL,
/* 0x43 */"CT_$CONTROL", "x,x,x,x",
/* 0x44 */"CT_$WAIT", "x,x,x,x",
/* 0x45 */"DIR_$DROP_DIRU", "x,x,x,x",
/* 0x46 */"DIR_$SET_DEFAULT_ACL", "x,x,x,x",
/* 0x47 */"DIR_$GET_DEFAULT_ACL", "x,x,x,x",
/* 0x48 */"NAME_$READ_DIRS_PS", "x,x,x,x",
/* 0x49 */NULL, NULL,
/* 0x4a */"ACL_$GET_PROJ_LIST", "x,x,x,x",
/* 0x4b */"MST_$CHANGE_RIGHTS", "x,x,x,x",
/* 0x4c */NULL, NULL,
/* 0x4d */"FILE_$READ_LOCK_ENTRYI", "x,x,x,x",
/* 0x4e */"ROUTE_$INCOMING", "x,x,x,x",
/* 0x4f */NULL, NULL,
/* 0x50 */"SMD_$INQ_KBD_TYPE", "x,x,x,x",
/* 0x51 */"ROUTE_$GET_EC", "x,x,x,x",
/* 0x52 */NULL, NULL,
/* 0x53 */"SMD_$DM_COND_EVENT_WAIT", "x,x,x,x",
/* 0x54 */"DISK_$READ_MFG_BADSPOTS", "x,x,x,x",
/* 0x55 */"DISK_$GET_MNT_INFO", "x,x,x,x",
/* 0x56 */"PROC2_$SET_SIG_MASK", "x,x,x,x",
/* 0x57 */"PROC2_$SIGRETURN", "x,x,x,x",
/* 0x58 */"PROC2_$WAIT", "x,x,x,x",
/* 0x59 */"PROC2_$SIGNAL", "x,x,x,x",
/* 0x5a */"PROC2_$SIGNAL_PGROUP", "x,x,x,x",
/* 0x5b */"PROC2_$GET_CR_REC", "x,x,x,x",
/* 0x5c */"TTY_$K_SET_FUNC_CHAR", "x,x,x,x",
/* 0x5d */"TTY_$K_INQ_FUNC_CHAR", "x,x,x,x",
/* 0x5e */"TTY_$K_SET_INPUT_FLAG", "x,x,x,x",
/* 0x5f */"TTY_$K_SET_OUTPUT_FLAG", "x,x,x,x",
/* 0x60 */"TTY_$K_SET_ECHO_FLAG", "x,x,x,x",
/* 0x61 */"TTY_$K_ENABLE_FUNC", "x,x,x,x",
/* 0x62 */"SIO_$K_SET_PARAM", "x,x,x,x",
/* 0x63 */"SIO_$K_INQ_PARAM", "x,x,x,x",
/* 0x64 */"FILE_$SET_MGR_ATTR", "x,x,x,x",
/* 0x65 */"XPD_$GET_REGISTERS", "x,x,x,x",
/* 0x66 */"XPD_$PUT_REGISTERS", "x,x,x,x",
/* 0x67 */"FILE_$RESERVE", "u,x,x,x",
/* 0x68 */NULL, NULL,
/* 0x69 */"ACL_$GET_RES_SIDS", "x,x,x,x",
/* 0x6a */"FILE_$FW_PAGES", "x,x,x,x",
/* 0x6b */"PROC2_$SET_ACCT_INFO", "s,x,x,x",
/* 0x6c */"FILE_$IMPORT_LK", "x,x,x,x",
/* 0x6d */"FILE_$UNLOCK_D", "u,x,x,x",
/* 0x6e */"FILE_$SET_LEN_D", "x,x,x,x",
/* 0x6f */"FILE_$TRUNCATE_D", "x,x,x,x",
/* 0x70 */"FILE_$SET_DTM_F", "x,x,x,x",
/* 0x71 */"TTY_$K_SET_FLAG", "x,x,x,x",
/* 0x72 */NULL, NULL,
/* 0x73 */"MST_$UNMAP_AND_FREE_AREA", "x,x,x,x",
/* 0x74 */NULL, NULL,
/* 0x75 */"PROC2_$NAME_TO_UID", "S,w,u,x",
/* 0x76 */NULL, NULL,
/* 0x77 */NULL, NULL,
/* 0x78 */NULL, NULL,
/* 0x79 */"SCSI_$GET_EC", "x,x,x,x",
/* 0x7a */"SCSI_$GET_INFO", "x,x,x,x",
/* 0x7b */"SCSI_$SETUP", "x,x,x,x",
/* 0x7c */"SCSI_$WIRE", "x,x,x,x",
/* 0x7d */"MSG_$SHARE_SOCKET", "x,x,x,x",
/* 0x7e */"TTY_$K_INQ_DELAY", "x,x,x,x",
/* 0x7f */"TTY_$K_SET_DELAY", "x,x,x,x",
/* 0x80 */NULL, NULL,
/* 0x81 */NULL, NULL,
/* 0x82 */"PROC2_$PGROUP_INFO", "x,x,x,x",
/* 0x83 */"PROC2_$SET_PARM", "x,x,x,x",
/* 0x84 */NULL, NULL,
/* 0x85 */NULL, NULL,
/* 0x86 */"XPAGE_$PAGER_RENDEZVOUS", "x,x,x,x",
/* 0x87 */"FILE_$SET_DEVNO32", "x,x,x,x",
/* 0x88 */"MST_$SET_LIMIT", "x,x,x,x",
/* 0x89 */"THREAD_$SET_PRIORITY", "x,x,x,x",
/* 0x8a */"THREAD_$INFO", "x,x,x,x",
/* 0x8b */"TTY_$K_SET_DELAY_STYLE", "x,x,x,x",
/* 0x8c */"TTY_$K_INQ_DELAY_STYLE", "x,x,x,x" };

//------------------------------------------------------
//         TRAP5
//------------------------------------------------------

static const char* trap5[] = {
/* 0x00 */NULL, NULL,
/* 0x01 */"MST_$MAP_AREA", "x,x,x,u,x",
/* 0x02 */NULL, NULL,
/* 0x03 */NULL, NULL,
/* 0x04 */NULL, NULL,
/* 0x05 */"ACL_$RIGHTS", "x,x,x,x,x",
/* 0x06 */NULL, NULL,
/* 0x07 */"ASKNODE_$INFO", "x,x,x,x,x",
/* 0x08 */"DISK_$AS_READ", "x,x,x,x,x",
/* 0x09 */"DISK_$AS_WRITE", "x,x,x,x,x",
/* 0x0a */NULL, NULL,
/* 0x0b */NULL, NULL,
/* 0x0c */NULL, NULL,
/* 0x0d */NULL, NULL,
/* 0x0e */NULL, NULL,
/* 0x0f */NULL, NULL,
/* 0x10 */NULL, NULL,
/* 0x11 */"PBU_$GET_DCTE", "x,x,x,x,x",
/* 0x12 */"PBU_$READ_CSR", "x,x,x,x,x",
/* 0x13 */"PBU_$CONTROL", "x,x,x,x,x",
/* 0x14 */"PBU2_$WIRE_MAP", "x,x,x,x,x",
/* 0x15 */"PBU_$WRITE_CSR", "x,x,x,x,x",
/* 0x16 */"TPAD_$INQUIRE", "x,x,x,x,x",
/* 0x17 */"TPAD_$SET_MODE", "x,x,x,x,x",
/* 0x18 */"VFMT_$MAIN", "x,x,x,x,x",
/* 0x19 */"VOLX_$GET_INFO", "x,x,x,x,x",
/* 0x1a */"VTOC_$GET_UID", "x,x,x,x,x",
/* 0x1b */"NETLOG_$CNTL", "x,x,x,x,x",
/* 0x1c */"PROC2_$GET_UPIDS", "x,x,x,x,x",
/* 0x1d */NULL, NULL,
/* 0x1e */NULL, NULL,
/* 0x1f */"PBU2_$ALLOCATE_MAP", "x,x,x,x,x",
/* 0x20 */"PBU2_$MAP", "x,x,x,x,x",
/* 0x21 */"PBU2_$UNMAP", "x,x,x,x,x",
/* 0x22 */"PBU2_$UNWIRE", "x,x,x,x,x",
/* 0x23 */"MST_$GET_UID_ASID", "x,x,x,x,x",
/* 0x24 */"MST_$INVALIDATE", "x,x,u,x,x",
/* 0x25 */"FILE_$INVALIDATE", "x,x,x,x,x",
/* 0x26 */NULL, NULL,
/* 0x27 */NULL, NULL,
/* 0x28 */NULL, NULL,
/* 0x29 */"MST_$SET_TOUCH_AHEAD_CNT", "x,x,x,x,x",
/* 0x2a */"OS_$CHKSUM", "x,x,x,x,x",
/* 0x2b */"FILE_$GET_SEGMENT_MAP", "x,x,x,x,x",
/* 0x2c */NULL, NULL,
/* 0x2d */"PBU2_$ASSOC", "x,x,x,x,x",
/* 0x2e */"FILE_$UNLOCK_PROC", "x,x,x,x,x",
/* 0x2f */"CT_$READ", "x,x,x,x,x",
/* 0x30 */"CT_$WRITE", "x,x,x,x,x",
/* 0x31 */"DIR_$ADDU", "u,S,w,u,x",
/* 0x32 */"DIR_$DROPU", "x,x,x,x,x",
/* 0x33 */"DIR_$CREATE_DIRU", "x,x,x,x,x",
/* 0x34 */"DIR_$ADD_BAKU", "x,x,x,x,x",
/* 0x35 */NULL, NULL,
/* 0x36 */NULL, NULL,
/* 0x37 */NULL, NULL,
/* 0x38 */"DIR_$ADD_HARD_LINKU", "x,x,x,x,x",
/* 0x39 */NULL, NULL,
/* 0x3a */"RIP_$UPDATE", "x,x,x,x,x",
/* 0x3b */"DIR_$DROP_LINKU", "x,x,x,x,x",
/* 0x3c */"ACL_$CHECK_RIGHTS", "x,x,x,x,x",
/* 0x3d */"DIR_$DROP_HARD_LINKU", "x,x,x,x,x",
/* 0x3e */"ROUTE_$OUTGOING", "x,x,x,x,x",
/* 0x3f */NULL, NULL,
/* 0x40 */NULL, NULL,
/* 0x41 */NULL, NULL,
/* 0x42 */"NET_$GET_INFO", "x,x,x,x,x",
/* 0x43 */"DIR_$GET_ENTRYU", "x,x,x,x,x",
/* 0x44 */"AUDIT_$LOG_EVENT", "x,x,x,x,x",
/* 0x45 */"FILE_$SET_PROT", "x,x,x,x,x",
/* 0x46 */"TTY_$K_GET", "x,x,s,x,x",
/* 0x47 */"TTY_$K_PUT", "x,x,S,w,x",
/* 0x48 */"PROC2_$ALIGN_CTL", "x,x,x,x,x",
/* 0x49 */NULL, NULL,
/* 0x4a */"XPD_$READ_PROC", "x,x,x,x,x",
/* 0x4b */"XPD_$WRITE_PROC", "x,x,x,x,x",
/* 0x4c */"DIR_$SET_DEF_PROTECTION", "x,x,x,x,x",
/* 0x4d */"DIR_$GET_DEF_PROTECTION", "x,x,x,x,x",
/* 0x4e */"ACL_$COPY", "u,u,x,x,x",
/* 0x4f */"ACL_$CONVERT_FUNKY_ACL", "x,x,x,x,x",
/* 0x50 */"DIR_$SET_PROTECTION", "x,x,x,x,x",
/* 0x51 */"FILE_$OLD_AP", "x,x,x,x,x",
/* 0x52 */"ACL_$SET_RE_ALL_SIDS", "x,x,x,x,x",
/* 0x53 */"ACL_$GET_RE_ALL_SIDS", "x,x,x,x,x",
/* 0x54 */"FILE_$EXPORT_LK", "x,x,x,x,x",
/* 0x55 */"FILE_$CHANGE_LOCK_D", "x,x,x,x,x",
/* 0x56 */"XPD_$READ_PROC_ASYNC", "x,x,x,x,x",
/* 0x57 */NULL, NULL,
/* 0x58 */"SMD_$MAP_DISPLAY_MEMORY", "x,x,x,x,x",
/* 0x59 */NULL, NULL,
/* 0x5a */NULL, NULL,
/* 0x5b */NULL, NULL,
/* 0x5c */NULL, NULL,
/* 0x5d */"SCSI_$UNWIRE", "x,x,x,x,x",
/* 0x5e */"SMD_$UNMAP_DISPLAY_MEMORY", "x,x,x,x,x",
/* 0x5f */"SMD_$SET_COLOR_TABLE", "x,x,x,x,x",
/* 0x60 */"RIP_$TABLE_D", "x,x,x,x,x",
/* 0x61 */NULL, NULL,
/* 0x62 */"SMD_$DEVICE_INFO", "x,x,x,x,x",
/* 0x63 */NULL, NULL,
/* 0x64 */NULL, NULL,
/* 0x65 */"THREAD_$GET_STATE", "x,x,x,p,x",
/* 0x66 */"THREAD_$SET_STATE", "x,x,x,p,x",
/* 0x67 */"THREAD_$SET_STACK", "x,u,x,x,x",
/* 0x68 */NULL, NULL };

//------------------------------------------------------
//         TRAP7
//------------------------------------------------------

static const char* trap7[] = {
/* 0x00 */"FILE_$LOCK", "u,x,x,x,x,x,x",
/* 0x01 */"VFMT_$WRITE", "s,x,x,x,x,x,x",
/* 0x02 */"MST_$MAP_AREA_AT", "x,x,x,x,x,x,x",
/* 0x03 */NULL, NULL,
/* 0x04 */NULL, NULL,
/* 0x05 */NULL, NULL,
/* 0x06 */NULL, NULL,
/* 0x07 */NULL, NULL,
/* 0x08 */NULL, NULL,
/* 0x09 */NULL, NULL,
/* 0x0a */NULL, NULL,
/* 0x0b */NULL, NULL,
/* 0x0c */NULL, NULL,
/* 0x0d */"VFMT_$ENCODE/VFMT_$FORMAT", "x,x,x,x,x,x,x",
/* 0x0e */"VFMT_$ENCODE/VFMT_$FORMAT", "x,x,x,x,x,x,x",
/* 0x0f */"STOP_$WATCH", "x,x,x,x,x,x,x",
/* 0x10 */NULL, NULL,
/* 0x11 */NULL, NULL,
/* 0x12 */"ASKNODE_$GET_INFO", "x,x,x,x,x,x,x",
/* 0x13 */"DISK_$DIAG_IO", "x,x,x,x,x,x,x",
/* 0x14 */NULL, NULL,
/* 0x15 */NULL, NULL,
/* 0x16 */NULL, NULL,
/* 0x17 */"CT_$DIAG", "x,x,x,x,x,x,x",
/* 0x18 */"TIME_$SET_ITIMER", "x,x,x,x,x,x,x",
/* 0x19 */"OSINFO_$GET_STATS", "x,x,x,x,x,x,x",
/* 0x1a */"DIR_$CNAMEU", "x,x,x,x,x,x,x",
/* 0x1b */"DIR_$DELETE_FILEU", "u,S,w,x,x,x,x",
/* 0x1c */"DIR_$ADD_LINKU", "x,x,x,x,x,x,x",
/* 0x1d */NULL, NULL,
/* 0x1e */NULL, NULL,
/* 0x1f */NULL, NULL,
/* 0x20 */NULL, NULL,
/* 0x21 */"ASKNODE_$WHO_REMOTE", "x,x,x,x,x,x,x",
/* 0x22 */"MST_$REMAP", "x,x,x,x,x,x,x",
/* 0x23 */"DIR_$ROOT_ADDU", "x,x,x,x,x,x,x",
/* 0x24 */NULL, NULL,
/* 0x25 */NULL, NULL,
/* 0x26 */NULL, NULL,
/* 0x27 */NULL, NULL,
/* 0x28 */"ASKNODE_$WHO_NOTOPO", "x,x,x,x,x,x,x",
/* 0x29 */"NET_$OPEN", "x,x,x,x,x,x,x",
/* 0x2a */"NET_$CLOSE", "x,x,x,x,x,x,x",
/* 0x2b */"NET_$IOCTL", "x,x,x,x,x,x,x",
/* 0x2c */"DIR_$FIND_UID", "u,u,x,S,w,x,x",
/* 0x2d */"FILE_$GET_ATTRIBUTES", "u,x,x,x,x,x,-",
/* 0x2e */NULL, NULL,
/* 0x2f */"PCHIST_$UNIX_PROFIL_CNTL", "x,x,x,x,x,x,x",
/* 0x30 */"XPD_$RESTART", "x,x,x,x,x,x,x",
/* 0x31 */"FILE_$GET_ATTR_INFO", "u,x,x,x,x,x,p",
/* 0x32 */"ACL_$PRIM_CREATE", "x,x,x,x,x,x,x",
/* 0x33 */"PROC2_$GET_REGS", "x,x,x,x,x,x,x",
/* 0x34 */"ACL_$CONVERT_TO_9ACL", "x,x,x,x,x,x,x",
/* 0x35 */"ACL_$SET_RES_ALL_SIDS", "x,x,x,x,x,x,x",
/* 0x36 */"ACL_$GET_RES_ALL_SIDS", "x,x,x,x,x,x,x",
/* 0x37 */"FILE_$LOCK_D", "u,x,x,x,x,x,p",
/* 0x38 */"FILE_$CREATE_IT", "x,x,u,x,u,x,u",
/* 0x39 */"ACL_$RIGHTS_CHECK", "x,x,x,x,x,x,x",
/* 0x3a */"RIP_$UPDATE_D", "x,x,x,x,x,x,x",
/* 0x3b */"VOLX_$SWAPON", "x,x,x,x,x,x,x",
/* 0x3c */"MST_$ADDMAP", "x,x,x,x,x,x,x", };

//------------------------------------------------------
//         TRAP8
//------------------------------------------------------

static const char* trap8[] = {
/* 0x00 - 0x07 */"MST_$MAP", "u,x,x,x,x,x,x",
/* 0x01 - 0x08 */"MST_$MAP_AT", "x,u,x,x,x,x,x,x",
/* 0x02 - 0x07 */"MST_$MAP_GLOBAL", "u,x,x,x,x,x,x",
/* 0x03 - 0x0A */"VFMT_$WRITE", "s,x,x,x,x,x,x,x,x,x",
/* 0x04 - 0x06 */"VOLX_$DISMOUNT", "x,x,x,x,x,x",
/* 0x05 - 0x06 */"VOLX_$GET_UIDS", "x,x,x,x,x,x",
/* 0x06 - 0x07 */NULL, NULL,
/* 0x07 - 0x07 */"DISK_$PV_ASSIGN", "x,x,x,x,x,x,x",
/* 0x08 - 0x0B */"MSG_$RCV", "x,x,x,x,x,x,x,x,x,x,x",
/* 0x09 - 0x10 */"MSG_$SAR", "x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x",
/* 0x0a - 0x0A */"MSG_$SEND", "x,x,x,x,x,x,x,x,x,x",
/* 0x0b - 0x06 */NULL, NULL,
/* 0x0c - 0x07 */"SMD_$LOAD_CRSR_BITMAP", "x,x,x,x,x,x,x",
/* 0x0d - 0x07 */"SMD_$READ_CRSR_BITMAP", "x,x,x,x,x,x,x",
/* 0x0e - 0x0D */"VFMT_$ENCODE/VFMT_$FORMAT", "s,x,x,x,x,x,x,x,x,x,x,x,x",
/* 0x0f - 0x08 */"VFMT_$ENCODE/VFMT_$FORMAT", "s,x,x,x,x,x,x,x",
/* 0x10 - 0x0D */"VFMT_$ENCODE/VFMT_$FORMAT", "s,x,x,x,x,x,x,x,x,x,x,x,x",
/* 0x11 - 0x08 */"VFMT_$ENCODE/VFMT_$FORMAT", "s,x,x,x,x,x,x,x",
/* 0x12 - 0x06 */"CAL_$WRITE_CALENDAR", "x,x,x,x,x,x",
/* 0x13 - 0x08 */"MSG_$RCV_CONTIG", "x,x,x,x,x,x,x,x",
/* 0x14 - 0x06 */"OSINFO_$GET_MMAP", "x,x,x,x,x,x",
/* 0x15 - 0x06 */"ASKNODE_$INTERNET_INFO", "x,x,x,x,x,x",
/* 0x16 - 0x07 */"MST_$GET_VA_INFO", "x,x,x,x,x,x,x",
/* 0x17 - 0x0D */"MSG_$SENDI", "x,x,x,x,x,x,x,x,x,x,x,x,x",
/* 0x18 - 0x0F */"MSG_$RCVI", "x,x,x,x,x,x,x,x,x,x,x,x,x,x,x",
/* 0x19 - 0x0C */"MSG_$RCV_CONTIGI", "x,x,x,x,x,x,x,x,x,x,x,x",
/* 0x1a - 0x11 */"MSG_$SARI", "x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x",
/* 0x1b - 0x06 */NULL, NULL,
/* 0x1c - 0x0E */"MSG_$SEND_HW", "x,x,x,x,x,x,x,x,x,x,x,x,x,x",
/* 0x1d - 0x07 */"PBU_$WIRE_SPECIAL", "x,x,x,x,x,x,x",
/* 0x1e - 0x06 */"PBU_$DMA_START", "x,x,x,x,x,x",
/* 0x1f - 0x08 */NULL, NULL,
/* 0x20 - 0x07 */"MST_$MAP_TOP", "x,x,x,x,x,x,x",
/* 0x21 - 0x08 */"NET_$SEND", "x,x,x,x,x,x,x,x",
/* 0x22 - 0x08 */"NET_$RCV", "x,x,x,x,x,x,x,x",
/* 0x23 - 0x08 */"DIR_$DIR_READU", "u,S,w,x,x,x,x,x",
/* 0x24 - 0x07 */"DIR_$READ_LINKU", "x,x,x,x,x,x,x",
/* 0x25 - 0x07 */"PBU2_$DMA_START", "x,x,x,x,x,x,x",
/* 0x26 - 0x09 */NULL, NULL,
/* 0x27 - 0x09 */NULL, NULL,
/* 0x28 - 0x07 */"PROC2_$COMPLETE_VFORK", "x,x,x,x,x,x,x",
/* 0x29 - 0x07 */NULL, NULL,
/* 0x2a - 0x06 */NULL, NULL,
/* 0x2b - 0x0A */"DIR_$RESOLVE", "S,w,u,u,x,x,x,x,x,x",
/* 0x2c - 0x08 */"VOLX_$MOUNT", "x,x,x,x,x,x,x,x",
/* 0x2d - 0x06 */"ACL_$IMAGE", "x,x,x,x,x,x",
/* 0x2e - 0x09 */"DISK_$PV_ASSIGN_N", "x,x,x,x,x,x,x,x,x",
/* 0x2f - 0x07 */"DISK_$AS_XFER_MULTI", "x,x,x,x,x,x,x",
/* 0x30 - 0x07 */"PROC2_$FORK", "x,x,x,x,x,x,x",
/* 0x31 - 0x06 */"SCSI_$DO_COMMAND", "x,x,x,x,x,x",
/* 0x32 - 0x07 */"SCSI_$WAIT", "x,x,x,x,x,x,x",
/* 0x33 - 0x0A */"PROC2_$CREATE", "x,x,x,x,x,x,x,x,x,x",
/* 0x34 - 0x06 */"TPAD_$SET_UNIT_MODE", "x,x,x,x,x,x",
/* 0x35 - 0x06 */"TPAD_$INQUIRE_UNIT", "x,x,x,x,x,x",
/* 0x36 - 0x0B */"SCSI_$DO_SYS_DEVICE_CMD", "x,x,x,x,x,x,x,x,x,x,x",
/* 0x37 - 0x0D */"MSG_$RCV_HW", "x,x,x,x,x,x,x,x,x,x,x,x,x",
/* 0x38 - 0x06 */"VOLX_$VOL_INFO", "x,x,x,x,x,x",
/* 0x39 - 0x08 */"MST_$MAP_XTAL", "x,x,x,x,x,x,x,x",
/* 0x3a - 0x08 */"MST_$MAP_U", "x,u,x,x,x,x,x,x",
/* 0x3b - 0x06 */"MST_$GET_INFO_BY_VA", "x,x,x,x,x,x",
/* 0x3c - 0x06 */"VOLX_$QUOTA_CONTROL", "x,x,x,x,x,x",
/* 0x3d - 0x06 */"VOLX_$READ_QUOTA_TABLE", "x,x,x,x,x,x",
/* 0x3e - 0x06 */"THREAD_$LIST", "x,x,x,x,x,x",
/* 0x3f - 0x07 */"SCSI_$DO_COMMAND_2", "x,x,x,x,x,x,x" };

// get parameter string for parameter type and value at addr

static const char *get_param(m68000_base_device *m68k, UINT32 addr, char type)
{
	UINT32 value = ~0;

	// FIXME:
	static char sb[256];

	int i;
	char ch;
	int maxlen = sizeof(sb) - 2;
	UINT32 value1;

	sb[0] = 0;

	if (!m68k->mmu_tmp_buserror_occurred)
	{
		value = m68k->read32(addr);
		if (!m68k->mmu_tmp_buserror_occurred && (value != ~0))
		{
			switch (type)
			{
			case 'S': // string w/o terminating 0
				value1 = m68k->read32(addr + 4);
				if (!m68k->mmu_tmp_buserror_occurred && (value1 != ~0))
				{
					maxlen = m68k->read16(value1);
					if (maxlen > sizeof(sb) - 2)
					{
						maxlen = sizeof(sb) - 2;
					}
				}
			case 's': // string
				i = 0;
				sb[i++] = '"';
				while (i <= maxlen && (ch = m68k->read8(value++)) != 0
						&& !m68k->mmu_tmp_buserror_occurred)
				{
					sb[i++] = ch < 32 ? '.' : ch;
				}
				sb[i++] = '"';
				sb[i] = 0;
				break;
			case 'b': // byte (1 byte)
				sprintf(sb, "0x%x", m68k->read8(value));
				break;
			case 'p': // pointer
				sprintf(sb, "0x%x", value);
				break;
			case 'w': // word (2 byte)
				sprintf(sb, "0x%x", m68k->read16(value));
				break;
			case 'x': // default  (hex 32 bit)
				sprintf(sb, "0x%x", m68k->read32(value));
				break;
			case 'u': // uid
				sprintf(sb, "%08x.%08x", m68k->read32(value),
						m68k->read32(value + 4));
				break;
			default:
				sprintf(sb, "%c", type);
				break;
			}
		}
		m68k->mmu_tmp_buserror_occurred = 0;
	}
	return sb;
}

// get the svc call string

static const char* get_svc_call(m68000_base_device *m68k, int trap_no,
		int trap_code,  char *sb)
{
	UINT32 sp = REG_A(m68k)[7];
	UINT32 pa;
	const char * name = NULL;
	const char * param = NULL;

	switch (trap_no)
	{
	case 0:
		if (trap_no < sizeof(trap0) / 8)
		{
			name = trap0[trap_code * 2];
			param = trap0[trap_code * 2 + 1];
		}
		break;
	case 1:
		if (trap_no < sizeof(trap1) / 8)
		{
			name = trap1[trap_code * 2];
			param = trap1[trap_code * 2 + 1];
		}
		break;
	case 2:
		if (trap_no < sizeof(trap2) / 8)
		{
			name = trap2[trap_code * 2];
			param = trap2[trap_code * 2 + 1];
		}
		break;
	case 3:
		if (trap_no < sizeof(trap3) / 8)
		{
			name = trap3[trap_code * 2];
			param = trap3[trap_code * 2 + 1];
		}
		break;
	case 4:
		if (trap_no < sizeof(trap4) / 8)
		{
			name = trap4[trap_code * 2];
			param = trap4[trap_code * 2 + 1];
		}
		break;
	case 5:
		if (trap_no < sizeof(trap5) / 8)
		{
			name = trap5[trap_code * 2];
			param = trap5[trap_code * 2 + 1];
		}
		break;
	case 7:
		if (trap_no < sizeof(trap7) / 8)
		{
			name = trap7[trap_code * 2];
			param = trap7[trap_code * 2 + 1];
		}
		break;
	case 8:
		if (trap_no < sizeof(trap8) / 8)
		{
			name = trap8[trap_code * 2];
			param = trap8[trap_code * 2 + 1];
		}
		break;
	}

	sb[0] = '\0';

	if (name == NULL)
	{
		strcat(sb, "???");
	}
	else
	{
		strcat(sb, name);
		if (param != NULL)
		{
			int i;
			strcat(sb, "(");
			for (i = 0, pa = sp + 4; param[i] != '\0'; i++)
			{
				switch (param[i])
				{
				case ',':
					strcat(sb, ", ");
					pa += 4;
					break;
				default:
					strcat(sb, get_param(m68k, pa, param[i]));
					break;
				}
			}
			strcat(sb, ")");
		}
	}
	return sb;
}

static const char * disassemble(m68000_base_device *m68k, offs_t pc, char* sb)
{
	UINT8 oprom[10];
	UINT8 opram[10];
	UINT32 options = 0;

	// remember bus error state
	UINT32 tmp_buserror_occurred = m68k->mmu_tmp_buserror_occurred;
	UINT32 tmp_buserror_address = m68k->mmu_tmp_buserror_address;

	m68k->mmu_tmp_buserror_occurred = 0;
	m68k->mmu_tmp_rw = 1;

	int i;
	for (i = 0; i < sizeof(oprom); i++)
	{
		oprom[i] = opram[i] = m68k->read8(pc + i);
		if (m68k->mmu_tmp_buserror_occurred)
		{
			sprintf(sb, "- (apollo_disassemble failed at %08x)", pc + i);

			// restore previous bus error state
			m68k->mmu_tmp_buserror_occurred = tmp_buserror_occurred;
			m68k->mmu_tmp_buserror_address = tmp_buserror_address;

			return sb;
		}
	}
	m68k->disassemble(sb, pc, oprom, opram, options);

	// restore previous bus error state
	m68k->mmu_tmp_buserror_occurred = tmp_buserror_occurred;
	m68k->mmu_tmp_buserror_address = tmp_buserror_address;

	return sb;
}

static const UINT16 *get_data(m68000_base_device *m68k, offs_t addr)
{
	static UINT16 data[4];

	// remember bus error state
	UINT32 tmp_buserror_occurred = m68k->mmu_tmp_buserror_occurred;
	UINT32 tmp_buserror_address = m68k->mmu_tmp_buserror_address;

	m68k->mmu_tmp_buserror_occurred = 0;
	m68k->mmu_tmp_rw = 1;

	int i;
	for (i = 0; i < sizeof(data); i += 2)
	{
		data[i/2] = m68k->read16(addr + i);
	}

	// restore previous bus error state
	m68k->mmu_tmp_buserror_occurred = tmp_buserror_occurred;
	m68k->mmu_tmp_buserror_address = tmp_buserror_address;

	return data;
}

//-------------------------------------------------
//  instruction_hook - should be called by the CPU core
//  before executing each instruction
//-------------------------------------------------

int apollo_debug_instruction_hook(m68000_base_device *device, offs_t curpc)
{
	// trap data remembered for next rte
	static struct {
		UINT32 pc;
		UINT32 sp;
		UINT16 trap_no;
		UINT16 trap_code;
	} trap = { 0, 0, 0, 0 };

	if (apollo_config( APOLLO_CONF_TRAP_TRACE | APOLLO_CONF_FPU_TRACE))
	{
		UINT32 ppc_save;
		UINT16 ir;
		m68000_base_device *m68k = device;
		m68k->mmu_tmp_buserror_occurred = 0;

		/* Read next instruction */
		ir = (m68k->pref_addr == REG_PC(m68k)) ? m68k->pref_data : m68k->readimm16(REG_PC(m68k));

		// apollo_cpu_context expects the PC of current opcode in REG_PPC (not the previous PC)
		ppc_save = REG_PPC(m68k);
		REG_PPC(m68k) = REG_PC(m68k);

		if (m68k->mmu_tmp_buserror_occurred)
		{
			m68k->mmu_tmp_buserror_occurred = 0;
			// give up
		}
		else if ((ir & 0xff00) == 0xf200 && (apollo_config( APOLLO_CONF_FPU_TRACE)))
		{
			char sb[256];
			DLOG(("%s sp=%08x FPU: %x %s", apollo_cpu_context(device->machine().firstcpu),
					REG_A(m68k)[7], ir, disassemble(m68k, REG_PC(m68k), sb)));
		}
		else if (!m68k->pmmu_enabled)
		{
			// skip
		}
		else if (ir == 0x4e73) // RTE
		{
			const UINT16 *data = get_data(m68k, REG_A(m68k)[7]);
			if ( REG_USP(m68k) == 0 && (data[0] & 0x2000) == 0) {
				DLOG(("%s sp=%08x RTE: sr=%04x pc=%04x%04x v=%04x usp=%08x",
					apollo_cpu_context(device->machine().firstcpu),
					REG_A(m68k)[7], data[0], data[1], data[2], data[3], REG_USP(m68k)));
			}
		}
		else if ((ir & 0xfff0) == 0x4e40 && (ir & 0x0f) <= 8 && apollo_config(APOLLO_CONF_TRAP_TRACE))
		{
			// trap n
			trap.pc = REG_PC(m68k);
			trap.sp = REG_A(m68k)[7];
			trap.trap_no = ir & 0x0f;
			trap.trap_code = REG_D(m68k)[0] & 0xffff;

			char sb[1000];
			DLOG(("%s sp=%08x Domain/OS SVC: trap %x 0x%02x: %s",
					apollo_cpu_context(device->machine().firstcpu), trap.sp,
					trap.trap_no, trap.trap_code,
					get_svc_call(m68k, trap.trap_no, trap.trap_code, sb)));

		}
		else if (trap.pc == REG_PC(m68k) - 2 && trap.sp == REG_A(m68k)[7])
		{
			// rte
			char sb[1000];
			DLOG(("%s sp=%08x Domain/OS SVC:              %s D0=0x%x",
					apollo_cpu_context(device->machine().firstcpu), trap.sp,
					get_svc_call(m68k, trap.trap_no, trap.trap_code, sb), REG_D(m68k)[0]));

			trap.pc = 0;
			trap.sp = 0;
			trap.trap_no = 0;
			trap.trap_code = 0;
		}
		// restore previous PC
		REG_PPC(m68k) = ppc_save;
	}
	return 0;
}
