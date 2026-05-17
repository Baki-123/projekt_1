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

/**
 * @brief Train the model to predict the temperature.
 * 
 * @param[out] model Model to train.
 * @param[in] epochCount Number of epochs to train (default = 1000).
 * @param[in] learningRate Learning rate to use (default = 1 %).
 * 
 * @return True on success, false on failure.
 */
bool trainModel(ml::LinearRegressionInterface& model,
               const size_t epochCount = 1000U, const double learningRate = 0.01) noexcept
{
    // Input voltage (input for the model).
    const container::Vector<double> inputVoltage{0.0, 0.1, 0.2, 0.3, 0.4,
                                                 0.5, 0.6, 0.7, 0.8, 0.9};

    // Corresponding temperature in Celsius (expected output).
    // Computed with T = 100 * Vin - 50.
    const container::Vector<double> expectedTemp{-50.0, -40.0, -30.0, -20.0, -10.0,
                                                 0.0, 10.0, 20.0, 30.0, 40.0};

    // Train the model, return the result.
    return model.train(inputVoltage, expectedTemp, epochCount, learningRate);
}

} // namespace

/**
 * @brief Train a linear regression model, initialize and run the system on the target MCU.
 *
 * @return 0 on termination of the program (should never occur).
 */
int main()
{
    // Obtain a reference to the singleton serial device instance.
    auto& serial{Serial::getInstance()};
    serial.setEnabled(true);

    // Create and train the model before initialization.
    ml::LinearRegression model{};
    if (!trainModel(model))
    {
        serial.printf("Failed to train linear regression model, aborting...\n");
        return -1;
    }
    serial.printf("Successfully trained linear regression model!\n");

    // Initialize the GPIO devices.
    Gpio led{8U, Gpio::Direction::Output};
    Gpio button{13U, Gpio::Direction::InputPullup, buttonCallback};

    // Initialize the timers.
    Timer debounceTimer{300U, debounceTimerCallback};
    Timer predictionTimer{60000U, predictionTimerCallback};

    // Obtain a reference to the singleton watchdog timer instance.
    auto& watchdog{Watchdog::getInstance()};

    // Obtain a reference to the singleton EEPROM instance.
    auto& eeprom{Eeprom::getInstance()};

    // Obtain a reference to the singleton ADC instance.
    auto& adc{Adc::getInstance()};

    // Temperature sensor pin.
    constexpr uint8_t tempSensorPin{2U};

    // Initialize the system with the given hardware and model.
    target::System system{led, button, debounceTimer, predictionTimer, serial, watchdog, 
                          eeprom, adc, model, tempSensorPin};
    mySys = &system;

    // Run the system perpetually on the target MCU.
    mySys->run();

    // This point should never be reached; the system is intended to run indefinitely on the target MCU.
    return 0;
}
