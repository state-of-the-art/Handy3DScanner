var currentTool = null

function loadTool(name) {
    toolClean()
    Qt.include(name+".js", function(result) {
        if( result.status !== 0 )
            return console.error("Unable to load tool " + name)

        toolInit()
        console.log("Loaded tool " + name)
    })
}

// Functions will be overridden by loadTool function
function toolClean(){}
function toolInit() {}
function toolHit(hit, camera) {}
