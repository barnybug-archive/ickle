/*
 * Copyright (C) 2002 Dominic Sacré <bugcreator@gmx.de>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */
 
#ifndef CONTROLCOMMANDS_H
#define CONTROLCOMMANDS_H

enum CommandType
{
  CMD_SET_STATUS,
  CMD_GET_STATUS,
  CMD_SET_INVISIBLE,
  CMD_GET_INVISIBLE,
  CMD_SET_AWAY_MESSAGE,
  CMD_GET_AWAY_MESSAGE,
  CMD_ADD_CONTACT,
  CMD_SEND_MESSAGE,
  CMD_SET_SETTING,
  CMD_GET_SETTING,
  CMD_QUIT
};

enum CommandMessageType
{
 MESSAGE_Normal,
 MESSAGE_SMS
};

#endif
