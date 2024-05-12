local Metatable

AddQuaternionTo = function(f)
    return function(...)
        local ret = f(...)
        setmetatable(ret, Metatable)
        return ret
    end
end