
/*! \file   IdSource.h

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
    manage unique numeric for an id space

    $Id: IdSource.h 748 2015-07-23 52:09:00Z amarentes $
*/

#ifndef _IDSOURCE_H_
#define _IDSOURCE_H_


#include "stdincpp.h"
#include "Error.h"
#include "Logger.h"


namespace auction
{

/*! \short   generate unique id numbers
  
    The IdSource class can generate unique 32 bit integer numbers for
    use by other functions. A single number can be lent from the pool of
    available numbers, and this number will not be generated by a call to 
    the newId function until it has been previously released with a call
    to the freeId function.
*/

class IdSource
{
  private:

    unsigned short num;
    int unique;
    list<unsigned short> freeIds; //!< list of previously freed (now unused) ids

    typedef vector<unsigned short> IdsReservContainer;
    typedef IdsReservContainer::iterator IdsReservIterator;

	IdsReservContainer idReserved;
    
  public:

    //! construct and initialize a BidIdSource object
    //! if unique is set to 1 Ids should be unique until we wrap around 2^16
    //! We remove ids numbered: 0 		-- Reserved for qdisc
	//! 						1 		-- Reserved for root class
	//!						    0xFFFF  -- Reserved for default class
						       
    IdSource(int unique = 0);

    //! destroy a RuleIdSource object
    ~IdSource();

    /*! \short   generate a new internal id number

        return a new Id value that is currently not in use. This value will be
        marked as used and will not returned by a call to newId unless it has
        been released again with a call to freeId
        \returns unique unused id value
    */
    unsigned short newId( void );

    /*! \short   release an id number

        The released id number can be reused (i.e. returned by newId) after
        the call to freeId. Do not release numbers that have not been obtained
        by a call to newId this will result in unpredictable behaviour.

        \arg \c id - id value that is to be released for future use
    */
    void freeId( unsigned short id );

    //! dump a IdSource object
    void dump( ostream &os );



};


//! overload for <<, so that a IdSource object can be thrown into an iostream
ostream& operator<< ( ostream &os, IdSource &ris );

} // namespace auction

#endif // _IDSOURCE_H_
