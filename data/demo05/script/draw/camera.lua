function f ()

id = player_get_id()
screen = camera_get_screen()

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

-- set zoom
if( screen == 2 ) then -- PLAY screen
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
end

camera_set_zoom(_G.current_camera_Z[screen]);

----------------------
-- set camera position
if( screen == 0 or screen == 1) then -- SELECT screen or CREATE screen
	if( _G.dest_camera_X == nil ) then
		_G.dest_camera_X = camera_get_X()
		_G.from_camera_X = camera_get_X()
		_G.current_camera_X = camera_get_X()
		_G.start_camera_tick = 0
	end
	if( _G.dest_camera_Y == nil ) then
		_G.dest_camera_Y = camera_get_Y()
		_G.from_camera_Y = camera_get_Y()
		_G.current_camera_Y = camera_get_Y()
		_G.start_camera_tick = 0
	end

	-- detect movement
	if( _G.dest_camera_X ~= camera_get_X() or _G.dest_camera_Y ~= camera_get_Y()) then
		_G.from_camera_X = _G.current_camera_X
		_G.from_camera_Y = _G.current_camera_Y
		_G.dest_camera_X = camera_get_X()
		_G.dest_camera_Y = camera_get_Y()
		_G.start_camera_tick = get_tick()
	end

	current_tick = get_tick()
	move_duration_ms = 250
	delta_time = current_tick - _G.start_camera_tick

	if( delta_time < move_duration_ms) then
		_G.current_camera_X = _G.from_camera_X + ((_G.dest_camera_X - _G.from_camera_X) * delta_time / move_duration_ms)
		_G.current_camera_Y = _G.from_camera_Y + ((_G.dest_camera_Y - _G.from_camera_Y) * delta_time / move_duration_ms)
	else
		_G.current_camera_X = _G.dest_camera_X
		_G.current_camera_Y = _G.dest_camera_Y
	end

	camera_set_coord(_G.current_camera_X,_G.current_camera_Y) 
end

if( screen == 2 ) then -- PLAY screen
	camera_set_coord(_G.current_X[id], (_G.current_Y[id]))
end

return 0

end

