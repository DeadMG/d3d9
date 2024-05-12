local modules = {}
local envmeta = {
    __index = _G
}
import = function(filename)
    filename = "Resources/Scripts/" .. filename
    if modules[filename] then return modules[filename] end
	modules[filename] = setmetatable({}, envmeta)
	local ok, msg = pcall(setfenv(loadfile(filename), modules[filename]))
	if (!ok) then
	    LOG(msg)
	end
	return modules[filename]
end