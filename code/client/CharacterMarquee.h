/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2020 carabobz@gmail.com

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#ifndef CLIENT_CHARACTERMARQUEE_H_
#define CLIENT_CHARACTERMARQUEE_H_

#include <limits>
#include <string>
#include <vector>

class Anim;
class SdlItem;

class CharacterMarquee
{
public:
	static constexpr int NO_COORD = std::numeric_limits<int>::max();

	CharacterMarquee();
	virtual ~CharacterMarquee();

	const std::vector<Anim*>& getAnim() const;
	void setAnim(const std::vector<Anim*>& anim);

	const std::string& getId() const;
	void setId(const std::string& id);

	SdlItem* getItem() const;
	void setItem(SdlItem* item);

	const std::string& getName() const;
	void setName(const std::string& name);

	const std::string& getType() const;
	void setType(const std::string& type);

	int getWidth() const;
	void setWidth(int width);

	int getHeight() const;
	void setHeight(int height);

	int getX() const;
	void setX(int x);

	int getY() const;
	void setY(int y);

private:
	std::string m_id;
	std::string m_name;
	std::string m_type;
	std::vector<Anim *> m_anim;
	SdlItem * m_item;
	int m_width;
	int m_height;
	int m_x;
	int m_y;
};

#endif /* CLIENT_CHARACTERMARQUEE_H_ */
