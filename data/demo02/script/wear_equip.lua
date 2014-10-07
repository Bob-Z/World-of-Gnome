function f ()

id = player_get_id()

slot = character_get_selected_equipment_slot(id)
if slot == nil or slot == "" then
	print_text_id(id, "Please, select an equipment slot to equip")
	return
end

item = character_get_selected_inventory_id(id)
if item == nil or item == "" then
	print_text_id(id, "Please, select an item to equip")
	return
end

--remove previously equipped item
to_add_to_inventory = equipment_slot_get_item(id,slot)
if to_add_to_inventory ~= nil and to_add_to_inventory ~= "" then
	if equipment_slot_set_item(id,slot,"") == -1 then
		text = string.format("Cannot remove %s from your %s",to_add_to_inventory,slot)
	end
	inventory_add(id,to_add_to_inventory)
end

-- put item from inventory to equipment
if inventory_delete(id,item) ~= 1 then
	equipment_slot_set_item(id,slot,item)
end

end
