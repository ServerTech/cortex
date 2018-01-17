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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>

class Except
{
    protected:

        const short int id, sub_id; // ID and subtype ID.
        const std::string type; // String to represent type of exception.
        const std::string msg; // Message to return in what().

    public:

        Except(short int sub_id_arg, const std::string& what_arg);
        Except(short int id_arg, short int sub_id_arg,
            const std::string& type_arg, const std::string& what_arg);
        virtual int get_id() const;
        virtual int get_sub_id() const;
        virtual std::string get_type() const;
        virtual std::string what() const;
};

class InvalidArgsException: public Except
{
    public:

        InvalidArgsException(short int sub_id_arg,
            const std::string& what_arg);
};

class OutOfBoundsException: public Except
{
    public:

        OutOfBoundsException(short int sub_id_arg,
            const std::string& what_arg);
};

class BadCharacterException: public Except
{
    public:

        BadCharacterException(short int sub_id_arg,
            const std::string& what_arg);
};

class RuntimeErrorException: public Except
{
    public:

        RuntimeErrorException(short int sub_id_arg,
            const std::string& what_arg);
};

class InvalidMoveException: public Except
{
    public:

        InvalidMoveException(short int sub_id_arg,
            const std::string& what_arg);
};

#endif // EXCEPTIONS_H