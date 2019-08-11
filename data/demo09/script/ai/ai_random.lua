function f (timeout)

id = player_get_id()

map = character_get_map(id)
tile_x = character_get_x(id)
tile_y = character_get_y(id)

new_tile_x = tile_x
new_tile_y = tile_y
rand = math.random(-1,1)
if math.random(0,1) == 0 then
	new_tile_x = tile_x + rand
else
	new_tile_y = tile_y + rand
end

character_set_pos(id,map,new_tile_x,new_tile_y)

-- return the time in ms before the next NPC AI action
return timeout
end
