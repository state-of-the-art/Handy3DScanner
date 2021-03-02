import QtQuick 2.12
import QtQuick.Scene3D 2.12

import Qt3D.Core 2.12
import Qt3D.Render 2.12
import Qt3D.Extras 2.12

Scene3D {
    cameraAspectRatioMode: Scene3D.AutomaticAspectRatio

    function resetPosition() {
        cameraPos.resetFrontAngle()
        cameraPos.resetTranslation()
    }

    Entity {
        id: root

        components: [
            RenderSettings {
                activeFrameGraph: ForwardRenderer {
                    clearColor: Qt.rgba(1, 0, 1, 0)
                    camera: camera_view
                }
            }
        ]

        CylinderMesh {
            id: axis_line
            length: 5.0
            radius: 0.01
            rings: 2
            slices: 4
        }

        Entity {
            components: [ axis_line, xMaterial, xTransform ]

            Transform {
                id: xTransform
                rotation: fromAxisAndAngle(Qt.vector3d(0, 0, 1), -90)
            }

            PhongMaterial {
                id: xMaterial
                ambient: "#f00"
            }
        }
        Entity {
            components: [ axis_line, yMaterial ]

            PhongMaterial {
                id: yMaterial
                ambient: "#0f0"
            }
        }
        Entity {
            components: [ axis_line, zMaterial, zTransform ]

            Transform {
                id: zTransform
                rotation: fromAxisAndAngle(Qt.vector3d(1, 0, 0), 90)
            }

            PhongMaterial {
                id: zMaterial
                ambient: "#00f"
            }
        }

        // Not informative for user
        /*Entity {
            id: position
            components: [
                SphereMesh {
                    radius: 0.04
                    rings: 3
                    slices: 4
                },
                Transform {
                    translation: cameraPos.translation
                },
                PhongMaterial {
                    ambient: "#fff"
                }
            ]
        }*/

        Entity {
            components: [
                Transform {
                    id: objRot
                    // TODO: Reenable functionality
                    //rotation: cameraPos.quaternion
                }
            ]

            Camera {
                id: camera_view
                projectionType: CameraLens.PerspectiveProjection
                aspectRatio: 1
                nearPlane : 0.1
                farPlane : 10.0
                position: Qt.vector3d( 0.0, 0.0, 3.0 )
                upVector: Qt.vector3d( 0.0, 1.0, 0.0 )
                viewCenter: Qt.vector3d( 0.0, 0.0, 0.0 )
            }
        }
    }
}
