# vim:set ft=perl

use Test::Nginx::Socket::Lua 'no_plan';
use Cwd qw(cwd);

log_level('error');
repeat_each(2);

my $pwd = cwd();
add_block_preprocessor(sub {
    my $block = shift;

    my $http_config = $block->http_config || '';
    my $init_by_lua_block = $block->init_by_lua_block || '';

    $http_config .= <<_EOC_;

    lua_package_path "$pwd/lib/?.lua;;";
    init_by_lua_block {
        $init_by_lua_block
    }
_EOC_

    $block->set_value("http_config", $http_config);
});

run_tests();

__DATA__

=== TEST 1: change limit_rate_after dynamically
--- config
    location /t {
        content_by_lua_block {
            ngx.var.limit_rate_after = '1K'
            ngx.var.limit_rate = '5'
            ngx.say(("hello"):rep(10))
        }
    }
--- request
GET /t
--- timeout: 1
--- response_body
hellohellohellohellohellohellohellohellohellohello



=== TEST 2: get current limit_rate_after
--- config
    location /t {
        limit_rate_after 2K;
        content_by_lua_block {
            local res = ngx.location.capture("/sub")
            ngx.say(res.body)
            ngx.say(ngx.var.limit_rate_after)
            ngx.var.limit_rate_after = '1K'
            ngx.say(ngx.var.limit_rate_after)
        }
    }

    location /sub {
        content_by_lua_block {
            ngx.print(ngx.var.limit_rate_after)
        }
    }
--- request
GET /t
--- response_body
0
2048
1024


=== TEST 3: the error log level is "debug"
--- config
    location /t {
        content_by_lua_block {
            local example_or = require "example_or"
            ngx.print('Is "debug" the error log level? ',
                      example_or.get_error_log_level() == ngx.DEBUG)
        }
    }
--- log_level: debug
--- request
GET /t
--- response_body chomp
Is "debug" the error log level? true



=== TEST 4: the error log level is "emerg"
--- config
    location /t {
        content_by_lua_block {
            local example_or = require "example_or"
            ngx.print('Is "emerg" the error log level? ',
                      example_or.get_error_log_level() == ngx.EMERG)
        }
    }
--- log_level: emerg
--- request
GET /t
--- response_body chomp
Is "emerg" the error log level? true



=== TEST 5: get error log level during Nginx starts (init)
--- init_by_lua_block
    local example_or = require "example_or"
    package.loaded.log_level = example_or.get_error_log_level()

--- config
    location /t {
        content_by_lua_block {
            local log_level = package.loaded.log_level

            if log_level >= ngx.WARN then
                ngx.log(ngx.WARN, "log a warning event")
            else
                ngx.log(ngx.WARN, "do not log another warning event")
            end
        }
    }
--- log_level: warn
--- request
GET /t
--- error_log
log a warning event
--- no_error_log
do not log another warning event
