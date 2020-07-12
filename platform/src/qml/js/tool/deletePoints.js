function toolInit() {
    console.log("Tool init")
    if( cfg['UI.show_advise_messages'] )
        app.notice("Click on selected pointcloud point to remove selected points, on not selected - to remove unselected ones")
}

function toolHit(hit, camera) {
    var mesh_point_id = hit.entity.pc.getMeshPointId(hit.type, hit.primitiveIndex)

    if( hit.entity.objectName === "pointcloud" ) {
        if( hit.entity.pc.isPointMarked(mesh_point_id) )
            hit.entity.pc.deleteMarkedPoints()
        else if( hit.entity.pc.isPointsMarked() ) {
            hit.entity.pc.cropToSelection()
            hit.entity.pc.deleteUnmarkedPoints()
            hit.entity.pc.unmarkAll()
        } else
            app.warning("Delete point tool requires at least one selected point on the pointcloud")
    }
}

function toolClean() {
    console.log("Tool clean")
}
