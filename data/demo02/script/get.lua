function f ()

id = player_get_id()
map = character_get_map(id)
x = character_get_x(id)
y = character_get_y(id)
item = map_delete_item(map,0,x,y)
if item == nil then
	print_text_id(id, "No item here")
	return
end

inventory_add(id,item)

end
