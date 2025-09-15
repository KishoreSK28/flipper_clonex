import 'package:flutter/material.dart';
import '../services/esp32_service.dart';

class RFIDScreen extends StatefulWidget {
  const RFIDScreen({super.key});

  @override
  State<RFIDScreen> createState() => _RFIDScreenState();
}

class _RFIDScreenState extends State<RFIDScreen> {
  String status = "Disconnected";
  String lastTag = "None";
  bool isWriting = false; // To show a loading indicator

void _scanTag() async {
  setState(() {
    status = "Scanning...";
  });

  try {
    final result = await ESP32Service.scanRFID();
    setState(() {
      lastTag = "UID: ${result.uid}\nBlock1: ${result.block1}";
      status = "Tag Scanned Successfully ✅";
    });
  } catch (e) {
    setState(() {
      status = "Scan Failed ❌";
      lastTag = "unable to read tag.";
    });
  }
}

  void _writeTag() {
    setState(() {
      status = "Writing to tag...";
      isWriting = true;
    });
    Future.delayed(const Duration(seconds: 2), () {
      if (mounted) {
        setState(() {
          status = "Write Successful ✅";
          isWriting = false;
        });
      }
    });
  }

  void _clearHistory() {
    setState(() {
      lastTag = "None";
      status = "History Cleared";
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      appBar: AppBar(
        title: const Text('RFID Module'),
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
              _buildActionGrid(),
              const SizedBox(height: 16),
              _buildClearCard(),
              const SizedBox(height: 20),
              _buildStatusCard(),
            ],
          ),
        ),
      ),
    );
  }

  // --- Main Action Grid ---
  Widget _buildActionGrid() {
    return GridView.count(
      crossAxisCount: 2,
      shrinkWrap: true,
      crossAxisSpacing: 16,
      mainAxisSpacing: 16,
      physics: const NeverScrollableScrollPhysics(),
      children: [
        _buildActionTile(
          icon: Icons.rss_feed,
          label: 'Scan Tag',
          onTap: _scanTag,
          iconColor: Colors.cyanAccent,
        ),
        _buildActionTile(
          icon: Icons.edit,
          label: 'Write Tag',
          onTap: isWriting ? null : _writeTag,
          iconColor: Colors.greenAccent,
          isLoading: isWriting,
        ),
      ],
    );
  }

  Widget _buildActionTile({
    required IconData icon,
    required String label,
    required VoidCallback? onTap,
    required Color iconColor,
    bool isLoading = false,
  }) {
    final bool isEnabled = onTap != null;
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
              if (isLoading)
                const CircularProgressIndicator(color: Colors.cyanAccent)
              else
                Icon(icon, color: isEnabled ? iconColor : Colors.grey.shade600, size: 40),
              const SizedBox(height: 12),
              Text(
                label,
                style: TextStyle(
                  color: isEnabled ? Colors.white : Colors.grey.shade600,
                  fontSize: 16,
                  fontWeight: FontWeight.bold,
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }

  // --- Clear History Card (Styled like Stop Attacks) ---
  Widget _buildClearCard() {
    return Card(
      color: const Color(0xFF1A1A1A),
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(12),
        side: BorderSide(color: Colors.redAccent.withOpacity(0.5)),
      ),
      child: Padding(
        padding: const EdgeInsets.all(8.0),
        child: ElevatedButton.icon(
          icon: const Icon(Icons.delete_sweep_outlined, size: 18),
          label: const Text('Clear History'),
          onPressed: _clearHistory,
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

  // --- Status Card (Styled like Scanner/Tool Cards) ---
  Widget _buildStatusCard() {
    return _buildToolCard(
      title: 'RFID Status',
      icon: Icons.info_outline,
      content: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          _statusRow("Status:", status, Colors.cyanAccent),
          const SizedBox(height: 8),
          _statusRow("Last Tag:", lastTag, Colors.white),
        ],
      ),
    );
  }

  Widget _statusRow(String label, String value, Color valueColor) {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          label,
          style: const TextStyle(color: Colors.white70, fontWeight: FontWeight.bold),
        ),
        const SizedBox(width: 8),
        Expanded(
          child: Text(
            value,
            style: TextStyle(
              color: valueColor,
              fontWeight: FontWeight.bold,
              fontSize: 16,
              fontFamily: 'monospace', // Gives a techy feel
            ),
          ),
        ),
      ],
    );
  }

  // --- Reusable Tool Card Widget (Copied from your WifiScreen style) ---
  Widget _buildToolCard({
    required String title,
    required IconData icon,
    required Widget content,
  }) {
    return Card(
      color: const Color(0xFF1A1A1A), // Using a slightly darker shade
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
            const Divider(color: Colors.cyanAccent, height: 24, thickness: 0.5),
            content,
          ],
        ),
      ),
    );
  }
}