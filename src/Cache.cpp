/*
 * Cache
 *
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

#include "Cache.h"

namespace ICQ2000 {

  template <typename Key, typename Value>
  CacheItem<Key,Value>::CacheItem(const Key &k, const Value &v)
    : m_key(k), m_value(v),
      m_timestamp(time(NULL)) { }

  template <typename Key, typename Value>
  void CacheItem<Key,Value>::setTimestamp(time_t t) { m_timestamp = t; }
  
  template <typename Key, typename Value>
  time_t CacheItem<Key,Value>::getTimestamp() const { return m_timestamp; }
  
  template <typename Key, typename Value>
  Key& CacheItem<Key,Value>::getKey() {
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
  bool Cache<Key,Value>::exists(const Key &k) {
    literator curr = m_list.begin();
    while (curr != m_list.end()) {
      if (*curr.getKey() == k) return true;
      ++curr;
    }
    return false;
  }

  template <typename Key, typename Value>
  Value& Cache<Key,Value>::operator[](const Key &k) {
    literator curr = m_list.begin();
    while (curr != m_list.end()) {
      if (*curr.getKey() == k) return *curr.getValue();
      ++curr;
    }
    return insert(k, Value());
  }

  template <typename Key, typename Value>
  Value& Cache<Key,Value>::insert(const Key &k, const Value &v) {
    m_list.push_front( CacheItem<Key,Value>(k,v) );
    return v;
  }

  template <typename Key, typename Value>
  void Cache<Key,Value>::clearoutPoll() {
    time_t n = time(NULL) - m_timeout;;
    while (!m_list.empty() && m_list[0].getTimestamp() < n) {
      m_list.pop_front();
    }
  }
  
  template <typename Key, typename Value>
  unsigned int Cache<Key,Value>::getTimeout() { return m_timeout; }

  template <typename Key, typename Value>
  void Cache<Key,Value>::setTimeout(unsigned int s) { m_timeout = s; }
}
