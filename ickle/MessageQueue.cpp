/* $Id: MessageQueue.cpp,v 1.1 2002-03-28 18:29:02 barnabygray Exp $
 *
 * Queueing for MessageEvents.
 *
 * Copyright (C) 2002 Barnaby Gray <barnaby@beedesign.co.uk>,
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

#include "MessageQueue.h"

#include <algorithm>

using std::list;
using std::find;

using ICQ2000::ContactRef;

// ============================================================================
//  MessageQueue
// ============================================================================

MessageQueue::MessageQueue()
{ }

MessageQueue::~MessageQueue()
{
  while (!m_event_list.empty()) {
    MessageEvent *ev = m_event_list.front();
    m_event_list.pop_front();
    delete ev;
  }
}

void MessageQueue::add_to_queue(MessageEvent *ev)
{
  m_event_list.push_back(ev);
  added.emit(ev);
}

void MessageQueue::remove_from_queue(MessageEvent *ev)
{
  event_iter f = find( m_event_list.begin(), m_event_list.end(), ev );
  if ( f != m_event_list.end() ) {
    m_event_list.erase(f);
    removed.emit(ev);
    delete ev;
  }
}
  
MessageEvent* MessageQueue::get_first_message()
{
  return m_event_list.front();
}

unsigned int MessageQueue::get_size() const
{
  return m_event_list.size();
}

bool MessageQueue::empty() const
{
  return m_event_list.empty();
}

MessageEvent* MessageQueue::get_contact_first_message(const ContactRef& c)
{
  // this will be generalised to a general client-side contact eventually..
  // these could be done more efficiently

  event_iter curr = m_event_list.begin();
  while (curr != m_event_list.end()) {
    if ((*curr)->getServiceType() == MessageEvent::ICQ) {
      ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(*curr);
      if (c->getUIN() == icq->getICQContact()->getUIN()) {
	return *curr;
      }
    }
    ++curr;
  }

  return NULL;
}

unsigned int MessageQueue::get_contact_size(const ContactRef& c) const
{
  int count = 0;

  // this will be generalised to a general client-side contact eventually..

  event_const_iter curr = m_event_list.begin();
  while (curr != m_event_list.end()) {
    if ((*curr)->getServiceType() == MessageEvent::ICQ) {
      ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(*curr);
      if (c->getUIN() == icq->getICQContact()->getUIN()) ++count;
    }
    ++curr;
  }

  return count;
}

void MessageQueue::clear_queue_for_contact(const ContactRef& c)
{
  event_iter curr = m_event_list.begin();
  while (curr != m_event_list.end()) {
    event_iter next = curr;
    ++next;

    if ((*curr)->getServiceType() == MessageEvent::ICQ) {
      ICQMessageEvent *icq = static_cast<ICQMessageEvent*>(*curr);
      if (c->getUIN() == icq->getICQContact()->getUIN()) {
	m_event_list.erase(curr);
	removed.emit(icq);
	delete icq;
      }
    }
    
    curr = next;
  }
}
