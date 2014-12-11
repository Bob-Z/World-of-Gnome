function f ()

player_id = player_get_id()

listner_id = character_get_selected_character_id(player_id)
listner_type = character_get_type(listner_id)

action = string.format("speak/%s",listner_type)

call_action(action)

end
