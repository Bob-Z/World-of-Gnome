function f ()

id = player_get_id()

target = character_get_selected_character_id(id)
if target == nil then
        print_text_id(id, "Select a target character")
        return
end

character_attribute_change(target,"life",-1)

end
