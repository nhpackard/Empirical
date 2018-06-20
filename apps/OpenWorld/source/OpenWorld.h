/// This is the world for OpenOrgs

#ifndef OPEN_WORLD_H
#define OPEN_WORLD_H

#include "Evolve/World.h"
#include "geometry/Surface.h"
#include "hardware/signalgp_utils.h"
#include "tools/math.h"

#include "config.h"
#include "OpenOrg.h"

class OpenWorld : public emp::World<OpenOrg> {
private:
  static constexpr size_t TAG_WIDTH = 16;
  using hardware_t = OpenOrg::hardware_t;
  using program_t = hardware_t::Program;
  using prog_fun_t = hardware_t::Function;
  using prog_tag_t = hardware_t::affinity_t;
  using event_lib_t = hardware_t::event_lib_t;
  using inst_t = hardware_t::inst_t;
  using inst_lib_t = hardware_t::inst_lib_t;
  using hw_state_t = hardware_t::State;
 
  using surface_t = emp::Surface<OpenOrg>;
  using mutator_t = emp::SignalGPMutator<TAG_WIDTH>;

  OpenWorldConfig & config;
  inst_lib_t inst_lib;
  event_lib_t event_lib;
  surface_t surface;
  size_t next_id;

  mutator_t signalgp_mutator;

  // double pop_pressure = 1.0;  // How much pressure before an organism dies? 

  std::unordered_map<size_t, emp::Ptr<OpenOrg>> id_map;


public:  
  OpenWorld(OpenWorldConfig & _config)
    : config(_config), inst_lib(), event_lib(),
      surface({config.WORLD_X(), config.WORLD_Y()}),
      next_id(1), signalgp_mutator(), id_map()
  {
    SetPopStruct_Grow(false); // Don't automatically delete organism when new ones are born.

    // Make sure that we are tracking organisms by their IDs.
    OnPlacement( [this](size_t pos){
      size_t id = next_id++;
      GetOrg(pos).GetBrain().SetTrait((size_t)OpenOrg::Trait::ORG_ID, id);
      surface.AddBody(&GetOrg(pos));
      id_map[id] = &GetOrg(pos);
    });
    OnOrgDeath( [this](size_t pos){ id_map.erase( GetOrg(pos).GetID() ); } );

    signalgp_mutator.SetProgMinFuncCnt(config.PROGRAM_MIN_FUN_CNT());
    signalgp_mutator.SetProgMaxFuncCnt(config.PROGRAM_MAX_FUN_CNT());
    signalgp_mutator.SetProgMinFuncLen(config.PROGRAM_MIN_FUN_LEN());
    signalgp_mutator.SetProgMaxFuncLen(config.PROGRAM_MAX_FUN_LEN());
    signalgp_mutator.SetProgMinArgVal(config.PROGRAM_MIN_ARG_VAL());
    signalgp_mutator.SetProgMaxArgVal(config.PROGRAM_MAX_ARG_VAL());
    signalgp_mutator.SetProgMaxTotalLen(config.PROGRAM_MAX_FUN_CNT() * config.PROGRAM_MAX_FUN_LEN());

    signalgp_mutator.ARG_SUB__PER_ARG(config.ARG_SUB__PER_ARG());
    signalgp_mutator.INST_SUB__PER_INST(config.INST_SUB__PER_INST());
    signalgp_mutator.INST_INS__PER_INST(config.INST_INS__PER_INST());
    signalgp_mutator.INST_DEL__PER_INST(config.INST_DEL__PER_INST());
    signalgp_mutator.SLIP__PER_FUNC(config.SLIP__PER_FUNC());
    signalgp_mutator.FUNC_DUP__PER_FUNC(config.FUNC_DUP__PER_FUNC());
    signalgp_mutator.FUNC_DEL__PER_FUNC(config.FUNC_DEL__PER_FUNC());
    signalgp_mutator.TAG_BIT_FLIP__PER_BIT(config.TAG_BIT_FLIP__PER_BIT());

    // Setup the default instruction set.
    inst_lib.AddInst("Inc", hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
    inst_lib.AddInst("Dec", hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
    inst_lib.AddInst("Not", hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
    inst_lib.AddInst("Add", hardware_t::Inst_Add, 3, "Local memory: Arg3 = Arg1 + Arg2");
    inst_lib.AddInst("Sub", hardware_t::Inst_Sub, 3, "Local memory: Arg3 = Arg1 - Arg2");
    inst_lib.AddInst("Mult", hardware_t::Inst_Mult, 3, "Local memory: Arg3 = Arg1 * Arg2");
    inst_lib.AddInst("Div", hardware_t::Inst_Div, 3, "Local memory: Arg3 = Arg1 / Arg2");
    inst_lib.AddInst("Mod", hardware_t::Inst_Mod, 3, "Local memory: Arg3 = Arg1 % Arg2");
    inst_lib.AddInst("TestEqu", hardware_t::Inst_TestEqu, 3, "Local memory: Arg3 = (Arg1 == Arg2)");
    inst_lib.AddInst("TestNEqu", hardware_t::Inst_TestNEqu, 3, "Local memory: Arg3 = (Arg1 != Arg2)");
    inst_lib.AddInst("TestLess", hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
    inst_lib.AddInst("Call", hardware_t::Inst_Call, 0, "Call function that best matches call affinity.");
    inst_lib.AddInst("Return", hardware_t::Inst_Return, 0, "Return from current function if possible.");
    inst_lib.AddInst("SetMem", hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
    inst_lib.AddInst("CopyMem", hardware_t::Inst_CopyMem, 2, "Local memory: Arg1 = Arg2");
    inst_lib.AddInst("SwapMem", hardware_t::Inst_SwapMem, 2, "Local memory: Swap values of Arg1 and Arg2.");
    inst_lib.AddInst("Input", hardware_t::Inst_Input, 2, "Input memory Arg1 => Local memory Arg2.");
    inst_lib.AddInst("Output", hardware_t::Inst_Output, 2, "Local memory Arg1 => Output memory Arg2.");
    inst_lib.AddInst("Commit", hardware_t::Inst_Commit, 2, "Local memory Arg1 => Shared memory Arg2.");
    inst_lib.AddInst("Pull", hardware_t::Inst_Pull, 2, "Shared memory Arg1 => Shared memory Arg2.");
    inst_lib.AddInst("Nop", hardware_t::Inst_Nop, 0, "No operation.");
    inst_lib.AddInst("Fork", hardware_t::Inst_Fork, 0, "Fork a new thread. Local memory contents of callee are loaded into forked thread's input memory.");
    inst_lib.AddInst("Terminate", hardware_t::Inst_Terminate, 0, "Kill current thread.");
    // These next five instructions are 'block'-modifying instructions: they facilitate within-function flow control. 
    // The {"block_def"} property tells the SignalGP virtual hardware that this instruction defines a new 'execution block'. 
    // The {"block_close"} property tells the SignalGP virtual hardware that this instruction exits the current 'execution block'. 
    inst_lib.AddInst("If", hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
    inst_lib.AddInst("While", hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
    inst_lib.AddInst("Countdown", hardware_t::Inst_Countdown, 1, "Local memory: Countdown Arg1 to zero.", emp::ScopeType::BASIC, 0, {"block_def"});
    inst_lib.AddInst("Close", hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
    inst_lib.AddInst("Break", hardware_t::Inst_Break, 0, "Break out of current block.");

    // Setup new instructions for the instruction set.
    inst_lib.AddInst("Vroom", [this](hardware_t & hw, const inst_t & inst) {
      const size_t id = (size_t) hw.GetTrait((size_t) OpenOrg::Trait::ORG_ID);
      emp::Ptr<OpenOrg> org_ptr = id_map[id];
      emp::Angle facing = org_ptr->GetFacing();
      org_ptr->Translate( facing.GetPoint(1.0) );
    }, 1, "Move forward.");

    inst_lib.AddInst("SpinRight", [this](hardware_t & hw, const inst_t & inst) mutable {
      const size_t id = (size_t) hw.GetTrait((size_t) OpenOrg::Trait::ORG_ID);
      emp::Ptr<OpenOrg> org_ptr = id_map[id];
      org_ptr->RotateDegrees(-5.0);
    }, 1, "Rotate -5 degrees.");

    inst_lib.AddInst("SpinLeft", [this](hardware_t & hw, const inst_t & inst) mutable {
      const size_t id = (size_t) hw.GetTrait((size_t) OpenOrg::Trait::ORG_ID);
      emp::Ptr<OpenOrg> org_ptr = id_map[id];
      org_ptr->RotateDegrees(5.0);
    }, 1, "Rotate 5 degrees.");

    // On each update, run organisms and make sure they stay on the surface.
    OnUpdate([this](size_t){
      // Process all organisms.
      Process(5);

      // Update each organism.
      for (size_t pos = 0; pos < pop.size(); pos++) {
        if (pop[pos].IsNull()) continue;
        auto & org = *pop[pos];

        // Make sure organisms are on the surface (wrap around if not)
        double x = org.GetCenter().GetX();
        double y = org.GetCenter().GetY();
        if (x < 0.0) x += config.WORLD_X();
        if (y < 0.0) y += config.WORLD_Y();
        if (x >= config.WORLD_X()) x -= config.WORLD_X();
        if (y >= config.WORLD_Y()) y -= config.WORLD_Y();
        org.SetCenter({x,y});

        // Provide additional resources toward reproduction.
        org.AdjustEnergy( random_ptr->GetDouble(0.1) );

        // If an organism has enough energy to reproduce, do so.
        if (org.GetEnergy() > org.GetMass()) {
          // Remove energy for building offspring; cut rest in half, so it is effectively
          // split between parent and child when copied into child.
          org.SetEnergy((org.GetEnergy() - org.GetMass() / 2.0));
          DoBirth(org, pos);
//          emp::Alert("Birth!");
        }
      }
    });

    // Setup a mutation function.
    SetMutFun( [this](OpenOrg & org, emp::Random & random){
      signalgp_mutator.ApplyMutations(org.GetBrain().GetProgram(), random);
      org.SetRadius(org.GetRadius() * emp::Pow2(random.GetDouble(-0.1, 0.1)));
      return 1;
    });

    // Initialize a populaton of random organisms.
    Inject(OpenOrg(inst_lib, event_lib, random_ptr), config.INIT_POP_SIZE());
    for (size_t i = 0; i < config.INIT_POP_SIZE(); i++) {
      double x = random_ptr->GetDouble(config.WORLD_X());
      double y = random_ptr->GetDouble(config.WORLD_Y());
      GetOrg(i).SetCenter({x,y});
      surface.AddBody(&GetOrg(i));
      GetOrg(i).GetBrain().SetProgram(emp::GenRandSignalGPProgram(*random_ptr, inst_lib, config.PROGRAM_MIN_FUN_CNT(), config.PROGRAM_MAX_FUN_CNT(), config.PROGRAM_MIN_FUN_LEN(), config.PROGRAM_MAX_FUN_LEN(), config.PROGRAM_MIN_ARG_VAL(), config.PROGRAM_MAX_ARG_VAL()));
    }
  }
  ~OpenWorld() { ; }

  surface_t & GetSurface() { return surface; }

  /// Test if two bodies have collided and act accordingly if they have.
  bool TestPairCollision(OpenOrg & body1, OpenOrg & body2) {
    // if (body1.IsLinked(body2)) return false;  // Linked bodies can overlap.

    const emp::Point dist = body1.GetCenter() - body2.GetCenter();
    const double sq_pair_dist = dist.SquareMagnitude();
    const double radius_sum = body1.GetRadius() + body2.GetRadius();
    const double sq_min_dist = radius_sum * radius_sum;

    // If there was no collision, return false.
    if (sq_pair_dist >= sq_min_dist) { return false; }

    // If we made it this far, there was a collision!

    return true;
  }
};

#endif
