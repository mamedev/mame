	T.xcode4_workspace = { }
	
	local suite = T.xcode4_workspace
	
	--[[suite.--]]local CONSTANT_PROJECT_NAME = "MyProject" 
	
	local sln, prj
	function suite.teardown()
		sln = nil
		prj = nil
	end
	function suite.setup()
		_ACTION = "xcode4"

		sln = solution "MySolution"
		configurations { "Debug", "Release" }
		platforms {}
		
		prj = project (CONSTANT_PROJECT_NAME) --"MyProject"
		language "C++"
		kind "ConsoleApp"
		uuid "AE61726D-187C-E440-BD07-2556188A6565"
	end
	
	local function get_buffer()
		premake.bake.buildconfigs()
		premake.xcode4.workspace_generate(sln)
		local buffer = io.endcapture()
		return buffer
	end
	
	function suite.xmlDeclarationPresent()
		local buffer = get_buffer()
		test.string_contains(buffer, '<%?xml version="1%.0" encoding="UTF%-8"%?>')
	end
	
	function suite.workspace_detailsEnclosedByVersionOneWorkSpaceTag()
		local buffer = get_buffer()
		test.string_contains(buffer,'<Workspace%s+version = "1%.0">.*</Workspace>')
	end
	
	function suite.workspace_addsProjectInFileRefTags()
		local buffer = get_buffer()
		test.string_contains(buffer,'<Workspace%s+version = "1%.0">%s+<FileRef.*</FileRef>%s+</Workspace>')	
	end

	function suite.workspace_fileRefFormat_locationAndGroup()
		local buffer = get_buffer()
		test.string_contains(buffer,'.*<FileRef%s+location = "group:.*">%s+</FileRef>')	
	end	
	function suite.workspace_fileRefFormat_projectNameAndExtension()
		local buffer = get_buffer()
		test.string_contains(buffer,'.*<FileRef%s+location = "group:'
										.. CONSTANT_PROJECT_NAME .. '.xcodeproj'
										..'">%s+</FileRef>')	
	end	

	function suite.pathPrefixAndProjectName_pathIsSameDir_noPrefixAdded()
		local buffer = get_buffer()
		test.string_contains(buffer,'.*<FileRef%s+location = "group:'
										.. CONSTANT_PROJECT_NAME .. '.xcodeproj'
										..'">%s+</FileRef>')	
	end

	function suite.pathPrefixAndProjectName_pathIsPathIsDifferentDir_pathPostfixSlashAdded()
		prj.location = "foo"
		local buffer = get_buffer()
		test.string_contains(buffer,'.*<FileRef%s+location = "group:'
										.. prj.location .. '/'.. CONSTANT_PROJECT_NAME .. '.xcodeproj'
										..'">%s+</FileRef>')	
	end