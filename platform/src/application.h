#ifndef APPLICATION_H
#define APPLICATION_H

#include <QGuiApplication>

class Application
    : public QGuiApplication
{
    Q_OBJECT
public:
    inline static Application* create(int &argc, char **argv) { s_pInstance = new Application(argc, argv); return s_pInstance; }
    inline static Application* I() { return s_pInstance; }
    inline static void destroyI() { delete s_pInstance; }

public slots:
    Q_INVOKABLE void notice(QString message);
    Q_INVOKABLE void warning(QString message);
    Q_INVOKABLE void error(QString message);

signals:
    void notification(QString type, QString message);

private slots:
    void stateChanged(Qt::ApplicationState state);

private:
    static Application *s_pInstance;
    explicit Application(int &argc, char **argv);
    ~Application() override;
};

#endif // APPLICATION_H
