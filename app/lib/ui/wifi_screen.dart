import 'package:flutter/material.dart';
import '../services/esp32_service.dart';

class WifiScreen extends StatefulWidget {
  const WifiScreen({super.key});

  @override
  State<WifiScreen> createState() => _WifiScreenState();
}

class _WifiScreenState extends State<WifiScreen> {
  List<Map<String, dynamic>> networks = [];
  bool isLoading = false;

  void scanWifi() async {
    setState(() => isLoading = true);
    final result = await ESP32Service.scanWiFi();
    setState(() {
      networks = List<Map<String, dynamic>>.from(result);
      isLoading = false;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        title: const Text('Wi-Fi Tools'),
        backgroundColor: const Color.fromARGB(0, 0, 77, 64),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: [
            Wrap(
              spacing: 10,
              runSpacing: 10,
              children: [
                ElevatedButton(
                  onPressed: scanWifi,
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.cyanAccent.shade700,
                    foregroundColor: Colors.black,
                  ),
                  child: const Text('Scan Wi-Fi Networks'),
                ),
                ElevatedButton(
                  onPressed: () => ESP32Service.triggerEvilTwin(),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.cyanAccent.shade700,
                    foregroundColor: Colors.black,
                  ),
                  child: const Text('Launch Evil Twin'),
                ),
                ElevatedButton(
                  onPressed: () => ESP32Service.triggerDeauth(),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.cyanAccent.shade700,
                    foregroundColor: Colors.black,
                  ),
                  child: const Text('Deauth Attack'),
                ),
                ElevatedButton(
                  onPressed: () => ESP32Service.stopWiFiAttack(),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.cyanAccent.shade700,
                    foregroundColor: Colors.black,
                  ),
                  child: const Text('Stop Attack'),
                ),
              ],
            ),
            const SizedBox(height: 20),
            const Text(
              'Networks:',
              style: TextStyle(
                color: Colors.cyanAccent,
                fontSize: 20,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 10),
            if (isLoading)
              const CircularProgressIndicator(color: Colors.cyanAccent),
            if (!isLoading && networks.isEmpty)
              const Text(
                'No networks found',
                style: TextStyle(color: Colors.grey),
              ),
            if (!isLoading && networks.isNotEmpty)
              Expanded(
                child: ListView.builder(
                  itemCount: networks.length,
                  itemBuilder: (context, index) {
                    final ssid = networks[index]['ssid'] ?? 'Unknown SSID';
                    final bssid = networks[index]['bssid'] ?? 'N/A';

                    return Card(
                      color: Colors.teal.shade800,
                      child: ListTile(
                        title: Text(
                          ssid,
                          style: const TextStyle(
                            color: Colors.cyanAccent,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                        subtitle: Text(
                          'BSSID: $bssid',
                          style: const TextStyle(color: Colors.white70),
                        ),
                        leading: const Icon(
                          Icons.wifi,
                          color: Colors.cyanAccent,
                        ),
                      ),
                    );
                  },
                ),
              ),
          ],
        ),
      ),
    );
  }
}
