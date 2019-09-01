(function() {
    return {
        'validate': function(OPT, data) {
            var is_float = typeof data === "number" && data % 1 !== 0
            var is_in_range = OPT.min <= data && OPT.max >= data
            return is_float && is_in_range
        }
    }
})()
