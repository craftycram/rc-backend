// imports
const Serialport = require('serialport');
const Readline = require('@serialport/parser-readline');
const { exec } = require('child_process');
const git = require('simple-git/promise');

// DONTCHANGEME - AWS-Serververbindung via SSH Tunnel
const io = require('socket.io-client')('http://localhost:5000');
require('dotenv').config({ path: '/home/pi/ROBO_CONFIG.cfg' });

const config = require('../res/config.json');

// definitions
// /dev/ttyACM0
// /dev/cu.usbmodem14101
const port = new Serialport(config.port, {
  baudRate: 9600,
});
console.log('Port init');

const parser = new Readline({
  delimiter: '\r\n',
});
// Init connection
port.pipe(parser);

let lastData = '';
let isConnected = false;

// eigener "Raumname" ist aus Robotername und "_control zusammengesetzt"
const ownRoom = 'marcbot_control';

// Event handler
port.on('open', () => {
  // eslint-disable-next-line no-console
  console.log('Verbindung hergestellt.');

  io.on('connect', () => {
    console.log('Connected to Master');
    isConnected = true;

    // Verbunden, registiere für jeweiligen Bot-"Raum"
    io.emit('register_bot', {
      room: ownRoom,
      port: process.env.CAMPORT,
    });
    console.log('Registered bot room');

    // Eingehende Serialevents an zentralen SIO Server senden (für Debugging etc.)
    parser.on('data', (line) => {
      console.log(`Arduino: ${line}`);
      if (line !== lastData) {
        io.broadcast.to(ownRoom).emit('serialresponse', line);
        lastData = line;
      }
    });

    // Eingehende SIO Events an serialport via Arduino weiterleiten
    io.on('serialevent', (data) => {
      port.write(data.toString());
      console.log(`Wrote ${data}`);
    });

    io.on('control', (message) => {
      console.log('received: %s', message);
      port.write(`${message}\n`);
    });

    io.on('system', async (message) => {
      console.log('received: %s', message);
      switch (message) {
        case 'shutdown':
          console.log('shutting down');
          exec('sudo shutdown now', (error) => {
            console.log('Error shutting down!');
            console.log(error);
          });
          break;
        case 'reboot':
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

    // Verbindungsabbruch handhaben
    io.on('disconnect', () => {
      isConnected = false;
      console.log('Disconnected');
    });
  });
});
