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

#include <ql/instruments/singleleg.hpp>

namespace QuantLib {

    SingleLeg::SingleLeg(const Leg& leg)
    : Swap(1) {
        legs_[0] = leg;
	payer_[0]= 1.0;
        for (Leg::iterator i = legs_[0].begin(); i!= legs_[0].end(); ++i)
            registerWith(*i);
    }

}
