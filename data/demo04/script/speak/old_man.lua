price_list = { sword = 200, shield = 150, bag = 25, magic = 500 }

function trade (player,item)
	gold = inventory_get_by_name(player,"gold")
	if gold == nil then
		text = string.format("You don't have gold, get out of here !")
		portrait = "portrait/old_man_angry.gif"
		return text,portrait
	end

	gold_qty = resource_get_quantity(gold)
	price = price_list[item]
	if gold_qty >= price then
		resource_set_quantity(gold,gold_qty-price)
		new_item = item_create_from_template(item)
		inventory_add(player,new_item)
		text = string.format("You got a new %s. What do you need ?",item)
		portrait = "portrait/old_man_happy.gif"
		return text,portrait
	end

	text = string.format("You don't have enough gold, get out !")
	portrait = "portrait/old_man_angry.gif"
	return text,portrait
end

function talk (npc,portrait,player,text)
	func_param = {}
	for k, v in pairs(price_list) do
		keyword = k
		price = v
		speech = string.format("item/%s.gif",keyword)
		table.insert(func_param, speech)
		speech = string.format("Buy a %s (%d gold)",keyword,price)
		table.insert(func_param, speech)
		table.insert(func_param, keyword)
	end

	speak_send(npc,portrait,player,text, unpack(func_param))
end

function f (npc,player,keyword)

	if keyword == nil then
		keyword = "start"
	end

	if keyword == "start" then
		text = string.format("Hello %s. What do you need ?",character_get_name(player))
		portrait = "portrait/old_man.gif"
	else
		text,portrait = trade(player,keyword)
	end

	talk(npc,portrait,player,text)
end
