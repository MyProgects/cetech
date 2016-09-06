for k, v in pairs(cetech) do _G[k] = v end

require 'core/fpscamera'

--local Log = cetech.Log
--local Application = cetech.Application
--local Keyboard = cetech.Keyboard

Game = Game or {}

local quit_btn = Keyboard.button_index 'q'
local debug_btn = Keyboard.button_index 'f9'
local reload_btn = Keyboard.button_index 'r'
local capture_btn = Keyboard.button_index 'f10'
local screenshot_btn = Keyboard.button_index 'f11'

function Game:init()
    Log.info("boot.lua", "Platform %s", Application.get_platform())

    Log.info("boot.lua", "info")
    Log.warning("boot.lua", "warn")
    Log.error("boot.lua", "error")
    Log.debug("boot.lua", "debug")

    --    self.viewport = Renderer.GetViewport("default")
    --    self.world = World.Create()

    --self.unit = Unit.Spawn(self.world, "unit1");
    --    self.level = World.LoadLevel(self.world, "level1");
    --    self.level = World.LoadLevel(self.world, "level1",
    --                 Vec3f.make(2, 5.0, 0.0),
    --                 Quatf.Identity, Vec3f.Unit);

    --    self.level_unit = World.LevelUnit(self.world, self.level)

    --    self.unit2 = World.UnitByName(self.world, self.level, "box2")
    --    self.unit = self.unit1

    --self.camera_unit = Unit.Spawn(self.world, "camera");
    --self.camera = Camera.GetCamera(self.world, self.camera_unit);
    --self.fps_camera = FPSCamera(self.world, self.camera_unit)
    --Unit.Spawn(self.world, "unit11");

    self.debug = false
    self.capture = false
    self.switch_unit = false
end

function Game:shutdown()
    Log.info("boot.lua", "shutdown")
    --World.Destroy(self.world);
end

function rotator(world, node, delta_rot)
    local rot = SceneGraph.GetRotation(world, node)
    rot = rot * delta_rot
    SceneGraph.SetRotation(world, node, rot)
end

function Game:update(dt)
    if Keyboard.button_pressed(reload_btn) then
        ResourceCompilator.compile_all()
        ResourceManager.reload_all()
    end

    if Keyboard.button_pressed(quit_btn) then
        Application.quit()
    end

    if Keyboard.button_pressed(debug_btn) then
        self.debug = not self.debug;
        Renderer.set_debug(self.debug)
    end

    if Keyboard.button_pressed(capture_btn) then
        self.capture = not self.capture;

        if self.capture then
            RenderSystem.BeginCapture()
        else
            RenderSystem.EndCapture()
        end
    end

    if Keyboard.button_pressed(screenshot_btn) then
        RenderSystem.SaveScreenShot("screenshot");
    end

    if Keyboard.button_pressed(Keyboard.button_index('f8')) then
        self.switch_unit = not self.switch_unit
        if self.switch_unit then
            self.unit = self.level_unit
        else
            self.unit = self.unit1
        end
    end

    --    Log.info("dddd", "dddddd")
    --    local transform = Transform.GetTransform(self.world, self.camera_unit)
    --    local pos = Transform.GetPosition(self.world, transform)
    --
    --    if Keyboard.button_pressed(Keyboard.button_index('up')) then
    --        pos.Y = pos.Y + 1;
    --        Transform.SetPosition(self.world, transform, pos)
    --    end
    --
    --    local dx = 0
    --    local dy = 0
    --    if Mouse.ButtonState(Mouse.button_index("left")) then
    --        local m_axis = Mouse.Axis(Mouse.AxisIndex("delta"))
    --        dx, dy = m_axis.X, -m_axis.Y
    --    end
    --
    --    local up = Keyboard.ButtonState(Keyboard.button_index('w'))
    --    local down = Keyboard.ButtonState(Keyboard.button_index('s'))
    --    local left = Keyboard.ButtonState(Keyboard.button_index('a'))
    --    local right = Keyboard.ButtonState(Keyboard.button_index('d'))
    --
    --    local updown = 0.0
    --    local leftdown = 0.0
    --    if up then updown = 1.0 end
    --    if down then updown = -1.0 end
    --    if left then leftdown = -1.0 end
    --    if right then leftdown = 1.0 end
    --
    --    self.fps_camera:update(dt, dx * 0.01, dy * 0.01, updown, leftdown)
    --
    --    if Gamepad.IsActive(0) then
    --        local right_a = Gamepad.Axis(0, Gamepad.AxisIndex("right"))
    --        local left_a = Gamepad.Axis(0, Gamepad.AxisIndex("left"))
    --        -- -- Log.info("lua", "{0}, {1}, {2}", left_a.X, left_a.Y, left_a.Z)
    --        self.fps_camera:update(dt, right_a.X*-0.06, right_a.Y*-0.06, left_a.Y, left_a.X)
    --
    --        if Gamepad.ButtonState(0, Gamepad.button_index("right_shoulder")) then
    --            self.fps_camera.fly_mode = true
    --        else
    --            self.fps_camera.fly_mode = false
    --        end
    --    end
    --
    --- -    -- local unit1 = World.UnitByName(self.world, self.level, "box1")
    ---- -- local node = SceneGraph.GetNodeByName(self.world, unit1, "n_geom_0")
    ---- -- rotator(self.world, node, Quatf.FromAxisAngle(Vec3f.UnitY, 0.05))
    --
    -- unit1 = World.UnitByName(self.world, self.level, "box2")
    -- node = SceneGraph.GetNodeByName(self.world, unit1, "n_cube")
    --
    -- rotator(self.world, node, Quatf.FromAxisAngle(Vec3f.UnitZ, 0.1))
    --
    -- World.Update(self.world, dt)
    -- -- print("%f, %f", m_axis.x, m_axis.y)
    -- -- print(dt)
end

function Game:render()
    Renderer.render_world(self.world, self.camera, self.viewport)
end

function foo(value)
    return value
end
