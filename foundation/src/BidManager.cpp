
/*! \file   BidManager.cpp

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
	bid database
    Code based on Netmate Implementation

    $Id: BidManager.h 748 2015-07-23 14:00:00Z amarentes $

*/

#include "ParserFcts.h"
#include "BidManager.h"
// #include "CtrlComm.h"
#include "Constants.h"

using namespace auction;

/* ------------------------- BidManager ------------------------- */

BidManager::BidManager( string fdname, string fvname) 
    : bids(0), 
	  fieldDefFileName(fdname), fieldValFileName(fvname), idSource(1)
{
    log = Logger::getInstance();
    ch = log->createChannel("BidManager");
    
#ifdef DEBUG
    log->dlog(ch,"Starting");
#endif

    // load the field def list
    loadFieldDefs(fieldDefFileName);
	
    // load the field val list
    loadFieldVals(fieldValFileName);


}


/* ------------------------- ~BidManager ------------------------- */

BidManager::~BidManager()
{
    bidDBIter_t iter;

#ifdef DEBUG
    log->dlog(ch,"Shutdown");
#endif

    for (iter = bidDB.begin(); iter != bidDB.end(); iter++) {
        if (*iter != NULL) {
            // delete rule
            saveDelete(*iter);
        } 
    }

    for (bidDoneIter_t i = bidDone.begin(); i != bidDone.end(); i++) {
        saveDelete(*i);
    }
}


/* -------------------- isReadableFile -------------------- */

static int isReadableFile( string fileName ) {

    FILE *fp = fopen(fileName.c_str(), "r");

    if (fp != NULL) {
        fclose(fp);
        return 1;
    } else {
        return 0;
    }
}

/* -------------------- loadFieldDefs -------------------- */

void BidManager::loadFieldDefs(string fname)
{
    if (fieldDefFileName.empty()) {
        if (fname.empty()) {
            fname = FIELDDEF_FILE;
		}
    } else {
        fname = fieldDefFileName;
    }

#ifdef DEBUG
    log->dlog(ch, "filename %s", fname.c_str());
#endif

    if (isReadableFile(fname)) {
        if (fieldDefs.empty() && !fname.empty()) {
            FieldDefParser f = FieldDefParser(fname.c_str());
            f.parse(&fieldDefs);
        }
    
    }else{
#ifdef DEBUG
    log->dlog(ch, "filename %s is not readable", fname.c_str());
#endif    
    }
    
}

/* -------------------- loadFieldVals -------------------- */

void BidManager::loadFieldVals( string fname )
{
    if (fieldValFileName.empty()) {
        if (fname.empty()) {
            fname = FIELDVAL_FILE;
        }
    } else {
        fname = fieldValFileName;
    }

    if (isReadableFile(fname)) {
        if (fieldVals.empty() && !fname.empty()) {
            FieldValParser f = FieldValParser(fname.c_str());
            f.parse(&fieldVals);
        }
    }
}

/* -------------------------- getRule ----------------------------- */

Bid *BidManager::getBid(int uid)
{
    if ((uid >= 0) && ((unsigned int)uid <= bidDB.size())) {
        return bidDB[uid];
    } else {
        return NULL;
    }
}



/* -------------------- getBid -------------------- */

Bid *BidManager::getBid(string sname, string rname)
{
    bidSetIndexIter_t iter;
    bidIndexIter_t iter2;

    iter = bidSetIndex.find(sname);
    if (iter != bidSetIndex.end()) {		
        iter2 = iter->second.find(rname);
        if (iter2 != iter->second.end()) {
            return getBid(iter2->second);
        }
        else
        {
#ifdef DEBUG
    log->dlog(ch,"BidId not found %s.%s",sname.c_str(), rname.c_str());
#endif		
			
		}
    }
    else
    {
#ifdef DEBUG
    log->dlog(ch,"Bidset not found");
#endif		
	}

    return NULL;
}


/* -------------------- getBids -------------------- */

bidIndex_t *BidManager::getBids(string sname)
{
    bidSetIndexIter_t iter;

    iter = bidSetIndex.find(sname);
    if (iter != bidSetIndex.end()) {
        return &(iter->second);
    }

    return NULL;
}

/* -------------------- getBids -------------------- */
vector<int> BidManager::getBids(string aset, string aname)
{

    auctionSetBidIndexIter_t iter;

    iter = bidAuctionSetIndex.find(aset);
    if (iter != bidAuctionSetIndex.end()) {
        auctionBidIndexIter_t auctionBidIndexIter = (iter->second).find(aname);
        if (auctionBidIndexIter != (iter->second).end()){
			return auctionBidIndexIter->second;
		}
    }
	
	vector<int> list_return;
    return list_return;
	 
}


bidDB_t BidManager::getBids()
{
    bidDB_t ret;

    for (bidSetIndexIter_t r = bidSetIndex.begin(); r != bidSetIndex.end(); r++) {
        for (bidIndexIter_t i = r->second.begin(); i != r->second.end(); i++) {
            ret.push_back(getBid(i->second));
        }
    }

    return ret;
}

/* ----------------------------- parseBids --------------------------- */

bidDB_t *BidManager::parseBids(string fname)
{

#ifdef DEBUG
    log->dlog(ch,"ParseBids");
#endif

    bidDB_t *new_bids = new bidDB_t();

    try {	
	
        BidFileParser rfp = BidFileParser(fname);
        rfp.parse(&fieldDefs, &fieldVals, new_bids, &idSource);

#ifdef DEBUG
    log->dlog(ch, "bids parsed");
#endif

        return new_bids;

    } catch (Error &e) {

        for(bidDBIter_t i=new_bids->begin(); i != new_bids->end(); i++) {
           saveDelete(*i);
        }
        saveDelete(new_bids);
        throw e;
    }
}


/* -------------------- parseBidsBuffer -------------------- */

bidDB_t *BidManager::parseBidsBuffer(char *buf, int len)
{
    bidDB_t *new_bids = new bidDB_t();

    try {
        // load the filter def list
        loadFieldDefs("");
	
        // load the filter val list
        loadFieldVals("");
			
        BidFileParser rfp = BidFileParser(buf, len);
        rfp.parse(&fieldDefs, &fieldVals, new_bids, &idSource);

        return new_bids;
	
    } catch (Error &e) {

        for(bidDBIter_t i=new_bids->begin(); i != new_bids->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_bids);
        throw e;
    }
}


/* -------------------- parseBidsMessage -------------------- */

bidDB_t *
BidManager::parseBidMessage(ipap_message *messageIn, ipap_template_container *templates)
{
    bidDB_t *new_bids = new bidDB_t();

    try {
			
        MAPIBidParser rfp = MAPIBidParser();
        
        rfp.parse(&fieldDefs, &fieldVals, messageIn, new_bids, templates);

        return new_bids;
	
    } catch (Error &e) {

        for(bidDBIter_t i=new_bids->begin(); i != new_bids->end(); i++) {
            saveDelete(*i);
        }
        saveDelete(new_bids);
        throw e;
    }
}


/* ---------------------------------- addBids ----------------------------- */

void BidManager::addBids(bidDB_t * _bids, EventScheduler *e)
{
    bidDBIter_t        iter;
    bidTimeIndex_t     start;
    bidTimeIndex_t     stop;
    bidTimeIndexIter_t iter2;
    time_t              now = time(NULL);
    
    // add bids
    for (iter = _bids->begin(); iter != _bids->end(); iter++) {
        Bid *b = (*iter);
        
        try {
			bidIntervalList_t intervalList;
            addBid(b);
			b->calculateIntervals(now,  &intervalList);
			
            bidIntervalListIter_t intervIter;
            for ( intervIter = intervalList.begin(); intervIter != intervalList.end(); ++intervIter ){ 
				start[(intervIter->second).start].push_back(b);
				if ((intervIter->second).stop) 
				{
					stop[(intervIter->second).stop].push_back(b);
				}
			}
        } catch (Error &e ) {
            saveDelete(b);
            // if only one rule return error
            if (_bids->size() == 1) {
                throw e;
            }
            // FIXME else return number of successively installed bids
        }
      
    }
    
#ifdef DEBUG    
    log->dlog(ch, "Start all bids - it is going to activate them");
#endif      

    // group bids with same start time
    for (iter2 = start.begin(); iter2 != start.end(); iter2++) {
		bidDBIter_t bids_iter;
		// Iterates over the bids starting.
		for (bids_iter = (iter2->second).begin(); bids_iter != (iter2->second).end(); bids_iter++) 
		{ 
			Bid *bid = (*bids_iter);
					
#ifdef DEBUG    
			log->dlog(ch, "Schedulling insert bid auction event - set: %s, name:  %s", 
						bid->getAuctionSet().c_str(), 
						bid->getAuctionName().c_str());
#endif 					
			e->addEvent( new InsertBidAuctionEvent(iter2->first-now, bid, 
										  bid->getAuctionSet(),
										  bid->getAuctionName()));
		}
    }
    
    // group rules with same stop time
    for (iter2 = stop.begin(); iter2 != stop.end(); iter2++) {
		bidDBIter_t bids_iter;
		// Iterates over the bids starting.
		for (bids_iter = (iter2->second).begin(); bids_iter != (iter2->second).end(); bids_iter++) 
		{
			Bid *bid = (*bids_iter); 
#ifdef DEBUG    
			log->dlog(ch, "Schedulling stop bid auction event - set: %s, name:  %s", 
						bid->getAuctionSet().c_str(), 
						bid->getAuctionName().c_str());
#endif			
			// Iterates over the auctions configured for the bid.
			e->addEvent( new RemoveBidAuctionEvent(iter2->first-now, bid, 
									bid->getAuctionSet(),
									bid->getAuctionName()));
		}
    }

#ifdef DEBUG    
    log->dlog(ch, "Finished adding bids");
#endif      

}


/* -------------------- addBid -------------------- */

void BidManager::addBid(Bid *b)
{
  
#ifdef DEBUG    
    log->dlog(ch, "adding new bid with name = '%s'",
              b->getBidName().c_str());
#endif  
				  
			  
    // test for presence of bidSource/bidName combination
    // in bidDatabase in particular set
    if (getBid(b->getBidSet(), b->getBidName())) {
        log->elog(ch, "bid %s.%s already installed",
                  b->getBidSet().c_str(), b->getBidName().c_str());
        throw Error(408, "Bid with this name is already installed");
    }

    try {

		// Assigns the new Id.
		b->setUId(idSource.newId());

        // could do some more checks here
        b->setState(BS_VALID);

#ifdef DEBUG    
		log->dlog(ch, "Bid Id = '%d'", b->getUId());
#endif 

        // resize vector if necessary
        if ((unsigned int)b->getUId() >= bidDB.size()) {
            bidDB.reserve(b->getUId() * 2 + 1);
            bidDB.resize(b->getUId() + 1);
        }

        // insert bid
        bidDB[b->getUId()] = b; 	

        // add new entry in index
        bidSetIndex[b->getBidSet()][b->getBidName()] = b->getUId();


		string aSet = b->getAuctionSet();
		string aName = b->getAuctionName();
		
		auctionSetBidIndexIter_t setIter;
		setIter = bidAuctionSetIndex.find(aSet);
		if (setIter != bidAuctionSetIndex.end())
		{
			auctionBidIndexIter_t actBidIter = (setIter->second).find(aName);
			if (actBidIter != (setIter->second).end()){
				(actBidIter->second).push_back(b->getUId());
			}
			else{
				vector<int> listBids;
				listBids.push_back(b->getUId());
				bidAuctionSetIndex[aSet][aName] = listBids;
			}
		} else {
			vector<int> listBids;
			listBids.push_back(b->getUId());
			bidAuctionSetIndex[aSet][aName] = listBids;
		}
        
        bids++;

#ifdef DEBUG    
    log->dlog(ch, "finish adding new bid with name = '%s'",
              b->getBidName().c_str());
#endif  

    } catch (Error &e) { 

        // adding new bid failed in some component
        // something failed -> remove bid from database
        delBid(b->getBidSet(), b->getBidName(), NULL);
	
        throw e;
    }
}

void BidManager::activateBids(bidDB_t *bids, EventScheduler *e)
{
    bidDBIter_t             iter;

    for (iter = bids->begin(); iter != bids->end(); iter++) {
        Bid *r = (*iter);
        log->dlog(ch, "activate bid with name = '%s'", r->getBidName().c_str());
        r->setState(BS_ACTIVE);
	 
        /* TODO AM: Evaluate this code to understand if it has to be adjusted or not
        // set flow timeout
        if (r->isFlagEnabled(RULE_FLOW_TIMEOUT)) {
            unsigned long timeout = r->getFlowTimeout();
            if (timeout == 0) {
                // use the default
                timeout = FLOW_IDLE_TIMEOUT;
            }
            // flow timeout for flow based reporting (1 event per rule!)
			if (r->isFlagEnabled(RULE_AUTO_FLOWS)) {
				// check every second because we don't wanna readjust the event based on the
				// auto flows last packets, this would potentially mean lots of timeout events
				// expiring each second (however with this approach we might have an error of almost 1 s)
				e->addEvent(new FlowTimeoutEvent((unsigned long)1, r->getUId(), timeout, (unsigned long)1000));
			} else {
				// try to optimize the timeout checking, only check every timeout seconds
				// and readjust event based on last packet timestamp
				e->addEvent(new FlowTimeoutEvent(timeout, r->getUId(), timeout, timeout*1000));
			}
        }*/
    }

}


/* ------------------------- getInfo ------------------------- */

string BidManager::getInfo(Bid *r)
{
    ostringstream s;

#ifdef DEBUG
    log->dlog(ch, "looking up Bid with uid = %d", r->getUId());
#endif

    s << r->getInfo() << endl;
    
    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string BidManager::getInfo(string sname, string rname)
{
    ostringstream s;
    string info;
    Bid *r;
  
    r = getBid(sname, rname);

    if (r == NULL) {
        // check done tasks
        for (bidDoneIter_t i = bidDone.begin(); i != bidDone.end(); i++) {
            if (((*i)->getBidName() == rname) && ((*i)->getBidSet() == sname)) {
                info = (*i)->getInfo();
            }
        }
        
        if (info.empty()) {
            throw Error("no bid with bid name '%s.%s'", sname.c_str(), rname.c_str());
        }
    } else {
        // rule with given identification is in database
        info = r->getInfo();
    }
    
    s << info;

    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string BidManager::getInfo(string sname)
{
    ostringstream s;
    bidSetIndexIter_t b;

    b = bidSetIndex.find(sname);

    if (b != bidSetIndex.end()) {
        for (bidIndexIter_t i = b->second.begin(); i != b->second.end(); i++) {
            s << getInfo(sname, i->first);
        }
    } else {
        s << "No such bidset" << endl;
    }
    
    return s.str();
}


/* ------------------------- getInfo ------------------------- */

string BidManager::getInfo()
{
    ostringstream s;
    bidSetIndexIter_t iter;

    for (iter = bidSetIndex.begin(); iter != bidSetIndex.end(); iter++) {
        s << getInfo(iter->first);
    }
    
    return s.str();
}


/* ------------------------- delBid ------------------------- */

void BidManager::delBid(string sname, string rname, EventScheduler *e)
{
    Bid *r;

#ifdef DEBUG    
    log->dlog(ch, "Deleting bid set= %s name = '%s'",
              sname.c_str(), rname.c_str());
#endif  


    if (sname.empty() && rname.empty()) {
        throw Error("incomplete rule set or name specified");
    }

    r = getBid(sname, rname);

    if (r != NULL) {
        delBid(r, e);
    } else {
        throw Error("bid %s.%s does not exist", sname.c_str(),rname.c_str());
    }
}


/* ------------------------- delBid ------------------------- */

void BidManager::delBid(int uid, EventScheduler *e)
{
    Bid *r;

    r = getBid(uid);

    if (r != NULL) {
        delBid(r, e);
    } else {
        throw Error("bid uid %d does not exist", uid);
    }
}


/* ------------------------- delBids ------------------------- */

void BidManager::delBids(string sname, EventScheduler *e)
{
    
    if (bidSetIndex.find(sname) != bidSetIndex.end()) 
    {
		bidSetIndexIter_t iter = bidSetIndex.find(sname);
		bidIndex_t bidIndex = iter->second;
        for (bidIndexIter_t i = bidIndex.begin(); i != bidIndex.end(); i++) 
        {
            delBid(getBid(sname, i->first),e);
        }
    }
}


/* ------------------------- delBid ------------------------- */

void BidManager::delBid(Bid *r, EventScheduler *e)
{
#ifdef DEBUG    
    log->dlog(ch, "removing bid with name = '%s'", r->getBidName().c_str());
#endif

    // remove bid from database and from index
    storeBidAsDone(r);
    bidDB[r->getUId()] = NULL;
    bidSetIndex[r->getBidSet()].erase(r->getBidName());
    
    // Find the corresponding nodes in the auction bid index and deletes
	vector<int>::iterator actBidIter;
	auctionSetBidIndexIter_t setNode;  
	auctionBidIndexIter_t auctionNode; 
	
	setNode = bidAuctionSetIndex.find(r->getAuctionSet());
	auctionNode = (setNode->second).find(r->getAuctionName());
		
	for (actBidIter = (auctionNode->second).begin(); 
			actBidIter != (auctionNode->second).end(); ++actBidIter){
					
		if (*actBidIter == r->getUId()){
			(auctionNode->second).erase(actBidIter);
			break;
		}
	}
		
	// delete the auction name if empty.
	auctionNode = (setNode->second).find(r->getAuctionName());
	if ((auctionNode->second).empty()){
		bidAuctionSetIndex[r->getAuctionSet()].erase(auctionNode);
	}
			
	// delete bid auction set if empty
	setNode = bidAuctionSetIndex.find(r->getAuctionSet());
	if ((setNode->second).empty()) {
		bidAuctionSetIndex.erase(setNode);
	}
	
    // delete bid set if empty
    if (bidSetIndex[r->getBidSet()].empty()) {
        bidSetIndex.erase(r->getBidSet());
    }
    
    if (e != NULL) {
        e->delBidEvents(r->getUId());
    }

    bids--;
}


/* ------------------------- delBids ------------------------- */

void BidManager::delBids(bidDB_t *bids, EventScheduler *e)
{
    bidDBIter_t iter;

    for (iter = bids->begin(); iter != bids->end(); iter++) {
        delBid(*iter, e);
    }
}


/* -------------------- storeBidAsDone -------------------- */

void BidManager::storeBidAsDone(Bid *r)
{
    
    r->setState(BS_DONE);
    bidDone.push_back(r);

    if (bidDone.size() > DONE_LIST_SIZE) {
        // release id
        idSource.freeId(bidDone.front()->getUId());
        // remove rule
        saveDelete(bidDone.front());
        bidDone.pop_front();
    }
}


/* ------------------------- dump ------------------------- */

void BidManager::dump( ostream &os )
{
    
    os << "BidManager dump :" << endl;
    os << getInfo() << endl;
    
}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, BidManager &rm )
{
    rm.dump(os);
    return os;
}
