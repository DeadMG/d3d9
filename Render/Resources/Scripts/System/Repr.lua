repr = function(t, space, used)
    space = space or ""
	used = used or {}
	if (type(t) == "number") or (type(t) == "string") then
	    return t .. "\n"
    end
	if t == _G then
	    return "_G\n"
    end
	if type(t) == "nil" then
	    return "nil\n"
    end
	if type(t) == "boolean" then
	    if (t) then
		    return "true\n"
		else
		    return "false\n"
		end
	end
	if type(t) == "function" then
	    return "function"
	end
	local result = "{\n"
	if not used[t] then
	    used[t] = true
	else
	    error("Attempted to repr a cyclic graph! The current state was: " .. debug.traceback())
	end
	for k, v in pairs(t) do
	    if type(k) == "table" then
		     k = "table"
	    end
		if type(k) == "function" then
		     k = "function"
	    end
		if type(k) == "number" then
		     k = "[" .. k .. "]"
	    end
		result = result .. space .. "    " .. k .. " = " .. repr(v, space .. "    ", used)
	end
	result = result .. space .. "}\n"
	return result
end