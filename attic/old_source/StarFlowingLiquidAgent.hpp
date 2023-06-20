#ifndef STAR_FLOWING_LIQUID_AGENT_HPP
#define STAR_FLOWING_LIQUID_AGENT_HPP

#include "StarRandom.hpp"
#include "StarVector.hpp"
#include "StarLiquidTypes.hpp"

namespace Star {

STAR_CLASS(LivingWorldAgent);
STAR_CLASS(LivingWorldFacade);
STAR_CLASS(FlowingLiquidAgent); 

class FlowingLiquidAgent {
public:
  FlowingLiquidAgent();

	void bind(LivingWorldFacadePtr world, LivingWorldAgent* livingWorld);
	void processLocation(Vec2I const& location);

private:
	LiquidLevel getLiquidLevel(int x, int y);
	bool hasBackground(int x, int y);
	bool isOcean(int x, int y);
	LiquidId oceanLiquid(int x, int y);
	uint16_t oceanLiquidPressure(int x, int y);


	void moveLiquid(int x, int y, int dx, int dy, LiquidLevel proposedSourceLevel, LiquidLevel proposedTargetLevel);
	void setLiquidLevel(int x, int y, LiquidLevel level);

	bool moveLiquidDown(Vec2I c, LiquidLevel above, bool attemptMoveOut);
	bool moveLiquidSideWays(Vec2I c, int fountain);
	int checkMoveLiquidSideways(int x, int y, LiquidLevel level);
	bool moveLiquidUpwards(Vec2I c, LiquidLevel level);
	bool moveLiquidOut(Vec2I c);

  bool sanityCheckLiquid(LiquidLevel const& level) const;

	LivingWorldAgent* m_livingWorld;
	RandomSource m_random;
	LivingWorldFacadePtr m_world;
};

}

#endif
