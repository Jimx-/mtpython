#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <stdexcept>

namespace mtpython {

class NotImplementedException:public std::runtime_error
{
public:
    NotImplementedException (const char * error_msg) :runtime_error(error_msg){}
    virtual ~NotImplementedException() throw() {}
};

class FileNotFoundException:public std::runtime_error
{
public:
    FileNotFoundException (const char * error_msg) :runtime_error(error_msg){}
    virtual ~FileNotFoundException() throw() {}
};

class SyntaxError:public std::runtime_error
{
public:
    SyntaxError (const char * error_msg) :runtime_error(error_msg){}
    virtual ~SyntaxError() throw() {}
};

}

#endif /* _EXCEPTIONS_H_ */
