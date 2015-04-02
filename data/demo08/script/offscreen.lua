function f (dest_x,dest_y)

text = string.format("offscreen coord = %d,%d",dest_x,dest_y)
print_text_debug(text)

player_id = player_get_id()
map = character_get_map(player_id)

map_w = character_get_map_w(player_id)
map_h = character_get_map_h(player_id)

map_x = map_attribute_get(map,"x")
map_y = map_attribute_get(map,"y")

new_map_x = map_x
new_map_y = map_y

player_x = dest_x
player_y = dest_y

if( tonumber(dest_x) < 0 ) then  
	new_map_x = map_x - 1 
	player_x = map_w-1
end
if( tonumber(dest_x) >= map_w ) then
	new_map_x = map_x + 1
	player_x = 0
end
if( tonumber(dest_y) < 0 ) then
	new_map_y = map_y - 1
	player_y = map_h-1
end
if( tonumber(dest_y) >= map_h ) then
	new_map_y = map_y + 1
	player_y = 0
end

map_name = string.format("M%d_%d",new_map_x,new_map_y)

res = character_set_pos(player_id,map_name,0,player_x,player_y)

if res == -1 then
        call_script("add_map.lua",map_name,new_map_x,new_map_y);
        res = character_set_pos(player_id,map_name,0,player_x,player_y)
end

return res

end
