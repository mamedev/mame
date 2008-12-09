/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

// file will be included in all cpu variants
// timing value should move to separate array

static void PREFIX186(_pusha)(i8086_state *cpustate);
static void PREFIX186(_popa)(i8086_state *cpustate);
static void PREFIX186(_bound)(i8086_state *cpustate);
static void PREFIX186(_push_d16)(i8086_state *cpustate);
static void PREFIX186(_imul_d16)(i8086_state *cpustate);
static void PREFIX186(_push_d8)(i8086_state *cpustate);
static void PREFIX186(_imul_d8)(i8086_state *cpustate);
static void PREFIX186(_rotshft_bd8)(i8086_state *cpustate);
static void PREFIX186(_rotshft_wd8)(i8086_state *cpustate);
static void PREFIX186(_enter)(i8086_state *cpustate);
static void PREFIX186(_leave)(i8086_state *cpustate);
static void PREFIX186(_insb)(i8086_state *cpustate);
static void PREFIX186(_insw)(i8086_state *cpustate);
static void PREFIX186(_outsb)(i8086_state *cpustate);
static void PREFIX186(_outsw)(i8086_state *cpustate);

/* changed instructions */
static void PREFIX186(_repne)(i8086_state *cpustate);
static void PREFIX186(_repe)(i8086_state *cpustate);
