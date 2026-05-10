// Interface for linear regression models.
#pragma once

#include "container/vector.h"

namespace ml
{

// Interface for linear regression models.
class LinearRegressionInterface
{
public:

    // Delete the model.
    virtual ~LinearRegressionInterface() noexcept = default;

    // Train the model with given training data.
    virtual bool train(const container::Vector<double>& trainInput,
                       const container::Vector<double>& trainOutput,
                       const size_t epochs,
                       const double learningRate) = 0;

    // Predict output for a given input value.
    virtual double predict(const double input) const = 0;
};
} // namespace ml
