#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <Arduino.h>

#include "MultiTask.h"
/*
    Command processor is responsible for recieve bytes from the serial
    port and executing G-Code commands.
*/

/* Implement this interface to parse commands from incoming characters */
class CommandParser {
    public:
        /* Adds character to parsing buffer returns true if there is a valid command*/
        virtual bool receive(char inChar) = 0;

        /* Returns a string of the currently buffered command*/
        virtual std::string buffer() = 0; 

        /* Clears the buffer */
        virtual void clear() = 0;
};


/* Implement this interface to add a specific command, takes a CommandParser which 
   has a command is ready to be processed*/
class Command {
    public:
        /* Given a CommandParser with a valid command should this command execute?*/
        virtual bool match(CommandParser& commandParser) = 0;

        /* Executes this command */
        virtual void execute() = 0;
};

class CommandProcessor {
    private:
        Command* findCommand();
        void processCommand();
        void processSerial();

        std::vector<Command*>& commands;

    protected:
        CommandParser& commandParser;
        Stream& serialDevice;
        MultiTask& multitask;

        virtual void echo(char inChar) = 0; 
        virtual void ready() = 0; 
        virtual void unknownCommand() = 0; 

    public:
        CommandProcessor(Stream& serialDevice, MultiTask& multitask, CommandParser& commandParser, std::vector<Command*>& commands );
        virtual void init();

};

#endif