/*
 * Buffer class
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

#include "buffer.h"

Buffer::Buffer() : endn(BIG), out_pos(0), data() { }

Buffer::Buffer(const unsigned char* d, int size) : endn(BIG), out_pos(0), data(d, d+size) { }

Buffer::Buffer(Buffer& b, unsigned int start, unsigned int data_len)
  : endn(BIG), out_pos(0), data(b.Data()+start, b.Data()+start+data_len) { }

unsigned char& Buffer::operator[](unsigned int p) {
  return data[p];
}

void Buffer::clear() {
  data.clear();
  out_pos = 0;
}

bool Buffer::empty() {
  return data.empty();
}

void Buffer::chopOffBuffer(Buffer& b, unsigned int sz) {
  copy( data.begin(), data.begin()+sz, back_inserter(b.data) );
  data.erase( data.begin(), data.begin()+sz );
  out_pos = 0;
}

void Buffer::Pack(const unsigned char *d, int size) {
  copy(d, d+size, back_inserter(data));
}

void Buffer::Pack(const string& s) {
  copy(s.begin(), s.end(), back_inserter(data));
}

void Buffer::UnpackUint32String(string& s) {
  unsigned int l;
  (*this) >> l;
  Unpack(s, l);
}

void Buffer::UnpackUint16StringNull(string& s) {
  unsigned short sh;
  (*this) >> sh;
  Unpack(s, sh-1);
}

void Buffer::Unpack(string& s, int size) {
  if (size > data.size()-out_pos) size = data.size()-out_pos;
  copy(data.begin()+out_pos, data.begin()+out_pos+size, back_inserter(s));
  out_pos += size;
}

// -- Input stream methods --

Buffer& Buffer::operator<<(unsigned char l) {
  data.push_back(l);
  return (*this);
}

Buffer& Buffer::operator<<(unsigned short l) {
  if (endn == BIG) {
    data.push_back((l>>8) & 0xFF);
    data.push_back(l & 0xFF);
  } else {
    data.push_back(l & 0xFF);
    data.push_back((l>>8) & 0xFF);
  }    
  return (*this);
}

Buffer& Buffer::operator<<(unsigned int l) {
  if (endn == BIG) {
    data.push_back((l >> 24) & 0xFF);
    data.push_back((l >> 16) & 0xFF);
    data.push_back((l >> 8) & 0xFF);
    data.push_back(l & 0xFF);
  } else {
    data.push_back(l & 0xFF);
    data.push_back((l >> 8) & 0xFF);
    data.push_back((l >> 16) & 0xFF);
    data.push_back((l >> 24) & 0xFF);
  }
  return (*this);
}

// strings stored as length (2 bytes), string data, _not_ null-terminated
Buffer& Buffer::operator<<(const string& s) {
  unsigned short sz = s.size();
  data.push_back((sz>>8) & 0xFF);
  data.push_back(sz & 0xFF);
  Pack(s);
  return (*this);
}

// -- Output stream methods --

Buffer& Buffer::operator>>(unsigned char& l) {
  if (out_pos + 1 > data.size()) l = 0;
  else l = data[out_pos++];
  return (*this);
}

Buffer& Buffer::operator>>(unsigned short& l) {
  if (out_pos + 2 > data.size()) {
    l = 0;
    out_pos += 2;
  } else {
    if (endn == BIG) {
      l = ((unsigned short)data[out_pos++] << 8)
	+ ((unsigned short)data[out_pos++]);
    } else {
      l = ((unsigned short)data[out_pos++])
	+ ((unsigned short)data[out_pos++] << 8);
    }
  }
  return (*this);
}

Buffer& Buffer::operator>>(unsigned int& l) {
  if (out_pos + 4 > data.size()) {
    l = 0;
    out_pos += 4;
  } else {
    if (endn == BIG) {
      l = ((unsigned int)data[out_pos++] << 24)
	+ ((unsigned int)data[out_pos++] << 16)
	+ ((unsigned int)data[out_pos++] << 8)
	+ ((unsigned int)data[out_pos++]);
    } else {
      l = ((unsigned int)data[out_pos++])
	+ ((unsigned int)data[out_pos++] << 8)
	+ ((unsigned int)data[out_pos++] << 16)
	+ ((unsigned int)data[out_pos++] << 24);
    }
  }
  return (*this);
}

// strings stored as length (2 bytes), string data, _not_ null-terminated
Buffer& Buffer::operator>>(string& s) {
  if (out_pos + 2 > data.size()) {
    s = ""; // clear() method doesn't seem to exist!
    out_pos += 2;
  } else {
    unsigned short sz;
    (*this) >> sz;
    Unpack(s, sz);
  }
  return (*this);
}

void Buffer::setEndianness(endian e) {
  endn = e;
}

void Buffer::dump(ostream& out) {
  char d[] = "123456789abcdef0";
  out << hex << setfill('0');
  int m = ((data.size()+15)/16)*16;
  for (int a = 0; a < m; a++) {
    if (a % 16 == 0) out << setw(4) << a << "  ";
    if (a < data.size()) {
      out << setw(2) << (int)data[a] << " ";
      if (data[a] < 32) d[a%16] = '.';
      else d[a%16] = data[a];
    } else {
      out << "   ";
      d[a%16] = ' ';
    }
    if (a % 16 == 15) out << " " << d << endl;
  }
}

ostream& operator<<(ostream& out, Buffer& b) { b.dump(out); return out; }
