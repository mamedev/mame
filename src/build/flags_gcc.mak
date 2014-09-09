# TODO: needs to use $(CC)
TEST_GCC := $(shell gcc --version)

ifeq ($(findstring 4.7.,$(TEST_GCC)),4.7.)
	CCOMFLAGS += -Wno-narrowing -Wno-attributes
endif

ifeq ($(findstring 4.8.,$(TEST_GCC)),4.8.)
	CCOMFLAGS += -Wno-narrowing -Wno-attributes -Wno-unused-local-typedefs
	# array bounds checking seems to be buggy in 4.8.1 (try it on video/stvvdp1.c and video/model1.c without -Wno-array-bounds)
	CCOMFLAGS += -Wno-unused-variable -Wno-array-bounds
endif

ifeq ($(findstring 4.9.,$(TEST_GCC)),4.9.)
	CCOMFLAGS += -Wno-narrowing -Wno-attributes -Wno-unused-local-typedefs
	CCOMFLAGS += -Wno-array-bounds
endif

ifeq ($(findstring arm,$(UNAME)),arm)
	CCOMFLAGS += -Wno-cast-align
endif