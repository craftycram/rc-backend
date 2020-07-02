// imports
const Serialport = require('serialport');
const Readline = require('@serialport/parser-readline');

// definitions
const port = new Serialport('/dev/cu.usbmodem14101', {
  baudRate: 9600,
});
const parser = new Readline({
  delimiter: '\r\n',
});

let speed = 70;
let dir = true;

function run() {
  // eslint-disable-next-line consistent-return
  port.write(`${speed}\n`, (err) => {
    if (err) {
      return console.log('Error on write: ', err.message);
    }
  });
  // console.log(speed)
  if (dir) {
    speed += 5;
  } else {
    speed -= 5;
  }
  // dir ? speed += 10 : speed -= 10;

  if (speed >= 100) {
    dir = false;
  } else if (speed <= 50) {
    dir = true;
  }
}

// Init connection
port.pipe(parser);

// Event handler
port.on('open', () => {
  // eslint-disable-next-line no-console
  console.log('Verbindung hergestellt.');
  setInterval(run, 1000);
});

parser.on('data', (data) => {
  // eslint-disable-next-line no-console
  console.log(`Arduino: ${data}`);
});
