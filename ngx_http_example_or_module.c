#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static ngx_int_t ngx_http_example_or_add_variables(ngx_conf_t *cf);
static ngx_int_t ngx_http_variable_request_get_limit_rate_after(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static void ngx_http_variable_request_set_size(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);

static ngx_http_module_t ngx_http_example_or_module_ctx = {
    ngx_http_example_or_add_variables,       /* preconfiguration */
    NULL,                                    /* postconfiguration */

    NULL,                                    /* create main configuration */
    NULL,                                    /* init main configuration */

    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */

    NULL,                                    /* create location configuration */
    NULL                                     /* merge location configuration */
};


ngx_module_t ngx_http_example_or_module = {
    NGX_MODULE_V1,
    &ngx_http_example_or_module_ctx,   /* module context */
    NULL,                              /* module directives */
    NGX_HTTP_MODULE,                   /* module type */
    NULL,                              /* init master */
    NULL,                              /* init module */
    NULL,                              /* init process */
    NULL,                              /* init thread */
    NULL,                              /* exit thread */
    NULL,                              /* exit process */
    NULL,                              /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_http_variable_t  ngx_http_example_or_variables[] = {
    { ngx_string("limit_rate_after"), ngx_http_variable_request_set_size,
      ngx_http_variable_request_get_limit_rate_after,
      offsetof(ngx_http_request_t, limit_rate_after),
      NGX_HTTP_VAR_CHANGEABLE|NGX_HTTP_VAR_NOCACHEABLE, 0 },
    { ngx_null_string, NULL, NULL, 0, 0, 0 }
};


static ngx_int_t
ngx_http_example_or_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t  *var, *v;

    for (v = ngx_http_example_or_variables; v->name.len; v++) {
        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        var->get_handler = v->get_handler;
        var->set_handler = v->set_handler;
        var->data = v->data;
    }

    return NGX_OK;
}

static ngx_int_t
ngx_http_variable_request_get_limit_rate_after(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    size_t                    *sp;
    u_char                    *data_end;
    ngx_http_core_loc_conf_t  *clcf;

    sp = (size_t *) ((char *) r + data);

    v->data = ngx_pnalloc(r->pool, NGX_SIZE_T_LEN);
    if (v->data == NULL) {
        return NGX_ERROR;
    }
    if (*sp == 0) {
        // use configured limit_rate_after as default
        clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);
        *sp = clcf->limit_rate_after;
        data_end = ngx_sprintf(v->data, "%uz", clcf->limit_rate_after);
    } else {
        data_end = ngx_sprintf(v->data, "%uz", *sp);
    }

    v->len = data_end - v->data;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


// Just copy it out of Nginx source because the original function is static.
static void
ngx_http_variable_request_set_size(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ssize_t    s, *sp;
    ngx_str_t  val;

    val.len = v->len;
    val.data = v->data;

    s = ngx_parse_size(&val);

    if (s == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "invalid size \"%V\"", &val);
        return;
    }

    sp = (ssize_t *) ((char *) r + data);

    *sp = s;

    return;
}


int
ngx_http_example_or_ffi_get_error_log_level(ngx_http_request_t *r)
{
    ngx_log_t                   *log;
    int                          log_level;

    if (r && r->connection && r->connection->log) {
        log = r->connection->log;

    } else {
        log = ngx_cycle->log;
    }

    log_level = log->log_level;
    if (log_level == NGX_LOG_DEBUG_ALL) {
        log_level = NGX_LOG_DEBUG;
    }

    return log_level;
}
