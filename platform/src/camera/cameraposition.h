#ifndef CAMERAPOSITION_H
#define CAMERAPOSITION_H

#include <QObject>
#include <QVector3D>
#include <QQuaternion>

class QRotationSensor;

class CameraPosition: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVector3D translation READ getTranslation NOTIFY translationChanged)
    Q_PROPERTY(QQuaternion quaternion READ getQuaternion NOTIFY quaternionChanged)
    Q_PROPERTY(bool isSupportRotation READ isSupportRotation NOTIFY supportRotationChanged)
    Q_PROPERTY(bool isSupportPosition READ isSupportPosition NOTIFY supportPositionChanged)

public:
    inline static CameraPosition* I() { if( s_pInstance == nullptr ) s_pInstance = new CameraPosition(); return s_pInstance; }
    inline static void destroyI() { delete s_pInstance; }

    const QVector3D getTranslation() { return m_translation - m_translation_origin; }
    const QQuaternion getQuaternion() { return m_quaternion_origin * m_quaternion; }

    bool isSupportRotation() { return m_support_rotation; }
    bool isSupportPosition() { return m_support_position; }

    Q_INVOKABLE void resetFrontAngle();
    Q_INVOKABLE void resetTranslation();

signals:
    void quaternionChanged();
    void translationChanged();
    void supportRotationChanged();
    void supportPositionChanged();

private slots:
#ifdef ANDROID
    // TODO: Restore functional
    //void setARCore();
#endif
    void checkGyro();
    void setGyro();

private:
    static CameraPosition *s_pInstance;
    explicit CameraPosition(QObject *parent = nullptr);
    ~CameraPosition() override;

#ifdef ANDROID
    // TODO: Restore functional
    //void initARCore();
#endif
    void initGyro();

    void setSupportRotation(bool value) { if( m_support_rotation != value ) { m_support_rotation = value; emit supportRotationChanged(); } }
    void setSupportPosition(bool value) { if( m_support_position != value ) { m_support_position = value; emit supportPositionChanged(); } }

    QVector3D m_translation;
    QVector3D m_translation_origin;
    QQuaternion m_quaternion;
    QQuaternion m_quaternion_origin;

    bool m_support_rotation;
    bool m_support_position;

    // For gyro
    QRotationSensor *m_rotation_sensor;
};

#endif // CAMERAPOSITION_H
