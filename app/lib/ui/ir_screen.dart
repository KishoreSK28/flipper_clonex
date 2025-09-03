import 'dart:async';
import 'package:flutter/material.dart';
import '../services/esp32_service.dart'; // Make sure this service file is up to date

class IRScreen extends StatefulWidget {
  const IRScreen({Key? key}) : super(key: key);

  @override
  State<IRScreen> createState() => _IRScreenState();
}

class _IRScreenState extends State<IRScreen> {
  List<String> savedSignals = [];
  String? capturedData;
  int? capturedBits;

  bool isCapturing = false;
  bool isSending = false;
  final TextEditingController _nameController = TextEditingController();

  @override
  void initState() {
    super.initState();
    _loadSignals();
  }

  @override
  void dispose() {
    _nameController.dispose();
    super.dispose();
  }

  Future<void> _loadSignals() async {
    try {
      final signals = await ESP32Service.getAvailableIRSignals();
      if (mounted) setState(() => savedSignals = signals);
    } catch (e) {
      _showErrorSnackbar('Failed to load signals: ${e.toString()}');
    }
  }

  Future<void> _captureSignal() async {
    setState(() => isCapturing = true);
    try {
      final result = await ESP32Service.getCapturedIR();
      if (mounted) {
        setState(() {
          capturedData = result['data']?.toString();
          capturedBits = result['bits'] as int?;
        });
      }
    } catch (e) {
      _showErrorSnackbar('Failed to capture signal: ${e.toString()}');
    } finally {
      if (mounted) setState(() => isCapturing = false);
    }
  }

  Future<void> _sendSavedSignal(String name) async {
    try {
      await ESP32Service.sendIRSignal(name);
      _showSuccessSnackbar("Signal Sent: $name");
    } catch (e) {
      _showErrorSnackbar('Failed to send signal: ${e.toString()}');
    }
  }

  Future<void> _sendCapturedSignal() async {
    if (capturedData == null || capturedBits == null) return;

    setState(() => isSending = true);
    try {
      // ❗️ CORRECTED THIS LINE
      await ESP32Service.sendRawIR(capturedData!, capturedBits!);
      _showSuccessSnackbar("Raw signal sent successfully!");
    } catch (e) {
      _showErrorSnackbar('Failed to send raw signal: ${e.toString()}');
    } finally {
      if (mounted) setState(() => isSending = false);
    }
  }

  Future<void> _saveSignal() async {
    final name = _nameController.text.trim();
    if (name.isEmpty) {
      _showErrorSnackbar("Please enter a name to save the signal.");
      return;
    }

    if (capturedData != null && capturedBits != null) {
      try {
        await ESP32Service.saveIRSignal(name, capturedData!, capturedBits!);
        _nameController.clear();
        await _loadSignals(); // Refresh the list
        if (mounted) {
          setState(() {
            capturedData = null;
            capturedBits = null;
          });
          _showSuccessSnackbar("Signal saved as '$name'");
        }
      } catch (e) {
        _showErrorSnackbar('Failed to save signal: ${e.toString()}');
      }
    }
  }

  void _showErrorSnackbar(String message) {
    if (!mounted) return;
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(message), backgroundColor: Colors.redAccent),
    );
  }

  void _showSuccessSnackbar(String message) {
    if (!mounted) return;
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(message), backgroundColor: Colors.green.shade600),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        title: const Text('IR Remote Tool'),
        backgroundColor: Colors.transparent,
        elevation: 0,
        centerTitle: true,
        titleTextStyle: const TextStyle(
          color: Colors.cyanAccent,
          fontSize: 20,
          fontWeight: FontWeight.bold,
        ),
        actions: [
          IconButton(
            icon: Icon(Icons.refresh, color: Colors.cyanAccent.withOpacity(0.8)),
            onPressed: _loadSignals,
          ),
        ],
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            _buildCaptureCard(),
            const SizedBox(height: 20),
            if (capturedData != null) _buildCapturedSignalCard(),
            const SizedBox(height: 20),
            _buildSavedSignalsCard(),
          ],
        ),
      ),
    );
  }

  Widget _buildCaptureCard() {
    return Card(
      color: const Color(0xFF1A1A1A),
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(12),
        side: BorderSide(color: Colors.cyanAccent.withOpacity(0.5)),
      ),
      child: Padding(
        padding: const EdgeInsets.all(8.0),
        child: ElevatedButton.icon(
          icon: isCapturing
              ? const SizedBox(
                  width: 18,
                  height: 18,
                  child: CircularProgressIndicator(strokeWidth: 2, color: Colors.black),
                )
              : const Icon(Icons.sensors, size: 18),
          label: Text(isCapturing ? 'Waiting for Signal...' : 'Capture New IR Signal'),
          onPressed: isCapturing ? null : _captureSignal,
          style: ElevatedButton.styleFrom(
            backgroundColor: Colors.cyanAccent.shade700,
            foregroundColor: Colors.black,
            minimumSize: const Size(double.infinity, 48),
            shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
            textStyle: const TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
          ),
        ),
      ),
    );
  }

  Widget _buildCapturedSignalCard() {
    return _buildToolCard(
      title: 'Newly Captured Signal',
      icon: Icons.track_changes,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text("Data: $capturedData", style: const TextStyle(color: Colors.white, fontFamily: 'monospace')),
          Text("Bits: $capturedBits", style: const TextStyle(color: Colors.white, fontFamily: 'monospace')),
          const SizedBox(height: 16),
          TextField(
            controller: _nameController,
            style: const TextStyle(color: Colors.white),
            decoration: InputDecoration(
              labelText: "Enter name to save...",
              labelStyle: const TextStyle(color: Colors.cyanAccent),
              enabledBorder: OutlineInputBorder(
                borderSide: BorderSide(color: Colors.cyan.withOpacity(0.5)),
              ),
              focusedBorder: const OutlineInputBorder(
                borderSide: BorderSide(color: Colors.cyanAccent),
              ),
            ),
          ),
          const SizedBox(height: 12),
          Row(
            children: [
              Expanded(
                child: ElevatedButton.icon(
                  icon: const Icon(Icons.send, size: 16),
                  label: const Text("Send"),
                  onPressed: isSending ? null : _sendCapturedSignal,
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.blue.shade400,
                    foregroundColor: Colors.white,
                    padding: const EdgeInsets.symmetric(vertical: 14),
                  ),
                ),
              ),
              const SizedBox(width: 10),
              Expanded(
                child: ElevatedButton.icon(
                  icon: const Icon(Icons.save, size: 16),
                  label: const Text("Save"),
                  onPressed: isSending ? null : _saveSignal,
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.green.shade600,
                    foregroundColor: Colors.white,
                    padding: const EdgeInsets.symmetric(vertical: 14),
                  ),
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildSavedSignalsCard() {
    return _buildToolCard(
      title: 'Saved Signals',
      icon: Icons.memory,
      content: savedSignals.isEmpty
          ? const Center(
              child: Padding(
                padding: EdgeInsets.all(16.0),
                child: Text("No signals saved yet.", style: TextStyle(color: Colors.white70)),
              ),
            )
          : ListView.builder(
              shrinkWrap: true,
              physics: const NeverScrollableScrollPhysics(),
              itemCount: savedSignals.length,
              itemBuilder: (context, index) {
                final signal = savedSignals[index];
                return Card(
                  color: Colors.teal.shade900.withOpacity(0.5),
                  margin: const EdgeInsets.symmetric(vertical: 4),
                  child: ListTile(
                    title: Text(signal, style: const TextStyle(color: Colors.white, fontWeight: FontWeight.bold)),
                    trailing: IconButton(
                      icon: const Icon(Icons.send, color: Colors.cyanAccent),
                      onPressed: () => _sendSavedSignal(signal),
                    ),
                  ),
                );
              },
            ),
    );
  }

  Widget _buildToolCard({
    required String title,
    required IconData icon,
    required Widget content,
  }) {
    return Card(
      color: const Color(0xFF1A1A1A),
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(12),
        side: const BorderSide(color: Colors.cyanAccent, width: 0.5),
      ),
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Icon(icon, color: Colors.cyanAccent, size: 24),
                const SizedBox(width: 10),
                Text(
                  title,
                  style: const TextStyle(color: Colors.white, fontSize: 18, fontWeight: FontWeight.bold),
                ),
              ],
            ),
            const Divider(color: Colors.cyanAccent, height: 24, thickness: 0.5),
            content,
          ],
        ),
      ),
    );
  }
}