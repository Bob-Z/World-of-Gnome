function f (map_name,map_x,map_y)

player_id = player_get_id()
map = character_get_map(player_id)

map_w = character_get_map_w(map)
map_h = character_get_map_h(map)

map_new(map_name,map_w,map_h,72,72)
map_add_layer(map_name,0,-1,-1,-1,-1,"tile/gr1.png","fl")

map_set_offscreen(map_name,"offscreen.lua")

map_set_custom_column(map_name,-1,0,54,-36)
map_set_custom_column(map_name,-1,1,54,36)

map_attribute_set(map_name,"x",map_x)
map_attribute_set(map_name,"y",map_y)

end
