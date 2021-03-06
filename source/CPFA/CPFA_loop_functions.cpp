#include "CPFA_loop_functions.h"

CPFA_loop_functions::CPFA_loop_functions() :
	RNG(argos::CRandom::CreateRNG("argos")),
	MaxSimTime(3600 * GetSimulator().GetPhysicsEngine("dyn2d").GetInverseSimulationClockTick()),
	ResourceDensityDelay(0),
	RandomSeed(GetSimulator().GetRandomSeed()),
	SimCounter(0),
	MaxSimCounter(1),
	VariableFoodPlacement(0),
	OutputData(0),
	DrawDensityRate(4),
	DrawIDs(1),
	DrawTrails(1),
	DrawTargetRays(1),
	FoodDistribution(2),
	FoodPDF(0),
	FoodItemCount(256),
	NumberOfClusters(4),
	ClusterWidthX(8),
	ClusterLengthY(8),
	PowerRank(4),
	ProbabilityOfSwitchingToSearching(0.0),
	ProbabilityOfReturningToNest(0.0),
	UninformedSearchVariation(0.0),
	RateOfInformedSearchDecay(0.0),
	RateOfSiteFidelity(0.0),
	RateOfLayingPheromone(0.0),
	RateOfPheromoneDecay(0.0),
	FoodRadius(0.05),
	FoodRadiusSquared(0.0025),
	NestRadius(0.25),
	NestRadiusSquared(0.0625),
	NestElevation(0.01),
	// We are looking at a 4 by 4 square (3 targets + 2*1/2 target gaps)
	SearchRadiusSquared((4.0 * FoodRadius) * (4.0 * FoodRadius)),
	NumDistributedFood(0),
	score(0),
	PrintFinalScore(0)
{}

void CPFA_loop_functions::Init(argos::TConfigurationNode &node) {	
	argos::CDegrees USV_InDegrees;
	argos::TConfigurationNode CPFA_node = argos::GetNode(node, "CPFA");

	argos::GetNodeAttribute(CPFA_node, "ProbabilityOfSwitchingToSearching", ProbabilityOfSwitchingToSearching);
	argos::GetNodeAttribute(CPFA_node, "ProbabilityOfReturningToNest",      ProbabilityOfReturningToNest);
	argos::GetNodeAttribute(CPFA_node, "UninformedSearchVariation",         USV_InDegrees);
	argos::GetNodeAttribute(CPFA_node, "RateOfInformedSearchDecay",         RateOfInformedSearchDecay);
	argos::GetNodeAttribute(CPFA_node, "RateOfSiteFidelity",                RateOfSiteFidelity);
	argos::GetNodeAttribute(CPFA_node, "RateOfLayingPheromone",             RateOfLayingPheromone);
	argos::GetNodeAttribute(CPFA_node, "RateOfPheromoneDecay",              RateOfPheromoneDecay);
	argos::GetNodeAttribute(CPFA_node, "PrintFinalScore",                   PrintFinalScore);

	UninformedSearchVariation = ToRadians(USV_InDegrees);
	argos::TConfigurationNode settings_node = argos::GetNode(node, "settings");

	argos::GetNodeAttribute(settings_node, "MaxSimTimeInSeconds", MaxSimTime);

	MaxSimTime *= GetSimulator().GetPhysicsEngine("dyn2d").GetInverseSimulationClockTick();

	argos::GetNodeAttribute(settings_node, "MaxSimCounter", MaxSimCounter);
	argos::GetNodeAttribute(settings_node, "VariableFoodPlacement", VariableFoodPlacement);
	argos::GetNodeAttribute(settings_node, "OutputData", OutputData);
	argos::GetNodeAttribute(settings_node, "DrawIDs", DrawIDs);
	argos::GetNodeAttribute(settings_node, "DrawTrails", DrawTrails);
	argos::GetNodeAttribute(settings_node, "DrawTargetRays", DrawTargetRays);
	argos::GetNodeAttribute(settings_node, "FoodDistribution", FoodDistribution);
	argos::GetNodeAttribute(settings_node, "FoodPDF", FoodPDF);
	argos::GetNodeAttribute(settings_node, "FoodItemCount", FoodItemCount);
	argos::GetNodeAttribute(settings_node, "NumberOfClusters", NumberOfClusters);
	argos::GetNodeAttribute(settings_node, "ClusterWidthX", ClusterWidthX);
	argos::GetNodeAttribute(settings_node, "ClusterLengthY", ClusterLengthY);
	argos::GetNodeAttribute(settings_node, "FoodRadius", FoodRadius);
	argos::GetNodeAttribute(settings_node, "NestElevation", NestElevation);

	FoodRadiusSquared = FoodRadius*FoodRadius;

    //Number of distributed foods
    if (FoodDistribution == 1){
        NumDistributedFood = ClusterWidthX*ClusterLengthY*NumberOfClusters;
    }
    else
    NumDistributedFood = FoodItemCount;  
	// calculate the forage range and compensate for the robot's radius of 0.085m
	argos::CVector3 ArenaSize = GetSpace().GetArenaSize();
	argos::Real rangeX = (ArenaSize.GetX() / 2.0) - 0.085;
	argos::Real rangeY = (ArenaSize.GetY() / 2.0) - 0.085;
	ForageRangeX.Set(-rangeX, rangeX);
	ForageRangeY.Set(-rangeY, rangeY);

	// Send a pointer to this loop functions object to each controller.
	argos::CSpace::TMapPerType& footbots = GetSpace().GetEntitiesByType("foot-bot");
	argos::CSpace::TMapPerType::iterator it;

    Num_robots = footbots.size();
	for(it = footbots.begin(); it != footbots.end(); it++) {
		argos::CFootBotEntity& footBot = *argos::any_cast<argos::CFootBotEntity*>(it->second);
		BaseController& c = dynamic_cast<BaseController&>(footBot.GetControllableEntity().GetController());
		CPFA_controller& c2 = dynamic_cast<CPFA_controller&>(c);

		c2.SetLoopFunctions(this);
	}

	SetFoodDistribution();
    ForageList.clear(); 

  
  // output headers
  /*
	ofstream log_output_stream;
  std::string fname = "clusters_";
  fname += std::to_string(RandomSeed);
  fname += ".csv";
	log_output_stream.open(fname, ios::out | ios::trunc);
  log_output_stream << "time";
  for(int i = 1; i <= (NumberOfClusters * 4); ++i) {
    log_output_stream << ",c" << i;
  }
  log_output_stream << "\n";
	log_output_stream.close();

  fname = "posistions_";
  fname += std::to_string(RandomSeed);
  fname += ".csv";
	log_output_stream.open(fname, ios::out | ios::trunc);
  log_output_stream << "time,robot,x,y" << std::endl;
	log_output_stream.close();
  */

	ofstream log_output_stream;
  std::string fname = "targets_";
  fname += std::to_string(RandomSeed);
  fname += ".csv";
	log_output_stream.open(fname, ios::out | ios::trunc);
  log_output_stream << "time,x,y" << std::endl;
	log_output_stream.close();
}

void CPFA_loop_functions::Reset() {
	if(VariableFoodPlacement == 0) {
		RNG->Reset();
	}

	GetSpace().Reset();
	GetSpace().GetFloorEntity().Reset();
	MaxSimCounter = SimCounter;
	SimCounter = 0;
  score = 0.0;

	FoodList.clear();
  SectorList.clear();
	FoodColoringList.clear();
	PheromoneList.clear();
	FidelityList.clear();
	TargetRayList.clear();

	SetFoodDistribution();
	argos::CSpace::TMapPerType& footbots = GetSpace().GetEntitiesByType("foot-bot");
	argos::CSpace::TMapPerType::iterator it;

	for(it = footbots.begin(); it != footbots.end(); it++) {
		argos::CFootBotEntity& footBot = *argos::any_cast<argos::CFootBotEntity*>(it->second);
		BaseController& c = dynamic_cast<BaseController&>(footBot.GetControllableEntity().GetController());
		CPFA_controller& c2 = dynamic_cast<CPFA_controller&>(c);

		MoveEntity(footBot.GetEmbodiedEntity(), c2.GetStartPosition(), argos::CQuaternion(), false);
    c2.Reset();
	}
}

void CPFA_loop_functions::PreStep() {
	UpdatePheromoneList();

	if(GetSpace().GetSimulationClock() > ResourceDensityDelay) {
		for(size_t i = 0; i < FoodColoringList.size(); i++) {
			FoodColoringList[i] = argos::CColor::BLACK;
		}
	}

	if(FoodList.size() == 0) {
		FidelityList.clear();
		TargetRayList.clear();
		PheromoneList.clear();
	}
}

void CPFA_loop_functions::PostStep() {
	// nothing... yet...

  /*
	GetSpace().GetEntitiesByType("foot-bot").size();
	ofstream log_output_stream;
  std::string fname = "positions_";
  fname += std::to_string(RandomSeed);
  fname += ".csv";
	log_output_stream.open(fname, ios::app);
  for(auto& bot : GetSpace().GetEntitiesByType("foot-bot")) {
    CFootBotEntity* b = any_cast<CFootBotEntity*>(bot.second);
    argos::CVector3 pos = b->GetEmbodiedEntity().GetOriginAnchor().Position;
    log_output_stream << GetSpace().GetSimulationClock()
      << "," << b->GetControllableEntity().GetController().GetId() << ","
      << pos.GetX()
      << ","
      << pos.GetY()
      << std::endl;
  }
	log_output_stream.close();


  fname = "clusters_";
  fname += std::to_string(RandomSeed);
  fname += ".csv";
	log_output_stream.open(fname, ios::app);
  log_output_stream << GetSpace().GetSimulationClock();
  for(int i = 1; i <= (NumberOfClusters * 4); ++i) {
    log_output_stream << "," << cluster_association_counts[i];
  }
  log_output_stream << "\n";
	log_output_stream.close();
  */

  /*
  // now maybe mess with clusters every so often
  if (GetSpace().GetSimulationClock() % 50 == 0) {
    argos::Real randomNumber;
    // first consider eliminating each cluster independently
    int num_active = active_cluster_ids.size();
    for (int i = 0; i < num_active;) {
      randomNumber = RNG->Uniform(argos::CRange<argos::Real>(0.0, 1.0));
      if (randomNumber < 0.001) {
        // delete cluster
        for (int j = 0; j < FoodList.size();) {
          if (FoodList[j].GetClusterID() == active_cluster_ids[i]) {
            FoodList.erase(FoodList.begin() + j);
            FoodColoringList.erase(FoodColoringList.begin() + j);
          } else {
            ++j;
          }
        }

        argos::LOG << "[" << GetSpace().GetSimulationClock()
          << "] Deleting cluster "
          << active_cluster_ids[i] << " ["
          << active_cluster_ids.size() - 1 << "/"
          << NumberOfClusters << "]\n";

        inactive_cluster_ids.push_back(active_cluster_ids[i]);
        active_cluster_ids.erase(active_cluster_ids.begin() + i);
        num_active--;
      } else {
        ++i;
      }
    }

    // next consider creating a cluster
    if (active_cluster_ids.size() < NumberOfClusters) {
      randomNumber = RNG->Uniform(argos::CRange<argos::Real>(0.0, 1.0));
      argos::CVector2 placementPosition;
      SmartFood food_item;
      argos::Real foodOffset  = 3.0 * FoodRadius;
      if (randomNumber < 0.01) {
        // create cluster
        size_t new_cluster_id = inactive_cluster_ids[0];

        argos::LOG << "[" << GetSpace().GetSimulationClock()
          << "] Creating cluster "
          << new_cluster_id << " [" << active_cluster_ids.size() + 1
          << "/" << NumberOfClusters << "]\n";

        active_cluster_ids.push_back(inactive_cluster_ids[0]);
        inactive_cluster_ids.erase(inactive_cluster_ids.begin());
        placementPosition.Set(RNG->Uniform(ForageRangeX), RNG->Uniform(ForageRangeY));

        while(IsOutOfBounds(placementPosition, ClusterLengthY, ClusterWidthX)) {
          placementPosition.Set(RNG->Uniform(ForageRangeX), RNG->Uniform(ForageRangeY));
        }

        for(size_t j = 0; j < ClusterLengthY; j++) {
          for(size_t k = 0; k < ClusterWidthX; k++) {
            // we reserve cluster ID 0 for being an idependent agent
            food_item = SmartFood(new_cluster_id, placementPosition);
            FoodList.push_back(food_item);
            FoodColoringList.push_back(argos::CColor::BLACK);
            placementPosition.SetX(placementPosition.GetX() + foodOffset);
          }

          placementPosition.SetX(placementPosition.GetX() - (ClusterWidthX * foodOffset));
          placementPosition.SetY(placementPosition.GetY() + foodOffset);
        }
      }
    }
  }
  */
}

bool CPFA_loop_functions::IsExperimentFinished() {
	bool isFinished = false;

	if(FoodList.size() == 0 || GetSpace().GetSimulationClock() >= MaxSimTime) {
		isFinished = true;
	}

	if(isFinished == true && MaxSimCounter > 1) {
		size_t newSimCounter = SimCounter + 1;
		size_t newMaxSimCounter = MaxSimCounter - 1;

		PostExperiment();
		Reset();

		SimCounter    = newSimCounter;
		MaxSimCounter = newMaxSimCounter;
		isFinished    = false;
	}

	return isFinished;
}

void CPFA_loop_functions::PostExperiment() {
	//if (PrintFinalScore == 1) printf("%f, %f\n", getSimTimeInSeconds(), score);
}

argos::CColor CPFA_loop_functions::GetFloorColor(const argos::CVector2 &c_pos_on_floor) {
	return argos::CColor::WHITE;
}

void CPFA_loop_functions::UpdatePheromoneList() {
	// Return if this is not a tick that lands on a 0.5 second interval
	if ((int)(GetSpace().GetSimulationClock()) % ((int)(GetSimulator().GetPhysicsEngine("dyn2d").GetInverseSimulationClockTick()) / 2) != 0) return;
	
	std::vector<Pheromone> new_p_list; 

	argos::Real t = GetSpace().GetSimulationClock() / GetSimulator().GetPhysicsEngine("dyn2d").GetInverseSimulationClockTick();

	//ofstream log_output_stream;
	//log_output_stream.open("time.txt", ios::app);
	//log_output_stream << t << ", " << GetSpace().GetSimulationClock() << ", " << GetSimulator().GetPhysicsEngine("default").GetInverseSimulationClockTick() << endl;
	//log_output_stream.close();

	for(size_t i = 0; i < PheromoneList.size(); i++) {

		PheromoneList[i].Update(t);

		if(PheromoneList[i].IsActive() == true) {
			new_p_list.push_back(PheromoneList[i]);
		}
	}

	PheromoneList = new_p_list;
}

void CPFA_loop_functions::SetFoodDistribution() {
	switch(FoodDistribution) {
		case 0:
			RandomFoodDistribution();
			break;
		case 1:
      if (FoodPDF == 0) {
        ClusterFoodDistribution();
      } else {
        ClusterFoodDistributionPDF();
      }
			break;
		case 2:
			PowerLawFoodDistribution();
			break;
		default:
			argos::LOGERR << "ERROR: Invalid food distribution in XML file.\n";
	}
}

void CPFA_loop_functions::ClusterFoodDistributionPDF() {
  FoodList.clear();
  FoodColoringList.clear();
  SectorList.clear();
	argos::Real     sectorOffset  = 1;
	size_t          foodToPlace = NumberOfClusters * ClusterWidthX * ClusterLengthY;
	size_t          foodPlaced = 0;
	argos::CVector2 placementPosition;
  SectorFood food_sector;
  SmartFood food_item;

	FoodItemCount = foodToPlace;

  argos::Real seed_prob = 0.6;

  // we reserve cluster 0, so just say nobody's in it
  // We also use more than the total number of clusters so that we increase the
  // downtime for a cluster ID when one is destroyed
  cluster_association_counts.push_back((size_t)getNumberOfRobots());
  for (size_t i = 0; i < (NumberOfClusters * 4); ++i) {
    cluster_association_counts.push_back(0);
    if (i < NumberOfClusters) {
      active_cluster_ids.push_back(i+1); // start with first group active
    } else {
      inactive_cluster_ids.push_back(i+1); // rest inactive
    }
  }

	for (size_t i = 0; i < NumberOfClusters; i++) {
		placementPosition.Set(RNG->Uniform(ForageRangeX), RNG->Uniform(ForageRangeY));

		while(IsOutOfBounds(placementPosition, ClusterLengthY, ClusterWidthX)) {
			placementPosition.Set(RNG->Uniform(ForageRangeX), RNG->Uniform(ForageRangeY));
		}

		for(size_t j = 0; j < ClusterLengthY; j++) {
			for(size_t k = 0; k < ClusterWidthX; k++) {
				foodPlaced++;

        argos::CVector2 lleft(std::floor(placementPosition.GetX()),
            std::floor(placementPosition.GetY()));
        argos::CVector2 uright(std::floor(placementPosition.GetX()) + 1,
            std::floor(placementPosition.GetY()) + 1);

        argos::Real mx, my;
        mx = (lleft.GetX() + uright.GetX()) / 2.0;
        my = (lleft.GetY() + uright.GetY()) / 2.0;
        argos::CVector2 mid(mx, my);

        food_sector = SectorFood(lleft, uright, seed_prob);
				SectorList.push_back(food_sector);

        food_item = SmartFood(FoodRadius, mid);
        FoodList.push_back(food_item);
				FoodColoringList.push_back(argos::CColor::BLACK);
				placementPosition.SetX(placementPosition.GetX() + sectorOffset);
			}

			placementPosition.SetX(placementPosition.GetX() - (ClusterWidthX * sectorOffset));
			placementPosition.SetY(placementPosition.GetY() + sectorOffset);
		}
	}
}

void CPFA_loop_functions::RandomFoodDistribution() {
	FoodList.clear();

  SmartFood food_item;

	argos::CVector2 placementPosition;

	for(size_t i = 0; i < FoodItemCount; i++) {
		placementPosition.Set(RNG->Uniform(ForageRangeX), RNG->Uniform(ForageRangeY));

		while(IsOutOfBounds(placementPosition, 1, 1)) {
			placementPosition.Set(RNG->Uniform(ForageRangeX), RNG->Uniform(ForageRangeY));
		}

    food_item = SmartFood(FoodRadius, placementPosition, i+1);
		FoodList.push_back(food_item);
		FoodColoringList.push_back(argos::CColor::BLACK);
	}
}

void CPFA_loop_functions::ClusterFoodDistribution() {
        FoodList.clear();
	argos::Real     foodOffset  = 3.0 * FoodRadius;
	size_t          foodToPlace = NumberOfClusters * ClusterWidthX * ClusterLengthY;
	size_t          foodPlaced = 0;
	argos::CVector2 placementPosition;
  SmartFood food_item;

	FoodItemCount = foodToPlace;

  // we reserve cluster 0, so just say nobody's in it
  // We also use more than the total number of clusters so that we increase the
  // downtime for a cluster ID when one is destroyed
  cluster_association_counts.push_back((size_t)getNumberOfRobots());
  for (size_t i = 0; i < (NumberOfClusters * 4); ++i) {
    cluster_association_counts.push_back(0);
    if (i < NumberOfClusters) {
      active_cluster_ids.push_back(i+1); // start with first group active
    } else {
      inactive_cluster_ids.push_back(i+1); // rest inactive
    }
  }

	for (size_t i = 0; i < NumberOfClusters; i++) {
		placementPosition.Set(RNG->Uniform(ForageRangeX), RNG->Uniform(ForageRangeY));

		while(IsOutOfBounds(placementPosition, ClusterLengthY, ClusterWidthX)) {
			placementPosition.Set(RNG->Uniform(ForageRangeX), RNG->Uniform(ForageRangeY));
		}

		for(size_t j = 0; j < ClusterLengthY; j++) {
			for(size_t k = 0; k < ClusterWidthX; k++) {
				foodPlaced++;
				/*
				#include <argos3/plugins/simulator/entities/box_entity.h>

				string label("my_box_");
				label.push_back('0' + foodPlaced++);

				CBoxEntity *b = new CBoxEntity(label,
					CVector3(placementPosition.GetX(),
					placementPosition.GetY(), 0.0), CQuaternion(), true,
					CVector3(0.1, 0.1, 0.001), 1.0);
				AddEntity(*b);
				*/

        // we reserve cluster ID 0 for being an idependent agent
        food_item = SmartFood(FoodRadius, placementPosition, active_cluster_ids[i]);
				FoodList.push_back(food_item);
				FoodColoringList.push_back(argos::CColor::BLACK);
				placementPosition.SetX(placementPosition.GetX() + foodOffset);
			}

			placementPosition.SetX(placementPosition.GetX() - (ClusterWidthX * foodOffset));
			placementPosition.SetY(placementPosition.GetY() + foodOffset);
		}
	}
}

void CPFA_loop_functions::PowerLawFoodDistribution() {
 FoodList.clear();

  SmartFood food_item;

	argos::Real foodOffset     = 3.0 * FoodRadius;
	size_t      foodPlaced     = 0;
	size_t      powerLawLength = 1;
	size_t      maxTrials      = 200;
	size_t      trialCount     = 0;

	std::vector<size_t> powerLawClusters;
	std::vector<size_t> clusterSides;
	argos::CVector2     placementPosition;

    //-----Wayne: Dertermine PowerRank and food per PowerRank group
    size_t priorPowerRank = 0;
    size_t power4 = 0;
    size_t FoodCount = 0;
    size_t diffFoodCount = 0;
    size_t singleClusterCount = 0;
    size_t otherClusterCount = 0;
    size_t modDiff = 0;
    
    //Wayne: priorPowerRank is determined by what power of 4
    //plus a multiple of power4 increases the food count passed required count
    //this is how powerlaw works to divide up food into groups
    //the number of groups is the powerrank
    while (FoodCount < FoodItemCount){
        priorPowerRank++;
        power4 = pow (4.0, priorPowerRank);
        FoodCount = power4 + priorPowerRank * power4;
    }
    
    //Wayne: Actual powerRank is prior + 1
    PowerRank = priorPowerRank + 1;
    
    //Wayne: Equalizes out the amount of food in each group, with the 1 cluster group taking the
    //largest loss if not equal, when the powerrank is not a perfect fit with the amount of food.
    diffFoodCount = FoodCount - FoodItemCount;
    modDiff = diffFoodCount % PowerRank;
    
    if (FoodItemCount % PowerRank == 0){
        singleClusterCount = FoodItemCount / PowerRank;
        otherClusterCount = singleClusterCount;
    }
    else {
        otherClusterCount = FoodItemCount / PowerRank + 1;
        singleClusterCount = otherClusterCount - modDiff;
    }
    //-----Wayne: End of PowerRank and food per PowerRank group
    
	for(size_t i = 0; i < PowerRank; i++) {
		powerLawClusters.push_back(powerLawLength * powerLawLength);
		powerLawLength *= 2;
	}

	for(size_t i = 0; i < PowerRank; i++) {
		powerLawLength /= 2;
		clusterSides.push_back(powerLawLength);
	}

    /*Wayne: Modified to break from loops if food count reached.
     Provides support for unequal clusters and odd food numbers.
     Necessary for DustUp and Jumble Distribution changes. */
	for(size_t h = 0; h < powerLawClusters.size(); h++) {
		for(size_t i = 0; i < powerLawClusters[h]; i++) {
			placementPosition.Set(RNG->Uniform(ForageRangeX), RNG->Uniform(ForageRangeY));

			while(IsOutOfBounds(placementPosition, clusterSides[h], clusterSides[h])) {
				trialCount++;
				placementPosition.Set(RNG->Uniform(ForageRangeX), RNG->Uniform(ForageRangeY));

				if(trialCount > maxTrials) {
					argos::LOGERR << "PowerLawDistribution(): Max trials exceeded!\n";
					break;
				}
			}

            trialCount = 0;
			for(size_t j = 0; j < clusterSides[h]; j++) {
				for(size_t k = 0; k < clusterSides[h]; k++) {
					foodPlaced++;
          food_item = SmartFood(FoodRadius, placementPosition); // TODO: cluster ID
					FoodList.push_back(food_item);
					FoodColoringList.push_back(argos::CColor::BLACK);
					placementPosition.SetX(placementPosition.GetX() + foodOffset);
                    if (foodPlaced == singleClusterCount + h * otherClusterCount) break;
				}

				placementPosition.SetX(placementPosition.GetX() - (clusterSides[h] * foodOffset));
				placementPosition.SetY(placementPosition.GetY() + foodOffset);
                if (foodPlaced == singleClusterCount + h * otherClusterCount) break;
			}
            if (foodPlaced == singleClusterCount + h * otherClusterCount) break;
		}
	}

	FoodItemCount = foodPlaced;
}

bool CPFA_loop_functions::IsOutOfBounds(argos::CVector2 p, size_t length, size_t width) {
	argos::CVector2 placementPosition = p;
  argos::Real x_min, x_max, y_min, y_max;

  argos::Real foodOffset   = 3.0 * FoodRadius;

  if (FoodPDF == 0) {
    argos::Real widthOffset  = 3.0 * FoodRadius * (argos::Real)width;
    argos::Real lengthOffset = 3.0 * FoodRadius * (argos::Real)length;

    x_min = p.GetX() - FoodRadius;
    x_max = p.GetX() + FoodRadius + widthOffset;

    y_min = p.GetY() - FoodRadius;
    y_max = p.GetY() + FoodRadius + lengthOffset;
  } else {
    x_min = std::floor(p.GetX());
    x_max = x_min + width;

    y_min = std::floor(p.GetY());
    y_max = y_min + length;
  }

	if((x_min < (ForageRangeX.GetMin() + FoodRadius))
			|| (x_max > (ForageRangeX.GetMax() - FoodRadius)) ||
			(y_min < (ForageRangeY.GetMin() + FoodRadius)) ||
			(y_max > (ForageRangeY.GetMax() - FoodRadius)))
	{
		return true;
	}

  argos::Real offset;
  if (FoodPDF == 0) {
    offset = foodOffset;
  } else {
    offset = 1;
  }

	for(size_t j = 0; j < length; j++) {
		for(size_t k = 0; k < width; k++) {
			if(IsCollidingWithFood(placementPosition)) return true;
			if(IsCollidingWithNest(placementPosition)) return true;
			placementPosition.SetX(placementPosition.GetX() + offset);
		}

		placementPosition.SetX(placementPosition.GetX() - (width * offset));
		placementPosition.SetY(placementPosition.GetY() + offset);
	}

	return false;
}

bool CPFA_loop_functions::IsCollidingWithNest(argos::CVector2 p) {
  if (FoodPDF == 0) {
    return IsCollidingWithNestDiscrete(p);
  } else {
    return IsCollidingWithNestPDF(p);
  }
}

bool CPFA_loop_functions::IsCollidingWithNestDiscrete(argos::CVector2 p) {
	argos::Real nestRadiusPlusBuffer = NestRadius + FoodRadius;
	argos::Real NRPB_squared = nestRadiusPlusBuffer * nestRadiusPlusBuffer;

	return ((p - NestPosition).SquareLength() < NRPB_squared);
}

bool CPFA_loop_functions::IsCollidingWithNestPDF(argos::CVector2 p) {
  return ((argos::CVector2(p.GetX(), 0) - NestPosition).SquareLength() <= 1 ||
      (argos::CVector2(0, p.GetY()) - NestPosition).SquareLength() <= 1);
}

bool CPFA_loop_functions::IsCollidingWithFood(argos::CVector2 p) {
  if (FoodPDF == 0) {
    return IsCollidingWithDiscreteFood(p);
  } else {
    return IsCollidingWithPDFFood(p);
  }
}

bool CPFA_loop_functions::IsCollidingWithDiscreteFood(argos::CVector2 p) {
	argos::Real foodRadiusPlusBuffer = 2.0 * FoodRadius;
	argos::Real FRPB_squared = foodRadiusPlusBuffer * foodRadiusPlusBuffer;

	for(size_t i = 0; i < FoodList.size(); i++) {
		if((p - FoodList[i].Position()).SquareLength() < FRPB_squared) return true;
	}

	return false;
}

bool CPFA_loop_functions::IsCollidingWithPDFFood(argos::CVector2 p) {
	for(size_t i = 0; i < SectorList.size(); ++i) {
    if (SectorList.at(i).ContainsPoint(p)) {
      return true;
    }
	}
	return false;
}

unsigned int CPFA_loop_functions::getNumberOfRobots() {
	return GetSpace().GetEntitiesByType("foot-bot").size();
}

double CPFA_loop_functions::getProbabilityOfSwitchingToSearching() {
	return ProbabilityOfSwitchingToSearching;
}

double CPFA_loop_functions::getProbabilityOfReturningToNest() {
	return ProbabilityOfReturningToNest;
}

// Value in Radians
double CPFA_loop_functions::getUninformedSearchVariation() {
	return UninformedSearchVariation.GetValue();
}

double CPFA_loop_functions::getRateOfInformedSearchDecay() {
	return RateOfInformedSearchDecay;
}

double CPFA_loop_functions::getRateOfSiteFidelity() {
	return RateOfSiteFidelity;
}

double CPFA_loop_functions::getRateOfLayingPheromone() {
	return RateOfLayingPheromone;
}

double CPFA_loop_functions::getRateOfPheromoneDecay() {
	return RateOfPheromoneDecay;
}

argos::Real CPFA_loop_functions::getSimTimeInSeconds() {
	int ticks_per_second = GetSimulator().GetPhysicsEngine("Default").GetInverseSimulationClockTick();
	float sim_time = GetSpace().GetSimulationClock();
	return sim_time/ticks_per_second;
}

void CPFA_loop_functions::SetTrial(unsigned int v) {
}

void CPFA_loop_functions::setScore(double s) {
	score = s;
	if (score >= FoodItemCount) {
		PostExperiment();
	}
}

double CPFA_loop_functions::Score() {	
	return score;
}

void CPFA_loop_functions::ConfigureFromGenome(Real* g)
{
	// Assign genome generated by the GA to the appropriate internal variables.
	ProbabilityOfSwitchingToSearching = g[0];
	ProbabilityOfReturningToNest      = g[1];
	UninformedSearchVariation.SetValue(g[2]);
	RateOfInformedSearchDecay         = g[3];
	RateOfSiteFidelity                = g[4];
	RateOfLayingPheromone             = g[5];
	RateOfPheromoneDecay              = g[6];
}

void CPFA_loop_functions::AssociateWithCluster(size_t cid) {
  //cluster_association_counts[cid]++;
}

void CPFA_loop_functions::DissociateFromCluster(size_t cid) {
  //cluster_association_counts[cid]--;
}

void CPFA_loop_functions::LogControllerTarget(argos::CVector2 t) {
	ofstream log_output_stream;
  std::string fname = "targets_";
  fname += std::to_string(RandomSeed);
  fname += ".csv";
	log_output_stream.open(fname, ios::app);
  log_output_stream << GetSpace().GetSimulationClock() << ","
    << t.GetX() << "," << t.GetY() << std::endl;
	log_output_stream.close();
}

bool CPFA_loop_functions::IsFoodHere(const argos::CVector2& p, const argos::Real& tol) {
  bool foodIsHere;
  if (FoodPDF == 0) {
    foodIsHere = IsDiscreteFoodHere(p, tol);
  } else {
    foodIsHere = IsPDFFoodHere(p);
  }
  return foodIsHere;
}

bool CPFA_loop_functions::IsDiscreteFoodHere(const argos::CVector2& p, const argos::Real& tol) {
  bool foundFood = false;
  std::vector<SmartFood> newFoodList;
  std::vector<argos::CColor> newFoodColoringList;
  size_t i = 0, j = 0;
  for(i = 0; i < FoodList.size(); i++) {
    if((p - FoodList[i].Position()).SquareLength() < tol) {
      // We found food! Calculate the nearby food density.
      foundFood = true;

      //RegisterWithCluster(LoopFunctions->FoodList[i].GetClusterID());
      //CPFA_state = SURVEYING;

      // original sets j=i+1 to remove this seed. Setting j=i makes food
      // constant
      j = i + 1;
      //j = i;

      break;
    } else {
      //Return this unfound-food position to the list
      newFoodList.push_back(FoodList[i]);
      newFoodColoringList.push_back(FoodColoringList[i]);
    }
  }

  for(; j < FoodList.size(); j++) {
    newFoodList.push_back(FoodList[j]);
    newFoodColoringList.push_back(FoodColoringList[j]);
  }

  if(foundFood) {
    FoodList = newFoodList;
    FoodColoringList = newFoodColoringList;
  } 

  return foundFood;
}

bool CPFA_loop_functions::IsPDFFoodHere(const argos::CVector2& p) {
  bool foundFood = false;
  argos::CRange<argos::Real> prob(0,1);
  for (size_t i = 0; i < SectorList.size(); ++i) {
    if (SectorList.at(i).ContainsPoint(p)) {
      argos::Real dart = RNG->Uniform(prob);
      argos::LOG << "Dart: " << dart << "\n";
      if (dart < SectorList.at(i).GetProbability()) {
        argos::LOG << "Awarding seed.\n";
        foundFood = true;
      }
      break;
    }
  }

  return foundFood;
}

size_t CPFA_loop_functions::ResourceDensity(const argos::CVector2& p) {
  size_t foodFound;
  if (FoodPDF == 0) {
    foodFound = DiscreteResourceDensity(p);
  } else {
    foodFound = PDFResourceDensity(p);
  }
  return foodFound;
}


size_t CPFA_loop_functions::DiscreteResourceDensity(const argos::CVector2& p) {

  size_t foodFound = 0;
	argos::CVector2 distance;

	for(size_t i = 0; i < FoodList.size(); ++i) {
		distance = p - FoodList[i].Position();

		if(distance.SquareLength() < SearchRadiusSquared*2) {
			foodFound++;
			FoodColoringList[i] = argos::CColor::ORANGE;
      //ResourceDensityDelay = SimulationTick() + SimulationTicksPerSecond() * 10;
			ResourceDensityDelay = GetSpace().GetSimulationClock() +
        GetSimulator().GetPhysicsEngine("default").GetInverseSimulationClockTick() * 10;
		}
	}

  return foodFound;
}

size_t CPFA_loop_functions::PDFResourceDensity(const argos::CVector2& p) {

  size_t foodFound = 0;
  size_t num_samples = 8;

  argos::CRange<argos::Real> prob(0,1);
  for (size_t i = 0; i < SectorList.size(); ++i) {
    if (SectorList.at(i).ContainsPoint(p)) {
      for (size_t j = 0; j < num_samples; ++j) {
        if (RNG->Uniform(prob) < SectorList.at(i).GetProbability()) {
          foodFound++;
        }
      }
    }
  }

  return foodFound;
}


REGISTER_LOOP_FUNCTIONS(CPFA_loop_functions, "CPFA_loop_functions")
