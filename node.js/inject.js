var buffer=require("buffer");
var dgram = require("dgram");

var lengthNumberFieldInBytes 	= 4;
var HostFieldLength 			= 20;

// Payload struct
// Ersten 4 byte Nummer des Hosts zu dem geschickt werden soll.
// Danach, fixed width, immer 20 bytes mit einem Hostname/IP Adresse

function createPayload() {
	var payload = new Buffer(256);
	var i;
	
	for (i=0; i< 256; ++i) {
		payload[i] = '\x00';
	}
	
	payload.writeUInt32LE(3,0);	// number of hosts
	payload.writeUInt32LE(1,4);	// current hosts

	var off = lengthNumberFieldInBytes;
	payload.write('162.25.52.169', off);
	off += HostFieldLength;
	payload.write('162.25.52.170', off);
	off += HostFieldLength;
	payload.write('162.25.52.171', off);
	
	return payload;
}

function getHostInPayload(NumberOfHost, payload) {
	var start = lengthNumberFieldInBytes + HostFieldLength * NumberOfHost;
	return payload.slice( start, start + HostFieldLength );
}

var buf = createPayload();

//var host = buf.toString('utf8', 4+20*1, 4+20*1+20);
var host = getHostInPayload(1, buf);
console.log( "[" + host + "]" );

var server = dgram.createSocket("udp4");
server.send( buf, 0, 256, 41234, host, function(err, bytes) {
	console.log("buffer sent");
	server.close();
});

