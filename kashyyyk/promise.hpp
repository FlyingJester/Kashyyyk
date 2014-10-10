#pragma once

#include <memory>

namespace Kashyyyk {

  /*
  A Promise represents some future result. At some point in the future,
  the result will be ready. Sort of like saying the function will `return'
  at some future point.

  Promises are designed to be easily and efficiently passed by value.

  A base Promise only represents the future event of the result, not
  any result at all.

  The derived class PromiseValue allows a value to be given along with
  the result.

  In that way, a Promise along is like some future void return, and a
  PromiseValue<T> is like return (T) _.

  */

  class Promise {
public:

      Promise();
      virtual ~Promise();

      bool IsReady();
      void SetReady();

private:

    struct Promise_Impl;
    std::shared_ptr<Promise_Impl> state;

  };

  template <class T>
  class PromiseValue : public Promise {

protected:

    T t;

public:

    PromiseValue(T def) // Default value.
      : Promise()
      , t(def){

    }


    virtual ~PromiseValue(){}


    void Finalize(T a){
        t = a;
    }


    T Finish(){
        return t;
    }

  };

}
