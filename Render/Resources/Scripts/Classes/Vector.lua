-- Many of these functions could be simply delegated to C++
-- But are currently implemented here because the less that
-- has to cross the boundary, the better.
-- Wide uses a left-handed system, not right-handed.

local Metatable

AddVectorTo = function(f)
    return function(...)
        local ret = f(...)
        setmetatable(ret, Metatable)
        return ret
    end
end

Wide.Vector.RotateByQuaternion = AddVectorTo(Wide.Vector.RotateByQuaternion)

local RotateByQuaternion = Wide.Vector.RotateByQuaternion
local sqrt = math.sqrt
local Dot = function(self, other)
    return Vector(self.x * other.x, self.y * other.y, self.z * other.z)
end
local Normalize = function(self)
    return self / #self
end

Metatable = {
    __add = function(lhs, rhs)
        return Vector(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z)
    end,
    __sub = function(lhs, rhs)
        return Vector(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z)
    end,
    __mul = function(lhs, scale)
        if type(scale) == "number" then
            return Vector(lhs.x * scale, rhs.x * scale, rhs.x * scale)
        end
        if getmetatable(lhs) == Metatable then
            return RotateByQuaternion(lhs, scale)
        else
            return RotateByQuaternion(scale, lhs)
        end
    end,
    __div = function(lhs, scale)
        return Vector(lhs.x / scale, lhs.y / scale, lhs.z / scale)
    end,
    __unm = function(self)
        return Vector(-self.x, -self.y, -self.z)
    end,
    __eq = function(lhs, rhs)
        return lhs.x == rhs.x and lhs.y == rhs.y and lhs.z == rhs.z
    end,
    __len = function(self)
        return sqrt(self.x^2, self.y^2, self.z^2)
    end,
    __index = function(t, k)
        if k == "Normalize" then
            return Normalize
        else if k == "Dot" then
            return Dot
        end
        return rawget(t, k)
    end,
    __newindex = function(t, k, v)
        if k != "Normalize" and k != "Dot" then
             return rawset(t, k, v)
        end
    end 
}

Vector = function(x, y, z)
    local ret = {
        x = x or 0,
        y = y or 0,
        z = z or 0
    }
    setmetatable(ret, Metatable)
    return ret
end