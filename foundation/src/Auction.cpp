
/*! \file   Auction.cpp

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
    Auctions in the system.

    $Id: Auction.cpp 748 2015-07-23 9:58:00Z amarentes $
*/


#include <sstream>
#include <assert.h>

#include "ParserFcts.h"
#include "Auction.h"
#include "Error.h"
#include "IpAp_def.h"
#include "IpAp_message.h"

using namespace auction;

/* ------------------------- Auction ------------------------- */

Auction::Auction(time_t now, string sname, string aname, action_t &a, 
				 miscList_t &m, auctionTemplateFieldList_t &templFields,
				 ipap_message *message )
   : state(AS_NEW), auctionName(aname), resource(), setName(sname), action(a), 
	 miscList(m)
{

    unsigned long duration;

    log = Logger::getInstance();
    ch = log->createChannel("Auction");
    
#ifdef DEBUG
    log->dlog(ch, "Auction constructor");
#endif    

    try {
	
        parseAuctionName(aname);
	
        if (aname.empty()) {
            // we tolerate an empty sname but not an empty rname
            throw Error("missing auction identifier value in rule description");
        }
        if (sname.empty()) {
            sname = DEFAULT_SETNAME;
        }
		
        /* time stuff */
        start = now;
        // stop = 0 indicates infinite running time
        stop = 0;
        // duration = 0 indicates no duration set
        duration = 0;
	    
        // get the configured values
        string sstart = getMiscVal("Start");
        string sstop = getMiscVal("Stop");
        string sduration = getMiscVal("Duration");
        string sinterval = getMiscVal("Interval");
        string salign = getMiscVal("Align");
	    
        if (!sstart.empty() && !sstop.empty() && !sduration.empty()) {
            throw Error(409, "illegal to specify: start+stop+duration time");
        }
	
        if (!sstart.empty()) {
            start = parseTime(sstart);
            if(start == 0) {
                throw Error(410, "invalid start time %s", sstart.c_str());
            }
        }

        if (!sstop.empty()) {
            stop = parseTime(sstop);
            if(stop == 0) {
                throw Error(411, "invalid stop time %s", sstop.c_str());
            }
        }
	
        if (!sduration.empty()) {
            duration = ParserFcts::parseULong(sduration);
        }

        if (duration) {
            if (stop) {
                // stop + duration specified
                start = stop - duration;
            } else {
                // stop [+ start] specified
                stop = start + duration;
            }
        }
	
        // now start has a defined value, while stop may still be zero 
        // indicating an infinite rule
	    
        // do we have a stop time defined that is in the past ?
        if ((stop != 0) && (stop <= now)) {
            throw Error(300, "task running time is already over");
        }
	
        if (start < now) {
            // start late tasks immediately
            start = now;
        }
		
        // get export module params
        int interval = 0;
		if (!sinterval.empty()) {
			interval = ParserFcts::parseInt(sinterval);
        }
        int align = (!salign.empty()) ? 1 : 0;
				
	    if (interval > 0) {
           	// add to intervals list
           	interval_t ientry;

           	ientry.interval = interval;
           	ientry.align = align;
			
#ifdef DEBUG
    log->dlog(ch, "Interval: %d - Align:%d", interval, align);
#endif    
			
           	mainInterval = ientry;
	    }
	    
	    // Iterate over the field templates to calculate the number of 
	    // fields in the template bid and template allocation.
	    auctionTemplateFieldListIter_t fieldIter;
	    int numBidFields = 0;
	    int numAllocFields = 0;
	    int numOptBidFields = 0;
	    for (fieldIter = templFields.begin(); fieldIter != templFields.end();  ++fieldIter)
	    {
			if ((fieldIter->second).isBidtemplate){ numBidFields++; }
			if ((fieldIter->second).isOptBidTemplate){ numOptBidFields++; }
			if ((fieldIter->second).isAllocTemplate){ numAllocFields++; }
		} 

#ifdef DEBUG
    log->dlog(ch, "bid template fields: %d - opt bid template fields:%d \
					- allocation template fields:%d", 
						numBidFields, numOptBidFields, numAllocFields);
#endif  
		
	    // Create the bid template associated with the auction
	    uint16_t templId = message->new_data_template(numBidFields, IPAP_SETID_BID_TEMPLATE);
	    for (fieldIter = templFields.begin(); fieldIter != templFields.end(); ++fieldIter)
	    {
			if ((fieldIter->second).isBidtemplate){
				message->add_field(templId, (fieldIter->second).field.eno,
											(fieldIter->second).field.ftype);
			}
		} 

		// Add a reference to the template.
		templates.add_template(message->get_template_object(templId));

#ifdef DEBUG
		log->dlog(ch, "template Bid Id %d", templId);
#endif  

		// The user can create an auction without option field templates.
		if (numOptBidFields > 0){
			// Create the bid options template associated with the auction.
			uint16_t templOptId = message->new_data_template(numOptBidFields,
									IPAP_OPTNS_BID_TEMPLATE);
								
			for (fieldIter = templFields.begin(); fieldIter != templFields.end(); ++fieldIter)
			{
				if ((fieldIter->second).isOptBidTemplate){
					message->add_field(templOptId, (fieldIter->second).field.eno,
												   (fieldIter->second).field.ftype);
				}
			} 
			// Add a reference to the template.
			templates.add_template(message->get_template_object(templOptId));

#ifdef DEBUG
			log->dlog(ch, "template Option Bid Id %d NumFields:%d", templOptId, numOptBidFields);
#endif 
		}
		else{
#ifdef DEBUG
			log->dlog(ch, "Without fields to define the option Bid template");
#endif 		
		}	
		// create the allocation template associated with the auction
		uint16_t templAllocId = message->new_data_template(numAllocFields, 
							IPAP_SETID_ALLOCATION_TEMPLATE);
							
	    for (fieldIter = templFields.begin(); fieldIter != templFields.end(); ++fieldIter)
	    {
			if ((fieldIter->second).isAllocTemplate){
				message->add_field(templAllocId, (fieldIter->second).field.eno,
											(fieldIter->second).field.ftype);			
			}
		} 

#ifdef DEBUG
		log->dlog(ch, "template Allocation Id %d", templAllocId);
#endif  

		
		// Add a reference to both templates.
		templates.add_template(message->get_template_object(templAllocId));

#ifdef DEBUG
		log->dlog(ch, "Ending Constructor Auction");
#endif 

    } catch(ipap_bad_argument &e){
		throw Error("Auction %s.%s: %s", sname.c_str(), aname.c_str(), e.what());	
    } catch (Error &e) {    
        state = AS_ERROR;
        throw Error("Auction %s.%s: %s", sname.c_str(), aname.c_str(), e.getError().c_str());
    }

}

Auction::Auction(const Auction &rhs): 
	state(rhs.state), auctionName(rhs.auctionName), setName(rhs.setName), 
	 resource(rhs.resource)/*,action(),intervals(), miscList()*/
{

    log = Logger::getInstance();
    ch = log->createChannel("Auction");
	
	start = rhs.start;
	stop = rhs.stop;

	// Copy action
	action.name = rhs.action.name;
	action.defaultAct = rhs.action.defaultAct;
	configItemListConstIter_t iter;
	for (iter = (rhs.action.conf).begin(); iter != (rhs.action.conf).end(); ++iter)
	{
		cout << "Entro configured item action" << endl;
		configItem_t item;
		item.group = iter->group;
		item.module = iter->module;
		item.name = iter->name;
		item.value = iter->value;
		item.type = iter->type;
		action.conf.push_back(item);
	} 
	
	
	// Copy the main interval
	mainInterval = rhs.mainInterval;
		
	// Copy miscList
	miscListConstIter_t misc_it;
	miscList.clear();
	for (misc_it = (rhs.miscList).begin(); misc_it != (rhs.miscList).end(); ++misc_it)
	{
		configItem_t item;
		item.group = (misc_it->second).group;
		item.module = (misc_it->second).module;
		item.name = (misc_it->second).name;
		item.value = (misc_it->second).value;
		item.type = (misc_it->second).type;
		miscList.insert( std::pair<string,configItem_t>(item.name,item) );
	}
	
}


/* ------------------------- ~Auction ------------------------- */

Auction::~Auction()
{
#ifdef DEBUG
    log->dlog(ch, "Auction destructor Id: %d", uid);
#endif

}

/* functions for accessing the templates */
string Auction::getMiscVal(string name)
{
    miscListIter_t iter;

    iter = miscList.find(name);
    if (iter != miscList.end()) {
        return iter->second.value;
    } else {
        return "";
    }
}


void Auction::parseAuctionName(string rname)
{
    int n;

    if (rname.empty()) {
        throw Error("malformed auction identifier %s, "
                    "use <identifier> or <source>.<identifier> ",
                    rname.c_str());
    }

    if ((n = rname.find(".")) > 0) {
        resource = rname.substr(0,n);
        id = rname.substr(n+1, rname.length()-n);
    } else {
        // no dot so everything is recognized as id
        id = rname;
    }

}


/* ------------------------- parseTime ------------------------- */

time_t Auction::parseTime(string timestr)
{
    struct tm  t;
  
    if (timestr[0] == '+') {
        // relative time in secs to start
        try {
	    struct tm tm;
            int secs = ParserFcts::parseInt(timestr.substr(1,timestr.length()));
            time_t start = time(NULL) + secs;
            return mktime(localtime_r(&start,&tm));
        } catch (Error &e) {
            throw Error("Incorrect relative time value '%s'", timestr.c_str());
        }
    } else {
        // absolute time
        if (timestr.empty() || (strptime(timestr.c_str(), TIME_FORMAT.c_str(), &t) == NULL)) {
            return 0;
        }
    }
    return mktime(&t);
}

/* ------------------------- getActions ------------------------- */


action_t *Auction::getAction()
{
    return &action;
}


/* ------------------------- getMisc ------------------------- */


miscList_t *Auction::getMisc()
{
    return &miscList;
}

/* ------------------- getTemplateList ---------------------- */

ipap_template_container * Auction::getTemplateList()
{
	return &templates;
}

/* --------------- getTemplateAssociations ------------------ */

remoteAssociationTemplateList_t * Auction::getTemplateAssociations()
{
	return &templateAssociationList;
}


/* ------------------------- dump ------------------------- */

void Auction::dump( ostream &os )
{
    os << "Rule dump :" << endl;
    os << getInfo() << endl;
  
}

/* ------------------------- getInfo ------------------------- */

string Auction::getInfo(void)
{
    ostringstream s;

    s << getSetName() << "." << getAuctionName() << " ";

	s << getStart() << " & " << getStop() << " ";

    switch (getState()) {
    case AS_NEW:
        s << "new";
        break;
    case AS_VALID:
        s << "validated";
        break;
    case AS_SCHEDULED:
        s << "scheduled";
        break;
    case AS_ACTIVE:
        s << "active";
        break;
    case AS_DONE:
        s << "done";
        break;
    case AS_ERROR:
        s << "error";
        break;
    default:
        s << "unknown";
    }

    s << ": ";
	
    s << action.name << " default:";
    s << action.defaultAct;

	configItemListIter_t iter;
	for (iter = action.conf.begin(); iter != action.conf.end(); ++iter)
	{
		s << "Group:" << iter->group 
		  << " module:" << iter->module
		  << " name:" << iter->name
		  << " value:" << iter->value
		  << " type:" << iter->type;
	}
	
	
    s << " | ";
	
    miscListIter_t mi = miscList.begin();
    while (mi != miscList.end()) {
        s << mi->second.name << " = " << mi->second.value;

        mi++;

        if (mi != miscList.end()) {
            s << ", ";
        }
    }
	
    s << " | ";


	s << "Interval:" << (mainInterval).interval << "align:" << (mainInterval).align;
			
    s << endl;

    return s.str();
}

