/*
 * Buffer class header
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

#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <iostream>
#include <iomanip.h>
#include <string>
#include <iterator>
#include <algorithm>


class Buffer {
 public:
  enum endian { BIG, LITTLE };

 private:
  vector<unsigned char> data;
  endian endn;
  unsigned int out_pos;

 public:
  Buffer();
  Buffer(const unsigned char *d, int size); // construct from an array
  Buffer(Buffer& b, unsigned int start, unsigned int data_len); // construct by copying from another Buffer

  const unsigned char* Data() { return data.begin(); }
  unsigned int size() const { return data.size(); }
  unsigned int pos() const { return out_pos; }
  unsigned int remains() const { return data.size() - out_pos; }

  void clear();
  bool empty();
  void advance(unsigned int ad) { out_pos += ad; }
  bool beforeEnd() const { return (out_pos < data.size()); }
  void setPos(unsigned int o) { out_pos = o; }
  void chopOffBuffer(Buffer& b, unsigned int sz);

  void setEndianness(endian e);

  Buffer& operator<<(unsigned char);
  Buffer& operator<<(unsigned short);
  Buffer& operator<<(unsigned int);
  Buffer& operator<<(signed char l) { return (*this) << (unsigned char)l; }
  Buffer& operator<<(signed short l) { return (*this) << (unsigned short)l; }
  Buffer& operator<<(signed int l) { return (*this) << (unsigned int)l; }
  Buffer& operator<<(const string&);

  Buffer& operator>>(unsigned char&);
  Buffer& operator>>(unsigned short&);
  Buffer& operator>>(unsigned int&);
  Buffer& operator>>(string&);

  void Pack(const unsigned char *d, int size);
  void Pack(const string& s);
  void Unpack(string& s, int size);

  void UnpackUint32String(string& s);

  unsigned char& operator[](unsigned int p);

  void dump(ostream& out);
};

ostream& operator<<(ostream&,Buffer&);

#endif
