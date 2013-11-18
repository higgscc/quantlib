/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Chris Higgs

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file singleleg.hpp
    \brief single leg instrument
*/

#ifndef quantlib_singleleg_hpp
#define quantlib_singleleg_hpp

#include <ql/instruments/swap.hpp>

namespace QuantLib {

    //! Single leg instrument
    /*! Can be used alone to represent Money Market trades, 
        on in a CombinationInstrument to represent exotic IRD structures

        \ingroup instruments
    */
    class SingleLeg : public Swap {
      public:
        //! \name Constructors
        //@{

        SingleLeg(const Leg& leg);

        //@}
      protected:
        // data members
    };
}
#endif
