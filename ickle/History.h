/* $Id: History.h,v 1.8 2001-12-26 23:24:19 barnabygray Exp $
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
#include <sigc++/signal_system.h>

#include <libicq2000/events.h>

using ICQ2000::MessageEvent;

using std::string;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::vector;
using std::streampos;

class History : public SigC::Object {
  
 private:

  ifstream              m_if;
  bool                  m_streamlock;
  string                m_filename;
  guint                 m_size;
  vector<streampos>     m_index;
  bool                  m_builtindex;
  unsigned int          m_uin;

  void                  quote_output    (ostream& ostr, const string& text);
  void                  build_index     ();

  void                  touch           ();

 public:

  struct Entry {
    enum Direction {
      RECEIVED,
      SENT
    };
  
    MessageEvent::MessageType type;
    time_t timestamp;
    Direction dir;
    bool offline;
    bool multiparty;
    string message;
    string URL;
    bool receipt;
    bool delivered; // for SMS receipts
  };

  History();
  History(unsigned int uin);
  ~History();

  void                  log             (MessageEvent *ev, bool received) throw (std::runtime_error);
  void                  get_msg         (guint index, Entry &he)
    throw(std::out_of_range, std::runtime_error);

  void                  stream_lock     () throw (std::runtime_error);
  void                  stream_release  () throw (std::runtime_error);
  string                getFilename     () const { return m_filename; }
  guint                 size            ();

  SigC::Signal1<void,Entry *> new_entry;
};

#endif

