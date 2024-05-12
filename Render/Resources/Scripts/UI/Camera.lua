-- The camera's position and other vectors have a special metatable, because they delegate
-- directly to C++ instead of being a stored Lua Vector.
-- This makes it much more intuitive to manipulate those values from Lua
-- In addition, the valid_cameras table is used to ensure that a destroyed object is never used from Lua

local AddVectorTo = import("Classes/Vector.lua").AddVectorTo

-- Hook these engine functions to add vector operations
Wide.Camera.GetPosition = AddVectorTo(Wide.Camera.GetPosition)
Wide.Camera.GetLookAt = AddVectorTo(Wide.Camera.GetLookAt)
Wide.Camera.GetUp = AddVectorTo(Wide.Camera.GetUp)
Wide.Camera.Unproject = AddVectorTo(Wide.Camera.Unproject)

-- and local them for increased access speed
local GetPosition = Wide.Camera.GetPosition
local SetPosition = Wide.Camera.SetPosition
local GetLookAt = Wide.Camera.GetLookAt
local SetLookAt = Wide.Camera.SetLookAt
local GetUp = Wide.Camera.GetUp
local SetUp = Wide.Camera.SetUp
local Create = Wide.Camera.Create
local Destroy = Wide.Camera.Destroy
local SetFoVY = Wide.Camera.SetFoVY
local GetFoVY = Wide.Camera.GetFoVY
local SetNearPlane = Wide.Camera.SetNearPlane
local SetFarPlane = Wide.Camera.SetFarPlane
local GetNearPlane = Wide.Camera.GetNearPlane
local GetFarPlane = Wide.Camera.GetFarPlane
local GetDimensions = Wide.Camera.GetDimensions
local Unproject = Wide.Camera.Unproject

local pos_or_param = function(pos, get)
    if type(pos) != "table" then
        return pos
    end
    if pos.Owner and pos.Owner.InternalHandle then
        pos = get(pos.Owner.InternalHandle)
    end
    return pos
end

local GetMetatableForSpecialVector = function(get, set)
    return {
        __index = function(t, k)
            return get(t.Owner.InternalHandle)[k]
        end,
        __newindex = function(t, k, v)
            local pos = get(t.Owner.InternalHandle)
            pos[k] = v
            set(t.Owner.InternalHandle, pos)
        end,
        -- For arithmetic overloads, remember we may be on the lhs *or* the rhs
        __add = function(lhs, rhs)
            return pos_or_param(lhs, get) + pos_or_param(rhs, get)
        end,
        __sub = function(lhs, rhs)
            return pos_or_param(lhs, get) - pos_or_param(rhs, get)
        end,
        __mul = function(lhs, rhs)
            return pos_or_param(lhs, get) * pos_or_param(rhs, get)
        end,
        __div = function(lhs, rhs)     
            return pos_or_param(lhs, get) / pos_or_param(rhs, get)
        end,
        __len = function(pos)
            return #get(pos.Owner.InternalHandle)
        end
    }
end

local cam_meta = {
    __index = function(t, k)
        if k == "NearPlane" then
            return GetNearPlane(t.InternalHandle)
        else if k == "FarPlane" then
            return GetFarPlane(t.InternalHandle)
        else if k == "FoVY" then
            return GetFoVY(t.InternalHandle)
        else if k == "Dimensions" then
            return GetDimensions(t.InternalHandle)
        end
        return rawget(t, k)
    end
    __newindex = function(t, k, v)
        if k == "NearPlane" then
            SetNearPlane(t.InternalHandle, v)
            return v
        else if k == "FarPlane" then
            SetFarPlane(t.InternalHandle, v)
            return v
        else if k == "FoVY" then
            SetFoVY(t.InternalHandle, v)
            return v
        end
        return rawset(t, k, v)
    end
}

Camera = function(dimensions)
    local cam_pos = {}
    local result = {
        InternalHandle = Create(dimensions),
        Position = {
            Owner = result
        },
        LookAt = {
            Owner = result
        },
        Up = {
            Owner = result
        },
        Destroy = function(self)
            Destroy(self.InternalHandle)
        end,
        Unproject = function(self, vector)
            return Unproject(self.InternalHandle, vector)
        end
    }
    setmetatable(result, cam_meta)
    setmetatable(result.Position, GetMetatableForSpecialVector(GetPosition, SetPosition))
    setmetatable(result.LookAt, GetMetatableForSpecialVector(GetLookAt, SetLookAt))
    setmetatable(result.Position, GetMetatableForSpecialVector(GetUp, SetUp))
    return result
end