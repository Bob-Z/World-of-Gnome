price_list = { sword = 200, shield = 150, bag = 25, magic = 500 }

player_portrait = nil;

function set_player_portrait(player_id,mood)
		player_type = character_get_type(player_id)
		player_portrait = string.format("portrait/%s_%s.gif",player_type,mood)
end

function trade (player_id,item)
	gold = inventory_get_by_name(player_id,"gold")
	if gold == nil then
		text = string.format("You don't have gold, get out of here !")
		portrait = "portrait/old_man_angry.gif"

		set_player_portrait(player_id,"angry")

		return text,portrait
	end

	gold_qty = resource_get_quantity(gold)
	price = price_list[item]
	if gold_qty >= price then
		resource_set_quantity(gold,gold_qty-price)
		new_item = item_create_from_template(item)
		inventory_add(player_id,new_item)
		text = string.format("You got a new %s. What do you need ?",item)
		portrait = "portrait/old_man_happy.gif"
		set_player_portrait(player_id,"happy")
		return text,portrait
	end

	text = string.format("You don't have enough gold, get out !")
	portrait = "portrait/old_man_angry.gif"
	set_player_portrait(player_id,"angry")
	return text,portrait
end

function talk (portrait,player_id,text)
	table.insert(element,"image");
	table.insert(element,portrait);
	table.insert(element,"text");
	table.insert(element,text);
	table.insert(element,"eol");

	table.insert(element,"image");
	table.insert(element,player_portrait);
	gold_id = inventory_get_by_name(player_id,"gold")
	if gold_id then
		gold = resource_get_quantity(gold_id)
	else
		gold = 0
	end
	text = string.format("You have %d gold.",gold)
	table.insert(element,"text");
	table.insert(element,text);
	table.insert(element,"eol");

	for k, v in pairs(price_list) do
		keyword = k
		price = v
		speech = string.format("item/%s.gif",keyword)
		table.insert(element,"image")
		table.insert(element,speech)
		speech = string.format("Buy a %s (%d gold)",keyword,price)
		table.insert(element,"action")
		table.insert(element,"speak/old_man")
		table.insert(element,keyword)
		table.insert(element,"text")
		table.insert(element,speech)
		table.insert(element,"eol")
	end
	table.insert(element,"eop")
	table.insert(element,"action")
	table.insert(element,"popup_end")
	table.insert(element,"")
	table.insert(element,"text")
	table.insert(element,"Bye !")

	popup_send(player_id,unpack(element))
end

function f (keyword)
	element = {}
	player_id = player_get_id()
	player_portrait = character_get_portrait(player_id)

	if keyword == nil then
		keyword = "start"
	end

	if keyword == "start" then
		text = string.format("Hello %s. What do you need ?",character_get_name(player_id))
		portrait = "portrait/old_man.gif"
	else
		text,portrait = trade(player_id,keyword)
	end

	talk(portrait,player_id,text)
end
