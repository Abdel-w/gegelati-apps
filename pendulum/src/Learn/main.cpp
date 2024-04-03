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
    char* saveFolderPath = createFolderWithCurrentTime();

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

	// Instantiate and init the learning agent
	Learn::LearningAgent la(pendulumLE, set, params);
	la.init();

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
	Log::LABasicLogger basicLogger(la);

    //CSV logger
    char* CSVfilename = concatenateStrings(saveFolderPath, "/training_data.csv");
    std::ofstream CSVof;
    CSVof.open (CSVfilename, std::ios::out);
    if (!CSVof.is_open())
    {
        std::cout << "Cannot open file " << CSVfilename << std::endl;
        exit(0);
    }
    Log::LABasicLogger csvLogger(la, CSVof, 0, ",");


    // Create an exporter for all graphs
    char buff[BUFFER_SIZE];
    sprintf(buff, "%s/Graphs/out_0000.dot", saveFolderPath);
    File::TPGGraphDotExporter dotExporter(buff, *la.getTPGGraph());

	// Logging best policy stat.
	std::ofstream stats;
	stats.open("bestPolicyStats.md");
	Log::LAPolicyStatsLogger policyStatsLogger(la, stats);

	// Export parameters before starting training.
	// These may differ from imported parameters because of LE or machine specific
	// settings such as thread count of number of actions.
	File::ParametersParser::writeParametersToJson("exported_params.json", params);

	// Train for params.nbGenerations generations
    for (uint64_t i = 0; i < params.nbGenerations && !exitProgram; i++) {
        sprintf(buff, "%s/Graphs/out_%04ld.dot", saveFolderPath, i);
		dotExporter.setNewFilePath(buff);
		dotExporter.print();

		la.trainOneGeneration(i);

#ifndef NO_CONSOLE_CONTROL
		generation = i;
		if (toggleDisplay && !exitProgram) {
			bestRoot = la.getBestRoot().first;
			doDisplay = true;
			while (doDisplay && !exitProgram);
		}
#endif
	}

	// Keep best policy
	la.keepBestPolicy();

	// Clear introns instructions
	la.getTPGGraph()->clearProgramIntrons();

	// Export the graph
    sprintf(buff, "%s/Graphs/out_best.dot", saveFolderPath);
    dotExporter.setNewFilePath(buff);
	dotExporter.print();

    // Export stats file to logs directory
	TPG::PolicyStats ps;
	ps.setEnvironment(la.getTPGGraph()->getEnvironment());
	ps.analyzePolicy(la.getBestRoot().first);
	std::ofstream bestStats;
    sprintf(buff, "%s/out_best_stats.md", saveFolderPath);
	bestStats.open("buff");
	bestStats << ps;
	bestStats.close();
	stats.close();

	// cleanup
	for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
		delete (&set.getInstruction(i));
	}

    delete[] saveFolderPath;
#ifndef NO_CONSOLE_CONTROL
	// Exit the thread
	std::cout << "Exiting program, press a key then [enter] to exit if nothing happens.";
	threadDisplay.join();
#endif

	return 0;
}