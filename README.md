# VehicleNN2
Self Driving Vehicle Setup in Unreal Engine 4 (In Both C++ and Blueprints)

# The files you're looking for
### C++
Path: VehicleNN2/Source/VehicleNN2/NN/ <br>
Link: https://github.com/Vieper1/VehicleNN2/tree/master/Source/VehicleNN2/NN

### Blueprints
Path: VehicleNN2/Content/VehicleAdvBP/Blueprints/VehicleNNBlueprint5_5IP_Both_Simplified.uasset <br>
Link: https://github.com/Vieper1/VehicleNN2/blob/master/Content/VehicleAdvBP/Blueprints/VehicleNNBlueprint5_5IP_Both_Simplified.uasset

# Config
In the CustomVehicle_NN_5_2.cpp file
- To toggle Training Mode, change the following bool value: ShouldTrain

The current build doesn't have the Throttle Inputs rigged to the Network, but can be done in a few steps, same as the Steering Inputs

# Future Works
1. Create a simple 2-Parallel-Walls-Track-Builder Blueprint to train the Network for various tracks
2. Add a save file to store training data

# To run the project
1. Simply download the project
2. Go to directory
3. Open VehicleNN2.uproject
