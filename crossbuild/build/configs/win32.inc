
#-------------------------------------------------------------------
# Overrides for win32 builds 
#-------------------------------------------------------------------

# The following will return short filenames and allow building on paths
# with spaces in them. 
PWD = $(subst \,/,$(shell command /c cd))

PATH_SEP = \\
ECHO = echo
TOUCH = echo touch >
MKDIRP = -mkdir.exe -p
RMF = rm.exe -f
WIN32_LIMITED_TOOLSET = 1
MVF = cmd /c move /y

