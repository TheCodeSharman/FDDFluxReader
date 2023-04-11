#ifndef FLUX_READER_COMMAND_PROCESSOR_H
#define FLUX_READER_COMMAND_PROCESSOR_H


#include "FluxReaderCommandParser.h"
#include "commands/FluxReaderCommands.h"

class FluxReaderCommandProcessor : public CommandProcessor {
    private:
        FluxReaderCommandParser fluxReaderParser;
        EnterBootloaderCommand enterBootloaderCommand;
        NullCommand nullCommand;
        std::vector<Command*> fluxReaderCommands = { &nullCommand, &enterBootloaderCommand };

    protected:
        void echo(char inChar) {
            Serial.print(inChar);
        }

        void ready() { 
            Serial.print("> "); 
        }

        void unknownCommand() {
            serialDevice.printf("Unknown command: %s\r\n", commandParser.buffer().c_str() );
        }

    public:
        FluxReaderCommandProcessor(Stream& serialDevice, MultiTask& multitask) 
            : CommandProcessor(serialDevice, multitask, fluxReaderParser, fluxReaderCommands) {
        }
};

#endif