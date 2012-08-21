###########################################################################
#
#   windows.mak
#
#   MESS Windows-specific makefile
#
###########################################################################

MESS_WINSRC = src/mess/osd/windows
MESS_WINOBJ = $(OBJ)/mess/osd/windows

OBJDIRS += \
	$(MESSOBJ)/osd \
	$(MESSOBJ)/osd/windows

RESFILE = $(MESS_WINOBJ)/mess.res

$(LIBOSD): $(OSDOBJS)

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOCORE_NOMAIN): $(OSDCOREOBJS:$(WINOBJ)/main.o=)

$(RESFILE): $(MESS_WINSRC)/mess.rc $(WINOBJ)/mamevers.rc

#-------------------------------------------------
# generic rules for the resource compiler
#-------------------------------------------------

$(MESS_WINOBJ)/%.res: $(MESS_WINSRC)/%.rc
	@echo Compiling resources $<...
	$(RC) $(RCDEFS) $(RCFLAGS) --include-dir mess/$(OSD) -o $@ -i $<
