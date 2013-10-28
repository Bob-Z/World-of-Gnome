id = player_get_id()

num_item = inventory_count(id,"apple")
if num_item >= 1 then
	item_id = inventory_get_by_name(id,"apple");
	if( item_id ~= nil ) then
		inventory_delete(id,item_id)
	end
	return
end

print_text_id(id, "No more apple to eat");
