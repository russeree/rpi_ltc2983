console.log("I loaded something :D \n");

// Load the HTTP module to create a server
var http = require("http");

console.log("Init Complete");
// Configure server to respong with hello-world for all requests
var server = http.createServer(function(request, response) {
    response.writeHead(200, {"Content-Type": "text/plain"});
    response.end("Hello World\n");
});

// Listen on port 8000, IP Defaults 127.0.0.1
server.listen(8000);

// Put a freindly message on the terminal
console.log("Server running at http://127.0.0.1:8000/");
