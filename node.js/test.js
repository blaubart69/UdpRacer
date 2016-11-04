var buffer=require("buffer");

var buf = new Buffer(256);


for (i=0; i< 256; ++i) {
	buf[i] = '\x00';
}
len = buf.write('\u0010');

var off = 1;
buf.write('162.25.52.170', off);
off += 20;
buf.write('162.25.52.171', off);
off += 20;
buf.write('162.25.52.172', off);


var x = buf.readUInt8(0)
console.log("x [" + x + "]");
console.log( "[" + buf.toString('utf8', 21, 21+20) + "]" );

