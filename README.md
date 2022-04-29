# Simple C++ HTTP Server

### Feature list

- [x] Multiple paths
- [x] 404 page
- [x] No extra HTTP libraries
- [x] Multiple clients
- [x] GET, POST, HEAD, DELETE, OPTIONS, PATCH, PUT
- [x] Live memory view
- [x] Request logging
- [x] MD5 Hash generator
- [ ] C++ Templating engine for HTML
- [ ] HTTPs support
- [ ] Rate limit system

### How to compile
```
gcc server.cpp md5.cpp -o server.exe -lws2_32 -lstdc++ -Os
```
### Example route
```cpp
// Example route for /example with GET
if (app.get("/example", path, method)) {
    html += "<h1>C++ Server /example page</h1>";
    html += "<p>Hello, world!</p>";
    content_type = "text/html\r\n";
}
```
### Example logs
```console
[SERVER] 127.0.0.1 GET /memory HTTP/1.1
[SERVER] 127.0.0.1 GET / HTTP/1.1
[SERVER] 127.0.0.1 POST /md5 HTTP/1.
```

## C++ - JS Express
![Screenshot](https://pico.codes/pictures/s6q87ch8tegc8tr.png)

## Contributing

Found a bug or problem? File an issue or submit a pull request with the fix.
