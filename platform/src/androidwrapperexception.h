#ifndef ANDROIDWRAPPEREXCEPTION_H
#define ANDROIDWRAPPEREXCEPTION_H

#include <QException>

class AndroidWrapperException
    : public QException
{
public:
    explicit AndroidWrapperException(QString const& message) :
        message(message)
    {}

    virtual ~AndroidWrapperException() {}

    void raise() const { throw *this; }
    AndroidWrapperException *clone() const { return new AndroidWrapperException(*this); }

    QString getMessage() const { return message; }

private:
    QString message;
};

#endif // ANDROIDWRAPPEREXCEPTION_H
