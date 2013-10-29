id = player_get_id();

target = character_get_selected_character_id(id)
if target == nil then
	print_text_id(id, "Select a target character")
	return
end

target_agility = character_attribute_get(target,"agility")
player = player_get_id()
player_agility = character_attribute_get(player,"agility")

hit = math.random(target_agility+player_agility);

if hit <= player_agility then
	print_text_id(id, "hit !")
	character_attribute_change(target,"life",-1)
else
	print_text_id(id, "missed")
end
