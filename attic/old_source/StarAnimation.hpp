#ifndef STAR_ANIMATION_HPP
#define STAR_ANIMATION_HPP

#include "StarVariant.hpp"
#include "StarColor.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"

namespace Star {

class AnimationControl;
typedef std::shared_ptr<AnimationControl> AnimationControlPtr;
class AnimationState;
typedef std::shared_ptr<AnimationState> AnimationStatePtr;
class AnimationDefinition;
typedef std::shared_ptr<AnimationDefinition> AnimationDefinitionPtr;
class AnimationDriver;
typedef std::shared_ptr<AnimationDriver> AnimationDriverPtr;
class AnimationSequence;
typedef std::shared_ptr<AnimationSequence> AnimationSequencePtr;

class AnimationControl {
public:
  virtual ~AnimationControl();
  virtual void stopSound() = 0;
  virtual void playSound(String const& resource) = 0;
  virtual void setGraphic(String const& resource) = 0;
  virtual void setWireLevel(int wire, bool level) = 0;
  virtual void requestChildTransition(String const& child, String const& state) = 0;

  virtual void bind(AnimationDefinitionPtr definition) = 0;
};

class AnimationState {
public:
  virtual ~AnimationState();
  virtual String currentFrame() = 0;
  virtual int frameSequence() = 0;
  virtual bool sequenceCompleted() = 0;
  virtual bool soundPlaying() = 0;
  virtual bool hasRequestedFrame() = 0;
  virtual String requestedFrame() = 0;
  virtual bool getWireLevel(int wire) = 0;
  virtual AnimationStatePtr childState(String const& child) = 0;

  virtual void bind(AnimationDefinitionPtr definition) = 0;
};

enum class AnimationDriverMode {
  Server,
  Client
};

class AnimationDriver {
public:
  AnimationDriver(AnimationControlPtr control, AnimationStatePtr state, Variant const& config, AnimationDriverMode mode);
  void step(float timeStep);

private:
  AnimationDefinitionPtr m_definition;
  AnimationControlPtr m_control;
  AnimationStatePtr m_state;
  AnimationDriverMode m_mode;
};

}

#endif
