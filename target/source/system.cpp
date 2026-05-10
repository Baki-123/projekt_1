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
// The pin connected to the TMP36 sensor.
static constexpr uint8_t tempSensorPin{0U};

// The timer timeout in milliseconds, 60 seconds.
static constexpr uint32_t predictionTimeout_ms{60000U};
} // namespace

namespace target
{
// -----------------------------------------------------------------------------
System::System(driver::GpioInterface& led, driver::GpioInterface& button,
               driver::TimerInterface& debounceTimer, driver::TimerInterface& predictionTimer,
               driver::SerialInterface& serial, driver::WatchdogInterface& watchdog,
               driver::EepromInterface& eeprom, driver::AdcInterface& adc,
               ml::LinearRegressionInterface& model) noexcept
    : myLed{led}
    , myButton{button}
    , myDebounceTimer{debounceTimer}
    , myPredictionTimer{predictionTimer}
    , mySerial{serial}
    , myWatchdog{watchdog}
    , myEeprom{eeprom}
    , myAdc{adc}
    , myModel{model}
{
    myButton.enableInterrupt(true);
    mySerial.setEnabled(true);
    myWatchdog.setEnabled(true);
    myAdc.setEnabled(true);
    myPredictionTimer.setTimeout_ms(predictionTimeout_ms);
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
    predictAndPrintTemperature();
    myPredictionTimer.restart();
}

// -----------------------------------------------------------------------------
void System::predictAndPrintTemperature() noexcept
{
    const double inputVoltage{myAdc.inputVoltage(tempSensorPin)};
    const double temperature{myModel.predict(inputVoltage)};
    mySerial.printf("Temperature: %.1f C\n", temperature);
}
} // namespace target
