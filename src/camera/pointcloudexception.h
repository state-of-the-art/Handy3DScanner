#ifndef POINTCLOUDEXCEPTION_H
#define POINTCLOUDEXCEPTION_H

#include <QException>

class PointCloudException : public QException
{
public:
    explicit PointCloudException(QString const& message) :
        message(message)
    {}

    virtual ~PointCloudException() {}

    void raise() const { throw *this; }
    PointCloudException *clone() const { return new PointCloudException(*this); }

    QString getMessage() const { return message; }

private:
    QString message;
};

#endif // POINTCLOUDEXCEPTION_H
