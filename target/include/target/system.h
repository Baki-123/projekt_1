/**
 * @brief Generic system implementation for an MCU with configurable hardware devices.
 */
#pragma once

namespace driver
{
/** ADC (A/D converter) interface. */
class AdcInterface;

/** EEPROM (Electrically Erasable Programmable ROM) stream interface. */
class EepromInterface;

/** GPIO interface. */
class GpioInterface;

/** Serial transmission interface. */
class SerialInterface;

/** Timer interface. */
class TimerInterface;

/** Watchdog timer interface. */
class WatchdogInterface;
} // namespace driver

namespace ml
{
// Linear regression model interface.
class LinearRegressionInterface;
} // namespace ml

namespace target
{
/**
 * @brief Generic system for an MCU with configurable hardware devices.
 *
 *        The following devices are used:
 *
 *            - A button connected to the device triggers a  prediction.
 *
 *            - A timer triggers a prediction every 60 seconds.
 *
 *            - Another timer reduces the effect of contact bounces after pushing the button.
 *
 *            - A watchdog timer is used to restart the program if it gets stuck somewhere.
 *
 *            - A trained model predicts the temperature from sensor input.
 *
 *        This class is non-copyable and non-movable.
 */
class System final
{
public:
    /**
     * @brief Create a new system.
     *
     * @param[in] led LED enabled on temperature prediction.
     * @param[in] button Button used to trigger a prediction.
     * @param[in] debounceTimer Timer used to mitigate effects of contact bounces.
     * @param[in] predictionTimer Timer used to trigger periodic predictions.
     * @param[in] serial Serial device used to print predictions.
     * @param[in] watchdog Watchdog timer that resets the program if it becomes unresponsive.
     * @param[in] eeprom EEPROM stream (Currently unused).
     * @param[in] adc ADC used to read the temperature sensor input.
     * @param[in] model Trained model for predictions.
     * @param[in] tempSensorPin Temperature sensor pin.
     */
    explicit System(driver::GpioInterface& led, driver::GpioInterface& button,
                    driver::TimerInterface& debounceTimer, driver::TimerInterface& predictionTimer,
                    driver::SerialInterface& serial, driver::WatchdogInterface& watchdog,
                    driver::EepromInterface& eeprom, driver::AdcInterface& adc,
                    ml::LinearRegressionInterface& model, uint8_t tempSensorPin) noexcept;

    /**
     * @brief Delete system.
     */
    ~System() noexcept;

    /**
     * @brief Enable serial transmission.
     * 
     * @param[in] enable Indicate whether to enable serial transmission.
     */
    void enableSerialTransmission(const bool enable) noexcept;

    /**
     * @brief Button interrupt handler.
     *
     *        Trigger a prediction whenever the button is pressed.
     *
     *        Pin change interrupts are disabled for 300 ms after a press to mitigate the effects 
     *        of contact bounce.
     */
    void handleButtonInterrupt() noexcept;

    /**
     * @brief Debounce timer interrupt handler.
     * 
     *        Enable pin change interrupts 300 ms after a press to mitigate the effects of contact bounce.
     */
    void handleDebounceTimerInterrupt() noexcept;

    /**
     * @brief Prediction timer interrupt handler.
     *
     *        Predict and print the temperature every 60 seconds.
     */
    void handlePredictionTimerInterrupt() noexcept;

    /**
     * @brief Run the system as long as voltage is supplied.                                                               
     */
    void run() noexcept;

    System()                         = delete; // No default constructor.
    System(const System&)            = delete; // No copy constructor.
    System(System&&)                 = delete; // No move constructor.
    System& operator=(const System&) = delete; // No copy assignment.
    System& operator=(System&&)      = delete; // No move assignment.

private:
    void handleButtonPressed() noexcept;
    void predictAndPrintTemperature() noexcept;

    /** Reference to the LED (currently unused). */
    driver::GpioInterface& myLed;

    /** Button used to trigger a temperature prediction. */
    driver::GpioInterface& myButton;

    /** Debounce timer used to mitigate effects of contact bounces. */
    driver::TimerInterface& myDebounceTimer;

    /** Timer used to trigger periodic temperature predictions. */
    driver::TimerInterface& myPredictionTimer;

    /** Serial device used to print temperature predictions. */
    driver::SerialInterface& mySerial;

    /** Watchdog timer that resets the program if it becomes unresponsive. */
    driver::WatchdogInterface& myWatchdog;

    /** EEPROM stream (currently unused). */
    driver::EepromInterface& myEeprom;

    /** ADC used to read the temperature sensor input. */
    driver::AdcInterface& myAdc;

    // Trained linear regression model for temperature prediction.
    ml::LinearRegressionInterface& myModel;

    /** Temperature sensor pin. */
    const uint8_t myTempSensorPin;
};
} // namespace target