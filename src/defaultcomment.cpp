/*

    Copyright (C) 2003 Florian Schmitt, Science and Computing AG
                       f.schmitt@science-computing.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "defaultcomment.h"

DefaultComment::DefaultComment()
{
  m_text="";
  m_microaccount=false;
}

DefaultComment::DefaultComment(const QString& text, bool microaccount)
{
  m_text=text;
  m_microaccount=microaccount;
}

bool DefaultComment::isMicroAccount()
{
  return m_microaccount;
}

QString DefaultComment::getText()
{
  return m_text;
}
