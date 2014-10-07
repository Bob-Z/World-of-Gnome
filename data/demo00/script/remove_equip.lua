function f ()

id = player_get_id()

slot = character_get_selected_equipment_slot(id)
if slot == nil or slot == "" then
	print_text_id(id, "Please, select an equipment to remove")
	return
end
item = equipment_slot_get_item(id,slot)

if equipment_slot_set_item(id,slot,"") ~= -1 then
	inventory_add(id,item)
end

end
