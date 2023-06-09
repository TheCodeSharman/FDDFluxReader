#ifndef EVERY_H
#define EVERY_H

#include <Arduino.h>
#include <vector>
#include <functional>

/*
    Keeps track of multiple functions to be called at specified intervals.

    To use declare an instance of Every:

            MultiTask tasks;

        In some initialisation function add some callbacks:

            tasks.every(1000,callbackFunction1);
            tasks.every(500,callbackFunction2);

        Then in the main loop call process():

            tasks.process();

    It's up to the impementor to ensure there is no blocking code and return 
    promptly - otherwise nothing will be called whilst a function is being run.
*/
class MultiTask {
  uint32_t current;

  public:
    class CallbackFunction {
      friend class MultiTask;

      private:
        CallbackFunction( std::function<void()> callback, uint32_t period )
          : callback(callback), period(period)  {
            nextCall = micros() + period;
          }

        std::function<void()> callback;
        uint32_t nextCall;
        uint32_t period;
        bool enabled = false;
      public:
        void start() { 
          nextCall = micros() + this->period;
          enabled = true; 
        }
        void stop() { enabled = false; }
        bool isEnabled() { return enabled; }
        uint32_t getPeriod() { return period; }
        void setPeriod( uint32_t period) { this->period = period; }

    };

  private:
    std::vector<CallbackFunction*> callbacks;

  public:

    ~MultiTask();
    
    /* Add a call back function to be called every time the clock ticks over the 
       specified delay in microseconds */
    CallbackFunction* every(uint32_t delay, std::function<void()> callback, bool startNow = true);

    /* Needs to be called as often as possible */
    void process();
};


#endif