#ifndef FLUX_READER_COMMAND_PROCESSOR_H
#define FLUX_READER_COMMAND_PROCESSOR_H

#include "utils/stm32utils.h"

class FluxReaderCommandProcessor {
    private:
        MultiTask& multitask;
        USBSerial& serialDevice;

        std::string command;

        bool receive(char inChar) {
            if ( inChar == '\n' ) {
                return true;
            }

            if ( inChar != '\r' ) {
                command.push_back(inChar);
            }
            return false;
        }   

        void processSerial() {
            while ( serialDevice.available() > 0 ) {
                char inChar = serialDevice.read();
                echo(inChar);
                if ( receive( inChar ) ) {
                    processCommand();
                }
            }
        }

        void processCommand() {
            if ( command.empty() ) {
                // do nothing
            } else if ( command == "bootloader" )
                enter_dfu_bootloader();
            else 
                unknownCommand();

            command.clear();
            ready();
        }

        void echo(char inChar) {
            serialDevice.print(inChar);
        }

        void ready() { 
            serialDevice.print("> "); 
        }

        void unknownCommand() {
            serialDevice.printf("Unknown command: %s\r\n", command.c_str() );
        }

    public:
        FluxReaderCommandProcessor(USBSerial& serialDevice, MultiTask& multitask) 
            : serialDevice(serialDevice), multitask(multitask) {
        }

        void init() {
            multitask.every(0,std::bind(&FluxReaderCommandProcessor::processSerial, this));
        } 
};

#endif