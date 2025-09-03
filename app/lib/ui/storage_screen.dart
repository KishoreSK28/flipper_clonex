import 'package:flutter/material.dart';
import '../services/esp32_service.dart';
import 'package:open_filex/open_filex.dart';
import 'package:file_picker/file_picker.dart';

class StorageScreen extends StatefulWidget {
  const StorageScreen({super.key});

  @override
  State<StorageScreen> createState() => _StorageScreenState();
}

class _StorageScreenState extends State<StorageScreen> {
  List<Map<String, dynamic>> files = [];

  @override
  void initState() {
    super.initState();
    loadFiles();
  }

  Future<void> loadFiles() async {
    try {
      final fetchedFiles = await ESP32Service.getFileList();
      setState(() {
        files = fetchedFiles;
      });
    } catch (e) {
      // Handle error
      print("Error fetching files: $e");
    }
  }

  IconData getIcon(String type) {
    switch (type) {
      case 'pdf':
        return Icons.picture_as_pdf;
      case 'image':
        return Icons.image;
      case 'folder':
        return Icons.folder;
      default:
        return Icons.insert_drive_file;
    }
  }

  @override
  Widget build(BuildContext context) {
    var gridView = GridView.builder(
      itemCount: files.length,
      gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
        crossAxisCount: 2,
        mainAxisSpacing: 12,
        crossAxisSpacing: 12,
      ),
      itemBuilder: (context, index) {
        final file = files[index];
        return GestureDetector(
          onTap: () async {
            await OpenFilex.open(file['path']);
          },
          child: Container(
            decoration: BoxDecoration(
              color: Colors.grey[900],
              borderRadius: BorderRadius.circular(12),
            ),
            padding: const EdgeInsets.all(16),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Icon(getIcon(file['type']), color: Colors.cyanAccent, size: 40),
                const SizedBox(height: 10),
                Text(
                  file['name'],
                  style: const TextStyle(color: Colors.white),
                  overflow: TextOverflow.ellipsis,
                ),
              ],
            ),
          ),
        );
      },
    );
    return Scaffold(
      appBar: AppBar(
        title: const Text("DriveX"),
        backgroundColor: Colors.black,
        leading: IconButton(
          icon: const Icon(Icons.arrow_back, color: Colors.cyanAccent),
          onPressed: () {
            Navigator.pop(context);
          },
        ),
        actions: [
          IconButton(
            icon: const Icon(Icons.search, color: Colors.cyanAccent),
            onPressed: () {},
          ),
        ],
      ),
      backgroundColor: Colors.black,
      body: files.isEmpty
          ? const Center(
              child: CircularProgressIndicator(color: Colors.cyanAccent),
            )
          : Padding(padding: const EdgeInsets.all(12.0), child: gridView),
      floatingActionButton: FloatingActionButton(
        onPressed: () async {
          try {
            final result = await FilePicker.platform.pickFiles();
            if (result != null && result.files.isNotEmpty) {
              final filePath = result.files.single.path!;
              // Call the extension static method
              await ESP32Service.uploadFileFromPath(filePath);
              await loadFiles(); // Refresh grid after upload
            }
          } catch (e) {
            print("Error uploading file: $e");
          }
        },
        backgroundColor: Colors.cyanAccent,
        child: const Icon(Icons.upload),
      ),
    );
  }
}
