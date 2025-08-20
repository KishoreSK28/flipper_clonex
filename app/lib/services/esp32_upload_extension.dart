import 'dart:io';

import 'esp32_service.dart';

extension ESP32UploadExtension on ESP32Service {
  static Future<void> uploadFileFromPath(String filePath) async {
    final file = File(filePath);
    if (!file.existsSync()) {
      throw Exception("File does not exist: $filePath");
    }
    final bytes = await file.readAsBytes();
    final filename = file.path.split("/").last;
    await ESP32Service.uploadFile(filename, bytes);
  }
}
