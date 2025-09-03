import 'package:flutter/material.dart';
import 'dart:async';
import '../services/esp32_service.dart';

class WifiScreen extends StatefulWidget {
  const WifiScreen({super.key});

  @override
  State<WifiScreen> createState() => _WifiScreenState();
}

class _WifiScreenState extends State<WifiScreen> {
  List<Map<String, dynamic>> networks = [];
  Map<String, dynamic>? selectedNetwork;
  Map<String, dynamic> capturedCreds = {}; // <-- added
  bool isLoading = false;

  Future<void> getCapturedCredentials() async { // <-- added
    final creds = await ESP32Service.getCapturedCredentials();
    if (creds.isNotEmpty) {
      setState(() {
        capturedCreds = creds;
      });
    }
  }

  void scanWifi() async {
    setState(() {
      isLoading = true;
      selectedNetwork = null;
      networks = [];
    });

    try {
      final result = await ESP32Service.scanWiFi();
      if (mounted) {
        setState(() {
          networks = List<Map<String, dynamic>>.from(result);
        });
      }
    } catch (e) {
      print("Error scanning Wi-Fi: $e");
    } finally {
      if (mounted) {
        setState(() {
          isLoading = false;
        });
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    final bool isNetworkSelected = selectedNetwork != null;

    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        title: const Text('Wi-Fi Tools'),
        backgroundColor: Colors.transparent,
        elevation: 0,
        centerTitle: true,
        titleTextStyle: const TextStyle(
          color: Colors.cyanAccent,
          fontSize: 20,
          fontWeight: FontWeight.bold,
        ),
      ),
      body: SingleChildScrollView(
        child: Padding(
          padding: const EdgeInsets.all(16.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              _buildAttackGrid(isNetworkSelected),
              const SizedBox(height: 16),
              _buildStopCard(),
              const SizedBox(height: 20),
              _buildScannerCard(),
              const SizedBox(height: 20),
              if (capturedCreds.isNotEmpty) _buildCapturedCard(), // <-- added
            ],
          ),
        ),
      ),
    );
  }

  // --- Attack Grid Widgets ---

  Widget _buildAttackGrid(bool isNetworkSelected) {
    String targetSSID = isNetworkSelected
        ? selectedNetwork!['ssid']
        : 'No Target';

    return GridView.count(
      crossAxisCount: 2,
      shrinkWrap: true,
      crossAxisSpacing: 16,
      mainAxisSpacing: 16,
      physics: const NeverScrollableScrollPhysics(),
      children: [
        _buildAttackTile(
          icon: Icons.wifi_tethering,
          label: 'Evil Twin',
          target: targetSSID,
          isEnabled: isNetworkSelected,
          onTap: () async {
            await ESP32Service.triggerEvilTwin(selectedNetwork!);
            await Future.delayed(const Duration(seconds: 2));
            getCapturedCredentials(); // <-- fetch creds after attack
          },
        ),
        _buildAttackTile(
          icon: Icons.signal_wifi_off_outlined,
          label: 'Deauth',
          target: targetSSID,
          isEnabled: isNetworkSelected,
          onTap: () => ESP32Service.triggerDeauth(selectedNetwork!),
        ),
      ],
    );
  }

  Widget _buildAttackTile({
    required IconData icon,
    required String label,
    required String target,
    required bool isEnabled,
    required VoidCallback onTap,
  }) {
    final Color color = isEnabled ? Colors.white : Colors.grey.shade600;
    final Color iconColor = isEnabled
        ? Colors.cyanAccent
        : Colors.grey.shade600;

    return InkWell(
      onTap: isEnabled ? onTap : null,
      borderRadius: BorderRadius.circular(16),
      child: Container(
        decoration: BoxDecoration(
          color: const Color(0xFF1A1A1A),
          borderRadius: BorderRadius.circular(16),
          border: Border.all(
            color: Colors.cyanAccent.withOpacity(isEnabled ? 0.5 : 0.2),
          ),
        ),
        child: Opacity(
          opacity: isEnabled ? 1.0 : 0.5,
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(icon, color: iconColor, size: 36),
              const SizedBox(height: 8),
              Text(
                label,
                style: TextStyle(
                  color: color,
                  fontSize: 16,
                  fontWeight: FontWeight.bold,
                ),
              ),
              const SizedBox(height: 4),
              Text(
                target,
                style: TextStyle(color: color.withOpacity(0.7), fontSize: 12),
                overflow: TextOverflow.ellipsis,
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildStopCard() {
    return Card(
      color: const Color(0xFF1A1A1A),
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(12),
        side: BorderSide(color: Colors.redAccent.withOpacity(0.5)),
      ),
      child: Padding(
        padding: const EdgeInsets.all(8.0),
        child: ElevatedButton.icon(
          icon: const Icon(Icons.stop_circle_outlined, size: 18),
          label: const Text('Stop All Attacks'),
          onPressed: ESP32Service.stopWiFiAttack,
          style: ElevatedButton.styleFrom(
            backgroundColor: Colors.redAccent.shade400,
            foregroundColor: Colors.white,
            minimumSize: const Size(double.infinity, 40),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(10),
            ),
          ),
        ),
      ),
    );
  }

  // --- Scanner Widgets and Helpers ---

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

  Widget _buildScannerCard() {
    return _buildToolCard(
      title: 'Wi-Fi Scanner',
      icon: Icons.radar,
      content: Column(
        children: [
          ElevatedButton.icon(
            icon: isLoading
                ? const SizedBox(
                    width: 18,
                    height: 18,
                    child: CircularProgressIndicator(
                      strokeWidth: 2,
                      color: Colors.black,
                    ),
                  )
                : const Icon(Icons.search, size: 18),
            label: Text(isLoading ? 'Scanning...' : 'Scan for Networks'),
            onPressed: isLoading ? null : scanWifi,
            style: ElevatedButton.styleFrom(
              minimumSize: const Size(double.infinity, 40),
              backgroundColor: Colors.cyanAccent.shade700,
              foregroundColor: Colors.black,
            ),
          ),
          const SizedBox(height: 10),
          _buildNetworkList(),
        ],
      ),
    );
  }

  Widget _buildNetworkList() {
    if (isLoading && networks.isEmpty) {
      return const Padding(
        padding: EdgeInsets.all(16.0),
        child: Center(
          child: CircularProgressIndicator(color: Colors.cyanAccent),
        ),
      );
    }

    if (networks.isEmpty && !isLoading) {
      return const Padding(
        padding: EdgeInsets.all(16.0),
        child: Center(
          child: Text(
            'No networks found.',
            style: TextStyle(color: Colors.grey, fontSize: 16),
          ),
        ),
      );
    }

    return ListView.builder(
      shrinkWrap: true,
      physics: const NeverScrollableScrollPhysics(),
      itemCount: networks.length,
      itemBuilder: (context, index) {
        final network = networks[index];
        final ssid = network['ssid'] as String? ?? 'Unknown SSID';
        final bssid = network['bssid'] as String? ?? 'N/A';
        final rssi = network['rssi'] as int? ?? -100;
        final channel = network['channel'] as int? ?? 0;
        final isSelected = selectedNetwork?['bssid'] == bssid;

        return Card(
          color: Colors.teal.shade900.withOpacity(0.5),
          margin: const EdgeInsets.symmetric(vertical: 4),
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(10),
            side: BorderSide(
              color: isSelected ? Colors.cyanAccent : Colors.transparent,
              width: 1.5,
            ),
          ),
          child: ListTile(
            leading: _getSignalIcon(rssi),
            title: Text(
              ssid,
              style: const TextStyle(
                color: Colors.white,
                fontWeight: FontWeight.bold,
              ),
            ),
            subtitle: Text(
              'BSSID: $bssid\nChannel: $channel',
              style: const TextStyle(color: Colors.white70),
            ),
            trailing: Text(
              '$rssi dBm',
              style: const TextStyle(
                color: Colors.cyanAccent,
                fontWeight: FontWeight.w500,
              ),
            ),
            onTap: () {
              setState(() {
                selectedNetwork = network;
              });
            },
          ),
        );
      },
    );
  }

  Icon _getSignalIcon(int rssi) {
    if (rssi >= -67) {
      return const Icon(Icons.wifi, color: Colors.green);
    } else if (rssi >= -80) {
      return const Icon(Icons.wifi, color: Colors.yellow);
    } else {
      return const Icon(Icons.wifi, color: Colors.red);
    }
  }

  // --- Captured Credentials Card ---
  Widget _buildCapturedCard() { // <-- added
    return _buildToolCard(
      title: "Captured Credentials",
      icon: Icons.lock_open,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text("SSID: ${capturedCreds['ssid'] ?? 'N/A'}",
              style: const TextStyle(color: Colors.white)),
          Text("Username: ${capturedCreds['username'] ?? 'N/A'}",
              style: const TextStyle(color: Colors.white)),
          Text("Password: ${capturedCreds['password'] ?? 'N/A'}",
              style: const TextStyle(color: Colors.white)),
        ],
      ),
    );
  }
}
