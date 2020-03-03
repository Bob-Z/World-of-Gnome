/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2017-2020 carabobz@gmail.com

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

#include "Camera.h"

/******************************************************
 ******************************************************/
Camera::Camera() :
		m_Screen(Screen::SELECT), m_Zoom(0), m_X(0), m_Y(0)
{
}

/******************************************************
 ******************************************************/
Camera::~Camera()
{
}

/******************************************************
 ******************************************************/
Screen Camera::getScreen()
{
	return m_Screen;
}

/******************************************************
 ******************************************************/
void Camera::setScreen(Screen & screen)
{
	m_Screen = screen;
}

/******************************************************
 ******************************************************/
int Camera::getZoom()
{
	return m_Zoom;
}

/******************************************************
 ******************************************************/
void Camera::setZoom(int p_Zoom)
{
	m_Zoom = p_Zoom;
}

/******************************************************
 ******************************************************/
int Camera::getX()
{
	return m_X;
}

/******************************************************
 ******************************************************/
void Camera::setX(int p_X)
{
	m_X = p_X;
}

/******************************************************
 ******************************************************/
int Camera::getY()
{
	return m_Y;
}

/******************************************************
 ******************************************************/
void Camera::setY(int p_Y)
{
	m_Y = p_Y;
}
