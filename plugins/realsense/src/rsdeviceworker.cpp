#include "rsdeviceworker.h"

#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QDataStream>
#include <QElapsedTimer>
#include <QTimer>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(rsdeviceworker, "RealSensePlugin::RSDeviceWorker")

//#include "src/camera/cameraposition.h"
//#include "src/settings.h"

#include "rsdevice.h"

RSDeviceWorker::RSDeviceWorker(rs2::pipeline *pipe, rs2::frame_queue *queue, RSDevice *device)
    : m_stopped(true)
    , m_make_shot(false)
    , m_make_shot_series(false)
    , m_mutex()
    , m_pipe(pipe)
    , m_queue(queue)
    , m_device(device)
{
    qCDebug(rsdeviceworker) << "Init RSDeviceWorker";
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
    m_device->setIsStreaming(true);

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

    rs2::frameset frames;

    rs2::depth_frame out_depth_frame = rs2::frame();
    rs2::video_frame out_color_frame = rs2::frame();

    try {
        frame_timer.start();
        while( true ) {
            if( m_pipe == nullptr ) {
                qCWarning(rsdeviceworker) << "Pipeline is not defined";
                break;
            }

            fwt_timer.restart();
            if( !m_pipe->try_wait_for_frames(&frames) ) { // Wait for next set of frames from the camera
                qCDebug(rsdeviceworker) << "No frames received from the pipeline";
                continue;
            }
            if( frames.size() == 0 )
                continue;
            fwt += fwt_timer.nsecsElapsed();

            frames_count++;
            //frames.keep();
            frames_time = frame_timer.elapsed();

            {
                QMutexLocker locker(&m_mutex);
                if( m_stopped )
                    break;
            }

            auto enabled_streams = m_device->getVideoStreams();

            for( VideoSourceStreamObject* stream : enabled_streams ) {
                fpt_timer.restart();
                if( stream->rs2_stream_type == RS2_STREAM_DEPTH ) {
                    rs2::depth_frame depth_frame = frames.get_depth_frame();
                    if( ! depth_frame )
                        continue;
                    //qCDebug(rsdeviceworker) << "Distance to center:" << depth_frame.get_distance(depth_frame.get_width()/2, depth_frame.get_height()/2);

                    out_depth_frame = depth_frame;

                    //out_depth_frame = dec_filter.process(out_depth_frame);
                    if( m_use_disparity_filter )
                        out_depth_frame = depth_to_disparity.process(out_depth_frame);
                    if( m_use_spatial_filter )
                        out_depth_frame = spat_filter.process(out_depth_frame);
                    if( m_use_temporal_filter )
                        out_depth_frame = temp_filter.process(out_depth_frame);
                    if( m_use_disparity_filter )
                        out_depth_frame = disparity_to_depth.process(out_depth_frame);

                    stream->newStreamImage(frameToQImage(color_map.colorize(out_depth_frame)));
                } else if( stream->rs2_stream_type == RS2_STREAM_COLOR ) {
                    out_color_frame = frames.get_color_frame();
                    if( ! out_color_frame )
                        continue;
                    stream->newStreamImage(frameToQImage(out_color_frame));
                } else if( stream->rs2_stream_type == RS2_STREAM_INFRARED ) {
                    rs2::video_frame frame = frames.get_infrared_frame();
                    if( ! frame )
                        continue;
                    stream->newStreamImage(frameToQImage(frame));
                } else if( stream->rs2_stream_type == RS2_STREAM_FISHEYE ) {
                    rs2::video_frame frame = frames.get_fisheye_frame();
                    if( ! frame )
                        continue;
                    stream->newStreamImage(frameToQImage(frame));
                } else
                    qCWarning(rsdeviceworker) << "Unable to process stream" << rs2_stream_to_string((rs2_stream)stream->rs2_stream_type);

                fpt += fpt_timer.nsecsElapsed();

                if( frames_time > 1000 ) {
                    qreal time = frames_time/1000.0;
                    stream->fps(frames_count/time);
                    stream->fwt(fwt/1000000.0/time);
                    stream->fpt(fpt/1000000.0/time);
                }
            }

            if( m_make_shot || m_make_shot_series ) {
                qCDebug(rsdeviceworker) << "Saving pointcloud to memory storage";

                points = pc.calculate(out_depth_frame);

                if( out_color_frame )
                    pc.map_to(out_color_frame);
                else
                    qCWarning(rsdeviceworker) << "No color frame found";

                PointCloudData *pcdata = toPointCloud(points, out_depth_frame, out_color_frame, static_cast<size_t>(out_depth_frame.get_width()));

                if( pcdata != nullptr ) {
                    m_make_shot = false;
                    // Push object to the parent thread to use it there
                    //pcdata->moveToThread(m_device->thread());
                    emit newPointCloudData(pcdata);
                }
            }

            if( frames_time >= 1000 ) {
                frames_count = 0;
                fwt = 0;
                fpt = 0;
                frame_timer.restart();
            }
        }
    } catch( const rs2::error & e ) {
        emit errorOccurred(e.what());
    } catch( const std::exception& e ) {
        emit errorOccurred(e.what());
    }

    m_device->setIsStreaming(false);
    emit stopped();
}

PointCloudData* RSDeviceWorker::toPointCloud(rs2::points points, rs2::depth_frame depth_frame, rs2::video_frame texture, size_t width)
{
    qCDebug(rsdeviceworker) << "Creating a new PointCloudData object";
    const auto vertices = points.get_vertices();

    PointCloudData *out = new PointCloudData();

    out->shoot_time = QDateTime::currentDateTime();
    out->name = QString("shot_").append(out->shoot_time.toString("yyyyMMdd_hhmmss"));

    // TODO: Attach orientation & position data
    // Inverting x axis due to realsense z is positive (opengl z is negative)
    /*QQuaternion orientation(CameraPosition::I()->getQuaternion());
    orientation.setX(-orientation.x());
    out->setOrientation(orientation);
    // Apply the camera offset & inverting x axis
    QVector3D position = CameraPosition::I()->getTranslation();
    position += orientation * QVector3D(float(Settings::I()->val("Camera.Realsense.offset_x").toDouble()),
                                        float(Settings::I()->val("Camera.Realsense.offset_y").toDouble()),
                                        float(Settings::I()->val("Camera.Realsense.offset_z").toDouble()));
    position.setX(-position.x());
    out->setPosition(position);*/

    // Record all the available metadata attributes
    auto device = m_pipe->get_active_profile().get_device();
    for( size_t i = 0; i < RS2_CAMERA_INFO_COUNT; i++ ) {
        if( device.supports(static_cast<rs2_camera_info>(i)) ) {
            QString key(QString("rs2.device.").append(rs2_camera_info_to_string(static_cast<rs2_camera_info>(i))));
            out->metadata[key] = QString(device.get_info(static_cast<rs2_camera_info>(i)));
        }
    }
    for( size_t i = 0; i < RS2_FRAME_METADATA_COUNT; i++ ) {
        if( depth_frame.supports_frame_metadata(static_cast<rs2_frame_metadata_value>(i)) ) {
            QString key(QString("rs2.frame.").append(rs2_frame_metadata_to_string(static_cast<rs2_frame_metadata_value>(i))));
            out->metadata[key] = QString::number(depth_frame.get_frame_metadata(static_cast<rs2_frame_metadata_value>(i)));
        }
    }

    out->points_number = points.size();

    // Record height/width for rectangular pointclouds
    if( width == 0 || points.size() % width != 0) {
        width = out->points_number;
        out->height = 1;
    } else
        out->height = out->points_number / width;
    out->width = width;

    qCDebug(rsdeviceworker) << "Processing point data";

    out->points.reserve(out->points_number*3*sizeof(float));
    for( quint32 i = 0; i < out->points_number; ++i ) {
        // Adding point coordinates
        out->points.append(reinterpret_cast<const char *>(&vertices[i].x), sizeof(float)); // X
        out->points.append(reinterpret_cast<const char *>(&vertices[i].y), sizeof(float)); // Y
        out->points.append(reinterpret_cast<const char *>(&vertices[i].z), sizeof(float)); // Z
    }

    if( texture ) {
        qCDebug(rsdeviceworker) << "Processing color data";
        out->colors.reserve(out->points_number*4);
        const auto texcoords = points.get_texture_coordinates();
        const auto texture_data = reinterpret_cast<const uint8_t*>(texture.get_data());
        for( quint32 i = 0; i < out->points_number; ++i ) {
            // Getting point rgb color
            const int w = texture.get_width(), h = texture.get_height();
            int x = std::min(std::max(int(texcoords[i].u*w + .5f), 0), w - 1);
            int y = std::min(std::max(int(texcoords[i].v*h + .5f), 0), h - 1);
            int idx = x * texture.get_bits_per_pixel() / 8 + y * texture.get_stride_in_bytes();

            out->colors.append(reinterpret_cast<const char *>(&texture_data[idx]), sizeof(uint8_t)); // Red
            out->colors.append(reinterpret_cast<const char *>(&texture_data[idx + 1]), sizeof(uint8_t)); // Green
            out->colors.append(reinterpret_cast<const char *>(&texture_data[idx + 2]), sizeof(uint8_t)); // Blue
            out->colors.append('\xff'); // Alpha
        }
    }

    qCDebug(rsdeviceworker) << "Completed pcdata composing";

    return out;
}

QImage RSDeviceWorker::frameToQImage(const rs2::frame& f)
{
    using namespace rs2;

    auto vf = f.as<video_frame>();
    const int w = vf.get_width();
    const int h = vf.get_height();

    switch( f.get_profile().format() ) {
    case( RS2_FORMAT_RGB8 ):
        return QImage(reinterpret_cast<const uchar *>(f.get_data()), w, h, QImage::Format_RGB888);
    case( RS2_FORMAT_RGBA8 ):
        return QImage(reinterpret_cast<const uchar *>(f.get_data()), w, h, QImage::Format_RGBA8888);
    case( RS2_FORMAT_BGR8 ):
        return QImage(reinterpret_cast<const uchar *>(f.get_data()), w, h, QImage::Format_BGR888);
    case( RS2_FORMAT_Z16 ):
        // Use Format_Grayscale8 since we translating it by color_map.colorize()
        return QImage(reinterpret_cast<const uchar *>(f.get_data()), w, h, QImage::Format_Grayscale8);
    case( RS2_FORMAT_Y16 ):
        // TODO: Fix the incorrect interpretation (part of the image & lines)
        return QImage(reinterpret_cast<const uchar *>(f.get_data()), w, h, QImage::Format_Grayscale16);
    case( RS2_FORMAT_Y8 ):
        return QImage(reinterpret_cast<const uchar *>(f.get_data()), w, h, QImage::Format_Grayscale8);
    default:
        throw std::runtime_error("Frame format is not supported yet!");
    }
}
