-- This script can be run either in starbound or in an external lua interpreter.

-- Run in starbound:
--  /spawnitem questgentest
-- Place the questgentest object down, point at it with the cursor and run:
--  /entityeval blocksWorld()
--  /entityeval campfire()
--  /entityeval recipes()

-- To run in an external interpreter:
--  cd assets/packed
--  lua -i ../devel/scripts/questgen/plannerTests.lua
--  > setupTests()
--  > blocksWorld()
--  > campfire()
--  > recipes()
function setupTests()
  if sb then
    require("/scripts/util.lua")
    require("/scripts/questgen/util.lua")
    require("/scripts/questgen/planner.lua")
  else
    require("scripts.util")
    require("scripts.questgen.util")
    require("scripts.questgen.planner")
    sb = {}
    sb.logInfo = function (...)
        print(string.format(...))
      end
  end
end

function generatePlan(planner, initialState, goalState)
  local co = coroutine.create(function () return planner:generatePlan(initialState, goalState) end)
  local status, result
  while coroutine.status(co) ~= "dead" do
    status, result = coroutine.resume(co)
    if not status then
      sb.logInfo("Planner broke: %s", debug.traceback(co, result))
      result = nil
    end
  end
  return result
end

function blocksWorld()
  local planner = Planner.new(20)
  planner.debug = true

  planner:setConstants({
    Table = "Table",
    A = "A",
    B = "B",
    C = "C"
  })

  function isBlock(entity)
    return entity == "A" or entity == "B" or entity == "C"
  end

  planner:addRelations({
    defineQueryRelation("Block", true) {
      [case(1, NonNil)] = function (self, entity)
          if xor(isBlock(entity), self.negated) then
            return {{entity}}
          else
            return Relation.empty
          end
        end,

      default = Relation.some
    },
    createRelation("Handempty"),
    createRelation("Clear"),
    defineRelation("On") {
      contradicts = function (self, condition)
          if self.negated then return false end
          return condition:containsTerm(self.new(false, {self.predicands[2], self.predicands[1]}, self.context))
        end
    },
    createRelation("Holding")
  })

  planner:addOperators({
    pickup_from_table = {
      preconditions = {
        {"Block", "b"},
        {"Handempty"},
        {"Clear", "b"},
        {"On", "b", "Table"}
      },
      postconditions = {
        {"Holding", "b"},
        {"!Handempty"},
        {"!On", "b", "Table"}
      }
    },
    pickup_from_block = {
      preconditions = {
        {"Block", "b"},
        {"Handempty"},
        {"Clear", "b"},
        {"On", "b", "c"},
        {"Block", "c"},
        {"!=", "b", "c"}
      },
      postconditions = {
        {"Holding", "b"},
        {"Clear", "c"},
        {"!Handempty"},
        {"!On", "b", "c"}
      }
    },
    putdown_on_table = {
      preconditions = {
        {"Block", "b"},
        {"Holding", "b"}
      },
      postconditions = {
        {"Handempty"},
        {"On", "b", "Table"},
        {"!Holding", "b"}
      }
    },
    putdown_on_block = {
      preconditions = {
        {"Block", "b"},
        {"Holding", "b"},
        {"Block", "c"},
        {"Clear", "c"},
        {"!=", "b", "c"}
      },
      postconditions = {
        {"Handempty"},
        {"On", "b", "c"},
        {"!Holding", "b"},
        {"!Clear", "c"}
      }
    }
  })

  local initialState = planner:parseConjunction({
    {"Handempty"},
    {"Clear", "A"},
    {"Clear", "C"},
    {"On", "A", "B"},
    {"On", "B", "Table"},
    {"On", "C", "Table"}
  })
  local goalState = planner:parseConjunction({
    {"On", "A", "B"},
    {"On", "B", "C"}
  })

  generatePlan(planner, initialState, goalState)
  planner:clearVariables()
end

function campfire()
  local planner = Planner.new(20)
  planner.debug = true

  planner:setConstants({
    Log = "Log"
  })

  planner:addRelations({
    createRelation("Holding"),
    createRelation("Handempty"),
    createRelation("AtWoods"),
    createRelation("AtCampfire"),
    createRelation("Warm")
  })

  planner:addOperators({
    collect = {
      preconditions = {
        {"AtWoods", "origCount", "x"},
        {"Handempty"},
        {"+", "newCount", 1, "origCount"}
      },
      postconditions = {
        {"!Handempty"},
        {"Holding", "x"},
        {"!AtWoods", "origCount", "x"},
        {"AtWoods", "newCount", "x"}
      }
    },
    drop = {
      preconditions = {
        {"Holding", "y"},
        {"AtCampfire", "origCount", "y"},
        {"+", "origCount", 1, "newCount"}
      },
      postconditions = {
        {"!AtCampfire", "origCount", "y"},
        {"AtCampfire", "newCount", "y"},
        {"!Holding", "y"},
        {"Handempty"}
      }
    },
    lightFire = {
      preconditions = {
        {"AtCampfire", "count", "Log"},
        {">=", "count", 2}
      },
      postconditions = {
        {"Warm"}
      }
    },
    resetCampfire = {
      preconditions = {},
      postconditions = {
        {"AtCampfire", 0, "y"}
      }
    }
  })

  local initialState = planner:parseConjunction({
    {"AtWoods", 2, "Log"},
    {"Handempty"}
  })
  local goalState = planner:parseConjunction({
    {"Warm"}
  })

  generatePlan(planner, initialState, goalState)
  planner:clearVariables()
end

function recipes()
  local planner = Planner.new(20)
  planner.debug = true

  planner:setConstants({
    Cake = "Cake",
    Pancakes = "Pancakes",
    Wheat = "Wheat",
    Egg = "Egg",
    Milk = "Milk",
    Sugar = "Sugar",
    Pearlpeas = "Pearlpeas",
    Eggshoot = "Eggshoot"
  })

  local recipes = {
    Cake = PrintableTable.new({
      Wheat = 3,
      Sugar = 1,
      Milk = 1,
      Egg = 2
    }),
    Pancakes = PrintableTable.new({
      Wheat = 3,
      Milk = 1,
      Egg = 1,
      Pearlpeas = 1,
      Eggshoot = 2
    })
  }

  local delicious = {
    Cake = true,
    Pancakes = true
  }

  local ingredients = {
    Wheat = true,
    Sugar = true,
    Milk = true,
    Egg = true,
    Pearlpeas = true,
    Eggshoot = true
  }

  planner:addRelations({
    defineQueryRelation("have") {
      [case(0, Any, 0)] = function (self)
          return {self.predicands}
        end,

      default = Relation.empty
    },

    defineQueryRelation("isRecipe", true) {
      [case(0, NonNil, NonNil)] = function (self, food, ingredients)
          if xor(self.negated, recipes[food] == ingredients) then
            return {{food, ingredients}}
          end
          return Relation.empty
        end,

      [case(1, NonNil, Nil)] = function (self, food)
          if xor(self.negated, recipes[food]) then
            return {{food, recipes[food]}}
          end
          return Relation.empty
        end,

      [case(2, Nil, Nil)] = function (self)
          if self.negated then return Relation.some end
          local results = {}
          for food, ingredients in pairs(recipes) do
            results[#results+1] = {food, ingredients}
          end
          return results
        end,

      [case(3, Nil, NonNil)] = Relation.some,
      default = Relation.empty
    },

    defineRelation("haveRecipe") {
      satisfiable = function (self)
          return true
        end,
      implications = function (self)
          return self:unpackPredicands {
            [case(0, NonNil, Any)] = function (self, ingredients, vars)
                local terms = {}

                if vars == nil or vars == Nil then
                  vars = PrintableTable.new({ old = PrintableTable.new(), new = PrintableTable.new()})
                  self.predicands[2]:bindToValue(vars)
                end

                function variable(table, ingredient)
                  if not table[ingredient] then
                    table[ingredient] = planner:newVariable(ingredient)
                  end
                  return table[ingredient]
                end

                function addTerm(term)
                  if term.static then
                    term:applyConstraints()
                  end
                  terms[#terms+1] = term
                end

                if not self.negated then
                  -- We're in the preconditions.
                  -- Create a variable for the counts of each ingredient we
                  -- currently have that is at least the amount needed by the
                  -- recipe. The variable used is stashed in the magic predicand
                  -- so that it can be constrained against later.
                  --
                  -- For each ingredient, the implications are:
                  --  have(ingredient, oldCount)
                  --  >=(oldCount, amountNeededByRecipe)
                  --  +(newCount, amountNeededByRecipe, oldCount)
                  for ingredient, amountNeeded in pairs(ingredients) do
                    local oldCount = variable(vars.old, ingredient)
                    local newCount = variable(vars.new, ingredient)
                    addTerm(planner.relations.have.new(false, {ingredient, oldCount}, self.context))
                    addTerm(planner.relations[">="].new(false, {oldCount, amountNeeded}, self.context))
                    addTerm(planner.relations["+"].new(false, {newCount, amountNeeded, oldCount}, self.context))
                  end

                else
                  -- We're in the postconditions, and the results of crafting
                  -- are being applied for each ingredient:
                  --  !have(ingredient, oldCount)
                  --  have(ingredient, newCount)
                  for ingredient, amountNeeded in pairs(ingredients) do
                    local oldCount = variable(vars.old, ingredient)
                    local newCount = variable(vars.new, ingredient)
                    addTerm(planner.relations.have.new(true, {ingredient, oldCount}, self.context))
                    addTerm(planner.relations.have.new(false, {ingredient, newCount}, self.context))
                  end

                end
                return Conjunction.new(terms)
              end,

            default = Conjunction.new()
          }
        end
    },

    defineQueryRelation("isDelicious", true) {
      [case(0, NonNil)] = function (self, food)
          if xor(self.negated, delicious[food]) then
            return {food}
          end
          return Relation.empty
        end,

      [case(1, Nil)] = function (self)
          if self.negated then return Relation.some end
          local results = {}
          for food,_ in pairs(delicious) do
            results[#results+1] = {food}
          end
          return results
        end,

      default = Relation.some
    },

    defineQueryRelation("isIngredient", true) {
      [case(0, NonNil)] = function (self, item)
          if xor(self.negated, ingredients[item]) then
            return {item}
          end
          return Relation.empty
        end,
      [case(1, Nil)] = function (self)
          if self.negated then return Relation.some end
          local results = {}
          for item,_ in pairs(ingredients) do
            results[#results+1] = {item}
          end
          return results
        end,

      default = Relation.some
    },

    createRelation("yum")
  })

  planner:addOperators({
    fetch = {
      preconditions = {
        {">=", "count", 1},
        {"isIngredient", "item"}
      },
      postconditions = {
        {"have", "item", "count"}
      },
      objectives = {
        {"have", "item", "count"}
      }
    },
    bake = {
      preconditions = {
        {"have", "food", "foodCount"},
        {"isRecipe", "food", "ingredients"},
        {"haveRecipe", "ingredients", "magic"},
        {"+", "foodCount", 1, "newFoodCount"},
        {">=", "foodCount", 0}
      },
      postconditions = {
        {"!haveRecipe", "ingredients", "magic"},
        {"have", "food", "newFoodCount"}
      },
      objectives = {
        {"have", "food", "newFoodCount"}
      }
    },
    eat = {
      preconditions = {
        {"have", "foodA", "countA"},
        {"isDelicious", "foodA"},
        {"+", "newCountA", 1, "countA"},
        {">=", "countA", 1},
        {"have", "foodB", "countB"},
        {"isDelicious", "foodB"},
        {"+", "newCountB", 1, "countB"},
        {">=", "countB", 1},
        {"!=", "foodA", "foodB"}
      },
      postconditions = {
        {"!have", "foodA", "countA"},
        {"have", "foodA", "newCountA"},
        {"!have", "foodB", "countB"},
        {"have", "foodB", "newCountB"},
        {"yum"}
      },
      objectives = {
        {"yum"}
      }
    }
  })

  local initialState = planner:parseConjunction({})
  local goalState = planner:parseConjunction({
    {"yum"}
  })

  generatePlan(planner, initialState, goalState)
  planner:clearVariables()
end
