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

var hhkbKeys = [
  '2', 'q', 'w', 's', 'a', 'z', 'x', 'c',
  '3', '4', 'r', 'e', 'd', 'f', 'v', 'b',
  '5', '6', 'y', 't', 'g', 'h', 'n', null,
  '1', 'Esc', 'Tab', 'LControl', 'LShift', 'LAlt', 'LMeta', 'Space',
  '7', '8', 'u', 'i', 'k', 'j', 'm', null,
  '\\', '`', 'Backspace', 'Return', 'Fn', 'RShift', 'RAlt', 'RMeta',
  '9', '0', 'o', 'p', ';', 'l', ',', null,
  '-', '+', ']', '[', "'", '/', '.', null,
];

var hhkbIndexes = {};
hhkbKeys.forEach(function(k, i) {
  if (k != null) hhkbIndexes[k] = i;
});

var nkroKeys = [
  'LControl', 'LShift', 'LAlt', 'LMeta', 'RControl', 'RShift', 'RAlt', 'RMeta',
  null, null, null, null,'a', 'b', 'c', 'd',
  'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
  'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
  'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
  '3', '4', '5', '6', '7', '8', '9', '0',
  'Return', 'Esc', 'Backspace', 'Tab', 'Space', '-', '+', '[',
  ']', null, '\\', ';', "'", '`', ',',
  '.', '/', 'CapsLock', 'F1', 'F2', 'F3', 'F4', 'F5',
  'F6', 'F7', 'F8', 'F9', 'F10', 'F11', 'F12', 'PrintScreen',
  'ScrollLock', 'Pause', 'Insert', 'Home', 'PageUp', 'Delete', 'End', 'PageDown',
  'RightArrow', 'LeftArrow', 'DownArrow', 'UpArrow',
];

var nkroIndexes = {};
nkroKeys.forEach(function(k, i) {
  if (k != null) nkroIndexes[k] = i;
});

var keymap = hhkbKeys.map(function(k) {
  return nkroIndexes[k]|0;
});

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
  var pressedKeys = [];
  for (var i = 0; i < 64; i++) {
    var pressed = getBit(matrix, i);
    setBit(keys, keymap[i], pressed);
    if (pressed) {
      pressedKeys.push(hhkbKeys[i]);
    }
  }
  setTimeout(function() {
  chrome.hid.send(connectionId, 0, keyMessage.buffer, function() {});
}, 20);

  var time = new Uint32Array(data.slice(8, 12));
  var s = (time[0] / 1000) + ' ' + pressedKeys.join('+');

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
