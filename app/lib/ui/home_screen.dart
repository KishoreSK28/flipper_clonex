import 'package:flutter/material.dart';
import '../widgets/feature_card.dart';
import '../widgets/bottom_nav.dart';
import '../services/esp32_service.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen>
    with SingleTickerProviderStateMixin {
  late Future<StorageData> _storageInfoFuture;
  late AnimationController _animationController;
  late Animation<double> _fadeAnimation;

  @override
  void initState() {
    super.initState();
    _storageInfoFuture = ESP32Service.getStorageInfo();
    _animationController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 600),
    );
    _fadeAnimation = CurvedAnimation(
      parent: _animationController,
      curve: Curves.easeIn,
    );
    _animationController.forward();
  }

  @override
  void dispose() {
    _animationController.dispose();
    super.dispose();
  }

  void _refreshStorageInfo() {
    setState(() {
      _storageInfoFuture = ESP32Service.getStorageInfo();
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      bottomNavigationBar: const BottomNavBar(),
      body: SafeArea(
        child: FadeTransition(
          opacity: _fadeAnimation,
          // 1. REMOVED SingleChildScrollView to keep the top part fixed
          child: Padding(
            padding: const EdgeInsets.symmetric(horizontal: 18.0, vertical: 12),
            child: Column(
              // The layout is a Column
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // These widgets will be fixed at the top
                _buildHeader(),
                const SizedBox(height: 6),
                const Text(
                  "Your portable hacking multi-tool",
                  style: TextStyle(fontSize: 14, color: Colors.white70),
                ),
                const SizedBox(height: 20),
                _buildStorageCard(),
                const SizedBox(height: 22),

                // 2. ADDED Expanded here to make the grid fill the rest of the screen
                _buildFeatureGrid(),
              ],
            ),
          ),
        ),
      ),
    );
  }
  // In HomeScreen.dart

  // In HomeScreen.dart

  // In HomeScreen.dart

  // In HomeScreen.dart

  Widget _buildHeader() {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.center, // Align items vertically
      children: [
        const Expanded(
          child: Text(
            "Flipper Clone-X",
            style: TextStyle(
              fontSize: 22,
              fontWeight: FontWeight.bold,
              color: Colors.white,
              fontFamily: 'orbitran',
            ),
            overflow: TextOverflow.ellipsis,
            maxLines: 1,
          ),
        ),
        IconButton(
          padding: EdgeInsets.zero,
          constraints: const BoxConstraints(),
          icon: Image.asset(
            'assets/img/im.png',
            // REDUCED the size here to give the title more space
            width: 80,
            height: 80,
            fit: BoxFit.contain,
          ),
          onPressed: () => Navigator.pushNamed(context, '/settings'),
        ),
      ],
    );
  }

  // In HomeScreen.dart

  // In HomeScreen.dart

  Widget _buildStorageCard() {
    return Container(
      width: double.infinity,
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: const Color(0xFF1A1A1A),
        borderRadius: BorderRadius.circular(16),
        border: Border.all(color: Colors.cyanAccent.withOpacity(0.2)),
      ),
      child: Column(
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              const Text(
                "PORTABLE STORAGE SERVER",
                style: TextStyle(
                  color: Colors.cyanAccent,
                  fontWeight: FontWeight.bold,
                  fontSize: 16,
                ),
              ),
              IconButton(
                icon: const Icon(
                  Icons.refresh,
                  color: Colors.cyanAccent,
                  size: 20,
                ),
                onPressed: _refreshStorageInfo,
              ),
            ],
          ),
          const SizedBox(height: 12),
          FutureBuilder<StorageData>(
            future: _storageInfoFuture,
            builder: (context, snapshot) {
              if (snapshot.connectionState == ConnectionState.waiting) {
                return const Center(
                  child: CircularProgressIndicator(color: Colors.cyanAccent),
                );
              } else if (snapshot.hasError) {
                return const Text(
                  "Error loading storage",
                  style: TextStyle(color: Colors.redAccent),
                );
              } else if (snapshot.hasData) {
                final storage = snapshot.data!;
                final usedPercent = (storage.total > 0)
                    ? storage.used / storage.total
                    : 0.0;
                return Column(
                  children: [
                    Text(
                      storage.formatted,
                      style: const TextStyle(color: Colors.white, fontSize: 16),
                    ),
                    const SizedBox(height: 8),
                    ClipRRect(
                      borderRadius: BorderRadius.circular(10),
                      child: LinearProgressIndicator(
                        value: usedPercent,
                        backgroundColor: Colors.grey.shade700,
                        valueColor: const AlwaysStoppedAnimation<Color>(
                          Colors.cyanAccent,
                        ),
                        minHeight: 10,
                      ),
                    ),
                  ],
                );
              } else {
                return const Text(
                  "No storage data",
                  style: TextStyle(color: Colors.white70),
                );
              }
            },
          ),
          const SizedBox(height: 16),
          // --- START OF NEW CUSTOM BUTTON CODE ---
          InkWell(
            onTap: () => Navigator.pushNamed(context, '/storage'),
            borderRadius: BorderRadius.circular(12),
            child: Container(
              padding: const EdgeInsets.symmetric(vertical: 14.0),
              decoration: BoxDecoration(
                // Gradient and Shadow are now in the same decoration
                gradient: const LinearGradient(
                  colors: [Colors.cyan, Colors.tealAccent],
                  begin: Alignment.topLeft,
                  end: Alignment.bottomRight,
                ),
                borderRadius: BorderRadius.circular(12),
                boxShadow: [
                  BoxShadow(
                    color: Colors.cyanAccent.withOpacity(0.6),
                    blurRadius: 18.0,
                  ),
                ],
              ),
              child: const Center(
                child: Text(
                  "START",
                  style: TextStyle(
                    fontSize: 16,
                    color: Colors.black,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
            ),
          ),
          // --- END OF NEW CUSTOM BUTTON CODE ---
        ],
      ),
    );
  }

  Widget _buildFeatureGrid() {
    // WRAP GridView in Expanded
    return Expanded(
      child: GridView.count(
        crossAxisCount: 2,
        crossAxisSpacing: 16,
        mainAxisSpacing: 16,
        // 3. REMOVED shrinkWrap and physics to allow the grid to scroll naturally
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
            onTap: () => Navigator.pushNamed(context, '/rfid'),
          ),
          FeatureCard(
            icon: Icons.lightbulb_outline,
            label: "IR\nREMOTE",
            onTap: () => Navigator.pushNamed(context, '/ir'),
          ),
        ],
      ),
    );
  }
}
