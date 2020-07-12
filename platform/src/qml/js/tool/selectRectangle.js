var previous_point = null

function toolInit() {
    console.log("Tool init")
    if( cfg['UI.show_advise_messages'] )
        app.notice("To select rectangle you need to click on start and end point of the pointcloud")
}

function toolHit(hit, camera) {
    var mesh_point_id = hit.entity.pc.getMeshPointId(hit.type, hit.primitiveIndex)

    hit.entity.pc.markPoint(mesh_point_id)
    if( previous_point === null ) {
        previous_point = mesh_point_id
        if( cfg['UI.show_advise_messages'] )
            app.notice("Now you can choose end point to complete selection")
    } else {
        hit.entity.pc.markRectangle(previous_point, mesh_point_id)
        previous_point = null
    }
}

function toolClean() {
    console.log("Tool clean")
    previous_point = null
}
