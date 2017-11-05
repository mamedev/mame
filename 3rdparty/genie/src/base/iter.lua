iter = {}

-- sortByKeys iterates over the table where the keys are in sort order
	
	function iter.sortByKeys(arr, f)
		local a = table.keys(arr)
		table.sort(a, f)
		
		local i = 0
		return function()
			i = i + 1
			if a[i] ~= nil then 
				return a[i], arr[a[i]]
			end
		end
	end
