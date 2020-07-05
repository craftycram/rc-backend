// imports
const Serialport = require('serialport');
const Readline = require('@serialport/parser-readline');
const express = require('express');
const cors = require('cors');
const { exec } = require('child_process');
const git = require('simple-git/promise');

const app = express();
app.use(cors);

const http = require('http').Server(app);
const io = require('socket.io')(http);

const config = require('../res/config.json');

// definitions
// /dev/ttyACM0
// /dev/cu.usbmodem14101
const port = new Serialport(config.port, {
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
    client.on('control-left', (message) => {
      console.log('received: %s', message);
      port.write(`l${message}\n`);
    });
    client.on('control-right', (message) => {
      console.log('received: %s', message);
      port.write(`r${message}\n`);
    });

    client.on('system', async (message) => {
      console.log('received: %s', message);
      switch (message) {
        case 'shutdown':
          console.log('shutting down');
          exec('sudo shutdown now', (error) => {
            console.log('Error shutting down!');
            console.log(error);
          });
          break;
        case 'restart':
          console.log('shutting down');
          exec('sudo reboot now', (error) => {
            console.log('Error shutting down!');
            console.log(error);
          });
          break;
        case 'update':
          await git(config.botPathCurrent).pull();
          await process.exit();
          break;
        default:
          break;
      }
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
