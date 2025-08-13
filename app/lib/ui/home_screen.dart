import 'package:flutter/material.dart';
import '../widgets/feature_card.dart';
import '../widgets/bottom_nav.dart';
import '../services/esp32_service.dart';

class HomeScreen extends StatelessWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      bottomNavigationBar: const BottomNavBar(),
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 18.0, vertical: 12),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              // Greeting and Settings Icon Row
              Row(
                children: [
                  const Text(
                    "Flipper Clone-x",
                    style: TextStyle(
                      fontSize: 22,
                      fontWeight: FontWeight.bold,
                      color: Colors.white,
                      fontFamily: 'orbitran',
                    ),
                  ),
                  Flexible(
                    child: Align(
                      alignment: Alignment.centerRight,
                      child: Image.asset(
                        'assets/img/im.png',
                        width: 100, // Reduce width to prevent overflow
                        height: 100,
                        fit: BoxFit.contain,
                      ),
                    ),
                  ),
                ],
              ),

              const SizedBox(height: 6),
              const Text(
                "Your portable storage server",
                style: TextStyle(fontSize: 14, color: Colors.white70),
              ),
              const SizedBox(height: 20),

              // Full-width Portable Server Card
              Container(
                width: double.infinity,
                padding: const EdgeInsets.all(16),
                decoration: BoxDecoration(
                  color: const Color(0xFF1A1A1A),
                  borderRadius: BorderRadius.circular(16),
                ),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.center,
                  children: [
                    const Text(
                      "PORTABLE STORAGE SERVER",
                      style: TextStyle(
                        color: Colors.cyanAccent,
                        fontWeight: FontWeight.bold,
                        fontSize: 16,
                      ),
                    ),
                    const SizedBox(height: 6),
                    const Text(
                      "192.168.4.1",
                      style: TextStyle(color: Colors.white70, fontSize: 14),
                    ),
                    const SizedBox(height: 12),

                    // Storage Info FutureBuilder
                    FutureBuilder<StorageData>(
                      future: ESP32Service.getStorageInfo(),
                      builder: (context, snapshot) {
                        if (snapshot.connectionState ==
                            ConnectionState.waiting) {
                          return const CircularProgressIndicator(
                            color: Colors.cyanAccent,
                          );
                        } else if (snapshot.hasError) {
                          return const Text(
                            "Error loading storage",
                            style: TextStyle(color: Colors.redAccent),
                          );
                        } else if (snapshot.hasData) {
                          return Row(
                            mainAxisAlignment: MainAxisAlignment.center,
                            children: [
                              const Icon(
                                Icons.storage,
                                color: Colors.cyanAccent,
                                size: 18,
                              ),
                              const SizedBox(width: 6),
                              Text(
                                "Storage Used: ${snapshot.data!.formatted}",
                                style: const TextStyle(
                                  color: Colors.white70,
                                  fontSize: 14,
                                ),
                              ),
                            ],
                          );
                        } else {
                          return const SizedBox();
                        }
                      },
                    ),
                    const SizedBox(height: 12),

                    SizedBox(
                      width: double.infinity,
                      child: OutlinedButton(
                        onPressed: () =>
                            Navigator.pushNamed(context, '/storage'),
                        style: OutlinedButton.styleFrom(
                          foregroundColor: Colors.cyanAccent,
                          side: const BorderSide(color: Colors.cyanAccent),
                        ),
                        child: const Padding(
                          padding: EdgeInsets.symmetric(vertical: 12.0),
                          child: Text("START", style: TextStyle(fontSize: 16)),
                        ),
                      ),
                    ),
                  ],
                ),
              ),

              const SizedBox(height: 22),

              // Feature Cards Grid
              Expanded(
                child: GridView.count(
                  crossAxisCount: 2,
                  crossAxisSpacing: 16,
                  mainAxisSpacing: 16,
                  children: [
                    FeatureCard(
                      icon: Icons.wifi,
                      label: "WI-FI",
                      onTap: () => Navigator.pushNamed(context, '/wifi'),
                    ),
                    FeatureCard(
                      icon: Icons.bluetooth,
                      label: "BLUETOOTH",
                      onTap: () => Navigator.pushNamed(context, '/bluetooth'),
                    ),
                    FeatureCard(
                      icon: Icons.nfc,
                      label: "RFID\nEMULATOR",
                      onTap: () => ESP32Service.triggerRFID(),
                    ),
                    FeatureCard(
                      icon: Icons.lightbulb_outline,
                      label: "IR\nREMOTE",
                      onTap: () => ESP32Service.triggerIR(),
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
