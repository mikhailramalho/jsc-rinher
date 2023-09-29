function __print(x::Union{Int32, Int64, String, Bool, Tuple{Any, Any}})
    println(x)
    return x
end

function __print(x)
    println("<#closure>");
    return "<#closure>"
end

function __add(x::Int32, y::Int32)
    return x + y
end

function __add(x::String, y::String)
    return x * y
end

function __add(x::String, y::Int32)
    return x * string(y)
end

function __add(x::Int32, y::String)
    return string(x) * y
end

function __sub(x::Int32, y::Int32)
    return x - y
end

function __mul(x::Int32, y::Int32)
    return x * y
end

function __div(x::Int32, y::Int32) ::Int32
    return trunc(Int32, x / y)
end

function __rem(x::Int32, y::Int32)
    return trunc(Int32, x % y)
end

function __eq(x::Bool, y::Bool)
    return x == y
end

function __eq(x::Int32, y::Int32)
    return x == y
end

function __eq(x::String, y::String)
    return x == y
end

function __noteq(x::Bool, y::Bool)
    return x != y
end

function __noteq(x::Int32, y::Int32)
    return x != y
end

function __noteq(x::String, y::String)
    return x != y
end

function __lte(x::Int32, y::Int32)
    return x <= y
end

function __lt(x::Int32, y::Int32)
    return x < y
end

function __gte(x::Int32, y::Int32)
    return x >= y
end

function __gt(x::Int32, y::Int32)
    return x > y
end

function __and(x::Bool, y::Bool)
    return x && y
end

function __or(x::Bool, y::Bool)
    return x || y
end