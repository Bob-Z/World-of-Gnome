function f (x, y)

player_id = player_get_id()
px = character_get_x(player_id)
py = character_get_y(player_id)
map = character_get_map(player_id)

map_x = map_attribute_get(map,"x")
map_y = map_attribute_get(map,"y")

new_map_x = map_x + x
new_map_y = map_y + y

map_name = string.format("M%d_%d",new_map_x,new_map_y)

if( x == "1" ) then px = 1 end
if( x == "-1" ) then px = 30 end
if( y == "1" ) then py = 1 end
if( y == "-1" ) then py = 30 end

res = character_set_pos(player_id,map_name,px,py)

if res == -1 then
	call_script("add_map.lua",map_name,new_map_x,new_map_y);
	res = character_set_pos(player_id,map_name,px,py)
end


end
