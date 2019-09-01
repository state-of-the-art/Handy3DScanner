function toolInit() {
    console.log("Tool init")
    if( cfg['UI.show_advise_messages'] )
        app.notice("Click on pointcloud point to select it")
}

function toolHit(hit, camera) {
    var mesh_point_id = hit.entity.pc.getMeshPointId(hit.type, hit.primitiveIndex)
    hit.entity.pc.markPoint(mesh_point_id)
}

function toolClean() {
    console.log("Tool clean")
}
