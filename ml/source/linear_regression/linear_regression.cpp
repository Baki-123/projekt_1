// Implementation of the linear regression model.
#include "ml/linear_regression/linear_regression.h"

namespace ml
{
bool LinearRegression::train(const container::Vector<double>& trainInput,
                             const container::Vector<double>& trainOutput,
                             const size_t epochs,
                             const double learningRate)
{
    if (trainInput.size() == 0U || trainOutput.size() == 0U) { return false; }
    if (trainInput.size() != trainOutput.size()) { return false; }
    if (epochs == 0U) { return false; }
    if (learningRate <= 0.0) { return false; }

    const size_t trainSetCount{trainInput.size()};

    for (size_t epoch{0U}; epoch < epochs; ++epoch)
    {
        for (size_t i{0U}; i < trainSetCount; ++i)
        {
            const double prediction{myM + myK * trainInput[i]};
            const double error{prediction - trainOutput[i]};
            myM -= learningRate * error;
            myK -= learningRate * error * trainInput[i];
        }
    }

    return true;
}

double LinearRegression::predict(const double input) const noexcept
{
    return myM + myK * input;
}
} // namespace ml
