#include <iostream>
#include <numeric>
#include <thread>
#include <atomic>
#include <chrono>
#include <inttypes.h>
#include <cstdlib>
#include <gegelati.h>

#include "mnist.h"
#include "tools.h"
void getKey(std::atomic<bool>& exit, std::atomic<bool>& printStats) {
	std::cout << std::endl;
	std::cout << "Press `q` then [Enter] to exit." << std::endl;
	std::cout << "Press `p` then [Enter] to print classification statistics of the best root." << std::endl;
	std::cout.flush();

	exit = false;

	while (!exit) {
		char c;
		std::cin >> c;
		switch (c) {
		case 'q':
		case 'Q':
			exit = true;
			break;
		case 'p':
		case 'P':
			printStats = true;
			break;
		default:
			printf("Invalid key '%c' pressed.", c);
			std::cout.flush();
		}
	}

	printf("Program will terminate at the end of next generation.\n");
	std::cout.flush();
}

int main(int argc, char *argv[]) {

    // Extract the seed from the command line argument
    uint64_t init_seed = std::atoi(argv[1]);

    //Create a folder for storing the results of the training experimentations
    char file_name_nb_agent[BUFFER_SIZE];
    sprintf(file_name_nb_agent, "/logs/FL/%s_Agents/", argv[2]);
    char* saveFolderPath = createFolderWithCurrentTime(file_name_nb_agent, argv[3] );

    // Export the src/instructions.cpp file and the params.json file to
    // keep traceability when looking at the logs
    char filename_src[BUFFER_SIZE];
    char filename_dest[BUFFER_SIZE];

    sprintf(filename_src, "%s/params.json", ROOT_DIR);
    sprintf(filename_dest, "%s/params.json", saveFolderPath);
    copyFile(filename_src, filename_dest);

    std::cout << "Start MNIST application." << std::endl;

	// Create the instruction set for programs
	Instructions::Set set;
	auto minus = [](double a, double b)->double {return a - b; };
	auto add = [](double a, double b)->double {return a + b; };
	auto mult = [](double a, double b)->double {return a * b; };
	auto div = [](double a, double b)->double {return a / b; };
	auto max = [](double a, double b)->double {return std::max(a, b); };
	auto ln = [](double a)->double {return std::log(a); };
	auto exp = [](double a)->double {return std::exp(a); };
	auto multByConst = [](double a, Data::Constant c)->double {return a * (double)c / 10.0; };
	auto sobelMagn = [](const double a[3][3])->double {
		double result = 0.0;
		double gx =
			-a[0][0] + a[0][2]
			- 2.0 * a[1][0] + 2.0 * a[1][2]
			- a[2][0] + a[2][2];
		double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2]
			+ a[2][0] + 2.0 * a[2][1] + a[2][2];
		result = sqrt(gx * gx + gy * gy);
		return result;
	};

	auto sobelDir = [](const double a[3][3])->double {
		double result = 0.0;
		double gx =
			-a[0][0] + a[0][2]
			- 2.0 * a[1][0] + 2.0 * a[1][2]
			- a[2][0] + a[2][2];
		double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2]
			+ a[2][0] + 2.0 * a[2][1] + a[2][2];
		result = std::atan(gy / gx);
		return result;
	};

	set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(mult)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(div)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
	set.add(*(new Instructions::LambdaInstruction<double>(exp)));
	set.add(*(new Instructions::LambdaInstruction<double>(ln)));
	set.add(*(new Instructions::LambdaInstruction<double, Data::Constant>(multByConst)));
	set.add(*(new Instructions::LambdaInstruction<const double[3][3]>(sobelMagn)));
	set.add(*(new Instructions::LambdaInstruction<const double[3][3]>(sobelDir)));

	// Set the parameters for the learning process.
	// (Controls mutations probability, program lengths, and graph size
	// among other things)
	// Loads them from the file params.json
	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json", params);
    // change the nbGenerationPerAggregation parame
    if ( argc >= 3){
        params.maxNbOfConnections = (uint64_t) (std::atoi(argv[3]) - 1) ;
        if (argc >= 4)
            params.nbGenerationPerAggregation = (uint64_t) std::atoi(argv[3]);
    }
#ifdef NB_GENERATIONS
	params.nbGenerations = NB_GENERATIONS;
#endif // !NB_GENERATIONS

	// Instantiate the LearningEnvironment
	MNIST mnistLE;

	std::cout << "Number of threads: " << params.nbThreads << std::endl;

//	// Instantiate and init the learning agent
//	Learn::ClassificationLearningAgent la(mnistLE, set, params);
//	la.init();
    //Instantiate, connect and init the learning agents with different seeds
    Learn::FLAgentManager<Learn::ClassificationLearningAgent<Learn::ParallelLearningAgent>> laM(std::atoi(argv[2]),mnistLE, set, params);
    Mutator::RNG rng(init_seed);
    laM.connectAgentsPseudoRandomly(rng.getUnsignedInt64(0, 20));
    for (int i = 0; i < laM.nbAgents; ++i) {
        laM.agents[i]->init(init_seed);
        rng.setSeed(init_seed);
        init_seed = rng.getUnsignedInt64(0, 20);
    }
    // laM.connectAgents(laM.agents[0],laM.agents[1]);
    // Instantiate and init the learning agent

	// Start a thread for controlling the loop
//#ifndef NO_CONSOLE_CONTROL
//	std::atomic<bool> exitProgram = true; // (set to false by other thread)
//	std::atomic<bool> printStats = false;
//
//	std::thread threadKeyboard(getKey, std::ref(exitProgram), std::ref(printStats));
//
//	while (exitProgram); // Wait for other thread to print key info.
//#else
//	std::atomic<bool> exitProgram = false; // (set to false by other thread)
//	std::atomic<bool> printStats = false;
//#endif

	// Adds a logger to the LA (to get statistics on learning) on std::cout
	//Log::LABasicLogger logCout(la);
    //Log::LABasicLogger basicLogger(*laM.agents[0]);
    //CSV logger
//    char* CSVfilename = concatenateStrings(saveFolderPath, "/training_data.csv");
//    std::ofstream CSVof;
//    CSVof.open (CSVfilename, std::ios::out);
//    if (!CSVof.is_open())
//    {
//        std::cout << "Cannot open file " << CSVfilename << std::endl;
//        exit(0);
//    }
//    Log::LABasicLogger csvLogger(la, CSVof, 0, ",");
//
//    char buff[BUFFER_SIZE];
//    sprintf(buff, "%s/Graphs/out_0000.dot", saveFolderPath);
//    File::TPGGraphDotExporter dotExporter(buff, *la.getTPGGraph());
//    // Logging best policy stat.
//    std::ofstream stats;
//    stats.open("bestPolicyStats.md");
//    Log::LAPolicyStatsLogger policyStatsLogger(la, stats);


    //CSV logger

    std::vector<std::ofstream> CSVof;
    char* CSVfilename[laM.agents.size()] ;
    char buff[BUFFER_SIZE];
    for (int i = 0; i < laM.nbAgents; ++i) {
        sprintf(buff, "/training_data_agent%01d.csv",i);
        CSVfilename[i] = concatenateStrings(saveFolderPath, buff);

        CSVof.emplace_back(CSVfilename[i], std::ios::out);
        if (!CSVof[i].is_open())
        {
            std::cout << "Cannot open file " << CSVfilename[i] << std::endl;
            exit(0);
        }
    }

    std::vector<Log::LABasicLogger> csvLogger;
    csvLogger.reserve(laM.agents.size()); //Danger!!!!!!!!!!!!!
    for (int i = 0; i < laM.agents.size(); ++i) {
        csvLogger.emplace_back(*laM.agents[i], CSVof[i], 0, ",");
    }
    // Create an exporter for all graphs
    std::vector<File::TPGGraphDotExporter*> dotExporters;
    for (int i = 0; i < laM.nbAgents; ++i) {
        sprintf(buff, "%s/Graphs/out%d_0000.dot", saveFolderPath,i);
        dotExporters.push_back(new File::TPGGraphDotExporter(buff, *laM.agents[i]->getTPGGraph()));
    }
    // Logging best policy stat.
    std::vector<std::ofstream> stats;
    stats.reserve(laM.nbAgents);//Danger!!!!!!!!!!!!!
    std::vector<Log::LAPolicyStatsLogger> policyStatsLogger;
    policyStatsLogger.reserve(laM.nbAgents);//Danger!!!!!!!!!!!!!
    for (int i = 0; i < laM.nbAgents; ++i) {
        sprintf(buff,"bestPolicyStats_%01d.md",i);
        stats.emplace_back(buff,std::ios::out);
        stats[i].open(buff,std::ios::out);
        policyStatsLogger.emplace_back(*laM.agents[i], stats[i]);

    }

	// Export parameters before starting training.
	// These may differ from imported parameters because of LE or machine specific
	// settings such as thread count of number of actions.
	File::ParametersParser::writeParametersToJson("exported_params.json", params);

	// Train for NB_GENERATIONS generations
    uint64_t aggregationNumber = 0;
    for (uint64_t i = 0; i < params.nbGenerations ; i++) {

        for (int j = 0; j < laM.nbAgents; ++j) {
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
                          [](auto *agent){

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

//		if (printStats) {
//			mnistLE.printClassifStatsTable(laM.agents[0]->getTPGGraph()->getEnvironment(), laM.agents[0]->getBestRoot().first);
//			printStats = false;
//		}

	}


    TPG::PolicyStats ps;
    for (int i = 0; i < laM.nbAgents; ++i) {
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
        sprintf(buff, "%s/out%01d_best_stats.md", saveFolderPath, i);
        bestStats.open("buff");
        bestStats << ps;
        bestStats.close();
        stats[i].close();
    }

    // Print stats one last time
//    mnistLE.printClassifStatsTable(laM.agents[0]->getTPGGraph()->getEnvironment(), laM.agents[0]->getBestRoot().first);

    // cleanup
    for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
        delete (&set.getInstruction(i));
    }

    for (int i = 0; i < dotExporters.size(); ++i) {
        delete dotExporters[i];
    }
    delete[] saveFolderPath;

//#ifndef NO_CONSOLE_CONTROL
//	// Exit the thread
//	std::cout << "Exiting program, press a key then [enter] to exit if nothing happens.";
//	threadKeyboard.join();
//#endif

	return 0;
}
