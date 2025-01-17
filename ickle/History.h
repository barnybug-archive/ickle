/* $Id: History.h,v 1.17 2003-11-02 16:31:30 cborni Exp $
 *
 * Logging and loading of history.
 *
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>,
 * Copyright (C) 2001 Nils Nordman <nino@nforced.com>.
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

#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <glib.h>
#include <sys/types.h>
#include <time.h>

#include "ucompose.h"

#include <sigc++/object.h>
#include <sigc++/signal.h>

#include <libicq2000/events.h>

class History : public SigC::Object
{
  
 private:
  std::ifstream           m_if;
  bool                    m_streamlock;
  std::string             m_filename;
  guint                   m_size;
  std::vector<std::streampos>  m_index;
  bool                    m_builtindex;
  unsigned int            m_uin;
  bool                    m_utf8;
  guint                   current_search;
  guint                   current_on_line;


  guint                 give_possible_entry(const Glib::ustring searchtext, const guint start,
                        const bool case_sensitive);
  void                  quote_output    (std::ostream& ostr, const std::string& text);
  void                  build_index     ();
  void                  touch           ();
  guint                 position_to_index(std::streampos position);




  public:
  struct Entry {
    enum Direction {
      RECEIVED,
      SENT
    };
  
    ICQ2000::MessageEvent::MessageType type;
    time_t timestamp;
    Direction dir;
    bool offline;
    bool multiparty;
    Glib::ustring message;
    std::string URL;
    bool receipt;
    bool delivered; // for SMS receipts
    bool urgent;
  };

  struct FoundText {
    guint entry;
    guint charfrom;
    guint charto;
    bool found;
  };

  History(const std::string& historyfile);
  ~History();

  void                  log             (ICQ2000::MessageEvent *ev, bool received) throw (std::runtime_error);
  void                  get_msg         (guint index, Entry &he)
    throw(std::out_of_range, std::runtime_error);
  Glib::ustring         get_msg         (guint index)
    throw(std::out_of_range, std::runtime_error);
  struct FoundText *    find_msg         (const Glib::ustring searchtext, bool fromstart, bool case_sensitive) 	       throw(std::runtime_error);
  void                  stream_lock     () throw (std::runtime_error);
  void                  stream_release  () throw (std::runtime_error);
  std::string           getFilename     () const;
  guint                 size            ();

  SigC::Signal1<void,Entry *> new_entry;
};

#endif

