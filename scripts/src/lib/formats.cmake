add_library(formats ${LIBTYPE})

addprojectflags(formats)

target_include_directories(formats PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/emu
	${MAME_DIR}/src/lib
	${MAME_DIR}/src/lib/util
	${GEN_DIR}
	${EXT_INCLUDEDIR_ZLIB}
)

target_link_libraries(formats PUBLIC
	utils
)

target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/all.cpp
	${MAME_DIR}/src/lib/formats/all.h

	${MAME_DIR}/src/lib/formats/imageutl.cpp
	${MAME_DIR}/src/lib/formats/imageutl.h

	${MAME_DIR}/src/lib/formats/cassimg.cpp
	${MAME_DIR}/src/lib/formats/cassimg.h
	${MAME_DIR}/src/lib/formats/wavfile.cpp
	${MAME_DIR}/src/lib/formats/wavfile.h

	${MAME_DIR}/src/lib/formats/flopimg.cpp
	${MAME_DIR}/src/lib/formats/flopimg.h

	${MAME_DIR}/src/lib/formats/cqm_dsk.cpp
	${MAME_DIR}/src/lib/formats/cqm_dsk.h
	${MAME_DIR}/src/lib/formats/dsk_dsk.cpp
	${MAME_DIR}/src/lib/formats/dsk_dsk.h
	${MAME_DIR}/src/lib/formats/ipf_dsk.cpp
	${MAME_DIR}/src/lib/formats/ipf_dsk.h
	${MAME_DIR}/src/lib/formats/td0_dsk.cpp
	${MAME_DIR}/src/lib/formats/td0_dsk.h
	${MAME_DIR}/src/lib/formats/hxchfe_dsk.cpp
	${MAME_DIR}/src/lib/formats/hxchfe_dsk.h
	${MAME_DIR}/src/lib/formats/hxcmfm_dsk.cpp
	${MAME_DIR}/src/lib/formats/hxcmfm_dsk.h
	${MAME_DIR}/src/lib/formats/mfi_dsk.cpp
	${MAME_DIR}/src/lib/formats/mfi_dsk.h
	${MAME_DIR}/src/lib/formats/imd_dsk.cpp
	${MAME_DIR}/src/lib/formats/imd_dsk.h
	${MAME_DIR}/src/lib/formats/upd765_dsk.cpp
	${MAME_DIR}/src/lib/formats/upd765_dsk.h
	${MAME_DIR}/src/lib/formats/pc_dsk.cpp
	${MAME_DIR}/src/lib/formats/pc_dsk.h
	${MAME_DIR}/src/lib/formats/d88_dsk.cpp
	${MAME_DIR}/src/lib/formats/d88_dsk.h
	${MAME_DIR}/src/lib/formats/dfi_dsk.cpp
	${MAME_DIR}/src/lib/formats/dfi_dsk.h
	${MAME_DIR}/src/lib/formats/fdi_dsk.cpp

	${MAME_DIR}/src/lib/formats/fsmgr.h
	${MAME_DIR}/src/lib/formats/fsmgr.cpp
	${MAME_DIR}/src/lib/formats/fsblk_vec.h
	${MAME_DIR}/src/lib/formats/fsblk_vec.cpp
	${MAME_DIR}/src/lib/formats/fs_unformatted.h
	${MAME_DIR}/src/lib/formats/fs_unformatted.cpp
)

##################################################
##
##@src/lib/formats/2d_dsk.h,list(APPEND FORMATS 2D_DSK)
##################################################

if (("2D_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/2d_dsk.cpp
	${MAME_DIR}/src/lib/formats/2d_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/a26_cas.h,list(APPEND FORMATS A26_CAS)
##################################################

if (("A26_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/a26_cas.cpp
	${MAME_DIR}/src/lib/formats/a26_cas.h
)
endif()

##################################################
##
##@src/lib/formats/a5105_dsk.h,list(APPEND FORMATS A5105_DSK)
##################################################

if (("A5105_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/a5105_dsk.cpp
	${MAME_DIR}/src/lib/formats/a5105_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/abc800_dsk.h,list(APPEND FORMATS ABC800_DSK)
##################################################

if (("ABC800_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/abc800_dsk.cpp
	${MAME_DIR}/src/lib/formats/abc800_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/abcfd2_dsk.h,list(APPEND FORMATS ABCFD2_DSK)
##################################################

if (("ABCFD2_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/abcfd2_dsk.cpp
	${MAME_DIR}/src/lib/formats/abcfd2_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/ace_tap.h,list(APPEND FORMATS ACE_TAP)
##################################################

if (("ACE_TAP" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ace_tap.cpp
	${MAME_DIR}/src/lib/formats/ace_tap.h
)
endif()

##################################################
##
##@src/lib/formats/acorn_dsk.h,list(APPEND FORMATS ACORN_DSK)
##################################################

if (("ACORN_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/acorn_dsk.cpp
	${MAME_DIR}/src/lib/formats/acorn_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/adam_cas.h,list(APPEND FORMATS ADAM_CAS)
##################################################

if (("ADAM_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/adam_cas.cpp
	${MAME_DIR}/src/lib/formats/adam_cas.h
)
endif()

##################################################
##
##@src/lib/formats/adam_dsk.h,list(APPEND FORMATS ADAM_DSK)
##################################################

if (("ADAM_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/adam_dsk.cpp
	${MAME_DIR}/src/lib/formats/adam_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/afs_dsk.h,list(APPEND FORMATS AFS_DSK)
##################################################

if (("AFS_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/afs_dsk.cpp
	${MAME_DIR}/src/lib/formats/afs_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/agat840k_hle_dsk.h,list(APPEND FORMATS AGAT840K_HLE_DSK)
##################################################

if (("AGAT840K_HLE_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/agat840k_hle_dsk.cpp
	${MAME_DIR}/src/lib/formats/agat840k_hle_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/aim_dsk.h,list(APPEND FORMATS AIM_DSK)
##################################################

if (("AIM_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/aim_dsk.cpp
	${MAME_DIR}/src/lib/formats/aim_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/ami_dsk.h,list(APPEND FORMATS AMI_DSK)
##################################################

if (("AMI_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ami_dsk.cpp
	${MAME_DIR}/src/lib/formats/ami_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/ap2_dsk.h,list(APPEND FORMATS AP2_DSK)
##################################################

if (("AP2_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ap2_dsk.cpp
	${MAME_DIR}/src/lib/formats/ap2_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/apd_dsk.h,list(APPEND FORMATS APD_DSK)
##################################################

if (("APD_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/apd_dsk.cpp
	${MAME_DIR}/src/lib/formats/apd_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/apf_apt.h,list(APPEND FORMATS APF_APT)
##################################################

if (("APF_APT" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/apf_apt.cpp
	${MAME_DIR}/src/lib/formats/apf_apt.h
)
endif()

##################################################
##
##@src/lib/formats/apollo_dsk.h,list(APPEND FORMATS APOLLO_DSK)
##################################################

if (("APOLLO_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/apollo_dsk.cpp
	${MAME_DIR}/src/lib/formats/apollo_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/applix_dsk.h,list(APPEND FORMATS APPLIX_DSK)
##################################################

if (("APPLIX_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/applix_dsk.cpp
	${MAME_DIR}/src/lib/formats/applix_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/apridisk.h,list(APPEND FORMATS APRIDISK)
##################################################

if (("APRIDISK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/apridisk.cpp
	${MAME_DIR}/src/lib/formats/apridisk.h
)
endif()

##################################################
##
##@src/lib/formats/ap_dsk35.h,list(APPEND FORMATS AP_DSK35)
##################################################

if (("AP_DSK35" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ap_dsk35.cpp
	${MAME_DIR}/src/lib/formats/ap_dsk35.h
)
endif()

##################################################
##
##@src/lib/formats/aquarius_caq.h,list(APPEND FORMATS AQUARIUS_CAQ)
##################################################

if (("AQUARIUS_CAQ" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/aquarius_caq.cpp
	${MAME_DIR}/src/lib/formats/aquarius_caq.h
)
endif()

##################################################
##
##@src/lib/formats/asst128_dsk.h,list(APPEND FORMATS ASST128_DSK)
##################################################

if (("ASST128_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/asst128_dsk.cpp
	${MAME_DIR}/src/lib/formats/asst128_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/atari_dsk.h,list(APPEND FORMATS ATARI_DSK)
##################################################

if (("ATARI_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/atari_dsk.cpp
	${MAME_DIR}/src/lib/formats/atari_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/atom_dsk.h,list(APPEND FORMATS ATOM_DSK)
##################################################

if (("ATOM_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/atom_dsk.cpp
	${MAME_DIR}/src/lib/formats/atom_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/atom_tap.h,list(APPEND FORMATS ATOM_TAP)
##################################################

if (("ATOM_TAP" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/atom_tap.cpp
	${MAME_DIR}/src/lib/formats/atom_tap.h
)
endif()

##################################################
##
##@src/lib/formats/basicdsk.h,list(APPEND FORMATS BASICDSK)
##################################################

if (("BASICDSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/basicdsk.cpp
	${MAME_DIR}/src/lib/formats/basicdsk.h
)
endif()

##################################################
##
##@src/lib/formats/bw12_dsk.h,list(APPEND FORMATS BW12_DSK)
##################################################

if (("BW12_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/bw12_dsk.cpp
	${MAME_DIR}/src/lib/formats/bw12_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/bw2_dsk.h,list(APPEND FORMATS BW2_DSK)
##################################################

if (("BW2_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/bw2_dsk.cpp
	${MAME_DIR}/src/lib/formats/bw2_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/c3040_dsk.h,list(APPEND FORMATS C3040_DSK)
##################################################

if (("C3040_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/c3040_dsk.cpp
	${MAME_DIR}/src/lib/formats/c3040_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/c4040_dsk.h,list(APPEND FORMATS C4040_DSK)
##################################################

if (("C4040_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/c4040_dsk.cpp
	${MAME_DIR}/src/lib/formats/c4040_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/c8280_dsk.h,list(APPEND FORMATS C8280_DSK)
##################################################

if (("C8280_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/c8280_dsk.cpp
	${MAME_DIR}/src/lib/formats/c8280_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/camplynx_cas.h,list(APPEND FORMATS CAMPLYNX_CAS)
##################################################

if (("CAMPLYNX_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/camplynx_cas.cpp
	${MAME_DIR}/src/lib/formats/camplynx_cas.h
)
endif()

##################################################
##
##@src/lib/formats/camplynx_dsk.h,list(APPEND FORMATS CAMPLYNX_DSK)
##################################################

if (("CAMPLYNX_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/camplynx_dsk.cpp
	${MAME_DIR}/src/lib/formats/camplynx_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/cbm_crt.h,list(APPEND FORMATS CBM_CRT)
##################################################

if (("CBM_CRT" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/cbm_crt.cpp
	${MAME_DIR}/src/lib/formats/cbm_crt.h
)
endif()

##################################################
##
##@src/lib/formats/cbm_tap.h,list(APPEND FORMATS CBM_TAP)
##################################################

if (("CBM_TAP" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/cbm_tap.cpp
	${MAME_DIR}/src/lib/formats/cbm_tap.h
)
endif()

##################################################
##
##@src/lib/formats/ccvf_dsk.h,list(APPEND FORMATS CCVF_DSK)
##################################################

if (("CCVF_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ccvf_dsk.cpp
	${MAME_DIR}/src/lib/formats/ccvf_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/cd90_640_dsk.h,list(APPEND FORMATS CD90_640_DSK)
##################################################

if (("CD90_640_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/cd90_640_dsk.cpp
	${MAME_DIR}/src/lib/formats/cd90_640_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/cgenie_dsk.h,list(APPEND FORMATS CGENIE_DSK)
##################################################

if (("CGENIE_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/cgenie_dsk.cpp
	${MAME_DIR}/src/lib/formats/cgenie_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/cgen_cas.h,list(APPEND FORMATS CGEN_CAS)
##################################################

if (("CGEN_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/cgen_cas.cpp
	${MAME_DIR}/src/lib/formats/cgen_cas.h
)
endif()

##################################################
##
##@src/lib/formats/coco_cas.h,list(APPEND FORMATS COCO_CAS)
##################################################

if (("COCO_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/coco_cas.cpp
	${MAME_DIR}/src/lib/formats/coco_cas.h
)
endif()

##################################################
##
##@src/lib/formats/comx35_dsk.h,list(APPEND FORMATS COMX35_DSK)
##################################################

if (("COMX35_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/comx35_dsk.cpp
	${MAME_DIR}/src/lib/formats/comx35_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/concept_dsk.h,list(APPEND FORMATS CONCEPT_DSK)
##################################################

if (("CONCEPT_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/concept_dsk.cpp
	${MAME_DIR}/src/lib/formats/concept_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/coupedsk.h,list(APPEND FORMATS COUPEDSK)
##################################################

if (("COUPEDSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/coupedsk.cpp
	${MAME_DIR}/src/lib/formats/coupedsk.h
)
endif()

##################################################
##
##@src/lib/formats/cpis_dsk.h,list(APPEND FORMATS CPIS_DSK)
##################################################

if (("CPIS_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/cpis_dsk.cpp
	${MAME_DIR}/src/lib/formats/cpis_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/csw_cas.h,list(APPEND FORMATS CSW_CAS)
##################################################

if (("CSW_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/csw_cas.cpp
	${MAME_DIR}/src/lib/formats/csw_cas.h
)
endif()

##################################################
##
##@src/lib/formats/d64_dsk.h,list(APPEND FORMATS D64_DSK)
##################################################

if (("D64_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/d64_dsk.cpp
	${MAME_DIR}/src/lib/formats/d64_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/d71_dsk.h,list(APPEND FORMATS D71_DSK)
##################################################

if (("D71_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/d71_dsk.cpp
	${MAME_DIR}/src/lib/formats/d71_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/d80_dsk.h,list(APPEND FORMATS D80_DSK)
##################################################

if (("D80_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/d80_dsk.cpp
	${MAME_DIR}/src/lib/formats/d80_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/d81_dsk.h,list(APPEND FORMATS D81_DSK)
##################################################

if (("D81_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/d81_dsk.cpp
	${MAME_DIR}/src/lib/formats/d81_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/d82_dsk.h,list(APPEND FORMATS D82_DSK)
##################################################

if (("D82_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/d82_dsk.cpp
	${MAME_DIR}/src/lib/formats/d82_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/dcp_dsk.h,list(APPEND FORMATS DCP_DSK)
##################################################

if (("DCP_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/dcp_dsk.cpp
	${MAME_DIR}/src/lib/formats/dcp_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/dim_dsk.h,list(APPEND FORMATS DIM_DSK)
##################################################

if (("DIM_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/dim_dsk.cpp
	${MAME_DIR}/src/lib/formats/dim_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/dip_dsk.h,list(APPEND FORMATS DIP_DSK)
##################################################

if (("DIP_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/dip_dsk.cpp
	${MAME_DIR}/src/lib/formats/dip_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/dmk_dsk.h,list(APPEND FORMATS DMK_DSK)
##################################################

if (("DMK_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/dmk_dsk.cpp
	${MAME_DIR}/src/lib/formats/dmk_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/ds9_dsk.h,list(APPEND FORMATS DS9_DSK)
##################################################

if (("DS9_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ds9_dsk.cpp
	${MAME_DIR}/src/lib/formats/ds9_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/sdf_dsk.h,list(APPEND FORMATS SDF_DSK)
##################################################

if (("SDF_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/sdf_dsk.cpp
	${MAME_DIR}/src/lib/formats/sdf_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/ep64_dsk.h,list(APPEND FORMATS EP64_DSK)
##################################################

if (("EP64_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ep64_dsk.cpp
	${MAME_DIR}/src/lib/formats/ep64_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/dmv_dsk.h,list(APPEND FORMATS DMV_DSK)
##################################################

if (("DMV_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/dmv_dsk.cpp
	${MAME_DIR}/src/lib/formats/dmv_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/dvk_mx_dsk.h,list(APPEND FORMATS DVK_MX_DSK)
##################################################

if (("DVK_MX_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/dvk_mx_dsk.cpp
	${MAME_DIR}/src/lib/formats/dvk_mx_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/esq16_dsk.h,list(APPEND FORMATS ESQ16_DSK)
##################################################

if (("ESQ16_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/esq16_dsk.cpp
	${MAME_DIR}/src/lib/formats/esq16_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/esq8_dsk.h,list(APPEND FORMATS ESQ8_DSK)
##################################################

if (("ESQ8_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/esq8_dsk.cpp
	${MAME_DIR}/src/lib/formats/esq8_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/excali64_dsk.h,list(APPEND FORMATS EXCALI64_DSK)
##################################################

if (("EXCALI64_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/excali64_dsk.cpp
	${MAME_DIR}/src/lib/formats/excali64_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/fc100_cas.h,list(APPEND FORMATS FC100_CAS)
##################################################

if (("FC100_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/fc100_cas.cpp
	${MAME_DIR}/src/lib/formats/fc100_cas.h
)
endif()

##################################################
##
##@src/lib/formats/fdd_dsk.h,list(APPEND FORMATS FDD_DSK)
##################################################

if (("FDD_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/fdd_dsk.cpp
	${MAME_DIR}/src/lib/formats/fdd_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/fl1_dsk.h,list(APPEND FORMATS FL1_DSK)
##################################################

if (("FL1_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/fl1_dsk.cpp
	${MAME_DIR}/src/lib/formats/fl1_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/flex_dsk.h,list(APPEND FORMATS FLEX_DSK)
##################################################

if (("FLEX_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/flex_dsk.cpp
	${MAME_DIR}/src/lib/formats/flex_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/uniflex_dsk.h,list(APPEND FORMATS UNIFLEX_DSK)
##################################################

if (("UNIFLEX_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/uniflex_dsk.cpp
	${MAME_DIR}/src/lib/formats/uniflex_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/fm7_cas.h,list(APPEND FORMATS FM7_CAS)
##################################################

if (("FM7_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/fm7_cas.cpp
	${MAME_DIR}/src/lib/formats/fm7_cas.h
)
endif()

##################################################
##
##@src/lib/formats/fmsx_cas.h,list(APPEND FORMATS FMSX_CAS)
##################################################

if (("FMSX_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/fmsx_cas.cpp
	${MAME_DIR}/src/lib/formats/fmsx_cas.h
)
endif()

##################################################
##
##@src/lib/formats/fmtowns_dsk.h,list(APPEND FORMATS FMTOWNS_DSK)
##################################################

if (("FMTOWNS_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/fmtowns_dsk.cpp
	${MAME_DIR}/src/lib/formats/fmtowns_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/fsd_dsk.h,list(APPEND FORMATS FSD_DSK)
##################################################

if (("FSD_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/fsd_dsk.cpp
	${MAME_DIR}/src/lib/formats/fsd_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/g64_dsk.h,list(APPEND FORMATS G64_DSK)
##################################################

if (("G64_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/g64_dsk.cpp
	${MAME_DIR}/src/lib/formats/g64_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/gtp_cas.h,list(APPEND FORMATS GTP_CAS)
##################################################

if (("GTP_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/gtp_cas.cpp
	${MAME_DIR}/src/lib/formats/gtp_cas.h
)
endif()

##################################################
##
##@src/lib/formats/guab_dsk.h,list(APPEND FORMATS GUAB_DSK)
##################################################

if (("GUAB_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/guab_dsk.cpp
	${MAME_DIR}/src/lib/formats/guab_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/h8_cas.h,list(APPEND FORMATS H8_CAS)
##################################################

if (("H8_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/h8_cas.cpp
	${MAME_DIR}/src/lib/formats/h8_cas.h
)
endif()

##################################################
##
##@src/lib/formats/hector_minidisc.h,list(APPEND FORMATS HECTOR_MINIDISC)
##################################################

if (("HECTOR_MINIDISC" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/hector_minidisc.cpp
	${MAME_DIR}/src/lib/formats/hector_minidisc.h
)
endif()

##################################################
##
##@src/lib/formats/hect_dsk.h,list(APPEND FORMATS HECT_DSK)
##################################################

if (("HECT_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/hect_dsk.cpp
	${MAME_DIR}/src/lib/formats/hect_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/hect_tap.h,list(APPEND FORMATS HECT_TAP)
##################################################

if (("HECT_TAP" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/hect_tap.cpp
	${MAME_DIR}/src/lib/formats/hect_tap.h
)
endif()

##################################################
##
##@src/lib/formats/hti_tape.h,list(APPEND FORMATS HTI_TAP)
##################################################

if (("HTI_TAP" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/hti_tape.cpp
	${MAME_DIR}/src/lib/formats/hti_tape.h
)
endif()

##################################################
##
##@src/lib/formats/hpi_dsk.h,list(APPEND FORMATS HPI_DSK)
##################################################

if (("HPI_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/hpi_dsk.cpp
	${MAME_DIR}/src/lib/formats/hpi_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/hp_ipc_dsk.h,list(APPEND FORMATS HP_IPC_DSK)
##################################################

if (("HP_IPC_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/hp_ipc_dsk.cpp
	${MAME_DIR}/src/lib/formats/hp_ipc_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/img_dsk.h,list(APPEND FORMATS IMG_DSK)
##################################################

if (("IMG_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/img_dsk.cpp
	${MAME_DIR}/src/lib/formats/img_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/iq151_dsk.h,list(APPEND FORMATS IQ151_DSK)
##################################################

if (("IQ151_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/iq151_dsk.cpp
	${MAME_DIR}/src/lib/formats/iq151_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/itt3030_dsk.h,list(APPEND FORMATS ITT3030_DSK)
##################################################

if (("ITT3030_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/itt3030_dsk.cpp
	${MAME_DIR}/src/lib/formats/itt3030_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/juku_dsk.h,list(APPEND FORMATS JUKU_DSK)
##################################################

if (("JUKU_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/juku_dsk.cpp
	${MAME_DIR}/src/lib/formats/juku_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/jvc_dsk.h,list(APPEND FORMATS JVC_DSK)
##################################################

if (("JVC_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/jvc_dsk.cpp
	${MAME_DIR}/src/lib/formats/jvc_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/os9_dsk.h,list(APPEND FORMATS OS9_DSK)
##################################################

if (("OS9_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/os9_dsk.cpp
	${MAME_DIR}/src/lib/formats/os9_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/jfd_dsk.h,list(APPEND FORMATS JFD_DSK)
##################################################

if (("JFD_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/jfd_dsk.cpp
	${MAME_DIR}/src/lib/formats/jfd_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/kaypro_dsk.h,list(APPEND FORMATS KAYPRO_DSK)
##################################################

if (("KAYPRO_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/kaypro_dsk.cpp
	${MAME_DIR}/src/lib/formats/kaypro_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/kc85_dsk.h,list(APPEND FORMATS KC85_DSK)
##################################################

if (("KC85_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/kc85_dsk.cpp
	${MAME_DIR}/src/lib/formats/kc85_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/kc_cas.h,list(APPEND FORMATS KC_CAS)
##################################################

if (("KC_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/kc_cas.cpp
	${MAME_DIR}/src/lib/formats/kc_cas.h
)
endif()

##################################################
##
##@src/lib/formats/kim1_cas.h,list(APPEND FORMATS KIM1_CAS)
##################################################

if (("KIM1_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/kim1_cas.cpp
	${MAME_DIR}/src/lib/formats/kim1_cas.h
)
endif()

##################################################
##
##@src/lib/formats/lviv_lvt.h,list(APPEND FORMATS LVIV_LVT)
##################################################

if (("LVIV_LVT" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/lviv_lvt.cpp
	${MAME_DIR}/src/lib/formats/lviv_lvt.h
)
endif()

##################################################
##
##@src/lib/formats/m20_dsk.h,list(APPEND FORMATS M20_DSK)
##################################################

if (("M20_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/m20_dsk.cpp
	${MAME_DIR}/src/lib/formats/m20_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/m5_dsk.h,list(APPEND FORMATS M5_DSK)
##################################################

if (("M5_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/m5_dsk.cpp
	${MAME_DIR}/src/lib/formats/m5_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/mbee_cas.h,list(APPEND FORMATS MBEE_CAS)
##################################################

if (("MBEE_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/mbee_cas.cpp
	${MAME_DIR}/src/lib/formats/mbee_cas.h
)
endif()

##################################################
##
##@src/lib/formats/mdos_dsk.h,list(APPEND FORMATS MDOS_DSK)
##################################################

if (("MDOS_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/mdos_dsk.cpp
	${MAME_DIR}/src/lib/formats/mdos_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/mfm_hd.h,list(APPEND FORMATS MFM_HD)
##################################################

if (("MFM_HD" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/mfm_hd.cpp
	${MAME_DIR}/src/lib/formats/mfm_hd.h
)
endif()

##################################################
##
##@src/lib/formats/mm_dsk.h,list(APPEND FORMATS MM_DSK)
##################################################

if (("MM_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/mm_dsk.cpp
	${MAME_DIR}/src/lib/formats/mm_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/ms0515_dsk.h,list(APPEND FORMATS MS0515_DSK)
##################################################

if (("MS0515_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ms0515_dsk.cpp
	${MAME_DIR}/src/lib/formats/ms0515_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/msx_dsk.h,list(APPEND FORMATS MSX_DSK)
##################################################

if (("MSX_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/msx_dsk.cpp
	${MAME_DIR}/src/lib/formats/msx_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/mtx_dsk.h,list(APPEND FORMATS MTX_DSK)
##################################################

if (("MTX_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/mtx_dsk.cpp
	${MAME_DIR}/src/lib/formats/mtx_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/mz_cas.h,list(APPEND FORMATS MZ_CAS)
##################################################

if (("MZ_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/mz_cas.cpp
	${MAME_DIR}/src/lib/formats/mz_cas.h
)
endif()

##################################################
##
##@src/lib/formats/nanos_dsk.h,list(APPEND FORMATS NANOS_DSK)
##################################################

if (("NANOS_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/nanos_dsk.cpp
	${MAME_DIR}/src/lib/formats/nanos_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/nascom_dsk.h,list(APPEND FORMATS NASCOM_DSK)
##################################################

if (("NASCOM_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/nascom_dsk.cpp
	${MAME_DIR}/src/lib/formats/nascom_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/naslite_dsk.h,list(APPEND FORMATS NASLITE_DSK)
##################################################

if (("NASLITE_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/naslite_dsk.cpp
	${MAME_DIR}/src/lib/formats/naslite_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/nes_dsk.h,list(APPEND FORMATS NES_DSK)
##################################################

if (("NES_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/nes_dsk.cpp
	${MAME_DIR}/src/lib/formats/nes_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/nfd_dsk.h,list(APPEND FORMATS NFD_DSK)
##################################################

if (("NFD_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/nfd_dsk.cpp
	${MAME_DIR}/src/lib/formats/nfd_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/opd_dsk.h,list(APPEND FORMATS OPD_DSK)
##################################################

if (("OPD_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/opd_dsk.cpp
	${MAME_DIR}/src/lib/formats/opd_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/orao_cas.h,list(APPEND FORMATS ORAO_CAS)
##################################################

if (("ORAO_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/orao_cas.cpp
	${MAME_DIR}/src/lib/formats/orao_cas.h
)
endif()

##################################################
##
##@src/lib/formats/oric_dsk.h,list(APPEND FORMATS ORIC_DSK)
##################################################

if (("ORIC_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/oric_dsk.cpp
	${MAME_DIR}/src/lib/formats/oric_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/oric_tap.h,list(APPEND FORMATS ORIC_TAP)
##################################################

if (("ORIC_TAP" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/oric_tap.cpp
	${MAME_DIR}/src/lib/formats/oric_tap.h
)
endif()

##################################################
##
##@src/lib/formats/ibmxdf_dsk.h,list(APPEND FORMATS IBMXDF_DSK)
##################################################

if (("IBMXDF_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ibmxdf_dsk.cpp
	${MAME_DIR}/src/lib/formats/ibmxdf_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/p2000t_cas.h,list(APPEND FORMATS P2000T_CAS)
##################################################

if (("P2000T_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/p2000t_cas.cpp
	${MAME_DIR}/src/lib/formats/p2000t_cas.h
)
endif()


##################################################
##
##@src/lib/formats/p6001_cas.h,list(APPEND FORMATS P6001_CAS)
##################################################

if (("P6001_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/p6001_cas.cpp
	${MAME_DIR}/src/lib/formats/p6001_cas.h
)
endif()

##################################################
##
##@src/lib/formats/pasti_dsk.h,list(APPEND FORMATS PASTI_DSK)
##################################################

if (("PASTI_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/pasti_dsk.cpp
	${MAME_DIR}/src/lib/formats/pasti_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/pc98fdi_dsk.h,list(APPEND FORMATS PC98FDI_DSK)
##################################################

if (("PC98FDI_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/pc98fdi_dsk.cpp
	${MAME_DIR}/src/lib/formats/pc98fdi_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/pc98_dsk.h,list(APPEND FORMATS PC98_DSK)
##################################################

if (("PC98_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/pc98_dsk.cpp
	${MAME_DIR}/src/lib/formats/pc98_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/phc25_cas.h,list(APPEND FORMATS PHC25_CAS)
##################################################

if (("PHC25_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/phc25_cas.cpp
	${MAME_DIR}/src/lib/formats/phc25_cas.h
)
endif()

##################################################
##
##@src/lib/formats/pk8020_dsk.h,list(APPEND FORMATS PK8020_DSK)
##################################################

if (("PK8020_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/pk8020_dsk.cpp
	${MAME_DIR}/src/lib/formats/pk8020_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/pmd_cas.h,list(APPEND FORMATS PMD_CAS)
##################################################

if (("PMD_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/pmd_cas.cpp
	${MAME_DIR}/src/lib/formats/pmd_cas.h
)
endif()

##################################################
##
##@src/lib/formats/poly_dsk.h,list(APPEND FORMATS POLY_DSK)
##################################################

if (("POLY_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/poly_dsk.cpp
	${MAME_DIR}/src/lib/formats/poly_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/ppg_dsk.h,list(APPEND FORMATS PPG_DSK)
##################################################

if (("PPG_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ppg_dsk.cpp
	${MAME_DIR}/src/lib/formats/ppg_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/primoptp.h,list(APPEND FORMATS PRIMOPTP)
##################################################

if (("PRIMOPTP" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/primoptp.cpp
	${MAME_DIR}/src/lib/formats/primoptp.h
)
endif()

##################################################
##
##@src/lib/formats/pyldin_dsk.h,list(APPEND FORMATS PYLDIN_DSK)
##################################################

if (("PYLDIN_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/pyldin_dsk.cpp
	${MAME_DIR}/src/lib/formats/pyldin_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/ql_dsk.h,list(APPEND FORMATS QL_DSK)
##################################################

if (("QL_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ql_dsk.cpp
	${MAME_DIR}/src/lib/formats/ql_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/rc759_dsk.h,list(APPEND FORMATS RC759_DSK)
##################################################

if (("RC759_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/rc759_dsk.cpp
	${MAME_DIR}/src/lib/formats/rc759_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/rk_cas.h,list(APPEND FORMATS RK_CAS)
##################################################

if (("RK_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/rk_cas.cpp
	${MAME_DIR}/src/lib/formats/rk_cas.h
)
endif()

##################################################
##
##@src/lib/formats/rx50_dsk.h,list(APPEND FORMATS RX50_DSK)
##################################################

if (("RX50_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/rx50_dsk.cpp
	${MAME_DIR}/src/lib/formats/rx50_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/sc3000_bit.h,list(APPEND FORMATS SC3000_BIT)
##################################################

if (("SC3000_BIT" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/sc3000_bit.cpp
	${MAME_DIR}/src/lib/formats/sc3000_bit.h
)
endif()

##################################################
##
##@src/lib/formats/sdd_dsk.h,list(APPEND FORMATS SDD_DSK)
##################################################

if (("SDD_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/sdd_dsk.cpp
	${MAME_DIR}/src/lib/formats/sdd_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/sf7000_dsk.h,list(APPEND FORMATS SF7000_DSK)
##################################################

if (("SF7000_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/sf7000_dsk.cpp
	${MAME_DIR}/src/lib/formats/sf7000_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/smx_dsk.h,list(APPEND FORMATS SMX_DSK)
##################################################

if (("SMX_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/smx_dsk.cpp
	${MAME_DIR}/src/lib/formats/smx_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/sol_cas.h,list(APPEND FORMATS SOL_CAS)
##################################################

if (("SOL_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/sol_cas.cpp
	${MAME_DIR}/src/lib/formats/sol_cas.h
)
endif()

##################################################
##
##@src/lib/formats/sorc_cas.h,list(APPEND FORMATS SORC_CAS)
##################################################

if (("SORC_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/sorc_cas.cpp
	${MAME_DIR}/src/lib/formats/sorc_cas.h
)
endif()

##################################################
##
##@src/lib/formats/sorc_dsk.h,list(APPEND FORMATS SORC_DSK)
##################################################

if (("SORC_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/sorc_dsk.cpp
	${MAME_DIR}/src/lib/formats/sorc_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/sord_cas.h,list(APPEND FORMATS SORD_CAS)
##################################################

if (("SORD_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/sord_cas.cpp
	${MAME_DIR}/src/lib/formats/sord_cas.h
)
endif()

##################################################
##
##@src/lib/formats/spc1000_cas.h,list(APPEND FORMATS SPC1000_CAS)
##################################################

if (("SPC1000_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/spc1000_cas.cpp
	${MAME_DIR}/src/lib/formats/spc1000_cas.h
)
endif()

##################################################
##
##@src/lib/formats/st_dsk.h,list(APPEND FORMATS ST_DSK)
##################################################

if (("ST_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/st_dsk.cpp
	${MAME_DIR}/src/lib/formats/st_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/svi_cas.h,list(APPEND FORMATS SVI_CAS)
##################################################

if (("SVI_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/svi_cas.cpp
	${MAME_DIR}/src/lib/formats/svi_cas.h
)
endif()

##################################################
##
##@src/lib/formats/svi_dsk.h,list(APPEND FORMATS SVI_DSK)
##################################################

if (("SVI_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/svi_dsk.cpp
	${MAME_DIR}/src/lib/formats/svi_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/swd_dsk.h,list(APPEND FORMATS SWD_DSK)
##################################################

if (("SWD_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/swd_dsk.cpp
	${MAME_DIR}/src/lib/formats/swd_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/tandy2k_dsk.h,list(APPEND FORMATS TANDY2K_DSK)
##################################################

if (("TANDY2K_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/tandy2k_dsk.cpp
	${MAME_DIR}/src/lib/formats/tandy2k_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/thom_cas.h,list(APPEND FORMATS THOM_CAS)
##################################################

if (("THOM_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/thom_cas.cpp
	${MAME_DIR}/src/lib/formats/thom_cas.h
)
endif()

##################################################
##
##@src/lib/formats/thom_dsk.h,list(APPEND FORMATS THOM_DSK)
##################################################

if (("THOM_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/thom_dsk.cpp
	${MAME_DIR}/src/lib/formats/thom_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/ti99_dsk.h,list(APPEND FORMATS TI99_DSK)
##################################################

if (("TI99_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/ti99_dsk.cpp
	${MAME_DIR}/src/lib/formats/ti99_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/tiki100_dsk.h,list(APPEND FORMATS TIKI100_DSK)
##################################################

if (("TIKI100_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/tiki100_dsk.cpp
	${MAME_DIR}/src/lib/formats/tiki100_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/trd_dsk.h,list(APPEND FORMATS TRD_DSK)
##################################################

if (("TRD_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/trd_dsk.cpp
	${MAME_DIR}/src/lib/formats/trd_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/trs80_dsk.h,list(APPEND FORMATS TRS80_DSK)
##################################################

if (("TRS80_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/trs80_dsk.cpp
	${MAME_DIR}/src/lib/formats/trs80_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/trs_cas.h,list(APPEND FORMATS TRS_CAS)
##################################################

if (("TRS_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/trs_cas.cpp
	${MAME_DIR}/src/lib/formats/trs_cas.h
)
endif()

##################################################
##
##@src/lib/formats/tvc_cas.h,list(APPEND FORMATS TVC_CAS)
##################################################

if (("TVC_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/tvc_cas.cpp
	${MAME_DIR}/src/lib/formats/tvc_cas.h
)
endif()

##################################################
##
##@src/lib/formats/tvc_dsk.h,list(APPEND FORMATS TVC_DSK)
##################################################

if (("TVC_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/tvc_dsk.cpp
	${MAME_DIR}/src/lib/formats/tvc_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/tzx_cas.h,list(APPEND FORMATS TZX_CAS)
##################################################

if (("TZX_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/tzx_cas.cpp
	${MAME_DIR}/src/lib/formats/tzx_cas.h
)
endif()

##################################################
##
##@src/lib/formats/uef_cas.h,list(APPEND FORMATS UEF_CAS)
##################################################

if (("UEF_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/uef_cas.cpp
	${MAME_DIR}/src/lib/formats/uef_cas.h
)
endif()

##################################################
##
##@src/lib/formats/vdk_dsk.h,list(APPEND FORMATS VDK_DSK)
##################################################

if (("VDK_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/vdk_dsk.cpp
	${MAME_DIR}/src/lib/formats/vdk_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/vector06_dsk.h,list(APPEND FORMATS VECTOR06_DSK)
##################################################

if (("VECTOR06_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/vector06_dsk.cpp
	${MAME_DIR}/src/lib/formats/vector06_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/vg5k_cas.h,list(APPEND FORMATS VG5K_CAS)
##################################################

if (("VG5K_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/vg5k_cas.cpp
	${MAME_DIR}/src/lib/formats/vg5k_cas.h
)
endif()

##################################################
##
##@src/lib/formats/victor9k_dsk.h,list(APPEND FORMATS VICTOR9K_DSK)
##################################################

if (("VICTOR9K_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/victor9k_dsk.cpp
	${MAME_DIR}/src/lib/formats/victor9k_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/vt_cas.h,list(APPEND FORMATS VT_CAS)
##################################################

if (("VT_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/vt_cas.cpp
	${MAME_DIR}/src/lib/formats/vt_cas.h
)
endif()

##################################################
##
##@src/lib/formats/vt_dsk.h,list(APPEND FORMATS VT_DSK)
##################################################

if (("VT_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/vt_dsk.cpp
	${MAME_DIR}/src/lib/formats/vt_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/fs_vtech.h,list(APPEND FORMATS FS_VTECH)
##################################################

if (("FS_VTECH" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/fs_vtech.cpp
	${MAME_DIR}/src/lib/formats/fs_vtech.h
)
endif()

##################################################
##
##@src/lib/formats/wd177x_dsk.h,list(APPEND FORMATS WD177X_DSK)
##################################################

if (("WD177X_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/wd177x_dsk.cpp
	${MAME_DIR}/src/lib/formats/wd177x_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/x07_cas.h,list(APPEND FORMATS X07_CAS)
##################################################

if (("X07_CAS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/x07_cas.cpp
	${MAME_DIR}/src/lib/formats/x07_cas.h
)
endif()

##################################################
##
##@src/lib/formats/x1_tap.h,list(APPEND FORMATS X1_TAP)
##################################################

if (("X1_TAP" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/x1_tap.cpp
	${MAME_DIR}/src/lib/formats/x1_tap.h
)
endif()

##################################################
##
##@src/lib/formats/xdf_dsk.h,list(APPEND FORMATS XDF_DSK)
##################################################

if (("XDF_DSK" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/xdf_dsk.cpp
	${MAME_DIR}/src/lib/formats/xdf_dsk.h
)
endif()

##################################################
##
##@src/lib/formats/zx81_p.h,list(APPEND FORMATS ZX81_P)
##################################################

if (("ZX81_P" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/zx81_p.cpp
	${MAME_DIR}/src/lib/formats/zx81_p.h
)
endif()

##################################################
##
##@src/lib/formats/fs_prodos.h,list(APPEND FORMATS FS_PRODOS)
##################################################

if (("FS_PRODOS" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/fs_prodos.cpp
	${MAME_DIR}/src/lib/formats/fs_prodos.h
)
endif()

##################################################
##
##@src/lib/formats/fs_oric_jasmin.h,list(APPEND FORMATS FS_ORIC_JASMIN)
##################################################

if (("FS_ORIC_JASMIN" IN_LIST FORMATS) OR TOOLS)
target_sources(formats PRIVATE
	${MAME_DIR}/src/lib/formats/fs_oric_jasmin.cpp
	${MAME_DIR}/src/lib/formats/fs_oric_jasmin.h
)
endif()
