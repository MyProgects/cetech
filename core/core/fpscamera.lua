require 'core/class'

local Transform = cetech.Transform
local Camera = cetech.Camera
local Mat4f = cetech.Mat4f
local Quatf = cetech.Quatf

FPSCamera = class(FPSCamera)

function FPSCamera:init(world, camera_unit, fly_mode)
    self.world = world
    self.unit = camera_unit
    self.camera = Camera.get(world, camera_unit)
    self.transform = Transform.get(world, camera_unit)

    self.fly_mode = fly_mode or false
end

function FPSCamera:setFlyMode(enable)
    self.fly_mode = enable
end

function FPSCamera:update(dt, dx, dy, updown, leftright)
    local pos = Transform.get_position(self.world, self.transform)
    local rot = Transform.get_rotation(self.world, self.transform)

    --Log.Info("pos", "{0} {1} {2}", pos.X, pos.Y, pos.Z)

    local m_world = Transform.get_world_matrix(self.world, self.transform)
    local z_dir = -Mat4f.Z(m_world)
    local x_dir = Mat4f.X(m_world)

    -- Rotation
    local rotation_around_world_up = Quatf.from_axis_angle(Vec3f.UnitY, dx * 1.0)
    local rotation_around_camera_right = Quatf.from_axis_angle(x_dir, -dy * 1.0)
    local rotation = rotation_around_world_up * rotation_around_camera_right
    Transform.set_rotation(self.world, self.transform, rotation * rot)

    -- Position
    if not self.fly_mode then
        z_dir.Y = 0.0
    end

    pos = pos + z_dir * updown
    pos = pos + x_dir * leftright

    --Log.Info("z_dir", "{0} {1} {2}", z_dir.X, z_dir.Y, z_dir.Z)

    Transform.set_position(self.world, self.transform, pos)
end