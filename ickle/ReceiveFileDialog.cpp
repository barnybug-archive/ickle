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

#include "ReceiveFileDialog.h"

#include <gtkmm/box.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/fileselection.h>
#include <gtkmm/messagedialog.h>
#include <glibmm/convert.h>

#include <libicq2000/Client.h>
#include "main.h"
#include "Settings.h"
#include "sstream_fix.h"
#include "ickle.h"

#include <stdlib.h>
#include <iostream>

using std::ostringstream;
using std::endl;
using std::cerr;

ReceiveFileDialog::ReceiveFileDialog(Gtk::Window * parent, const ICQ2000::ContactRef& contact, FileTransferICQMessageEvent *ev)
  : Gtk::Dialog( _("File Transfer Request from "), parent),
    m_ok("Send Response"), m_close("Close"),
    m_accept("Accept", 0), m_refuse("Refuse", 0),
    m_contact(contact), m_refuse_label("Enter your refusal message:", 0),
    m_ev(ev->getEvent()),
    m_msg_label_frame("Request message"),
    m_filename_label_frame("Filenames to receive")
{
  // Default savepath is HOME
  m_ev->setSavePath( getenv("HOME") );

  //Get encoding from settings so we can convert to UTF-8
  std::string encoding = g_settings.getValueString( "encoding" ); 

  icqclient.filetransfer_update_signal.connect( this, &ReceiveFileDialog::icq_filetransfer_update_cb);

  ostringstream ostr;
  ostr << "File Transfer Request from "
	  << Glib::convert(contact->getNameAlias(), "utf8", encoding); 
  set_title(ostr.str());
  set_transient_for (*parent);

  m_ok.signal_clicked().connect( SigC::slot(*this,&ReceiveFileDialog::ok_cb));
  m_close.signal_clicked().connect( SigC::slot(*this,&ReceiveFileDialog::cancel_cb) );
  
  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing (10);

  Gtk::VBox *vbox2 = manage( new Gtk::VBox() );
  vbox2->set_spacing(5);


  
       
  //Adding label with frame for received message.
  m_msg_label.set_text( Glib::convert(ev->getMessage(), "utf8", encoding) );
  m_msg_label.set_justify(Gtk::JUSTIFY_FILL);
  m_msg_label.set_line_wrap(true);
  m_msg_label_frame.add(m_msg_label);
  vbox2->pack_start( m_msg_label_frame );

  //Adding label with frame for received filename.
  m_filename_label.set_text( Glib::convert(ev->getDescription(), "utf8", encoding) );
  m_filename_label.set_justify(Gtk::JUSTIFY_FILL);
  m_filename_label.set_line_wrap(true);
  m_filename_label_frame.add(m_filename_label);
  vbox2->pack_start( m_filename_label_frame );

  m_accept.signal_clicked().connect( SigC::slot( *this, &ReceiveFileDialog::accept_clicked_cb ) );
  vbox2->pack_start( m_accept );
  Gtk::RadioButton::Group group = m_accept.get_group();
  m_refuse.signal_clicked().connect( SigC::slot( *this, &ReceiveFileDialog::refuse_clicked_cb ) );
  m_refuse.set_group( group );
  vbox2->pack_start( m_refuse );

  // by default accept is selected - no message for accept	
  m_accept.set_active(true);
    
  //m_refuse_label.set_sensitive(false);
  vbox2->pack_start( m_refuse_label );

  //Add textview with scroll
  m_text.set_wrap_mode(Gtk::WRAP_WORD);
  m_text.set_editable(true);
  m_text.set_sensitive(false);
  m_text_scr_win.add(m_text);
  m_text_scr_win.set_shadow_type(Gtk::SHADOW_IN);
  m_text_scr_win.set_size_request(0, -1); // workaround for textview horizontal resizing
  m_text_scr_win.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
  vbox2->pack_start(m_text_scr_win, Gtk::PACK_EXPAND_WIDGET);


  //Change this so the xpm is loaded from pixmap/open.xpm
  //and so the labeltext can be changed.
  m_file.add_pixlabel("info.xpm", m_ev->getSavePath() );
  vbox2->pack_start( m_file );
  m_file.set_sensitive(true);
  m_file.signal_clicked().connect( SigC::slot(*this, &ReceiveFileDialog::file_cb) );

  vbox2->pack_start( *manage( new Gtk::Label(_("Current:"),  Gtk::ALIGN_LEFT)), Gtk::PACK_SHRINK);
  vbox2->pack_start(m_ProgressBar_file, true, true, 0 );
  vbox2->pack_start( *manage( new Gtk::Label(_("Total:"),  Gtk::ALIGN_LEFT)), Gtk::PACK_SHRINK);
  vbox2->pack_start(m_ProgressBar_batch, true, true, 0 );
  m_ProgressBar_file.set_text("0%");
  m_ProgressBar_batch.set_text("0%");
  vbox->pack_start(*vbox2);

  Gtk::HButtonBox *hbox = get_action_area();
  hbox->set_border_width(0);
  Gtk::HButtonBox *hbbox = manage( new Gtk::HButtonBox() );
  hbbox->pack_start(m_ok, true, true, 0);
  hbbox->pack_start(m_close, true, true, 0);
  hbox->pack_start( *hbbox );
  
  set_default_size(350,250);
  set_border_width(10);


  show_all();

  if (m_ev->getState() == ICQ2000::FileTransferEvent::CANCELLED) {
	cancelled("FileTransfer was cancelled by other side");
	m_text.set_sensitive(false);
	m_accept.set_sensitive(false);
	m_refuse.set_sensitive(false);
	m_ok.set_sensitive(false);
	m_file.set_sensitive(false);  
  }
}

void ReceiveFileDialog::icq_filetransfer_update_cb(ICQ2000::FileTransferEvent *ev)
{
  if (m_ev != ev) return;
  switch (ev->getState()) {
  case ICQ2000::FileTransferEvent::COMPLETE:
	  progressbar_update_cb();
	  if (m_connection_id_timeout)
		m_connection_id_timeout.disconnect();
	  cancelled("FileTransfer completed correctly");
	  break;
  case ICQ2000::FileTransferEvent::RECEIVE:
	  // Could come an speed change or an "new file" update
	  break;
  case ICQ2000::FileTransferEvent::ERROR:
	  if (m_connection_id_timeout)
	     m_connection_id_timeout.disconnect();
	  cancelled(ev->getError());
	  break;
  case ICQ2000::FileTransferEvent::CANCELLED:
	  if (m_connection_id_timeout)
	    m_connection_id_timeout.disconnect();
	  cancelled("File Transfer was cancelled by other side.");
	  m_text.set_sensitive(false);
	  m_accept.set_sensitive(false);
	  m_refuse.set_sensitive(false);
	  m_ok.set_sensitive(false);
	  m_file.set_sensitive(false);
	  break;
  case ICQ2000::FileTransferEvent::CLOSE:
	  if (m_connection_id_timeout)
	    m_connection_id_timeout.disconnect();
	  cancelled("File Transfer was cancelled.");
	  break;
  }
}

//Add timeout
bool ReceiveFileDialog::progressbar_update_cb() {
  double val1, val2;
  if (m_ev->getTotalSize() > 0) {
    val1 = double(m_ev->getPos())/m_ev->getSize();
    val2 = double(m_ev->getTotalPos())/m_ev->getTotalSize();
  } else {
    val1 = 0.0;
    val2 = 0.0;
  }
  
  m_ProgressBar_file.set_fraction(val1);
  m_ProgressBar_batch.set_fraction(val2);
  ostringstream ostr;
  ostr << int(val1*100) << "%";
  ostringstream ostr2;
  ostr2 << int(val2*100) << "%"; 
  m_ProgressBar_file.set_text(ostr.str());
  m_ProgressBar_batch.set_text(ostr2.str());

  // returns true so it will be called again.
  return true;
}

void ReceiveFileDialog::accept_clicked_cb()
{
  m_text.set_sensitive(false);
  m_file.set_sensitive(false);
  m_file.set_sensitive(true);
}

void ReceiveFileDialog::refuse_clicked_cb()
{
  m_text.set_sensitive(true);
  m_file.set_sensitive(true);
  m_file.set_sensitive(false);
}

void ReceiveFileDialog::ok_cb() {
  if (m_accept.get_active()) {
	  std::cerr << "ok_cb accept" << std::endl;
	m_ev->setState(ICQ2000::FileTransferEvent::ACCEPTED);
	//Start timeout for progressbar and update info.. (New file)  
	m_connection_id_timeout =
		Glib::signal_timeout().connect( SigC::slot(*this, &ReceiveFileDialog::progressbar_update_cb), 50 );

  } else {
	  std::cerr << "ok_cb reject" << std::endl;
	m_ev->setState(ICQ2000::FileTransferEvent::REJECTED);
  }
  m_text.set_sensitive(false);
  m_accept.set_sensitive(false);
  m_refuse.set_sensitive(false);
  m_ok.set_sensitive(false);
  m_file.set_sensitive(false);
  
  if (m_refuse.get_active())
    m_ev->setRefusalMessage( m_text.get_buffer()->get_text() );

  std::cerr << "ok_cb sendfiletransfer ack" << std::endl;  
  icqclient.SendFileTransferACK( m_ev );
  
}

void ReceiveFileDialog::file_cb() {
    Gtk::FileSelection fs_dialog("Select Download Directory");
    fs_dialog.set_transient_for(*this);
    
    //Prevent the user from selecting a file.
    fs_dialog.get_file_list()->get_parent()->hide(); 
    
    fs_dialog.set_filename(m_ev->getSavePath());
    int result = fs_dialog.run();
    //Handle the response:
    switch(result)
    {
    case(Gtk::RESPONSE_OK):
    {
	 std::cout << "Folder selected: " << fs_dialog.get_filename() << std::endl;
	 m_ev->setSavePath( fs_dialog.get_filename() );
	 break;
    }
    case(Gtk::RESPONSE_CANCEL):
    {
      std::cout << "Cancel clicked." << std::endl;
      break;
    }
    default:
    {
      std::cout << "Unexpected button clicked." << std::endl;
      break;
    }
  }
}


void ReceiveFileDialog::cancelled(std::string str)
{
  str = Glib::convert(str, "utf8", g_settings.getValueString( "encoding" )); 
  Gtk::MessageDialog dialog(*this, str);
  dialog.run();
}


void ReceiveFileDialog::cancel_cb(){
	if (m_ev->getState() == ICQ2000::FileTransferEvent::RECEIVE)
	   m_ev->setState(ICQ2000::FileTransferEvent::CLOSE);

	icqclient.CancelFileTransfer(m_ev);
	
	delete m_ev;
	delete this;
}


