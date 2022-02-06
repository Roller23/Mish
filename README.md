# Mish
Mish stands for Multitasked Interpreter Serving Http

## Compiling

UNIX-like machines should be able to compile the whole project by running `make`.

While the interpreter part of the project is Windows compatible, the server part is not due to POSIX threads and sockets. Windows users are encouraged to use WSL, MinGW, or Cygwin.

## Installing

WIP

## Running

Simply run the `mish` command in the directory with your web server files.

## Scripting

Mish includes a fork of the [Ckript programming language](https://github.com/Roller23/ckript-lang) modified to suit backend development better.

The following functions were added:
- `echo([any, ]) void` – expects at least one argument of any type. Renders the string representation of the passed arguments to the client.
- `render(str filePath) void` – reads the file specified by the passed path and renders it.
- `abort([str]) void` – aborts the script early with an optional message rendered to the client.
- `redirect(str location) void` – redirects the client to the specified location.
- `query(str key) str` – reads and returns the query value specified by the key.
- `body(str key) str` – returns a value from the request body by the specified key.
- `res_header(str key[, str value]) str` – appends a new header to the response. If the `value` argument is omitted, the function returns a value from the response headers by the specified key.
- `req_header(str key) str` – returns a value from the request headers by the specified key.
- `code(int) void` – sets the response status code.
- `decode_uri_component(str component) str` – decodes an encoded URI component and returns it.
- `cors(void)` – enables cross-origin resource sharing for the particular request.
- `date(str format[, int timestamp])` – returns a formatted string of the current date. If the second argument is present, the function uses the specified timestamp instead of the current date.

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