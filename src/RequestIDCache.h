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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_EXT_HASH_MAP
# include <ext/hash_map>
#elif HAVE_HASH_MAP
# include <hash_map>
#else
# error "hash_map not defined"
#endif

#include <list>
#include <utility>
#include <time.h>

using std::list;
using std::hash;
using std::pair;

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
    
    Key& getKey();
    Value& getValue();
    time_t getTimestamp() const;
    void setTimestamp(time_t t);
  };

  template < typename Key, typename Value >
  class Cache {
   protected:
    typedef list< pair<Key,Value> >::iterator literator;

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

    bool exists(const Key &k);

    Value& operator[](const Key &k);
    virtual void remove(const Key &k);
    Value& insert(const Key &k, const Value &v);

    unsigned int getTimeout(); // seconds
    void setTimeout(unsigned int s);

    void clearoutPoll();
  };

}

#endif
