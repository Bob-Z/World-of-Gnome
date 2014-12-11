function f (target_id, aggro_on)

if( aggro_on == "0" ) then
	return
end

if( character_get_npc(target_id) == 1 ) then
	return
end

call_action("speak/oracle","start",target_id)

end
