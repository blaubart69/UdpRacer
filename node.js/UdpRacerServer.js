var dgram = require("dgram");

var server = dgram.createSocket("udp4");
var Port = 41234;
var counter = 0;

var lengthNumberFieldInBytes 	= 8;
var HostFieldLength 			= 20;

function getHostInPayload(IndexOfHost, payload) {
	var start = lengthNumberFieldInBytes + HostFieldLength * IndexOfHost;
	return payload.slice( start, start + HostFieldLength );
}

server.on("message", function (msg, rinfo) {

	var NumberHosts	= msg.readUInt32LE(msg,0);
	var currIdx		= msg.readUInt32LE(msg,4);
	currIdx += 1;
	
	if ( currIdx >= NumberHosts ) {
		currIdx = 0;
	}
	
	msg.writeUInt32LE(currIdx,4);
	var nextHost 	= getHostInPayload( currIdx, msg );
  
	counter++;
	server.send(msg, 0, msg.length, Port, nextHost);
});

server.on("listening", function () {
  var address = server.address();
  console.log("server listening " +
      address.address + ":" + address.port);
});

setInterval(function() {
                console.log(counter + " per second");
                counter = 0;
},1000);

server.bind(myPort);
// server listening 0.0.0.0:41234
