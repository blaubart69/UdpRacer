var buffer=require("buffer");
var buf = new Buffer(256);

function x(obj) {
	obj.i += 1;
}

var a = {};
a.i = 1;

console.log( a.i );
x(a)
console.log( a.i );
x(a)
console.log( a.i );