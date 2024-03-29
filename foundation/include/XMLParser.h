
/*!  \file   XMLParser.h

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

    This file is part of Network Auction Manager System (NETAUM).

    NETAUM is free software; you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    NETAUM is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this software; if not, write to the Free Software 
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Description:
    parse XML files, base class for specific parsers

    $Id: XMLParser.h 748 2015-03-03 12:48:00Z amarentes $
*/

#ifndef _XMLPARSER_H_
#define _XMLPARSER_H_


#include "stdincpp.h"
#include "libxml/parser.h"
#include "Logger.h"

namespace auction
{

class XMLParser
{

  private: 
    Logger *log;
    int ch;

    //! xml file name
    string fileName;

    //! corresponding dtd file name
    string dtdName;

    static string err, warn;

    //! callback for parser errors
    static void XMLErrorCB(void *ctx, const char *msg, ...);

    //! callback for parser warnings
    static void XMLWarningCB(void *ctx, const char *msg, ...);

    //! validates doc vs. dtd
    void validate(string root);

  protected:

    //! pointer to the root of the doc
    xmlDocPtr XMLDoc;

    //! name space
    xmlNsPtr ns;
  
  public:

    XMLParser(string dtdname, string fname, string root);

    XMLParser(string dtdname, char *buf, int len, string root);

    ~XMLParser();
	
    string xmlCharToString(xmlChar *in);
    
    inline string getFileName() { return fileName; }
    
    inline string getDtdName() { return dtdName; }

};

} // namespace auction


#endif // _XMLPARSER_H_
