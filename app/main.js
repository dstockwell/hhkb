'use strict';

var connectionId;

function onconnect(newConnectionId) {
  connectionId = newConnectionId;
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

var nkroKeys = [
  'LControl', 'LShift', 'LAlt', 'LMeta', 'RControl', 'RShift', 'RAlt', 'RMeta',
  null, null, null, null, 'a', 'b', 'c', 'd',
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

var hhkbLayout = (function() {
  var hhkbMatrix = [
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
  hhkbMatrix.forEach(function(k, i) {
    if (k != null) hhkbIndexes[k] = i;
  });

  return new Layout(hhkbMatrix, hhkbIndexes);
})();

function Layout(matrix, indexes) {
  this.matrix = matrix || Array(64);
  this.indexes = indexes || {};
  matrix && indexes || this.reset();
}

Layout.prototype = {
  reset: function() {
    this.matrix = hhkbLayout.matrix.concat();
    this.indexes = JSON.parse(JSON.stringify(hhkbLayout.indexes));
    this._keymap = null;
    return this;
  },
  clear: function() {
    this.matrix = this.matrix.map(function() { return null; });
    this.indexes = {};
    this._keymap = null;
    return this;
  },
  remove: function(key) {
    var index = this.indexes[key];
    if (index != null) {
      delete this.indexes[key];
      this.matrix[index] = null;
      this._keymap = null;
    }
    return this;
  },
  map: function(from, to) {
    var index = hhkbLayout.indexes[from];

    // Remove duplicate mapping.
    this.remove(to);

    // Remove existing mapping.
    this.remove(this.matrix[index]);

    // Establish new mapping.
    this.matrix[index] = to;
    this.indexes[to] = index;
    this._keymap = null;
    return this;
  },
  get keymap() {
    if (!this._keymap) {
      this._keymap = this.matrix.map(function(k) {
        var index = nkroIndexes[k];
        return index != null ? index : -1;
      });
    }
    return this._keymap;
  },
};

var layout = new Layout();

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
    var mapped = layout.keymap[i];
    if (mapped >= 0) {
      setBit(keys, mapped, pressed);
    }
    if (pressed) {
      pressedKeys.push(hhkbLayout.matrix[i]);
    }
  }
  chrome.hid.send(connectionId, 0, keyMessage.buffer, function() {});

  var time = new Uint32Array(data.slice(8, 12));
}

(function() {
  var device, connection;
  setInterval(function() {
    chrome.hid.getDevices({}, function(devices) {
      if (!devices.length) {
        // Device not available.
        device = null;
        connection = null;
        return;
      }
      if (device && devices[0].deviceId != device.deviceId) {
        // Device id changed, need a new connection.
        connection = null;
        device = null;
      }
      if (connection) {
        // Still connected.
        return;
      }
      device = devices[0];
      connection = 'pending';
      chrome.hid.connect(device.deviceId, function(newConnection) {
        connection = newConnection;
        if (connection) {
          onconnect(connection.connectionId);
        }
      });
    });
  }, 1000);
})();
