#ifndef FLUX_READER_COMMAND_PARSER_H
#define FLUX_READER_COMMAND_PARSER_H

#include "CommandProcessor.h"

class FluxReaderCommand : public Command {
    protected:
        std::string command;
    public:
        virtual bool match(CommandParser& commandParser);
        virtual void execute() = 0;

};

class FluxReaderCommandParser : public CommandParser {
    friend FluxReaderCommand;

    private:
        std::string command;

    public:
        virtual void clear();
        virtual bool receive(char inChar);
        virtual std::string buffer(); 
};

#endif