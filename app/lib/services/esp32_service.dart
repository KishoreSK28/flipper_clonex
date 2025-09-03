import 'dart:convert';
import 'dart:typed_data';
import 'package:http/http.dart' as http;
import 'package:http_parser/http_parser.dart';
import 'dart:io';

// Data model for storage information
class StorageData {
  final double total;
  final double used;
  final double free;

  StorageData({required this.total, required this.used, required this.free});

  factory StorageData.fromJson(Map<String, dynamic> json) {
    return StorageData(
      total: (json['SD_total'] ?? 0).toDouble(),
      used: (json['SD_used'] ?? 0).toDouble(),
      free: (json['SD_free'] ?? 0).toDouble(),
    );
  }

  String get formatted =>
      "${(used / 1024 / 1024).toStringAsFixed(1)} MB / ${(total / 1024 / 1024).toStringAsFixed(1)} MB";
}

// Data model for RFID scan result
class RfidScanResult {
  final String uid;
  final String block1;

  RfidScanResult({required this.uid, required this.block1});

  factory RfidScanResult.fromJson(Map<String, dynamic> json) {
    return RfidScanResult(
      uid: json['uid'] ?? 'N/A',
      block1: json['block1'] ?? 'N/A',
    );
  }
}

/// Service class to handle all API communications with the ESP32 device.
class ESP32Service {
  static const String baseUrl = 'http://192.18.4.1';

  // ---------- FILE MANAGEMENT ----------

  /// Fetches a list of files from the device.
  static Future<List<Map<String, dynamic>>> getFileList() async {
    final uri = Uri.parse('$baseUrl/files');
    final response = await http.get(uri);

    if (response.statusCode == 200) {
      final List<dynamic> data = json.decode(response.body);
      return data
          .map((item) => {"name": item["name"], "type": item["type"]})
          .toList();
    } else {
      throw Exception('Failed to load files');
    }
  }

  /// Uploads a file from a given file path.
  static Future<void> uploadFileFromPath(String filePath) async {
    final file = File(filePath);
    if (!file.existsSync()) {
      throw Exception("File does not exist: $filePath");
    }
    final bytes = await file.readAsBytes();
    final filename = file.path.split("/").last;
    await _uploadFileBytes(filename, bytes);
  }

  /// (Private) Handles the multipart file upload request.
  static Future<void> _uploadFileBytes(
    String filename,
    Uint8List fileBytes,
  ) async {
    final uri = Uri.parse('$baseUrl/upload');
    var request = http.MultipartRequest('POST', uri)
      ..files.add(
        http.MultipartFile.fromBytes(
          'file',
          fileBytes,
          filename: filename,
          contentType: MediaType('application', 'octet-stream'),
        ),
      );

    final response = await request.send();
    if (response.statusCode != 200) {
      throw Exception('File upload failed');
    }
  }

  /// Downloads a file from the device.
  static Future<Uint8List> downloadFile(String filename) async {
    final response = await http.get(
      Uri.parse('$baseUrl/download?name=$filename'),
    );
    if (response.statusCode == 200) {
      return response.bodyBytes;
    } else {
      throw Exception('Failed to download file');
    }
  }

  /// Deletes a file on the device.
  static Future<void> deleteFile(String filename) async {
    final response = await http.post(
      Uri.parse('$baseUrl/delete'),
      body: {'name': filename},
    );
    if (response.statusCode != 200) {
      throw Exception('Delete failed');
    }
  }

  // ---------- STORAGE ----------

  /// Fetches storage information (total, used, free).
  static Future<StorageData> getStorageInfo() async {
    final response = await http.get(Uri.parse('$baseUrl/storageinfo'));
    if (response.statusCode == 200) {
      final jsonData = jsonDecode(response.body);
      return StorageData.fromJson(jsonData);
    } else {
      throw Exception('Failed to load storage info');
    }
  }

  // ---------- WIFI ----------

  /// Scans for nearby Wi-Fi networks.
  static Future<List<dynamic>> scanWiFi() async {
    final response = await http.get(Uri.parse('$baseUrl/wifi/scan'));
    if (response.statusCode == 200) {
      return jsonDecode(response.body);
    } else {
      throw Exception('Wi-Fi scan failed');
    }
  }

  /// Starts an Evil Twin attack.
  static Future<void> triggerEvilTwin(Map<String, dynamic> network) async {
    final response = await http.get(Uri.parse('$baseUrl/wifi/eviltwin'));
    print("Evil Twin Triggered: ${response.body}");
  }

  /// Starts a Deauthentication attack on a specific network.
  static Future<void> triggerDeauth(Map<String, dynamic> network) async {
    final uri = Uri.parse('$baseUrl/wifi/deauth');
    final headers = {'Content-Type': 'application/json'};
    final body = jsonEncode({
      'bssid': network['bssid'],
      'channel': network['channel'],
    });

    final response = await http.post(uri, headers: headers, body: body);
    if (response.statusCode != 200) {
      throw Exception('Deauth attack failed');
    }
  }

  /// Stops all ongoing Wi-Fi attacks.
  static Future<void> stopWiFiAttack() async {
    final response = await http.get(Uri.parse('$baseUrl/wifi/stop'));
    print("WiFi Attack Stopped: ${response.body}");
  }

  // ---------- BLUETOOTH ----------

  /// Scans for nearby Bluetooth Low Energy devices.
  static Future<List<dynamic>> scanBluetooth() async {
    final response = await http.get(Uri.parse('$baseUrl/bluetooth/scan'));
    if (response.statusCode == 200) {
      return jsonDecode(response.body);
    } else {
      throw Exception('Bluetooth scan failed');
    }
  }

  /// Starts the fake GATT server on the device.
  static Future<void> startGattServer() async {
    final response = await http.get(Uri.parse('$baseUrl/bluetooth/gatt'));
    if (response.statusCode != 200) {
      throw Exception('Failed to start GATT server');
    }
  }

  /// Starts BLE advertisement spoofing.
  static Future<void> startBleSpoof() async {
    final response = await http.get(Uri.parse('$baseUrl/bluetooth/spoof'));
    if (response.statusCode != 200) {
      throw Exception('Failed to start BLE spoofing');
    }
  }

  // ---------- IR (Infrared) ----------
  // Placeholders - Implement C++ backend endpoints for these.

  /*
  /// Sends an IR signal.
  static Future<void> sendIR(Map<String, dynamic> irData) async {
    final uri = Uri.parse('$baseUrl/ir/send');
    final headers = {'Content-Type': 'application/json'};
    final body = jsonEncode(irData); // e.g., {'protocol': 'NEC', 'data': '0x...'}
    final response = await http.post(uri, headers: headers, body: body);
    if (response.statusCode != 200) {
      throw Exception('Failed to send IR signal');
    }
  }

  /// Starts listening for an IR signal to record.
  static Future<Map<String, dynamic>> receiveIR() async {
    final response = await http.get(Uri.parse('$baseUrl/ir/receive'));
    if (response.statusCode == 200) {
      return jsonDecode(response.body);
    } else {
      throw Exception('Failed to receive IR signal');
    }
  }
  */

  // ---------- RFID ----------

  /// Scans for an RFID card and returns its UID and block 1 data.
  static Future<RfidScanResult> scanRFID() async {
    final response = await http.get(Uri.parse('$baseUrl/rfid/scan'));
    if (response.statusCode == 200) {
      return RfidScanResult.fromJson(jsonDecode(response.body));
    } else {
      throw Exception('RFID scan failed');
    }
  }

  /// Writes custom data to a specific block on an RFID card.
  static Future<void> writeRfidBlock(int block, String data) async {
    final uri = Uri.parse('$baseUrl/rfid/write');
    final headers = {'Content-Type': 'application/json'};
    final body = jsonEncode({'block': block, 'data': data});

    final response = await http.post(uri, headers: headers, body: body);
    if (response.statusCode != 200) {
      throw Exception('RFID write failed');
    }
  }

  /// Writes a new UID to a magic (Gen1a) RFID card.
  static Future<void> writeMagicRfid(String uid) async {
    final uri = Uri.parse('$baseUrl/rfid/magic/write');
    final headers = {'Content-Type': 'application/json'};
    final body = jsonEncode({'uid': uid});

    final response = await http.post(uri, headers: headers, body: body);
    if (response.statusCode != 200) {
      throw Exception('Magic RFID write failed');
    }
  }

  /// Copies the last scanned card to a new magic card.
  static Future<void> copyRFID() async {
    final response = await http.get(Uri.parse('$baseUrl/rfid/copy'));
    if (response.statusCode != 200) {
      throw Exception('RFID copy failed');
    }
  }

  static Future<Map<String, dynamic>> getCapturedCredentials() async {
    try {
      final response = await http.get(
        Uri.parse('$baseUrl/wifi/eviltwin/credentials'),
      );
      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        // Check if the response is not an empty object
        if (data is Map && data.isNotEmpty) {
          return Map<String, dynamic>.from(data);
        }
      }
    } catch (e) {
      print("Credential check failed: $e");
    }
    return {}; // Return an empty map if no credentials
  }

  static Future<List<String>> getAvailableIRSignals() async {
    final response = await http.get(Uri.parse('$baseUrl/ir/list'));
    if (response.statusCode == 200) {
      final data = jsonDecode(response.body);
      return List<String>.from(
        data['signals'],
      ); // { "signals": ["tv_power", "ac_on"] }
    } else {
      throw Exception('Failed to load IR signals');
    }
  }

  // 🟢 Capture a new IR signal
  static Future<Map<String, dynamic>> getCapturedIR() async {
    final response = await http.get(Uri.parse('$baseUrl/ir/capture'));
    if (response.statusCode == 200) {
      final data = jsonDecode(response.body);
      return Map<String, dynamic>.from(data); // fix type mismatch
    } else {
      throw Exception('Failed to capture IR signal');
    }
  }

  // 🟢 Send IR signal by name
  static Future<void> saveIRSignal(String name, String data, int bits) async {
    // Define the target endpoint on your ESP32
    final uri = Uri.parse('$baseUrl/ir/save');

    // Set the HTTP headers to indicate that you're sending JSON data
    final headers = {'Content-Type': 'application/json'};

    // Create the JSON body with the signal information
    final body = jsonEncode({'name': name, 'data': data, 'bits': bits});

    // Send the POST request
    final response = await http.post(uri, headers: headers, body: body);

    // Check if the request was successful
    if (response.statusCode != 200) {
      // If not, throw an error to notify the UI
      throw Exception(
        'Failed to save IR signal. Status code: ${response.statusCode}',
      );
    }
  }

  static Future<void> sendIRSignal(String name) async {
    final uri = Uri.parse('$baseUrl/ir/send');
    final headers = {'Content-Type': 'application/json'};
    final body = jsonEncode({'name': name});

    // Added a timeout for better network robustness.
    final response = await http
        .post(uri, headers: headers, body: body)
        .timeout(const Duration(seconds: 5));

    // Updated to throw the specific, more detailed exception on failure.
    if (response.statusCode != 200) {
      throw(
        'Failed to send IR signal "$name"',
        statusCode: response.statusCode,
        uri: uri,
      );
    }
  }
  static Future<void> sendRawIR(String data, int bits) async {
    final uri = Uri.parse('$baseUrl/ir/sendraw');
    final headers = {'Content-Type': 'application/json'};
    final body = jsonEncode({'data': data, 'bits': bits});

    // Added a timeout for better network robustness.
    final response = await http
        .post(uri, headers: headers, body: body)
        .timeout(const Duration(seconds: 5));

    // Updated to throw the specific, more detailed exception on failure.
    if (response.statusCode != 200) {
      throw(
        'Failed to send raw IR signal',
        statusCode: response.statusCode,
        uri: uri,
      );
    }
  }
}
