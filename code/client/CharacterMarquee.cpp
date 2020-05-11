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

#include "CharacterMarquee.h"
#include "log.h"
#include "SdlItem.h"

/*****************************************************************************/
CharacterMarquee::CharacterMarquee() :
		m_id(), m_name(), m_type(), m_anim(), m_item(nullptr), m_width(0), m_height(), m_x(NO_COORD), m_y(NO_COORD)
{
}

/*****************************************************************************/
CharacterMarquee::~CharacterMarquee()
{
}

/*****************************************************************************/
const std::vector<Anim*>& CharacterMarquee::getAnim() const
{
	return m_anim;
}

/*****************************************************************************/
void CharacterMarquee::setAnim(const std::vector<Anim*>& anim)
{
	m_anim = anim;
}

/*****************************************************************************/
const std::string& CharacterMarquee::getId() const
{
	return m_id;
}

/*****************************************************************************/
void CharacterMarquee::setId(const std::string& id)
{
	m_id = id;
}

/*****************************************************************************/
SdlItem* CharacterMarquee::getItem() const
{
	return m_item;
}

/*****************************************************************************/
void CharacterMarquee::setItem(SdlItem* item)
{
	m_item = item;
}

/*****************************************************************************/
const std::string& CharacterMarquee::getName() const
{
	return m_name;
}

/*****************************************************************************/
void CharacterMarquee::setName(const std::string& name)
{
	m_name = name;
}

/*****************************************************************************/
const std::string& CharacterMarquee::getType() const
{
	return m_type;
}

/*****************************************************************************/
void CharacterMarquee::setType(const std::string& type)
{
	m_type = type;
}

/****************************************************************************/
int CharacterMarquee::getWidth() const
{
	return m_width;
}

/*****************************************************************************/
void CharacterMarquee::setWidth(int width)
{
	m_width = width;
}

/*****************************************************************************/
int CharacterMarquee::getHeight() const
{
	return m_height;
}

/*****************************************************************************/
void CharacterMarquee::setHeight(int height)
{
	m_height = height;
}

/*****************************************************************************/
int CharacterMarquee::getX() const
{
	return m_x;
}

/*****************************************************************************/
void CharacterMarquee::setX(int x)
{
	m_x = x;
}

/*****************************************************************************/
int CharacterMarquee::getY() const
{
	return m_y;
}

/*****************************************************************************/
void CharacterMarquee::setY(int y)
{
	m_y = y;
}
