/*
 * SendAuthReqDialog
 * Copyright (C) 2002 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#ifndef SENDFILEDIALOG_H
#define SENDFILEDIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/entry.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/statusbar.h>

#include <libicq2000/events.h>
#include <libicq2000/Contact.h>


class SendFileDialog : public Gtk::Dialog,
		       public sigslot::has_slots<>
{
 private:
  ICQ2000::ContactRef m_contact;
  Gtk::Button m_send, m_close;
  Gtk::ScrolledWindow m_msg_scr_win;
  Gtk::TextView m_msg_text;
  Gtk::Entry m_filename;
  Gtk::Button m_fileselectbutton;
  Gtk::ProgressBar m_progressbar_file, m_progressbar_batch;
  Gtk::Statusbar m_statusbar;
  ICQ2000::FileTransferEvent *m_ev;
    
  unsigned int m_speed;
  
  SigC::Connection m_connection_id_timeout, m_respons_id_timeout;
  void on_button_file_cb();
  void cancelled(const Glib::ustring& str);
  bool respons_timeout_cb();
  
 protected:
  virtual void on_response(int response_id);
  void icq_filetransfer_update_cb(ICQ2000::FileTransferEvent *ev);
  bool progressbar_update_cb();
  
 public:
  SendFileDialog(Gtk::Window& parent, const ICQ2000::ContactRef& contact);
};

#endif
