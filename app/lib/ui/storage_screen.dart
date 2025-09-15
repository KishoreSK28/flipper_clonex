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
  bool isLoading = false; // ✅ Added state variable

  @override
  void initState() {
    super.initState();
    loadFiles();
  }

  Future<void> loadFiles() async {
    setState(() {
      isLoading = true;
      files = [];
    });

    try {
      final fetchedFiles = await ESP32Service.getFileList();
      await Future.delayed(const Duration(seconds: 2)); // Simulate delay
      setState(() {
        files = fetchedFiles;
      });
    } catch (e) {
      print("Error fetching files: $e");
      setState(() {
        files = [];
      });
    } finally {
      setState(() {
        isLoading = false;
      });
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
      body: isLoading
          ? const Center(
              child: CircularProgressIndicator(color: Colors.cyanAccent),
            )
          : files.isEmpty
          ? Center(
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: GestureDetector(
                  onTap: loadFiles,
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: const [
                      Icon(
                        Icons.info_outline,
                        color: Colors.cyanAccent,
                        size: 48,
                      ),
                      SizedBox(height: 12),
                      Text(
                        "No files found.\nTap to retry.",
                        textAlign: TextAlign.center,
                        style: TextStyle(color: Colors.white70, fontSize: 16),
                      ),
                    ],
                  ),
                ),
              ),
            )
          : Padding(padding: const EdgeInsets.all(12.0), child: gridView),
      floatingActionButton: FloatingActionButton(
        onPressed: () async {
          try {
            final result = await FilePicker.platform.pickFiles();
            if (result != null && result.files.isNotEmpty) {
              final filePath = result.files.single.path!;
              await ESP32Service.uploadFileFromPath(filePath);
              await loadFiles(); // Refresh grid after upload
            }
          } catch (e) {
            print("Error uploading file: $e");

            // Show error message in the app
            ScaffoldMessenger.of(context).showSnackBar(
              SnackBar(
                content: Text("Error uploading file: $e"),
                backgroundColor: Colors.redAccent,
              ),
            );
          }
        },
        backgroundColor: Colors.cyanAccent,
        child: const Icon(Icons.upload),
      ),
    );
  }
}
