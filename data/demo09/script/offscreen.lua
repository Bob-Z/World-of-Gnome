function f (dest_x,dest_y)

text = string.format("offscreen coord = %d,%d",dest_x,dest_y)
print_text_debug(text)

player_id = player_get_id()
map = character_get_map(player_id)

map_w = character_get_map_w(map)
map_h = character_get_map_h(map)

map_x = map_attribute_get(map,"x")
map_y = map_attribute_get(map,"y")

new_map_x = map_x
new_map_y = map_y

player_x = tonumber(dest_x)
player_y = tonumber(dest_y)

-- Get new map coordinates
if( tonumber(dest_x) < 0 ) then  
	new_map_x = map_x - 1 
end
if( tonumber(dest_x) >= map_w ) then
	new_map_x = map_x + 1
end
if( tonumber(dest_y) < 0 ) then
	new_map_y = map_y - 1
end
if( tonumber(dest_y) >= map_h ) then
	new_map_y = map_y + 1
end

new_map = string.format("M%d_%d",new_map_x,new_map_y)
new_map_w = character_get_map_w(new_map)
new_map_h = character_get_map_h(new_map)

-- Set player coordinates in new map
if( tonumber(dest_x) < 0 ) then  
	player_x = new_map_w-1
end
if( tonumber(dest_x) >= map_w ) then
	player_x = 0
end
if( tonumber(dest_y) < 0 ) then
	player_y = new_map_h-1
end
if( tonumber(dest_y) >= map_h ) then
	player_y = 0
end

-- Avoid player being outside a map
if( player_x >= new_map_w ) then
	player_x = new_map_w -1
end
if( player_y >= new_map_h ) then
	player_y = new_map_h -1
end

res = character_set_pos(player_id,new_map,player_x,player_y)

if res == -1 then
	-- Only human can create map
	player_type = character_get_type(player_id)
	if player_type ~= "human" then
		return -1
	end
	call_script("add_map.lua",new_map,new_map_x,new_map_y);
        res = character_set_pos(player_id,new_map,player_x,player_y)

        text = string.format("Select new map's default tile ?")
        popup_send(player_id,
                "action","set_map_grass","",
                "image","tile/grass1.png",
                "action","set_map_grass","",
                "text","Green grass",

		"eol",

                "action","set_map_water","",
                "image","tile/water1.png",
                "action","set_map_water","",
                "text","Water",

		"eol",

--                "action","set_map","grass",
--                "image","tile/grass1.png",
--                "action","set_map","grass",
--                "text","Green grass",

--		"eol",

		"eop",

                "action", "popup_end", "",
                "text","Done")



	res = 0
end

return res

end
