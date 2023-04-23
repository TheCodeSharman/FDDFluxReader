#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "utils/stm32utils.h"
#include "MultiTask.h"
#include "PinSampler.h"

class CommandProcessor {
    private:
        MultiTask& multitask;
        USBSerial& serialDevice;
        PinSampler& readSampler;
        std::string command;
        bool attached = false;
        bool outputAllowed() {
            return !(readSampler.getState() == PinSampler::SAMPLING
                || readSampler.getState() == PinSampler::STOPPING_SAMPLING
                || readSampler.getState() == PinSampler::WAITING_FOR_INDEX);
        }

        bool receive(char inChar);
        void processSerial();
        void processCommand();
        void echo(char inChar);
        void ready();
        void unknownCommand();
        void help();

    public:
        CommandProcessor(USBSerial& serialDevice, MultiTask& multitask, PinSampler& readSampler);
        void init();
};

#endif