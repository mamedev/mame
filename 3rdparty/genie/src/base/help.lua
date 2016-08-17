--
-- help.lua
-- User help, displayed on /help option.
-- Copyright (c) 2002-2008 Jason Perkins and the Premake project
--


	function premake.showhelp()

		-- display the basic usage
		printf("")
		printf("Usage: genie [options] action [arguments]")
		printf("")


		-- display all options
		printf("OPTIONS")
		printf("")
		for option in premake.option.each() do
			local trigger = option.trigger
			local description = option.description
			if (option.value) then trigger = trigger .. "=" .. option.value end
			if (option.allowed) then description = description .. "; one of:" end

			printf(" --%-15s %s", trigger, description)
			if (option.allowed) then
				for _, value in ipairs(option.allowed) do
					printf("     %-14s %s", value[1], value[2])
				end
			end
			printf("")
		end

		-- display all actions
		printf("ACTIONS")
		printf("")
		for action in premake.action.each() do
			printf(" %-17s %s", action.trigger, action.description)
		end
		printf("")


		-- see more
		printf("For additional information, see https://github.com/bkaradzic/genie")

	end


