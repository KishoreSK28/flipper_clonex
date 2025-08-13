import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({super.key});

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> {
  String deviceName = "FlipperCloneX";
  String firmwareVersion = "v1.0.0";
  String esp32Ip = "192.168.4.1";
  bool showAdvanced = false;
  String selectedTool = "WiFi Scanner";

  final List<String> tools = [
    "WiFi Scanner",
    "RFID Reader",
    "IR Controller",
    "Bluetooth Explorer",
  ];

  void copyToClipboard(String text) {
    Clipboard.setData(ClipboardData(text: text));
    ScaffoldMessenger.of(
      context,
    ).showSnackBar(const SnackBar(content: Text("Copied to clipboard")));
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        title: const Text("Settings"),
        backgroundColor: Colors.black,
        foregroundColor: Colors.cyanAccent,
        elevation: 0,
      ),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          buildSectionTitle("Device Info"),
          buildListTile("Device Name", trailing: Text(deviceName)),
          buildListTile("Firmware Version", trailing: Text(firmwareVersion)),

          const SizedBox(height: 10),
          ListTile(
            title: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                const Text(
                  'ESP32 IP Address',
                  style: TextStyle(color: Colors.white),
                ),
                const SizedBox(height: 4),
                const SelectableText(
                  '192.168.4.1',
                  style: TextStyle(color: Colors.white70),
                ),
              ],
            ),
            trailing: IconButton(
              icon: const Icon(Icons.copy, color: Colors.cyanAccent),
              onPressed: () {
                Clipboard.setData(const ClipboardData(text: '192.168.4.1'));
                ScaffoldMessenger.of(context).showSnackBar(
                  const SnackBar(content: Text('IP address copied')),
                );
              },
            ),
          ),
          const SizedBox(height: 10),
          buildSectionTitle("Tool Configuration"),
          ListTile(
            title: const Text(
              "Default Tool",
              style: TextStyle(color: Colors.white),
            ),
            trailing: DropdownButton<String>(
              dropdownColor: Colors.black87,
              value: selectedTool,
              style: const TextStyle(color: Colors.cyanAccent),
              onChanged: (String? value) {
                setState(() {
                  selectedTool = value!;
                });
              },
              items: tools.map((String tool) {
                return DropdownMenuItem<String>(value: tool, child: Text(tool));
              }).toList(),
            ),
          ),

          const SizedBox(height: 10),
          buildSectionTitle("Storage"),
          buildListTile(
            "Format Storage",
            icon: Icons.delete_forever,
            onTap: () {
              // Add format logic here
            },
          ),

          const SizedBox(height: 10),
          buildSectionTitle("Advanced"),
          ExpansionTile(
            collapsedIconColor: Colors.white70,
            iconColor: Colors.cyanAccent,
            collapsedBackgroundColor: Colors.grey[900],
            backgroundColor: Colors.grey[850],
            title: const Text(
              "Advanced Settings",
              style: TextStyle(color: Colors.white),
            ),
            children: [
              buildListTile("Debug Options", onTap: () {}),
              buildListTile(
                "ESP32 Console Access",
                onTap: () {
                  // Navigate or show console access screen
                },
              ),
              buildListTile(
                "Reset to Defaults",
                icon: Icons.restore,
                onTap: () {
                  // Add reset logic
                },
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget buildSectionTitle(String title) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 8),
      child: Text(
        title,
        style: const TextStyle(
          fontSize: 18,
          fontWeight: FontWeight.bold,
          color: Colors.cyanAccent,
        ),
      ),
    );
  }

  Widget buildListTile(
    String title, {
    String? subtitle,
    Widget? trailing,
    IconData? icon,
    VoidCallback? onTap,
  }) {
    return ListTile(
      title: Text(title, style: const TextStyle(color: Colors.white)),
      subtitle: subtitle != null
          ? Text(subtitle, style: const TextStyle(color: Colors.white70))
          : null,
      trailing: trailing,
      leading: icon != null ? Icon(icon, color: Colors.cyanAccent) : null,
      onTap: onTap,
    );
  }
}
