/* $Id: WizardDialog.h,v 1.2 2002-07-21 00:23:37 bugcreator Exp $
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

#ifndef WIZARDDIALOG_H
#define WIZARDDIALOG_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libicq2000/events.h>

#include <gtk--/window.h>
#include <gtk--/dialog.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/radiobutton.h>
#include <gtk--/box.h>
#include <gtk--/entry.h>
#include <gtk--/table.h>

class WizardDialog : public Gtk::Dialog {

 private:

  // action area
  Gtk::Button btn_next, btn_prev, btn_cancel;

  // welcome page
  Gtk::VBox page_intro;
  Gtk::RadioButton rb_newuin, rb_existing_uin;

  // new account - password page 
  Gtk::VBox page_new_pass;
  Gtk::Entry en_new_pass1, en_new_pass2;

  // existing account - details page
  Gtk::VBox page_existing_details;
  Gtk::Entry en_uin, en_existing_pass1, en_existing_pass2;
       
  // finished page
  Gtk::VBox page_finished;
  Gtk::Entry en_newuin;

  // poup window
  Gtk::Window win_regpopup;
  Gtk::Label win_lbl;

  Gtk::Widget *curpage;
  unsigned int newuin;
  bool retval;

  void next_cb();
  void prev_cb();
  void cancel_cb();

  void intro_next();
  void new_pass_next();
  void existing_details_next();
  void finished_next();
  void new_pass_prev();
  void existing_details_prev();
  void finished_prev();

  virtual int delete_event_impl(GdkEventAny *event);

  int popup_delete_cb(GdkEventAny *) { return 1; }
  gint timeout_cb();
  void newuin_cb(ICQ2000::NewUINEvent *nue);
    
 public:

  WizardDialog();

  bool run();
};

#endif
