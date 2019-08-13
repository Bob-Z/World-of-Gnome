function f ()

player = player_get_id()

castle_layer = 17
player_layer = 15
floor_layer = 11

selected_tx = character_get_selected_map_tile_x(player)
selected_ty = character_get_selected_map_tile_y(player)

if selected_tx == -1 or selected_ty == -1 then
        print_text_id(player, "You must choose a tile")
        return
end

map = character_get_map(player)

-- Create exterior
id = character_create_from_template("castle",map,castle_layer,selected_tx,selected_ty)
if id == nil then
        text = string.format("Cannot create %s here",part)
        print_text_id(player, text)
        return
end
character_set_npc(id,1)

-- Exterior tile type
for map_tx = selected_tx-6, selected_tx+6 do
        for map_ty = selected_ty-3, selected_ty+6 do
                map_set_tile_type(map,player_layer,"w",map_tx,map_ty)
        end
end

-- Door
for door_tx = selected_tx-6, selected_tx-2 do
	map_set_tile_type(map,player_layer,"w",door_tx,selected_ty+7)
end
for door_tx = selected_tx+2, selected_tx+6 do
	map_set_tile_type(map,player_layer,"w",door_tx,selected_ty+7)
end

-- Stairs
for stairs_tx = selected_tx-6, selected_tx-4 do
	map_set_tile_type(map,player_layer,"w",stairs_tx,selected_ty+8)
end
for stairs_tx = selected_tx+4, selected_tx+6 do
	map_set_tile_type(map,player_layer,"w",stairs_tx,selected_ty+8)
end

-- Interior
new_map = map_new("",32,32,32,32)
map_add_layer(new_map,floor_layer,32,32,32,32,"tile/castle_floor01.png","f")
map_set_tile(new_map,floor_layer,"tile/castle_floor02.png",15,31)
map_set_tile(new_map,floor_layer,"tile/castle_floor02.png",16,31)

-- Exterior event (door)
event = map_add_event(map,player_layer,"goto.lua",selected_tx-1,selected_ty+7)
map_add_event_param(map,player_layer,event,new_map)
map_add_event_param(map,player_layer,event,15)
map_add_event_param(map,player_layer,event,30)

event = map_add_event(map,player_layer,"goto.lua",selected_tx+0,selected_ty+7)
map_add_event_param(map,player_layer,event,new_map)
map_add_event_param(map,player_layer,event,15)
map_add_event_param(map,player_layer,event,30)

event = map_add_event(map,player_layer,"goto.lua",selected_tx+1,selected_ty+7)
map_add_event_param(map,player_layer,event,new_map)
map_add_event_param(map,player_layer,event,16)
map_add_event_param(map,player_layer,event,30)

-- Interior event
event = map_add_event(new_map,player_layer,"goto.lua",15,31)
map_add_event_param(new_map,player_layer,event,map)
map_add_event_param(new_map,player_layer,event,selected_tx+0)
map_add_event_param(new_map,player_layer,event,selected_ty+8)

event = map_add_event(new_map,player_layer,"goto.lua",16,31)
map_add_event_param(new_map,player_layer,event,map)
map_add_event_param(new_map,player_layer,event,selected_tx+0)
map_add_event_param(new_map,player_layer,event,selected_ty+8)

end

