local ffi = require 'ffi'
local C = ffi.C
local getfenv = getfenv
local tonumber = tonumber


local _M = {}

if not pcall(ffi.typeof, "ngx_http_request_t") then
    ffi.cdef[[
        struct ngx_http_request_s;
        typedef struct ngx_http_request_s  ngx_http_request_t;
    ]]
end

ffi.cdef[[
int ngx_http_example_or_ffi_get_error_log_level(ngx_http_request_t *r);
]]

function _M.get_error_log_level()
    local r = getfenv(0).__ngx_req
    return tonumber(C.ngx_http_example_or_ffi_get_error_log_level(r))
end

return _M
