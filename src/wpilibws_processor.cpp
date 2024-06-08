#include "wpilibws_processor.h"
// #include "OLED.h"

// Utility functions
double round3dp(double input) {
  return (int)(input * 1000 + 0.5) / 1000.0;
}

namespace wpilibws {
  WPILibWSProcessor::WPILibWSProcessor() :
        _pwmCallback([](int, double){}),
        _dsGenericCallback([]() {}),
        _dsEnabledCallback([](bool) {}),
        _encoderInitCallback([](int, bool, int, int){}),
        _dioCallback([](int, bool){}) {
          
  }

  void WPILibWSProcessor::processMessage(JsonDocument& jsonMsg) {
    // Valid messages should have type, data and device fields
    if (jsonMsg.containsKey("type") && jsonMsg.containsKey("data")) {
      // Pass off to handlers
      if (jsonMsg["type"] == "PWM") {
        this->handlePWMMessage(jsonMsg);
        // String jsonStringTxt;
        // serializeJson(jsonMsg, jsonStringTxt);
        // Serial.println(jsonStringTxt);
        
      }
      else if (jsonMsg["type"] == "DriverStation") {
        // Serial.println("DriverStation");
        this->handleDSMessage(jsonMsg);
      }
      else if (jsonMsg["type"] == "Encoder") {
        // Serial.println("Encoder");
        this->handleEncoderMessage(jsonMsg);
        // String jsonStringTxt;
        // serializeJson(jsonMsg, jsonStringTxt);
        // Serial.println(jsonStringTxt);
      }
      else if (jsonMsg["type"] == "DIO") {
        // Serial.println("DIO");
        this->handleDIOMessage(jsonMsg);
      }
      else if (jsonMsg["type"] == "Gyro") {
        // Serial.println("Gyro");
        // this->handleGyroMessage(jsonMsg);
      }
    }
  }

  // Callback Handlers
  void WPILibWSProcessor::onPWMMessage(PWMCallback callback) {
    this->_pwmCallback = callback;
  }

  void WPILibWSProcessor::onDSGenericMessage(DSGenericCallback callback) {
    this->_dsGenericCallback = callback;
  }

  void WPILibWSProcessor::onDSEnabledMessage(DSEnabledCallback callback) {
    this->_dsEnabledCallback = callback;
  }

  void WPILibWSProcessor::onEncoderInitMessage(EncoderInitCallback callback) {
    this->_encoderInitCallback = callback;
  }

  void WPILibWSProcessor::onDIOMessage(DIOCallback callback) {
    this->_dioCallback = callback;
  }

  void WPILibWSProcessor::onGyroInitMessage(GyroInitCallback callback) {
    this->_gyroInitCallback = callback;
  }

  // Message generators
  std::string WPILibWSProcessor::makeEncoderMessage(int deviceId, int count) {
    JsonDocument msg;
    msg["type"] = "Encoder";
    msg["device"] = std::to_string(deviceId);
    msg["data"][">count"] = count;

    std::string ret;
    serializeJson(msg, ret);
    return ret;
  }

  std::string WPILibWSProcessor::makeGyroMessage(float rates[3], float angles[3]) {
    JsonDocument msg;
    msg["type"] = "Gyro";
    msg["device"] = "RomiGyro";
    msg["data"][">rate_x"] = round3dp(rates[0]);
    msg["data"][">rate_y"] = round3dp(rates[1]);
    msg["data"][">rate_z"] = round3dp(rates[2]);
    msg["data"][">angle_x"] = round3dp(angles[0]);
    msg["data"][">angle_y"] = round3dp(angles[1]);
    msg["data"][">angle_z"] = round3dp(angles[2]);

    std::string ret;
    serializeJson(msg, ret);
    return ret;
  }

  // Privates
  void WPILibWSProcessor::handlePWMMessage(JsonDocument& pwmMsg) {
    // Get the channel
    int channel = atoi(pwmMsg["device"].as<const char*>());
    auto data = pwmMsg["data"];

    if (data.containsKey("<speed")) {
      // Speed values are [-1.0, 1.0]
      double value = atof(data["<speed"].as<std::string>().c_str());
      // this->encoder_counts[channel] = this->encoder_counts[channel] + (value * 1000);
      // String jsonStringTxt;
      // serializeJson(pwmMsg, jsonStringTxt);
      // Serial.println(jsonStringTxt);
      // clearLine(4 + channel);
      // drawText(4 + channel, 0, String(value));
      this->_pwmCallback(channel, value);
    }
    else if (data.containsKey("<position")) {
      // Position information is [0.0, 1.0]. We should convert to [-1.0, 1.0]
      double value = atof(data["<speed"].as<std::string>().c_str());
      value = (2.0 * value) - 1.0;
      // this->_pwmCallback(channel, value);
    }
  }

  void WPILibWSProcessor::handleDSMessage(JsonDocument& dsMsg) {
    // Send the DSGenericCallback
    this->_dsGenericCallback();

    auto data = dsMsg["data"];
    if (data.containsKey(">enabled")) {
      // String jsonStringTxt;
      // serializeJson(dsMsg, jsonStringTxt);
      // Serial.println(jsonStringTxt);
      // drawDSState(data[">enabled"].as<bool>());     
      this->_dsEnabledCallback(data[">enabled"].as<bool>());
    }
  }

  void WPILibWSProcessor::handleEncoderMessage(JsonDocument& encoderMsg) {
    int deviceNum = atoi(encoderMsg["device"].as<const char*>());
    auto data = encoderMsg["data"];

    if (data.containsKey("<init")) {
      bool initValue = data["<init"];
      int chA = -1;
      int chB = -1;

      if (data.containsKey("<channel_a")) {
        chA = data["<channel_a"];
      }
      if (data.containsKey("<channel_b")) {
        chB = data["<channel_b"];
      }

      this->_encoderInitCallback(deviceNum, initValue, chA, chB);
    }
  }

  void WPILibWSProcessor::handleDIOMessage(JsonDocument& dioMsg) {
    int channel = atoi(dioMsg["device"].as<const char*>());
    auto data = dioMsg["data"];

    if (data.containsKey("<>value")) {
      this->_dioCallback(channel, data["<>value"]);
    }
  }

  void WPILibWSProcessor::handleGyroMessage(JsonDocument& gyroMsg) {
    std::string gyroName = gyroMsg["device"];
    auto data = gyroMsg["data"];

    if (data.containsKey("<init")) {
      bool initValue = data["<init"];

      this->_gyroInitCallback(gyroName, initValue);
    }
  }
}
