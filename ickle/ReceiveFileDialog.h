/*
 * ReceiveFileDialog
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

#ifndef RECEIVEFILEDIALOG_H
#define RECEIVEFILEDIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/frame.h>

#include <libicq2000/events.h>
#include <libicq2000/Contact.h>

#include <MessageEvent.h>

class ReceiveFileDialog : public Gtk::Dialog,
	public sigslot::has_slots<>
{
 private:
  ICQ2000::ContactRef m_contact;
  ICQ2000::FileTransferEvent *m_ev;

  Gtk::Button m_ok, m_close, m_file;
  Gtk::RadioButton m_accept, m_refuse;
  Gtk::Label m_msg_label, m_filename_label, m_refuse_label;
  Gtk::Frame m_msg_label_frame, m_filename_label_frame;
  Gtk::TextView m_text;
  Gtk::ScrolledWindow m_text_scr_win;
  Gtk::ProgressBar m_progressbar_file, m_progressbar_batch;  
  SigC::Connection m_connection_id_timeout;
  
  bool m_recvcancel;
  
  void accept_clicked_cb();
  void refuse_clicked_cb();

  void send_event();
  
  void cancelled(const Glib::ustring& str);

  void icq_filetransfer_update_cb(ICQ2000::FileTransferEvent *ev);

  void ok_cb();
  void cancel_cb();
  void file_cb();
  bool progressbar_update_cb();
  
 public:
  ReceiveFileDialog(Gtk::Window * parent, const ICQ2000::ContactRef& contact, FileTransferICQMessageEvent *ev);
  
};

#endif
