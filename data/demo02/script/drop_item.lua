function f ()

id = player_get_id()

item = character_get_selected_inventory_id(id)
if item == nil or item == "" then
	print_text_id(id, "Please select an item to drop")
	return
end

if inventory_delete(id,item) == -1 then
	return
end

map = character_get_map(id)
x = character_get_x(id)
y = character_get_y(id)
map_add_item(map,0,item,x,y)

end
