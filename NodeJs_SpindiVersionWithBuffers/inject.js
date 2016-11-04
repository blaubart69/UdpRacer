var buffer=require("buffer");
var dgram = require("dgram");

var lengthNumberFieldInBytes 	= 8;
var HostFieldLength 			= 20;

var bufferSize = 88;

// Payload struct
// Ersten 4 byte Nummer des Hosts zu dem geschickt werden soll.
// Danach, fixed width, immer 20 bytes mit einem Hostname/IP Adresse

function createPayload() {

	var payload = new Buffer(bufferSize);
	var i;
	
	for (i=0; i< bufferSize; ++i) {
		payload[i] = '\x00';
	}
	
	var numberOfHosts = 2;
	
	payload.writeUInt32LE(numberOfHosts,0);	// number of hosts
	payload.writeUInt32LE(0,4);	// current hosts

	var off = lengthNumberFieldInBytes;
	//payload.write('162.25.52.170', off);	off += HostFieldLength;
	payload.write('10.19.2.92', off);	off += HostFieldLength;
	payload.write('10.19.2.94', off);	off += HostFieldLength;
	//payload.write('127.0.0.1', off);	off += HostFieldLength;
	
	return payload;
}

function getHostInPayload(NumberOfHost, payload) {
	var start = lengthNumberFieldInBytes + HostFieldLength * NumberOfHost;
	return payload.slice( start, start + HostFieldLength );
}

var buf = createPayload();

//var host = buf.toString('utf8', 4+20*1, 4+20*1+20);
var host = getHostInPayload(0, buf);
console.log( "[" + host + "]" );

var server = dgram.createSocket("udp4");
var i = 0;

function SendBuffer(bufferToSend) {
	server.send( bufferToSend, 0, bufferSize, 41234, host, function(err, bytes) {
	
			var hosti = getHostInPayload(0, bufferToSend);
			console.log( "[" + hosti + "]" );
			console.log("buffer sent " + i);
			
			i++;
			if ( i < 30 ) {
				SendBuffer(bufferToSend);
			}
			else
			{
				server.close();
			}
		});
}

SendBuffer(buf);

