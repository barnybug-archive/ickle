/* $Id: History.cpp,v 1.22 2003-01-04 19:42:45 barnabygray Exp $
 * 
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#include "main.h"
#include "History.h"
#include "utils.h"

#include "ickle.h"
#include "ucompose.h"

#include <libicq2000/Contact.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>

using ICQ2000::NormalMessageEvent;
using ICQ2000::URLMessageEvent;
using ICQ2000::SMSMessageEvent;
using ICQ2000::SMSReceiptEvent;
using ICQ2000::EmailExEvent;
using ICQ2000::WebPagerEvent;
using ICQ2000::ContactRef;

using std::string;
using std::endl;
using std::out_of_range;
using std::runtime_error;
using std::istringstream;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::vector;
using std::streampos;

/**
 *  Note the historyfile should be given relative to the current CONTACT_DIR.
 */
History::History(const string &historyfile)
{
  m_filename = CONTACT_DIR + historyfile;
  m_builtindex = false;
  m_size = 0;
  m_streamlock = false;
  m_utf8 = true;

  touch();
}

History::~History()
{
  if( m_streamlock )
  {
    cerr << Utils::console( Glib::ustring( _("Warning: History::~History: stream was not released properly") ) ) << endl;
    m_if.close();
  }
}

void History::touch()
{
  // should just create a blank history file if none exists
  ofstream( m_filename.c_str(), std::ios::out | std::ios::app );
}

void History::log(ICQ2000::MessageEvent *ev, bool received) throw(runtime_error)
{
  Entry he;
  ofstream of;

  ContactRef c = ev->getContact();

  /* there are some other events that aren't logged in history
     (auth requests for example) */
  if (!(ev->getType() == ICQ2000::MessageEvent::Normal
	|| ev->getType() == ICQ2000::MessageEvent::URL
	|| ev->getType() == ICQ2000::MessageEvent::SMS
	|| ev->getType() == ICQ2000::MessageEvent::SMS_Receipt
	|| ev->getType() == ICQ2000::MessageEvent::EmailEx
	|| ev->getType() == ICQ2000::MessageEvent::WebPager))
    return;

  of.open( m_filename.c_str(), std::ios::out | std::ios::app );
  if (!of.is_open())
    throw runtime_error( String::ucompose( _("History::log: Could not open historyfile for writing: %1"), m_filename ) );

  // add to index
  of.seekp( 0, std::ios::end );
  if( m_builtindex )
  {
    if( m_size >= m_index.size() )
      m_index.resize( m_index.size() + 100 );
    m_index[ m_size++ ] = of.tellp();
  }

  // at the same time convert the stuff into a Entry, for the signal
  he.type = ev->getType();
  he.timestamp = ev->getTime();
  he.dir = received ? Entry::RECEIVED : Entry::SENT;

  of << "Time: " << ev->getTime() << endl 
     << "Direction: " << ( received ? "Received" : "Sent" ) << endl;

  ICQ2000::ICQMessageEvent *icq = dynamic_cast<ICQ2000::ICQMessageEvent*>(ev);
  if (icq != NULL && icq->isUrgent())
  {
    of << "Urgent: Yes" << endl;
  }
  
  if (ev->getType() == ICQ2000::MessageEvent::Normal)
  {

    ICQ2000::NormalMessageEvent *nev = static_cast<ICQ2000::NormalMessageEvent*>(ev);
    he.message = nev->getMessage();
    he.offline = nev->isOfflineMessage();
    he.multiparty = nev->isMultiParty();

    of << "Type: Normal" << endl
       << "Offline: " << ( nev->isOfflineMessage() ? "Yes" : "No" ) << endl
       << "Multiparty: " << ( nev->isMultiParty() ? "Yes" : "No" ) << endl
       << "Message: ";

    if (m_utf8)
      quote_output( of, nev->getMessage() );
    else
      quote_output( of, Utils::locale_from_utf8_with_fallback(nev->getMessage()) );
      
    of << endl;
      
  }
  else if (ev->getType() == ICQ2000::MessageEvent::URL)
  {

    ICQ2000::URLMessageEvent *uev = static_cast<ICQ2000::URLMessageEvent*>(ev);
    he.message = uev->getMessage();
    he.offline = uev->isOfflineMessage();
    he.URL = uev->getURL();

    of << "Type: URL" << endl
       << "Offline: " << ( uev->isOfflineMessage() ? "Yes" : "No" ) << endl
       << "Message: ";

    if (m_utf8)
      quote_output( of, uev->getMessage() );
    else
      quote_output( of, Utils::locale_from_utf8_with_fallback(uev->getMessage()) );
    of << "URL: ";
    if (m_utf8)
      quote_output( of, uev->getURL() );
    else
      quote_output( of, Utils::locale_from_utf8_with_fallback(uev->getURL()) );
    of << endl;
      
  }
  else if (ev->getType() == ICQ2000::MessageEvent::SMS)
  {

    ICQ2000::SMSMessageEvent *sev = static_cast<ICQ2000::SMSMessageEvent*>(ev);
    he.message = sev->getMessage();

    of << "Type: SMS" << endl;
    if (!received) {
      of << "Receipt: " << ( sev->getRcpt() ? "Yes" : "No" ) << endl;
      he.receipt = sev->getRcpt();
    }
    of << "Message: ";
    if (m_utf8)
      quote_output( of, sev->getMessage() );
    else
      quote_output( of, Utils::locale_from_utf8_with_fallback(sev->getMessage()) );
    of << endl;

  }
  else if (ev->getType() == ICQ2000::MessageEvent::SMS_Receipt)
  {

    ICQ2000::SMSReceiptEvent *srev = static_cast<ICQ2000::SMSReceiptEvent*>(ev);
    he.message = srev->getMessage();

    of << "Type: SMSReceipt" << endl;
    of << "Delivered: " << ( srev->delivered() ? "Yes" : "No" ) << endl;
    of << "Message: ";
    if (m_utf8)
      quote_output( of, srev->getMessage() );
    else
      quote_output( of, Utils::locale_from_utf8_with_fallback(srev->getMessage()) );
    of << endl;
  }
  else if (ev->getType() == ICQ2000::MessageEvent::EmailEx)
  {

    ICQ2000::EmailExEvent *ee = static_cast<ICQ2000::EmailExEvent*>(ev);
    he.message = ee->getMessage();

    of << "Type: EmailExpress" << endl
       << "Message: ";
    if (m_utf8)
      quote_output( of, ee->getMessage() );
    else
      quote_output( of, Utils::locale_from_utf8_with_fallback(ee->getMessage()) );
    of << endl;
  }
  else if (ev->getType() == ICQ2000::MessageEvent::WebPager)
  {

    ICQ2000::WebPagerEvent *ee = static_cast<ICQ2000::WebPagerEvent*>(ev);
    he.message = ee->getMessage();

    of << "Type: WebPager" << endl
       << "Message: ";
    if (m_utf8)
      quote_output( of, ee->getMessage() );
    else
      quote_output( of, Utils::locale_from_utf8_with_fallback(ee->getMessage()) );
    of << endl;
  }
  of.close();
  new_entry.emit( &he );
}

void History::quote_output(ostream& ostr, const string& text)
{
  int a = 0, l;
  while ( (l = text.find( '\n', a )) != -1 ) {
    ostr << text.substr( a, l-a ) << "\\" << endl;
    a = l + 1;
  }
  ostr << text.substr( a ) << " " << endl;
}

void History::build_index()
{
  string s;
  bool escaped = false; // was this empty line escaped?

  if( !m_streamlock )
    m_if.open( m_filename.c_str() );

  if( !m_if.is_open() )
  {
    // this does not warrant an exception
    cerr << Utils::console( String::ucompose( _("Could not open historyfile for reading: %1"), m_filename ) ) << endl;
    return;
  }

  getline( m_if, s );
  if ( s == "UTF8" )
  {
    m_utf8 = true;
  }
  else
  {
    m_utf8 = false;
    m_if.seekg(0);
  }

  m_index.resize( 100 );
  m_index[ m_size++ ] = m_if.tellg(); // first msg at ios::beg

  while (true)
  {
    getline( m_if, s );
    if( m_if.eof() )
      break;

    if( !s.size() && !escaped )
    {
      if( m_size >= m_index.size() )
        m_index.resize( m_index.size() + 100 );
      m_index[ m_size++ ] = m_if.tellg();
    }
    escaped = s[ s.size() -1 ] == '\\';
  }
  
  // Ok, we got one msg to many, lose the last one
  m_index[ --m_size ] = 0;
  
  if( !m_streamlock )
    m_if.close();

  m_builtindex = true;
}

void History::get_msg(guint index, Entry &e) throw(out_of_range, runtime_error)
{
  string s,s2;

  if( !m_builtindex )
    build_index();

  if( index >= m_size )
  {
    throw out_of_range( String::ucompose( _("History::get_msg illegal index: %1"), index ) );
  }
  
  if( !m_streamlock )
    m_if.open( m_filename.c_str() );

  if( !m_if.is_open() )
    throw runtime_error( String::ucompose( _("History::get_msg: Could not open historyfile for reading: %1"), m_filename ) );

  m_if.seekg( m_index[ index ] );

  e.timestamp = 0;
  e.multiparty = false;
  e.dir = Entry::SENT;
  e.offline = false;
  e.urgent = false;
  e.receipt = false;
  e.delivered = true;
  e.message.erase();
  e.URL.erase();

  while (true)
  {
    getline( m_if, s );
    if( m_if.eof() || !s.size() ) // eof or end of entry
      break;

    if( s.find( "Type: " ) != string::npos )
    {
      s2 = s.substr( string( "Type: ").size() );
      if( s2 == "Normal" )
        e.type = ICQ2000::MessageEvent::Normal;
      else if( s2 == "SMS" )
        e.type = ICQ2000::MessageEvent::SMS;
      else if( s2 == "SMSReceipt" )
        e.type = ICQ2000::MessageEvent::SMS_Receipt;
      else if( s2 == "EmailExpress" )
        e.type = ICQ2000::MessageEvent::EmailEx;
      else if( s2 == "WebPager" )
        e.type = ICQ2000::MessageEvent::WebPager;
      else if( s2 == "URL" )
        e.type = ICQ2000::MessageEvent::URL;
    }
    else if( s.find( "Time: " ) != string::npos )
    {
      // TODO urrghh... istringstream! i18n!
      istringstream iss( s.substr( string( "Time: ").size() ) );
      iss >> e.timestamp;
    }
    else if( s.find( "Direction: " ) != string::npos )
    {
      s2 = s.substr( string( "Direction: ").size() );
      if( s2 == "Sent" )
        e.dir = Entry::SENT;
      else if( s2 == "Received" )
        e.dir = Entry::RECEIVED;
    }
    else if( s.find( "Offline: " ) != string::npos )
    {
      e.offline = s.substr( string( "Offline: ").size() ) == "Yes";
    }
    else if( s.find( "Multiparty: " ) != string::npos )
    {
      e.multiparty = s.substr( string( "Multiparty: ").size() ) == "Yes";
    }
    else if( s.find( "URL: " ) != string::npos )
    {
      e.URL = s.substr( string( "URL: ").size() );
      if (!m_utf8)
	e.URL = Utils::locale_to_utf8(e.URL);
    }
    else if( s.find( "Receipt: " ) != string::npos )
    {
      e.receipt = s.substr( string( "Receipt: ").size() ) == "Yes";
    }
    else if( s.find( "Delivered: " ) != string::npos )
    {
      e.delivered = s.substr( string( "Delivered: ").size() ) == "Yes";
    }
    else if( s.find( "Urgent: " ) != string::npos )
    {
      e.urgent = s.substr( string( "Urgent: ").size() ) == "Yes";
    }
    else if( s.find( "Message: " ) != string::npos )
    {
      int start = string( "Message: ").size();
      e.message = s.substr( start, s.size() - start - 1 );

      while( s[ s.size() - 1 ] != ' ' )
      {
        e.message += '\n';
        getline( m_if, s );
	e.message += s.substr( 0, s.size() - 1 );
      }

      if (!m_utf8)
	e.message = Utils::locale_to_utf8(e.message);
    }
  }
  if( !m_streamlock )
    m_if.close();
}

void History::stream_lock() throw(runtime_error)
{
  if( m_streamlock )
    throw( runtime_error( _("History::stream_lock: stream is already locked!") ) );
  
  struct stat fs;
  if ( stat( m_filename.c_str(), &fs ) == -1 && errno == ENOENT ) touch();

  m_if.open( m_filename.c_str() );
  if( !m_if.is_open() )
    throw runtime_error( String::ucompose( _("History::stream_lock: Could not open historyfile for reading: %1" ),
					   m_filename ) );
  m_streamlock = true;
  m_if.clear();
}

void History::stream_release() throw(runtime_error)
{
  if( !m_if.is_open() )
    throw( runtime_error( _("History::stream_release: stream is not locked!") ) );

  m_if.close();
  m_streamlock = false;
}

guint History::size()
{
  if( !m_builtindex )
    build_index();

  return m_size;
}

/**
  Returns the name of the current historyfile relative to the current CONTACT_DIR.
*/
string History::getFilename() const
{
  return m_filename.substr( CONTACT_DIR.size() );
}
