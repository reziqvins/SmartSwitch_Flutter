import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'dart:async';
import 'package:connectivity_plus/connectivity_plus.dart';

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text(
                "Relay Control",
                style: TextStyle(
                  color: Colors.black,
                  fontSize: 20,
                  fontWeight: FontWeight.bold,
                ),
              ),
              Builder(
                builder: (BuildContext context) {
                  return IconButton(
                    icon: Icon(Icons.info_outline, color: Colors.black),
                    onPressed: () {
                      Navigator.push(
                        context,
                        MaterialPageRoute(builder: (context) => AboutUsPage()),
                      );
                    },
                  );
                },
              ),
            ],
          ),
          backgroundColor: Colors.white,
          elevation: 0,
          centerTitle: true,
        ),
        body: ControlPanel(),
      ),
    );
  }
}

class ControlPanel extends StatefulWidget {
  @override
  _ControlPanelState createState() => _ControlPanelState();
}

class _ControlPanelState extends State<ControlPanel> {
  final String serverUrl = "http://192.168.1.15"; // Replace with your microcontroller's IP
  bool isOn = false;
  bool scheduleEnabled = false;
  TimeOfDay? turnOnTime;
  TimeOfDay? turnOffTime;
  Timer? timer;

  @override
  void initState() {
    super.initState();
    checkConnectivity();
    startTimer();
  }

  @override
  void dispose() {
    timer?.cancel();
    super.dispose();
  }

  Future<bool> isConnectedToWiFi() async {
    ConnectivityResult connectivityResult = await Connectivity().checkConnectivity();
    return connectivityResult == ConnectivityResult.wifi;
  }

  void checkConnectivity() async {
    if (await isConnectedToWiFi()) {
      print("Connected to Wi-Fi.");
    } else {
      showConnectivityAlert();
    }
  }

  void showConnectivityAlert() {
    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: Text("Tidak Terhubung ke Wifi"),
          content: Text("Hubungkan ke Wifi DARUSMAN HOME untuk menggunakan fitur ini"),
          actions: [
            TextButton(
              child: Text("OK"),
              onPressed: () {
                Navigator.of(context).pop();
              },
            ),
          ],
        );
      },
    );
  }

  void startTimer() {
    timer = Timer.periodic(Duration(seconds: 10), (Timer t) {
      checkSchedule();
    });
  }

  void toggleRelay(String action) async {
    if (await isConnectedToWiFi()) {
      try {
        final response = await http.get(Uri.parse('$serverUrl/relay/$action'));
        if (response.statusCode == 200) {
          print('Relay $action successfully');
          setState(() {
            isOn = action == "on";
          });
        } else {
          print('Failed to toggle relay');
        }
      } catch (e) {
        print('Error: $e');
      }
    } else {
      showConnectivityAlert();
    }
  }

  void checkSchedule() {
    if (scheduleEnabled) {
      final now = TimeOfDay.now();
      if (turnOnTime != null && now.hour == turnOnTime!.hour && now.minute == turnOnTime!.minute) {
        toggleRelay("on");
      }
      if (turnOffTime != null && now.hour == turnOffTime!.hour && now.minute == turnOffTime!.minute) {
        toggleRelay("off");
      }
    }
  }

  Future<void> selectTime(BuildContext context, bool isTurnOnTime) async {
    final TimeOfDay? picked = await showTimePicker(
      context: context,
      initialTime: TimeOfDay.now(),
    );
    if (picked != null) {
      setState(() {
        if (isTurnOnTime) {
          turnOnTime = picked;
          print('Nyalakan pada jam ${turnOnTime!.format(context)}');
        } else {
          turnOffTime = picked;
          print('Matikan pada jam ${turnOffTime!.format(context)}');
        }
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Text(
            'Pompa Air ${isOn ? "OFF" : "ON"}',
            style: TextStyle(
              fontSize: 24,
              fontWeight: FontWeight.bold,
              color: isOn ? Colors.red : Colors.green,
            ),
          ),
          SizedBox(height: 20),
          CircularControl(
            isOn: isOn,
            toggleRelay: toggleRelay,
          ),
          SizedBox(height: 30),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Padding(
                padding: const EdgeInsets.symmetric(horizontal: 16.0),
                child: Text(
                  'Atur Jadwal',
                  style: TextStyle(fontSize: 16),
                ),
              ),
              Switch(
                value: scheduleEnabled,
                onChanged: (value) {
                  setState(() {
                    scheduleEnabled = value;
                    if (!scheduleEnabled) {
                      turnOnTime = null; 
                      turnOffTime = null;
                    }
                  });
                },
              ),
            ],
          ),
          if (scheduleEnabled) ...[
            Padding(
              padding: const EdgeInsets.symmetric(vertical: 10.0),
              child: ElevatedButton(
                onPressed: () => selectTime(context, true),
                child: Text(
                  turnOnTime != null
                      ? 'Nyalakan pada ${turnOnTime!.format(context)}'
                      : 'Mengatur Waktu Nyalakan',
                ),
              ),
            ),
            Padding(
              padding: const EdgeInsets.symmetric(vertical: 10.0),
              child: ElevatedButton(
                onPressed: () => selectTime(context, false),
                child: Text(
                  turnOffTime != null
                      ? 'Matikan pada ${turnOffTime!.format(context)}'
                      : 'Mengatur Waktu Mati',
                ),
              ),
            ),
          ],
        ],
      ),
    );
  }
}

class CircularControl extends StatelessWidget {
  final bool isOn;
  final Function(String) toggleRelay;

  CircularControl({required this.isOn, required this.toggleRelay});

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: () {
        toggleRelay(isOn ? "off" : "on");
      },
      child: Stack(
        alignment: Alignment.center,
        children: [
          SizedBox(
            width: 200,
            height: 200,
            child: CircularProgressIndicator(
              value: isOn ? 1.0 : 0.0,
              strokeWidth: 8,
              valueColor: AlwaysStoppedAnimation<Color>(
                isOn ? Colors.green : Colors.grey,
              ),
              backgroundColor: Colors.grey.shade200,
            ),
          ),
          Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(
                isOn ? Icons.power_settings_new : Icons.power_off,
                size: 40,
                color: isOn ? Colors.green : Colors.red,
              ),
              SizedBox(height: 10),
              Text(
                isOn ? "Nyalakan" : "Matikan",
                style: TextStyle(fontSize: 16),
              ),
            ],
          ),
        ],
      ),
    );
  }
}

class AboutUsPage extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(
          "About Us",
          style: TextStyle(
            color: Colors.black,
            fontSize: 20,
            fontWeight: FontWeight.bold,
          ),
        ),
        backgroundColor: Colors.white,
        elevation: 0,
        centerTitle: true,
      ),
      body: Center(
        child: Padding(
          padding: const EdgeInsets.all(8.0),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Image.network(
                "https://res.cloudinary.com/dap6ohre8/image/upload/v1725270097/iseng/WhatsApp_Image_2024-09-02_at_16.41.16_1af48ae9_txxipp.jpg",
                height: 500, // Adjust height as needed
                fit: BoxFit.cover,
              ),
              SizedBox(height: 20),
              Text(
                "Reziq Vins",
                style: TextStyle(
                  fontSize: 24,
                  fontWeight: FontWeight.bold,
                ),
              ),
              SizedBox(height: 20),
              Text(
                "This is a sample application to demonstrate relay control and scheduling features.",
                textAlign: TextAlign.center,
                style: TextStyle(
                  fontSize: 16,
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
