function f ()

id = player_get_id()

map = character_get_map(id)
pos_x = character_get_x(id)
pos_y = character_get_y(id)
map_x = character_get_map_x(id)
map_y = character_get_map_y(id)

new_pos_x = pos_x
new_pos_y = pos_y + 1

if new_pos_y >= map_y then
        return
end

character_set_pos(id,map,new_pos_x,new_pos_y)

end
