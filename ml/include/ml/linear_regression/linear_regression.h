/**
 * @brief Linear regression model implementation.
 */
#pragma once

#include "ml/linear_regression/interface.h"

namespace ml
{

// Implements a linear regression model (y = k * x + m) with training using gradient descent.
class LinearRegression final : public LinearRegressionInterface
{
public:

    // Create a new linear regression model with k and m starting with zero.
    LinearRegression() noexcept = default;

    // Delete the linear regression model.
    ~LinearRegression() noexcept override = default;

    // Train the model with given training data.
    bool train(const container::Vector<double>& trainInput,
               const container::Vector<double>& trainOutput,
               const size_t epochs,
               const double learningRate) override;

    // Predict output for a given input value.
    double predict(const double input) const noexcept override;

    LinearRegression(const LinearRegression&)            = delete;
    LinearRegression(LinearRegression&&)                 = delete;
    LinearRegression& operator=(const LinearRegression&) = delete;
    LinearRegression& operator=(LinearRegression&&)      = delete;

private:
    // The learned k of the model.
    double myK{0};

    // The learned m of the model.
    double myM{0};
};
} // namespace ml
