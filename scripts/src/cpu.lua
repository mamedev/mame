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
-- Dynamic recompiler objects
--------------------------------------------------

DRC_CPUS = { "E1", "SH", "MIPS3", "POWERPC", "ARM7", "ADSP21062", "MB86235", "DSP16", "UNSP" }
CPU_INCLUDE_DRC = false
for i, v in ipairs(DRC_CPUS) do
	if (CPUS[v]~=null) then
		CPU_INCLUDE_DRC = true
		break
	end
end


if (CPU_INCLUDE_DRC) then
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
		MAME_DIR .. "src/devices/cpu/x86log.cpp",
		MAME_DIR .. "src/devices/cpu/x86log.h",
		MAME_DIR .. "src/devices/cpu/drcumlsh.h",
	}
	if not _OPTIONS["FORCE_DRC_C_BACKEND"] then
		files {
			MAME_DIR .. "src/devices/cpu/drcbex64.cpp",
			MAME_DIR .. "src/devices/cpu/drcbex64.h",
			MAME_DIR .. "src/devices/cpu/drcbex86.cpp",
			MAME_DIR .. "src/devices/cpu/drcbex86.h",
		}
	end

	if _OPTIONS["targetos"]=="macosx" and _OPTIONS["gcc"]~=nil then
		if string.find(_OPTIONS["gcc"], "clang") and (str_to_version(_OPTIONS["gcc_version"]) < 80000) then
			defines {
				"TARGET_OS_OSX=1",
			}
		end
	end
end

--------------------------------------------------
-- Signetics 8X300 / Scientific Micro Systems SMS300
--@src/devices/cpu/8x300/8x300.h,CPUS["8X300"] = true
--------------------------------------------------

if CPUS["8X300"] then
	files {
		MAME_DIR .. "src/devices/cpu/8x300/8x300.cpp",
		MAME_DIR .. "src/devices/cpu/8x300/8x300.h",
	}
end

if opt_tool(CPUS, "8X300") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/8x300/8x300dasm.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/8x300/8x300dasm.cpp")
end

--------------------------------------------------
-- 3DO Don's Super Performing Processor (DSPP)
--@src/devices/cpu/dspp/dspp.h,CPUS["DSPP"] = true
--------------------------------------------------

if CPUS["DSPP"] then
	files {
		MAME_DIR .. "src/devices/cpu/dspp/dspp.cpp",
		MAME_DIR .. "src/devices/cpu/dspp/dspp.h",
		MAME_DIR .. "src/devices/cpu/dspp/dsppdrc.cpp",
		MAME_DIR .. "src/devices/cpu/dspp/dsppfe.cpp",
		MAME_DIR .. "src/devices/cpu/dspp/dsppfe.h",
	}
end

if opt_tool(CPUS, "DSPP") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dspp/dsppdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dspp/dsppdasm.h")
end

--------------------------------------------------
-- ARCangent A4
--@src/devices/cpu/arc/arc.h,CPUS["ARC"] = true
--------------------------------------------------

if CPUS["ARC"] then
	files {
		MAME_DIR .. "src/devices/cpu/arc/arc.cpp",
		MAME_DIR .. "src/devices/cpu/arc/arc.h",
	}
end

if opt_tool(CPUS, "ARC") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arc/arcdasm.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arc/arcdasm.cpp")
end

--------------------------------------------------
-- ARcompact (ARCtangent-A5, ARC 600, ARC 700)
--@src/devices/cpu/arcompact/arcompact.h,CPUS["ARCOMPACT"] = true
--------------------------------------------------

if CPUS["ARCOMPACT"] then
	files {
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact.h",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_00to01.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_02to03.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_04.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_04_jumps.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_04_loop.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_04_aux.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_04_2f_sop.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_04_2f_3f_zop.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_04_3x.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_05.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_05_2f_sop.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_06to0b.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_0c_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_0d_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_0e_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_0f_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_0f_00_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_0f_00_07_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_12to16_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_17_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_18_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_19_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_1ato1c_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_execute_ops_1dto1f_16bit.cpp",
		MAME_DIR .. "src/devices/cpu/arcompact/arcompact_helper.ipp",
	}
end

if opt_tool(CPUS, "ARCOMPACT") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompact_common.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_internal.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops_00to01.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops_02to03.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops_04.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops_04_2f_sop.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops_04_2f_3f_zop.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops_04_3x.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops_05.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops_05_2f_sop.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops_06to0b.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arcompact/arcompactdasm_ops_16bit.cpp")
end

--------------------------------------------------
-- Acorn ARM series
--
--@src/devices/cpu/arm/arm.h,CPUS["ARM"] = true
--@src/devices/cpu/arm7/arm7.h,CPUS["ARM7"] = true
--------------------------------------------------

if CPUS["ARM"] then
	files {
		MAME_DIR .. "src/devices/cpu/arm/arm.cpp",
		MAME_DIR .. "src/devices/cpu/arm/arm.h",
	}
end

if opt_tool(CPUS, "ARM") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arm/armdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arm/armdasm.h")
end

if CPUS["ARM7"] then
	files {
		MAME_DIR .. "src/devices/cpu/arm7/arm7.cpp",
		MAME_DIR .. "src/devices/cpu/arm7/arm7.h",
		MAME_DIR .. "src/devices/cpu/arm7/arm7thmb.cpp",
		MAME_DIR .. "src/devices/cpu/arm7/arm7ops.cpp",
		MAME_DIR .. "src/devices/cpu/arm7/ap2010cpu.cpp",
		MAME_DIR .. "src/devices/cpu/arm7/ap2010cpu.h",
		MAME_DIR .. "src/devices/cpu/arm7/lpc210x.cpp",
		MAME_DIR .. "src/devices/cpu/arm7/lpc210x.h",
		MAME_DIR .. "src/devices/cpu/arm7/upd800468.cpp",
		MAME_DIR .. "src/devices/cpu/arm7/upd800468.h",
		MAME_DIR .. "src/devices/cpu/arm7/arm7core.h",
		MAME_DIR .. "src/devices/cpu/arm7/arm7core.hxx",
		MAME_DIR .. "src/devices/cpu/arm7/arm7drc.hxx",
		MAME_DIR .. "src/devices/cpu/arm7/arm7help.h",
		MAME_DIR .. "src/devices/cpu/arm7/arm7tdrc.hxx",
		MAME_DIR .. "src/devices/cpu/arm7/cecalls.hxx",
	}
end

if opt_tool(CPUS, "ARM7") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arm7/arm7dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/arm7/arm7dasm.h")
end

--------------------------------------------------
-- Advanced Digital Chips SE3208
--@src/devices/cpu/se3208/se3208.h,CPUS["SE3208"] = true
--------------------------------------------------

if CPUS["SE3208"] then
	files {
		MAME_DIR .. "src/devices/cpu/se3208/se3208.cpp",
		MAME_DIR .. "src/devices/cpu/se3208/se3208.h",
	}
end

if opt_tool(CPUS, "SE3208") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/se3208/se3208dis.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/se3208/se3208dis.h")
end

--------------------------------------------------
-- American Microsystems, Inc.(AMI) S2000 series
--@src/devices/cpu/amis2000/amis2000.h,CPUS["AMIS2000"] = true
--------------------------------------------------

if CPUS["AMIS2000"] then
	files {
		MAME_DIR .. "src/devices/cpu/amis2000/amis2000.cpp",
		MAME_DIR .. "src/devices/cpu/amis2000/amis2000.h",
		MAME_DIR .. "src/devices/cpu/amis2000/amis2000op.cpp",
	}
end

if opt_tool(CPUS, "AMIS2000") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/amis2000/amis2000d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/amis2000/amis2000d.h")
end

--------------------------------------------------
-- Analog Devices ADSP21xx series
--@src/devices/cpu/adsp2100/adsp2100.h,CPUS["ADSP21XX"] = true
--------------------------------------------------

if CPUS["ADSP21XX"] then
	files {
		MAME_DIR .. "src/devices/cpu/adsp2100/adsp2100.cpp",
		MAME_DIR .. "src/devices/cpu/adsp2100/adsp2100.h",
		MAME_DIR .. "src/devices/cpu/adsp2100/2100ops.hxx",
	}
end

if opt_tool(CPUS, "ADSP21XX") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/adsp2100/2100dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/adsp2100/2100dasm.h")
end

--------------------------------------------------
-- Analog Devices "Sharc" ADSP21062
--@src/devices/cpu/sharc/sharc.h,CPUS["ADSP21062"] = true
--------------------------------------------------

if CPUS["ADSP21062"] then
	files {
		MAME_DIR .. "src/devices/cpu/sharc/sharc.cpp",
		MAME_DIR .. "src/devices/cpu/sharc/sharc.h",
		MAME_DIR .. "src/devices/cpu/sharc/compute.hxx",
		MAME_DIR .. "src/devices/cpu/sharc/sharcdma.hxx",
		MAME_DIR .. "src/devices/cpu/sharc/sharcmem.hxx",
		MAME_DIR .. "src/devices/cpu/sharc/sharcops.h",
		MAME_DIR .. "src/devices/cpu/sharc/sharcops.hxx",
		MAME_DIR .. "src/devices/cpu/sharc/sharcdrc.cpp",
		MAME_DIR .. "src/devices/cpu/sharc/sharcfe.cpp",
		MAME_DIR .. "src/devices/cpu/sharc/sharcfe.h",
	}
end

if opt_tool(CPUS, "ADSP21062") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sharc/sharcdsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sharc/sharcdsm.h")
end

--------------------------------------------------
-- APEXC
--@src/devices/cpu/apexc/apexc.h,CPUS["APEXC"] = true
--------------------------------------------------

if CPUS["APEXC"] then
	files {
		MAME_DIR .. "src/devices/cpu/apexc/apexc.cpp",
		MAME_DIR .. "src/devices/cpu/apexc/apexc.h",
	}
end

if opt_tool(CPUS, "APEXC") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/apexc/apexcdsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/apexc/apexcdsm.h")
end

--------------------------------------------------
-- WE|AT&T DSP16
--@src/devices/cpu/dsp16/dsp16.h,CPUS["DSP16"] = true
--------------------------------------------------

if CPUS["DSP16"] then
	files {
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16.cpp",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16.h",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16core.cpp",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16core.h",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16core.ipp",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16fe.cpp",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16fe.h",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16rc.cpp",
		MAME_DIR .. "src/devices/cpu/dsp16/dsp16rc.h",
	}
end

if opt_tool(CPUS, "DSP16") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp16/dsp16dis.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp16/dsp16dis.h")
end

--------------------------------------------------
-- AT&T DSP32C
--@src/devices/cpu/dsp32/dsp32.h,CPUS["DSP32C"] = true
--------------------------------------------------

if CPUS["DSP32C"] then
	files {
		MAME_DIR .. "src/devices/cpu/dsp32/dsp32.cpp",
		MAME_DIR .. "src/devices/cpu/dsp32/dsp32.h",
		MAME_DIR .. "src/devices/cpu/dsp32/dsp32ops.hxx",
	}
end

if opt_tool(CPUS, "DSP32C") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp32/dsp32dis.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp32/dsp32dis.h")
end

--------------------------------------------------
-- Atari custom RISC processor
--@src/devices/cpu/asap/asap.h,CPUS["ASAP"] = true
--------------------------------------------------

if CPUS["ASAP"] then
	files {
		MAME_DIR .. "src/devices/cpu/asap/asap.cpp",
		MAME_DIR .. "src/devices/cpu/asap/asap.h",
	}
end

if opt_tool(CPUS, "ASAP") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/asap/asapdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/asap/asapdasm.h")
end

--------------------------------------------------
-- AMD Am29000
--@src/devices/cpu/am29000/am29000.h,CPUS["AM29000"] = true
--------------------------------------------------

if CPUS["AM29000"] then
	files {
		MAME_DIR .. "src/devices/cpu/am29000/am29000.cpp",
		MAME_DIR .. "src/devices/cpu/am29000/am29000.h",
		MAME_DIR .. "src/devices/cpu/am29000/am29ops.h",
	}
end

if opt_tool(CPUS, "AM29000") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/am29000/am29dasm.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/am29000/am29dasm.cpp")
end

--------------------------------------------------
-- Atari Jaguar custom DSPs
--@src/devices/cpu/jaguar/jaguar.h,CPUS["JAGUAR"] = true
--------------------------------------------------

if CPUS["JAGUAR"] then
	files {
		MAME_DIR .. "src/devices/cpu/jaguar/jaguar.cpp",
		MAME_DIR .. "src/devices/cpu/jaguar/jaguar.h",
	}
end

if opt_tool(CPUS, "JAGUAR") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/jaguar/jagdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/jaguar/jagdasm.h")
end

--------------------------------------------------
-- Simutrek Cube Quest bit-sliced CPUs
--@src/devices/cpu/cubeqcpu/cubeqcpu.h,CPUS["CUBEQCPU"] = true
--------------------------------------------------

if CPUS["CUBEQCPU"] then
	files {
		MAME_DIR .. "src/devices/cpu/cubeqcpu/cubeqcpu.cpp",
		MAME_DIR .. "src/devices/cpu/cubeqcpu/cubeqcpu.h",
	}
end

if opt_tool(CPUS, "CUBEQCPU") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cubeqcpu/cubedasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cubeqcpu/cubedasm.h")
end

--------------------------------------------------
-- Ensoniq ES5510 ('ESP') DSP
--@src/devices/cpu/es5510/es5510.h,CPUS["ES5510"] = true
--------------------------------------------------

if CPUS["ES5510"] then
	files {
		MAME_DIR .. "src/devices/cpu/es5510/es5510.cpp",
		MAME_DIR .. "src/devices/cpu/es5510/es5510.h",
	}
end

if opt_tool(CPUS, "ES5510") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/es5510/es5510d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/es5510/es5510d.h")
end

--------------------------------------------------
-- Entertainment Sciences AM29116-based RIP
--@src/devices/cpu/esrip/esrip.h,CPUS["ESRIP"] = true
--------------------------------------------------

if CPUS["ESRIP"] then
	files {
		MAME_DIR .. "src/devices/cpu/esrip/esrip.cpp",
		MAME_DIR .. "src/devices/cpu/esrip/esrip.h",
	}
end

if opt_tool(CPUS, "ESRIP") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/esrip/esripdsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/esrip/esripdsm.h")
end

--------------------------------------------------
-- Seiko Epson E0C6200 series
--@src/devices/cpu/e0c6200/e0c6200.h,CPUS["E0C6200"] = true
--------------------------------------------------

if CPUS["E0C6200"] then
	files {
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200.cpp",
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200.h",
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6s46.cpp",
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6s46.h",
		MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200op.cpp",
	}
end

if opt_tool(CPUS, "E0C6200") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/e0c6200/e0c6200d.h")
end

--------------------------------------------------
-- RCA COSMAC
--@src/devices/cpu/cosmac/cosmac.h,CPUS["COSMAC"] = true
--------------------------------------------------

if CPUS["COSMAC"] then
	files {
		MAME_DIR .. "src/devices/cpu/cosmac/cosmac.cpp",
		MAME_DIR .. "src/devices/cpu/cosmac/cosmac.h",
	}
end

if opt_tool(CPUS, "COSMAC") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cosmac/cosdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cosmac/cosdasm.h")
end

--------------------------------------------------
-- National Semiconductor COPS(MM57) family
--@src/devices/cpu/cops1/mm5799.h,CPUS["COPS1"] = true
--------------------------------------------------

if CPUS["COPS1"] then
	files {
		MAME_DIR .. "src/devices/cpu/cops1/cops1base.cpp",
		MAME_DIR .. "src/devices/cpu/cops1/cops1base.h",
		MAME_DIR .. "src/devices/cpu/cops1/mm5799.cpp",
		MAME_DIR .. "src/devices/cpu/cops1/mm5799.h",
		MAME_DIR .. "src/devices/cpu/cops1/mm5799op.cpp",
	}
end

if opt_tool(CPUS, "COPS1") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cops1/cops1d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cops1/cops1d.h")
end

--------------------------------------------------
-- National Semiconductor COPS(COP400) family
--@src/devices/cpu/cop400/cop400.h,CPUS["COP400"] = true
--------------------------------------------------

if CPUS["COP400"] then
	files {
		MAME_DIR .. "src/devices/cpu/cop400/cop400.cpp",
		MAME_DIR .. "src/devices/cpu/cop400/cop400.h",
		MAME_DIR .. "src/devices/cpu/cop400/cop400op.hxx",
	}
end

if opt_tool(CPUS, "COP400") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop410ds.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop410ds.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop420ds.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop420ds.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop444ds.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop444ds.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop424ds.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cop400/cop424ds.h")
end

--------------------------------------------------
-- CP1610
--@src/devices/cpu/cp1610/cp1610.h,CPUS["CP1610"] = true
--------------------------------------------------

if CPUS["CP1610"] then
	files {
		MAME_DIR .. "src/devices/cpu/cp1610/cp1610.cpp",
		MAME_DIR .. "src/devices/cpu/cp1610/cp1610.h",
	}
end

if opt_tool(CPUS, "CP1610") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cp1610/1610dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cp1610/1610dasm.h")
end

--------------------------------------------------
-- Cinematronics vector "CPU"
--@src/devices/cpu/ccpu/ccpu.h,CPUS["CCPU"] = true
--------------------------------------------------

if CPUS["CCPU"] then
	files {
		MAME_DIR .. "src/devices/cpu/ccpu/ccpu.cpp",
		MAME_DIR .. "src/devices/cpu/ccpu/ccpu.h",
	}
end

if opt_tool(CPUS, "CCPU") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ccpu/ccpudasm.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ccpu/ccpudasm.cpp")
end

--------------------------------------------------
-- DEC T-11
--@src/devices/cpu/t11/t11.h,CPUS["T11"] = true
--------------------------------------------------

if CPUS["T11"] then
	files {
		MAME_DIR .. "src/devices/cpu/t11/t11.cpp",
		MAME_DIR .. "src/devices/cpu/t11/t11.h",
		MAME_DIR .. "src/devices/cpu/t11/t11ops.hxx",
		MAME_DIR .. "src/devices/cpu/t11/t11table.hxx",
	}
end

if opt_tool(CPUS, "T11") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/t11/t11dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/t11/t11dasm.h")
end

--------------------------------------------------
-- DEC PDP-8
--@src/devices/cpu/pdp8/pdp8.h,CPUS["PDP8"] = true
--@src/devices/cpu/pdp8/hd6120.h,CPUS["PDP8"] = true
--------------------------------------------------

if CPUS["PDP8"] then
	files {
		MAME_DIR .. "src/devices/cpu/pdp8/hd6120.cpp",
		MAME_DIR .. "src/devices/cpu/pdp8/hd6120.h",
		MAME_DIR .. "src/devices/cpu/pdp8/pdp8.cpp",
		MAME_DIR .. "src/devices/cpu/pdp8/pdp8.h",
	}
end

if opt_tool(CPUS, "PDP8") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pdp8/pdp8dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pdp8/pdp8dasm.h")
end

--------------------------------------------------
-- F8
--@src/devices/cpu/f8/f8.h,CPUS["F8"] = true
--------------------------------------------------

if CPUS["F8"] then
	files {
		MAME_DIR .. "src/devices/cpu/f8/f8.cpp",
		MAME_DIR .. "src/devices/cpu/f8/f8.h",
	}
end

if opt_tool(CPUS, "F8") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/f8/f8dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/f8/f8dasm.h")
end

--------------------------------------------------
-- Fujitsu FR
--@src/devices/cpu/fr/fr.h,CPUS["FR"] = true
--------------------------------------------------

if CPUS["FR"] then
	files {
		MAME_DIR .. "src/devices/cpu/fr/fr.cpp",
		MAME_DIR .. "src/devices/cpu/fr/fr.h",
	}
end

if opt_tool(CPUS, "FR") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/fr/frdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/fr/frdasm.h")
end

--------------------------------------------------
-- G65816
--@src/devices/cpu/g65816/g65816.h,CPUS["G65816"] = true
--------------------------------------------------

if CPUS["G65816"] then
	files {
		MAME_DIR .. "src/devices/cpu/g65816/g65816.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816.h",
		MAME_DIR .. "src/devices/cpu/g65816/g65816cm.h",
		MAME_DIR .. "src/devices/cpu/g65816/g65816ds.h",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o0.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o1.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o2.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o3.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816o4.cpp",
		MAME_DIR .. "src/devices/cpu/g65816/g65816op.ipp",
	}
end

if opt_tool(CPUS, "G65816") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/g65816/g65816ds.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/g65816/g65816ds.h")
end

--------------------------------------------------
-- Hitachi H16
--@src/devices/cpu/h16/hd641016.h,CPUS["H16"] = true
--------------------------------------------------

if CPUS["H16"] then
	files {
		MAME_DIR .. "src/devices/cpu/h16/hd641016.cpp",
		MAME_DIR .. "src/devices/cpu/h16/hd641016.h",
	}
end

if opt_tool(CPUS, "H16") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/h16/h16dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/h16/h16dasm.h")
end

--------------------------------------------------
-- Hitachi H8 (16/32-bit H8/300, H8/300H, H8S2000 and H8S2600 series)
--@src/devices/cpu/h8/h8.h,CPUS["H8"] = true
--------------------------------------------------

if CPUS["H8"] then
	files {
		MAME_DIR .. "src/devices/cpu/h8/h8.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8.h",
		MAME_DIR .. "src/devices/cpu/h8/h8h.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8h.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2000.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2000.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2600.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2600.h",
		MAME_DIR .. "src/devices/cpu/h8/h8325.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8325.h",
		MAME_DIR .. "src/devices/cpu/h8/h83002.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83002.h",
		MAME_DIR .. "src/devices/cpu/h8/h83003.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83003.h",
		MAME_DIR .. "src/devices/cpu/h8/h83006.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83006.h",
		MAME_DIR .. "src/devices/cpu/h8/h83008.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83008.h",
		MAME_DIR .. "src/devices/cpu/h8/h83032.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83032.h",
		MAME_DIR .. "src/devices/cpu/h8/h83042.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83042.h",
		MAME_DIR .. "src/devices/cpu/h8/h83048.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83048.h",
		MAME_DIR .. "src/devices/cpu/h8/h83217.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83217.h",
		MAME_DIR .. "src/devices/cpu/h8/h83337.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h83337.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2245.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2245.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2319.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2319.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2329.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2329.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2357.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2357.h",
		MAME_DIR .. "src/devices/cpu/h8/h8s2655.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8s2655.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_adc.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_adc.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_dma.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_dma.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_dtc.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_dtc.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_intc.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_intc.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_port.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_port.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer8.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer8.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer16.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_timer16.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_sci.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_sci.h",
		MAME_DIR .. "src/devices/cpu/h8/h8_watchdog.cpp",
		MAME_DIR .. "src/devices/cpu/h8/h8_watchdog.h",
		MAME_DIR .. "src/devices/cpu/h8/gt913.cpp",
		MAME_DIR .. "src/devices/cpu/h8/gt913.h",
		MAME_DIR .. "src/devices/cpu/h8/swx00.cpp",
		MAME_DIR .. "src/devices/cpu/h8/swx00.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/cpu/h8/h8.cpp",       GEN_DIR .. "emu/cpu/h8/h8.hxx" },
		{ MAME_DIR .. "src/devices/cpu/h8/h8h.cpp",      GEN_DIR .. "emu/cpu/h8/h8h.hxx" },
		{ MAME_DIR .. "src/devices/cpu/h8/h8s2000.cpp",  GEN_DIR .. "emu/cpu/h8/h8s2000.hxx" },
		{ MAME_DIR .. "src/devices/cpu/h8/h8s2600.cpp",  GEN_DIR .. "emu/cpu/h8/h8s2600.hxx" },
		{ MAME_DIR .. "src/devices/cpu/h8/gt913.cpp",    GEN_DIR .. "emu/cpu/h8/gt913.hxx" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8.hxx",       { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8-300 source file...",   PYTHON .. " $(1) $(<) s o   $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8h.hxx",      { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8-300H source file...",  PYTHON .. " $(1) $(<) s h   $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8s2000.hxx",  { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8S/2000 source file...", PYTHON .. " $(1) $(<) s s20 $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/h8/h8.lst" , GEN_DIR .. "emu/cpu/h8/h8s2600.hxx",  { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8S/2600 source file...", PYTHON .. " $(1) $(<) s s26 $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/h8/gt913.lst" , GEN_DIR .. "emu/cpu/h8/gt913.hxx", { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating GT913 source file...",    PYTHON .. " $(1) $(<) s g   $(@)" }},
	}
end

if opt_tool(CPUS, "H8") then
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/h8/h8.lst" ,    GEN_DIR .. "emu/cpu/h8/h8d.hxx",      { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8-300 disassembler source file...",   PYTHON .. " $(1) $(<) d o   $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/h8/h8.lst" ,    GEN_DIR .. "emu/cpu/h8/h8hd.hxx",     { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8-300H disassembler source file...",  PYTHON .. " $(1) $(<) d h   $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/h8/h8.lst" ,    GEN_DIR .. "emu/cpu/h8/h8s2000d.hxx", { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8S/2000 disassembler source file...", PYTHON .. " $(1) $(<) d s20 $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/h8/h8.lst" ,    GEN_DIR .. "emu/cpu/h8/h8s2600d.hxx", { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating H8S/2600 disassembler source file...", PYTHON .. " $(1) $(<) d s26 $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/h8/gt913.lst" , GEN_DIR .. "emu/cpu/h8/gt913d.hxx",   { MAME_DIR .. "src/devices/cpu/h8/h8make.py" }, {"@echo Generating GT913 disassembler source file...",    PYTHON .. " $(1) $(<) d g   $(@)" }})

	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/h8/h8d.cpp",       GEN_DIR .. "emu/cpu/h8/h8d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/h8/h8hd.cpp",      GEN_DIR .. "emu/cpu/h8/h8hd.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/h8/h8s2000d.cpp",  GEN_DIR .. "emu/cpu/h8/h8s2000d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/h8/h8s2600d.cpp",  GEN_DIR .. "emu/cpu/h8/h8s2600d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/h8/gt913d.cpp",    GEN_DIR .. "emu/cpu/h8/gt913d.hxx" })

	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/h8/h8d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/h8/h8d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/h8/h8hd.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/h8/h8hd.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/h8/h8s2000d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/h8/h8s2000d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/h8/h8s2600d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/h8/h8s2600d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/h8/gt913d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/h8/gt913d.h")
end

--------------------------------------------------
-- Hitachi H8/500 series
--@src/devices/cpu/h8500/h8500.h,CPUS["H8500"] = true
--------------------------------------------------

if CPUS["H8500"] then
	files {
		MAME_DIR .. "src/devices/cpu/h8500/h8500.cpp",
		MAME_DIR .. "src/devices/cpu/h8500/h8500.h",
		MAME_DIR .. "src/devices/cpu/h8500/h8510.cpp",
		MAME_DIR .. "src/devices/cpu/h8500/h8510.h",
		MAME_DIR .. "src/devices/cpu/h8500/h8520.cpp",
		MAME_DIR .. "src/devices/cpu/h8500/h8520.h",
		MAME_DIR .. "src/devices/cpu/h8500/h8532.cpp",
		MAME_DIR .. "src/devices/cpu/h8500/h8532.h",
		MAME_DIR .. "src/devices/cpu/h8500/h8534.cpp",
		MAME_DIR .. "src/devices/cpu/h8500/h8534.h",
	}
end

if opt_tool(CPUS, "H8500") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/h8500/h8500dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/h8500/h8500dasm.h")
end

--------------------------------------------------
-- Hitachi HCD62121
--@src/devices/cpu/hcd62121/hcd62121.h,CPUS["HCD62121"] = true
--------------------------------------------------

if CPUS["HCD62121"] then
	files {
		MAME_DIR .. "src/devices/cpu/hcd62121/hcd62121.cpp",
		MAME_DIR .. "src/devices/cpu/hcd62121/hcd62121.h",
	}
end

if opt_tool(CPUS, "HCD62121") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hcd62121/hcd62121d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hcd62121/hcd62121d.h")
end

--------------------------------------------------
-- Hitachi HMCS40 series
--@src/devices/cpu/hmcs40/hmcs40.h,CPUS["HMCS40"] = true
--------------------------------------------------

if CPUS["HMCS40"] then
	files {
		MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40.cpp",
		MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40.h",
		MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40op.cpp",
	}
end

if opt_tool(CPUS, "HMCS40") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hmcs40/hmcs40d.h")
end

--------------------------------------------------
-- Hitachi SuperH series (SH1/SH2/SH3/SH4)
--@src/devices/cpu/sh/sh2.h,CPUS["SH"] = true
--@src/devices/cpu/sh/sh4.h,CPUS["SH"] = true
--------------------------------------------------

if CPUS["SH"] then
	files {
		MAME_DIR .. "src/devices/cpu/sh/sh_fe.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh.h",
		MAME_DIR .. "src/devices/cpu/sh/sh2.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh2.h",
		MAME_DIR .. "src/devices/cpu/sh/sh2fe.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh3comn.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh3comn.h",
		MAME_DIR .. "src/devices/cpu/sh/sh4.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh4.h",
		MAME_DIR .. "src/devices/cpu/sh/sh4comn.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh4comn.h",
		MAME_DIR .. "src/devices/cpu/sh/sh4dmac.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh4dmac.h",
		MAME_DIR .. "src/devices/cpu/sh/sh4fe.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh4regs.h",
		MAME_DIR .. "src/devices/cpu/sh/sh4tmu.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh4tmu.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_bsc.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_bsc.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_dmac.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_dmac.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_intc.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_intc.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_mtu.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_mtu.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_port.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_port.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_sci.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7014_sci.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7014.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7014.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7021.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7021.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7032.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7032.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7042.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7042.h",
		MAME_DIR .. "src/devices/cpu/sh/sh_adc.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh_adc.h",
		MAME_DIR .. "src/devices/cpu/sh/sh_bsc.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh_bsc.h",
		MAME_DIR .. "src/devices/cpu/sh/sh_cmt.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh_cmt.h",
		MAME_DIR .. "src/devices/cpu/sh/sh_dmac.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh_dmac.h",
		MAME_DIR .. "src/devices/cpu/sh/sh_intc.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh_intc.h",
		MAME_DIR .. "src/devices/cpu/sh/sh_mtu.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh_mtu.h",
		MAME_DIR .. "src/devices/cpu/sh/sh_port.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh_port.h",
		MAME_DIR .. "src/devices/cpu/sh/sh_sci.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh_sci.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7604_bus.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7604_bus.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7604_sci.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7604_sci.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7604_wdt.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7604_wdt.h",
		MAME_DIR .. "src/devices/cpu/sh/sh7604.cpp",
		MAME_DIR .. "src/devices/cpu/sh/sh7604.h",
	}
end

if opt_tool(CPUS, "SH") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sh/sh_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sh/sh_dasm.h")
end

--------------------------------------------------
-- STmicro ST62xx
--@src/devices/cpu/st62xx/st62xx.h,CPUS["ST62XX"] = true
--------------------------------------------------

if CPUS["ST62XX"] then
	files {
		MAME_DIR .. "src/devices/cpu/st62xx/st62xx.cpp",
		MAME_DIR .. "src/devices/cpu/st62xx/st62xx.h",
	}
end

if opt_tool(CPUS, "ST62XX") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/st62xx/st62xx_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/st62xx/st62xx_dasm.h")
end

--------------------------------------------------
-- HP Hybrid processor
--@src/devices/cpu/hphybrid/hphybrid.h,CPUS["HPHYBRID"] = true
--------------------------------------------------

if CPUS["HPHYBRID"] then
	files {
		MAME_DIR .. "src/devices/cpu/hphybrid/hphybrid.cpp",
		MAME_DIR .. "src/devices/cpu/hphybrid/hphybrid.h",
	}
end

if opt_tool(CPUS, "HPHYBRID") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hphybrid/hphybrid_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hphybrid/hphybrid_dasm.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hphybrid/hphybrid_defs.h")
end

--------------------------------------------------
-- HP Nanoprocessor
--@src/devices/cpu/nanoprocessor/nanoprocessor.h,CPUS["NANOPROCESSOR"] = true
--------------------------------------------------

if CPUS["NANOPROCESSOR"] then
	files {
		MAME_DIR .. "src/devices/cpu/nanoprocessor/nanoprocessor.cpp",
		MAME_DIR .. "src/devices/cpu/nanoprocessor/nanoprocessor.h",
	}
end

if opt_tool(CPUS, "NANOPROCESSOR") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nanoprocessor/nanoprocessor_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nanoprocessor/nanoprocessor_dasm.h")
end

--------------------------------------------------
-- HP Capricorn
--@src/devices/cpu/capricorn/capricorn.h,CPUS["CAPRICORN"] = true
--------------------------------------------------

if CPUS["CAPRICORN"] then
	files {
		MAME_DIR .. "src/devices/cpu/capricorn/capricorn.cpp",
		MAME_DIR .. "src/devices/cpu/capricorn/capricorn.h",
	}
end

if opt_tool(CPUS, "CAPRICORN") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/capricorn/capricorn_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/capricorn/capricorn_dasm.h")
end

--------------------------------------------------
-- Hudsonsoft 6280
--@src/devices/cpu/h6280/h6280.h,CPUS["H6280"] = true
--------------------------------------------------

if CPUS["H6280"] then
	files {
		MAME_DIR .. "src/devices/cpu/h6280/h6280.cpp",
		MAME_DIR .. "src/devices/cpu/h6280/h6280.h",
	}
end

if opt_tool(CPUS, "H6280") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/h6280/6280dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/h6280/6280dasm.h")
end

--------------------------------------------------
-- Hyperstone E1 series
--@src/devices/cpu/e132xs/e132xs.h,CPUS["E1"] = true
--------------------------------------------------

if CPUS["E1"] then
	files {
		MAME_DIR .. "src/devices/cpu/e132xs/e132xs.cpp",
		MAME_DIR .. "src/devices/cpu/e132xs/e132xs.h",
		MAME_DIR .. "src/devices/cpu/e132xs/32xsdefs.h",
		MAME_DIR .. "src/devices/cpu/e132xs/e132xsop.hxx",
		MAME_DIR .. "src/devices/cpu/e132xs/e132xsfe.cpp",
		MAME_DIR .. "src/devices/cpu/e132xs/e132xsdrc.cpp",
		MAME_DIR .. "src/devices/cpu/e132xs/e132xsdrc_ops.hxx",
	}
end

if opt_tool(CPUS, "E1") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/e132xs/32xsdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/e132xs/32xsdasm.h")
end

--------------------------------------------------
-- 15IE-00-013 CPU ("Microprogrammed Control Device")
--@src/devices/cpu/ie15/ie15.h,CPUS["IE15"] = true
--------------------------------------------------

if CPUS["IE15"] then
	files {
		MAME_DIR .. "src/devices/cpu/ie15/ie15.cpp",
		MAME_DIR .. "src/devices/cpu/ie15/ie15.h",
	}
end

if opt_tool(CPUS, "IE15") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ie15/ie15dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ie15/ie15dasm.h")
end

--------------------------------------------------
-- Intel MCS-40
--@src/devices/cpu/mcs40/mcs40.h,CPUS["MCS40"] = true
--------------------------------------------------

if CPUS["MCS40"] then
	files {
		MAME_DIR .. "src/devices/cpu/mcs40/mcs40.cpp",
		MAME_DIR .. "src/devices/cpu/mcs40/mcs40.h",
	}
end

if opt_tool(CPUS, "MCS40") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mcs40/mcs40dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mcs40/mcs40dasm.h")
end

--------------------------------------------------
-- Intel 8008
--@src/devices/cpu/i8008/i8008.h,CPUS["I8008"] = true
--------------------------------------------------

if CPUS["I8008"] then
	files {
		MAME_DIR .. "src/devices/cpu/i8008/i8008.cpp",
		MAME_DIR .. "src/devices/cpu/i8008/i8008.h",
	}
end

if opt_tool(CPUS, "I8008") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8008/8008dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8008/8008dasm.h")
end

--------------------------------------------------
--  National Semiconductor SC/MP
--@src/devices/cpu/scmp/scmp.h,CPUS["SCMP"] = true
--------------------------------------------------

if CPUS["SCMP"] then
	files {
		MAME_DIR .. "src/devices/cpu/scmp/scmp.cpp",
		MAME_DIR .. "src/devices/cpu/scmp/scmp.h",
	}
end

if opt_tool(CPUS, "SCMP") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/scmp/scmpdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/scmp/scmpdasm.h")
end

--------------------------------------------------
-- Intel 8080/8085A
--@src/devices/cpu/i8085/i8085.h,CPUS["I8085"] = true
--------------------------------------------------

if CPUS["I8085"] then
	files {
		MAME_DIR .. "src/devices/cpu/i8085/i8085.cpp",
		MAME_DIR .. "src/devices/cpu/i8085/i8085.h",
	}
end

if opt_tool(CPUS, "I8085") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8085/8085dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8085/8085dasm.h")
end

--------------------------------------------------
-- Intel 8089
--@src/devices/cpu/i8089/i8089.h,CPUS["I8089"] = true
--------------------------------------------------

if CPUS["I8089"] then
	files {
		MAME_DIR .. "src/devices/cpu/i8089/i8089.cpp",
		MAME_DIR .. "src/devices/cpu/i8089/i8089.h",
		MAME_DIR .. "src/devices/cpu/i8089/i8089_channel.cpp",
		MAME_DIR .. "src/devices/cpu/i8089/i8089_channel.h",
		MAME_DIR .. "src/devices/cpu/i8089/i8089_ops.cpp",
	}
end

if opt_tool(CPUS, "I8089") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8089/i8089_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i8089/i8089_dasm.h")
end

--------------------------------------------------
-- Intel MCS-48 (8039 and derivatives)
--@src/devices/cpu/mcs48/mcs48.h,CPUS["MCS48"] = true
--------------------------------------------------

if CPUS["MCS48"] then
	files {
		MAME_DIR .. "src/devices/cpu/mcs48/mcs48.cpp",
		MAME_DIR .. "src/devices/cpu/mcs48/mcs48.h",
	}
end

if opt_tool(CPUS, "MCS48") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mcs48/mcs48dsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mcs48/mcs48dsm.h")
end

--------------------------------------------------
-- Intel 8051 and derivatives
--@src/devices/cpu/mcs51/mcs51.h,CPUS["MCS51"] = true
--------------------------------------------------

if CPUS["MCS51"] then
	files {
		MAME_DIR .. "src/devices/cpu/mcs51/mcs51.cpp",
		MAME_DIR .. "src/devices/cpu/mcs51/mcs51.h",
		MAME_DIR .. "src/devices/cpu/mcs51/mcs51ops.hxx",
	}
end

if opt_tool(CPUS, "MCS51") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mcs51/mcs51dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mcs51/mcs51dasm.h")
end

--------------------------------------------------
-- Appotech AXC-51 (extended Intel 8051)
--@src/devices/cpu/axc51/axc51.h,CPUS["AXC51"] = true
--------------------------------------------------

if CPUS["AXC51"] then
	files {
		MAME_DIR .. "src/devices/cpu/axc51/axc51.cpp",
		MAME_DIR .. "src/devices/cpu/axc51/axc51.h",
		MAME_DIR .. "src/devices/cpu/axc51/axc51ops.hxx",
		MAME_DIR .. "src/devices/cpu/axc51/axc51extops.hxx",
	}
end

if opt_tool(CPUS, "AXC51") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/axc51/axc51dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/acx51/axc51dasm.h")
end


--------------------------------------------------
-- Philips XA (80c51 inspired)
--@src/devices/cpu/xa/xa.h,CPUS["XA"] = true
--------------------------------------------------

if CPUS["XA"] then
	files {
		MAME_DIR .. "src/devices/cpu/xa/xa.cpp",
		MAME_DIR .. "src/devices/cpu/xa/xa_ops.cpp",
		MAME_DIR .. "src/devices/cpu/xa/xa.h",
	}
end

if opt_tool(CPUS, "XA") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/xa/xadasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/xa/xadasm.h")
end


--------------------------------------------------
-- Intel MCS-96
--@src/devices/cpu/mcs96/mcs96.h,CPUS["MCS96"] = true
--------------------------------------------------

if CPUS["MCS96"] then
	files {
		MAME_DIR .. "src/devices/cpu/mcs96/mcs96.cpp",
		MAME_DIR .. "src/devices/cpu/mcs96/mcs96.h",
		MAME_DIR .. "src/devices/cpu/mcs96/i8x9x.cpp",
		MAME_DIR .. "src/devices/cpu/mcs96/i8x9x.h",
		MAME_DIR .. "src/devices/cpu/mcs96/i8xc196.cpp",
		MAME_DIR .. "src/devices/cpu/mcs96/i8xc196.h",
	}
	dependency {
		{ MAME_DIR .. "src/devices/cpu/mcs96/mcs96.cpp",   GEN_DIR .. "emu/cpu/mcs96/mcs96.hxx" },
		{ MAME_DIR .. "src/devices/cpu/mcs96/i8x9x.cpp",   GEN_DIR .. "emu/cpu/mcs96/i8x9x.hxx" },
		{ MAME_DIR .. "src/devices/cpu/mcs96/i8xc196.cpp", GEN_DIR .. "emu/cpu/mcs96/i8xc196.hxx" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/mcs96/mcs96ops.lst" , GEN_DIR .. "emu/cpu/mcs96/mcs96.hxx",   { MAME_DIR .. "src/devices/cpu/mcs96/mcs96make.py" }, {"@echo Generating mcs96 source file...", PYTHON .. " $(1) s mcs96 $(<) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/mcs96/mcs96ops.lst" , GEN_DIR .. "emu/cpu/mcs96/i8x9x.hxx",   { MAME_DIR .. "src/devices/cpu/mcs96/mcs96make.py" }, {"@echo Generating i8x9x source file...", PYTHON .. " $(1) s i8x9x $(<) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/mcs96/mcs96ops.lst" , GEN_DIR .. "emu/cpu/mcs96/i8xc196.hxx", { MAME_DIR .. "src/devices/cpu/mcs96/mcs96make.py" }, {"@echo Generating i8xc196 source file...", PYTHON .. " $(1) s i8xc196 $(<) $(@)" }},
	}
end

if opt_tool(CPUS, "MCS96") then
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/mcs96/mcs96ops.lst",  GEN_DIR .. "emu/cpu/mcs96/i8x9xd.hxx",   { MAME_DIR .. "src/devices/cpu/mcs96/mcs96make.py" }, {"@echo Generating i8x9x disassembler source file...", PYTHON .. " $(1) d i8x9x $(<) $(2) $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/mcs96/mcs96ops.lst",  GEN_DIR .. "emu/cpu/mcs96/i8xc196d.hxx", { MAME_DIR .. "src/devices/cpu/mcs96/mcs96make.py" }, {"@echo Generating i8xc196 disassembler source file...", PYTHON .. " $(1) d i8xc196 $(<) $(2) $(@)" }})

	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/mcs96/i8x9xd.cpp",   GEN_DIR .. "emu/cpu/mcs96/i8x9xd.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/mcs96/i8xc196d.cpp", GEN_DIR .. "emu/cpu/mcs96/i8xc196d.hxx" })

	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/mcs96/mcs96d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/mcs96/i8x9xd.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/mcs96/i8xc196d.cpp")
 end

--------------------------------------------------
-- Intel 80x86 series (also a dynamic recompiler target)
--@src/devices/cpu/i86/i86.h,CPUS["I86"] = true
--@src/devices/cpu/i86/i286.h,CPUS["I86"] = true
--@src/devices/cpu/i386/i386.h,CPUS["I386"] = true
--------------------------------------------------

if CPUS["I86"] then
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

-- Beware that opt_tool can set the value, so we want both to be executed always
local want_disasm_i86  = opt_tool(CPUS, "I86")
local want_disasm_i386 = opt_tool(CPUS, "I386")
if want_disasm_i86 or want_disasm_i386 or CPU_INCLUDE_DRC then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i386/i386dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i386/i386dasm.h")
end

if CPUS["I386"] then
	files {
		MAME_DIR .. "src/devices/cpu/i386/i386.cpp",
		MAME_DIR .. "src/devices/cpu/i386/i386.h",
		MAME_DIR .. "src/devices/cpu/i386/athlon.cpp",
		MAME_DIR .. "src/devices/cpu/i386/athlon.h",
		MAME_DIR .. "src/devices/cpu/i386/cache.h",
		MAME_DIR .. "src/devices/cpu/i386/cycles.h",
		MAME_DIR .. "src/devices/cpu/i386/i386op16.hxx",
		MAME_DIR .. "src/devices/cpu/i386/i386op32.hxx",
		MAME_DIR .. "src/devices/cpu/i386/i386ops.h",
		MAME_DIR .. "src/devices/cpu/i386/i386ops.hxx",
		MAME_DIR .. "src/devices/cpu/i386/i386priv.h",
		MAME_DIR .. "src/devices/cpu/i386/i386segs.hxx",
		MAME_DIR .. "src/devices/cpu/i386/i486ops.hxx",
		MAME_DIR .. "src/devices/cpu/i386/pentops.hxx",
		MAME_DIR .. "src/devices/cpu/i386/x87ops.hxx",
		MAME_DIR .. "src/devices/cpu/i386/x87priv.h",
		MAME_DIR .. "src/devices/cpu/i386/cpuidmsrs.hxx",
	}
end

--------------------------------------------------
-- Intel i860
--@src/devices/cpu/i860/i860.h,CPUS["I860"] = true
--------------------------------------------------

if CPUS["I860"] then
	files {
		MAME_DIR .. "src/devices/cpu/i860/i860.cpp",
		MAME_DIR .. "src/devices/cpu/i860/i860.h",
		MAME_DIR .. "src/devices/cpu/i860/i860dec.hxx",
	}
end

if opt_tool(CPUS, "I860") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i860/i860dis.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i860/i860dis.h")
end

--------------------------------------------------
-- Intel i960
--@src/devices/cpu/i960/i960.h,CPUS["I960"] = true
--------------------------------------------------

if CPUS["I960"] then
	files {
		MAME_DIR .. "src/devices/cpu/i960/i960.cpp",
		MAME_DIR .. "src/devices/cpu/i960/i960.h",
	}
end

if opt_tool(CPUS, "I960") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i960/i960dis.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/i960/i960dis.h")
end

--------------------------------------------------
-- LH5801
--@src/devices/cpu/lh5801/lh5801.h,CPUS["LH5801"] = true
--------------------------------------------------

if CPUS["LH5801"] then
	files {
		MAME_DIR .. "src/devices/cpu/lh5801/lh5801.cpp",
		MAME_DIR .. "src/devices/cpu/lh5801/lh5801.h",
		MAME_DIR .. "src/devices/cpu/lh5801/5801tbl.hxx",
	}
end

if opt_tool(CPUS, "LH5801") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lh5801/5801dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lh5801/5801dasm.h")
end
--------
------------------------------------------
-- Manchester Small-Scale Experimental Machine
--@src/devices/cpu/ssem/ssem.h,CPUS["SSEM"] = true
--------------------------------------------------

if CPUS["SSEM"] then
	files {
		MAME_DIR .. "src/devices/cpu/ssem/ssem.cpp",
		MAME_DIR .. "src/devices/cpu/ssem/ssem.h",
	}
end

if opt_tool(CPUS, "SSEM") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ssem/ssemdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ssem/ssemdasm.h")
end

------------------------------------------
-- Diablo Systems printer CPU
--@src/devices/cpu/diablo/diablo1300.h,CPUS["DIABLO"] = true
--------------------------------------------------

if CPUS["DIABLO"] then
	files {
		MAME_DIR .. "src/devices/cpu/diablo/diablo1300.cpp",
		MAME_DIR .. "src/devices/cpu/diablo/diablo1300.h",
	}
end

if opt_tool(CPUS, "DIABLO") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/diablo/diablo1300dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/diablo/diablo1300dasm.h")
end

--------------------------------------------------
-- Fujitsu MB88xx
--@src/devices/cpu/mb88xx/mb88xx.h,CPUS["MB88XX"] = true
--------------------------------------------------

if CPUS["MB88XX"] then
	files {
		MAME_DIR .. "src/devices/cpu/mb88xx/mb88xx.cpp",
		MAME_DIR .. "src/devices/cpu/mb88xx/mb88xx.h",
	}
end

if opt_tool(CPUS, "MB88XX") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb88xx/mb88dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb88xx/mb88dasm.h")
end

--------------------------------------------------
-- Fujitsu MB86233
--@src/devices/cpu/mb86233/mb86233.h,CPUS["MB86233"] = true
--------------------------------------------------

if CPUS["MB86233"] then
	files {
		MAME_DIR .. "src/devices/cpu/mb86233/mb86233.cpp",
		MAME_DIR .. "src/devices/cpu/mb86233/mb86233.h",
	}
end

if opt_tool(CPUS, "MB86233") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb86233/mb86233d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb86233/mb86233d.h")
end

--------------------------------------------------
-- Fujitsu MB86235
--@src/devices/cpu/mb86235/mb86235.h,CPUS["MB86235"] = true
--------------------------------------------------

if CPUS["MB86235"] then
	files {
		MAME_DIR .. "src/devices/cpu/mb86235/mb86235.cpp",
		MAME_DIR .. "src/devices/cpu/mb86235/mb86235.h",
		MAME_DIR .. "src/devices/cpu/mb86235/mb86235drc.cpp",
		MAME_DIR .. "src/devices/cpu/mb86235/mb86235fe.cpp",
		MAME_DIR .. "src/devices/cpu/mb86235/mb86235fe.h",
		MAME_DIR .. "src/devices/cpu/mb86235/mb86235ops.cpp",
	}
end

if opt_tool(CPUS, "MB86235") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb86235/mb86235d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mb86235/mb86235d.h")
end

--------------------------------------------------
-- Microchip PIC16C5x
--@src/devices/cpu/pic16c5x/pic16c5x.h,CPUS["PIC16C5X"] = true
--------------------------------------------------

if CPUS["PIC16C5X"] then
	files {
		MAME_DIR .. "src/devices/cpu/pic16c5x/pic16c5x.cpp",
		MAME_DIR .. "src/devices/cpu/pic16c5x/pic16c5x.h",
	}
end

if opt_tool(CPUS, "PIC16C5X") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16c5x/16c5xdsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16c5x/16c5xdsm.h")
end

--------------------------------------------------
-- PIC1670 - Disassembler only temporarily
--@src/devices/cpu/pic1670/pic1670.h,CPUS["PIC1670"] = true
--------------------------------------------------

if opt_tool(CPUS, "PIC1670") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic1670/pic1670d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic1670/pic1670d.h")
end

--------------------------------------------------
-- Microchip PIC16C62x
--@src/devices/cpu/pic16c62x/pic16c62x.h,CPUS["PIC16C62X"] = true
--------------------------------------------------

if CPUS["PIC16C62X"] then
	files {
		MAME_DIR .. "src/devices/cpu/pic16c62x/pic16c62x.cpp",
		MAME_DIR .. "src/devices/cpu/pic16c62x/pic16c62x.h",
	}
end

if opt_tool(CPUS, "PIC16C62X") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16c62x/16c62xdsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16c62x/16c62xdsm.h")
end

--------------------------------------------------
-- Microchip PIC16x8x
--@src/devices/cpu/pic16x8x/pic16x8x.h,CPUS["PIC16X8X"] = true
--------------------------------------------------

if CPUS["PIC16X8X"] then
	files {
		MAME_DIR .. "src/devices/cpu/pic16x8x/pic16x8x.cpp",
		MAME_DIR .. "src/devices/cpu/pic16x8x/pic16x8x.h",
	}
end

if opt_tool(CPUS, "PIC16X8X") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16x8x/16x8xdsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16x8x/16x8xdsm.h")
end

--------------------------------------------------
-- Generic PIC16 - Disassembler only
--@src/devices/cpu/pic16/pic16.h,CPUS["PIC16"] = true
--------------------------------------------------

if opt_tool(CPUS, "PIC16") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16/pic16d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic16/pic16d.h")
end

--------------------------------------------------
-- Microchip PIC17
--@src/devices/cpu/pic17/pic17.h,CPUS["PIC17"] = true
--------------------------------------------------

if CPUS["PIC17"] then
	files {
		MAME_DIR .. "src/devices/cpu/pic17/pic17.cpp",
		MAME_DIR .. "src/devices/cpu/pic17/pic17.h",
		MAME_DIR .. "src/devices/cpu/pic17/pic17c4x.cpp",
		MAME_DIR .. "src/devices/cpu/pic17/pic17c4x.h",
	}
end

if opt_tool(CPUS, "PIC17") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic17/pic17d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pic17/pic17d.h")
end

--------------------------------------------------
-- MIPS R3000 (MIPS I/II) series
--@src/devices/cpu/mips/mips1.h,CPUS["MIPS1"] = true
--------------------------------------------------

if CPUS["MIPS1"] then
	files {
		MAME_DIR .. "src/devices/cpu/mips/mips1.cpp",
		MAME_DIR .. "src/devices/cpu/mips/mips1.h",
	}
end

if opt_tool(CPUS, "MIPS1") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mips/mips1dsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mips/mips1dsm.h")
end

--------------------------------------------------
-- MIPS R4000 (MIPS III/IV) series
--@src/devices/cpu/mips/mips3.h,CPUS["MIPS3"] = true
--@src/devices/cpu/mips/r4000.h,CPUS["MIPS3"] = true
--------------------------------------------------

if CPUS["MIPS3"] then
	files {
		MAME_DIR .. "src/devices/cpu/mips/mips3com.cpp",
		MAME_DIR .. "src/devices/cpu/mips/mips3com.h",
		MAME_DIR .. "src/devices/cpu/mips/mips3.cpp",
		MAME_DIR .. "src/devices/cpu/mips/mips3.h",
		MAME_DIR .. "src/devices/cpu/mips/mips3fe.cpp",
		MAME_DIR .. "src/devices/cpu/mips/mips3fe.h",
		MAME_DIR .. "src/devices/cpu/mips/mips3drc.cpp",
		MAME_DIR .. "src/devices/cpu/mips/o2dprintf.hxx",
		MAME_DIR .. "src/devices/cpu/mips/ps2vu.cpp",
		MAME_DIR .. "src/devices/cpu/mips/ps2vu.h",
		MAME_DIR .. "src/devices/cpu/mips/ps2vif1.cpp",
		MAME_DIR .. "src/devices/cpu/mips/ps2vif1.h",
		MAME_DIR .. "src/devices/cpu/mips/r4000.cpp",
		MAME_DIR .. "src/devices/cpu/mips/r4000.h",
	}
end

if opt_tool(CPUS, "MIPS3") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mips/mips3dsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mips/mips3dsm.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mips/vudasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mips/vudasm.h")
end

--------------------------------------------------
-- Sony PlayStation CPU (R3000-based + GTE)
--@src/devices/cpu/psx/psx.h,CPUS["PSX"] = true
--------------------------------------------------

if CPUS["PSX"] then
	files {
		MAME_DIR .. "src/devices/cpu/psx/psx.cpp",
		MAME_DIR .. "src/devices/cpu/psx/psx.h",
		MAME_DIR .. "src/devices/cpu/psx/psxdefs.h",
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
	}
end

if opt_tool(CPUS, "PSX") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/psx/psxdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/psx/psxdasm.h")
end

--------------------------------------------------
-- MIPS-X
--@src/devices/cpu/mipsx/mipsx.h,CPUS["MIPSX"] = true
--------------------------------------------------

if CPUS["MIPSX"] then
	files {
		MAME_DIR .. "src/devices/cpu/mipsx/mipsx.cpp",
		MAME_DIR .. "src/devices/cpu/mipsx/mipsx.h",
	}
end

if opt_tool(CPUS, "MIPSX") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mipsx/mipsxdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mipsx/mipsxdasm.h")
end

--------------------------------------------------
-- Mitsubishi MELPS 4 series
--@src/devices/cpu/melps4/melps4.h,CPUS["MELPS4"] = true
--------------------------------------------------

if CPUS["MELPS4"] then
	files {
		MAME_DIR .. "src/devices/cpu/melps4/melps4.cpp",
		MAME_DIR .. "src/devices/cpu/melps4/melps4.h",
		MAME_DIR .. "src/devices/cpu/melps4/melps4op.cpp",
		MAME_DIR .. "src/devices/cpu/melps4/m58846.cpp",
		MAME_DIR .. "src/devices/cpu/melps4/m58846.h",
	}
end

if opt_tool(CPUS, "MELPS4") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/melps4/melps4d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/melps4/melps4d.h")
end

--------------------------------------------------
-- Mitsubishi M16C, disassembler only
--------------------------------------------------

if opt_tool(CPUS, "M16C") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m16c/m16cdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m16c/m16cdasm.h")
end

--------------------------------------------------
-- Mitsubishi M32C, disassembler only
--------------------------------------------------

if opt_tool(CPUS, "M32C") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m32c/m32cdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m32c/m32cdasm.h")
end

--------------------------------------------------
-- Mitsubishi M37702 and M37710 (based on 65C816)
--@src/devices/cpu/m37710/m37710.h,CPUS["M37710"] = true
--------------------------------------------------

if CPUS["M37710"] then
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
	}
end

if opt_tool(CPUS, "M37710") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m37710/m7700ds.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m37710/m7700ds.h")
end

--------------------------------------------------
-- MOS Technology 6502 and its many derivatives
--@src/devices/cpu/m6502/deco16.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/gew7.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/gew12.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m3745x.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m37640.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m4510.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m50734.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m5074x.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6500_1.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6502.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6503.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6504.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6507.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6509.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6510.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m6510t.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m65ce02.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m65c02.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m65sc02.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m740.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m7501.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/m8502.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/r65c02.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/r65c19.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/rp2a03.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/st2xxx.h,CPUS["ST2XXX"] = true
--@src/devices/cpu/m6502/st2204.h,CPUS["ST2XXX"] = true
--@src/devices/cpu/m6502/st2205u.h,CPUS["ST2XXX"] = true
--@src/devices/cpu/m6502/w65c02s.h,CPUS["M6502"] = true
--@src/devices/cpu/m6502/xavix.h,CPUS["XAVIX"] = true
--@src/devices/cpu/m6502/xavix.h,CPUS["XAVIX2000"] = true

--------------------------------------------------

if CPUS["M6502"] then
	files {
		MAME_DIR .. "src/devices/cpu/m6502/deco16.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/deco16.h",
		MAME_DIR .. "src/devices/cpu/m6502/gew7.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/gew7.h",
		MAME_DIR .. "src/devices/cpu/m6502/gew12.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/gew12.h",
		MAME_DIR .. "src/devices/cpu/m6502/m3745x.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m3745x.h",
		MAME_DIR .. "src/devices/cpu/m6502/m37640.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m37640.h",
		MAME_DIR .. "src/devices/cpu/m6502/m4510.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m4510.h",
		MAME_DIR .. "src/devices/cpu/m6502/m50734.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m50734.h",
		MAME_DIR .. "src/devices/cpu/m6502/m5074x.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m5074x.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6500_1.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m6500_1.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6502.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m6502.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6502mcu.h",
		MAME_DIR .. "src/devices/cpu/m6502/m6502mcu.ipp",
		MAME_DIR .. "src/devices/cpu/m6502/m6503.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m6503.h",
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
		MAME_DIR .. "src/devices/cpu/m6502/m65c02.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m65c02.h",
		MAME_DIR .. "src/devices/cpu/m6502/m65ce02.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m65ce02.h",
		MAME_DIR .. "src/devices/cpu/m6502/m65sc02.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m65sc02.h",
		MAME_DIR .. "src/devices/cpu/m6502/m740.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m740.h",
		MAME_DIR .. "src/devices/cpu/m6502/m7501.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m7501.h",
		MAME_DIR .. "src/devices/cpu/m6502/m8502.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/m8502.h",
		MAME_DIR .. "src/devices/cpu/m6502/r65c02.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/r65c02.h",
		MAME_DIR .. "src/devices/cpu/m6502/r65c19.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/r65c19.h",
		MAME_DIR .. "src/devices/cpu/m6502/rp2a03.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/rp2a03.h",
		MAME_DIR .. "src/devices/cpu/m6502/w65c02s.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/w65c02s.h",
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/m6502/odeco16.lst",  GEN_DIR .. "emu/cpu/m6502/deco16.hxx",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/ddeco16.lst"  }, {"@echo Generating deco16 instruction source file...", PYTHON .. " $(1) s deco16 $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om4510.lst",   GEN_DIR .. "emu/cpu/m6502/m4510.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm4510.lst"   }, {"@echo Generating m4510 instruction source file...", PYTHON .. " $(1) s m4510 $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om6502.lst",   GEN_DIR .. "emu/cpu/m6502/m6502.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm6502.lst"   }, {"@echo Generating m6502 instruction source file...", PYTHON .. " $(1) s m6502 $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om65c02.lst",  GEN_DIR .. "emu/cpu/m6502/m65c02.hxx",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm65c02.lst"  }, {"@echo Generating m65c02 instruction source file...", PYTHON .. " $(1) s m65c02 $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om65ce02.lst", GEN_DIR .. "emu/cpu/m6502/m65ce02.hxx", { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm65ce02.lst" }, {"@echo Generating m65ce02 instruction source file...", PYTHON .. " $(1) s m65ce02 $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om6509.lst",   GEN_DIR .. "emu/cpu/m6502/m6509.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm6509.lst"   }, {"@echo Generating m6509 instruction source file...", PYTHON .. " $(1) s m6509 $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om6510.lst",   GEN_DIR .. "emu/cpu/m6502/m6510.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm6510.lst"   }, {"@echo Generating m6510 instruction source file...", PYTHON .. " $(1) s m6510 $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/om740.lst" ,   GEN_DIR .. "emu/cpu/m6502/m740.hxx",    { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm740.lst"    }, {"@echo Generating m740 instruction source file...", PYTHON .. " $(1) s m740 $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/dr65c02.lst",  GEN_DIR .. "emu/cpu/m6502/r65c02.hxx",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",                                                     }, {"@echo Generating r65c02 instruction source file...", PYTHON .. " $(1) s r65c02 - $(<) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/or65c19.lst",  GEN_DIR .. "emu/cpu/m6502/r65c19.hxx",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dr65c19.lst"  }, {"@echo Generating r65c19 instruction source file...", PYTHON .. " $(1) s r65c19 $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/orp2a03.lst",  GEN_DIR .. "emu/cpu/m6502/rp2a03.hxx",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/drp2a03.lst"  }, {"@echo Generating rp2a03 instruction source file...", PYTHON .. " $(1) s rp2a03_core $(<) $(2) $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6502/ow65c02s.lst", GEN_DIR .. "emu/cpu/m6502/w65c02s.hxx", { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dw65c02s.lst" }, {"@echo Generating w65c02s instruction source file...", PYTHON .. " $(1) s w65c02s $(<) $(2) $(@)" }},
	}

	dependency {
		{ MAME_DIR .. "src/devices/cpu/m6502/deco16.cpp",   GEN_DIR .. "emu/cpu/m6502/deco16.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m4510.cpp",    GEN_DIR .. "emu/cpu/m6502/m4510.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m6502.cpp",    GEN_DIR .. "emu/cpu/m6502/m6502.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m6509.cpp",    GEN_DIR .. "emu/cpu/m6502/m6509.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m6510.cpp",    GEN_DIR .. "emu/cpu/m6502/m6510.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m65c02.cpp",   GEN_DIR .. "emu/cpu/m6502/m65c02.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m65ce02.cpp",  GEN_DIR .. "emu/cpu/m6502/m65ce02.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6502/m740.cpp",     GEN_DIR .. "emu/cpu/m6502/m740.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6502/r65c02.cpp",   GEN_DIR .. "emu/cpu/m6502/r65c02.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6502/r65c19.cpp",   GEN_DIR .. "emu/cpu/m6502/r65c19.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6502/rp2a03.cpp",   GEN_DIR .. "emu/cpu/m6502/rp2a03.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6502/w65c02s.cpp",  GEN_DIR .. "emu/cpu/m6502/w65c02s.hxx" },
	}
end

if CPUS["ST2XXX"] then
	files {
		MAME_DIR .. "src/devices/cpu/m6502/st2xxx.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/st2xxx.h",
		MAME_DIR .. "src/devices/cpu/m6502/st2204.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/st2204.h",
		MAME_DIR .. "src/devices/cpu/m6502/st2205u.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/st2205u.h",
	}
end

if CPUS["XAVIX"] then
	files {
		MAME_DIR .. "src/devices/cpu/m6502/xavix.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/xavix.h",
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/m6502/oxavix.lst",   GEN_DIR .. "emu/cpu/m6502/xavix.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dxavix.lst"   }, {"@echo Generating xavix instruction source file...", PYTHON .. " $(1) s xavix $(<) $(2) $(@)" }},
	}

	dependency {
		{ MAME_DIR .. "src/devices/cpu/m6502/xavix.cpp",    GEN_DIR .. "emu/cpu/m6502/xavix.hxx" },
	}
end

if CPUS["XAVIX2000"] then
	files {
		MAME_DIR .. "src/devices/cpu/m6502/xavix2000.cpp",
		MAME_DIR .. "src/devices/cpu/m6502/xavix2000.h",
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/m6502/oxavix2000.lst",   GEN_DIR .. "emu/cpu/m6502/xavix2000.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dxavix2000.lst"   }, {"@echo Generating xavix2000 instruction source file...", PYTHON .. " $(1) s xavix2000 $(<) $(2) $(@)" }},
	}

	dependency {
		{ MAME_DIR .. "src/devices/cpu/m6502/xavix2000.cpp",    GEN_DIR .. "emu/cpu/m6502/xavix2000.hxx" },
	}
end

if opt_tool(CPUS, "M6502") then
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/odeco16.lst",  GEN_DIR .. "emu/cpu/m6502/deco16d.hxx",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/ddeco16.lst"  }, {"@echo Generating deco16 disassembler source file...", PYTHON .. " $(1) d deco16 $(<) $(2) $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/om4510.lst",   GEN_DIR .. "emu/cpu/m6502/m4510d.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm4510.lst"   }, {"@echo Generating m4510 disassembler source file...", PYTHON .. " $(1) d m4510 $(<) $(2) $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/om6502.lst",   GEN_DIR .. "emu/cpu/m6502/m6502d.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm6502.lst"   }, {"@echo Generating m6502 disassembler source file...", PYTHON .. " $(1) d m6502 $(<) $(2) $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/om65c02.lst",  GEN_DIR .. "emu/cpu/m6502/m65c02d.hxx",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm65c02.lst"  }, {"@echo Generating m65c02 disassembler source file...", PYTHON .. " $(1) d m65c02 $(<) $(2) $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/om65ce02.lst", GEN_DIR .. "emu/cpu/m6502/m65ce02d.hxx", { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm65ce02.lst" }, {"@echo Generating m65ce02 disassembler source file...", PYTHON .. " $(1) d m65ce02 $(<) $(2) $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/om6509.lst",   GEN_DIR .. "emu/cpu/m6502/m6509d.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm6509.lst"   }, {"@echo Generating m6509 disassembler source file...", PYTHON .. " $(1) d m6509 $(<) $(2) $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/om6510.lst",   GEN_DIR .. "emu/cpu/m6502/m6510d.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm6510.lst"   }, {"@echo Generating m6510 disassembler source file...", PYTHON .. " $(1) d m6510 $(<) $(2) $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/om740.lst" ,   GEN_DIR .. "emu/cpu/m6502/m740d.hxx",    { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dm740.lst"    }, {"@echo Generating m740 disassembler source file...", PYTHON .. " $(1) d m740 $(<) $(2) $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/dr65c02.lst",  GEN_DIR .. "emu/cpu/m6502/r65c02d.hxx",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",                                                     }, {"@echo Generating r65c02 disassembler source file...", PYTHON .. " $(1) d r65c02 - $(<) $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/or65c19.lst",  GEN_DIR .. "emu/cpu/m6502/r65c19d.hxx",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dr65c19.lst"  }, {"@echo Generating r65c19 disassembler source file...", PYTHON .. " $(1) d r65c19 $(<) $(2) $(@)" }})
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/orp2a03.lst",  GEN_DIR .. "emu/cpu/m6502/rp2a03d.hxx",  { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/drp2a03.lst"  }, {"@echo Generating rp2a03 disassembler source file...", PYTHON .. " $(1) d rp2a03 $(<) $(2) $(@)" }})

	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/deco16d.cpp",   GEN_DIR .. "emu/cpu/m6502/deco16d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/m4510d.cpp",    GEN_DIR .. "emu/cpu/m6502/m4510d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/m6502d.cpp",    GEN_DIR .. "emu/cpu/m6502/m6502d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/m6509d.cpp",    GEN_DIR .. "emu/cpu/m6502/m6509d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/m6510d.cpp",    GEN_DIR .. "emu/cpu/m6502/m6510d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/m65c02d.cpp",   GEN_DIR .. "emu/cpu/m6502/m65c02d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/m65ce02d.cpp",  GEN_DIR .. "emu/cpu/m6502/m65ce02d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/m740d.cpp",     GEN_DIR .. "emu/cpu/m6502/m740d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/r65c02d.cpp",   GEN_DIR .. "emu/cpu/m6502/r65c02d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/r65c19d.cpp",   GEN_DIR .. "emu/cpu/m6502/r65c19d.hxx" })
	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/rp2a03d.cpp",   GEN_DIR .. "emu/cpu/m6502/rp2a03d.hxx" })

	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/deco16d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/deco16d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m4510d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m4510d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m6502d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m6502d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m6509d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m6509d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m6510d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m6510d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m65c02d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m65c02d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m65ce02d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m65ce02d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m740d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/m740d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/r65c02d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/r65c02d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/r65c19d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/r65c19d.h")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/rp2a03d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/rp2a03d.h")
end

if opt_tool(CPUS, "XAVIX") then
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/oxavix.lst",   GEN_DIR .. "emu/cpu/m6502/xavixd.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dxavix.lst"   }, {"@echo Generating xavix disassembler source file...", PYTHON .. " $(1) d xavix $(<) $(2) $(@)" }})

	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/xavixd.cpp",    GEN_DIR .. "emu/cpu/m6502/xavixd.hxx" })

	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/xavixd.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/xavixd.h")
end

if opt_tool(CPUS, "XAVIX2000") then
	table.insert(disasm_custombuildtask, { MAME_DIR .. "src/devices/cpu/m6502/oxavix2000.lst",   GEN_DIR .. "emu/cpu/m6502/xavix2000d.hxx",   { MAME_DIR .. "src/devices/cpu/m6502/m6502make.py",   MAME_DIR  .. "src/devices/cpu/m6502/dxavix2000.lst"   }, {"@echo Generating xavix2000 disassembler source file...", PYTHON .. " $(1) d xavix2000 $(<) $(2) $(@)" }})

	table.insert(disasm_dependency, { MAME_DIR .. "src/devices/cpu/m6502/xavix2000d.cpp",    GEN_DIR .. "emu/cpu/m6502/xavix2000d.hxx" })

	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/xavix2000d.cpp")
	table.insert(disasm_files, MAME_DIR .. "src/devices/cpu/m6502/xavix2000d.h")
end

--------------------------------------------------
-- Motorola 680x
--@src/devices/cpu/m6800/m6800.h,CPUS["M6800"] = true
--@src/devices/cpu/m6800/m6801.h,CPUS["M6800"] = true
--------------------------------------------------

if CPUS["M6800"] then
	files {
		MAME_DIR .. "src/devices/cpu/m6800/m6800.cpp",
		MAME_DIR .. "src/devices/cpu/m6800/m6800.h",
		MAME_DIR .. "src/devices/cpu/m6800/m6801.cpp",
		MAME_DIR .. "src/devices/cpu/m6800/m6801.h",
		MAME_DIR .. "src/devices/cpu/m6800/6800ops.hxx",
	}
end

if opt_tool(CPUS, "M6800") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6800/6800dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6800/6800dasm.h")
end

--------------------------------------------------
-- Motorola 6805
--@src/devices/cpu/m6805/m6805.h,CPUS["M6805"] = true
--------------------------------------------------

if CPUS["M6805"] then
	files {
		MAME_DIR .. "src/devices/cpu/m6805/m6805.cpp",
		MAME_DIR .. "src/devices/cpu/m6805/m6805.h",
		MAME_DIR .. "src/devices/cpu/m6805/m6805defs.h",
		MAME_DIR .. "src/devices/cpu/m6805/6805ops.hxx",
		MAME_DIR .. "src/devices/cpu/m6805/m68705.cpp",
		MAME_DIR .. "src/devices/cpu/m6805/m68705.h",
		MAME_DIR .. "src/devices/cpu/m6805/m68hc05.cpp",
		MAME_DIR .. "src/devices/cpu/m6805/m68hc05.h",
		MAME_DIR .. "src/devices/cpu/m6805/m68hc05e1.cpp",
		MAME_DIR .. "src/devices/cpu/m6805/m68hc05e1.h",
	}
end

if opt_tool(CPUS, "M6805") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6805/6805dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6805/6805dasm.h")
end

--------------------------------------------------
-- Motorola 6809
--@src/devices/cpu/m6809/m6809.h,CPUS["M6809"] = true
--@src/devices/cpu/m6809/hd6309.h,CPUS["M6809"] = true
--@src/devices/cpu/m6809/konami.h,CPUS["M6809"] = true
--------------------------------------------------

if CPUS["M6809"] then
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
		{ MAME_DIR .. "src/devices/cpu/m6809/m6809.cpp",   GEN_DIR .. "emu/cpu/m6809/m6809.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6809/hd6309.cpp",  GEN_DIR .. "emu/cpu/m6809/hd6309.hxx" },
		{ MAME_DIR .. "src/devices/cpu/m6809/konami.cpp",  GEN_DIR .. "emu/cpu/m6809/konami.hxx" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/m6809/m6809.lst"  , GEN_DIR .. "emu/cpu/m6809/m6809.hxx",   { MAME_DIR .. "src/devices/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/devices/cpu/m6809/base6x09.lst"  }, {"@echo Generating m6809 source file...", PYTHON .. " $(1) $(<) > $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6809/hd6309.lst" , GEN_DIR .. "emu/cpu/m6809/hd6309.hxx",  { MAME_DIR .. "src/devices/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/devices/cpu/m6809/base6x09.lst"  }, {"@echo Generating hd6309 source file...", PYTHON .. " $(1) $(<) > $(@)" }},
		{ MAME_DIR .. "src/devices/cpu/m6809/konami.lst" , GEN_DIR .. "emu/cpu/m6809/konami.hxx",  { MAME_DIR .. "src/devices/cpu/m6809/m6809make.py"  , MAME_DIR .. "src/devices/cpu/m6809/base6x09.lst"  }, {"@echo Generating konami source file...", PYTHON .. " $(1) $(<) > $(@)" }},
	}
end

if opt_tool(CPUS, "M6809") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6809/6x09dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m6809/6x09dasm.h")
end

--------------------------------------------------
-- Motorola 68HC11
--@src/devices/cpu/mc68hc11/mc68hc11.h,CPUS["MC68HC11"] = true
--------------------------------------------------

if CPUS["MC68HC11"] then
	files {
		MAME_DIR .. "src/devices/cpu/mc68hc11/mc68hc11.cpp",
		MAME_DIR .. "src/devices/cpu/mc68hc11/mc68hc11.h",
		MAME_DIR .. "src/devices/cpu/mc68hc11/hc11ops.h",
		MAME_DIR .. "src/devices/cpu/mc68hc11/hc11ops.hxx",
	}
end

if opt_tool(CPUS, "MC68HC11") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mc68hc11/hc11dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mc68hc11/hc11dasm.h")
end

--------------------------------------------------
-- Motorola 68000 series
--@src/devices/cpu/m68000/m68000.h,CPUS["M680X0"] = true
--@src/devices/cpu/m68000/m68008.h,CPUS["M680X0"] = true
--@src/devices/cpu/m68000/m68010.h,CPUS["M680X0"] = true
--@src/devices/cpu/m68000/m68020.h,CPUS["M680X0"] = true
--@src/devices/cpu/m68000/m68030.h,CPUS["M680X0"] = true
--@src/devices/cpu/m68000/m68040.h,CPUS["M680X0"] = true
--@src/devices/cpu/m68000/scc68070.h,CPUS["M680X0"] = true
--@src/devices/cpu/m68000/fscpu32.h,CPUS["M680X0"] = true
--@src/devices/cpu/m68000/mcf5206e.h,CPUS["M680X0"] = true
--@src/devices/cpu/m68000/tmp68301.h,CPUS["M680X0"] = true
--------------------------------------------------

if CPUS["M680X0"] then
	files {
		MAME_DIR .. "src/devices/cpu/m68000/m68kcpu.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68kcpu.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68kops.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68kops.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68kfpu.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68kmmu.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68kmusashi.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68kcommon.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68kcommon.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68000.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000.lst",
		MAME_DIR .. "src/devices/cpu/m68000/m68000gen.py",
		MAME_DIR .. "src/devices/cpu/m68000/m68000-decode.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000-head.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68000-sdf.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000-sif.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000-sdp.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000-sip.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000mcu-head.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68000mcu-sdfm.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000mcu-sifm.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000mcu-sdpm.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000mcu-sipm.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000mcu.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68000mcu.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68008-head.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68008-sdf8.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68008-sif8.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68008-sdp8.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68008-sip8.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68008.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68008.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68000musashi.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68000musashi.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68010.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68010.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68020.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68020.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68030.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68030.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/m68040.h",
		MAME_DIR .. "src/devices/cpu/m68000/m68040.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/scc68070.h",
		MAME_DIR .. "src/devices/cpu/m68000/scc68070.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/fscpu32.h",
		MAME_DIR .. "src/devices/cpu/m68000/fscpu32.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/mcf5206e.h",
		MAME_DIR .. "src/devices/cpu/m68000/mcf5206e.cpp",
		MAME_DIR .. "src/devices/cpu/m68000/tmp68301.h",
		MAME_DIR .. "src/devices/cpu/m68000/tmp68301.cpp",
	}
end

if opt_tool(CPUS, "M680X0") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m68000/m68kdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m68000/m68kdasm.h")
end

--------------------------------------------------
-- Motorola/Freescale DSP56156
--@src/devices/cpu/dsp56156/dsp56156.h,CPUS["DSP56156"] = true
--------------------------------------------------

if CPUS["DSP56156"] then
	files {
		MAME_DIR .. "src/devices/cpu/dsp56156/dsp56156.cpp",
		MAME_DIR .. "src/devices/cpu/dsp56156/dsp56156.h",
		MAME_DIR .. "src/devices/cpu/dsp56156/dsp56mem.cpp",
		MAME_DIR .. "src/devices/cpu/dsp56156/dsp56mem.h",
		MAME_DIR .. "src/devices/cpu/dsp56156/dsp56pcu.cpp",
		MAME_DIR .. "src/devices/cpu/dsp56156/dsp56pcu.h",
		MAME_DIR .. "src/devices/cpu/dsp56156/dsp56def.h",
		MAME_DIR .. "src/devices/cpu/dsp56156/dsp56ops.hxx",
	}
end

if opt_tool(CPUS, "DSP56156") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56156/dsp56dsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56156/dsp56dsm.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56156/opcode.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56156/opcode.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56156/inst.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56156/inst.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56156/pmove.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56156/pmove.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56156/tables.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56156/tables.h")
end

--------------------------------------------------
-- PDP-1
--@src/devices/cpu/pdp1/pdp1.h,CPUS["PDP1"] = true
--------------------------------------------------

if CPUS["PDP1"] then
	files {
		MAME_DIR .. "src/devices/cpu/pdp1/pdp1.cpp",
		MAME_DIR .. "src/devices/cpu/pdp1/pdp1.h",
	}
end

if opt_tool(CPUS, "PDP1") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pdp1/pdp1dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pdp1/pdp1dasm.h")
end

--------------------------------------------------
-- PATINHO FEIO - Escola Politecnica - USP (Brazil)
--@src/devices/cpu/patinhofeio/patinhofeio_cpu.h,CPUS["PATINHOFEIO"] = true
--------------------------------------------------

if CPUS["PATINHOFEIO"] then
	files {
		MAME_DIR .. "src/devices/cpu/patinhofeio/patinho_feio.cpp",
		MAME_DIR .. "src/devices/cpu/patinhofeio/patinhofeio_cpu.h",
	}
end

if opt_tool(CPUS, "PATINHOFEIO") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/patinhofeio/patinho_feio_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/patinhofeio/patinho_feio_dasm.h")
end

--------------------------------------------------
-- Motorola PowerPC series
--@src/devices/cpu/powerpc/ppc.h,CPUS["POWERPC"] = true
--------------------------------------------------

if CPUS["POWERPC"] then
	files {
		MAME_DIR .. "src/devices/cpu/powerpc/ppccom.cpp",
		MAME_DIR .. "src/devices/cpu/powerpc/ppccom.h",
		MAME_DIR .. "src/devices/cpu/powerpc/ppcfe.cpp",
		MAME_DIR .. "src/devices/cpu/powerpc/ppcfe.h",
		MAME_DIR .. "src/devices/cpu/powerpc/ppcdrc.cpp",
		MAME_DIR .. "src/devices/cpu/powerpc/ppc.h",
	}
end

if opt_tool(CPUS, "POWERPC") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/powerpc/ppc_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/powerpc/ppc_dasm.h")
end

--------------------------------------------------
-- NEC V-series Intel-compatible
--@src/devices/cpu/nec/nec.h,CPUS["NEC"] = true
--@src/devices/cpu/nec/v25.h,CPUS["NEC"] = true
--@src/devices/cpu/nec/v5x.h,CPUS["NEC"] = true
--@src/devices/cpu/v30mz/v30mz.h,CPUS["V30MZ"] = true
--------------------------------------------------

if CPUS["NEC"] then
	files {
		MAME_DIR .. "src/devices/cpu/nec/nec.cpp",
		MAME_DIR .. "src/devices/cpu/nec/nec.h",
		MAME_DIR .. "src/devices/cpu/nec/necea.h",
		MAME_DIR .. "src/devices/cpu/nec/necinstr.h",
		MAME_DIR .. "src/devices/cpu/nec/necinstr.hxx",
		MAME_DIR .. "src/devices/cpu/nec/nec80inst.hxx",
		MAME_DIR .. "src/devices/cpu/nec/necmacro.h",
		MAME_DIR .. "src/devices/cpu/nec/necmodrm.h",
		MAME_DIR .. "src/devices/cpu/nec/necpriv.ipp",
		MAME_DIR .. "src/devices/cpu/nec/v25instr.h",
		MAME_DIR .. "src/devices/cpu/nec/v25instr.hxx",
		MAME_DIR .. "src/devices/cpu/nec/v25priv.ipp",
		MAME_DIR .. "src/devices/cpu/nec/v25.cpp",
		MAME_DIR .. "src/devices/cpu/nec/v25.h",
		MAME_DIR .. "src/devices/cpu/nec/v25sfr.cpp",
		MAME_DIR .. "src/devices/cpu/nec/v5x.cpp",
		MAME_DIR .. "src/devices/cpu/nec/v5x.h",
	}
end

if opt_tool(CPUS, "NEC") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nec/necdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nec/necdasm.h")
end

if CPUS["V30MZ"] then
	files {
		MAME_DIR .. "src/devices/cpu/v30mz/v30mz.cpp",
		MAME_DIR .. "src/devices/cpu/v30mz/v30mz.h",
	}
end

if opt_tool(CPUS, "V30MZ") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nec/necdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nec/necdasm.h")
end

--------------------------------------------------
-- NEC V60/V70
--@src/devices/cpu/v60/v60.h,CPUS["V60"] = true
--------------------------------------------------

if CPUS["V60"] then
	files {
		MAME_DIR .. "src/devices/cpu/v60/v60.cpp",
		MAME_DIR .. "src/devices/cpu/v60/v60.h",
		MAME_DIR .. "src/devices/cpu/v60/am.hxx",
		MAME_DIR .. "src/devices/cpu/v60/am1.hxx",
		MAME_DIR .. "src/devices/cpu/v60/am2.hxx",
		MAME_DIR .. "src/devices/cpu/v60/am3.hxx",
		MAME_DIR .. "src/devices/cpu/v60/op12.hxx",
		MAME_DIR .. "src/devices/cpu/v60/op2.hxx",
		MAME_DIR .. "src/devices/cpu/v60/op3.hxx",
		MAME_DIR .. "src/devices/cpu/v60/op4.hxx",
		MAME_DIR .. "src/devices/cpu/v60/op5.hxx",
		MAME_DIR .. "src/devices/cpu/v60/op6.hxx",
		MAME_DIR .. "src/devices/cpu/v60/op7a.hxx",
		MAME_DIR .. "src/devices/cpu/v60/optable.hxx",
	}
end

if opt_tool(CPUS, "V60") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v60/v60d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v60/v60d.h")
end

--------------------------------------------------
-- NEC V810 (uPD70732)
--@src/devices/cpu/v810/v810.h,CPUS["V810"] = true
--------------------------------------------------

if CPUS["V810"] then
	files {
		MAME_DIR .. "src/devices/cpu/v810/v810.cpp",
		MAME_DIR .. "src/devices/cpu/v810/v810.h",
	}
end

if opt_tool(CPUS, "V810") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v810/v810dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v810/v810dasm.h")
end

--------------------------------------------------
-- NEC V850, disassembler only
--------------------------------------------------

if opt_tool(CPUS, "V850") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v850/v850dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v850/v850dasm.h")
end

--------------------------------------------------
-- NEC uPD7725
--@src/devices/cpu/upd7725/upd7725.h,CPUS["UPD7725"] = true
--------------------------------------------------

if CPUS["UPD7725"] then
	files {
		MAME_DIR .. "src/devices/cpu/upd7725/upd7725.cpp",
		MAME_DIR .. "src/devices/cpu/upd7725/upd7725.h",
	}
end

if opt_tool(CPUS, "UPD7725") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd7725/dasm7725.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd7725/dasm7725.h")
end

--------------------------------------------------
-- NEC uPD7810 series
--@src/devices/cpu/upd7810/upd7810.h,CPUS["UPD7810"] = true
--@src/devices/cpu/upd7810/upd7811.h,CPUS["UPD7810"] = true
--------------------------------------------------

if CPUS["UPD7810"] then
	files {
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810.cpp",
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810.h",
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810_opcodes.cpp",
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810_table.cpp",
		MAME_DIR .. "src/devices/cpu/upd7810/upd7810_macros.h",
	}
end

if opt_tool(CPUS, "UPD7810") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd7810/upd7810_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd7810/upd7810_dasm.h")
end

--------------------------------------------------
-- NEC uCOM-4 series
--@src/devices/cpu/ucom4/ucom4.h,CPUS["UCOM4"] = true
--------------------------------------------------

if CPUS["UCOM4"] then
	files {
		MAME_DIR .. "src/devices/cpu/ucom4/ucom4.cpp",
		MAME_DIR .. "src/devices/cpu/ucom4/ucom4.h",
		MAME_DIR .. "src/devices/cpu/ucom4/ucom4op.cpp",
	}
end

if opt_tool(CPUS, "UCOM4") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ucom4/ucom4d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ucom4/ucom4d.h")
end

--------------------------------------------------
-- Nintendo Minx
--@src/devices/cpu/minx/minx.h,CPUS["MINX"] = true
--------------------------------------------------

if CPUS["MINX"] then
	files {
		MAME_DIR .. "src/devices/cpu/minx/minx.cpp",
		MAME_DIR .. "src/devices/cpu/minx/minx.h",
		MAME_DIR .. "src/devices/cpu/minx/minxfunc.h",
		MAME_DIR .. "src/devices/cpu/minx/minxopce.h",
		MAME_DIR .. "src/devices/cpu/minx/minxopcf.h",
		MAME_DIR .. "src/devices/cpu/minx/minxops.h",
	}
end

if opt_tool(CPUS, "MINX") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/minx/minxd.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/minx/minxd.h")
end

--------------------------------------------------
-- Nintendo/SGI RSP (R3000-based + vector processing)
--@src/devices/cpu/rsp/rsp.h,CPUS["RSP"] = true
--------------------------------------------------

if CPUS["RSP"] then
	files {
		MAME_DIR .. "src/devices/cpu/rsp/rsp.cpp",
		MAME_DIR .. "src/devices/cpu/rsp/rsp.h",
		MAME_DIR .. "src/devices/cpu/rsp/rspdefs.h",
		MAME_DIR .. "src/devices/cpu/rsp/rspdiv.h",
	}
end

if opt_tool(CPUS, "RSP") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/rsp/rsp_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/rsp/rsp_dasm.h")
end

--------------------------------------------------
-- Matsushita (Panasonic) MN1400
--@src/devices/cpu/mn1400/mn1400.h,CPUS["MN1400"] = true
--------------------------------------------------

if CPUS["MN1400"] then
	files {
		MAME_DIR .. "src/devices/cpu/mn1400/mn1400base.cpp",
		MAME_DIR .. "src/devices/cpu/mn1400/mn1400base.h",
		MAME_DIR .. "src/devices/cpu/mn1400/mn1400.cpp",
		MAME_DIR .. "src/devices/cpu/mn1400/mn1400.h",
		MAME_DIR .. "src/devices/cpu/mn1400/mn1400op.cpp",
	}
end

if opt_tool(CPUS, "MN1400") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mn1400/mn1400d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mn1400/mn1400d.h")
end

--------------------------------------------------
-- Panafacom MN1610, disassembler only
--@src/devices/cpu/mn1610/mn1610d.h,CPUS["MN1610"] = true
--------------------------------------------------

if opt_tool(CPUS, "MN1610") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mn1610/mn1610d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mn1610/mn1610d.h")
end

--------------------------------------------------
-- Panasonic MN1880
--@src/devices/cpu/mn1880/mn1880.h,CPUS["MN1880"] = true
--------------------------------------------------

if CPUS["MN1880"] then
	files {
		MAME_DIR .. "src/devices/cpu/mn1880/mn1880.cpp",
		MAME_DIR .. "src/devices/cpu/mn1880/mn1880.h",
	}
end

if opt_tool(CPUS, "MN1880") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mn1880/mn1880d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mn1880/mn1880d.h")
end

--------------------------------------------------
-- Panasonic MN10200
--@src/devices/cpu/mn10200/mn10200.h,CPUS["MN10200"] = true
--------------------------------------------------

if CPUS["MN10200"] then
	files {
		MAME_DIR .. "src/devices/cpu/mn10200/mn10200.cpp",
		MAME_DIR .. "src/devices/cpu/mn10200/mn10200.h",
	}
end

if opt_tool(CPUS, "MN10200") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mn10200/mn102dis.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mn10200/mn102dis.h")
end

--------------------------------------------------
-- Saturn
--@src/devices/cpu/saturn/saturn.h,CPUS["SATURN"] = true
--------------------------------------------------

if CPUS["SATURN"] then
	files {
		MAME_DIR .. "src/devices/cpu/saturn/saturn.cpp",
		MAME_DIR .. "src/devices/cpu/saturn/saturn.h",
		MAME_DIR .. "src/devices/cpu/saturn/satops.ipp",
		MAME_DIR .. "src/devices/cpu/saturn/sattable.ipp",
	}
end

if opt_tool(CPUS, "SATURN") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/saturn/saturnds.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/saturn/saturnds.h")
end

--------------------------------------------------
-- Sharp SM510 series
--@src/devices/cpu/sm510/sm510.h,CPUS["SM510"] = true
--@src/devices/cpu/sm510/sm511.h,CPUS["SM510"] = true
--@src/devices/cpu/sm510/sm530.h,CPUS["SM510"] = true
--@src/devices/cpu/sm510/sm590.h,CPUS["SM510"] = true
--@src/devices/cpu/sm510/sm5a.h,CPUS["SM510"] = true
--------------------------------------------------

if CPUS["SM510"] then
	files {
		MAME_DIR .. "src/devices/cpu/sm510/sm510base.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm510base.h",
		MAME_DIR .. "src/devices/cpu/sm510/sm510.h",
		MAME_DIR .. "src/devices/cpu/sm510/sm510.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm510op.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm511.h",
		MAME_DIR .. "src/devices/cpu/sm510/sm511.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm500.h",
		MAME_DIR .. "src/devices/cpu/sm510/sm500.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm500op.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm5a.h",
		MAME_DIR .. "src/devices/cpu/sm510/sm5a.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm530.h",
		MAME_DIR .. "src/devices/cpu/sm510/sm530.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm530op.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm590.h",
		MAME_DIR .. "src/devices/cpu/sm510/sm590.cpp",
		MAME_DIR .. "src/devices/cpu/sm510/sm590op.cpp",
	}
end

if opt_tool(CPUS, "SM510") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sm510/sm510d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sm510/sm510d.h")
end

--------------------------------------------------
-- Sharp SM8500
--@src/devices/cpu/sm8500/sm8500.h,CPUS["SM8500"] = true
--------------------------------------------------

if CPUS["SM8500"] then
	files {
		MAME_DIR .. "src/devices/cpu/sm8500/sm8500.cpp",
		MAME_DIR .. "src/devices/cpu/sm8500/sm8500.h",
		MAME_DIR .. "src/devices/cpu/sm8500/sm85ops.h",
	}
end

if opt_tool(CPUS, "SM8500") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sm8500/sm8500d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sm8500/sm8500d.h")
end

--------------------------------------------------
-- Signetics 2650
--@src/devices/cpu/s2650/s2650.h,CPUS["S2650"] = true
--------------------------------------------------

if CPUS["S2650"] then
	files {
		MAME_DIR .. "src/devices/cpu/s2650/s2650.cpp",
		MAME_DIR .. "src/devices/cpu/s2650/s2650.h",
		MAME_DIR .. "src/devices/cpu/s2650/s2650cpu.h",
	}
end

if opt_tool(CPUS, "S2650") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/s2650/2650dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/s2650/2650dasm.h")
end

--------------------------------------------------
-- SC61860
--@src/devices/cpu/sc61860/sc61860.h,CPUS["SC61860"] = true
--------------------------------------------------

if CPUS["SC61860"] then
	files {
		MAME_DIR .. "src/devices/cpu/sc61860/sc61860.cpp",
		MAME_DIR .. "src/devices/cpu/sc61860/sc61860.h",
		--MAME_DIR .. "src/devices/cpu/sc61860/readpc.cpp",
		MAME_DIR .. "src/devices/cpu/sc61860/scops.hxx",
		MAME_DIR .. "src/devices/cpu/sc61860/sctable.hxx",
	}
end

if opt_tool(CPUS, "SC61860") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sc61860/scdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sc61860/scdasm.h")
end

--------------------------------------------------
-- Sony/Nintendo SPC700
--@src/devices/cpu/spc700/spc700.h,CPUS["SPC700"] = true
--------------------------------------------------

if CPUS["SPC700"] then
	files {
		MAME_DIR .. "src/devices/cpu/spc700/spc700.cpp",
		MAME_DIR .. "src/devices/cpu/spc700/spc700.h",
		MAME_DIR .. "src/devices/cpu/spc700/spc700ds.h",
	}
end

if opt_tool(CPUS, "SPC700") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/spc700/spc700ds.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/spc700/spc700ds.h")
end

--------------------------------------------------
-- SSP1601
--@src/devices/cpu/ssp1601/ssp1601.h,CPUS["SSP1601"] = true
--------------------------------------------------

if CPUS["SSP1601"] then
	files {
		MAME_DIR .. "src/devices/cpu/ssp1601/ssp1601.cpp",
		MAME_DIR .. "src/devices/cpu/ssp1601/ssp1601.h",
	}
end

if opt_tool(CPUS, "SSP1601") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ssp1601/ssp1601d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ssp1601/ssp1601d.h")
end

--------------------------------------------------
-- SunPlus u'nSP
--@src/devices/cpu/unsp/unsp.h,CPUS["UNSP"] = true
--------------------------------------------------

if CPUS["UNSP"] then
	files {
		MAME_DIR .. "src/devices/cpu/unsp/unsp.cpp",
		MAME_DIR .. "src/devices/cpu/unsp/unsp.h",
		MAME_DIR .. "src/devices/cpu/unsp/unsp_extended.cpp",
		MAME_DIR .. "src/devices/cpu/unsp/unsp_jumps.cpp",
		MAME_DIR .. "src/devices/cpu/unsp/unsp_exxx.cpp",
		MAME_DIR .. "src/devices/cpu/unsp/unsp_fxxx.cpp",
		MAME_DIR .. "src/devices/cpu/unsp/unsp_other.cpp",
		MAME_DIR .. "src/devices/cpu/unsp/unspdefs.h",
		MAME_DIR .. "src/devices/cpu/unsp/unspdrc.cpp",
		MAME_DIR .. "src/devices/cpu/unsp/unspfe.cpp",
		MAME_DIR .. "src/devices/cpu/unsp/unspfe.h",
	}
end

if opt_tool(CPUS, "UNSP") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/unsp/unspdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/unsp/unspdasm.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/unsp/unspdasm_extended.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/unsp/unspdasm_jumps.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/unsp/unspdasm_exxx.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/unsp/unspdasm_fxxx.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/unsp/unspdasm_other.cpp")
end

--------------------------------------------------
-- Atmel 8-bit AVR
--@src/devices/cpu/avr8/avr8.h,CPUS["AVR8"] = true
--------------------------------------------------

if CPUS["AVR8"] then
	files {
		MAME_DIR .. "src/devices/cpu/avr8/avr8.cpp",
		MAME_DIR .. "src/devices/cpu/avr8/avr8.h",
	}
end

if opt_tool(CPUS, "AVR8") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/avr8/avr8dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/avr8/avr8dasm.h")
end

--------------------------------------------------
-- Texas Instruments TMS1000 series
--@src/devices/cpu/tms1000/tms1000.h,CPUS["TMS1000"] = true
--@src/devices/cpu/tms1000/tms1000c.h,CPUS["TMS1000"] = true
--@src/devices/cpu/tms1000/tms1100.h,CPUS["TMS1000"] = true
--@src/devices/cpu/tms1000/tms1400.h,CPUS["TMS1000"] = true
--@src/devices/cpu/tms1000/tms2100.h,CPUS["TMS1000"] = true
--@src/devices/cpu/tms1000/tms2400.h,CPUS["TMS1000"] = true
--@src/devices/cpu/tms1000/tms0970.h,CPUS["TMS1000"] = true
--@src/devices/cpu/tms1000/tms0980.h,CPUS["TMS1000"] = true
--@src/devices/cpu/tms1000/tms0270.h,CPUS["TMS1000"] = true
--@src/devices/cpu/tms1000/tp0320.h,CPUS["TMS1000"] = true
--@src/devices/cpu/tms1000/smc1102.h,CPUS["TMS1000"] = true
--------------------------------------------------

if CPUS["TMS1000"] then
	files {
		MAME_DIR .. "src/devices/cpu/tms1000/tms1k_base.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/tms1k_base.h",
		MAME_DIR .. "src/devices/cpu/tms1000/tms1000.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/tms1000.h",
		MAME_DIR .. "src/devices/cpu/tms1000/tms1000c.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/tms1000c.h",
		MAME_DIR .. "src/devices/cpu/tms1000/tms1100.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/tms1100.h",
		MAME_DIR .. "src/devices/cpu/tms1000/tms1400.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/tms1400.h",
		MAME_DIR .. "src/devices/cpu/tms1000/tms2100.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/tms2100.h",
		MAME_DIR .. "src/devices/cpu/tms1000/tms2400.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/tms2400.h",
		MAME_DIR .. "src/devices/cpu/tms1000/tms0970.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/tms0970.h",
		MAME_DIR .. "src/devices/cpu/tms1000/tms0980.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/tms0980.h",
		MAME_DIR .. "src/devices/cpu/tms1000/tms0270.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/tms0270.h",
		MAME_DIR .. "src/devices/cpu/tms1000/tp0320.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/tp0320.h",
		MAME_DIR .. "src/devices/cpu/tms1000/smc1102.cpp",
		MAME_DIR .. "src/devices/cpu/tms1000/smc1102.h",
	}
end

if opt_tool(CPUS, "TMS1000") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms1000/tms1k_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms1000/tms1k_dasm.h")
end

--------------------------------------------------
-- Texas Instruments TMS7000 series
--@src/devices/cpu/tms7000/tms7000.h,CPUS["TMS7000"] = true
--------------------------------------------------

if CPUS["TMS7000"] then
	files {
		MAME_DIR .. "src/devices/cpu/tms7000/tms7000.cpp",
		MAME_DIR .. "src/devices/cpu/tms7000/tms7000.h",
		MAME_DIR .. "src/devices/cpu/tms7000/tms7000op.cpp",
	}
end

if opt_tool(CPUS, "TMS7000") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms7000/7000dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms7000/7000dasm.h")
end

--------------------------------------------------
-- Texas Instruments TMS99xx series
--@src/devices/cpu/tms9900/tms9900.h,CPUS["TMS9900"] = true
--@src/devices/cpu/tms9900/tms9980a.h,CPUS["TMS9900"] = true
--@src/devices/cpu/tms9900/tms9995.h,CPUS["TMS9900"] = true
--@src/devices/cpu/tms9900/ti990_10.h,CPUS["TMS9900"] = true
--------------------------------------------------

if CPUS["TMS9900"] then
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

if opt_tool(CPUS, "TMS9900") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms9900/9900dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms9900/9900dasm.h")
end

--------------------------------------------------
-- Texas Instruments TMS340x0 graphics controllers
--@src/devices/cpu/tms34010/tms34010.h,CPUS["TMS340X0"] = true
--------------------------------------------------

if CPUS["TMS340X0"] then
	files {
		MAME_DIR .. "src/devices/cpu/tms34010/tms34010.cpp",
		MAME_DIR .. "src/devices/cpu/tms34010/tms34010.h",
		MAME_DIR .. "src/devices/cpu/tms34010/34010fld.hxx",
		MAME_DIR .. "src/devices/cpu/tms34010/34010gfx.hxx",
		MAME_DIR .. "src/devices/cpu/tms34010/34010ops.h",
		MAME_DIR .. "src/devices/cpu/tms34010/34010ops.hxx",
		MAME_DIR .. "src/devices/cpu/tms34010/34010tbl.hxx",
	}
end

if opt_tool(CPUS, "TMS340X0") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms34010/34010dsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms34010/34010dsm.h")
end

--------------------------------------------------
-- Texas Instruments TMS3201x DSP
--@src/devices/cpu/tms32010/tms32010.h,CPUS["TMS32010"] = true
--------------------------------------------------

if CPUS["TMS32010"] then
	files {
		MAME_DIR .. "src/devices/cpu/tms32010/tms32010.cpp",
		MAME_DIR .. "src/devices/cpu/tms32010/tms32010.h",
	}
end

if opt_tool(CPUS, "TMS32010") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32010/32010dsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32010/32010dsm.h")
end

--------------------------------------------------
-- Texas Instruments TMS3202x DSP
--@src/devices/cpu/tms32025/tms32025.h,CPUS["TMS32025"] = true
--------------------------------------------------

if CPUS["TMS32025"] then
	files {
		MAME_DIR .. "src/devices/cpu/tms32025/tms32025.cpp",
		MAME_DIR .. "src/devices/cpu/tms32025/tms32025.h",
	}
end

if opt_tool(CPUS, "TMS32025") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32025/32025dsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32025/32025dsm.h")
end

--------------------------------------------------
-- Texas Instruments TMS3203x DSP
--@src/devices/cpu/tms32031/tms32031.h,CPUS["TMS32031"] = true
--------------------------------------------------

if CPUS["TMS32031"] then
	files {
		MAME_DIR .. "src/devices/cpu/tms32031/tms32031.cpp",
		MAME_DIR .. "src/devices/cpu/tms32031/tms32031.h",
		MAME_DIR .. "src/devices/cpu/tms32031/32031ops.hxx",
	}
end

if opt_tool(CPUS, "TMS32031") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32031/dis32031.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32031/dis32031.h")
end

--------------------------------------------------
-- Texas Instruments TMS3205x DSP
--@src/devices/cpu/tms32051/tms32051.h,CPUS["TMS32051"] = true
--------------------------------------------------

if CPUS["TMS32051"] then
	files {
		MAME_DIR .. "src/devices/cpu/tms32051/tms32051.cpp",
		MAME_DIR .. "src/devices/cpu/tms32051/tms32051.h",
		MAME_DIR .. "src/devices/cpu/tms32051/32051ops.h",
		MAME_DIR .. "src/devices/cpu/tms32051/32051ops.hxx",
	}
end

if opt_tool(CPUS, "TMS32051") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32051/dis32051.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32051/dis32051.h")
end

--------------------------------------------------
-- Texas Instruments TMS3208x DSP
--@src/devices/cpu/tms32082/tms32082.h,CPUS["TMS32082"] = true
--------------------------------------------------

if CPUS["TMS32082"] then
	files {
		MAME_DIR .. "src/devices/cpu/tms32082/tms32082.cpp",
		MAME_DIR .. "src/devices/cpu/tms32082/tms32082.h",
		MAME_DIR .. "src/devices/cpu/tms32082/mp_ops.cpp",
	}
end

if opt_tool(CPUS, "TMS32082") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32082/dis_mp.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32082/dis_mp.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32082/dis_pp.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms32082/dis_pp.h")
end

--------------------------------------------------
-- Texas Instruments TMS57002 DSP
--@src/devices/cpu/tms57002/tms57002.h,CPUS["TMS57002"] = true
--------------------------------------------------

if CPUS["TMS57002"] then
	files {
		MAME_DIR .. "src/devices/cpu/tms57002/tms57002.cpp",
		MAME_DIR .. "src/devices/cpu/tms57002/tms57002.h",
		MAME_DIR .. "src/devices/cpu/tms57002/tmsops.cpp",
		MAME_DIR .. "src/devices/cpu/tms57002/tms57kdec.cpp",
	}
	dependency {
		{ MAME_DIR .. "src/devices/cpu/tms57002/tms57kdec.cpp", GEN_DIR .. "emu/cpu/tms57002/tms57002.hxx" },
		{ MAME_DIR .. "src/devices/cpu/tms57002/tms57002.cpp",  GEN_DIR .. "emu/cpu/tms57002/tms57002.hxx" },
		{ MAME_DIR .. "src/devices/cpu/tms57002/tmsops.cpp",  GEN_DIR .. "emu/cpu/tms57002/tms57002.hxx" },
	}
	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/tms57002/tmsinstr.lst" , GEN_DIR .. "emu/cpu/tms57002/tms57002.hxx",   { MAME_DIR .. "src/devices/cpu/tms57002/tmsmake.py" }, {"@echo Generating TMS57002 source file...", PYTHON .. " $(1) s $(<) $(@)" } }
	}
end

if opt_tool(CPUS, "TMS57002") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms57002/57002dsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tms57002/57002dsm.h")
	table.insert(disasm_dependency , { MAME_DIR .. "src/devices/cpu/tms57002/57002dsm.cpp",  GEN_DIR .. "emu/cpu/tms57002/tms57002d.hxx" } )
	table.insert(disasm_dependency , { MAME_DIR .. "src/devices/cpu/tms57002/57002dsm.cpp",  GEN_DIR .. "emu/cpu/tms57002/tms57002.hxx" } )
	table.insert(disasm_custombuildtask , { MAME_DIR .. "src/devices/cpu/tms57002/tmsinstr.lst" , GEN_DIR .. "emu/cpu/tms57002/tms57002d.hxx",   { MAME_DIR .. "src/devices/cpu/tms57002/tmsmake.py" }, {"@echo Generating TMS57002 source file...", PYTHON .. " $(1) d $(<) $(@)" }})
	table.insert(disasm_custombuildtask , { MAME_DIR .. "src/devices/cpu/tms57002/tmsinstr.lst" , GEN_DIR .. "emu/cpu/tms57002/tms57002.hxx",    { MAME_DIR .. "src/devices/cpu/tms57002/tmsmake.py" }, {"@echo Generating TMS57002 source file...", PYTHON .. " $(1) s $(<) $(@)" }})
end

--------------------------------------------------
-- Toshiba TLCS-90 Series
--@src/devices/cpu/tlcs90/tlcs90.h,CPUS["TLCS90"] = true
--------------------------------------------------

if CPUS["TLCS90"] then
	files {
		MAME_DIR .. "src/devices/cpu/tlcs90/tlcs90.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs90/tlcs90.h",
	}
end

if opt_tool(CPUS, "TLCS90") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tlcs90/tlcs90d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tlcs90/tlcs90d.h")
end

--------------------------------------------------
-- Toshiba TLCS-870 Series
--@src/devices/cpu/tlcs870/tlcs870.h,CPUS["TLCS870"] = true
--------------------------------------------------

if CPUS["TLCS870"] then
	files {
		MAME_DIR .. "src/devices/cpu/tlcs870/tlcs870.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs870/tlcs870_ops.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs870/tlcs870_ops_reg.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs870/tlcs870_ops_src.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs870/tlcs870_ops_dst.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs870/tlcs870_ops_helper.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs870/tlcs870.h",
	}
end

if opt_tool(CPUS, "TLCS870") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tlcs870/tlcs870d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tlcs870/tlcs870d.h")
end

--------------------------------------------------
-- Toshiba TLCS-900 Series
--@src/devices/cpu/tlcs900/tlcs900.h,CPUS["TLCS900"] = true
--------------------------------------------------

if CPUS["TLCS900"] then
	files {
		MAME_DIR .. "src/devices/cpu/tlcs900/tlcs900.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs900/tlcs900.h",
		MAME_DIR .. "src/devices/cpu/tlcs900/900tbl.hxx",
		MAME_DIR .. "src/devices/cpu/tlcs900/900htbl.hxx",
		MAME_DIR .. "src/devices/cpu/tlcs900/tmp95c061.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs900/tmp95c061.h",
		MAME_DIR .. "src/devices/cpu/tlcs900/tmp95c063.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs900/tmp95c063.h",
		MAME_DIR .. "src/devices/cpu/tlcs900/tmp96c141.cpp",
		MAME_DIR .. "src/devices/cpu/tlcs900/tmp96c141.h",
	}
end

if opt_tool(CPUS, "TLCS900") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tlcs900/dasm900.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tlcs900/dasm900.h")
end

--------------------------------------------------
-- TX0
--@src/devices/cpu/tx0/tx0.h,CPUS["TX0"] = true
--------------------------------------------------

if CPUS["TX0"] then
	files {
		MAME_DIR .. "src/devices/cpu/tx0/tx0.cpp",
		MAME_DIR .. "src/devices/cpu/tx0/tx0.h",
	}
end

if opt_tool(CPUS, "TX0") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tx0/tx0dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/tx0/tx0dasm.h")
end

--------------------------------------------------
-- Zilog Z80
--@src/devices/cpu/z80/z80.h,CPUS["Z80"] = true
--@src/devices/cpu/z80/z80n.h,CPUS["Z80N"] = true
--@src/devices/cpu/z80/kc82.h,CPUS["KC80"] = true
--@src/devices/cpu/z80/kl5c80a12.h,CPUS["KC80"] = true
--@src/devices/cpu/z80/kl5c80a16.h,CPUS["KC80"] = true
--@src/devices/cpu/z80/ky80.h,CPUS["KC80"] = true
--------------------------------------------------

if CPUS["Z80"] or CPUS["KC80"] or CPUS["Z80N"] then
	files {
		MAME_DIR .. "src/devices/cpu/z80/z80.cpp",
		MAME_DIR .. "src/devices/cpu/z80/z80.h",
		MAME_DIR .. "src/devices/cpu/z80/t6a84.cpp",
		MAME_DIR .. "src/devices/cpu/z80/t6a84.h",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c011.cpp",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c011.h",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c015.cpp",
		MAME_DIR .. "src/devices/cpu/z80/tmpz84c015.h",
		MAME_DIR .. "src/devices/cpu/z80/ez80.cpp",
		MAME_DIR .. "src/devices/cpu/z80/ez80.h",
		MAME_DIR .. "src/devices/cpu/z80/lz8420m.cpp",
		MAME_DIR .. "src/devices/cpu/z80/lz8420m.h",
		MAME_DIR .. "src/devices/cpu/z80/mc8123.cpp",
		MAME_DIR .. "src/devices/cpu/z80/mc8123.h",
		MAME_DIR .. "src/devices/cpu/z80/r800.cpp",
		MAME_DIR .. "src/devices/cpu/z80/r800.h",
		MAME_DIR .. "src/devices/cpu/z80/z84c015.cpp",
		MAME_DIR .. "src/devices/cpu/z80/z84c015.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/cpu/z80/z80.cpp", GEN_DIR .. "emu/cpu/z80/z80.hxx" },
		{ MAME_DIR .. "src/devices/cpu/z80/z80.cpp", GEN_DIR .. "emu/cpu/z80/ncs800.hxx" },
		{ MAME_DIR .. "src/devices/cpu/z80/r800.cpp", GEN_DIR .. "emu/cpu/z80/r800.hxx" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/z80/z80.lst", GEN_DIR .. "emu/cpu/z80/z80.hxx", { MAME_DIR .. "src/devices/cpu/z80/z80make.py" }, { "@echo Generating Z80 source file...",   PYTHON .. "  $(1) $(<) $(@)" } },
		{ MAME_DIR .. "src/devices/cpu/z80/z80.lst", GEN_DIR .. "emu/cpu/z80/ncs800.hxx", { MAME_DIR .. "src/devices/cpu/z80/z80make.py" }, { "@echo Generating NSC800 source file...",   PYTHON .. " $(1) ncs800 $(<) $(@)" } },
		{ MAME_DIR .. "src/devices/cpu/z80/z80.lst", GEN_DIR .. "emu/cpu/z80/r800.hxx", { MAME_DIR .. "src/devices/cpu/z80/z80make.py" }, { "@echo Generating R800 source file...",   PYTHON .. "  $(1) r800 $(<) $(@)" } },
	}
end

if CPUS["Z80N"] then
	files {
		MAME_DIR .. "src/devices/cpu/z80/z80n.cpp",
		MAME_DIR .. "src/devices/cpu/z80/z80n.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/cpu/z80/z80n.cpp", GEN_DIR .. "emu/cpu/z80/z80n.hxx" },
	}

	custombuildtask {
		{ MAME_DIR .. "src/devices/cpu/z80/z80.lst", GEN_DIR .. "emu/cpu/z80/z80n.hxx", { MAME_DIR .. "src/devices/cpu/z80/z80make.py" }, { "@echo Generating Z80N source file...",   PYTHON .. "  $(1) z80n $(<) $(@)" } },
	}
end

if CPUS["KC80"] then
	files {
		MAME_DIR .. "src/devices/cpu/z80/kc82.cpp",
		MAME_DIR .. "src/devices/cpu/z80/kc82.h",
		MAME_DIR .. "src/devices/cpu/z80/kl5c80a12.cpp",
		MAME_DIR .. "src/devices/cpu/z80/kl5c80a12.h",
		MAME_DIR .. "src/devices/cpu/z80/kl5c80a16.cpp",
		MAME_DIR .. "src/devices/cpu/z80/kl5c80a16.h",
		MAME_DIR .. "src/devices/cpu/z80/kp63.cpp",
		MAME_DIR .. "src/devices/cpu/z80/kp63.h",
		MAME_DIR .. "src/devices/cpu/z80/kp64.cpp",
		MAME_DIR .. "src/devices/cpu/z80/kp64.h",
		MAME_DIR .. "src/devices/cpu/z80/kp69.cpp",
		MAME_DIR .. "src/devices/cpu/z80/kp69.h",
		MAME_DIR .. "src/devices/cpu/z80/ky80.cpp",
		MAME_DIR .. "src/devices/cpu/z80/ky80.h",
	}
end

local want_disasm_z80  = opt_tool(CPUS, "Z80")
local want_disasm_kc80 = opt_tool(CPUS, "KC80")

if want_disasm_z80 or want_disasm_kc80 then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z80/r800dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z80/r800dasm.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z80/z80dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z80/z80dasm.h")
end

if opt_tool(CPUS, "Z80N") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z80/z80ndasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z80/z80dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z80/z80ndasm.h")
end

--------------------------------------------------
-- Sharp LR35902 (Game Boy CPU)
--@src/devices/cpu/lr35902/lr35902.h,CPUS["LR35902"] = true
--@src/devices/cpu/lr35902/lr35902d.h,CPUS["LR35902"] = true
--------------------------------------------------

if CPUS["LR35902"] then
	files {
		MAME_DIR .. "src/devices/cpu/lr35902/lr35902.cpp",
		MAME_DIR .. "src/devices/cpu/lr35902/lr35902.h",
		MAME_DIR .. "src/devices/cpu/lr35902/opc_cb.hxx",
		MAME_DIR .. "src/devices/cpu/lr35902/opc_main.hxx",
	}
end

if opt_tool(CPUS, "LR35902") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lr35902/lr35902d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lr35902/lr35902d.h")
end

--------------------------------------------------
-- Zilog Z180
--@src/devices/cpu/z180/z180.h,CPUS["Z180"] = true
--------------------------------------------------

if CPUS["Z180"] then
	files {
		MAME_DIR .. "src/devices/cpu/z180/hd647180x.cpp",
		MAME_DIR .. "src/devices/cpu/z180/hd647180x.h",
		MAME_DIR .. "src/devices/cpu/z180/z180.cpp",
		MAME_DIR .. "src/devices/cpu/z180/z180.h",
		MAME_DIR .. "src/devices/cpu/z180/z180cb.hxx",
		MAME_DIR .. "src/devices/cpu/z180/z180dd.hxx",
		MAME_DIR .. "src/devices/cpu/z180/z180ed.hxx",
		MAME_DIR .. "src/devices/cpu/z180/z180fd.hxx",
		MAME_DIR .. "src/devices/cpu/z180/z180op.hxx",
		MAME_DIR .. "src/devices/cpu/z180/z180xy.hxx",
		MAME_DIR .. "src/devices/cpu/z180/z180ops.h",
		MAME_DIR .. "src/devices/cpu/z180/z180tbl.h",
		MAME_DIR .. "src/devices/cpu/z180/z180asci.cpp",
		MAME_DIR .. "src/devices/cpu/z180/z180asci.h",
		MAME_DIR .. "src/devices/cpu/z180/z180csio.cpp",
		MAME_DIR .. "src/devices/cpu/z180/z180csio.h",
	}
end

if opt_tool(CPUS, "Z180") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z180/z180dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z180/z180dasm.h")
end

--------------------------------------------------
-- Zilog Z8000
--@src/devices/cpu/z8000/z8000.h,CPUS["Z8000"] = true
--------------------------------------------------

if CPUS["Z8000"] then
	files {
		MAME_DIR .. "src/devices/cpu/z8000/z8000.cpp",
		MAME_DIR .. "src/devices/cpu/z8000/z8000.h",
		--MAME_DIR .. "src/devices/cpu/z8000/makedab.cpp",
		MAME_DIR .. "src/devices/cpu/z8000/z8000cpu.h",
		MAME_DIR .. "src/devices/cpu/z8000/z8000dab.h",
		MAME_DIR .. "src/devices/cpu/z8000/z8000ops.hxx",
		MAME_DIR .. "src/devices/cpu/z8000/z8000tbl.hxx",
	}
end

if opt_tool(CPUS, "Z8000") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z8000/8000dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z8000/8000dasm.h")
end

--------------------------------------------------
-- Zilog Z8
--@src/devices/cpu/z8/z8.h,CPUS["Z8"] = true
--------------------------------------------------

if CPUS["Z8"] then
	files {
		MAME_DIR .. "src/devices/cpu/z8/z8.cpp",
		MAME_DIR .. "src/devices/cpu/z8/z8.h",
		MAME_DIR .. "src/devices/cpu/z8/z8ops.hxx",
	}
end

if opt_tool(CPUS, "Z8") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z8/z8dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/z8/z8dasm.h")
end

--------------------------------------------------
-- Argonaut SuperFX
--@src/devices/cpu/superfx/superfx.h,CPUS["SUPERFX"] = true
--------------------------------------------------

if CPUS["SUPERFX"] then
	files {
		MAME_DIR .. "src/devices/cpu/superfx/superfx.cpp",
		MAME_DIR .. "src/devices/cpu/superfx/superfx.h",
	}
end

if opt_tool(CPUS, "SUPERFX") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/superfx/sfx_dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/superfx/sfx_dasm.h")
end

--------------------------------------------------
-- Rockwell A/B5000 family
--@src/devices/cpu/rw5000/a5000.h,CPUS["RW5000"] = true
--@src/devices/cpu/rw5000/a5500.h,CPUS["RW5000"] = true
--@src/devices/cpu/rw5000/a5900.h,CPUS["RW5000"] = true
--@src/devices/cpu/rw5000/b5000.h,CPUS["RW5000"] = true
--@src/devices/cpu/rw5000/b5500.h,CPUS["RW5000"] = true
--@src/devices/cpu/rw5000/b6000.h,CPUS["RW5000"] = true
--@src/devices/cpu/rw5000/b6100.h,CPUS["RW5000"] = true
--------------------------------------------------

if CPUS["RW5000"] then
	files {
		MAME_DIR .. "src/devices/cpu/rw5000/rw5000base.cpp",
		MAME_DIR .. "src/devices/cpu/rw5000/rw5000base.h",
		MAME_DIR .. "src/devices/cpu/rw5000/b5000.cpp",
		MAME_DIR .. "src/devices/cpu/rw5000/b5000.h",
		MAME_DIR .. "src/devices/cpu/rw5000/b5000op.cpp",
		MAME_DIR .. "src/devices/cpu/rw5000/b5500.cpp",
		MAME_DIR .. "src/devices/cpu/rw5000/b5500.h",
		MAME_DIR .. "src/devices/cpu/rw5000/b6000.cpp",
		MAME_DIR .. "src/devices/cpu/rw5000/b6000.h",
		MAME_DIR .. "src/devices/cpu/rw5000/b6100.cpp",
		MAME_DIR .. "src/devices/cpu/rw5000/b6100.h",
		MAME_DIR .. "src/devices/cpu/rw5000/a5000.cpp",
		MAME_DIR .. "src/devices/cpu/rw5000/a5000.h",
		MAME_DIR .. "src/devices/cpu/rw5000/a5500.cpp",
		MAME_DIR .. "src/devices/cpu/rw5000/a5500.h",
		MAME_DIR .. "src/devices/cpu/rw5000/a5900.cpp",
		MAME_DIR .. "src/devices/cpu/rw5000/a5900.h",
	}
end

if opt_tool(CPUS, "RW5000") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/rw5000/rw5000d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/rw5000/rw5000d.h")
end

--------------------------------------------------
-- Rockwell PPS-4
--@src/devices/cpu/pps4/pps4.h,CPUS["PPS4"] = true
--------------------------------------------------

if CPUS["PPS4"] then
	files {
		MAME_DIR .. "src/devices/cpu/pps4/pps4.cpp",
		MAME_DIR .. "src/devices/cpu/pps4/pps4.h",
	}
end

if opt_tool(CPUS, "PPS4") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pps4/pps4dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pps4/pps4dasm.h")
end

--------------------------------------------------
-- Rockwell PPS-4/1
--@src/devices/cpu/pps41/mm75.h,CPUS["PPS41"] = true
--@src/devices/cpu/pps41/mm76.h,CPUS["PPS41"] = true
--@src/devices/cpu/pps41/mm78.h,CPUS["PPS41"] = true
--@src/devices/cpu/pps41/mm78la.h,CPUS["PPS41"] = true
--------------------------------------------------

if CPUS["PPS41"] then
	files {
		MAME_DIR .. "src/devices/cpu/pps41/pps41base.cpp",
		MAME_DIR .. "src/devices/cpu/pps41/pps41base.h",
		MAME_DIR .. "src/devices/cpu/pps41/mm75.cpp",
		MAME_DIR .. "src/devices/cpu/pps41/mm75.h",
		MAME_DIR .. "src/devices/cpu/pps41/mm75op.cpp",
		MAME_DIR .. "src/devices/cpu/pps41/mm76.cpp",
		MAME_DIR .. "src/devices/cpu/pps41/mm76.h",
		MAME_DIR .. "src/devices/cpu/pps41/mm76op.cpp",
		MAME_DIR .. "src/devices/cpu/pps41/mm78.cpp",
		MAME_DIR .. "src/devices/cpu/pps41/mm78.h",
		MAME_DIR .. "src/devices/cpu/pps41/mm78op.cpp",
		MAME_DIR .. "src/devices/cpu/pps41/mm78la.cpp",
		MAME_DIR .. "src/devices/cpu/pps41/mm78la.h",
		MAME_DIR .. "src/devices/cpu/pps41/mm78laop.cpp",
	}
end

if opt_tool(CPUS, "PPS41") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pps41/pps41d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pps41/pps41d.h")
end

--------------------------------------------------
-- Hitachi HD61700
--@src/devices/cpu/hd61700/hd61700.h,CPUS["HD61700"] = true
--------------------------------------------------

if CPUS["HD61700"] then
	files {
		MAME_DIR .. "src/devices/cpu/hd61700/hd61700.cpp",
		MAME_DIR .. "src/devices/cpu/hd61700/hd61700.h",
	}
end

if opt_tool(CPUS, "HD61700") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hd61700/hd61700d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hd61700/hd61700d.h")
end

--------------------------------------------------
-- Sanyo LC8670
--@src/devices/cpu/lc8670/lc8670.h,CPUS["LC8670"] = true
--------------------------------------------------

if CPUS["LC8670"] then
	files {
		MAME_DIR .. "src/devices/cpu/lc8670/lc8670.cpp",
		MAME_DIR .. "src/devices/cpu/lc8670/lc8670.h",
	}
end

if opt_tool(CPUS, "LC8670") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lc8670/lc8670dsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lc8670/lc8670dsm.h")
end

--------------------------------------------------
-- Sega SCU DSP
--@src/devices/cpu/scudsp/scudsp.h,CPUS["SCUDSP"] = true
--------------------------------------------------

if CPUS["SCUDSP"] then
	files {
		MAME_DIR .. "src/devices/cpu/scudsp/scudsp.cpp",
		MAME_DIR .. "src/devices/cpu/scudsp/scudsp.h",
	}
end

if opt_tool(CPUS, "SCUDSP") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/scudsp/scudspdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/scudsp/scudspdasm.h")
end

--------------------------------------------------
-- Sunplus Technology S+core
--@src/devices/cpu/score/score.h,CPUS["SCORE"] = true
--------------------------------------------------

if CPUS["SCORE"] then
	files {
		MAME_DIR .. "src/devices/cpu/score/score.cpp",
		MAME_DIR .. "src/devices/cpu/score/score.h",
		MAME_DIR .. "src/devices/cpu/score/scorem.h",
	}
end

if opt_tool(CPUS, "SCORE") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/score/scoredsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/score/scoredsm.h")
end

--------------------------------------------------
-- Xerox Alto-II
--@src/devices/cpu/alto2/alto2cpu.h,CPUS["ALTO2"] = true
--------------------------------------------------

if CPUS["ALTO2"] then
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

if opt_tool(CPUS, "ALTO2") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/alto2/alto2dsm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/alto2/alto2dsm.h")
end

------------------------------------------
-- Sun SPARCv7, SPARCv8 implementation
--@src/devices/cpu/sparc/sparc.h,CPUS["SPARC"] = true
--------------------------------------------------

if CPUS["SPARC"] then
	files {
		MAME_DIR .. "src/devices/cpu/sparc/sparc.cpp",
		MAME_DIR .. "src/devices/cpu/sparc/sparcdefs.h",
		MAME_DIR .. "src/devices/cpu/sparc/sparc_intf.h",
		MAME_DIR .. "src/devices/cpu/sparc/sparc.h",
	}
end

if opt_tool(CPUS, "SPARC") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sparc/sparcdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sparc/sparcdasm.h")
end

--------------------------------------------------
-- Intergraph CLIPPER (C100/C300/C400) series
--@src/devices/cpu/clipper/clipper.h,CPUS["CLIPPER"] = true
--------------------------------------------------

if CPUS["CLIPPER"] then
	files {
		MAME_DIR .. "src/devices/cpu/clipper/clipper.cpp",
		MAME_DIR .. "src/devices/cpu/clipper/clipper.h",
	}
end

if opt_tool(CPUS, "CLIPPER") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/clipper/clipperd.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/clipper/clipperd.h")
end


--------------------------------------------------
-- VM Labs Nuon, disassembler only
--------------------------------------------------

if opt_tool(CPUS, "NUON") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nuon/nuondasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nuon/nuondasm.h")
end

--------------------------------------------------
-- DEC Alpha (EV4/EV5/EV6/EV7) series
--@src/devices/cpu/alpha/alpha.h,CPUS["ALPHA"] = true
--------------------------------------------------

if CPUS["ALPHA"] then
	files {
		MAME_DIR .. "src/devices/cpu/alpha/alpha.cpp",
		MAME_DIR .. "src/devices/cpu/alpha/alpha.h",
	}
end

if opt_tool(CPUS, "ALPHA") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/alpha/alphad.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/alpha/alphad.h")
end

--------------------------------------------------
-- Hewlett-Packard HP2100 (disassembler only)
--@src/devices/cpu/hp2100/hp2100.h,CPUS["HP2100"] = true
--------------------------------------------------

if opt_tool(CPUS, "HP2100") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hp2100/hp2100d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hp2100/hp2100d.h")
end

--------------------------------------------------
-- SDS Sigma 2 (disassembler only)
--@src/devices/cpu/sigma2/sigma2.h,CPUS["SIGMA2"] = true
--------------------------------------------------

if opt_tool(CPUS, "SIGMA2") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sigma2/sigma2d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/sigma2/sigma2d.h")
end

--------------------------------------------------
-- Control Data Corporation 1700 (disassembler only)
--@src/devices/cpu/cdc1700/cdc1700.h,CPUS["CDC1700"] = true
--------------------------------------------------

if opt_tool(CPUS, "CDC1700") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cdc1700/cdc1700d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cdc1700/cdc1700d.h")
end

--------------------------------------------------
-- National Semiconductor HPC
--@src/devices/cpu/hpc/hpc.h,CPUS["HPC"] = true
--------------------------------------------------

if CPUS["HPC"] then
	files {
		MAME_DIR .. "src/devices/cpu/hpc/hpc.cpp",
		MAME_DIR .. "src/devices/cpu/hpc/hpc.h",
	}
end

if opt_tool(CPUS, "HPC") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hpc/hpcdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/hpc/hpcdasm.h")
end

--------------------------------------------------
-- Yamaha SWP30
--@src/devices/sound/swp30.h,CPUS["SWP30"] = true
--------------------------------------------------

if CPUS["SWP30"] then
	files {
		MAME_DIR .. "src/devices/sound/swp30.cpp",
		MAME_DIR .. "src/devices/sound/swp30.h",
	}
end

if opt_tool(CPUS, "SWP30") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/sound/swp30d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/sound/swp30d.h")
end

--------------------------------------------------
-- Yamaha DSPV
--@src/devices/sound/dspv.h,CPUS["DSPV"] = true
--------------------------------------------------

if CPUS["DSPV"] then
	files {
		MAME_DIR .. "src/devices/sound/dspv.cpp",
		MAME_DIR .. "src/devices/sound/dspv.h",
	}
end

if opt_tool(CPUS, "DSPV") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/sound/dspvd.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/sound/dspvd.h")
end

--------------------------------------------------
--  National Semiconductor NS32000 series
--@src/devices/cpu/ns32000/ns32000.h,CPUS["NS32000"] = true
--------------------------------------------------

if CPUS["NS32000"] then
	files {
		MAME_DIR .. "src/devices/cpu/ns32000/ns32000.cpp",
		MAME_DIR .. "src/devices/cpu/ns32000/ns32000.h",
		MAME_DIR .. "src/devices/cpu/ns32000/common.h",
	}
end

if opt_tool(CPUS, "NS32000") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ns32000/ns32000d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ns32000/ns32000d.h")
end

--------------------------------------------------
-- Elan RISC II series
--@src/devices/cpu/rii/riscii.h,CPUS["RII"] = true
--------------------------------------------------

if CPUS["RII"] then
	files {
		MAME_DIR .. "src/devices/cpu/rii/riscii.cpp",
		MAME_DIR .. "src/devices/cpu/rii/riscii.h",
	}
end

if opt_tool(CPUS, "RII") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/rii/riidasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/rii/riidasm.h")
end

--------------------------------------------------
-- National Semiconductor BCP
--@src/devices/cpu/bcp/dp8344.h,CPUS["BCP"] = true
--------------------------------------------------

if CPUS["BCP"] then
	files {
		MAME_DIR .. "src/devices/cpu/bcp/dp8344.cpp",
		MAME_DIR .. "src/devices/cpu/bcp/dp8344.h",
	}
end

if opt_tool(CPUS, "BCP") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/bcp/bcpdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/bcp/bcpdasm.h")
end

--------------------------------------------------
-- Fujitsu F2MC-16 series
--@src/devices/cpu/f2mc16/f2mc16.h,CPUS["F2MC16"] = true
--------------------------------------------------

if CPUS["F2MC16"] then
	files {
		MAME_DIR .. "src/devices/cpu/f2mc16/f2mc16.cpp",
		MAME_DIR .. "src/devices/cpu/f2mc16/f2mc16.h",
		MAME_DIR .. "src/devices/cpu/f2mc16/mb9061x.cpp",
		MAME_DIR .. "src/devices/cpu/f2mc16/mb9061x.h",
	}
end

if opt_tool(CPUS, "F2MC16") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/f2mc16/f2mc16d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/f2mc16/f2mc16d.h")
end

--------------------------------------------------
-- National Semiconductor CR16B
--@src/devices/cpu/cr16b/cr16b.h,CPUS["CR16B"] = true
--------------------------------------------------

if CPUS["CR16B"] then
	files {
		MAME_DIR .. "src/devices/cpu/cr16b/cr16b.cpp",
		MAME_DIR .. "src/devices/cpu/cr16b/cr16b.h",
	}
end

if opt_tool(CPUS, "CR16B") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cr16b/cr16bdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cr16b/cr16bdasm.h")
end

--------------------------------------------------
-- National Semiconductor CR16C, disassembler only
--------------------------------------------------

if opt_tool(CPUS, "CR16C") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cr16c/cr16cdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/cr16c/cr16cdasm.h")
end

--------------------------------------------------
-- Gigatron
--@src/devices/cpu/gigatron/gigatron.h,CPUS["GTRON"] = true
--------------------------------------------------

if CPUS["GTRON"] then
	files {
		MAME_DIR .. "src/devices/cpu/gigatron/gigatron.cpp",
		MAME_DIR .. "src/devices/cpu/gigatron/gigatron.h",
	}
end

if opt_tool(CPUS, "GTRON") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/gigatron/gigatrondasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/gigatron/gigatrondasm.h")
end

--------------------------------------------------
-- Motorola DSP56000
--@src/devices/cpu/dsp56000/dsp56000.h,CPUS["DSP56000"] = true
--------------------------------------------------

if CPUS["DSP56000"] then
	files {
		MAME_DIR .. "src/devices/cpu/dsp56000/dsp56000.cpp",
		MAME_DIR .. "src/devices/cpu/dsp56000/dsp56000.h",
	}
end

if opt_tool(CPUS, "DSP56000") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56000/dsp56000d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/dsp56000/dsp56000d.h")
end

--------------------------------------------------
-- DEC VAX, disassembler only
--@src/devices/cpu/vax/vax.h,CPUS["VAX"] = true
--------------------------------------------------

if opt_tool(CPUS, "VAX") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/vax/vaxdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/vax/vaxdasm.h")
end

--------------------------------------------------
-- DEC VT50/VT52
--@src/devices/cpu/vt50/vt50.h,CPUS["VT50"] = true
--------------------------------------------------

if CPUS["VT50"] then
	files {
		MAME_DIR .. "src/devices/cpu/vt50/vt50.cpp",
		MAME_DIR .. "src/devices/cpu/vt50/vt50.h",
	}
end

if opt_tool(CPUS, "VT50") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/vt50/vt50dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/vt50/vt50dasm.h")
end

--------------------------------------------------
-- DEC VT61
--@src/devices/cpu/vt61/vt61.h,CPUS["VT61"] = true
--------------------------------------------------

if CPUS["VT61"] then
	files {
		MAME_DIR .. "src/devices/cpu/vt61/vt61.cpp",
		MAME_DIR .. "src/devices/cpu/vt61/vt61.h",
	}
end

if opt_tool(CPUS, "VT61") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/vt61/vt61dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/vt61/vt61dasm.h")
end

--------------------------------------------------
-- National Semiconductor PACE/INS8900
--@src/devices/cpu/pace/pace.h,CPUS["PACE"] = true
--------------------------------------------------

if CPUS["PACE"] then
	files {
		MAME_DIR .. "src/devices/cpu/pace/pace.cpp",
		MAME_DIR .. "src/devices/cpu/pace/pace.h",
	}
end

if opt_tool(CPUS, "PACE") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pace/pacedasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/pace/pacedasm.h")
end

--------------------------------------------------
-- AT&T WE32000/WE32100/WE32200
--@src/devices/cpu/we32000/we32100.h,CPUS["WE32000"] = true
--------------------------------------------------

if CPUS["WE32000"] then
	files {
		MAME_DIR .. "src/devices/cpu/we32000/we32100.cpp",
		MAME_DIR .. "src/devices/cpu/we32000/we32100.h",
	}
end

if opt_tool(CPUS, "WE32000") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/we32000/we32100d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/we32000/we32100d.h")
end

--------------------------------------------------
-- DEC RX01
--@src/devices/cpu/rx01/rx01.h,CPUS["RX01"] = true
--------------------------------------------------

if CPUS["RX01"] then
	files {
		MAME_DIR .. "src/devices/cpu/rx01/rx01.cpp",
		MAME_DIR .. "src/devices/cpu/rx01/rx01.h",
	}
end

if opt_tool(CPUS, "RX01") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/rx01/rx01dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/rx01/rx01dasm.h")
end

--------------------------------------------------
-- Motorola M88000
--@src/devices/cpu/m88000/m88000.h,CPUS["M88000"] = true
--------------------------------------------------

if CPUS["M88000"] then
	files {
		MAME_DIR .. "src/devices/cpu/m88000/m88000.cpp",
		MAME_DIR .. "src/devices/cpu/m88000/m88000.h",
	}
end

if opt_tool(CPUS, "M88000") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m88000/m88000d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m88000/m88000d.h")
end

--------------------------------------------------
-- XAVIX2
--@src/devices/cpu/xavix2/xavix2.h,CPUS["XAVIX2"] = true
--------------------------------------------------

if CPUS["XAVIX2"] then
	files {
		MAME_DIR .. "src/devices/cpu/xavix2/xavix2.cpp",
		MAME_DIR .. "src/devices/cpu/xavix2/xavix2.h",
	}
end

if opt_tool(CPUS, "XAVIX2") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/xavix2/xavix2d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/xavix2/xavix2d.h")
end

--------------------------------------------------
-- NEC 78K
--@src/devices/cpu/upd78k/upd78k0.h,CPUS["UPD78K"] = true
--@src/devices/cpu/upd78k/upd78k2.h,CPUS["UPD78K"] = true
--@src/devices/cpu/upd78k/upd78k3.h,CPUS["UPD78K"] = true
--@src/devices/cpu/upd78k/upd78k4.h,CPUS["UPD78K"] = true
--------------------------------------------------

if CPUS["UPD78K"] then
	files {
		MAME_DIR .. "src/devices/cpu/upd78k/upd78k0.cpp",
		MAME_DIR .. "src/devices/cpu/upd78k/upd78k0.h",
		MAME_DIR .. "src/devices/cpu/upd78k/upd78k2.cpp",
		MAME_DIR .. "src/devices/cpu/upd78k/upd78k2.h",
		MAME_DIR .. "src/devices/cpu/upd78k/upd78k3.cpp",
		MAME_DIR .. "src/devices/cpu/upd78k/upd78k3.h",
		MAME_DIR .. "src/devices/cpu/upd78k/upd78k4.cpp",
		MAME_DIR .. "src/devices/cpu/upd78k/upd78k4.h",
	}
end

if opt_tool(CPUS, "UPD78K") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78kd.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78kd.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78k0d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78k0d.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78k1d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78k1d.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78k2d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78k2d.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78k3d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78k3d.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78k4d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd78k/upd78k4d.h")
end

--------------------------------------------------
-- IBM ROMP
--@src/devices/cpu/romp/romp.h,CPUS["ROMP"] = true
--------------------------------------------------

if CPUS["ROMP"] then
	files {
		MAME_DIR .. "src/devices/cpu/romp/romp.cpp",
		MAME_DIR .. "src/devices/cpu/romp/romp.h",
		MAME_DIR .. "src/devices/cpu/romp/rsc.h",
	}
end

if opt_tool(CPUS, "ROMP") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/romp/rompdasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/romp/rompdasm.h")
end

--------------------------------------------------
-- KS0164
--@src/devices/cpu/ks0164/ks0164.h,CPUS["KS0164"] = true
--------------------------------------------------

if CPUS["KS0164"] then
	files {
		MAME_DIR .. "src/devices/cpu/ks0164/ks0164.cpp",
		MAME_DIR .. "src/devices/cpu/ks0164/ks0164.h",
	}
end

if opt_tool(CPUS, "KS0164") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ks0164/ks0164d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ks0164/ks0164d.h")
end

--------------------------------------------------
-- uPD177x - Disassembler only
--@src/devices/cpu/upd177x/upd177x.h,CPUS["UPD177X"] = true
--------------------------------------------------

if opt_tool(CPUS, "UPD177X") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd177x/upd177xd.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/upd177x/upd177xd.h")
end

--------------------------------------------------
-- Sanyo LC58 - Disassembler only
--@src/devices/cpu/lc58/lc58.h,CPUS["LC58"] = true
--------------------------------------------------

if opt_tool(CPUS, "LC58") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lc58/lc58d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lc58/lc58d.h")
end

--------------------------------------------------
-- OKI MSM6502/6512 - Disassembler only
--@src/devices/cpu/msm65x2/msm65x2.h,CPUS["MSM65X2"] = true
--------------------------------------------------

if opt_tool(CPUS, "MSM65X2") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/msm65x2/msm65x2d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/msm65x2/msm65x2d.h")
end

--------------------------------------------------
-- Sanyo LC57 - Disassembler only
--@src/devices/cpu/lc57/lc57.h,CPUS["LC57"] = true
--------------------------------------------------

if opt_tool(CPUS, "LC57") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lc57/lc57d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/lc57/lc57d.h")
end

--------------------------------------------------
-- Mark I (Andrew Holme)
--@src/devices/cpu/mk1/mk1.h,CPUS["MK1"] = true
--------------------------------------------------

if CPUS["MK1"] then
	files {
		MAME_DIR .. "src/devices/cpu/mk1/mk1.cpp",
		MAME_DIR .. "src/devices/cpu/mk1/mk1.h",
	}
end

if opt_tool(CPUS, "MK1") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mk1/mk1dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/mk1/mk1dasm.h")
end

--------------------------------------------------
-- Motorola M68HC16 (CPU16)
--@src/devices/cpu/m68hc16/cpu16.h,CPUS["M68HC16"] = true
--------------------------------------------------

if CPUS["M68HC16"] then
	files {
		MAME_DIR .. "src/devices/cpu/m68hc16/cpu16.cpp",
		MAME_DIR .. "src/devices/cpu/m68hc16/cpu16.h",
		MAME_DIR .. "src/devices/cpu/m68hc16/m68hc16z.cpp",
		MAME_DIR .. "src/devices/cpu/m68hc16/m68hc16z.h",
	}
end

if opt_tool(CPUS, "M68HC16") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m68hc16/cpu16dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/m68hc16/cpu16dasm.h")
end

--------------------------------------------------
-- Varian 620, disassembler only
--------------------------------------------------

if opt_tool(CPUS, "V620") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v620/v620dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/v620/v620dasm.h")
end

--------------------------------------------------
-- Altera Nios II
--@src/devices/cpu/nios2/nios2.h,CPUS["NIOS2"] = true
--------------------------------------------------

if CPUS["NIOS2"] then
	files {
		MAME_DIR .. "src/devices/cpu/nios2/nios2.cpp",
		MAME_DIR .. "src/devices/cpu/nios2/nios2.h",
	}
end

if opt_tool(CPUS, "NIOS2") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nios2/nios2dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nios2/nios2dasm.h")
end

--------------------------------------------------
-- IBM 1800, disassembler only
--@src/devices/cpu/ibm1800/ibm1800.h,CPUS["IBM1800"] = true
--------------------------------------------------

if opt_tool(CPUS, "IBM1800") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ibm1800/ibm1800d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ibm1800/ibm1800d.h")
end

--------------------------------------------------
-- Data General Nova, disassembler only
--@src/devices/cpu/nova/nova.h,CPUS["NOVA"] = true
--------------------------------------------------

if opt_tool(CPUS, "NOVA") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nova/novadasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/nova/novadasm.h")
end

--------------------------------------------------
-- Interdata Series 16, disassembler only
--@src/devices/cpu/interdata16/interdata16.h,CPUS["INTERDATA16"] = true
--------------------------------------------------

if opt_tool(CPUS, "INTERDATA16") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/interdata16/dasm16.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/interdata16/dasm16.h")
end

--------------------------------------------------
-- SGS-Thomson ST9
--@src/devices/cpu/st9/st905x.h,CPUS["ST9"] = true
--------------------------------------------------

if CPUS["ST9"] then
	files {
		MAME_DIR .. "src/devices/cpu/st9/st905x.cpp",
		MAME_DIR .. "src/devices/cpu/st9/st905x.h",
	}
end

if opt_tool(CPUS, "ST9") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/st9/st9dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/st9/st9dasm.h")
end

--------------------------------------------------
-- 3C/Honeywell DDP-516, disassembler only
--@src/devices/cpu/ddp516/ddp516.h,CPUS["DDP516"] = true
--------------------------------------------------

if opt_tool(CPUS, "DDP516") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ddp516/ddp516d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ddp516/ddp516d.h")
end

--------------------------------------------------
-- Whatever is in the Evolution
--@src/devices/cpu/evolution/evo.h,CPUS["EVOLUTION"] = true
--------------------------------------------------

if CPUS["EVOLUTION"] then
	files {
		MAME_DIR .. "src/devices/cpu/evolution/evo.cpp",
		MAME_DIR .. "src/devices/cpu/evolution/evo.h",
	}
end

if opt_tool(CPUS, "EVOLUTION") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/evolution/evod.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/evolution/evod.h")
end

--------------------------------------------------
-- Tensilica Xtensa
--@src/devices/cpu/xtensa/xtensa.h,CPUS["XTENSA"] = true
--------------------------------------------------

if CPUS["XTENSA"] then
	files {
		MAME_DIR .. "src/devices/cpu/xtensa/xtensa.cpp",
		MAME_DIR .. "src/devices/cpu/xtensa/xtensa.h",
		MAME_DIR .. "src/devices/cpu/xtensa/xtensa_helper.cpp",
		MAME_DIR .. "src/devices/cpu/xtensa/xtensa_helper.h",
	}
end

if opt_tool(CPUS, "XTENSA") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/xtensa/xtensad.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/xtensa/xtensad.h")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/xtensa/xtensa_helper.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/xtensa/xtensa_helper.h")
end

--------------------------------------------------
-- Holtek HT1130
--@src/devices/cpu/ht1130/ht1130.h,CPUS["HT1130"] = true
--------------------------------------------------

if CPUS["HT1130"] then
	files {
		MAME_DIR .. "src/devices/cpu/ht1130/ht1130.cpp",
		MAME_DIR .. "src/devices/cpu/ht1130/ht1130.h",
	}
end

if opt_tool(CPUS, "HT1130") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ht1130/ht1130d.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/ht1130/ht1130d.h")
end

--------------------------------------------------
-- Epson C33 STD, C33 ADV, etc.
--@src/devices/cpu/c33/c33common.h,CPUS["C33"] = true
--------------------------------------------------

if CPUS["C33"] then
	files {
		MAME_DIR .. "src/devices/cpu/c33/c33common.h",
		MAME_DIR .. "src/devices/cpu/c33/c33helpers.ipp",
		MAME_DIR .. "src/devices/cpu/c33/c33std.cpp",
		MAME_DIR .. "src/devices/cpu/c33/c33std.h",
		MAME_DIR .. "src/devices/cpu/c33/s1c33209.cpp",
		MAME_DIR .. "src/devices/cpu/c33/s1c33209.h",
	}
end

if opt_tool(CPUS, "C33") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/c33/c33dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/c33/c33dasm.h")
end

--------------------------------------------------
-- IBM PALM
--@src/devices/cpu/palm/palm.h,CPUS["PALM"] = true
--------------------------------------------------

if CPUS["PALM"] then
	files {
		MAME_DIR .. "src/devices/cpu/palm/palm.cpp",
		MAME_DIR .. "src/devices/cpu/palm/palm.h",
	}
end

if opt_tool(CPUS, "PALM") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/palm/palmd.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/palm/palmd.h")
end

--------------------------------------------------
-- Oki OLMS-66K/nX-8 series
--@src/devices/cpu/olms66k/msm665xx.h,CPUS["OLMS66K"] = true
--------------------------------------------------

if CPUS["OLMS66K"] then
	files {
		MAME_DIR .. "src/devices/cpu/olms66k/msm665xx.cpp",
		MAME_DIR .. "src/devices/cpu/olms66k/msm665xx.h",
	}
end

if opt_tool(CPUS, "OLMS66K") then
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/olms66k/nx8dasm.cpp")
	table.insert(disasm_files , MAME_DIR .. "src/devices/cpu/olms66k/nx8dasm.h")
end
