function f ()

id = player_get_id()

target = character_get_selected_character_id(id)
character_attribute_change(target,"life",-1)

end
