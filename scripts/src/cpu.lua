---------------------------------------------------------------------------
--
--   cpu.lua
--
--   Rules for building CPU cores
--
--   Copyright Nicola Salmoria and the MAME Team.
--   Visit http://mamedev.org for licensing and usage restrictions.
--
---------------------------------------------------------------------------

--------------------------------------------------
-- Shared code
--------------------------------------------------

files {
	MAME_DIR .. "src/emu/cpu/vtlb.c",
}

--------------------------------------------------
-- Dynamic recompiler objects
--------------------------------------------------

if (CPUS["SH2"]~=null or CPUS["MIPS"]~=null or CPUS["POWERPC"]~=null or CPUS["RSP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/drcbec.c",
		MAME_DIR .. "src/emu/cpu/drcbeut.c",
		MAME_DIR .. "src/emu/cpu/drccache.c",
		MAME_DIR .. "src/emu/cpu/drcfe.c",
		MAME_DIR .. "src/emu/cpu/drcuml.c",
		MAME_DIR .. "src/emu/cpu/uml.c",
		MAME_DIR .. "src/emu/cpu/i386/i386dasm.c",
		MAME_DIR .. "src/emu/cpu/x86log.c",
		MAME_DIR .. "src/emu/cpu/drcbex86.c",
		MAME_DIR .. "src/emu/cpu/drcbex64.c",
	}
end

--------------------------------------------------
-- Signetics 8X300 / Scientific Micro Systems SMS300
--@src/emu/cpu/8x300/8x300.h,CPUS += 8X300
--------------------------------------------------

if (CPUS["8X300"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/8x300/8x300.c",
	}
end

if (CPUS["8X300"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/8x300/8x300dasm.c")
end

--------------------------------------------------
-- ARCangent A4
--@src/emu/cpu/arc/arc.h,CPUS += ARC
--------------------------------------------------

if (CPUS["ARC"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/arc/arc.c",
	}
end

if (CPUS["ARC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arc/arcdasm.c")
end

--------------------------------------------------
-- ARcompact (ARCtangent-A5, ARC 600, ARC 700)
--@src/emu/cpu/arc/arc.h,CPUS += ARCOMPACT
--------------------------------------------------

if (CPUS["ARCOMPACT"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/arcompact/arcompact.c",
		MAME_DIR .. "src/emu/cpu/arcompact/arcompact_execute.c",
	}
	dependency {
		{ MAME_DIR .. "src/emu/cpu/arcompact/arcompact.c",  	   GEN_DIR .. "emu/cpu/arcompact/arcompact.inc" },
		{ MAME_DIR .. "src/emu/cpu/arcompact/arcompact_execute.c", GEN_DIR .. "emu/cpu/arcompact/arcompact.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/emu/cpu/arcompact/arcompact_make.py" , GEN_DIR .. "emu/cpu/arcompact/arcompact.inc",   { MAME_DIR .. "src/emu/cpu/arcompact/arcompact_make.py" }, {"@echo Generating arcompact source .inc files...", PYTHON .. " $(1)  $(@)" }},
	}
end

if (CPUS["ARCOMPACT"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arcompact/arcompactdasm.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arcompact/arcompactdasm_dispatch.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arcompact/arcompactdasm_ops.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arcompact/arcompact_common.c")
end

--------------------------------------------------
-- Acorn ARM series
--
---@src/emu/cpu/arm/arm.h,CPUS += ARM
---@src/emu/cpu/arm7/arm7.h,CPUS += ARM7
--------------------------------------------------

if (CPUS["ARM"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/arm/arm.c",
	}
end

if (CPUS["ARM"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arm/armdasm.c")
end

if (CPUS["ARM7"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/arm7/arm7.c",
		MAME_DIR .. "src/emu/cpu/arm7/arm7thmb.c",
		MAME_DIR .. "src/emu/cpu/arm7/arm7ops.c",
	}
end

if (CPUS["ARM7"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arm7/arm7dasm.c")
end

--------------------------------------------------
-- Advanced Digital Chips SE3208
---@src/emu/cpu/se3208/se3208.h,CPUS += SE3208
--------------------------------------------------

if (CPUS["SE3208"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/se3208/se3208.c",
	}
end

if (CPUS["SE3208"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/se3208/se3208dis.c")
end

--------------------------------------------------
-- American Microsystems, Inc.(AMI) S2000 series
---@src/emu/cpu/amis2000/amis2000.h,CPUS += AMIS2000
--------------------------------------------------

if (CPUS["AMIS2000"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/amis2000/amis2000.c",
	}
end

if (CPUS["AMIS2000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/amis2000/amis2000d.c")
end

--------------------------------------------------
-- Alpha 8201
---@src/emu/cpu/alph8201/alph8201.h,CPUS += ALPHA8201
--------------------------------------------------

if (CPUS["ALPHA8201"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/alph8201/alph8201.c",
	}
end

if (CPUS["ALPHA8201"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/alph8201/8201dasm.c")
end

--------------------------------------------------
-- Analog Devices ADSP21xx series
---@src/emu/cpu/adsp2100/adsp2100.h,CPUS += ADSP21XX
--------------------------------------------------

if (CPUS["ADSP21XX"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/adsp2100/adsp2100.c",
	}
end

if (CPUS["ADSP21XX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/adsp2100/2100dasm.c")
end

--------------------------------------------------
-- Analog Devices "Sharc" ADSP21062
---@src/emu/cpu/sharc/sharc.h,CPUS += ADSP21062
--------------------------------------------------

if (CPUS["ADSP21062"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/sharc/sharc.c",
	}
end

if (CPUS["ADSP21062"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/sharc/sharcdsm.c")
end

--------------------------------------------------
-- APEXC
---@src/emu/cpu/apexc/apexc.h,CPUS += APEXC
--------------------------------------------------

if (CPUS["APEXC"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/apexc/apexc.c",
	}
end

if (CPUS["APEXC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/apexc/apexcdsm.c")
end

--------------------------------------------------
-- AT&T DSP16A
---@src/emu/cpu/dsp16/dsp16.h,CPUS += DSP16A
--------------------------------------------------

if (CPUS["DSP16A"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/dsp16/dsp16.c",
	}
end

if (CPUS["DSP16A"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp16/dsp16dis.c")
end

--------------------------------------------------
-- AT&T DSP32C
---@src/emu/cpu/dsp32/dsp32.h,CPUS += DSP32C
--------------------------------------------------

if (CPUS["DSP32C"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/dsp32/dsp32.c",
	}
end

if (CPUS["DSP32C"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp32/dsp32dis.c")
end

--------------------------------------------------
-- Atari custom RISC processor
---@src/emu/cpu/asap/asap.h,CPUS += ASAP
--------------------------------------------------

if (CPUS["ASAP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/asap/asap.c",
	}
end

if (CPUS["ASAP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/asap/asapdasm.c")
end

--------------------------------------------------
-- AMD Am29000
---@src/emu/cpu/am29000/am29000.h,CPUS += AM29000
--------------------------------------------------

if (CPUS["AM29000"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/am29000/am29000.c",
	}
end

if (CPUS["AM29000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/am29000/am29dasm.c")
end

--------------------------------------------------
-- Atari Jaguar custom DSPs
---@src/emu/cpu/jaguar/jaguar.h,CPUS += JAGUAR
--------------------------------------------------

if (CPUS["JAGUAR"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/jaguar/jaguar.c",
	}
end

if (CPUS["JAGUAR"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/jaguar/jagdasm.c")
end

--------------------------------------------------
-- Simutrek Cube Quest bit-sliced CPUs
---@src/emu/cpu/cubeqcpu/cubeqcpu.h,CPUS += CUBEQCPU
--------------------------------------------------

if (CPUS["CUBEQCPU"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/cubeqcpu/cubeqcpu.c",
	}
end

if (CPUS["CUBEQCPU"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cubeqcpu/cubedasm.c")
end

--------------------------------------------------
-- Ensoniq ES5510 ('ESP') DSP
---@src/emu/cpu/es5510/es5510.h,CPUS += ES5510
--------------------------------------------------

if (CPUS["ES5510"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/es5510/es5510.c",
	}
end

--------------------------------------------------
-- Entertainment Sciences AM29116-based RIP
---@src/emu/cpu/esrip/esrip.h,CPUS += ESRIP
--------------------------------------------------

if (CPUS["ESRIP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/esrip/esrip.c",
	}
end

if (CPUS["ESRIP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/esrip/esripdsm.c")
end

--------------------------------------------------
-- RCA COSMAC
---@src/emu/cpu/cosmac/cosmac.h,CPUS += COSMAC
--------------------------------------------------

if (CPUS["COSMAC"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/cosmac/cosmac.c",
	}
end

if (CPUS["COSMAC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cosmac/cosdasm.c")
end

--------------------------------------------------
-- National Semiconductor COP400 family
---@src/emu/cpu/cop400/cop400.h,CPUS += COP400
--------------------------------------------------

if (CPUS["COP400"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/cop400/cop400.c",
	}
end

if (CPUS["COP400"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cop400/cop410ds.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cop400/cop420ds.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cop400/cop440ds.c")
end

--------------------------------------------------
-- CP1610
---@src/emu/cpu/cp1610/cp1610.h,CPUS += CP1610
--------------------------------------------------

if (CPUS["CP1610"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/cp1610/cp1610.c",
	}
end

if (CPUS["CP1610"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cp1610/1610dasm.c")
end

--------------------------------------------------
-- Cinematronics vector "CPU"
---@src/emu/cpu/ccpu/ccpu.h,CPUS += CCPU
--------------------------------------------------

if (CPUS["CCPU"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/ccpu/ccpu.c",
	}
end

if (CPUS["CCPU"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/ccpu/ccpudasm.c")
end

--------------------------------------------------
-- DEC T-11
---@src/emu/cpu/t11/t11.h,CPUS += T11
--------------------------------------------------

if (CPUS["T11"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/t11/t11.c",
	}
end

if (CPUS["T11"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/t11/t11dasm.c")
end

--------------------------------------------------
-- F8
---@src/emu/cpu/f8/f8.h,CPUS += F8
--------------------------------------------------

if (CPUS["F8"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/f8/f8.c",
	}
end

if (CPUS["F8"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/f8/f8dasm.c")
end

--------------------------------------------------
-- G65816
---@src/emu/cpu/g65816/g65816.h,CPUS += G65816
--------------------------------------------------

if (CPUS["G65816"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/g65816/g65816.c",
		MAME_DIR .. "src/emu/cpu/g65816/g65816o0.c",
		MAME_DIR .. "src/emu/cpu/g65816/g65816o1.c",
		MAME_DIR .. "src/emu/cpu/g65816/g65816o2.c",
		MAME_DIR .. "src/emu/cpu/g65816/g65816o3.c",
		MAME_DIR .. "src/emu/cpu/g65816/g65816o4.c",
	}
end

if (CPUS["G65816"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/g65816/g65816ds.c")
end

--------------------------------------------------
-- Hitachi H8 (16/32-bit H8/300, H8/300H, H8S2000 and H8S2600 series)
---@src/emu/cpu/h8/h8.h,CPUS += H8
--------------------------------------------------

if (CPUS["H8"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/h8/h8.c",
		MAME_DIR .. "src/emu/cpu/h8/h8h.c",
		MAME_DIR .. "src/emu/cpu/h8/h8s2000.c",
		MAME_DIR .. "src/emu/cpu/h8/h8s2600.c",
		MAME_DIR .. "src/emu/cpu/h8/h83337.c",
		MAME_DIR .. "src/emu/cpu/h8/h83002.c",
		MAME_DIR .. "src/emu/cpu/h8/h83006.c",
		MAME_DIR .. "src/emu/cpu/h8/h83008.c",
		MAME_DIR .. "src/emu/cpu/h8/h83048.c",
		MAME_DIR .. "src/emu/cpu/h8/h8s2245.c",
		MAME_DIR .. "src/emu/cpu/h8/h8s2320.c",
		MAME_DIR .. "src/emu/cpu/h8/h8s2357.c",
		MAME_DIR .. "src/emu/cpu/h8/h8s2655.c",
		MAME_DIR .. "src/emu/cpu/h8/h8_adc.c",
		MAME_DIR .. "src/emu/cpu/h8/h8_port.c",
		MAME_DIR .. "src/emu/cpu/h8/h8_intc.c",
		MAME_DIR .. "src/emu/cpu/h8/h8_timer8.c",
		MAME_DIR .. "src/emu/cpu/h8/h8_timer16.c",
		MAME_DIR .. "src/emu/cpu/h8/h8_sci.c",
	}
	
	dependency {
		{ MAME_DIR .. "src/emu/cpu/h8/h8.c",       GEN_DIR .. "emu/cpu/h8/h8.inc" },
		{ MAME_DIR .. "src/emu/cpu/h8/h8h.c",      GEN_DIR .. "emu/cpu/h8/h8h.inc" },
		{ MAME_DIR .. "src/emu/cpu/h8/h8s2000.c",  GEN_DIR .. "emu/cpu/h8/h8s2000.inc" },
		{ MAME_DIR .. "src/emu/cpu/h8/h8s2600.c",  GEN_DIR .. "emu/cpu/h8/h8s2600.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/emu/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8.inc",       { MAME_DIR .. "src/emu/cpu/h8/h8make.py" }, {"@echo Generating H8-300 source file...",   PYTHON .. " $(1) $(<) o   $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8h.inc",      { MAME_DIR .. "src/emu/cpu/h8/h8make.py" }, {"@echo Generating H8-300H source file...",  PYTHON .. " $(1) $(<) h   $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8s2000.inc",  { MAME_DIR .. "src/emu/cpu/h8/h8make.py" }, {"@echo Generating H8S/2000 source file...", PYTHON .. " $(1) $(<) s20 $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8s2600.inc",  { MAME_DIR .. "src/emu/cpu/h8/h8make.py" }, {"@echo Generating H8S/2600 source file...", PYTHON .. " $(1) $(<) s26 $(@)" }},
	}
end

--------------------------------------------------
-- Hitachi HCD62121
---@src/emu/cpu/hcd62121/hcd62121.h,CPUS += HCD62121
--------------------------------------------------

if (CPUS["HCD62121"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/hcd62121/hcd62121.c",
	}
end

if (CPUS["HCD62121"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/hcd62121/hcd62121d.c")
end

--------------------------------------------------
-- Hitachi HMCS40 series
---@src/emu/cpu/hmcs40/hmcs40.h,CPUS += HMCS40
--------------------------------------------------

if (CPUS["HMCS40"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/hmcs40/hmcs40.c",
	}
end

if (CPUS["HMCS40"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/hmcs40/hmcs40d.c")
end

--------------------------------------------------
-- Hitachi SH1/SH2
---@src/emu/cpu/sh2/sh2.h,CPUS += SH2
--------------------------------------------------

if (CPUS["SH2"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/sh2/sh2.c",
		MAME_DIR .. "src/emu/cpu/sh2/sh2fe.c",
	}
end

if (CPUS["SH2"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/sh2/sh2dasm.c")
end

--------------------------------------------------
-- Hitachi SH4
---@src/emu/cpu/sh4/sh4.h,CPUS += SH4
--------------------------------------------------

if (CPUS["SH4"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/sh4/sh4.c",
		MAME_DIR .. "src/emu/cpu/sh4/sh4comn.c",
		MAME_DIR .. "src/emu/cpu/sh4/sh3comn.c",
		MAME_DIR .. "src/emu/cpu/sh4/sh4tmu.c",
		MAME_DIR .. "src/emu/cpu/sh4/sh4dmac.c",
	}
end

if (CPUS["SH4"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/sh4/sh4dasm.c")
end

--------------------------------------------------
-- Hudsonsoft 6280
---@src/emu/cpu/h6280/h6280.h,CPUS += H6280
--------------------------------------------------

if (CPUS["H6280"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/h6280/h6280.c",
	}
end

if (CPUS["H6280"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/h6280/6280dasm.c")
end

--------------------------------------------------
-- Hyperstone E1 series
---@src/emu/cpu/e132xs/e132xs.h,CPUS += E1
--------------------------------------------------

if (CPUS["E1"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/e132xs/e132xs.c",
	}
end

if (CPUS["E1"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/e132xs/32xsdasm.c")
end

--------------------------------------------------
-- 15IE-00-013 CPU ("Microprogrammed Control Device")
---@src/emu/cpu/ie15/ie15.h,CPUS += IE15
--------------------------------------------------

if (CPUS["IE15"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/ie15/ie15.c",
	}
end

if (CPUS["IE15"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/ie15/ie15dasm.c")
end

--------------------------------------------------
-- Intel 4004
---@src/emu/cpu/i4004/i4004.h,CPUS += I4004
--------------------------------------------------

if (CPUS["I4004"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i4004/i4004.c",
	}
end

if (CPUS["I4004"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i4004/4004dasm.c")
end

--------------------------------------------------
-- Intel 8008
---@src/emu/cpu/i8008/i8008.h,CPUS += I8008
--------------------------------------------------

if (CPUS["I8008"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i8008/i8008.c",
	}
end

if (CPUS["I8008"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i8008/8008dasm.c")
end

--------------------------------------------------
--  National Semiconductor SC/MP
---@src/emu/cpu/scmp/scmp.h,CPUS += SCMP
--------------------------------------------------

if (CPUS["SCMP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/scmp/scmp.c",
	}
end

if (CPUS["SCMP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/scmp/scmpdasm.c")
end

--------------------------------------------------
-- Intel 8080/8085A
---@src/emu/cpu/i8085/i8085.h,CPUS += I8085
--------------------------------------------------

if (CPUS["I8085"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i8085/i8085.c",
	}
end

if (CPUS["I8085"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i8085/8085dasm.c")
end

--------------------------------------------------
-- Intel 8089
---@src/emu/cpu/i8085/i8089.h,CPUS += I8089
--------------------------------------------------

if (CPUS["I8089"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i8089/i8089.c",
		MAME_DIR .. "src/emu/cpu/i8089/i8089_channel.c",
		MAME_DIR .. "src/emu/cpu/i8089/i8089_ops.c",
	}
end

if (CPUS["I8089"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i8089/i8089_dasm.c")
end

--------------------------------------------------
-- Intel MCS-48 (8039 and derivatives)
---@src/emu/cpu/mcs48/mcs48.h,CPUS += MCS48
--------------------------------------------------

if (CPUS["MCS48"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mcs48/mcs48.c",
	}
end

if (CPUS["MCS48"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mcs48/mcs48dsm.c")
end

--------------------------------------------------
-- Intel 8051 and derivatives
---@src/emu/cpu/mcs51/mcs51.h,CPUS += MCS51
--------------------------------------------------

if (CPUS["MCS51"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mcs51/mcs51.c",
	}
end

if (CPUS["MCS51"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mcs51/mcs51dasm.c")
end

--------------------------------------------------
-- Intel MCS-96
---@src/emu/cpu/mcs96/mcs96.h,CPUS += MCS96
--------------------------------------------------

if (CPUS["MCS96"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mcs96/mcs96.c",
		MAME_DIR .. "src/emu/cpu/mcs96/i8x9x.c",
		MAME_DIR .. "src/emu/cpu/mcs96/i8xc196.c",
	}
	dependency {
		{ MAME_DIR .. "src/emu/cpu/mcs96/mcs96.c",   GEN_DIR .. "emu/cpu/mcs96/mcs96.inc" },
		{ MAME_DIR .. "src/emu/cpu/mcs96/i8x9x.c",   GEN_DIR .. "emu/cpu/mcs96/i8x9x.inc" },
		{ MAME_DIR .. "src/emu/cpu/mcs96/i8xc196.c", GEN_DIR .. "emu/cpu/mcs96/i8xc196.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/emu/cpu/mcs96/mcs96ops.lst" , GEN_DIR .. "emu/cpu/mcs96/mcs96.inc",   { MAME_DIR .. "src/emu/cpu/mcs96/mcs96make.py" }, {"@echo Generating mcs96 source file...", PYTHON .. " $(1) mcs96 $(<) $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/mcs96/mcs96ops.lst" , GEN_DIR .. "emu/cpu/mcs96/i8x9x.inc",   { MAME_DIR .. "src/emu/cpu/mcs96/mcs96make.py" }, {"@echo Generating i8x9x source file...", PYTHON .. " $(1) i8x9x $(<) $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/mcs96/mcs96ops.lst" , GEN_DIR .. "emu/cpu/mcs96/i8xc196.inc", { MAME_DIR .. "src/emu/cpu/mcs96/mcs96make.py" }, {"@echo Generating i8xc196 source file...", PYTHON .. " $(1) i8xc196 $(<) $(@)" }},
	}
end

--------------------------------------------------
-- Intel 80x86 series
---@src/emu/cpu/i86/i86.h,CPUS += I86
---@src/emu/cpu/i86/i286.h,CPUS += I86
---@src/emu/cpu/i386/i386.h,CPUS += I386
--------------------------------------------------

if (CPUS["I86"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i86/i86.c",
		MAME_DIR .. "src/emu/cpu/i86/i186.c",
		MAME_DIR .. "src/emu/cpu/i86/i286.c",
	}
end

if (CPUS["I86"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i386/i386dasm.c")
end

if (CPUS["I386"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i386/i386.c",
	}
end

if (CPUS["I386"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i386/i386dasm.c")
end

--------------------------------------------------
-- Intel i860
---@src/emu/cpu/i860/i860.h,CPUS += I860
--------------------------------------------------

if (CPUS["I860"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i860/i860.c",
	}
end

if (CPUS["I860"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i860/i860dis.c")
end

--------------------------------------------------
-- Intel i960
---@src/emu/cpu/i960/i960.h,CPUS += I960
--------------------------------------------------

if (CPUS["I960"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i960/i960.c",
	}
end

if (CPUS["I960"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i960/i960dis.c")
end

--------------------------------------------------
-- LH5801
---@src/emu/cpu/lh5801/lh5801.h,CPUS += LH5801
--------------------------------------------------

if (CPUS["LH5801"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/lh5801/lh5801.c",
	}
end

if (CPUS["LH5801"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/lh5801/5801dasm.c")
end
--------
------------------------------------------
-- Manchester Small-Scale Experimental Machine
---@src/emu/cpu/ssem/ssem.h,CPUS += SSEM
--------------------------------------------------

if (CPUS["SSEM"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/ssem/ssem.c",
	}
end

if (CPUS["SSEM"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/ssem/ssemdasm.c")
end

--------------------------------------------------
-- Fujitsu MB88xx
---@src/emu/cpu/mb88xx/mb88xx.h,CPUS += MB88XX
--------------------------------------------------

if (CPUS["MB88XX"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mb88xx/mb88xx.c",
	}
end

if (CPUS["MB88XX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mb88xx/mb88dasm.c")
end

--------------------------------------------------
-- Fujitsu MB86233
---@src/emu/cpu/mb86233/mb86233.h,CPUS += MB86233
--------------------------------------------------

if (CPUS["MB86233"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mb86233/mb86233.c",
	}
end

if (CPUS["MB86233"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mb86233/mb86233d.c")
end

--------------------------------------------------
-- Fujitsu MB86235
---@src/emu/cpu/mb86233/mb86235.h,CPUS += MB86235
--------------------------------------------------

if (CPUS["MB86235"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mb86235/mb86235.c",
	}
end

if (CPUS["MB86235"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mb86235/mb86235d.c")
end

--------------------------------------------------
-- Microchip PIC16C5x
---@src/emu/cpu/pic16c5x/pic16c5x.h,CPUS += PIC16C5X
--------------------------------------------------

if (CPUS["PIC16C5X"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/pic16c5x/pic16c5x.c",
	}
end

if (CPUS["PIC16C5X"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/pic16c5x/16c5xdsm.c")
end

--------------------------------------------------
-- Microchip PIC16C62x
---@src/emu/cpu/pic16c62x/pic16c62x.h,CPUS += PIC16C62X
--------------------------------------------------

if (CPUS["PIC16C62X"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/pic16c62x/pic16c62x.c",
	}
end

if (CPUS["PIC16C62X"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/pic16c62x/16c62xdsm.c")
end

--------------------------------------------------
-- MIPS R3000 (MIPS I/II) series
-- MIPS R4000 (MIPS III/IV) series
---@src/emu/cpu/mips/mips3.h,CPUS += MIPS
--------------------------------------------------

if (CPUS["MIPS"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mips/r3000.c",
		MAME_DIR .. "src/emu/cpu/mips/mips3com.c",
		MAME_DIR .. "src/emu/cpu/mips/mips3.c",
		MAME_DIR .. "src/emu/cpu/mips/mips3fe.c",
		MAME_DIR .. "src/emu/cpu/mips/mips3drc.c",
	}
end

if (CPUS["MIPS"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mips/r3kdasm.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mips/mips3dsm.c")
end

--------------------------------------------------
-- Sony PlayStation CPU (R3000-based + GTE)
---@src/emu/cpu/psx/psx.h,CPUS += PSX
--------------------------------------------------

if (CPUS["PSX"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/psx/psx.c",
		MAME_DIR .. "src/emu/cpu/psx/gte.c",
		MAME_DIR .. "src/emu/cpu/psx/dma.c",
		MAME_DIR .. "src/emu/cpu/psx/irq.c",
		MAME_DIR .. "src/emu/cpu/psx/mdec.c",
		MAME_DIR .. "src/emu/cpu/psx/rcnt.c",
		MAME_DIR .. "src/emu/cpu/psx/sio.c",
	}
end

if (CPUS["PSX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/psx/psxdasm.c")
end

--------------------------------------------------
-- Mitsubishi M37702 and M37710 (based on 65C816)
---@src/emu/cpu/m37710/m37710.h,CPUS += M37710
--------------------------------------------------

if (CPUS["M37710"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/m37710/m37710.c",
		MAME_DIR .. "src/emu/cpu/m37710/m37710o0.c",
		MAME_DIR .. "src/emu/cpu/m37710/m37710o1.c",
		MAME_DIR .. "src/emu/cpu/m37710/m37710o2.c",
		MAME_DIR .. "src/emu/cpu/m37710/m37710o3.c",
	}
end

if (CPUS["M37710"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m37710/m7700ds.c")
end

--------------------------------------------------
-- Mostek 6502 and its many derivatives
---@src/emu/cpu/m6502/m6502.h,CPUS += M6502
---@src/emu/cpu/m6502/deco16.h,CPUS += M6502
---@src/emu/cpu/m6502/m4510.h,CPUS += M6502
---@src/emu/cpu/m6502/m65ce02.h,CPUS += M6502
---@src/emu/cpu/m6502/m65c02.h,CPUS += M6502
---@src/emu/cpu/m6502/r65c02.h,CPUS += M6502
---@src/emu/cpu/m6502/m65sc02.h,CPUS += M6502
---@src/emu/cpu/m6502/m6504.h,CPUS += M6502
---@src/emu/cpu/m6502/m6509.h,CPUS += M6502
---@src/emu/cpu/m6502/m6510.h,CPUS += M6502
---@src/emu/cpu/m6502/m6510t.h,CPUS += M6502
---@src/emu/cpu/m6502/m7501.h,CPUS += M6502
---@src/emu/cpu/m6502/m8502.h,CPUS += M6502
---@src/emu/cpu/m6502/n2a03.h,CPUS += M6502
---@src/emu/cpu/m6502/m740.h,CPUS += M6502
---@src/emu/cpu/m6502/m3745x.h,CPUS += M6502
---@src/emu/cpu/m6502/m5074x.h,CPUS += M6502

--------------------------------------------------

if (CPUS["M6502"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/m6502/deco16.c",
		MAME_DIR .. "src/emu/cpu/m6502/m4510.c",
		MAME_DIR .. "src/emu/cpu/m6502/m6502.c",
		MAME_DIR .. "src/emu/cpu/m6502/m65c02.c",
		MAME_DIR .. "src/emu/cpu/m6502/m65ce02.c",
		MAME_DIR .. "src/emu/cpu/m6502/m65sc02.c",
		MAME_DIR .. "src/emu/cpu/m6502/m6504.c",
		MAME_DIR .. "src/emu/cpu/m6502/m6509.c",
		MAME_DIR .. "src/emu/cpu/m6502/m6510.c",
		MAME_DIR .. "src/emu/cpu/m6502/m6510t.c",
		MAME_DIR .. "src/emu/cpu/m6502/m7501.c",
		MAME_DIR .. "src/emu/cpu/m6502/m8502.c",
		MAME_DIR .. "src/emu/cpu/m6502/n2a03.c",
		MAME_DIR .. "src/emu/cpu/m6502/r65c02.c",
		MAME_DIR .. "src/emu/cpu/m6502/m740.c",
		MAME_DIR .. "src/emu/cpu/m6502/m3745x.c",
		MAME_DIR .. "src/emu/cpu/m6502/m5074x.c",
	}
	
	dependency {
		{ MAME_DIR .. "src/emu/cpu/m6502/deco16.c",   GEN_DIR .. "emu/cpu/m6502/deco16.inc" },
		{ MAME_DIR .. "src/emu/cpu/m6502/m4510.c",    GEN_DIR .. "emu/cpu/m6502/m4510.inc" },
		{ MAME_DIR .. "src/emu/cpu/m6502/m6502.c",    GEN_DIR .. "emu/cpu/m6502/m6502.inc" },
		{ MAME_DIR .. "src/emu/cpu/m6502/m65c02.c",   GEN_DIR .. "emu/cpu/m6502/m65c02.inc" },
		{ MAME_DIR .. "src/emu/cpu/m6502/m65ce02.c",  GEN_DIR .. "emu/cpu/m6502/m65ce02.inc" },
		{ MAME_DIR .. "src/emu/cpu/m6502/m6509.c",    GEN_DIR .. "emu/cpu/m6502/m6509.inc" },
		{ MAME_DIR .. "src/emu/cpu/m6502/m6510.c",    GEN_DIR .. "emu/cpu/m6502/m6510.inc" },
		{ MAME_DIR .. "src/emu/cpu/m6502/n2a03.c",    GEN_DIR .. "emu/cpu/m6502/n2a03.inc" },
		{ MAME_DIR .. "src/emu/cpu/m6502/r65c02.c",   GEN_DIR .. "emu/cpu/m6502/r65c02.inc" },
		{ MAME_DIR .. "src/emu/cpu/m6502/m740.c",     GEN_DIR .. "emu/cpu/m6502/m740.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/emu/cpu/m6502/odeco16.lst", GEN_DIR .. "emu/cpu/m6502/deco16.inc", { MAME_DIR .. "src/emu/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/emu/cpu/m6502/ddeco16.lst"  }, {"@echo Generating deco16 source file...", PYTHON .. " $(1) deco16_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/m6502/om4510.lst",  GEN_DIR .. "emu/cpu/m6502/m4510.inc",  { MAME_DIR .. "src/emu/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/emu/cpu/m6502/dm4510.lst"   }, {"@echo Generating m4510 source file...", PYTHON .. " $(1) m4510_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/m6502/om6502.lst",  GEN_DIR .. "emu/cpu/m6502/m6502.inc",  { MAME_DIR .. "src/emu/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/emu/cpu/m6502/dm6502.lst"   }, {"@echo Generating m6502 source file...", PYTHON .. " $(1) m6502_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/m6502/om65c02.lst", GEN_DIR .. "emu/cpu/m6502/m65c02.inc", { MAME_DIR .. "src/emu/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/emu/cpu/m6502/dm65c02.lst"  }, {"@echo Generating m65c02 source file...", PYTHON .. " $(1) m65c02_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/m6502/om65ce02.lst",GEN_DIR .. "emu/cpu/m6502/m65ce02.inc",{ MAME_DIR .. "src/emu/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/emu/cpu/m6502/dm65ce02.lst" }, {"@echo Generating m65ce02 source file...", PYTHON .. " $(1) m65ce02_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/m6502/om6509.lst",  GEN_DIR .. "emu/cpu/m6502/m6509.inc",  { MAME_DIR .. "src/emu/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/emu/cpu/m6502/dm6509.lst"   }, {"@echo Generating m6509 source file...", PYTHON .. " $(1) m6509_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/m6502/om6510.lst",  GEN_DIR .. "emu/cpu/m6502/m6510.inc",  { MAME_DIR .. "src/emu/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/emu/cpu/m6502/dm6510.lst"   }, {"@echo Generating m6510 source file...", PYTHON .. " $(1) m6510_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/m6502/on2a03.lst",  GEN_DIR .. "emu/cpu/m6502/n2a03.inc",  { MAME_DIR .. "src/emu/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/emu/cpu/m6502/dn2a03.lst"   }, {"@echo Generating n2a03 source file...", PYTHON .. " $(1) n2a03_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/m6502/om740.lst" ,  GEN_DIR .. "emu/cpu/m6502/m740.inc",   { MAME_DIR .. "src/emu/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/emu/cpu/m6502/dm740.lst"    }, {"@echo Generating m740 source file...", PYTHON .. " $(1) m740_device $(<) $(2) $(@)" }},

		{ MAME_DIR .. "src/emu/cpu/m6502/dr65c02.lst", GEN_DIR .. "emu/cpu/m6502/r65c02.inc", { MAME_DIR .. "src/emu/cpu/m6502/m6502make.py" }, {"@echo Generating r65c02 source file...", PYTHON .. " $(1) r65c02_device - $(<) $(@)" }},
	}
end

--------------------------------------------------
-- Motorola 680x
---@src/emu/cpu/m6800/m6800.h,CPUS += M6800
--------------------------------------------------

if (CPUS["M6800"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/m6800/m6800.c",
	}
end

if (CPUS["M6800"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m6800/6800dasm.c")
end

--------------------------------------------------
-- Motorola 6805
---@src/emu/cpu/m6805/m6805.h,CPUS += M6805
--------------------------------------------------

if (CPUS["M6805"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/m6805/m6805.c",
	}
end

if (CPUS["M6805"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m6805/6805dasm.c")
end

--------------------------------------------------
-- Motorola 6809
---@src/emu/cpu/m6809/m6809.h,CPUS += M6809
---@src/emu/cpu/m6809/hd6309.h,CPUS += M6809
---@src/emu/cpu/m6809/konami.h,CPUS += M6809
--------------------------------------------------

if (CPUS["M6809"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/m6809/m6809.c",
		MAME_DIR .. "src/emu/cpu/m6809/hd6309.c",
		MAME_DIR .. "src/emu/cpu/m6809/konami.c",
	}

	dependency {
		{ MAME_DIR .. "src/emu/cpu/m6809/m6809.c",   GEN_DIR .. "emu/cpu/m6809/m6809.inc" },
		{ MAME_DIR .. "src/emu/cpu/m6809/hd6309.c",  GEN_DIR .. "emu/cpu/m6809/hd6309.inc" },
		{ MAME_DIR .. "src/emu/cpu/m6809/konami.c",  GEN_DIR .. "emu/cpu/m6809/konami.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/emu/cpu/m6809/m6809.ops"  , GEN_DIR .. "emu/cpu/m6809/m6809.inc",   { MAME_DIR .. "src/emu/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/emu/cpu/m6809/base6x09.ops"  }, {"@echo Generating m6809 source file...", PYTHON .. " $(1) $(<) > $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/m6809/hd6309.ops" , GEN_DIR .. "emu/cpu/m6809/hd6309.inc",  { MAME_DIR .. "src/emu/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/emu/cpu/m6809/base6x09.ops"  }, {"@echo Generating hd6309 source file...", PYTHON .. " $(1) $(<) > $(@)" }},
		{ MAME_DIR .. "src/emu/cpu/m6809/konami.ops" , GEN_DIR .. "emu/cpu/m6809/konami.inc",  { MAME_DIR .. "src/emu/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/emu/cpu/m6809/base6x09.ops"  }, {"@echo Generating konami source file...", PYTHON .. " $(1) $(<) > $(@)" }},
	}
end

if (CPUS["M6809"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m6809/6809dasm.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m6809/6309dasm.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m6809/knmidasm.c")
end

--------------------------------------------------
-- Motorola 68HC11
---@src/emu/cpu/mc68hc11/mc68hc11.h,CPUS += MC68HC11
--------------------------------------------------

if (CPUS["MC68HC11"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mc68hc11/mc68hc11.c",
	}
end

if (CPUS["MC68HC11"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mc68hc11/hc11dasm.c")
end

--------------------------------------------------
-- Motorola 68000 series
---@src/emu/cpu/m68000/m68000.h,CPUS += M680X0
--------------------------------------------------

if (CPUS["M680X0"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/m68000/m68kcpu.c",
		MAME_DIR .. "src/emu/cpu/m68000/m68kops.c",
	}
end

if (CPUS["M680X0"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m68000/m68kdasm.c")
end

--------------------------------------------------
-- Motorola/Freescale dsp56k
---@src/emu/cpu/dsp56k/dsp56k.h,CPUS += DSP56156
--------------------------------------------------

if (CPUS["DSP56156"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/dsp56k/dsp56k.c",
		MAME_DIR .. "src/emu/cpu/dsp56k/dsp56mem.c",
		MAME_DIR .. "src/emu/cpu/dsp56k/dsp56pcu.c",
	}
end

if (CPUS["DSP56156"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp56k/dsp56dsm.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp56k/opcode.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp56k/inst.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp56k/pmove.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp56k/tables.c")
end

--------------------------------------------------
-- PDP-1
-- TX0
---@src/emu/cpu/pdp1/pdp1.h,CPUS += PDP1
---@src/emu/cpu/pdp1/tx0.h,CPUS += PDP1
--------------------------------------------------

if (CPUS["PDP1"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/pdp1/pdp1.c",
		MAME_DIR .. "src/emu/cpu/pdp1/tx0.c",
	}
end

if (CPUS["PDP1"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/pdp1/pdp1dasm.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/pdp1/tx0dasm.c")
end

--------------------------------------------------
-- Motorola PowerPC series
---@src/emu/cpu/powerpc/ppc.h,CPUS += POWERPC
--------------------------------------------------

if (CPUS["POWERPC"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/powerpc/ppccom.c",
		MAME_DIR .. "src/emu/cpu/powerpc/ppcfe.c",
		MAME_DIR .. "src/emu/cpu/powerpc/ppcdrc.c",
	}
end

if (CPUS["POWERPC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/powerpc/ppc_dasm.c")
end

--------------------------------------------------
-- NEC V-series Intel-compatible
---@src/emu/cpu/nec/nec.h,CPUS += NEC
---@src/emu/cpu/v30mz/v30mz.h,CPUS += V30MZ
--------------------------------------------------

if (CPUS["NEC"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/nec/nec.c",
		MAME_DIR .. "src/emu/cpu/nec/v25.c",
		MAME_DIR .. "src/emu/cpu/nec/v25sfr.c",
		MAME_DIR .. "src/emu/cpu/nec/v53.c",
	}
end

if (CPUS["NEC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/nec/necdasm.c")
end

if (CPUS["V30MZ"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/v30mz/v30mz.c",
	}
end

if (CPUS["V30MZ"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/nec/necdasm.c")
end

--------------------------------------------------
-- NEC V60/V70
---@src/emu/cpu/v60/v60.h,CPUS += V60
--------------------------------------------------

if (CPUS["V60"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/v60/v60.c",
	}
end

if (CPUS["V60"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/v60/v60d.c")
end

--------------------------------------------------
-- NEC V810 (uPD70732)
---@src/emu/cpu/v810/v810.h,CPUS += V810
--------------------------------------------------

if (CPUS["V810"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/v810/v810.c",
	}
end

if (CPUS["V810"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/v810/v810dasm.c")
end

--------------------------------------------------
-- NEC uPD7725
---@src/emu/cpu/upd7725/upd7725.h,CPUS += UPD7725
--------------------------------------------------

if (CPUS["UPD7725"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/upd7725/upd7725.c",
	}
end

if (CPUS["UPD7725"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/upd7725/dasm7725.c")
end

--------------------------------------------------
-- NEC uPD7810 series
---@src/emu/cpu/upd7810/upd7810.h,CPUS += UPD7810
--------------------------------------------------

if (CPUS["UPD7810"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/upd7810/upd7810.c",
		MAME_DIR .. "src/emu/cpu/upd7810/upd7810_opcodes.c",
		MAME_DIR .. "src/emu/cpu/upd7810/upd7810_table.c",
	}
end

if (CPUS["UPD7810"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/upd7810/upd7810_dasm.c")
end

--------------------------------------------------
-- NEC uCOM-4 series
---@src/emu/cpu/ucom4/ucom4.h,CPUS += UCOM4
--------------------------------------------------

if (CPUS["UCOM4"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/ucom4/ucom4.c",
	}
end

if (CPUS["UCOM4"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/ucom4/ucom4d.c")
end

--------------------------------------------------
-- Nintendo Minx
---@src/emu/cpu/minx/minx.h,CPUS += MINX
--------------------------------------------------

if (CPUS["MINX"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/minx/minx.c",
	}
end

if (CPUS["MINX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/minx/minxd.c")
end

--------------------------------------------------
-- Nintendo/SGI RSP (R3000-based + vector processing)
---@src/emu/cpu/rsp/rsp.h,CPUS += RSP
--------------------------------------------------

if (CPUS["RSP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/rsp/rsp.c",
		MAME_DIR .. "src/emu/cpu/rsp/rspdrc.c",
		MAME_DIR .. "src/emu/cpu/rsp/rspfe.c",
		MAME_DIR .. "src/emu/cpu/rsp/rspcp2.c",
		MAME_DIR .. "src/emu/cpu/rsp/rspcp2d.c",
	}
end

if (CPUS["RSP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/rsp/rsp_dasm.c")
end

--------------------------------------------------
-- Panasonic MN10200
---@src/emu/cpu/mn10200/mn10200.h,CPUS += MN10200
--------------------------------------------------

if (CPUS["MN10200"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mn10200/mn10200.c",
	}
end

if (CPUS["MN10200"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mn10200/mn102dis.c")
end

--------------------------------------------------
-- Saturn
---@src/emu/cpu/saturn/saturn.h,CPUS += SATURN
--------------------------------------------------

if (CPUS["SATURN"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/saturn/saturn.c",
	}
end

if (CPUS["SATURN"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/saturn/saturnds.c")
end

--------------------------------------------------
-- Signetics 2650
---@src/emu/cpu/s2650/s2650.h,CPUS += S2650
--------------------------------------------------

if (CPUS["S2650"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/s2650/s2650.c",
	}
end

if (CPUS["S2650"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/s2650/2650dasm.c")
end

--------------------------------------------------
-- SC61860
---@src/emu/cpu/sc61860/sc61860.h,CPUS += SC61860
--------------------------------------------------

if (CPUS["SC61860"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/sc61860/sc61860.c",
	}
end

if (CPUS["SC61860"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/sc61860/scdasm.c")
end

--------------------------------------------------
-- SM8500
---@src/emu/cpu/sm8500/sm8500.h,CPUS += SM8500
--------------------------------------------------

if (CPUS["SM8500"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/sm8500/sm8500.c",
	}
end

if (CPUS["SM8500"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/sm8500/sm8500d.c")
end

--------------------------------------------------
-- Sony/Nintendo SPC700
---@src/emu/cpu/spc700/spc700.h,CPUS += SPC700
--------------------------------------------------

if (CPUS["SPC700"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/spc700/spc700.c",
	}
end

if (CPUS["SPC700"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/spc700/spc700ds.c")
end

--------------------------------------------------
-- SSP1601
---@src/emu/cpu/ssp1601/ssp1601.h,CPUS += SSP1601
--------------------------------------------------

if (CPUS["SSP1601"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/ssp1601/ssp1601.c",
	}
end

if (CPUS["SSP1601"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/ssp1601/ssp1601d.c")
end

--------------------------------------------------
-- SunPlus u'nSP
---@src/emu/cpu/unsp/unsp.h,CPUS += UNSP
--------------------------------------------------

if (CPUS["UNSP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/unsp/unsp.c",
	}
end

if (CPUS["UNSP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/unsp/unspdasm.c")
end

--------------------------------------------------
-- Atmel 8-bit AVR
---@src/emu/cpu/avr8/avr8.h,CPUS += AVR8
--------------------------------------------------

if (CPUS["AVR8"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/avr8/avr8.c",
	}
end

if (CPUS["AVR8"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/avr8/avr8dasm.c")
end

--------------------------------------------------
-- Texas Instruments TMS0980
---@src/emu/cpu/tms0980/tms0980.h,CPUS += TMS0980
--------------------------------------------------

if (CPUS["TMS0980"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms0980/tms0980.c",
	}
end

if (CPUS["TMS0980"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms0980/tms0980d.c")
end

--------------------------------------------------
-- Texas Instruments TMS7000 series
---@src/emu/cpu/tms7000/tms7000.h,CPUS += TMS7000
--------------------------------------------------

if (CPUS["TMS7000"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms7000/tms7000.c",
	}
end

if (CPUS["TMS7000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms7000/7000dasm.c")
end

--------------------------------------------------
-- Texas Instruments TMS99xx series
---@src/emu/cpu/tms9900/tms9900.h,CPUS += TMS9900
---@src/emu/cpu/tms9900/tms9980a.h,CPUS += TMS9900
---@src/emu/cpu/tms9900/tms9995.h,CPUS += TMS9900

--------------------------------------------------

if (CPUS["TMS9900"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms9900/tms9900.c",
		MAME_DIR .. "src/emu/cpu/tms9900/tms9980a.c",
		MAME_DIR .. "src/emu/cpu/tms9900/tms9995.c",
		MAME_DIR .. "src/emu/cpu/tms9900/ti990_10.c",
	}
end

if (CPUS["TMS9900"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms9900/9900dasm.c")
end

--------------------------------------------------
-- Texas Instruments TMS340x0 graphics controllers
---@src/emu/cpu/tms34010/tms34010.h,CPUS += TMS340X0
--------------------------------------------------

if (CPUS["TMS340X0"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms34010/tms34010.c",
	}
end

if (CPUS["TMS340X0"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms34010/34010dsm.c")
end

--------------------------------------------------
-- Texas Instruments TMS3201x DSP
---@src/emu/cpu/tms32010/tms32010.h,CPUS += TMS32010
--------------------------------------------------

if (CPUS["TMS32010"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms32010/tms32010.c",
	}
end

if (CPUS["TMS32010"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32010/32010dsm.c")
end

--------------------------------------------------
-- Texas Instruments TMS3202x DSP
---@src/emu/cpu/tms32025/tms32025.h,CPUS += TMS32025
--------------------------------------------------

if (CPUS["TMS32025"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms32025/tms32025.c",
	}
end

if (CPUS["TMS32025"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32025/32025dsm.c")
end

--------------------------------------------------
-- Texas Instruments TMS3203x DSP
---@src/emu/cpu/tms32031/tms32031.h,CPUS += TMS32031
--------------------------------------------------

if (CPUS["TMS32031"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms32031/tms32031.c",
	}
end

if (CPUS["TMS32031"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32031/dis32031.c")
end

--------------------------------------------------
-- Texas Instruments TMS3205x DSP
---@src/emu/cpu/tms32051/tms32051.h,CPUS += TMS32051
--------------------------------------------------

if (CPUS["TMS32051"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms32051/tms32051.c",
	}
end

if (CPUS["TMS32051"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32051/dis32051.c")
end

--------------------------------------------------
-- Texas Instruments TMS3208x DSP
---@src/emu/cpu/tms32082/tms32082.h,CPUS += TMS32082_MP
--------------------------------------------------

if (CPUS["TMS32082"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms32082/tms32082.c",
		MAME_DIR .. "src/emu/cpu/tms32082/mp_ops.c",
	}
end

if (CPUS["TMS32082"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32082/dis_mp.c")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32082/dis_pp.c")
end

--------------------------------------------------
-- Texas Instruments TMS57002 DSP
---@src/emu/cpu/tms57002/tms57002.h,CPUS += TMS57002
--------------------------------------------------

if (CPUS["TMS57002"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms57002/tms57002.c",
		MAME_DIR .. "src/emu/cpu/tms57002/tms57kdec.c",
	}
	dependency {
		{ MAME_DIR .. "src/emu/cpu/tms57002/tms57kdec.c", GEN_DIR .. "emu/cpu/tms57002/tms57002.inc" },
		{ MAME_DIR .. "src/emu/cpu/tms57002/tms57002.c",  GEN_DIR .. "emu/cpu/tms57002/tms57002.inc" },
	}
	custombuildtask { 	
		{ MAME_DIR .. "src/emu/cpu/tms57002/tmsinstr.lst" , GEN_DIR .. "emu/cpu/tms57002/tms57002.inc",   { MAME_DIR .. "src/emu/cpu/tms57002/tmsmake.py" }, {"@echo Generating TMS57002 source file...", PYTHON .. " $(1) $(<) $(@)" } }
	}
end

if (CPUS["TMS57002"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms57002/57002dsm.c")
	table.insert(disasm_dependency , { MAME_DIR .. "src/emu/cpu/tms57002/57002dsm.c",  GEN_DIR .. "emu/cpu/tms57002/tms57002.inc" } )
	table.insert(disasm_custombuildtask , { MAME_DIR .. "src/emu/cpu/tms57002/tmsinstr.lst" , GEN_DIR .. "emu/cpu/tms57002/tms57002.inc",   { MAME_DIR .. "src/emu/cpu/tms57002/tmsmake.py" }, {"@echo Generating TMS57002 source file...", PYTHON .. " $(1) $(<) $(@)" }})
end

--------------------------------------------------
-- Toshiba TLCS-90 Series
---@src/emu/cpu/tlcs90/tlcs90.h,CPUS += TLCS90
--------------------------------------------------

if (CPUS["TLCS90"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tlcs90/tlcs90.c",
	}
end

--------------------------------------------------
-- Toshiba TLCS-900 Series
---@src/emu/cpu/tlcs900/tlcs900.h,CPUS += TLCS900
--------------------------------------------------

if (CPUS["TLCS900"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tlcs900/tlcs900.c",
	}
end

if (CPUS["TLCS900"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tlcs900/dasm900.c")
end

--------------------------------------------------
-- Zilog Z80
---@src/emu/cpu/z80/z80.h,CPUS += Z80
--------------------------------------------------

if (CPUS["Z80"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/z80/z80.c",
		MAME_DIR .. "src/emu/cpu/z80/z80daisy.c",
		MAME_DIR .. "src/emu/cpu/z80/tmpz84c011.c",
		MAME_DIR .. "src/emu/cpu/z80/tmpz84c015.c",
		MAME_DIR .. "src/emu/cpu/z80/kl5c80a12.c",
	}
end

if (CPUS["Z80"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/z80/z80dasm.c")
end

--------------------------------------------------
-- Sharp LR35902 (Game Boy CPU)
---@src/emu/cpu/lr35902/lr35902.h,CPUS += LR35902
--------------------------------------------------

if (CPUS["LR35902"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/lr35902/lr35902.c",
	}
end

if (CPUS["LR35902"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/lr35902/lr35902d.c")
end

--------------------------------------------------
-- Zilog Z180
---@src/emu/cpu/z180/z180.h,CPUS += Z180
--------------------------------------------------

if (CPUS["Z180"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/z180/z180.c",
		MAME_DIR .. "src/emu/cpu/z80/z80daisy.c",
	}
end

if (CPUS["Z180"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/z180/z180dasm.c")
end

--------------------------------------------------
-- Zilog Z8000
---@src/emu/cpu/z8000/z8000.h,CPUS += Z8000
--------------------------------------------------

if (CPUS["Z8000"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/z8000/z8000.c",
	}
end

if (CPUS["Z8000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/z8000/8000dasm.c")
end

--------------------------------------------------
-- Zilog Z8
---@src/emu/cpu/z8/z8.h,CPUS += Z8
--------------------------------------------------

if (CPUS["Z8"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/z8/z8.c",
	}
end

if (CPUS["Z8"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/z8/z8dasm.c")
end

--------------------------------------------------
-- Argonaut SuperFX
---@src/emu/cpu/superfx/superfx.h,CPUS += SUPERFX
--------------------------------------------------

if (CPUS["SUPERFX"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/superfx/superfx.c",
	}
end

if (CPUS["SUPERFX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/superfx/sfx_dasm.c")
end

--------------------------------------------------
-- Rockwell PPS-4
---@src/emu/cpu/pps4/pps4.h,CPUS += PPS4
--------------------------------------------------

if (CPUS["PPS4"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/pps4/pps4.c",
	}
end

if (CPUS["PPS4"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/pps4/pps4dasm.c")
end

--------------------------------------------------
-- Hitachi HD61700
---@src/emu/cpu/hd61700/hd61700.h,CPUS += HD61700
--------------------------------------------------

if (CPUS["HD61700"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/hd61700/hd61700.c",
	}
end

if (CPUS["HD61700"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/hd61700/hd61700d.c")
end

--------------------------------------------------
-- Sanyo LC8670
---@src/emu/cpu/lc8670/lc8670.h,CPUS += LC8670
--------------------------------------------------

if (CPUS["LC8670"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/lc8670/lc8670.c",
	}
end

if (CPUS["LC8670"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/lc8670/lc8670dsm.c")
end

--------------------------------------------------
-- Sega SCU DSP
---@src/emu/cpu/scudsp/scudsp.h,CPUS += SCUDSP
--------------------------------------------------

if (CPUS["SCUDSP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/scudsp/scudsp.c",
	}
end

if (CPUS["SCUDSP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/scudsp/scudspdasm.c")
end

--------------------------------------------------
-- Sunplus Technology S+core
---@src/emu/cpu/score/score.h,CPUS += SCORE
--------------------------------------------------

if (CPUS["SCORE"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/score/score.c",
	}
end

if (CPUS["SCORE"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/score/scoredsm.c")
end

--------------------------------------------------
-- Xerox Alto-II
---@src/emu/cpu/alto2/alto2cpu.h,CPUS += ALTO2
--------------------------------------------------

if (CPUS["ALTO2"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/alto2/alto2cpu.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2disk.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2disp.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2curt.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2dht.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2dvt.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2dwt.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2emu.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2ether.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2hw.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2kbd.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2ksec.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2kwd.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2mem.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2mouse.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2mrt.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2part.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2ram.c",
		MAME_DIR .. "src/emu/cpu/alto2/a2roms.c",
	}
end

if (CPUS["ALTO2"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/alto2/alto2dsm.c")
end

