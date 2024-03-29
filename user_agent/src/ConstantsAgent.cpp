
/*! \file ConstantsAgent.cpp

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia.

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
	here all string and numeric constants for the netagent toolset are declared

	$Id: ConstantsAgent.cpp 748 2015-07-23 9:48:00 amarentes $

*/

#include "stdincpp.h"
#include "ConstantsAgent.h"

using namespace std;

namespace auction
{

// Agent.h
const string AGNT_DEFAULT_CONFIG_FILE = DEF_SYSCONFDIR "/netagnt.conf.xml";
const string AGNT_LOCK_FILE			  = DEF_SYSCONFDIR "/netagent.pid";

// CtrlComm.cpp
const string AGNT_REPLY_TEMPLATE  = DEF_SYSCONFDIR "/reply.xml";   //!< html response template
const string AGNT_MAIN_PAGE_FILE  = DEF_SYSCONFDIR "/main_agnt.html";
const string AGNT_XSL_PAGE_FILE   = DEF_SYSCONFDIR "/reply.xsl";
const int    AGNT_EXPIRY_TIME     = 3600; 		  //!< expiry time for web pages served from cache
const int    AGNT_DEF_PORT        = 12246;         //!< default TCP port to connect to

// Logger.h
extern const string AGNT_DEFAULT_LOG_FILE = DEF_STATEDIR "/log/netagent.log";


// ConfigParser.h
const string AGNT_CONFIGFILE_DTD  = DEF_SYSCONFDIR "/netagnt.conf.dtd";

#ifdef USE_SSL
// certificate file location (SSL)
const string QOS_CERT_FILE = DEF_SYSCONFDIR "/netagent.pem";
#endif

// ResourceRequestFileParser.cpp
const string RESOURCE_FILE_DTD = DEF_SYSCONFDIR "/resourcerequestfile.dtd";

};
