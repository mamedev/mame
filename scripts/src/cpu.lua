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
	MAME_DIR .. "src/devices/cpu/vtlb.cpp",
}

--------------------------------------------------
-- Dynamic recompiler objects
--------------------------------------------------

if (CPUS["SH2"]~=null or CPUS["MIPS"]~=null or CPUS["POWERPC"]~=null or CPUS["RSP"]~=null or CPUS["ARM7"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/drcbec.cpp",
		MAME_DIR .. "src/devices/cpu/drcbec.h",
		MAME_DIR .. "src/devices/cpu/drcbeut.cpp",
		MAME_DIR .. "src/devices/cpu/drcbeut.h",
		MAME_DIR .. "src/devices/cpu/drccache.cpp",
		MAME_DIR .. "src/devices/cpu/drccache.h",
		MAME_DIR .. "src/devices/cpu/drcfe.cpp",
		MAME_DIR .. "src/devices/cpu/drcfe.h",
		MAME_DIR .. "src/devices/cpu/drcuml.cpp",
		MAME_DIR .. "src/devices/cpu/drcuml.h",
		MAME_DIR .. "src/devices/cpu/uml.cpp",
		MAME_DIR .. "src/devices/cpu/uml.h",
		MAME_DIR .. "src/devices/cpu/i386/i386dasm.cpp",
		MAME_DIR .. "src/devices/cpu/x86log.cpp",
		MAME_DIR .. "src/devices/cpu/x86log.h",
		MAME_DIR .. "src/devices/cpu/drcbex86.cpp",
		MAME_DIR .. "src/devices/cpu/drcbex86.h",
		MAME_DIR .. "src/devices/cpu/drcbex64.cpp",
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
		MAME_DIR .. "src/devices/cpu/8x300/8x300.cpp",
		MAME_DIR .. "src/devices/cpu/8x300/8x300.h",
	}
end

if (CPUS["8X300"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/8x300/8x300dasm.cpp")
end

--------------------------------------------------
-- ARCangent A4
--@src/devices/cpu/arc/arc.h,CPUS["ARC"] = true
--------------------------------------------------

if (CPUS["ARC"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/arc/arc.cpp",
		MAME_DIR .. "src/devices/cpu/arc/arc.h",
	}
end

if (CPUS["ARC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arc/arcdasm.cpp")
end

--------------------------------------------------
-- ARcompact (ARCtangent-A5, ARC 600, ARC 700)
--@src/devices/cpu/arc/arc.h,CPUS["ARCOMPACT"] = true
--------------------------------------------------

if (CPUS["ARCOMPACT"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact.h",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_dispatch.h",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops.h",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_common.h",
	}
	dependency {
		{ MAME_DIR .. "src/devices/cpu/arcompact/arcompact.cpp",  	   GEN_DIR .. "emu/cpu/arcompact/arcompact.inc" },
		{ MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute.cpp", GEN_DIR .. "emu/cpu/arcompact/arcompact.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/arcompact/arcompact_make.py" , GEN_DIR .. "emu/cpu/arcompact/arcompact.inc",   { MAME_DIR .. "src/devices/cpu/arcompact/arcompact_make.py" }, {"@echo Generating arcompact source .inc files...", PYTHON .. " $(1)  $(@)" }},
	}
end

if (CPUS["ARCOMPACT"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_dispatch.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompact_common.cpp")
end

--------------------------------------------------
-- Acorn ARM series
--
--@src/devices/cpu/arm/arm.h,CPUS["ARM"] = true
--@src/devices/cpu/arm7/arm7.h,CPUS["ARM7"] = true
--------------------------------------------------

if (CPUS["ARM"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/arm/arm.cpp",
		MAME_DIR .. "src/devices/cpu/arm/arm.h",
	}
end

if (CPUS["ARM"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arm/armdasm.cpp")
end

if (CPUS["ARM7"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/arm7/arm7.cpp",
		MAME_DIR .. "src/devices/cpu/arm7/arm7.h",
		MAME_DIR .. "src/devices/cpu/arm7/arm7thmb.cpp",
		MAME_DIR .. "src/devices/cpu/arm7/arm7ops.cpp",
		MAME_DIR .. "src/devices/cpu/arm7/lpc210x.cpp",
		MAME_DIR .. "src/devices/cpu/arm7/lpc210x.h",
		MAME_DIR .. "src/devices/cpu/arm7/arm7core.h",
		MAME_DIR .. "src/devices/cpu/arm7/arm7core.inc",
		MAME_DIR .. "src/devices/cpu/arm7/arm7drc.inc",
		MAME_DIR .. "src/devices/cpu/arm7/arm7help.h",
		MAME_DIR .. "src/devices/cpu/arm7/arm7tdrc.inc",
	}
end

if (CPUS["ARM7"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arm7/arm7dasm.cpp")
end

--------------------------------------------------
-- Advanced Digital Chips SE3208
--@src/devices/cpu/se3208/se3208.h,CPUS["SE3208"] = true
--------------------------------------------------

if (CPUS["SE3208"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/se3208/se3208.cpp",
		MAME_DIR .. "src/devices/cpu/se3208/se3208.h",
	}
end

if (CPUS["SE3208"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/se3208/se3208dis.cpp")
end

--------------------------------------------------
-- American Microsystems, Inc.(AMI) S2000 series
--@src/devices/cpu/amis2000/amis2000.h,CPUS["AMIS2000"] = true
--------------------------------------------------

if (CPUS["AMIS2000"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/amis2000/amis2000.cpp",
		MAME_DIR .. "src/devices/cpu/amis2000/amis2000.h",
		MAME_DIR .. "src/devices/cpu/amis2000/amis2000op.cpp",
	}
end

if (CPUS["AMIS2000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/amis2000/amis2000d.cpp")
end

--------------------------------------------------
-- Alpha 8201
--@src/devices/cpu/alph8201/alph8201.h,CPUS["ALPHA8201"] = true
--------------------------------------------------

if (CPUS["ALPHA8201"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/alph8201/alph8201.cpp",
		MAME_DIR .. "src/devices/cpu/alph8201/alph8201.h",
	}
end

if (CPUS["ALPHA8201"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/alph8201/8201dasm.cpp")
end

--------------------------------------------------
-- Analog Devices ADSP21xx series
--@src/devices/cpu/adsp2100/adsp2100.h,CPUS["ADSP21XX"] = true
--------------------------------------------------

if (CPUS["ADSP21XX"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/adsp2100/adsp2100.cpp",
		MAME_DIR .. "src/devices/cpu/adsp2100/adsp2100.h",
		MAME_DIR .. "src/devices/cpu/adsp2100/2100ops.inc",
	}
end

if (CPUS["ADSP21XX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/adsp2100/2100dasm.cpp")
end

--------------------------------------------------
-- Analog Devices "Sharc" ADSP21062
--@src/devices/cpu/sharc/sharc.h,CPUS["ADSP21062"] = true
--------------------------------------------------

if (CPUS["ADSP21062"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sharc/sharc.cpp",
		MAME_DIR .. "src/devices/cpu/sharc/sharc.h",
		MAME_DIR .. "src/devices/cpu/sharc/compute.inc",
		MAME_DIR .. "src/devices/cpu/sharc/sharcdma.inc",
		MAME_DIR .. "src/devices/cpu/sharc/sharcdsm.h",
		MAME_DIR .. "src/devices/cpu/sharc/sharcmem.inc",
		MAME_DIR .. "src/devices/cpu/sharc/sharcops.h",
		MAME_DIR .. "src/devices/cpu/sharc/sharcops.inc",
	}
end

if (CPUS["ADSP21062"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sharc/sharcdsm.cpp")
end

--------------------------------------------------
-- APEXC
--@src/devices/cpu/apexc/apexc.h,CPUS["APEXC"] = true
--------------------------------------------------

if (CPUS["APEXC"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/apexc/apexc.cpp",
		MAME_DIR .. "src/devices/cpu/apexc/apexc.h",
	}
end

if (CPUS["APEXC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/apexc/apexcdsm.cpp")
end

--------------------------------------------------
-- AT&T DSP16A
--@src/devices/cpu/dsp16/dsp16.h,CPUS["DSP16A"] = true
--------------------------------------------------

if (CPUS["DSP16A"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16.cpp",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16.h",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16ops.inc",
	}
end

if (CPUS["DSP16A"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp16/dsp16dis.cpp")
end

--------------------------------------------------
-- AT&T DSP32C
--@src/devices/cpu/dsp32/dsp32.h,CPUS["DSP32C"] = true
--------------------------------------------------

if (CPUS["DSP32C"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/dsp32/dsp32.cpp",
		MAME_DIR .. "src/devices/cpu/dsp32/dsp32.h",
		MAME_DIR .. "src/devices/cpu/dsp32/dsp32ops.inc",
	}
end

if (CPUS["DSP32C"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp32/dsp32dis.cpp")
end

--------------------------------------------------
-- Atari custom RISC processor
--@src/devices/cpu/asap/asap.h,CPUS["ASAP"] = true
--------------------------------------------------

if (CPUS["ASAP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/asap/asap.cpp",
		MAME_DIR .. "src/devices/cpu/asap/asap.h",
	}
end

if (CPUS["ASAP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/asap/asapdasm.cpp")
end

--------------------------------------------------
-- AMD Am29000
--@src/devices/cpu/am29000/am29000.h,CPUS["AM29000"] = true
--------------------------------------------------

if (CPUS["AM29000"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/am29000/am29000.cpp",
		MAME_DIR .. "src/devices/cpu/am29000/am29000.h",
		MAME_DIR .. "src/devices/cpu/am29000/am29ops.h",
	}
end

if (CPUS["AM29000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/am29000/am29dasm.cpp")
end

--------------------------------------------------
-- Atari Jaguar custom DSPs
--@src/devices/cpu/jaguar/jaguar.h,CPUS["JAGUAR"] = true
--------------------------------------------------

if (CPUS["JAGUAR"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/jaguar/jaguar.cpp",
		MAME_DIR .. "src/devices/cpu/jaguar/jaguar.h",
	}
end

if (CPUS["JAGUAR"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/jaguar/jagdasm.cpp")
end

--------------------------------------------------
-- Simutrek Cube Quest bit-sliced CPUs
--@src/devices/cpu/cubeqcpu/cubeqcpu.h,CPUS["CUBEQCPU"] = true
--------------------------------------------------

if (CPUS["CUBEQCPU"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/cubeqcpu/cubeqcpu.cpp",
		MAME_DIR .. "src/devices/cpu/cubeqcpu/cubeqcpu.h",
	}
end

if (CPUS["CUBEQCPU"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cubeqcpu/cubedasm.cpp")
end

--------------------------------------------------
-- Ensoniq ES5510 ('ESP') DSP
--@src/devices/cpu/es5510/es5510.h,CPUS["ES5510"] = true
--------------------------------------------------

if (CPUS["ES5510"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/es5510/es5510.cpp",
		MAME_DIR .. "src/devices/cpu/es5510/es5510.h",
	}
end

--------------------------------------------------
-- Entertainment Sciences AM29116-based RIP
--@src/devices/cpu/esrip/esrip.h,CPUS["ESRIP"] = true
--------------------------------------------------

if (CPUS["ESRIP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/esrip/esrip.cpp",
		MAME_DIR .. "src/devices/cpu/esrip/esrip.h",
	}
end

if (CPUS["ESRIP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/esrip/esripdsm.cpp")
end

--------------------------------------------------
-- Seiko Epson E0C6200 series
--@src/devices/cpu/e0c6200/e0c6200.h,CPUS["E0C6200"] = true
--------------------------------------------------

if (CPUS["E0C6200"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200.cpp",
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200.h",
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6s46.cpp",
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6s46.h",
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200op.inc",
	}
end

if (CPUS["E0C6200"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200d.cpp")
end

--------------------------------------------------
-- RCA COSMAC
--@src/devices/cpu/cosmac/cosmac.h,CPUS["COSMAC"] = true
--------------------------------------------------

if (CPUS["COSMAC"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/cosmac/cosmac.cpp",
		MAME_DIR .. "src/devices/cpu/cosmac/cosmac.h",
	}
end

if (CPUS["COSMAC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cosmac/cosdasm.cpp")
end

--------------------------------------------------
-- National Semiconductor COP400 family
--@src/devices/cpu/cop400/cop400.h,CPUS["COP400"] = true
--------------------------------------------------

if (CPUS["COP400"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/cop400/cop400.cpp",
		MAME_DIR .. "src/devices/cpu/cop400/cop400.h",
		MAME_DIR .. "src/devices/cpu/cop400/cop400op.inc",
	}
end

if (CPUS["COP400"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop410ds.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop420ds.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop440ds.cpp")
end

--------------------------------------------------
-- CP1610
--@src/devices/cpu/cp1610/cp1610.h,CPUS["CP1610"] = true
--------------------------------------------------

if (CPUS["CP1610"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/cp1610/cp1610.cpp",
		MAME_DIR .. "src/devices/cpu/cp1610/cp1610.h",
	}
end

if (CPUS["CP1610"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cp1610/1610dasm.cpp")
end

--------------------------------------------------
-- Cinematronics vector "CPU"
--@src/devices/cpu/ccpu/ccpu.h,CPUS["CCPU"] = true
--------------------------------------------------

if (CPUS["CCPU"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/ccpu/ccpu.cpp",
		MAME_DIR .. "src/devices/cpu/ccpu/ccpu.h",
	}
end

if (CPUS["CCPU"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ccpu/ccpudasm.cpp")
end

--------------------------------------------------
-- DEC T-11
--@src/devices/cpu/t11/t11.h,CPUS["T11"] = true
--------------------------------------------------

if (CPUS["T11"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/t11/t11.cpp",
		MAME_DIR .. "src/devices/cpu/t11/t11.h",
		MAME_DIR .. "src/devices/cpu/t11/t11ops.inc",
		MAME_DIR .. "src/devices/cpu/t11/t11table.inc",
	}
end

if (CPUS["T11"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/t11/t11dasm.cpp")
end

--------------------------------------------------
-- DEC PDP-8
--@src/devices/cpu/pdp8/pdp8.h,CPUS["PDP8"] = true
--------------------------------------------------

if (CPUS["PDP8"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/pdp8/pdp8.cpp",
		MAME_DIR .. "src/devices/cpu/pdp8/pdp8.h",
	}
end

if (CPUS["PDP8"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pdp8/pdp8dasm.cpp")
end

--------------------------------------------------
-- F8
--@src/devices/cpu/f8/f8.h,CPUS["F8"] = true
--------------------------------------------------

if (CPUS["F8"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/f8/f8.cpp",
		MAME_DIR .. "src/devices/cpu/f8/f8.h",
	}
end

if (CPUS["F8"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/f8/f8dasm.cpp")
end

--------------------------------------------------
-- G65816
--@src/devices/cpu/g65816/g65816.h,CPUS["G65816"] = true
--------------------------------------------------

if (CPUS["G65816"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/g65816/g65816.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816.h",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o0.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o1.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o2.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o3.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o4.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816cm.h",
		MAME_DIR .. "src/devices/cpu/g65816/g65816ds.h",
		MAME_DIR .. "src/devices/cpu/g65816/g65816op.h",
	}
end

if (CPUS["G65816"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/g65816/g65816ds.cpp")
end

--------------------------------------------------
-- Hitachi H8 (16/32-bit H8/300, H8/300H, H8S2000 and H8S2600 series)
--@src/devices/cpu/h8/h8.h,CPUS["H8"] = true
--------------------------------------------------

if (CPUS["H8"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/h8/h8.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8.h",
		MAME_DIR .. "src/devices/cpu/h8/h8h.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8h.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2000.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2000.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2600.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2600.h",
		MAME_DIR .. "src/devices/cpu/h8/h83337.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83337.h",
		MAME_DIR .. "src/devices/cpu/h8/h83002.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83002.h",
		MAME_DIR .. "src/devices/cpu/h8/h83006.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83006.h",
		MAME_DIR .. "src/devices/cpu/h8/h83008.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83008.h",
		MAME_DIR .. "src/devices/cpu/h8/h83048.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83048.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2245.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2245.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2320.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2320.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2357.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2357.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2655.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2655.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_adc.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_adc.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_port.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_port.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_intc.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_intc.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer8.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer8.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer16.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer16.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_sci.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_sci.h",
	}
	
	dependency {
		{ MAME_DIR .. "src/devices/cpu/h8/h8.cpp",       GEN_DIR .. "emu/cpu/h8/h8.inc" },
		{ MAME_DIR .. "src/devices/cpu/h8/h8h.cpp",      GEN_DIR .. "emu/cpu/h8/h8h.inc" },
		{ MAME_DIR .. "src/devices/cpu/h8/h8s2000.cpp",  GEN_DIR .. "emu/cpu/h8/h8s2000.inc" },
		{ MAME_DIR .. "src/devices/cpu/h8/h8s2600.cpp",  GEN_DIR .. "emu/cpu/h8/h8s2600.inc" },
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
		MAME_DIR .. "src/devices/cpu/hcd62121/hcd62121.cpp",
		MAME_DIR .. "src/devices/cpu/hcd62121/hcd62121.h",
		MAME_DIR .. "src/devices/cpu/hcd62121/hcd62121_ops.h",
	}
end

if (CPUS["HCD62121"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hcd62121/hcd62121d.cpp")
end

--------------------------------------------------
-- Hitachi HMCS40 series
--@src/devices/cpu/hmcs40/hmcs40.h,CPUS["HMCS40"] = true
--------------------------------------------------

if (CPUS["HMCS40"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40.cpp",
		MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40.h",
		MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40op.cpp",
	}
end

if (CPUS["HMCS40"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40d.cpp")
end

--------------------------------------------------
-- Hitachi SH1/SH2
--@src/devices/cpu/sh2/sh2.h,CPUS["SH2"] = true
--------------------------------------------------

if (CPUS["SH2"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sh2/sh2.cpp",
		MAME_DIR .. "src/devices/cpu/sh2/sh2.h",
		MAME_DIR .. "src/devices/cpu/sh2/sh2fe.cpp",
		--MAME_DIR .. "src/devices/cpu/sh2/sh2comn.cpp",
		--MAME_DIR .. "src/devices/cpu/sh2/sh2comn.h",
		--MAME_DIR .. "src/devices/cpu/sh2/sh2drc.cpp",
	}
end

if (CPUS["SH2"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sh2/sh2dasm.cpp")
end

--------------------------------------------------
-- Hitachi SH4
--@src/devices/cpu/sh4/sh4.h,CPUS["SH4"] = true
--------------------------------------------------

if (CPUS["SH4"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sh4/sh4.cpp",
		MAME_DIR .. "src/devices/cpu/sh4/sh4.h",
		MAME_DIR .. "src/devices/cpu/sh4/sh4comn.cpp",
		MAME_DIR .. "src/devices/cpu/sh4/sh4comn.h",
		MAME_DIR .. "src/devices/cpu/sh4/sh3comn.cpp",
		MAME_DIR .. "src/devices/cpu/sh4/sh3comn.h",
		MAME_DIR .. "src/devices/cpu/sh4/sh4tmu.cpp",
		MAME_DIR .. "src/devices/cpu/sh4/sh4tmu.h",
		MAME_DIR .. "src/devices/cpu/sh4/sh4dmac.cpp",
		MAME_DIR .. "src/devices/cpu/sh4/sh4dmac.h",
		MAME_DIR .. "src/devices/cpu/sh4/sh4regs.h",
	}
end

if (CPUS["SH4"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sh4/sh4dasm.cpp")
end

--------------------------------------------------
-- HP Hybrid processor
--@src/devices/cpu/hphybrid/hphybrid.h,CPUS["HPHYBRID"] = true
--------------------------------------------------

if (CPUS["HPHYBRID"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/hphybrid/hphybrid.cpp",
		MAME_DIR .. "src/devices/cpu/hphybrid/hphybrid.h",
	}
end

if (CPUS["HPHYBRID"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hphybrid/hphybrid_dasm.cpp")
end

--------------------------------------------------
-- Hudsonsoft 6280
--@src/devices/cpu/h6280/h6280.h,CPUS["H6280"] = true
--------------------------------------------------

if (CPUS["H6280"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/h6280/h6280.cpp",
		MAME_DIR .. "src/devices/cpu/h6280/h6280.h",
	}
end

if (CPUS["H6280"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/h6280/6280dasm.cpp")
end

--------------------------------------------------
-- Hyperstone E1 series
--@src/devices/cpu/e132xs/e132xs.h,CPUS["E1"] = true
--------------------------------------------------

if (CPUS["E1"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/e132xs/e132xs.cpp",
		MAME_DIR .. "src/devices/cpu/e132xs/e132xs.h",
		MAME_DIR .. "src/devices/cpu/e132xs/e132xsop.inc",
	}
end

if (CPUS["E1"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/e132xs/32xsdasm.cpp")
end

--------------------------------------------------
-- 15IE-00-013 CPU ("Microprogrammed Control Device")
--@src/devices/cpu/ie15/ie15.h,CPUS["IE15"] = true
--------------------------------------------------

if (CPUS["IE15"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/ie15/ie15.cpp",
		MAME_DIR .. "src/devices/cpu/ie15/ie15.h",
	}
end

if (CPUS["IE15"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ie15/ie15dasm.cpp")
end

--------------------------------------------------
-- Intel 4004
--@src/devices/cpu/i4004/i4004.h,CPUS["I4004"] = true
--------------------------------------------------

if (CPUS["I4004"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i4004/i4004.cpp",
		MAME_DIR .. "src/devices/cpu/i4004/i4004.h",
	}
end

if (CPUS["I4004"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i4004/4004dasm.cpp")
end

--------------------------------------------------
-- Intel 8008
--@src/devices/cpu/i8008/i8008.h,CPUS["I8008"] = true
--------------------------------------------------

if (CPUS["I8008"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i8008/i8008.cpp",
		MAME_DIR .. "src/devices/cpu/i8008/i8008.h",
	}
end

if (CPUS["I8008"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8008/8008dasm.cpp")
end

--------------------------------------------------
--  National Semiconductor SC/MP
--@src/devices/cpu/scmp/scmp.h,CPUS["SCMP"] = true
--------------------------------------------------

if (CPUS["SCMP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/scmp/scmp.cpp",
		MAME_DIR .. "src/devices/cpu/scmp/scmp.h",
	}
end

if (CPUS["SCMP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/scmp/scmpdasm.cpp")
end

--------------------------------------------------
-- Intel 8080/8085A
--@src/devices/cpu/i8085/i8085.h,CPUS["I8085"] = true
--------------------------------------------------

if (CPUS["I8085"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i8085/i8085.cpp",
		MAME_DIR .. "src/devices/cpu/i8085/i8085.h",
		MAME_DIR .. "src/devices/cpu/i8085/i8085cpu.h",
	}
end

if (CPUS["I8085"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8085/8085dasm.cpp")
end

--------------------------------------------------
-- Intel 8089
--@src/devices/cpu/i8089/i8089.h,CPUS["I8089"] = true
--------------------------------------------------

if (CPUS["I8089"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i8089/i8089.cpp",
		MAME_DIR .. "src/devices/cpu/i8089/i8089.h",
		MAME_DIR .. "src/devices/cpu/i8089/i8089_channel.cpp",
		MAME_DIR .. "src/devices/cpu/i8089/i8089_channel.h",
		MAME_DIR .. "src/devices/cpu/i8089/i8089_ops.cpp",
	}
end

if (CPUS["I8089"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8089/i8089_dasm.cpp")
end

--------------------------------------------------
-- Intel MCS-48 (8039 and derivatives)
--@src/devices/cpu/mcs48/mcs48.h,CPUS["MCS48"] = true
--------------------------------------------------

if (CPUS["MCS48"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mcs48/mcs48.cpp",
		MAME_DIR .. "src/devices/cpu/mcs48/mcs48.h",
	}
end

if (CPUS["MCS48"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mcs48/mcs48dsm.cpp")
end

--------------------------------------------------
-- Intel 8051 and derivatives
--@src/devices/cpu/mcs51/mcs51.h,CPUS["MCS51"] = true
--------------------------------------------------

if (CPUS["MCS51"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mcs51/mcs51.cpp",
		MAME_DIR .. "src/devices/cpu/mcs51/mcs51.h",
		MAME_DIR .. "src/devices/cpu/mcs51/mcs51ops.inc",
	}
end

if (CPUS["MCS51"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mcs51/mcs51dasm.cpp")
end

--------------------------------------------------
-- Intel MCS-96
--@src/devices/cpu/mcs96/mcs96.h,CPUS["MCS96"] = true
--------------------------------------------------

if (CPUS["MCS96"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mcs96/mcs96.cpp",
		MAME_DIR .. "src/devices/cpu/mcs96/mcs96.h",
		MAME_DIR .. "src/devices/cpu/mcs96/i8x9x.cpp",
		MAME_DIR .. "src/devices/cpu/mcs96/i8x9x.h",
		MAME_DIR .. "src/devices/cpu/mcs96/i8xc196.cpp",
		MAME_DIR .. "src/devices/cpu/mcs96/i8xc196.h",
	}
	dependency {
		{ MAME_DIR .. "src/devices/cpu/mcs96/mcs96.cpp",   GEN_DIR .. "emu/cpu/mcs96/mcs96.inc" },
		{ MAME_DIR .. "src/devices/cpu/mcs96/i8x9x.cpp",   GEN_DIR .. "emu/cpu/mcs96/i8x9x.inc" },
		{ MAME_DIR .. "src/devices/cpu/mcs96/i8xc196.cpp", GEN_DIR .. "emu/cpu/mcs96/i8xc196.inc" },
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
		MAME_DIR .. "src/devices/cpu/i86/i86.cpp",
		MAME_DIR .. "src/devices/cpu/i86/i86.h",
		MAME_DIR .. "src/devices/cpu/i86/i186.cpp",
		MAME_DIR .. "src/devices/cpu/i86/i186.h",
		MAME_DIR .. "src/devices/cpu/i86/i286.cpp",
		MAME_DIR .. "src/devices/cpu/i86/i286.h",
		MAME_DIR .. "src/devices/cpu/i86/i86inline.h",
	}
end

if (CPUS["I86"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i386/i386dasm.cpp")
end

if (CPUS["I386"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i386/i386.cpp",
		MAME_DIR .. "src/devices/cpu/i386/i386.h",
		MAME_DIR .. "src/devices/cpu/i386/cycles.h",
		MAME_DIR .. "src/devices/cpu/i386/i386op16.inc",
		MAME_DIR .. "src/devices/cpu/i386/i386op32.inc",
		MAME_DIR .. "src/devices/cpu/i386/i386ops.h",
		MAME_DIR .. "src/devices/cpu/i386/i386ops.inc",
		MAME_DIR .. "src/devices/cpu/i386/i386priv.h",
		MAME_DIR .. "src/devices/cpu/i386/i486ops.inc",
		MAME_DIR .. "src/devices/cpu/i386/pentops.inc",
		MAME_DIR .. "src/devices/cpu/i386/x87ops.inc",
	}
end

if (CPUS["I386"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i386/i386dasm.cpp")
end

--------------------------------------------------
-- Intel i860
--@src/devices/cpu/i860/i860.h,CPUS["I860"] = true
--------------------------------------------------

if (CPUS["I860"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i860/i860.cpp",
		MAME_DIR .. "src/devices/cpu/i860/i860.h",
		--MAME_DIR .. "src/devices/cpu/i860/i860dasm.cpp",
		MAME_DIR .. "src/devices/cpu/i860/i860dec.inc",
	}
end

if (CPUS["I860"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i860/i860dis.cpp")
end

--------------------------------------------------
-- Intel i960
--@src/devices/cpu/i960/i960.h,CPUS["I960"] = true
--------------------------------------------------

if (CPUS["I960"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/i960/i960.cpp",
		MAME_DIR .. "src/devices/cpu/i960/i960.h",		
	}
end

if (CPUS["I960"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i960/i960dis.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i960/i960dis.h")
end

--------------------------------------------------
-- LH5801
--@src/devices/cpu/lh5801/lh5801.h,CPUS["LH5801"] = true
--------------------------------------------------

if (CPUS["LH5801"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/lh5801/lh5801.cpp",
		MAME_DIR .. "src/devices/cpu/lh5801/lh5801.h",
		MAME_DIR .. "src/devices/cpu/lh5801/5801tbl.inc",
	}
end

if (CPUS["LH5801"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lh5801/5801dasm.cpp")
end
--------
------------------------------------------
-- Manchester Small-Scale Experimental Machine
--@src/devices/cpu/ssem/ssem.h,CPUS["SSEM"] = true
--------------------------------------------------

if (CPUS["SSEM"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/ssem/ssem.cpp",
		MAME_DIR .. "src/devices/cpu/ssem/ssem.h",
	}
end

if (CPUS["SSEM"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ssem/ssemdasm.cpp")
end

--------------------------------------------------
-- Fujitsu MB88xx
--@src/devices/cpu/mb88xx/mb88xx.h,CPUS["MB88XX"] = true
--------------------------------------------------

if (CPUS["MB88XX"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mb88xx/mb88xx.cpp",
		MAME_DIR .. "src/devices/cpu/mb88xx/mb88xx.h",
	}
end

if (CPUS["MB88XX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb88xx/mb88dasm.cpp")
end

--------------------------------------------------
-- Fujitsu MB86233
--@src/devices/cpu/mb86233/mb86233.h,CPUS["MB86233"] = true
--------------------------------------------------

if (CPUS["MB86233"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mb86233/mb86233.cpp",
		MAME_DIR .. "src/devices/cpu/mb86233/mb86233.h",
	}
end

if (CPUS["MB86233"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb86233/mb86233d.cpp")
end

--------------------------------------------------
-- Fujitsu MB86235
--@src/devices/cpu/mb86233/mb86235.h,CPUS["MB86235"] = true
--------------------------------------------------

if (CPUS["MB86235"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mb86235/mb86235.cpp",
		MAME_DIR .. "src/devices/cpu/mb86235/mb86235.h",
	}
end

if (CPUS["MB86235"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb86235/mb86235d.cpp")
end

--------------------------------------------------
-- Microchip PIC16C5x
--@src/devices/cpu/pic16c5x/pic16c5x.h,CPUS["PIC16C5X"] = true
--------------------------------------------------

if (CPUS["PIC16C5X"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/pic16c5x/pic16c5x.cpp",
		MAME_DIR .. "src/devices/cpu/pic16c5x/pic16c5x.h",
		--MAME_DIR .. "src/devices/cpu/pic16c5x/dis16c5x.cpp",
	}
end

if (CPUS["PIC16C5X"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16c5x/16c5xdsm.cpp")
end

--------------------------------------------------
-- Microchip PIC16C62x
--@src/devices/cpu/pic16c62x/pic16c62x.h,CPUS["PIC16C62X"] = true
--------------------------------------------------

if (CPUS["PIC16C62X"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/pic16c62x/pic16c62x.cpp",
		MAME_DIR .. "src/devices/cpu/pic16c62x/pic16c62x.h",
		--MAME_DIR .. "src/devices/cpu/pic16c62x/dis16c62x.cpp",
	}
end

if (CPUS["PIC16C62X"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16c62x/16c62xdsm.cpp")
end

--------------------------------------------------
-- MIPS R3000 (MIPS I/II) series
-- MIPS R4000 (MIPS III/IV) series
--@src/devices/cpu/mips/mips3.h,CPUS["MIPS"] = true
--------------------------------------------------

if (CPUS["MIPS"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mips/r3000.cpp",
		MAME_DIR .. "src/devices/cpu/mips/r3000.h",
		MAME_DIR .. "src/devices/cpu/mips/mips3com.cpp",
		MAME_DIR .. "src/devices/cpu/mips/mips3com.h",
		MAME_DIR .. "src/devices/cpu/mips/mips3.cpp",
		MAME_DIR .. "src/devices/cpu/mips/mips3.h",
		MAME_DIR .. "src/devices/cpu/mips/mips3fe.cpp",
		MAME_DIR .. "src/devices/cpu/mips/mips3fe.h",
		MAME_DIR .. "src/devices/cpu/mips/mips3drc.cpp",
	}
end

if (CPUS["MIPS"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mips/r3kdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mips/mips3dsm.cpp")
end

--------------------------------------------------
-- Sony PlayStation CPU (R3000-based + GTE)
--@src/devices/cpu/psx/psx.h,CPUS["PSX"] = true
--------------------------------------------------

if (CPUS["PSX"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/psx/psx.cpp",
		MAME_DIR .. "src/devices/cpu/psx/psx.h",
		MAME_DIR .. "src/devices/cpu/psx/gte.cpp",
		MAME_DIR .. "src/devices/cpu/psx/gte.h",
		MAME_DIR .. "src/devices/cpu/psx/dma.cpp",
		MAME_DIR .. "src/devices/cpu/psx/dma.h",
		MAME_DIR .. "src/devices/cpu/psx/irq.cpp",
		MAME_DIR .. "src/devices/cpu/psx/irq.h",
		MAME_DIR .. "src/devices/cpu/psx/mdec.cpp",
		MAME_DIR .. "src/devices/cpu/psx/mdec.h",
		MAME_DIR .. "src/devices/cpu/psx/rcnt.cpp",
		MAME_DIR .. "src/devices/cpu/psx/rcnt.h",
		MAME_DIR .. "src/devices/cpu/psx/sio.cpp",
		MAME_DIR .. "src/devices/cpu/psx/sio.h",
		--MAME_DIR .. "src/devices/cpu/psx/dismips.cpp",
	}
end

if (CPUS["PSX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/psx/psxdasm.cpp")
end

--------------------------------------------------
-- Mitsubishi MELPS 4 series
--@src/devices/cpu/melps4/melps4.h,CPUS["MELPS4"] = true
--------------------------------------------------

if (CPUS["MELPS4"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/melps4/melps4.cpp",
		MAME_DIR .. "src/devices/cpu/melps4/melps4.h",
		MAME_DIR .. "src/devices/cpu/melps4/melps4op.cpp",
		MAME_DIR .. "src/devices/cpu/melps4/m58846.cpp",
		MAME_DIR .. "src/devices/cpu/melps4/m58846.h",
	}
end

if (CPUS["MELPS4"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/melps4/melps4d.cpp")
end

--------------------------------------------------
-- Mitsubishi M37702 and M37710 (based on 65C816)
--@src/devices/cpu/m37710/m37710.h,CPUS["M37710"] = true
--------------------------------------------------

if (CPUS["M37710"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/m37710/m37710.cpp",
		MAME_DIR .. "src/devices/cpu/m37710/m37710.h",
		MAME_DIR .. "src/devices/cpu/m37710/m37710o0.cpp",
		MAME_DIR .. "src/devices/cpu/m37710/m37710o1.cpp",
		MAME_DIR .. "src/devices/cpu/m37710/m37710o2.cpp",
		MAME_DIR .. "src/devices/cpu/m37710/m37710o3.cpp",
		MAME_DIR .. "src/devices/cpu/m37710/m37710cm.h",
		MAME_DIR .. "src/devices/cpu/m37710/m37710il.h",
		MAME_DIR .. "src/devices/cpu/m37710/m37710op.h",
		MAME_DIR .. "src/devices/cpu/m37710/m7700ds.h",	
	}
end

if (CPUS["M37710"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m37710/m7700ds.cpp")
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
		MAME_DIR .. "src/devices/cpu/m6502/deco16.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/deco16.h",
		MAME_DIR .. "src/devices/cpu/m6502/m4510.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m4510.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6502.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m6502.h",
		MAME_DIR .. "src/devices/cpu/m6502/m65c02.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m65c02.h",
		MAME_DIR .. "src/devices/cpu/m6502/m65ce02.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m65ce02.h",
		MAME_DIR .. "src/devices/cpu/m6502/m65sc02.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m65sc02.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6504.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m6504.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6507.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m6507.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6509.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m6509.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6510.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m6510.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6510t.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m6510t.h",
		MAME_DIR .. "src/devices/cpu/m6502/m7501.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m7501.h",
		MAME_DIR .. "src/devices/cpu/m6502/m8502.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m8502.h",
		MAME_DIR .. "src/devices/cpu/m6502/n2a03.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/n2a03.h",
		MAME_DIR .. "src/devices/cpu/m6502/r65c02.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/r65c02.h",
		MAME_DIR .. "src/devices/cpu/m6502/m740.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m740.h",
		MAME_DIR .. "src/devices/cpu/m6502/m3745x.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m3745x.h",
		MAME_DIR .. "src/devices/cpu/m6502/m5074x.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m5074x.h",
	}
	
	dependency {
		{ MAME_DIR .. "src/devices/cpu/m6502/deco16.cpp",   GEN_DIR .. "emu/cpu/m6502/deco16.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m4510.cpp",    GEN_DIR .. "emu/cpu/m6502/m4510.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m6502.cpp",    GEN_DIR .. "emu/cpu/m6502/m6502.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m65c02.cpp",   GEN_DIR .. "emu/cpu/m6502/m65c02.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m65ce02.cpp",  GEN_DIR .. "emu/cpu/m6502/m65ce02.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m6509.cpp",    GEN_DIR .. "emu/cpu/m6502/m6509.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m6510.cpp",    GEN_DIR .. "emu/cpu/m6502/m6510.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/n2a03.cpp",    GEN_DIR .. "emu/cpu/m6502/n2a03.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/r65c02.cpp",   GEN_DIR .. "emu/cpu/m6502/r65c02.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m740.cpp",     GEN_DIR .. "emu/cpu/m6502/m740.inc" },
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
		MAME_DIR .. "src/devices/cpu/m6800/m6800.cpp",
		MAME_DIR .. "src/devices/cpu/m6800/m6800.h",
		MAME_DIR .. "src/devices/cpu/m6800/6800ops.inc",
		MAME_DIR .. "src/devices/cpu/m6800/6800tbl.inc",
	}
end

if (CPUS["M6800"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6800/6800dasm.cpp")
end

--------------------------------------------------
-- Motorola 6805
--@src/devices/cpu/m6805/m6805.h,CPUS["M6805"] = true
--------------------------------------------------

if (CPUS["M6805"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/m6805/m6805.cpp",
		MAME_DIR .. "src/devices/cpu/m6805/m6805.h",
		MAME_DIR .. "src/devices/cpu/m6805/6805ops.inc",
	}
end

if (CPUS["M6805"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6805/6805dasm.cpp")
end

--------------------------------------------------
-- Motorola 6809
--@src/devices/cpu/m6809/m6809.h,CPUS["M6809"] = true
--@src/devices/cpu/m6809/hd6309.h,CPUS["M6809"] = true
--@src/devices/cpu/m6809/konami.h,CPUS["M6809"] = true
--------------------------------------------------

if (CPUS["M6809"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/m6809/m6809.cpp",
		MAME_DIR .. "src/devices/cpu/m6809/m6809.h",
		MAME_DIR .. "src/devices/cpu/m6809/hd6309.cpp",
		MAME_DIR .. "src/devices/cpu/m6809/hd6309.h",
		MAME_DIR .. "src/devices/cpu/m6809/konami.cpp",
		MAME_DIR .. "src/devices/cpu/m6809/konami.h",
		MAME_DIR .. "src/devices/cpu/m6809/m6809inl.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/cpu/m6809/m6809.cpp",   GEN_DIR .. "emu/cpu/m6809/m6809.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6809/hd6309.cpp",  GEN_DIR .. "emu/cpu/m6809/hd6309.inc" },
		{ MAME_DIR .. "src/devices/cpu/m6809/konami.cpp",  GEN_DIR .. "emu/cpu/m6809/konami.inc" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/m6809/m6809.ops"  , GEN_DIR .. "emu/cpu/m6809/m6809.inc",   { MAME_DIR .. "src/devices/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/devices/cpu/m6809/base6x09.ops"  }, {"@echo Generating m6809 source file...", PYTHON .. " $(1) $(<) > $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6809/hd6309.ops" , GEN_DIR .. "emu/cpu/m6809/hd6309.inc",  { MAME_DIR .. "src/devices/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/devices/cpu/m6809/base6x09.ops"  }, {"@echo Generating hd6309 source file...", PYTHON .. " $(1) $(<) > $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6809/konami.ops" , GEN_DIR .. "emu/cpu/m6809/konami.inc",  { MAME_DIR .. "src/devices/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/devices/cpu/m6809/base6x09.ops"  }, {"@echo Generating konami source file...", PYTHON .. " $(1) $(<) > $(@)" }},
	}
end

if (CPUS["M6809"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6809/6809dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6809/6309dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6809/knmidasm.cpp")
end

--------------------------------------------------
-- Motorola 68HC11
--@src/devices/cpu/mc68hc11/mc68hc11.h,CPUS["MC68HC11"] = true
--------------------------------------------------

if (CPUS["MC68HC11"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mc68hc11/mc68hc11.cpp",
		MAME_DIR .. "src/devices/cpu/mc68hc11/mc68hc11.h",
		MAME_DIR .. "src/devices/cpu/mc68hc11/hc11ops.h",
		MAME_DIR .. "src/devices/cpu/mc68hc11/hc11ops.inc",
	}
end

if (CPUS["MC68HC11"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mc68hc11/hc11dasm.cpp")
end

--------------------------------------------------
-- Motorola 68000 series
--@src/devices/cpu/m68000/m68000.h,CPUS["M680X0"] = true
--------------------------------------------------

if (CPUS["M680X0"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/m68000/m68kcpu.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68kcpu.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68kops.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68kops.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68000.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68kfpu.inc",
		--MAME_DIR .. "src/devices/cpu/m68000/m68kmake.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68kmmu.h",
		--MAME_DIR .. "src/devices/cpu/m68000/m68k_in.cpp",
	}
end

if (CPUS["M680X0"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m68000/m68kdasm.cpp")
end

--------------------------------------------------
-- Motorola/Freescale dsp56k
--@src/devices/cpu/dsp56k/dsp56k.h,CPUS["DSP56156"] = true
--------------------------------------------------

if (CPUS["DSP56156"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56k.cpp",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56k.h",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56mem.cpp",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56mem.h",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56pcu.cpp",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56pcu.h",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56def.h",
		MAME_DIR .. "src/devices/cpu/dsp56k/dsp56ops.inc",
		MAME_DIR .. "src/devices/cpu/dsp56k/inst.h",
		MAME_DIR .. "src/devices/cpu/dsp56k/opcode.h",
		MAME_DIR .. "src/devices/cpu/dsp56k/pmove.h",
		MAME_DIR .. "src/devices/cpu/dsp56k/tables.h",
	}
end

if (CPUS["DSP56156"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56k/dsp56dsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56k/opcode.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56k/inst.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56k/pmove.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56k/tables.cpp")
end

--------------------------------------------------
-- PDP-1
-- TX0
--@src/devices/cpu/pdp1/pdp1.h,CPUS["PDP1"] = true
--@src/devices/cpu/pdp1/tx0.h,CPUS["PDP1"] = true
--------------------------------------------------

if (CPUS["PDP1"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/pdp1/pdp1.cpp",
		MAME_DIR .. "src/devices/cpu/pdp1/pdp1.h",
		MAME_DIR .. "src/devices/cpu/pdp1/tx0.cpp",
		MAME_DIR .. "src/devices/cpu/pdp1/tx0.h",
	}
end

if (CPUS["PDP1"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pdp1/pdp1dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pdp1/tx0dasm.cpp")
end

--------------------------------------------------
-- PATINHO FEIO - Escola Politecnica - USP (Brazil)
--@src/devices/cpu/patinhofeio/patinho_feio.h,CPUS["PATINHOFEIO"] = true
--------------------------------------------------

if (CPUS["PATINHOFEIO"]~=null) then
    files {
        MAME_DIR .. "src/devices/cpu/patinhofeio/patinho_feio.cpp",
        MAME_DIR .. "src/devices/cpu/patinhofeio/patinho_feio.h",
    }
end

if (CPUS["PATINHOFEIO"]~=null or _OPTIONS["with-tools"]) then
    table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/patinhofeio/patinho_feio_dasm.cpp")
end

--------------------------------------------------
-- Motorola PowerPC series
--@src/devices/cpu/powerpc/ppc.h,CPUS["POWERPC"] = true
--------------------------------------------------

if (CPUS["POWERPC"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/powerpc/ppccom.cpp",
		MAME_DIR .. "src/devices/cpu/powerpc/ppccom.h",
		MAME_DIR .. "src/devices/cpu/powerpc/ppcfe.cpp",
		MAME_DIR .. "src/devices/cpu/powerpc/ppcfe.h",
		MAME_DIR .. "src/devices/cpu/powerpc/ppcdrc.cpp",
		
		--MAME_DIR .. "src/devices/cpu/powerpc/drc_ops.cpp",
		MAME_DIR .. "src/devices/cpu/powerpc/drc_ops.h",
		--MAME_DIR .. "src/devices/cpu/powerpc/ppc.cpp",
		MAME_DIR .. "src/devices/cpu/powerpc/ppc.h",
		MAME_DIR .. "src/devices/cpu/powerpc/ppc403.inc",
		MAME_DIR .. "src/devices/cpu/powerpc/ppc602.inc",
		MAME_DIR .. "src/devices/cpu/powerpc/ppc603.inc",
		MAME_DIR .. "src/devices/cpu/powerpc/ppc_mem.inc",
		MAME_DIR .. "src/devices/cpu/powerpc/ppc_ops.h",
		MAME_DIR .. "src/devices/cpu/powerpc/ppc_ops.inc",
	}
end

if (CPUS["POWERPC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/powerpc/ppc_dasm.cpp")
end

--------------------------------------------------
-- NEC V-series Intel-compatible
--@src/devices/cpu/nec/nec.h,CPUS["NEC"] = true
--@src/devices/cpu/v30mz/v30mz.h,CPUS["V30MZ"] = true
--------------------------------------------------

if (CPUS["NEC"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/nec/nec.cpp",
		MAME_DIR .. "src/devices/cpu/nec/nec.h",
		MAME_DIR .. "src/devices/cpu/nec/necea.h",
		MAME_DIR .. "src/devices/cpu/nec/necinstr.h",
		MAME_DIR .. "src/devices/cpu/nec/necinstr.inc",
		MAME_DIR .. "src/devices/cpu/nec/necmacro.h",
		MAME_DIR .. "src/devices/cpu/nec/necmodrm.h",
		MAME_DIR .. "src/devices/cpu/nec/necpriv.h",
		MAME_DIR .. "src/devices/cpu/nec/v25instr.h",
		MAME_DIR .. "src/devices/cpu/nec/v25instr.inc",
		MAME_DIR .. "src/devices/cpu/nec/v25priv.h",
		MAME_DIR .. "src/devices/cpu/nec/v25.cpp",
		MAME_DIR .. "src/devices/cpu/nec/v25.h",
		MAME_DIR .. "src/devices/cpu/nec/v25sfr.cpp",
		MAME_DIR .. "src/devices/cpu/nec/v53.cpp",
		MAME_DIR .. "src/devices/cpu/nec/v53.h",
	}
end

if (CPUS["NEC"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nec/necdasm.cpp")
end

if (CPUS["V30MZ"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/v30mz/v30mz.cpp",
		MAME_DIR .. "src/devices/cpu/v30mz/v30mz.h",
	}
end

if (CPUS["V30MZ"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nec/necdasm.cpp")
end

--------------------------------------------------
-- NEC V60/V70
--@src/devices/cpu/v60/v60.h,CPUS["V60"] = true
--------------------------------------------------

if (CPUS["V60"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/v60/v60.cpp",
		MAME_DIR .. "src/devices/cpu/v60/v60.h",
		MAME_DIR .. "src/devices/cpu/v60/am.inc",
		MAME_DIR .. "src/devices/cpu/v60/am1.inc",
		MAME_DIR .. "src/devices/cpu/v60/am2.inc",
		MAME_DIR .. "src/devices/cpu/v60/am3.inc",
		MAME_DIR .. "src/devices/cpu/v60/op12.inc",
		MAME_DIR .. "src/devices/cpu/v60/op2.inc",
		MAME_DIR .. "src/devices/cpu/v60/op3.inc",
		MAME_DIR .. "src/devices/cpu/v60/op4.inc",
		MAME_DIR .. "src/devices/cpu/v60/op5.inc",
		MAME_DIR .. "src/devices/cpu/v60/op6.inc",
		MAME_DIR .. "src/devices/cpu/v60/op7a.inc",
		MAME_DIR .. "src/devices/cpu/v60/optable.inc",
	}
end

if (CPUS["V60"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v60/v60d.cpp")
end

--------------------------------------------------
-- NEC V810 (uPD70732)
--@src/devices/cpu/v810/v810.h,CPUS["V810"] = true
--------------------------------------------------

if (CPUS["V810"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/v810/v810.cpp",
		MAME_DIR .. "src/devices/cpu/v810/v810.h",
	}
end

if (CPUS["V810"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v810/v810dasm.cpp")
end

--------------------------------------------------
-- NEC uPD7725
--@src/devices/cpu/upd7725/upd7725.h,CPUS["UPD7725"] = true
--------------------------------------------------

if (CPUS["UPD7725"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/upd7725/upd7725.cpp",
		MAME_DIR .. "src/devices/cpu/upd7725/upd7725.h",
	}
end

if (CPUS["UPD7725"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd7725/dasm7725.cpp")
end

--------------------------------------------------
-- NEC uPD7810 series
--@src/devices/cpu/upd7810/upd7810.h,CPUS["UPD7810"] = true
--------------------------------------------------

if (CPUS["UPD7810"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810.cpp",
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810.h",
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810_opcodes.cpp",
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810_table.cpp",
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810_macros.h",
	}
end

if (CPUS["UPD7810"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd7810/upd7810_dasm.cpp")
end

--------------------------------------------------
-- NEC uCOM-4 series
--@src/devices/cpu/ucom4/ucom4.h,CPUS["UCOM4"] = true
--------------------------------------------------

if (CPUS["UCOM4"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/ucom4/ucom4.cpp",
		MAME_DIR .. "src/devices/cpu/ucom4/ucom4.h",
		MAME_DIR .. "src/devices/cpu/ucom4/ucom4op.cpp",
	}
end

if (CPUS["UCOM4"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ucom4/ucom4d.cpp")
end

--------------------------------------------------
-- Nintendo Minx
--@src/devices/cpu/minx/minx.h,CPUS["MINX"] = true
--------------------------------------------------

if (CPUS["MINX"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/minx/minx.cpp",
		MAME_DIR .. "src/devices/cpu/minx/minx.h",
		MAME_DIR .. "src/devices/cpu/minx/minxfunc.h",
		MAME_DIR .. "src/devices/cpu/minx/minxopce.h",
		MAME_DIR .. "src/devices/cpu/minx/minxopcf.h",
		MAME_DIR .. "src/devices/cpu/minx/minxops.h",	
	}
end

if (CPUS["MINX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/minx/minxd.cpp")
end

--------------------------------------------------
-- Nintendo/SGI RSP (R3000-based + vector processing)
--@src/devices/cpu/rsp/rsp.h,CPUS["RSP"] = true
--------------------------------------------------

if (CPUS["RSP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/rsp/rsp.cpp",
		MAME_DIR .. "src/devices/cpu/rsp/rsp.h",
		MAME_DIR .. "src/devices/cpu/rsp/rspdrc.cpp",
		MAME_DIR .. "src/devices/cpu/rsp/rspfe.cpp",
		MAME_DIR .. "src/devices/cpu/rsp/rspfe.h",
		MAME_DIR .. "src/devices/cpu/rsp/rspcp2.cpp",
		MAME_DIR .. "src/devices/cpu/rsp/rspcp2.h",
		MAME_DIR .. "src/devices/cpu/rsp/rspcp2d.cpp",
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
		MAME_DIR .. "src/devices/cpu/rsp/rspdiv.h",
	}
end

if (CPUS["RSP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/rsp/rsp_dasm.cpp")
end

--------------------------------------------------
-- Panasonic MN10200
--@src/devices/cpu/mn10200/mn10200.h,CPUS["MN10200"] = true
--------------------------------------------------

if (CPUS["MN10200"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/mn10200/mn10200.cpp",
		MAME_DIR .. "src/devices/cpu/mn10200/mn10200.h",
	}
end

if (CPUS["MN10200"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mn10200/mn102dis.cpp")
end

--------------------------------------------------
-- Saturn
--@src/devices/cpu/saturn/saturn.h,CPUS["SATURN"] = true
--------------------------------------------------

if (CPUS["SATURN"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/saturn/saturn.cpp",
		MAME_DIR .. "src/devices/cpu/saturn/saturn.h",
		MAME_DIR .. "src/devices/cpu/saturn/satops.inc",
		MAME_DIR .. "src/devices/cpu/saturn/sattable.inc",		
	}
end

if (CPUS["SATURN"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/saturn/saturnds.cpp")
end

--------------------------------------------------
-- Sharp SM510 series
--@src/devices/cpu/sm510/sm510.h,CPUS["SM510"] = true
--------------------------------------------------

if (CPUS["SM510"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sm510/sm510.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm510.h",
		MAME_DIR .. "src/devices/cpu/sm510/sm510op.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm510core.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm511core.cpp",
	}
end

if (CPUS["SM510"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sm510/sm510d.cpp")
end

--------------------------------------------------
-- Sharp SM8500
--@src/devices/cpu/sm8500/sm8500.h,CPUS["SM8500"] = true
--------------------------------------------------

if (CPUS["SM8500"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sm8500/sm8500.cpp",
		MAME_DIR .. "src/devices/cpu/sm8500/sm8500.h",
		MAME_DIR .. "src/devices/cpu/sm8500/sm85ops.h",
	}
end

if (CPUS["SM8500"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sm8500/sm8500d.cpp")
end

--------------------------------------------------
-- Signetics 2650
--@src/devices/cpu/s2650/s2650.h,CPUS["S2650"] = true
--------------------------------------------------

if (CPUS["S2650"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/s2650/s2650.cpp",
		MAME_DIR .. "src/devices/cpu/s2650/s2650.h",
		MAME_DIR .. "src/devices/cpu/s2650/s2650cpu.h",
	}
end

if (CPUS["S2650"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/s2650/2650dasm.cpp")
end

--------------------------------------------------
-- SC61860
--@src/devices/cpu/sc61860/sc61860.h,CPUS["SC61860"] = true
--------------------------------------------------

if (CPUS["SC61860"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/sc61860/sc61860.cpp",
		MAME_DIR .. "src/devices/cpu/sc61860/sc61860.h",
		--MAME_DIR .. "src/devices/cpu/sc61860/readpc.cpp",
		MAME_DIR .. "src/devices/cpu/sc61860/scops.inc",
		MAME_DIR .. "src/devices/cpu/sc61860/sctable.inc",
	}
end

if (CPUS["SC61860"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sc61860/scdasm.cpp")
end

--------------------------------------------------
-- Sony/Nintendo SPC700
--@src/devices/cpu/spc700/spc700.h,CPUS["SPC700"] = true
--------------------------------------------------

if (CPUS["SPC700"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/spc700/spc700.cpp",
		MAME_DIR .. "src/devices/cpu/spc700/spc700.h",
		MAME_DIR .. "src/devices/cpu/spc700/spc700ds.h",
	}
end

if (CPUS["SPC700"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/spc700/spc700ds.cpp")
end

--------------------------------------------------
-- SSP1601
--@src/devices/cpu/ssp1601/ssp1601.h,CPUS["SSP1601"] = true
--------------------------------------------------

if (CPUS["SSP1601"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/ssp1601/ssp1601.cpp",
		MAME_DIR .. "src/devices/cpu/ssp1601/ssp1601.h",
	}
end

if (CPUS["SSP1601"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ssp1601/ssp1601d.cpp")
end

--------------------------------------------------
-- SunPlus u'nSP
--@src/devices/cpu/unsp/unsp.h,CPUS["UNSP"] = true
--------------------------------------------------

if (CPUS["UNSP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/unsp/unsp.cpp",
		MAME_DIR .. "src/devices/cpu/unsp/unsp.h",
	}
end

if (CPUS["UNSP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/unsp/unspdasm.cpp")
end

--------------------------------------------------
-- Atmel 8-bit AVR
--@src/devices/cpu/avr8/avr8.h,CPUS["AVR8"] = true
--------------------------------------------------

if (CPUS["AVR8"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/avr8/avr8.cpp",
		MAME_DIR .. "src/devices/cpu/avr8/avr8.h",
	}
end

if (CPUS["AVR8"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/avr8/avr8dasm.cpp")
end

--------------------------------------------------
-- Texas Instruments TMS0980
--@src/devices/cpu/tms0980/tms0980.h,CPUS["TMS0980"] = true
--------------------------------------------------

if (CPUS["TMS0980"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms0980/tms0980.cpp",
		MAME_DIR .. "src/devices/cpu/tms0980/tms0980.h",
	}
end

if (CPUS["TMS0980"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms0980/tms0980d.cpp")
end

--------------------------------------------------
-- Texas Instruments TMS7000 series
--@src/devices/cpu/tms7000/tms7000.h,CPUS["TMS7000"] = true
--------------------------------------------------

if (CPUS["TMS7000"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms7000/tms7000.cpp",
		MAME_DIR .. "src/devices/cpu/tms7000/tms7000.h",
		MAME_DIR .. "src/devices/cpu/tms7000/tms70op.inc",
	}
end

if (CPUS["TMS7000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms7000/7000dasm.cpp")
end

--------------------------------------------------
-- Texas Instruments TMS99xx series
--@src/devices/cpu/tms9900/tms9900.h,CPUS["TMS9900"] = true
--@src/devices/cpu/tms9900/tms9980a.h,CPUS["TMS9900"] = true
--@src/devices/cpu/tms9900/tms9995.h,CPUS["TMS9900"] = true

--------------------------------------------------

if (CPUS["TMS9900"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms9900/tms9900.cpp",
		MAME_DIR .. "src/devices/cpu/tms9900/tms9900.h",
		MAME_DIR .. "src/devices/cpu/tms9900/tms9980a.cpp",
		MAME_DIR .. "src/devices/cpu/tms9900/tms9980a.h",
		MAME_DIR .. "src/devices/cpu/tms9900/tms9995.cpp",
		MAME_DIR .. "src/devices/cpu/tms9900/tms9995.h",
		MAME_DIR .. "src/devices/cpu/tms9900/ti990_10.cpp",
		MAME_DIR .. "src/devices/cpu/tms9900/ti990_10.h",
		MAME_DIR .. "src/devices/cpu/tms9900/tms99com.h",
	}
end

if (CPUS["TMS9900"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms9900/9900dasm.cpp")
end

--------------------------------------------------
-- Texas Instruments TMS340x0 graphics controllers
--@src/devices/cpu/tms34010/tms34010.h,CPUS["TMS340X0"] = true
--------------------------------------------------

if (CPUS["TMS340X0"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms34010/tms34010.cpp",
		MAME_DIR .. "src/devices/cpu/tms34010/tms34010.h",
		MAME_DIR .. "src/devices/cpu/tms34010/34010fld.inc",
		MAME_DIR .. "src/devices/cpu/tms34010/34010gfx.inc",
		MAME_DIR .. "src/devices/cpu/tms34010/34010ops.h",
		MAME_DIR .. "src/devices/cpu/tms34010/34010ops.inc",
		MAME_DIR .. "src/devices/cpu/tms34010/34010tbl.inc",
		--MAME_DIR .. "src/devices/cpu/tms34010/dis34010.cpp",
	}
end

if (CPUS["TMS340X0"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms34010/34010dsm.cpp")
end

--------------------------------------------------
-- Texas Instruments TMS3201x DSP
--@src/devices/cpu/tms32010/tms32010.h,CPUS["TMS32010"] = true
--------------------------------------------------

if (CPUS["TMS32010"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms32010/tms32010.cpp",
		MAME_DIR .. "src/devices/cpu/tms32010/tms32010.h",
		--MAME_DIR .. "src/devices/cpu/tms32010/dis32010.cpp",
	}
end

if (CPUS["TMS32010"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32010/32010dsm.cpp")
end

--------------------------------------------------
-- Texas Instruments TMS3202x DSP
--@src/devices/cpu/tms32025/tms32025.h,CPUS["TMS32025"] = true
--------------------------------------------------

if (CPUS["TMS32025"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms32025/tms32025.cpp",
		MAME_DIR .. "src/devices/cpu/tms32025/tms32025.h",
		--MAME_DIR .. "src/devices/cpu/tms32025/dis32025.cpp",
	}
end

if (CPUS["TMS32025"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32025/32025dsm.cpp")
end

--------------------------------------------------
-- Texas Instruments TMS3203x DSP
--@src/devices/cpu/tms32031/tms32031.h,CPUS["TMS32031"] = true
--------------------------------------------------

if (CPUS["TMS32031"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms32031/tms32031.cpp",
		MAME_DIR .. "src/devices/cpu/tms32031/tms32031.h",
		MAME_DIR .. "src/devices/cpu/tms32031/32031ops.inc",
	}
end

if (CPUS["TMS32031"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32031/dis32031.cpp")
end

--------------------------------------------------
-- Texas Instruments TMS3205x DSP
--@src/devices/cpu/tms32051/tms32051.h,CPUS["TMS32051"] = true
--------------------------------------------------

if (CPUS["TMS32051"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms32051/tms32051.cpp",
		MAME_DIR .. "src/devices/cpu/tms32051/tms32051.h",
		MAME_DIR .. "src/devices/cpu/tms32051/32051ops.h",
		MAME_DIR .. "src/devices/cpu/tms32051/32051ops.inc",
	}
end

if (CPUS["TMS32051"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32051/dis32051.cpp")
end

--------------------------------------------------
-- Texas Instruments TMS3208x DSP
--@src/devices/cpu/tms32082/tms32082.h,CPUS["TMS32082"] = true
--------------------------------------------------

if (CPUS["TMS32082"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms32082/tms32082.cpp",
		MAME_DIR .. "src/devices/cpu/tms32082/tms32082.h",
		MAME_DIR .. "src/devices/cpu/tms32082/mp_ops.cpp",
	}
end

if (CPUS["TMS32082"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32082/dis_mp.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32082/dis_pp.cpp")
end

--------------------------------------------------
-- Texas Instruments TMS57002 DSP
--@src/devices/cpu/tms57002/tms57002.h,CPUS["TMS57002"] = true
--------------------------------------------------

if (CPUS["TMS57002"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tms57002/tms57002.cpp",
		MAME_DIR .. "src/devices/cpu/tms57002/tms57002.h",
		MAME_DIR .. "src/devices/cpu/tms57002/tms57kdec.cpp",
	}
	dependency {
		{ MAME_DIR .. "src/devices/cpu/tms57002/tms57kdec.cpp", GEN_DIR .. "emu/cpu/tms57002/tms57002.inc" },
		{ MAME_DIR .. "src/devices/cpu/tms57002/tms57002.cpp",  GEN_DIR .. "emu/cpu/tms57002/tms57002.inc" },
	}
	custombuildtask { 	
		{ MAME_DIR .. "src/devices/cpu/tms57002/tmsinstr.lst" , GEN_DIR .. "emu/cpu/tms57002/tms57002.inc",   { MAME_DIR .. "src/devices/cpu/tms57002/tmsmake.py" }, {"@echo Generating TMS57002 source file...", PYTHON .. " $(1) $(<) $(@)" } }
	}
end

if (CPUS["TMS57002"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms57002/57002dsm.cpp")
	table.insert(disasm_dependency , { MAME_DIR .. "src/devices/cpu/tms57002/57002dsm.cpp",  GEN_DIR .. "emu/cpu/tms57002/tms57002.inc" } )
	table.insert(disasm_custombuildtask , { MAME_DIR .. "src/devices/cpu/tms57002/tmsinstr.lst" , GEN_DIR .. "emu/cpu/tms57002/tms57002.inc",   { MAME_DIR .. "src/devices/cpu/tms57002/tmsmake.py" }, {"@echo Generating TMS57002 source file...", PYTHON .. " $(1) $(<) $(@)" }})
end

--------------------------------------------------
-- Toshiba TLCS-90 Series
--@src/devices/cpu/tlcs90/tlcs90.h,CPUS["TLCS90"] = true
--------------------------------------------------

if (CPUS["TLCS90"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tlcs90/tlcs90.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs90/tlcs90.h",
	}
end

--------------------------------------------------
-- Toshiba TLCS-900 Series
--@src/devices/cpu/tlcs900/tlcs900.h,CPUS["TLCS900"] = true
--------------------------------------------------

if (CPUS["TLCS900"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/tlcs900/tlcs900.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs900/tlcs900.h",
		MAME_DIR .. "src/devices/cpu/tlcs900/900tbl.inc",
	}
end

if (CPUS["TLCS900"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tlcs900/dasm900.cpp")
end

--------------------------------------------------
-- Zilog Z80
--@src/devices/cpu/z80/z80.h,CPUS["Z80"] = true
--@src/devices/cpu/z80/z80daisy.h,CPUS["Z80"] = true
--------------------------------------------------

if (CPUS["Z80"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/z80/z80.cpp",
		MAME_DIR .. "src/devices/cpu/z80/z80.h",
		MAME_DIR .. "src/devices/cpu/z80/z80daisy.cpp",
		MAME_DIR .. "src/devices/cpu/z80/z80daisy.h",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c011.cpp",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c011.h",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c015.cpp",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c015.h",
		MAME_DIR .. "src/devices/cpu/z80/kl5c80a12.cpp",
		MAME_DIR .. "src/devices/cpu/z80/kl5c80a12.h",
	}
end

if (CPUS["Z80"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z80/z80dasm.cpp")
end

--------------------------------------------------
-- Sharp LR35902 (Game Boy CPU)
--@src/devices/cpu/lr35902/lr35902.h,CPUS["LR35902"] = true
--------------------------------------------------

if (CPUS["LR35902"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/lr35902/lr35902.cpp",
		MAME_DIR .. "src/devices/cpu/lr35902/lr35902.h",
		MAME_DIR .. "src/devices/cpu/lr35902/opc_cb.inc",
		MAME_DIR .. "src/devices/cpu/lr35902/opc_main.inc",
	}
end

if (CPUS["LR35902"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lr35902/lr35902d.cpp")
end

--------------------------------------------------
-- Zilog Z180
--@src/devices/cpu/z180/z180.h,CPUS["Z180"] = true
--------------------------------------------------

if (CPUS["Z180"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/z180/z180.cpp",
		MAME_DIR .. "src/devices/cpu/z180/z180.h",
		MAME_DIR .. "src/devices/cpu/z180/z180cb.inc",
		MAME_DIR .. "src/devices/cpu/z180/z180dd.inc",
		MAME_DIR .. "src/devices/cpu/z180/z180ed.inc",
		MAME_DIR .. "src/devices/cpu/z180/z180fd.inc",
		MAME_DIR .. "src/devices/cpu/z180/z180op.inc",
		MAME_DIR .. "src/devices/cpu/z180/z180ops.h",
		MAME_DIR .. "src/devices/cpu/z180/z180tbl.h",
		MAME_DIR .. "src/devices/cpu/z180/z180xy.inc",
		MAME_DIR .. "src/devices/cpu/z80/z80daisy.cpp",
		MAME_DIR .. "src/devices/cpu/z80/z80daisy.h",
	}
end

if (CPUS["Z180"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z180/z180dasm.cpp")
end

--------------------------------------------------
-- Zilog Z8000
--@src/devices/cpu/z8000/z8000.h,CPUS["Z8000"] = true
--------------------------------------------------

if (CPUS["Z8000"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/z8000/z8000.cpp",
		MAME_DIR .. "src/devices/cpu/z8000/z8000.h",
		--MAME_DIR .. "src/devices/cpu/z8000/makedab.cpp",
		MAME_DIR .. "src/devices/cpu/z8000/z8000cpu.h",
		MAME_DIR .. "src/devices/cpu/z8000/z8000dab.h",
		MAME_DIR .. "src/devices/cpu/z8000/z8000ops.inc",
		MAME_DIR .. "src/devices/cpu/z8000/z8000tbl.inc",
	}
end

if (CPUS["Z8000"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z8000/8000dasm.cpp")
end

--------------------------------------------------
-- Zilog Z8
--@src/devices/cpu/z8/z8.h,CPUS["Z8"] = true
--------------------------------------------------

if (CPUS["Z8"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/z8/z8.cpp",
		MAME_DIR .. "src/devices/cpu/z8/z8.h",
		MAME_DIR .. "src/devices/cpu/z8/z8ops.inc",
	}
end

if (CPUS["Z8"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z8/z8dasm.cpp")
end

--------------------------------------------------
-- Argonaut SuperFX
--@src/devices/cpu/superfx/superfx.h,CPUS["SUPERFX"] = true
--------------------------------------------------

if (CPUS["SUPERFX"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/superfx/superfx.cpp",
		MAME_DIR .. "src/devices/cpu/superfx/superfx.h",
	}
end

if (CPUS["SUPERFX"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/superfx/sfx_dasm.cpp")
end

--------------------------------------------------
-- Rockwell PPS-4
--@src/devices/cpu/pps4/pps4.h,CPUS["PPS4"] = true
--------------------------------------------------

if (CPUS["PPS4"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/pps4/pps4.cpp",
		MAME_DIR .. "src/devices/cpu/pps4/pps4.h",
	}
end

if (CPUS["PPS4"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pps4/pps4dasm.cpp")
end

--------------------------------------------------
-- Hitachi HD61700
--@src/devices/cpu/hd61700/hd61700.h,CPUS["HD61700"] = true
--------------------------------------------------

if (CPUS["HD61700"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/hd61700/hd61700.cpp",
		MAME_DIR .. "src/devices/cpu/hd61700/hd61700.h",
	}
end

if (CPUS["HD61700"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hd61700/hd61700d.cpp")
end

--------------------------------------------------
-- Sanyo LC8670
--@src/devices/cpu/lc8670/lc8670.h,CPUS["LC8670"] = true
--------------------------------------------------

if (CPUS["LC8670"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/lc8670/lc8670.cpp",
		MAME_DIR .. "src/devices/cpu/lc8670/lc8670.h",
	}
end

if (CPUS["LC8670"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lc8670/lc8670dsm.cpp")
end

--------------------------------------------------
-- Sega SCU DSP
--@src/devices/cpu/scudsp/scudsp.h,CPUS["SCUDSP"] = true
--------------------------------------------------

if (CPUS["SCUDSP"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/scudsp/scudsp.cpp",
		MAME_DIR .. "src/devices/cpu/scudsp/scudsp.h",
	}
end

if (CPUS["SCUDSP"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/scudsp/scudspdasm.cpp")
end

--------------------------------------------------
-- Sunplus Technology S+core
--@src/devices/cpu/score/score.h,CPUS["SCORE"] = true
--------------------------------------------------

if (CPUS["SCORE"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/score/score.cpp",
		MAME_DIR .. "src/devices/cpu/score/score.h",
		MAME_DIR .. "src/devices/cpu/score/scorem.h",
	}
end

if (CPUS["SCORE"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/score/scoredsm.cpp")
end

--------------------------------------------------
-- Xerox Alto-II
--@src/devices/cpu/alto2/alto2cpu.h,CPUS["ALTO2"] = true
--------------------------------------------------

if (CPUS["ALTO2"]~=null) then
	files {
		MAME_DIR .. "src/devices/cpu/alto2/alto2cpu.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/alto2cpu.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2disk.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2disk.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2disp.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2disp.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2curt.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2curt.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2dht.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2dht.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2dvt.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2dvt.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2dwt.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2dwt.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2emu.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2emu.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2ether.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2ether.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2hw.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2hw.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2kbd.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2kbd.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2ksec.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2ksec.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2kwd.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2kwd.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2mem.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2mem.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2mouse.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2mouse.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2mrt.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2mrt.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2part.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2part.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2ram.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2ram.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2roms.cpp",
		MAME_DIR .. "src/devices/cpu/alto2/a2roms.h",
		MAME_DIR .. "src/devices/cpu/alto2/a2jkff.h",
	}
end

if (CPUS["ALTO2"]~=null or _OPTIONS["with-tools"]) then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/alto2/alto2dsm.cpp")
end

