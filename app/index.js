'use strict';

document.body.style.whiteSpace = 'pre';
var log = [];

function onreport(report, data) {
  var matrix = new Uint8Array(data.slice(0, 8));
  var s = '';
  for (var i = 0; i < 8; i++) {
    var bits = matrix[i].toString(2);
    s += ('00000000'.substring(0, 8 - bits.length) + bits).split("").reverse().join("");
  }

  var time = new Uint32Array(data.slice(8, 12));
  s += ' ' + (time[0] / 1000);

  log.unshift(s);
  while (log.length > 20) log.pop();
  document.body.textContent = log.join('\n');
}

chrome.hid.getDevices({}, function(devices) {
  var device = devices[0];
  chrome.hid.connect(device.deviceId, function(connection) {
    function receive() {
      chrome.hid.receive(connection.connectionId, function(report, data) {
        receive();
        onreport(report, data);
      });
    }
    receive();
  });
});
