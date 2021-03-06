/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2011, 2012 Ferdinando Ametrano

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

/*! \file observable.hpp
    \brief observer/observable pattern
*/

#ifndef quantlib_observable_hpp
#define quantlib_observable_hpp

#include <ql/patterns/singleton.hpp>
#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <ql/utilities/tracing.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>

namespace QuantLib {

    class Observer;
    class Observable;

    //! global repository for run-time library settings
    class ObservableSettings : public Singleton<ObservableSettings> {
        friend class Singleton<ObservableSettings>;
        friend class Observable;
      public:
        void disableUpdates(bool deferred=false) {updatesEnabled_=false; updatesDeferred_=deferred;}
        void enableUpdates();

        bool updatesEnabled()  {return updatesEnabled_;}
        bool updatesDeferred() {return updatesDeferred_;}
      private:
        ObservableSettings() : updatesEnabled_(true), updatesDeferred_(false) {}
        void registerDeferredObservers(boost::unordered_set<Observer*>& observers);
        void unregisterDeferredObserver(Observer*);

        typedef boost::unordered_set<Observer*>::iterator iterator;
        boost::unordered_set<Observer*> deferredObservers_;
        bool updatesEnabled_;
        bool updatesDeferred_;
    };

    //! Object that notifies its changes to a set of observers
    /*! \ingroup patterns */
    class Observable {
        friend class Observer;
      public:
        // constructors, assignment, destructor
        Observable() : settings_(ObservableSettings::instance()) {}
        Observable(const Observable&);
        Observable& operator=(const Observable&);
        virtual ~Observable() {}
        /*! This method should be called at the end of non-const methods
            or when the programmer desires to notify any changes.
        */
        void notifyObservers();
      private:
        typedef boost::unordered_set<Observer*>::iterator iterator;
        std::pair<iterator, bool> registerObserver(Observer*);
        Size unregisterObserver(Observer*);
        boost::unordered_set<Observer*> observers_;
	ObservableSettings& settings_;
    };

    //! Object that gets notified when a given observable changes
    /*! \ingroup patterns */
    class Observer {
      public:
        // constructors, assignment, destructor
        Observer() {}
        Observer(const Observer&);
        Observer& operator=(const Observer&);
        virtual ~Observer();
        // observer interface
        std::pair<boost::unordered_set<boost::shared_ptr<Observable> >::iterator, bool>
                            registerWith(const boost::shared_ptr<Observable>&);
        Size unregisterWith(const boost::shared_ptr<Observable>&);
        /*! This method must be implemented in derived classes. An
            instance of %Observer does not call this method directly:
            instead, it will be called by the observables the instance
            registered with when they need to notify any changes.
        */
        void unregisterWithAll();
        virtual void update() = 0;
      private:
        boost::unordered_set<boost::shared_ptr<Observable> > observables_;
        typedef boost::unordered_set<boost::shared_ptr<Observable> >::iterator iterator;
    };


    // inline definitions

    inline void ObservableSettings::registerDeferredObservers(boost::unordered_set<Observer*>& observers) {
        if (updatesDeferred())
	{
		QL_TRACE("adding " << observers.size() << " observers to the deferred list");
        	deferredObservers_.insert(observers.begin(), observers.end());
	}
    }

    inline void ObservableSettings::unregisterDeferredObserver(Observer* o) {
	QL_TRACE("removing observer " << o << " from the deferred list");
    	deferredObservers_.erase(o);
    }

    inline void ObservableSettings::enableUpdates() {
    	updatesEnabled_  = true;
    	updatesDeferred_ = false;

    	// if there are outstanding deferred updates, do the notification
        if (deferredObservers_.size() > 0)
        {
            bool successful = true;
            std::string errMsg;

            QL_TRACE("deferred notification of " << deferredObservers_.size() << " observers");
            for (iterator i=deferredObservers_.begin(); i!=deferredObservers_.end(); ++i) {
                try {
                    (*i)->update();
                } catch (std::exception& e) {
                    successful = false;
                    errMsg = e.what();
                } catch (...) {
                    successful = false;
                }
            }

            deferredObservers_.clear();

            QL_ENSURE(successful,
                  "could not notify one or more observers: " << errMsg);
        }
    }

    inline Observable::Observable(const Observable&) : settings_(ObservableSettings::instance()) {
        // the observer set is not copied; no observer asked to
        // register with this object
    }

    /*! \warning notification is sent before the copy constructor has
                 a chance of actually change the data
                 members. Therefore, observers whose update() method
                 tries to use their observables will not see the
                 updated values. It is suggested that the update()
                 method just raise a flag in order to trigger
                 a later recalculation.
    */
    inline Observable& Observable::operator=(const Observable& o) {
        // as above, the observer set is not copied. Moreover,
        // observers of this object must be notified of the change
        if (&o != this)
            notifyObservers();
        return *this;
    }

    inline std::pair<boost::unordered_set<Observer*>::iterator, bool>
    Observable::registerObserver(Observer* o) {
        return observers_.insert(o);
    }

    inline Size Observable::unregisterObserver(Observer* o) {
    	// in case the observer is in the deferred notifications list
    	// remove it
        if (settings_.updatesDeferred())
    	    settings_.unregisterDeferredObserver(o);

    	return observers_.erase(o);
    }

    inline void Observable::notifyObservers() {
    	// check whether the notifications should be triggered
    	if (!settings_.updatesEnabled())
    	{
    	    // if updates are only deferred, flag this for later notification
    	    // these are held centrally by the settings singleton
            if (settings_.updatesDeferred())
    		settings_.registerDeferredObservers(observers_);

    	    return;
    	}

        if (observers_.size() > 0)
        {
            bool successful = true;
            std::string errMsg;

            QL_TRACE("direct notification of " << observers_.size() << " observers");
            for (iterator i=observers_.begin(); i!=observers_.end(); ++i) {
                try {
                   (*i)->update();
                } catch (std::exception& e) {
                    // quite a dilemma. If we don't catch the exception,
                    // other observers will not receive the notification
                    // and might be left in an incorrect state. If we do
                    // catch it and continue the loop (as we do here) we
                    // lose the exception. The least evil might be to try
                    // and notify all observers, while raising an
                    // exception if something bad happened.
                    successful = false;
                    errMsg = e.what();
                } catch (...) {
                    successful = false;
                }
            }
            QL_ENSURE(successful,
                  "could not notify one or more observers: " << errMsg);
        }
    }


    inline Observer::Observer(const Observer& o)
    : observables_(o.observables_) {
        for (iterator i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->registerObserver(this);
    }

    inline Observer& Observer::operator=(const Observer& o) {
        iterator i;
        for (i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->unregisterObserver(this);
        observables_ = o.observables_;
        for (i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->registerObserver(this);
        return *this;
    }

    inline Observer::~Observer() {
        for (iterator i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->unregisterObserver(this);
    }

    inline std::pair<boost::unordered_set<boost::shared_ptr<Observable> >::iterator, bool>
    Observer::registerWith(const boost::shared_ptr<Observable>& h) {
        if (h) {
            h->registerObserver(this);
            return observables_.insert(h);
        }
        return std::make_pair(observables_.end(), false);
    }

    inline
    Size Observer::unregisterWith(const boost::shared_ptr<Observable>& h) {
        if (h)
            h->unregisterObserver(this);
        return observables_.erase(h);
    }

    inline void Observer::unregisterWithAll() {
        for (iterator i=observables_.begin(); i!=observables_.end(); ++i)
            (*i)->unregisterObserver(this);
        observables_.clear();
    }

}

#endif
