// imports
const Serialport = require('serialport');
const Readline = require('@serialport/parser-readline');
const express = require('express');
const cors = require('cors');

const app = express();
app.use(cors);

const http = require('http').Server(app);
const io = require('socket.io')(http);

// definitions
const port = new Serialport('/dev/cu.usbmodem14101', {
  baudRate: 9600,
});
const parser = new Readline({
  delimiter: '\r\n',
});

// Init connection
port.pipe(parser);

// Event handler
port.on('open', () => {
  // eslint-disable-next-line no-console
  console.log('Verbindung hergestellt.');

  io.on('connection', (client) => {
    client.on('control', (message) => {
      console.log('received: %s', message);
      port.write(`${message}\n`);
    });

    parser.on('data', (data) => {
      // eslint-disable-next-line no-console
      console.log(`Arduino: ${data}`);
      client.emit('arduino', data);
    });
  });

  // start our server
  const server = http.listen(3001, () => {
    console.log(`Server started on port ${server.address().port} :)`);
  });
});
