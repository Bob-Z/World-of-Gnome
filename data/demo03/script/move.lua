function f (move_x,move_y)

id = player_get_id()

map = character_get_map(id)
tile_x = character_get_x(id)
tile_y = character_get_y(id)
map_w = character_get_map_w(map)
map_h = character_get_map_h(map)

new_tile_x = tile_x + move_x
new_tile_y = tile_y + move_y

if new_tile_x < 0 then
        return
end
if new_tile_x >= map_w then
        return
end
if new_tile_y < 0 then
        return
end
if new_tile_y >= map_h then
        return
end

character_set_pos(id,map,new_tile_x,new_tile_y)

--Get all item on the current tile
found_item = {map_get_item(map,0,new_tile_x,new_tile_y)}
i=1
while found_item[i] ~= nil do
	map_delete_item(map,0,new_tile_x,new_tile_y)
	inventory_add(id,found_item[i])
	i = i+1
end

end
