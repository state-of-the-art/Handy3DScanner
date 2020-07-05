#include "cameraposition.h"

#include "settings.h"
#include "application.h"
#ifdef ANDROID
//#include "provider/arcore/arcore.h"
#endif

#include <QDebug>
#include <QRotationSensor>
#include <Qt3DCore/QTransform>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(camerapos, "CameraPosition")

#include <QtMath>

CameraPosition* CameraPosition::s_pInstance = nullptr;

CameraPosition::CameraPosition(QObject *parent)
    : QObject(parent)
    , m_support_rotation(false)
    , m_support_position(false)
    , m_rotation_sensor(nullptr)
{
    qCDebug(camerapos, "Create object");

    if( Settings::I()->val("Position.Gyroscope.enable_gyroscope").toBool() )
        checkGyro();
#ifdef ANDROID
    if( Settings::I()->val("Position.ARCore.enable_arcore").toBool() )
        QObject::connect(ARCore::I(), &ARCore::initializedChanged, this, &CameraPosition::initARCore);
#endif
}

CameraPosition::~CameraPosition()
{
    qCDebug(camerapos, "Destroy object");
    delete m_rotation_sensor;
}

#ifdef ANDROID
void CameraPosition::initARCore()
{
    qCDebug(camerapos, "Init ARCore positioning...");
    if( m_rotation_sensor )
        m_rotation_sensor->stop();

    setSupportPosition(true);
    setSupportRotation(true);

    QObject::connect(ARCore::I(), &ARCore::cameraTransformChanged, this, &CameraPosition::setARCore);
}
#endif

void CameraPosition::checkGyro()
{
    qDebug() << "DEBUG!!! Available sensors:" << QSensor::sensorTypes();
    if( QSensor::sensorTypes().contains(QRotationSensor::type) )
        initGyro();
    else
        Application::I()->notice("Your system is not supporting gyro sensor");
}

void CameraPosition::initGyro()
{
    qCDebug(camerapos, "Init gyroscope positioning...");
    m_rotation_sensor = new QRotationSensor(this);

    QObject::connect(m_rotation_sensor, &QRotationSensor::readingChanged, this, &CameraPosition::setGyro);
    m_rotation_sensor->setAxesOrientationMode(QSensor::UserOrientation);
    m_rotation_sensor->setUserOrientation(0);
    m_rotation_sensor->setSkipDuplicates(false);
    m_rotation_sensor->start();

    if( m_rotation_sensor->isActive() )
        setSupportRotation(true);
}

void CameraPosition::resetFrontAngle()
{
    // Getting the current Y rotation and store it as the origin quaternion
    m_quaternion_origin = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, -m_quaternion.toEulerAngles().y());
}

void CameraPosition::resetTranslation()
{
    // Set current translation as the origin
    m_translation_origin = m_translation;
}

#ifdef ANDROID
void CameraPosition::setARCore()
{
    QQuaternion new_quaternion(ARCore::I()->getCameraQuaternion());
    QVector3D new_translation(ARCore::I()->getCameraTranslation());

    if( !m_quaternion.isIdentity() && (
         m_quaternion.vector().distanceToPoint(new_quaternion.vector()) > 0.1f ||
         qAbs(m_quaternion.scalar() - new_quaternion.scalar()) > 0.1f ||
         m_translation.distanceToPoint(new_translation) > 0.05f) )
        Application::I()->warning("Rough position change detected, please be gentle");

    m_quaternion = new_quaternion;
    m_translation = ARCore::I()->getCameraTranslation();

    emit quaternionChanged();
    emit translationChanged();
}
#endif

void CameraPosition::setGyro()
{
    QRotationReading *r = m_rotation_sensor->reading();

    Qt3DCore::QTransform tr;
    tr.setRotationZ(-r->x()); // Roll
    tr.setRotationX(-(r->y() + 90.0f)); // Up/Down
    tr.setRotationY(r->z()); // Left/Right

    QQuaternion new_quaternion(tr.rotation().scalar(), tr.rotation().vector());

    if( !m_quaternion.isIdentity() && (
         m_quaternion.vector().distanceToPoint(new_quaternion.vector()) > 0.1f ||
         qAbs(m_quaternion.scalar() - new_quaternion.scalar()) > 0.1f) )
        Application::I()->warning("Rough position change detected, please be gentle");

    m_quaternion = new_quaternion;

    emit quaternionChanged();
}
