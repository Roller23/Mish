# Mish
Mish stands for Multitasked Interpreter Serving Http

## Compiling

UNIX-like machines should be able to compile the whole project by running `make`.

While the interpreter part of the project is Windows compatible, the server part is not due to POSIX threads and sockets. Windows users are encouraged to use WSL, MinGW, or Cygwin.

## Installing

WIP

## Running

Simply run the `mish` command in the directory with your web server files.

## Configuration

There are serveral options that allow you to configure the server. The available CLI flags are:

- `--port=<number>` - sets the port number the HTTP server will be running on. The default port is 8080.
- `--max_connections=<number>` - sets the maximum number of pending connections. The default value is 1000.
- `--request_size=<number>` - sets the maximum size of body in requests in megabytes. The default size is 8MB.
- `--headers_size=<number>` - sets the maximum size of headers in requests in kilobytes. The default size is 16KB.
- `--enable_cors=true` – enables cross-origin resource sharing for all responses.
- `--threads=<number>` - sets the size of the thread pool. The default value is the number of processing cores on your machine.
- `--config_file=<string>` - sets the configuration file path. The default path is “<current_path>/server.config”.
- `--root=<string>` - sets a custom root path for serving files. The default path is the current path.
- `--disable_logs=true` – disables logging additional information about incoming requests to the console. Under heavy loads this will speed up the server a bit.

All of these options are configurable through a `server.config`, although the syntax is a bit different there, e.g.:

```
port: 80
disable_logs: true
enable_cors: true
```