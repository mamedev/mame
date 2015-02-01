--
-- tests/actions/xcode/test_xcode_dependencies.lua
-- Automated test suite for Xcode project dependencies.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	T.xcode3_deps = { }
	
	local suite = T.xcode3_deps
	local xcode = premake.xcode


---------------------------------------------------------------------------
-- Setup/Teardown
---------------------------------------------------------------------------

	local sln, prj, prj2, tr
	function suite.setup()
		premake.action.set("xcode3")
		xcode.used_ids = { } -- reset the list of generated IDs

		sln, prj = test.createsolution()
		links { "MyProject2" }

		prj2 = test.createproject(sln)
		kind "StaticLib"
		configuration "Debug"
		targetsuffix "-d"
	end

	local function prepare()
		premake.bake.buildconfigs()
		xcode.preparesolution(sln)
		tr = xcode.buildprjtree(premake.solution.getproject(sln, 1))
	end


---------------------------------------------------------------------------
-- PBXBuildFile tests
---------------------------------------------------------------------------

	function suite.PBXBuildFile_ListsDependencyTargets_OnStaticLib()
		prepare()
		xcode.PBXBuildFile(tr)
		test.capture [[
/* Begin PBXBuildFile section */
		[libMyProject2-d.a:build] /* libMyProject2-d.a in Frameworks */ = {isa = PBXBuildFile; fileRef = [libMyProject2-d.a] /* libMyProject2-d.a */; };
/* End PBXBuildFile section */
		]]
	end

	function suite.PBXBuildFile_ListsDependencyTargets_OnSharedLib()
		kind "SharedLib"
		prepare()
		xcode.PBXBuildFile(tr)
		test.capture [[
/* Begin PBXBuildFile section */
		[libMyProject2-d.dylib:build] /* libMyProject2-d.dylib in Frameworks */ = {isa = PBXBuildFile; fileRef = [libMyProject2-d.dylib] /* libMyProject2-d.dylib */; };
/* End PBXBuildFile section */
		]]
	end


---------------------------------------------------------------------------
-- PBXContainerItemProxy tests
---------------------------------------------------------------------------

	function suite.PBXContainerItemProxy_ListsProjectConfigs()
		prepare()
		xcode.PBXContainerItemProxy(tr)
		test.capture [[
/* Begin PBXContainerItemProxy section */
		[MyProject2.xcodeproj:prodprox] /* PBXContainerItemProxy */ = {
			isa = PBXContainerItemProxy;
			containerPortal = [MyProject2.xcodeproj] /* MyProject2.xcodeproj */;
			proxyType = 2;
			remoteGlobalIDString = [libMyProject2-d.a:product];
			remoteInfo = "libMyProject2-d.a";
		};
		[MyProject2.xcodeproj:targprox] /* PBXContainerItemProxy */ = {
			isa = PBXContainerItemProxy;
			containerPortal = [MyProject2.xcodeproj] /* MyProject2.xcodeproj */;
			proxyType = 1;
			remoteGlobalIDString = [libMyProject2-d.a:target];
			remoteInfo = "libMyProject2-d.a";
		};
/* End PBXContainerItemProxy section */
		]]		
	end


---------------------------------------------------------------------------
-- PBXFileReference tests
---------------------------------------------------------------------------

	function suite.PBXFileReference_ListsDependencies()
		prepare()
		xcode.PBXFileReference(tr)
		test.capture [[
/* Begin PBXFileReference section */
		[MyProject:product] /* MyProject */ = {isa = PBXFileReference; explicitFileType = "compiled.mach-o.executable"; includeInIndex = 0; name = "MyProject"; path = "MyProject"; sourceTree = BUILT_PRODUCTS_DIR; };
		[MyProject2.xcodeproj] /* MyProject2.xcodeproj */ = {isa = PBXFileReference; lastKnownFileType = "wrapper.pb-project"; name = "MyProject2.xcodeproj"; path = "MyProject2.xcodeproj"; sourceTree = SOURCE_ROOT; };
/* End PBXFileReference section */
		]]
	end

	function suite.PBXFileReference_UsesRelativePaths()
		prj.location = "MyProject"
		prj2.location = "MyProject2"
		prepare()
		xcode.PBXFileReference(tr)
		test.capture [[
/* Begin PBXFileReference section */
		[MyProject:product] /* MyProject */ = {isa = PBXFileReference; explicitFileType = "compiled.mach-o.executable"; includeInIndex = 0; name = "MyProject"; path = "MyProject"; sourceTree = BUILT_PRODUCTS_DIR; };
		[MyProject2.xcodeproj] /* MyProject2.xcodeproj */ = {isa = PBXFileReference; lastKnownFileType = "wrapper.pb-project"; name = "MyProject2.xcodeproj"; path = "../MyProject2/MyProject2.xcodeproj"; sourceTree = SOURCE_ROOT; };
/* End PBXFileReference section */
		]]
	end


---------------------------------------------------------------------------
-- PBXFrameworksBuildPhase tests
---------------------------------------------------------------------------

	function suite.PBXFrameworksBuildPhase_ListsDependencies_OnStaticLib()
		prepare()
		xcode.PBXFrameworksBuildPhase(tr)
		test.capture [[
/* Begin PBXFrameworksBuildPhase section */
		[MyProject:fxs] /* Frameworks */ = {
			isa = PBXFrameworksBuildPhase;
			buildActionMask = 2147483647;
			files = (
				[libMyProject2-d.a:build] /* libMyProject2-d.a in Frameworks */,
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXFrameworksBuildPhase section */
		]]
	end

	function suite.PBXFrameworksBuildPhase_ListsDependencies_OnSharedLib()
		kind "SharedLib"
		prepare()
		xcode.PBXFrameworksBuildPhase(tr)
		test.capture [[
/* Begin PBXFrameworksBuildPhase section */
		[MyProject:fxs] /* Frameworks */ = {
			isa = PBXFrameworksBuildPhase;
			buildActionMask = 2147483647;
			files = (
				[libMyProject2-d.dylib:build] /* libMyProject2-d.dylib in Frameworks */,
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXFrameworksBuildPhase section */
		]]
	end

---------------------------------------------------------------------------
-- PBXGroup tests
---------------------------------------------------------------------------

	function suite.PBXGroup_ListsDependencies()
		prepare()
		xcode.PBXGroup(tr)
		test.capture [[
/* Begin PBXGroup section */
		[MyProject] /* MyProject */ = {
			isa = PBXGroup;
			children = (
				[Products] /* Products */,
				[Projects] /* Projects */,
			);
			name = "MyProject";
			sourceTree = "<group>";
		};
		[Products] /* Products */ = {
			isa = PBXGroup;
			children = (
				[MyProject:product] /* MyProject */,
			);
			name = "Products";
			sourceTree = "<group>";
		};
		[Projects] /* Projects */ = {
			isa = PBXGroup;
			children = (
				[MyProject2.xcodeproj] /* MyProject2.xcodeproj */,
			);
			name = "Projects";
			sourceTree = "<group>";
		};
		[MyProject2.xcodeproj:prodgrp] /* Products */ = {
			isa = PBXGroup;
			children = (
				[libMyProject2-d.a] /* libMyProject2-d.a */,
			);
			name = Products;
			sourceTree = "<group>";
		};
/* End PBXGroup section */
		]]
	end


---------------------------------------------------------------------------
-- PBXNativeTarget tests
---------------------------------------------------------------------------

	function suite.PBXNativeTarget_ListsDependencies()
		prepare()
		xcode.PBXNativeTarget(tr)
		test.capture [[
/* Begin PBXNativeTarget section */
		[MyProject:target] /* MyProject */ = {
			isa = PBXNativeTarget;
			buildConfigurationList = [MyProject:cfg] /* Build configuration list for PBXNativeTarget "MyProject" */;
			buildPhases = (
				[MyProject:rez] /* Resources */,
				[MyProject:src] /* Sources */,
				[MyProject:fxs] /* Frameworks */,
			);
			buildRules = (
			);
			dependencies = (
				[MyProject2.xcodeproj:targdep] /* PBXTargetDependency */,
			);
			name = "MyProject";
			productInstallPath = "$(HOME)/bin";
			productName = "MyProject";
			productReference = [MyProject:product] /* MyProject */;
			productType = "com.apple.product-type.tool";
		};
/* End PBXNativeTarget section */
		]]
	end


---------------------------------------------------------------------------
-- PBXProject tests
---------------------------------------------------------------------------

	function suite.PBXProject_ListsDependencies()
		prepare()
		xcode.PBXProject(tr)
		test.capture [[
/* Begin PBXProject section */
		08FB7793FE84155DC02AAC07 /* Project object */ = {
			isa = PBXProject;
			buildConfigurationList = 1DEB928908733DD80010E9CD /* Build configuration list for PBXProject "MyProject" */;
			compatibilityVersion = "Xcode 3.2";
			hasScannedForEncodings = 1;
			mainGroup = [MyProject] /* MyProject */;
			projectDirPath = "";
			projectReferences = (
				{
					ProductGroup = [MyProject2.xcodeproj:prodgrp] /* Products */;
					ProjectRef = [MyProject2.xcodeproj] /* MyProject2.xcodeproj */;
				},
			);
			projectRoot = "";
			targets = (
				[MyProject:target] /* MyProject */,
			);
		};
/* End PBXProject section */
		]]
	end


---------------------------------------------------------------------------
-- PBXReferenceProxy tests
---------------------------------------------------------------------------

	function suite.PBXReferenceProxy_ListsDependencies()
		prepare()
		xcode.PBXReferenceProxy(tr)
		test.capture [[
/* Begin PBXReferenceProxy section */
		[libMyProject2-d.a] /* libMyProject2-d.a */ = {
			isa = PBXReferenceProxy;
			fileType = archive.ar;
			path = "libMyProject2-d.a";
			remoteRef = [MyProject2.xcodeproj:prodprox] /* PBXContainerItemProxy */;
			sourceTree = BUILT_PRODUCTS_DIR;
		};
/* End PBXReferenceProxy section */
		]]
	end


---------------------------------------------------------------------------
-- PBXTargetDependency tests
---------------------------------------------------------------------------

	function suite.PBXTargetDependency_ListsDependencies()
		prepare()
		xcode.PBXTargetDependency(tr)
		test.capture [[
/* Begin PBXTargetDependency section */
		[MyProject2.xcodeproj:targdep] /* PBXTargetDependency */ = {
			isa = PBXTargetDependency;
			name = "libMyProject2-d.a";
			targetProxy = [MyProject2.xcodeproj:targprox] /* PBXContainerItemProxy */;
		};
/* End PBXTargetDependency section */
		]]
	end
