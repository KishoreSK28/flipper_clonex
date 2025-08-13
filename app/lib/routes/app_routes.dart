import 'package:flutter/material.dart';
import '../ui/home_screen.dart';
import '../ui/settings_screen.dart';
import '../ui/storage_screen.dart';
import '../ui/wifi_screen.dart';
import '../ui/bluetooth_screen.dart';
// Add other screens here when needed

class AppRoutes {
  static const String home = '/';
  static const String settings = '/settings';
  
  static const String storage = '/storage';
  static const String wifi = '/wifi';
  static const String bluetooth = '/bluetooth';
  static const String rfid = '/rfid';
  static const String ir = '/ir';

  static Route<dynamic> generateRoute(RouteSettings settings) {
    switch (settings.name) {
      case home:
        return MaterialPageRoute(builder: (_) => const HomeScreen());
      case AppRoutes.settings:
        return MaterialPageRoute(builder: (_) => const SettingsScreen());
      case AppRoutes.storage:
        return MaterialPageRoute(builder: (_) => StorageScreen());
      case AppRoutes.wifi:
        return MaterialPageRoute(builder: (_) => WifiScreen());
      case AppRoutes.bluetooth:
        return MaterialPageRoute(builder: (_) => BluetoothScreen());

      default:
        return MaterialPageRoute(
          builder: (_) =>
              const Scaffold(body: Center(child: Text("Page not found"))),
        );
    }
  }
}
