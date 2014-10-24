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

if( x == "1" ) then px = 0 end
if( x == "-1" ) then px = 31 end
if( y == "1" ) then py = 0 end
if( y == "-1" ) then py = 31 end

res = character_set_pos(player_id,map_name,px,py)

if res == -1 then
	call_script("add_map.lua",map_name,new_map_x,new_map_y);
	res = character_set_pos(player_id,map_name,px,py)
end

tile_type = map_get_tile_type(map_name,px,py)

if tile_type == "b" then
	map_set_tile(map_name,"d.gif",px,py,0)
	map_set_tile_type(map_name,"d",px,py)
end
end
