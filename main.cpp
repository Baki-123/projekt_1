/**
 * @brief Temperature prediction system using linear regression:
 *
 *        On startup, an ML model is trained on a TMP36 sensors data.
 *        On button press or after every 60 seconds, the program 
 *        prints the predicted temperature to a serial terminal.
 */
#include "container/vector.h"
#include "driver/atmega328p/adc.h"
#include "driver/atmega328p/eeprom.h"
#include "driver/atmega328p/gpio.h"
#include "driver/atmega328p/serial.h"
#include "driver/atmega328p/timer.h"
#include "driver/atmega328p/watchdog.h"
#include "ml/linear_regression/linear_regression.h"
#include "target/system.h"

using namespace driver::atmega328p;

namespace
{
/** Pointer to the system implementation. */
target::System* mySys{nullptr};

/**
 * @brief Callback for the button.
 */
void buttonCallback() noexcept { mySys->handleButtonInterrupt(); }

/**
 * @brief Callback for the debounce timer.
 *
 *        This callback is invoked whenever the debounce timer elapses.
 */
void debounceTimerCallback() noexcept { mySys->handleDebounceTimerInterrupt(); }

/**
 * @brief Callback for the prediction timer.
 *
 *        This callback is invoked whenever the prediction timer elapses.
 */
void predictionTimerCallback() noexcept { mySys->handlePredictionTimerInterrupt(); }

} // namespace

/**
 * @brief Train a linear regression model, initialize and run the system on the target MCU.
 *
 * @return 0 on termination of the program (should never occur).
 */
int main()
{
    // Training data derived from: U_IN = ADC_value / 1023 * 5, T = (U_IN - 0.5) * 100.
    container::Vector<double> trainInput;
    trainInput.pushBack(0.499);
    trainInput.pushBack(0.552);
    trainInput.pushBack(0.601);
    trainInput.pushBack(0.650);
    trainInput.pushBack(0.699);
    trainInput.pushBack(0.753);
    trainInput.pushBack(0.802);
    trainInput.pushBack(0.851);
    trainInput.pushBack(0.899);
    trainInput.pushBack(0.948);
    trainInput.pushBack(1.002);
    trainInput.pushBack(1.051);

    container::Vector<double> trainOutput;
    trainOutput.pushBack(-0.10);
    trainOutput.pushBack(5.22);
    trainOutput.pushBack(10.10);
    trainOutput.pushBack(15.00);
    trainOutput.pushBack(19.90);
    trainOutput.pushBack(25.32);
    trainOutput.pushBack(30.21);
    trainOutput.pushBack(35.09);
    trainOutput.pushBack(39.92);
    trainOutput.pushBack(44.81);
    trainOutput.pushBack(50.24);
    trainOutput.pushBack(55.13);

    // Create and train the model before initialization.
    ml::LinearRegression model;
    model.train(trainInput, trainOutput, 1000U, 0.01);

    // Initialize the GPIO devices.
    Gpio led{8U, Gpio::Direction::Output};
    Gpio button{13U, Gpio::Direction::InputPullup, buttonCallback};

    // Initialize the timers.
    Timer debounceTimer{300U, debounceTimerCallback};
    Timer predictionTimer{60000U, predictionTimerCallback};

    // Obtain a reference to the singleton serial device instance.
    auto& serial{Serial::getInstance()};

    // Obtain a reference to the singleton watchdog timer instance.
    auto& watchdog{Watchdog::getInstance()};

    // Obtain a reference to the singleton EEPROM instance.
    auto& eeprom{Eeprom::getInstance()};

    // Obtain a reference to the singleton ADC instance.
    auto& adc{Adc::getInstance()};

    // Initialize the system with the given hardware and model.
    target::System system{led, button, debounceTimer, predictionTimer, serial, watchdog, eeprom, adc, model};
    mySys = &system;

    // Run the system perpetually on the target MCU.
    mySys->run();

    // This point should never be reached; the system is intended to run indefinitely on the target MCU.
    return 0;
}
