cc = cc or {}

-- constant
cc.MAX_INT = 2147483647
cc.MAX_FLOAT = 3.4028235E38

function cc.d2r(d)
    return d * 0.01745329252
end

function cc.r2d(r)
    return r * 57.29577951
end

function cc.clamp(v, min, max)
    return math.min(math.max(v, min), max)
end