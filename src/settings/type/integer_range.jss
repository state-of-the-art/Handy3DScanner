(function() {
    return {
        'validate': function(OPT, data) {
            var is_integer = typeof data === "number" && data === parseInt(data)
            var is_in_range = OPT.min <= data && OPT.max >= data
            return is_integer && is_in_range
        }
    }
})()
