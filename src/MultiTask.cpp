#include "MultiTask.h"

MultiTask::~MultiTask() {
    for( CallbackFunction* cb : callbacks ) {
        delete cb;
    }
}

MultiTask::CallbackFunction* MultiTask::every(uint32_t delay, std::function<void()> callback, bool startNow) {
    CallbackFunction* cb = new CallbackFunction(callback, delay);
    callbacks.push_back(cb);
    if ( startNow ) cb->start();
    return cb;
}

void MultiTask::process() {
    current = micros();
    for(CallbackFunction* cb : callbacks ) {
        if ( cb->nextCall < current && cb->isEnabled() ) {
            cb->nextCall = current+cb->getPeriod();
            cb->callback();
        }
    }
}