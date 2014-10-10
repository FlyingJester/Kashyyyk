#pragma once

namespace Kashyyyk{

template <class T>
class AutoLocker{
protected:
  T t;
  bool know;

public:

  AutoLocker(T a)
    : know(true) {
      t = a;
      t->lock();
  }


  ~AutoLocker(){
      if(know)
        t->unlock();
  }


  void forget(){
      t->unlock();
      know = false;
  }


  void tell(T a){
      know = true;
      t = a;
  }

};

}
