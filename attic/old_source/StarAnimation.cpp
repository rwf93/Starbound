#include "StarAnimation.hpp"

namespace Star {

AnimationControl::~AnimationControl() { }

AnimationState::~AnimationState() { }

namespace Animation {

class Frame;
typedef std::shared_ptr<Frame> FramePtr;
class Rule;
typedef std::shared_ptr<Rule> RulePtr;

class Frame {
public:
  virtual ~Frame() { }
  virtual void apply(AnimationControlPtr controller) = 0;
  static FramePtr construct(Variant const& config);
};

class StopSound: public Frame {
public:
  StopSound(Variant const& config) {
  }

  virtual void apply(AnimationControlPtr controller) {
    controller->stopSound();
  }
};

class PlaySound: public Frame {
public:
  PlaySound(Variant const& config) {
    m_sound = config.getString(1);
  }
  virtual void apply(AnimationControlPtr controller) {
    controller->playSound(m_sound);
  }

private:
  String m_sound;
};

class GraphicsFrame: public Frame {
public:
  GraphicsFrame(Variant const& config) {
    m_frame = config.getString(1);
  }
  virtual void apply(AnimationControlPtr controller) {
    controller->setGraphic(m_frame);
  }

private:
  String m_frame;
};

class WireFrame: public Frame {
public:
  WireFrame(Variant const& config) {
    m_wire = config.getInt(1);
    m_level = config.getBool(2);
  }

  virtual void apply(AnimationControlPtr controller) {
    controller->setWireLevel(m_wire, m_level);
  }

private:
  int m_wire;
  bool m_level;
};

class ChildTransitionFrame: public Frame {
public:
  ChildTransitionFrame(Variant const& config) {
    m_child = config.getString(1);
    m_state = config.getString(2);
  }
  virtual void apply(AnimationControlPtr controller) {
    controller->requestChildTransition(m_child, m_state);
  }

private:
  String m_child;
  String m_state;
};

FramePtr Frame::construct(Variant const& config) {
  String code;
  VariantList arguments;
  if (config.type() == Variant::Type::LIST) {
    arguments = config.list();
    code = config.getString(0);
  }
  else
  {
    code = "graphic";
    arguments = {code, config};
  }

  if (code == "play")
    return make_shared < PlaySound > (arguments);
  if (code == "stopSound")
    return make_shared < StopSound > (arguments);
  if (code == "graphic")
    return make_shared < GraphicsFrame > (arguments);
  if (code == "wire")
    return make_shared < WireFrame > (arguments);
  if (code == "transition")
    return make_shared < ChildTransitionFrame > (arguments);

  throw StarException("Unknown animation frame kind '" + code + "'");
  return {};
}

class Rule {
public:
  virtual ~Rule() { }
  virtual bool check(AnimationStatePtr state) = 0;
  static RulePtr construct(String const& transition, Variant const& config);
};

class RequestedRule: public Rule {
public:
  RequestedRule(Variant const& config);
  virtual bool check(AnimationStatePtr state);

private:
  String m_state;
};

RequestedRule::RequestedRule(Variant const& config) {
  m_state = config.getString(1);
}

bool RequestedRule::check(AnimationStatePtr state) {
  if (!state->hasRequestedFrame())
    return false;
  return state->requestedFrame() == m_state;
}

class CompletedRule: public Rule {
public:
  CompletedRule(Variant const& config) {
  }
  virtual bool check(AnimationStatePtr state) {
    return state->sequenceCompleted();
  }
};

class StateRule: public Rule {
public:
  StateRule(Variant const& config) {
    m_child = config.getString(1);
    m_state = config.getString(2);
  }

  virtual bool check(AnimationStatePtr state) {
    return state->childState(m_child)->currentFrame() == m_state;
  }

private:
  String m_child;
  String m_state;
};

RulePtr Rule::construct(String const& transition, Variant const& config) {
  String ruleName;
  VariantList arguments;
  if (config.type() == Variant::Type::LIST) {
    arguments = config.list();
    ruleName = config.getString(0);
  }
  else {
    ruleName = config.toString();
    arguments = {ruleName, transition};
  }

  if (ruleName == "requested")
    return make_shared < RequestedRule > (arguments);
  else if (ruleName == "completed")
    return make_shared < CompletedRule > (arguments);
  else if (ruleName == "state")
    return make_shared < StateRule > (arguments);
  throw StarException("Unknown animation transition rule '" + ruleName + "'");
  return {};
}

class Transition {
public:
  Transition(Variant const& config);

  String target;
  List<RulePtr> rules;
};

Transition::Transition(Variant const& config) {
  auto list = config.list();
  target = config.getString(0);
  for (size_t i = 1; i < list.size(); i++)
    rules.append(Rule::construct(target, list[i]));
}

}

class AnimationSequence {
public:
  AnimationSequence(Variant const& config);
  void step(float timeStep);

private:
  List<Animation::FramePtr> m_frames;
  List<float> m_frameDuration;
  List<Animation::Transition> m_transitions;
};

AnimationSequence::AnimationSequence(Variant const& config) {
  for (auto frame : config.getList(0))
    m_frames.append(Animation::Frame::construct(frame));
  for (auto transition : config.getList(1))
    m_transitions.append(Animation::Transition(transition));
  if (config.size() == 3)
      {
    if (config.list()[2].type() != Variant::Type::LIST) {
      for (size_t i = 0; i < m_frames.size(); i++)
        m_frameDuration.append(config.getFloat(2));
    }
    else {
      for (auto duration : config.getList(2))
        m_frameDuration.append(duration.toFloat());
    }
  }
}

void AnimationSequence::step(float timeStep) {
  //tooodles
}

class AnimationDefinition {
public:
  AnimationDefinition(Variant const& config);

private:
  Map<String, AnimationSequencePtr> m_sequnces;
  Map<String, AnimationDefinitionPtr> m_nestedDefinitions;
};

AnimationDefinition::AnimationDefinition(Variant const& config) {
  for (auto nested : config.getMap("nested")) {
    m_nestedDefinitions.add(nested.first, make_shared < AnimationDefinition > (nested.second));
  }
  for (auto sequence : config.getMap("sequences")) {
    m_sequnces.add(sequence.first, make_shared < AnimationSequence > (sequence.second));
  }
}

AnimationDriver::AnimationDriver(AnimationControlPtr control, AnimationStatePtr state, Variant const& config, AnimationDriverMode mode) {
  m_definition = make_shared < AnimationDefinition > (config);
  m_control = control;
  m_state = state;
  m_mode = mode;
}

void AnimationDriver::step(float timeStep) {
/// 1 magic 2 ... 3 profit
}

}
