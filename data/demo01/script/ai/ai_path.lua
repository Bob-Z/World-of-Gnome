function f ()

x = { 1,2,3,3,3,2,1,1 }
y = { 1,1,1,2,3,3,3,2 }
num_move = 8

id = player_get_id()

index = character_attribute_get(id,"path_index")

if index == -1 then
	index = 1
end

index = index+1
if index > num_move then
	index = 1
end

map = character_get_map(id)
tile_x = character_get_x(id)
tile_y = character_get_y(id)

character_set_pos(id,map,x[index],y[index])

character_attribute_set(id,"path_index",index)

-- return the time in ms before the next NPC AI action
return 1000
end
