.pragma library

// Makes some ugly string beautiful
function beautifulName(name, separator = ' ') {
    return name.split('_').map( it => it.charAt(0).toUpperCase() + it.slice(1) ).join(separator)
}
