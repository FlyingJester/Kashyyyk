#include "promise.hpp"

#include <atomic>

enum State {NotReady, Ready};

namespace Kashyyyk {

struct Promise::Promise_Impl{
    std::atomic<State> state;
};


Promise::Promise()
  : state(new Promise_Impl()){
    state->state = NotReady;
}


Promise::~Promise(){}


bool Promise::IsReady() {return state->state;}
void Promise::SetReady() {state->state = Ready; }

}
