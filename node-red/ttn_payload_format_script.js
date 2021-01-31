function Decoder(bytes, port) {
  // Decode an uplink message from a buffer
  // (array) of bytes to an object of fields.
  //var decoded = {};

  // if (port === 1) decoded.led = bytes[0];

  //return decoded;

  /*return {
    message: String.fromCharCode.apply(null,bytes)
  };*/


//   if (port === 1) {
//     return {
//       "latitude": String.fromCharCode.apply(null,bytes).substring(0,12),
//       "longitude": String.fromCharCode.apply(null,bytes).substring(12,24)
//     };
//   }
//   else if (port === 2) {
//     return {
//       "temperature [°C]": String.fromCharCode.apply(null,bytes).substring(0,4),
//       "humidity [% rH]": String.fromCharCode.apply(null,bytes).substring(4,8)
//     };
//   }

/*  if (port === 1) {
    return {
      "temperature [°C]": String.fromCharCode.apply(null,bytes).substring(0,4),
      "humidity [% rH]": String.fromCharCode.apply(null,bytes).substring(4,8),
      "latitude": String.fromCharCode.apply(null,bytes).substring(8,20),
      "longitude": String.fromCharCode.apply(null,bytes).substring(20,32)
    };
  }*/

  var newLoRaMsg = {};

  if (port === 1) {
    newLoRaMsg.climate = {
      "temperature [°C]": String.fromCharCode.apply(null,bytes).substring(0,5),
      "humidity [% rH]": String.fromCharCode.apply(null,bytes).substring(5,9)
    };

    newLoRaMsg.gps_avg = {
      "latitude [°]": String.fromCharCode.apply(null,bytes).substring(9,18),
      "longitude [°]": String.fromCharCode.apply(null,bytes).substring(18,27)
    };

    var uptime_s = parseInt(String.fromCharCode.apply(null,bytes).substring(27,36), 10); // 10: base is decimal
    //var uptime_s = 184703; // for testing

    var days    = Math.floor(uptime_s / 3600 / 24);
    var hours   = Math.floor((uptime_s - (days * 3600 * 24)) / 3600);
    var minutes = Math.floor((uptime_s - (days * 3600 * 24) - (hours * 3600)) / 60);
    var seconds = uptime_s - (days * 3600 * 24) - (hours * 3600) - (minutes * 60);

    if (days    < 10) {days    = "0"+days;}
    if (hours   < 10) {hours   = "0"+hours;}
    if (minutes < 10) {minutes = "0"+minutes;}
    if (seconds < 10) {seconds = "0"+seconds;}

    var uptime_str = days+'d '+hours+':'+minutes+':'+seconds;

    newLoRaMsg.uptime = {
      "uptime": uptime_str,
      "uptime [s]": uptime_s
    };
  }

  return newLoRaMsg;

  /*return {
    "temperature": String.fromCharCode.apply(null,bytes).substring(0,4),
    "humidity": String.fromCharCode.apply(null,bytes).substring(4,8)
  };*/

  /*// Based on https://stackoverflow.com/a/37471538 by Ilya Bursov
  function bytesToFloat(bytes) {
    // JavaScript bitwise operators yield a 32 bits integer, not a float.
    // Assume LSB (least significant byte first).
    var bits = bytes[3]<<24 | bytes[2]<<16 | bytes[1]<<8 | bytes[0];
    var sign = (bits>>>31 === 0) ? 1.0 : -1.0;
    var e = bits>>>23 & 0xff;
    var m = (e === 0) ? (bits & 0x7fffff)<<1 : (bits & 0x7fffff) | 0x800000;
    var f = sign * m * Math.pow(2, e - 150);
    return f;
  }

  // Test with 0082d241 for 26.3134765625
  return {
    // Take bytes 0 to 4 (not including), and convert to float:
    temp: bytesToFloat(bytes.slice(0, 4))
  };
  */
}
