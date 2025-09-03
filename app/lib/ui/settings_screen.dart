import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({super.key});

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> {
  // State variables remain the same
  String deviceName = "FlipperCloneX";
  String firmwareVersion = "v1.0.0";
  String esp32Ip = "192.168.4.1";
  String selectedTool = "WiFi Scanner";

  final List<String> tools = [
    "WiFi Scanner",
    "RFID Reader",
    "IR Controller",
    "Bluetooth Explorer",
  ];

  void copyToClipboard(String text, String message) {
    Clipboard.setData(ClipboardData(text: text));
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message),
        backgroundColor: Colors.teal,
        duration: const Duration(seconds: 2),
      ),
    );
  }
  
  // --- UI Build Methods ---

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        title: const Text("Settings"),
        backgroundColor: Colors.transparent,
        elevation: 0,
        centerTitle: true,
        titleTextStyle: const TextStyle(
          color: Colors.cyanAccent,
          fontSize: 20,
          fontWeight: FontWeight.bold,
        ),
      ),
      body: ListView(
        padding: const EdgeInsets.symmetric(horizontal: 16.0, vertical: 8.0),
        children: [
          _buildDeviceInfoCard(),
          const SizedBox(height: 16),
          _buildToolConfigCard(),
          const SizedBox(height: 16),
          _buildStorageCard(),
          const SizedBox(height: 16),
          _buildAdvancedCard(),
        ],
      ),
    );
  }

  // A generic card widget for consistent section styling
  Widget _buildSectionCard({required List<Widget> children}) {
    return Card(
      color: Colors.grey[900],
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(12),
        side: BorderSide(color: Colors.cyanAccent.withOpacity(0.3)),
      ),
      child: Column(children: children),
    );
  }
  
  Widget _buildDeviceInfoCard() {
    return _buildSectionCard(
      children: [
        _buildListTile(
          "Device Name",
          icon: Icons.badge_outlined,
          trailing: Text(deviceName, style: const TextStyle(color: Colors.white70)),
        ),
        _buildDivider(),
        _buildListTile(
          "Firmware Version",
          icon: Icons.verified_outlined,
          trailing: Text(firmwareVersion, style: const TextStyle(color: Colors.white70)),
        ),
        _buildDivider(),
        _buildListTile(
          "ESP32 IP Address",
          subtitle: esp32Ip,
          icon: Icons.wifi_find_outlined,
          trailing: Icon(Icons.copy, color: Colors.cyanAccent.withOpacity(0.7)),
          onTap: () => copyToClipboard(esp32Ip, 'IP Address copied!'),
        ),
      ],
    );
  }

  Widget _buildToolConfigCard() {
    return _buildSectionCard(
      children: [
        ListTile(
          leading: const Icon(Icons.build_outlined, color: Colors.cyanAccent),
          title: const Text("Default Tool", style: TextStyle(color: Colors.white)),
          trailing: DropdownButton<String>(
            value: selectedTool,
            onChanged: (String? value) {
              setState(() => selectedTool = value!);
            },
            items: tools.map((String tool) {
              return DropdownMenuItem<String>(value: tool, child: Text(tool));
            }).toList(),
            style: const TextStyle(color: Colors.white, fontSize: 16),
            dropdownColor: Colors.grey[900],
            iconEnabledColor: Colors.cyanAccent,
            underline: Container(height: 2, color: Colors.cyanAccent.withOpacity(0.5)),
          ),
        ),
      ],
    );
  }

  Widget _buildStorageCard() {
    return _buildSectionCard(
      children: [
        _buildListTile(
          "Format Storage",
          icon: Icons.delete_forever_outlined,
          isDestructive: true,
          onTap: () {
            _showConfirmationDialog(
              title: 'Format Storage?',
              content: 'This will permanently erase all data on the device. This action cannot be undone.',
              onConfirm: () {
                print("Formatting storage..."); // Add actual format logic here
                Navigator.of(context).pop();
              },
            );
          },
        ),
      ],
    );
  }
  
  Widget _buildAdvancedCard() {
    return Card(
      color: Colors.grey[900],
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(12),
        side: BorderSide(color: Colors.cyanAccent.withOpacity(0.3)),
      ),
      clipBehavior: Clip.antiAlias, // Ensures children follow the rounded corners
      child: ExpansionTile(
        collapsedIconColor: Colors.white70,
        iconColor: Colors.cyanAccent,
        title: const Text("Advanced Settings", style: TextStyle(color: Colors.white, fontWeight: FontWeight.bold)),
        children: [
          _buildListTile(
            "Debug Options",
            icon: Icons.bug_report_outlined,
            onTap: () {},
          ),
          _buildDivider(),
          _buildListTile(
            "ESP32 Console Access",
            icon: Icons.terminal_outlined,
            onTap: () {},
          ),
          _buildDivider(),
          _buildListTile(
            "Reset to Defaults",
            icon: Icons.restore_outlined,
            isDestructive: true,
            onTap: () {
              _showConfirmationDialog(
                title: 'Reset to Defaults?',
                content: 'All settings will be reverted to their factory defaults.',
                onConfirm: () {
                  print("Resetting to defaults..."); // Add actual reset logic
                  Navigator.of(context).pop();
                },
              );
            },
          ),
        ],
      ),
    );
  }

  // --- Helper Widgets & Methods ---

  Widget _buildListTile(
    String title, {
    String? subtitle,
    Widget? trailing,
    IconData? icon,
    VoidCallback? onTap,
    bool isDestructive = false,
  }) {
    final color = isDestructive ? Colors.redAccent.shade100 : Colors.white;
    final iconColor = isDestructive ? Colors.redAccent.shade100 : Colors.cyanAccent;

    return ListTile(
      leading: icon != null ? Icon(icon, color: iconColor) : null,
      title: Text(title, style: TextStyle(color: color, fontWeight: FontWeight.w500)),
      subtitle: subtitle != null
          ? Text(subtitle, style: TextStyle(color: Colors.white.withOpacity(0.7)))
          : null,
      trailing: trailing,
      onTap: onTap,
    );
  }

  Divider _buildDivider() => Divider(
        height: 1,
        color: Colors.white.withOpacity(0.1),
        indent: 16,
        endIndent: 16,
      );

  Future<void> _showConfirmationDialog({
    required String title,
    required String content,
    required VoidCallback onConfirm,
  }) async {
    return showDialog<void>(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          backgroundColor: Colors.grey[900],
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
          title: Text(title, style: const TextStyle(color: Colors.white)),
          content: Text(content, style: const TextStyle(color: Colors.white70)),
          actions: <Widget>[
            TextButton(
              child: const Text('Cancel', style: TextStyle(color: Colors.white70)),
              onPressed: () => Navigator.of(context).pop(),
            ),
            TextButton(
              style: TextButton.styleFrom(backgroundColor: Colors.redAccent.shade400),
              child: const Text('Confirm', style: TextStyle(color: Colors.white)),
              onPressed: onConfirm,
            ),
          ],
        );
      },
    );
  }
}