
/*!\file   CtrlComm.cc

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia.

    This file is part of Network Quality Manager System (NETAUM).

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
	auction control interface

$Id: CtrlComm.cpp 748 2015-07-31 9:25:00 amarentes $
*/

#include "ParserFcts.h"
#include "Error.h"
#include "Logger.h"
#include "AuctionTimer.h"
#include "CtrlComm.h"
#include "AuctionManager.h"
#include "ConstantsAgent.h"
#include "EventAgent.h"
#include "anslp_ipap_xml_message.h"
#include "anslp_ipap_message.h"
#include "anslp_ipap_exception.h" 


using namespace std;
using namespace auction;


CtrlComm *CtrlComm::s_instance = NULL;
const int CNTRL_AGNT_MIN_TIMEOUT = 100000;


/* ------------------------- CtrlComm ------------------------- */

CtrlComm::CtrlComm(auction::ConfigManager *cnf, int threaded)
    : AuctionManagerComponent(cnf, "CtrlComm",threaded),
      portnum(0), flags(0)
{
    int ret;

#ifdef DEBUG
    log->dlog(ch, "Starting" );
#endif
  
    portnum = auction::AGNT_DEF_PORT;
	
    // get port number from config manager if available
    string txt = cnf->getValue("ControlPort", "CONTROL");
    if (txt != "") {
        portnum = atoi(txt.c_str());
    }
	 
    if (portnum <= 1024 || portnum > 65536) {
        throw Error("CtrlComm: illegal port number %d "
                    "(must be 1023<x<65535)", portnum );
    }
    
    log->log(ch, "listening on port number %d.", portnum);

    // set this *before* trying to start httpd
    s_instance = this;
    
    // register callback functions
    httpd_register_access_check (s_access_check);
    httpd_register_log_request  (s_log_request);
    httpd_register_log_error    (s_log_error);
    httpd_register_parse_request(s_parse_request);

    // init server
#ifdef USE_SSL
    ret = httpd_init(portnum, "NetAgent", 
                     cnf->isTrue("UseSSL", "CONTROL"),
                     CERT_FILE.c_str(), SSL_PASSWD, 
                     cnf->isTrue("UseIPv6", "CONTROL"));
#else
    ret = httpd_init(portnum, "NetAgent", 
                     0, NULL, NULL, 
                     cnf->isTrue("UseIPv6", "CONTROL"));
#endif
    
    if (ret < 0) {
        throw Error("CtrlComm: cannot init communication facilities" );
    }
    addFd(ret);

    // adds to accessList in CtrlComm all IP addr for hosts known by DNS
    checkHosts( cnf->getAccessList(), cnf->isTrue("UseIPv6", "CONTROL") );
    
    if (cnf->isTrue("LogOnConnect", "CONTROL")) {
        flags |= LOG_CONNECT;
    }
    
    if (cnf->isTrue("LogAumCommand", "CONTROL")) {
        flags |= LOG_COMMAND;
    }
    
    // load html/xsl files
    pcache.addPageFile("/",             auction::AGNT_MAIN_PAGE_FILE );
    pcache.addPageFile("/help",         auction::AGNT_MAIN_PAGE_FILE );
    pcache.addPageFile("/xsl/reply.xsl", auction::AGNT_XSL_PAGE_FILE );

    // load reply template
    string line;
    ifstream in(auction::REPLY_TEMPLATE.c_str());
    
    if (!in) {
        throw Error("cannot access/read file '%s'", auction::AGNT_REPLY_TEMPLATE.c_str());
    }

    while (getline(in, line)) {
        rtemplate += line;
        rtemplate += "\n";
    }

}


/* ------------------------- ~CtrlComm ------------------------- */

CtrlComm::~CtrlComm()
{
#ifdef DEBUG
    log->dlog(ch, "Shutdown" );
#endif

    httpd_shutdown();
}


/* -------------------- accessCheck -------------------- */

// this function is called either with host!=NULL OR user!=NULL
int CtrlComm::accessCheck(char *host, char *user)
{
    // FIXME ugly structure - both, hosts and users stored in one access list member
    
#ifdef DEBUG
    if (host != NULL) {
        cerr << "checking access permission for host " << host << endl;
    } else if (user != NULL) {
        cerr << "checking access permission for user " << user << endl;
    }
#endif

    auction::configADListIter_t iter;
    // host check
    if (host != NULL) {
        for (iter = accessList.begin(); iter != accessList.end(); iter++) {
            if ((iter->type == "Host") &&
                ((iter->value == "All") ||
                 (!strcmp(iter->resolve_addr.c_str(), host)))) {
                std::cout << "returning ok" << std::endl;
                return ((iter->ad == auction::ALLOW) ? 0 : -1);
            }
        }
    }
        
    // user check
    // FIXME no encrypted password support yet!
    if (user != NULL) {
        for (iter = accessList.begin(); iter != accessList.end(); iter++) {
			
			std::cout << "user:" << iter->value.c_str() << std::endl;
            if ((iter->type == "User") &&
                ((iter->value == "All") ||
                 (!strncmp(iter->value.c_str(), user,
                           strlen(iter->value.c_str()))))) {
                return ((iter->ad == auction::ALLOW) ? 0 : -1);
            }
        }
    }

    return -1; // no match found -> DENY
}


/* -------------------- checkHosts -------------------- */

void CtrlComm::checkHosts( auction::configADList_t &list, bool useIPv6 )
{
    int rc;
    auction::configADItem_t entry;
    auction::configADListIter_t iter;
    struct addrinfo ask, *tmp, *res = NULL;
    
    sethostent(1);
    
    memset(&ask,0,sizeof(ask));
    ask.ai_socktype = SOCK_STREAM;
    ask.ai_flags = 0;
    
    for (iter = list.begin(); iter != list.end(); iter++) {
        if (iter->type == "Host") {
            if (iter->value == "All") { // always copy if host name == 'All'
                accessList.push_back(*iter);
            } else {
                // set timeout
                g_timeout = 0;
                alarm(2);

                // if hostname = 'localhost' valgrind reports
                // an uninitialized value error
                rc = getaddrinfo((iter->value).c_str(), NULL, &ask, &res);
                
                alarm(0);

                if (g_timeout) {
                    freeaddrinfo(res);
                    throw Error("check hosts on access list: DNS timeout");
                }
                
                if (rc == 0) {
                    auction::configADItem_t entry = *iter;		    
                    char host[65];
                    bool isIPv4addr;
                    
                    tmp = res;
                    while(tmp) {
                        if (getnameinfo(tmp->ai_addr, tmp->ai_addrlen, host,
                                        64, NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV) < 0) {
                            freeaddrinfo(res);
                            throw Error("getnameinfo");
                        }
                        
                        isIPv4addr = strchr(host, '.') && !strchr(host, ':');
                        
                        // check every cobination of isIPv4addr (false/true) and useIPv6 (false/true)
                        if (!useIPv6) {
                            if (isIPv4addr) { // isv4addr && usev4listen
                                entry.resolve_addr = host;
                                accessList.push_back(entry);
                            } else {                          // isv6addr && usev4listen
                                log->wlog(ch, "ignoring allowed host with IPv6 address '%s' (only"
                                          " listening to IPv4 control connects)", host);
                            }
                        } else {
                            if (isIPv4addr) {                 // isv4addr && usev6listen
                                // add v4 address also in case that host has no v6 address (v4 fallback)
                                entry.resolve_addr = host;
                                accessList.push_back(entry);
#ifdef DEBUG
                                if (!entry.resolve_addr.empty()) {
                                    cerr << "adding allowed control host '" << entry.resolve_addr 
                                         << "' to access list." << endl;
                                }
#endif
                                // add IPv4 in IPv6 encapsulated address for access list
                                entry.resolve_addr = string("::ffff:") + host;
                                accessList.push_back(entry);
                            } else {                          // isv6addr && usev6listen
                                entry.resolve_addr = host;
                                accessList.push_back(entry);
                            }
                        }
#ifdef DEBUG
                        if (!entry.resolve_addr.empty()) {
                            cerr << "adding allowed control host '" << entry.resolve_addr 
                                 << "' to access list." << endl;
                        }
#endif
                        tmp = tmp->ai_next;
                    }
                } else {
                    log->wlog(ch, "removing unknown host '%s' from access list",
                              (iter->value).c_str() );
                }

                if (res != NULL) {
                    freeaddrinfo(res);
                    res = NULL;
                }
            }
        } else if (iter->type == "User") {
            // copy all 'User' items in list to accessList
            accessList.push_back(*iter);
        } else {
            // ignore all 'non-Host && non-User' items in list
        }
    }
#ifdef DEBUG2
    cerr << accessList;
#endif
    endhostent();
}


/* -------------------- sendMsg -------------------- */

string CtrlComm::xmlQuote(string s)
{
    string res;
   
    for( string::iterator i = s.begin(); i != s.end(); i++ ) {
        switch (*i) {
        case '<':
            res.append( "&lt;" );
            break;
        case '>':
            res.append( "&gt;" );
            break;
        case '&':
            res.append( "&amp;" );
            break;
            /*
              case '\n':
              res.append( "<br/>" );
              break;
            */
            
        default:
            res.push_back(*i);
        }
    }
    
    return res;
}


void CtrlComm::sendMsg(string msg, struct REQUEST *req, fd_sets_t *fds, int quote)
{
    string rep = rtemplate;

    rep.replace(rep.find("@STATUS@"), strlen("@STATUS@"), "OK");
    rep.replace(rep.find("@MESSAGE@"), strlen("@MESSAGE@"), (quote) ? xmlQuote(msg) : msg);
#ifdef DEBUG2
    cerr << rep;
#endif
    req->body = strdup(rep.c_str());
    req->mime = "text/xml";
    httpd_send_response(req, fds);
}


void CtrlComm::sendErrMsg(string msg, struct REQUEST *req, fd_sets_t *fds)
{
    string rep = rtemplate;

    rep.replace(rep.find("@STATUS@"), strlen("@STATUS@"), "Error");
    rep.replace(rep.find("@MESSAGE@"), strlen("@MESSAGE@"), xmlQuote(msg));
#ifdef DEBUG2
    cerr << rep;
#endif
    req->body = strdup(rep.c_str());
    req->mime = "text/xml";
    if (fds != NULL) {
        httpd_send_response(req, fds);
    } else {
        httpd_send_immediate_response(req);
    }
}


/* -------------------- handleFDEvent -------------------- */

int CtrlComm::handleFDEvent(auction::eventVec_t *e, fd_set *rset, fd_set *wset, fd_sets_t *fds)
{

#ifdef DEBUG
    log->dlog(ch, "Starting handleFDEvent" );
#endif


    assert(e != NULL);

    // make the pointer global for ctrlcomm
    retEventVec = e;
    retEvent = NULL;
    
	int http_handle = httpd_handle_event(rset, wset, fds);
	
	if (http_handle < 0) {
		cout << "ERROR HANDLING THE EVENT" << endl;
		throw Error("ctrlcomm handle event error");
	}
	
#ifdef DEBUG
    log->dlog(ch, "Ending handleFDEvent" );
#endif		
		
    // return resulting event (freed by event scheduler)
    return 0;



}


/* -------------------- processCmd -------------------- */

parseReq_t CtrlComm::parseRequest(struct REQUEST *req)
{
    parseReq_t preq;

#ifdef DEBUG
    log->dlog(ch, "Start parseRequest");
#endif


    preq.comm = req->path;

    // parse headers
    struct strlist *hdr = req->header;
    while (hdr) {
        string l = hdr->line;
        int p = l.find(":");
        if (p > 0) {
            preq.params[l.substr(0,p)] = l.substr(p+1, l.length());
        }
        hdr = hdr->next;
    }

    // parse URL parameters
    if (req->query != NULL) {
        string q = req->query;
    
        int p1 = 0, p2 = q.length(), p3 = 0;
        while((p2 = q.find("&",p1)) > 0) {
            cerr << p2 << endl;
            p3 = q.find("=",p1);
            if (p3 > 0) {
                preq.params[q.substr(p1,p3-p1)] = q.substr(p3+1, p2-p3-1);
            }

            p1 = p2+1;
        } 
        p3 = q.find("=",p1);
        if (p3 > 0) {
            preq.params[q.substr(p1,p3-p1)] = q.substr(p3+1, p2-p3-1);
        }
    
    }

    // parse POST parameters in body
    if (req->lbreq != 0) {
        string q = req->post_body;
    
        int p1 = 0, p2 = q.length(), p3 = 0;
        while((p2 = q.find("&",p1)) > 0) {
            p3 = q.find("=",p1);
            if (p3 > 0) {
                preq.params[q.substr(p1,p3-p1)] = q.substr(p3+1, p2-p3-1);
            }

            p1 = p2+1;
        } 
        p3 = q.find("=",p1);
        if (p3 > 0) {
            preq.params[q.substr(p1,p3-p1)] = q.substr(p3+1, p2-p3-1);
        }
    
    }

#ifdef DEBUG
    log->dlog(ch, "End parseRequest");

    for (paramListIter_t iter=preq.params.begin(); iter != preq.params.end(); iter++) {
        log->dlog(ch, "%s :: %s", (iter->first).c_str(), (iter->second).c_str());
    }
#endif

    return preq;
}


int CtrlComm::processCmd(struct REQUEST *req)
{
    parseReq_t preq;
    
#ifdef PROFILING
    unsigned long long ini, end;
#endif

#ifdef PROFILING
    ini = auction::AuctionTimer::readTSC();
#endif

#ifdef DEBUG
    log->dlog(ch, "client requested cmd:%s Params: %s Body:%s", 
						req->path, req->query, req->post_body);
#endif
		
    if (isEnabled(LOG_COMMAND)) {
        log->log(ch, "client requested cmd: '%s' and params '%s%s'", req->path, req->query,
                 req->post_body);
    }

    if ((req->path == NULL) || (strlen(req->path) == 0)) {
        return -1;
    }

    preq = parseRequest(req);


    // try lookup in repository of static auction pages
    string page = pcache.getPage(preq.comm); 

#ifdef DEBUG
    log->dlog(ch, "Command:%s Page:%s", (preq.comm).c_str(), page.c_str() );
#endif	


    try {
        if (page != "") {

			
            // send page
            req->body = strdup(page.c_str());  // FIXME not very performant
            req->mime = get_mime((char *) pcache.getFileName(preq.comm).c_str());
            // generate Expires header
            req->lifespan = auction::AGNT_EXPIRY_TIME;
            // immediatly send response
            httpd_send_immediate_response(req);
            
            // FIXME  better register those callback funtions onto the command name
        } else if (preq.comm == "/get_info") {
            processGetInfo(&preq);
        } else if (preq.comm == "/add_request") {
            processAddRequest(&preq);
        } else {
            // unknown command will produce a 404 http error
            return -1;
        }
    } catch (Error &e) {
        sendErrMsg(e.getError(), req, NULL);
        return 0;
    }
    
#ifdef PROFILING
    end = auction::AuctionTimer::readTSC();
   
    cerr << "parse cmd in " << auction::AuctionTimer::ticks2ns(end-ini) << " ns" << endl;
#endif

    if (retEvent != NULL) {
        retEvent->setReq(req);
        retEventVec->push_back(retEvent);
    }

#ifdef DEBUG
    log->dlog(ch, "ending processCmd");
#endif	


    return 0;
}


/* ----------------- processResponseSessionCreate -------------------- */

char *CtrlComm::processAddRequest(parseReq_t *preq)
{

#ifdef DEBUG
    log->log(ch, "starting processAddRequest");
#endif
	    
    paramListIter_t message = preq->params.find("Message");
    if (message == preq->params.end()) {
        throw Error("processResponseSessionCreate: missing parameter 'Message'" );
    }

	retEvent = new auction::AddResourceRequestsCtrlCommEvent(message->second);

#ifdef DEBUG
		log->log(ch, "ending processResponseSessionCreate");
#endif
	    
    return NULL;

}


/* ------------------------- processGetInfo ------------------------- */

char *CtrlComm::processGetInfo( parseReq_t *preq )
{
    auction::AgentManagerInfo infos;

    paramListIter_t type = preq->params.find("IType");
    paramListIter_t param = preq->params.find("IParam");
    
    if (type == preq->params.end()) {
        throw Error("get_info: missing parameter 'IType'" );
    }
   
    if (param == preq->params.end()) {
        infos.addInfo(type->second);
    } else { 
        infos.addInfo(type->second, param->second);
    }

    retEvent = new auction::GetInfoEvent(infos.getList());
    
    return NULL;
}

/* -------------------- logAccess -------------------- */

int CtrlComm::logAccess(struct REQUEST *req, time_t now )
{
  
    if (isEnabled(LOG_CONNECT)) {
        if (0 == req->status) {
            req->status = 400; /* bad request */
        }
        if (400 == req->status) {
            log->log(ch,"%s - - \"-\" 400 %d",
                     req->peerhost,
                     req->bc);
        } else {
            log->log(ch,"%s - - \"%s %s HTTP/%d.%d\" %d %d",
                     req->peerhost,
                     req->type,
                     req->uri,
                     req->major,
                     req->minor,
                     req->status,
                     req->bc);
        }
    }

    return 0;
}


int CtrlComm::logError(int eno, int loglevel, char *txt, char *peerhost)
{
    char buf[256];
  
    buf[0] = '\0';

    // FIXME loglevel is currently ignored
    if (eno) {
        if (peerhost) {
            sprintf(buf,"%s: %s (peer=%s)", txt, strerror(errno), peerhost);
        } else {
            sprintf(buf,"%s: %s", txt, strerror(errno));
        }
    } else {
        if (peerhost) {
            sprintf(buf,"%s (peer=%s)", txt, peerhost);
        } else {
            sprintf(buf,"%s", txt);
        }
    }

    log->elog(ch,"%s", buf);

    return 0;
}


/* ------------------------- dump ------------------------- */

void CtrlComm::dump( ostream &os )
{
    os << "CtrlComm dump : listening on port " << portnum  << endl ;
}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, CtrlComm &cio )
{
    cio.dump(os);
    return os;
}
