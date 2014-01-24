id = player_get_id()

map = character_get_map(id)
pos_x = character_get_x(id)
pos_y = character_get_y(id)

new_pos_x = pos_x
new_pos_y = pos_y
rand = math.random(-1,1)
if math.random(0,1) == 0 then
	new_pos_x = pos_x + rand
else
	new_pos_y = pos_y + rand
end

character_set_pos(id,map,new_pos_x,new_pos_y)

-- return the time in ms before the next NPC AI action
return 1000

