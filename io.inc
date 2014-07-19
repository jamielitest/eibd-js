/** 
    EIBD client library for Javascript
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>
    Copyright (C) 2014 Elias Karakoulakis <elias.karakoulakis@gmail.com
    
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
# 
#   In addition to the permissions in the GNU General Public License, 
#   you may link the compiled version of this file into combinations
#   with other programs, & distribute those combinations without any 
#   restriction coming from the use of this file. (The General Public 
#   License restrictions do apply in other respects; for example, they 
#   cover modification of the file, & distribution when not linked into 
#   a combine executable.)
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
# 
*/

//import errno;
//import socket;

function EIBBuffer(buf) {
    this.buffer = buf || [];
}

function EIBAddr(value) {
    this.data = value || 0;
}

function EIBInt8(value) {
    this.data = value || 0;
}

function EIBInt16(value) {
    this.data = value || 0;
}

function EIBInt32(value) {
    this.data = value || 0;
}

function EIBConnection() {
    this.data = [];
    this.readlen = 0;
    this.datalen = 0;
    this.fd = null;
    this.errno = 0;
    this.__complete = null;
}

EIBConnection.prototype.EIBSocketLocal = function(path) {
    if (this.fd != null) {
      this.errno = errno.EUSERS;
      return -1
    }
    fd = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    fd.connect(path)
    this.data = []
    this.readlen = 0
    this.fd = fd
    return 0
}

EIBConnection.prototype.EIBSocketRemote = function(host, port = 6720) {
    if (this.fd != null) {
      this.errno = errno.EUSERS
      return -1
    fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    fd.connect((host, port))
    this.data = []
    this.readlen = 0
    this.fd = fd
    return 0

}

EIBConnection.prototype.EIBSocketURL = function(url) {
    if (url[0:6] == 'local:') {
      return this.EIBSocketLocal(url[6:])
    if (url[0:3] == 'ip:') {
      parts=url.split(':')
      if (len(parts) == 2) {
        parts.append(6720)
      return this.EIBSocketRemote(parts[1], int(parts[2]))
    this.errno = errno.EINVAL
    return -1

}

EIBConnection.prototype.EIBComplete = function() {
    if (this.__complete == null) {
      this.errno = errno.EINVAL
      return -1
    }
    return this.__complete()

}

EIBConnection.prototype.EIBClose = function() {
    if (this.fd == null) {
      this.errno = errno.EINVAL
      return -1
    this.fd.close()
    this.fd = null

}

EIBConnection.prototype.EIBClose_sync = function() {
    this.EIBReset()
    return this.EIBClose()

}

EIBConnection.prototype.__EIB_SendRequest = function(data) {
    if (this.fd == null) {
      this.errno = errno.ECONNRESET
      return -1
    if (len(data) < 2 or len(data) > 0xffff) {
      this.errno = errno.EINVAL
      return -1
    data = [ (len(data)>>8)&0xff, (len(data))&0xff ] + data;
    result = '';
    for (i in data)       result += chr(i);
    this.fd.send(result);
    return 0;

}

EIBConnection.prototype.EIB_Poll_FD = function() {
    if (this.fd == null) {
      this.errno = errno.EINVAL
      return -1
    }
    return this.fd;
}

EIBConnection.prototype.EIB_Poll_Complete = function() {
    if (this.__EIB_CheckRequest(False) == -1)  return -1;
    if (this.readlen < 2 or (this.readlen >= 2 & this.readlen < this.datalen + 2)) return 0;
    return 1;
}

EIBConnection.prototype.__EIB_GetRequest = function() {
    while (true) {
      if (this.__EIB_CheckRequest(true) == -1) {
        return -1
	  }
      if ((this.readlen >= 2) && (this.readlen >= this.datalen + 2)) {
        this.readlen = 0;
        return 0;
      }
    }
}

EIBConnection.prototype.__EIB_CheckRequest = function(block) {
    if (this.fd == null) {
      this.errno = errno.ECONNRESET
      return -1
    }
    if (this.readlen == 0) {
      this.head = []
      this.data = []
    }
    if (this.readlen < 2) {
      this.fd.setblocking(block)
      result = this.fd.recv (2-this.readlen)
      for (a in result)         this.head.append(ord(a))
      this.readlen += len(result)
    }
    if (this.readlen < 2) {
      return 0;
    }
    this.datalen = (this.head[0] << 8) | this.head[1]);
    if (this.readlen < this.datalen + 2) {
      this.fd.setblocking(block);
      result = this.fd.recv (this.datalen + 2 -this.readlen)
      for (a in result)         this.data.append(ord(a))
      this.readlen += len(result);
    }
    return 0;
}