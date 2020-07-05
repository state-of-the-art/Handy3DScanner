#ifndef ARCOREWORKER_H
#define ARCOREWORKER_H

#include <QObject>
#include <QMutex>

class ARCoreWorker : public QObject
{
    Q_OBJECT
public:
    explicit ARCoreWorker();

    typedef struct ArSession_ ArSession;

    void setSession(ArSession **session);
    void stop();

public slots:
    void doWork();

signals:
    void stopped();
    void errorOccurred(const QString &error);
    void cameraMatrix(float *camera_matrix);

private:
    QMutex m_mutex;

    bool m_stopped;

    typedef struct ArFrame_ ArFrame;

    ArSession *m_session;
    ArFrame *m_frame;
};

#endif // ARCOREWORKER_H
