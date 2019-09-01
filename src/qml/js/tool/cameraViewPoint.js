function toolInit() {
    console.log("Tool init")
    if( cfg['UI.show_advise_messages'] )
        app.notice("You can select point on the pointcloud to aim the camera")
}

function toolHit(hit, camera) {
    if( hit.entity.objectName === "pointcloud" ) {
        camera.setViewCenter(Qt.vector3d(hit.worldIntersection.x, hit.worldIntersection.y, hit.worldIntersection.z))
        camera.setUpVector(Qt.vector3d(0.0, -1.0, 0.0)) // To make sure camera will be horizon-aligned
    }
}

function toolClean() {
    console.log("Tool clean")
}
