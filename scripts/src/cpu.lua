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
	MAME_DIR .. "src/emu/cpu/vtlb.*",
}

--------------------------------------------------
-- Dynamic recompiler objects
--------------------------------------------------

if (CPUS["SH2"]~=null or CPUS["MIPS"]~=null or CPUS["POWERPC"]~=null or CPUS["RSP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/drcbec.*",
		MAME_DIR .. "src/emu/cpu/drcbeut.*",
		MAME_DIR .. "src/emu/cpu/drccache.*",
		MAME_DIR .. "src/emu/cpu/drcfe.*",
		MAME_DIR .. "src/emu/cpu/drcuml.*",
		MAME_DIR .. "src/emu/cpu/uml.*",
		MAME_DIR .. "src/emu/cpu/i386/i386dasm.*",
		MAME_DIR .. "src/emu/cpu/x86log.*",
		MAME_DIR .. "src/emu/cpu/drcbex86.*",
		MAME_DIR .. "src/emu/cpu/drcbex64.*",
	}
end

--------------------------------------------------
-- Signetics 8X300 / Scientific Micro Systems SMS300
--@src/emu/cpu/8x300/8x300.h,CPUS += 8X300
--------------------------------------------------

if (CPUS["8X300"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/8x300/8x300.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/8x300/8x300dasm.*")
end

--------------------------------------------------
-- ARCangent A4
--@src/emu/cpu/arc/arc.h,CPUS += ARC
--------------------------------------------------

if (CPUS["ARC"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/arc/arc.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arc/arcdasm.*")
end

--------------------------------------------------
-- ARcompact (ARCtangent-A5, ARC 600, ARC 700)
--@src/emu/cpu/arc/arc.h,CPUS += ARCOMPACT
--------------------------------------------------

if (CPUS["ARCOMPACT"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/arcompact/arcompact.*",
		MAME_DIR .. "src/emu/cpu/arcompact/arcompact_execute.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arcompact/arcompactdasm.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arcompact/arcompactdasm_dispatch.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arcompact/arcompactdasm_ops.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arcompact/arcompact_common.*")
end

--------------------------------------------------
-- Acorn ARM series
--
---@src/emu/cpu/arm/arm.h,CPUS += ARM
---@src/emu/cpu/arm7/arm7.h,CPUS += ARM7
--------------------------------------------------

if (CPUS["ARM"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/arm/arm.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arm/armdasm.*")
end

if (CPUS["ARM7"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/arm7/arm7.*",
		MAME_DIR .. "src/emu/cpu/arm7/arm7thmb.*",
		MAME_DIR .. "src/emu/cpu/arm7/arm7ops.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/arm7/arm7dasm.*")
end

--------------------------------------------------
-- Advanced Digital Chips SE3208
---@src/emu/cpu/se3208/se3208.h,CPUS += SE3208
--------------------------------------------------

if (CPUS["SE3208"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/se3208/se3208.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/se3208/se3208dis.*")
end

--------------------------------------------------
-- American Microsystems, Inc.(AMI) S2000 series
---@src/emu/cpu/amis2000/amis2000.h,CPUS += AMIS2000
--------------------------------------------------

if (CPUS["AMIS2000"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/amis2000/amis2000.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/amis2000/amis2000d.*")
end
--------------------------------------------------
-- Alpha 8201
---@src/emu/cpu/alph8201/alph8201.h,CPUS += ALPHA8201
--------------------------------------------------

if (CPUS["ALPHA8201"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/alph8201/alph8201.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/alph8201/8201dasm.*")
end

--------------------------------------------------
-- Analog Devices ADSP21xx series
---@src/emu/cpu/adsp2100/adsp2100.h,CPUS += ADSP21XX
--------------------------------------------------

if (CPUS["ADSP21XX"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/adsp2100/adsp2100.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/adsp2100/2100dasm.*")
end

--------------------------------------------------
-- Analog Devices "Sharc" ADSP21062
---@src/emu/cpu/sharc/sharc.h,CPUS += ADSP21062
--------------------------------------------------

if (CPUS["ADSP21062"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/sharc/sharc.*",
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/sharc/sharcdsm.*")
	}
end

--------------------------------------------------
-- APEXC
---@src/emu/cpu/apexc/apexc.h,CPUS += APEXC
--------------------------------------------------

if (CPUS["APEXC"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/apexc/apexc.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/apexc/apexcdsm.*")
end

--------------------------------------------------
-- AT&T DSP16A
---@src/emu/cpu/dsp16/dsp16.h,CPUS += DSP16A
--------------------------------------------------

if (CPUS["DSP16A"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/dsp16/dsp16.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp16/dsp16dis.*")
end

--------------------------------------------------
-- AT&T DSP32C
---@src/emu/cpu/dsp32/dsp32.h,CPUS += DSP32C
--------------------------------------------------

if (CPUS["DSP32C"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/dsp32/dsp32.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp32/dsp32dis.*")
end

--------------------------------------------------
-- Atari custom RISC processor
---@src/emu/cpu/asap/asap.h,CPUS += ASAP
--------------------------------------------------

if (CPUS["ASAP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/asap/asap.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/asap/asapdasm.*")
end

--------------------------------------------------
-- AMD Am29000
---@src/emu/cpu/am29000/am29000.h,CPUS += AM29000
--------------------------------------------------

if (CPUS["AM29000"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/am29000/am29000.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/am29000/am29dasm.*")
end

--------------------------------------------------
-- Atari Jaguar custom DSPs
---@src/emu/cpu/jaguar/jaguar.h,CPUS += JAGUAR
--------------------------------------------------

if (CPUS["JAGUAR"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/jaguar/jaguar.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/jaguar/jagdasm.*")
end

--------------------------------------------------
-- Simutrek Cube Quest bit-sliced CPUs
---@src/emu/cpu/cubeqcpu/cubeqcpu.h,CPUS += CUBEQCPU
--------------------------------------------------

if (CPUS["CUBEQCPU"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/cubeqcpu/cubeqcpu.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cubeqcpu/cubedasm.*")
end

--------------------------------------------------
-- Ensoniq ES5510 ('ESP') DSP
---@src/emu/cpu/es5510/es5510.h,CPUS += ES5510
--------------------------------------------------

if (CPUS["ES5510"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/es5510/es5510.*",
	}
end

--------------------------------------------------
-- Entertainment Sciences AM29116-based RIP
---@src/emu/cpu/esrip/esrip.h,CPUS += ESRIP
--------------------------------------------------

if (CPUS["ESRIP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/esrip/esrip.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/esrip/esripdsm.*")
end

--------------------------------------------------
-- RCA COSMAC
---@src/emu/cpu/cosmac/cosmac.h,CPUS += COSMAC
--------------------------------------------------

if (CPUS["COSMAC"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/cosmac/cosmac.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cosmac/cosdasm.*")
end

--------------------------------------------------
-- National Semiconductor COP400 family
---@src/emu/cpu/cop400/cop400.h,CPUS += COP400
--------------------------------------------------

if (CPUS["COP400"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/cop400/cop400.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cop400/cop410ds.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cop400/cop420ds.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cop400/cop440ds.*")
end

--------------------------------------------------
-- CP1610
---@src/emu/cpu/cp1610/cp1610.h,CPUS += CP1610
--------------------------------------------------

if (CPUS["CP1610"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/cp1610/cp1610.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/cp1610/1610dasm.*")
end

--------------------------------------------------
-- Cinematronics vector "CPU"
---@src/emu/cpu/ccpu/ccpu.h,CPUS += CCPU
--------------------------------------------------

if (CPUS["CCPU"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/ccpu/ccpu.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/ccpu/ccpudasm.*")
end

--------------------------------------------------
-- DEC T-11
---@src/emu/cpu/t11/t11.h,CPUS += T11
--------------------------------------------------

if (CPUS["T11"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/t11/t11.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/t11/t11dasm.*")
end

--------------------------------------------------
-- F8
---@src/emu/cpu/f8/f8.h,CPUS += F8
--------------------------------------------------

if (CPUS["F8"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/f8/f8.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/f8/f8dasm.*")
end

--------------------------------------------------
-- G65816
---@src/emu/cpu/g65816/g65816.h,CPUS += G65816
--------------------------------------------------

if (CPUS["G65816"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/g65816/g65816.*",
		MAME_DIR .. "src/emu/cpu/g65816/g65816o0.*",
		MAME_DIR .. "src/emu/cpu/g65816/g65816o1.*",
		MAME_DIR .. "src/emu/cpu/g65816/g65816o2.*",
		MAME_DIR .. "src/emu/cpu/g65816/g65816o3.*",
		MAME_DIR .. "src/emu/cpu/g65816/g65816o4.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/g65816/g65816ds.*")
end

--------------------------------------------------
-- Hitachi H8 (16/32-bit H8/300, H8/300H, H8S2000 and H8S2600 series)
---@src/emu/cpu/h8/h8.h,CPUS += H8
--------------------------------------------------

if (CPUS["H8"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/h8/h8.*",
		MAME_DIR .. "src/emu/cpu/h8/h8h.*",
		MAME_DIR .. "src/emu/cpu/h8/h8s2000.*",
		MAME_DIR .. "src/emu/cpu/h8/h8s2600.*",
		MAME_DIR .. "src/emu/cpu/h8/h83337.*",
		MAME_DIR .. "src/emu/cpu/h8/h83002.*",
		MAME_DIR .. "src/emu/cpu/h8/h83006.*",
		MAME_DIR .. "src/emu/cpu/h8/h83008.*",
		MAME_DIR .. "src/emu/cpu/h8/h83048.*",
		MAME_DIR .. "src/emu/cpu/h8/h8s2245.*",
		MAME_DIR .. "src/emu/cpu/h8/h8s2320.*",
		MAME_DIR .. "src/emu/cpu/h8/h8s2357.*",
		MAME_DIR .. "src/emu/cpu/h8/h8s2655.*",
		MAME_DIR .. "src/emu/cpu/h8/h8_adc.*",
		MAME_DIR .. "src/emu/cpu/h8/h8_port.*",
		MAME_DIR .. "src/emu/cpu/h8/h8_intc.*",
		MAME_DIR .. "src/emu/cpu/h8/h8_timer8.*",
		MAME_DIR .. "src/emu/cpu/h8/h8_timer16.*",
		MAME_DIR .. "src/emu/cpu/h8/h8_sci.*",
	}
end

--------------------------------------------------
-- Hitachi HCD62121
---@src/emu/cpu/hcd62121/hcd62121.h,CPUS += HCD62121
--------------------------------------------------

if (CPUS["HCD62121"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/hcd62121/hcd62121.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/hcd62121/hcd62121d.*")
end

--------------------------------------------------
-- Hitachi HMCS40 series
---@src/emu/cpu/hmcs40/hmcs40.h,CPUS += HMCS40
--------------------------------------------------

if (CPUS["HMCS40"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/hmcs40/hmcs40.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/hmcs40/hmcs40d.*")
end

--------------------------------------------------
-- Hitachi SH1/SH2
---@src/emu/cpu/sh2/sh2.h,CPUS += SH2
--------------------------------------------------

if (CPUS["SH2"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/sh2/sh2.*",
		MAME_DIR .. "src/emu/cpu/sh2/sh2fe.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/sh2/sh2dasm.*")
end

--------------------------------------------------
-- Hitachi SH4
---@src/emu/cpu/sh4/sh4.h,CPUS += SH4
--------------------------------------------------

if (CPUS["SH4"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/sh4/sh4.*",
		MAME_DIR .. "src/emu/cpu/sh4/sh4comn.*",
		MAME_DIR .. "src/emu/cpu/sh4/sh3comn.*",
		MAME_DIR .. "src/emu/cpu/sh4/sh4tmu.*",
		MAME_DIR .. "src/emu/cpu/sh4/sh4dmac.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/sh4/sh4dasm.*")
end

--------------------------------------------------
-- Hudsonsoft 6280
---@src/emu/cpu/h6280/h6280.h,CPUS += H6280
--------------------------------------------------

if (CPUS["H6280"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/h6280/h6280.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/h6280/6280dasm.*")
end

--------------------------------------------------
-- Hyperstone E1 series
---@src/emu/cpu/e132xs/e132xs.h,CPUS += E1
--------------------------------------------------

if (CPUS["E1"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/e132xs/e132xs.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/e132xs/32xsdasm.*")
end

--------------------------------------------------
-- 15IE-00-013 CPU ("Microprogrammed Control Device")
---@src/emu/cpu/ie15/ie15.h,CPUS += IE15
--------------------------------------------------

if (CPUS["IE15"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/ie15/ie15.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/ie15/ie15dasm.*")
end

--------------------------------------------------
-- Intel 4004
---@src/emu/cpu/i4004/i4004.h,CPUS += I4004
--------------------------------------------------

if (CPUS["I4004"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i4004/i4004.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i4004/4004dasm.*")
end

--------------------------------------------------
-- Intel 8008
---@src/emu/cpu/i8008/i8008.h,CPUS += I8008
--------------------------------------------------

if (CPUS["I8008"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i8008/i8008.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i8008/8008dasm.*")
end

--------------------------------------------------
--  National Semiconductor SC/MP
---@src/emu/cpu/scmp/scmp.h,CPUS += SCMP
--------------------------------------------------

if (CPUS["SCMP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/scmp/scmp.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/scmp/scmpdasm.*")
end

--------------------------------------------------
-- Intel 8080/8085A
---@src/emu/cpu/i8085/i8085.h,CPUS += I8085
--------------------------------------------------

if (CPUS["I8085"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i8085/i8085.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i8085/8085dasm.*")
end

--------------------------------------------------
-- Intel 8089
---@src/emu/cpu/i8085/i8089.h,CPUS += I8089
--------------------------------------------------

if (CPUS["I8089"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i8089/i8089.*",
		MAME_DIR .. "src/emu/cpu/i8089/i8089_channel.*",
		MAME_DIR .. "src/emu/cpu/i8089/i8089_ops.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i8089/i8089_dasm.*")
end

--------------------------------------------------
-- Intel MCS-48 (8039 and derivatives)
---@src/emu/cpu/mcs48/mcs48.h,CPUS += MCS48
--------------------------------------------------

if (CPUS["MCS48"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mcs48/mcs48.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mcs48/mcs48dsm.*")
end

--------------------------------------------------
-- Intel 8051 and derivatives
---@src/emu/cpu/mcs51/mcs51.h,CPUS += MCS51
--------------------------------------------------

if (CPUS["MCS51"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mcs51/mcs51.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mcs51/mcs51dasm.*")
end

--------------------------------------------------
-- Intel MCS-96
---@src/emu/cpu/mcs96/mcs96.h,CPUS += MCS96
--------------------------------------------------

if (CPUS["MCS96"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mcs96/mcs96.*",
		MAME_DIR .. "src/emu/cpu/mcs96/i8x9x.*",
		MAME_DIR .. "src/emu/cpu/mcs96/i8xc196.*",
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
		MAME_DIR .. "src/emu/cpu/i86/i86.*",
		MAME_DIR .. "src/emu/cpu/i86/i186.*",
		MAME_DIR .. "src/emu/cpu/i86/i286.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i386/i386dasm.*")
end

if (CPUS["I386"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i386/i386.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i386/i386dasm.*")
end

--------------------------------------------------
-- Intel i860
---@src/emu/cpu/i860/i860.h,CPUS += I860
--------------------------------------------------

if (CPUS["I860"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i860/i860.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i860/i860dis.*")
end

--------------------------------------------------
-- Intel i960
---@src/emu/cpu/i960/i960.h,CPUS += I960
--------------------------------------------------

if (CPUS["I960"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/i960/i960.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/i960/i960dis.*")
end

--------------------------------------------------
-- LH5801
---@src/emu/cpu/lh5801/lh5801.h,CPUS += LH5801
--------------------------------------------------

if (CPUS["LH5801"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/lh5801/lh5801.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/lh5801/5801dasm.*")
end

--------------------------------------------------
-- Manchester Small-Scale Experimental Machine
---@src/emu/cpu/ssem/ssem.h,CPUS += SSEM
--------------------------------------------------

if (CPUS["SSEM"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/ssem/ssem.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/ssem/ssemdasm.*")
end

--------------------------------------------------
-- Fujitsu MB88xx
---@src/emu/cpu/mb88xx/mb88xx.h,CPUS += MB88XX
--------------------------------------------------

if (CPUS["MB88XX"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mb88xx/mb88xx.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mb88xx/mb88dasm.*")
end

--------------------------------------------------
-- Fujitsu MB86233
---@src/emu/cpu/mb86233/mb86233.h,CPUS += MB86233
--------------------------------------------------

if (CPUS["MB86233"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mb86233/mb86233.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mb86233/mb86233d.*")
end

--------------------------------------------------
-- Fujitsu MB86235
---@src/emu/cpu/mb86233/mb86235.h,CPUS += MB86235
--------------------------------------------------

if (CPUS["MB86235"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mb86235/mb86235.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mb86235/mb86235d.*")
end

--------------------------------------------------
-- Microchip PIC16C5x
---@src/emu/cpu/pic16c5x/pic16c5x.h,CPUS += PIC16C5X
--------------------------------------------------

if (CPUS["PIC16C5X"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/pic16c5x/pic16c5x.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/pic16c5x/16c5xdsm.*")
end

--------------------------------------------------
-- Microchip PIC16C62x
---@src/emu/cpu/pic16c62x/pic16c62x.h,CPUS += PIC16C62X
--------------------------------------------------

if (CPUS["PIC16C62X"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/pic16c62x/pic16c62x.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/pic16c62x/16c62xdsm.*")
end

--------------------------------------------------
-- MIPS R3000 (MIPS I/II) series
-- MIPS R4000 (MIPS III/IV) series
---@src/emu/cpu/mips/mips3.h,CPUS += MIPS
--------------------------------------------------

if (CPUS["MIPS"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mips/r3000.*",
		MAME_DIR .. "src/emu/cpu/mips/mips3com.*",
		MAME_DIR .. "src/emu/cpu/mips/mips3.*",
		MAME_DIR .. "src/emu/cpu/mips/mips3fe.*",
		MAME_DIR .. "src/emu/cpu/mips/mips3drc.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mips/r3kdasm.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mips/mips3dsm.*")
end

--------------------------------------------------
-- Sony PlayStation CPU (R3000-based + GTE)
---@src/emu/cpu/psx/psx.h,CPUS += PSX
--------------------------------------------------

if (CPUS["PSX"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/psx/psx.*",
		MAME_DIR .. "src/emu/cpu/psx/gte.*",
		MAME_DIR .. "src/emu/cpu/psx/dma.*",
		MAME_DIR .. "src/emu/cpu/psx/irq.*",
		MAME_DIR .. "src/emu/cpu/psx/mdec.*",
		MAME_DIR .. "src/emu/cpu/psx/rcnt.*",
		MAME_DIR .. "src/emu/cpu/psx/sio.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/psx/psxdasm.*")
end

--------------------------------------------------
-- Mitsubishi M37702 and M37710 (based on 65C816)
---@src/emu/cpu/m37710/m37710.h,CPUS += M37710
--------------------------------------------------

if (CPUS["M37710"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/m37710/m37710.*",
		MAME_DIR .. "src/emu/cpu/m37710/m37710o0.*",
		MAME_DIR .. "src/emu/cpu/m37710/m37710o1.*",
		MAME_DIR .. "src/emu/cpu/m37710/m37710o2.*",
		MAME_DIR .. "src/emu/cpu/m37710/m37710o3.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m37710/m7700ds.*")
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
		MAME_DIR .. "src/emu/cpu/m6502/deco16.*",
		MAME_DIR .. "src/emu/cpu/m6502/m4510.*",
		MAME_DIR .. "src/emu/cpu/m6502/m6502.*",
		MAME_DIR .. "src/emu/cpu/m6502/m65c02.*",
		MAME_DIR .. "src/emu/cpu/m6502/m65ce02.*",
		MAME_DIR .. "src/emu/cpu/m6502/m65sc02.*",
		MAME_DIR .. "src/emu/cpu/m6502/m6504.*",
		MAME_DIR .. "src/emu/cpu/m6502/m6509.*",
		MAME_DIR .. "src/emu/cpu/m6502/m6510.*",
		MAME_DIR .. "src/emu/cpu/m6502/m6510t.*",
		MAME_DIR .. "src/emu/cpu/m6502/m7501.*",
		MAME_DIR .. "src/emu/cpu/m6502/m8502.*",
		MAME_DIR .. "src/emu/cpu/m6502/n2a03.*",
		MAME_DIR .. "src/emu/cpu/m6502/r65c02.*",
		MAME_DIR .. "src/emu/cpu/m6502/m740.*",
		MAME_DIR .. "src/emu/cpu/m6502/m3745x.*",
		MAME_DIR .. "src/emu/cpu/m6502/m5074x.*",
	}
end

--------------------------------------------------
-- Motorola 680x
---@src/emu/cpu/m6800/m6800.h,CPUS += M6800
--------------------------------------------------

if (CPUS["M6800"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/m6800/m6800.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m6800/6800dasm.*")
end

--------------------------------------------------
-- Motorola 6805
---@src/emu/cpu/m6805/m6805.h,CPUS += M6805
--------------------------------------------------

if (CPUS["M6805"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/m6805/m6805.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m6805/6805dasm.*")
end

--------------------------------------------------
-- Motorola 6809
---@src/emu/cpu/m6809/m6809.h,CPUS += M6809
---@src/emu/cpu/m6809/hd6309.h,CPUS += M6809
---@src/emu/cpu/m6809/konami.h,CPUS += M6809
--------------------------------------------------

if (CPUS["M6809"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/m6809/m6809.*",
		MAME_DIR .. "src/emu/cpu/m6809/hd6309.*",
		MAME_DIR .. "src/emu/cpu/m6809/konami.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m6809/6809dasm.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m6809/6309dasm.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m6809/knmidasm.*")
end

--------------------------------------------------
-- Motorola 68HC11
---@src/emu/cpu/mc68hc11/mc68hc11.h,CPUS += MC68HC11
--------------------------------------------------

if (CPUS["MC68HC11"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mc68hc11/mc68hc11.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mc68hc11/hc11dasm.*")
end

--------------------------------------------------
-- Motorola 68000 series
---@src/emu/cpu/m68000/m68000.h,CPUS += M680X0
--------------------------------------------------

if (CPUS["M680X0"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/m68000/m68kcpu.*",
		GEN_DIR .. "emu/cpu/m68000/m68kops.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/m68000/m68kdasm.*")
end

--------------------------------------------------
-- Motorola/Freescale dsp56k
---@src/emu/cpu/dsp56k/dsp56k.h,CPUS += DSP56156
--------------------------------------------------

if (CPUS["DSP56156"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/dsp56k/dsp56k.*",
		MAME_DIR .. "src/emu/cpu/dsp56k/dsp56mem.*",
		MAME_DIR .. "src/emu/cpu/dsp56k/dsp56pcu.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp56k/dsp56dsm.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp56k/opcode.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp56k/inst.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp56k/pmove.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/dsp56k/tables.*")
end


--------------------------------------------------
-- PDP-1
-- TX0
---@src/emu/cpu/pdp1/pdp1.h,CPUS += PDP1
---@src/emu/cpu/pdp1/tx0.h,CPUS += PDP1
--------------------------------------------------

if (CPUS["PDP1"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/pdp1/pdp1.*",
		MAME_DIR .. "src/emu/cpu/pdp1/tx0.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/pdp1/pdp1dasm.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/pdp1/tx0dasm.*")
end

--------------------------------------------------
-- Motorola PowerPC series
---@src/emu/cpu/powerpc/ppc.h,CPUS += POWERPC
--------------------------------------------------

if (CPUS["POWERPC"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/powerpc/ppccom.*",
		MAME_DIR .. "src/emu/cpu/powerpc/ppcfe.*",
		MAME_DIR .. "src/emu/cpu/powerpc/ppcdrc.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/powerpc/ppc_dasm.*")
end

--------------------------------------------------
-- NEC V-series Intel-compatible
---@src/emu/cpu/nec/nec.h,CPUS += NEC
---@src/emu/cpu/v30mz/v30mz.h,CPUS += V30MZ
--------------------------------------------------

if (CPUS["NEC"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/nec/nec.*",
		MAME_DIR .. "src/emu/cpu/nec/v25.*",
		MAME_DIR .. "src/emu/cpu/nec/v25sfr.*",
		MAME_DIR .. "src/emu/cpu/nec/v53.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/nec/necdasm.*")
end

if (CPUS["V30MZ"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/v30mz/v30mz.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/nec/necdasm.*")
end

--------------------------------------------------
-- NEC V60/V70
---@src/emu/cpu/v60/v60.h,CPUS += V60
--------------------------------------------------

if (CPUS["V60"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/v60/v60.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/v60/v60d.*")
end

--------------------------------------------------
-- NEC V810 (uPD70732)
---@src/emu/cpu/v810/v810.h,CPUS += V810
--------------------------------------------------

if (CPUS["V810"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/v810/v810.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/v810/v810dasm.*")
end

--------------------------------------------------
-- NEC uPD7725
---@src/emu/cpu/upd7725/upd7725.h,CPUS += UPD7725
--------------------------------------------------

if (CPUS["UPD7725"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/upd7725/upd7725.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/upd7725/dasm7725.*")
end

--------------------------------------------------
-- NEC uPD7810 series
---@src/emu/cpu/upd7810/upd7810.h,CPUS += UPD7810
--------------------------------------------------

if (CPUS["UPD7810"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/upd7810/upd7810.*",
		MAME_DIR .. "src/emu/cpu/upd7810/upd7810_opcodes.*",
		MAME_DIR .. "src/emu/cpu/upd7810/upd7810_table.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/upd7810/upd7810_dasm.*")
end

--------------------------------------------------
-- NEC uCOM-4 series
---@src/emu/cpu/ucom4/ucom4.h,CPUS += UCOM4
--------------------------------------------------

if (CPUS["UCOM4"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/ucom4/ucom4.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/ucom4/ucom4d.*")
end

--------------------------------------------------
-- Nintendo Minx
---@src/emu/cpu/minx/minx.h,CPUS += MINX
--------------------------------------------------

if (CPUS["MINX"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/minx/minx.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/minx/minxd.*")
end

--------------------------------------------------
-- Nintendo/SGI RSP (R3000-based + vector processing)
---@src/emu/cpu/rsp/rsp.h,CPUS += RSP
--------------------------------------------------

if (CPUS["RSP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/rsp/rsp.*",
		MAME_DIR .. "src/emu/cpu/rsp/rspdrc.*",
		MAME_DIR .. "src/emu/cpu/rsp/rspfe.*",
		MAME_DIR .. "src/emu/cpu/rsp/rspcp2.*",
		MAME_DIR .. "src/emu/cpu/rsp/rspcp2d.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/rsp/rsp_dasm.*")
end

--------------------------------------------------
-- Panasonic MN10200
---@src/emu/cpu/mn10200/mn10200.h,CPUS += MN10200
--------------------------------------------------

if (CPUS["MN10200"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/mn10200/mn10200.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/mn10200/mn102dis.*")
end

--------------------------------------------------
-- Saturn
---@src/emu/cpu/saturn/saturn.h,CPUS += SATURN
--------------------------------------------------

if (CPUS["SATURN"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/saturn/saturn.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/saturn/saturnds.*")
end

--------------------------------------------------
-- Signetics 2650
---@src/emu/cpu/s2650/s2650.h,CPUS += S2650
--------------------------------------------------

if (CPUS["S2650"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/s2650/s2650.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/s2650/2650dasm.*")
end

--------------------------------------------------
-- SC61860
---@src/emu/cpu/sc61860/sc61860.h,CPUS += SC61860
--------------------------------------------------

if (CPUS["SC61860"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/sc61860/sc61860.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/sc61860/scdasm.*")
end

--------------------------------------------------
-- SM8500
---@src/emu/cpu/sm8500/sm8500.h,CPUS += SM8500
--------------------------------------------------

if (CPUS["SM8500"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/sm8500/sm8500.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/sm8500/sm8500d.*")
end

--------------------------------------------------
-- Sony/Nintendo SPC700
---@src/emu/cpu/spc700/spc700.h,CPUS += SPC700
--------------------------------------------------

if (CPUS["SPC700"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/spc700/spc700.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/spc700/spc700ds.*")
end

--------------------------------------------------
-- SSP1601
---@src/emu/cpu/ssp1601/ssp1601.h,CPUS += SSP1601
--------------------------------------------------

if (CPUS["SSP1601"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/ssp1601/ssp1601.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/ssp1601/ssp1601d.*")
end

--------------------------------------------------
-- SunPlus u'nSP
---@src/emu/cpu/unsp/unsp.h,CPUS += UNSP
--------------------------------------------------

if (CPUS["UNSP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/unsp/unsp.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/unsp/unspdasm.*")
end

--------------------------------------------------
-- Atmel 8-bit AVR
---@src/emu/cpu/avr8/avr8.h,CPUS += AVR8
--------------------------------------------------

if (CPUS["AVR8"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/avr8/avr8.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/avr8/avr8dasm.*")
end

--------------------------------------------------
-- Texas Instruments TMS0980
---@src/emu/cpu/tms0980/tms0980.h,CPUS += TMS0980
--------------------------------------------------

if (CPUS["TMS0980"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms0980/tms0980.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms0980/tms0980d.*")
end

--------------------------------------------------
-- Texas Instruments TMS7000 series
---@src/emu/cpu/tms7000/tms7000.h,CPUS += TMS7000
--------------------------------------------------

if (CPUS["TMS7000"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms7000/tms7000.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms7000/7000dasm.*")
end

--------------------------------------------------
-- Texas Instruments TMS99xx series
---@src/emu/cpu/tms9900/tms9900.h,CPUS += TMS9900
---@src/emu/cpu/tms9900/tms9980a.h,CPUS += TMS9900
---@src/emu/cpu/tms9900/tms9995.h,CPUS += TMS9900

--------------------------------------------------

if (CPUS["TMS9900"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms9900/tms9900.*",
		MAME_DIR .. "src/emu/cpu/tms9900/tms9980a.*",
		MAME_DIR .. "src/emu/cpu/tms9900/tms9995.*",
		MAME_DIR .. "src/emu/cpu/tms9900/ti990_10.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms9900/9900dasm.*")
end

--------------------------------------------------
-- Texas Instruments TMS340x0 graphics controllers
---@src/emu/cpu/tms34010/tms34010.h,CPUS += TMS340X0
--------------------------------------------------

if (CPUS["TMS340X0"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms34010/tms34010.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms34010/34010dsm.*")
end

--------------------------------------------------
-- Texas Instruments TMS3201x DSP
---@src/emu/cpu/tms32010/tms32010.h,CPUS += TMS32010
--------------------------------------------------

if (CPUS["TMS32010"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms32010/tms32010.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32010/32010dsm.*")
end

--------------------------------------------------
-- Texas Instruments TMS3202x DSP
---@src/emu/cpu/tms32025/tms32025.h,CPUS += TMS32025
--------------------------------------------------

if (CPUS["TMS32025"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms32025/tms32025.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32025/32025dsm.*")
end

--------------------------------------------------
-- Texas Instruments TMS3203x DSP
---@src/emu/cpu/tms32031/tms32031.h,CPUS += TMS32031
--------------------------------------------------

if (CPUS["TMS32031"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms32031/tms32031.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32031/dis32031.*")
end

--------------------------------------------------
-- Texas Instruments TMS3205x DSP
---@src/emu/cpu/tms32051/tms32051.h,CPUS += TMS32051
--------------------------------------------------

if (CPUS["TMS32051"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms32051/tms32051.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32051/dis32051.*")
end

--------------------------------------------------
-- Texas Instruments TMS3208x DSP
---@src/emu/cpu/tms32082/tms32082.h,CPUS += TMS32082_MP
--------------------------------------------------

if (CPUS["TMS32082"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms32082/tms32082.*",
		MAME_DIR .. "src/emu/cpu/tms32082/mp_ops.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32082/dis_mp.*")
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms32082/dis_pp.*")
end

--------------------------------------------------
-- Texas Instruments TMS57002 DSP
---@src/emu/cpu/tms57002/tms57002.h,CPUS += TMS57002
--------------------------------------------------

if (CPUS["TMS57002"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tms57002/tms57002.*",
		MAME_DIR .. "src/emu/cpu/tms57002/tms57kdec.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tms57002/57002dsm.*")
end

--------------------------------------------------
-- Toshiba TLCS-90 Series
---@src/emu/cpu/tlcs90/tlcs90.h,CPUS += TLCS90
--------------------------------------------------

if (CPUS["TLCS90"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tlcs90/tlcs90.*",
	}
end

--------------------------------------------------
-- Toshiba TLCS-900 Series
---@src/emu/cpu/tlcs900/tlcs900.h,CPUS += TLCS900
--------------------------------------------------

if (CPUS["TLCS900"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/tlcs900/tlcs900.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/tlcs900/dasm900.*")
end

--------------------------------------------------
-- Zilog Z80
---@src/emu/cpu/z80/z80.h,CPUS += Z80
--------------------------------------------------

if (CPUS["Z80"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/z80/z80.*",
		MAME_DIR .. "src/emu/cpu/z80/z80daisy.*",
		MAME_DIR .. "src/emu/cpu/z80/tmpz84c011.*",
		MAME_DIR .. "src/emu/cpu/z80/tmpz84c015.*",
		MAME_DIR .. "src/emu/cpu/z80/kl5c80a12.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/z80/z80dasm.*")
end

--------------------------------------------------
-- Sharp LR35902 (Game Boy CPU)
---@src/emu/cpu/lr35902/lr35902.h,CPUS += LR35902
--------------------------------------------------

if (CPUS["LR35902"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/lr35902/lr35902.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/lr35902/lr35902d.*")
end

--------------------------------------------------
-- Zilog Z180
---@src/emu/cpu/z180/z180.h,CPUS += Z180
--------------------------------------------------

if (CPUS["Z180"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/z180/z180.*",
		MAME_DIR .. "src/emu/cpu/z80/z80daisy.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/z180/z180dasm.*")
end

--------------------------------------------------
-- Zilog Z8000
---@src/emu/cpu/z8000/z8000.h,CPUS += Z8000
--------------------------------------------------

if (CPUS["Z8000"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/z8000/z8000.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/z8000/8000dasm.*")
end

--------------------------------------------------
-- Zilog Z8
---@src/emu/cpu/z8/z8.h,CPUS += Z8
--------------------------------------------------

if (CPUS["Z8"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/z8/z8.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/z8/z8dasm.*")
end

--------------------------------------------------
-- Argonaut SuperFX
---@src/emu/cpu/superfx/superfx.h,CPUS += SUPERFX
--------------------------------------------------

if (CPUS["SUPERFX"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/superfx/superfx.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/superfx/sfx_dasm.*")
end

--------------------------------------------------
-- Rockwell PPS-4
---@src/emu/cpu/pps4/pps4.h,CPUS += PPS4
--------------------------------------------------

if (CPUS["PPS4"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/pps4/pps4.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/pps4/pps4dasm.*")
end

--------------------------------------------------
-- Hitachi HD61700
---@src/emu/cpu/hd61700/hd61700.h,CPUS += HD61700
--------------------------------------------------

if (CPUS["HD61700"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/hd61700/hd61700.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/hd61700/hd61700d.*")
end

--------------------------------------------------
-- Sanyo LC8670
---@src/emu/cpu/lc8670/lc8670.h,CPUS += LC8670
--------------------------------------------------

if (CPUS["LC8670"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/lc8670/lc8670.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/lc8670/lc8670dsm.*")
end

--------------------------------------------------
-- Sega SCU DSP
---@src/emu/cpu/scudsp/scudsp.h,CPUS += SCUDSP
--------------------------------------------------

if (CPUS["SCUDSP"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/scudsp/scudsp.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/scudsp/scudspdasm.*")
end

--------------------------------------------------
-- Sunplus Technology S+core
---@src/emu/cpu/score/score.h,CPUS += SCORE
--------------------------------------------------

if (CPUS["SCORE"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/score/score.*",
	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/score/scoredsm.*")
end

--------------------------------------------------
-- Xerox Alto-II
---@src/emu/cpu/alto2/alto2cpu.h,CPUS += ALTO2
--------------------------------------------------

if (CPUS["ALTO2"]~=null) then
	files {
		MAME_DIR .. "src/emu/cpu/alto2/alto2cpu.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2disk.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2disp.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2curt.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2dht.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2dvt.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2dwt.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2emu.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2ether.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2hw.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2kbd.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2ksec.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2kwd.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2mem.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2mouse.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2mrt.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2part.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2ram.*",
		MAME_DIR .. "src/emu/cpu/alto2/a2roms.*",

	}
	table.insert(disasm_files , MAME_DIR .. "src/emu/cpu/alto2/alto2dsm.*")
end
