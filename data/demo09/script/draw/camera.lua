function f ()

id = player_get_id()

-- Init tables
if( _G.camera_start_tick == nil ) then
	_G.camera_start_tick = {}
end
if( _G.dest_camera_Z == nil ) then
	_G.dest_camera_Z = {}
end
if( _G.current_camera_Z == nil ) then
	_G.current_camera_Z = {}
end

camera_set_coord(_G.current_X[id], (_G.current_Y[id]))

return 0

end

