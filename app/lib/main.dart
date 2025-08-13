import 'package:flutter/material.dart';
import 'routes/app_routes.dart';

void main() {
  runApp(const FlipperCloneXApp());
}

class FlipperCloneXApp extends StatelessWidget {
  const FlipperCloneXApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flipper CloneX',
      debugShowCheckedModeBanner: false,
      theme: ThemeData.dark().copyWith(
        scaffoldBackgroundColor: Colors.black,
        primaryColor: Colors.cyanAccent,
      ),
      initialRoute: AppRoutes.home,
      onGenerateRoute: AppRoutes.generateRoute,
    );
  }
}
