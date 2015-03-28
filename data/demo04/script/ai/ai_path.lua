function f ()

x = { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 }
y = { 18,19,20,21,22,23,24,24,24,24,24,23,22,21,20,19,18,18,18,18 }
num_move = 20

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

character_set_pos(id,map,0,x[index],y[index])

character_attribute_set(id,"path_index",index)

-- return the time in ms before the next NPC AI action
return 1000

end
