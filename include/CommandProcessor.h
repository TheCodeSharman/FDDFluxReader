#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "utils/stm32utils.h"
#include "MultiTask.h"

class CommandProcessor {
    private:
        MultiTask& multitask;
        USBSerial& serialDevice;
        std::string command;

        bool receive(char inChar);
        void processSerial();
        void processCommand();
        void echo(char inChar);
        void ready();
        void unknownCommand() ;

    public:
        CommandProcessor(USBSerial& serialDevice, MultiTask& multitask);
        void init();
};

#endif