/* ucompose - the only purpose of this file is to partially specialise
 * for the case of inserting a Glib::ustring. gcc won't compile it if
 * the specialisation is in the header code (either as included in
 * declaration, or defined separately.. hmff).  Can't vouch for how
 * the hell other compilers or even other versions of gcc will take to
 * this.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */

#include "ucompose.h"

namespace StringPrivate
{
  
  template <>
  Glib::ustring
  Composition::as_ustring<Glib::ustring>(const Glib::ustring& obj)
  {
    return obj;
  }
}

