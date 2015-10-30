-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   cpu.lua
--
--   Rules for building CPU cores
--
---------------------------------------------------------------------------

--------------------------------------------------
-- Shared code
--------------------------------------------------

files {
	MAME_DIR .. "src/devices/cpu/vtlb.c",
}

--------------------------------------------------
-- Dynamic recompiler objects
--------------------------------------------------

if (CPUS["SH2"]~=null or CPUS["MIPS"]~=null or CPUS["POWERPC"]~=null or CPUS["RSP"]~=null or CPUS["ARM7"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/drcbec.c",
		MAME_DIR .. "src/devices/cpu/drcbec.h",
		MAME_DIR .. "src/devices/cpu/drcbeut.c",
		MAME_DIR .. "src/devices/cpu/drcbeut.h",
		MAME_DIR .. "src/devices/cpu/drccache.c",
		MAME_DIR .. "src/devices/cpu/drccache.h",
		MAME_DIR .. "src/devices/cpu/drcfe.c",
		MAME_DIR .. "src/devices/cpu/drcfe.h",
		MAME_DIR .. "src/devices/cpu/drcuml.c",
		MAME_DIR .. "src/devices/cpu/drcuml.h",
		MAME_DIR .. "src/devices/cpu/uml.c",
		MAME_DIR .. "src/devices/cpu/uml.h",
		MAME_DIR .. "src/devices/cpu/i386/i386dasm.c",
		MAME_DIR .. "src/devices/cpu/x86log.c",
		MAME_DIR .. "src/devices/cpu/x86log.h",
		MAME_DIR .. "src/devices/cpu/drcbex86.c",
		MAME_DIR .. "src/devices/cpu/drcbex86.h",
		MAME_DIR .. "src/devices/cpu/drcbex64.c",
		MAME_DIR .. "src/devices/cpu/drcbex64.h",
		MAME_DIR .. "src/devices/cpu/drcumlsh.h",
		MAME_DIR .. "src/devices/cpu/vtlb.h",
		MAME_DIR .. "src/devices/cpu/x86emit.h",		
	}
end

--------------------------------------------------
-- Signetics 8X300 / Scientific Micro Systems SMS300
--@src/devices/cpu/8x300/8x300.h,CPUS["8X300"] = true
--------------------------------------------------

if (CPUS["8X300"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/8x300/8x300.c",
		MAME_DIR .. "src/devices/cpu/8x300/8x300.h",
	}
end

if (CPUS["8X300"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/8x300/8x300dasm.c")
end

--------------------------------------------------
-- ARCangent A4
--@src/devices/cpu/arc/arc.h,CPUS["ARC"] = true
--------------------------------------------------

if (CPUS["ARC"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/arc/arc.c",
		MAME_DIR .. "src/devices/cpu/arc/arc.h",
	}
end

if (CPUS["ARC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arc/arcdasm.c")
end

--------------------------------------------------
-- ARcompact (ARCtangent-A5, ARC 600, ARC 700)
--@src/devices/cpu/arc/arc.h,CPUS["ARCOMPACT"] = true
--------------------------------------------------

if (CPUS["ARCOMPACT"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact.c",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact.h",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute.c",
	}
	dependency {
		{ MAME_DIR .. "src/devices/cpu/arcompact/arcompact.c",  	   GEN_DIR .. "emu/cpu/arcompact/arcompact.inc" },
		{ MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute.c", GEN_DIR .. "emu/cpu/arcompact/arcompact.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/arcompact/arcompact_make.py" , GEN_DIR .. "emu/cpu/arcompact/arcompact.inc",   { MAME_DIR .. "src/devices/cpu/arcompact/arcompact_make.py" }, {"@echo Generating arcompact source .inc files...", PYTHON .. " $(1)  $(@)" }},
	}
end

if (CPUS["ARCOMPACT"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_dispatch.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompact_common.c")
end

--------------------------------------------------
-- Acorn ARM series
--
--@src/devices/cpu/arm/arm.h,CPUS["ARM"] = true
--@src/devices/cpu/arm7/arm7.h,CPUS["ARM7"] = true
--------------------------------------------------

if (CPUS["ARM"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/arm/arm.c",
		MAME_DIR .. "src/devices/cpu/arm/arm.h",
	}
end

if (CPUS["ARM"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arm/armdasm.c")
end

if (CPUS["ARM7"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/arm7/arm7.c",
		MAME_DIR .. "src/devices/cpu/arm7/arm7.h",
		MAME_DIR .. "src/devices/cpu/arm7/arm7thmb.c",
		MAME_DIR .. "src/devices/cpu/arm7/arm7ops.c",
		MAME_DIR .. "src/devices/cpu/arm7/lpc210x.c",
		MAME_DIR .. "src/devices/cpu/arm7/lpc210x.h",
	}
end

if (CPUS["ARM7"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arm7/arm7dasm.c")
end

--------------------------------------------------
-- Advanced Digital Chips SE3208
--@src/devices/cpu/se3208/se3208.h,CPUS["SE3208"] = true
--------------------------------------------------

if (CPUS["SE3208"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/se3208/se3208.c",
		MAME_DIR .. "src/devices/cpu/se3208/se3208.h",
	}
end

if (CPUS["SE3208"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/se3208/se3208dis.c")
end

--------------------------------------------------
-- American Microsystems, Inc.(AMI) S2000 series
--@src/devices/cpu/amis2000/amis2000.h,CPUS["AMIS2000"] = true
--------------------------------------------------

if (CPUS["AMIS2000"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/amis2000/amis2000.c",
		MAME_DIR .. "src/devices/cpu/amis2000/amis2000.h",
		MAME_DIR .. "src/devices/cpu/amis2000/amis2000op.c",
	}
end

if (CPUS["AMIS2000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/amis2000/amis2000d.c")
end

--------------------------------------------------
-- Alpha 8201
--@src/devices/cpu/alph8201/alph8201.h,CPUS["ALPHA8201"] = true
--------------------------------------------------

if (CPUS["ALPHA8201"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/alph8201/alph8201.c",
		MAME_DIR .. "src/devices/cpu/alph8201/alph8201.h",
	}
end

if (CPUS["ALPHA8201"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/alph8201/8201dasm.c")
end

--------------------------------------------------
-- Analog Devices ADSP21xx series
--@src/devices/cpu/adsp2100/adsp2100.h,CPUS["ADSP21XX"] = true
--------------------------------------------------

if (CPUS["ADSP21XX"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/adsp2100/adsp2100.c",
		MAME_DIR .. "src/devices/cpu/adsp2100/adsp2100.h",
	}
end

if (CPUS["ADSP21XX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/adsp2100/2100dasm.c")
end

--------------------------------------------------
-- Analog Devices "Sharc" ADSP21062
--@src/devices/cpu/sharc/sharc.h,CPUS["ADSP21062"] = true
--------------------------------------------------

if (CPUS["ADSP21062"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sharc/sharc.c",
		MAME_DIR .. "src/devices/cpu/sharc/sharc.h",
	}
end

if (CPUS["ADSP21062"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sharc/sharcdsm.c")
end

--------------------------------------------------
-- APEXC
--@src/devices/cpu/apexc/apexc.h,CPUS["APEXC"] = true
--------------------------------------------------

if (CPUS["APEXC"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/apexc/apexc.c",
		MAME_DIR .. "src/devices/cpu/apexc/apexc.h",
	}
end

if (CPUS["APEXC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/apexc/apexcdsm.c")
end

--------------------------------------------------
-- AT&T DSP16A
--@src/devices/cpu/dsp16/dsp16.h,CPUS["DSP16A"] = true
--------------------------------------------------

if (CPUS["DSP16A"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16.c",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16.h",
	}
end

if (CPUS["DSP16A"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp16/dsp16dis.c")
end

--------------------------------------------------
-- AT&T DSP32C
--@src/devices/cpu/dsp32/dsp32.h,CPUS["DSP32C"] = true
--------------------------------------------------

if (CPUS["DSP32C"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/dsp32/dsp32.c",
		MAME_DIR .. "src/devices/cpu/dsp32/dsp32.h",
	}
end

if (CPUS["DSP32C"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp32/dsp32dis.c")
end

--------------------------------------------------
-- Atari custom RISC processor
--@src/devices/cpu/asap/asap.h,CPUS["ASAP"] = true
--------------------------------------------------

if (CPUS["ASAP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/asap/asap.c",
		MAME_DIR .. "src/devices/cpu/asap/asap.h",
	}
end

if (CPUS["ASAP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/asap/asapdasm.c")
end

--------------------------------------------------
-- AMD Am29000
--@src/devices/cpu/am29000/am29000.h,CPUS["AM29000"] = true
--------------------------------------------------

if (CPUS["AM29000"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/am29000/am29000.c",
		MAME_DIR .. "src/devices/cpu/am29000/am29000.h",
	}
end

if (CPUS["AM29000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/am29000/am29dasm.c")
end

--------------------------------------------------
-- Atari Jaguar custom DSPs
--@src/devices/cpu/jaguar/jaguar.h,CPUS["JAGUAR"] = true
--------------------------------------------------

if (CPUS["JAGUAR"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/jaguar/jaguar.c",
		MAME_DIR .. "src/devices/cpu/jaguar/jaguar.h",
	}
end

if (CPUS["JAGUAR"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/jaguar/jagdasm.c")
end

--------------------------------------------------
-- Simutrek Cube Quest bit-sliced CPUs
--@src/devices/cpu/cubeqcpu/cubeqcpu.h,CPUS["CUBEQCPU"] = true
--------------------------------------------------

if (CPUS["CUBEQCPU"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/cubeqcpu/cubeqcpu.c",
		MAME_DIR .. "src/devices/cpu/cubeqcpu/cubeqcpu.h",
	}
end

if (CPUS["CUBEQCPU"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cubeqcpu/cubedasm.c")
end

--------------------------------------------------
-- Ensoniq ES5510 ('ESP') DSP
--@src/devices/cpu/es5510/es5510.h,CPUS["ES5510"] = true
--------------------------------------------------

if (CPUS["ES5510"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/es5510/es5510.c",
		MAME_DIR .. "src/devices/cpu/es5510/es5510.h",
	}
end

--------------------------------------------------
-- Entertainment Sciences AM29116-based RIP
--@src/devices/cpu/esrip/esrip.h,CPUS["ESRIP"] = true
--------------------------------------------------

if (CPUS["ESRIP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/esrip/esrip.c",
		MAME_DIR .. "src/devices/cpu/esrip/esrip.h",
	}
end

if (CPUS["ESRIP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/esrip/esripdsm.c")
end

--------------------------------------------------
-- Seiko Epson E0C6200 series
--@src/devices/cpu/e0c6200/e0c6200.h,CPUS["E0C6200"] = true
--------------------------------------------------

if (CPUS["E0C6200"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200.c",
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200.h",
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6s46.c",
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6s46.h",
	}
end

if (CPUS["E0C6200"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200d.c")
end

--------------------------------------------------
-- RCA COSMAC
--@src/devices/cpu/cosmac/cosmac.h,CPUS["COSMAC"] = true
--------------------------------------------------

if (CPUS["COSMAC"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/cosmac/cosmac.c",
		MAME_DIR .. "src/devices/cpu/cosmac/cosmac.h",
	}
end

if (CPUS["COSMAC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cosmac/cosdasm.c")
end

--------------------------------------------------
-- National Semiconductor COP400 family
--@src/devices/cpu/cop400/cop400.h,CPUS["COP400"] = true
--------------------------------------------------

if (CPUS["COP400"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/cop400/cop400.c",
		MAME_DIR .. "src/devices/cpu/cop400/cop400.h",
	}
end

if (CPUS["COP400"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop410ds.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop420ds.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop440ds.c")
end

--------------------------------------------------
-- CP1610
--@src/devices/cpu/cp1610/cp1610.h,CPUS["CP1610"] = true
--------------------------------------------------

if (CPUS["CP1610"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/cp1610/cp1610.c",
		MAME_DIR .. "src/devices/cpu/cp1610/cp1610.h",
	}
end

if (CPUS["CP1610"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cp1610/1610dasm.c")
end

--------------------------------------------------
-- Cinematronics vector "CPU"
--@src/devices/cpu/ccpu/ccpu.h,CPUS["CCPU"] = true
--------------------------------------------------

if (CPUS["CCPU"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/ccpu/ccpu.c",
		MAME_DIR .. "src/devices/cpu/ccpu/ccpu.h",
	}
end

if (CPUS["CCPU"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ccpu/ccpudasm.c")
end

--------------------------------------------------
-- DEC T-11
--@src/devices/cpu/t11/t11.h,CPUS["T11"] = true
--------------------------------------------------

if (CPUS["T11"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/t11/t11.c",
		MAME_DIR .. "src/devices/cpu/t11/t11.h",
	}
end

if (CPUS["T11"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/t11/t11dasm.c")
end

--------------------------------------------------
-- DEC PDP-8
--@src/devices/cpu/pdp8/pdp8.h,CPUS["PDP8"] = true
--------------------------------------------------

if (CPUS["PDP8"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/pdp8/pdp8.c",
		MAME_DIR .. "src/devices/cpu/pdp8/pdp8.h",
	}
end

if (CPUS["PDP8"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pdp8/pdp8dasm.c")
end

--------------------------------------------------
-- F8
--@src/devices/cpu/f8/f8.h,CPUS["F8"] = true
--------------------------------------------------

if (CPUS["F8"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/f8/f8.c",
		MAME_DIR .. "src/devices/cpu/f8/f8.h",
	}
end

if (CPUS["F8"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/f8/f8dasm.c")
end

--------------------------------------------------
-- G65816
--@src/devices/cpu/g65816/g65816.h,CPUS["G65816"] = true
--------------------------------------------------

if (CPUS["G65816"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/g65816/g65816.c",
		MAME_DIR .. "src/devices/cpu/g65816/g65816.h",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o0.c",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o1.c",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o2.c",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o3.c",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o4.c",
	}
end

if (CPUS["G65816"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/g65816/g65816ds.c")
end

--------------------------------------------------
-- Hitachi H8 (16/32-bit H8/300, H8/300H, H8S2000 and H8S2600 series)
--@src/devices/cpu/h8/h8.h,CPUS["H8"] = true
--------------------------------------------------

if (CPUS["H8"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/h8/h8.c",
		MAME_DIR .. "src/devices/cpu/h8/h8.h",
		MAME_DIR .. "src/devices/cpu/h8/h8h.c",
		MAME_DIR .. "src/devices/cpu/h8/h8h.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2000.c",
		MAME_DIR .. "src/devices/cpu/h8/h8s2000.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2600.c",
		MAME_DIR .. "src/devices/cpu/h8/h8s2600.h",
		MAME_DIR .. "src/devices/cpu/h8/h83337.c",
		MAME_DIR .. "src/devices/cpu/h8/h83337.h",
		MAME_DIR .. "src/devices/cpu/h8/h83002.c",
		MAME_DIR .. "src/devices/cpu/h8/h83002.h",
		MAME_DIR .. "src/devices/cpu/h8/h83006.c",
		MAME_DIR .. "src/devices/cpu/h8/h83006.h",
		MAME_DIR .. "src/devices/cpu/h8/h83008.c",
		MAME_DIR .. "src/devices/cpu/h8/h83008.h",
		MAME_DIR .. "src/devices/cpu/h8/h83048.c",
		MAME_DIR .. "src/devices/cpu/h8/h83048.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2245.c",
		MAME_DIR .. "src/devices/cpu/h8/h8s2245.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2320.c",
		MAME_DIR .. "src/devices/cpu/h8/h8s2320.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2357.c",
		MAME_DIR .. "src/devices/cpu/h8/h8s2357.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2655.c",
		MAME_DIR .. "src/devices/cpu/h8/h8s2655.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_adc.c",
		MAME_DIR .. "src/devices/cpu/h8/h8_adc.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_port.c",
		MAME_DIR .. "src/devices/cpu/h8/h8_port.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_intc.c",
		MAME_DIR .. "src/devices/cpu/h8/h8_intc.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer8.c",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer8.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer16.c",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer16.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_sci.c",
		MAME_DIR .. "src/devices/cpu/h8/h8_sci.h",
	}
	
	dependency {
		{ MAME_DIR .. "src/devices/cpu/h8/h8.c",       GEN_DIR .. "emu/cpu/h8/h8.inc" },
		{ MAME_DIR .. "src/devices/cpu/h8/h8h.c",      GEN_DIR .. "emu/cpu/h8/h8h.inc" },
		{ MAME_DIR .. "src/devices/cpu/h8/h8s2000.c",  GEN_DIR .. "emu/cpu/h8/h8s2000.inc" },
		{ MAME_DIR .. "src/devices/cpu/h8/h8s2600.c",  GEN_DIR .. "emu/cpu/h8/h8s2600.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8.inc",       { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8-300 source file...",   PYTHON .. " $(1) $(<) o   $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8h.inc",      { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8-300H source file...",  PYTHON .. " $(1) $(<) h   $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8s2000.inc",  { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8S/2000 source file...", PYTHON .. " $(1) $(<) s20 $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8s2600.inc",  { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8S/2600 source file...", PYTHON .. " $(1) $(<) s26 $(@)" }},
	}
end

--------------------------------------------------
-- Hitachi HCD62121
--@src/devices/cpu/hcd62121/hcd62121.h,CPUS["HCD62121"] = true
--------------------------------------------------

if (CPUS["HCD62121"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/hcd62121/hcd62121.c",
		MAME_DIR .. "src/devices/cpu/hcd62121/hcd62121.h",
	}
end

if (CPUS["HCD62121"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hcd62121/hcd62121d.c")
end

--------------------------------------------------
-- Hitachi HMCS40 series
--@src/devices/cpu/hmcs40/hmcs40.h,CPUS["HMCS40"] = true
--------------------------------------------------

if (CPUS["HMCS40"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40.c",
		MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40.h",
		MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40op.c",
	}
end

if (CPUS["HMCS40"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40d.c")
end

--------------------------------------------------
-- Hitachi SH1/SH2
--@src/devices/cpu/sh2/sh2.h,CPUS["SH2"] = true
--------------------------------------------------

if (CPUS["SH2"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sh2/sh2.c",
		MAME_DIR .. "src/devices/cpu/sh2/sh2.h",
		MAME_DIR .. "src/devices/cpu/sh2/sh2fe.c",
	}
end

if (CPUS["SH2"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sh2/sh2dasm.c")
end

--------------------------------------------------
-- Hitachi SH4
--@src/devices/cpu/sh4/sh4.h,CPUS["SH4"] = true
--------------------------------------------------

if (CPUS["SH4"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sh4/sh4.c",
		MAME_DIR .. "src/devices/cpu/sh4/sh4.h",
		MAME_DIR .. "src/devices/cpu/sh4/sh4comn.c",
		MAME_DIR .. "src/devices/cpu/sh4/sh4comn.h",
		MAME_DIR .. "src/devices/cpu/sh4/sh3comn.c",
		MAME_DIR .. "src/devices/cpu/sh4/sh3comn.h",
		MAME_DIR .. "src/devices/cpu/sh4/sh4tmu.c",
		MAME_DIR .. "src/devices/cpu/sh4/sh4tmu.h",
		MAME_DIR .. "src/devices/cpu/sh4/sh4dmac.c",
		MAME_DIR .. "src/devices/cpu/sh4/sh4dmac.h",
	}
end

if (CPUS["SH4"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sh4/sh4dasm.c")
end

--------------------------------------------------
-- HP Hybrid processor
--@src/devices/cpu/hphybrid/hphybrid.h,CPUS["HPHYBRID"] = true
--------------------------------------------------

if (CPUS["HPHYBRID"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/hphybrid/hphybrid.c",
		MAME_DIR .. "src/devices/cpu/hphybrid/hphybrid.h",
	}
end

if (CPUS["HPHYBRID"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hphybrid/hphybrid_dasm.c")
end

--------------------------------------------------
-- Hudsonsoft 6280
--@src/devices/cpu/h6280/h6280.h,CPUS["H6280"] = true
--------------------------------------------------

if (CPUS["H6280"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/h6280/h6280.c",
		MAME_DIR .. "src/devices/cpu/h6280/h6280.h",
	}
end

if (CPUS["H6280"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/h6280/6280dasm.c")
end

--------------------------------------------------
-- Hyperstone E1 series
--@src/devices/cpu/e132xs/e132xs.h,CPUS["E1"] = true
--------------------------------------------------

if (CPUS["E1"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/e132xs/e132xs.c",
		MAME_DIR .. "src/devices/cpu/e132xs/e132xs.h",
	}
end

if (CPUS["E1"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/e132xs/32xsdasm.c")
end

--------------------------------------------------
-- 15IE-00-013 CPU ("Microprogrammed Control Device")
--@src/devices/cpu/ie15/ie15.h,CPUS["IE15"] = true
--------------------------------------------------

if (CPUS["IE15"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/ie15/ie15.c",
		MAME_DIR .. "src/devices/cpu/ie15/ie15.h",
	}
end

if (CPUS["IE15"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ie15/ie15dasm.c")
end

--------------------------------------------------
-- Intel 4004
--@src/devices/cpu/i4004/i4004.h,CPUS["I4004"] = true
--------------------------------------------------

if (CPUS["I4004"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i4004/i4004.c",
		MAME_DIR .. "src/devices/cpu/i4004/i4004.h",
	}
end

if (CPUS["I4004"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i4004/4004dasm.c")
end

--------------------------------------------------
-- Intel 8008
--@src/devices/cpu/i8008/i8008.h,CPUS["I8008"] = true
--------------------------------------------------

if (CPUS["I8008"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i8008/i8008.c",
		MAME_DIR .. "src/devices/cpu/i8008/i8008.h",
	}
end

if (CPUS["I8008"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8008/8008dasm.c")
end

--------------------------------------------------
--  National Semiconductor SC/MP
--@src/devices/cpu/scmp/scmp.h,CPUS["SCMP"] = true
--------------------------------------------------

if (CPUS["SCMP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/scmp/scmp.c",
		MAME_DIR .. "src/devices/cpu/scmp/scmp.h",
	}
end

if (CPUS["SCMP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/scmp/scmpdasm.c")
end

--------------------------------------------------
-- Intel 8080/8085A
--@src/devices/cpu/i8085/i8085.h,CPUS["I8085"] = true
--------------------------------------------------

if (CPUS["I8085"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i8085/i8085.c",
		MAME_DIR .. "src/devices/cpu/i8085/i8085.h",
	}
end

if (CPUS["I8085"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8085/8085dasm.c")
end

--------------------------------------------------
-- Intel 8089
--@src/devices/cpu/i8089/i8089.h,CPUS["I8089"] = true
--------------------------------------------------

if (CPUS["I8089"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i8089/i8089.c",
		MAME_DIR .. "src/devices/cpu/i8089/i8089.h",
		MAME_DIR .. "src/devices/cpu/i8089/i8089_channel.c",
		MAME_DIR .. "src/devices/cpu/i8089/i8089_channel.h",
		MAME_DIR .. "src/devices/cpu/i8089/i8089_ops.c",
	}
end

if (CPUS["I8089"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8089/i8089_dasm.c")
end

--------------------------------------------------
-- Intel MCS-48 (8039 and derivatives)
--@src/devices/cpu/mcs48/mcs48.h,CPUS["MCS48"] = true
--------------------------------------------------

if (CPUS["MCS48"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mcs48/mcs48.c",
		MAME_DIR .. "src/devices/cpu/mcs48/mcs48.h",
	}
end

if (CPUS["MCS48"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mcs48/mcs48dsm.c")
end

--------------------------------------------------
-- Intel 8051 and derivatives
--@src/devices/cpu/mcs51/mcs51.h,CPUS["MCS51"] = true
--------------------------------------------------

if (CPUS["MCS51"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mcs51/mcs51.c",
		MAME_DIR .. "src/devices/cpu/mcs51/mcs51.h",
	}
end

if (CPUS["MCS51"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mcs51/mcs51dasm.c")
end

--------------------------------------------------
-- Intel MCS-96
--@src/devices/cpu/mcs96/mcs96.h,CPUS["MCS96"] = true
--------------------------------------------------

if (CPUS["MCS96"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mcs96/mcs96.c",
		MAME_DIR .. "src/devices/cpu/mcs96/mcs96.h",
		MAME_DIR .. "src/devices/cpu/mcs96/i8x9x.c",
		MAME_DIR .. "src/devices/cpu/mcs96/i8x9x.h",
		MAME_DIR .. "src/devices/cpu/mcs96/i8xc196.c",
		MAME_DIR .. "src/devices/cpu/mcs96/i8xc196.h",
	}
	dependency {
		{ MAME_DIR .. "src/devices/cpu/mcs96/mcs96.c",   GEN_DIR .. "emu/cpu/mcs96/mcs96.inc" },
		{ MAME_DIR .. "src/devices/cpu/mcs96/i8x9x.c",   GEN_DIR .. "emu/cpu/mcs96/i8x9x.inc" },
		{ MAME_DIR .. "src/devices/cpu/mcs96/i8xc196.c", GEN_DIR .. "emu/cpu/mcs96/i8xc196.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/mcs96/mcs96ops.lst" , GEN_DIR .. "emu/cpu/mcs96/mcs96.inc",   { MAME_DIR .. "src/devices/cpu/mcs96/mcs96make.py" }, {"@echo Generating mcs96 source file...", PYTHON .. " $(1) mcs96 $(<) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/mcs96/mcs96ops.lst" , GEN_DIR .. "emu/cpu/mcs96/i8x9x.inc",   { MAME_DIR .. "src/devices/cpu/mcs96/mcs96make.py" }, {"@echo Generating i8x9x source file...", PYTHON .. " $(1) i8x9x $(<) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/mcs96/mcs96ops.lst" , GEN_DIR .. "emu/cpu/mcs96/i8xc196.inc", { MAME_DIR .. "src/devices/cpu/mcs96/mcs96make.py" }, {"@echo Generating i8xc196 source file...", PYTHON .. " $(1) i8xc196 $(<) $(@)" }},
	}
end

--------------------------------------------------
-- Intel 80x86 series
--@src/devices/cpu/i86/i86.h,CPUS["I86"] = true
--@src/devices/cpu/i86/i286.h,CPUS["I86"] = true
--@src/devices/cpu/i386/i386.h,CPUS["I386"] = true
--------------------------------------------------

if (CPUS["I86"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i86/i86.c",
		MAME_DIR .. "src/devices/cpu/i86/i86.h",
		MAME_DIR .. "src/devices/cpu/i86/i186.c",
		MAME_DIR .. "src/devices/cpu/i86/i186.h",
		MAME_DIR .. "src/devices/cpu/i86/i286.c",
		MAME_DIR .. "src/devices/cpu/i86/i286.h",
	}
end

if (CPUS["I86"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i386/i386dasm.c")
end

if (CPUS["I386"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i386/i386.c",
		MAME_DIR .. "src/devices/cpu/i386/i386.h",
	}
end

if (CPUS["I386"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i386/i386dasm.c")
end

--------------------------------------------------
-- Intel i860
--@src/devices/cpu/i860/i860.h,CPUS["I860"] = true
--------------------------------------------------

if (CPUS["I860"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i860/i860.c",
		MAME_DIR .. "src/devices/cpu/i860/i860.h",
	}
end

if (CPUS["I860"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i860/i860dis.c")
end

--------------------------------------------------
-- Intel i960
--@src/devices/cpu/i960/i960.h,CPUS["I960"] = true
--------------------------------------------------

if (CPUS["I960"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i960/i960.c",
		MAME_DIR .. "src/devices/cpu/i960/i960.h",
	}
end

if (CPUS["I960"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i960/i960dis.c")
end

--------------------------------------------------
-- LH5801
--@src/devices/cpu/lh5801/lh5801.h,CPUS["LH5801"] = true
--------------------------------------------------

if (CPUS["LH5801"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/lh5801/lh5801.c",
		MAME_DIR .. "src/devices/cpu/lh5801/lh5801.h",
	}
end

if (CPUS["LH5801"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lh5801/5801dasm.c")
end
--------
------------------------------------------
-- Manchester Small-Scale Experimental Machine
--@src/devices/cpu/ssem/ssem.h,CPUS["SSEM"] = true
--------------------------------------------------

if (CPUS["SSEM"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/ssem/ssem.c",
		MAME_DIR .. "src/devices/cpu/ssem/ssem.h",
	}
end

if (CPUS["SSEM"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ssem/ssemdasm.c")
end

--------------------------------------------------
-- Fujitsu MB88xx
--@src/devices/cpu/mb88xx/mb88xx.h,CPUS["MB88XX"] = true
--------------------------------------------------

if (CPUS["MB88XX"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mb88xx/mb88xx.c",
		MAME_DIR .. "src/devices/cpu/mb88xx/mb88xx.h",
	}
end

if (CPUS["MB88XX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb88xx/mb88dasm.c")
end

--------------------------------------------------
-- Fujitsu MB86233
--@src/devices/cpu/mb86233/mb86233.h,CPUS["MB86233"] = true
--------------------------------------------------

if (CPUS["MB86233"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mb86233/mb86233.c",
		MAME_DIR .. "src/devices/cpu/mb86233/mb86233.h",
	}
end

if (CPUS["MB86233"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb86233/mb86233d.c")
end

--------------------------------------------------
-- Fujitsu MB86235
--@src/devices/cpu/mb86233/mb86235.h,CPUS["MB86235"] = true
--------------------------------------------------

if (CPUS["MB86235"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mb86235/mb86235.c",
		MAME_DIR .. "src/devices/cpu/mb86235/mb86235.h",
	}
end

if (CPUS["MB86235"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb86235/mb86235d.c")
end

--------------------------------------------------
-- Microchip PIC16C5x
--@src/devices/cpu/pic16c5x/pic16c5x.h,CPUS["PIC16C5X"] = true
--------------------------------------------------

if (CPUS["PIC16C5X"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/pic16c5x/pic16c5x.c",
		MAME_DIR .. "src/devices/cpu/pic16c5x/pic16c5x.h",
	}
end

if (CPUS["PIC16C5X"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16c5x/16c5xdsm.c")
end

--------------------------------------------------
-- Microchip PIC16C62x
--@src/devices/cpu/pic16c62x/pic16c62x.h,CPUS["PIC16C62X"] = true
--------------------------------------------------

if (CPUS["PIC16C62X"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/pic16c62x/pic16c62x.c",
		MAME_DIR .. "src/devices/cpu/pic16c62x/pic16c62x.h",
	}
end

if (CPUS["PIC16C62X"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16c62x/16c62xdsm.c")
end

--------------------------------------------------
-- MIPS R3000 (MIPS I/II) series
-- MIPS R4000 (MIPS III/IV) series
--@src/devices/cpu/mips/mips3.h,CPUS["MIPS"] = true
--------------------------------------------------

if (CPUS["MIPS"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mips/r3000.c",
		MAME_DIR .. "src/devices/cpu/mips/r3000.h",
		MAME_DIR .. "src/devices/cpu/mips/mips3com.c",
		MAME_DIR .. "src/devices/cpu/mips/mips3com.h",
		MAME_DIR .. "src/devices/cpu/mips/mips3.c",
		MAME_DIR .. "src/devices/cpu/mips/mips3.h",
		MAME_DIR .. "src/devices/cpu/mips/mips3fe.c",
		MAME_DIR .. "src/devices/cpu/mips/mips3fe.h",
		MAME_DIR .. "src/devices/cpu/mips/mips3drc.c",
	}
end

if (CPUS["MIPS"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mips/r3kdasm.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mips/mips3dsm.c")
end

--------------------------------------------------
-- Sony PlayStation CPU (R3000-based + GTE)
--@src/devices/cpu/psx/psx.h,CPUS["PSX"] = true
--------------------------------------------------

if (CPUS["PSX"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/psx/psx.c",
		MAME_DIR .. "src/devices/cpu/psx/psx.h",
		MAME_DIR .. "src/devices/cpu/psx/gte.c",
		MAME_DIR .. "src/devices/cpu/psx/gte.h",
		MAME_DIR .. "src/devices/cpu/psx/dma.c",
		MAME_DIR .. "src/devices/cpu/psx/dma.h",
		MAME_DIR .. "src/devices/cpu/psx/irq.c",
		MAME_DIR .. "src/devices/cpu/psx/irq.h",
		MAME_DIR .. "src/devices/cpu/psx/mdec.c",
		MAME_DIR .. "src/devices/cpu/psx/mdec.h",
		MAME_DIR .. "src/devices/cpu/psx/rcnt.c",
		MAME_DIR .. "src/devices/cpu/psx/rcnt.h",
		MAME_DIR .. "src/devices/cpu/psx/sio.c",
		MAME_DIR .. "src/devices/cpu/psx/sio.h",
	}
end

if (CPUS["PSX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/psx/psxdasm.c")
end

--------------------------------------------------
-- Mitsubishi MELPS 4 series
--@src/devices/cpu/melps4/melps4.h,CPUS["MELPS4"] = true
--------------------------------------------------

if (CPUS["MELPS4"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/melps4/melps4.c",
		MAME_DIR .. "src/devices/cpu/melps4/melps4.h",
		MAME_DIR .. "src/devices/cpu/melps4/melps4op.c",
		MAME_DIR .. "src/devices/cpu/melps4/m58846.c",
		MAME_DIR .. "src/devices/cpu/melps4/m58846.h",
	}
end

if (CPUS["MELPS4"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/melps4/melps4d.c")
end

--------------------------------------------------
-- Mitsubishi M37702 and M37710 (based on 65C816)
--@src/devices/cpu/m37710/m37710.h,CPUS["M37710"] = true
--------------------------------------------------

if (CPUS["M37710"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/m37710/m37710.c",
		MAME_DIR .. "src/devices/cpu/m37710/m37710.h",
		MAME_DIR .. "src/devices/cpu/m37710/m37710o0.c",
		MAME_DIR .. "src/devices/cpu/m37710/m37710o1.c",
		MAME_DIR .. "src/devices/cpu/m37710/m37710o2.c",
		MAME_DIR .. "src/devices/cpu/m37710/m37710o3.c",
	}
end

if (CPUS["M37710"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m37710/m7700ds.c")
end

--------------------------------------------------
-- Mostek 6502 and its many derivatives
--@src/devices/cpu/m6502/m6502.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/deco16.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m4510.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m65ce02.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m65c02.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/r65c02.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m65sc02.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6504.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6507.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6509.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6510.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6510t.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m7501.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m8502.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/n2a03.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m740.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m3745x.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m5074x.h,CPUS["M6502"] = true

--------------------------------------------------

if (CPUS["M6502"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/m6502/deco16.c",
		MAME_DIR .. "src/devices/cpu/m6502/deco16.h",
		MAME_DIR .. "src/devices/cpu/m6502/m4510.c",
		MAME_DIR .. "src/devices/cpu/m6502/m4510.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6502.c",
		MAME_DIR .. "src/devices/cpu/m6502/m6502.h",
		MAME_DIR .. "src/devices/cpu/m6502/m65c02.c",
		MAME_DIR .. "src/devices/cpu/m6502/m65c02.h",
		MAME_DIR .. "src/devices/cpu/m6502/m65ce02.c",
		MAME_DIR .. "src/devices/cpu/m6502/m65ce02.h",
		MAME_DIR .. "src/devices/cpu/m6502/m65sc02.c",
		MAME_DIR .. "src/devices/cpu/m6502/m65sc02.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6504.c",
		MAME_DIR .. "src/devices/cpu/m6502/m6504.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6507.c",
		MAME_DIR .. "src/devices/cpu/m6502/m6507.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6509.c",
		MAME_DIR .. "src/devices/cpu/m6502/m6509.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6510.c",
		MAME_DIR .. "src/devices/cpu/m6502/m6510.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6510t.c",
		MAME_DIR .. "src/devices/cpu/m6502/m6510t.h",
		MAME_DIR .. "src/devices/cpu/m6502/m7501.c",
		MAME_DIR .. "src/devices/cpu/m6502/m7501.h",
		MAME_DIR .. "src/devices/cpu/m6502/m8502.c",
		MAME_DIR .. "src/devices/cpu/m6502/m8502.h",
		MAME_DIR .. "src/devices/cpu/m6502/n2a03.c",
		MAME_DIR .. "src/devices/cpu/m6502/n2a03.h",
		MAME_DIR .. "src/devices/cpu/m6502/r65c02.c",
		MAME_DIR .. "src/devices/cpu/m6502/r65c02.h",
		MAME_DIR .. "src/devices/cpu/m6502/m740.c",
		MAME_DIR .. "src/devices/cpu/m6502/m740.h",
		MAME_DIR .. "src/devices/cpu/m6502/m3745x.c",
		MAME_DIR .. "src/devices/cpu/m6502/m3745x.h",
		MAME_DIR .. "src/devices/cpu/m6502/m5074x.c",
		MAME_DIR .. "src/devices/cpu/m6502/m5074x.h",
	}
	
	dependency {
		{ MAME_DIR .. "src/devices/cpu/m6502/deco16.c",   GEN_DIR .. "emu/cpu/m6502/deco16.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m4510.c",    GEN_DIR .. "emu/cpu/m6502/m4510.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m6502.c",    GEN_DIR .. "emu/cpu/m6502/m6502.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m65c02.c",   GEN_DIR .. "emu/cpu/m6502/m65c02.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m65ce02.c",  GEN_DIR .. "emu/cpu/m6502/m65ce02.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m6509.c",    GEN_DIR .. "emu/cpu/m6502/m6509.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m6510.c",    GEN_DIR .. "emu/cpu/m6502/m6510.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/n2a03.c",    GEN_DIR .. "emu/cpu/m6502/n2a03.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/r65c02.c",   GEN_DIR .. "emu/cpu/m6502/r65c02.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m740.c",     GEN_DIR .. "emu/cpu/m6502/m740.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/m6502/odeco16.lst", GEN_DIR .. "emu/cpu/m6502/deco16.inc", { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/ddeco16.lst"  }, {"@echo Generating deco16 source file...", PYTHON .. " $(1) deco16_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om4510.lst",  GEN_DIR .. "emu/cpu/m6502/m4510.inc",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm4510.lst"   }, {"@echo Generating m4510 source file...", PYTHON .. " $(1) m4510_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om6502.lst",  GEN_DIR .. "emu/cpu/m6502/m6502.inc",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm6502.lst"   }, {"@echo Generating m6502 source file...", PYTHON .. " $(1) m6502_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om65c02.lst", GEN_DIR .. "emu/cpu/m6502/m65c02.inc", { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm65c02.lst"  }, {"@echo Generating m65c02 source file...", PYTHON .. " $(1) m65c02_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om65ce02.lst",GEN_DIR .. "emu/cpu/m6502/m65ce02.inc",{ MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm65ce02.lst" }, {"@echo Generating m65ce02 source file...", PYTHON .. " $(1) m65ce02_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om6509.lst",  GEN_DIR .. "emu/cpu/m6502/m6509.inc",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm6509.lst"   }, {"@echo Generating m6509 source file...", PYTHON .. " $(1) m6509_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om6510.lst",  GEN_DIR .. "emu/cpu/m6502/m6510.inc",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm6510.lst"   }, {"@echo Generating m6510 source file...", PYTHON .. " $(1) m6510_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/on2a03.lst",  GEN_DIR .. "emu/cpu/m6502/n2a03.inc",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dn2a03.lst"   }, {"@echo Generating n2a03 source file...", PYTHON .. " $(1) n2a03_device $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om740.lst" ,  GEN_DIR .. "emu/cpu/m6502/m740.inc",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm740.lst"    }, {"@echo Generating m740 source file...", PYTHON .. " $(1) m740_device $(<) $(2) $(@)" }},

		{ MAME_DIR .. "src/devices/cpu/m6502/dr65c02.lst", GEN_DIR .. "emu/cpu/m6502/r65c02.inc", { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py" }, {"@echo Generating r65c02 source file...", PYTHON .. " $(1) r65c02_device - $(<) $(@)" }},
	}
end

--------------------------------------------------
-- Motorola 680x
--@src/devices/cpu/m6800/m6800.h,CPUS["M6800"] = true
--------------------------------------------------

if (CPUS["M6800"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/m6800/m6800.c",
		MAME_DIR .. "src/devices/cpu/m6800/m6800.h",
	}
end

if (CPUS["M6800"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6800/6800dasm.c")
end

--------------------------------------------------
-- Motorola 6805
--@src/devices/cpu/m6805/m6805.h,CPUS["M6805"] = true
--------------------------------------------------

if (CPUS["M6805"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/m6805/m6805.c",
		MAME_DIR .. "src/devices/cpu/m6805/m6805.h",
	}
end

if (CPUS["M6805"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6805/6805dasm.c")
end

--------------------------------------------------
-- Motorola 6809
--@src/devices/cpu/m6809/m6809.h,CPUS["M6809"] = true
--@src/devices/cpu/m6809/hd6309.h,CPUS["M6809"] = true
--@src/devices/cpu/m6809/konami.h,CPUS["M6809"] = true
--------------------------------------------------

if (CPUS["M6809"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/m6809/m6809.c",
		MAME_DIR .. "src/devices/cpu/m6809/m6809.h",
		MAME_DIR .. "src/devices/cpu/m6809/hd6309.c",
		MAME_DIR .. "src/devices/cpu/m6809/hd6309.h",
		MAME_DIR .. "src/devices/cpu/m6809/konami.c",
		MAME_DIR .. "src/devices/cpu/m6809/konami.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/cpu/m6809/m6809.c",   GEN_DIR .. "emu/cpu/m6809/m6809.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6809/hd6309.c",  GEN_DIR .. "emu/cpu/m6809/hd6309.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6809/konami.c",  GEN_DIR .. "emu/cpu/m6809/konami.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/m6809/m6809.ops"  , GEN_DIR .. "emu/cpu/m6809/m6809.inc",   { MAME_DIR .. "src/devices/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/devices/cpu/m6809/base6x09.ops"  }, {"@echo Generating m6809 source file...", PYTHON .. " $(1) $(<) > $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6809/hd6309.ops" , GEN_DIR .. "emu/cpu/m6809/hd6309.inc",  { MAME_DIR .. "src/devices/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/devices/cpu/m6809/base6x09.ops"  }, {"@echo Generating hd6309 source file...", PYTHON .. " $(1) $(<) > $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6809/konami.ops" , GEN_DIR .. "emu/cpu/m6809/konami.inc",  { MAME_DIR .. "src/devices/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/devices/cpu/m6809/base6x09.ops"  }, {"@echo Generating konami source file...", PYTHON .. " $(1) $(<) > $(@)" }},
	}
end

if (CPUS["M6809"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6809/6809dasm.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6809/6309dasm.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6809/knmidasm.c")
end

--------------------------------------------------
-- Motorola 68HC11
--@src/devices/cpu/mc68hc11/mc68hc11.h,CPUS["MC68HC11"] = true
--------------------------------------------------

if (CPUS["MC68HC11"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mc68hc11/mc68hc11.c",
		MAME_DIR .. "src/devices/cpu/mc68hc11/mc68hc11.h",
	}
end

if (CPUS["MC68HC11"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mc68hc11/hc11dasm.c")
end

--------------------------------------------------
-- Motorola 68000 series
--@src/devices/cpu/m68000/m68000.h,CPUS["M680X0"] = true
--------------------------------------------------

if (CPUS["M680X0"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/m68000/m68kcpu.c",
		MAME_DIR .. "src/devices/cpu/m68000/m68kcpu.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68kops.c",
		MAME_DIR .. "src/devices/cpu/m68000/m68kops.h",
	}
end

if (CPUS["M680X0"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m68000/m68kdasm.c")
end

--------------------------------------------------
-- Motorola/Freescale dsp56k
--@src/devices/cpu/dsp56k/dsp56k.h,CPUS["DSP56156"] = true
--------------------------------------------------

if (CPUS["DSP56156"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56k.c",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56k.h",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56mem.c",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56mem.h",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56pcu.c",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56pcu.h",
	}
end

if (CPUS["DSP56156"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56k/dsp56dsm.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56k/opcode.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56k/inst.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56k/pmove.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56k/tables.c")
end

--------------------------------------------------
-- PDP-1
-- TX0
--@src/devices/cpu/pdp1/pdp1.h,CPUS["PDP1"] = true
--@src/devices/cpu/pdp1/tx0.h,CPUS["PDP1"] = true
--------------------------------------------------

if (CPUS["PDP1"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/pdp1/pdp1.c",
		MAME_DIR .. "src/devices/cpu/pdp1/pdp1.h",
		MAME_DIR .. "src/devices/cpu/pdp1/tx0.c",
		MAME_DIR .. "src/devices/cpu/pdp1/tx0.h",
	}
end

if (CPUS["PDP1"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pdp1/pdp1dasm.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pdp1/tx0dasm.c")
end

--------------------------------------------------
-- Motorola PowerPC series
--@src/devices/cpu/powerpc/ppc.h,CPUS["POWERPC"] = true
--------------------------------------------------

if (CPUS["POWERPC"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/powerpc/ppccom.c",
		MAME_DIR .. "src/devices/cpu/powerpc/ppccom.h",
		MAME_DIR .. "src/devices/cpu/powerpc/ppcfe.c",
		MAME_DIR .. "src/devices/cpu/powerpc/ppcfe.h",
		MAME_DIR .. "src/devices/cpu/powerpc/ppcdrc.c",
	}
end

if (CPUS["POWERPC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/powerpc/ppc_dasm.c")
end

--------------------------------------------------
-- NEC V-series Intel-compatible
--@src/devices/cpu/nec/nec.h,CPUS["NEC"] = true
--@src/devices/cpu/v30mz/v30mz.h,CPUS["V30MZ"] = true
--------------------------------------------------

if (CPUS["NEC"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/nec/nec.c",
		MAME_DIR .. "src/devices/cpu/nec/nec.h",
		MAME_DIR .. "src/devices/cpu/nec/v25.c",
		MAME_DIR .. "src/devices/cpu/nec/v25.h",
		MAME_DIR .. "src/devices/cpu/nec/v25sfr.c",
		MAME_DIR .. "src/devices/cpu/nec/v53.c",
		MAME_DIR .. "src/devices/cpu/nec/v53.h",
	}
end

if (CPUS["NEC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nec/necdasm.c")
end

if (CPUS["V30MZ"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/v30mz/v30mz.c",
		MAME_DIR .. "src/devices/cpu/v30mz/v30mz.h",
	}
end

if (CPUS["V30MZ"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nec/necdasm.c")
end

--------------------------------------------------
-- NEC V60/V70
--@src/devices/cpu/v60/v60.h,CPUS["V60"] = true
--------------------------------------------------

if (CPUS["V60"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/v60/v60.c",
		MAME_DIR .. "src/devices/cpu/v60/v60.h",
	}
end

if (CPUS["V60"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v60/v60d.c")
end

--------------------------------------------------
-- NEC V810 (uPD70732)
--@src/devices/cpu/v810/v810.h,CPUS["V810"] = true
--------------------------------------------------

if (CPUS["V810"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/v810/v810.c",
		MAME_DIR .. "src/devices/cpu/v810/v810.h",
	}
end

if (CPUS["V810"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v810/v810dasm.c")
end

--------------------------------------------------
-- NEC uPD7725
--@src/devices/cpu/upd7725/upd7725.h,CPUS["UPD7725"] = true
--------------------------------------------------

if (CPUS["UPD7725"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/upd7725/upd7725.c",
		MAME_DIR .. "src/devices/cpu/upd7725/upd7725.h",
	}
end

if (CPUS["UPD7725"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd7725/dasm7725.c")
end

--------------------------------------------------
-- NEC uPD7810 series
--@src/devices/cpu/upd7810/upd7810.h,CPUS["UPD7810"] = true
--------------------------------------------------

if (CPUS["UPD7810"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810.c",
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810.h",
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810_opcodes.c",
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810_table.c",
	}
end

if (CPUS["UPD7810"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd7810/upd7810_dasm.c")
end

--------------------------------------------------
-- NEC uCOM-4 series
--@src/devices/cpu/ucom4/ucom4.h,CPUS["UCOM4"] = true
--------------------------------------------------

if (CPUS["UCOM4"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/ucom4/ucom4.c",
		MAME_DIR .. "src/devices/cpu/ucom4/ucom4.h",
		MAME_DIR .. "src/devices/cpu/ucom4/ucom4op.c",
	}
end

if (CPUS["UCOM4"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ucom4/ucom4d.c")
end

--------------------------------------------------
-- Nintendo Minx
--@src/devices/cpu/minx/minx.h,CPUS["MINX"] = true
--------------------------------------------------

if (CPUS["MINX"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/minx/minx.c",
		MAME_DIR .. "src/devices/cpu/minx/minx.h",
	}
end

if (CPUS["MINX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/minx/minxd.c")
end

--------------------------------------------------
-- Nintendo/SGI RSP (R3000-based + vector processing)
--@src/devices/cpu/rsp/rsp.h,CPUS["RSP"] = true
--------------------------------------------------

if (CPUS["RSP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/rsp/rsp.c",
		MAME_DIR .. "src/devices/cpu/rsp/rsp.h",
		MAME_DIR .. "src/devices/cpu/rsp/rspdrc.c",
		MAME_DIR .. "src/devices/cpu/rsp/rspfe.c",
		MAME_DIR .. "src/devices/cpu/rsp/rspfe.h",
		MAME_DIR .. "src/devices/cpu/rsp/rspcp2.c",
		MAME_DIR .. "src/devices/cpu/rsp/rspcp2.h",
		MAME_DIR .. "src/devices/cpu/rsp/rspcp2d.c",
		MAME_DIR .. "src/devices/cpu/rsp/rspcp2d.h",
		MAME_DIR .. "src/devices/cpu/rsp/clamp.h",
		MAME_DIR .. "src/devices/cpu/rsp/vabs.h",
		MAME_DIR .. "src/devices/cpu/rsp/vadd.h",
		MAME_DIR .. "src/devices/cpu/rsp/vaddc.h",
		MAME_DIR .. "src/devices/cpu/rsp/vand.h",
		MAME_DIR .. "src/devices/cpu/rsp/vch.h",
		MAME_DIR .. "src/devices/cpu/rsp/vcl.h",
		MAME_DIR .. "src/devices/cpu/rsp/vcmp.h",
		MAME_DIR .. "src/devices/cpu/rsp/vcr.h",
		MAME_DIR .. "src/devices/cpu/rsp/vdivh.h",
		MAME_DIR .. "src/devices/cpu/rsp/vldst.h",
		MAME_DIR .. "src/devices/cpu/rsp/vmac.h",
		MAME_DIR .. "src/devices/cpu/rsp/vmov.h",
		MAME_DIR .. "src/devices/cpu/rsp/vmrg.h",
		MAME_DIR .. "src/devices/cpu/rsp/vmudh.h",
		MAME_DIR .. "src/devices/cpu/rsp/vmul.h",
		MAME_DIR .. "src/devices/cpu/rsp/vmulh.h",
		MAME_DIR .. "src/devices/cpu/rsp/vmull.h",
		MAME_DIR .. "src/devices/cpu/rsp/vmulm.h",
		MAME_DIR .. "src/devices/cpu/rsp/vmuln.h",
		MAME_DIR .. "src/devices/cpu/rsp/vor.h",
		MAME_DIR .. "src/devices/cpu/rsp/vrcpsq.h",
		MAME_DIR .. "src/devices/cpu/rsp/vrsq.h",
		MAME_DIR .. "src/devices/cpu/rsp/vsub.h",
		MAME_DIR .. "src/devices/cpu/rsp/vsubc.h",
		MAME_DIR .. "src/devices/cpu/rsp/vxor.h",
	}
end

if (CPUS["RSP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/rsp/rsp_dasm.c")
end

--------------------------------------------------
-- Panasonic MN10200
--@src/devices/cpu/mn10200/mn10200.h,CPUS["MN10200"] = true
--------------------------------------------------

if (CPUS["MN10200"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mn10200/mn10200.c",
		MAME_DIR .. "src/devices/cpu/mn10200/mn10200.h",
	}
end

if (CPUS["MN10200"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mn10200/mn102dis.c")
end

--------------------------------------------------
-- Saturn
--@src/devices/cpu/saturn/saturn.h,CPUS["SATURN"] = true
--------------------------------------------------

if (CPUS["SATURN"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/saturn/saturn.c",
		MAME_DIR .. "src/devices/cpu/saturn/saturn.h",
	}
end

if (CPUS["SATURN"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/saturn/saturnds.c")
end

--------------------------------------------------
-- Sharp SM510 series
--@src/devices/cpu/sm510/sm510.h,CPUS["SM510"] = true
--------------------------------------------------

if (CPUS["SM510"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sm510/sm510.c",
		MAME_DIR .. "src/devices/cpu/sm510/sm510.h",
		MAME_DIR .. "src/devices/cpu/sm510/sm510op.c",
		MAME_DIR .. "src/devices/cpu/sm510/sm510core.c",
		MAME_DIR .. "src/devices/cpu/sm510/sm511core.c",
	}
end

if (CPUS["SM510"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sm510/sm510d.c")
end

--------------------------------------------------
-- Sharp SM8500
--@src/devices/cpu/sm8500/sm8500.h,CPUS["SM8500"] = true
--------------------------------------------------

if (CPUS["SM8500"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sm8500/sm8500.c",
		MAME_DIR .. "src/devices/cpu/sm8500/sm8500.h",
	}
end

if (CPUS["SM8500"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sm8500/sm8500d.c")
end

--------------------------------------------------
-- Signetics 2650
--@src/devices/cpu/s2650/s2650.h,CPUS["S2650"] = true
--------------------------------------------------

if (CPUS["S2650"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/s2650/s2650.c",
		MAME_DIR .. "src/devices/cpu/s2650/s2650.h",
	}
end

if (CPUS["S2650"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/s2650/2650dasm.c")
end

--------------------------------------------------
-- SC61860
--@src/devices/cpu/sc61860/sc61860.h,CPUS["SC61860"] = true
--------------------------------------------------

if (CPUS["SC61860"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sc61860/sc61860.c",
		MAME_DIR .. "src/devices/cpu/sc61860/sc61860.h",
	}
end

if (CPUS["SC61860"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sc61860/scdasm.c")
end

--------------------------------------------------
-- Sony/Nintendo SPC700
--@src/devices/cpu/spc700/spc700.h,CPUS["SPC700"] = true
--------------------------------------------------

if (CPUS["SPC700"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/spc700/spc700.c",
		MAME_DIR .. "src/devices/cpu/spc700/spc700.h",
	}
end

if (CPUS["SPC700"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/spc700/spc700ds.c")
end

--------------------------------------------------
-- SSP1601
--@src/devices/cpu/ssp1601/ssp1601.h,CPUS["SSP1601"] = true
--------------------------------------------------

if (CPUS["SSP1601"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/ssp1601/ssp1601.c",
		MAME_DIR .. "src/devices/cpu/ssp1601/ssp1601.h",
	}
end

if (CPUS["SSP1601"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ssp1601/ssp1601d.c")
end

--------------------------------------------------
-- SunPlus u'nSP
--@src/devices/cpu/unsp/unsp.h,CPUS["UNSP"] = true
--------------------------------------------------

if (CPUS["UNSP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/unsp/unsp.c",
		MAME_DIR .. "src/devices/cpu/unsp/unsp.h",
	}
end

if (CPUS["UNSP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/unsp/unspdasm.c")
end

--------------------------------------------------
-- Atmel 8-bit AVR
--@src/devices/cpu/avr8/avr8.h,CPUS["AVR8"] = true
--------------------------------------------------

if (CPUS["AVR8"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/avr8/avr8.c",
		MAME_DIR .. "src/devices/cpu/avr8/avr8.h",
	}
end

if (CPUS["AVR8"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/avr8/avr8dasm.c")
end

--------------------------------------------------
-- Texas Instruments TMS0980
--@src/devices/cpu/tms0980/tms0980.h,CPUS["TMS0980"] = true
--------------------------------------------------

if (CPUS["TMS0980"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms0980/tms0980.c",
		MAME_DIR .. "src/devices/cpu/tms0980/tms0980.h",
	}
end

if (CPUS["TMS0980"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms0980/tms0980d.c")
end

--------------------------------------------------
-- Texas Instruments TMS7000 series
--@src/devices/cpu/tms7000/tms7000.h,CPUS["TMS7000"] = true
--------------------------------------------------

if (CPUS["TMS7000"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms7000/tms7000.c",
		MAME_DIR .. "src/devices/cpu/tms7000/tms7000.h",
	}
end

if (CPUS["TMS7000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms7000/7000dasm.c")
end

--------------------------------------------------
-- Texas Instruments TMS99xx series
--@src/devices/cpu/tms9900/tms9900.h,CPUS["TMS9900"] = true
--@src/devices/cpu/tms9900/tms9980a.h,CPUS["TMS9900"] = true
--@src/devices/cpu/tms9900/tms9995.h,CPUS["TMS9900"] = true

--------------------------------------------------

if (CPUS["TMS9900"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms9900/tms9900.c",
		MAME_DIR .. "src/devices/cpu/tms9900/tms9900.h",
		MAME_DIR .. "src/devices/cpu/tms9900/tms9980a.c",
		MAME_DIR .. "src/devices/cpu/tms9900/tms9980a.h",
		MAME_DIR .. "src/devices/cpu/tms9900/tms9995.c",
		MAME_DIR .. "src/devices/cpu/tms9900/tms9995.h",
		MAME_DIR .. "src/devices/cpu/tms9900/ti990_10.c",
		MAME_DIR .. "src/devices/cpu/tms9900/ti990_10.h",
	}
end

if (CPUS["TMS9900"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms9900/9900dasm.c")
end

--------------------------------------------------
-- Texas Instruments TMS340x0 graphics controllers
--@src/devices/cpu/tms34010/tms34010.h,CPUS["TMS340X0"] = true
--------------------------------------------------

if (CPUS["TMS340X0"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms34010/tms34010.c",
		MAME_DIR .. "src/devices/cpu/tms34010/tms34010.h",
	}
end

if (CPUS["TMS340X0"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms34010/34010dsm.c")
end

--------------------------------------------------
-- Texas Instruments TMS3201x DSP
--@src/devices/cpu/tms32010/tms32010.h,CPUS["TMS32010"] = true
--------------------------------------------------

if (CPUS["TMS32010"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms32010/tms32010.c",
		MAME_DIR .. "src/devices/cpu/tms32010/tms32010.h",
	}
end

if (CPUS["TMS32010"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32010/32010dsm.c")
end

--------------------------------------------------
-- Texas Instruments TMS3202x DSP
--@src/devices/cpu/tms32025/tms32025.h,CPUS["TMS32025"] = true
--------------------------------------------------

if (CPUS["TMS32025"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms32025/tms32025.c",
		MAME_DIR .. "src/devices/cpu/tms32025/tms32025.h",
	}
end

if (CPUS["TMS32025"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32025/32025dsm.c")
end

--------------------------------------------------
-- Texas Instruments TMS3203x DSP
--@src/devices/cpu/tms32031/tms32031.h,CPUS["TMS32031"] = true
--------------------------------------------------

if (CPUS["TMS32031"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms32031/tms32031.c",
		MAME_DIR .. "src/devices/cpu/tms32031/tms32031.h",
	}
end

if (CPUS["TMS32031"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32031/dis32031.c")
end

--------------------------------------------------
-- Texas Instruments TMS3205x DSP
--@src/devices/cpu/tms32051/tms32051.h,CPUS["TMS32051"] = true
--------------------------------------------------

if (CPUS["TMS32051"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms32051/tms32051.c",
		MAME_DIR .. "src/devices/cpu/tms32051/tms32051.h",
	}
end

if (CPUS["TMS32051"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32051/dis32051.c")
end

--------------------------------------------------
-- Texas Instruments TMS3208x DSP
--@src/devices/cpu/tms32082/tms32082.h,CPUS["TMS32082"] = true
--------------------------------------------------

if (CPUS["TMS32082"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms32082/tms32082.c",
		MAME_DIR .. "src/devices/cpu/tms32082/tms32082.h",
		MAME_DIR .. "src/devices/cpu/tms32082/mp_ops.c",
	}
end

if (CPUS["TMS32082"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32082/dis_mp.c")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32082/dis_pp.c")
end

--------------------------------------------------
-- Texas Instruments TMS57002 DSP
--@src/devices/cpu/tms57002/tms57002.h,CPUS["TMS57002"] = true
--------------------------------------------------

if (CPUS["TMS57002"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms57002/tms57002.c",
		MAME_DIR .. "src/devices/cpu/tms57002/tms57002.h",
		MAME_DIR .. "src/devices/cpu/tms57002/tms57kdec.c",
	}
	dependency {
		{ MAME_DIR .. "src/devices/cpu/tms57002/tms57kdec.c", GEN_DIR .. "emu/cpu/tms57002/tms57002.inc" },
		{ MAME_DIR .. "src/devices/cpu/tms57002/tms57002.c",  GEN_DIR .. "emu/cpu/tms57002/tms57002.inc" },
	}
	custombuildtask { 	
		{ MAME_DIR .. "src/devices/cpu/tms57002/tmsinstr.lst" , GEN_DIR .. "emu/cpu/tms57002/tms57002.inc",   { MAME_DIR .. "src/devices/cpu/tms57002/tmsmake.py" }, {"@echo Generating TMS57002 source file...", PYTHON .. " $(1) $(<) $(@)" } }
	}
end

if (CPUS["TMS57002"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms57002/57002dsm.c")
	table.insert(disasm_dependency , { MAME_DIR .. "src/devices/cpu/tms57002/57002dsm.c",  GEN_DIR .. "emu/cpu/tms57002/tms57002.inc" } )
	table.insert(disasm_custombuildtask , { MAME_DIR .. "src/devices/cpu/tms57002/tmsinstr.lst" , GEN_DIR .. "emu/cpu/tms57002/tms57002.inc",   { MAME_DIR .. "src/devices/cpu/tms57002/tmsmake.py" }, {"@echo Generating TMS57002 source file...", PYTHON .. " $(1) $(<) $(@)" }})
end

--------------------------------------------------
-- Toshiba TLCS-90 Series
--@src/devices/cpu/tlcs90/tlcs90.h,CPUS["TLCS90"] = true
--------------------------------------------------

if (CPUS["TLCS90"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tlcs90/tlcs90.c",
		MAME_DIR .. "src/devices/cpu/tlcs90/tlcs90.h",
	}
end

--------------------------------------------------
-- Toshiba TLCS-900 Series
--@src/devices/cpu/tlcs900/tlcs900.h,CPUS["TLCS900"] = true
--------------------------------------------------

if (CPUS["TLCS900"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tlcs900/tlcs900.c",
		MAME_DIR .. "src/devices/cpu/tlcs900/tlcs900.h",
	}
end

if (CPUS["TLCS900"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tlcs900/dasm900.c")
end

--------------------------------------------------
-- Zilog Z80
--@src/devices/cpu/z80/z80.h,CPUS["Z80"] = true
--@src/devices/cpu/z80/z80daisy.h,CPUS["Z80"] = true
--------------------------------------------------

if (CPUS["Z80"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/z80/z80.c",
		MAME_DIR .. "src/devices/cpu/z80/z80.h",
		MAME_DIR .. "src/devices/cpu/z80/z80daisy.c",
		MAME_DIR .. "src/devices/cpu/z80/z80daisy.h",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c011.c",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c011.h",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c015.c",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c015.h",
		MAME_DIR .. "src/devices/cpu/z80/kl5c80a12.c",
		MAME_DIR .. "src/devices/cpu/z80/kl5c80a12.h",
	}
end

if (CPUS["Z80"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z80/z80dasm.c")
end

--------------------------------------------------
-- Sharp LR35902 (Game Boy CPU)
--@src/devices/cpu/lr35902/lr35902.h,CPUS["LR35902"] = true
--------------------------------------------------

if (CPUS["LR35902"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/lr35902/lr35902.c",
		MAME_DIR .. "src/devices/cpu/lr35902/lr35902.h",
	}
end

if (CPUS["LR35902"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lr35902/lr35902d.c")
end

--------------------------------------------------
-- Zilog Z180
--@src/devices/cpu/z180/z180.h,CPUS["Z180"] = true
--------------------------------------------------

if (CPUS["Z180"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/z180/z180.c",
		MAME_DIR .. "src/devices/cpu/z180/z180.h",
		MAME_DIR .. "src/devices/cpu/z80/z80daisy.c",
		MAME_DIR .. "src/devices/cpu/z80/z80daisy.h",
	}
end

if (CPUS["Z180"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z180/z180dasm.c")
end

--------------------------------------------------
-- Zilog Z8000
--@src/devices/cpu/z8000/z8000.h,CPUS["Z8000"] = true
--------------------------------------------------

if (CPUS["Z8000"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/z8000/z8000.c",
		MAME_DIR .. "src/devices/cpu/z8000/z8000.h",
	}
end

if (CPUS["Z8000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z8000/8000dasm.c")
end

--------------------------------------------------
-- Zilog Z8
--@src/devices/cpu/z8/z8.h,CPUS["Z8"] = true
--------------------------------------------------

if (CPUS["Z8"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/z8/z8.c",
		MAME_DIR .. "src/devices/cpu/z8/z8.h",
	}
end

if (CPUS["Z8"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z8/z8dasm.c")
end

--------------------------------------------------
-- Argonaut SuperFX
--@src/devices/cpu/superfx/superfx.h,CPUS["SUPERFX"] = true
--------------------------------------------------

if (CPUS["SUPERFX"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/superfx/superfx.c",
		MAME_DIR .. "src/devices/cpu/superfx/superfx.h",
	}
end

if (CPUS["SUPERFX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/superfx/sfx_dasm.c")
end

--------------------------------------------------
-- Rockwell PPS-4
--@src/devices/cpu/pps4/pps4.h,CPUS["PPS4"] = true
--------------------------------------------------

if (CPUS["PPS4"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/pps4/pps4.c",
		MAME_DIR .. "src/devices/cpu/pps4/pps4.h",
	}
end

if (CPUS["PPS4"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pps4/pps4dasm.c")
end

--------------------------------------------------
-- Hitachi HD61700
--@src/devices/cpu/hd61700/hd61700.h,CPUS["HD61700"] = true
--------------------------------------------------

if (CPUS["HD61700"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/hd61700/hd61700.c",
		MAME_DIR .. "src/devices/cpu/hd61700/hd61700.h",
	}
end

if (CPUS["HD61700"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hd61700/hd61700d.c")
end

--------------------------------------------------
-- Sanyo LC8670
--@src/devices/cpu/lc8670/lc8670.h,CPUS["LC8670"] = true
--------------------------------------------------

if (CPUS["LC8670"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/lc8670/lc8670.c",
		MAME_DIR .. "src/devices/cpu/lc8670/lc8670.h",
	}
end

if (CPUS["LC8670"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lc8670/lc8670dsm.c")
end

--------------------------------------------------
-- Sega SCU DSP
--@src/devices/cpu/scudsp/scudsp.h,CPUS["SCUDSP"] = true
--------------------------------------------------

if (CPUS["SCUDSP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/scudsp/scudsp.c",
		MAME_DIR .. "src/devices/cpu/scudsp/scudsp.h",
	}
end

if (CPUS["SCUDSP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/scudsp/scudspdasm.c")
end

--------------------------------------------------
-- Sunplus Technology S+core
--@src/devices/cpu/score/score.h,CPUS["SCORE"] = true
--------------------------------------------------

if (CPUS["SCORE"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/score/score.c",
		MAME_DIR .. "src/devices/cpu/score/score.h",
	}
end

if (CPUS["SCORE"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/score/scoredsm.c")
end

--------------------------------------------------
-- Xerox Alto-II
--@src/devices/cpu/alto2/alto2cpu.h,CPUS["ALTO2"] = true
--------------------------------------------------

if (CPUS["ALTO2"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/alto2/alto2cpu.c",
		MAME_DIR .. "src/devices/cpu/alto2/alto2cpu.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2disk.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2disk.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2disp.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2disp.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2curt.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2curt.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2dht.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2dht.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2dvt.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2dvt.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2dwt.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2dwt.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2emu.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2emu.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2ether.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2ether.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2hw.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2hw.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2kbd.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2kbd.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2ksec.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2ksec.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2kwd.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2kwd.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2mem.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2mem.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2mouse.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2mouse.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2mrt.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2mrt.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2part.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2part.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2ram.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2ram.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2roms.c",
		MAME_DIR .. "src/devices/cpu/alto2/a2roms.h",
	}
end

if (CPUS["ALTO2"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/alto2/alto2dsm.c")
end

