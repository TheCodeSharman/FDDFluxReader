#include "CommandProcessor.h"

bool CommandProcessor::receive(char inChar) {
    if ( inChar == '\n' ) {
        return true;
    }

    if ( inChar != '\r' ) {
        command.push_back(inChar);
    }
    return false;
}   

void CommandProcessor::processSerial() {
    while ( serialDevice.available() > 0 ) {
        char inChar = serialDevice.read();
        echo(inChar);
        if ( receive( inChar ) ) {
            processCommand();
        }
    }
}

void CommandProcessor::processCommand() {
    if ( command.empty() ) {
        // do nothing
    } else if ( command == "bootloader" ) {
        enter_dfu_bootloader();
    } else if ( command == "start_sampling" ) {
        readSampler.startSampling();
        serialDevice.println("sampling started.");
    } else if ( command == "stop_sampling" ) {
        readSampler.stopSampling();
        serialDevice.println("sampling stopped.");
    } else if ( command == "help" ) {
        serialDevice.println("FDDFluxReader v0.1");
        serialDevice.println("\navailable commands:");
        serialDevice.println("\tbootloader");
        serialDevice.println("\tstart_sampling");
        serialDevice.println("\tstop_sampling");
        serialDevice.println("\thelp\n");
    }
    else { 
        unknownCommand();
    }

    command.clear();
    ready();
}

void CommandProcessor::echo(char inChar) {
    serialDevice.print(inChar);
}

void CommandProcessor::ready() { 
    serialDevice.print("> "); 
}

void CommandProcessor::unknownCommand() {
    serialDevice.printf("Unknown command: %s\r\n", command.c_str() );
}


CommandProcessor::CommandProcessor(USBSerial& serialDevice, MultiTask& multitask, PinSampler& readSampler) 
    : serialDevice(serialDevice), multitask(multitask), readSampler(readSampler) {
}

void CommandProcessor::init() {
    multitask.every(0,std::bind(&CommandProcessor::processSerial, this));
} 