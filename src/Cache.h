/*
 * Cache
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

#ifndef CACHE_H
#define CACHE_H

#include <list>
#include <time.h>

using std::list;

namespace ICQ2000 {
  
  /*
   * This class will cache an id to an object, it's templated
   * since it'll be useful in several places with different sorts of
   * ids and objects.
   */
  
  template <typename Key, typename Value> class CacheItem {
   protected:
    time_t m_timestamp;
    Key m_key;
    Value m_value;

   public:
    CacheItem(const Key &k, const Value &v);
    
    const Key& getKey() const;
    Value& getValue();
    time_t getTimestamp() const;
    void setTimestamp(time_t t);
  };

  template < typename Key, typename Value >
  class Cache {
   protected:
    typedef list< CacheItem<Key,Value> >::iterator literator;
    typedef list< CacheItem<Key,Value> >::const_iterator citerator;

    unsigned int m_timeout;
    
    /*
     * list for storing them in order to timeout
     * a hash could be used as well, but efficiency isn't really an
     * issue - there shouldn't be more than 10 items in here at any one point
     */
    list< CacheItem<Key,Value> > m_list;
    
   public:
    Cache();
    virtual ~Cache();

    bool exists(const Key &k) const;

    Value& operator[](const Key &k);

    void remove(const Key &k)  {
      literator curr = m_list.begin();
      while (curr != m_list.end()) {
	if ((*curr).getKey() == k) {
	  removeItem(curr);
	  return;
	}
	++curr;
      }
    }

    virtual void removeItem(const literator& l) {
      m_list.erase(l);
    }

    virtual void expireItem(const literator& l) {
      // might want to override to add signalling on expiring of items
      removeItem(l);
    }

    Value& insert(const Key &k, const Value &v) {
      m_list.push_front( CacheItem<Key,Value>(k,v) );
      return m_list.front().getValue();
    }

    unsigned int getTimeout() { return m_timeout; }
    void setTimeout(unsigned int s) { m_timeout = s; }

    void clearoutPoll() {
      time_t n = time(NULL) - m_timeout;
      while (!m_list.empty() && m_list.front().getTimestamp() < n)
	expireItem( m_list.begin() );
    }

  };

  template <typename Key, typename Value>
  CacheItem<Key,Value>::CacheItem(const Key &k, const Value &v)
    : m_key(k), m_value(v),
      m_timestamp(time(NULL)) { }

  template <typename Key, typename Value>
  void CacheItem<Key,Value>::setTimestamp(time_t t) { m_timestamp = t; }
  
  template <typename Key, typename Value>
  time_t CacheItem<Key,Value>::getTimestamp() const { return m_timestamp; }
  
  template <typename Key, typename Value>
  const Key& CacheItem<Key,Value>::getKey() const {
    return m_key;
  }

  template <typename Key, typename Value>
  Value& CacheItem<Key,Value>::getValue() {
    return m_value;
  }

  template <typename Key, typename Value>
  Cache<Key,Value>::Cache() {
    setTimeout(60); // default timeout
  }

  template <typename Key, typename Value>
  Cache<Key,Value>::~Cache() {
    
  }
 
  template <typename Key, typename Value>
  bool Cache<Key,Value>::exists(const Key &k) const {
    citerator curr = m_list.begin();
    while (curr != m_list.end()) {
      if ((*curr).getKey() == k) return true;
      ++curr;
    }
    return false;
  }

  template <typename Key, typename Value>
  Value& Cache<Key,Value>::operator[](const Key &k) {
    literator curr = m_list.begin();
    while (curr != m_list.end()) {
      if ((*curr).getKey() == k) return (*curr).getValue();
      ++curr;
    }
    return insert(k, Value());
  }

}

#endif
