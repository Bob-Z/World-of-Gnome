function f ()

id = player_get_id()

map = character_get_map(id)
pos_x = character_get_x(id)
pos_y = character_get_y(id)
map_w = character_get_map_w(id)
map_h = character_get_map_h(id)

new_pos_x = pos_x
new_pos_y = pos_y - 1

if new_pos_y < 0 then
        return
end

character_set_pos(id,map,new_pos_x,new_pos_y)

--Get all item on the current tile
found_item = {map_get_item(map,new_pos_x,new_pos_y)}
i=1
while found_item[i] ~= nil do
        map_delete_item(map,new_pos_x,new_pos_y)
        inventory_add(id,found_item[i])
        i = i+1
end

end
