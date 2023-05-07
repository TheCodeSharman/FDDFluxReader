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
    if ( serialDevice.dtr() && !attached ) {
        attached = true;
        delay(10);
        help();
        ready();
    } else if ( !serialDevice.dtr() && attached ) {
        attached = false;
    }

    while ( serialDevice.available() > 0 ) {
        char inChar = serialDevice.read();
        echo(inChar);
        if ( receive( inChar ) ) {
            processCommand();
        }
    }
}

void CommandProcessor::help() {
    if ( outputAllowed() ) {
        serialDevice.println("\nFDDFluxReader v0.1");
        serialDevice.println("\navailable commands:");
        serialDevice.println("\tbootloader");
        serialDevice.println("\tstart_sampling");
        serialDevice.println("\tstop_sampling");
        serialDevice.println("\thome");
        serialDevice.println("\tseek_track <track>");
        serialDevice.println("\thelp\n");
    }
}

void CommandProcessor::processCommand() {
    if ( command.empty() ) {
        // do nothing
    } else if ( command == "bootloader" ) {
        enter_dfu_bootloader();
    } else if ( command == "start_sampling" ) {
        readSampler.startSampling();
    } else if ( command == "stop_sampling" ) {
        readSampler.stopSampling();
    } else if ( command == "home" ) {
        if ( readSampler.findTrack0() ){
            serialDevice.println("Success! Head at track 0.\n");
        } else {
            serialDevice.println("ERROR: Unable to seek track 0.\n");
        }
    } else if ( command == "seek_track" ) { // parse int
        readSampler.seekTrack(80);
    } else if ( command == "help" ) {
        help();
    }
    else { 
        unknownCommand();
    }

    command.clear();
    ready();
}

void CommandProcessor::echo(char inChar) {
    if ( outputAllowed() ) {
        serialDevice.print(inChar);
    }
}

void CommandProcessor::ready() { 
    if ( outputAllowed() ) {
        serialDevice.print("> "); 
    }
}

void CommandProcessor::unknownCommand() {
    if ( outputAllowed() ) {
        serialDevice.printf("Unknown command: %s\r\n", command.c_str() );
    }
}


CommandProcessor::CommandProcessor(USBSerial& serialDevice, MultiTask& multitask, PinSampler& readSampler) 
    : serialDevice(serialDevice), multitask(multitask), readSampler(readSampler) {
}

void CommandProcessor::init() {
    multitask.every(0,std::bind(&CommandProcessor::processSerial, this));
} 