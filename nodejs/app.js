var io = require('/usr/local/lib/node_modules/socket.io').listen(3000);
var fs = require('fs');

io.sockets.on('connection', function (socket) {
  setInterval(function() {
    fs.readFile('/var/www/bbqpi.csv', 'utf-8', function(err, data) {
      if (err) throw err;
      var lines = data.trim().split('\n');
      var lastline = lines.slice(-1)[0];
      var fields = lastline.split(',');
      var t = parseInt(fields[0],10);
      var x = parseFloat(fields[1],10);
      var y = parseInt(fields[2],10);
      var z = parseInt(fields[3],10);
      socket.emit('sample', {
	t: t,
        x: x,
        y: y,
        z: z
      });
      console.info("emitted: [" + t + "," + x + "," + y + "," + z + "]");
    });
  }, 12000);
});
