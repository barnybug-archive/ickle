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

#include "SendFileDialog.h"

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>
#include <gtkmm/fileselection.h>
#include <gtkmm/messagedialog.h>

#include <libicq2000/Client.h>

#include "ickle.h"
#include "main.h"
#include "Settings.h"
#include "ucompose.h"

#include "Icons.h"

SendFileDialog::SendFileDialog(Gtk::Window& parent, const ICQ2000::ContactRef& contact)
  : Gtk::Dialog(),
    m_contact(contact),
    m_ev(NULL),
    m_send( _("Send") ), m_close( _("Close") )
{
  icqclient.filetransfer_update_signal.connect( this, &SendFileDialog::icq_filetransfer_update_cb);

  set_title( String::ucompose( _("Send file to %1"), Glib::ustring(m_contact->getNameAlias())) );
  set_transient_for(parent);

  Gtk::VBox *vbox = get_vbox();
  vbox->set_spacing(10);
  
  Gtk::VBox *vbox2 = manage(new Gtk::VBox());
  vbox2->pack_start( *manage(new Gtk::Label( _("Enter your message:"), 0)) );

  m_msg_text.set_wrap_mode(Gtk::WRAP_WORD);
  m_msg_text.set_editable(true);
  m_msg_scr_win.add(m_msg_text);
  m_msg_scr_win.set_shadow_type(Gtk::SHADOW_IN);
  m_msg_scr_win.set_size_request(0, -1); // workaround for textview horizontal resizing
  m_msg_scr_win.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
  vbox2->pack_start(m_msg_scr_win, Gtk::PACK_EXPAND_WIDGET);
  

  vbox2->pack_start( *manage(new Gtk::Label( _("Enter your filename:"), 0)), true, true, 0);
  m_filename.set_editable(true);
  
  //Adding filename Entry and fileselection button
  Gtk::HBox *hbox = manage(new Gtk::HBox()); 
  hbox->pack_start( m_filename , true, true, 0);
  m_fileselectbutton.add(*manage(new Gtk::Image( g_icons.get_icon_for_event( ICQMessageEvent::FileTransfer ) )));
  
  hbox->pack_start( m_fileselectbutton , Gtk::PACK_SHRINK);
  vbox2->pack_start( *hbox, true, true, 10 );
  m_fileselectbutton.signal_clicked().connect( SigC::slot(*this, &SendFileDialog::on_button_file_cb) );

  vbox2->pack_start( *manage( new Gtk::Label(_("Current:"), Gtk::ALIGN_LEFT)), Gtk::PACK_SHRINK);
  vbox2->pack_start( m_progressbar_file, true, true, 0 );
  vbox2->pack_start( *manage( new Gtk::Label(_("Total:"), Gtk::ALIGN_LEFT )), Gtk::PACK_SHRINK);
  vbox2->pack_start( m_progressbar_batch, true, true, 0 );
  m_progressbar_file.set_text( String::ucompose( _("%1%%"), 0 ) );
  m_progressbar_batch.set_text( String::ucompose( _("%1%%"), 0 ) );
  vbox->pack_start (*vbox2, true, true);

  m_send.signal_clicked().connect( SigC::bind<int>(SigC::slot(*this,&Gtk::Dialog::response) , Gtk::RESPONSE_OK));
  m_close.signal_clicked().connect( SigC::bind<int>(SigC::slot(*this,&Gtk::Dialog::response), Gtk::RESPONSE_CLOSE));
  
  Gtk::HButtonBox *hbbox = get_action_area();
  hbbox->set_border_width(0);
  Gtk::HButtonBox *hbbox2 = manage( new Gtk::HButtonBox() );
  hbbox2->pack_start(m_send, true, true, 0);
  hbbox2->pack_start(m_close, true, true, 0);
  hbbox->pack_start( *hbbox2 );
  
  set_default_size(350,250);
  set_border_width(10);
  show_all();

}

void SendFileDialog::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
  {
    m_msg_text.set_sensitive(false);
    m_filename.set_sensitive(false);
    m_fileselectbutton.set_sensitive(false);
    m_send.set_sensitive(false);
    
    m_ev = new ICQ2000::FileTransferEvent(m_contact,
					  m_msg_text.get_buffer()->get_text(),
					  m_filename.get_text(),100, 0);
    m_speed = 100;
        
    icqclient.SendFileTransfer(m_ev);
  }
  else
  {
    if (m_ev != NULL)
    {
      if (m_ev->getState() == ICQ2000::FileTransferEvent::SEND)
      {
	m_ev->setState(ICQ2000::FileTransferEvent::CLOSE);
	icqclient.CancelFileTransfer(m_ev);
      }
      else if (m_ev->getState() == ICQ2000::FileTransferEvent::WAIT_RESPONS)
      {
	icqclient.CancelFileTransfer(m_ev);
      }
    }
	  
    if (m_ev != NULL)
      delete m_ev;

    m_ev = NULL;
    delete this;
  }
}

void SendFileDialog::on_button_file_cb()
{
  Gtk::FileSelection fs_dialog( _("Please choose a file") );
  fs_dialog.set_transient_for(*this);
  
  if (!m_filename.get_text().empty())
    fs_dialog.set_filename(m_filename.get_text());
  
  int result = fs_dialog.run();

  // Handle the response:
  switch(result)
  {
  case(Gtk::RESPONSE_OK):
  {
    std::string filename = fs_dialog.get_filename();
    m_filename.set_text(filename);
    break;
  }
  case(Gtk::RESPONSE_CANCEL):
  {
    break;
  }
  default:
  {
    break;
  }
  }    
}

void SendFileDialog::icq_filetransfer_update_cb(ICQ2000::FileTransferEvent *ev)
{
  if (m_ev != ev) return;
  
  switch (ev->getState())
  {
  case ICQ2000::FileTransferEvent::NOT_CONNECTED:
    break; 
  case ICQ2000::FileTransferEvent::ACCEPTED:
    ev->setState(ICQ2000::FileTransferEvent::SEND);
    //Start timeout for progressbar
    m_connection_id_timeout = Glib::signal_timeout().connect( SigC::slot(*this, &SendFileDialog::progressbar_update_cb), 50 );
	 
    icqclient.SendFileTransfer(m_ev);
    break;
  case ICQ2000::FileTransferEvent::REJECTED:
  {
    cancelled( String::ucompose( _("File transfer was refused:\n%1"), ev->getRefusalMessage() ) );
    break;
  }
  case ICQ2000::FileTransferEvent::SEND:
    // Speed Change or something 	
    break; 
  case ICQ2000::FileTransferEvent::RECEIVE:
    // Speed Change or something 	
    break; 
  case ICQ2000::FileTransferEvent::ERROR:
    if (m_connection_id_timeout)
      m_connection_id_timeout.disconnect();
    progressbar_update_cb();

    icqclient.CancelFileTransfer(ev);
    cancelled(ev->getError());
    break;
  case ICQ2000::FileTransferEvent::COMPLETE:
    progressbar_update_cb();
    icqclient.CancelFileTransfer(m_ev);
    if (m_connection_id_timeout)
      m_connection_id_timeout.disconnect();
    progressbar_update_cb();
    
    cancelled( _("File transfer completed successfully") );
    break;
  case ICQ2000::FileTransferEvent::CANCELLED:
    if (m_connection_id_timeout)
      m_connection_id_timeout.disconnect();
    progressbar_update_cb();

    icqclient.CancelFileTransfer(ev);
    cancelled( _("File transfer was cancelled by other side") );
    break;
  case ICQ2000::FileTransferEvent::WAIT_RESPONS:
    break;
  case ICQ2000::FileTransferEvent::TIMEOUT:
    // Notify the other side that I did a timeout and cancelled the connection
    ev->setState(ICQ2000::FileTransferEvent::WAIT_RESPONS);
    icqclient.CancelFileTransfer(ev);

    if (ev->isDirect())
      cancelled( _("Trying to send direct timed out") );
    else
      cancelled( _("Trying to send through server timed out") );
    break;
  case ICQ2000::FileTransferEvent::CLOSE:
    if (m_connection_id_timeout)
      m_connection_id_timeout.disconnect();
    progressbar_update_cb();
    
    cancelled( _("File transfer was cancelled") );
    break;
  default:
    break;
  }
  
}

bool SendFileDialog::progressbar_update_cb()
{
  double val1, val2;

  if (m_ev->getTotalSize() > 0)
  {
    val1 = double(m_ev->getPos())/m_ev->getSize();
    val2 = double(m_ev->getTotalPos())/m_ev->getTotalSize();
  }
  else
  {
    val1 = 0.0;
    val2 = 0.0;
  }
  
  m_progressbar_file.set_fraction(val1);
  m_progressbar_batch.set_fraction(val2);
  m_progressbar_file.set_text( String::ucompose( _("%1%%"), int(val1*100) ) );
  m_progressbar_batch.set_text( String::ucompose( _("%1%%"), int(val2*100) ) );

  // returns true so it will be called again.
  return true;
}

void SendFileDialog::cancelled(const Glib::ustring& str)
{
  m_msg_text.set_sensitive(true);
  m_filename.set_sensitive(true);
  m_fileselectbutton.set_sensitive(true);
  m_send.set_sensitive(true);

  Gtk::MessageDialog dialog(*this, str);
  dialog.run();
}



