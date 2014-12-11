function f (keyword)

player = player_get_id()

player_portrait = character_get_portrait(player)
player_name = character_get_name(player)

if keyword == nil then
        keyword = "start"
end

if keyword == "start" then
        popup_send(player,
                "text","Hello",
                "eop",
                "image","icon/greendot.gif",
                "action","speak/gob","how",
                "text","How are you ?",
		"eol",
                "image","icon/greendot.gif",
                "action","popup_end","bye",
                "text","Goodbye")
end

if keyword == "how" then
        popup_send(player,
                "text","Fine thank you",
                "eop",
                "image","icon/greendot.gif",
                "action","popup_end","bye",
                "text","Goodbye")
end

end
