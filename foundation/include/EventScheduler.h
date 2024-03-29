
/*! \file   EventScheduler.h

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
    event scheduling
    Code based on Netmate Implementation

    $Id: EventScheduler.h 748 2015-07-29 15:45:00Z amarentes $
*/

#ifndef _EVENTSCHEDULER_H_
#define _EVENTSCHEDULER_H_


#include "stdincpp.h"
#include "Logger.h"
#include "Event.h"
#include "Timeval.h"

namespace auction
{

class Event;  // forward declaration

//! comparison operator for events
struct lttv
{
    bool operator()(const struct timeval t1, const struct timeval t2) const
    {
        return (Timeval::cmp(t1, t2) < 0);
    }
};

//! event list definition (sorted by expiry time)
typedef multimap<struct timeval, Event*,lttv>            eventList_t;
typedef multimap<struct timeval, Event*,lttv>::iterator  eventListIter_t;


/*! \short   schedule timed events and execute the corresponding function at the correct time
  
    The EventScheduler's task is to schedule and execute timed events in the
    meter system such as timed addition/removal of tasks (rules).
    A number of different events that are derived from the basic event class
    can be put into the EventSchedulers event queue.
*/

class EventScheduler
{
  protected:

    Logger *log;  //!< link to global logger object
    int ch;       //!< logging channel number used by objects of this class

    eventList_t events;    //!< event list
    
  public:
    
    //! construct and initialize an EventScheduler object
    EventScheduler();
    

    //! destroy an EventScheduler object
    ~EventScheduler();


    /*! \short   add an Event to the event queue

        \arg \c ev - an event (or an object derived from Event) that is
                     schdeuled for a particular time (possibly repeatedly 
                     at a given interval)
    */
    void addEvent(Event *ev);

	
    /*! \short   delete all events for a given bidding object

        delete all Events related to the specified bid from the list of events

        \arg \c uid  - the unique identification number of the bid
    */
    void delBiddingObjectEvents(int uid);

    /*! \short   delete all events for a given auction

        delete all Events related to the specified auction from the list of events

        \arg \c uid  - the unique identification number of the auction
    */
    void delAuctionEvents(int uid);

    // get pointer to first/next event
    Event *getNextEvent();

    //! requeus (if recurring) the event ev advancing its expiry time
    void reschedNextEvent(Event *ev);

    //! return the time of the next event due
    struct timeval getNextEventTime();

    //! dump an EventScheduler object
    void dump(ostream &os);
    
    virtual void delResourceRequestEvents(int uid) {}

	//! delete the session from all events 
	virtual void delSessionEvents(int uid);
	
	//! get the number of events in the scheduller
	
	inline size_t getNbrEvents(){ return events.size(); }

};


//! overload for << so that a EventScheduler object can be thrown into an ostream
ostream& operator<<(ostream &os, EventScheduler &es);

} // namespace auction

#endif // _EVENTSCHEDULER_H_
