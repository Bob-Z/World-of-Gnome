function f (move_x,move_y)

player_id = player_get_id()

map = character_get_map(player_id)
tile_x = character_get_x(player_id)
tile_y = character_get_y(player_id)
map_w = character_get_map_w(map)
map_h = character_get_map_h(map)

new_tile_x = tile_x + move_x
new_tile_y = tile_y + move_y

if new_tile_x < 0 then
	call_script("goto.lua","-1","0");
        return
end
if new_tile_x >= map_w then
	call_script("goto.lua","1","0");
        return
end
if new_tile_y < 0 then
	call_script("goto.lua","0","-1");
        return
end
if new_tile_y >= map_h then
	call_script("goto.lua","0","1");
        return
end

character_set_pos(player_id,map,new_tile_x,new_tile_y)

tile_type = map_get_tile_type(map,0,new_tile_x,new_tile_y)

if tile_type == "b" then
	map_set_tile(map,0,"d.gif",new_tile_x,new_tile_y)
	map_set_tile_type(map,0,"d",new_tile_x,new_tile_y)
end

end
