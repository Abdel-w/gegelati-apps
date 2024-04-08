#include <iostream>
#include <numeric>
#include <thread>
#include <atomic>
#include <chrono>
#include <inttypes.h>
#define _USE_MATH_DEFINES // To get M_PI
#include <math.h>

#include "pendulum.h"
#include "render.h"
#include "instructions.h"

#include "tools.h"
int main() {
    //Create a folder for storing the results of the training experimentations
    char* saveFolderPath = createFolderWithCurrentTime("/logs/FL/2Agents/");

    // Export the src/instructions.cpp file and the params.json file to
    // keep traceability when looking at the logs
    char filename_src[BUFFER_SIZE];
    char filename_dest[BUFFER_SIZE];

    sprintf(filename_src, "%s/src/Learn/instructions.cpp", ROOT_DIR);
    sprintf(filename_dest, "%s/instructions.cpp", saveFolderPath);
    copyFile(filename_src, filename_dest);

    sprintf(filename_src, "%s/params.json", ROOT_DIR);
    sprintf(filename_dest, "%s/params.json", saveFolderPath);
    copyFile(filename_src, filename_dest);

    std::cout << "Start Pendulum application." << std::endl;

	// Create the instruction set for programs
	Instructions::Set set;
	fillInstructionSet(set);

	// Set the parameters for the learning process.
	// (Controls mutations probability, program lengths, and graph size
	// among other things)
	// Loads them from the file params.json
	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json", params);
#ifdef NB_GENERATIONS
	params.nbGenerations = NB_GENERATIONS;
#endif

	// Instantiate the LearningEnvironment
	Pendulum pendulumLE({ 0.05, 0.1, 0.2, 0.4, 0.6, 0.8, 1.0 });

	std::cout << "Number of threads: " << params.nbThreads << std::endl;

    // Instantiate and initialize the FLAgent (LA)
    Learn::FLAgentManager<Learn::LearningAgent> laM(2,pendulumLE, set, params);
    laM.connectAgentsPseudoRandomly();
	// Instantiate and init the learning agent
//	Learn::ParallelLearningAgent la(pendulumLE, set, params);
//	la.init();

	// Start a thread for controlling the loop
#ifndef NO_CONSOLE_CONTROL
	// Console
	std::atomic<bool> exitProgram = true; // (set to false by other thread) 
	std::atomic<bool> toggleDisplay = true;
	std::atomic<bool> doDisplay = false;
	std::atomic<uint64_t> generation = 0;
    const TPG::TPGVertex* bestRoot = NULL;

	std::thread threadDisplay(Render::controllerLoop, std::ref(exitProgram), std::ref(toggleDisplay), std::ref(doDisplay),
		&bestRoot, std::ref(set), std::ref(pendulumLE), std::ref(params), std::ref(generation));

	while (exitProgram); // Wait for other thread to print key info.
#else 
	std::atomic<bool> exitProgram = false; // (set to false by other thread) 
	std::atomic<bool> toggleDisplay = false;
#endif

	// Basic logger
	//Log::LABasicLogger basicLogger(la);
    Log::LABasicLogger basicLogger(*laM.agents[0]);
    //Log::LABasicLogger basicLoggera(*laM.agents[1]);


    //CSV logger

    std::vector<std::ofstream> CSVof;
    char* CSVfilename[laM.agents.size()] ;
    char buff[BUFFER_SIZE];
    for (int i = 0; i < laM.agents.size(); ++i) {
        //init agents
        laM.agents[i]->init();
        sprintf(buff, "/training_data_agent%01d.csv",i);
        CSVfilename[i] = concatenateStrings(saveFolderPath, buff);

        CSVof.push_back(std::ofstream());
        CSVof[i].open(CSVfilename[i], std::ios::out);
        if (!CSVof[i].is_open())
        {
            std::cout << "Cannot open file " << CSVfilename[i] << std::endl;
            exit(0);
        }
    }

    //Log::LABasicLogger csvLogger(la, CSVof, 0, ",");
    std::vector< Log::LABasicLogger> csvLogger;
    for (int i = 0; i < laM.agents.size(); ++i) {
        csvLogger.push_back(Log::LABasicLogger(*laM.agents[i], CSVof[i], 0, ","));
    }

    // Create an exporter for all graphs
    std::vector<File::TPGGraphDotExporter*> dotExporters;
    for (int i = 0; i < laM.agents.size(); ++i) {
        sprintf(buff, "%s/Graphs/out%d_0000.dot", saveFolderPath,i);
        dotExporters.push_back(new File::TPGGraphDotExporter(buff, *laM.agents[i]->getTPGGraph()));
    }
	// Logging best policy stat.
	std::ofstream stats;
	stats.open("bestPolicyStats.md");
    for (int i = 0; i < laM.agents.size(); ++i) {
        Log::LAPolicyStatsLogger policyStatsLogger(*laM.agents[i], stats);
    }

	// Export parameters before starting training.
	// These may differ from imported parameters because of LE or machine specific
	// settings such as thread count of number of actions.
	File::ParametersParser::writeParametersToJson("exported_params.json", params);

	// Train for params.nbGenerations generations
    uint64_t aggregationNumber = 0;
    for (uint64_t i = 0; i < params.nbGenerations && !exitProgram; i++) {

        for (int j = 0; j < laM.agents.size(); ++j) {
            sprintf(buff, "%s/Graphs/out%01d_%04ld.dot", saveFolderPath, j, i);
            dotExporters.at(j)->setNewFilePath(buff);
            dotExporters.at(j)->print();
        }

		//la.trainOneGeneration(i);
        // Train one generation
        if (i == laM.agents[0]->params.nbGenerationPerAggregation * (aggregationNumber+1))
        {
            laM.exchangeBestBranchs();
            //for each agent copy all received branchs in the TPGGraph
            std::for_each(laM.agents.begin(),laM.agents.end(),
                          [](Learn::FLAgent<Learn::LearningAgent> *agent){

                              // copy all branchs
                              for (auto branch : agent->getBestBranch()){
                                  Mutator::BranchMutator::copyBranch(branch, *(agent->getTPGGraph()));
                              }
                              //Empty Epmty the bestBranchs vector to get ready to receive new ones
                              agent->emptyBranchs();
                          });
            aggregationNumber++;
        }
        for (auto agent : laM.agents)
        {
            agent->trainOneGeneration(i);
        }

#ifndef NO_CONSOLE_CONTROL
		generation = i;
		if (toggleDisplay && !exitProgram) {
			bestRoot = laM.agents[0]->getBestRoot().first;
			doDisplay = true;
			while (doDisplay && !exitProgram);
		}
#endif
	}

	// Keep best policy
    // la.keepBestPolicy();

    // Clear introns instructions
    //la.getTPGGraph()->clearProgramIntrons();
    TPG::PolicyStats ps;
    for (int i = 0; i < laM.agents.size(); ++i) {
        // Keep best policy
        laM.agents[i]->keepBestPolicy();
        // Clear introns instructions
        laM.agents[i]->getTPGGraph()->clearProgramIntrons();
        // Export the graph
        sprintf(buff, "%s/Graphs/out%01d_best.dot", saveFolderPath, i);
        dotExporters.at(i)->setNewFilePath(buff);
        dotExporters.at(i)->print();


        // Export stats file to logs directory
        ps.setEnvironment(laM.agents[i]->getTPGGraph()->getEnvironment());
        ps.analyzePolicy(laM.agents[i]->getBestRoot().first);
        std::ofstream bestStats;
        sprintf(buff, "%s/out%01d_best_stats.md", saveFolderPath,i);
        bestStats.open("buff");
        bestStats << ps;
        bestStats.close();
        stats.close();

    }



	// Export the graph
//    sprintf(buff, "%s/Graphs/out_best.dot", saveFolderPath);
//    dotExporter.setNewFilePath(buff);
//	dotExporter.print();

    // Export stats file to logs directory
//	TPG::PolicyStats ps;
//	ps.setEnvironment(laM.agents[0].getTPGGraph()->getEnvironment());
//	ps.analyzePolicy(la.getBestRoot().first);
//	std::ofstream bestStats;
//    sprintf(buff, "%s/out_best_stats.md", saveFolderPath);
//	bestStats.open("buff");
//	bestStats << ps;
//	bestStats.close();
//	stats.close();

	// cleanup
	for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
		delete (&set.getInstruction(i));
	}

    for (int i = 0; i < dotExporters.size(); ++i) {
        delete dotExporters[i];
    }
    delete[] saveFolderPath;
#ifndef NO_CONSOLE_CONTROL
	// Exit the thread
	std::cout << "Exiting program, press a key then [enter] to exit if nothing happens.";
	threadDisplay.join();
#endif

	return 0;
}