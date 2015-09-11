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

res = character_set_pos(player_id,map_name,player_x,player_y)

if res == -1 then
	-- Only shyde can create map
	player_type = character_get_type(player_id)
	if player_type ~= "shyde" then
		return -1
	end
	call_script("add_map.lua",map_name,new_map_x,new_map_y);
        res = character_set_pos(player_id,map_name,player_x,player_y)

        text = string.format("Select new map's default tile ?")
        popup_send(player_id,
                "action","set_map","gr",
                "image","tile/gr1.png",
                "action","set_map","gr",
                "text","Green grass",

		"eol",

                "action","set_map","dr",
                "image","tile/dr1.png",
                "action","set_map","dr",
                "text","Dry grass",

		"eol",

                "action","set_map","le",
                "image","tile/le1.png",
                "action","set_map","le",
                "text","Leaf litter",

		"eol",

                "action","set_map","se",
                "image","tile/se1.png",
                "action","set_map","se",
                "text","Semi-dry",

		"eol",

                "action","set_map","ea",
                "image","tile/ea1.png",
                "action","set_map","ea",
                "text","Earthy floor",

		"eol",

                "action","set_map","fla",
                "image","tile/fla1.png",
                "action","set_map","fla",
                "text","Flagstones dark",

		"eol",

                "action","set_map","flo",
                "image","tile/flo1.png",
                "action","set_map","flo",
                "text","Floor",

		"eol",

                "action","set_map","hil",
                "image","tile/hil1.png",
                "action","set_map","hil",
                "text","Hills variation",

		"eol",

                "action","set_map","pa",
                "image","tile/pa1.png",
                "action","set_map","pa",
                "text","Path",

		"eol",

                "action","set_map","oc.gif",
                "image","tile/oc.gif",
                "action","set_map","oc.gif",
                "text","Ocean",

		"eol",

                "action","set_map","co.gif",
                "image","tile/co.gif",
                "action","set_map","co.gif",
                "text","Coast tropical water",

		"eol",

                "action","set_map","wa.gif",
                "image","tile/wa.gif",
                "action","set_map","wa.gif",
                "text","Water",

		"eop",

                "action", "popup_end", "",
                "text","Done")



	res = 0
end

return res

end
