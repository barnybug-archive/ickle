/*
 * UserInfoDialog
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "UserInfoDialog.h"

#include "sstream_fix.h"
#include "main.h"

#include <gtkmm/buttonbox.h>

#include <libicq2000/Client.h>
#include <libicq2000/socket.h>
#include <libicq2000/userinfohelpers.h>

#include <vector>

using std::string;
using std::ostringstream;
using std::vector;

using namespace ICQ2000;

UserInfoDialog::UserInfoDialog(Gtk::Window& parent, const ContactRef& c, bool self)
  : Gtk::Dialog(), m_self(self)
{
  //  set_transient_for(parent);

  ostringstream ostr;
  if (m_self)
  {
    ostr << "My User Info";
  }
  else
  {
    ostr << "User Info - " << c->getAlias() << " (";
    if (c->isICQContact()) {
      ostr << c->getUIN();
    } else {
      ostr << c->getMobileNo();
    }
    ostr << ")";
  }
  
  //  set_title(ostr.str());

}

UserInfoDialog::~UserInfoDialog()
{
  m_signal_destroy.emit();
}

SigC::Signal0<void>& UserInfoDialog::signal_destroy()
{
  return m_signal_destroy;
}

SigC::Signal0<void>& UserInfoDialog::signal_fetch()
{
  return m_signal_fetch;
}

SigC::Signal0<void>& UserInfoDialog::signal_upload()
{
  return m_signal_upload;
}
