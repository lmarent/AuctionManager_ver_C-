
/*!\file   AuctionTimer.cc

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
    performance timing functions
    Code based on Netmate Implementation

    $Id: AuctionTimer.cpp 748 2015-07-26 9:33:00 amarentes $
*/

#include "AuctionTimer.h"

using namespace auction;

const int FBUFSZ = 1000;
const char* CPUINFO = "/proc/cpuinfo";

// init static
AuctionTimer *AuctionTimer::s_instance = NULL;


/* -------------------- getInstance -------------------- */

AuctionTimer* AuctionTimer::getInstance() 
{ 
    if (s_instance == NULL) { 
        s_instance = new AuctionTimer();
    }
    return s_instance;
}


/* ------------------------- AuctionTimer ------------------------- */

AuctionTimer::AuctionTimer()
{
    log = Logger::getInstance();
    ch = log->createChannel("AuctionTimer");

#ifdef DEBUG
    log->dlog(ch, "Starting");
#endif

    clockSpeed = readClockSpeed();
    clockSpeed_int = (unsigned long long)(clockSpeed + 0.5);

#ifdef DEBUG
    log->dlog(ch, "system clock speed is %.3lf MHz", clockSpeed);
#endif

    for (int i = 0; i < MAXAUCTSLOTS; i++) {
	
        ticks[i].start = 0;
        ticks[i].last = 0;
        ticks[i].sum = 0;
        ticks[i].num = 0;
    }

    // record overhead for readTSC call
    {
        unsigned long long a,b;

        a = readTSC();
        b = readTSC();
	
        overhead = (unsigned int)(b-a);
#ifdef DEBUG
        log->dlog(ch, "two successive readTSC take %d ticks", overhead);
#endif
    }

#ifdef HRTIME_PROFILING
    int error = hrtime_init();
    if (error < 0) {
        throw Error("ERROR: cannot init hrtime library (%s)",
                    strerror(error));
    }

    // record overhead1 for get_hrvtime_self call
    {
        hrtime_t a,b;

        get_hrvtime_self(&a);
        /* smooth out fluctuations by averaging call time /8 */
        get_hrvtime_self(&b);
        get_hrvtime_self(&b);
        get_hrvtime_self(&b);
        get_hrvtime_self(&b);
        get_hrvtime_self(&b);
        get_hrvtime_self(&b);
        get_hrvtime_self(&b);
        get_hrvtime_self(&b);
        overhead2 = (unsigned int)(b-a)/8;
        log->log(ch, "two successive hrvtime_self take %d ticks", overhead2);
        overhead2_st = overhead2;
    }
#endif
}

/* ------------------------- ~AuctionTimer ------------------------- */

AuctionTimer::~AuctionTimer()
{
#ifdef DEBUG
    log->dlog(ch, "Shutdown");
#endif

}


/* ------------------------- readClockSpeed ------------------------- */

double AuctionTimer::readClockSpeed()
{
    FILE *cinfo;
    char buf[FBUFSZ];
    int clock_int, clock_frac;

    clock_int = -1;

    cinfo = fopen(CPUINFO, "rt");

    if (cinfo == NULL) {
        return -1.0;
    }

    while (1) {
        fgets(buf, FBUFSZ-1, cinfo);
        if (feof(cinfo) != 0 || sscanf( buf, "cpu MHz         : %d.%d\n",
                                        &clock_int, &clock_frac) == 2) {
            break;
        }
    }
    fclose(cinfo);

    if (clock_int == -1) {
        return -1.0;
    } else {
        return ((double)clock_int + (double)clock_frac / 1000.0);
    }
}


/* ------------------------- start ------------------------- */

void AuctionTimer::start( int slot )
{
    unsigned long long *ptr;

    if (slot<0 || slot>MAXAUCTSLOTS) {
        return;
    }

    //  ticks[slot].start = readTSC();
    ptr = &ticks[slot].start;
    *ptr = readTSC();
}


/* ------------------------- stop ------------------------- */

void AuctionTimer::stop( int slot )
{
    unsigned long long tickCount;
   
    tickCount = readTSC();
    
    if (slot<0 || slot>MAXAUCTSLOTS) {
        return;
    }

    ticks[slot].last = tickCount - ticks[slot].start;
    ticks[slot].sum += ticks[slot].last;
    ticks[slot].num += 1;
}


/* ------------------------- latest ------------------------- */

unsigned long long AuctionTimer::latest( int slot )
{
    if (slot<0 || slot>MAXAUCTSLOTS) {
        return 0;
    }
	
	
    if (ticks[slot].num == 0) {
        return 0; 	
    } else {
        return ((ticks[slot].last-overhead) * 1000 / clockSpeed_int);
    }
}


/* ------------------------- account ------------------------- */

void AuctionTimer::account( int slot, unsigned long long clticks )
{
    if (slot<0 || slot>MAXAUCTSLOTS) {
        return;
    }

    ticks[slot].last = clticks;
    ticks[slot].sum += clticks;
    ticks[slot].num += 1;
}


/* ------------------------- avg ------------------------- */

unsigned long long AuctionTimer::avg( int slot )
{
    if (slot<0 || slot>MAXAUCTSLOTS) {
        return 0;
    }

    if (ticks[slot].num == 0) {
        return 0; 	
    } else {
        return ((ticks[slot].sum-overhead*ticks[slot].num)
                * 1000 / ticks[slot].num / clockSpeed_int);
    }
}


/* ------------------------- ticks2ns ------------------------- */

unsigned long long AuctionTimer::ticks2ns( unsigned long long ticks )
{
    return ((ticks - s_instance->overhead)
            * 1000 / s_instance->clockSpeed_int);
}


/* ------------------------- ticks2ns ------------------------- */

unsigned long long AuctionTimer::ticks2ns( unsigned int runs, 
                                        unsigned long long ticks )
{
    if (runs == 0) {
        return 0; 	
    } else {
        return ((ticks - s_instance->overhead * runs)
                * 1000 / runs / s_instance->clockSpeed_int);
    }
}

/* ------------------------- dump ------------------------- */

void AuctionTimer::dump( ostream &os )
{
    os << "AuctionTimer dump :" << endl;

    for (int i = 0; i<MAXAUCTSLOTS; i++) {

        os << "slot " << i << " : " 
           <<  avg(i) << " nsec ( " 
           << ticks[i].num << " runs )" << endl;
    }
}


/* ------------------------- operator<< ------------------------- */

ostream& operator<< ( ostream &os, AuctionTimer &obj )
{
    obj.dump(os);
    return os;
}
