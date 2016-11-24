local xcode6 = premake.xcode.xcode6

function xcode6.generate_project_config(prj)
	_p("PRODUCT_NAME = $(TARGET_NAME)")
	
	if premake.isswiftproject(prj) then
		_p("OTHER_SWIFT_FLAGS = -DXcode")
		_p("USE_HEADERMAP = NO")
	end
end