function f ()

id = player_get_id()
screen = 0

-- Init tables
if( _G.camera_start_tick == nil ) then
	_G.camera_start_tick = {}
end
if( _G.dest_camera_Z == nil ) then
	_G.dest_camera_Z = {}
end
if( _G.from_camera_Z == nil ) then
	_G.from_camera_Z = {}
end
if( _G.current_camera_Z == nil ) then
	_G.current_camera_Z = {}
end

-- Init per screen variables
if( _G.camera_start_tick[screen] == nil ) then
        _G.camera_start_tick[screen] = 0
end
if( _G.dest_camera_Z[screen] == nil ) then
        _G.dest_camera_Z[screen] = 1.0
end
if( _G.from_camera_Z[screen] == nil ) then
        _G.from_camera_Z[screen] = 1.0
end
if( _G.current_camera_Z[screen] == nil ) then
        _G.current_camera_Z[screen] = 1.0
end

-- convert zoom unit to zoom percentage
zoom_unit = camera_get_zoom();
zoom_percent = 1.0
if( zoom_unit ~= 0 ) then
	if( zoom_unit > 0 ) then
		for i=1,zoom_unit do
			zoom_percent = zoom_percent * 1.1
		end
	else
		zoom_percent = 1.0
		for i=zoom_unit,-1 do
			zoom_percent = zoom_percent / 1.1
		end
	end
end

-- smooth zoom
if( _G.dest_camera_Z[screen] ~= zoom_percent) then
	_G.dest_camera_Z[screen] = zoom_percent
	_G.from_camera_Z[screen] = _G.current_camera_Z[screen]
	_G.camera_start_tick[screen] = get_tick()
end

move_duration_ms = 200
delta_time = get_tick() - _G.camera_start_tick[screen]

if( delta_time < move_duration_ms) then
        _G.current_camera_Z[screen] = _G.from_camera_Z[screen] + ((_G.dest_camera_Z[screen] - _G.from_camera_Z[screen]) * delta_time / move_duration_ms)
else
	_G.current_camera_Z[screen] = _G.dest_camera_Z[screen]
end

camera_set_zoom(_G.current_camera_Z[screen]);

-- set camera position
camera_set_coord(_G.current_X[id], (_G.current_Y[id]))

return 0

end

