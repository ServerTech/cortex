/*
    Cortex - Self-learning Chess Engine
    @filename exceptions.h
    @author Shreyas Vinod
    @version 1.0.2 02/08/2015

    Lists all exceptions that various functions can throw.

    ******************** VERSION CONTROL ********************
    * 28/07/2015 File created.
    * 28/07/2015 1.0.0 Initial version.
    * 29/07/2015 1.0.1 Added subtype IDs.
    * 02/08/2015 1.0.2 Added InvalidMoveException.
*/

#include "exceptions.h"

Except::Except(short int sub_id_arg, const std::string& what_arg)
:id(0), sub_id(sub_id_arg), type("GenericException"), msg(what_arg)
{}

Except::Except(short int id_arg, short int sub_id_arg,
    const std::string& type_arg, const std::string& what_arg)
:id(id_arg), sub_id(sub_id_arg), type(type_arg), msg(what_arg)
{}

int Except::get_id() const
{
    return id;
}

int Except::get_sub_id() const
{
    return sub_id;
}

std::string Except::get_type() const
{
    return type;
}

std::string Except::what() const
{
    return ("Exception occurred: " + std::to_string(id) + "." + sub_id + " " +
        type + ". " + msg);
}

InvalidArgsException::InvalidArgsException(short int sub_id_arg,
    const std::string& what_arg)
:Except(1, sub_id_arg, "InvalidArgsException", what_arg)
{}

OutOfBoundsException::OutOfBoundsException(short int sub_id_arg,
    const std::string& what_arg)
:Except(2, sub_id_arg, "OutOfBoundsException", what_arg)
{}

BadCharacterException::BadCharacterException(short int sub_id_arg,
    const std::string& what_arg)
:Except(3, sub_id_arg, "BadCharacterException", what_arg)
{}

RuntimeErrorException::RuntimeErrorException(short int sub_id_arg,
    const std::string& what_arg)
:Except(4, sub_id_arg, "RuntimeErrorException", what_arg)
{}

InvalidMoveException::InvalidMoveException(short int sub_id_arg,
    const std::string& what_arg)
:Except(5, sub_id_arg, "InvalidMoveException", what_arg)
{}