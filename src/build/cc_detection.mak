ifneq (,$(findstring clang,$(CC)))
	include $(SRC)/build/flags_clang.mak
else
	ifneq (,$(findstring emcc,$(CC)))
		# Emscripten compiler is based on clang
		include $(SRC)/build/flags_clang.mak
	else
		TEST_GCC = $(shell gcc --version)
		# is it Clang symlinked/renamed to GCC (Xcode 5.0 on OS X)?
		ifeq ($(findstring clang,$(TEST_GCC)),clang)
			include $(SRC)/build/flags_clang.mak
		else
			include $(SRC)/build/flags_gcc.mak
		endif
	endif
endif
