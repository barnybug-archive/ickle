/*
 * RequestIDCache
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

#ifndef REQUESTIDCACHE_H
#define REQUESTIDCACHE_H

#include "Cache.h"

namespace ICQ2000 {

  class RequestIDCacheValue {
   public:
    enum Type {
      UserInfo
    };

    virtual Type getType() const = 0;
  };

  class UserInfoCacheValue : public RequestIDCacheValue {
   private:
    unsigned int m_uin;

   public:
    UserInfoCacheValue(unsigned int uin) : m_uin(uin) { }
    unsigned int getUIN() const { return m_uin; }

    Type getType() const { return UserInfo; }
  };

  class RequestIDCache : public Cache<unsigned int, RequestIDCacheValue*> {
   public:
    RequestIDCache() { }

    void removeItem(const RequestIDCache::literator& l) {
      delete ((*l).getValue());
      Cache<unsigned int, RequestIDCacheValue*>::removeItem(l);
    }
  };
  
}

#endif
