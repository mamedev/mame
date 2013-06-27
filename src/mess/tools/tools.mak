###########################################################################
#
#   tools.mak
#
#   MESS tools makefile
#
###########################################################################


# add include path to tools directory
INCPATH += -I$(MESSSRC)/tools

# tools object directory
MESS_TOOLS = $(MESSOBJ)/tools

include $(MESSSRC)/tools/imgtool/imgtool.mak
TOOLS += $(IMGTOOL)

include $(MESSSRC)/tools/castool/castool.mak
TOOLS += $(CASTOOL)

include $(MESSSRC)/tools/floptool/floptool.mak
TOOLS += $(FLOPTOOL)
