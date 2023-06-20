#include "StarStepInterpolation.hpp"
#include "StarRandom.hpp"

using namespace Star;

int main(int argc, char** argv) {
  double serverStep = 0.0;
  double dataPoint = 0.0;

  StepStream<double> doubleStream;
  doubleStream.setExtrapolation(15);

  StepTracker stepTracker(6, 0.05);

  // Simulate simple motion with some step skips, jitter, and pauses.
  for (size_t i = 1; i < 200; ++i) {
    // Produce a server step point.

    if (i < 90)
      dataPoint = i * 3;
    else if (i >= 120 && i < 150)
      dataPoint = (i - 30) * 3;
    else if (i >= 150)
      dataPoint = (270 - i) * 3;
    else
      dataPoint = -1.0f;

    size_t doSteps = 0;
    if (i == 50)
      doSteps = 9;
    else if (i == 51 || i == 52)
      doSteps = 0;
    else if (i % 11 == 0)
      doSteps = 4;
    else if (i % 19 == 0)
      doSteps = 2;
    else
      doSteps = 3;

    serverStep += 3.0;

    // Consume the server point on the client, and do doSteps of client
    // iterations.
  
    stepTracker.heartbeat(serverStep);
    if (dataPoint >= 0.0f) {
      doubleStream.addDataPoint(stepTracker.leadStep(), dataPoint);
      // coutf("%s %s\n", predictedStep, dataPoint);
    } else {
      doubleStream.heartbeat(stepTracker.leadStep());
    }

    for (size_t s = 0; s < doSteps; ++s) {
      stepTracker.update(stepTracker.currentStep() + 1.0f);
      doubleStream.setStep(stepTracker.currentStep());

      coutf("%s\n", doubleStream.interpolate());
    }
  }

  return 0;
}
