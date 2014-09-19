function f ()

id = player_get_id()
map = character_get_map(id)
x = character_get_x(id)
y = character_get_y(id)
tile = map_get_tile(map,x,y,0)

if tile == "tile/black.jpg" then
	new_item_id = item_create_from_template("apple")
	inventory_add(id,new_item_id)
	return
end

print_text_id(id, "You are not on a black tile")

end
