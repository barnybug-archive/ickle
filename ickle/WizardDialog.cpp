/* $Id: WizardDialog.cpp,v 1.9 2003-01-02 16:40:02 barnabygray Exp $
 *
 * Copyright (C) 2001 Nils Nordman <nino@nforced.com>
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

#include "WizardDialog.h"

#include "sstream_fix.h"
#include <libicq2000/Client.h>
#include "main.h"
#include "Settings.h"
#include "PromptDialog.h"

#include <gtkmm/main.h>
#include <gtkmm/frame.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>

using std::string;
using std::ostringstream;

using ICQ2000::Status;

WizardDialog::WizardDialog()
  : Gtk::Dialog("Ickle Wizard"),
    btn_next("Next >>"),
    btn_prev("<< Previous"),
    btn_cancel(Gtk::Stock::CANCEL),
    win_regpopup(Gtk::WINDOW_POPUP),
    newuin(0),
    retval(false)
{
  // for use with manage and temporary vars
  Gtk::Frame *fr; 
  Gtk::Label *lbl;
  Gtk::VBox *vb;

  // ----- intro page ----------------------------------------------------------
  rb_newuin.add_label( "Register a new account. Choose this if you haven't used\n"
                       "ICQ before to create a new account." );
  rb_existing_uin.add_label( "Existing account. Choose this if you already have an\n"
                             "existing ICQ account that you would like to use." );
  Gtk::RadioButton::Group gp = rb_newuin.get_group();
  rb_existing_uin.set_group( gp );
  rb_newuin.set_active(true);

  lbl = dynamic_cast<Gtk::Label *>( rb_newuin.get_child() );
  lbl->set_justify(Gtk::JUSTIFY_LEFT);
  lbl = dynamic_cast<Gtk::Label *>( rb_existing_uin.get_child() );
  lbl->set_justify(Gtk::JUSTIFY_LEFT);

  vb = manage( new Gtk::VBox() );
  vb->set_spacing(5);
  vb->pack_start( rb_newuin, true, true );
  vb->pack_start( rb_existing_uin, true, true );
  fr = manage( new Gtk::Frame( "Account registration" ) );
  fr->set_border_width(5);
  fr->add( *vb );

  lbl = manage( new Gtk::Label(
                "Welcome to Ickle!\n\n"

    "This wizard will help you to get started with ickle."
                ) );
  page_intro.pack_start( *lbl, true, true, 10 );
  page_intro.pack_end( *fr, true, true, 10 );
  
  // ----- new account - password page / existing account - details page -------

  en_new_pass1.set_visibility(false);
  en_new_pass2.set_visibility(false);

  lbl = manage( new Gtk::Label( "Please enter the password to use with this account below.\n"
                                "You will need to enter it twice to ensure it's correct.\n"
                                "Clicking next will cause ickle to attempt to connect "
                                "to the server and register the account, so be sure you "
                                "are connected to the Internet before proceeding.\n"
				"Note: If registration fails it can be because the password "
				"you entered was too simple. Try to make up more complicated password "
				"by mixing numbers and letters.") );
  //  lbl->set_default_size( 300, 0 );
  lbl->set_justify(Gtk::JUSTIFY_FILL);
  lbl->set_line_wrap(true);
  page_new_pass.pack_start( *lbl, true, true, 10 );

  lbl = manage( new Gtk::Label( "Password:" ) );
  lbl->set_justify(Gtk::JUSTIFY_LEFT);
  page_new_pass.pack_start( *lbl );
  page_new_pass.pack_start( en_new_pass1 );
  
  lbl = manage( new Gtk::Label( "Re-enter password:" ) );
  lbl->set_justify(Gtk::JUSTIFY_LEFT);
  page_new_pass.pack_start( *lbl );
  page_new_pass.pack_start( en_new_pass2 );

  // ----- existing account - details page -------------------------------------

  en_existing_pass1.set_visibility(false);
  en_existing_pass2.set_visibility(false);

  lbl = manage( new Gtk::Label( "Please enter your existing UIN and your password below.\n"
                                "You will need to enter the password twice to ensure it's\n"
                                "correct.\n" ) );
  lbl->set_justify(Gtk::JUSTIFY_LEFT);
  page_existing_details.pack_start( *lbl, true, true, 10 );

  lbl = manage( new Gtk::Label( "Existing UIN:" ) );
  lbl->set_justify(Gtk::JUSTIFY_LEFT);
  page_existing_details.pack_start( *lbl );
  page_existing_details.pack_start( en_uin );

  lbl = manage( new Gtk::Label( "Password:" ) );
  lbl->set_justify(Gtk::JUSTIFY_LEFT);
  page_existing_details.pack_start( *lbl );
  page_existing_details.pack_start( en_existing_pass1 );
  
  lbl = manage( new Gtk::Label( "Re-enter password:" ) );
  lbl->set_justify(Gtk::JUSTIFY_LEFT);
  page_existing_details.pack_start( *lbl );
  page_existing_details.pack_start( en_existing_pass2 );

  // ----- finished page -------------------------------------------------------

  lbl = manage( new Gtk::Label( "Congratulations!\n"
                                "\n"
                                "You have now successfully configured ickle. You can see your\n"
                                "assigned UIN below, you can later find it in the settings\n"
                                "dialog as well. The first time you connect to the ICQ network\n"
                                "you will be prompted to enter your personal information.\n"
                                "Ickle will now try to connect to the ICQ network with the\n"
                                "given account. Press next to continue.\n"
                                "\n"
                                "Your UIN:"
                                ) );
  lbl->set_justify( Gtk::JUSTIFY_LEFT );
  page_finished.pack_start( *lbl );
  en_newuin.set_editable(false);
  page_finished.pack_start(en_newuin, true, true, 10 );
  
  // ----- action area ---------------------------------------------------------

  btn_cancel.signal_clicked().connect( SigC::slot(*this, &WizardDialog::cancel_cb) );
  btn_next.signal_clicked().connect( SigC::slot(*this, &WizardDialog::next_cb) );
  btn_prev.signal_clicked().connect( SigC::slot(*this, &WizardDialog::prev_cb) );
  btn_prev.set_sensitive(false);
  Gtk::HButtonBox *bb = get_action_area();
  bb->add( btn_cancel);
  bb->add( btn_prev );
  bb->add( btn_next );

  curpage = &page_intro;        
  vb = get_vbox();      
  vb->add( *curpage );          
  
  // ----- dlg -----------------------------------------------------------------

  icqclient.newuin.connect( this, &WizardDialog::newuin_cb );

  set_border_width(10);
  show_all();
}

bool WizardDialog::run()
{
  Gtk::Main::run();
  return retval;
}

void WizardDialog::next_cb()
{
  Gtk::VBox *vb = get_vbox();
  Gtk::Widget *prevpage = curpage;
  
  if( curpage == &page_intro ) { intro_next(); }
  else if( curpage == &page_new_pass ) { new_pass_next(); }
  else if( curpage == &page_existing_details ) { existing_details_next(); }
  else if( curpage == &page_finished ) { finished_next(); }

  if( prevpage != curpage )
  {
    vb->remove( *prevpage );
    vb->add( *curpage );
    show_all();
  }
}

void WizardDialog::prev_cb()
{
  Gtk::VBox *vb = get_vbox();
  Gtk::Widget *prevpage = curpage;
  
  if( curpage == &page_new_pass ) { new_pass_prev(); }
  if( curpage == &page_existing_details ) { existing_details_prev(); }
  else if( curpage == &page_finished ) { finished_prev(); }

  if( prevpage != curpage )
  {
    vb->remove( *prevpage );
    vb->add( *curpage );
    show_all();
  }
}

void WizardDialog::cancel_cb()
{
  PromptDialog pd( *this, Gtk::MESSAGE_QUESTION,
                   "Are you sure you want to exit the wizard?" );
  if( pd.run() == Gtk::RESPONSE_YES )
  {
    Gtk::Main::quit();
  }
}

void WizardDialog::intro_next()
{
    btn_prev.set_sensitive( true );
    if( rb_existing_uin.get_active() )
      curpage = &page_existing_details;
    else
      curpage = &page_new_pass;
}

void WizardDialog::new_pass_next()
{
  if( !en_new_pass1.get_text_length() )
  {
    PromptDialog pd( *this, Gtk::MESSAGE_INFO,
                     "You must supply a password before registering the account" );
    pd.run();
  }
  else if( en_new_pass1.get_text() != en_new_pass2.get_text() )
  {
    PromptDialog pd( *this, Gtk::MESSAGE_INFO,
                     "The given passwords do not match, please correct this and try again" );
    pd.run();
  }
  else
  {
    Gtk::Frame *fr = manage( new Gtk::Frame( "Process:" ) );
    win_lbl.set_text( "Registering new account..." );
    fr->add( win_lbl );
    fr->set_border_width(5);
    win_regpopup.add( *fr );
    win_regpopup.set_modal(true);
    win_regpopup.set_transient_for (*this);
    win_regpopup.set_border_width(5);
    win_regpopup.signal_delete_event().connect( SigC::slot(*this, &WizardDialog::popup_delete_cb) );
    win_regpopup.show_all();
    icqclient.setPassword( en_new_pass1.get_text() );
    icqclient.RegisterUIN();
    Glib::signal_idle().connect( SigC::slot(*this, &WizardDialog::timeout_cb), 1000 * 30 );

    Gtk::Main::run();
    if( newuin )
      curpage = &page_finished;
  }
}

void WizardDialog::existing_details_next()
{
  if( !en_uin.get_text_length() )
  {
    PromptDialog pd( *this, Gtk::MESSAGE_INFO,
                     "You must supply an UIN before continuing!" );
    pd.run();
  }
  else if( !en_existing_pass1.get_text_length() )
  {
    PromptDialog pd( *this, Gtk::MESSAGE_INFO,
                     "You must supply a password before continuing!" );
    pd.run();
  }
  else if( en_existing_pass1.get_text() != en_existing_pass2.get_text() )
  {
    PromptDialog pd( *this, Gtk::MESSAGE_INFO,
                     "The given passwords do not match, please correct this and try again" );
    pd.run();
  }
  else {
    newuin = ICQ2000::Contact::StringtoUIN(en_uin.get_text());
    if( !newuin ) {
      ostringstream os;
      os << "'" << en_uin.get_text() << "' is not a valid UIN, please correct this and try again";
      PromptDialog pd( *this, Gtk::MESSAGE_INFO, os.str() );
      pd.run();
    }
    else {
      curpage = &page_finished;
      en_newuin.set_text( en_uin.get_text() );
    }
  }
}

void WizardDialog::finished_next()
{
  icqclient.setUIN( newuin );
  g_settings.setValue("uin", newuin);

  string password;
  if( rb_existing_uin.get_active() )
    password = en_existing_pass1.get_text();
  else
    password = en_new_pass1.get_text();
  
  g_settings.setValue("password", password);
  icqclient.setPassword(password);

  retval = true;
  Gtk::Main::quit();
}

void WizardDialog::new_pass_prev()
{
  curpage = &page_intro;
  btn_prev.set_sensitive( false );
}

void WizardDialog::existing_details_prev()
{
  curpage = &page_intro;
  btn_prev.set_sensitive( false );
}

void WizardDialog::finished_prev()
{
  if( rb_existing_uin.get_active() )
    curpage = &page_existing_details;
  else
    curpage = &page_new_pass;
}

bool WizardDialog::on_delete_event(GdkEventAny *)
{
  PromptDialog pd( *this, Gtk::MESSAGE_QUESTION,
                   "Are you sure you want to exit the wizard?" );
  if( pd.run() == Gtk::RESPONSE_YES )
  {
    Gtk::Main::quit();
    return false;
  }
  else
  {
    return true;
  }
}

bool WizardDialog::timeout_cb()
{
    PromptDialog pd( *this, Gtk::MESSAGE_QUESTION, "Registration attempt timed out. Would you like to retry?" );
    if( pd.run() == Gtk::RESPONSE_YES )
    {
      win_lbl.set_text( "Registering new account...[Retry]" );
      icqclient.RegisterUIN();
      return 1;
    }
    else
    {
      Gtk::Main::quit();
      return 0;
    }
}

void WizardDialog::newuin_cb(ICQ2000::NewUINEvent *nue)
{
  if( nue->isSuccess() )
  {
    newuin = nue->getUIN();
    en_newuin.set_text( ICQ2000::Contact::UINtoString(newuin) );
    Gtk::Main::quit();
  }
  else
  {
    PromptDialog pd( *this, Gtk::MESSAGE_QUESTION, "Registration attempt failed. Would you like to retry?" );
    if( pd.run() == Gtk::RESPONSE_YES )
    {
      win_lbl.set_text( "Registering new account...[Retry]" );
      icqclient.RegisterUIN();
    }
    else
    {
      Gtk::Main::quit();
    }
  }
}
