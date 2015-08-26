
/*! \file   EventAuctioner.h

    Copyright 2014-2015 Universidad de los Andes, Bogotá, Colombia

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
    EventAuctioner classes for all specific events handled by auctioner.
    Code based on Netmate Implementation

    $Id: EventAuctioner.h 748 2015-07-23 18:14:00 amarentes $
*/

#ifndef _EVENT_AUCTIONER_H_
#define _EVENT_AUCTIONER_H_


#include "stdincpp.h"
#include "ProcModuleInterface.h"
#include "BidFileParser.h"
#include "AuctionFileParser.h"
#include "AuctionManagerInfo.h"
#include "Auction.h"
#include "Event.h"

/* --------------------------------- events ------------------------------ */




class PushExecutionEvent : public Event
{
  private:
    auctionDB_t auctions;
    procnames_t procs;
    int final;

  public:

    PushExecutionEvent(struct timeval time, auctionDB_t &a, procnames_t e, unsigned long ival=0, int align=0) 
      : Event(PUSH_EXECUTION, time, ival, align), auctions(a), procs(e), final(0) {  }

    PushExecutionEvent(time_t offs_sec, auctionDB_t &a, procnames_t e, unsigned long ival=0, int align=0) 
      : Event(PUSH_EXECUTION, offs_sec, 0, ival, align), auctions(a), procs(e), final(0) { }

    PushExecutionEvent(auctionDB_t &a, procnames_t e, unsigned long ival=0, int align=0) 
      : Event(PUSH_EXECUTION, ival, align), auctions(a), procs(e), final(0) { }

    auctionDB_t *getAuctions()
    {
        return &auctions;
    }
    
    procnames_t getProcMods()
    {
        return procs;
    }

    int deleteAuction(int uid)
    {
        int ret = 0;
        auctionDBIter_t iter;
           
        for (iter=auctions.begin(); iter != auctions.end(); iter++) {
            if ((*iter)->getUId() == uid) {
                auctions.erase(iter);
                ret++;
                break;
            }   
        }
           
        if (auctions.empty()) {
            return ++ret;
        }
        
        return ret;
    }

    void setFinal(int f)
    {
      final = f;
    }

    int isFinal() 
    {
      return final;
    }
};




class ProcTimerEvent : public Event
{
private:	
    unsigned int tmID;
    timeout_func_t tmFunc;
    
public:

    ProcTimerEvent( timeout_func_t timeout, timers_t *timer ) :
      Event( PROC_MODULE_TIMER,
             timer->ival_msec/1000, timer->ival_msec%1000,
             ((timer->flags & TM_RECURRING) ? timer->ival_msec : 0),
             timer->flags & TM_ALIGNED),
      tmID(timer->id),
      tmFunc(timeout)
      {}
    
    int signalTimeout() { return tmFunc( tmID ); }

};


/* ------------------------------- ctrlcomm events ------------------------ */

class GetInfoEvent : public CtrlCommEvent
{
  private:
    infoList_t *infos;
   
  public:

    GetInfoEvent(infoList_t *i)
      : CtrlCommEvent(GET_INFO), infos(i) {}

    ~GetInfoEvent()
    {
        saveDelete(infos);
    }

    infoList_t *getInfos()
    {
        return infos;
    }
};


class GetModInfoEvent : public CtrlCommEvent
{
  private:

    string modname;
    
  public:

    GetModInfoEvent( string modulename )
      : CtrlCommEvent(GET_MODINFO), modname(modulename) {}

    ~GetModInfoEvent() {}

    string getModName()
    {
        return modname;
    }
};



#endif // _EVENT_AUCTIONER_H_
