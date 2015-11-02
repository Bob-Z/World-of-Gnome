function f (move_x,move_y)

player_id = player_get_id()

map = character_get_map(player_id)
pos_x = character_get_x(player_id)
pos_y = character_get_y(player_id)
map_w = character_get_map_w(map)
map_h = character_get_map_h(map)

new_pos_x = pos_x + move_x
new_pos_y = pos_y + move_y

if new_pos_x < 0 then
	call_script("goto.lua","-1","0");
        return
end
if new_pos_x >= map_w then
	call_script("goto.lua","1","0");
        return
end
if new_pos_y < 0 then
	call_script("goto.lua","0","-1");
        return
end
if new_pos_y >= map_h then
	call_script("goto.lua","0","1");
        return
end

character_set_pos(player_id,map,new_pos_x,new_pos_y)

tile_type = map_get_tile_type(map,0,new_pos_x,new_pos_y)

if tile_type == "b" then
	map_set_tile(map,0,"d.gif",new_pos_x,new_pos_y)
	map_set_tile_type(map,0,"d",new_pos_x,new_pos_y)
end

end
