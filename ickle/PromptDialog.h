/*
 * PromptDialog
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

#ifndef PROMPTDIALOG_H
#define PROMPTDIALOG_H

#include <gtk--/dialog.h>

#include <string>

class PromptDialog : public Gtk::Dialog {
 public:
  enum PromptType {
    PROMPT_WARNING,
    PROMPT_CONFIRM,
    PROMPT_QUESTION,
    PROMPT_INFO
  };

 private:
  PromptType m_type;
  bool m_finish_bool;
  bool m_modal;

 public:
  PromptDialog(Gtk::Window * parent, PromptType t, const std::string& msg, bool modal = true);

  bool run();

  void true_cb();
  void false_cb();
};

#endif
