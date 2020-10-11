#include "rsdeviceworker.h"

#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QDataStream>
#include <QElapsedTimer>
#include <QTimer>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(rsdeviceworker, "RealSensePlugin::RSDeviceWorker")

//#include "src/camera/pointcloud.h"
//#include "src/camera/cameraposition.h"
//#include "src/settings.h"

#include "rsdevice.h"

RSDeviceWorker::RSDeviceWorker(rs2::pipeline *pipe, rs2::frame_queue *queue, RSDevice *device)
    : m_mutex()
    , m_pipe(pipe)
    , m_queue(queue)
    , m_device(device)
{
    qCDebug(rsdeviceworker) << "Init RSDeviceWorker";
}

void RSDeviceWorker::makeShot()
{
    m_make_shot = true;
}

void RSDeviceWorker::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stopped = true;
}

void RSDeviceWorker::setPipeline(rs2::pipeline *pipe)
{
    m_pipe = pipe;
}

void RSDeviceWorker::doWork()
{
    rs2::colorizer color_map;
    m_stopped = false;

    m_use_disparity_filter = true;//Settings::I()->value("Camera.Streams.Depth.use_disparity_filter").toBool();
    m_use_spatial_filter = true;//Settings::I()->value("Camera.Streams.Depth.use_spatial_filter").toBool();
    m_use_temporal_filter = true;//Settings::I()->value("Camera.Streams.Depth.use_temporal_filter").toBool();

    // Filters to improve the quality
    rs2::decimation_filter dec_filter;                 // Decimation - reduces depth frame density
    rs2::spatial_filter spat_filter;                   // Spatial    - edge-preserving spatial smoothing
    rs2::temporal_filter temp_filter;                  // Temporal   - reduces temporal noise

    rs2::disparity_transform depth_to_disparity(true);  // Declare disparity transform from depth to disparity
    rs2::disparity_transform disparity_to_depth(false);

    rs2::pointcloud pc;
    rs2::points points;

    // Calculating time parameters
    qint16 frames_count = 0;
    QElapsedTimer frame_timer;
    qint64 frames_time = 0;

    QElapsedTimer fwt_timer; // Frame wait time
    qint64 fwt = 0;
    QElapsedTimer fpt_timer; // Frame processing time
    qint64 fpt = 0;

    try {
        frame_timer.start();
        while( true ) {
            if( m_pipe == nullptr ) {
                qCWarning(rsdeviceworker) << "Pipeline is not defined";
                break;
            }

            frames_time = frame_timer.elapsed();
            if( frames_time > 1000 ) {
                qreal time = frames_time/1000.0;
                emit streamFPS(frames_count/time);
                emit streamFWT(fwt/1000000.0/time);
                emit streamFPT(fpt/1000000.0/time);
                frames_count = 0;
                fwt = 0;
                fpt = 0;
                frame_timer.restart();
            }

            fwt_timer.restart();
            rs2::frameset frames = m_pipe->wait_for_frames(); // Wait for next set of frames from the camera
            fwt += fwt_timer.nsecsElapsed();

            frames_count++;

            fpt_timer.restart();
            frames.keep();

            {
                QMutexLocker locker(&m_mutex);
                if( m_stopped ) {
                    emit stopped();
                    break;
                }
            }

            auto enabled_streams = m_device->getVideoStreams();

            for( VideoSourceStreamObject* stream : enabled_streams ) {
                if( stream->rs2_stream_type == RS2_STREAM_DEPTH ) {
                    rs2::depth_frame depth_frame = frames.get_depth_frame();
                    if( ! depth_frame )
                        continue;
                    //qCDebug(rsdeviceworker) << "Distance to center:" << depth_frame.get_distance(depth_frame.get_width()/2, depth_frame.get_height()/2);

                    rs2::depth_frame filtered = depth_frame;

                    //filtered = dec_filter.process(filtered);
                    if( m_use_disparity_filter )
                        filtered = depth_to_disparity.process(filtered);
                    if( m_use_spatial_filter )
                        filtered = spat_filter.process(filtered);
                    if( m_use_temporal_filter )
                        filtered = temp_filter.process(filtered);
                    if( m_use_disparity_filter )
                        filtered = disparity_to_depth.process(filtered);

                    emit stream->newStreamImage(frameToQImage(color_map.colorize(filtered)));
                    break;
                } else if( stream->rs2_stream_type == RS2_STREAM_COLOR ) {
                    rs2::video_frame frame = frames.get_color_frame();
                    if( ! frame )
                        continue;
                    emit stream->newStreamImage(frameToQImage(frame));
                    break;
                } else if( stream->rs2_stream_type == RS2_STREAM_INFRARED ) {
                    rs2::video_frame frame = frames.get_infrared_frame();
                    if( ! frame )
                        continue;
                    emit stream->newStreamImage(frameToQImage(frame));
                    break;
                } else if( stream->rs2_stream_type == RS2_STREAM_FISHEYE ) {
                    rs2::video_frame frame = frames.get_fisheye_frame();
                    if( ! frame )
                        continue;
                    emit stream->newStreamImage(frameToQImage(frame));
                    break;
                } else
                    qCWarning(rsdeviceworker) << "Unable to process stream" << rs2_stream_to_string((rs2_stream)stream->rs2_stream_type);
            }

            fpt += fpt_timer.nsecsElapsed();

            /*if( m_make_shot ) {
                qCDebug(rsdeviceworker) << "Saving pointcloud to memory storage";

                points = pc.calculate(filtered);

                rs2::video_frame color = frames.get_color_frame();
                if( color )
                    pc.map_to(color);
                else
                    qCWarning(rsdeviceworker) << "No color frame found";

                PointCloud *pc = toPointCloud(points, depth_frame, color, static_cast<size_t>(filtered.get_width()));

                if( pc != nullptr ) {
                    m_make_shot = false;
                    // Push object to the parent thread to use it there
                    pc->moveToThread(m_device->thread());
                    emit newPointCloud(pc);
                }
            }*/
        }
    } catch( const rs2::error & e ) {
        emit errorOccurred(e.what());
        emit stopped();
    } catch( const std::exception& e ) {
        emit errorOccurred(e.what());
        emit stopped();
    }
}

/*PointCloud* RSDeviceWorker::toPointCloud(rs2::points points, rs2::depth_frame depth_frame, rs2::video_frame texture, size_t width)
{
    qCDebug(rsworker) << "Creating a new PointCloud object";
    const auto vertices = points.get_vertices();

    PointCloud *out = new PointCloud();
    out->setShootTime(QDateTime::currentDateTime());
    // Inverting x axis due to realsense z is positive (opengl z is negative)
    QQuaternion orientation(CameraPosition::I()->getQuaternion());
    orientation.setX(-orientation.x());
    out->setOrientation(orientation);
    // Apply the camera offset & inverting x axis
    QVector3D position = CameraPosition::I()->getTranslation();
    position += orientation * QVector3D(float(Settings::I()->val("Camera.Realsense.offset_x").toDouble()),
                                        float(Settings::I()->val("Camera.Realsense.offset_y").toDouble()),
                                        float(Settings::I()->val("Camera.Realsense.offset_z").toDouble()));
    position.setX(-position.x());
    out->setPosition(position);
    out->setName(QString("shot_").append(out->getShootTime().toString("yyyyMMdd_hhmmss")));

    // Record all the available metadata attributes
    auto device = m_pipe->get_active_profile().get_device();
    for( size_t i = 0; i < RS2_CAMERA_INFO_COUNT; i++ ) {
        if( device.supports(static_cast<rs2_camera_info>(i)) ) {
            QString key(QString("rs2.device.").append(rs2_camera_info_to_string(static_cast<rs2_camera_info>(i))));
            out->addMetadata(key, QString(device.get_info(static_cast<rs2_camera_info>(i))));
        }
    }
    for( size_t i = 0; i < RS2_FRAME_METADATA_COUNT; i++ ) {
        if( depth_frame.supports_frame_metadata(static_cast<rs2_frame_metadata_value>(i)) ) {
            QString key(QString("rs2.frame.").append(rs2_frame_metadata_to_string(static_cast<rs2_frame_metadata_value>(i))));
            out->addMetadata(key, QString::number(depth_frame.get_frame_metadata(static_cast<rs2_frame_metadata_value>(i))));
        }
    }

    quint32 points_number = static_cast<quint32>(points.size());

    if( width == 0 || points.size() % width != 0) {
        width = points.size();
        out->setHeight(1);
    } else
        out->setHeight(points_number / width);
    out->setWidth(static_cast<quint32>(width));

    out->setPointsNumber(points_number);

    qCDebug(rsworker) << "Processing point data";

    QByteArray vertex_buffer;
    vertex_buffer.reserve(points_number*3*sizeof(float));
    for( quint32 i = 0; i < points_number; ++i ) {
        // Adding point coordinates
        vertex_buffer.append(reinterpret_cast<const char *>(&vertices[i].x), sizeof(float)); // X
        vertex_buffer.append(reinterpret_cast<const char *>(&vertices[i].y), sizeof(float)); // Y
        vertex_buffer.append(reinterpret_cast<const char *>(&vertices[i].z), sizeof(float)); // Z
    }
    out->setPCPoints(vertex_buffer);

    qCDebug(rsworker) << "Point data is set";

    if( texture ) {
        qCDebug(rsworker) << "Processing color data";
        QByteArray color_buffer;
        color_buffer.reserve(points_number*4);
        const auto texcoords = points.get_texture_coordinates();
        const auto texture_data = reinterpret_cast<const uint8_t*>(texture.get_data());
        for( qint32 i = 0; i < points_number; ++i ) {
            // Getting point rgb color
            const int w = texture.get_width(), h = texture.get_height();
            int x = std::min(std::max(int(texcoords[i].u*w + .5f), 0), w - 1);
            int y = std::min(std::max(int(texcoords[i].v*h + .5f), 0), h - 1);
            int idx = x * texture.get_bits_per_pixel() / 8 + y * texture.get_stride_in_bytes();

            color_buffer.append(reinterpret_cast<const char *>(&texture_data[idx]), sizeof(uint8_t)); // Red
            color_buffer.append(reinterpret_cast<const char *>(&texture_data[idx + 1]), sizeof(uint8_t)); // Green
            color_buffer.append(reinterpret_cast<const char *>(&texture_data[idx + 2]), sizeof(uint8_t)); // Blue
            color_buffer.append('\xff'); // Alpha
        }
        out->setPCColor(color_buffer);
    }

    return out;
}*/

QImage RSDeviceWorker::frameToQImage(const rs2::frame& f)
{
    using namespace rs2;

    auto vf = f.as<video_frame>();
    const int w = vf.get_width();
    const int h = vf.get_height();

    switch( f.get_profile().format() ) {
    case( RS2_FORMAT_RGB8 ):
        return QImage(reinterpret_cast<const uchar *>(f.get_data()), w, h, QImage::Format_RGB888);
    case( RS2_FORMAT_Z16 ):
        return QImage(reinterpret_cast<const uchar *>(f.get_data()), w, h, QImage::Format_Grayscale8);
    case( RS2_FORMAT_Y16 ):
        return QImage(reinterpret_cast<const uchar *>(f.get_data()), w, h, QImage::Format_Grayscale8);
    default:
        throw std::runtime_error("Frame format is not supported yet!");
    }
}


