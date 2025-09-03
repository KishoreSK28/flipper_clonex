import 'dart:convert';
import 'dart:async';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

// Data model for the devices returned by your API
class BleDevice {
  final String name;
  final String mac;

  BleDevice({required this.name, required this.mac});

  factory BleDevice.fromJson(Map<String, dynamic> json) {
    return BleDevice(
      name: json['name'] ?? 'Unknown',
      mac: json['mac'] ?? 'No MAC Address',
    );
  }
}

class BluetoothScreen extends StatefulWidget {
  const BluetoothScreen({Key? key}) : super(key: key);

  @override
  State<BluetoothScreen> createState() => _BluetoothScreenState();
}

class _BluetoothScreenState extends State<BluetoothScreen> {
  // The IP address of your ESP32 Access Point
  final String _baseUrl = 'http://192.168.4.1';

  bool _isLoading = false;
  bool _isSpoofing = false;
  bool _isGattServerRunning = false;
  List<BleDevice> _scanResults = [];

  // --- API Communication Methods ---

  Future<void> _showErrorDialog(String error) async {
    if (!mounted) return;
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(error, style: const TextStyle(color: Colors.white)),
        backgroundColor: Colors.redAccent,
      ),
    );
  }

  // Corresponds to your blescan() C++ function
  Future<void> startScanning() async {
    setState(() {
      _isLoading = true;
      _scanResults = [];
    });

    try {
      final response = await http
          .get(Uri.parse('$_baseUrl/bluetooth/scan'))
          .timeout(const Duration(seconds: 15));

      if (response.statusCode == 200) {
        final List<dynamic> data = json.decode(response.body);
        setState(() {
          _scanResults = data.map((json) => BleDevice.fromJson(json)).toList();
        });
      } else {
        _showErrorDialog('Failed to scan. Status code: ${response.statusCode}');
      }
    } catch (e) {
      _showErrorDialog('Error scanning for devices: $e');
    } finally {
      if (mounted) {
        setState(() {
          _isLoading = false;
        });
      }
    }
  }

  // Corresponds to your handleblespoof() C++ function
  Future<void> startSpoofing() async {
    setState(() => _isSpoofing = true);
    try {
      final response = await http.get(Uri.parse('$_baseUrl/bluetooth/spoof'));
      if (response.statusCode != 200) {
        _showErrorDialog('Failed to start spoofing.');
        setState(() => _isSpoofing = false);
      }
    } catch (e) {
      _showErrorDialog('Error starting spoofing: $e');
      setState(() => _isSpoofing = false);
    }
  }

  // Corresponds to your gattserver() C++ function
  Future<void> toggleGattServer(bool value) async {
    setState(() => _isGattServerRunning = value);

    // Only send start command, stop is handled by the universal stop endpoint
    if (value) {
      try {
        final response = await http.get(Uri.parse('$_baseUrl/bluetooth/gatt'));
        if (response.statusCode != 200) {
          _showErrorDialog('Failed to start GATT server.');
          setState(() => _isGattServerRunning = false);
        }
      } catch (e) {
        _showErrorDialog('Error starting GATT server: $e');
        setState(() => _isGattServerRunning = false);
      }
    } else {
      // If turning off, use the stop command
      stopAllActions();
    }
  }

  // Corresponds to your stopattacks() C++ function
  Future<void> stopAllActions() async {
    try {
      await http.get(Uri.parse('$_baseUrl/wifi/stop'));
      setState(() {
        _isSpoofing = false;
        _isGattServerRunning = false;
      });
    } catch (e) {
      _showErrorDialog('Error sending stop command: $e');
    }
  }

  // --- UI Build Methods ---
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        backgroundColor: Colors.transparent,
        elevation: 0,
        leading: IconButton(
          icon: const Icon(Icons.arrow_back, color: Colors.cyanAccent),
          onPressed: () => Navigator.pop(context),
        ),
        title: const Text(
          "Bluetooth Tools",
          style: TextStyle(
            color: Colors.cyanAccent,
            fontWeight: FontWeight.bold,
            fontSize: 20,
          ),
        ),
        centerTitle: true,
      ),
      body: SingleChildScrollView(
        child: Padding(
          padding: const EdgeInsets.all(16.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              _buildToolCard(
                title: 'BLE Spoofing',
                icon: Icons.wifi_tethering,
                content: _buildSpoofingControls(),
              ),
              const SizedBox(height: 20),
              _buildToolCard(
                title: 'GATT Server',
                icon: Icons.dns_outlined,
                content: _buildGattServerControls(),
              ),
              const SizedBox(height: 20),
              _buildToolCard(
                title: 'BLE Scanner',
                icon: Icons.bluetooth_searching,
                content: _buildScannerControls(),
              ),
              const SizedBox(height: 10),
              _buildScanResultList(),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildToolCard({
    required String title,
    required IconData icon,
    required Widget content,
  }) {
    return Card(
      color: Colors.grey[900],
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
                  style: const TextStyle(
                    color: Colors.white,
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
            const Divider(color: Colors.cyanAccent, height: 20),
            content,
          ],
        ),
      ),
    );
  }

  Widget _buildSpoofingControls() {
    return ElevatedButton.icon(
      icon: Icon(
        _isSpoofing ? Icons.stop_circle_outlined : Icons.play_circle_outline,
      ),
      label: Text(_isSpoofing ? 'Stop Spoofing' : 'Start Spoofing'),
      style: ElevatedButton.styleFrom(
        foregroundColor: Colors.black,
        backgroundColor: Colors.cyanAccent,
        minimumSize: const Size(double.infinity, 40),
      ),
      onPressed: _isSpoofing ? stopAllActions : startSpoofing,
    );
  }

  Widget _buildGattServerControls() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        const Text(
          'Run Local GATT Server',
          style: TextStyle(color: Colors.white, fontSize: 16),
        ),
        Switch(
          value: _isGattServerRunning,
          onChanged: (value) => toggleGattServer(value),
          activeColor: Colors.cyanAccent,
        ),
      ],
    );
  }

  Widget _buildScannerControls() {
    return ElevatedButton.icon(
      icon: _isLoading
          ? const SizedBox(
              width: 20,
              height: 20,
              child: CircularProgressIndicator(
                strokeWidth: 2,
                color: Colors.black,
              ),
            )
          : const Icon(Icons.search),
      label: Text(_isLoading ? 'Scanning...' : 'Start Scan'),
      style: ElevatedButton.styleFrom(
        foregroundColor: Colors.black,
        backgroundColor: Colors.cyanAccent,
        minimumSize: const Size(double.infinity, 40),
      ),
      onPressed: _isLoading ? null : startScanning,
    );
  }

  Widget _buildScanResultList() {
    return ListView.builder(
      shrinkWrap: true,
      physics: const NeverScrollableScrollPhysics(),
      itemCount: _scanResults.length,
      itemBuilder: (context, index) {
        final device = _scanResults[index];
        return Card(
          margin: const EdgeInsets.symmetric(vertical: 6),
          color: Colors.grey[850],
          child: ListTile(
            title: Text(
              device.name,
              style: const TextStyle(color: Colors.white),
            ),
            subtitle: Text(
              device.mac,
              style: const TextStyle(color: Colors.white70),
            ),
          ),
        );
      },
    );
  }
}
