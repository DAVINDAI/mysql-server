/*
   Copyright (c) 2000, 2012, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
*/


/* yaSSL externel header defines yaSSL API
 */


#ifndef yaSSL_EXT_HPP
#define yaSSL_EXT_HPP


namespace yaSSL {


#ifdef _WIN32
    typedef unsigned int SOCKET_T;
#else
    typedef int          SOCKET_T;
#endif


class Client {
public:
    Client();
    ~Client();

    // basics
    int Connect(SOCKET_T);
    int Write(const void*, int);
    int Read(void*, int);

    // options
    void SetCA(const char*);
    void SetCert(const char*);
    void SetKey(const char*);
private:
    struct ClientImpl;
    ClientImpl* pimpl_;

    Client(const Client&);              // hide copy
    Client& operator=(const Client&);   // and assign  
};


class Server {
public:
    Server();
    ~Server();

    // basics
    int Accept(SOCKET_T);
    int Write(const void*, int);
    int Read(void*, int);

    // options
    void SetCA(const char*);
    void SetCert(const char*);
    void SetKey(const char*);
private:
    struct ServerImpl;
    ServerImpl* pimpl_;

    Server(const Server&);              // hide copy
    Server& operator=(const Server&);   // and assign
};


} // namespace yaSSL
#endif // yaSSL_EXT_HPP
