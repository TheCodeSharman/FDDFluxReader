#include "MultiTask.h"

MultiTask::~MultiTask() {
    for( CallbackFunction* cb : callbacks ) {
        delete cb;
    }
}

MultiTask::CallbackFunction* MultiTask::every(uint32_t delay, std::function<void()> callback) {
    CallbackFunction* cb = new CallbackFunction(callback, 0, delay);
    callbacks.push_back(cb);
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