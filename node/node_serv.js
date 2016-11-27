// Magiks Genetation
var port = 9777;
var serv_ip = '127.0.0.1';
var conn_count = 0;

// Load the HTTP module to create a server
var http = require('http');
var net = require('net');

console.log("Welcome to the LTC2983 themocouple management System.");
// Configure server to respong with hello-world for all requests
var server = http.createServer(function(request, response) {
    response.writeHead(200, {"Content-Type": "text/plain"});
    response.end("Hello World\n" + conn_count);
});

// Create a new socket server to connect to the rPI thermocouple server
var thermo_client = new net.Socket();
thermo_client.connect(port, serv_ip, function() {
    console.log('Connected to RPI Thermocontroller Server');
    conn_count++;
    thermo_client.write('Init');
});

// On any recived client data print the message onto the screen 
thermo_client.on('data', function(data) {
    console.log('Received ' + data);
    thermo_client.destroy();
});

// On client close print that the client has closed
thermo_client.on('close', function(){
    console.log('Connection Closed'); 
});

// Listen on port 8000, IP Defaults 127.0.0.1
server.listen(8000);

// Put a freindly message on the terminal
console.log("Server running at http://127.0.0.1:8000/");
