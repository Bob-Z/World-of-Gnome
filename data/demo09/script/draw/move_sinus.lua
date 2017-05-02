function f ()

-- id = player_get_id()
id = context_get_id()

item_X = item_get_x()
item_Y = item_get_y()
item_W = item_get_w()
item_H = item_get_h()

-- Init tables
if( _G.start_tick == nil ) then
	_G.start_tick = {}
end
if( _G.dest_X == nil ) then
	_G.dest_X = {}
end
if( _G.dest_Y == nil ) then
	_G.dest_Y = {}
end
if( _G.current_X == nil ) then
	_G.current_X = {}
end
if( _G.current_Y == nil ) then
	_G.current_Y = {}
end
if( _G.current_W == nil ) then
	_G.current_W = {}
end
if( _G.current_H == nil ) then
	_G.current_H = {}
end
if( _G.from_X == nil ) then
	_G.from_X = {}
end
if( _G.from_Y == nil ) then
	_G.from_Y = {}
end
if( _G.orientation == nil ) then
	_G.orientation = {}
end
if( _G.map == nil ) then
	_G.map = {}
end
if( _G.reset == nil ) then
	_G.reset = {}
end
if( _G.sinus_time == nil ) then
	_G.sinus_time = {}
end


-- Init per context variables
if( _G.start_tick[id] == nil ) then
	_G.start_tick[id] = 0
end
if( _G.dest_X[id] == nil ) then
	_G.current_X[id] = item_X
	_G.dest_X[id] = item_X
end
if( _G.dest_Y[id] == nil ) then
	_G.current_Y[id] = item_Y
	_G.dest_Y[id] = item_Y
end
if( _G.map[id] == nil ) then
	_G.map[id] = context_get_map()
end
if( _G.orientation[id] == nil ) then
	_G.orientation[id] = "s"
end
if( _G.reset[id] == nil ) then
	_G.reset[id] = true
end
if( _G.sinus_time[id] == nil ) then
	_G.sinus_time[id] = get_tick()
end


-- Detect movement
dX = 0
dY = 0

-- Force position on map change
if( _G.reset[id] == true ) then
	_G.current_X[id] = item_X
	_G.current_Y[id] = item_Y
	_G.from_X[id] = item_X
	_G.from_Y[id] = item_Y
	_G.dest_X[id] = item_X
	_G.dest_Y[id] = item_Y
	_G.start_tick[id] = get_tick()
	_G.map[id] = context_get_map()
	_G.reset[id] = false
end

if( _G.dest_X[id] ~= item_X ) then
	_G.from_X[id] = _G.current_X[id]
	_G.from_Y[id] = _G.current_Y[id]
	_G.dest_X[id] = item_X
	_G.start_tick[id] = get_tick()
	dX = _G.dest_X[id] - _G.from_X[id]
-- text = string.format("_G.from_X = %d, _G.dest_X = %d, _G.start_tick = %d",_G.from_X[id], _G.dest_X[id], _G.start_tick[id])
-- print_text_debug(text)
end

if( _G.dest_Y[id] ~= item_Y ) then
	_G.from_X[id] = _G.current_X[id]
	_G.from_Y[id] = _G.current_Y[id]
	_G.dest_Y[id] = item_Y
	_G.start_tick[id] = get_tick()
	dY = _G.dest_Y[id] - _G.from_Y[id]
-- text = string.format("_G.from_Y = %d, _G.dest_Y = %d, _G.start_tick = %d",_G.from_Y[id], _G.dest_Y[id], _G.start_tick[id])
--	 print_text_debug(text)
end

if( _G.map[id] ~= context_get_map() ) then
	_G.reset[id] = true
end

-- save width for other scripts
_G.current_W[id] = item_W
_G.current_H[id] = item_H

-- Calculate orientation
if( dX ~= 0 or dY ~= 0) then
	if( math.abs(dX) > math.abs(dY) ) then
		if( dX > 0 ) then
			_G.orientation[id] = "e"
		else
			_G.orientation[id] = "w"
		end
	else
		if( dY > 0 ) then
			_G.orientation[id] = "s"
		else
			_G.orientation[id] = "n"
		end
	end
end

current_tick = get_tick()

move_duration_ms = 1000
if( context_get_npc() == 0 ) then
	move_duration_ms = 100
end

delta_time = current_tick - _G.start_tick[id]

if( delta_time < move_duration_ms) then
	_G.current_X[id] = _G.from_X[id] + ((_G.dest_X[id] - _G.from_X[id]) * delta_time / move_duration_ms)
	_G.current_Y[id] = _G.from_Y[id] + ((_G.dest_Y[id] - _G.from_Y[id]) * delta_time / move_duration_ms)
	item_set_anim_from_context(id,"sprite_move_" .. _G.orientation[id]);
else
	_G.current_X[id] = _G.dest_X[id]
	_G.current_Y[id] = _G.dest_Y[id]
	item_set_anim_from_context(id,"sprite_" .. _G.orientation[id]);
end

--  text = string.format("_G.current_X[%s] = %d, _G.current_Y[%s] = %d",id, _G.current_X[id], id, _G.current_Y[id])
--  print_text_debug(text)

delta_time = current_tick - _G.sinus_time[id]
calculated_x = _G.current_X[id] + 5 *( math.sin(delta_time / 405))
calculated_y = _G.current_Y[id] + 5 *( math.sin(delta_time / 333))

 item_set_x(calculated_x)
 item_set_y(calculated_y)

return 0

end

