function f ()

id = player_get_id()

item_id = inventory_get_by_name(id,"apple")
if( item_id ~= nil ) then
	num_item = item_get_quantity(item_id)

	if num_item >= 1 then
		item_set_quantity(item_id,num_item-1)
		return
	end
end

print_text_id(id, "No more apple to eat")

end
