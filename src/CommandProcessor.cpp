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
    } else if ( command == "bootloader" )
        enter_dfu_bootloader();
    else 
        unknownCommand();

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


CommandProcessor::CommandProcessor(USBSerial& serialDevice, MultiTask& multitask) 
    : serialDevice(serialDevice), multitask(multitask) {
}

void CommandProcessor::init() {
    multitask.every(0,std::bind(&CommandProcessor::processSerial, this));
} 