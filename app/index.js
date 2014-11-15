'use strict';

document.body.style.whiteSpace = 'pre';
var log = [];
var connectionId;

function onconnect() {
  function receive() {
    chrome.hid.receive(connectionId, function(report, data) {
      receive();
      onreport(report, data);
    });
  }
  receive();

  var array = new Uint8Array(64);
  array[0] = 1;
  chrome.hid.send(connectionId, 0, array.buffer, function() {});
}

var keyMessage = new Uint8Array(64);
keyMessage[0] = 2;
var keys = keyMessage.subarray(1, 17);

var keymap = [
  [39, 28, 34, 30, 12, 37, 35, 14],
  [40, 41, 29, 16, 15, 17, 33, 13],
  [42, 43, 36, 31, 18, 19, 25,  0],
  [38, 49, 51,  4,  1,  2,  3, 52],
  [44, 45, 32, 20, 22, 21, 24,  0],
  [58, 61, 50, 48,  0,  5,  6,  7],
  [46, 47, 26, 27, 59, 23, 62,  0],
  [53, 54, 56, 55, 60, 64, 63,  0],
];

function getBit(vector, offset) {
  var byte = offset / 8|0;
  var bit = offset & 7;
  return vector[byte] & (1 << bit);
}

function setBit(vector, offset, value) {
  var byte = offset / 8|0;
  var bit = offset & 7;
  if (value) {
    vector[byte] |= 1 << bit;
  } else {
    vector[byte] &= ~(1 << bit);
  }
}

function onreport(report, data) {
  var matrix = new Uint8Array(data.slice(0, 8));
  var s = '';
  for (var i = 0; i < 8; i++) {
    var bits = matrix[i].toString(2);
    s += ('00000000'.substring(0, 8 - bits.length) + bits).split('').reverse().join('');
  }
  for (var row = 0; row < 8; row++) {
    for (var col = 0; col < 8; col++) {
      setBit(keys, keymap[row][col], getBit(matrix, row * 8 + col));
    }
  }
  chrome.hid.send(connectionId, 0, keyMessage.buffer, function() {});

  var time = new Uint32Array(data.slice(8, 12));
  s += ' ' + (time[0] / 1000);

  log.unshift(s);
  while (log.length > 20) log.pop();
  document.body.textContent = log.join('\n');
}

chrome.hid.getDevices({}, function(devices) {
  var device = devices[0];
  chrome.hid.connect(device.deviceId, function(connection) {
    connectionId = connection.connectionId;
    onconnect();
  });
});
