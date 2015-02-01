--
-- tests/actions/make/test_make_escaping.lua
-- Validate the escaping of literal values in Makefiles.
-- Copyright (c) 2010 Jason Perkins and the Premake project
--

	T.make_escaping = { }
	local suite = T.make_escaping


	function suite.Escapes_Spaces()
		test.isequal("Program\\ Files", _MAKE.esc("Program Files"))
	end

	function suite.Escapes_Backslashes()
		test.isequal("Program\\\\Files", _MAKE.esc("Program\\Files"))
	end
	
	function suite.Escapes_Parens()
		test.isequal("Debug\\(x86\\)", _MAKE.esc("Debug(x86)"))
	end
	
	function suite.DoesNotEscape_ShellReplacements()
		test.isequal("-L$(NVSDKCUDA_ROOT)/C/lib", _MAKE.esc("-L$(NVSDKCUDA_ROOT)/C/lib"))
	end
	
	function suite.CanEscape_ShellReplacementCapturesShortest()
		test.isequal("a\\(x\\)b$(ROOT)c\\(y\\)d", _MAKE.esc("a(x)b$(ROOT)c(y)d"))
	end

