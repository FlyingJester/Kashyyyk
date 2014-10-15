#pragma once

namespace Kashyyyk{

//!
//! A simple RAII wrapper for classes with lock and unlock methods.
//!
//! This is primarily intended for Kashyyyk::LockingReciever objects, although
//! it will work with any class that has a lock and unlock method.
template <class T>
class AutoLocker{
protected:
      T t;
      bool know;

public:

    //!
    //! Constructor.
    //!
    //! Calls lock. unlock will be called when the AutoLocker is destroyed.
    //! @param a object to wrap.
    AutoLocker(T a)
    : know(true) {
      t = a;
      t->lock();
    }


    ~AutoLocker(){
      if(know)
        t->unlock();
    }

    //!
    //! Forget the wrapped object.
    //!
    //! It will not be unlocked when the AutoLocker is destroyed.
    void forget(){
      t->unlock();
      know = false;
    }

    //!
    //! Change the object pointed to.
    //!
    //! This will not unlock the previously wrapped object (if one exists).
    //! @param a new object to wrap.
    void tell(T a){
      know = true;
      t = a;
    }

    };

}
