import QtQuick 2.12
import QtQuick.Scene3D 2.12

import Qt3D.Core 2.12
import Qt3D.Render 2.12
import Qt3D.Input 2.12
import Qt3D.Extras 2.12

import "js/tool/tool.js" as Tool

Scene3D {
    id: scene_3d
    cameraAspectRatioMode: Scene3D.AutomaticAspectRatio

    property var cameraView: camera_view
    property real pointSize: cfg['UI.Edit.PointCloud.point_size']
    property var primitiveViewType: cfg['UI.Edit.display_mode'] === 'POINTS' ? GeometryRenderer.Points : GeometryRenderer.Triangles
    property var caster: caster_obj

    onPrimitiveViewTypeChanged: {
        // Going trough the pointclouds and switching index attribute
        // TODO: Reenable functionality
        /*for( var i in camera.pointclouds ) {
            camera.pointclouds[i].switchIndexAttribute(primitiveViewType !== GeometryRenderer.Points)
        }*/
    }

    function getRoot() {
        return root
    }

    function getPointCloudEntity(pc) {
        for( var i in root.childNodes ) {
            var child = root.childNodes[i]
            if( child && child.objectName === "pointcloud" && child.pc === pc )
                return child
        }
        return null
    }

    function showPointCloud(pc) {
        var pointcloud = getPointCloudEntity(pc)
        if( pointcloud )
            pointcloud.enabled = true
        else {
            console.log("Creating new PointCloud entity " + pc.name)
            var ent = pointCloudComponent.createObject(root, { "pc": pc })
            if( ent === null )
                console.log("Error creating PointCloud entity")
        }
    }

    function hidePointCloud(pc) {
        getPointCloudEntity(pc).enabled = false
    }

    function setTool(name) {
        Tool.loadTool(name)
    }

    Entity {
        id: root

        Camera {
            id: camera_view
            projectionType: CameraLens.PerspectiveProjection
            fieldOfView: 45
            aspectRatio: 16/9
            nearPlane : 0.001
            farPlane : 1000.0
            position: Qt.vector3d( 0.0, 0.0, -1.0 )
            upVector: Qt.vector3d( 0.0, -1.0, 0.0 )
            viewCenter: Qt.vector3d( 0.0, 0.0, 0.0 )
        }

        components: [
            RenderSettings {
                activeFrameGraph: ForwardRenderer {
                    clearColor: "#444444"
                    camera: camera_view
                }
                renderPolicy: scene_3d.visible ? RenderSettings.Always : RenderSettings.OnDemand

                pickingSettings.pickMethod: PickingSettings.PrimitivePicking
                pickingSettings.faceOrientationPickingMode: PickingSettings.FrontAndBackFace
                pickingSettings.pickResultMode: PickingSettings.NearestPick

                // https://bugreports.qt.io/browse/QTBUG-75493
                //pickingSettings.worldSpaceTolerance: 0.01
                Component.onCompleted: pickingSettings.worldSpaceTolerance = 0.01 // To be able to run on Qt > 5.12.0
            },
            ScreenRayCaster {
                id: caster_obj
                onHitsChanged: {
                    if( hits.length > 0 ) {
                        console.log("We've got hits: " + hits.length)
                        console.log("Hit type: " + hits[0].type)
                        console.log("Hit primitive index: " + hits[0].primitiveIndex)
                        console.log("Pos: "+ hits[0].localIntersection.x + " " + hits[0].localIntersection.y + " " + hits[0].localIntersection.z)

                        Tool.toolHit(hits[0], camera_view)
                    }
                }
            }
        ]

        Material {
            id: pointcloudMaterial
            effect: Effect {
                FilterKey {
                    id: forward
                    name: "renderingStyle"
                    value: "forward"
                }
                techniques: [
                    Technique {
                        filterKeys: [forward]
                        renderPasses: RenderPass {
                            shaderProgram: ShaderProgram {
                                vertexShaderCode: loadSource("qrc:/qml/shaders/es3/pointcloud.vert")
                                fragmentShaderCode: loadSource("qrc:/qml/shaders/es3/pointcloud.frag")
                            }
                        }
                        graphicsApiFilter {
                            api: GraphicsApiFilter.OpenGLES
                            profile: GraphicsApiFilter.CoreProfile
                            majorVersion: 3
                            minorVersion: 0
                        }
                    }
                ]
            }
            parameters: [
                Parameter { name: "pointSize"; value: pointSize }
            ]
        }

        Component {
            id: pointCloudComponent

            Entity {
                objectName: "pointcloud"
                components: [ pointcloudMesh, pointcloudTransform, pointcloudMaterial ]

                property var pc

                GeometryRenderer {
                    id: pointcloudMesh
                    primitiveType: primitiveViewType

                    geometry: pc.geometry
                }

                Transform {
                    id: pointcloudTransform
                    translation: pc.objectPosition
                    rotation: pc.objectOrientation
                }
            }
        }
    }
}
