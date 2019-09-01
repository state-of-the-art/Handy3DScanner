#include "application.h"

#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(app, "Application")


Application* Application::s_pInstance = nullptr;

Application::Application(int &argc, char **argv)
    : QGuiApplication(argc, argv)
{
    qCDebug(app, "Create object");
    QObject::connect(this, &QGuiApplication::applicationStateChanged, this, &Application::stateChanged);
}

Application::~Application()
{
    qCDebug(app, "Destroy object");
}

void Application::notice(QString message)
{
    qCDebug(app) << "UI Notice:" << message;
    emit notification("notice", message);
}

void Application::warning(QString message)
{
    qCWarning(app) << "UI Warning:" << message;
    emit notification("warning", message);
}

void Application::error(QString message)
{
    qCWarning(app) << "UI Error:" << message;
    emit notification("error", message);
}

void Application::stateChanged(Qt::ApplicationState state)
{
    qCDebug(app) << "Application state changed" << state;
}
