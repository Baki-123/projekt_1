/**
 * @brief Generic system implementation details for an MCU with configurable hardware devices.
 */
#include <stdint.h>

#include "driver/adc/interface.h"
#include "driver/eeprom/interface.h"
#include "driver/gpio/interface.h"
#include "driver/serial/interface.h"
#include "driver/timer/interface.h"
#include "driver/watchdog/interface.h"
#include "ml/linear_regression/interface.h"
#include "target/system.h"

namespace
{
// -----------------------------------------------------------------------------
constexpr int round(const float num) noexcept
{
    const double rounded{0.0 <= num ? num + 0.5 : num - 0.5};
    return static_cast<int>(rounded);
}

} // namespace

namespace target
{
// -----------------------------------------------------------------------------
System::System(driver::GpioInterface& led, driver::GpioInterface& button,
               driver::TimerInterface& debounceTimer, driver::TimerInterface& predictionTimer,
               driver::SerialInterface& serial, driver::WatchdogInterface& watchdog,
               driver::EepromInterface& eeprom, driver::AdcInterface& adc,
               ml::LinearRegressionInterface& model, uint8_t tempSensorPin) noexcept
    : myLed{led}
    , myButton{button}
    , myDebounceTimer{debounceTimer}
    , myPredictionTimer{predictionTimer}
    , mySerial{serial}
    , myWatchdog{watchdog}
    , myEeprom{eeprom}
    , myAdc{adc}
    , myModel{model}
    , myTempSensorPin{tempSensorPin}
{
    myButton.enableInterrupt(true);
    mySerial.setEnabled(true);
    myWatchdog.setEnabled(true);
    myAdc.setEnabled(true);
    myPredictionTimer.start();
}

// -----------------------------------------------------------------------------
System::~System() noexcept
{
    myButton.enableInterrupt(false);
    myDebounceTimer.stop();
    myPredictionTimer.stop();
    myWatchdog.setEnabled(false);
}

// -----------------------------------------------------------------------------
void System::enableSerialTransmission(const bool enable) noexcept
{
    mySerial.setEnabled(enable);
}

// -----------------------------------------------------------------------------
void System::handleButtonInterrupt() noexcept
{
    myButton.enableInterruptOnPort(false);
    myDebounceTimer.start();
    if (myButton.read()) { handleButtonPressed(); }
}

// -----------------------------------------------------------------------------
void System::handleDebounceTimerInterrupt() noexcept
{
    myDebounceTimer.stop();
    myButton.enableInterruptOnPort(true);

    // Disable the LED (it was enabled on button press).
    myLed.write(false);
}

// -----------------------------------------------------------------------------
void System::handlePredictionTimerInterrupt() noexcept
{
    predictAndPrintTemperature();
}

// -----------------------------------------------------------------------------
void System::run() noexcept
{
    mySerial.printf("Running the system!\n");

    while (1)
    {
        if (myPredictionTimer.hasTimedOut()) { predictAndPrintTemperature(); }
        myWatchdog.reset();
    }
}

// -----------------------------------------------------------------------------
void System::handleButtonPressed() noexcept
{
    // Enable the LED for 300 ms after pressdown (just for show).
    myLed.write(true);
    predictAndPrintTemperature();
    myPredictionTimer.restart();
}

// -----------------------------------------------------------------------------
void System::predictAndPrintTemperature() noexcept
{
    const double inputVoltage{myAdc.inputVoltage(myTempSensorPin)};
    const int temperature{round(myModel.predict(inputVoltage))};
    mySerial.printf("Temperature: %d C\n", temperature);
}
} // namespace target
