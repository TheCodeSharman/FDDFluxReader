#include "FluxReaderCommandParser.h"

bool FluxReaderCommand::match(CommandParser& commandParser) {
    auto fluxCommand = static_cast<FluxReaderCommandParser&>(commandParser);
    return fluxCommand.command == command;
}

bool FluxReaderCommandParser::receive(char inChar) {
    if ( inChar == '\n' ) {
        return true;
    }

    if ( inChar != '\r' ) {
        command.push_back(inChar);
    }
    return false;
}

void FluxReaderCommandParser::clear() {
    command.clear();
}

std::string FluxReaderCommandParser::buffer() {
    return command;
}