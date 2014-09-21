function f ()

id = player_get_id()
map = character_get_map(id)
x = character_get_x(id)
y = character_get_y(id)
tile = map_get_tile(map,x,y,0)

if tile == "tile/black.jpg" then
	item_id = inventory_get_by_name(id,"apple")
	if( item_id ~= nil ) then
		num_item = item_get_quantity(item_id)
		item_set_quantity(item_id,num_item+1)
	end
	return
end

print_text_id(id, "You must be on a black tile to get an apple")

end
