本项目是 [为 OpenResty 项目编写自定义 Nginx C 模块](https://segmentfault.com/a/1190000010637226) 一文中的示例代码。

### 包含内容

```
├── config # Nginx 模块编译配置
├── lib
│   └── example_or.lua # 通过 FFI 调用 Nginx C 函数的 Lua 代码库
├── ngx_http_example_or_module.c # Nginx C 模块代码文件
└── t
    └── example_or.t # 测试代码
```

### 如何安装一个 Nginx C 模块

##### 静态链接

编译 OpenResty 时，在 `./configure` 最后添加一个小尾巴：`--add-module=/path/to/ngx_http_example_or_module`。

##### 动态加载

切换 OpenResty 源码安装包下的 `bundle/nginx-*` 目录，执行 Nginx 的 `./configure`，带上 `--add-dynamic-module=/path/to/your/module` 参数。注意此处指定的参数需要同实际编译OpenResty 时一致（但移除仅跟 OpenResty 相关的选项）。因为 Nginx 在编译动态链接库时，需要确保 ABI 上的兼容（包括是否开启 ssl，额外引入的模块，等等）。比如：

```
./configure --with-cc-opt='-I/usr/local/Cellar/pcre/8.39/include/' --with-ld-opt='-L/usr/local/Cellar/pcre/8.39/lib' --with-openssl=/Users/lzx/lib/openssl-1.0.2h --with-http_realip_module --with-http_ssl_module --add-dynamic-module=/path/to/ngx_http_example_or_module
```

然后运行 `make modules`。

最后在配置文件中加载编译出的动态链接库即可。
```
load_module /path/to/objs/ngx_http_example_or_module.so;
```

### 运行测试

为了保证示例的正确性，我编写了相关的测试代码。
运行测试的方式：首先确保 Nginx 已经通过静态链接的方式加载了示例模块，然后安装 `test-nginx`，运行 `prove -r t`。
