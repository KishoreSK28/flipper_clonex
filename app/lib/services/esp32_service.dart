import 'dart:convert';
import 'dart:typed_data';
import 'package:http/http.dart' as http;
import 'package:http_parser/http_parser.dart';
import 'dart:io';

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

class ESP32Service {
  static const String baseUrl = 'http://192.168.4.1';

  // ---------- FILE MANAGEMENT ----------
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

  static Future<List<String>> listFiles() async {
    final response = await http.get(Uri.parse('$baseUrl/list'));
    if (response.statusCode == 200) {
      return List<String>.from(jsonDecode(response.body));
    } else {
      throw Exception('Failed to list files');
    }
  }

  static Future<void> uploadFile(String filename, Uint8List fileBytes) async {
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
    if (response.statusCode == 200) {
      print('File uploaded');
    } else {
      throw Exception('File upload failed');
    }
  }

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

  static Future<void> deleteFile(String filename) async {
    final response = await http.post(
      Uri.parse('$baseUrl/delete'),
      body: {'name': filename},
    );
    if (response.statusCode == 200) {
      print('File deleted');
    } else {
      throw Exception('Delete failed');
    }
  }

  // ---------- STORAGE ----------
  static Future<void> startStorageServer() async {
    final response = await http.get(Uri.parse('$baseUrl/start'));
    print("Start Storage Server: ${response.body}");
  }

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
  static Future<List<dynamic>> scanWiFi() async {
    final response = await http.get(Uri.parse('$baseUrl/wifi/scan'));
    if (response.statusCode == 200) {
      return jsonDecode(response.body);
    } else {
      throw Exception('Wi-Fi scan failed');
    }
  }

  static Future<void> triggerEvilTwin() async {
    final response = await http.get(Uri.parse('$baseUrl/wifi/eviltwin'));
    print("Evil Twin Triggered: ${response.body}");
  }

  static Future<void> triggerDeauth() async {
    final response = await http.get(Uri.parse('$baseUrl/wifi/deauth'));
    print("Deauth Triggered: ${response.body}");
  }

  static Future<void> stopWiFiAttack() async {
    final response = await http.get(Uri.parse('$baseUrl/wifi/stop'));
    print("WiFi Attack Stopped: ${response.body}");
  }

  // ---------- BLUETOOTH ----------
  static Future<void> triggerBluetooth() async {
    final response = await http.get(Uri.parse('$baseUrl/bluetooth'));
    print("Bluetooth Triggered: ${response.body}");
  }

  // ---------- IR ----------
  static Future<void> triggerIR() async {
    final response = await http.get(Uri.parse('$baseUrl/ir'));
    print("IR Triggered: ${response.body}");
  }

  // ---------- RFID ----------
  static Future<void> triggerRFID() async {
    final response = await http.get(Uri.parse('$baseUrl/rfid'));
    print("RFID Triggered: ${response.body}");
  }
   static Future<void> uploadFileFromPath(String filePath) async {
    final file = File(filePath);
    if (!file.existsSync()) {
      throw Exception("File does not exist: $filePath");
    }
    final bytes = await file.readAsBytes();
    final filename = file.path.split("/").last;
    await uploadFile(filename, bytes);
  }
}
