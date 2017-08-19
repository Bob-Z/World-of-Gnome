function f ()

player = player_get_id()

castle_image_up = "scenery/castle_up.png"
castle_image_down = "scenery/castle_down.png"
castle_layer_up = 17
castle_layer_down = 13
player_layer = 15
floor_layer = 11

x = character_get_selected_map_tile_x(player)
y = character_get_selected_map_tile_y(player)
if x == -1 or y == -1 then
        print_text_id(player, "You must choose a tile")
        return
end

map = character_get_map(player)

-- Get coordinate in pixel
px = tile_get_x(map,1,x,y)
py = tile_get_y(map,1,x,y)

map_add_scenery(map,castle_layer_up,px,py,castle_image_up)
map_add_scenery(map,castle_layer_down,px,py+350,castle_image_down)

-- create castle interior
new_map = map_new("",32,32,32,32)
map_add_layer(new_map,floor_layer,32,32,32,32,"tile/castle_floor01.png","f")
map_set_tile(new_map,floor_layer,"tile/castle_floor02.png",15,31)
map_set_tile(new_map,floor_layer,"tile/castle_floor02.png",16,31)

-- set tile type
for tx=x, x+12 do
        for ty=y+9, y+14 do
                map_set_tile_type(map,player_layer,"w",tx,ty)
        end
end
-- castle entry
for tx=x, x+4 do
	map_set_tile_type(map,player_layer,"w",tx,y+15)
end
for tx=x+8, x+12 do
	map_set_tile_type(map,player_layer,"w",tx,y+15)
end
for tx=x, x+3 do
	map_set_tile_type(map,player_layer,"w",tx,y+16)
end
for tx=x+9, x+12 do
	map_set_tile_type(map,player_layer,"w",tx,y+16)
end

-- Exterior event
event = map_add_event(map,player_layer,"goto.lua",x+5,y+15)
map_add_event_param(map,player_layer,event,new_map)
map_add_event_param(map,player_layer,event,15)
map_add_event_param(map,player_layer,event,30)

event = map_add_event(map,player_layer,"goto.lua",x+6,y+15)
map_add_event_param(map,player_layer,event,new_map)
map_add_event_param(map,player_layer,event,15)
map_add_event_param(map,player_layer,event,30)

event = map_add_event(map,player_layer,"goto.lua",x+7,y+15)
map_add_event_param(map,player_layer,event,new_map)
map_add_event_param(map,player_layer,event,16)
map_add_event_param(map,player_layer,event,30)

-- Interior event
event = map_add_event(new_map,player_layer,"goto.lua",15,31)
map_add_event_param(new_map,player_layer,event,map)
map_add_event_param(new_map,player_layer,event,x+6)
map_add_event_param(new_map,player_layer,event,y+16)

event = map_add_event(new_map,player_layer,"goto.lua",16,31)
map_add_event_param(new_map,player_layer,event,map)
map_add_event_param(new_map,player_layer,event,x+6)
map_add_event_param(new_map,player_layer,event,y+16)

end

