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

        bool receive(char inChar);
        void processSerial();
        void processCommand();
        void echo(char inChar);
        void ready();
        void unknownCommand() ;

    public:
        CommandProcessor(USBSerial& serialDevice, MultiTask& multitask, PinSampler& readSampler);
        void init();
};

#endif