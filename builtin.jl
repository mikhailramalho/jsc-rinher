function __print(x::Union{Int, String, Bool, Tuple{Any, Any}})
    println(x)
    return x
end

function __print(x)
    println("<#closure>");
    return "<#closure>"
end

function __add(x::Int, y::Int)
    return x + y
end

function __add(x::String, y::String)
    return x * y
end

function __add(x::String, y::Int)
    return x * string(y)
end

function __add(x::Int, y::String)
    return string(x) * y
end

function __sub(x::Int, y::Int)
    return x - y
end

function __mul(x::Int, y::Int)
    return x * y
end

function __div(x::Int, y::Int) ::Int
    return trunc(Int, x / y)
end

function __rem(x::Int, y::Int)
    return trunc(Int, x % y)
end

function __eq(x::Bool, y::Bool)
    return x == y
end

function __eq(x::Int, y::Int)
    return x == y
end

function __eq(x::String, y::String)
    return x == y
end

function __noteq(x::Bool, y::Bool)
    return x != y
end

function __noteq(x::Int, y::Int)
    return x != y
end

function __noteq(x::String, y::String)
    return x != y
end

function __lte(x::Int, y::Int)
    return x <= y
end

function __lt(x::Int, y::Int)
    return x < y
end

function __gte(x::Int, y::Int)
    return x >= y
end

function __gt(x::Int, y::Int)
    return x > y
end

function __and(x::Bool, y::Bool)
    return x && y
end

function __or(x::Bool, y::Bool)
    return x || y
end