var dgram = require("dgram");

var server = dgram.createSocket("udp4");
var Port = 41234;
var counter = 0;
var millisecondsToMeasure = 1000;

var lengthNumberFieldInBytes 	= 8;
var HostFieldLength 			= 20;

function getHostInPayload(IndexOfHost, payload) {
	var start = lengthNumberFieldInBytes + HostFieldLength * IndexOfHost;
	return payload.slice( start, start + HostFieldLength );
}

server.on("message", function (msg, rinfo) {

	if ( msg.length < 10 ) {
		console.log("msg.length [" + msg.length + "]" );
		console.log("msg [" + msg.toString('utf8') + "]" );
		return;
	}

	var NumberHosts	= msg.readUInt32LE(0);
	var currIdx		= msg.readUInt32LE(4);
	currIdx += 1;
	
	if ( currIdx >= NumberHosts ) {
		currIdx = 0;
	}
	msg.writeUInt32LE(currIdx,4);
	
	var start 		= lengthNumberFieldInBytes + HostFieldLength * currIdx;
	var nextHost 	= msg.slice( start, start + HostFieldLength );
	
	counter++;
	server.send(msg, 0, msg.length, Port, nextHost);
});

server.on("listening", function () {
  var address = server.address();
  console.log("server listening " +
      address.address + ":" + address.port);
});

setInterval(function() {
                console.log(counter + " per [" + millisecondsToMeasure + "] milliseconds");
                counter = 0;
}, millisecondsToMeasure);

server.bind(Port);
