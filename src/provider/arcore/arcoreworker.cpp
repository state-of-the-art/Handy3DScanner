#include "arcoreworker.h"

#include "arcore_c_api.h"

#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QOffscreenSurface>
#include <QThread>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(arcoreworker, "ARCoreWorker")


ARCoreWorker::ARCoreWorker()
    : m_session(nullptr)
    , m_frame(nullptr)
{
    qCDebug(arcoreworker, "Create object");
}

void ARCoreWorker::setSession(ARCoreWorker::ArSession **session)
{
    m_session = *session;
}

void ARCoreWorker::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stopped = true;
}

void ARCoreWorker::doWork()
{
    qCDebug(arcoreworker, "Running worker process");
    m_stopped = false;

    // Init opengl context required for ARCore
    QOpenGLContext context;
    qCDebug(arcoreworker) << "Context format:" << context.format();
    context.create();
    if( !context.isValid() ) {
        qCWarning(arcoreworker) << "Unable to create GL context";
        return;
    }

    QOffscreenSurface surface;
    qCDebug(arcoreworker) << "Surface format:" << surface.format();
    surface.create();
    if( !surface.isValid() ) {
        qCWarning(arcoreworker) << "Unable to create offscreen surface";
        return;
    }

    context.makeCurrent(&surface);

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    ArSession_setCameraTextureName(m_session, texture_id);

    // Resume ARCore session
    if( ArSession_resume(m_session) != AR_SUCCESS ) {
        qCWarning(arcoreworker) << "Unable to resume the session";
        return;
    }

    float cam_pose_quattrans[7];

    ArFrame_create(m_session, &m_frame);
    if( !m_frame ) {
        qCWarning(arcoreworker) << "Unable create a frame";
        return;
    }


    qCDebug(arcoreworker, "Starting main loop");
    try {
        while( true ) {
            if( m_session == nullptr ) {
                qCWarning(arcoreworker) << "Session is not defined";
                break;
            }

            {
                QMutexLocker locker(&m_mutex);
                if( m_stopped ) {
                    emit stopped();
                    break;
                }
            }

            if( ArSession_update(m_session, m_frame) != AR_SUCCESS ) {
                qCWarning(arcoreworker) << "Unable to get a new frame";
                thread()->sleep(1000);
                //m_stopped = true;
            }

            // Get camera position
            ArCamera* ar_camera;
            ArFrame_acquireCamera(m_session, m_frame, &ar_camera);
            if( ar_camera ) {
                ArPose* camera_pose = nullptr;
                ArPose_create(m_session, nullptr, &camera_pose);
                ArCamera_getPose(m_session, ar_camera, camera_pose);

                ArPose_getPoseRaw(m_session, camera_pose, cam_pose_quattrans);
                ArPose_destroy(camera_pose);
                emit cameraMatrix(cam_pose_quattrans);
            } else
                qCWarning(arcoreworker) << "Unable acquire a camera";

            ArCamera_release(ar_camera);
        }
    } catch( const std::exception& e ) {
        qCWarning(arcoreworker) << "Error occurred:" << e.what();
        emit errorOccurred(e.what());
        emit stopped();
    }

    ArSession_resume(m_session);

    ArFrame_destroy(m_frame);

    qCDebug(arcoreworker, "Ending main loop");
}
